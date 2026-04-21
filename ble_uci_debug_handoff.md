# BLE UCI Debug Handoff

## Goal
Bring BLE up inside the `UCI` firmware for `nRF52840DK + DWM3000` without breaking the existing USB/UCI ranging path.

## Known-Good Baseline
- Working non-BLE image:
  - `SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/nRF52840DK-UCI-FreeRTOS-working-dwm3000.hex`
- This is the patched UCI build that works with DWM3000 and Python UCI tools.
- Critical board patch:
  - `SDK/Firmware/Src/Boards/Src/nRF52840DK/platform_l1_config.c`
- Antenna delay tuning settled at:
  - `16366`

## Why Stock UCI Was Not Enough
- Stock prebuilt UCI image booted, but reverting to it lost the DWM3000 OTP/platform fixes.
- Result after reverting to stock:
  - USB ACM came back
  - Python calibration/ranging path no longer behaved like the working DWM3000-patched build

## BLE Integration Attempt
BLE was added into the `UCI` build by modifying:
- `SDK/Firmware/Projects/FreeRTOS/UCI/Common/main.c`
- `SDK/Firmware/Projects/FreeRTOS/UCI/Common/cmakefiles/UCI-FreeRTOS.cmake`
- `SDK/Firmware/Projects/FreeRTOS/UCI/nRF52840DK/project_UCI.cmake`
- `SDK/Firmware/Projects/FreeRTOS/UCI/nRF52840DK/nRF52840.ld`
- `SDK/Firmware/Projects/FreeRTOS/UCI/nRF52840DK/ProjectDefinition/sdk_config.h`
- `SDK/Firmware/Src/Comm/Src/BLE/nrfx/ble.c`

## SoftDevice / Memory Layout
The BLE-enabled `UCI` build needed the SoftDevice layout, mirroring `QANI`:
- `.soft_device` at `0x00000000`
- `.application` at `0x0001C000`
- RAM origin at `0x20002700`

Current linker file:
- `SDK/Firmware/Projects/FreeRTOS/UCI/nRF52840DK/nRF52840.ld`

## Current BLE Source State
`ble.c` is currently a minimal advertising-only implementation, not the original QNIS/ANIS service stack:
- file:
  - `SDK/Firmware/Src/Comm/Src/BLE/nrfx/ble.c`
- behavior:
  - minimal SoftDevice enable
  - GAP name set
  - bare advertising init/start
  - no custom services
  - `nrf_sdh_freertos_init(advertising_start, NULL)`

## Debugging Method Used
Direct RTT prints were added with `SEGGER_RTT_WriteString(...)`.

### `main.c` RTT markers
File:
- `SDK/Firmware/Projects/FreeRTOS/UCI/Common/main.c`

Markers currently present:
- `main: BoardInit done`
- `main: before create_log_processing_task`
- `main: after create_log_processing_task`
- `main: AppConfigInit done`
- `main: starting BLE`
- `main: ble_init returned`
- `main: board_interface_init done`
- `main: FlushTaskInit done`
- `main: ControlTaskInit done`
- `main: UCI helper initialized`
- `main: starting scheduler`

### `log_processing.c` RTT markers
File:
- `SDK/Firmware/Src/Logger/log_processing.c`

Markers currently present:
- `log: create task begin`
- `log: init_log begin`
- `log: NRF_LOG_INIT returned`
- `log: default backends init done`
- `log: init_log done`
- `log: qsignal_init ok|failed`
- `log: qmalloc ok|failed`
- `log: qthread_create ok|failed`

## Important Finding
The previous debug image stopped after:
- `main: BoardInit done`

That was misleading at first because it looked like `create_log_processing_task()` was failing.

Actual cause:
- `QLOGI("main: BoardInit done");` was being called before logger initialization.
- That early `QLOGI` call itself was killing startup.

Fix applied:
- removed early `QLOGI` calls from `main.c`
- kept direct RTT prints only

## Latest RTT Result
After removing the premature `QLOGI` calls, RTT progressed through logger init and BLE init:

- `log: default backends init done`
- `log: init_log done`
- `log: qsignal_init ok`
- `log: qmalloc ok`
- `log: qthread_create ok`
- `main: after create_log_processing_task`
- `main: AppConfigInit done`
- `main: starting BLE`
- `ble: init begin`
- `ble: disable request`
- `ble: softdevice enabled`
- `ble: default cfg set`
- `ble: BLE enabled`
- `ble: GAP name set`
- `ble: advertising_init done`
- `ble: starting freertos helper`
- `ble: init end`
- `main: ble_init returned`

Then the firmware failed with:

- `ERROR 8 [NRF_ERROR_INVALID_STATE]`
- file:
  - `SDK/Firmware/SDK_BSP/Nordic/SDK_17_1_0/components/libraries/usbd/app_usbd.c:863`

## Root Cause of Current Failure
The current failure is not in BLE bring-up.

It is a USB init ordering conflict:
- BLE enables SoftDevice first
- later `board_interface_init()` calls `Usb.init(...)`
- `app_usbd_init()` calls `nrf_drv_power_init(NULL)`
- with `SOFTDEVICE_PRESENT`, `nrf_drv_power_init()` returns `NRF_ERROR_INVALID_STATE` if SoftDevice is already enabled

Relevant files:
- `SDK/Firmware/SDK_BSP/Nordic/SDK_17_1_0/components/libraries/usbd/app_usbd.c`
- `SDK/Firmware/SDK_BSP/Nordic/SDK_17_1_0/integration/nrfx/legacy/nrf_drv_power.c`
- `SDK/Firmware/Src/HAL/Src/nrfx/HAL_usb.c`
- `SDK/Firmware/Src/Boards/Src/nRF52840DK/FreeRTOS/nRF52840DK.c`

## Fix Applied
I changed init order in:
- `SDK/Firmware/Projects/FreeRTOS/UCI/Common/main.c`

New order:
1. `BoardInit()`
2. logger init
3. `AppConfigInit()`
4. `board_interface_init()`
5. `ble_init()`

Reason:
- USB init must happen before SoftDevice enable, otherwise `app_usbd_init()` fails in the Nordic power driver path.

## Current Debug Image
Latest image built after fixing USB/BLE init order:
- `SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/nRF52840DK-UCI-FreeRTOS-ble-debug-usb-first.hex`

This is the image that should be flashed next for continued BLE bring-up.

## Next Step
Flash:

```bash
nrfjprog --program /home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/nRF52840DK-UCI-FreeRTOS-ble-debug-usb-first.hex --chiperase --verify --reset -f NRF52 --snr 1050210311
```

Then inspect RTT with:

```bash
JLinkExe -device nRF52840_XXAA -if SWD -speed 4000 -autoconnect 1
```

At `J-Link>`:

```text
r
g
```

Then:

```bash
JLinkRTTClient
```

## What To Look For Next
After flashing the new image, verify:
- Nordic USB ACM device comes back
- BLE advertising appears in nRF Connect
- RTT no longer shows the `app_usbd.c:863` invalid-state error

If it still fails after this change, the next failure point will be after USB init, not the logger and not the previous USB power-driver conflict.

## Flash Targets To Keep
- Working non-BLE ranging baseline:
  - `SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/nRF52840DK-UCI-FreeRTOS-working-dwm3000.hex`
- Current BLE debug image:
  - `SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/nRF52840DK-UCI-FreeRTOS-ble-debug-usb-first.hex`
