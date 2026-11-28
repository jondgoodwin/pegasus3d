# Checks tests/encodings.out against LLVM: every line of it is "bytes | the text LLVM must show", and llvm-mc
# --disassemble must read the bytes as that one instruction. A testing tool only; nothing ships with it.
# Run it after 'congo test' (which proves encodings.out is what the encoder makes):  python tests/check_encodings.py
import subprocess, sys, os

here = os.path.dirname(os.path.abspath(__file__))
out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(here, "encodings.out")
mc = os.environ.get("LLVM_MC", r"C:\LLVM\23\bin\llvm-mc.exe")

cases = [l.rstrip("\n").split(" | ", 1) for l in open(out) if " | " in l]
text = "\n".join(" ".join("0x" + b for b in c[0].split(" ")) for c in cases) + "\n"
r = subprocess.run([mc, "--disassemble", "-triple=x86_64", "--output-asm-variant=1"], input=text,
                   capture_output=True, text=True)
got = [" ".join(l.split()) for l in r.stdout.splitlines() if l.strip()]
if r.stderr.strip():
    print("llvm-mc said:", r.stderr.strip().splitlines()[:5])
if len(got) != len(cases):
    print("llvm-mc read %d instructions for %d cases (a length disagreement)" % (len(got), len(cases)))
    sys.exit(1)
bad = 0
for (b, want), g in zip(cases, got):
    if " ".join(want.split()) != g:
        bad += 1
        if bad <= 20:
            print("MISMATCH  %s | want: %s | llvm: %s" % (b, want, g))
if bad:
    print("%d of %d encodings disagree with LLVM" % (bad, len(cases)))
    sys.exit(1)
print("all %d encodings agree with llvm-mc" % len(cases))
