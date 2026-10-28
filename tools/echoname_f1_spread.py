# Does the expansion reach F1 after all? `sp1` is the mean over the six word
# pairs of |mean F1(x) - mean F1(y)| in Hz -- the F1 analogue of `between`.
# `direction` is whether the right word gets the right F1.
import sys, math
ARMS = ("fresh", "none", "taught", "nodrive", "yoked")
def fold(s):
    for a, b in (("its EAR","its_EAR"),("aud WHILE","aud_WHILE"),
                 ("its null","its_null"),("F1 spread","F1_spread")):
        s = s.replace(a, b)
    return s
path = sys.argv[1]; idx = None
for line in open(path):
    f = fold(line).split()
    if len(f) > 4 and f[0] == "seed" and f[1] == "arm":
        idx = {n: i for i, n in enumerate(f)}; break
by = {}
for line in open(path):
    f = fold(line).split()
    if len(f) >= 16 and f[0].isdigit() and f[1] in ARMS:
        by.setdefault(f[1], {})[int(f[0])] = (
            float(f[idx["F1_spread"]]), float(f[idx["direction"]]),
            float(f[idx["its_null"]]), float(f[idx["between"]]))
seeds = sorted(by["none"])
def mse(v):
    n = len(v); m = sum(v)/n
    sd = math.sqrt(sum((x-m)**2 for x in v)/(n-1))
    return m, sd/math.sqrt(n)
print(f"{'arm':9} {'F1 spread Hz':>16} {'direction':>16} {'its null':>10}")
for a in ARMS:
    s, _ = mse([by[a][x][0] for x in seeds])
    ss = mse([by[a][x][0] for x in seeds])[1]
    d, ds = mse([by[a][x][1] for x in seeds])
    nl, _ = mse([by[a][x][2] for x in seeds])
    print(f"{a:9} {s:10.1f}+-{ss:4.1f} {d:10.3f}+-{ds:.3f} {nl:10.3f}")
print()
print(f"{'contrast':16} {'d F1 spread Hz':>18} {'SE u':>5} {'ratio':>7} {'d direction':>16} {'SE u':>5}")
for hi, lo in (("taught","none"),("yoked","none"),("taught","yoked")):
    ds = [by[hi][x][0]-by[lo][x][0] for x in seeds]
    dd = [by[hi][x][1]-by[lo][x][1] for x in seeds]
    m, se = mse(ds); md, sed = mse(dd)
    r0 = mse([by[lo][x][0] for x in seeds])[0]
    r1 = mse([by[hi][x][0] for x in seeds])[0]
    print(f"{hi+' - '+lo:16} {m:+11.1f}+-{se:4.1f} {abs(m)/se:5.1f} {r1/r0:6.2f}x"
          f" {md:+10.4f}+-{sed:.4f} {abs(md)/sed:5.1f}")
