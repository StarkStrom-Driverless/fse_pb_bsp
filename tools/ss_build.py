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
import os



def get_dir_of_file(file : str):
    parts = file.split("/")
    return "/".join(parts[:len(parts)-1])

def build(  make_file : str = "../../Makefile"):
    cmd = [
        "bear", "--", "make"
    ]

    directory = get_dir_of_file(make_file)

    print(directory)

    result = subprocess.run(
        cmd,
        cwd=directory,
        capture_output=True,
        text=True
    )

    print(result.stdout)
    print(result.stderr)

def clean(make_file : str = "../../Makefile"):
    cmd = [
        "make",
        "clean"
    ]

    directory = get_dir_of_file(make_file)

    result = subprocess.run(
        cmd,
        cwd=directory,
        capture_output=True,
        text=True
    )

    print(result.stdout)
    print(result.stderr)

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

    result = subprocess.run(
        cmd,
        cwd=f"{directory}/fse_pb_bsp/examples",
        capture_output=True,
        text=True
    )

    print(result.stdout)
    print(result.stderr)

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
    clean()
    build()
    create_bin()

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