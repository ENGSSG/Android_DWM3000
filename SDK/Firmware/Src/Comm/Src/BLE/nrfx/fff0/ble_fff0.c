/**
 * @file      ble_fff0.c
 *
 * @brief     FFF0 UWB OOB GATT service implementation. Modelled on ble_qnis.c.
 */

#include "sdk_common.h"

#include "ble.h"
#include "ble_fff0.h"
#include "ble_srv_common.h"
#include "qlog.h"

static void on_connect(ble_fff0_t *p_fff0, ble_evt_t const *p_ble_evt)
{
    ret_code_t err_code;
    ble_fff0_client_context_t *p_client = NULL;

    err_code = blcm_link_ctx_get(p_fff0->p_link_ctx_storage,
                                 p_ble_evt->evt.gap_evt.conn_handle,
                                 (void *)&p_client);
    if (err_code != NRF_SUCCESS)
    {
        QLOGE("FFF0: link ctx get failed for conn 0x%02X", p_ble_evt->evt.gap_evt.conn_handle);
        return;
    }

    if (p_client != NULL)
    {
        p_client->is_notification_enabled = false;
    }
}

static void on_write(ble_fff0_t *p_fff0, ble_evt_t const *p_ble_evt)
{
    ret_code_t err_code;
    ble_fff0_evt_t evt;
    ble_fff0_client_context_t *p_client = NULL;
    ble_gatts_evt_write_t const *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    err_code = blcm_link_ctx_get(p_fff0->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *)&p_client);
    if (err_code != NRF_SUCCESS)
    {
        QLOGE("FFF0: link ctx get failed for conn 0x%02X", p_ble_evt->evt.gatts_evt.conn_handle);
    }

    memset(&evt, 0, sizeof(evt));
    evt.p_fff0 = p_fff0;
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
    evt.p_link_ctx = p_client;

    /* CCCD on FFF2 (notify) */
    if ((p_evt_write->handle == p_fff0->resp_handles.cccd_handle) && (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            bool enabled = ble_srv_is_notification_enabled(p_evt_write->data);
            p_client->is_notification_enabled = enabled;
            evt.type = enabled ? BLE_FFF0_EVT_NOTIFY_ENABLED : BLE_FFF0_EVT_NOTIFY_DISABLED;

            if (p_fff0->event_handler != NULL)
            {
                p_fff0->event_handler(&evt);
            }
        }
        return;
    }

    /* Write to FFF1 (session params) */
    if ((p_evt_write->handle == p_fff0->params_handles.value_handle) && (p_fff0->event_handler != NULL))
    {
        evt.type = BLE_FFF0_EVT_PARAMS_WRITTEN;
        evt.data.params.p_data = p_evt_write->data;
        evt.data.params.length = p_evt_write->len;
        p_fff0->event_handler(&evt);
    }
}

void ble_fff0_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context)
{
    if ((p_ble_evt == NULL) || (p_context == NULL))
    {
        return;
    }

    ble_fff0_t *p_fff0 = (ble_fff0_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_fff0, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_fff0, p_ble_evt);
            break;

        default:
            break;
    }
}

uint32_t ble_fff0_init(ble_fff0_t *p_fff0, ble_fff0_init_t const *p_init)
{
    ret_code_t err_code;
    ble_uuid_t ble_uuid;
    ble_add_char_params_t add_char_params;

    VERIFY_PARAM_NOT_NULL(p_fff0);
    VERIFY_PARAM_NOT_NULL(p_init);

    p_fff0->event_handler = p_init->event_handler;

    /* SIG 16-bit UUID — no vs_add needed. */
    ble_uuid.type = BLE_UUID_TYPE_BLE;
    ble_uuid.uuid = BLE_UUID_FFF0_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_fff0->service_handle);
    VERIFY_SUCCESS(err_code);

    /* FFF1: write (with response) — exactly 16 bytes. */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid = BLE_UUID_FFF1_PARAMS_CHAR;
    add_char_params.uuid_type = BLE_UUID_TYPE_BLE;
    add_char_params.max_len = BLE_FFF0_PARAMS_LEN;
    add_char_params.init_len = 0;
    add_char_params.is_var_len = true;
    add_char_params.char_props.write = 1;
    add_char_params.char_props.write_wo_resp = 1;
    add_char_params.read_access = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;

    err_code = characteristic_add(p_fff0->service_handle, &add_char_params, &p_fff0->params_handles);
    VERIFY_SUCCESS(err_code);

    /* FFF2: notify — 2-byte UWB short address. */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid = BLE_UUID_FFF2_RESP_CHAR;
    add_char_params.uuid_type = BLE_UUID_TYPE_BLE;
    add_char_params.max_len = BLE_FFF0_ADDRESS_LEN;
    add_char_params.init_len = 0;
    add_char_params.is_var_len = true;
    add_char_params.char_props.notify = 1;
    add_char_params.read_access = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    return characteristic_add(p_fff0->service_handle, &add_char_params, &p_fff0->resp_handles);
}

uint32_t ble_fff0_address_notify(ble_fff0_t *p_fff0, uint8_t const *address, uint16_t conn_handle)
{
    ret_code_t err_code;
    ble_gatts_hvx_params_t hvx_params;
    ble_fff0_client_context_t *p_client = NULL;
    uint16_t length = BLE_FFF0_ADDRESS_LEN;

    VERIFY_PARAM_NOT_NULL(p_fff0);
    VERIFY_PARAM_NOT_NULL(address);

    err_code = blcm_link_ctx_get(p_fff0->p_link_ctx_storage, conn_handle, (void *)&p_client);
    VERIFY_SUCCESS(err_code);

    if ((conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
    {
        return NRF_ERROR_NOT_FOUND;
    }

    if (!p_client->is_notification_enabled)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));
    hvx_params.handle = p_fff0->resp_handles.value_handle;
    hvx_params.p_data = address;
    hvx_params.p_len = &length;
    hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}
