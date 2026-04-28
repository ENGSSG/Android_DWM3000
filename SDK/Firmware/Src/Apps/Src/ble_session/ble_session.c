/**
 * @file      ble_session.c
 *
 * @brief     BLE-OOB driven FiRa Controlee session bring-up.
 *
 *            UCI is brought up at boot from main(). When a 16-byte OOB payload
 *            arrives on FFF1 the BLE callback raises a signal; our worker task
 *            (created in ble_session_init) tears UCI down and starts a FiRa
 *            Static-STS Controlee session via the programmatic fira_helper API.
 *            On BLE disconnect the worker stops the session and brings UCI
 *            back up, so USB UCI is the resting state.
 *
 *            Modelled on Projects/FreeRTOS/QANI/Common/src/fira/fira_niq.c.
 */

#include "ble_session.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "app.h"
#include "common_fira.h"
#include "fira_app_config.h"
#include "fira_default_params.h"
#include "net/fira_region_params.h"
#include "uwbmac/fira_helper.h"
#include "uwbmac/uwbmac.h"
#include "qlog.h"
#include "qirq.h"
#include "qmalloc.h"
#include "qsignal.h"
#include "qthread.h"
#include "SEGGER_RTT.h"
#include "int_priority.h"

extern const app_definition_t helpers_uci_node[];
/* Provided by Src/Apps/Src/uci/task_uci/task_uci.c (header is private to uci lib). */
extern void uci_terminate(void);
extern void uci_helper(void const *arg);

/* AndroidX UWB CONFIG_MULTICAST_DS_TWR + RANGING_UPDATE_RATE_FREQUENT.
 * Pulled from AOSP ConfigurationManager / Utils.RangingTimingParams for
 * CONFIG_ID_2 (CONFIG_MULTICAST_DS_TWR): 120ms block, 2400 RSTU slot,
 * 20 slots/round, hopping enabled. Every value here must match the phone
 * modem's programming or polls land outside our RX window → status=33. */
#define BLE_SESSION_DEFAULT_SLOT_RSTU   2400u
#define BLE_SESSION_DEFAULT_BLOCK_MS    120u
#define BLE_SESSION_DEFAULT_ROUND_SLOTS 20u
#define BLE_SESSION_TASK_STACK_BYTES    4096u

struct ble_session_params
{
    uint32_t session_id;
    uint8_t channel;
    uint8_t preamble;
    uint8_t session_key[8];
    uint16_t controller_addr;
    uint16_t conn_handle;
    bool valid;
};

/* Latest pending OOB payload, filled by the BLE callback, drained by worker.
 * Two edge-triggered flags so a fast disconnect right after FFF1 still gets
 * the session up before tearing it down — see ble_session_worker. */
static volatile struct ble_session_params m_pending;
static volatile bool m_start_pending = false;
static volatile bool m_stop_pending = false;

/* Owned by the worker task. */
static struct qthread *m_worker = NULL;
static struct qsignal *m_signal = NULL;
static struct uwbmac_context *m_uwbmac_ctx = NULL;
static struct fira_context m_fira_ctx;
static uint32_t m_session_handle = 0;
static bool m_session_started = false;
static fira_param_t m_fira_param;

static void ble_session_worker(void *arg);
static void start_fira(const struct ble_session_params *p);
static void stop_fira(void);
static void fira_notification_cb(enum fira_helper_cb_type cb_type, const void *content, void *user_data);

static void decode_oob_payload(const uint8_t *p, struct ble_session_params *out)
{
    out->session_id = ((uint32_t)p[0]) | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    out->channel = p[4];
    out->preamble = p[5];
    memcpy(out->session_key, &p[6], 8);
    out->controller_addr = ((uint16_t)p[14]) | ((uint16_t)p[15] << 8);
}

void ble_session_init(void)
{
    memset((void *)&m_pending, 0, sizeof(m_pending));
    memset(&m_fira_param, 0, sizeof(m_fira_param));

    m_signal = qsignal_init();
    if (!m_signal)
    {
        QLOGE("ble_session: qsignal_init failed");
        return;
    }

    uint8_t *stack = qmalloc(BLE_SESSION_TASK_STACK_BYTES);
    if (!stack)
    {
        QLOGE("ble_session: qmalloc stack failed");
        return;
    }

    m_worker = qthread_create(ble_session_worker, NULL, "BLES", stack, BLE_SESSION_TASK_STACK_BYTES, PRIO_StartDefaultTask);
    if (!m_worker)
    {
        QLOGE("ble_session: qthread_create failed");
        return;
    }

    QLOGI("ble_session: worker ready");
}

void ble_session_submit_params(const uint8_t payload[BLE_SESSION_PARAMS_LEN], uint16_t conn_handle)
{
    struct ble_session_params decoded;
    decode_oob_payload(payload, &decoded);
    decoded.conn_handle = conn_handle;
    decoded.valid = true;

    uint32_t key = qirq_lock();
    m_pending = decoded;
    m_start_pending = true;
    qirq_unlock(key);

    QLOGI("BLES sid=%02X%02X%02X%02X",
          payload[0], payload[1], payload[2], payload[3]);
    QLOGI("BLES ch=%u pc=%u dst=0x%04X",
          (unsigned)payload[4], (unsigned)payload[5],
          (unsigned)decoded.controller_addr);
    QLOGI("BLES key0=%02X%02X%02X%02X",
          payload[6], payload[7], payload[8], payload[9]);
    QLOGI("BLES key1=%02X%02X%02X%02X",
          payload[10], payload[11], payload[12], payload[13]);

    if (m_signal)
    {
        qsignal_raise(m_signal, 1);
    }
}

void ble_session_submit_disconnect(void)
{
    uint32_t key = qirq_lock();
    /* Only request a stop if there's something to stop — either an in-flight
     * start request that hasn't been serviced yet, or a session already running. */
    bool need_stop = m_start_pending || m_session_started;
    if (need_stop)
    {
        m_stop_pending = true;
    }
    qirq_unlock(key);

    if (!need_stop)
    {
        return;
    }

    QLOGI("ble_session: disconnect, returning to UCI");
    if (m_signal)
    {
        qsignal_raise(m_signal, 1);
    }
}

static void ble_session_worker(void *arg)
{
    (void)arg;
    int last_value = 0;

    while (1)
    {
        qsignal_wait(m_signal, &last_value, QOSAL_WAIT_FOREVER);

        /* Drain start, then stop, in order — so a fast disconnect right after
         * FFF1 still gets the session up before tearing it down. */
        bool do_start;
        struct ble_session_params snapshot;
        uint32_t key = qirq_lock();
        do_start = m_start_pending;
        m_start_pending = false;
        snapshot = (struct ble_session_params)m_pending;
        qirq_unlock(key);

        if (do_start && !m_session_started && snapshot.valid)
        {
            SEGGER_RTT_WriteString(0, "BLES: start, calling uci_terminate\r\n");
            QLOGI("ble_session: stopping UCI to start FiRa session");
            uci_terminate();
            SEGGER_RTT_WriteString(0, "BLES: uci_terminate returned, calling start_fira\r\n");
            QLOGI("ble_session: uci_terminate done");
            start_fira(&snapshot);
            SEGGER_RTT_WriteString(0, "BLES: start_fira returned\r\n");
        }

        bool do_stop;
        key = qirq_lock();
        do_stop = m_stop_pending;
        m_stop_pending = false;
        qirq_unlock(key);

        if (do_stop && m_session_started)
        {
            QLOGI("ble_session: stopping FiRa, restarting UCI");
            stop_fira();
            QLOGI("ble_session: stop_fira done, restarting UCI");
            uci_helper(NULL);
            QLOGI("ble_session: uci restarted");
        }
    }
}

static void apply_params_to_fira(const struct ble_session_params *p, fira_param_t *fp)
{
    memset(fp, 0, sizeof(*fp));

    fp->session_id = p->session_id;
    fp->short_addr = BLE_SESSION_LOCAL_SHORT_ADDR;
    fp->config_state = FIRA_APP_CONFIG_DEFAULT;
    fp->app_type = FIRA_APP_RESPF;

    struct session_parameters *s = &fp->session;

    s->device_type = QUWBS_FBS_DEVICE_TYPE_CONTROLEE;
    s->device_role = QUWBS_FBS_DEVICE_ROLE_RESPONDER;
    /* AndroidX UWB CONFIG_MULTICAST_DS_TWR → ONE_TO_MANY. */
    s->multi_node_mode = FIRA_MULTI_NODE_MODE_ONE_TO_MANY;
    s->ranging_round_usage = FIRA_DEFAULT_RANGING_ROUND_USAGE;
    s->rframe_config = FIRA_DEFAULT_RFRAME_CONFIG;
    s->sfd_id = FIRA_DEFAULT_SFD_ID;
    s->phr_data_rate = FIRA_PRF_MODE_BPRF;
    s->prf_mode = FIRA_PRF_MODE_BPRF;
    s->slot_duration_rstu = BLE_SESSION_DEFAULT_SLOT_RSTU;
    s->block_duration_ms = BLE_SESSION_DEFAULT_BLOCK_MS;
    s->round_duration_slots = BLE_SESSION_DEFAULT_ROUND_SLOTS;
    /* AOSP RangingTimingParams for CONFIG_MULTICAST_DS_TWR sets hoppingEnabled=true,
     * which the phone modem then programs as HOPPING_MODE_FIRA_HOPPING_ENABLE. The
     * Controlee must match or the round-start times drift apart and rounds time out. */
    s->round_hopping = true;
    s->schedule_mode = FIRA_SCHEDULE_MODE_TIME_SCHEDULED;
    s->preamble_duration = FIRA_PREAMBLE_DURATION_64;
    s->number_of_sts_segments = FIRA_STS_SEGMENTS_1;
    s->sts_length = FIRA_STS_LENGTH_64;

    s->channel_number = p->channel;
    s->preamble_code_index = p->preamble;

    s->short_addr = BLE_SESSION_LOCAL_SHORT_ADDR;
    s->n_destination_short_address = 1;
    s->destination_short_address[0] = p->controller_addr;

    /* AndroidX UWB sessionKeyInfo[8] split:
     *   VENDOR_ID    = sessionKeyInfo[0..1]
     *   STATIC_STS_IV = sessionKeyInfo[2..7]
     *
     * Per FiRa MAC v2.0 §5.9.5: V_UPPER_64 (MSB-first) =
     *   STATIC_STS_IV (6 octets) || VENDOR_ID (2 octets)
     *
     * AOSP UWB has a runtime feature flag `isReversedByteOrderFiraParams`
     * that, when TRUE (the AOSP default and what most OEMs ship), reverses
     * the VENDOR_ID bytes on the wire — the phone's modem then composes
     * V_UPPER_64 with the VID nibble swapped relative to the spec. Direct
     * byte order failed (status=36); try the reversed-VID variant. */
    memcpy(&s->vupper64[0], &p->session_key[2], 6); /* STATIC_STS_IV */
    s->vupper64[6] = p->session_key[1];             /* VENDOR_ID[1] (reversed) */
    s->vupper64[7] = p->session_key[0];             /* VENDOR_ID[0] (reversed) */

    QLOGI("BLES vup0=%02X%02X%02X%02X",
          s->vupper64[0], s->vupper64[1], s->vupper64[2], s->vupper64[3]);
    QLOGI("BLES vup1=%02X%02X%02X%02X",
          s->vupper64[4], s->vupper64[5], s->vupper64[6], s->vupper64[7]);

    /* AOSP CONFIG_MULTICAST_DS_TWR requests AoA results (azimuth+elevation+FoM).
     * The Controlee has to enable the same report bits or the modem may drop
     * rounds whose result frame doesn't contain the expected fields. */
    s->result_report_config |= fira_helper_bool_to_result_report_config(true, true, true, true);
    s->ranging_round_control |= fira_helper_bool_to_ranging_round_control(true, false);

    fp->controlees_params.n_controlees = 0;
}

static void start_fira(const struct ble_session_params *p)
{
    enum qerr r;
    struct fbs_session_init_rsp rsp;

    apply_params_to_fira(p, &m_fira_param);
    QLOGI("ble_session: ch=%u pcode=%u peer=0x%04X session=0x%08lX",
          (unsigned)p->channel, (unsigned)p->preamble,
          (unsigned)p->controller_addr, (unsigned long)p->session_id);

    SEGGER_RTT_WriteString(0, "BLES: fira_uwb_mcps_init\r\n");
    QLOGI("ble_session: fira_uwb_mcps_init...");
    r = fira_uwb_mcps_init(&m_uwbmac_ctx);
    if (r != QERR_SUCCESS)
    {
        QLOGE("ble_session: fira_uwb_mcps_init failed (%d)", (int)r);
        return;
    }

    uwbmac_set_short_addr(m_uwbmac_ctx, m_fira_param.session.short_addr);

    QLOGI("ble_session: fira_helper_open...");
    r = fira_helper_open(&m_fira_ctx, m_uwbmac_ctx, fira_notification_cb, "endless", 0, NULL);
    if (r != QERR_SUCCESS)
    {
        QLOGE("ble_session: fira_helper_open failed (%d)", (int)r);
        goto deinit_mcps;
    }

    QLOGI("ble_session: fira_helper_set_scheduler...");
    r = fira_helper_set_scheduler(&m_fira_ctx);
    if (r != QERR_SUCCESS)
    {
        QLOGE("ble_session: fira_helper_set_scheduler failed (%d)", (int)r);
        goto close_helper;
    }

    QLOGI("ble_session: fira_prepare_measurement_sequence...");
    r = fira_prepare_measurement_sequence(m_uwbmac_ctx, &m_fira_param.session, false);
    if (r != QERR_SUCCESS)
    {
        QLOGE("ble_session: fira_prepare_measurement_sequence failed (%d)", (int)r);
        goto close_helper;
    }

    QLOGI("ble_session: fira_helper_init_session...");
    r = fira_helper_init_session(&m_fira_ctx, m_fira_param.session_id, QUWBS_FBS_SESSION_TYPE_RANGING_NO_IN_BAND_DATA, &rsp);
    if (r != QERR_SUCCESS)
    {
        QLOGE("ble_session: fira_helper_init_session failed (%d)", (int)r);
        goto close_helper;
    }
    m_session_handle = rsp.session_handle;

    QLOGI("ble_session: fira_set_session_parameters (handle=0x%08lX)...", (unsigned long)m_session_handle);
    QLOGI("ble_session: cadence in: block=%ums slot=%uRSTU round_slots=%u",
          (unsigned)m_fira_param.session.block_duration_ms,
          (unsigned)m_fira_param.session.slot_duration_rstu,
          (unsigned)m_fira_param.session.round_duration_slots);
    r = fira_set_session_parameters(&m_fira_ctx, m_session_handle, &m_fira_param.session);
    if (r != QERR_SUCCESS)
    {
        QLOGE("ble_session: fira_set_session_parameters failed (%d)", (int)r);
        goto deinit_session;
    }

    /* fira_set_session_parameters skips a handful of UCI app-config TLVs that
     * the AndroidX phone always programs explicitly. If the FiRa stack's
     * compiled defaults don't match the UCI defaults the phone assumes,
     * STS verification fails (status=36) on every round. Pin them down. */
#define BLES_SET(name, value)                                                  \
    do {                                                                       \
        r = fira_helper_set_session_##name(&m_fira_ctx, m_session_handle,      \
                                           (value));                           \
        if (r != QERR_SUCCESS) {                                               \
            QLOGE("ble_session: set " #name " failed (%d)", (int)r);           \
            goto deinit_session;                                               \
        }                                                                      \
    } while (0)
    /* AndroidX UWB CONFIG_MULTICAST_DS_TWR + UCI defaults. */
    BLES_SET(sts_length,               FIRA_STS_LENGTH_64);          /* 1 */
    BLES_SET(number_of_sts_segments,   FIRA_STS_SEGMENTS_1);         /* 1 */
    BLES_SET(preamble_duration,        FIRA_PREAMBLE_DURATION_64);   /* 1 */
    BLES_SET(psdu_data_rate,           FIRA_PSDU_DATA_RATE_6M81);    /* 0 */
    BLES_SET(key_rotation,             0);

    /* Topology/timing TLVs the phone modem programs for CONFIG_MULTICAST_DS_TWR
     * (CONFIG_ID_2). Pinning these via helpers in addition to the struct init
     * defends against stack defaults silently overriding session_parameters. */
    BLES_SET(multi_node_mode,          FIRA_MULTI_NODE_MODE_ONE_TO_MANY);
    BLES_SET(ranging_round_usage,      FIRA_RANGING_ROUND_USAGE_DSTWR_DEFERRED);
    BLES_SET(rframe_config,            FIRA_RFRAME_CONFIG_SP3);
    BLES_SET(schedule_mode,            FIRA_SCHEDULE_MODE_TIME_SCHEDULED);
    BLES_SET(slot_duration_rstu,       BLE_SESSION_DEFAULT_SLOT_RSTU);
    BLES_SET(block_duration_ms,        BLE_SESSION_DEFAULT_BLOCK_MS);
    BLES_SET(round_duration_slots,     BLE_SESSION_DEFAULT_ROUND_SLOTS);
    BLES_SET(round_hopping,            1);
    BLES_SET(in_band_termination_attempt_count, 3);
    BLES_SET(result_report_config,
             fira_helper_bool_to_result_report_config(true, true, true, true));
#undef BLES_SET

    /* Belt-and-braces: also push VENDOR_ID and STATIC_STS_IV via their
     * dedicated setters (in case the FiRa stack reads them independently of
     * the vupper64 byte buffer). Use the same byte arrangement we put in
     * vupper64 — see comment above for the reversed-VID rationale. */
    {
        uint8_t vid[2];
        uint8_t iv[6];
        memcpy(vid, &m_fira_param.session.vupper64[6], 2);
        memcpy(iv,  &m_fira_param.session.vupper64[0], 6);
        r = fira_helper_set_session_vendor_id(&m_fira_ctx, m_session_handle, vid);
        if (r != QERR_SUCCESS) {
            QLOGE("ble_session: set vendor_id failed (%d)", (int)r);
            goto deinit_session;
        }
        r = fira_helper_set_session_static_sts_iv(&m_fira_ctx, m_session_handle, iv);
        if (r != QERR_SUCCESS) {
            QLOGE("ble_session: set static_sts_iv failed (%d)", (int)r);
            goto deinit_session;
        }
    }

    QLOGI("ble_session: uwbmac_start...");
    r = uwbmac_start(m_uwbmac_ctx);
    if (r != QERR_SUCCESS)
    {
        QLOGE("ble_session: uwbmac_start failed (%d)", (int)r);
        goto deinit_session;
    }

    QLOGI("ble_session: fira_helper_start_session...");
    r = fira_helper_start_session(&m_fira_ctx, m_session_handle);
    if (r != QERR_SUCCESS)
    {
        QLOGE("ble_session: fira_helper_start_session failed (%d)", (int)r);
        uwbmac_stop(m_uwbmac_ctx);
        goto deinit_session;
    }

    m_session_started = true;
    SEGGER_RTT_WriteString(0, "BLES: FiRa Controlee ACTIVE\r\n");
    QLOGI("ble_session: FiRa Controlee active (session=0x%08lX, short=0x%04X, peer=0x%04X)",
          (unsigned long)m_fira_param.session_id,
          (unsigned)m_fira_param.session.short_addr,
          (unsigned)m_fira_param.session.destination_short_address[0]);
    return;

deinit_session:
    fira_helper_deinit_session(&m_fira_ctx, m_session_handle);
close_helper:
    fira_helper_close(&m_fira_ctx);
deinit_mcps:
    fira_uwb_mcps_deinit(m_uwbmac_ctx);
    m_uwbmac_ctx = NULL;
}

static void fira_notification_cb(enum fira_helper_cb_type cb_type, const void *content, void *user_data)
{
    (void)user_data;

    if (cb_type != FIRA_HELPER_CB_TYPE_TWR_RANGE_NTF || content == NULL)
    {
        return;
    }

    const struct fira_twr_ranging_results *results = content;
    for (int i = 0; i < results->n_measurements; i++)
    {
        const struct fira_twr_measurements *rm = &results->measurements[i];
        if (rm->status == 0)
        {
            QLOGI("ble_session: range peer=0x%04X d=%dcm",
                  (unsigned)rm->short_addr, (int)rm->distance_cm);
        }
        else
        {
            QLOGI("ble_session: range peer=0x%04X status=%u",
                  (unsigned)rm->short_addr, (unsigned)rm->status);
        }
    }
}

static void stop_fira(void)
{
    if (!m_session_started)
    {
        return;
    }

    enum qerr r = fira_helper_stop_session(&m_fira_ctx, m_session_handle);
    if (r != QERR_SUCCESS)
    {
        QLOGE("ble_session: fira_helper_stop_session failed (%d)", (int)r);
    }

    uwbmac_stop(m_uwbmac_ctx);

    r = fira_helper_deinit_session(&m_fira_ctx, m_session_handle);
    if (r != QERR_SUCCESS && r != QERR_EBUSY)
    {
        QLOGE("ble_session: fira_helper_deinit_session failed (%d)", (int)r);
    }

    fira_helper_close(&m_fira_ctx);
    /* fira_uwb_mcps_deinit only releases llhw/l1/qplatform — uwbmac global
     * state still holds the context and would EBUSY a fresh uwbmac_init from
     * UCI restart. fira_app.c::cleanup does the same pairing. */
    uwbmac_exit(m_uwbmac_ctx);
    fira_uwb_mcps_deinit(m_uwbmac_ctx);
    m_uwbmac_ctx = NULL;
    m_session_started = false;
    QLOGI("ble_session: FiRa Controlee stopped");
}
