# TWO FREE CHECKS THE EXSWEEP RUNS MADE POSSIBLE.
#
# (1) CROSS-EXPERIMENT REPRODUCTION. `exsweep`'s s=0 arm is configured exactly as
#     `echoname`'s `taught` arm: same genome, same per-rep seed (header seed +
#     r*7919, which depends on r alone and not on the arm index), same
#     context_slots, same kVLTaught regime, same CtxDrive, same budget, same
#     560k echo buffer. So every shared column must come back IDENTICAL. Two
#     independently written experiments agreeing creature by creature validates
#     both harnesses in a way neither can validate itself.
#
# (2) AN INDEPENDENT REPLICATION OF THE NAMING MILESTONE, free. The fresh-seed
#     s=0 arm is a taught arm on 36 creatures no result has ever been read from.
#     echoname's headline was +0.1790 above its own null at 9.1 SE.
import sys, math

def fold(s):
    for a, b in (("its EAR","its_EAR"),("aud WHILE","aud_WHILE"),
                 ("its null","its_null"),("F1 spread","F1_spread")):
        s = s.replace(a, b)
    return s

def load_echoname(path, arm):
    idx, out = None, {}
    for line in open(path):
        f = fold(line).split()
        if len(f) > 4 and f[0] == "seed" and f[1] == "arm":
            idx = {n: i for i, n in enumerate(f)}; continue
        if idx and len(f) >= 16 and f[0].isdigit() and f[1] == arm:
            out[int(f[0])] = {k: f[idx[k]] for k in
                              ("direction","its_null","artic","F1_spread","voiced",
                               "within","between","sep")}
    return out

def load_exsweep(path, arm):
    out = {}
    for line in open(path):
        f = line.split()
        if len(f) >= 12 and f[0].isdigit() and f[1] == "s" and f[2] == arm:
            out[int(f[0])] = {"direction": f[3], "its_null": f[4], "artic": f[7],
                              "voiced": f[8], "within": f[10],
                              "between": f[11], "sep": f[12]}
    return out

en = load_echoname(sys.argv[1], "taught")
xs = load_exsweep(sys.argv[2], "0")
print(f"(1) CROSS-EXPERIMENT REPRODUCTION, echoname `taught` vs exsweep `s 0`")
print(f"    creatures: echoname {len(en)}, exsweep {len(xs)}")
keys = ("direction","its_null","artic","voiced","within","between","sep")
seeds = sorted(set(en) & set(xs))
bad = {k: 0 for k in keys}
for s in seeds:
    for k in keys:
        if en[s][k] != xs[s][k]:
            bad[k] += 1
tot = sum(bad.values())
if tot == 0:
    print(f"    IDENTICAL on all {len(seeds)} creatures x {len(keys)} shared columns")
else:
    print(f"    {tot} mismatches:")
    for k, v in bad.items():
        if v: print(f"      {k:11} {v} of {len(seeds)}")
        
fresh = load_exsweep(sys.argv[3], "0")
def mse(v):
    n = len(v); m = sum(v)/n
    sd = math.sqrt(sum((x-m)**2 for x in v)/(n-1))
    return m, sd/math.sqrt(n), n
print(f"\n(2) THE NAMING MILESTONE ON FRESH CREATURES (exsweep s=0, seed 20261029)")
a = [float(fresh[s]["direction"]) - float(fresh[s]["its_null"]) for s in sorted(fresh)]
m, se, n = mse(a)
print(f"    above-null direction  {m:+.4f} +- {se:.4f}   ({abs(m)/se:.1f} SE, n={n})")
d = [float(en[s]["direction"]) - float(en[s]["its_null"]) for s in sorted(en)]
md, sed, nd = mse(d)
print(f"    echoname's own        {md:+.4f} +- {sed:.4f}   ({abs(md)/sed:.1f} SE, n={nd})")
print(f"    -> {'REPLICATED on creatures never used before' if m > 0 and abs(m)/se >= 3 else 'NOT replicated'}")
