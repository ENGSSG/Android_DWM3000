/**
 * @file      ble_fff0.h
 *
 * @brief     Interface for the FFF0 UWB OOB GATT service used by Android UWB apps
 *            (pixel_uwb, DWM3000 Tracker). Modelled on ble_qnis.h.
 *
 *            Service contract:
 *              FFF0 — Primary service, 16-bit SIG-style UUID.
 *              FFF1 — Write characteristic. Phone writes a 16-byte session-params
 *                     payload here: sessionId(4 LE) | channel(1) | preamble(1) |
 *                     sessionKey(8) | controllerAddress(2 LE).
 *              FFF2 — Notify characteristic. Firmware sends back its 2-byte UWB
 *                     short address once it has consumed the params.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "ble_link_ctx_manager.h"

#define BLE_UUID_FFF0_SERVICE      0xFFF0
#define BLE_UUID_FFF1_PARAMS_CHAR  0xFFF1
#define BLE_UUID_FFF2_RESP_CHAR    0xFFF2

#define BLE_FFF0_PARAMS_LEN        16
#define BLE_FFF0_ADDRESS_LEN       2

#ifndef BLE_FFF0_BLE_OBSERVER_PRIO
#define BLE_FFF0_BLE_OBSERVER_PRIO 2
#endif

#define BLE_FFF0_DEF(_name, _max_clients)                                                                          \
    BLE_LINK_CTX_MANAGER_DEF(CONCAT_2(_name, _link_ctx_storage), (_max_clients), sizeof(ble_fff0_client_context_t)); \
    static ble_fff0_t _name = {                                                                                     \
        .p_link_ctx_storage = &CONCAT_2(_name, _link_ctx_storage)};                                                 \
    NRF_SDH_BLE_OBSERVER(_name##_obs, BLE_FFF0_BLE_OBSERVER_PRIO, ble_fff0_on_ble_evt, &_name)

typedef enum
{
    BLE_FFF0_EVT_PARAMS_WRITTEN,    /**< 16-byte params payload received on FFF1. */
    BLE_FFF0_EVT_NOTIFY_ENABLED,    /**< Peer enabled CCCD on FFF2. */
    BLE_FFF0_EVT_NOTIFY_DISABLED,   /**< Peer disabled CCCD on FFF2. */
} ble_fff0_evt_type_t;

typedef struct ble_fff0_s ble_fff0_t;

typedef struct
{
    bool is_notification_enabled;
} ble_fff0_client_context_t;

typedef struct
{
    uint8_t const *p_data;          /**< Points at exactly 16 bytes when type == PARAMS_WRITTEN. */
    uint16_t length;
} ble_fff0_params_evt_t;

typedef struct
{
    ble_fff0_evt_type_t type;
    ble_fff0_t *p_fff0;
    uint16_t conn_handle;
    ble_fff0_client_context_t *p_link_ctx;
    union
    {
        ble_fff0_params_evt_t params;
    } data;
} ble_fff0_evt_t;

typedef void (*ble_fff0_event_handler_t)(ble_fff0_evt_t *p_evt);

typedef struct
{
    ble_fff0_event_handler_t event_handler;
} ble_fff0_init_t;

struct ble_fff0_s
{
    uint16_t service_handle;
    ble_gatts_char_handles_t params_handles;            /**< FFF1 (write). */
    ble_gatts_char_handles_t resp_handles;              /**< FFF2 (notify). */
    blcm_link_ctx_storage_t *const p_link_ctx_storage;
    ble_fff0_event_handler_t event_handler;
};

/**
 * Initialize the FFF0 service.
 */
uint32_t ble_fff0_init(ble_fff0_t *p_fff0, ble_fff0_init_t const *p_init);

/**
 * Forward SoftDevice BLE events to the service. Called via NRF_SDH_BLE_OBSERVER.
 */
void ble_fff0_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context);

/**
 * Send the 2-byte UWB short address as an FFF2 notification.
 */
uint32_t ble_fff0_address_notify(ble_fff0_t *p_fff0, uint8_t const *address, uint16_t conn_handle);
