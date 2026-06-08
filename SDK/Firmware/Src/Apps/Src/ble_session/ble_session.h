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

/** Fallback UWB short address this device exposes as the controlee. */
#define BLE_SESSION_DEFAULT_LOCAL_SHORT_ADDR 0x0001

/** Length of the FFF1 OOB payload, fixed by the contract. */
#define BLE_SESSION_PARAMS_LEN 16

/** One-time init: spawn the worker task. Call from ble_init() after services_init(). */
void ble_session_init(void);

/** Set/read the local UWB short address echoed on FFF2 and used in FiRa params. */
void ble_session_set_local_short_addr(uint16_t short_addr);
uint16_t ble_session_get_local_short_addr(void);

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

/**
 * Compact per-round NLOS / link-quality record, computed from the RX
 * segment-metric diagnostics of the FiRa ranging round and streamed verbatim
 * (little-endian, packed) to the phone over the BLE FFF3 characteristic. The
 * host tracker uses it to make its face<->UWB association NLOS-aware: a
 * body-occluded back-pocket tag shows a large first-path gap and low SNR.
 * Keep this layout in sync with the Android parser (BleCentral / LinkQuality).
 */
#define BLE_SESSION_LINK_QUALITY_LEN 6
typedef struct __attribute__((packed))
{
    uint8_t nlos_score; /**< 0 = clean LOS .. 255 = heavy NLOS / occluded. */
    uint8_t rsl_q1;     /**< |RSL| in 0.5 dB units, clamped 0..255. */
    uint8_t fp_gap_q1;  /**< (RSL - first-path RSL) gap in 0.5 dB units; large => NLOS. */
    uint8_t aoa_fom;    /**< AoA figure of merit (raw, 0 if absent). */
    uint8_t seq;        /**< Wrapping per-record sequence for loss detection. */
    uint8_t flags;      /**< bit0 valid, bit1 status_success, bit2 aoa_present. */
} ble_session_link_quality_t;

#define BLE_SESSION_LQ_FLAG_VALID (1u << 0)
#define BLE_SESSION_LQ_FLAG_SUCCESS (1u << 1)
#define BLE_SESSION_LQ_FLAG_AOA_PRESENT (1u << 2)

/**
 * Link-quality sink. The FiRa worker hands the raw BLE_SESSION_LINK_QUALITY_LEN
 * bytes to the registered sink once per ranging round (when diagnostics are
 * enabled). The sink runs in the FiRa notification context, so it MUST be
 * non-blocking (e.g. a single best-effort BLE notify, dropped if the TX queue
 * is full). NULL clears the sink; when none is registered no work is done.
 *
 * Inverted dependency: the BLE transport layer (ble.c) depends on this module,
 * so it registers its FFF3 notifier here rather than being called directly.
 */
typedef void (*ble_session_link_quality_sink_t)(const uint8_t *payload, uint16_t len);
void ble_session_register_link_quality_sink(ble_session_link_quality_sink_t sink);

#ifdef __cplusplus
}
#endif
