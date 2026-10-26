#!/usr/bin/env python3
"""HOW FAR does the shipped delay term actually move a delay? Arithmetic, no run.

This is the explanation for the null. §3.5 sets

    m_frac = traffic / (traffic + traffic_half)
    delay  = int(birth * (1 - (1 - delay_floor_frac) * m_frac) + 0.5)

and the genome's own comment states where traffic lands: "traffic settles near
(presynaptic rate in Hz) x (tau in seconds)". With traffic_tau_ms = 10000 that is
rate x 10, against traffic_half = 50 -- so an axon must carry 5 Hz to be HALF
myelinated, and `calibrate` measures the modules free-running at 2.5-7.2 Hz.

A half-myelinated axon at delay_floor_frac = 0.5 keeps 1 - 0.5*0.5 = 75% of its birth
delay. On birth delays of 3-7 ticks at dt_ms = 1.0, that is a change of ONE TICK.
"""
import re

GENOME = "dna/default.toml"
txt = open(GENOME).read()


def f(name, default):
    m = re.search(r"^\s*%s\s*=\s*([0-9.]+)" % name, txt, re.M)
    return float(m.group(1)) if m else default


tau_s = f("traffic_tau_ms", 10000.0) / 1000.0
half = f("traffic_half", 50.0)
floor = f("delay_floor_frac", 0.5)
dt = f("dt_ms", 1.0)
span = 1.0 - floor

# Free-running rates as `calibrate` measures them on the shipped genome.
RATES = {"central": 6.60, "auditory": 2.52, "vision": 2.53,
         "somato": 4.45, "vocal": 4.12, "expression": 7.24}

print("traffic_tau %.0f s, traffic_half %.0f -> an axon is half-myelinated at %.1f Hz"
      % (tau_s, half, half / tau_s))
print("delay_floor_frac %.2f, so a FULLY myelinated axon keeps %.0f%% of its birth delay\n"
      % (floor, 100 * floor))

print("%-11s %6s %8s %9s   %s" % ("module", "Hz", "traffic", "m_frac", "birth -> moved (ticks)"))
for mod, hz in sorted(RATES.items(), key=lambda kv: -kv[1]):
    traffic = hz * tau_s
    m = traffic / (traffic + half)
    moved = []
    for birth in (3, 4, 5, 6, 7):
        d = max(1, int(birth * (1 - span * m) + 0.5))
        moved.append("%d->%d" % (birth, d))
    print("%-11s %6.2f %8.1f %9.3f   %s" % (mod, hz, traffic, m, "  ".join(moved)))

print("\nSo at the shipped operating point a typical delay moves by ONE tick, i.e. %.0f ms."
      % dt)
print("The plasticity window this has to matter against is tau_elig_ms-scale, and the")
print("vowels the creature is scored on are held for hundreds of ms. That is the null.")
print("\nFor comparison: Vassallo clamps delays to 25 timesteps and Bouhadjar sweeps")
print("dmax 15-31, both an order of magnitude more range than 3-7 with a 1-tick swing.")
