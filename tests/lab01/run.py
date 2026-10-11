#!/usr/bin/env python3
"""Build and test in a temporary copy; preserve the working fs.img."""
import argparse
import os
from pathlib import Path
import selectors
import shutil
import subprocess
import tempfile
import time


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--toolprefix", default="riscv64-linux-gnu-")
    parser.add_argument("--qemu", default="qemu-system-riscv64")
    parser.add_argument("--extra-cflags", default="")
    parser.add_argument("--log", type=Path, default=Path("/tmp/xv6-lab01-qemu.log"))
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[2]
    with tempfile.TemporaryDirectory(prefix="xv6-lab01-") as tmp:
        root = Path(tmp) / "xv6"
        shutil.copytree(repo / "xv6-labs-2024", root,
                        ignore=shutil.ignore_patterns(".git", "*.img", "*.o", "*.d", "*.asm", "*.sym"))
        # Clean only this disposable copy, including copied host executables.
        subprocess.run(["make", "TOOLPREFIX=" + args.toolprefix, "clean"], cwd=root, check=True,
                       stdout=subprocess.DEVNULL)
        programs = ["cptest", "difftest", "linetest"]
        for name in programs:
            shutil.copy2(repo / "tests/lab01" / (name + ".c"), root / "user" / (name + ".c"))
        (root / "tests.mk").write_text(
            "include Makefile\nCFLAGS += " + args.extra_cflags + "\n"
            "UPROGS += user/_cptest user/_difftest user/_linetest\n"
            "fs.img: user/_cptest user/_difftest user/_linetest\n")
        subprocess.run(["make", "-f", "tests.mk", "TOOLPREFIX=" + args.toolprefix,
                        "kernel/kernel", "fs.img"], cwd=root, check=True)
        cmd = [args.qemu, "-machine", "virt", "-bios", "none", "-kernel", "kernel/kernel",
               "-m", "128M", "-smp", "3", "-nographic", "-global", "virtio-mmio.force-legacy=false",
               "-drive", "file=fs.img,if=none,format=raw,id=x0",
               "-device", "virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0"]
        proc = subprocess.Popen(cmd, cwd=root, stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        selector = selectors.DefaultSelector()
        selector.register(proc.stdout, selectors.EVENT_READ)
        output = bytearray()
        pending = bytearray()
        sent = 0
        complete = False
        deadline = time.monotonic() + 120
        try:
            while time.monotonic() < deadline and not complete:
                for key, _ in selector.select(1):
                    chunk = os.read(key.fileobj.fileno(), 65536)
                    if not chunk:
                        raise RuntimeError("QEMU exited before tests finished")
                    output.extend(chunk)
                    pending.extend(chunk)
                    if b"$ " in pending:
                        pending.clear()
                        if sent == len(programs):
                            complete = True
                            break
                        proc.stdin.write((programs[sent] + "\n").encode())
                        proc.stdin.flush()
                        sent += 1
            markers = [b"ALL CP TESTS PASSED", b"ALL DIFF TESTS PASSED", b"ALL LINE TESTS PASSED"]
            if not complete or b"FAIL:" in output or not all(m in output for m in markers):
                raise RuntimeError("Tests failed or timed out; inspect QEMU log")
        finally:
            selector.close()
            if proc.poll() is None:
                try:
                    proc.stdin.write(b"\x01x")
                    proc.stdin.flush()
                    proc.wait(timeout=3)
                except (BrokenPipeError, subprocess.TimeoutExpired):
                    proc.terminate()
                    proc.wait(timeout=5)
            args.log.parent.mkdir(parents=True, exist_ok=True)
            args.log.write_bytes(output)
            print(output.decode(errors="replace"))
            proc.stdin.close()
            proc.stdout.close()
        print("PASS: cp, diff and line reader; log:", args.log)


if __name__ == "__main__":
    main()
