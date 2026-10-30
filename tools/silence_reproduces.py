# DID RAISING THE REP COUNT RE-ROLL THE CREATURES? An integer control, no run.
#
# `silence` was published at 12 creatures and re-run at 36 to make its per-seed threshold
# climb joinable against `dirbudget`'s 36. That is only safe if the extra creatures are
# APPENDED rather than the set being redrawn: the seed is `header().seed + r * 7919`, so
# it does not depend on the rep count, and creatures 0-11 should come back bit-identical.
#
# "Should" is the wrong footing for a published result. This checks it, on the one column
# both logs print per creature -- the `ip` arm's delivered total, an integer. A single
# seed differing means the re-run is a DIFFERENT creature set and the earlier numbers
# cannot be quoted beside the new ones.
#
# The companion worry runs the other way and is NOT checked here because it cannot be:
# appending or dropping an ARM re-rolls the noise draw, so this control would fail loudly
# if the arms had changed -- which is why the arm list was left alone.
#
#   python3 tools/silence_reproduces.py results/silence.log results/silence-n36.log
import sys


def totals(path):
    """{seed: delivered} from `THE ip ARM'S PER-SEED DELIVERED TOTAL`.

    Anchored on that table's own header, whose first field is `seed` and whose last is
    `was`. Recognising rows by "a leading integer counting up" has twice harvested the
    wrong column in this project's tools, so it is not used.
    """
    out, in_table = {}, False
    for line in open(path):
        f = line.split()
        if len(f) == 4 and f[0] == "seed" and f[2] == "nominal" and f[3] == "was":
            out, in_table = {}, True
            continue
        if not in_table:
            continue
        if len(f) != 3 or not all(x.isdigit() for x in f):
            in_table = False
            continue
        out[int(f[0])] = int(f[1])
    return out


if len(sys.argv) != 3:
    print("usage: silence_reproduces.py <earlier log> <later log>")
    sys.exit(2)

old, new = totals(sys.argv[1]), totals(sys.argv[2])
if not old or not new:
    print("VOID: read %d and %d per-seed rows; both logs need that table."
          % (len(old), len(new)))
    sys.exit(2)

shared = sorted(set(old) & set(new))
print("earlier log   %d creatures" % len(old))
print("later log     %d creatures" % len(new))
print("shared        %d\n" % len(shared))
if not shared:
    print("VOID: the two logs share no seed, so there is nothing to reproduce.")
    sys.exit(2)

bad = [(s, old[s], new[s]) for s in shared if old[s] != new[s]]
for s, a, b in bad[:10]:
    print("  seed %-4d earlier %6d   later %6d" % (s, a, b))

if bad:
    print("\nRE-ROLLED. %d of %d shared creatures deliver a different total, so raising"
          % (len(bad), len(shared)))
    print("the rep count did NOT simply append creatures. The earlier log's numbers and")
    print("the later log's are two different creature sets and must not be quoted")
    print("together, whatever else agrees.")
    sys.exit(1)

print("REPRODUCED. All %d shared creatures deliver the IDENTICAL total, so raising the"
      % len(shared))
print("rep count appended creatures and redrew nothing. The earlier log's published")
print("numbers remain the same creatures' numbers, and the %d new ones are additions."
      % (len(new) - len(shared)))
