from typing import List, Dict
import subprocess
import argparse



def get_dir_of_file(file : str):
    parts = file.split("/")
    return "/".join(parts[:len(parts)-1])

def build(  make_file : str = "../../Makefile"):
    cmd = [
        "make"
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
        f"{directory}/bp_test.signed.confirmed.bin"
    ]

    subprocess.run(cmd)


def create_bin( elf_file : str = "../../bp_test.elf",
                bin_file : str = "../../bp_test.bin"):
    cmd = [
        "arm-none-eabi-objcopy",
        "-O",
        "binary",
        elf_file,
        bin_file
    ]

    subprocess.run(cmd)

def handle_build(args):
    clean()
    build()
    create_bin()

def handle_clean(args):
    clean()

def build_add_sub(sub):
    parser_build = sub.add_parser("build", help="build programm")
    parser_build.set_defaults(func=handle_build)

    parser_clean = sub.add_parser("clean", help="clean workspace")
    parser_clean.set_defaults(func=handle_clean)

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