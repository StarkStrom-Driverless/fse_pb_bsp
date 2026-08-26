"""
@author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
@company Startstrom Augsburg
@mail    <maximilian.hoffmann@startstrom-augsburg.de>

Copyright (c) 2025 Startstrom Augsburg
All rights reserved.
"""

from typing import List, Dict, Tuple
import subprocess
import argparse
import os
import sys
import time


ELF_NAME = "build/bp_test.elf"


def ansi(code: str) -> str:
    if not sys.stdout.isatty() or os.environ.get("NO_COLOR"):
        return ""

    return code


def human_size(path: str) -> str:
    try:
        return f"{os.path.getsize(path) / 1024:.1f} kB"
    except OSError:
        return "missing"


def build_status(rc: int, root: str, elf: str, seconds: float) -> None:
    bold = ansi("\033[1m")
    dim = ansi("\033[2m")
    reset = ansi("\033[0m")

    if rc == 0:
        head = ansi("\033[32m") + bold + "build ok" + reset
        target = f"{ELF_NAME}  {dim}{human_size(elf)}{reset}"
        made = f"{seconds:.1f} s"
    else:
        head = ansi("\033[31m") + bold + "build failed" + reset
        target = f"{ELF_NAME}  {dim}{human_size(elf)}{reset}"
        made = f"exit {rc} after {seconds:.1f} s"

    print(f"\n  {head}")
    print(f"    location   {root}")
    print(f"    target     {target}")
    print(f"    make       {made}\n")


def run(cmd: List[str], cwd: str) -> Tuple[int, float]:
    start = time.monotonic()
    rc = subprocess.run(cmd, cwd=cwd).returncode

    return rc, time.monotonic() - start



def get_dir_of_file(file : str):
    parts = file.split("/")
    return "/".join(parts[:len(parts)-1])

def build(  make_file : str = "../../Makefile"):
    directory = get_dir_of_file(make_file)

    return run(["bear", "--", "make"], directory)

def clean(make_file : str = "../../Makefile"):
    cmd = [
        "make",
        "clean"
    ]

    directory = get_dir_of_file(make_file)

    run(cmd, directory)

    cmd = [
        "rm",
        "-rf",
        f"{directory}/build/bp_test.signed.confirmed.bin"
    ]

    subprocess.run(cmd)


def create_bin( elf_file : str = "../../build/bp_test.elf",
                bin_file : str = "../../build/bp_test.bin"):
    cmd = [
        "arm-none-eabi-objcopy",
        "-O",
        "binary",
        elf_file,
        bin_file
    ]

    subprocess.run(cmd)

def example_build( target : str,
                    make_file : str = "../../Makefile"):
    directory = get_dir_of_file(make_file)

    cmd = [
        "make",
        target
    ]

    run(cmd, f"{directory}/fse_pb_bsp/examples")

    c_file = os.path.abspath(f"{directory}/fse_pb_bsp/examples/{target}.c")
    print(f">> {c_file}")

    return c_file

def example_list(make_file : str = "../../Makefile") -> List[str]:
    directory = get_dir_of_file(make_file)
    examples_dir = f"{directory}/fse_pb_bsp/examples"

    names = sorted(
        os.path.splitext(f)[0]
        for f in os.listdir(examples_dir)
        if f.endswith(".c")
    )

    for name in names:
        print(name)

    return names

def handle_build(args = None):
    directory = get_dir_of_file("../../Makefile")
    elf = os.path.join(directory, ELF_NAME)

    clean()
    rc, seconds = build()

    if rc == 0:
        create_bin()

    build_status(rc, os.path.abspath(directory), elf, seconds)

    sys.exit(rc)

def handle_clean(args):
    clean()

def handle_example_build(args):
    example_build(args.target)

def handle_example_list(args):
    example_list()

def build_add_sub(sub):
    parser_build = sub.add_parser("build", help="build programm")
    parser_build.set_defaults(func=handle_build)

    parser_clean = sub.add_parser("clean", help="clean workspace")
    parser_clean.set_defaults(func=handle_clean)

    parser_example_build = sub.add_parser("example_build", help="build one of the fse_pb_bsp examples (adc, pwm, can_tx, ...)")
    parser_example_build.add_argument("target", help="example name, e.g. adc, pwm, can_tx, uart, ...")
    parser_example_build.set_defaults(func=handle_example_build)

    parser_example_list = sub.add_parser("example_list", help="list available fse_pb_bsp examples")
    parser_example_list.set_defaults(func=handle_example_list)

def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd")

    build_add_sub(sub)

    args = parser.parse_args()

    if hasattr(args, 'func'):
        args.func(args)
    else:
        parser.print_help()

if __name__ == '__main__':
    main()