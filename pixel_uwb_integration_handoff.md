# UCI nRF52840DK + DWM3000 to `pixel_uwb` Handoff

## Current Working Baseline

- Working firmware base: stock Qorvo `UCI` target for `nRF52840DK`
- Target file: `SDK/Firmware/Projects/FreeRTOS/UCI/nRF52840DK/project_UCI.cmake`
- Build system entry: `SDK/Firmware/Projects/FreeRTOS/UCI/Common/cmakefiles/UCI-FreeRTOS.cmake`
- Build output directory: `SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release`
- Main image: `SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/nRF52840DK-UCI-FreeRTOS.hex`

## Firmware Patch Required for DWM3000

The key firmware fix that made `nRF52840DK + DWM3000` work with Android/UCI was in:

- `SDK/Firmware/Src/Boards/Src/nRF52840DK/platform_l1_config.c`

What changed:

- accept `PLATFORM_ID_DW3000`
- accept `PLATFORM_ID_DW3001C`
- keep existing `QM33110` / `QM33120` handling
- keep the `DW3001C PVT2` channel-5 antenna-delay workaround

Why it mattered:

- the stock `nRF52840DK` board path was effectively assuming QM331x OTP layouts
- `DWM3000` / `DW3001C` OTP handling needed to be accepted on the nRF52840 host build

## Board-Specific Pins

Current UWB pin mapping is in:

- `SDK/Firmware/Projects/FreeRTOS/UCI/nRF52840DK/ProjectDefinition/uwb_stack_llhw.cmake`

If the custom `nRF52840 + DWM3000` board differs from the DK mapping, this file must be updated.

## Calibration Status

### Bias

- `ant_delay` was the main bias knob
- settled value for now: `16366`

Practical note:

- on this setup, empirical tuning showed that increasing `ant_delay` moved the measured distance down
- final value was chosen empirically, not from a fixed theoretical conversion

### Jitter

- `ant_delay` is for bias, not jitter
- next board-side knobs identified for jitter:
  - `ant0.ch9.pg_delay`
  - `ant0.ch9.pg_count`

The Python helper was extended to support:

- `--ant-delay`
- `--pg-delay`
- `--pg-count`
- `--channel 5|9`
- `--ants all|tx|rx|txrx|aoa`

File:

- `uwb-ranging-main/new_python_script/set_ant_delay.py`

## Python Tooling Changes

### Compact ranging output

File:

- `uwb-ranging-main/new_python_script/run_fira_twr.py`

Added:

- `--compact`
- running average / median
- `--trim-outliers`
- `--csv`

Important note:

- `655.35 m` is an invalid/sentinel value and should not be treated as real range data

### Calibration helper

File:

- `uwb-ranging-main/new_python_script/set_ant_delay.py`

Current purpose:

- write calibration keys through UCI test mode
- suppress noisy default ranging notifications while attached

## UCI vs `pixel_uwb`

### How UCI works

`UCI` is host-driven.

Flow:

1. board boots the UCI app
2. host connects over transport, currently USB
3. host sends UCI commands
4. board applies those commands to create/manage ranging sessions
5. board returns UCI responses and notifications

Relevant files:

- `SDK/Firmware/Projects/FreeRTOS/UCI/Common/main.c`
- `SDK/Firmware/Src/Comm/CMakeLists.txt`
- `SDK/Firmware/Src/Apps/Src/uci`

Important fact:

- current `UCI` build uses USB transport only
- `SDK/Firmware/Src/Comm/CMakeLists.txt` currently builds `Interface` from `Src/InterfUsb.c`

### How `pixel_uwb` works

`pixel_uwb` is Android-app-driven.

Flow:

1. Android app creates a UWB scope using AndroidX UWB APIs
2. app exchanges out-of-band session data over BLE
3. Android system UWB service starts ranging using those parameters
4. app receives ranging results from AndroidX UWB APIs

Relevant app files:

- `~/pixel_uwb/app/src/main/java/com/example/pixeluwb/uwb/UwbRangingManager.kt`
- `~/pixel_uwb/app/src/main/java/com/example/pixeluwb/ble/BleCentral.kt`
- `~/pixel_uwb/app/src/main/java/com/example/pixeluwb/ble/BlePeripheral.kt`
- `~/pixel_uwb/app/src/main/java/com/example/pixeluwb/UwbSessionParams.kt`

## Why `pixel_uwb` Does Not Talk to Current Firmware

The mismatch is the BLE control plane.

`pixel_uwb` expects:

- BLE service UUID `FFF0`
- params characteristic `FFF1`
- response characteristic `FFF2`
- a fixed 16-byte payload defined in `UwbSessionParams.kt`

Current firmware BLE path does not expose that contract.

Existing firmware BLE stack is here:

- `SDK/Firmware/Src/Comm/Src/BLE/nrfx/ble.c`
- `SDK/Firmware/Src/Comm/Src/BLE/nrfx/qnis/ble_qnis.c`
- `SDK/Firmware/Src/Comm/Src/BLE/niq/ble_niq.c`

That BLE path is NI/QNIS-oriented, not `FFF0/FFF1/FFF2`.

## BLE Investigation Result

The SDK already has a BLE runtime, but it is wired into `QANI`, not `UCI`.

Files:

- `SDK/Firmware/Projects/FreeRTOS/QANI/Common/cmakefiles/QANI-FreeRTOS.cmake`
- `SDK/Firmware/Src/Comm/Src/BLE/CMakeLists.txt`

Key fact:

- `QANI` pulls in BLE + SoftDevice
- `UCI` does not

## BLE Parameter Exchange Work Already Added

An initial versioned parameter-exchange contract was added to:

- `SDK/Firmware/Src/Comm/Src/BLE/niq/ble_niq.c`

Implemented message IDs:

- `0x20` get device params
- `0x21` set device params
- `0x22` set params ack

This was built successfully on the BLE-enabled target:

- `SDK/Firmware/BuildOutput/QANI/FreeRTOS/nRF52840DK/Release`

This is useful as a reference, but it does not make `pixel_uwb` compatible by itself.

## Recommended Integration Direction

If the goal is to work with `pixel_uwb` with minimum Android changes:

1. keep the working `UCI` firmware baseline
2. add BLE runtime support into the `UCI` build
3. implement the `FFF0/FFF1/FFF2` BLE contract exactly as `pixel_uwb` expects
4. keep USB UCI intact for debug / fallback

Reason:

- this preserves the current known-good UCI/UWB behavior
- it adds a BLE side channel for wireless OOB parameter exchange
- it avoids rewriting `pixel_uwb` to match QNIS

## Ordered Task List for `pixel_uwb` Compatibility

1. Freeze current working `UCI` firmware and calibration baseline.
2. Keep `platform_l1_config.c` DW3000/DW3001C OTP patch unchanged.
3. Port BLE runtime pieces from `QANI` into the `UCI` build:
   - BLE library
   - SoftDevice inclusion
   - init sequence
4. Add a BLE service matching `pixel_uwb`:
   - `FFF0`
   - `FFF1`
   - `FFF2`
5. Parse the 16-byte `pixel_uwb` BLE payload on firmware side.
6. Return the board UWB short address over the response characteristic.
7. Feed the BLE parameters into the board-side UWB session config.
8. Start ranging only after BLE OOB exchange completes successfully.
9. Test wireless phone-to-board ranging without PC involvement.

## Cautions Before Making Changes

### Architecture

- Do not conflate `UCI` and `QANI`; they are different app stacks.
- Do not replace USB UCI when adding BLE; add BLE as a side channel first.

### BLE

- `pixel_uwb` does not speak the current QNIS/NIQ BLE protocol.
- Reusing the existing QNIS stack without adapting the Android app will not work.

### Session Ownership

- `UCI` assumes a host commands the accessory.
- `pixel_uwb` assumes Android orchestrates the session over BLE OOB and Android UWB APIs.
- If both control models are active simultaneously, session state can conflict.

### Memory / Build Risk

- Pulling BLE into `UCI` changes memory footprint, startup path, and image composition.
- Keep changes staged:
  - first build with BLE runtime
  - then add simple advertising / service
  - then add parameter exchange
  - then start UWB from BLE data

### Calibration

- Do not mix calibration work into the first BLE milestone.
- First BLE milestone should only prove:
  - BLE discovery
  - parameter exchange
  - clean UWB session start

## Operational Expectation After BLE Is Added

If BLE OOB is implemented correctly in the `UCI`-based firmware:

- the phone should not need to be physically connected to the computer
- the phone should communicate wirelessly with the board
- the PC would still be useful for flashing, logs, and debug

## Next Recommended Step

Implement BLE in the `UCI` build with a `pixel_uwb`-compatible GATT contract, rather than modifying `pixel_uwb` to speak the current QNIS protocol.
