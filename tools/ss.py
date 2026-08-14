"""
@author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
@company Startstrom Augsburg
@mail    <maximilian.hoffmann@startstrom-augsburg.de>

Copyright (c) 2025 Startstrom Augsburg
All rights reserved.
"""

from typing import List, Dict
import subprocess
import argparse
import platform
import sys


def add_linux_subs(sub):
    # imported here and not at module level: these modules pull in telnetlib3
    # and python-can while being imported, neither of which exists on windows
    from ss_flash import flash_add_sub
    from ss_oocd import oocd_add_sub
    from ss_build import build_add_sub
    from ss_matlab import matlab_add_sub
    from ss_dbc import dbc_add_sub

    flash_add_sub(sub)
    build_add_sub(sub)
    oocd_add_sub(sub)
    matlab_add_sub(sub)
    dbc_add_sub(sub)


def add_windows_subs(sub):
    # openocd (replaced by STM32_Programmer_CLI) and the socketcan based
    # canflash have no windows implementation
    from ssw import ssw_add_sub
    from ssw_matlab import matlab_add_sub, dbc_add_sub

    ssw_add_sub(sub)
    matlab_add_sub(sub)
    dbc_add_sub(sub)


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd")

    windows = platform.system() == "Windows"

    if windows:
        add_windows_subs(sub)
    else:
        add_linux_subs(sub)

    args = parser.parse_args()

    if not hasattr(args, 'func'):
        parser.print_help()
        return

    if not windows:
        args.func(args)
        return

    from ssw import SswError

    try:
        args.func(args)
    except SswError as e:
        print(f"\nerror: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
