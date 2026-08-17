#!/usr/bin/env python3
"""Set one field inside the [[projection]] block matching src/dst.

Usage: setproj.py <in.toml> <out.toml> <src>-><dst>:<field>=<value> [more...]

Asserts every edit matched exactly one line, for the same reason setfield.py
does: a sweep whose edit silently misses measures the same genome N times.
"""
import re
import sys

src_path, dst_path = sys.argv[1], sys.argv[2]
edits = []
for spec in sys.argv[3:]:
    route, rest = spec.split(":", 1)
    a, b = route.split("->")
    field, value = rest.split("=", 1)
    edits.append((a, b, field, value))

lines = open(src_path).read().split("\n")

# Block boundaries: a [[projection]] runs to the next line starting with '['.
blocks = []
start = None
for i, ln in enumerate(lines):
    if ln.strip() == "[[projection]]":
        if start is not None:
            blocks.append((start, i))
        start = i
    elif ln.startswith("[") and start is not None:
        blocks.append((start, i))
        start = None
if start is not None:
    blocks.append((start, len(lines)))


def field_of(lo, hi, name):
    for i in range(lo, hi):
        m = re.match(rf'^{name}\s*=\s*"([^"]+)"', lines[i])
        if m:
            return m.group(1)
    return None


for a, b, field, value in edits:
    hits = 0
    for lo, hi in blocks:
        if field_of(lo, hi, "src") != a or field_of(lo, hi, "dst") != b:
            continue
        for i in range(lo, hi):
            if re.match(rf'^{re.escape(field)}\s*=', lines[i]):
                comment = ""
                if "#" in lines[i]:
                    comment = "  " + lines[i][lines[i].index("#"):]
                lines[i] = f"{field} = {value}{comment}"
                hits += 1
    assert hits == 1, f"{a}->{b}.{field}: matched {hits} lines, expected 1"

open(dst_path, "w").write("\n".join(lines))
