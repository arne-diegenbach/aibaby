#!/usr/bin/env python3
"""Why did the NO-INDEX arm improve under gating? Tested on columns already in both logs.

The gated run's headline fall turned out to be the control arm RISING (+0.0731, 3.3 SE)
rather than indexed naming falling. That rise needs an account, and one was offered: gating
changes the OUTPUT DISTRIBUTION so that raw target-matching improves with no conditioning --
a narrower or better-centred voice would do it.

That is testable without a run. The per-creature table carries, per arm:

    F1 spread, F2 spread   mean absolute pairwise separation of the four words, per axis
    ctx_match              how well the index tracks the word
    direction              the directional naming score
    dir-null               the same against its own null
    nearest                the four-way nearest-target score

THE PREDICTION, if the account is right: the no-index arm's SPREAD should FALL under gating
(a narrower voice) while its `nearest` RISES. A spread that does not move would refuse the
account and leave the rise unexplained.

Everything is paired: gating changes no wiring, so creature r is the same creature in both
runs.
"""
import math
import re

COLS = ["f1_spread", "f2_spread", "ctx_match", "direction", "dir_null", "nearest"]


def read(path):
    out = {}
    for line in open(path):
        m = re.match(r"\s+(\d+)\s+(off|oracle)\s+(.*)$", line)
        if not m:
            continue
        vals = [float(x) for x in m.group(3).split()]
        if len(vals) != len(COLS):
            continue
        out.setdefault(int(m.group(1)), {})[m.group(2)] = dict(zip(COLS, vals))
    return {s: v for s, v in out.items() if len(v) == 2}


def jack(v):
    n = len(v)
    if n < 3:
        return 0.0, 0.0
    tot = sum(v)
    th = [(tot - x) / (n - 1) for x in v]
    mb = sum(th) / n
    return tot / n, math.sqrt(sum((x - mb) ** 2 for x in th) * (n - 1) / n)


g = read("results/ctxgrid-trim-gated.log")
u = read("results/ctxgrid-trim.log")
sh = sorted(set(g) & set(u))
print("paired on %d creatures (gating changes no wiring, so these are the same creatures)\n"
      % len(sh))

for arm in ("off", "oracle"):
    print("  %s ARM" % arm.upper())
    print("    %-11s %9s %9s   %-22s" % ("column", "ungated", "gated", "paired change"))
    for c in COLS:
        gv = [g[s][arm][c] for s in sh]
        uv = [u[s][arm][c] for s in sh]
        mg, _ = jack(gv)
        mu, _ = jack(uv)
        d, se = jack([a - b for a, b in zip(gv, uv)])
        nse = abs(d) / se if se else 0.0
        mark = "  <== RESOLVED" if nse >= 3.0 else ""
        print("    %-11s %9.4f %9.4f   %+8.4f +/- %.4f (%.1f SE)%s"
              % (c, mu, mg, d, se, nse, mark))
    print()

print("READING THE PREDICTION:")
print("  the account says the no-index arm's SPREAD should FALL while its `nearest` RISES.")
print("  A spread that does not move refuses the account and leaves the rise unexplained.")
