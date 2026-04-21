#!/usr/bin/env python3

import argparse
import logging
import sys

import serial.tools.list_ports

from uci.v1_0 import Client, Device, ProtocolError


def format_ports():
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        return "  none"
    return "\n".join(
        f"  {port.device} | {port.description} | hwid={port.hwid}" for port in ports
    )


def format_config(config_items):
    lines = []
    for item, _, value in config_items:
        name = getattr(item, "name", str(item))
        lines.append(f"  {name}: {value}")
    return "\n".join(lines) if lines else "  none"


def main():
    parser = argparse.ArgumentParser(
        description="Probe an old-firmware UCI device before starting Android ranging."
    )
    parser.add_argument(
        "-p",
        "--port",
        default="/dev/ttyACM0",
        help="serial port to probe (default: %(default)s)",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="enable DEBUG logging",
        default=False,
    )
    parser.add_argument(
        "--session-id",
        type=int,
        default=42,
        help="session id used for the probe init/deinit test",
    )
    args = parser.parse_args()

    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)

    print("Available serial ports:")
    print(format_ports())

    try:
        client = Client(port=args.port)
    except Exception as exc:
        print(f"\nFailed to open {args.port}: {exc}")
        print("If the board is connected but the port is missing, this is a USB enumeration or cable issue.")
        return 2

    try:
        print(f"\nOpened {args.port}")

        info = client.info()
        print(f"core info: {info}")

        caps = client.get_caps()
        print(f"caps: {caps}")

        status, config = client.get_config(
            [Device.State, Device.LowPowerMode, Device.ChannelNumber]
        )
        print(f"device config status: {status}")
        print(format_config(config))

        session_status = client.session_init(args.session_id, 0)
        print(f"session_init({args.session_id}): {session_status}")
        if getattr(session_status, "name", "") == "Ok":
            deinit_status = client.session_deinit(args.session_id)
            print(f"session_deinit({args.session_id}): {deinit_status}")

        print(
            "\nIf this probe succeeds but `uci_uart_fira_test.py` still prints `Device -> Error`, "
            "the likely causes are incompatible firmware/script pairing or rejected ranging parameters."
        )
        print(
            "This repository expects:\n"
            "  old firmware  -> python_script/uci_uart_fira_test.py\n"
            "  new firmware  -> new_python_script/run_fira_twr.py"
        )
        return 0
    except ProtocolError as exc:
        print(f"\nUCI protocol error: {exc}")
        print(
            "The board responded unexpectedly or timed out. Common causes are wrong port, wrong firmware, "
            "or the device already being in a bad state."
        )
        return 1
    except Exception as exc:
        print(f"\nProbe failed: {exc}")
        return 1
    finally:
        client.close()


if __name__ == "__main__":
    sys.exit(main())
