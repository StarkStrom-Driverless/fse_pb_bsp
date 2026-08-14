"""
@author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
@company Startstrom Augsburg
@mail    <maximilian.hoffmann@startstrom-augsburg.de>

Copyright (c) 2025 Startstrom Augsburg
All rights reserved.

Windows implementation of build / flash / clean.

Linux keeps using ss_build, ss_flash and ss_oocd unchanged - ss.py decides at
startup which implementation to register, so nothing in this file is ever
imported on Linux (and nothing in the linux modules is imported on Windows,
which matters because they pull in telnetlib3 and python-can at import time).

Toolchain: STM32CubeCLT (arm-none-eabi-* and STM32_Programmer_CLI).
Overridable through the environment:

    STM32CUBECLT_PATH   installation root, if auto detection fails
    SS_MAKE             path to make.exe
    SS_SHELL            path to the sh.exe make should use for its recipes
"""

from typing import List, Optional
import argparse
import glob
import os
import re
import shutil
import subprocess
import sys


BINARY = "bp_test"

# the application lives in slot0, above the bootloader, and has to be signed
SLOT0_ADDRESS = "0x08020000"

# the bootloader itself sits at the reset vector and is written unsigned -
# it is what verifies everything else, nothing verifies it
BOOTLOADER_ADDRESS = "0x08000000"

# same parameters the linux path signs with (ss_flash.sign_img) - these have to
# match what the bootloader expects, do not "clean them up"
IMG_HEADER_SIZE = "0x200"
IMG_ALIGN = "4"
IMG_VERSION = "1.0.0"
IMG_SLOT_SIZE = "0x60000"


class SswError(Exception):
    """Something is missing or misconfigured - reported without a traceback."""


def repo_root() -> str:
    here = os.path.dirname(os.path.abspath(__file__))
    return os.path.abspath(os.path.join(here, "..", ".."))


def tools_dir() -> str:
    return os.path.dirname(os.path.abspath(__file__))


def build_dir() -> str:
    return os.path.join(repo_root(), "build")


def make_path(path: str) -> str:
    """Turn a windows path into something make and its shell can swallow.

    Forward slashes because make treats backslashes as escapes, and the 8.3
    short name when the path contains spaces - a bare "C:/Program Files/..."
    would split into two arguments inside a recipe.
    """
    if " " in path:
        path = short_path(path)

    return path.replace("\\", "/")


def short_path(path: str) -> str:
    import ctypes

    buf = ctypes.create_unicode_buffer(1024)
    n = ctypes.windll.kernel32.GetShortPathNameW(path, buf, len(buf))

    # returns 0 on failure and the long path again if 8.3 names are disabled
    # on the volume, both of which we can only pass through
    if n and buf.value:
        return buf.value

    return path


###############################################################################
# toolchain discovery


def version_key(path: str):
    m = re.search(r"(\d+(?:\.\d+)*)\s*$", os.path.basename(path))

    if not m:
        return (0,)

    return tuple(int(p) for p in m.group(1).split("."))


_cubeclt_cache: Optional[str] = None


def cubeclt_root() -> str:
    global _cubeclt_cache

    if _cubeclt_cache:
        return _cubeclt_cache

    override = os.environ.get("STM32CUBECLT_PATH")
    if override:
        if not os.path.isdir(override):
            raise SswError(f"STM32CUBECLT_PATH is set to '{override}', which is not a directory")

        _cubeclt_cache = override
        return _cubeclt_cache

    searched = [
        r"C:\ST",
        os.path.join(os.environ.get("ProgramFiles", r"C:\Program Files"), "ST"),
        os.path.join(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"), "ST"),
    ]

    found = []
    for base in searched:
        found += [p for p in glob.glob(os.path.join(base, "STM32CubeCLT*")) if os.path.isdir(p)]

    if not found:
        raise SswError(
            "no STM32CubeCLT installation found.\n"
            "  searched: " + "\n            ".join(searched) + "\n"
            "  install it from st.com, or point STM32CUBECLT_PATH at it"
        )

    _cubeclt_cache = max(found, key=version_key)
    return _cubeclt_cache


def gcc_bin_dir() -> str:
    clt = cubeclt_root()

    direct = os.path.join(clt, "GNU-tools-for-STM32", "bin")
    if os.path.isfile(os.path.join(direct, "arm-none-eabi-gcc.exe")):
        return direct

    # tolerate a renamed toolchain folder in future CLT releases
    hits = glob.glob(os.path.join(clt, "*", "bin", "arm-none-eabi-gcc.exe"))
    if hits:
        return os.path.dirname(hits[0])

    raise SswError(f"no arm-none-eabi-gcc.exe below {clt}")


def gcc_tool(name: str) -> str:
    path = os.path.join(gcc_bin_dir(), f"arm-none-eabi-{name}.exe")

    if not os.path.isfile(path):
        raise SswError(f"{path} not found")

    return path


def programmer_cli() -> str:
    clt = cubeclt_root()

    candidates = [
        os.path.join(clt, "STM32CubeProgrammer", "bin", "STM32_Programmer_CLI.exe"),
        os.path.join(
            os.environ.get("ProgramFiles", r"C:\Program Files"),
            "STMicroelectronics", "STM32Cube", "STM32CubeProgrammer", "bin",
            "STM32_Programmer_CLI.exe",
        ),
    ]

    candidates += glob.glob(os.path.join(clt, "*", "bin", "STM32_Programmer_CLI.exe"))

    for c in candidates:
        if os.path.isfile(c):
            return c

    raise SswError(
        "STM32_Programmer_CLI.exe not found.\n"
        "  searched: " + "\n            ".join(candidates)
    )


def find_make() -> str:
    override = os.environ.get("SS_MAKE")
    if override:
        if not os.path.isfile(override):
            raise SswError(f"SS_MAKE is set to '{override}', which does not exist")

        return override

    for name in ("make", "mingw32-make"):
        found = shutil.which(name)
        if found:
            return found

    # CubeCLT is built around cmake/ninja and may not ship one
    for pattern in ("*/bin/make.exe", "*/make.exe"):
        hits = glob.glob(os.path.join(cubeclt_root(), pattern))
        if hits:
            return hits[0]

    raise SswError(
        "no make.exe on PATH.\n"
        "  the xpack windows-build-tools archive contains make.exe and sh.exe:\n"
        "  https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases\n"
        "  unpack it and either add its bin/ to PATH or point SS_MAKE at make.exe"
    )


def find_sh() -> Optional[str]:
    """The posix shell make runs its recipes with.

    The makefiles use mkdir -p and rm -r, so cmd.exe as SHELL is not an option.
    Returning None lets make pick its own default, which only works if a sh.exe
    happens to sit on PATH.
    """
    override = os.environ.get("SS_SHELL")
    if override:
        if not os.path.isfile(override):
            raise SswError(f"SS_SHELL is set to '{override}', which does not exist")

        return override

    # xpack ships sh.exe right next to make.exe
    try:
        beside = os.path.join(os.path.dirname(find_make()), "sh.exe")
        if os.path.isfile(beside):
            return beside
    except SswError:
        pass

    candidates = [
        shutil.which("sh"),
        os.path.join(os.environ.get("ProgramFiles", r"C:\Program Files"),
                     "Git", "usr", "bin", "sh.exe"),
        os.path.join(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"),
                     "Git", "usr", "bin", "sh.exe"),
    ]

    for c in candidates:
        if c and os.path.isfile(c):
            return c

    return None


def posix_bin_dirs() -> List[str]:
    """Directories that have to be on PATH while make runs.

    For recipe lines without shell metacharacters make skips SHELL entirely and
    calls CreateProcess directly, so 'rm -f x' needs a real rm.exe on PATH - a
    shell builtin does not help. That is what makes libopencm3 fail at
    Makefile:73 with the fairly unhelpful 'Error -1'.

    CubeCLT ships rm/mkdir/echo/sh next to make.exe. find, cp and printf only
    exist in a full coreutils set, e.g. the one git for windows installs.
    """
    dirs: List[str] = []

    override = os.environ.get("SS_POSIX_BIN")
    if override:
        dirs.append(override)

    try:
        dirs.append(os.path.dirname(find_make()))
    except SswError:
        pass

    for base in (os.environ.get("ProgramFiles", r"C:\Program Files"),
                 os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")):
        candidate = os.path.join(base, "Git", "usr", "bin")
        if os.path.isfile(os.path.join(candidate, "rm.exe")):
            dirs.append(candidate)

    out: List[str] = []
    for d in dirs:
        if os.path.isdir(d) and d not in out:
            out.append(d)

    return out


###############################################################################
# make


def make_overrides() -> List[str]:
    """Command line variables - they win over the ?= defaults in rules.mk and
    Makefile.include, so none of the makefiles have to be touched."""
    prefix = os.path.join(gcc_bin_dir(), "arm-none-eabi-")

    overrides = ["PREFIX=" + make_path(prefix)]

    sh = find_sh()
    if sh:
        overrides.append("SHELL=" + make_path(sh))

    return overrides


def run_make(targets: List[str], cwd: str, jobs: int = 1) -> int:
    cmd = [find_make()]

    if jobs > 1:
        cmd += ["-j", str(jobs)]

    cmd += make_overrides() + targets

    env = os.environ.copy()

    extra = posix_bin_dirs()
    if extra:
        env["PATH"] = os.pathsep.join(extra + [env.get("PATH", "")])

    print(">> " + " ".join(cmd))
    print(f"   in {cwd}")

    # streamed, not captured - a build you cannot watch is a build you cannot
    # debug, and the linux path buffering everything is a wart not a feature
    return subprocess.run(cmd, cwd=cwd, env=env).returncode


def build_libopencm3(targets: str = "stm32/f4", jobs: int = 1) -> None:
    """Called by init.ps1 - lives here so the toolchain discovery does not have
    to be duplicated in powershell.

    Only stm32/f4 is built by default; that is the family Makefile.include asks
    for (LIBNAME = opencm3_stm32f4) and building everything takes many times
    longer.
    """
    directory = os.path.join(repo_root(), "fse_pb_bsp", "libopencm3")

    if not os.path.isfile(os.path.join(directory, "Makefile")):
        raise SswError(
            f"{directory} has no Makefile - the libopencm3 submodule is not checked out.\n"
            "  git submodule update --init --recursive"
        )

    rc = run_make([f"TARGETS={targets}"], directory, jobs=jobs)
    if rc != 0:
        raise SswError(f"libopencm3 build failed with exit code {rc}")

    lib = glob.glob(os.path.join(directory, "lib", "*.a"))
    if not lib:
        raise SswError(f"libopencm3 build reported success but produced no .a in {directory}/lib")

    for path in sorted(lib):
        print(f"   {path}")


###############################################################################
# vscode


def gdb_server() -> str:
    clt = cubeclt_root()

    candidates = [
        os.path.join(clt, "STLink-gdb-server", "bin", "ST-LINK_gdbserver.exe"),
    ]
    candidates += glob.glob(os.path.join(clt, "*", "bin", "ST-LINK_gdbserver.exe"))

    for c in candidates:
        if os.path.isfile(c):
            return c

    raise SswError(
        "ST-LINK_gdbserver.exe not found.\n"
        "  searched: " + "\n            ".join(candidates)
    )


def svd_file() -> Optional[str]:
    hits = glob.glob(os.path.join(cubeclt_root(), "STMicroelectronics_CMSIS_SVD",
                                  "STM32F405.svd"))
    return hits[0] if hits else None


def write_vscode_config() -> None:
    """Generate .vscode/launch.json and tasks.json from the installed toolchain.

    Written rather than committed because the paths carry the CubeCLT version -
    a config checked into git goes stale the moment somebody updates it. Re-run
    init.ps1 to regenerate.
    """
    vscode = os.path.join(repo_root(), ".vscode")
    os.makedirs(vscode, exist_ok=True)

    def fwd(p: str) -> str:
        return p.replace("\\", "/")

    svd = svd_file()
    svd_line = f'\n            "svdFile": "{fwd(svd)}",' if svd else ""

    block = f'''            "type": "cortex-debug",
            "request": "launch",
            "cwd": "${{workspaceFolder}}",
            "executable": "${{workspaceFolder}}/build/{BINARY}.elf",

            // DO NOT set this to program the elf. The application is signed
            // and written by ./ss flash; letting the debugger flash the elf
            // puts an UNSIGNED image into slot0 and mcuboot refuses to boot it.
            "loadFiles": [],

            "servertype": "stlink",
            "serverpath": "{fwd(gdb_server())}",
            "stm32cubeprogrammer": "{fwd(os.path.dirname(programmer_cli()))}",
            "armToolchainPath": "{fwd(gcc_bin_dir())}",{svd_line}
            "device": "STM32F405RG",
            "interface": "swd",

            // mcuboot runs first and jumps into slot0, so main is only
            // reached once the bootloader has handed over
            "runToEntryPoint": "main"'''

    launch = f'''// GENERATED by fse_pb_bsp/init.ps1 - re-run it to refresh.
// The paths below carry the STM32CubeCLT version, so this file goes stale
// when CubeCLT is updated. It is gitignored for that reason.
{{
    "version": "0.2.0",
    "configurations": [
        {{
            "name": "Flash + Debug ({BINARY})",
            "preLaunchTask": "ss flash",
{block}
        }},
        {{
            "name": "Debug only (already flashed)",
{block}
        }}
    ]
}}
'''

    tasks = '''// GENERATED by fse_pb_bsp/init.ps1 - re-run it to refresh.
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "ss build",
            "type": "shell",
            "command": "${workspaceFolder}/ss.cmd",
            "args": ["build"],
            "group": { "kind": "build", "isDefault": true },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "ss flash",
            "type": "shell",
            "command": "${workspaceFolder}/ss.cmd",
            "args": ["flash"],
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "ss clean",
            "type": "shell",
            "command": "${workspaceFolder}/ss.cmd",
            "args": ["clean"],
            "problemMatcher": []
        }
    ]
}
'''

    for name, payload in (("launch.json", launch), ("tasks.json", tasks)):
        path = os.path.join(vscode, name)
        with open(path, "w", encoding="utf-8") as f:
            f.write(payload)

        print(f"wrote    {path}")

    if not svd:
        print("note: no STM32F405.svd found, the peripheral view will be empty")


def init_vscode() -> int:
    """Entry point for init.ps1 - reports SswError as a message, not a traceback."""
    try:
        write_vscode_config()
        return 0
    except SswError as e:
        print(f"\nerror: {e}", file=sys.stderr)
        return 1


def init_libopencm3() -> int:
    """Entry point for init.ps1 - reports SswError as a message, not a traceback."""
    try:
        build_libopencm3()
        return 0
    except SswError as e:
        print(f"\nerror: {e}", file=sys.stderr)
        return 1


###############################################################################
# image handling


def elf_path() -> str:
    return os.path.join(build_dir(), f"{BINARY}.elf")


def bin_path() -> str:
    return os.path.join(build_dir(), f"{BINARY}.bin")


def signed_path() -> str:
    return os.path.join(build_dir(), f"{BINARY}.signed.confirmed.bin")


def create_bin(elf: str, out: str) -> None:
    objcopy = gcc_tool("objcopy")

    print(f">> objcopy {elf} -> {out}")

    rc = subprocess.run([objcopy, "-O", "binary", elf, out]).returncode
    if rc != 0:
        raise SswError(f"objcopy failed with exit code {rc}")


def sign_image(src: str, dst: str) -> None:
    imgtool = os.path.join(tools_dir(), "imgtool.py")
    key = os.path.join(tools_dir(), "root-rsa-2048.pem")

    for path in (imgtool, key):
        if not os.path.isfile(path):
            raise SswError(f"{path} not found")

    cmd = [
        sys.executable, imgtool, "sign",
        "--key", key,
        "--header-size", IMG_HEADER_SIZE,
        "--pad-header",
        "--align", IMG_ALIGN,
        "--version", IMG_VERSION,
        "--slot-size", IMG_SLOT_SIZE,
        "--pad",
        "--confirm",
        src, dst,
    ]

    print(">> " + " ".join(cmd))

    rc = subprocess.run(cmd).returncode
    if rc != 0:
        raise SswError(
            f"imgtool failed with exit code {rc}\n"
            "  it needs cryptography, click, intelhex, cbor and pyyaml in the venv"
        )


def resolve_image(path: str) -> str:
    """Find an image the user named on the command line.

    Relative paths are taken from the project root, because that is where the
    shell is when running .\\ss.cmd - the process itself has been pushd'd into
    fse_pb_bsp/tools by the launcher.
    """
    if os.path.isabs(path):
        candidates = [path]
    else:
        candidates = [os.path.join(repo_root(), path), os.path.abspath(path)]

    for c in candidates:
        if os.path.isfile(c):
            return c

    raise SswError(
        f"'{path}' not found.\n"
        "  looked at: " + "\n             ".join(candidates)
    )


def write_image(image: str, address: str) -> None:
    cmd = [
        programmer_cli(),
        "-c", "port=SWD",
        "-w", image, address,
        "-rst",
    ]

    print(">> " + " ".join(cmd))

    rc = subprocess.run(cmd).returncode
    if rc != 0:
        raise SswError(f"STM32_Programmer_CLI failed with exit code {rc}")


###############################################################################
# commands


def handle_build(args) -> None:
    if getattr(args, "clean", False):
        clean_outputs()

    rc = run_make([], repo_root(), jobs=getattr(args, "jobs", 1))
    if rc != 0:
        raise SswError(f"make failed with exit code {rc}")

    if not os.path.isfile(elf_path()):
        raise SswError(
            f"make succeeded but {elf_path()} does not exist.\n"
            "  the project Makefile has to produce build/bp_test.elf - compare\n"
            "  fse_pb_bsp/examples/examples.mk, which sets BUILDDIR, BINARY and\n"
            "  adds $(LIB_OBJS) to OBJS"
        )

    print(f"\n{elf_path()}")


def handle_flash(args) -> None:
    handle_build(args)

    create_bin(elf_path(), bin_path())
    sign_image(bin_path(), signed_path())

    write_image(signed_path(), SLOT0_ADDRESS)


def handle_bootloader(args) -> None:
    # deliberately no build and no signing - this is the bootloader image
    # itself, built out of tree (fse_pb_bootloader -> zephyr.bin)
    image = resolve_image(args.bin_file)

    print(f">> bootloader {image} -> {args.position}")

    write_image(image, args.position)


def clean_outputs() -> None:
    """Done in python rather than through 'make clean', which would need rm on
    the recipe shell's PATH."""
    removed = False

    if os.path.isdir(build_dir()):
        shutil.rmtree(build_dir())
        print(f"removed {build_dir()}")
        removed = True

    for path in glob.glob(os.path.join(repo_root(), "generated.*")):
        os.remove(path)
        print(f"removed {path}")
        removed = True

    if not removed:
        print("nothing to clean")


def handle_clean(args) -> None:
    clean_outputs()


def ssw_add_sub(sub) -> None:
    parser_build = sub.add_parser("build", help="build programm")
    parser_build.add_argument("--clean", action="store_true",
                              help="wipe the build directory first")
    parser_build.add_argument("-j", "--jobs", type=int, default=1,
                              help="parallel make jobs (default: 1)")
    parser_build.set_defaults(func=handle_build)

    parser_flash = sub.add_parser("flash", help="build, sign and flash via STM32_Programmer_CLI")
    parser_flash.add_argument("--clean", action="store_true",
                              help="wipe the build directory first")
    parser_flash.add_argument("-j", "--jobs", type=int, default=1,
                              help="parallel make jobs (default: 1)")
    parser_flash.set_defaults(func=handle_flash)

    parser_bootloader = sub.add_parser(
        "bootloader",
        help="flash a bootloader image (unsigned) via STM32_Programmer_CLI")
    parser_bootloader.add_argument("--bin_file", default="zephyr.bin",
                                   help="bootloader binary such as zephyr.bin, "
                                        "relative paths are taken from the project root")
    parser_bootloader.add_argument("--position", default=BOOTLOADER_ADDRESS,
                                   help=f"flash address to write the bootloader to "
                                        f"(default: {BOOTLOADER_ADDRESS})")
    parser_bootloader.set_defaults(func=handle_bootloader)

    parser_clean = sub.add_parser("clean", help="clean workspace")
    parser_clean.set_defaults(func=handle_clean)


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd")

    ssw_add_sub(sub)

    args = parser.parse_args()

    if hasattr(args, 'func'):
        args.func(args)
    else:
        parser.print_help()


if __name__ == '__main__':
    main()
