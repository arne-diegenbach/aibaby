# IS THE DECLINE DRIVEN BY EXPERIENCE OR BY TICKS? A free test on the ladder log.
#
# `dirbudget` matched its rungs on TICKS. `dlvry` then showed the delivered fraction
# collapsing within a session, and -- the part that makes this test possible -- delivery
# varies a lot BETWEEN creatures at the same budget: 1648 to 2538 delivered trials at
# 13.6M, a 1.54x spread. So "ticks" and "experience" are not the same axis here, and the
# ladder log already contains the variation needed to tell them apart. No run required.
#
# THE TEST, within each step, paired per creature: regress the change in `excess` on the
# change in delivered `trials`. Both are differences of the same creature's own numbers,
# so creature-level offsets cancel.
#
# PRE-REGISTERED PREDICTIONS, written before looking:
#
#   EXPERIENCE-DRIVEN -- the creature walks one curve parameterised by delivered trials,
#   rising then falling. A creature that gained MORE trials in a step travels FURTHER
#   along that curve, so it should gain more early and lose more late: a correlation that
#   is POSITIVE in the first step and NEGATIVE in the top step. **A SIGN FLIP across the
#   steps is the signature**, and it is what would explain the decline.
#
#   TICK-DRIVEN -- something that tracks elapsed time rather than answered trials (the
#   climbing larynx threshold, drift, any slow regulator). Then the between-creature
#   delivery spread is irrelevant and every within-step correlation sits at ~0.
#
# THE ARTEFACT THAT WOULD VOID IT, checked first and reported either way: `excess` is
# `direction - its null`, and both are ESTIMATES whose precision depends on how many
# utterances went into them. If a creature with more delivered trials gets a lower null
# merely by estimating it better, then excess rises with trials for a reason that has
# nothing to do with learning, and a positive correlation means nothing. So the null's
# own dependence on trials is measured before the primary, and the primary is reported as
# UNINTERPRETABLE if it is live.
#
#   python3 tools/dirbudget_experience.py results/dirbudget.log
import sys


def fold(s):
    for a, b in (("its null", "its_null"), ("F1 spread", "F1_spread"),
                 ("F2 spread", "F2_spread")):
        s = s.replace(a, b)
    return s


def read(path):
    """{seed: {arm: {col: float}}} from the per-seed table, plus the arm order."""
    idx, out, order = None, {}, []
    for line in open(path):
        f = fold(line).split()
        if len(f) > 4 and f[0] == "seed" and f[1] == "arm":
            idx = {n: i for i, n in enumerate(f)}
            continue
        if idx is None or len(f) != len(idx) or not f[0].isdigit() or not f[2].isdigit():
            continue
        row = {}
        for n, i in idx.items():
            if n not in ("seed", "arm"):
                row[n] = float(f[i])
        out.setdefault(int(f[0]), {})[f[1]] = row
        if f[1] not in order:
            order.append(f[1])
    return out, order


def pearson(xs, ys):
    n = float(len(xs))
    mx, my = sum(xs) / n, sum(ys) / n
    sxy = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
    sxx = sum((x - mx) ** 2 for x in xs)
    syy = sum((y - my) ** 2 for y in ys)
    return sxy / (sxx * syy) ** 0.5 if sxx > 0 and syy > 0 else 0.0


def jack(xs, ys):
    """r and its delete-one jackknife SE, the convention the C++ side uses."""
    r = pearson(xs, ys)
    n = len(xs)
    if n < 4:
        return r, float("inf")
    reps = [pearson(xs[:i] + xs[i + 1:], ys[:i] + ys[i + 1:]) for i in range(n)]
    m = sum(reps) / n
    acc = sum((v - m) ** 2 for v in reps)
    return r, (acc * (n - 1) / n) ** 0.5


if len(sys.argv) != 2:
    print("usage: dirbudget_experience.py <dirbudget log>")
    sys.exit(2)

tab, arms = read(sys.argv[1])
if len(arms) < 2:
    print("VOID: read %d arms, need at least 2 to form a step" % len(arms))
    sys.exit(2)

full = sorted(s for s in tab if all(a in tab[s] for a in arms))
print("creatures with all %d rungs   %d" % (len(arms), len(full)))
print("rungs                         %s\n" % ", ".join(arms))

print("THE ARTEFACT CHECK FIRST. does the null fall as delivered trials rise?")
print("  rung      n   r(trials, its null)        r(trials, direction)")
live = False
for a in arms:
    xs = [tab[s][a]["trials"] for s in full]
    rn, sn = jack(xs, [tab[s][a]["its_null"] for s in full])
    rd, sd = jack(xs, [tab[s][a]["direction"] for s in full])
    hot = abs(rn) > 2.0 * sn
    live = live or hot
    print("  %-8s %3d   %+.3f +- %.3f (%4.1f SE)%s  %+.3f +- %.3f (%4.1f SE)"
          % (a, len(full), rn, sn, abs(rn / sn) if sn else 0.0,
             " <--" if hot else "    ", rd, sd, abs(rd / sd) if sd else 0.0))

if live:
    print("\n  THE ARTEFACT IS LIVE on at least one rung: the null itself moves with")
    print("  delivered trials, so `excess` is not a clean readout of learning against")
    print("  delivery and the primary below is UNINTERPRETABLE as stated. Reported for")
    print("  the record only; do not quote it.")
else:
    print("\n  Clean: the null does not track delivered trials at 2 SE on any rung, so")
    print("  `excess` differences are not an estimation artefact of how much was said.")

print("\nTHE PRIMARY. within each step, paired per creature: d(excess) vs d(trials)")
print("  step                       n   r                      mean d(trials)")
rs = []
for q in range(len(arms) - 1):
    lo, hi = arms[q], arms[q + 1]
    dt = [tab[s][hi]["trials"] - tab[s][lo]["trials"] for s in full]
    de = [tab[s][hi]["excess"] - tab[s][lo]["excess"] for s in full]
    r, se = jack(dt, de)
    rs.append((lo + "->" + hi, r, se))
    print("  %-24s %3d   %+.3f +- %.3f (%4.1f SE)   %+8.1f"
          % (lo + " -> " + hi, len(full), r, se, abs(r / se) if se else 0.0,
             sum(dt) / len(dt)))

first, last = rs[0], rs[-1]
flip = first[1] > 0 and last[1] < 0
resolved = [x for x in rs if abs(x[1]) > 2.0 * x[2]]

print()
if flip and any(x[0] == last[0] for x in resolved):
    print("THE SIGN FLIP IS PRESENT and the top step is resolved. The decline goes with")
    print("EXPERIENCE: a creature that answered more trials in the top step lost more")
    print("direction, which is what one curve in delivered trials predicts and what a")
    print("tick-driven cause does not.")
elif not resolved:
    print("NO STEP RESOLVES at 2 SE. The between-creature delivery spread does not")
    print("predict who gains or loses direction, in any step. That is the TICK-DRIVEN")
    print("reading: whatever moves the rotation is not counting answered trials. It is")
    print("also what too little spread would look like, so the mean d(trials) column")
    print("above is the thing to read before believing it.")
else:
    print("MIXED: %d of %d steps resolve and the first/top signs are %+.3f / %+.3f, so"
          % (len(resolved), len(rs), first[1], last[1]))
    print("the clean sign flip is NOT present. Neither reading is supported as stated;")
    print("the resolved steps are listed above and need explaining one at a time.")
