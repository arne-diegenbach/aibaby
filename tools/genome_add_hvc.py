#!/usr/bin/env python3
"""Append a chain module (an HVC) to a genome and project it into a target.

Usage:
  genome_add_hvc.py <in.toml> <out.toml> DST [key=value ...]

  genome_add_hvc.py dna/default.toml out.toml vocal out_w=0.30 out_d=0.20

Knobs (with defaults): neurons=400 group=20 chain_w=0.30 chain_d=1.0
chain_delay=8.0 out_d=0.20 out_w=0.30 out_delay=4.0 noise=0.24 threshold=0.60
target=5.0 name=hvc

WHY A SEPARATE MODULE. DNA v42's chain works inside `central` — seqprobe measures
a travelling wave whose centre climbs 107 -> 330 over 96 ms — and putting the
same chain inside `vocal` gives a reproducible utterance shape spanning only
4-6 Hz. That is structural, not a tuning failure. `vocal` is 126 neurons in nine
groups of fourteen, and a group's output is a CENTROID over its fourteen cells.
To sweep that centroid a chain would need links of three or four neurons, and a
link that small gives each target about three inputs, which cannot fire
anything. Convergence and readout resolution are irreconcilable at that size.

The songbird arrangement solves it by not putting HVC inside RA. A dedicated
nucleus is large enough for real links, and each chain position drives a
different pattern in the motor module THROUGH THE PROJECTION rather than by
sweeping a centroid — so chain length and formant resolution stop competing for
the same neurons.

The module and its projection are appended LAST, which is load-bearing for the
same reason genome_add_relay.py appends: modules are built and projections wired
in genome order off one RNG stream, so appending leaves every pre-existing
projection drawing exactly the numbers it drew before. An `out_w=0` arm is then
a real control — same wiring, chain present, chain disconnected — rather than a
different creature.

Nothing triggers the chain deliberately. The module's own noise starts it, so
each run begins where it begins; the creature's voicing onset is what trajprobe
aligns on, and if the chain drives the larynx at all then onset and chain
position align by construction.
"""
import re
import sys

src_path, dst_path, DST = sys.argv[1:4]
opt = {"neurons": "400", "group": "20", "chain_w": "0.30", "chain_d": "1.0",
       "chain_delay": "8.0", "out_d": "0.20", "out_w": "0.30", "out_delay": "4.0",
       "noise": "0.24", "threshold": "0.60", "target": "5.0", "name": "hvc",
       # A new projection into the larynx has to be PAID FOR out of the
       # larynx's own noise, or the creature simply drones: at out_w=0.30 with
       # vocal's noise left alone the duty cycle goes to 1.00. The genome says
       # this about the auditory projection in its own words -- "the two have to
       # add up to the same thing" -- and it is rule 1 of the calibration
       # invariant. 0 leaves vocal untouched, which is the arm that shows why
       # this knob exists.
       "dst_noise": "0"}
for spec in sys.argv[4:]:
    k, v = spec.split("=", 1)
    assert k in opt, f"unknown knob {k!r}; have {sorted(opt)}"
    opt[k] = v
NAME = opt["name"]

text = open(src_path).read()
assert f'name = "{DST}"' in text, f"no module named {DST} in {src_path}"
assert f'name = "{NAME}"' not in text, f"{NAME} already exists"

module = f'''
[[module]]
name = "{NAME}"
role = "association"
neurons = {opt["neurons"]}
n_max = {opt["neurons"]}
max_out_degree = 512
extent = [1.0, 1.0, 1.0]
conn_radius = 0.4
conn_density = 0.5
# The reason this module exists: a chain with links big enough to converge.
chain_weight = {opt["chain_w"]}
chain_density = {opt["chain_d"]}
chain_group = {opt["group"]}
chain_delay_ms = {opt["chain_delay"]}
threshold = {opt["threshold"]}
v_rest = 0.0
leak_tau_ms = 20.0
refractory_ms = 3.0
target_rate_hz = {opt["target"]}
inhib_fraction = 0.2
inhib_gain = 2.5
weight_init = 0.12
noise_amp = {opt["noise"]}
ip_wake_scale = 1.0
ip_sleep_scale = 1.0
syn_wake_scale = 1.0
syn_sleep_scale = 1.0
explore_scale = 0.0
norm_gain = 1.0
eta_scale = 1.0
nm_external = 1.0
nm_hunger = 1.0
nm_comfort = 1.0
nm_curiosity = 1.0
ffi_source = -1
ffi_gain = 0.0
ffi_apical = 0
ffi_learn = 0.0
apical_tau_ms = 30.0
apical_threshold = 0.0
apical_gain = 0.0
apical_plateau_ms = 0.0
theta_hz = 0.0
theta_amp = 0.0
gamma_hz = 0.0
gamma_amp = 0.0
gamma_theta_coupling = 0.0
critical_tau_ms = 0.0
critical_floor = 1.0
plateau_gate = 0.0
lateral_gain = 0.0
lateral_sigma = 0.10
lateral_fields = 1
burst_ms = 0.0
burst_refrac_scale = 1.0
elig_tau_scale = 1.0
'''

# Mirror the shape of an existing projection so every required key is present.
blocks = re.split(r'^\[\[projection\]\]', text, flags=re.M)
template = None
for b in blocks[1:]:
    if re.search(r'^\s*src\s*=\s*"central"', b, re.M) and \
       re.search(r'^\s*dst\s*=\s*"vocal"', b, re.M):
        template = b
        break
assert template is not None, "no central->vocal projection to copy the key set from"

proj = ["\n[[projection]]"]
for line in template.split("\n"):
    m = re.match(r'^([a-z_0-9]+)\s*=\s*(.*)$', line)
    if not m:
        continue
    k, v = m.group(1), m.group(2).split("#")[0].strip()
    if k == "src":
        v = f'"{NAME}"'
    elif k == "dst":
        v = f'"{DST}"'
    elif k == "density":
        v = opt["out_d"]
    elif k == "weight":
        v = opt["out_w"]
    elif k == "delay_ms":
        v = opt["out_delay"]
    proj.append(f"{k} = {v}")
proj = "\n".join(proj) + "\n"

if float(opt["dst_noise"]) > 0:
    blocks = re.split(r'(^\[\[module\]\])', text, flags=re.M)
    out = []
    for i, b in enumerate(blocks):
        if f'name = "{DST}"' in b and b.strip().startswith("name"):
            b, n = re.subn(r'^(noise_amp\s*=\s*)[-\d.]+', r'\g<1>' + opt["dst_noise"],
                           b, count=1, flags=re.M)
            assert n == 1, f"could not find {DST}'s noise_amp to rebalance"
        out.append(b)
    text = "".join(out)

open(dst_path, "w").write(text.rstrip("\n") + "\n" + module + proj)
print(f"wrote {dst_path}: {NAME} of {opt['neurons']} neurons, "
      f"{int(opt['neurons']) // int(opt['group'])} links of {opt['chain_delay']} ms "
      f"({int(opt['neurons']) // int(opt['group']) * float(opt['chain_delay']):.0f} ms end to end), "
      f"-> {DST} at d={opt['out_d']} w={opt['out_w']}")
