# THE FREE CONTROL ON `dirbudget`, and it is a bit-identity check rather than a
# comparison. `dirbudget`'s rung 1 at the spec minimum is argument-for-argument
# `echoname`'s taught arm -- same seed rule (header.seed + r*7919), same session call,
# same echo buffer, same 6800000 ticks -- so its `direction` and `its null` columns must
# reproduce the published ones EXACTLY, seed for seed. Not closely: exactly.
#
# That is worth more than it costs. A new instrument's real risk is that it has quietly
# changed the protocol, and no amount of reading the source settles that as well as the
# published column coming back unchanged through a different code path.
#
#   python3 tools/dirbudget_control.py results/echoname-sep-full.log <dirbudget log> 6.8M
import sys


def fold(s):
    # The tables use space-separated headers, so the two-word ones have to survive a split.
    for a, b in (("its EAR", "its_EAR"), ("aud WHILE", "aud_WHILE"),
                 ("its null", "its_null"), ("F1 spread", "F1_spread"),
                 ("F2 spread", "F2_spread")):
        s = s.replace(a, b)
    return s


def read(path, arm):
    """Per-seed (direction, its null) for one arm, keyed by seed, as the log PRINTS them."""
    idx, out = None, {}
    for line in open(path):
        f = fold(line).split()
        if len(f) > 4 and f[0] == "seed" and f[1] == "arm":
            idx = {n: i for i, n in enumerate(f)}
            continue
        if idx is None or len(f) < 4 or not f[0].isdigit() or f[1] != arm:
            continue
        if idx["direction"] >= len(f) or idx["its_null"] >= len(f):
            continue
        # Kept as STRINGS. Parsing to float and comparing with a tolerance would turn a
        # bit-identity check into a similarity check, which is not what licenses the run.
        out[int(f[0])] = (f[idx["direction"]], f[idx["its_null"]])
    return out


if len(sys.argv) != 4:
    print(__doc__ or "usage: dirbudget_control.py <echoname log> <dirbudget log> <rung arm>")
    sys.exit(2)

pub = read(sys.argv[1], "taught")
new = read(sys.argv[2], sys.argv[3])

if not pub:
    print("VOID: no `taught` rows in %s -- wrong log?" % sys.argv[1])
    sys.exit(2)
if not new:
    print("VOID: no `%s` rows in %s -- wrong rung name?" % (sys.argv[3], sys.argv[2]))
    sys.exit(2)

shared = sorted(set(pub) & set(new))
if not shared:
    print("VOID: the two logs share no seed")
    sys.exit(2)

bad = [s for s in shared if pub[s] != new[s]]
print("seeds compared   %d  (published %d, this run %d)" % (len(shared), len(pub), len(new)))
print("columns          direction, its null -- compared as PRINTED, not to a tolerance")
if bad:
    print("\nDIFFERS ON %d OF %d SEEDS. The ladder is VOID: rung 1 is not echoname's" % (len(bad), len(shared)))
    print("taught arm, so whatever the rungs measure, it is not the published quantity.")
    print("\n  seed   published (dir, null)      this run (dir, null)")
    for s in bad[:12]:
        print("  %-6d %-26s %s" % (s, "%s  %s" % pub[s], "%s  %s" % new[s]))
    if len(bad) > 12:
        print("  ... and %d more" % (len(bad) - 12))
    sys.exit(1)

if len(shared) < len(pub):
    print("\nIDENTICAL on all %d shared seeds, but the published log has %d." % (len(shared), len(pub)))
    print("A partial reproduction is a partial control: say so rather than rounding up.")
    sys.exit(0)

print("\nBIT-IDENTICAL on all %d seeds. Rung 1 IS echoname's taught arm, so the" % len(shared))
print("instrument measures the published quantity and the ladder is licensed.")
