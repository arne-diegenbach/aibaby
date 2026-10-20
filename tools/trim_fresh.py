#!/usr/bin/env python3
"""The trim's two claims, each tested the way its own provenance demands.

TWO DIFFERENT TREATMENTS, and the split is the whole point:

  THE PRIMARY -- "naming survives the trim" -- was the pre-registered question the
  trim existed to answer. The hypothesis was NOT selected from the data, so the two
  independent samples POOL by inverse variance and the answer comes from n=72.

  THE F2 HINT -- "the trimmed creature names F2 better" -- was READ OFF the discovery
  sample after the fact. Pooling it would re-introduce the very selection that makes
  it suspect, so it is tested on the FRESH SAMPLE ALONE. That is the discipline
  aibaby-pairing-on-discovery-sample was written for, where +14.8 at 2.5 SE on its own
  sample became +10.5 at 1.4 SE on fresh creatures.

Unpaired throughout: trimming re-rolls the wiring, so the two genomes are different
creatures and seed r of one is not matched to seed r of the other.
"""
import math

# (discovery mean, discovery se), (fresh mean, fresh se), per arm. n=36 each sample.
DATA = {
    "nearest": {"trim": ((0.157, 0.029), (0.114, 0.023)),
                "full": ((0.137, 0.023), (0.123, 0.029))},
    "F1":      {"trim": ((0.122, 0.023), (0.099, 0.027)),
                "full": ((0.126, 0.023), (0.136, 0.030))},
    "F2":      {"trim": ((0.127, 0.024), (0.082, 0.018)),
                "full": ((0.082, 0.021), (0.064, 0.027))},
}


def pool(samples):
    """Inverse-variance pool of independent sample means."""
    w = [1.0 / (se * se) for _, se in samples]
    m = sum(mu * wi for (mu, _), wi in zip(samples, w)) / sum(w)
    return m, math.sqrt(1.0 / sum(w))


def diff(a, b):
    """Unpaired difference a - b, in SE units."""
    (ma, sa), (mb, sb) = a, b
    d = ma - mb
    se = math.sqrt(sa * sa + sb * sb)
    return d, se, (abs(d) / se if se else 0.0)


print("=== THE PRIMARY: naming survives the trim (pooled, n=72 per arm) ===")
print("   pre-registered question, so both samples count\n")
for k in ("nearest", "F1", "F2"):
    t = pool(DATA[k]["trim"])
    f = pool(DATA[k]["full"])
    d, se, nse = diff(t, f)
    print("  %-8s trim %+.4f +/- %.4f   full %+.4f +/- %.4f   "
          "trim-full %+.4f +/- %.4f (%.2f SE)"
          % (k, t[0], t[1], f[0], f[1], d, se, nse))

print("\n=== THE F2 HINT: tested on the FRESH SAMPLE ALONE (n=36 per arm) ===")
print("   discovered in sample 1, so sample 1 cannot test it\n")
for k in ("nearest", "F1", "F2"):
    d, se, nse = diff(DATA[k]["trim"][1], DATA[k]["full"][1])
    print("  %-8s fresh trim-full %+.4f +/- %.4f (%.2f SE)%s"
          % (k, d, se, nse, "   <- the hint" if k == "F2" else ""))

print("\n=== and what the discovery sample alone had said, for the record ===")
for k in ("nearest", "F1", "F2"):
    d, se, nse = diff(DATA[k]["trim"][0], DATA[k]["full"][0])
    print("  %-8s discovery trim-full %+.4f +/- %.4f (%.2f SE)" % (k, d, se, nse))

# The bound a null is worth stating as: how much naming could the trim cost and still
# read like this? Two SE on the pooled difference, against the effect's own size.
t = pool(DATA["nearest"]["trim"])
f = pool(DATA["nearest"]["full"])
d, se, _ = diff(t, f)
print("\n=== the null as a BOUND, which is the only honest way to read it ===")
print("  pooled difference        %+.4f +/- %.4f" % (d, se))
print("  2 SE bound on any loss   %.4f" % (abs(d) + 2 * se))
print("  against an effect of     %.4f  (the full creature's pooled `nearest`)" % f[0])
print("  so the trim costs at most %.0f%% of the effect at 2 SE, and the point"
      % (100.0 * (abs(d) + 2 * se) / f[0]))
print("  estimate is %+.4f -- indistinguishable, NEVER identical." % d)
