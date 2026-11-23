#!/usr/bin/env python3
"""Reads Lucide SVGs (ISC licence, C:/src/lucide/icons/icons) and writes src/paletteicons.cone: each icon as absolute
M/L/C/Z commands in the 24x24 box, which palette.cone strokes through Cone's vector2d renderer into the icon atlas.
Run: python gen_palette_icons.py   (paths are absolute)"""
import math, re, sys, xml.etree.ElementTree as ET

SRC = "C:/src/lucide/icons/icons/"
OUT = "C:/src/pegasus3d-wt-studiopalette/guidebrowser/src/paletteicons.cone"
NAMES = """undo-2 square-split-horizontal focus minimize-2 pencil mic file-plus external-link arrow-left arrow-right archive
folder-open rotate-cw globe square move-up-right tag type trash layout-dashboard zoom-in zoom-out arrow-down-to-line eye
message-circle pin sun-dim refresh-cw folder-plus palette scan-eye skip-forward skip-back play circle-dot lock-open
list-restart info x laptop-minimal-check monitor-off send keyboard""".split()


def arc_to_cubics(x1, y1, rx, ry, phi, fa, fs, x2, y2):
    if rx == 0 or ry == 0:
        return [("L", x2, y2)]
    rx, ry = abs(rx), abs(ry)
    p = math.radians(phi)
    c, s = math.cos(p), math.sin(p)
    dx, dy = (x1 - x2) / 2, (y1 - y2) / 2
    x1p, y1p = c * dx + s * dy, -s * dx + c * dy
    lam = x1p ** 2 / rx ** 2 + y1p ** 2 / ry ** 2
    if lam > 1:
        rx *= math.sqrt(lam)
        ry *= math.sqrt(lam)
    num = rx ** 2 * ry ** 2 - rx ** 2 * y1p ** 2 - ry ** 2 * x1p ** 2
    den = rx ** 2 * y1p ** 2 + ry ** 2 * x1p ** 2
    co = math.sqrt(max(0, num / den)) if den else 0
    if fa == fs:
        co = -co
    cxp, cyp = co * rx * y1p / ry, -co * ry * x1p / rx
    cx = c * cxp - s * cyp + (x1 + x2) / 2
    cy = s * cxp + c * cyp + (y1 + y2) / 2

    def ang(ux, uy, vx, vy):
        return math.atan2(ux * vy - uy * vx, ux * vx + uy * vy)

    th1 = ang(1, 0, (x1p - cxp) / rx, (y1p - cyp) / ry)
    dth = ang((x1p - cxp) / rx, (y1p - cyp) / ry, (-x1p - cxp) / rx, (-y1p - cyp) / ry)
    if not fs and dth > 0:
        dth -= 2 * math.pi
    if fs and dth < 0:
        dth += 2 * math.pi
    n = max(1, int(math.ceil(abs(dth) / (math.pi / 2) - 1e-6)))
    d = dth / n
    t = 4 / 3 * math.tan(d / 4)
    out = []
    a = th1

    def pt(u, v):
        return (c * rx * u - s * ry * v + cx, s * rx * u + c * ry * v + cy)

    for _ in range(n):
        b = a + d
        p1 = pt(math.cos(a) - t * math.sin(a), math.sin(a) + t * math.cos(a))
        p2 = pt(math.cos(b) + t * math.sin(b), math.sin(b) - t * math.cos(b))
        p3 = pt(math.cos(b), math.sin(b))
        out.append(("C", p1[0], p1[1], p2[0], p2[1], p3[0], p3[1]))
        a = b
    return out


def parse_path(d):
    toks = re.findall(r"[MmLlHhVvCcSsQqTtAaZz]|-?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?", d)
    i = 0
    cmds = []
    cx = cy = sx = sy = 0.0
    cmd = None
    lastc = None
    lastq = None

    def num():
        nonlocal i
        v = float(toks[i])
        i += 1
        return v

    def flag():
        # arc flags may be glued to the next number ("0 01.5")
        nonlocal i
        t = toks[i]
        if len(t) > 1 and t[0] in "01" and not t.startswith("0."):
            toks[i] = t[1:]
            return float(t[0])
        i += 1
        return float(t)

    while i < len(toks):
        if re.match(r"[A-Za-z]", toks[i]):
            cmd = toks[i]
            i += 1
        rel = cmd.islower()
        C = cmd.upper()
        ox, oy = (cx, cy) if rel else (0.0, 0.0)
        if C == "Z":
            cmds.append(("Z",))
            cx, cy = sx, sy
            lastc = lastq = None
            continue
        if C == "M":
            cx, cy = ox + num(), oy + num()
            sx, sy = cx, cy
            cmds.append(("M", cx, cy))
            cmd = "l" if rel else "L"
            lastc = lastq = None
        elif C == "L":
            cx, cy = ox + num(), oy + num()
            cmds.append(("L", cx, cy))
            lastc = lastq = None
        elif C == "H":
            cx = (cx if rel else 0) + num()
            cmds.append(("L", cx, cy))
            lastc = lastq = None
        elif C == "V":
            cy = (cy if rel else 0) + num()
            cmds.append(("L", cx, cy))
            lastc = lastq = None
        elif C == "C":
            x1, y1, x2, y2, x, y = ox + num(), oy + num(), ox + num(), oy + num(), ox + num(), oy + num()
            cmds.append(("C", x1, y1, x2, y2, x, y))
            lastc = (x2, y2)
            lastq = None
            cx, cy = x, y
        elif C == "S":
            x1, y1 = (2 * cx - lastc[0], 2 * cy - lastc[1]) if lastc else (cx, cy)
            x2, y2, x, y = ox + num(), oy + num(), ox + num(), oy + num()
            cmds.append(("C", x1, y1, x2, y2, x, y))
            lastc = (x2, y2)
            lastq = None
            cx, cy = x, y
        elif C in "QT":
            if C == "Q":
                qx, qy = ox + num(), oy + num()
            else:
                qx, qy = (2 * cx - lastq[0], 2 * cy - lastq[1]) if lastq else (cx, cy)
            x, y = ox + num(), oy + num()
            cmds.append(("C", cx + 2 / 3 * (qx - cx), cy + 2 / 3 * (qy - cy), x + 2 / 3 * (qx - x), y + 2 / 3 * (qy - y), x, y))
            lastq = (qx, qy)
            lastc = None
            cx, cy = x, y
        elif C == "A":
            rx, ry, phi = num(), num(), num()
            fa = flag()
            fs = flag()
            x, y = ox + num(), oy + num()
            cmds += arc_to_cubics(cx, cy, rx, ry, phi, int(fa), int(fs), x, y)
            cx, cy = x, y
            lastc = lastq = None
    return cmds


def ellipse(cx, cy, rx, ry):
    k = 0.5522847498
    return [("M", cx + rx, cy), ("C", cx + rx, cy + ry * k, cx + rx * k, cy + ry, cx, cy + ry),
            ("C", cx - rx * k, cy + ry, cx - rx, cy + ry * k, cx - rx, cy),
            ("C", cx - rx, cy - ry * k, cx - rx * k, cy - ry, cx, cy - ry),
            ("C", cx + rx * k, cy - ry, cx + rx, cy - ry * k, cx + rx, cy), ("Z",)]


def rrect(x, y, w, h, rx, ry):
    rx = min(rx, w / 2)
    ry = min(ry or rx, h / 2)
    k = 0.5522847498
    if rx <= 0:
        return [("M", x, y), ("L", x + w, y), ("L", x + w, y + h), ("L", x, y + h), ("Z",)]
    return [("M", x + rx, y), ("L", x + w - rx, y), ("C", x + w - rx + k * rx, y, x + w, y + ry - k * ry, x + w, y + ry),
            ("L", x + w, y + h - ry), ("C", x + w, y + h - ry + k * ry, x + w - rx + k * rx, y + h, x + w - rx, y + h),
            ("L", x + rx, y + h), ("C", x + rx - k * rx, y + h, x, y + h - ry + k * ry, x, y + h - ry),
            ("L", x, y + ry), ("C", x, y + ry - k * ry, x + rx - k * rx, y, x + rx, y), ("Z",)]


def icon(name):
    root = ET.parse(SRC + name + ".svg").getroot()
    cmds = []
    for e in root.iter():
        tag = e.tag.split("}")[1]
        a = e.attrib

        def f(k, d=0.0):
            return float(a.get(k, d))

        if tag == "path":
            cmds += parse_path(a["d"])
        elif tag == "circle":
            cmds += ellipse(f("cx"), f("cy"), f("r"), f("r"))
        elif tag == "ellipse":
            cmds += ellipse(f("cx"), f("cy"), f("rx"), f("ry"))
        elif tag == "rect":
            cmds += rrect(f("x"), f("y"), f("width"), f("height"), f("rx"), f("ry"))
        elif tag == "line":
            cmds += [("M", f("x1"), f("y1")), ("L", f("x2"), f("y2"))]
        elif tag in ("polyline", "polygon"):
            n = [float(v) for v in re.findall(r"-?[\d.]+", a["points"])]
            cmds += [("M" if j == 0 else "L", n[2 * j], n[2 * j + 1]) for j in range(len(n) // 2)]
            if tag == "polygon":
                cmds.append(("Z",))
    return " ".join(" ".join([c[0]] + ["%.3f" % v for v in c[1:]]) for c in cmds)


lines = ["// Generated by tools/icons/gen_palette_icons.py from Lucide icons (ISC licence, https://lucide.dev); do not edit.",
         "// Each icon: absolute M x y / L x y / C x1 y1 x2 y2 x y / Z commands in a 24 x 24 box, to be stroked 2 wide with round caps and joins.",
         "", "const ICON_COUNT = %du32;" % len(NAMES), "", "fn iconNames() List[String] {", "  var n = List[String]();"]
lines += ['  n.push(String.from("%s"));' % n for n in NAMES]
lines += ["  n", "}", "", "fn iconCommands(name: &str) String {"]
for n in NAMES:
    lines.append('  if name == "%s" { return String.from("%s"); }' % (n, icon(n)))
lines += ["  String()", "}", ""]
with open(OUT, "w", newline="\n") as fh:
    fh.write("\n".join(lines))
print("wrote", OUT, len(NAMES), "icons")
