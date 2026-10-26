#!/usr/bin/env python3
"""How many distinct delay values can the shipped creature ever hold?

Pure arithmetic from `dna/default.toml` plus two kernel rules, so it needs no run:

    dt_ms = 1.0, so one tick is one millisecond
    delay_ticks(ms)  = clamp(int(ms + 0.5), 1, max_delay_ticks - 1)    network.cpp:928
    myelinate()      = int(birth * (1 - span*m_frac) + 0.5), span = 1 - delay_floor_frac
                       with m_frac = traffic/(traffic+half) in [0,1)   network.cpp:3240

The point of the count: `myelinate()`'s delay term is the one shipped mechanism in this
creature with no measurement against it, and the delay-learning literature it belongs
to (Vassallo & Taherinejad 2026; Bouhadjar & Neftci 2026) operates at dmax 15-31. If
the shipped genome only ever occupies a handful of integers, the axis is
quantisation-bound before any learning rule is argued about.
"""
import re
import sys

GENOME = sys.argv[1] if len(sys.argv) > 1 else "dna/default.toml"
txt = open(GENOME).read()


def field(name, default):
    m = re.search(r"^\s*%s\s*=\s*([0-9.]+)" % name, txt, re.M)
    return float(m.group(1)) if m else default


dt_ms = field("dt_ms", 1.0)
max_slots = int(field("max_delay_ticks", 32))
floor_frac = field("delay_floor_frac", 0.5)
span = 1.0 - floor_frac

pairs, cur = [], None
for line in txt.splitlines():
    m = re.match(r"\s*delay_ms\s*=\s*([0-9.]+)", line)
    if m:
        cur = float(m.group(1))
        continue
    j = re.match(r"\s*delay_jitter_ms\s*=\s*([0-9.]+)", line)
    if j and cur is not None:
        pairs.append((cur, float(j.group(1))))
        cur = None


def ticks(ms):
    return max(1, min(max_slots - 1, int(ms / dt_ms + 0.5)))


print("%s: dt_ms %.1f, max_delay_ticks %d, delay_floor_frac %.2f"
      % (GENOME, dt_ms, max_slots, floor_frac))
print("%d projections carry an explicit delay_ms/jitter pair\n" % len(pairs))
print("%-9s %-7s %-22s %-12s" % ("delay_ms", "jitter", "birth (ticks)", "myelinated"))
seen = set()
for d, j in sorted(set(pairs)):
    births = sorted({ticks(d + x) for x in (-j, -j / 2, 0.0, j / 2, j)})
    # m_frac asymptotes to 1 but never reaches it; 0.999 is the practical limit.
    mye = sorted({max(1, int(b * (1 - span * 0.999) + 0.5)) for b in births})
    seen.update(births)
    seen.update(mye)
    print("%-9.1f %-7.1f %-22s %-12s" % (d, j, births, mye))

vals = sorted(seen)
print("\nevery delay value this genome can ever hold: %s" % vals)
print("%d distinct integers, against %d provisioned in the ring buffer"
      % (len(vals), max_slots))
print("the literature's range: Vassallo clamps to 25 timesteps; Bouhadjar sweeps "
      "dmax 15 and 31.")
