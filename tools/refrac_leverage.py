#!/usr/bin/env python3
"""How much leverage does `refractory_ms` have on this creature's operating point?

Parses `calibrate` logs from two sweeps and reports the three numbers that decide whether
a per-neuron spread on this constant can deliver a dose comparable to DNA v71's.

    refrac_leverage.py uniform <log>   one log per refractory_ms, uniform across modules
    refrac_leverage.py sigma   <log>   one log per refractory_sigma, thresholds untouched

THE UNIFORM SWEEP gives the elasticity `d log rate / d log refractory`, fitted across the
whole ladder rather than between its endpoints -- the three-readings-of-one-curve error
`stageprobe` records, where one exponent fitted first-rung-to-last INVERTED the answer a
per-interval fit gave. If the elasticity is small, a spread in the constant is a much
smaller spread in rate, and rate is the quantity v71's 61% was delivered in.

THE SIGMA SWEEP asks the same question `calibrate` already answered for the threshold,
which FAILS at threshold_sigma 0.04. That makes it a sensitive, pre-calibrated detector of
a spread that has moved the operating point, and the sigma at which each constant breaks it
is a like-for-like dose comparison on one instrument.

THE MARGIN is why the elasticity comes out where it does. A refractory period of R ms caps
a neuron at 1000/R Hz. The creature runs at 4-7 Hz. A constant whose ceiling sits ~50x
above the operating point is not a constant the creature operates near, and the share of
input it controls is `rate * R` -- about 1.5% of ticks.
"""
import math
import re
import sys

MODE, LOG = sys.argv[1], sys.argv[2]
# `vocal` is off target by design and `auditory`/`vision` are driven by the retina rather
# than by themselves, so their free-running rate says little about their own constants.
SELF_DRIVEN = ("central", "somato", "expression")

blocks = []  # (knob value, {module: (free, target)}, passed)
cur_val, cur = None, {}
passed = None
for line in open(LOG):
    m = re.match(r"^#+\s*refractory_(?:ms|sigma)\s*=\s*([0-9.]+)", line)
    if m:
        if cur_val is not None:
            blocks.append((cur_val, cur, passed))
        cur_val, cur, passed = float(m.group(1)), {}, None
        continue
    m = re.match(r"\s+([a-z]+)\s+([0-9.]+) Hz\s+([0-9.]+) Hz", line)
    if m and cur_val is not None:
        cur[m.group(1)] = (float(m.group(2)), float(m.group(3)))
    if "calibrate PASS" in line:
        passed = True
    elif "calibrate FAIL" in line:
        passed = False
if cur_val is not None:
    blocks.append((cur_val, cur, passed))

if not blocks:
    sys.exit("no blocks parsed from %s -- expected '#### refractory_ms = X ####' headers" % LOG)

knob = "refractory_ms" if MODE == "uniform" else "refractory_sigma"
print("%s sweep, %d rungs from %s\n" % (knob, len(blocks), LOG))
mods = [m for m in SELF_DRIVEN if m in blocks[0][1]]
print("%-10s %s %s" % (knob, " ".join("%11s" % m for m in mods), "  calibrate"))
print("-" * (11 + 12 * len(mods) + 12))
for val, got, ok in blocks:
    cells = " ".join("%8.2f Hz" % got[m][0] if m in got else "%11s" % "-" for m in mods)
    print("%-10.2f %s   %s" % (val, cells, "PASS" if ok else "FAIL" if ok is False else "?"))

if MODE == "uniform":
    print("\nelasticity d log rate / d log refractory, least squares over the whole ladder")
    print("-" * 72)
    for m in mods + (["vocal"] if "vocal" in blocks[0][1] else []):
        xs = [math.log(v) for v, g, _ in blocks if m in g]
        ys = [math.log(g[m][0]) for v, g, _ in blocks if m in g]
        n = len(xs)
        mx, my = sum(xs) / n, sum(ys) / n
        sxy = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
        sxx = sum((x - mx) ** 2 for x in xs)
        slope = sxy / sxx if sxx else 0.0
        # Monotonicity matters more than the fit: a non-monotone ladder means the rungs are
        # reading seed noise rather than the constant.
        seq = [g[m][0] for v, g, _ in blocks if m in g]
        mono = all(b <= a for a, b in zip(seq, seq[1:])) or all(b >= a for a, b in zip(seq, seq[1:]))
        print("  %-11s %+.4f   %s" % (m, slope, "monotone" if mono else "NOT monotone"))

    print("\nhow far the creature sits from the ceiling this constant sets")
    print("-" * 72)
    base = next((g for v, g, _ in blocks if abs(v - 3.0) < 1e-9), None)
    if base:
        ceiling = 1000.0 / 3.0
        print("  refractory_ms 3.0 caps a neuron at %.1f Hz" % ceiling)
        for m in sorted(base, key=lambda k: -base[k][0]):
            rate = base[m][0]
            print("  %-11s %5.2f Hz   %4.0fx below the ceiling   %4.2f%% of ticks blacked out"
                  % (m, rate, ceiling / rate, 100.0 * rate * 3.0 / 1000.0))
else:
    broke = [v for v, g, ok in blocks if ok is False]
    held = [v for v, g, ok in blocks if ok is True]
    print("\ndose, against the same instrument's reading for the threshold")
    print("-" * 72)
    print("  threshold_sigma   breaks calibrate at 0.04   (DNA v71, recorded)")
    if broke:
        print("  refractory_sigma  holds to %.2f, breaks at %.2f" % (max(held), min(broke)))
        print("  ratio in sigma    >= %.0fx" % (min(broke) / 0.04))
    else:
        print("  refractory_sigma  holds at every rung tested, to %.2f" % max(held))
        print("  ratio in sigma    >= %.0fx" % (max(held) / 0.04))
    print("\n  EQUAL SIGMA IS NOT EQUAL DOSE. A lesson cost measured at matched sigma is not")
    print("  like-for-like against the threshold's 61%, and a null there would mean only")
    print("  that a much weaker perturbation did less.")
