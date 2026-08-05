"""
@author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
@company Startstrom Augsburg
@mail    <maximilian.hoffmann@startstrom-augsburg.de>

Copyright (c) 2025 Startstrom Augsburg
All rights reserved.
"""

from typing import List
import argparse
import json
import os
import re
import sys

from ss_matlab import matlab_run, model_dir, cache_clean


MAX_SIGNAL_BITS = 32


def sanitize(name: str) -> str:
    out = re.sub(r"[^A-Za-z0-9_]", "_", name)
    if not out or not out[0].isalpha():
        out = "s_" + out

    return out


def dbc_to_spec(path: str, channel: int, with_valid: bool, default_cycle_ms: int,
                poll_ms: int) -> dict:
    try:
        import cantools
    except ImportError:
        print("error: cantools is missing, run: .venv/bin/pip install cantools")
        sys.exit(1)

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
        print("error: no usable messages found")
        sys.exit(1)

    return spec


def handle_can_gen(args):
    if not os.path.isfile(args.dbc):
        print(f"error: {args.dbc} not found")
        sys.exit(1)

    spec = dbc_to_spec(args.dbc, args.channel, args.with_valid, args.tx_cycle_ms,
                       args.rx_poll_ms)

    out = model_dir()
    os.makedirs(out, exist_ok=True)

    spec_path = os.path.join(out, f"can_{spec['name']}_spec.json")
    with open(spec_path, "w") as f:
        json.dump(spec, f, indent=2)

    n_sig = sum(len(m["signals"]) for m in spec["messages"])
    print(f"{len(spec['messages'])} messages, {n_sig} signals -> {spec_path}")

    rc = matlab_run([f"ss_can_lib_generate('{spec_path}');"])
    if rc == 0:
        cache_clean()

    sys.exit(rc)


def dbc_add_sub(sub):
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

    dbc_add_sub(sub)

    args = parser.parse_args()

    if hasattr(args, 'func'):
        args.func(args)
    else:
        parser.print_help()


if __name__ == '__main__':
    main()
