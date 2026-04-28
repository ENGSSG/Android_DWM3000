# DWM3000 ↔ AndroidX UWB Static-STS Interop Notes

How the nRF52840DK + DWM3000 firmware was made to range against an AndroidX UWB
Pixel phone (Controller) using a BLE OOB hand-off + FiRa Static-STS Controlee
session.

## Critical fixes (in priority order)

### 1. Reverse the VENDOR_ID bytes in `vupper64`

AOSP UWB ships with `UwbFeatureFlags.isReversedByteOrderFiraParams = true` by
default (Pixel keeps this default). When that flag is true,
`ConfigurationManager.createOpenSessionParams` reverses the VENDOR_ID bytes
before the UCI `VENDOR_ID` TLV is sent to the phone's modem. The modem then
composes `V_UPPER_64` per FiRa MAC v2.0 §5.9.5 (`STATIC_STS_IV[6] || VENDOR_ID[2]`)
using the already-reversed VID.

To match on the Controlee, program `vupper64[8]` as:

```
vupper64[0..5] = sessionKey[2..7]   // STATIC_STS_IV (in order)
vupper64[6]    = sessionKey[1]      // VID byte 1
vupper64[7]    = sessionKey[0]      // VID byte 0  (reversed)
```

Symptom of getting this wrong: every round returns
`status=36 (PHY_STS_FAILED)` or `status=33 (RX_TIMEOUT)`.

Reference: AOSP `packages/modules/Uwb/service/java/com/android/server/uwb/config/ConfigurationManager.java:553-566`.

### 2. Don't tear down FiRa on BLE disconnect

The phone uses BLE strictly as a one-shot OOB channel: connect → write 16
bytes on FFF1 → read FFF2 → `BluetoothGatt.close()` → start UWB ranging on
its modem. There is no per-round BLE traffic.

The `BLE_GAP_EVT_DISCONNECTED` handler in `Src/Comm/Src/BLE/nrfx/ble.c` must
only restart advertising — never call any FiRa teardown. Otherwise ranging
dies the moment OOB completes, and the phone will keep showing AndroidX's
smoothed-stale filter output decaying toward zero (e.g.
`0.22999988 → 0.22999984 → ...`).

### 3. Belt-and-braces: also call the dedicated VID/IV setters

The Qorvo `fira_helper.h` exposes `set_session_vupper64`,
`set_session_vendor_id`, and `set_session_static_sts_iv`. Without source for
the closed-source `uwbstack` library, we can't tell whether they share
backing storage. After `fira_set_session_parameters` runs (which calls the
`vupper64` setter), call `fira_helper_set_session_vendor_id` and
`fira_helper_set_session_static_sts_iv` explicitly with the same byte
arrangement. Cheap insurance.

## Necessary-but-not-sufficient prerequisites

These had to be right for the session to start and produce the 33/36 failure
mode (instead of failing earlier), but didn't fix STS by themselves.

### 4. Explicit STS-related TLVs after `fira_set_session_parameters`

`Src/Apps/Src/fira/common_fira.c:208-258` shows that
`fira_set_session_parameters` only programs a fixed subset of session
params. It does NOT program `sts_length`, `number_of_sts_segments`,
`preamble_duration`, `psdu_data_rate`, or `key_rotation` — those are left
at the FiRa stack's compiled defaults, which aren't guaranteed to match
what AndroidX UCI implicitly assumes.

The `BLES_SET` block in `start_fira()` pins them to the AndroidX
`CONFIG_MULTICAST_DS_TWR` defaults:

```
sts_length              = FIRA_STS_LENGTH_64        (1)
number_of_sts_segments  = FIRA_STS_SEGMENTS_1       (1)
preamble_duration       = FIRA_PREAMBLE_DURATION_64 (1)
psdu_data_rate          = FIRA_PSDU_DATA_RATE_6M81  (0)
key_rotation            = 0
```

### 5. Session params alignment with `CONFIG_MULTICAST_DS_TWR`

In `apply_params_to_fira()`:

- `multi_node_mode = FIRA_MULTI_NODE_MODE_ONE_TO_MANY` (the AndroidX
  config is one-to-many even with a single peer)
- `device_type = QUWBS_FBS_DEVICE_TYPE_CONTROLEE`,
  `device_role = QUWBS_FBS_DEVICE_ROLE_RESPONDER`
- `block_duration_ms = 120`, `slot_duration_rstu = 2400`,
  `round_duration_slots = 20`, `round_hopping = true`
- `ranging_round_usage = DSTWR_DEFERRED`,
  `rframe_config = SP3`, `sfd_id = 2`
- `result_report_config` requests ToF + AoA azimuth + AoA elevation +
  AoA FoM (matches what the Pixel modem expects in the result frame)
- `ranging_round_control` set via `fira_helper_bool_to_ranging_round_control`

### 5a. Slot-schedule alignment for fast updates (added 2026-04-28)

After fixes 1–5 the session ranged but the phone HUD updated only every
several seconds, with values stuck for ~10 s and decaying through the
AndroidX filter. Root cause: `CONFIG_MULTICAST_DS_TWR` is `TIME_SCHEDULED +
ONE_TO_MANY` — the Controlee must already know the **entire slot map**
before the session starts, because no per-round Ranging Control Message
is transmitted. Anywhere the Controlee's schedule disagreed with the
phone-Controller's schedule, the poll landed outside our RX window and
the round closed as `status=33 (RX_TIMEOUT)`. Sparse successes starved
the AndroidX smoother and produced the sticky display.

The phone modem's schedule comes from AOSP
`packages/modules/Uwb/androidx_backend/src/androidx/core/uwb/backend/impl/internal/Utils.java`
(`RangingTimingParams` for `CONFIG_MULTICAST_DS_TWR`) and
`ConfigurationManager.java` (CONFIG_ID_2 block). For
`RANGING_UPDATE_RATE_FREQUENT` it's:

```
block_duration_ms       = 120        (rangingIntervalFast)
slot_duration_rstu      = 2400
slots_per_ranging_round = 20         (was 25 in our firmware → mismatch)
round_hopping           = true       (was false in our firmware → mismatch)
multi_node_mode         = ONE_TO_MANY
ranging_round_usage     = DS_TWR_DEFERRED
rframe_config           = SP3        (FiRa default — AOSP doesn't override)
schedule_mode           = TIME_SCHEDULED
in_band_termination_attempt_count = 3
result_report_config    = ToF + AoA_az + AoA_el + AoA_FoM
```

The two values that mattered most for cadence were `round_duration_slots`
(25 → 20) and `round_hopping` (false → true). Both are programmed twice
now: once in the `session_parameters` struct and again via the
`fira_helper_set_session_*` setters in the BLES_SET block, because we
proved earlier (see fix #4) that `fira_set_session_parameters` doesn't
propagate every TLV down to the modem.

Added a one-shot probe in `start_fira()` immediately before
`fira_set_session_parameters` that prints
`cadence in: block=Xms slot=YRSTU round_slots=Z`. If the line is missing
from RTT, the build wasn't rebuilt/reflashed — that mistake cost a debug
cycle.

Symptom of getting this wrong (with VID/IV byte order already correct):
ranging starts, occasional `d=Xcm` lines come through, but most rounds
return `status=33` and the phone HUD updates every several seconds with
stale-feeling values. After the fix: rounds succeed back-to-back at
~120 ms cadence and the phone HUD tracks in real time.

Reference: AOSP
[`ConfigurationManager.java`](https://github.com/GrapheneOS/platform_packages_modules_Uwb/blob/main/androidx_backend/src/androidx/core/uwb/backend/impl/internal/ConfigurationManager.java)
(CONFIG_ID_2 block at lines 113–147) and
[`Utils.java`](https://github.com/GrapheneOS/platform_packages_modules_Uwb/blob/main/androidx_backend/src/androidx/core/uwb/backend/impl/internal/Utils.java)
(`setRangingTimingParams(CONFIG_MULTICAST_DS_TWR, …)` at lines 253–262).

### 6. Clean UCI ↔ FiRa state transitions

Board boots in USB-UCI host-responder mode. When BLE OOB params arrive, UCI
must be fully torn down before bringing up FiRa Controlee — otherwise the
uwbmac / MAC / SPI subsystems are contested. The order in `start_fira` is:
`uci_terminate()` → `fira_uwb_mcps_init` → `fira_helper_open` →
`fira_prepare_measurement_sequence` → `fira_helper_init_session` →
`fira_set_session_parameters` → STS TLV pins → `uwbmac_start` →
`fira_helper_start_session`. After session stop, `uci_helper(NULL)`
reinitializes UCI from scratch.

### 7. RTT logging hygiene

Nordic's `QLOGI` macro caps at 15 args (`LOG_INTERNAL_15`); past that
you get an "implicit declaration of LOG_INTERNAL_16" compile error. Even
within that limit, long format strings hit a runtime
`WARNING: Too long to print...` truncation in the deferred logger.
Splitting OOB-param dumps into 4-byte hex chunks
(`QLOGI("BLES key0=%02X%02X%02X%02X", ...)`) was what finally made the
byte-level diagnostics readable.

## Files touched

All inside `SDK/Firmware/Src/`:

- `Apps/Src/ble_session/ble_session.c` — OOB param decode, FiRa session
  start/stop, vupper64 byte arrangement, explicit STS TLV setters
- `Comm/Src/BLE/nrfx/ble.c` — BLE event handler, the disconnect-doesn't-
  stop-FiRa fix
- `Comm/Src/BLE/nrfx/ble_fff0.{c,h}` — custom GATT service implementing
  the FFF0/FFF1/FFF2 OOB protocol the Android app expects
- `Projects/FreeRTOS/UCI/Common/cmakefiles/UCI-FreeRTOS.cmake` — added
  `ble_session` + SoftDevice subdirs, enabled `USE_SOFTDEVICE`,
  `CONFIG_LOG`, `CONFIG_QLOG_LEVEL=4`

## Per-section summary

**1. VENDOR_ID byte reversal in `vupper64`.** Pixel UWB stack reverses
the VID before TLV transmit; Controlee must mirror that arrangement or
every round fails STS verification (`status=36`). Critical, root cause.

**2. Don't tear down FiRa on BLE disconnect.** Phone closes BLE the
instant FFF1 is written, by design. Only restart advertising in the
disconnect handler — never stop FiRa. Otherwise the session dies at
hand-off and the HUD shows AndroidX's filter decaying to zero.

**3. Belt-and-braces VID/IV setters.** `fira_helper_set_session_vendor_id`
and `..._static_sts_iv` are called explicitly in addition to vupper64,
since the closed-source stack may read them independently.

**4. Explicit STS-related TLVs.** `fira_set_session_parameters` is not
exhaustive — `sts_length`, `number_of_sts_segments`, `preamble_duration`,
`psdu_data_rate`, `key_rotation` must be pinned via `fira_helper_set_*`
to match what the AndroidX UCI implicitly assumes for
`CONFIG_MULTICAST_DS_TWR`.

**5. Session params alignment with `CONFIG_MULTICAST_DS_TWR`.** Topology
and frame config (`multi_node_mode`, `device_role`, `rframe_config`,
`ranging_round_usage`, `schedule_mode`, `result_report_config` enabling
ToF+AoA) configured to match AOSP CONFIG_ID_2 defaults.

**5a. Slot-schedule alignment for fast updates.** The schedule is fully
pre-shared in `TIME_SCHEDULED + ONE_TO_MANY` — there is no per-round
RCM. `round_duration_slots = 20` (was 25) and `round_hopping = true`
(was false) brought the Controlee onto the same slot map the phone
modem programs for `RANGING_UPDATE_RATE_FREQUENT`. Each timing/topology
TLV is also pinned via `fira_helper_set_session_*`. Without this, ~30%
of rounds completed and the phone HUD lagged ~10 s; with it, rounds
succeed back-to-back at ~120 ms cadence and the HUD is real-time.

**6. Clean UCI ↔ FiRa state transitions.** Board boots in USB-UCI mode;
on OOB arrival, fully terminate UCI before bringing FiRa Controlee up
(`uci_terminate → fira_uwb_mcps_init → … → fira_helper_start_session`).
Otherwise uwbmac/MAC/SPI subsystems collide.

**7. RTT logging hygiene.** `QLOGI` is capped at 15 args
(`LOG_INTERNAL_15`) and long format strings hit a deferred-logger
truncation. Splitting OOB-param dumps into 4-byte hex chunks made
byte-level diagnostics readable.

## One-line summary

The board ranges with the phone, in real time, because (a) the VID bytes
in `vupper64[6..7]` are reversed to match AOSP
`isReversedByteOrderFiraParams=true`, (b) BLE disconnect leaves the
FiRa session alone, and (c) the Controlee's pre-shared slot schedule
(20 slots/round, hopping enabled, 120 ms block) matches the phone's
`CONFIG_MULTICAST_DS_TWR` + `RANGING_UPDATE_RATE_FREQUENT` programming
so polls land inside our RX window and rounds succeed back-to-back.
