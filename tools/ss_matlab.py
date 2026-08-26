"""
@author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
@company Startstrom Augsburg
@mail    <maximilian.hoffmann@startstrom-augsburg.de>

Copyright (c) 2025 Startstrom Augsburg
All rights reserved.
"""

from typing import List
import argparse
import glob
import os
import re
import shutil
import subprocess
import sys


GCC_ENV_DEFAULT = "~/micromamba/envs/matlab-gcc13/bin"


def repo_root() -> str:
    here = os.path.dirname(os.path.abspath(__file__))
    return os.path.abspath(os.path.join(here, "..", ".."))


def simulink_dir() -> str:
    return os.path.join(repo_root(), "fse_pb_bsp", "simulink")


def model_dir() -> str:
    return os.path.join(repo_root(), "usr", "simulink")


def model_name(name: str = None) -> str:
    if name:
        base = name
    else:
        base = os.path.basename(repo_root())

    mdl = re.sub(r"[^A-Za-z0-9_]", "_", base)
    if not mdl or not mdl[0].isalpha():
        mdl = "m_" + mdl

    return mdl


def matlab_env() -> dict:
    env = os.environ.copy()

    gcc = os.path.expanduser(os.environ.get("SS_MATLAB_GCC", GCC_ENV_DEFAULT))
    if os.path.isdir(gcc):
        env["PATH"] = gcc + os.pathsep + env["PATH"]
    else:
        print(f"warning: no matlab gcc env at {gcc}, using system compiler")

    return env


def matlab_run(statements: List[str], gui: bool = False) -> int:
    exe = os.environ.get("SS_MATLAB", "matlab")

    if shutil.which(exe) is None:
        print(f"error: '{exe}' not found in PATH (override with SS_MATLAB)")
        return 1

    prolog = f"addpath(genpath('{simulink_dir()}')); ss_path_add;"
    cmd = " ".join([prolog] + statements)

    args = [exe, "-sd", simulink_dir()]
    if gui:
        args += ["-desktop", "-r", cmd]
    else:
        args += ["-batch", cmd]

    print(f">> matlab: {cmd}")

    return subprocess.run(args, env=matlab_env()).returncode


def matlab_running() -> bool:
    try:
        out = subprocess.run(["pgrep", "-f", "MATLAB.*glnxa64"],
                             capture_output=True, text=True)
        return out.returncode == 0
    except FileNotFoundError:
        return False


def cache_clean() -> None:
    caches = glob.glob(os.path.expanduser("~/.matlab/*/lbstream"))

    if not caches:
        print("no library browser cache found")
        return

    if matlab_running():
        print("warning: matlab is still running, the cache will be rewritten on exit")
        print("         close matlab and run ./ss matlab_cache_clean again")

    for c in caches:
        shutil.rmtree(c)
        print(f"removed {c}")


def handle_matlab_cache_clean(args):
    cache_clean()


def handle_matlab_init(args):
    mdl = model_name(args.name)

    os.makedirs(model_dir(), exist_ok=True)

    statements = ["ss_path_install;"]
    if args.modules:
        mods = ", ".join(f"'{m}'" for m in args.modules)
        statements.append(f"ss_simulink_build({mods});")
    else:
        statements.append("ss_simulink_build;")
    statements.append(f"ss_model_create('{mdl}');")

    rc = matlab_run(statements)
    if rc == 0:
        cache_clean()
        print(f"\nmodel: {os.path.join(model_dir(), mdl + '.slx')}")
        print("open it with: ./ss matlab_open")

    sys.exit(rc)


def handle_matlab_build(args):
    mdl = model_name(args.name)
    sys.exit(matlab_run([f"build_model('{mdl}');"]))


ASCII_FALLBACK = {
    "\u2550": "=", "\u2500": "-", "\u2502": "|", "\u251c": "+",
    "\u2514": "+", "\u2510": "+", "\u2524": "+", "\u25bc": "v",
}


def write(text: str) -> None:
    enc = sys.stdout.encoding or "ascii"

    try:
        text.encode(enc)
    except UnicodeEncodeError:
        for k, v in ASCII_FALLBACK.items():
            text = text.replace(k, v)

    print(text)


def tree_entry(prefix: str, name: str, desc: str) -> str:
    return ("      " + prefix + " " + name).ljust(33) + desc


def matlab_help_text(mdl: str) -> str:
    art = """    ./ss matlab_open           open the model in the matlab gui, runs
         │                     usr/simulink/config.m (pin map, board version)
         │
         │   edit blocks, then generate code
         │
         ├── ctrl+B in simulink ────┐   needs ./ss matlab_config
         │                          │   once, see below
         ├── ./ss matlab_build ─────┘   always works by itself
         │
         ▼
    usr/simulink/{mdl}_ert_rtw/*.c
         │
         │   ./ss build
         ▼
    build/bp_test.elf
         │
         │   ./ss flash
         ▼
    target""".replace("{mdl}", mdl)
    cfg = """    ss_model_config.m          ert.tlc, GenCodeOnly, solver, include paths
         │
         ├── ./ss matlab_build ───>  set in memory for this one build,
         │                           the .slx file is never touched
         │
         └── ./ss matlab_config ──>  written into the model and saved
                                           │
                                           ▼
                                    ctrl+B reads it from there"""

    tree = "\n".join([
        "    usr/simulink/",
        tree_entry("├─", mdl + ".slx", "the model"),
        tree_entry("├─", "config.m", "pin map and board version"),
        tree_entry("├─", "*_ert_rtw/", "generated c code, built by the makefile"),
        tree_entry("└─", "slprj/", "simulink build cache, safe to delete"),
        "    fse_pb_bsp/simulink/",
        tree_entry("└─", "ss_*/", "legacy code tool wrappers, block libs"),
    ])

    return f"""
  simulink workflow
  ════════════════════════════════════════════════════════════════════════

  once per checkout
  ────────────────────────────────────────────────────────────────────────

    ./ss matlab_init           matlab path + startup.m, build the ss block
                               libraries, create the model
    ./ss matlab_config         store the code generation settings in the model
    ./ss can_gen <file.dbc>    turn a dbc into can message blocks
                               -> usr/simulink/can_<bus>_lib.slx

  every day
  ────────────────────────────────────────────────────────────────────────

{art}

  when you need matlab_config
  ────────────────────────────────────────────────────────────────────────

    the settings come from fse_pb_bsp/simulink/ss_model_config.m, not from
    usr/simulink/config.m, and reach a build on two paths

{cfg}

    run it once  after ./ss matlab_init or a fresh clone
                 after ss_model_config.m changed
                 after someone saved the model with different settings

    skip it and ctrl+B keeps GenCodeOnly off: simulink hands the generated
    code to gcc and the build dies on ss_can_sl.h and rt_main.c

  where things live
  ────────────────────────────────────────────────────────────────────────

{tree}

  when something is off
  ────────────────────────────────────────────────────────────────────────

    gui build runs gcc and dies on    ./ss matlab_config, then build again
    ss_can_sl.h or rt_main.c
    generated code in the wrong       restart matlab, ss_path_add sets the
    folder                            code generation folder at startup
    ss blocks missing in the          ./ss matlab_cache_clean   (matlab closed)
    library browser
    stale code or cache               ./ss matlab_clean
    ss module changed in the bsp      ./ss matlab_libs
    pin name rejected                 ./ss matlab_pins
"""

def handle_matlab_help(args):
    write(matlab_help_text(model_name(getattr(args, "name", None))))


def handle_matlab_config(args):
    mdl = model_name(args.name)
    sys.exit(matlab_run([f"ss_model_config_save('{mdl}');"]))


def handle_matlab_libs(args):
    if args.modules:
        mods = ", ".join(f"'{m}'" for m in args.modules)
        stmt = f"ss_simulink_build({mods});"
    else:
        stmt = "ss_simulink_build;"

    rc = matlab_run([stmt])
    if rc == 0:
        cache_clean()

    sys.exit(rc)


def handle_matlab_pins(args):
    sys.exit(matlab_run(["ss_pin_list;"]))


def handle_matlab_open(args):
    mdl = model_name(args.name)
    path = os.path.join(model_dir(), mdl + ".slx")

    if not os.path.isfile(path):
        print(f"error: {path} not found, run ./ss matlab_init first")
        sys.exit(1)

    sys.exit(matlab_run(["ss_config_load;", f"open_system('{path}');",
                         f"ss_model_config('{mdl}');"], gui=True))


def remove(path: str) -> None:
    if os.path.isdir(path) and not os.path.islink(path):
        shutil.rmtree(path)
        print(f"removed {path}")
    elif os.path.exists(path):
        os.remove(path)
        print(f"removed {path}")


def handle_matlab_clean(args):
    out = model_dir()

    targets = []
    for d in (out, repo_root(), simulink_dir()):
        targets += glob.glob(os.path.join(d, "*_ert_rtw"))
        targets += glob.glob(os.path.join(d, "slprj"))
    targets += glob.glob(os.path.join(out, "*.slxc"))
    targets += glob.glob(os.path.join(out, "*.autosave"))
    targets += glob.glob(os.path.join(out, "can_*_lib.slx"))
    targets += glob.glob(os.path.join(out, "can_*_spec.json"))
    targets += glob.glob(os.path.join(out, "slblocks.m"))

    if args.libs:
        sl = simulink_dir()
        for pattern in ("*/*_sfcn.c", "*/*_sfcn.tlc", "*/*_sfcn.tlc.bak",
                        "*/*.mexa64", "*/*.mexw64", "*/*.mexmaci64",
                        "*/*_lib.slx", "*/*.slxc", "*/slprj"):
            targets += glob.glob(os.path.join(sl, pattern))

    if not targets:
        print("nothing to clean")
        return

    for t in targets:
        remove(t)

    if args.libs:
        print("\nblock libraries removed, run ./ss matlab_init to rebuild them")


def matlab_add_sub(sub):
    p_init = sub.add_parser("matlab_init",
                            help="install matlab paths, build the ss block libraries and create the model")
    p_init.add_argument("--name", help="model name (default: repository folder name)")
    p_init.add_argument("--modules", nargs="*",
                        help="only build these ss modules, e.g. ss_gpio")
    p_init.set_defaults(func=handle_matlab_init)

    p_build = sub.add_parser("matlab_build", help="generate c code from the simulink model")
    p_build.add_argument("--name", help="model name (default: repository folder name)")
    p_build.set_defaults(func=handle_matlab_build)

    p_help = sub.add_parser("matlab_help",
                            help="show the simulink workflow overview")
    p_help.add_argument("--name", help="model name (default: repository folder name)")
    p_help.set_defaults(func=handle_matlab_help)

    p_config = sub.add_parser("matlab_config",
                              help="write the code generation settings into the model and save it")
    p_config.add_argument("--name", help="model name (default: repository folder name)")
    p_config.set_defaults(func=handle_matlab_config)

    p_libs = sub.add_parser("matlab_libs", help="rebuild the ss block libraries only")
    p_libs.add_argument("--modules", nargs="*",
                        help="only build these ss modules, e.g. ss_gpio")
    p_libs.set_defaults(func=handle_matlab_libs)

    p_open = sub.add_parser("matlab_open", help="open the simulink model in the matlab gui")
    p_open.add_argument("--name", help="model name (default: repository folder name)")
    p_open.set_defaults(func=handle_matlab_open)

    p_pins = sub.add_parser("matlab_pins",
                            help="list the pin names the ss blocks accept")
    p_pins.set_defaults(func=handle_matlab_pins)

    p_cache = sub.add_parser("matlab_cache_clean",
                             help="drop the library browser icon cache (run with matlab closed)")
    p_cache.set_defaults(func=handle_matlab_cache_clean)

    p_clean = sub.add_parser("matlab_clean",
                             help="remove generated code, caches and generated can libraries")
    p_clean.add_argument("--libs", action="store_true",
                         help="also remove the compiled ss block libraries in fse_pb_bsp/simulink")
    p_clean.set_defaults(func=handle_matlab_clean)


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd")

    matlab_add_sub(sub)

    args = parser.parse_args()

    if hasattr(args, 'func'):
        args.func(args)
    else:
        parser.print_help()


if __name__ == '__main__':
    main()
