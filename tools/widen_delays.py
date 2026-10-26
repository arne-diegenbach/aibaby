#!/usr/bin/env python3
"""Widen the creature's delay axis to the range the delay literature operates in.

WHY. The shipped genome's projection delays are 3-6 ms at dt_ms = 1.0, so the whole
delay axis is the six integers 2-7 ticks against `max_delay_ticks = 32` provisioned, and
§3.5 moves a typical delay by ONE tick. That measured null (g2 primary -0.0019 +/-
0.0041 at n=36) is explained by the range, not by delays being useless: Vassallo clamps
to 25 timesteps and Bouhadjar sweeps dmax 15-31.

WHAT IS SCALED. Only `delay_ms` and `delay_jitter_ms` inside projection blocks -- the
inter-module axons myelination acts on. `chain_delay_ms` is left alone deliberately: it
sets the synfire chain's STEP, so scaling it changes that mechanism's function rather
than its resolution, and the chain line is already refuted here. `self_delay_ms` is 0.

WHAT THIS IS NOT. It is not a free parameter flip. Conduction latency changes everywhere,
so the result is a DIFFERENT CREATURE: the calibration invariant has to be re-checked and
a vacuity pre-flight run before any comparison is believed.
"""
import re
import sys

SCALE = float(sys.argv[1]) if len(sys.argv) > 1 else 4.0
SRC = sys.argv[2] if len(sys.argv) > 2 else "dna/default.toml"
DST = sys.argv[3] if len(sys.argv) > 3 else "results/delay-wide.toml"
MAX_SLOTS = 32

out, changed, over = [], 0, []
for line in open(SRC):
    m = re.match(r"^(delay_ms|delay_jitter_ms)(\s*=\s*)([0-9.]+)(.*)$", line)
    if m:
        key, eq, val, rest = m.groups()
        new = float(val) * SCALE
        # delay_ticks() clamps at delay_slots_ - 1; a delay that would clamp is a
        # silent change of meaning, so refuse rather than scale it.
        if key == "delay_ms" and new > MAX_SLOTS - 2:
            over.append((val, new))
        out.append("%s%s%.1f%s\n" % (key, eq, new, rest))
        changed += 1
    else:
        out.append(line)

if over:
    sys.exit("REFUSED: %d delays would exceed max_delay_ticks-1 = %d: %s"
             % (len(over), MAX_SLOTS - 1, over))

open(DST, "w").writelines(out)
print("scaled %d delay fields by %.1fx: %s -> %s" % (changed, SCALE, SRC, DST))
