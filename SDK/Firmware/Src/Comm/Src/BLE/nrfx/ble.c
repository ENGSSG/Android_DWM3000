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

#define APP_BLE_OBSERVER_PRIO 3
#define APP_BLE_CONN_CFG_TAG 1

#define APP_ADV_INTERVAL_FAST 320
#define APP_ADV_DURATION 0
#define MIN_CONN_INTERVAL MSEC_TO_UNITS(25, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(250, UNIT_1_25_MS)
#define SLAVE_LATENCY 6
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(16000, UNIT_10_MS)

BLE_ADVERTISING_DEF(m_advertising);

static void advertising_start(void *parm);

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
            QLOGI("BLE connected");
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            QLOGI("BLE disconnected");
            advertising_start(NULL);
            break;

        default:
            break;
    }
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
    rtt_trace("ble: advertising_start");
    QLOGI("ble: advertising_start");
    err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

void ble_init(char *gap_name)
{
    rtt_trace("ble: init begin");
    QLOGI("ble: init begin");
    ble_stack_init();
    gap_params_init(gap_name);
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
