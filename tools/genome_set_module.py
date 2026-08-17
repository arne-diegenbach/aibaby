#!/usr/bin/env python3
"""Set one field inside one named [[module]] block of a genome TOML.

Usage: setfield.py <in.toml> <out.toml> <module>:<field>=<value> [more...]

Asserts that every requested edit actually matched. A sweep whose regex
silently misses is a sweep that measures the same genome N times and reports
a flat line -- that has happened in this project once already.
"""
import re
import sys

src, dst = sys.argv[1], sys.argv[2]
edits = []
for spec in sys.argv[3:]:
    mod, rest = spec.split(":", 1)
    field, value = rest.split("=", 1)
    edits.append((mod, field, value))

lines = open(src).read().split("\n")

# Index the [[module]] blocks by name.
blocks = {}
start = None
name = None
for i, ln in enumerate(lines):
    if ln.strip() == "[[module]]":
        if name is not None:
            blocks[name] = (start, i)
        start, name = i, None
    elif ln.startswith("[") and not ln.startswith("[[module]]") and start is not None and name is not None:
        blocks[name] = (start, i)
        start, name = None, None
    elif start is not None and name is None:
        m = re.match(r'^name\s*=\s*"([^"]+)"', ln)
        if m:
            name = m.group(1)
if name is not None:
    blocks[name] = (start, len(lines))

for mod, field, value in edits:
    assert mod in blocks, f"no [[module]] named {mod!r}; have {sorted(blocks)}"
    lo, hi = blocks[mod]
    hits = 0
    for i in range(lo, hi):
        if re.match(rf'^{re.escape(field)}\s*=', lines[i]):
            # Keep any trailing comment: it is usually the reason for the value.
            comment = ""
            if "#" in lines[i]:
                comment = "  " + lines[i][lines[i].index("#"):]
            lines[i] = f"{field} = {value}{comment}"
            hits += 1
    assert hits == 1, f"{mod}.{field}: matched {hits} lines, expected 1"

open(dst, "w").write("\n".join(lines))
