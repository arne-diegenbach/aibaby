#!/usr/bin/env python3
"""Wire a module's travelling wave onto one vocal motor group (DNA v43).

Usage: genome_add_topographic.py <in.toml> <out.toml> [--group N] [--reverse]
                                 [--density D] [--weight W] [--src-hi F]
                                 [--sigma S] [--src MODULE]

  genome_add_topographic.py dna/default.toml out.toml --group 2
  genome_add_topographic.py out.toml out2.toml --group 3 --reverse

WHY. `VocalDecoder` reads each motor parameter as the centroid of firing rate
WITHIN one 14-neuron group: F1 is the centre of mass of vocal[28..42]. Moving F1
means differentially activating positions inside that one slice. Every tract
into the larynx has been `kind = "random"`, which touches every position with
equal probability — so its centroid sits at 0.5 however loud it is. That is the
measured HVC result (more weight drones before it articulates) and the reason
genome_add_relay.py records for central->vocal.

Group order is fixed by the decoder: 0 f0, 2 F1, 3 F2, 4 F3, 5-7 bandwidths.

--reverse maps the source backwards, which is not a flourish. [i] -> [a] raises
F1 while F2 falls, so a forward map into group 2 and a reversed one into group 3
turn ONE travelling wave into a diphthong; two forward maps would slide both
formants together, which is one articulatory dimension and not a vowel
trajectory.

DEFAULTS THAT ARE DERIVED, NOT PICKED:
  --sigma 0.07   one destination neuron: 1/14 of the group, the resolution at
                 which adjacent wave positions stay distinguishable. Wider and
                 each source reaches most of the group, which is `random` with
                 extra steps and returns the centroid to 0.5.
  --src-hi 0.70  central's measured wave sweeps neurons 14 -> 265 of 400, so
                 mapping the whole module would spend a third of the
                 destination range on source positions the wave never visits.

`--density` and `--weight` are NOT derived and must be calibrated: run `babble`
and watch the duty cycle. Driving one group is structurally safer than the HVC
route, which drove all nine and so pushed voicing and amplitude with it.
"""
import re
import sys

VOCAL_GROUPS = 9

args = sys.argv[1:]
if len(args) < 2:
    sys.exit(__doc__)
src_path, dst_path = args[0], args[1]

opt = {"--group": "2", "--density": "0.35", "--weight": "0.14",
       "--src-hi": "0.70", "--sigma": "0.07", "--src": "central",
       # A projection can also aim at an arbitrary sub-range of an arbitrary
       # module, which is what a chain TRIGGER is: every source cell landing on
       # the first link, so a sensory onset starts the wave. --dst overrides the
       # vocal-group form and --dst-lo/--dst-hi give the range directly.
       "--dst": "", "--dst-lo": "", "--dst-hi": "",
       # A TRIGGER MUST BE TRANSIENT. A plain projection onto the chain head
       # delivers sustained drive and raises the head's rate without launching
       # anything; seqprobe's kick is 10 ticks and then off. DNA v36's
       # depressing synapse passes the first spike of a burst and little of the
       # rest, which is an onset detector built from a mechanism that already
       # ships. Set --stp-use ~0.7 with a long --stp-recover for that.
       "--stp-use": "", "--stp-recover": "", "--stp-facil": ""}
reverse = False
i = 2
while i < len(args):
    if args[i] == "--reverse":
        reverse = True
        i += 1
        continue
    if args[i] not in opt:
        sys.exit(f"unknown option {args[i]!r}\n{__doc__}")
    if i + 1 >= len(args):
        sys.exit(f"{args[i]} needs a value")
    opt[args[i]] = args[i + 1]
    i += 2

group = int(opt["--group"])
if not 0 <= group < VOCAL_GROUPS:
    sys.exit(f"group must be 0..{VOCAL_GROUPS - 1}; the decoder defines only those")

text = open(src_path).read()
names = re.findall(r'^\s*name\s*=\s*"([^"]+)"', text, flags=re.M)
for needed in (opt["--src"], "vocal"):
    if needed not in names:
        sys.exit(f"no module named {needed!r} in {src_path}")

# Clone an existing projection so that a field added in a later DNA version
# cannot silently default to zero here — the way genome_add_relay.py went stale
# by four keys when v42 landed.
blocks = re.split(r"^\[\[projection\]\]", text, flags=re.M)
template = None
for b in blocks[1:]:
    body = b.split("[[")[0]
    if re.search(r'^\s*src\s*=\s*"central"', body, flags=re.M) and \
       re.search(r'^\s*dst\s*=\s*"vocal"', body, flags=re.M):
        template = body
        break
if template is None:
    sys.exit(f"no central->vocal projection in {src_path} to clone")

dst_module = opt["--dst"] or "vocal"
if opt["--dst"]:
    if not (opt["--dst-lo"] and opt["--dst-hi"]):
        sys.exit("--dst needs --dst-lo and --dst-hi")
    lo, hi = float(opt["--dst-lo"]), float(opt["--dst-hi"])
    if dst_module not in names:
        sys.exit(f"no module named {dst_module!r} in {src_path}")
else:
    lo = group / VOCAL_GROUPS
    hi = (group + 1) / VOCAL_GROUPS
dst_lo, dst_hi = (hi, lo) if reverse else (lo, hi)

out = ["", "[[projection]]"]
for line in template.split("\n"):
    m = re.match(r"^([a-z_0-9]+)\s*=\s*([^#]*)", line)
    if not m:
        continue
    k, v = m.group(1), m.group(2).strip()
    if k == "src":       v = f'"{opt["--src"]}"'
    elif k == "dst":     v = f'"{dst_module}"'
    elif k == "kind":    v = '"topographic"'
    elif k == "density": v = opt["--density"]
    elif k == "weight":  v = opt["--weight"]
    elif k == "stp_use" and opt["--stp-use"]:         v = opt["--stp-use"]
    elif k == "stp_recover_ms" and opt["--stp-recover"]: v = opt["--stp-recover"]
    elif k == "stp_facil_ms" and opt["--stp-facil"]:  v = opt["--stp-facil"]
    out.append(f"{k} = {v}")
out += [
    "topo_src_lo = 0.0",
    f'topo_src_hi = {opt["--src-hi"]}',
    f"topo_dst_lo = {dst_lo:.6f}",
    f"topo_dst_hi = {dst_hi:.6f}",
    f'topo_sigma = {opt["--sigma"]}',
]
open(dst_path, "w").write(text.rstrip("\n") + "\n" + "\n".join(out) + "\n")
print(f"wrote {dst_path}: {opt['--src']}->{dst_module} "
      f"({'reversed' if reverse else 'forward'}), "
      f"dst {dst_lo:.3f}..{dst_hi:.3f}, density {opt['--density']}, "
      f"weight {opt['--weight']}")
