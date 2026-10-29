#!/usr/bin/env python3
"""Paired per-creature statistics for DNA v73's g2 cells.

Usage: v73_g2_stats.py <control.log>[@sigma] <test.log>[@sigma] [more...]

The first log is the control; every later one is contrasted against it. An optional
`@sigma` suffix tags a log with the spread it was run at, and when three or more logs carry
one the pre-registered TREND is reported as well.

THE PAIRING. Each cell runs the same four seed families in the same order, and a family's
seed fixes the wiring, so creature (family k, index i) in one cell is the SAME creature in
every other cell. The statistic is therefore a per-creature difference of differences:

    per creature   lesson = F1 motor(praised) - F1 motor(yoked)
    per cell pair  d = lesson(test) - lesson(control)

which removes both the creature and the yoked baseline. Reported as a jackknifed mean over
the 36 creatures, matching v71 and v72 so the three generations sit in one table.

THE TREND IS THE PRIMARY, not any single contrast. Every creature is measured at every
sigma on identical wiring, so a per-creature slope is both better powered and spends the
multiplicity once -- instead of inviting the best of three contrasts to be promoted
afterwards, which is exactly how `exsweep` manufactured an improvement that withdrew on
fresh seeds. v71's own dose-response switched to this statistic for the same reason.

VALIDATED BEFORE USE, against results this project has already published:

    v73_g2_stats.py results/het-g2-A.log@0 results/het-g2-s005.log@0.05 \\
                    results/het-g2-s015.log@0.15 results/het-g2-B.log@0.30

reproduces v71's contrasts (motor -0.0207 +/- 0.0059, -61%, praise won 30/36 -> 19/36) and
its trend (motor -0.0689 +/- 0.0183, 25/36 negative, -20% per 0.1 sigma); the v72 logs
reproduce the leak's +0.0007 +/- 0.0070 and -0.0014 +/- 0.0068. A reader that cannot
reproduce a published number is not a reader to measure a new one with.

WHAT THIS DOES NOT DO. It does not decide whether the comparison is MEANINGFUL. v73's
cells differ from v71's by at least 15x in delivered dose, and a null here is a statement
about this constant at a sigma that does not move the operating point, not about
heterogeneity in general. See `tools/refrac_leverage.py`.
"""
import math
import re
import sys

ROW = re.compile(r"^\s+(\d+)\s+praised\s+([-+0-9.]+) vs\s+([-+0-9.]+)\s+"
                 r"([-+0-9.]+) vs\s+([-+0-9.]+)\s+([-+0-9.]+) vs\s+([-+0-9.]+)\s+(praise|yoked)")
COLS = ("rate", "share", "motor")


def load(path):
    """-> {(family, index): {col: praised - yoked}}, plus the praise-won count."""
    out, won, family = {}, 0, -1
    for line in open(path):
        if line.startswith("===== family"):
            family = int(line.split()[2])
            continue
        m = ROW.match(line)
        if not m:
            continue
        vals = [float(v) for v in m.groups()[1:7]]
        out[(family, int(m.group(1)))] = {
            "rate": vals[0] - vals[1],
            "share": vals[2] - vals[3],
            "motor": vals[4] - vals[5],
        }
        # Group 8, not 7: groups 2..7 are the six numbers, so 7 is the motor YOKED value.
        # Keying on 7 parsed every number correctly and reported praise won 0/36, which is
        # impossible -- v71 recorded 30/36. The numbers being right is what let it survive
        # a first reading, and it is the fourth index off-by-one this project has paid for.
        if m.group(8) == "praise":
            won += 1
    return out, won


def jackknife(xs):
    """Mean and leave-one-out SE."""
    n = len(xs)
    if n < 2:
        return (xs[0] if xs else 0.0), 0.0
    total = sum(xs)
    loo = [(total - x) / (n - 1) for x in xs]
    lm = sum(loo) / n
    var = (n - 1) / n * sum((v - lm) ** 2 for v in loo)
    return total / n, math.sqrt(var)


def split(arg):
    """`path@sigma` -> (path, sigma or None)."""
    if "@" in arg:
        p, s = arg.rsplit("@", 1)
        return p, float(s)
    return arg, None


paths = [split(a) for a in sys.argv[1:]]
if not paths:
    sys.exit(__doc__)

ctrl_path, ctrl_sigma = paths[0]
ctrl, ctrl_won = load(ctrl_path)
if not ctrl:
    sys.exit("no creature rows parsed from %s" % ctrl_path)
print("control  %s   n=%d   praise won %d/%d" % (ctrl_path, len(ctrl), ctrl_won, len(ctrl)))
for col in COLS:
    m, se = jackknife([ctrl[k][col] for k in sorted(ctrl)])
    print("    %-6s %+.4f +/- %.4f" % (col, m, se))

loaded = [(ctrl_path, ctrl_sigma, ctrl)]
for path, sigma in paths[1:]:
    test, won = load(path)
    loaded.append((path, sigma, test))
    shared = sorted(set(ctrl) & set(test))
    print("\n%s   n=%d paired (of %d control, %d test)   praise won %d/%d"
          % (path, len(shared), len(ctrl), len(test), won, len(test)))
    if len(shared) != len(ctrl) or len(shared) != len(test):
        print("    WARNING: cells do not cover the same creatures -- the pairing is incomplete")
    for col in COLS:
        tm, _ = jackknife([test[k][col] for k in shared])
        cm, _ = jackknife([ctrl[k][col] for k in shared])
        d, se = jackknife([test[k][col] - ctrl[k][col] for k in shared])
        pct = 100.0 * d / cm if cm else float("nan")
        print("    %-6s %+.4f   d %+.4f +/- %.4f (%.1f SE)  %+.0f%% of control%s"
              % (col, tm, d, se, abs(d / se) if se else 0.0, pct,
                 "   <- like-for-like row" if col == "motor" else ""))
    neg = sum(1 for k in shared if test[k]["motor"] - ctrl[k]["motor"] < 0)
    print("    motor negative in %d of %d creatures" % (neg, len(shared)))

# ---- the pre-registered primary: a trend over the cells carrying a sigma tag ----
tagged = [(s, c) for _, s, c in loaded if s is not None]
if len(tagged) >= 3:
    tagged.sort()
    xs = [s for s, _ in tagged]
    mx = sum(xs) / len(xs)
    sxx = sum((x - mx) ** 2 for x in xs)
    common = sorted(set.intersection(*(set(c) for _, c in tagged)))
    print("\nPRE-REGISTERED PRIMARY: per-creature slope of F1 motor against refractory_sigma")
    print("across sigma %s, n=%d creatures on identical wiring" % (xs, len(common)))
    print("-" * 78)
    slopes = {}
    for col in COLS:
        acc = []
        for k in common:
            ys = [c[k][col] for _, c in tagged]
            my = sum(ys) / len(ys)
            acc.append(sum((x - mx) * (y - my) for x, y in zip(xs, ys)) / sxx)
        slopes[col] = acc
        m, se = jackknife(acc)
        print("  %-6s %+.4f +/- %.4f (%.1f SE)   negative in %d of %d%s"
              % (col, m, se, abs(m / se) if se else 0.0,
                 sum(1 for v in acc if v < 0), len(acc),
                 "   <- PRIMARY" if col == "motor" else ""))
    mslope, _ = jackknife(slopes["motor"])
    base, _ = jackknife([tagged[0][1][k]["motor"] for k in common])
    if base:
        print("\n  as a share of the control's own effect: %+.1f%% per 0.1 of sigma" %
              (100.0 * 0.1 * mslope / base))
        print("  v71's threshold spread, same statistic and unit:  -20%  per 0.1 of sigma")
