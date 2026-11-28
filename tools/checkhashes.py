"""Checks every world in guidebrowser/worlds with `guidebrowser --mesh-hashes FILE`.

The program builds the world with no window and prints one line per part: the part's name and a hash of its mesh
(positions, normals, uvs, indices, by their bits). The lines are compared with tools/mesh-hashes/NAME.txt. Any change
to how parts are built that moves one bit of one mesh shows here, so this is the determinism test for the vocabulary
table and the shims (and, later, the interpreter).

    python tools/checkhashes.py                 check; exit 1 if anything differs
    python tools/checkhashes.py --update        write the baselines from what the program says now
    python tools/checkhashes.py --exe PATH      another guidebrowser.exe

Build timing lines the program prints between ("build: union took ...") are not part of the lines compared: only
lines of the form "<name> <digits>" are.
"""
import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
WORLDS = REPO / "guidebrowser" / "worlds"
BASELINES = Path(__file__).resolve().parent / "mesh-hashes"
SDL3 = os.environ.get("SDL3_LIB", r"C:\libs\SDL3-3.4.16\lib\x64")
CEF = os.environ.get("CEF_RELEASE", r"C:\src\cef\cef_binary_156.0.28+g57389f0+chromium-156.0.8078.88_windows64_minimal\Release")
LINE = re.compile(r"^.+ \d+$")
TIMING = re.compile(r"^(build|loader): ")


def run(exe: Path, name: str):
    env = dict(os.environ, PATH=os.pathsep.join([os.environ.get("PATH", ""), SDL3, CEF]))
    done = subprocess.run([str(exe), "--mesh-hashes", name], cwd=WORLDS, env=env, capture_output=True, timeout=300)
    text = done.stdout.decode("utf-8", "replace").splitlines()
    return done.returncode, [l for l in text if LINE.match(l) and not TIMING.match(l)]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--update", action="store_true")
    ap.add_argument("--exe", default=str(REPO / "guidebrowser" / "build" / "release" / "guidebrowser.exe"))
    args = ap.parse_args()
    exe = Path(args.exe)
    BASELINES.mkdir(exist_ok=True)
    bad = 0
    for world in sorted(WORLDS.glob("*.ddd")):
        code, lines = run(exe, world.name)
        file = BASELINES / (world.stem + ".txt")
        if args.update:
            file.write_text("".join(l + "\n" for l in lines), encoding="utf-8")
            print(f"{world.name}: {len(lines)} part(s), exit {code}, written")
            continue
        want = file.read_text(encoding="utf-8").splitlines() if file.exists() else []
        ok = lines == want and code == 0
        print(f"{'ok  ' if ok else 'FAIL'} {world.name}: {len(lines)} part(s), exit {code}")
        if not ok:
            bad += 1
            for a, b in zip(lines, want):
                if a != b:
                    print(f"     got  {a}\n     want {b}")
                    break
            if len(lines) != len(want):
                print(f"     {len(lines)} lines, baseline has {len(want)}")
    print("all identical" if not bad and not args.update else (f"{bad} differ" if bad else "baselines written"))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
