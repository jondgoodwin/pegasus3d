r"""Runs the guide browser on each world for a fixed number of frames (board with two scripted clicks) and keeps the
lines the World actor prints ('world: step ...'), the behaviour baseline of step 3, lane C.

    python tools/behaviour-logs.py OUTDIR [--exe PATH]       writes OUTDIR\NAME.log, one per world
    python tools/behaviour-logs.py OUTDIR --compare BASEDIR  also reports every log that differs from BASEDIR's

The exe must sit in guidebrowser\build\release (it finds worlds beside it). SDL3 and the CEF runtime come from
SDL3_LIB and CEF_RELEASE (this machine's folders by default)."""
import argparse, os, re, subprocess, sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
SDL3 = os.environ.get("SDL3_LIB", r"C:\libs\SDL3-3.4.16\lib\x64")
CEF = os.environ.get("CEF_RELEASE", r"C:\src\cef\cef_binary_156.0.28+g57389f0+chromium-156.0.8078.88_windows64_minimal\Release")
FRAMES = "900"
# board and first: click a piece on frame 800, then a square on frame 830 (the two clicks of the pick-and-place rules)
PIECE_SQUARE = ["--click-xy", "0.2", "0.62", "800", "--click-xy", "0.53", "0.40", "830"]
CLICKS = {"board": PIECE_SQUARE, "first": PIECE_SQUARE,
          # guide2: walk up to the block (its area and derives), then click at the middle
          "guide2": ["--approach", "3", "--click-xy", "0.5", "0.5", "800"]}
# the others: a click at the middle on frame 800 (a click rule, if any, is on whatever stands there)
MIDDLE = ["--click-xy", "0.5", "0.5", "800"]


def run(exe, world, extra):
    path = os.pathsep.join([os.environ.get("PATH", ""), SDL3, CEF, str(REPO / "guidebrowser" / "cef" / "bin"),
                            str(REPO / "guidebrowser" / "shim" / "out")])
    env = dict(os.environ, PATH=path)
    cmd = [str(exe), "--world", world, "--exit-at", FRAMES] + extra
    done = subprocess.run(cmd, cwd=exe.parent, env=env, capture_output=True, timeout=300)
    lines = done.stdout.decode("utf-8", "replace").splitlines()
    return [l for l in lines if l.startswith("world:") or l.startswith("build:") or ("behaviors arrived" in l)]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--exe", default=str(REPO / "guidebrowser" / "build" / "release" / "guidebrowser.exe"))
    ap.add_argument("--compare")
    a = ap.parse_args()
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    exe = Path(a.exe).resolve()
    worlds = [w.stem for w in sorted((REPO / "guidebrowser" / "worlds").glob("*.ddd"))] + ["../../pegasus/worlds/first"]
    bad = 0
    for w in worlds:
        name = w.split("/")[-1]
        lines = run(exe, w, CLICKS.get(name, MIDDLE))
        # the frame a loop line names varies with timing: keep only that the behaviors arrived
        lines = ["loop: the world's behaviors arrived" if "behaviors arrived" in l else l for l in lines]
        # which step a world is created on and a click lands on varies with the machine's timing: they read 'step N'
        lines = [re.sub(r"\(\d+(\.\d+)? s\)", "(T s)", re.sub(r"step \d+", "step N", l)) for l in lines]
        (out / (name + ".log")).write_text("\n".join(lines) + "\n", encoding="utf-8")
        note = ""
        if a.compare:
            other = Path(a.compare) / (name + ".log")
            same = other.exists() and other.read_text(encoding="utf-8") == (out / (name + ".log")).read_text(encoding="utf-8")
            note = " same" if same else " DIFFERS"
            bad += 0 if same else 1
        print(f"{name}: {len(lines)} lines{note}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
