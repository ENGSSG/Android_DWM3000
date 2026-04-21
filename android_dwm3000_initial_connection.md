# Android to DWM3000 + nRF52840 Initial Connection Analysis

## Goal

Establish an initial working connection between an Android phone (Pixel 8 Pro) and a custom accessory built from:

- DWM3000 (UWB radio module based on DW3110)
- nRF52840 (host MCU, BLE, session control)

The immediate objective is **not** multi-node scheduling.  
The objective is to reproduce the kind of successful ranging behavior previously achieved with DWM3001, but now on a custom DWM3000 + nRF52840 platform.

---

## Known Starting Point

We already verified that Android UWB ranging can work using:

- Pixel 8 Pro
- DWM3001
- UCI-based example from:
  - https://github.com/sasodoma/uwb-ranging
- Fixed session setup
- Android app + Python script for manual OOB/session coordination

That means the Android side is already proven in principle.

The challenge is now porting that success to:

- DWM3000
- nRF52840 host
- custom firmware path
- likely BLE-based or equivalent OOB setup instead of Python/manual USB flow

---

## Main Insight

The hard part is **not** whether Android can connect to a UCI-based accessory.  
That was already demonstrated with DWM3001.

The hard part is the migration from:

- integrated, calibrated, known-good DWM3001 platform

to:

- DWM3000 + external host MCU (nRF52840)
- custom integration
- custom calibration / OTP handling
- custom control path

In other words:

**Android interoperability is already partially validated.  
The main new difficulty is embedded integration and session bootstrapping.**

---

## What Must Be Done

There are four major pieces to implement.

### 1. Port UWB control from DWM3001-style example to DWM3000 + nRF52840

Need to confirm or implement:

- SPI communication between nRF52840 and DWM3000
- IRQ handling
- reset / wake handling
- DW3000 initialization sequence
- TX/RX flow
- timestamp readout
- UCI command handling or equivalent session control logic

Questions Codex should answer:
- What parts of the DWM3001-based example depend on integrated hardware assumptions?
- Which driver layers need to be rewritten for nRF52840?
- What is the minimum bring-up sequence for DW3000 on nRF52840?

---

### 2. Replace manual Python-based OOB/session control

In the DWM3001 proof-of-concept, session parameters were coordinated manually using Python.

That is not suitable for a standalone Android-to-accessory connection.

Need a replacement mechanism, likely:

- BLE advertisement for discovery
- BLE GATT for out-of-band session parameter exchange

Phone should send or negotiate:
- MAC address / device address
- session ID
- channel / preamble / config information
- start command
- role information if needed

Questions Codex should answer:
- What exact parameters from the working DWM3001 flow must be exchanged out-of-band?
- How should a BLE GATT service be designed for this?
- Which fields are fixed for first prototype and which should remain configurable?

---

### 3. Handle calibration / OTP correctly on DWM3000

Important observation:
- DWM3001 worked clearly out of the box
- DWM3000 + nRF52840 required OTP adjustment to get valid readings

This suggests the custom hardware path has extra sensitivity in:
- antenna delay
- calibration values
- OTP configuration
- timing correctness

Questions Codex should answer:
- Which OTP/calibration settings are required for DW3000/DWM3000 to produce reliable ranging?
- Which of these can be hardcoded temporarily for prototype validation?
- How should calibration be separated from protocol debugging?

Important engineering principle:
- Do not mix protocol/interoperability debugging with calibration debugging.
- First establish correct initialization and message exchange.
- Then validate ranging quality.

---

### 4. Recreate a fixed-session Android connection first

Before attempting dynamic negotiation, first reproduce the known-good fixed-session model.

Recommended first milestone:
- fixed UWB parameters
- fixed session ID
- fixed role assignment
- fixed BLE payload or hardcoded values
- one phone, one accessory

Questions Codex should answer:
- What is the smallest end-to-end flow needed to reproduce the DWM3001 working path on DWM3000+nRF52840?
- What should be hardcoded first to reduce variables?
- What logs should be collected on both Android and nRF52840 sides?

---

## Recommended Development Order

### Phase 1: DWM3000 Bring-up
Implement and verify:
- SPI
- IRQ
- reset
- init
- TX/RX
- timestamps

Success criterion:
- nRF52840 can reliably talk to DWM3000 and send/receive frames

---

### Phase 2: UWB Session Control
Implement enough control logic to:
- configure fixed session parameters
- start ranging behavior
- observe valid message exchange

Success criterion:
- accessory behaves like the DWM3001 example at the UWB level

---

### Phase 3: OOB Replacement
Replace Python/manual control with BLE:
- advertisement
- connect
- GATT exchange
- session start trigger

Success criterion:
- Android app can configure the accessory without USB/Python

---

### Phase 4: Calibration Validation
After protocol flow works:
- tune OTP / antenna delay / calibration
- validate measurement stability

Success criterion:
- clear and repeatable ranging values

---

## Likely Pitfalls

### 1. Assuming Android is the main blocker
It probably is not.
The DWM3001 proof-of-concept already showed Android compatibility is possible.

### 2. Debugging calibration and protocol at the same time
This will create confusion.
Need to isolate:
- connection/setup failures
from
- poor ranging quality

### 3. Porting too much at once
Do not begin with:
- dynamic sessions
- multi-node support
- custom scheduling
- advanced BLE logic

Start with:
- single accessory
- fixed session
- minimum OOB exchange

### 4. Assuming DWM3001 example directly maps to DWM3000
It does not fully map because DWM3001 hides some integration complexity:
- integrated MCU path
- validated RF design
- known firmware environment

---

## What Codex Should Produce

Codex should help generate:

### A. Embedded Bring-up Plan
A step-by-step plan for:
- nRF52840 ↔ DWM3000 initialization
- required driver layers
- expected test checkpoints

### B. Minimal Firmware Architecture
Modules such as:
- dw3000_driver.c
- uwb_session.c
- ble_oob.c
- main_state_machine.c

### C. BLE OOB Design
Define:
- GATT service
- characteristics
- payload format
- session start sequence

### D. Fixed-Session Prototype Plan
A minimal path that reproduces the known DWM3001 success case with:
- one Android phone
- one DWM3000+nRF52840 board
- hardcoded parameters first

### E. Debug Strategy
Separate logs and checks for:
- hardware/SPI issues
- session configuration mismatch
- OTP/calibration issues
- ranging quality issues

---

## Concrete Request to Codex

Please analyze this project as an engineering migration problem:

We already have a working Android ↔ DWM3001 ranging proof-of-concept using the `sasodoma/uwb-ranging` example and a fixed session with manual OOB coordination.

Now we want to build the initial Android ↔ DWM3000+nRF52840 connection.

Please provide:

1. A step-by-step implementation plan
2. The minimum firmware modules needed on nRF52840
3. The BLE OOB design required to replace the Python script
4. The likely DW3000 bring-up and porting issues
5. A debugging checklist that separates:
   - hardware bring-up
   - session/control mismatch
   - OTP/calibration errors
6. A recommended order of implementation that minimizes risk

Focus only on:
- one Android phone
- one DWM3000+nRF52840 accessory
- fixed session first

Do not focus on:
- multi-node ranging
- collision avoidance
- MAC scheduling
- advanced scalability yet
