# SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo US, Inc.
# SPDX-License-Identifier: LicenseRef-QORVO-2

"""
This script serves as a helper for managing build and flash tasks in VSCode.
It allows tasks to store example type, board type, and CPU type in the project_config.json file.
The script is designed with specific functionality and is intended to be utilized solely with
the provided tasks.json file.

Usage:
    python script.py <example> <board>

Arguments:
    example: Example, available options are in examples list
    board: Board to be used, should be one of the keys in cpu_mapping dict
    build: Build type, available options are in builds list
    flags: Custom flags for the build, only available with build type 'Custom'

This script generates a json file named project_config.json with the following structure:
{
    "example": <example>,
    "board": <board>,
    "cpu": <cpu>,
    "build": <build>,
    "flags": <flags>
}

Where <cpu> is a value from cpu_mapping dict based on the <board> value.
"""

import json
import sys
import argparse

# CPU mapping based on board
cpu_mapping = {
    "DWM3001CDK": "nrf52833",
    "nRF52840DK": "nrf52840",
    "Type2AB_EVB": "nrf52840"
}

# Allowed board-example combinations
build_board_mapping = {
    "HelloWorld": ["DWM3001CDK", "nRF52840DK"],
    "QANI": ["DWM3001CDK", "nRF52840DK", "Type2AB_EVB"],
    "UCI": ["DWM3001CDK", "nRF52840DK", "Type2AB_EVB"],
    "CLI": ["DWM3001CDK", "nRF52840DK", "Type2AB_EVB"]
}

builds = ["Debug", "Release", "RelWithDebInfo", "MinSizeRel", "Custom"]

class CustomFlagsAction(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        if getattr(namespace, 'build', None) != 'Custom' and values != "":
            parser.error(f"{self.dest} parameter can only be set if --build is 'Custom' or if flags are an empty string")
        setattr(namespace, self.dest, values)

def create_json_file(example, board, build, flags):
    f"""
    Creates a json file with project configuration.

    Args:
        example: Example, available options are {build_board_mapping.keys()}
        board: Board to be used, available options are {cpu_mapping.keys()}. Must be a valid option for the specified example.
        build: Build type, available options are {builds}
        flags: Custom flags for the build, only available with build type 'Custom'.
    """
    data = {
        "example": example,
        "board": board,
        "cpu": cpu_mapping.get(board),
        "build": build,
        "flags": flags
    }
    with open('../project_config.json', 'w', encoding='utf-8') as json_file:
        json.dump(data, json_file, indent=4)
        json_file.write('\n')

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="This script manages build and flash tasks in VSCode.")
    parser.add_argument('example', choices=build_board_mapping.keys(), help="Example.")
    parser.add_argument('board', help="Board to be used. Must be a valid option for the specified example.")
    parser.add_argument('build', choices=builds, help="Build type.")
    parser.add_argument('--flags', type=str, action=CustomFlagsAction, help="Custom flags for the build, only available with build type 'Custom'.")

    args = parser.parse_args()

    # Check if the selected board is supported by the selected example
    if args.board not in build_board_mapping[args.example]:
        print(f"Error: '{args.board}' is not a supported board by the '{args.example}' example.")
        sys.exit(1)

    create_json_file(args.example, args.board, args.build, args.flags)
