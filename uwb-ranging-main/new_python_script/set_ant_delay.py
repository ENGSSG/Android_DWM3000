#!/usr/bin/env python3
"""Set DW3000 calibration values through UCI test_mode_calibrations_set.

Backward-compatible usage:
  python3 set_ant_delay.py <delay_dtu> [port]

Examples:
  python3 set_ant_delay.py 16366 /dev/ttyACM0
  python3 set_ant_delay.py --ant-delay 16366 --channel 9 --ants txrx -p /dev/ttyACM0
  python3 set_ant_delay.py --pg-delay 0x34 --channel 9 --ants tx -p /dev/ttyACM0
  python3 set_ant_delay.py --pg-count 0x1A --channel 9 --ants tx -p /dev/ttyACM0
  python3 set_ant_delay.py --ant-delay 16366 --pg-delay 0x34 --pg-count 0x1A --ants tx -p /dev/ttyACM0

Values are written to RAM and cleared on reset.
"""

import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from uci import Client, Gid, OidCore, OidQorvo, OidRanging, OidSession

DEFAULT_PORT = "/dev/ttyACM1"
DEFAULT_CHANNEL = 9


def silent_notif_handlers():
    return {
        (Gid.Ranging, OidRanging.Start): lambda _payload: None,
        (Gid.Qorvo, OidQorvo.TestDiag): lambda _payload: None,
        (Gid.Session, OidSession.Status): lambda _payload: None,
        (Gid.Core, OidCore.DeviceStatus): lambda _payload: None,
        ("default", "default"): lambda _gid, _oid, _payload: None,
    }


def parse_int(value):
    return int(value, 0)


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "ant_delay_pos",
        nargs="?",
        type=parse_int,
        help="backward-compatible positional antenna delay value",
    )
    parser.add_argument(
        "port_pos",
        nargs="?",
        help="backward-compatible positional serial port",
    )
    parser.add_argument(
        "-p",
        "--port",
        default=None,
        help=f"serial port to use (default: {DEFAULT_PORT})",
    )
    parser.add_argument(
        "-c",
        "--channel",
        type=int,
        choices=[5, 9],
        default=DEFAULT_CHANNEL,
        help=f"UWB channel to update (default: {DEFAULT_CHANNEL})",
    )
    parser.add_argument(
        "--ant-delay",
        type=parse_int,
        default=None,
        help="write ant<N>.ch<M>.ant_delay as a uint32 value",
    )
    parser.add_argument(
        "--pg-delay",
        type=parse_int,
        default=None,
        help="write ant<N>.ch<M>.pg_delay as a uint8 value",
    )
    parser.add_argument(
        "--pg-count",
        type=parse_int,
        default=None,
        help="write ant<N>.ch<M>.pg_count as a uint8 value",
    )
    parser.add_argument(
        "--ants",
        choices=["all", "tx", "rx", "txrx", "aoa"],
        default="all",
        help="which antenna paths to update: all=0,1,2 tx=0 rx=1 aoa=2 txrx=0,1 (default: %(default)s)",
    )

    args = parser.parse_args()

    if args.ant_delay is not None and args.ant_delay_pos is not None:
        parser.error("use either the positional antenna delay or --ant-delay, not both")

    if (
        args.ant_delay is None
        and args.ant_delay_pos is None
        and args.pg_delay is None
        and args.pg_count is None
    ):
        parser.error(
            "specify at least one of: positional antenna delay, --ant-delay, --pg-delay, --pg-count"
        )

    return args


def ant_selection(mode):
    if mode == "all":
        return (0, 1, 2)
    if mode == "tx":
        return (0,)
    if mode == "rx":
        return (1,)
    if mode == "txrx":
        return (0, 1)
    if mode == "aoa":
        return (2,)
    raise ValueError(f"unsupported antenna selection: {mode}")


def ensure_uint(value, bits, name):
    max_value = (1 << bits) - 1
    if value < 0 or value > max_value:
        raise ValueError(f"{name} must be in range 0..{max_value} (got {value})")
    return value


def format_value(value_bytes):
    value = int.from_bytes(value_bytes, "little")
    return f"{value} (0x{value:X})"


def main():
    args = parse_args()

    port = args.port or args.port_pos or DEFAULT_PORT
    ant_delay = args.ant_delay if args.ant_delay is not None else args.ant_delay_pos
    ants = ant_selection(args.ants)

    updates = []
    for ant in ants:
        prefix = f"ant{ant}.ch{args.channel}"
        if ant_delay is not None:
            value = ensure_uint(ant_delay, 32, "ant_delay").to_bytes(4, "little")
            updates.append((f"{prefix}.ant_delay", value))
        if args.pg_delay is not None:
            value = ensure_uint(args.pg_delay, 8, "pg_delay").to_bytes(1, "little")
            updates.append((f"{prefix}.pg_delay", value))
        if args.pg_count is not None:
            value = ensure_uint(args.pg_count, 8, "pg_count").to_bytes(1, "little")
            updates.append((f"{prefix}.pg_count", value))

    client = Client(port=port, notif_handlers=silent_notif_handlers())
    try:
        for key, value in updates:
            rts, _ = client.test_mode_calibrations_set([(key, value)])
            print(f"{key:<24} = {format_value(value):<12} -> {rts.name}")
    finally:
        client.close()


if __name__ == "__main__":
    main()
