#!/usr/bin/env python3
"""Append a module->itself projection to a genome, cloned from an existing one.

Usage: genome_add_recurrent.py <in.toml> <out.toml> <module> [field=value ...]

  genome_add_recurrent.py dna/default.toml out.toml central hebb=1e-3 density=0.05

The companion to genome_set_projection.py, which edits a projection that is
already there. This one CREATES one, for the case a recurrent tract does not
exist in the genome at all — the self-organising alternative to DNA v42's
innate chain needs `central` wired to itself before STDP has anything to shape.

It clones the central->vocal projection rather than writing a literal block, so
every field the current DNA version defines is present with a sane value and a
genome field added later cannot silently default to zero here. That failure has
happened twice in this project: `genome_add_relay.py` went stale by four module
keys when v42 landed, and a scratch genome written before a field was added
measured a creature the genome no longer described.

Asserts the template exists and that the module is real, for the same reason
the three set_ scripts assert their edits matched: a sweep that quietly writes
nothing measures the same genome N times and draws a flat line.
"""
import re
import sys

if len(sys.argv) < 4:
    sys.exit(__doc__)

src, dst, module = sys.argv[1], sys.argv[2], sys.argv[3]
overrides = {}
for arg in sys.argv[4:]:
    if "=" not in arg:
        sys.exit(f"not a field=value: {arg!r}")
    k, v = arg.split("=", 1)
    overrides[k.strip()] = v.strip()

text = open(src).read()

names = re.findall(r'^\s*name\s*=\s*"([^"]+)"', text, flags=re.M)
if module not in names:
    sys.exit(f"no module named {module!r} in {src} (have: {', '.join(sorted(set(names)))})")

blocks = re.split(r"^\[\[projection\]\]", text, flags=re.M)
template = None
for b in blocks[1:]:
    if re.search(r'^\s*src\s*=\s*"central"', b, flags=re.M) and \
       re.search(r'^\s*dst\s*=\s*"vocal"', b, flags=re.M):
        template = b
        break
if template is None:
    sys.exit(f"no central->vocal projection in {src} to clone")

out, seen = ["", "[[projection]]"], set()
for line in template.split("\n"):
    m = re.match(r"^([a-z_0-9]+)\s*=\s*(.*)$", line)
    if not m:
        continue
    key, value = m.group(1), m.group(2).split("#")[0].strip()
    if key in ("src", "dst"):
        value = f'"{module}"'
    elif key in overrides:
        value = overrides.pop(key)
    seen.add(key)
    out.append(f"{key} = {value}")

if overrides:
    sys.exit(f"no such projection field(s): {', '.join(sorted(overrides))} "
             f"(template has: {', '.join(sorted(seen))})")

open(dst, "w").write(text.rstrip("\n") + "\n" + "\n".join(out) + "\n")
print(f"wrote {dst}: added {module}->{module}")
