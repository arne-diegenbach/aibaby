#!/usr/bin/env python3
"""Four-word conditional naming, gated against ungated, PAIRED per creature.

WHY PAIRING IS LEGITIMATE HERE AND WAS NOT FOR THE TRIM. Gating changes no wiring: the same
genome and the same seed give the SAME CREATURE in both runs, differing only in whether it is
frozen after tick 1700. So creature r in one run and creature r in the other are matched
units. The trim comparison could not do this -- deleting modules re-rolled every later random
draw, so the two genomes were different creatures and the test had to be unpaired.

The per-creature quantity is the experiment's own: `nearest` in the oracle arm minus `nearest`
in the off arm, which is the paired excess the experiment reports pooled. The cross-run
statistic is the difference of those, jackknifed over creatures.
"""
import math
import re
import sys


def read(path):
    """seed -> {arm: nearest}. The `nearest` column is the last field."""
    out = {}
    for line in open(path):
        m = re.match(r"\s+(\d+)\s+(off|oracle)\s+(.*)$", line)
        if not m:
            continue
        seed, arm = int(m.group(1)), m.group(2)
        out.setdefault(seed, {})[arm] = float(m.group(3).split()[-1])
    return {s: v for s, v in out.items() if "off" in v and "oracle" in v}


def jackknife(vals):
    """Delete-one jackknife SE of the mean."""
    n = len(vals)
    if n < 3:
        return 0.0, 0.0
    tot = sum(vals)
    thetas = [(tot - v) / (n - 1) for v in vals]
    mbar = sum(thetas) / n
    var = sum((t - mbar) ** 2 for t in thetas) * (n - 1) / n
    return tot / n, math.sqrt(var)


g = read("results/ctxgrid-trim-gated.log")
u = read("results/ctxgrid-trim.log")
shared = sorted(set(g) & set(u))
print("creatures: gated %d, ungated %d, matched %d" % (len(g), len(u), len(shared)))

# Per creature, the experiment's own excess, then the cross-run difference.
exc_g = [g[s]["oracle"] - g[s]["off"] for s in shared]
exc_u = [u[s]["oracle"] - u[s]["off"] for s in shared]
d = [a - b for a, b in zip(exc_g, exc_u)]

for name, v in (("ungated excess", exc_u), ("gated excess", exc_g)):
    m, se = jackknife(v)
    print("  %-16s %+.4f +/- %.4f  (%.1f SE)" % (name, m, se, abs(m) / se if se else 0))

m, se = jackknife(d)
nse = m / se if se else 0.0
print("\nTHE PRIMARY, paired cross-run (gated minus ungated): %+.4f +/- %.4f (%.1f SE)"
      % (m, se, abs(nse)))
lo, hi = m - 2 * se, m + 2 * se
print("  2 SE interval %+.4f to %+.4f" % (lo, hi))
mu, _ = jackknife(exc_u)
print("  as a share of the ungated effect (%.4f): %.0f%% lost at the point estimate"
      % (mu, 100.0 * -m / mu if mu else 0.0))
pos = sum(1 for x in d if x > 0)
print("  sign: %d of %d creatures higher when gated" % (pos, len(d)))
print("\nVERDICT: %s" % ("a RESOLVED LOSS -- gating costs conditional naming" if nse <= -3
                         else "a RESOLVED GAIN" if nse >= 3
                         else "under 3 SE: no resolved difference, report as a BOUND"))
