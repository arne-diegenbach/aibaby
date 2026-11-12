#!/usr/bin/env python3
"""§60's registered PER-CREATURE reading of the primary, from a `hush` log.

    python3 tools/hush_percreature.py results/n60-hush-900k.log

WHY THIS IS A SEPARATE TOOL AND NOT A SECOND RUN.

The registration promised the primary "per creature with a jackknife SE"
beside the pooled share, because probes within a creature are not independent
and a pooled share is dominated by the creatures with the most silent probes
(`mean-of-ratios`). The instrument prints the pooled reading; this derives the
per-creature one from the `HMAT` dump, with NO new compute.

That is the whole point of dumping cells rather than a scalar. §59c's
`repdrift` recorded only a whole-session number, so its 190 fresh creatures
cannot be re-windowed offline and a control it could have given for free needs
another run. §59a's `MAT` had it right and §60 follows it.

TWO PARSER RULES, BOTH PAID FOR ALREADY.

`dlvry_predict.py` v1 keyed rows by a bare leading integer, so a `tenth` column
1..10 silently OVERWROTE seeds 1..10 and it reported 10 of 12 creatures
mismatched while the log showed them agreeing. `silence_nest.py` v1 then made
the mirror-image error in a tool written AFTER that was documented. So:

  1. Rows are recognised by the `HMAT` prefix and validated against the header
     line's field names -- never by the shape of a row.
  2. A duplicate (arm, rep, eighth) key ABORTS. Silent overwriting is how both
     earlier bugs produced a plausible wrong answer.

AND THE SELF-CHECK. The pooled share recomputed from these cells must equal
the value the instrument printed for itself. The tool reports that comparison
and exits non-zero if it disagrees, so a parsing error cannot reach a result.
"""

import math
import re
import sys

# The header the instrument prints immediately above the rows. Checked rather
# than assumed, so a column added or reordered in C++ fails loudly here instead
# of shifting every value by one field.
WANT_HEADER = ("arm rep e probes kept dark nov sil quiet nofr "
               "sn thr pin rate ref vfrac amp").split()

FIELDS = ["arm", "rep", "e", "probes", "kept", "dark", "nov", "sil", "quiet",
          "nofr", "sn", "thr", "pin", "rate", "ref", "vfrac", "amp"]


def parse(path):
    cells = {}
    header_seen = False
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            tok = line.split()
            if not tok or tok[0] != "HMAT":
                continue
            rest = tok[1:]
            if not header_seen:
                if rest != WANT_HEADER:
                    sys.exit("HMAT header does not match this tool's columns.\n"
                             "  log:  %s\n  tool: %s" %
                             (" ".join(rest), " ".join(WANT_HEADER)))
                header_seen = True
                continue
            if len(rest) != len(FIELDS):
                sys.exit("HMAT row has %d fields, expected %d: %s" %
                         (len(rest), len(FIELDS), line.rstrip()))
            row = dict(zip(FIELDS, rest))
            key = (row["arm"], int(row["rep"]), int(row["e"]))
            if key in cells:
                sys.exit("DUPLICATE HMAT key %s -- refusing to overwrite. "
                         "Two runs redirected to one file would look like this." % (key,))
            cells[key] = row
    if not header_seen:
        sys.exit("no HMAT header found in %s -- is this a `hush` log?" % path)
    if not cells:
        sys.exit("HMAT header found but no rows in %s" % path)
    return cells


def jackknife(values):
    """SE of the MEAN of `values`, leave-one-out. Matches the instrument's own
    jackknife (and `silence`'s) rather than a closed-form SD/sqrt(n), so the
    two readings are comparable as estimators and not merely as numbers."""
    n = len(values)
    if n < 3:
        return float("nan")
    total = sum(values)
    loo = [(total - v) / (n - 1.0) for v in values]
    m = sum(loo) / n
    acc = sum((v - m) ** 2 for v in loo)
    return math.sqrt(acc * (n - 1.0) / n)


def window(cells, arm, lo, hi):
    """lo..hi inclusive, 1-based eighths -- the same convention the log prints."""
    per_creature, pooled_s, pooled_q, empty = [], 0, 0, 0
    reps = sorted({r for (a, r, _e) in cells if a == arm})
    for r in reps:
        s = q = 0
        for e in range(lo, hi + 1):
            row = cells.get((arm, r, e))
            if row is None:
                continue
            s += int(row["sil"])
            q += int(row["quiet"])
        pooled_s += s
        pooled_q += q
        if s + q == 0:
            empty += 1
        else:
            per_creature.append(q / float(s + q))
    return per_creature, pooled_s, pooled_q, empty, len(reps)


def report(cells, lo, hi, title):
    print("\n  %s  (eighths e%d-e%d)" % (title, lo, hi))
    print("    arm           creatures  silent-none   pooled q    per-creature q"
          "       jackknife SE")
    arms = sorted({a for (a, _r, _e) in cells}, key=lambda s: (len(s), s))
    for arm in arms:
        pc, s, q, empty, nreps = window(cells, arm, lo, hi)
        pooled = q / float(s + q) if (s + q) else float("nan")
        mean = sum(pc) / len(pc) if pc else float("nan")
        se = jackknife(pc)
        print("    %-12s %9d  %11d   %8.4f   %15.4f   %16.4f" %
              (arm, nreps, empty, pooled, mean, se))
    return arms


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    path = sys.argv[1]
    cells = parse(path)
    n_arms = len({a for (a, _r, _e) in cells})
    print("  %s" % path)
    print("  %d HMAT cells, %d arms, no duplicate keys." % (len(cells), n_arms))

    report(cells, 8, 8, "THE PRIMARY, per creature")
    report(cells, 1, 2, "THE CONTROL WINDOW, per creature")

    # --- the self-check ----------------------------------------------------
    # The pooled share derived here must equal the one the instrument printed.
    # A parsing error that shifted a column would still produce a plausible
    # share, so this compares against the log's own authority.
    print("\n  SELF-CHECK against the instrument's own printed pooled share:")
    want = {}
    in_primary = False
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            if "THE PRIMARY. q =" in line:
                in_primary = True
                continue
            if in_primary and line.strip().startswith("THE SAME QUESTION"):
                break
            m = re.match(r"\s+(T1\.\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+([0-9.]+|nan)\s", line)
            if in_primary and m:
                want[m.group(1)] = (int(m.group(2)), int(m.group(3)))
    if not want:
        sys.exit("  could not find the instrument's printed primary table -- "
                 "the self-check cannot be skipped, so this is a failure.")
    bad = 0
    for arm, (wsil, wquiet) in sorted(want.items()):
        _pc, s, q, _e, _n = window(cells, arm, 8, 8)
        ok = (s == wsil and q == wquiet)
        if not ok:
            bad += 1
        print("    %-12s HMAT sil/quiet %5d/%-5d   printed %5d/%-5d   %s" %
              (arm, s, q, wsil, wquiet, "ok" if ok else "*** DIFFERS ***"))
    if bad:
        sys.exit("\n  SELF-CHECK FAILED on %d arm(s) -- nothing above is read." % bad)
    print("    all arms agree -- the cells reproduce the instrument exactly.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
