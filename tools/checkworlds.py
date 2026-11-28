"""Checks every world in guidebrowser/worlds with `guidebrowser --check-world FILE`.

Each world's problems (file:line:column: message, one per line) are compared with the expected-problems file
tools/checkworlds-expected/NAME.txt (absent or empty: no problems expected) and the exit code with whether any
are expected (0 for none, 1 otherwise). Then three unreadable inputs (an empty file, 4 KB of random bytes and a
5 MB file) must each give at least one problem and exit 1, not crash.

    python tools/checkworlds.py                 check; exit 1 if anything differs
    python tools/checkworlds.py --update        write the expected files from what the program says now
    python tools/checkworlds.py --exe PATH      another guidebrowser.exe

The program needs SDL3 and the CEF runtime on PATH to start, as it does for any run; the defaults below are this
machine's folders (environment variables SDL3_LIB and CEF_RELEASE override them).
"""
import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
WORLDS = REPO / "guidebrowser" / "worlds"
PEGASUS_WORLDS = REPO / "pegasus" / "worlds"      # the new browser's worlds, checked the same way
EXPECTED = Path(__file__).resolve().parent / "checkworlds-expected"
SDL3 = os.environ.get("SDL3_LIB", r"C:\libs\SDL3-3.4.16\lib\x64")
CEF = os.environ.get("CEF_RELEASE", r"C:\src\cef\cef_binary_156.0.28+g57389f0+chromium-156.0.8078.88_windows64_minimal\Release")


def run(exe: Path, folder: Path, name: str):
    """(exit code, stdout lines) of `guidebrowser --check-world name`, run in folder so the file names print alike."""
    env = dict(os.environ, PATH=os.pathsep.join([os.environ.get("PATH", ""), SDL3, CEF]))
    done = subprocess.run([str(exe), "--check-world", name], cwd=folder, env=env, capture_output=True, timeout=120)
    return done.returncode, done.stdout.decode("utf-8", "replace").splitlines()


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--update", action="store_true")
    ap.add_argument("--exe", default=str(REPO / "guidebrowser" / "build" / "release" / "guidebrowser.exe"))
    args = ap.parse_args()
    exe = Path(args.exe)
    worlds = sorted(WORLDS.glob("*.ddd")) + sorted(PEGASUS_WORLDS.glob("*.ddd"))
    bad = 0
    if args.update:
        EXPECTED.mkdir(exist_ok=True)
    for world in worlds:
        code, lines = run(exe, world.parent, world.name)
        want_file = EXPECTED / (world.stem + ".txt")
        if args.update:
            if lines:
                want_file.write_text("\n".join(lines) + "\n", encoding="utf-8")
            elif want_file.exists():
                want_file.unlink()
            print(f"{world.name}: {len(lines)} problem(s), exit {code}, written")
            continue
        want = want_file.read_text(encoding="utf-8").splitlines() if want_file.exists() else []
        ok = lines == want and code == (1 if want else 0)
        print(f"{'ok  ' if ok else 'FAIL'} {world.name}: {len(lines)} problem(s), exit {code}")
        if not ok:
            bad += 1
            print("  expected:", want)
            print("  got:     ", lines)
    if not args.update:
        with tempfile.TemporaryDirectory() as tmp:
            folder = Path(tmp)
            (folder / "empty.ddd").write_bytes(b"")
            (folder / "random.ddd").write_bytes(os.urandom(4096))
            (folder / "huge.ddd").write_bytes(b"x" * (5 * 1024 * 1024))
            for name in ("empty.ddd", "random.ddd", "huge.ddd"):
                code, lines = run(exe, folder, name)
                ok = code == 1 and len(lines) >= 1
                print(f"{'ok  ' if ok else 'FAIL'} {name}: {len(lines)} problem(s), exit {code}")
                bad += 0 if ok else 1
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
