# DOES THE ECHO RISE BECAUSE THE EAR FALLS? Free, on the archived log.
# Reward makes the ear WORSE (0.682 -> 0.602 -> 0.574) while making the vocal
# readout better. If that trade is causal within creatures, the creature whose
# ear fell furthest should be the creature whose echo rose most: a NEGATIVE
# correlation between the paired ear change and the paired artic change.
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
by = {}
for line in open(path):
    f = fold(line).split()
    if len(f) > 8 and f[0].isdigit() and f[1] in ARMS:
        by.setdefault(f[1], {})[int(f[0])] = (
            float(f[idx["artic"]]), float(f[idx["its_EAR"]]), float(f[idx["echo"]]))
seeds = sorted(by["none"])

def mean_se(v):
    n = len(v); m = sum(v) / n
    sd = math.sqrt(sum((x - m) ** 2 for x in v) / (n - 1))
    return m, sd / math.sqrt(n)
def corr(x, y):
    n = len(x); mx, my = sum(x) / n, sum(y) / n
    sxx = sum((a - mx) ** 2 for a in x); syy = sum((b - my) ** 2 for b in y)
    r = sum((a - mx) * (b - my) for a, b in zip(x, y)) / math.sqrt(sxx * syy)
    return r, abs(r) / ((1 - r * r) / math.sqrt(n - 3))

print(f"{'contrast':16} {'d artic':>18} {'d EAR':>18} {'corr':>8} {'r/SE':>5}")
for hi, lo in (("yoked", "none"), ("taught", "none"), ("none", "fresh")):
    da = [by[hi][s][0] - by[lo][s][0] for s in seeds]
    de = [by[hi][s][1] - by[lo][s][1] for s in seeds]
    ma, sa = mean_se(da); me, se = mean_se(de)
    r, rse = corr(de, da)
    print(f"{hi+' - '+lo:16} {ma:+10.4f}+-{sa:.4f} {me:+10.4f}+-{se:.4f} {r:+8.3f} {rse:5.1f}")
print("\nWITHIN each arm, over its creatures: corr(artic, its EAR)")
for a in ARMS:
    r, rse = corr([by[a][s][1] for s in seeds], [by[a][s][0] for s in seeds])
    print(f"  {a:9} {r:+.3f}  ({rse:.1f} SE)")
