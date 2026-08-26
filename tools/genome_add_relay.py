#!/usr/bin/env python3
"""Insert an inhibitory relay module between two modules of a genome.

Usage:
  genome_add_relay.py <in.toml> <out.toml> SRC DST [key=value ...]

  genome_add_relay.py dna/default.toml out.toml central vocal in_w=0.20 out_w=0.10

Knobs (with defaults): neurons=128 in_d=0.04 in_w=0.20 out_d=0.03 out_w=0.10
threshold=0.60 noise=0.24 target=3.0 name=<src><dst>relay

Why this exists. `central->vocal` is an all-positive random projection and
central's object code is balanced, which is the one combination that averages a
code away. A *signed* projection preserves it, and the signed projection cortex
actually builds is balanced feedforward inhibition where **each target draws its
own independent inhibitory sample**. DnaModule::ffi_source is not that -- it
subtracts one shared scalar from every target. A relay of interneurons is: each
target cell samples a few relay cells, each of which samples the source.

The module and both projections are appended **last**, which is load-bearing.
Modules are built and projections wired in genome order off one RNG stream, so
appending leaves every pre-existing projection drawing exactly the numbers it
drew before. That is what makes an `out_w=0` arm a real control -- same wiring,
relay present, relay silent -- rather than a different creature.
"""
import re
import sys

src_path, dst_path, SRC, DST = sys.argv[1:5]
opt = {"neurons": "128", "in_d": "0.04", "in_w": "0.20", "out_d": "0.03",
       "out_w": "0.10", "threshold": "0.60", "noise": "0.24", "target": "3.0",
       "name": "", "inhib": "1.0",
       # DNA v36 on each stage, so the tool can build a Webb pair: a depressing
       # synapse into the relay and a facilitating one out of it. All zero is
       # the constant-weight relay this tool built before v36 existed.
       "in_stp_use": "0.0", "in_stp_rec": "0.0", "in_stp_fac": "0.0",
       "out_stp_use": "0.0", "out_stp_rec": "0.0", "out_stp_fac": "0.0"}
for spec in sys.argv[5:]:
    k, v = spec.split("=", 1)
    assert k in opt, f"unknown knob {k!r}; have {sorted(opt)}"
    opt[k] = v
name = opt["name"] or f"{SRC}{DST}relay"

text = open(src_path).read()
for m in (SRC, DST):
    assert re.search(rf'^name = "{re.escape(m)}"$', text, re.M), f"no module named {m!r}"
assert not re.search(rf'^name = "{re.escape(name)}"$', text, re.M), f"{name!r} already exists"

in_stp_use, in_stp_rec, in_stp_fac = opt["in_stp_use"], opt["in_stp_rec"], opt["in_stp_fac"]
out_stp_use, out_stp_rec, out_stp_fac = opt["out_stp_use"], opt["out_stp_rec"], opt["out_stp_fac"]

module = f'''
[[module]]
# An inhibitory relay: a population that exists to be SAMPLED, not to represent
# anything. Every neuron is inhibitory, so {name}->{DST} is subtractive, and
# each {DST} cell draws its own few relay cells -- which is what makes the pair
# a signed projection of {SRC} rather than one more positive sum of it.
name = "{name}"
role = "interneuron"
neurons = {opt["neurons"]}
n_max = {opt["neurons"]}     # a relay must not grow: adding cells would change
                             # how every downstream neuron samples its source,
                             # which is the property it exists to hold fixed.
                             # `growable()` refuses this role anyway; equal
                             # n_max says so in the genome as well.
max_out_degree = 512
extent = [1.0, 1.0, 1.0]
conn_radius = 0.4
conn_density = 0.0           # no recurrence: a relay that talks to itself is a
                             # dynamical system, and this one is meant to be a
                             # projection
# DNA v42, all off for the same reason conn_density is: a relay is a projection,
# not a generator. Present because every genome field is required — this tool
# has now been stale by a new mechanism's keys twice, and a genome that will not
# load is the good failure. The silent one is a key that exists with a wrong
# default.
chain_weight = 0.0
chain_density = 0.0
chain_group = 0
chain_delay_ms = 1.0
threshold = {opt["threshold"]}
v_rest = 0.0
leak_tau_ms = 5.0            # central's constant, because it reads central's
                             # volleys and should read them as coincidence for
                             # the same reason central reads the retina's that way
refractory_ms = 3.0
target_rate_hz = {opt["target"]}  # measured, not chosen -- see [homeostasis]
inhib_fraction = {opt["inhib"]}   # 1.0 is the signed-projection relay this tool
                             # was written for; 0.0 makes it an EXCITATORY relay,
                             # which is what a Webb BN1->BN2 pair needs
inhib_gain = 1.0             # 1.0, not the cortical 2.5: the scale that matters
                             # is the projection weight below, and two multipliers
                             # for one quantity is a knob nobody can read
weight_init = 0.12
noise_amp = {opt["noise"]}
ip_wake_scale = 0.25
ip_sleep_scale = 4.0
syn_wake_scale = 0.25
syn_sleep_scale = 4.0
explore_scale = 0.0
norm_gain = 0.0
eta_scale = 1.0
nm_external = 1.0
nm_hunger = 1.0
nm_comfort = 1.0
nm_curiosity = 1.0
ffi_source = -1
ffi_gain = 0.0
ffi_apical = 0               # DNA v40
ffi_learn = 0.0              # DNA v40
apical_tau_ms = 30.0
apical_threshold = 0.0
apical_gain = 0.0
apical_plateau_ms = 50.0
theta_hz = 6.0
theta_amp = 0.0
gamma_hz = 40.0
gamma_amp = 0.0
gamma_theta_coupling = 0.0
critical_tau_ms = 0.0
critical_floor = 1.0
plateau_gate = 0.0
lateral_gain = 0.0
lateral_sigma = 0.15
lateral_fields = 0
burst_ms = 0.0               # DNA v37
burst_refrac_scale = 1.0     # DNA v37
elig_tau_scale = 1.0         # DNA v39

[[projection]]
src = "{SRC}"
dst = "{name}"
kind = "random"
source = "either"
density = {opt["in_d"]}
weight = {opt["in_w"]}
delay_ms = 2.0
delay_jitter_ms = 1.0
hebb = 0.0
apical = 0
exuberance = 1.0
birth_weight = 1.0
stp_use = {in_stp_use}       # DNA v36: the FIRST stage of a Webb pair
stp_recover_ms = {in_stp_rec}
stp_facil_ms = {in_stp_fac}
burst_learn = 0.0            # DNA v37

[[projection]]
src = "{name}"
dst = "{DST}"
kind = "random"
source = "either"
density = {opt["out_d"]}
weight = {opt["out_w"]}
delay_ms = 2.0
delay_jitter_ms = 1.0
hebb = 0.0
apical = 0
stp_use = {out_stp_use}      # DNA v36: the SECOND stage
stp_recover_ms = {out_stp_rec}
stp_facil_ms = {out_stp_fac}
burst_learn = 0.0            # DNA v37
exuberance = 1.0
birth_weight = 1.0
'''

open(dst_path, "w").write(text.rstrip("\n") + "\n" + module)
