#!/usr/bin/env python3
"""Set one field inside one top-level section of a genome TOML.

Usage: genome_set_header.py <in.toml> <out.toml> <section>:<field>=<value> [more...]

  genome_set_header.py dna/default.toml out.toml exploration:perturb_synaptic=1e-4

The companion to genome_set_module.py and genome_set_projection.py, for the
scalars that live in [stdp], [exploration], [homeo] and friends rather than in a
[[module]] or a [[projection]]. Same contract, and it exists for the same
reason: every requested edit must match exactly one line, because a sweep whose
regex silently misses measures the same genome N times and draws a flat line.

Scoped to the named section on purpose. Several field names appear in more than
one section, and a whole-file substitution would edit whichever came first.
"""
import re
import sys

src, dst = sys.argv[1], sys.argv[2]
edits = []
for spec in sys.argv[3:]:
    section, rest = spec.split(":", 1)
    field, value = rest.split("=", 1)
    edits.append((section, field, value))

lines = open(src).read().split("\n")

# Index the top-level [section] blocks. [[array]] tables end a section too --
# the module and projection blocks all sit after the header sections, and a
# section's range must not run past them.
blocks = {}
name, start = None, None
for i, ln in enumerate(lines):
    m = re.match(r'^\[([A-Za-z_][A-Za-z0-9_]*)\]\s*$', ln)
    if m:
        if name is not None:
            blocks[name] = (start, i)
        name, start = m.group(1), i + 1
    elif ln.startswith("[[") and name is not None:
        blocks[name] = (start, i)
        name, start = None, None
if name is not None:
    blocks[name] = (start, len(lines))

for section, field, value in edits:
    assert section in blocks, f"no [{section}] section; have {sorted(blocks)}"
    lo, hi = blocks[section]
    hits = 0
    for i in range(lo, hi):
        if re.match(rf'^{re.escape(field)}\s*=', lines[i]):
            # Keep any trailing comment: it is usually the reason for the value.
            comment = ""
            if "#" in lines[i]:
                comment = "  " + lines[i][lines[i].index("#"):]
            lines[i] = f"{field} = {value}{comment}"
            hits += 1
    assert hits == 1, f"[{section}].{field}: matched {hits} lines, expected 1"

open(dst, "w").write("\n".join(lines))
