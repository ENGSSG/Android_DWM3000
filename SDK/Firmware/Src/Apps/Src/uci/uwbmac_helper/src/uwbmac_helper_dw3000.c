/**
 * @file      uwbmac_helper_dw3000.c
 *
 * @brief     UWBMAC helper init and configuration
 *
 * @author    Qorvo Applications
 *
 * @copyright SPDX-FileCopyrightText: Copyright (c) 2024-2025 Qorvo US, Inc.
 *            SPDX-License-Identifier: LicenseRef-QORVO-2
 *
 */

#include "uwbmac_helper.h"
#include "llhw.h"
#include "qplatform.h"
#include "reporter.h"
#include "SEGGER_RTT.h"

extern struct l1_config_platform_ops l1_config_platform_ops;

static void rtt_trace(const char *msg)
{
    SEGGER_RTT_WriteString(0, msg);
    SEGGER_RTT_WriteString(0, "\r\n");
}

/** @brief Setup the UWB chip. */
int uwbmac_helper_init_fira(void)
{
    int ret;
    (void)ret;

    rtt_trace("uwbmac: before qplatform_init");
    ret = qplatform_init();
    if (ret != QERR_SUCCESS)
    {
        rtt_trace("uwbmac: qplatform_init failed");
        reporter_instance.print("qplatform_init failed", strlen("qplatform_init failed"));
        return ret;
    }
    rtt_trace("uwbmac: after qplatform_init");

    ret = l1_config_init(&l1_config_platform_ops);
    if (ret != QERR_SUCCESS)
    {
        rtt_trace("uwbmac: l1_config_init failed");
        reporter_instance.print("l1_config_init failed", strlen("l1_config_init failed"));
        return ret;
    }
    rtt_trace("uwbmac: after l1_config_init");

    ret = llhw_init();
    if (ret != QERR_SUCCESS)
    {
        rtt_trace("uwbmac: llhw_init failed");
        reporter_instance.print("llhw_init failed", strlen("llhw_init failed"));
        return ret;
    }
    rtt_trace("uwbmac: after llhw_init");

    return 0;
}

void uwbmac_helper_deinit(void)
{
    llhw_deinit();
    l1_config_deinit();
    qplatform_deinit();
}
