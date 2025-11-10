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

from ss_flash import *
from ss_oocd import *
from ss_build import *

def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd")
    
    flash_add_sub(sub)
    build_add_sub(sub)
    oocd_add_sub(sub)

    args = parser.parse_args()

    if hasattr(args, 'func'):
        args.func(args)
    else:
        parser.print_help()

if __name__ == '__main__':
    main()