# IS THE YOKE'S MAP ARBITRARY-BUT-COMMITTED, OR ABSENT?
#
# The yoke reads 0.918 held-out on the articulator echo, so a classifier CAN
# recover which word it heard from its vocal code: the word -> vocal-state map is
# CONSISTENT. Yet `direction` sits at its own null. Those two facts together say
# the map is stable and simply not aligned to the targets.
#
# The test: if each creature commits to its OWN arbitrary map, a creature that
# separated its vowels further should land further from chance in EITHER
# direction. So |direction - null| should rise with F1 spread while the SIGNED
# (direction - null) should not. If instead reward produces no map at all, both
# are flat and the direction spread should not exceed the unrewarded arm's.
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
K = ("direction", "its_null", "F1_spread", "artic")
by = {}
for line in open(path):
    f = fold(line).split()
    if len(f) >= 16 and f[0].isdigit() and f[1] in ARMS:
        by.setdefault(f[1], {})[int(f[0])] = {k: float(f[idx[k]]) for k in K}
seeds = sorted(by["none"])
def stats(v):
    n = len(v); m = sum(v)/n
    sd = math.sqrt(sum((x-m)**2 for x in v)/(n-1))
    return m, sd, sd/math.sqrt(n)
def corr(x, y):
    n = len(x); mx, my = sum(x)/n, sum(y)/n
    sxx = sum((a-mx)**2 for a in x); syy = sum((b-my)**2 for b in y)
    if sxx <= 0 or syy <= 0: return 0.0, 0.0
    r = sum((a-mx)*(b-my) for a,b in zip(x,y)) / math.sqrt(sxx*syy)
    return r, abs(r)/((1-r*r)/math.sqrt(n-3))

print("HOW FAR FROM ITS OWN NULL DOES EACH ARM SIT, AND HOW WIDELY?")
print(f"{'arm':9} {'signed mean':>14} {'SD':>7} {'|dev| mean':>12} {'SD':>7}"
      f" {'above/below':>12}")
for a in ARMS:
    s = [by[a][x]["direction"] - by[a][x]["its_null"] for x in seeds]
    if max(abs(v) for v in s) == 0.0:
        print(f"{a:9} {'(no utterances scored)':>14}")
        continue
    m, sd, se = stats(s)
    am, asd, ase = stats([abs(v) for v in s])
    up = sum(1 for v in s if v > 0)
    print(f"{a:9} {m:+9.4f}+-{se:.4f} {sd:7.4f} {am:12.4f} {asd:7.4f}"
          f" {up:5d}/{len(s)-up:<6d}")

print("\nDOES A CREATURE THAT SEPARATED MORE LAND FURTHER FROM CHANCE?")
print(f"{'arm':9} {'r(|dev|, F1 spread)':>22} {'SE':>5}   {'r(signed, F1 spread)':>22} {'SE':>5}")
for a in ("none", "yoked", "taught"):
    s = [by[a][x]["direction"] - by[a][x]["its_null"] for x in seeds]
    sp = [by[a][x]["F1_spread"] for x in seeds]
    r1, e1 = corr([abs(v) for v in s], sp)
    r2, e2 = corr(s, sp)
    print(f"{a:9} {r1:+22.3f} {e1:5.1f}   {r2:+22.3f} {e2:5.1f}")
