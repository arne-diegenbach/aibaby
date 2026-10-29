# IS THE ROTATION TRIAL-LIMITED OR SATURATED?
#
# Reward supplies a decodable, well-separated word -> vocal map; the lesson only
# rotates it onto the targets, and gets above-null direction to +0.18..+0.24 out
# of a possible ~+0.75. If more teaching buys more alignment, the rotation is
# trial-limited and a longer or denser lesson is an improvement lever. If it is
# flat, the rotation has saturated and more of the same will not help.
#
# Observational: `utts` and `praises` are OUTCOMES, not assignments -- a creature
# that vocalises more also gets taught more. So this bounds the lever's
# plausibility, it does not establish it. 72 creatures: the s=0 arms of both
# exsweep samples, which are the same taught configuration on two seed bases.
import sys, math

def load(path):
    out = []
    for line in open(path):
        f = line.split()
        if len(f) >= 13 and f[0].isdigit() and f[1] == "s" and f[2] == "0":
            # seed, arm(2), dir, dnull, utts, praises, artic, vfrac, mu, within, between, sep
            out.append((float(f[3]) - float(f[4]), float(f[5]), float(f[6]),
                        float(f[11]), float(f[12])))
    return out

rows = []
for p in sys.argv[1:]:
    r = load(p)
    print(f"{p}: {len(r)} creatures")
    rows += r
print(f"pooled: {len(rows)}\n")

def corr(x, y):
    n = len(x); mx, my = sum(x)/n, sum(y)/n
    sxx = sum((a-mx)**2 for a in x); syy = sum((b-my)**2 for b in y)
    if sxx <= 0 or syy <= 0: return 0.0, 0.0, 0.0
    r = sum((a-mx)*(b-my) for a, b in zip(x, y)) / math.sqrt(sxx*syy)
    se = (1 - r*r) / math.sqrt(n - 3)
    return r, abs(r)/se, sum((a-mx)*(b-my) for a, b in zip(x, y))/sxx

names = ("utterances", "praises", "between", "sep")
d = [r[0] for r in rows]
print("correlation of ABOVE-NULL DIRECTION with:")
for i, nm in enumerate(names, start=1):
    v = [r[i] for r in rows]
    c, se, slope = corr(v, d)
    mark = "  <--" if se >= 2.0 else ""
    print(f"  {nm:12} r {c:+.3f}   {se:4.1f} SE   slope {slope:+.3e} per unit{mark}")

print(f"\n2 SE at n={len(rows)} excludes |r| > {2/math.sqrt(len(rows)-3):.2f}")
lo = sorted(rows, key=lambda r: r[1])[:len(rows)//3]
hi = sorted(rows, key=lambda r: r[1])[-(len(rows)//3):]
def mse(v):
    n=len(v); m=sum(v)/n
    return m, math.sqrt(sum((x-m)**2 for x in v)/(n-1))/math.sqrt(n)
ml, sl = mse([r[0] for r in lo]); mh, sh = mse([r[0] for r in hi])
print(f"\nfewest-utterance third  (mean {sum(r[1] for r in lo)/len(lo):.0f} utts): "
      f"direction {ml:+.4f} +- {sl:.4f}")
print(f"most-utterance third    (mean {sum(r[1] for r in hi)/len(hi):.0f} utts): "
      f"direction {mh:+.4f} +- {sh:.4f}")
diff = mh - ml; sed = math.sqrt(sl*sl + sh*sh)
print(f"difference {diff:+.4f} +- {sed:.4f}  ({abs(diff)/sed:.1f} SE)")
