# IS THE ARTICULATOR CONTRAST BIGGER, OR IS `echo` JUST NEARER THE CEILING?
# `echo` reads 0.955 on the taught arm, so a difference measured there has less
# room than the same difference at 0.803. Dropping voicing and loudness costs the
# LOW arms ~0.065 and the HIGH arms ~0.021, which is exactly what ceiling
# compression looks like. A log-odds scale is insensitive to that, so if the
# contrast still grows there, the growth is not the ceiling.
#
# Per-creature accuracies can hit 1.000, where the logit is infinite, so an
# empirical-logit offset is used and the number of affected creatures PRINTED.
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
    if len(f) > 6 and f[0].isdigit() and f[1] in ARMS:
        by.setdefault(f[1], {})[int(f[0])] = (float(f[idx["echo"]]), float(f[idx["artic"]]))
seeds = sorted(by["none"])

# The six pairs are averaged into each printed value, so the finest possible
# step is 1/6 of a pair; the offset is half of the smallest nonzero step.
EPS = 1.0 / 12.0 / 6.0
def logit(p):
    q = min(max(p, EPS), 1.0 - EPS)
    return math.log(q / (1.0 - q))

def mean_se(v):
    n = len(v); m = sum(v) / n
    sd = math.sqrt(sum((x - m) ** 2 for x in v) / (n - 1))
    return m, sd / math.sqrt(n)

clamped = sum(1 for a in ARMS for s in seeds for v in by[a][s] if v >= 1.0 - EPS or v <= EPS)
print(f"creature-arm-column values clamped by the logit offset: {clamped} of "
      f"{len(ARMS) * len(seeds) * 2}   (offset {EPS:.5f})\n")
print(f"{'contrast':16} {'scale':9} {'echo':>18} {'artic':>18}   artic/echo")
for hi, lo in (("yoked", "none"), ("taught", "none")):
    row = {}
    for name, f in (("raw", lambda p: p), ("log-odds", logit)):
        de = [f(by[hi][s][0]) - f(by[lo][s][0]) for s in seeds]
        da = [f(by[hi][s][1]) - f(by[lo][s][1]) for s in seeds]
        me, see = mean_se(de); ma, sea = mean_se(da)
        row[name] = (me, ma)
        print(f"{hi+' - '+lo:16} {name:9} {me:+10.4f}+-{see:.4f} {ma:+10.4f}+-{sea:.4f}"
              f"   {ma/me:9.2f}x")
