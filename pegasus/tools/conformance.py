"""Conformance run: every world in guidebrowser/worlds, loaded in pegasus.exe and compared with the guide browser.

    python pegasus/tools/conformance.py                 run all, print the table; exit 1 on any difference in loads or meshes
    python pegasus/tools/conformance.py knight board    only these worlds
    python pegasus/tools/conformance.py --steps 600     steps pegasus runs (default 600)
    python pegasus/tools/conformance.py --pegasus PATH --guide PATH     (or the GUIDE_EXE environment variable; the default
                                                                         is the main checkout's guide browser build)

Per world:
  loads      the problem lines pegasus prints (loader) against `guidebrowser --check-world` (tools/checkworlds-expected is
             the same list, kept by tools/checkworlds.py); the guide browser's exit code says whether it found problems.
  meshes     the "meshhash NAME HASH" lines of `pegasus --world W --steps N` with PEGASUS_MESH_HASHES=1 against the
             guide browser's baselines tools/mesh-hashes/NAME.txt (what `guidebrowser --mesh-hashes` prints, held current
             by tools/checkhashes.py). Parts only the guide has and parts only pegasus has are listed with the counts.
  behaviour  the world's event lines ("created: ...", "X is now V") in tools/behaviour-baseline/NAME.log (the guide
             browser, driven by W119's clicks) against pegasus's "sim: step N, ..." lines (driven by pegasus's own
             script: walk, then a press at step 240). The two scripts differ, so only events in both are "same"; events
             only the guide has are reported (the guide's click is not reproduced), and pegasus-only ones likewise.
  checks     pegasus's own "check ..." lines and its final "pegasus: ok".
Needs SDL3 and the CEF runtime on PATH for the guide browser (SDL3_LIB, CEF_RELEASE override the folders).
"""
import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent.parent
WORLDS = REPO / "guidebrowser" / "worlds"
TOOLS = REPO / "tools"
SDL3 = os.environ.get("SDL3_LIB", r"C:\libs\SDL3-3.4.16\lib\x64")
CEF = os.environ.get("CEF_RELEASE", r"C:\src\cef\cef_binary_156.0.28+g57389f0+chromium-156.0.8078.88_windows64_minimal\Release")
GUIDE = Path(os.environ.get("GUIDE_EXE", r"C:\src\pegasus3d\guidebrowser\build\release\guidebrowser.exe"))
# The clicks of W119's behaviour baselines (tools/behaviour-logs.py) as presses on named parts: (step, part path). The
# guide clicked at screen fractions; pegasus presses along the ray from its own eye to the part's middle, so picking decides
# what is hit. board: a piece, then a square; guide2: the block; the knight and rook worlds have no click rule. The square
# is one no piece hides from pegasus's eye (the guide's sq[36] is behind the white king there), so the glide's target
# compares as 'sq[*]' (MASKED).
CLICKS = {
    "board": [(200, "pieces.whiteRook1"), (300, "squares.sq[35]")],
    "guide2": [(300, "block")],
    "knight": [(300, "knight")], "knightalone": [(300, "knight")], "knightclose": [(300, "knight")],
    "rook": [(300, "rooks")],
}
MASKED = {"board"}
# Events pegasus rightly has that the guide's baseline lacks, because the baseline's click (screen centre, measured 28 Nov
# also at y 0.62) did not reach the block, while pegasus's press on the block does: the world's own rule is
# on(clicked(block), start(keys(block.offset, ...))).
GUIDE_CLICK_MISSED = {"guide2": ["block clicked: keys on block.offset started"]}
MASK = [False]          # set per world: glide targets compare as one
PROBLEM = re.compile(r"^[^\s:][^:]*\.ddd:\d+:\d+: ")


def env(extra=None):
    e = dict(os.environ, PATH=os.pathsep.join([os.environ.get("PATH", ""), SDL3, CEF]))
    e.update(extra or {})
    return e


def guide_problems(exe: Path, world: str):
    done = subprocess.run([str(exe), "--check-world", world + ".ddd"], cwd=WORLDS, env=env(), capture_output=True, timeout=120)
    return done.returncode, done.stdout.decode("utf-8", "replace").splitlines()


def pegasus_run(exe: Path, world: str, steps: int, clicks=()):
    """(exit code, output lines) of pegasus run in the worlds folder, so problem lines name the file as the guide's do.
    The Loader's workers print a hash line in several writes, so two workers' lines can tear into each other; a run with a
    torn line is repeated (up to 5 times)."""
    for _ in range(5):
        cmd = [str(exe), "--world", world + ".ddd", "--steps", str(steps)]
        for step, path in clicks:
            cmd += ["--press-part", f"{step},{path}"]
        done = subprocess.run(cmd, cwd=WORLDS,
                              env=env({"PEGASUS_MESH_HASHES": "1"}), capture_output=True, timeout=300)
        lines = done.stdout.decode("utf-8", "replace").splitlines()
        if all(re.match(r"^meshhash \S+ \d+$", l) for l in lines if "meshhash" in l):
            break
    return done.returncode, lines


def hashes(lines, prefix=""):
    """(name, hash) pairs; a name can repeat (board has many parts called body)."""
    out = []
    for l in lines:
        m = re.match(r"^" + prefix + r"(\S+) (\d+)$", l)
        if m:
            out.append((m.group(1), m.group(2)))
    return out


def events_guide(name):
    f = TOOLS / "behaviour-baseline" / (name + ".log")
    out = []
    for l in f.read_text(encoding="utf-8").splitlines() if f.exists() else []:
        m = re.match(r"^world: step N(?: \([^)]*\))?, (.*)$", l)
        if m:
            out.append(norm_event(m.group(1)))
    return out


def events_pegasus(lines):
    out = []
    for l in lines:
        m = re.match(r"^sim: step \d+, (.*)$", l)
        if m and not m.group(1).startswith(("react:", "glide from")):     # pegasus's own diagnostics, not events
            out.append(norm_event(m.group(1)))
    return out


def norm_event(e):
    e = re.sub(r"\b\w+\.(\w+ is now)", r"\1", e)         # the guide names 'block.near', pegasus 'near'
    if MASK[0]:
        e = re.sub(r"glides to sq\[\d+\]", "glides to sq[*]", e)
    return e.replace(" is now true", " is now 1.0").replace(" is now false", " is now 0.0")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("worlds", nargs="*")
    ap.add_argument("--steps", type=int, default=600)
    ap.add_argument("--pegasus", default=str(REPO / "pegasus" / "build" / "release" / "pegasus.exe"))
    ap.add_argument("--guide", default=str(GUIDE))
    args = ap.parse_args()
    names = args.worlds or [p.stem for p in sorted(WORLDS.glob("*.ddd"))]
    rows, bad, behdiff = [], 0, 0
    for name in names:
        gcode, gprob = guide_problems(Path(args.guide), name)
        pcode, plines = pegasus_run(Path(args.pegasus), name, args.steps, CLICKS.get(name, ()))
        pprob = [l for l in plines if PROBLEM.match(l)]
        loads = "ok" if pprob == gprob else f"DIFF: guide {len(gprob)} problem(s), pegasus {len(pprob)}"
        loader = next((l for l in plines if l.startswith("loader: world")), "")
        m = re.search(r": (\d+) parts, (\d+) distinct mesh keys, (\d+) distinct material keys, (\d+) problems", loader)
        counts = f"{m.group(1)} parts, {m.group(4)} problems" if m else "no loader line"
        base = TOOLS / "mesh-hashes" / (name + ".txt")
        want = hashes(base.read_text(encoding="utf-8").splitlines()) if base.exists() else []
        got = hashes(plines, "meshhash ")
        # pegasus builds one mesh per distinct content key and names it by the first part, so parts the guide builds
        # twice (equal meshes) appear once: a guide part matches when pegasus built a mesh with its hash
        gotvals = {h for _, h in got}
        wantvals = {h for _, h in want}
        same = [k for k, h in want if h in gotvals]
        differ = [f"{k}={h}" for k, h in want if h not in gotvals]
        onlyg = []
        onlyp = [f"{k}={h}" for k, h in got if h not in wantvals]
        meshes = f"{len(same)}/{len(want)} match ({len(got)} distinct built)"
        if differ or onlyg or onlyp:
            meshes += f"; differ {differ}; guide only {onlyg}; pegasus only {onlyp}"
        MASK[0] = name in MASKED
        ge, pe = events_guide(name), events_pegasus(plines)
        missed = GUIDE_CLICK_MISSED.get(name, [])
        both = [e for e in ge if e in pe]
        clicked = name in CLICKS
        nopart = [l for l in plines if "press-part: no part" in l]
        behav = (f"{len(CLICKS[name])} click(s): " if clicked else "no clicks: ") + f"{len(both)} shared event(s)"
        same = ge == [e for e in pe if e not in missed]
        if clicked and same and not nopart:
            behav += ", identical in order" + (f" (pegasus also: {missed}, the guide's baseline click missed)" if missed else "")
        if clicked and (not same or nopart):
            behdiff += 1
        behav += "".join(f"; {l}" for l in nopart)
        if [e for e in ge if e not in pe]:
            behav += f"; guide only {[e for e in ge if e not in pe]}"
        if [e for e in pe if e not in ge]:
            behav += f"; pegasus only {[e for e in pe if e not in ge]}"
        checks = [l for l in plines if l.startswith("check ") and "FAILED" in l]
        okline = any(l == "pegasus: ok" for l in plines)
        chk = "ok" if okline and not checks else f"FAILED exit {pcode}: {checks}"
        if loads != "ok" or (differ or onlyg or onlyp) or chk != "ok":
            bad += 1
        rows.append((name, counts, loads, meshes, behav, chk))
    for r in rows:
        print(f"{r[0]}: [{r[1]}] loads {r[2]} | meshes {r[3]} | behaviour {r[4]} | pegasus checks {r[5]}")
    print(f"{len(rows)} world(s), {bad} with a difference in loads, meshes or checks; "
          f"{sum(1 for r in rows if r[0] in CLICKS)} with replayed clicks, {behdiff} of them with a different event list")
    return 1 if bad or behdiff else 0


if __name__ == "__main__":
    sys.exit(main())
