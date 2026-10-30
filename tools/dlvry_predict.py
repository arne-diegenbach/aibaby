# THE PREDICTION CHECK ON `dlvry`, and it is an identity rather than a comparison.
#
# Nothing that moves the creature depends on `n_trials` in this session: it sets the
# loop bound, the early/late thirds and the checkpoint cadence, and the only one of
# those with a side effect is the checkpoint's `consolidate_context_bias`, which is
# inert unless a CtxDrive asks for banking -- `dirbudget` does not. So the first N
# trials of a long session ARE a complete N-trial session, trial for trial.
#
# Therefore `dlvry`'s cumulative delivered count at each rung's trial count must equal
# `dirbudget`'s per-seed `trials` column for that rung EXACTLY, on every shared
# creature. These are integer counts, so "exactly" means equal, not close.
#
# If it holds, the ladder's four rungs are nested prefixes of one trajectory and the
# within-session delivery curve measured at the top rung explains the shorter ones. If
# it fails, that reading is not available and the curve says nothing about them.
#
#   python3 tools/dlvry_predict.py results/dirbudget.log results/dlvry.log
import sys


def fold(s):
    for a, b in (("its null", "its_null"), ("F1 spread", "F1_spread"),
                 ("F2 spread", "F2_spread"), ("nominal was", "nominal_was")):
        s = s.replace(a, b)
    return s


def read_ladder(path):
    """{seed: {arm: delivered trials}} from dirbudget's per-seed table."""
    idx, out = None, {}
    for line in open(path):
        f = fold(line).split()
        if len(f) > 4 and f[0] == "seed" and f[1] == "arm":
            idx = {n: i for i, n in enumerate(f)}
            continue
        if idx is None or len(f) < 4 or not f[0].isdigit() or not f[2].isdigit():
            continue
        out.setdefault(int(f[0]), {})[f[1]] = int(f[idx["trials"]])
    return out


def read_prefix(path):
    """{seed: {arm: cumulative delivered}} from dlvry's prefix table."""
    arms, out = None, {}
    for line in open(path):
        f = fold(line).split()
        if len(f) > 2 and f[0] == "seed" and f[-1] == "nominal_was":
            arms = f[1:-1]
            continue
        # The `tenth` column of the delivery table that follows runs 1..10 and would
        # otherwise overwrite seeds 1..10 with per-tenth counts -- which is exactly what
        # the first version of this tool did, reporting 10 of 12 creatures as mismatched
        # while the log plainly showed them agreeing. So a prefix row is recognised by
        # its SHAPE: the trailing "nominal was" field, which no other table carries.
        if arms is None or len(f) != len(arms) + 2 or not f[0].isdigit():
            continue
        # The nominal field is the rung's trial count, or a slash-list of one count per
        # rung when the table has several. Requiring exactly one part per arm is what
        # excludes `dlvry`'s delivery table, whose rows carry the same field count as
        # its four-rung prefix rows; a single-rung log like `silence` prints a bare
        # count there, which an earlier `"/" in f[-1]` test rejected along with it.
        nominal = f[-1].split("/")
        if len(nominal) != len(arms) or not all(n.isdigit() for n in nominal):
            continue
        vals = f[1:1 + len(arms)]
        if not all(v.isdigit() for v in vals):
            continue
        if int(f[0]) in out:
            print("VOID: creature %s appears twice in the prefix table" % f[0])
            sys.exit(2)
        out[int(f[0])] = {a: int(v) for a, v in zip(arms, vals)}
    return out, arms


def probe_name(path):
    """The log's own `experiment:` name, so a `silence` log is not labelled `dlvry`."""
    for line in open(path):
        f = line.split()
        if len(f) > 1 and f[0] == "experiment:":
            return f[1][:8]
    return "probe"


if len(sys.argv) != 3:
    print("usage: dlvry_predict.py <dirbudget log> <probe log>")
    sys.exit(2)

lad = read_ladder(sys.argv[1])
pre, arms = read_prefix(sys.argv[2])

if not lad:
    print("VOID: no per-seed rows in %s -- wrong log?" % sys.argv[1])
    sys.exit(2)
if not pre:
    print("VOID: no prefix rows in %s -- did the run reach its table?" % sys.argv[2])
    sys.exit(2)

shared = sorted(set(lad) & set(pre))
if not shared:
    print("VOID: the two logs share no creature")
    sys.exit(2)

probe = probe_name(sys.argv[2])
print("creatures compared   %d  (ladder %d, %s %d)" % (len(shared), len(lad), probe, len(pre)))
print("rungs                %s" % ", ".join(arms))
print("compared as INTEGER COUNTS -- these are predictions of an identity, not a fit\n")

bad_total = 0
for a in arms:
    pairs = [(s, lad[s][a], pre[s][a]) for s in shared if a in lad[s] and a in pre[s]]
    if not pairs:
        print("  %-8s NO SHARED ROWS -- the ladder log has no `%s` arm" % (a, a))
        continue
    bad = [(s, x, y) for s, x, y in pairs if x != y]
    bad_total += len(bad)
    tag = "IDENTICAL" if not bad else "DIFFERS on %d" % len(bad)
    print("  %-8s n=%2d   %s" % (a, len(pairs), tag))
    for s, x, y in bad[:6]:
        print("           seed %-4d ladder %6d   %-6s %6d   delta %+d" % (s, x, probe, y, y - x))
    if len(bad) > 6:
        print("           ... and %d more" % (len(bad) - 6))

if bad_total:
    print("\nTHE PREDICTION FAILS on %d creature-rungs. The rungs are NOT nested prefixes" % bad_total)
    print("of one trajectory, so the within-session curve measured at the top rung does")
    print("not explain the shorter budgets. Before reading anything into the curve, check")
    print("the genome is the one the ladder ran and that no CtxDrive asks for banking.")
    sys.exit(1)

print("\nTHE PREDICTION HOLDS on every compared creature-rung (%d creatures)." % len(shared))
print("The ladder's rungs ARE nested prefixes of one trajectory, so the delivery curve")
print("measured at the top rung is the curve that produced every shorter rung's")
print("delivered fraction.")
