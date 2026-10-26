#!/usr/bin/env python3
"""Match the operating point of a heterogeneous creature to the homogeneous one.

WHY THIS IS NECESSARY. A log-normal spread on `threshold` keeps the MEDIAN at the genome
value, but the drive-to-rate transfer is CONVEX: the neurons whose thresholds fell fire
far more than the ones whose rose fire less, and recurrent excitation amplifies the
asymmetry. Measured, with the wiring held identical:

    sigma    0.00   0.02   0.05   0.10   0.20   0.30
    central  6.60   6.63   6.75   7.08   8.29  10.17
    express  7.24   7.31   7.74   8.35  10.25  13.02

Monotone in every module. `calibrate` already FAILS at sigma 0.04, so above ~0.03 the
arms do not share an operating point and no outcome comparison between them means
anything (the calibration invariant).

WHAT THIS DOES. Raises each module's MEDIAN threshold until its free-running rate returns
to the genome target, leaving the spread intact. Then the heterogeneous arm and the
control differ in the SPREAD and not in the achieved rate, which is the one variable the
experiment is about.

Proportional feedback on all six modules at once, because `calibrate` reports every
module's error in a single run -- a per-module bisection would cost six times as much for
the same information. Moving `threshold` does NOT touch the divisive-norm setpoint
(`norm_target_[m] = target_rate_hz`), so unlike a target_rate_hz edit this is not a fixed
point and converges in one direction.
"""
import re
import subprocess
import sys

SIGMA = float(sys.argv[1]) if len(sys.argv) > 1 else 0.30
FIELD = sys.argv[2] if len(sys.argv) > 2 else "threshold_sigma"
DST = sys.argv[3] if len(sys.argv) > 3 else "results/het-retuned.toml"
ITERS = 14
GAIN = 0.06          # threshold units per Hz of error; small enough not to oscillate
# `vocal` is off target BY DESIGN ("holds the duty cycle down"), so it is not retuned.
SKIP = {"vocal"}

src = open("dna/default.toml").read().split("\n")

# Module blocks: the name line, then the threshold line that belongs to it.
mods = []           # (module name, index of its threshold line)
cur = None
for i, line in enumerate(src):
    m = re.match(r'^name\s*=\s*"([a-z]+)"', line)
    if m:
        cur = m.group(1)
    if re.match(r"^threshold\s*=", line) and cur:
        mods.append((cur, i))
        cur = None
print("retuning %d module thresholds at %s = %.2f\n" % (len(mods), FIELD, SIGMA))

thr = {}
for name, idx in mods:
    thr[name] = float(re.match(r"^threshold\s*=\s*([0-9.]+)", src[idx]).group(1))


def write(path):
    out = []
    for i, line in enumerate(src):
        hit = next((n for n, j in mods if j == i), None)
        out.append("threshold = %.5f" % thr[hit] if hit else line)
        if re.match(r"^leak_tau_ms\s*=", line):
            out.append("%s = %.4f" % (FIELD, SIGMA))
    open(path, "w").write("\n".join(out))


def measure(path):
    r = subprocess.run(["./build/aibaby", "--dna", path, "--experiment", "calibrate"],
                       capture_output=True, text=True)
    got = {}
    for line in r.stdout.split("\n"):
        m = re.match(r"\s+([a-z]+)\s+([0-9.]+) Hz\s+([0-9.]+) Hz", line)
        if m:
            got[m.group(1)] = (float(m.group(2)), float(m.group(3)))
    ok = "calibrate PASS" in r.stdout
    return got, ok


for it in range(ITERS):
    write(DST)
    got, ok = measure(DST)
    if not got:
        sys.exit("calibrate produced no module rows -- genome rejected?")
    worst = max((abs(v[0] - v[1]), n) for n, v in got.items() if n not in SKIP)
    print("  iter %2d  worst |err| %.3f Hz on %-11s %s"
          % (it, worst[0], worst[1], "calibrate PASS" if ok else ""))
    if ok:
        print("\nCONVERGED at iteration %d. Retuned thresholds:" % it)
        for n, _ in mods:
            print("    %-11s %.5f%s" % (n, thr[n], "   (not retuned, off by design)"
                                        if n in SKIP else ""))
        print("\nwrote %s" % DST)
        sys.exit(0)
    for n, (free, target) in got.items():
        if n in SKIP:
            continue
        thr[n] = max(0.05, thr[n] + GAIN * (free - target))

print("\nDID NOT CONVERGE in %d iterations -- report this rather than using the genome."
      % ITERS)
sys.exit(1)
