# IS THE CLIMBING LARYNX THRESHOLD THE CLOCK BEHIND `dirbudget`'s DECLINE?
#
# `dirbudget`'s rotation peaks between 13.6M and 27.2M and falls. Three explanations are
# refused: the magnitude trade, delivery starvation, and experience. What survives is that
# the cause must keep moving AFTER delivery stops changing, and exactly one measured
# quantity does -- `silence` found the larynx threshold still climbing +0.138 in the final
# tenth with the delivered fraction floored at 0.234 -> 0.233.
#
# THE JOIN, and why it costs no run of its own: `silence`'s per-creature threshold climb
# and `dirbudget`'s per-creature `direction` are the SAME creatures. Both derive the seed
# as `header().seed + r * 7919`, and `silence`'s delivered totals reproduced that ladder's
# 13.6M column exactly for the 12 it first ran. So the climb measured over the session's
# RISE can be asked to predict the LATER decline -- out of sample in time, which is a
# stronger shape than a contemporaneous correlation.
#
# PRE-REGISTERED PREDICTION, signed, from the experiment's own header:
#
#   THE THRESHOLD IS THE CLOCK -- a creature whose threshold climbs faster is further
#   past the peak by 27.2M, so r(climb, direction(27.2M) - direction(13.6M)) is NEGATIVE.
#
#   IT IS NOT -- r at or above 0, or unresolved. At n = 36 this join resolves about
#   |r| > 0.28 at 2 SE, so an unresolved read is reported as unresolved and NOT as a null.
#
# THE PRE-FLIGHT, read first and reported either way: `dirbudget`'s own free control
# measured r(trials, direction) = +0.583 (4.7 SE), so delivered trials are a KNOWN
# between-creature confound pointing the OPPOSITE way to the prediction. If the threshold
# climb tracks delivery between creatures, the primary is contaminated -- a negative would
# be fighting a positive confound (conservative) but a positive could be manufactured by
# it. Within a creature the two move oppositely; between creatures it is unmeasured.
#
# `direction` is used throughout and never `excess`: the null is an estimate that tracks
# how much was said, so excess is not comparable between creatures that deliver
# differently. The null's own correlation is printed beside every primary instead.
#
#   python3 tools/silence_thr_join.py results/dirbudget.log results/silence.log
import sys

LO, HI = "13.6M", "27.2M"


def fold(s):
    for a, b in (("its null", "its_null"), ("F1 spread", "F1_spread"),
                 ("F2 spread", "F2_spread")):
        s = s.replace(a, b)
    return s


def read_ladder(path):
    """{seed: {rung: {col: float}}} from `dirbudget`'s per-seed table."""
    idx, out = None, {}
    for line in open(path):
        f = fold(line).split()
        if len(f) > 4 and f[0] == "seed" and f[1] == "arm":
            idx = {n: i for i, n in enumerate(f)}
            continue
        if idx is None or len(f) != len(idx) or not f[0].isdigit() or not f[2].isdigit():
            continue
        row = {n: float(f[i]) for n, i in idx.items() if n not in ("seed", "arm")}
        out.setdefault(int(f[0]), {})[f[1]] = row
    return out


def read_climb(path):
    """{seed: (ip climb, ipoff climb, ip delivered)} from `silence`'s climb table.

    Anchored on the table's own header line, not on "a row whose first field counts
    up" -- that recognise-by-shape idiom has now harvested the wrong column twice in
    this project's tools (a `tenth` posing as a `seed`, and the reverse).
    """
    out, in_table = {}, False
    for line in open(path):
        f = line.split()
        # The header is `seed   ip climb   ipoff climb   ip delivered`, which splits into
        # SEVEN fields while its rows split into four -- so the header cannot be matched on
        # field count, and the first smoke of this reader failed on exactly that.
        if f[:3] == ["seed", "ip", "climb"]:
            in_table, out = True, {}
            continue
        if not in_table:
            continue
        if len(f) != 4 or not f[0].isdigit() or not f[3].isdigit():
            in_table = False
            continue
        out[int(f[0])] = (float(f[1]), float(f[2]), int(f[3]))
    return out


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


def ses(r, se):
    """The SE ratio as text. A degenerate SE is real -- the first smoke of this tool fed
    it a climb that was an exact linear function of the outcome and got 3.2e15 SE -- and a
    number that wide in a verdict line reads as a result rather than as a division by
    nearly zero, so it is capped and labelled instead."""
    if se <= 0.0 or not se < float("inf"):
        return "  n/a"
    q = abs(r / se)
    return ">999" if q > 999.0 else "%4.1f" % q


def show(label, xs, ys):
    r, se = jack(xs, ys)
    print("  %-34s %+.3f +- %.3f (%s SE)" % (label, r, se, ses(r, se)))
    return r, se


if len(sys.argv) != 3:
    print("usage: silence_thr_join.py <dirbudget log> <silence log>")
    sys.exit(2)

lad = read_ladder(sys.argv[1])
clm = read_climb(sys.argv[2])
if not clm:
    print("VOID: no per-seed threshold climb table in %s. That table was added when"
          % sys.argv[2])
    print("the second question was; a log from before it cannot answer this.")
    sys.exit(2)

shared = sorted(s for s in clm if s in lad and LO in lad[s] and HI in lad[s])
print("creatures in the climb table      %d" % len(clm))
print("with both %s and %s rungs    %d" % (LO, HI, len(shared)))
if len(shared) < 4:
    print("\nVOID: %d shared creatures is below the jackknife's floor." % len(shared))
    sys.exit(2)

# THE HARD CONTROL, before anything is correlated: these must be the same creatures. The
# climb table's delivered total is the `ip` arm at 13.6M, which IS the ladder's own call
# at its 13.6M rung. A single seed differing means the two logs are not one creature set
# and every correlation below is meaningless rather than merely noisy.
print("\nTHE HARD CONTROL. `ip` delivered total vs the ladder's %s trials:" % LO)
bad = [(s, clm[s][2], int(lad[s][LO]["trials"])) for s in shared
       if clm[s][2] != int(lad[s][LO]["trials"])]
if bad:
    print("  MISMATCH on %d of %d seeds -- the join is VOID, not suspect:" % (len(bad), len(shared)))
    for s, a, b in bad[:8]:
        print("    seed %-4d climb table %6d   ladder %6d" % (s, a, b))
    sys.exit(2)
print("  %d of %d seeds agree EXACTLY. One creature set, two logs." % (len(shared), len(shared)))

ip = [clm[s][0] for s in shared]
off = [clm[s][1] for s in shared]
dlv = [float(clm[s][2]) for s in shared]

# The dose check, per creature rather than pooled: `ipoff` must hold its threshold still.
# The pooled version is the experiment's own gate; this one would catch a few creatures
# carrying all the movement, which a mean cannot show.
moved = [s for s, v in zip(shared, off) if abs(v) > 1e-6]
print("\nTHE DOSE, per creature. `ipoff` creatures whose threshold moved at all: %d of %d"
      % (len(moved), len(shared)))
if moved:
    print("  seeds %s" % ", ".join(str(s) for s in moved[:10]))
    print("  The knob is not clean on these; the `ip` arm's climb is still the quantity")
    print("  of interest, but say so when quoting it.")

print("\nTHE PRE-FLIGHT. does the climb track how much the creature said?")
rp, sp = show("r(ip climb, ip delivered)", ip, dlv)
hot = abs(rp) > 2.0 * sp
if hot:
    print("  LIVE at 2 SE. Delivered trials are a known +0.583 confound on `direction`,")
    print("  so the primary below is CONTAMINATED. Read the sign carefully: a NEGATIVE")
    print("  primary is fighting the confound and survives it a fortiori; a POSITIVE one")
    print("  could be the confound alone and is not readable.")
else:
    print("  Clean at 2 SE: the threshold climb does not track delivery between")
    print("  creatures, so the known delivery confound does not enter the primary.")

print("\nTHE PRIMARY. climb (over the rise) against the LATER change in direction:")
d_dir = [lad[s][HI]["direction"] - lad[s][LO]["direction"] for s in shared]
d_nul = [lad[s][HI]["its_null"] - lad[s][LO]["its_null"] for s in shared]
r, se = show("r(climb, d direction %s->%s)" % (LO, HI), ip, d_dir)
show("  the null, for contamination", ip, d_nul)

print("\nFOR CONTEXT, the levels rather than the step:")
for rung in (LO, HI):
    show("r(climb, direction at %s)" % rung, ip, [lad[s][rung]["direction"] for s in shared])

resolved = abs(r) > 2.0 * se
print()
if resolved and r < 0 and not hot:
    print("THE THRESHOLD IS THE CANDIDATE AND IT HOLDS. Creatures whose larynx threshold")
    print("climbed faster over the session lost MORE direction afterwards, at %s SE."
          % ses(r, se).strip())
    print("That is the pre-registered sign, measured out of sample in time, and the")
    print("delivery confound that would have manufactured the opposite sign is not live.")
    print("It stays a BETWEEN-creature correlation: a brain that both regulates harder")
    print("and rotates less would read the same, and separating those needs the threshold")
    print("moved within a creature without moving its learning.")
elif resolved and r < 0:
    print("NEGATIVE AND RESOLVED at %s SE, with the delivery confound LIVE -- which it"
          % ses(r, se).strip())
    print("survives a fortiori, since the confound pushes the other way. The threshold")
    print("remains the candidate; the effect size is not quotable while the pre-flight")
    print("is hot.")
elif resolved:
    print("REFUSED, AND WITH THE WRONG SIGN. r = %+.3f at %s SE: creatures whose"
          % (r, ses(r, se).strip()))
    print("threshold climbed faster lost LESS direction, the opposite of what a threshold")
    print("clock predicts. So the last candidate for the decline with a named mechanism")
    print("is refused, and the decline has none left.")
    if hot:
        print("AND THE CONFOUND POINTS THIS WAY: delivered trials correlate with both the")
        print("climb (pre-flight, live) and `direction` (+0.583 in the ladder's own")
        print("control), so this positive may be the confound alone rather than a reversal")
        print("to explain. Either way the prediction failed; do not reach for an account")
        print("of the wrong sign before the confound is excluded.")
else:
    print("UNRESOLVED, NOT A NULL. r = %+.3f +- %.3f, inside the ~0.28 this join can" % (r, se))
    print("resolve at n = %d. The threshold is neither supported nor refused as the" % len(shared))
    print("clock, and the honest next step is a design that moves the threshold WITHIN a")
    print("creature rather than another between-creature read of the same width.")
