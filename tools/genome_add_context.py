#!/usr/bin/env python3
"""Append a zero-baseline context module to a genome and project it into a target.

Usage:
  genome_add_context.py <in.toml> <out.toml> DST [key=value ...]

  genome_add_context.py dna/default.toml out.toml vocal out_w=0.30 slot=32

Knobs (with defaults): slots=2 slot=32 out_d=0.50 out_w=0.30 out_delay=4.0
threshold=1.0 name=context dst_noise=0

WHAT THIS IS. A population that is **silent unless the host writes into it**:
`noise_amp`, `target_rate_hz` and all four ip/syn homeostasis scales are zero,
and it has no intra-module wiring at all, so its activity is exactly what was
injected and nothing else. It is cut into `slots` disjoint slices of `slot`
neurons, one per condition, the way `somato` is cut into one slice per caregiver
action -- and like a touch, whoever runs the creature says which slice is on.

WHY IT EXISTS, and it is the one variable this project never varied. Every
conditional-learning failure here has the same arithmetic, and the
synaptic-perturbation post-mortem writes it out: the presynaptic gate is a spike
count, a spike count is a neuron's baseline rate plus a few percent of
condition, so the credit factorises into a shared term and a differential one
and the shared term is far larger. That shared term has been subtracted (v24),
signed (v35), gated (v29, v37, v40), re-timed (v26, v42), re-routed (v43, v46)
and re-rewarded (v20) -- seven common-mode walls. Its CAUSE has never been
touched: requirements 3.1 makes rate homeostasis mandatory and drives every
neuron to a DNA target rate, so no population in this creature has ever had a
baseline of zero.

Two conditions delivered on disjoint zero-baseline slices share no presynaptic
unit and therefore no synapse. R-STDP's `trace_pre_` is exactly zero on every
inactive unit, so there is no common mode left to factor out and no
interference between the two lessons to remove. This is the HVC property the
birdsong rule was ported without: `genome_add_hvc.py` builds a chain module
driven by its own noise, which has a baseline like everything else here.

THE CONTROL IS THAT THE ORACLE STAYS SILENT -- within this genome, never
against the shipped one. The module and its projection are appended LAST, so
every pre-existing projection draws exactly the numbers it drew before and the
WIRING is untouched.

**The wiring is not the whole creature, and this tool measured it.** Noise is
`rng_->signed_uniform()` once per neuron per tick, over all modules in order, so
64 extra neurons advance the stream by 64 draws every tick and every existing
neuron gets a different noise realisation from tick two onward. `calibrate` on
the appended genome reads central 6.60 -> 6.76, somato 4.45 -> 4.16 (STALE) and
vocal 4.12 -> 3.85, and reads **exactly the same numbers at out_w = 0.0 as at
0.30** -- so the shift is the noise stream and not the tract, and not synaptic
scaling paying for the new input weight either.

So an appended-module genome is a different noise realisation of the same seed
family, and the only sound comparison is BETWEEN ARMS THAT SHARE IT: oracle
driven against oracle silent, or out_w swept, on one genome. That is what
genome_add_relay.py and genome_add_hvc.py actually do, so no result of theirs is
affected -- but both docstrings say "bit-identical", and for the simulation
stream that is not true. Do not quote an appended genome's number against one
measured on dna/default.toml.
"""
import re
import sys

src_path, dst_path, DST = sys.argv[1:4]
# out_w defaults LOW and that is measured, not cautious. `ctxlearn`'s own gain
# sweep reads the echo window's voiced fraction against an oracle-silent
# control of 0.84: at out_w 0.30 a 31 Hz oracle takes it to 0.38, at 0.03 to
# 0.65. The tract is what learning is supposed to GROW -- w_max is 0.50, so
# 0.03 leaves 16x of headroom -- and starting it where it already dominates the
# larynx is rule 1 of the calibration invariant arriving as a null.
opt = {"slots": "2", "slot": "32", "out_d": "0.50", "out_w": "0.03",
       "out_delay": "4.0", "threshold": "1.0", "name": "context",
       # A new projection into the larynx has to be PAID FOR out of the larynx's
       # own noise or the creature drones -- rule 1 of the calibration
       # invariant, and the genome says it about the auditory projection in its
       # own words. 0 leaves the target untouched, which is the arm that shows
       # why the knob exists.
       "dst_noise": "0"}
for spec in sys.argv[4:]:
    k, v = spec.split("=", 1)
    assert k in opt, f"unknown knob {k!r}; have {sorted(opt)}"
    opt[k] = v
NAME = opt["name"]
neurons = int(opt["slots"]) * int(opt["slot"])

text = open(src_path).read()
assert f'name = "{DST}"' in text, f"no module named {DST} in {src_path}"
assert f'name = "{NAME}"' not in text, f"{NAME} already exists"

module = f'''
[[module]]
# A condition, written from outside. Silent unless the host injects into one of
# its {opt["slots"]} slices of {opt["slot"]}; no noise, no homeostasis, no recurrence, so
# `trace_pre_` on its outgoing synapses is exactly zero for every unit whose
# condition is absent. That is the property the whole exercise is about.
name = "{NAME}"
role = "context"
neurons = {neurons}
n_max = {neurons}
max_out_degree = 512
extent = [1.0, 1.0, 1.0]
# No intra-module wiring. A context module represents what it was told and must
# not spread it: recurrence here would mix the slices, which is the one thing
# the slices exist to prevent. The radius is a hair rather than zero because the
# core rejects a zero radius outright; at 0.001 in a unit volume no pair is ever
# in range, and the density is zero as well.
conn_radius = 0.001
conn_density = 0.0
chain_weight = 0.0
chain_density = 0.0
chain_group = 0
chain_delay_ms = 0.0
threshold = {opt["threshold"]}
v_rest = 0.0
leak_tau_ms = 20.0
refractory_ms = 3.0
# Zero, and every one of these is load-bearing. A homeostatic setpoint is a
# guarantee of a non-zero baseline, which is exactly what this module exists not
# to have; intrinsic plasticity would raise excitability until the silent
# slices fired on their own, and synaptic scaling would equalise what the
# active slice delivers against what the silent ones do not.
target_rate_hz = 0.0
noise_amp = 0.0
ip_wake_scale = 0.0
ip_sleep_scale = 0.0
syn_wake_scale = 0.0
syn_sleep_scale = 0.0
# All excitatory: a slice must mean "this condition is present", and an
# inhibitory unit inside it would mean the opposite for half the target.
inhib_fraction = 0.0
inhib_gain = 2.5
weight_init = 0.12
explore_scale = 0.0
norm_gain = 0.0
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
rebound_source = -1
rebound_gain = 0.0
rebound_mean_tau_ms = 0.0
'''

# Mirror the shape of an existing projection into the same target, so every
# required key is present and only the ones this tool owns differ.
blocks = re.split(r'^\[\[projection\]\]', text, flags=re.M)
template = None
for b in blocks[1:]:
    if re.search(rf'^\s*dst\s*=\s*"{re.escape(DST)}"', b, re.M):
        template = b
        break
assert template is not None, f"no projection into {DST} to copy the key set from"

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
    elif k == "rule":
        v = '"random"'
    proj.append(f"{k} = {v}")
proj = "\n".join(proj) + "\n"

if float(opt["dst_noise"]) > 0:
    blocks = re.split(r'(^\[\[module\]\])', text, flags=re.M)
    out = []
    for b in blocks:
        if f'name = "{DST}"' in b and b.strip().startswith("name"):
            b, n = re.subn(r'^(noise_amp\s*=\s*)[-\d.]+', r'\g<1>' + opt["dst_noise"],
                           b, count=1, flags=re.M)
            assert n == 1, f"could not find {DST}'s noise_amp to rebalance"
        out.append(b)
    text = "".join(out)

open(dst_path, "w").write(text.rstrip("\n") + "\n" + module + proj)
print(f"wrote {dst_path}: {NAME}, {opt['slots']} slices of {opt['slot']} "
      f"({neurons} neurons, zero baseline) -> {DST} at d={opt['out_d']} "
      f"w={opt['out_w']}")
