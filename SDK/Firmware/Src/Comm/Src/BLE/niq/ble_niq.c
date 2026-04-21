/**
 * @file      ble_niq.c
 *
 * @brief     functions for niq BLE
 *
 * @author    Qorvo Applications
 *
 * @copyright SPDX-FileCopyrightText: Copyright (c) 2024-2025 Qorvo US, Inc.
 *            SPDX-License-Identifier: LicenseRef-QORVO-2
 *
 */

#include <stdint.h>

#include "nordic_common.h"
#include "ble_qnis.h"
#include "ble_anis.h"
#include "net/fira_region_params.h"
#include "niq.h"
#include "app_ble.h"
#include "qlog.h"

/* Defines from common_fira */
#define FIRA_BPRF_SET_1             (1) /* SP0 IEEE SFD */
#define FIRA_BPRF_SET_2             (2) /* SP0 4z SFD */
#define FIRA_BPRF_SET_3             (3) /* SP1 4z SFD */
#define FIRA_BPRF_SET_4             (4) /* SP3 4z SFD */
#define FIRA_BPRF_SET_5             (5) /* SP1 IEEE SFD */
#define FIRA_BPRF_SET_6             (6) /* SP3 IEEE SFD */

#define BLE_NIQ_DEFAULT_CONN_HANDLE 0xFFFF
#define BLE_DEVICE_PARAMS_VERSION   1

enum ble_device_params_status
{
    BLE_DEVICE_PARAMS_STATUS_OK = 0,
    BLE_DEVICE_PARAMS_STATUS_BAD_LEN = 1,
    BLE_DEVICE_PARAMS_STATUS_BAD_VERSION = 2,
};

uint16_t current_conn_handle = BLE_NIQ_DEFAULT_CONN_HANDLE;

static void send_accessory_config_data(uint16_t conn_handle);

struct ble_device_params
{
    uint8_t version;
    uint8_t role;
    uint8_t enc_payload;
    uint8_t ranging_round_usage;
    uint8_t rframe_config;
    uint8_t channel_number;
    uint8_t preamble_code;
    uint8_t mac_address_mode;
    uint8_t number_of_controlee;
    uint16_t src_addr;
    uint16_t dst_addr;
    uint16_t slot_duration_rstu;
    uint16_t block_timing_stability;
    uint32_t session_id;
    uint32_t block_duration_ms;
    uint32_t round_duration_rstu;
    uint32_t max_rr_retry;
} __attribute__((packed));

/* Initialize fira_config with default config */
fira_device_configure_t fira_config = {
    .role = QUWBS_FBS_DEVICE_ROLE_INITIATOR,
    .enc_payload = 1,
    .Session_ID = 1111,
    .Ranging_Round_Usage = FIRA_RANGING_ROUND_USAGE_DSTWR_DEFERRED,
    .Multi_Node_Mode = FIRA_MULTI_NODE_MODE_UNICAST,
    .Rframe_Config = FIRA_RFRAME_CONFIG_SP3,
    .ToF_Report = 1,
    .AoA_Azimuth_Report = 0,
    .AoA_Elevation_Report = 0,
    .AoA_FOM_Report = 0,
    .nonDeferred_Mode = 0,
    .STS_Config = 0,
    .Round_Hopping = 0,
    .Block_Striding = 0,
    .Block_Duration_ms = 100,
    .Round_Duration_RSTU = 18400,
    .Slot_Duration_RSTU = 2400,
    .Channel_Number = 9,
    .Preamble_Code = 11,
    .PRF_Mode = 0,
    .SP0_PHY_Set = FIRA_BPRF_SET_2,
    .SP1_PHY_Set = FIRA_BPRF_SET_3,
    .SP3_PHY_Set = FIRA_BPRF_SET_4,
    .MAX_RR_Retry = 1,
    .Constraint_Length_Conv_Code_HPRF = 0,
    .UWB_Init_Time_ms = 5,
    .Block_Timing_Stability = 0,
    .Key_Rotation = 0,
    .Key_Rotation_Rate = 0,
    .MAC_FCS_TYPE = 0,
    .MAC_ADDRESS_MODE = 0,
    .SRC_ADDR[0] = 0,
    .SRC_ADDR[1] = 0,
    .Number_of_Controlee = 1,
    .DST_ADDR[0] = 1,
    .DST_ADDR[1] = 0,
    .Vendor_ID[0] = 0,
    .Vendor_ID[1] = 0,
    .Static_STS_IV[0] = 0,
    .Static_STS_IV[1] = 0,
    .Static_STS_IV[2] = 0,
    .Static_STS_IV[3] = 0,
    .Static_STS_IV[4] = 0,
    .Static_STS_IV[5] = 0};

/**
 * @enum CustomMessageId
 * @brief Custom message identifiers for BLE communication.
 *
 * This enum extends the SampleAppMessageId enum. Care should be taken to ensure
 * that the values defined here do not overlap with the values in the SampleAppMessageId enum.
 *
 * @note The values in this enum are specific to the BLE NIQ module.
 */
enum CustomMessageId
{
    UserId_getDeviceStruct = 0x20,
    UserId_setDeviceStruct = 0x21,
    UserId_setDeviceStructAck = 0x22,

    UserId_iOSNotify = 0x2F
};

static uint16_t pack_u16(const uint8_t value[2])
{
    return (uint16_t)value[0] | ((uint16_t)value[1] << 8);
}

static void unpack_u16(uint16_t value, uint8_t out[2])
{
    out[0] = value & 0xFF;
    out[1] = (value >> 8) & 0xFF;
}

static void fill_device_params(struct ble_device_params *params)
{
    memset(params, 0, sizeof(*params));

    params->version = BLE_DEVICE_PARAMS_VERSION;
    params->role = fira_config.role;
    params->enc_payload = fira_config.enc_payload;
    params->ranging_round_usage = fira_config.Ranging_Round_Usage;
    params->rframe_config = fira_config.Rframe_Config;
    params->channel_number = fira_config.Channel_Number;
    params->preamble_code = fira_config.Preamble_Code;
    params->mac_address_mode = fira_config.MAC_ADDRESS_MODE;
    params->number_of_controlee = fira_config.Number_of_Controlee;
    params->src_addr = pack_u16(fira_config.SRC_ADDR);
    params->dst_addr = pack_u16(fira_config.DST_ADDR);
    params->slot_duration_rstu = (uint16_t)fira_config.Slot_Duration_RSTU;
    params->block_timing_stability = fira_config.Block_Timing_Stability;
    params->session_id = fira_config.Session_ID;
    params->block_duration_ms = fira_config.Block_Duration_ms;
    params->round_duration_rstu = fira_config.Round_Duration_RSTU;
    params->max_rr_retry = fira_config.MAX_RR_Retry;
}

static void apply_device_params(const struct ble_device_params *params)
{
    fira_config.role = params->role;
    fira_config.enc_payload = params->enc_payload;
    fira_config.Ranging_Round_Usage = params->ranging_round_usage;
    fira_config.Rframe_Config = params->rframe_config;
    fira_config.Channel_Number = params->channel_number;
    fira_config.Preamble_Code = params->preamble_code;
    fira_config.MAC_ADDRESS_MODE = params->mac_address_mode;
    fira_config.Number_of_Controlee = params->number_of_controlee;
    unpack_u16(params->src_addr, fira_config.SRC_ADDR);
    unpack_u16(params->dst_addr, fira_config.DST_ADDR);
    fira_config.Slot_Duration_RSTU = params->slot_duration_rstu;
    fira_config.Block_Timing_Stability = params->block_timing_stability;
    fira_config.Session_ID = params->session_id;
    fira_config.Block_Duration_ms = params->block_duration_ms;
    fira_config.Round_Duration_RSTU = params->round_duration_rstu;
    fira_config.MAX_RR_Retry = params->max_rr_retry;
}

static void send_device_params(uint16_t conn_handle)
{
    uint8_t buffer[1 + sizeof(struct ble_device_params)];
    ni_packet_t *packet = (ni_packet_t *)buffer;
    struct ble_device_params params;

    fill_device_params(&params);
    packet->message_id = (uint8_t)UserId_getDeviceStruct;
    memcpy(packet->payload, &params, sizeof(params));

    send_qnis_data(conn_handle, buffer, sizeof(buffer));
}

static void send_device_params_ack(uint16_t conn_handle, uint8_t status)
{
    uint8_t buffer[2];
    ni_packet_t *packet = (ni_packet_t *)buffer;
    packet->message_id = (uint8_t)UserId_setDeviceStructAck;
    packet->payload[0] = status;
    send_qnis_data(conn_handle, buffer, sizeof(buffer));
}

static void send_ack_uwb_started(uint16_t conn_handle)
{
    /* Send "UWB did start" BLE message to the app */
    uint8_t buffer[1];
    ni_packet_t *packet = (ni_packet_t *)buffer;
    packet->message_id = (uint8_t)MessageId_accessoryUwbDidStart;
    send_qnis_data(conn_handle, buffer, 1);
}

static void send_ack_uwb_stopped(uint16_t conn_handle)
{
    uint8_t buffer[1];
    ni_packet_t *packet = (ni_packet_t *)buffer;
    packet->message_id = (uint8_t)MessageId_accessoryUwbDidStop;
    send_qnis_data(conn_handle, buffer, 1);
}

void send_ios_notification(uint8_t distance, char *txt_message)
{
    uint8_t buffer[NI_MAX_PACKET_SIZE];
    uint8_t payload_size;

    if (current_conn_handle == BLE_NIQ_DEFAULT_CONN_HANDLE)
    {
        QLOGW("No connection detected yet for NI. Impossible to send notification to iOS.");
        return;
    }

    payload_size = strlen(txt_message) + 2; // txt_message len + 1 byte payload_size + 1 byte distance

    ni_packet_t *packet = (ni_packet_t *)buffer;
    packet->message_id = (uint8_t)UserId_iOSNotify;
    packet->payload[0] = payload_size;
    packet->payload[1] = distance;
    memcpy(&packet->payload[2], txt_message, strlen(txt_message));

    send_qnis_data(current_conn_handle, buffer, payload_size + 1);
}

/**
 * @brief Sending for handling new data from BLE
 */
static void send_accessory_config_data(uint16_t conn_handle)
{
    QLOGI("App requests accessory config data");

    // The Accessory Configuration Data in intended to be constructed by the embedded application.

    // The message structure is the following:
    // ------------------------
    //     majorVersion         -- the major version from section 3.3 of the Nearby Interaction Accessory Protocol Specification, Developer Preview, Release 1.
    //     minorVersion         -- the minor version from section 3.3 of the Nearby Interaction Accessory Protocol Specification, Developer Preview, Release 1.
    //     preferredUpdateRate  -- a selection of one of the options from table 3-3 in the Nearby Interaction Accessory Protocol Specification, Developer Preview, Release 1.
    //     rfu[10]              -- reserved for future use.
    //     uwbConfigDataLength  -- the length of the UWB config data as provided by the UWB middleware.
    //     uwbConfigData        -- the UWB config data as provided by the UWB middleware, according to section 4.4.2 on the UWB Interoperability Specification, Developer Preview, Release 1.
    // ------------------------
    //
    // In order to populate `uwbConfigData` and `uwbConfigDataLength`, the embedded appliaction needs to query
    // the UWB middleware which is compliant with the UWB Interoperability Specification.
    //
    // Once the Accessory Configuration Data is constucted and populated, the embedded application needs to send it to the iOS app.

    uint8_t buffer[sizeof(struct AccessoryConfigurationData) + 1]; // + 1 for the MessageId
    memset(buffer, 0, sizeof(struct AccessoryConfigurationData) + 1);

    ni_packet_t *packet = (ni_packet_t *)buffer;
    packet->message_id = (uint8_t)MessageId_accessoryConfigurationData;

    /* Embedded application responsibility */
    struct AccessoryConfigurationData *config = (struct AccessoryConfigurationData *)packet->payload;
    config->majorVersion = NI_ACCESSORY_PROTOCOL_SPEC_MAJOR_VERSION;
    config->minorVersion = NI_ACCESSORY_PROTOCOL_SPEC_MINOR_VERSION;
    config->preferredUpdateRate = PreferredUpdateRate_UserInteractive;

    /* UWB middleware responsibility */
    int ret = niq_populate_accessory_uwb_config_data(conn_handle, ACCESSORY_RANGING_ROLE, &config->uwbConfigData, &config->uwbConfigDataLength);
    if (ret != 0)
        QLOGE("niq_populate_accessory_uwb_config_data Failed");

    set_accessory_uwb_config_data(packet->payload);

    send_qnis_data(conn_handle, buffer, (uint16_t)(config->uwbConfigDataLength + ACCESSORY_CONFIGURATION_DATA_FIX_LEN + 1));
}

/**
 * @brief Function for handling new data from BLE.
 */
void handle_niq_data(uint16_t conn_handle, const uint8_t *data, int data_len)
{
    if (data_len < 1)
    {
        QLOGI("data_len must be at least 1. Return.");
        return;
    }

    if (current_conn_handle != conn_handle)
    {
        QLOGI("Request from unknown device: %d", conn_handle);
        return;
    }

    ni_packet_t *packet = (ni_packet_t *)data;

    switch (packet->message_id)
    {
        case MessageId_init:
        {
            /* Reply AccessoryConfigurationData to the app */
            send_accessory_config_data(conn_handle);
            break;
        }
        case MessageId_configure_and_start:
        {
            /* Configure accessory UWB and start */
            QLOGI("App requests config and start");
            int ret;
            ret = niq_configure_and_start_uwb(conn_handle, packet->payload, data_len - 1, &fira_config, NULL);
            switch (ret)
            {
                case -E_NIQ_INPVAL:
                    QLOGE("Data len wrong");
                    break;
                case -E_NIQ_VERSIONNOTSUPPORTED:
                    QLOGE("Protocol version not supported");
                    break;
                default:
                    QLOGI("Started");
                    send_ack_uwb_started(conn_handle);
                    break;
            }

            break;
        }
        case MessageId_stop:
        {
            QLOGI("App requests stop");
            /* Stop accessory UWB and reset state */
            niq_stop_uwb(conn_handle, NULL);

            send_ack_uwb_stopped(conn_handle);
            break;
        }
        case UserId_getDeviceStruct:
        {
            QLOGI("Device params requested");
            send_device_params(conn_handle);
            break;
        }
        case UserId_setDeviceStruct:
        {
            const int expected_len = 1 + (int)sizeof(struct ble_device_params);
            const struct ble_device_params *params = (const struct ble_device_params *)packet->payload;

            if (data_len != expected_len)
            {
                QLOGE("Device params payload size mismatch: got %d expected %d", data_len, expected_len);
                send_device_params_ack(conn_handle, BLE_DEVICE_PARAMS_STATUS_BAD_LEN);
                break;
            }

            if (params->version != BLE_DEVICE_PARAMS_VERSION)
            {
                QLOGE("Unsupported device params version: %u", params->version);
                send_device_params_ack(conn_handle, BLE_DEVICE_PARAMS_STATUS_BAD_VERSION);
                break;
            }

            apply_device_params(params);
            QLOGI("Device params updated: session=%lu ch=%u src=%02X:%02X dst=%02X:%02X",
                  (unsigned long)fira_config.Session_ID,
                  fira_config.Channel_Number,
                  fira_config.SRC_ADDR[1], fira_config.SRC_ADDR[0],
                  fira_config.DST_ADDR[1], fira_config.DST_ADDR[0]);
            send_device_params_ack(conn_handle, BLE_DEVICE_PARAMS_STATUS_OK);
            break;
        }
        default:
            /* Unsupported message ID */
            QLOGI("Unsupported message ID.");
            break;
    }
}

void ble_evt_disconnected_handler(uint16_t conn_handle)
{
    if (conn_handle == current_conn_handle)
    {
        niq_stop_uwb(conn_handle, NULL);
        current_conn_handle = BLE_NIQ_DEFAULT_CONN_HANDLE;
        return;
    }

    if (current_conn_handle != BLE_NIQ_DEFAULT_CONN_HANDLE)
    {
        QLOGW("Unknown device disconnected: %d", conn_handle);
    }
}

void ble_evt_connected_handler(uint16_t conn_handle)
{
    /* Only one connection is allowed for the NI.
       The first connection is kept. All others are ignored. */
    if (current_conn_handle == BLE_NIQ_DEFAULT_CONN_HANDLE)
    {
        current_conn_handle = conn_handle;
    }

    if (current_conn_handle != conn_handle)
    {
        QLOGI("Unknown device connected: %d", conn_handle);
        return;
    }
}
