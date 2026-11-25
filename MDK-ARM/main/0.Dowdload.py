#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
burn_hex.py - 单片机固件烧录工具（独立版）

流程:
  初始化: 选择 COM 口 -> 输入 HEX 路径（目录则自动枚举 .hex）
  每次烧录: 只选波特率 -> 调用 stm32flash.exe
"""

import os
import sys
import time
import glob
import shutil
import subprocess
import ctypes

try:
    sys.stdout.reconfigure(errors='replace')
    sys.stderr.reconfigure(errors='replace')
except Exception:
    pass

try:
    import serial
    import serial.tools.list_ports
    HAS_PYSERIAL = True
except ImportError:
    serial = None
    HAS_PYSERIAL = False

# ==================== 配置区 ====================
MCU_BAUD_OPTIONS = ["460800", "921600", "1000000", "1500000"]
STM32FLASH_EXE = "stm32flash.exe"
SKIP_TOKENS = {"skip", "s", "none", "no", "n", "跳过", "无"}
# ================================================

LINE = "=" * 50

CONFIG = {
    "com_port": None,
    "com_desc": None,
    "hex_file": None,
}


# ==================================================
# 通用工具
# ==================================================
def clear_screen():
    os.system("cls" if os.name == "nt" else "clear")


def set_title(title):
    try:
        ctypes.windll.kernel32.SetConsoleTitleW(title)
    except Exception:
        pass


def pause():
    if os.name == "nt":
        try:
            import msvcrt
            print("按任意键继续...", end="", flush=True)
            msvcrt.getch()
            print()
            return
        except Exception:
            pass
    input("按任意键继续...")


def print_banner(title):
    print(f"\n{LINE}")
    print(f"  {title}")
    print(f"{LINE}")


def choose_from_list(title, items, display_func=None):
    if not items:
        print(f"\n[错误] {title}: 没有找到可选项。")
        return None
    print_banner(title)
    for i, item in enumerate(items, 1):
        text = display_func(item) if display_func else str(item)
        print(f"  [{i}] {text}")
    print(LINE)
    while True:
        try:
            raw = input(f"请输入序号 (1-{len(items)}): ").strip()
            idx = int(raw) - 1
            if 0 <= idx < len(items):
                return items[idx]
            print(f"  [提示] 请输入 1 到 {len(items)} 之间的数字。")
        except ValueError:
            print(f"  [提示] 请输入 1 到 {len(items)} 之间的数字。")


def find_python():
    exe = sys.executable
    if exe:
        base = os.path.basename(exe).lower()
        if base.startswith("python") and "pythonw" not in base:
            return exe
    for name in ("python", "python3"):
        path = shutil.which(name)
        if path:
            return path
    return None


def ensure_pyserial():
    global serial, HAS_PYSERIAL
    if HAS_PYSERIAL:
        return True

    print("[提示] 未检测到 pyserial 库，正在安装...")
    python_cmd = find_python() or sys.executable
    try:
        r = subprocess.run([python_cmd, "-m", "pip", "install", "pyserial"])
    except Exception as e:
        print(f"[错误] 调用 pip 失败: {e}")
        return False

    if r.returncode != 0:
        print("[错误] pyserial 安装失败，请手动执行:")
        print(f"       {python_cmd} -m pip install pyserial")
        return False

    import importlib
    try:
        serial = importlib.import_module("serial")
        importlib.import_module("serial.tools.list_ports")
        HAS_PYSERIAL = True
        print("[信息] pyserial 安装成功。")
        print()
        return True
    except ImportError as e:
        print(f"[错误] pyserial 导入失败: {e}")
        return False


def list_com_ports():
    if HAS_PYSERIAL and serial is not None:
        try:
            from serial.tools import list_ports
            ports = [(p.device, p.description or p.device)
                     for p in list_ports.comports()]
            if ports:
                ports.sort(key=lambda x: x[0])
                return ports
        except Exception:
            pass

    ps_script = (
        "$OutputEncoding = [System.Text.Encoding]::UTF8; "
        "[Console]::OutputEncoding = [System.Text.Encoding]::UTF8; "
        "Get-WmiObject Win32_PnPEntity | "
        "Where-Object {$_.Name -match '\\(COM\\d+\\)'} | "
        "ForEach-Object { if ($_.Name -match '\\((COM\\d+)\\)') "
        "{ $matches[1] + '|' + $_.Name } }"
    )
    try:
        r = subprocess.run(
            ["powershell", "-NoProfile", "-Command", ps_script],
            capture_output=True, text=True, encoding="utf-8", errors="ignore",
        )
        ports = []
        for line in r.stdout.splitlines():
            line = line.strip()
            if "|" in line:
                port, desc = line.split("|", 1)
                ports.append((port.strip(), desc.strip()))
        return ports
    except Exception:
        return []


def _normalize_input_path(raw):
    if not raw:
        return ""
    s = raw.strip()
    while len(s) >= 2 and s[0] == s[-1] and s[0] in ('"', "'"):
        s = s[1:-1].strip()
    s = os.path.expanduser(os.path.expandvars(s))
    return s


def _scan_files_in_dir(folder, ext):
    ext = ext.lower().lstrip(".")
    files = set()
    for pat in (f"*.{ext}", f"*.{ext.upper()}"):
        files.update(glob.glob(os.path.join(folder, "**", pat), recursive=True))
    return sorted(f for f in files if os.path.isfile(f))


def ask_file_by_path(prompt, ext, default_dir="."):
    ext = ext.lower().lstrip(".")
    while True:
        raw = input(prompt)
        s = _normalize_input_path(raw)

        if s.lower() in SKIP_TOKENS:
            return None

        if not s:
            s = default_dir

        if not os.path.exists(s):
            print(f"  [错误] 路径不存在: {s}")
            print(f"         请重新输入（或输入 skip 跳过本项）。")
            continue

        if os.path.isdir(s):
            files = _scan_files_in_dir(s, ext)
            if not files:
                print(f"  [错误] 目录中未找到 .{ext} 文件: {s}")
                print(f"         请重新输入（或输入 skip 跳过本项）。")
                continue
            if len(files) == 1:
                print(f"  [信息] 自动选择唯一的 .{ext} 文件: {files[0]}")
                return files[0]
            selected = choose_from_list(
                f"目录中的 .{ext} 文件 (共 {len(files)} 个)",
                files,
                display_func=lambda f: f"{os.path.relpath(f, s)}  "
                                       f"({os.path.getsize(f):,} 字节)"
            )
            if selected is None:
                continue
            return selected
        else:
            if not s.lower().endswith(f".{ext}"):
                print(f"  [错误] 文件扩展名不是 .{ext}: {s}")
                print(f"         请重新输入（或输入 skip 跳过本项）。")
                continue
            return os.path.abspath(s)


# ==================================================
# 初始化配置
# ==================================================
def initial_setup():
    clear_screen()
    print_banner("单片机烧录 - 初始化配置")
    print("  请依次配置: 串口 -> HEX 文件路径")
    print()

    tmp = dict(CONFIG)

    # 1. 串口
    ports = list_com_ports()
    if not ports:
        print("\n[错误] 未检测到任何串口设备。")
        print("       请连接开发板后重新运行。")
        pause()
        return False

    port = choose_from_list("可用串口", ports,
                            display_func=lambda p: f"{p[0]} - {p[1]}")
    if not port:
        return False
    tmp["com_port"], tmp["com_desc"] = port
    print(f"  已选择串口: {tmp['com_port']}  {tmp['com_desc']}")

    # 2. HEX 路径
    print()
    print_banner("HEX 固件路径")
    print("  - 输入目录: 自动枚举其中所有 .hex 文件（含子目录）")
    print("  - 输入文件: 直接使用该 .hex 文件")
    print("  - 直接回车: 使用当前目录")
    print("  - 输入 skip: 跳过本项（保留原值）")
    print("  - 也可以直接把文件/文件夹拖到这里")
    print(LINE)
    hex_path = ask_file_by_path("  HEX 路径: ", "hex", default_dir=".")
    if hex_path is None:
        print("  [信息] 已跳过 HEX 配置。")
    else:
        tmp["hex_file"] = hex_path
        print(f"  已选择 HEX: {hex_path}")

    CONFIG.update(tmp)

    print()
    print_banner("配置完成")
    print(f"  串口: {CONFIG['com_port']}  {CONFIG['com_desc']}")
    print(f"  HEX : {CONFIG['hex_file'] or '(未配置)'}")
    print(LINE)

    if not CONFIG["hex_file"]:
        print("\n  [错误] HEX 文件未配置，无法继续。")
        pause()
        return False

    time.sleep(0.8)
    return True


# ==================================================
# HEX 烧录逻辑
# ==================================================
def burn_mcu():
    baud_display = lambda b: f"{b} bps   (推荐，稳定)" if b == "460800" else f"{b} bps"
    baud = choose_from_list("下载波特率", MCU_BAUD_OPTIONS,
                            display_func=baud_display)
    if not baud:
        return
    print(f"  已选择: {baud} bps")

    com_port = CONFIG["com_port"]
    hex_file = CONFIG["hex_file"]

    print()
    print_banner("开始烧录...")
    print()

    cmd = [
        f".\\{STM32FLASH_EXE}",
        "-i", "rts-dtr,rtsdtr,,,,,",
        "-b", baud,
        "-w", hex_file,
        "-g", "0x00",
        com_port,
    ]
    print(f"  命令: {' '.join(cmd)}")
    print()

    try:
        r = subprocess.run(cmd)
        exit_code = r.returncode
    except Exception as e:
        print(f"[错误] 调用 {STM32FLASH_EXE} 失败: {e}")
        exit_code = -1

    print()
    print(LINE)
    if exit_code == 0:
        print("  [成功] 单片机烧录完成")
    else:
        print(f"  [失败] 烧录出错，退出码: {exit_code}")
    print(LINE)


# ==================================================
# 主流程
# ==================================================
def main():
    try:
        script_dir = os.path.dirname(os.path.abspath(__file__))
    except NameError:
        script_dir = os.path.dirname(os.path.abspath(sys.argv[0]))
    try:
        os.chdir(script_dir)
    except Exception:
        pass

    set_title("单片机固件烧录工具")

    if not os.path.exists(STM32FLASH_EXE):
        print(f"\n[错误] 当前目录缺少 {STM32FLASH_EXE}")
        print("       请将其放在与脚本同一目录。")
        pause()
        return

    while True:
        if not initial_setup():
            print("\n[取消] 未完成初始化配置，程序退出。")
            return

        while True:
            clear_screen()
            print(LINE)
            print("  单片机固件烧录")
            print(LINE)
            print(f"  串口: {CONFIG['com_port']}  {CONFIG['com_desc']}")
            print(f"  HEX : {CONFIG['hex_file']}")
            print("-" * 50)
            print("  [1] 开始烧录")
            print("  [2] 重新配置")
            print("  [0] 退出")
            print(LINE)
            print()

            try:
                choice = input("请选择 [1/2/0]: ").strip().replace(" ", "")
            except KeyboardInterrupt:
                print("\n\n已退出。")
                return

            if choice == "1":
                if not os.path.exists(CONFIG["hex_file"]):
                    print(f"\n[错误] HEX 文件不存在: {CONFIG['hex_file']}")
                    print("       请按 [2] 重新配置。")
                    pause()
                    continue
                burn_mcu()
                print()
                pause()
            elif choice == "2":
                break
            elif choice == "0":
                print("\n已退出。")
                return
            else:
                print("\n[提示] 无效输入，请重新选择...")
                time.sleep(1)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n已退出。")
        sys.exit(0)