"""Make a large world out of real constructs (parts with lathes and boxes, materials, an each loop), not comments, for timing.

    python bigworld.py OUT.ddd BYTES

The world is `part big`, a materials part, then groups `part g0`, `part g1`, ... each a run of boxes and lathes (the lathe's
profile is spelled across lines, as a real world's is) and an each loop, until the text is about BYTES long. Each group sits at
indent 2, so `splitworld.py` cuts between groups: with F files wanted, make the world and cut with --max-indent 2; the number of
groups is what limits F, so the group size is chosen by --groups (default 150).
"""
import sys

MATERIALS = """part big                                              // a large world made of repeated real constructs
  part materials
    part lawn: physical(color: srgb(86, 120, 62), roughness: 0.9)
    part sand: physical(color: srgb(222, 204, 160), roughness: 0.95)
    part maple: [physical(color: srgb(226, 198, 150), roughness: 0.4), clearCoat(amount: 1, roughness: 0.06)]
    part walnut: [physical(color: srgb(92, 60, 40), roughness: 0.4), clearCoat(amount: 1, roughness: 0.06)]
"""


def lathe_part(name, x, z):
    return ("    part %s: lathe(profile: [(0, 0), (0.150, 0), (0.150, 0.018), (0.143, 0.026), (0.140, 0.034),\n"
            "                              (0.146, 0.042), (0.146, 0.050), (0.138, 0.058), (0.120, 0.064),\n"
            "                              (0.112, 0.074), (0.098, 0.092), (0.090, 0.115), (0.086, 0.160),\n"
            "                              (0.080, 0.330), (0.084, 0.370), (0.096, 0.392), (0.112, 0.400),\n"
            "                              (0.122, 0.560), (0.116, 0.568), (0.092, 0.568), (0, 0.568)],\n"
            "                   steps: 24),\n"
            "                 material: materials.maple, at: (%s, 0.1, %s)\n") % (name, x, z)


def group(k, parts):
    out = ["  part g%d, at: (%d, 0, %d)\n" % (k, (k % 30) * 3 - 45, (k // 30) * 3 - 6)]
    for i in range(parts):
        if i % 3 == 2:
            out.append(lathe_part("p%d_%d" % (k, i), i * 0.4, 0))
        else:
            out.append("    part b%d_%d: box(x: %.2f ..= %.2f, y: 0 ..= 0.2, z: -0.2 ..= 0.2), material: materials.walnut\n"
                       % (k, i, i * 0.4 - 0.15, i * 0.4 + 0.15))
    out.append("    part tiles%d, at: (0, 0, 1)\n      each f in 0 ..< 3 {\n        part t%d[f]: box(x: 0.5 * f - 0.2 ..= 0.5 * f + 0.2, y: 0 ..= 0.05, z: -0.2 ..= 0.2), material: materials.sand\n      }\n" % (k, k))
    return "".join(out)


def main():
    out, target = sys.argv[1], int(sys.argv[2])
    groups = 150
    for i, a in enumerate(sys.argv):
        if a == "--groups":
            groups = int(sys.argv[i + 1])
    # find the parts a group needs so that groups of that size reach the target
    parts = 1
    while len(MATERIALS) + sum(len(group(k, parts)) for k in range(groups)) < target:
        parts += 1
    text = MATERIALS + "".join(group(k, parts) for k in range(groups))
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    print("%s: %d bytes, %d groups of %d parts" % (out, len(text.encode("utf-8")), groups, parts))


main()
