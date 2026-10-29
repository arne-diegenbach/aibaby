# DOES THE s=10 IMPROVEMENT SURVIVE FRESH SEEDS?
# Written 2026-10-29 BEFORE the replication finished, so the reading rule is
# fixed ahead of the data.
#
# THE DISCOVERY ESTIMATE: s10 - s0 = +0.0612 +- 0.0242 on above-null direction,
# 2.5 SE, on its own 36 creatures with three contrasts tested.
# aibaby-pairing-on-discovery-sample records this exact shape reading 3.0 SE on
# its own 18 and 0.4 SE on 18 fresh, so the discovery number is expected to be
# inflated and is NOT the thing being tested here.
#
# THE PRE-REGISTERED READING RULE, in this order:
#   1. THE FRESH-SEED ESTIMATE DECIDES. Same sign and >= 2 SE  -> replicated.
#      Same sign but 1-2 SE -> suggestive, still not genome-ready.
#      Below 1 SE or wrong sign -> THE IMPROVEMENT IS WITHDRAWN. Say so plainly;
#      do not pool to rescue it.
#   2. The pooled estimate over all 72 creatures is reported SECOND and only as
#      a summary. Pooling a discovery sample with its own replication cannot
#      test the thing the replication exists to test.
#   3. The inverted U is a separate claim: it replicates only if s=100 is again
#      harmful at >= 2 SE AND s=10 is again the best rung.
#
# usage: exsweep_replicate.py DISCOVERY_LOG FRESH_LOG
import sys, math

ARMS = ("s 0", "s 10", "s 30", "s 100")

def load(path):
    by = {}
    for line in open(path):
        f = line.split()
        # per-seed rows: seed, arm (two tokens), dir, its null, utts, ...
        if len(f) >= 12 and f[0].isdigit() and f[1] == "s" and f[2] in ("0","10","30","100"):
            arm = "s " + f[2]
            try:
                by.setdefault(arm, {})[int(f[0])] = (float(f[3]), float(f[4]))
            except ValueError:
                pass
    return by

def mse(v):
    n = len(v); m = sum(v) / n
    if n < 2: return m, float("nan"), 0
    sd = math.sqrt(sum((x - m) ** 2 for x in v) / (n - 1))
    return m, sd / math.sqrt(n), n

def contrast(by, hi, lo):
    seeds = sorted(set(by.get(hi, {})) & set(by.get(lo, {})))
    # above-null direction, per creature: (dir - its own null)
    d = [(by[hi][s][0] - by[hi][s][1]) - (by[lo][s][0] - by[lo][s][1]) for s in seeds]
    return mse(d), d

disc, fresh = load(sys.argv[1]), load(sys.argv[2])
print(f"discovery arms: {sorted(disc)}\nfresh arms:     {sorted(fresh)}\n")

print("ABOVE-NULL DIRECTION, per arm")
print(f"{'arm':7} {'discovery':>20} {'fresh':>20}")
for a in ARMS:
    out = []
    for src in (disc, fresh):
        if a in src:
            m, se, n = mse([v[0] - v[1] for v in src[a].values()])
            out.append(f"{m:+.4f}+-{se:.4f} n{n}")
        else:
            out.append("--")
    print(f"{a:7} {out[0]:>20} {out[1]:>20}")

print("\n(1) THE FRESH-SEED ESTIMATE DECIDES")
(mf, sf, nf), df = contrast(fresh, "s 10", "s 0")
print(f"    s10 - s0 on FRESH seeds   {mf:+.4f} +- {sf:.4f}   ({abs(mf)/sf:.1f} SE, n={nf})")
print(f"    discovery was             +0.0612 +- 0.0242   (2.5 SE, n=36)")
if mf > 0 and abs(mf) / sf >= 2.0:
    print("    -> REPLICATED. Same sign, >= 2 SE on creatures it was not found on.")
elif mf > 0 and abs(mf) / sf >= 1.0:
    print("    -> SUGGESTIVE ONLY. Same sign, 1-2 SE. NOT genome-ready.")
else:
    print("    -> WITHDRAWN. The improvement does not survive fresh seeds.")
    print("       Do not pool to rescue it; report the withdrawal as prominently")
    print("       as the original claim.")

print("\n(2) pooled over all 72, reported second and only as a summary")
(md, sd_, nd), dd = contrast(disc, "s 10", "s 0")
pool = dd + df
mp, sp, np_ = mse(pool)
print(f"    s10 - s0 pooled   {mp:+.4f} +- {sp:.4f}   ({abs(mp)/sp:.1f} SE, n={np_})")

print("\n(3) does the INVERTED U replicate? needs s=100 harmful at >=2 SE")
print("    AND s=10 the best rung, both on fresh seeds")
(mh, sh, nh), _ = contrast(fresh, "s 100", "s 0")
print(f"    s100 - s0 on FRESH seeds  {mh:+.4f} +- {sh:.4f}   ({abs(mh)/sh:.1f} SE)")
best, bestv = None, None
for a in ARMS:
    if a not in fresh: continue
    m, _, _ = mse([v[0] - v[1] for v in fresh[a].values()])
    if bestv is None or m > bestv: best, bestv = a, m
print(f"    best rung on fresh seeds: {best} at {bestv:+.4f}")
ok_u = (mh < 0 and abs(mh) / sh >= 2.0 and best == "s 10")
print(f"    -> inverted U {'REPLICATES' if ok_u else 'does NOT replicate as specified'}")
