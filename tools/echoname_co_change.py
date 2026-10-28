# WHAT TRAVELS WITH THE EXPANSION? Free, on the archived per-seed columns.
#
# Reward differentiates the four heard words 3.0x beyond any rescaling. The next
# question is what else moves in the creatures where it moves most -- a map of
# co-variation to aim the next build at, not a causal claim.
#
# Paired yoked - none per creature, because the yoke learned nothing, so every
# change in it is reward's doing and none of it is the lesson's.
import sys, math
ARMS = ("fresh", "none", "taught", "nodrive", "yoked")
def fold(s):
    for a, b in (("its EAR", "its_EAR"), ("aud WHILE", "aud_WHILE"),
                 ("its null", "its_null"), ("F1 spread", "F1_spread")):
        s = s.replace(a, b)
    return s
path = sys.argv[1]
idx = None
for line in open(path):
    f = fold(line).split()
    if len(f) > 4 and f[0] == "seed" and f[1] == "arm":
        idx = {n: i for i, n in enumerate(f)}; break
COLS = ["artic", "its_EAR", "voiced", "F1_spread", "trials", "aud_WHILE",
        "drive", "within", "between", "sep"]
LOGS = {"within", "between", "sep", "drive"}   # variances/levels: log scale
by = {}
for line in open(path):
    f = fold(line).split()
    if len(f) >= 16 and f[0].isdigit() and f[1] in ARMS:
        by.setdefault(f[1], {})[int(f[0])] = {c: float(f[idx[c]]) for c in COLS}
seeds = sorted(by["none"])

def delta(c, s, hi="yoked", lo="none"):
    a, b = by[hi][s][c], by[lo][s][c]
    if c in LOGS:
        if a <= 0 or b <= 0: return None
        return math.log(a / b)
    return a - b

d = {c: [] for c in COLS}
use = []
for s in seeds:
    vals = {c: delta(c, s) for c in COLS}
    if any(v is None for v in vals.values()):
        continue
    use.append(s)
    for c in COLS:
        d[c].append(vals[c])
print(f"paired yoked - none on {len(use)} creatures "
      f"(log scale for drive/within/between/sep)\n")

def corr(x, y):
    n = len(x); mx, my = sum(x)/n, sum(y)/n
    sxx = sum((a-mx)**2 for a in x); syy = sum((b-my)**2 for b in y)
    if sxx <= 0 or syy <= 0: return 0.0, 0.0
    r = sum((a-mx)*(b-my) for a, b in zip(x, y)) / math.sqrt(sxx*syy)
    return r, abs(r) / ((1 - r*r) / math.sqrt(n - 3)) if abs(r) < 1 else 99.0

print("  correlation of each change with d log BETWEEN -- the result itself")
for c in COLS:
    if c == "between": continue
    r, se = corr(d["between"], d[c])
    mark = "  <--" if se >= 2.0 else ""
    print(f"    {c:12} r {r:+.3f}   {se:4.1f} SE{mark}")
print("\n  and with d log SEP -- the gain-invariant part")
for c in COLS:
    if c == "sep": continue
    r, se = corr(d["sep"], d[c])
    mark = "  <--" if se >= 2.0 else ""
    print(f"    {c:12} r {r:+.3f}   {se:4.1f} SE{mark}")
