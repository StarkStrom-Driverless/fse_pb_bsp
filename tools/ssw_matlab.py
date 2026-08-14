"""
@author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
@company Startstrom Augsburg
@mail    <maximilian.hoffmann@startstrom-augsburg.de>

Copyright (c) 2025 Startstrom Augsburg
All rights reserved.

Windows implementation of the matlab / simulink and dbc commands.

Mirrors ss_matlab.py and ss_dbc.py, which stay untouched for linux. The parts
that actually differ are small but unavoidable:

  * matlab.exe is normally not on PATH, so it gets discovered
  * -desktop is not a windows option, and matlab.exe returns immediately
    without -wait, which would make every exit code meaningless
  * the mex host compiler is MSVC or the MinGW-w64 add-on, not the
    micromamba gcc environment used on linux
  * the library browser cache lives under %APPDATA%\\MathWorks, not ~/.matlab
  * the process to look for is MATLAB.exe, not MATLAB...glnxa64

Overridable through the environment:

    SS_MATLAB    path to matlab.exe, if auto detection picks the wrong release
"""

from typing import List, Optional
import argparse
import glob
import json
import os
import re
import shutil
import subprocess
import sys

from ssw import SswError, repo_root


MAX_SIGNAL_BITS = 32

# Guards the mex based library build with a readable message instead of a wall
# of legacy_code output.
#
# No double quotes anywhere in here: the whole statement travels as one windows
# command line argument, and an embedded " gets eaten by the argument parser -
# matlab then sees a truncated string and a stray -setup option. Inside matlab
# single quoted strings a literal quote is written as ''.
MEX_GUARD = ("if isempty(mex.getCompilerConfigurations('C','Selected')), "
             "error('no mex C compiler selected. Run ''mex -setup C'' in matlab. "
             "Install Visual Studio with C++ tools, or the MinGW-w64 add-on.'); end;")


def simulink_dir() -> str:
    return os.path.join(repo_root(), "fse_pb_bsp", "simulink")


def model_dir() -> str:
    return os.path.join(repo_root(), "usr", "simulink")


def sanitize(name: str) -> str:
    out = re.sub(r"[^A-Za-z0-9_]", "_", name)
    if not out or not out[0].isalpha():
        out = "s_" + out

    return out


def model_name(name: str = None) -> str:
    base = name if name else os.path.basename(repo_root())

    mdl = re.sub(r"[^A-Za-z0-9_]", "_", base)
    if not mdl or not mdl[0].isalpha():
        mdl = "m_" + mdl

    return mdl


def mlpath(path: str) -> str:
    """Forward slashes for use inside matlab string literals.

    Matlab accepts them on windows, and it avoids worrying about how a
    backslash is treated inside the quoted statement we hand to -batch.
    """
    return path.replace("\\", "/")


###############################################################################
# matlab


def release_key(path: str):
    m = re.search(r"R(\d{4})([ab])", path)
    if not m:
        return (0, "")

    return (int(m.group(1)), m.group(2))


def matlab_exe() -> str:
    override = os.environ.get("SS_MATLAB")
    if override:
        if os.path.isfile(override):
            return override

        found = shutil.which(override)
        if found:
            return found

        raise SswError(f"SS_MATLAB is set to '{override}', which was not found")

    found = shutil.which("matlab")
    if found:
        return found

    searched = [
        os.path.join(os.environ.get("ProgramFiles", r"C:\Program Files"), "MATLAB"),
        os.path.join(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"), "MATLAB"),
    ]

    found_all = []
    for base in searched:
        found_all += glob.glob(os.path.join(base, "R*", "bin", "matlab.exe"))

    if not found_all:
        raise SswError(
            "matlab.exe not found.\n"
            "  searched PATH and: " + "\n                     ".join(searched) + "\n"
            "  point SS_MATLAB at it if it lives somewhere else"
        )

    return max(found_all, key=release_key)


def matlab_run(statements: List[str], gui: bool = False) -> int:
    exe = matlab_exe()

    prolog = f"addpath(genpath('{mlpath(simulink_dir())}')); ss_path_add;"
    cmd = " ".join([prolog] + statements)

    args = [exe, "-sd", simulink_dir()]

    if gui:
        # -desktop is linux/mac only; on windows the desktop is the default and
        # matlab.exe detaches, which is what we want for an interactive session
        args += ["-r", cmd]
    else:
        # -wait is windows only and required: without it matlab.exe returns
        # straight away and the exit code says nothing about the script
        args += ["-wait", "-nosplash", "-batch", cmd]

    print(f">> matlab: {cmd}")

    return subprocess.run(args).returncode


def matlab_running() -> bool:
    # deliberately NOT text=True: tasklist writes in the console codepage, and
    # on a german windows that produces bytes cp1252 cannot decode, which threw
    # a UnicodeDecodeError right after a perfectly successful matlab_init.
    # The needle is pure ascii, so compare bytes and skip decoding entirely.
    try:
        out = subprocess.run(
            ["tasklist", "/FI", "IMAGENAME eq MATLAB.exe", "/NH"],
            capture_output=True,
        )
        return b"MATLAB.exe" in out.stdout
    except (FileNotFoundError, OSError):
        return False


def cache_clean() -> None:
    """Drop the simulink library browser cache so regenerated blocks show up.

    The prefdir on windows is %APPDATA%\\MathWorks\\MATLAB\\<release>, not
    ~/.matlab/<release>. The linux code removes the lbstream directory; that
    name does not exist on R2025b, where the browser database is the file
    slblocks_master.db. Both are matched so older releases keep working.

    ss_simulink_build already calls sl_refresh_customizations, which is the
    supported way to refresh - this is only the fallback for when the browser
    still shows stale icons.
    """
    try:
        _cache_clean()
    except Exception as e:
        # cosmetic cleanup: it must never turn a successful matlab_init or
        # matlab_libs into a failure
        print(f"warning: could not clean the library browser cache: {e}")


def _cache_clean() -> None:
    appdata = os.environ.get("APPDATA")
    if not appdata:
        print("APPDATA is not set, skipping the library browser cache")
        return

    base = os.path.join(appdata, "MathWorks", "MATLAB")

    # only the release we actually drive. Several matlab versions side by side
    # is normal, and wiping the caches of the other three would be rude.
    try:
        m = re.search(r"R\d{4}[ab]", matlab_exe())
        release = m.group(0) if m else "*"
    except SswError:
        release = "*"

    caches = []
    for pattern in ("lbstream", "slblocks_master.db"):
        caches += glob.glob(os.path.join(base, release, pattern))

    if not caches:
        print("no library browser cache found")
        return

    if matlab_running():
        print("warning: matlab is still running, the cache will be rewritten on exit")
        print("         close matlab and run ./ss matlab_cache_clean again")
        return

    for c in caches:
        if os.path.isdir(c):
            shutil.rmtree(c, ignore_errors=True)
        else:
            try:
                os.remove(c)
            except OSError as e:
                print(f"could not remove {c}: {e}")
                continue

        print(f"removed {c}")


def remove(path: str) -> None:
    if os.path.isdir(path) and not os.path.islink(path):
        shutil.rmtree(path)
        print(f"removed {path}")
    elif os.path.exists(path):
        os.remove(path)
        print(f"removed {path}")


###############################################################################
# handlers


def handle_matlab_init(args) -> None:
    mdl = model_name(args.name)

    os.makedirs(model_dir(), exist_ok=True)

    statements = [MEX_GUARD, "ss_path_install;"]

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


def handle_matlab_build(args) -> None:
    mdl = model_name(args.name)
    sys.exit(matlab_run([f"build_model('{mdl}');"]))


def handle_matlab_libs(args) -> None:
    if args.modules:
        mods = ", ".join(f"'{m}'" for m in args.modules)
        stmt = f"ss_simulink_build({mods});"
    else:
        stmt = "ss_simulink_build;"

    rc = matlab_run([MEX_GUARD, stmt])
    if rc == 0:
        cache_clean()

    sys.exit(rc)


def handle_matlab_pins(args) -> None:
    sys.exit(matlab_run(["ss_pin_list;"]))


def handle_matlab_open(args) -> None:
    mdl = model_name(args.name)
    path = os.path.join(model_dir(), mdl + ".slx")

    if not os.path.isfile(path):
        raise SswError(f"{path} not found, run ./ss matlab_init first")

    sys.exit(matlab_run(["ss_config_load;", f"open_system('{mlpath(path)}');"], gui=True))


def handle_matlab_cache_clean(args) -> None:
    cache_clean()


def handle_matlab_clean(args) -> None:
    out = model_dir()

    targets = []
    targets += glob.glob(os.path.join(out, "*_ert_rtw"))
    targets += glob.glob(os.path.join(out, "slprj"))
    targets += glob.glob(os.path.join(out, "*.slxc"))
    targets += glob.glob(os.path.join(out, "*.autosave"))
    targets += glob.glob(os.path.join(out, "can_*_lib.slx"))
    targets += glob.glob(os.path.join(out, "can_*_spec.json"))
    targets += glob.glob(os.path.join(out, "slblocks.m"))

    if args.libs:
        sl = simulink_dir()
        # mexw64 is the windows artifact; the other two are kept so a tree
        # shared with linux/mac colleagues gets cleaned properly too
        for pattern in ("*/*_sfcn.c", "*/*_sfcn.tlc", "*/*_sfcn.tlc.bak",
                        "*/*.mexw64", "*/*.mexa64", "*/*.mexmaci64",
                        "*/*_lib.slx", "*/*.slxc", "slprj", "*/slprj"):
            targets += glob.glob(os.path.join(sl, pattern))

    if not targets:
        print("nothing to clean")
        return

    for t in targets:
        remove(t)

    if args.libs:
        print("\nblock libraries removed, run ./ss matlab_init to rebuild them")


###############################################################################
# dbc


def dbc_to_spec(path: str, channel: int, with_valid: bool, default_cycle_ms: int,
                poll_ms: int) -> dict:
    try:
        import cantools
    except ImportError:
        raise SswError(
            "cantools is missing.\n"
            "  .venv\\Scripts\\python.exe -m pip install cantools"
        )

    db = cantools.database.load_file(path)

    name = sanitize(os.path.splitext(os.path.basename(path))[0]).lower()
    spec = {"name": name, "channel": channel, "with_valid": with_valid,
            "tx_cycle_ms": default_cycle_ms, "rx_poll_ms": poll_ms,
            "messages": []}

    for msg in db.messages:
        signals = []

        for sig in msg.signals:
            if sig.multiplexer_ids is not None:
                print(f"skipped {msg.name}.{sig.name}: multiplexed signals are not supported")
                continue

            if sig.length > MAX_SIGNAL_BITS:
                print(f"skipped {msg.name}.{sig.name}: {sig.length} bits exceeds "
                      f"the {MAX_SIGNAL_BITS} bit block limit")
                continue

            signals.append({
                "name": sanitize(sig.name),
                "start": int(sig.start),
                "length": int(sig.length),
                "signed": bool(sig.is_signed),
                "big_endian": sig.byte_order != "little_endian",
                "scale": float(sig.scale),
                "offset": float(sig.offset),
            })

        if not signals:
            print(f"skipped message {msg.name}: no usable signals")
            continue

        spec["messages"].append({
            "name": sanitize(msg.name),
            "id": int(msg.frame_id),
            "dlc": int(msg.length),
            "signals": signals,
        })

    if not spec["messages"]:
        raise SswError("no usable messages found")

    return spec


def handle_can_gen(args) -> None:
    if not os.path.isfile(args.dbc):
        raise SswError(f"{args.dbc} not found")

    spec = dbc_to_spec(args.dbc, args.channel, args.with_valid, args.tx_cycle_ms,
                       args.rx_poll_ms)

    out = model_dir()
    os.makedirs(out, exist_ok=True)

    spec_path = os.path.join(out, f"can_{spec['name']}_spec.json")
    with open(spec_path, "w", encoding="utf-8") as f:
        json.dump(spec, f, indent=2)

    n_sig = sum(len(m["signals"]) for m in spec["messages"])
    print(f"{len(spec['messages'])} messages, {n_sig} signals -> {spec_path}")

    rc = matlab_run([f"ss_can_lib_generate('{mlpath(spec_path)}');"])
    if rc == 0:
        cache_clean()

    sys.exit(rc)


###############################################################################


def matlab_add_sub(sub) -> None:
    p_init = sub.add_parser("matlab_init",
                            help="install matlab paths, build the ss block libraries and create the model")
    p_init.add_argument("--name", help="model name (default: repository folder name)")
    p_init.add_argument("--modules", nargs="*",
                        help="only build these ss modules, e.g. ss_gpio")
    p_init.set_defaults(func=handle_matlab_init)

    p_build = sub.add_parser("matlab_build", help="generate c code from the simulink model")
    p_build.add_argument("--name", help="model name (default: repository folder name)")
    p_build.set_defaults(func=handle_matlab_build)

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


def dbc_add_sub(sub) -> None:
    p_gen = sub.add_parser("can_gen",
                           help="generate simulink message blocks from a dbc file")
    p_gen.add_argument("dbc", help="path to the .dbc file")
    p_gen.add_argument("--channel", type=int, default=1,
                       help="ss_can channel the messages live on (default: 1)")
    p_gen.add_argument("--with-valid", action="store_true",
                       help="add a valid output port per rx block, useful for timeout supervision")
    p_gen.add_argument("--tx-cycle-ms", type=int, default=100,
                       help="default value of the tx block cycle time dialog field (default: 100)")
    p_gen.add_argument("--rx-poll-ms", type=int, default=1,
                       help="how often the rx blocks poll the queue, must match the "
                            "model step to keep downstream signals smooth (default: 1)")
    p_gen.set_defaults(func=handle_can_gen)


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd")

    matlab_add_sub(sub)
    dbc_add_sub(sub)

    args = parser.parse_args()

    if hasattr(args, 'func'):
        args.func(args)
    else:
        parser.print_help()


if __name__ == '__main__':
    main()
