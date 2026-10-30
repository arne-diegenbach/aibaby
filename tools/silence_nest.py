# THE FINER IDENTITY: `silence`'s tenths must PAIR UP into `dlvry`'s.
#
# `dlvry` ran 9714 trials and binned them into ten tenths of 971. `silence`'s `ip` arm
# ran 4857 trials -- exactly half -- and binned them into ten tenths of 486. Same genome,
# same seeds, same shipped homeostat, and `dlvry_predict.py` has already shown the two
# runs' per-creature delivered totals agree as integers.
#
# So the two histograms must NEST: silence tenths (1,2) cover precisely the trials of
# dlvry tenth 1, (3,4) cover tenth 2, and so on through dlvry tenth 5, which is where
# silence's session ends. Summing each silence pair must reproduce dlvry's `nominal` and
# `delivered` counts EXACTLY -- 10 integers, not 10 approximations.
#
# `dlvry_predict.py` compares per-creature TOTALS, which a correct total with mis-binned
# tenths would still pass. This checks the binning itself, and it is free.
#
#   python3 tools/silence_nest.py results/dlvry.log results/silence.log
import sys


def deciles(path, arm=None):
    """[(nominal, delivered)] x 10 from a per-tenth table, optionally after `arm <name>`.

    Anchored on the `tenth ...` header. Recognising a row by "leading integer that counts
    1,2,3.. in order" is NOT enough: the per-seed prefix tables in both logs have a `seed`
    column that counts the same way, and the first version of this reader silently
    harvested dlvry's seed column (891, 863, 792...) and reported 5 of 5 pairs differing.
    That is the same column collision `dlvry_predict.py` documents, arrived at from the
    other side -- so the shape of the table, not the shape of a row, has to do the work.
    """
    rows, want, in_table = [], arm is None, False
    for line in open(path):
        f = line.split()
        if arm is not None and len(f) == 2 and f[0] == "arm":
            want, in_table = f[1] == arm, False
            continue
        if f and f[0] == "tenth":
            rows, in_table = [], want
            continue
        if not in_table:
            continue
        if len(f) < 3 or not all(x.isdigit() for x in f[:3]) or int(f[0]) != len(rows) + 1:
            in_table = False                 # the table ended before ten rows
            continue
        rows.append((int(f[1]), int(f[2])))
        if len(rows) == 10:
            break
    return rows


if len(sys.argv) != 3:
    print("usage: silence_nest.py <dlvry log> <silence log>")
    sys.exit(2)

coarse = deciles(sys.argv[1])
fine = deciles(sys.argv[2], arm="ip")

if len(coarse) != 10 or len(fine) != 10:
    print("VOID: read %d coarse and %d fine tenths, wanted 10 of each" % (len(coarse), len(fine)))
    sys.exit(2)

print("silence's `ip` tenths summed in PAIRS against dlvry's first five tenths")
print("compared as INTEGER COUNTS -- an identity the shared trajectory requires\n")
print("  dlvry    nominal            delivered")
print("  tenth    paired  logged     paired  logged")

bad = 0
for q in range(5):
    pn = fine[2 * q][0] + fine[2 * q + 1][0]
    pd = fine[2 * q][1] + fine[2 * q + 1][1]
    ln, ld = coarse[q]
    ok = pn == ln and pd == ld
    bad += not ok
    print("  %-7d  %6d  %6d     %6d  %6d   %s"
          % (q + 1, pn, ln, pd, ld, "" if ok else "<-- DIFFERS"))

if bad:
    print("\nTHE NESTING FAILS on %d of 5 pairs. The two runs did not walk the same" % bad)
    print("trajectory, or one of them bins its tenths wrongly. Either way the cross-run")
    print("comparison of decay magnitudes is not available.")
    sys.exit(1)

print("\nTHE NESTING HOLDS on all 5 pairs, nominal and delivered.")
print("So `silence`'s -0.5919 and `dlvry`'s -0.4554 are the SAME curve read at two")
print("resolutions, and they differ only because a tenth of 4857 trials starts higher")
print("on a falling curve than a tenth of 9714 does. Neither is a separate measurement.")
