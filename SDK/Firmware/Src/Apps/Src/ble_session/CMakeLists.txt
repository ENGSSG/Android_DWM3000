# SPDX-FileCopyrightText: Copyright (c) 2025 Qorvo US, Inc.
# SPDX-License-Identifier: LicenseRef-QORVO-2

add_library(ble_session STATIC ble_session.c)

target_include_directories(ble_session PUBLIC ./)

target_link_libraries(ble_session PUBLIC fira app_common Helpers HAL uwbstack_core uci)
