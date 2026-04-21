# UWB Troubleshooting

## Old vs new firmware/script pairing

The repository contains two different host-script stacks:

- `python_script/uci_uart_fira_test.py` for the old firmware in `firmware/`
- `new_python_script/run_fira_twr.py` for the newer firmware in `new_firmware/`

If the board was reflashed and the old script is still being used, the device can enter a UCI error state even though the serial port still opens.

## Quick probe for the old stack

Before running ranging, probe the port and basic UCI commands:

```bash
cd uwb-ranging-main/python_script
python device_probe.py -p /dev/ttyACM0
```

What the results mean:

- open failure: host cannot see the board on USB or the port name is wrong
- timeout / protocol error: wrong port, board wedged, or wrong firmware for this script
- `session_init(...): Ok` but ranging later fails: the transport is fine and the failure is likely in session config/start

## Most likely causes of `Device -> Error`

- The wrong script is being used for the currently flashed firmware.
- The board is no longer enumerating on the same serial node and the script is pointed at the wrong port.
- The firmware is entering an error state after session configuration because the parameter set no longer matches the flashed UCI build.
