"""Turns partbuild/vocab.list into partbuild/src/gen/vocabgen.cone: the vocabulary table and one shim per row.

    python partbuild/gen_vocab.py            write src/gen/vocabgen.cone
    python partbuild/gen_vocab.py --check    exit 1 if the committed file is not what the list gives (and write nothing)

The list's format is described at the top of vocab.list. What this writes:
  vocabulary(names)  the table (conescript's Vocabulary): each row's path, parameters (name, Ty, required, default as
                     slot words), what it makes and its slot count
  toolShims()        one shim per row, in the table's order: fn(handles, slots) bool. A shim reads every argument out
                     of the slots into a local before it writes anything (so its result may overwrite its arguments),
                     calls the row's expression, and writes the result over the slots from the first.
A wrong Cone type or expression in the list is a compile error in the generated file, not something this reads.
"""
import struct
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
LIST = HERE / "vocab.list"
OUT = HERE / "src" / "gen" / "vocabgen.cone"

FIRST_EIGHT = ["box", "plain", "srgb", "color", "rotation", "sunLight", "physical", "sphere"]

NUMBERS = {"Pure", "Length", "Angle", "Illuminance", "Duration", "Intensity"}
# Ty -> slots
SLOTS = {"Truth": 1, "SimTime": 1, "Point": 3, "Direction": 3, "Point2": 2, "LengthRange": 2, "Color": 4, "Rotation": 4}
# list and handle types: one slot, read through the handles
HANDLES = {
    "Shape": "readPieces", "Shapes": "readShapes", "Outline": "readOutline", "Outlines": "readOutlines", "Spine": "readSpine",
    "Stroke": "readStroke", "Strokes": "readStrokes", "Material": "readMaterial", "Modifier": "readModifier",
    "Modifiers": "readModifiers", "Points": "readPoints", "Points2": "readFlatPoints", "Numbers": "readNumbers",
}
RESULTS = {"Shape", "Material", "Color", "Rotation", "Light", "Volume", "SimTime", "Outline", "Spine", "Modifier",
           "Camera", "Stroke"}


def slots_of(ty):
    if ty in NUMBERS:
        return 1
    if ty in SLOTS:
        return SLOTS[ty]
    return 1


def f64_bits(x):
    return struct.unpack("<Q", struct.pack("<d", float(x)))[0]


def split_top(s, sep=","):
    """Split on sep outside parentheses."""
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == sep and depth == 0:
            out.append(cur)
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur)
    return [x.strip() for x in out]


def parse_default(text, ty, where):
    t = text.strip()
    if t == "none":
        return [0]
    if t in ("true", "false"):
        return [1 if t == "true" else 0]
    if t.startswith("("):
        words = [f64_bits(x) for x in split_top(t[1:-1])]
    else:
        words = [f64_bits(t)]
    if len(words) != slots_of(ty):
        sys.exit(f"{where}: default {t} has {len(words)} slot(s), a {ty} takes {slots_of(ty)}")
    return words


def rows():
    lines = []
    for raw in LIST.read_text(encoding="utf-8").splitlines():
        if not raw.strip() or raw.lstrip().startswith("#") and not raw.startswith((" ", "\t")):
            continue
        if raw.startswith((" ", "\t")) and lines:
            lines[-1] += " " + raw.strip()
        else:
            lines.append(raw.strip())
    out = []
    for line in lines:
        parts = line.split(" | ", 3)
        if len(parts) != 4:
            sys.exit(f"vocab.list: a row needs four fields (name | makes | params | expression): {line[:60]}")
        name, makes, params, expr = [p.strip() for p in parts]
        where = f"vocab.list row '{name}'"
        if makes not in RESULTS:
            sys.exit(f"{where}: unknown result type {makes}")
        plist = []
        for p in split_top(params):
            head, _, default = p.partition(" = ")
            pname, _, rest = head.partition(":")
            ty, _, cast = rest.strip().partition(" as ")
            pname, ty, cast = pname.strip(), ty.strip(), cast.strip()
            if ty not in NUMBERS and ty not in SLOTS and ty not in HANDLES:
                sys.exit(f"{where}: unknown parameter type {ty} for {pname}")
            plist.append({"name": pname, "ty": ty, "cast": cast,
                          "default": parse_default(default, ty, where) if default.strip() else None})
        names = {p["name"] for p in plist}
        import re
        for used in re.findall(r"\{(\w+)\}", expr):
            if used not in names:
                sys.exit(f"{where}: the expression uses {{{used}}}, which is not a parameter")
        out.append({"name": name, "makes": makes, "params": plist, "expr": expr})
    return out


def unpack(p, offset):
    ty, cast, o = p["ty"], p["cast"], offset

    def num(k):
        return f"f64.fromBits(slots[{k}usize])"
    if ty in NUMBERS:
        if cast == "f64":
            return "f64", num(o)
        if cast == "u32":
            return "u32", f"u32.from(i64.from({num(o)}))"
        return "f32", f"f32.from({num(o)})"
    if ty == "Truth":
        return "bool", f"slots[{o}usize] != 0u64"
    if ty in ("Point", "Direction"):
        return "Vec3", f"Vec3(f32.from({num(o)}), f32.from({num(o + 1)}), f32.from({num(o + 2)}))"
    if ty in ("Point2", "LengthRange"):
        return "Vec2", f"Vec2(f32.from({num(o)}), f32.from({num(o + 1)}))"
    if ty == "Color":
        return "Color", f"Color(f32.from({num(o)}), f32.from({num(o + 1)}), f32.from({num(o + 2)}), f32.from({num(o + 3)}))"
    if cast == "handle":
        return "u64", f"slots[{o}usize]"
    return "", f"{HANDLES[ty]}(handles, slots[{o}usize])"


def result_code(makes):
    if makes == "Color":
        return ["slots[0usize] = f64.from(result.red).bits();", "slots[1usize] = f64.from(result.green).bits();",
                "slots[2usize] = f64.from(result.blue).bits();", "slots[3usize] = f64.from(result.alpha).bits();"]
    if makes == "Rotation":
        return ["slots[0usize] = f64.from(result.x).bits();", "slots[1usize] = f64.from(result.y).bits();",
                "slots[2usize] = f64.from(result.z).bits();", "slots[3usize] = f64.from(result.w).bits();"]
    if makes == "SimTime":
        return ["slots[0usize] = result.bits();"]
    return ["slots[0usize] = handles.allocValue(result);"]


def generate():
    rs = rows()
    names = [r["name"] for r in rs]
    if names[:8] != FIRST_EIGHT:
        sys.exit(f"the first eight rows must be {FIRST_EIGHT} (--show prints min(row, 8)); they are {names[:8]}")
    if len(set(names)) != len(names):
        sys.exit("a row name appears twice")
    by = {r["name"]: r for r in rs}

    def layout(name, want):
        got = [p["name"] for p in by[name]["params"]][: len(want)]
        if got != want:
            sys.exit(f"row {name}: the checker reads its arguments by position as {want}; the list has {got}")
    layout("sphere", ["radius", "at"])
    layout("sim", ["time"])
    layout("rotation", ["angle", "about"])
    out = ["// GENERATED by partbuild/gen_vocab.py from partbuild/vocab.list. Do not edit: change the list and run the generator.",
           "// The vocabulary table, and one shim for each of its rows (see partbuild.cone).", ""]
    for i, r in enumerate(rs):
        out.append(f"// {i}: {r['name']}")
        out.append(f"fn row{i}(names: &mut Names) ToolSignature {{")
        out.append("  var path = List[u32]();")
        if not r["name"].startswith("["):
            for part in r["name"].split("."):
                out.append(f'  path.push(names.intern("{part}"));')
        out.append("  var params = List[ToolParam]();")
        for k, p in enumerate(r["params"]):
            out.append(f"  var fallback{k} = List[u64]();")
            if p["default"] is not None:
                for w in p["default"]:
                    out.append(f"  fallback{k}.push(0x{w:016X}u64);")
            out.append(f'  params.push(ToolParam(names.intern("{p["name"]}"), Ty.{p["ty"]}, {"true" if p["default"] is None else "false"}, fallback{k}));')
        out.append(f"  ToolSignature(path, params, Ty.{r['makes']}, {slots_of(r['makes'])}u32, 1u32)")
        out.append("}")
        out.append("")
    out.append("// The table: every row, in the list's order")
    out.append("pub fn vocabulary(names: &mut Names) Vocabulary {")
    out.append("  var v = Vocabulary.empty();")
    for i, r in enumerate(rs):
        out.append(f"  v.tools.push(row{i}(names));")
    out.append(f"  v.outlineList = {names.index('[outline]')}u32;")
    out.append(f"  v.materialList = {names.index('[material]')}u32;")
    out.append("  v")
    out.append("}")
    out.append("")
    for i, r in enumerate(rs):
        out.append(f"// {i}: {r['name']}")
        out.append(f"fn shim{i}(handles: &mut ToolHandles, slots: &mut Array[u64]) bool {{")
        off = 0
        expr = r["expr"]
        for p in r["params"]:
            cty, code = unpack(p, off)
            out.append(f"  let p_{p['name']}{': ' + cty if cty else ''} = {code};")
            off += slots_of(p["ty"])
        out.append("  if refused(handles) { return false; }")
        import re
        expr = re.sub(r"\{(\w+)\}", lambda m: "p_" + m.group(1), expr)
        out.append(f"  let result = {expr};")
        out.append("  if refused(handles) { return false; }")
        for line in result_code(r["makes"]):
            out.append("  " + line)
        out.append("  true")
        out.append("}")
        out.append("")
    out.append("// The shims, by the table's row numbers")
    out.append("pub fn toolShims() ToolShims {")
    out.append("  var s = ToolShims();")
    for i in range(len(rs)):
        out.append(f"  s.push(&shim{i});")
    out.append("  s")
    out.append("}")
    return "\n".join(out) + "\n"


def main():
    text = generate()
    if "--check" in sys.argv:
        have = OUT.read_text(encoding="utf-8") if OUT.exists() else ""
        if have.replace("\r\n", "\n") != text:
            print("partbuild/src/gen/vocabgen.cone is not what vocab.list gives: run python partbuild/gen_vocab.py")
            return 1
        print("vocabgen.cone is current")
        return 0
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(text, encoding="utf-8", newline="\n")
    print(f"wrote {OUT} ({len(text.splitlines())} lines)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
