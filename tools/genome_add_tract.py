#!/usr/bin/env python3
"""Append one plain random projection to a genome.

Usage:
  genome_add_tract.py <in.toml> <out.toml> SRC DST [--density D] [--weight W]
                      [--delay MS] [--hebb H] [--template SRC:DST]

Why this exists. The larynx has exactly one input that reliably turns into a
*distinguishable sound*: the auditory module, via the arcuate. The genome's own
comment on `auditory->vocal` records the word reaching vocal at 0.980 and the
voice at 0.840, where the seen object reaches vocal at 0.760 and the voice at
about 0.52. Same larynx, same readout -- so the difference is which pathway is
asking. This tool exists to give a sensory module a line into another one, and
the first thing it is for is `vision->auditory`: put the object into the module
that already knows how to make the larynx say something.

The projection is appended **last**, which is load-bearing. Projections are
wired in genome order off one RNG stream, so appending leaves every existing
projection drawing exactly the numbers it drew before -- which is what makes a
`--weight 0` arm a real control (same wiring, tract present, tract silent)
rather than a different creature. See tools/genome_add_relay.py, which says the
same thing about its relay, and the n_max note in tools/README.md.

Every field is cloned from an existing projection so that a key added in a
later DNA version cannot silently default to zero here.
"""
import re
import sys

if len(sys.argv) < 5:
    sys.exit(__doc__)
src_path, dst_path, SRC, DST = sys.argv[1:5]
opt = {"--density": "0.05", "--weight": "0.12", "--delay": "6.0",
       "--hebb": "0.0", "--template": "vision:vocal"}
args = sys.argv[5:]
for k, v in zip(args[::2], args[1::2]):
    if k not in opt:
        sys.exit(f"unknown flag {k!r}; have {sorted(opt)}")
    opt[k] = v

text = open(src_path).read()
names = re.findall(r'^\s*name\s*=\s*"([^"]+)"', text, flags=re.M)
for needed in (SRC, DST):
    if needed not in names:
        sys.exit(f"no module named {needed!r} in {src_path}")

t_src, t_dst = opt["--template"].split(":")
template = None
for b in re.split(r"^\[\[projection\]\]", text, flags=re.M)[1:]:
    body = b.split("[[")[0]
    if re.search(rf'^\s*src\s*=\s*"{t_src}"', body, flags=re.M) and \
       re.search(rf'^\s*dst\s*=\s*"{t_dst}"', body, flags=re.M):
        template = body
        break
if template is None:
    sys.exit(f"no {t_src}->{t_dst} projection in {src_path} to clone")

out = ["", "[[projection]]",
       f"# Appended by tools/genome_add_tract.py: {SRC} -> {DST}."]
for line in template.split("\n"):
    m = re.match(r"^([a-z_0-9]+)\s*=\s*([^#]*)", line)
    if not m:
        continue
    k, v = m.group(1), m.group(2).strip()
    if k == "src":              v = f'"{SRC}"'
    elif k == "dst":            v = f'"{DST}"'
    elif k == "density":        v = opt["--density"]
    elif k == "weight":         v = opt["--weight"]
    elif k == "delay_ms":       v = opt["--delay"]
    elif k == "hebb":           v = opt["--hebb"]
    out.append(f"{k} = {v}")
open(dst_path, "w").write(text.rstrip("\n") + "\n" + "\n".join(out) + "\n")
print(f"wrote {dst_path}: {SRC}->{DST} density {opt['--density']} "
      f"weight {opt['--weight']} hebb {opt['--hebb']}")
