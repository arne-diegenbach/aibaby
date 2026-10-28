# IS echoname's ECHO EXCESS A SELECTION EFFECT ON WHICH TRIALS GOT SCORED?
# Written 2026-10-28 against an ALREADY-PUBLISHED log -- no new run.
#
# THE WORRY. Fatigue accumulates with global spiking (brain.cpp:567) and sleep is
# its only discharge; an asleep echo trial is SKIPPED, not scored. The rewarded
# arms scored 159 and 162 of 200 against 190 and 200 elsewhere. An asleep trial's
# window is quiescent, so its row is uninformative, and dropping uninformative
# rows RAISES held-out accuracy. That has the right sign to manufacture the
# whole +0.0673, and across arms sleep is as rank-correlated with the echo as
# the voiced fraction was -- which quietecho then refused.
#
# FOUR TESTS, none of which costs a run:
#   1  within one arm, over its creatures: does the echo rise as trials fall?
#      Selection says the correlation is NEGATIVE.
#   2  the paired contrast on the creatures that barely slept, with the subset
#      defined on BOTH members (aibaby-filter-both-sides-of-a-ratio).
#   3  the artefact's own regression: paired echo diff ~ a + b * paired trials
#      diff. Selection says b < 0, and the INTERCEPT a is the excess
#      extrapolated to a ZERO sleep gap -- the selection-corrected estimate.
#   4  b_required = raw excess / mean trials gap, against b measured.
#
# usage: sleep_selection.py LOG [column]      column defaults to "echo";
#        pass "artic" to run the whole thing on the articulator column, which
#        drops voicing and loudness and so is free of the feature-10 worry.
import sys, math

path = sys.argv[1]
col = sys.argv[2] if len(sys.argv) > 2 else "echo"
ARMS = ("fresh", "none", "taught", "nodrive", "yoked")

# Several column headers are two words ("its EAR", "aud WHILE"), and each one
# would shift every index after it by one against the data rows, which are
# single-token throughout. Fold them before splitting. Data lines never contain
# these phrases, so folding them too is a no-op.
def fold(s):
    for a, b in (("its EAR", "its_EAR"), ("aud WHILE", "aud_WHILE"),
                 ("its null", "its_null"), ("F1 spread", "F1_spread"),
                 ("scored/200", "scored")):
        s = s.replace(a, b)
    return s

# Find the per-seed table's header so the column layout is read, not assumed:
# `artic` was inserted at field 4 and shifts everything after it.
idx = None
for line in open(path):
    f = fold(line).split()
    if len(f) > 4 and f[0] == "seed" and f[1] == "arm":
        # "its EAR", "aud WHILE" and "F1 spread" are two words each; only the
        # three single-word headers this script needs have to resolve.
        idx = {name: i for i, name in enumerate(f)}
        break
if idx is None or col not in idx or "trials" not in idx:
    sys.exit(f"could not locate columns {col!r} and 'trials' in the header of {path}")
c_val, c_trials = idx[col], idx["trials"]

by = {}
for line in open(path):
    f = fold(line).split()
    if len(f) > max(c_val, c_trials) and f[1] in ARMS and f[0].isdigit():
        try:
            by.setdefault(f[1], {})[int(f[0])] = (float(f[c_val]), int(f[c_trials]))
        except ValueError:
            pass
seeds = sorted(by["none"])
print(f"{path}\ncolumn {col!r} at field {c_val + 1}, 'trials' at field {c_trials + 1}, "
      f"{len(seeds)} creatures per arm\n")

def mean_se(v):
    n = len(v); m = sum(v) / n
    if n < 2: return m, float("nan")
    sd = math.sqrt(sum((x - m) ** 2 for x in v) / (n - 1))
    return m, sd / math.sqrt(n)

def regress(x, y):
    n = len(y); mx, my = sum(x) / n, sum(y) / n
    sxx = sum((v - mx) ** 2 for v in x)
    b = sum((v - mx) * (c - my) for v, c in zip(x, y)) / sxx
    a = my - b * mx
    s2 = sum((c - (a + b * v)) ** 2 for v, c in zip(x, y)) / (n - 2)
    return a, math.sqrt(s2 * (1.0 / n + mx * mx / sxx)), b, math.sqrt(s2 / sxx), mx, my

print(f"TEST 1 -- within each arm, over its creatures. selection says NEGATIVE.")
print(f"{'arm':9} {'trials min/mean/max':22} {'corr('+col+',trials)':20} {'r/SE':>5}")
for a in ARMS:
    v = [by[a][s][0] for s in seeds]
    t = [float(by[a][s][1]) for s in seeds]
    rng = f"{min(t):.0f} / {sum(t)/len(t):.1f} / {max(t):.0f}"
    if max(t) == min(t):
        print(f"{a:9} {rng:22} {'constant trials':20}     --")
        continue
    n = len(v); mv, mt = sum(v) / n, sum(t) / n
    svt = sum((p - mv) * (q - mt) for p, q in zip(v, t))
    r = svt / math.sqrt(sum((p - mv) ** 2 for p in v) * sum((q - mt) ** 2 for q in t))
    se = (1 - r * r) / math.sqrt(n - 3)
    print(f"{a:9} {rng:22} {r:+20.3f} {abs(r)/se:5.1f}")

print("\nTEST 2 -- the paired contrast on creatures that barely slept (BOTH arms)")
for hi, lo in (("yoked", "none"), ("taught", "none")):
    print(f"\n  {hi} - {lo}")
    print(f"  {'both scored >=':16} {'n':>3} {'diff':>10} {'SE':>7} {'SE u':>5} {'trials gap':>11}")
    for cut in (0, 140, 160, 170, 180):
        sel = [s for s in seeds if by[hi][s][1] >= cut and by[lo][s][1] >= cut]
        if len(sel) < 4: continue
        m, se = mean_se([by[hi][s][0] - by[lo][s][0] for s in sel])
        gm, _ = mean_se([by[hi][s][1] - by[lo][s][1] for s in sel])
        lab = "all creatures" if cut == 0 else f"{cut}/200"
        print(f"  {lab:16} {len(sel):3d} {m:+10.4f} {se:7.4f} {abs(m)/se:5.1f} {gm:+11.1f}")

print("\nTESTS 3 and 4 -- the artefact's own regression, all creatures")
print(f"  {'contrast':16} {'b required':>11} {'b measured':>11} {'SE':>9} {'apart':>9}"
      f" {'corrected excess':>20} {'2-SE share':>11}")
for hi, lo in (("yoked", "none"), ("taught", "none")):
    y = [by[hi][s][0] - by[lo][s][0] for s in seeds]
    x = [float(by[hi][s][1] - by[lo][s][1]) for s in seeds]
    a, se_a, b, se_b, mx, my = regress(x, y)
    need = my / mx
    print(f"  {hi+' - '+lo:16} {need:+11.6f} {b:+11.6f} {se_b:9.6f} "
          f"{abs(b - need)/se_b:6.1f} SE {a:+9.4f} +-{se_a:.4f} {100*(a - 2*se_a)/my:10.0f}%")
    print(f"  {'':16} trials gap {min(x):+.0f} to {max(x):+.0f}, mean {mx:+.1f}"
          f"   raw excess {my:+.4f}")
