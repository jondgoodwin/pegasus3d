"""Split a world's .ddd into many files and a manifest (see src/worldfetch.cone: a manifest ends in .world and lists the
files, one a line, in order; the world's text is the files' texts one after the other).

    python splitworld.py WORLD.ddd OUT_DIR NAME [--max-indent N] [--pad BYTES]

Each file starts at a line whose indent is at most --max-indent (default 4) and that begins a part or the like, so a file is
a run of whole declarations; the concatenation is the original text exactly. --pad adds that many bytes of comment lines to the
end of each file (a world whose files are bigger than first.ddd's few hundred bytes: the text is the same world, the files are
heavier, so a slow link has something to carry).
"""
import os
import sys


def main():
    args = sys.argv[1:]
    maxIndent = 4
    pad = 0
    while "--max-indent" in args:
        i = args.index("--max-indent"); maxIndent = int(args[i + 1]); del args[i:i + 2]
    while "--pad" in args:
        i = args.index("--pad"); pad = int(args[i + 1]); del args[i:i + 2]
    src, out, name = args
    lines = open(src, encoding="utf-8").read().split("\n")
    if lines and lines[-1] == "":
        lines.pop()
    chunks = [[]]
    for line in lines:
        stripped = line.lstrip(" ")
        indent = len(line) - len(stripped)
        starts = stripped.split("(")[0].split(" ")[0] in ("part", "worldAvatars", "fn", "each", "on", "solid", "field")
        if chunks[-1] and stripped and indent <= maxIndent and starts and not stripped.startswith("//"):
            chunks.append([])
        chunks[-1].append(line)
    os.makedirs(out, exist_ok=True)
    names = []
    for n, chunk in enumerate(chunks):
        text = "\n".join(chunk) + "\n"
        if pad:
            filler = "// padding to make the file heavier: " + "x" * 60 + "\n"
            while len(text.encode("utf-8")) < len("\n".join(chunk).encode("utf-8")) + pad:
                text += filler
        fname = "%s-%02d.ddd" % (name, n)
        with open(os.path.join(out, fname), "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        names.append(fname)
    with open(os.path.join(out, name + ".world"), "w", encoding="utf-8", newline="\n") as f:
        f.write("// the files of the world, in order\n" + "\n".join(names) + "\n")
    print("%s: %d files" % (name, len(names)))


main()
