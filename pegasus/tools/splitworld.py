"""Split a world's .ddd into many files and a manifest (see src/worldfetch.cone: a manifest ends in .world and has one line a
file, "NAME SIZE HASH PATH"; the world's text is the files' texts one after the other).

    python splitworld.py WORLD.ddd OUT_DIR NAME [--max-indent N] [--pad BYTES]

Each file starts at a line whose indent is at most --max-indent (default 2: the root's children) and that begins a part or the
like, so a file is a run of whole declarations; the concatenation is the original text exactly. --pad adds that many bytes of
comment lines to the end of each file (a world whose files are bigger than first.ddd's few hundred bytes: the text is the same
world, the files are heavier, so a slow link has something to carry).

The manifest line of a file: NAME; SIZE, its bytes; HASH, 16 hex digits of the sum below (the Loader's `contentHash`,
meshcache.cone `cacheSum`); PATH, the dotted path of the part declared on the file's first line, or when the file starts with
something else (a field, an each, an on), of the part that line sits in.
"""
import os
import sys

MASK = (1 << 64) - 1
FNV_OFFSET = 14695981039346656037
FNV_PRIME = 1099511628211


def content_hash(data):
    """meshcache.cone `cacheSum`: a word at a time (little-endian), then byte by byte, all in 64 bits."""
    h = FNV_OFFSET
    words = len(data) // 8
    for i in range(words):
        w = int.from_bytes(data[i * 8:i * 8 + 8], "little")
        h = ((((h ^ w) * FNV_PRIME) & MASK) ^ (h >> 29))
    for j in range(words * 8, len(data)):
        h = ((h ^ data[j]) * FNV_PRIME) & MASK
    return h


def part_name(stripped):
    """The name after 'part ' ('part lawn: physical(...)' is lawn), or None if the line does not declare a part."""
    if not stripped.startswith("part "):
        return None
    rest = stripped[5:]
    end = 0
    while end < len(rest) and (rest[end].isalnum() or rest[end] == "_"):
        end += 1
    return rest[:end] or None


def main():
    args = sys.argv[1:]
    maxIndent = 2
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
    paths = [None]
    stack = []  # (indent, part name) of the parts open at the current line
    for line in lines:
        stripped = line.lstrip(" ")
        indent = len(line) - len(stripped)
        starts = stripped.split("(")[0].split(" ")[0] in ("part", "worldAvatars", "fn", "each", "on", "solid", "field")
        code = bool(stripped) and not stripped.startswith("//")
        if code:
            while stack and stack[-1][0] >= indent:
                stack.pop()
        pname = part_name(stripped) if code else None
        if chunks[-1] and code and indent <= maxIndent and starts:
            chunks.append([])
            paths.append(None)
        if paths[-1] is None and code:
            paths[-1] = ".".join([n for _, n in stack] + ([pname] if pname else []))
        chunks[-1].append(line)
        if pname:
            stack.append((indent, pname))
    os.makedirs(out, exist_ok=True)
    entries = []
    for n, chunk in enumerate(chunks):
        text = "\n".join(chunk) + "\n"
        if pad:
            filler = "// padding to make the file heavier: " + "x" * 60 + "\n"
            while len(text.encode("utf-8")) < len("\n".join(chunk).encode("utf-8")) + pad:
                text += filler
        fname = "%s-%02d.ddd" % (name, n)
        data = text.encode("utf-8")
        with open(os.path.join(out, fname), "wb") as f:
            f.write(data)
        entries.append("%s %d %016x %s" % (fname, len(data), content_hash(data), paths[n] or name))
    with open(os.path.join(out, name + ".world"), "w", encoding="utf-8", newline="\n") as f:
        f.write("// the files of the world, in order: NAME SIZE HASH PATH\n" + "\n".join(entries) + "\n")
    print("%s: %d files" % (name, len(entries)))


main()
