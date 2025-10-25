from typing import List
import subprocess
import argparse

def oocd_is_running(    oocd_bin : str = "openocd") -> bool:
    cmd = [
        "pgrep",
        oocd_bin
    ]
    
    try:
        subprocess.check_output(cmd)
        return True
    except subprocess.CalledProcessError:
        return False

def oocd_state():
    if oocd_is_running():
        print("oocd active")
    else:
        print("oocd inactive")

def oocd_start( oocd_bin : str = "openocd",
                interface : str = "interface/stlink.cfg",
                target : str = "target/stm32f4x.cfg"):
    cmd : List = [
        oocd_bin,
        "-f",
        interface,
        "-f",
        target
    ]

    if oocd_is_running() == False:
        subprocess.Popen(
            cmd,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            stdin=subprocess.DEVNULL,
            start_new_session=True 
        )

    oocd_state()

def oocd_stop(  oocd_bin : str = "openocd"):
    cmd = [
        "pkill",
        oocd_bin
    ]

    if oocd_is_running() == True:
        subprocess.run(cmd)
        oocd_state()

def oocd_state_handle(args):
    oocd_state()

def oocd_start_handle(args):
    oocd_start()

def oocd_stop_handle(args):
    oocd_stop()

def oocd_add_sub(sub):
    parser_start = sub.add_parser("oocd_start", help="start oocd")
    parser_start.set_defaults(func=oocd_start_handle)

    parser_stop = sub.add_parser("oocd_stop", help="stop oocd")
    parser_stop.set_defaults(func=oocd_stop_handle)

    parser_state = sub.add_parser("oocd_state", help="stop oocd")
    parser_state.set_defaults(func=oocd_state_handle)

def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd")

    oocd_add_sub(sub)

    args = parser.parse_args()

    if hasattr(args, 'func'):
        args.func()
    else:
        parser.print_help()

if __name__ == '__main__':
    main()