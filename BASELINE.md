# Baseline Configuration & Reproducibility

The reproducible baseline for the UWB face-blur system: known-good firmware, pinned radio
parameters, build/run steps, the tracking parameter set, and a clear separation of **core** from
**experimental** code. See [ARCHITECTURE.md](ARCHITECTURE.md) and [DATA_FLOW.md](DATA_FLOW.md).

## 1. Core vs. experimental inventory

| Status | Path | Role |
|--------|------|------|
| **Core** | `DWM3000_Android_tracker/` (git submodule) | The phone app: BLE OOB, AndroidX UWB, vision, tracking/blur, UI. All entry/exit + blur logic lives here. |
| **Core** | `SDK/Firmware/Src/` | nRF52840DK + DWM3000 firmware (BLE peripheral + FiRa Controlee). |
| **Core** | `SYSTEM_OVERVIEW.md`, `INTEROP_NOTES.md` | Frozen BLE OOB contract + AOSP interop fixes. |
| Reference | `SDK/Tools/uwb-qorvo-tools/`, `Drivers/` | Vendor Qorvo tooling and DW3xxx driver API. |
| Experimental / reference | `uwb-ranging-main/` | Earlier PC-driven ranging prototype (incl. `new_python_script/run_fira_twr.py`). **No tracking/blur.** |
| Empty placeholder | `eskf/`, `tests/` | Reserved for future error-state Kalman filter / test suite; not yet implemented. |

> The Android app is a **separate git repository** pinned as a submodule. Tracking code changes are
> committed there first; the parent repo then bumps the submodule pointer.

## 2. Firmware baseline (the UWB tag)

- Known-good combined BLE + UCI/UWB image:
  `SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/nRF52840DK-UCI-FreeRTOS.hex`
  (the other `*-ble-debug-*.hex` variants in that directory are debugging builds — not baseline).
- Pinned FiRa parameters (must match the AndroidX UWB profile on the phone — see
  [INTEROP_NOTES.md](INTEROP_NOTES.md)):
  - Profile aligned to AOSP `CONFIG_ID_2` (multicast DS-TWR) + `RANGING_UPDATE_RATE_FREQUENT`
  - **120 ms** ranging block, **20 slots/round**, **round hopping ON**
  - `VENDOR_ID` bytes **reversed** in `V_UPPER_64`; explicit STS TLVs set
  - BLE disconnect must **not** tear down the FiRa session

## 3. Phone app baseline

```bash
cd DWM3000_Android_tracker
# local.properties must contain sdk.dir=<your Android SDK path>
./gradlew assembleDebug          # build
./gradlew installDebug           # flash to a connected device
```

Device requirements: a UWB-capable Android phone (e.g. Pixel 8/9 Pro). Per-device camera↔UWB
offsets live in `app/.../CalibrationConfig.kt`.

## 4. PC-side raw ranging (reference, optional)

For bench verification of the radio link only (no tracking/blur), the legacy Python client drives
the board over USB/UCI:

```bash
cd uwb-ranging-main/new_python_script
python run_fira_twr.py --port <serial-port> --session 42 --channel 9 \
    --round ds-deferred --ranging-span 120 --slot-span 2400 --slots-per-rr 6 \
    --aoa-report all-enabled --stats
```

## 5. Tracking / entry-exit parameters (baseline)

All defined in the `FacePredictionTracker` companion object
(`DWM3000_Android_tracker/app/.../tracking/FacePredictionTracker.kt`).

| Parameter | Value | Purpose |
|-----------|-------|---------|
| `MAX_PREDICTION_AGE_NS` | 1.0 s | Max age of a face track without a detection. |
| `ASSOCIATION_WINDOW_NS` | 1.5 s | Trajectory sliding window for association. |
| `MIN_ASSOCIATION_PAIRS` | 3 | Min UWB↔face pairs before a match can be considered. |
| `MAX_ASSOCIATION_COST` | 2.8 | Max trajectory cost for a match. |
| `MIN_ASSOCIATION_MARGIN` | 0.45 | Min best-vs-second-best cost margin. |
| **`ENTRY_CONFIRM_FRAMES`** | **3** | Consecutive stable frames required to CONFIRM a new target (false-positive filter). |
| **`UWB_LOSS_GRACE_NS`** | **2.0 s** | Blur kept after UWB dropout before EXIT (COASTING window). |
| **`REID_MEMORY_NS`** | **5.0 s** | How long an exited peer's offset is remembered. |
| **`REID_CONFIRM_FRAMES`** | **1** | Faster confirmation when re-identifying. |
| **`FAIL_SAFE_BLUR_DURING_COAST`** | **true** | Privacy-safe: keep blurring through transient UWB loss. |
| `REACQUIRE_CONFIRM_FRAMES` | 3 | Frames to confirm re-lock after visual loss. |
| `DEPTH_SMOOTHING_ALPHA` | 0.25 | EMA weight on UWB depth. |
| `FRONT_POCKET_ALPHA_FACE_HEIGHTS` | 4.8 | Vertical face→pocket offset prior (face-heights). |

## 6. Determinism & verification

The entry/exit logic is a pure function of (UWB samples + frame timestamps). It uses **no**
wall-clock time and **no** randomness, so a fixed input sequence always yields an identical
`TrackEvent` stream (`TrackEventLog`, Logcat tag `TrackEvent`).

**Scripted-sequence check (recommended unit test).** Feed `FacePredictionTracker` a scripted
sequence of `onCnnResult` / `predictForFrame` calls with a fake `TrackingSignalStore`, then assert
the emitted `TrackEvent` ordering. Cover at minimum:

1. Entry: `IDLE → PENDING → CONFIRMING → CONFIRMED` only after `ENTRY_CONFIRM_FRAMES`.
2. UWB dropout: `CONFIRMED → COASTING`, blur persists; after `> UWB_LOSS_GRACE_NS`,
   `COASTING → EXITED` and blur clears.
3. Re-entry within `REID_MEMORY_NS`: fast re-lock via `REID_CONFIRM_FRAMES`.
4. Two peers entering together: two locks, each face claimed once.

**On-device check.** Flash the baseline `.hex`, run the app, and use
`adb logcat -s TrackEvent` to watch the ordered transition log while walking a tagged person in,
briefly breaking UWB (move the board out of range while staying on camera), and back.
