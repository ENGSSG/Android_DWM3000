/**
 * @file    ble.c
 *
 * @brief   Minimal BLE advertising bring-up for UCI debug.
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "sdk_config.h"
#include "app_error.h"
#include "nrf.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_freertos.h"
#include "qlog.h"
#include "SEGGER_RTT.h"
#include "ble_fff0.h"
#include "ble_session.h"

#define APP_BLE_OBSERVER_PRIO 3
#define APP_BLE_CONN_CFG_TAG 1

#define APP_ADV_INTERVAL_FAST 320
#define APP_ADV_DURATION 0
#define MIN_CONN_INTERVAL MSEC_TO_UNITS(25, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(250, UNIT_1_25_MS)
#define SLAVE_LATENCY 6
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(16000, UNIT_10_MS)

BLE_ADVERTISING_DEF(m_advertising);
BLE_FFF0_DEF(m_fff0, NRF_SDH_BLE_TOTAL_LINK_COUNT);

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;
/* LE encoding of BLE_SESSION_LOCAL_SHORT_ADDR (0x0001). */
static const uint8_t m_local_uwb_addr[BLE_FFF0_ADDRESS_LEN] = { 0x01, 0x00 };

static ble_uuid_t m_adv_uuids[] = {
    { BLE_UUID_FFF0_SERVICE, BLE_UUID_TYPE_BLE },
};

static void advertising_start(void *parm);

static void fff0_event_handler(ble_fff0_evt_t *p_evt)
{
    switch (p_evt->type)
    {
        case BLE_FFF0_EVT_NOTIFY_ENABLED:
            QLOGI("FFF0: notifications enabled, conn=0x%04X", p_evt->conn_handle);
            break;

        case BLE_FFF0_EVT_NOTIFY_DISABLED:
            QLOGI("FFF0: notifications disabled, conn=0x%04X", p_evt->conn_handle);
            break;

        case BLE_FFF0_EVT_PARAMS_WRITTEN:
        {
            uint16_t len = p_evt->data.params.length;
            QLOGI("FFF0: %u-byte params received from conn=0x%04X", (unsigned)len, p_evt->conn_handle);
            /* Echo back the local UWB short address so the central can start ranging. */
            ret_code_t err = ble_fff0_address_notify(&m_fff0, m_local_uwb_addr, p_evt->conn_handle);
            if (err != NRF_SUCCESS)
            {
                QLOGE("FFF0: address_notify failed err=0x%08lX", (unsigned long)err);
            }
            if (len == BLE_SESSION_PARAMS_LEN)
            {
                ble_session_submit_params(p_evt->data.params.p_data, p_evt->conn_handle);
            }
            else
            {
                QLOGE("FFF0: ignoring %u-byte payload (expected %u)", (unsigned)len, (unsigned)BLE_SESSION_PARAMS_LEN);
            }
            break;
        }

        default:
            break;
    }
}

static void rtt_trace(const char *msg)
{
    SEGGER_RTT_WriteString(0, msg);
    SEGGER_RTT_WriteString(0, "\r\n");
}

static void gap_params_init(char *gap_name)
{
    ret_code_t err_code;
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)gap_name, strlen(gap_name));
    APP_ERROR_CHECK(err_code);
    rtt_trace("ble: GAP name set");
    QLOGI("ble: GAP name set to %s", gap_name);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_TAG);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    (void)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            QLOGI("BLE connected, handle=0x%04X", m_conn_handle);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            QLOGI("BLE disconnected");
            /* Do NOT tear down the FiRa session here. The OOB hand-off is
             * one-shot: the phone closes BLE immediately after writing the
             * params on FFF1 and starts ranging on its own. Stopping FiRa
             * here would kill ranging the moment OOB completes. Just resume
             * advertising so a future peer can re-trigger OOB. */
            advertising_start(NULL);
            break;

        default:
            break;
    }
}

static void services_init(void)
{
    ret_code_t err_code;
    ble_fff0_init_t fff0_init;

    memset(&fff0_init, 0, sizeof(fff0_init));
    fff0_init.event_handler = fff0_event_handler;

    err_code = ble_fff0_init(&m_fff0, &fff0_init);
    APP_ERROR_CHECK(err_code);
    rtt_trace("ble: FFF0 service registered");
    QLOGI("ble: FFF0 service registered");
}

static void ble_stack_init(void)
{
    ret_code_t err_code;
    uint32_t ram_start = 0;

    rtt_trace("ble: disable request");
    QLOGI("ble: disable request");
    nrf_sdh_disable_request();

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);
    rtt_trace("ble: softdevice enabled");
    QLOGI("ble: softdevice enabled");

    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);
    rtt_trace("ble: default cfg set");
    QLOGI("ble: default cfg set, ram_start=0x%08lx", (unsigned long)ram_start);

    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
    rtt_trace("ble: BLE enabled");
    QLOGI("ble: BLE enabled, ram_start=0x%08lx", (unsigned long)ram_start);

    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

static void advertising_init(void)
{
    ret_code_t err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));
    init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids = m_adv_uuids;

    init.config.ble_adv_on_disconnect_disabled = false;
    init.config.ble_adv_fast_enabled = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL_FAST;
    init.config.ble_adv_fast_timeout = APP_ADV_DURATION;
    init.config.ble_adv_slow_enabled = false;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);
    rtt_trace("ble: advertising_init done");
    QLOGI("ble: advertising_init done");

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

static void advertising_start(void *parm)
{
    ret_code_t err_code;
    (void)parm;
    rtt_trace("ble: advertising_start enter");
    QLOGI("ble: advertising_start");
    err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    /* The ble_advertising lib auto-restarts advertising on disconnect, so
     * INVALID_STATE here just means "already advertising" — not an error. */
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE)
    {
        APP_ERROR_CHECK(err_code);
    }
    rtt_trace("ble: advertising_start done");
}

void ble_init(char *gap_name)
{
    rtt_trace("ble: init begin");
    QLOGI("ble: init begin");
    ble_stack_init();
    gap_params_init(gap_name);
    services_init();
    ble_session_init();
    advertising_init();
    rtt_trace("ble: starting freertos helper");
    QLOGI("ble: starting freertos helper");
    nrf_sdh_freertos_init(advertising_start, NULL);
    rtt_trace("ble: init end");
    QLOGI("ble: init end");
}

void send_qnis_data(uint16_t conn_handle, uint8_t *buffer, uint16_t data_len)
{
    (void)conn_handle;
    (void)buffer;
    (void)data_len;
}
