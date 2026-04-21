/**
 * @file      HAL_usb.h
 *
 * @brief     Interface for HAL_usb
 *
 * @author    Qorvo Applications
 *
 * @copyright SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo US, Inc.
 *            SPDX-License-Identifier: LicenseRef-QORVO-2
 *
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "comm_helpers.h"

struct hal_usb_s
{
    void (*init)(CommRxCallback rx_callback);
    void (*deinit)(void);
    bool (*transmit)(uint8_t *tx_buffer, int size);
    void (*receive)(void);
    void (*update)(void);
    bool (*isTxBufferEmpty)(void);
};

extern const struct hal_usb_s Usb;

/* Enable USB power-event detection after SoftDevice has been enabled.
 * Must be called post-SD because it uses SD SVCs on nRF52840 builds. */
void deca_usb_start_power_events(void);
