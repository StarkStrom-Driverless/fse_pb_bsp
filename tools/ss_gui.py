"""
@author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
@company Startstrom Augsburg
@mail    <maximilian.hoffmann@startstrom-augsburg.de>

Copyright (c) 2025 Startstrom Augsburg
All rights reserved.
"""

from typing import Callable, Dict, List, Optional, Tuple
import glob
import os
import re
import shutil
import signal
import subprocess
import sys
import threading
import time

from ss_matlab import repo_root, model_dir, model_name


TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
SS_PY = os.path.join(TOOLS_DIR, "ss.py")

LOG_LIMIT = 4000

MCUBOOT_HEADER = 0x200
LINKER_SCRIPT = os.path.join("fse_pb_bsp", "stm32f4-discovery.ld")


class Runner:
    def __init__(self):
        self._lines: List[str] = []
        self._lock = threading.Lock()
        self._proc: Optional[subprocess.Popen] = None
        self._thread: Optional[threading.Thread] = None
        self.command = ""
        self.rc: Optional[int] = None
        self.seconds = 0.0

    def busy(self) -> bool:
        return self._thread is not None and self._thread.is_alive()

    def lines(self) -> List[str]:
        with self._lock:
            return list(self._lines)

    def clear(self) -> None:
        with self._lock:
            self._lines = []

    def _append(self, text: str) -> None:
        with self._lock:
            self._lines.append(text)
            if len(self._lines) > LOG_LIMIT:
                del self._lines[: len(self._lines) - LOG_LIMIT]

    def start(self, args: List[str], detached: bool = False) -> None:
        if detached:
            self._start_detached(args)
            return

        if self.busy():
            return

        self.command = " ".join(args)
        self.rc = None
        self._thread = threading.Thread(target=self._run, args=(args,), daemon=True)
        self._thread.start()

    def _start_detached(self, args: List[str]) -> None:
        self._append(f"$ ./ss {' '.join(args)}  &")

        try:
            subprocess.Popen(
                [sys.executable, SS_PY, *args],
                cwd=TOOLS_DIR,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                stdin=subprocess.DEVNULL,
                env=self._env(),
                start_new_session=True,
            )
        except OSError as e:
            self._append(f"failed to start: {e}")
            return

        self._append("-- started in the background, this window stays usable --")

    def _env(self) -> Dict[str, str]:
        env = dict(os.environ)
        env["NO_COLOR"] = "1"

        extra = toolchain_dir()

        if extra and extra not in env.get("PATH", "").split(os.pathsep):
            env["PATH"] = extra + os.pathsep + env.get("PATH", "")

        return env

    def _run(self, args: List[str]) -> None:
        started = time.monotonic()

        self._append(f"$ ./ss {' '.join(args)}")

        env = self._env()
        extra = toolchain_dir()

        if extra and extra not in os.environ.get("PATH", "").split(os.pathsep):
            self._append(f"-- PATH += {extra} --")

        try:
            self._proc = subprocess.Popen(
                [sys.executable, SS_PY, *args],
                cwd=TOOLS_DIR,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                stdin=subprocess.DEVNULL,
                text=True,
                bufsize=1,
                env=env,
            )
        except OSError as e:
            self._append(f"failed to start: {e}")
            self.rc = -1
            return

        for line in self._proc.stdout:
            self._append(line.rstrip("\n"))

        self._proc.wait()

        self.rc = self._proc.returncode
        self.seconds = time.monotonic() - started
        self._proc = None

        self._append(f"-- exit {self.rc} after {self.seconds:.1f} s --")

    def stop(self) -> None:
        proc = self._proc

        if proc is not None:
            proc.terminate()
            self._append("-- terminated --")


def mtime(path: str) -> float:
    try:
        return os.path.getmtime(path)
    except OSError:
        return 0.0


def age(path: str) -> str:
    stamp = mtime(path)

    if stamp == 0.0:
        return "-"

    seconds = time.time() - stamp

    if seconds < 90:
        return f"{seconds:.0f} s ago"
    if seconds < 5400:
        return f"{seconds / 60:.0f} min ago"
    if seconds < 172800:
        return f"{seconds / 3600:.0f} h ago"

    return f"{seconds / 86400:.0f} d ago"


def size_kb(path: str) -> str:
    try:
        return f"{os.path.getsize(path) / 1024:.1f} kB"
    except OSError:
        return "-"


def file_size(path: str) -> int:
    try:
        return os.path.getsize(path)
    except OSError:
        return 0


def kb(value: int) -> str:
    return f"{value / 1024:.1f} kB"


def memory_limits(root: str) -> Dict[str, int]:
    limits = {"rom": 0, "ram": 0}

    try:
        text = open(os.path.join(root, LINKER_SCRIPT)).read()
    except OSError:
        return limits

    for name in limits:
        found = re.search(name + r"\s*\([^)]*\)\s*:.*LENGTH\s*=\s*(\d+)\s*([KM]?)", text)

        if found:
            scale = {"": 1, "K": 1024, "M": 1024 * 1024}[found.group(2)]
            limits[name] = int(found.group(1)) * scale

    return limits


_model_cache: Dict[str, Tuple[float, Dict[str, str]]] = {}


def model_settings(path: str) -> Dict[str, str]:
    import zipfile

    stamp = mtime(path)

    if stamp == 0.0:
        return {}

    cached = _model_cache.get(path)

    if cached is not None and cached[0] == stamp:
        return cached[1]

    found: Dict[str, str] = {}

    try:
        with zipfile.ZipFile(path) as archive:
            text = archive.read("simulink/configSet0.xml").decode("utf-8", "replace")

        for key in ("FixedStep", "GenCodeOnly", "SystemTargetFile", "CodeInterfacePackaging"):
            hit = re.search('Name="' + key + r'"[^>]*>([^<]*)<', text)

            if hit:
                found[key] = hit.group(1)
    except (OSError, KeyError, zipfile.BadZipFile):
        found = {}

    _model_cache[path] = (stamp, found)

    return found


def config_define(root: str, name: str, header: str = "ss_config.h") -> Optional[float]:
    try:
        text = open(os.path.join(root, "usr", "inc", header)).read()
    except OSError:
        return None

    hit = re.search(r"#\s*define\s+" + name + r"\s+(-?\d+(?:\.\d+)?)", text)

    return float(hit.group(1)) if hit else None


_size_cache: Dict[str, Tuple[float, Optional[Tuple[int, int, int]]]] = {}


def elf_sections(path: str) -> Optional[Tuple[int, int, int]]:
    stamp = mtime(path)

    if stamp == 0.0:
        return None

    cached = _size_cache.get(path)

    if cached is not None and cached[0] == stamp:
        return cached[1]

    tool = tool_path("arm-none-eabi-size", toolchain_dir()) or shutil.which("size")
    result = None

    if tool is not None:
        try:
            out = subprocess.run([tool, path], capture_output=True, text=True, timeout=5)
            numbers = re.findall(r"\d+", out.stdout.splitlines()[-1])

            if len(numbers) >= 3:
                result = (int(numbers[0]), int(numbers[1]), int(numbers[2]))
        except (OSError, IndexError, ValueError, subprocess.SubprocessError):
            result = None

    _size_cache[path] = (stamp, result)

    return result


TOOLCHAIN_GLOBS = [
    "/opt/arm-gnu-toolchain-*/bin",
    "/opt/gcc-arm-none-eabi-*/bin",
    "/usr/local/arm-gnu-toolchain-*/bin",
    os.path.expanduser("~/opt/arm-gnu-toolchain-*/bin"),
]


def toolchain_dir() -> Optional[str]:
    found = shutil.which("arm-none-eabi-gcc")

    if found:
        return os.path.dirname(found)

    for pattern in TOOLCHAIN_GLOBS:
        matches = sorted(glob.glob(pattern), reverse=True)

        for directory in matches:
            if os.path.isfile(os.path.join(directory, "arm-none-eabi-gcc")):
                return directory

    return None


def tool_path(name: str, extra: Optional[str] = None) -> Optional[str]:
    if extra:
        candidate = os.path.join(extra, name)

        if os.path.isfile(candidate) and os.access(candidate, os.X_OK):
            return candidate

    return shutil.which(name)


def can_interfaces() -> List[Tuple[str, str]]:
    found = []

    for path in sorted(glob.glob("/sys/class/net/*")):
        name = os.path.basename(path)

        try:
            if open(os.path.join(path, "type")).read().strip() != "280":
                continue

            state = open(os.path.join(path, "operstate")).read().strip()
        except OSError:
            continue

        found.append((name, state))

    return found


def read_spec(path: str) -> List[dict]:
    import json

    try:
        with open(path) as f:
            return json.load(f).get("messages", [])
    except (OSError, ValueError):
        return []


class State:
    def __init__(self):
        self.root = repo_root()
        self.model = model_name()
        self.oocd = False
        self._oocd_checked = 0.0
        self._limits: Optional[Dict[str, int]] = None
        self.interfaces: List[Tuple[str, str]] = []
        self._interfaces_checked = 0.0
        self.matlab = False
        self._matlab_checked = 0.0

    def oocd_running(self) -> bool:
        now = time.monotonic()

        if now - self._oocd_checked > 1.0:
            from ss_oocd import oocd_is_running

            self.oocd = oocd_is_running()
            self._oocd_checked = now

        return self.oocd

    def matlab_open(self) -> bool:
        now = time.monotonic()

        if now - self._matlab_checked > 1.0:
            from ss_matlab import matlab_running

            self.matlab = matlab_running()
            self._matlab_checked = now

        return self.matlab

    def can_devices(self) -> List[Tuple[str, str]]:
        now = time.monotonic()

        if now - self._interfaces_checked > 1.0:
            self.interfaces = can_interfaces()
            self._interfaces_checked = now

        return self.interfaces

    def example_source(self, name: str) -> str:
        return os.path.join(self.root, "fse_pb_bsp", "examples", name + ".c")

    def can_spec(self, lib: str) -> str:
        return os.path.join(model_dir(), lib.replace("_lib", "") + "_spec.json")

    def elf(self) -> str:
        return os.path.join(self.root, "build", "bp_test.elf")

    def bin(self) -> str:
        return os.path.join(self.root, "build", "bp_test.bin")

    def signed(self) -> str:
        return os.path.join(self.root, "build", "bp_test.signed.confirmed.bin")

    def limits(self) -> Dict[str, int]:
        if self._limits is None:
            self._limits = memory_limits(self.root)

        return self._limits

    def model_file(self) -> str:
        return os.path.join(model_dir(), self.model + ".slx")

    def generated(self) -> str:
        return os.path.join(model_dir(), self.model + "_ert_rtw", self.model + ".c")

    def examples(self) -> List[str]:
        found = glob.glob(os.path.join(self.root, "fse_pb_bsp", "examples", "*.c"))

        return sorted(os.path.splitext(os.path.basename(f))[0] for f in found)

    def ss_libs(self) -> List[str]:
        found = glob.glob(os.path.join(self.root, "fse_pb_bsp", "simulink", "*", "*_lib.slx"))
        found += glob.glob(os.path.join(model_dir(), "*", "*_lib.slx"))

        return sorted(os.path.splitext(os.path.basename(f))[0] for f in found)

    def modules(self) -> List[str]:
        found = glob.glob(os.path.join(self.root, "fse_pb_bsp", "simulink", "*", "lc_*.m"))
        found += glob.glob(os.path.join(model_dir(), "*", "lc_*.m"))

        return sorted(os.path.basename(os.path.dirname(f)) for f in found)

    def can_libs(self) -> List[str]:
        found = glob.glob(os.path.join(model_dir(), "can_*_lib.slx"))

        return sorted(os.path.splitext(os.path.basename(f))[0] for f in found)

    def dbc_files(self) -> List[str]:
        found = glob.glob(os.path.join(self.root, "usr", "dbc", "*.dbc"))

        return sorted(os.path.relpath(f, self.root) for f in found)


def gui_main() -> int:
    try:
        from imgui_bundle import imgui, immapp, hello_imgui
        from imgui_bundle import imgui_color_text_edit as text_edit
    except ImportError:
        print("error: imgui-bundle is missing, run: .venv/bin/pip install imgui-bundle")
        return 1

    GREEN = imgui.ImVec4(0.30, 0.80, 0.35, 1.0)
    RED = imgui.ImVec4(0.90, 0.30, 0.30, 1.0)
    GREY = imgui.ImVec4(0.60, 0.60, 0.60, 1.0)
    WHITE = imgui.ImVec4(0.90, 0.90, 0.92, 1.0)
    AMBER = imgui.ImVec4(0.95, 0.70, 0.25, 1.0)

    runner = Runner()
    state = State()
    interrupted = threading.Event()

    def on_sigint(signum, frame) -> None:
        interrupted.set()

    try:
        signal.signal(signal.SIGINT, on_sigint)
        signal.signal(signal.SIGTERM, on_sigint)
    except ValueError:
        pass

    ui = {
        "example": 0,
        "modules": "",
        "dbc": 0,
        "channel": 1,
        "with_valid": True,
        "tx_cycle_ms": 100,
        "rx_poll_ms": 1,
        "canflash_id": "0x100",
        "canflash_bin": "build/bp_test.signed.confirmed.bin",
        "bootloader_bin": "zephyr.bin",
        "bootloader_pos": "0x08000000",
        "autoscroll": True,
        "clean_libs": False,
        "canflash_if": "can0",
        "example": 0,
        "example_loaded": "",
        "can_lib": 0,
        "can_lib_loaded": "",
        "messages": [],
        "log_count": -1,
    }

    editor = text_edit.TextEditor()
    editor.set_language(text_edit.TextEditor.Language.c())
    editor.set_read_only_enabled(True)
    editor.set_show_whitespaces_enabled(False)
    editor.set_text("")

    fonts: Dict[str, object] = {"mono": None}
    MONO_SIZE = 15.0

    log_editor = text_edit.TextEditor()
    log_editor.set_read_only_enabled(True)
    log_editor.set_show_scrollbar_mini_map_enabled(False)
    log_editor.set_show_line_numbers_enabled(False)
    log_editor.set_show_whitespaces_enabled(False)
    log_editor.set_text("")

    def run(*args: str, detached: bool = False) -> None:
        runner.start([a for a in args if a], detached)

    def action(label: str, args: List[str], width: float = 0.0,
               detached: bool = False) -> None:
        imgui.begin_disabled(runner.busy() and not detached)

        if imgui.button(label, imgui.ImVec2(width, 0)):
            run(*args, detached=detached)

        imgui.end_disabled()

    def dot(ok: bool, label: str, unknown: bool = False) -> None:
        color = GREY if unknown else (GREEN if ok else RED)

        origin = imgui.get_cursor_screen_pos()
        height = imgui.get_text_line_height()
        radius = height * 0.26

        imgui.get_window_draw_list().add_circle_filled(
            imgui.ImVec2(origin.x + radius + 1.0, origin.y + height * 0.5),
            radius,
            imgui.get_color_u32(color),
        )

        imgui.dummy(imgui.ImVec2(radius * 2.0 + 6.0, height))
        imgui.same_line()
        imgui.text(label)

    def field(name: str, value: str, note: str = "", warn: bool = False) -> None:
        imgui.text(f"{name:<18}")
        imgui.same_line(170)
        imgui.text_unformatted(value)

        if note:
            imgui.same_line()
            imgui.text_colored(AMBER if warn else GREY, note)

    def bar(used: int, total: int) -> None:
        if total <= 0:
            imgui.text_colored(GREY, "-")
            return

        fraction = used / total
        color = GREEN if fraction < 0.8 else (AMBER if fraction < 0.95 else RED)

        imgui.push_style_color(imgui.Col_.plot_histogram.value, color)
        imgui.progress_bar(fraction, imgui.ImVec2(260, 0),
                           f"{kb(used)} of {kb(total)}  ({fraction * 100:.0f} %)")
        imgui.pop_style_color()

    def build_tab() -> None:
        elf = state.elf()
        binary = state.bin()
        signed = state.signed()
        limits = state.limits()
        sections = elf_sections(elf)

        imgui.separator_text("tools")

        extra = toolchain_dir()

        for name in ("arm-none-eabi-gcc", "make", "bear", "openocd"):
            path = tool_path(name, extra)

            dot(path is not None, name)

            imgui.same_line(240)

            if path is not None:
                imgui.text_colored(GREY, path)
            else:
                imgui.text_colored(RED, "not found")

        if extra and extra not in os.environ.get("PATH", "").split(os.pathsep):
            imgui.text_colored(AMBER, "the toolchain is not on PATH, it is added for every"
                                      " command started here")

        imgui.separator_text("firmware")

        field("program", kb(file_size(binary)) if file_size(binary) else "-",
              f"build/bp_test.bin, {age(binary)}" if file_size(binary) else "run build")

        if sections is not None:
            text, data, bss = sections
            field("text / data / bss", f"{text} / {data} / {bss} B")

        imgui.text("flash")
        imgui.same_line(170)
        bar(file_size(binary) + MCUBOOT_HEADER, limits["rom"])
        imgui.same_line()
        imgui.text_colored(GREY, "incl. mcuboot header")

        if sections is not None:
            imgui.text("ram")
            imgui.same_line(170)
            bar(sections[1] + sections[2], limits["ram"])

        field("elf", size_kb(elf), age(elf))
        field("signed image", size_kb(signed),
              f"padded to the slot, {age(signed)}" if file_size(signed) else "")

        imgui.spacing()
        action("build", ["build"], 110)
        imgui.same_line()
        action("clean", ["clean"], 110)
        imgui.same_line()
        action("flash", ["flash"], 110)

        imgui.separator_text("openocd")

        running = state.oocd_running()
        dot(running, "openocd active" if running else "openocd inactive")

        imgui.spacing()
        action("oocd_start", ["oocd_start"], 110)
        imgui.same_line()
        action("oocd_stop", ["oocd_stop"], 110)
        imgui.same_line()
        action("oocd_state", ["oocd_state"], 110)

        imgui.separator_text("bootloader")

        imgui.set_next_item_width(260)
        _, ui["bootloader_bin"] = imgui.input_text("bin##bl", ui["bootloader_bin"])
        imgui.same_line()
        imgui.set_next_item_width(120)
        _, ui["bootloader_pos"] = imgui.input_text("position", ui["bootloader_pos"])
        imgui.same_line()
        action("bootloader", ["bootloader", "--bin_file", ui["bootloader_bin"],
                              "--position", ui["bootloader_pos"]])

        imgui.separator_text("canflash")

        devices = state.can_devices()
        wanted = ui["canflash_if"]
        found = dict(devices)

        if devices:
            for name, link in devices:
                dot(link == "up", f"{name}  {link}")
        else:
            dot(False, "no socketcan device", unknown=True)

        imgui.spacing()

        imgui.set_next_item_width(120)
        _, ui["canflash_if"] = imgui.input_text("interface", ui["canflash_if"])
        imgui.same_line()

        if wanted in found:
            if found[wanted] == "up":
                imgui.text_colored(GREEN, f"{wanted} is up")
            else:
                imgui.text_colored(AMBER, f"{wanted} exists but is {found[wanted]},"
                                          " canflash brings it up")
        else:
            imgui.text_colored(RED, f"{wanted} does not exist")

        imgui.set_next_item_width(260)
        _, ui["canflash_bin"] = imgui.input_text("bin##can", ui["canflash_bin"])
        imgui.same_line()
        imgui.set_next_item_width(120)
        _, ui["canflash_id"] = imgui.input_text("can id", ui["canflash_id"])

        imgui.begin_disabled(wanted not in found)
        action("canflash", ["canflash", "--bin_file", ui["canflash_bin"],
                            "--id", ui["canflash_id"], "--interface", wanted])
        imgui.end_disabled()

    def examples_tab() -> None:
        examples = state.examples()

        if not examples:
            imgui.text_colored(GREY, "no examples in fse_pb_bsp/examples")
            return

        ui["example"] = min(ui["example"], len(examples) - 1)
        name = examples[ui["example"]]

        if ui["example_loaded"] != name:
            try:
                editor.set_text(open(state.example_source(name)).read())
            except OSError as e:
                editor.set_text(f"// {e}")

            ui["example_loaded"] = name

        imgui.begin_child("##examplelist", imgui.ImVec2(180, 0))

        for index, entry in enumerate(examples):
            selected, _ = imgui.selectable(entry, index == ui["example"])

            if selected:
                ui["example"] = index

        imgui.end_child()

        imgui.same_line()

        imgui.begin_child("##examplecode", imgui.ImVec2(0, 0))

        imgui.text_colored(GREY, os.path.relpath(state.example_source(name), state.root))
        imgui.same_line()
        action("example_build", ["example_build", name])
        imgui.same_line()
        action("flash_example", ["flash_example", name])

        if fonts["mono"] is not None:
            imgui.push_font(fonts["mono"], MONO_SIZE)

        editor.render("##code", imgui.ImVec2(0, 0))

        if fonts["mono"] is not None:
            imgui.pop_font()

        imgui.end_child()

    def matlab_tab() -> None:
        if not imgui.begin_tab_bar("##matlabtabs"):
            return

        if imgui.begin_tab_item("Config")[0]:
            matlab_config()
            imgui.end_tab_item()

        if imgui.begin_tab_item("Workflow")[0]:
            matlab_workflow()
            imgui.end_tab_item()

        if imgui.begin_tab_item("CAN")[0]:
            can_tab()
            imgui.end_tab_item()

        imgui.end_tab_bar()

    def matlab_config() -> None:
        model = state.model_file()
        generated = state.generated()
        libs = state.ss_libs()
        modules = state.modules()

        imgui.separator_text("state")

        dot(os.path.isfile(model), f"model  {state.model}.slx", )
        field("", os.path.dirname(model) if os.path.isfile(model) else "run matlab_init",
              age(model) if os.path.isfile(model) else "")

        stale = (os.path.isfile(generated) and os.path.isfile(model)
                 and mtime(generated) < mtime(model))

        dot(os.path.isfile(generated), f"generated code  {state.model}_ert_rtw")
        field("", age(generated) if os.path.isfile(generated) else "run matlab_build",
              "model is newer, rebuild" if stale else "", warn=True)

        dot(bool(libs), f"block libraries  {len(libs)} of {len(modules)} modules")
        field("", ", ".join(m for m in modules) if modules else "-")

        if libs and len(libs) < len(modules):
            missing = [m for m in modules if f"{m}_lib" not in libs]
            imgui.text_colored(AMBER, "   missing: " + ", ".join(missing))

        dot(state.matlab_open(), "matlab gui running" if state.matlab_open()
            else "matlab gui not running", unknown=not state.matlab_open())

        imgui.separator_text("once per checkout")

        action("matlab_init", ["matlab_init"], 170)
        imgui.same_line()
        action("matlab_config", ["matlab_config"], 170)
        imgui.same_line()
        imgui.text_colored(GREY, "paths + libraries + model, then store the codegen settings")

        imgui.separator_text("every day")

        action("matlab_open", ["matlab_open"], 170, detached=True)
        imgui.same_line()
        action("matlab_build", ["matlab_build"], 170)
        imgui.same_line()
        imgui.text_colored(GREY, "edit the model, then generate code into usr/simulink")

        imgui.separator_text("libraries")

        imgui.set_next_item_width(260)
        _, ui["modules"] = imgui.input_text("modules (empty = all)", ui["modules"])

        mods = ui["modules"].split()
        action("matlab_libs", ["matlab_libs"] + (["--modules"] + mods if mods else []), 170)
        imgui.same_line()
        action("matlab_pins", ["matlab_pins"], 170)
        imgui.same_line()
        action("matlab_cache_clean", ["matlab_cache_clean"])

        imgui.separator_text("clean")

        _, ui["clean_libs"] = imgui.checkbox("also drop the compiled block libraries (--libs)",
                                             ui["clean_libs"])
        action("matlab_clean", ["matlab_clean"] + (["--libs"] if ui["clean_libs"] else []), 170)
        imgui.same_line()
        action("matlab_help", ["matlab_help"], 170)

    def flow_box(x: float, y: float, w: float, h: float, label: str, note: str,
                 color, args: Optional[List[str]] = None,
                 detached: bool = False) -> None:
        draw = imgui.get_window_draw_list()
        top_left = imgui.ImVec2(x, y)
        bottom_right = imgui.ImVec2(x + w, y + h)

        imgui.set_cursor_screen_pos(top_left)
        clicked = imgui.invisible_button("##flow" + label, imgui.ImVec2(w, h))
        hovered = imgui.is_item_hovered()

        background = imgui.ImVec4(0.18, 0.20, 0.23, 1.0) if not hovered else \
            imgui.ImVec4(0.24, 0.27, 0.31, 1.0)

        draw.add_rect_filled(top_left, bottom_right, imgui.get_color_u32(background), 5.0)
        draw.add_rect(top_left, bottom_right, imgui.get_color_u32(color), 5.0, 2.0)
        draw.add_text(imgui.ImVec2(x + 12, y + 9), imgui.get_color_u32(WHITE), label)
        draw.add_text(imgui.ImVec2(x + 12, y + 30), imgui.get_color_u32(color), note)

        if clicked and args and (detached or not runner.busy()):
            run(*args, detached=detached)

    def flow_arrow(x0: float, y0: float, x1: float, y1: float) -> None:
        draw = imgui.get_window_draw_list()
        color = imgui.get_color_u32(GREY)

        draw.add_line(imgui.ImVec2(x0, y0), imgui.ImVec2(x1, y1), color, 2.0)

        if abs(y1 - y0) < 1.0:
            draw.add_triangle_filled(imgui.ImVec2(x1, y1), imgui.ImVec2(x1 - 8, y1 - 5),
                                     imgui.ImVec2(x1 - 8, y1 + 5), color)
        else:
            draw.add_triangle_filled(imgui.ImVec2(x1, y1), imgui.ImVec2(x1 - 5, y1 - 8),
                                     imgui.ImVec2(x1 + 5, y1 - 8), color)

    def matlab_workflow() -> None:
        model = state.model_file()
        generated = state.generated()
        elf = state.elf()
        libs = state.ss_libs()
        modules = state.modules()
        settings = model_settings(model)

        has_model = os.path.isfile(model)
        has_libs = bool(libs) and len(libs) >= len(modules)
        has_code = os.path.isfile(generated)

        if has_libs and has_model:
            init_color, init_note = GREEN, f"{len(libs)} libraries + model"
        elif has_model or libs:
            init_color, init_note = AMBER, "incomplete"
        else:
            init_color, init_note = RED, "not run yet"

        wanted_step = config_define(state.root, "SS_MODEL_STEP_MS")
        model_step = settings.get("FixedStep")

        if not has_model:
            cfg_color, cfg_note = GREY, "needs a model"
        elif settings.get("GenCodeOnly") != "on":
            cfg_color, cfg_note = RED, "GenCodeOnly is off"
        elif (wanted_step is not None and model_step is not None
              and abs(float(model_step) * 1000.0 - wanted_step) > 1e-6):
            cfg_color, cfg_note = AMBER, f"{model_step} s vs {wanted_step:.0f} ms in config"
        else:
            cfg_color, cfg_note = GREEN, f"ert.tlc, step {model_step} s"

        if not has_code:
            build_color, build_note = RED, "no generated code"
        elif has_model and mtime(generated) < mtime(model):
            build_color, build_note = AMBER, "model is newer"
        else:
            build_color, build_note = GREEN, age(generated)

        if not os.path.isfile(elf):
            make_color, make_note = RED, "no elf"
        elif has_code and mtime(elf) < mtime(generated):
            make_color, make_note = AMBER, "code is newer"
        else:
            make_color, make_note = GREEN, age(elf)

        running = state.oocd_running()
        flash_color = GREEN if running else GREY
        flash_note = "openocd up" if running else "openocd down"

        if state.matlab_open():
            open_color, open_note = GREEN, "matlab is open"
        else:
            open_color, open_note = GREY, "edit the model"

        imgui.text_colored(GREY, "click a box to run it")
        imgui.spacing()

        origin = imgui.get_cursor_screen_pos()
        width, height, gap = 230.0, 56.0, 44.0

        row_y = origin.y + 10.0
        boxes = [
            ("matlab_init", init_note, init_color, ["matlab_init"], False),
            ("matlab_config", cfg_note, cfg_color, ["matlab_config"], False),
            ("matlab_open", open_note, open_color, ["matlab_open"], True),
            ("matlab_build", build_note, build_color, ["matlab_build"], False),
        ]

        for index, (label, note, color, args, detached) in enumerate(boxes):
            x = origin.x + index * (width + gap)
            flow_box(x, row_y, width, height, label, note, color, args, detached)

            if index:
                flow_arrow(x - gap + 4, row_y + height / 2, x - 4, row_y + height / 2)

        last_x = origin.x + 3 * (width + gap)
        second_y = row_y + height + 74.0

        draw = imgui.get_window_draw_list()
        draw.add_text(imgui.ImVec2(last_x, row_y + height + 12.0), imgui.get_color_u32(GREY),
                      f"usr/simulink/{state.model}_ert_rtw")

        flow_arrow(last_x + width / 2, row_y + height + 32.0,
                   last_x + width / 2, second_y - 6.0)

        flow_box(last_x, second_y, width, height, "build", make_note, make_color, ["build"])
        flow_box(last_x + width + gap, second_y, width, height, "flash", flash_note,
                 flash_color, ["flash"])
        flow_arrow(last_x + width + 4, second_y + height / 2,
                   last_x + width + gap - 4, second_y + height / 2)

        imgui.set_cursor_screen_pos(imgui.ImVec2(origin.x, second_y + height + 24.0))
        imgui.text_colored(GREY, "once per checkout:  matlab_init, matlab_config")
        imgui.text_colored(GREY, "every day:  matlab_open, matlab_build, build, flash")

    def can_tab() -> None:
        dbcs = state.dbc_files()
        libs = state.can_libs()

        imgui.separator_text("generated message libraries")

        if not libs:
            imgui.text_colored(GREY, "none yet, run can_gen below")
        else:
            ui["can_lib"] = min(ui["can_lib"], len(libs) - 1)

            imgui.begin_child("##canlibs", imgui.ImVec2(260, 210))

            for index, lib in enumerate(libs):
                selected, _ = imgui.selectable(lib, index == ui["can_lib"])

                if selected:
                    ui["can_lib"] = index

            imgui.end_child()

            imgui.same_line()

            lib = libs[ui["can_lib"]]

            if ui["can_lib_loaded"] != lib:
                ui["messages"] = read_spec(state.can_spec(lib))
                ui["can_lib_loaded"] = lib

            imgui.begin_child("##canmessages", imgui.ImVec2(0, 210))

            path = os.path.join(model_dir(), lib + ".slx")
            imgui.text_colored(GREY, f"{size_kb(path)}   {age(path)}   "
                                     f"{len(ui['messages'])} messages")

            if not ui["messages"]:
                imgui.text_colored(GREY, "no spec json next to the library")

            for message in ui["messages"]:
                signals = message.get("signals", [])
                label = (f"{message.get('name', '?')}   0x{message.get('id', 0):X}   "
                         f"dlc {message.get('dlc', 0)}   {len(signals)} signals")

                if imgui.tree_node_ex(label):
                    for signal in signals:
                        imgui.text_colored(
                            GREY,
                            f"    {signal.get('name', '?'):<28} "
                            f"start {signal.get('start', 0):>3}  "
                            f"len {signal.get('length', 0):>3}  "
                            f"scale {signal.get('scale', 1)}  "
                            f"offset {signal.get('offset', 0)}  "
                            f"{'signed' if signal.get('signed') else 'unsigned'}  "
                            f"{'motorola' if signal.get('big_endian') else 'intel'}")

                    imgui.tree_pop()

            imgui.end_child()

        imgui.separator_text("can_gen")

        if not dbcs:
            imgui.text_colored(GREY, "no .dbc found in usr/dbc")
            return

        ui["dbc"] = min(ui["dbc"], len(dbcs) - 1)

        imgui.set_next_item_width(320)
        _, ui["dbc"] = imgui.combo("database", ui["dbc"], dbcs)

        imgui.set_next_item_width(120)
        _, ui["channel"] = imgui.input_int("ss_can channel", ui["channel"])
        ui["channel"] = 1 if ui["channel"] < 1 else (2 if ui["channel"] > 2 else ui["channel"])

        imgui.set_next_item_width(120)
        _, ui["tx_cycle_ms"] = imgui.input_int("tx cycle (ms)", ui["tx_cycle_ms"], 10, 100)
        imgui.same_line()
        imgui.set_next_item_width(120)
        _, ui["rx_poll_ms"] = imgui.input_int("rx poll (ms)", ui["rx_poll_ms"], 1, 10)

        _, ui["with_valid"] = imgui.checkbox("valid port per rx block (--with-valid)",
                                             ui["with_valid"])

        args = ["can_gen", dbcs[ui["dbc"]],
                "--channel", str(ui["channel"]),
                "--tx-cycle-ms", str(ui["tx_cycle_ms"]),
                "--rx-poll-ms", str(ui["rx_poll_ms"])]

        if ui["with_valid"]:
            args.append("--with-valid")

        imgui.begin_disabled(runner.busy())

        if imgui.button("can_gen", imgui.ImVec2(170, 0)):
            ui["can_lib_loaded"] = ""
            run(*args)

        imgui.end_disabled()

        imgui.spacing()
        imgui.text_colored(GREY, "every sample time has to be a multiple of SS_MODEL_STEP_MS")

    def log_pane() -> None:
        imgui.separator_text("output")

        lines = runner.lines()

        _, ui["autoscroll"] = imgui.checkbox("autoscroll", ui["autoscroll"])
        imgui.same_line()

        if imgui.button("copy"):
            imgui.set_clipboard_text("\n".join(lines))

        imgui.same_line()

        if imgui.button("clear"):
            runner.clear()
            ui["log_count"] = -1

        imgui.same_line()

        imgui.begin_disabled(not runner.busy())
        if imgui.button("stop"):
            runner.stop()
        imgui.end_disabled()

        imgui.same_line()

        if runner.busy():
            imgui.text_colored(AMBER, f"running: ./ss {runner.command}")
        elif runner.rc is None:
            imgui.text_colored(GREY, "idle")
        elif runner.rc == 0:
            imgui.text_colored(GREEN, f"./ss {runner.command} ok ({runner.seconds:.1f} s)")
        else:
            imgui.text_colored(RED, f"./ss {runner.command} failed with {runner.rc}")

        if len(lines) != ui["log_count"]:
            log_editor.set_text("\n".join(lines))
            ui["log_count"] = len(lines)

            if ui["autoscroll"] and lines:
                log_editor.scroll_to_line(len(lines) - 1, text_edit.TextEditor.Scroll.align_bottom)

        if fonts["mono"] is not None:
            imgui.push_font(fonts["mono"], MONO_SIZE)

        log_editor.render("##log", imgui.ImVec2(0, 0))

        if fonts["mono"] is not None:
            imgui.pop_font()

    def gui() -> None:
        if interrupted.is_set():
            params.app_shall_exit = True

        imgui.text_colored(GREY, state.root)

        imgui.same_line()
        imgui.text("   ")
        imgui.same_line()
        dot(state.oocd_running(), "openocd")

        imgui.same_line()
        imgui.text("   ")
        imgui.same_line()
        elf = state.elf()
        dot(os.path.isfile(elf), f"elf {size_kb(elf)}", unknown=not os.path.isfile(elf))

        imgui.separator()

        top = imgui.get_content_region_avail().y * 0.62

        imgui.begin_child("##tabs", imgui.ImVec2(0, top))

        if imgui.begin_tab_bar("##tabbar"):
            if imgui.begin_tab_item("Build")[0]:
                build_tab()
                imgui.end_tab_item()

            if imgui.begin_tab_item("Examples")[0]:
                examples_tab()
                imgui.end_tab_item()

            if imgui.begin_tab_item("Matlab")[0]:
                matlab_tab()
                imgui.end_tab_item()

            imgui.end_tab_bar()

        imgui.end_child()

        log_pane()

    params = hello_imgui.RunnerParams()
    params.app_window_params.window_title = f"ss - {os.path.basename(state.root)}"
    params.ini_folder_type = hello_imgui.IniFolderType.app_user_config_folder
    params.ini_filename = f"ss_gui/{os.path.basename(state.root)}.ini"
    params.ini_filename_use_app_window_title = False
    params.app_window_params.window_geometry.size = (1100, 780)
    params.imgui_window_params.default_imgui_window_type = (
        hello_imgui.DefaultImGuiWindowType.provide_full_screen_window
    )
    params.fps_idling.fps_idle = 10
    params.callbacks.show_gui = gui

    def load_fonts() -> None:
        hello_imgui.imgui_default_settings.load_default_font_with_font_awesome_icons()
        fonts["mono"] = hello_imgui.load_font("fonts/Inconsolata-Medium.ttf", MONO_SIZE)

    params.callbacks.load_additional_fonts = load_fonts

    immapp.run(params)

    runner.stop()

    return 0


def handle_gui(args) -> None:
    sys.exit(gui_main())


def gui_add_sub(sub):
    parser_gui = sub.add_parser("gui", help="graphical front end for the ss commands")
    parser_gui.set_defaults(func=handle_gui)
