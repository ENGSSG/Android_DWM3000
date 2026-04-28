/**
 * @file      ble_session.h
 *
 * @brief     BLE-OOB driven FiRa Controlee session bring-up.
 *
 *            The BLE FFF1 write callback drops the 16-byte OOB payload here.
 *            A worker task tears down UCI, applies the OOB params, and starts
 *            a FiRa Static-STS Controlee session. On BLE disconnect the worker
 *            stops the session and restarts UCI, so USB UCI is the resting state.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** UWB short address this device exposes as the controlee (also echoed on FFF2). */
#define BLE_SESSION_LOCAL_SHORT_ADDR 0x0001

/** Length of the FFF1 OOB payload, fixed by the contract. */
#define BLE_SESSION_PARAMS_LEN 16

/** One-time init: spawn the worker task. Call from ble_init() after services_init(). */
void ble_session_init(void);

/**
 * Hand off a freshly-received FFF1 payload to the worker.
 * Safe to call from the SoftDevice/BLE callback context.
 */
void ble_session_submit_params(const uint8_t payload[BLE_SESSION_PARAMS_LEN], uint16_t conn_handle);

/**
 * Hand off a BLE disconnect event to the worker so it can stop ranging and
 * hand control back to UCI. No-op if no FiRa session is active.
 */
void ble_session_submit_disconnect(void);

#ifdef __cplusplus
}
#endif
