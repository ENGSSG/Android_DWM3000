/**
 * @file      main.c
 *
 * @brief     FreeRTOS main
 *
 * @author    Qorvo Applications
 *
 * @copyright SPDX-FileCopyrightText: Copyright (c) 2024-2025 Qorvo US, Inc.
 *            SPDX-License-Identifier: LicenseRef-QORVO-2
 *
 */

#include "qos.h"
#include "app.h"
#include "thisBoard.h"
#include "controlTask.h"
#include "HAL_error.h"
#include "HAL_cpu.h"
#include "HAL_usb.h"
#include "flushTask.h"
#include "defaultTask.h"
#include <stddef.h>
#include <stdio.h>
#include "app_error.h"
#include "boards.h"
#include "nrf.h"
#include "qlog.h"
#include "SEGGER_RTT.h"
#if CONFIG_LOG
#include <log_processing.h>
#endif

#define DEAD_BEEF 0xDEADBEEF

extern void ble_init(char *gap_name);

static void rtt_trace(const char *msg)
{
    SEGGER_RTT_WriteString(0, msg);
    SEGGER_RTT_WriteString(0, "\r\n");
}

void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static char advertising_name[32];

int main(void)
{
    handle_fpu_irq();
    BoardInit();
    bsp_board_leds_off();
    bsp_board_led_on(BSP_BOARD_LED_0);
    rtt_trace("main: BoardInit done");
#if CONFIG_LOG
    rtt_trace("main: before create_log_processing_task");
    /* Start Log processing task, right after configuring clock sources. */
    create_log_processing_task();
    rtt_trace("main: after create_log_processing_task");
#endif

    AppConfigInit();
    bsp_board_led_on(BSP_BOARD_LED_1);
    rtt_trace("main: AppConfigInit done");

    board_interface_init();
    rtt_trace("main: board_interface_init done");

    snprintf(advertising_name, sizeof(advertising_name), "UCI-%08X", (unsigned int)NRF_FICR->DEVICEADDR[0]);
    rtt_trace("main: starting BLE");
    ble_init(advertising_name);
    bsp_board_led_on(BSP_BOARD_LED_2);
    rtt_trace("main: ble_init returned");

    /* SD is up — enable USB power-event detection so USB CDC-ACM enumerates. */
    deca_usb_start_power_events();
    rtt_trace("main: usb power events enabled");

    /* Initialize interface output thread. */
    FlushTaskInit();
    rtt_trace("main: FlushTaskInit done");

    /* Initialize interface input thread. */
    ControlTaskInit();
    rtt_trace("main: ControlTaskInit done");

    /* Start UCI thread. */
    extern const app_definition_t helpers_uci_node[];
    AppSet(helpers_uci_node);
    AppGet()->helper(NULL);
    bsp_board_led_on(BSP_BOARD_LED_3);
    rtt_trace("main: UCI helper initialized");

    /* Start scheduler */
    rtt_trace("main: starting scheduler");
    qos_start();

    /* We should never get here as control is now taken by the scheduler. */
    while (1)
    {
    }

    return 0;
}
