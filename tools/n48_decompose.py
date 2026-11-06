#!/usr/bin/env python3
"""§48 POST-HOC: separate the schedule from the budget.

The three pre-registered contrasts leave one decomposition implied but unmeasured.
SEQ-AXIS beat SIMUL by +5.44 +- 1.47 Hz (3.7 SE), but that arm differs from SIMUL
in TWO ways: it is sequential AND it got 3x the total teaching. SEQ-TOTAL, which
is sequential at MATCHED budget, was null against SIMUL (+0.83 +- 1.39).

So the available contrast is SEQ-AXIS - SEQ-TOTAL: same schedule, 3x the budget.
That isolates budget with schedule held fixed.

THIS IS POST-HOC. It was not pre-registered, it is computed on the same sample
that produced the verdict, and it is reported as a decomposition of an already
measured effect -- never as a verdict of its own. Paired by seed, jackknife SE,
matching the instrument's own estimator.

Usage: python3 -I tools/n48_decompose.py results/n48-f3seq.log
"""
import math
import re
import sys

ROW = re.compile(
    r"\[\s*\d+/\d+\]\s+seed\s+(\d+)\s+(\S+)\s+dF1\s+(-?[\d.]+)\s+dF2\s+(-?[\d.]+)\s+df3\s+(-?[\d.]+)"
)


def jackknife(vals):
    """Mean and jackknife SE -- the project's estimator, not the naive SEM."""
    n = len(vals)
    if n < 2:
        return (vals[0] if vals else 0.0), 0.0
    total = sum(vals)
    mean = total / n
    leave = [(total - v) / (n - 1) for v in vals]
    lm = sum(leave) / n
    var = (n - 1) / n * sum((x - lm) ** 2 for x in leave)
    return mean, math.sqrt(var)


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else "results/n48-f3seq.log"
    # per-arm, per-seed final-third values
    data = {}
    for line in open(path):
        m = ROW.search(line)
        if not m:
            continue
        seed, arm = int(m.group(1)), m.group(2)
        data.setdefault(arm, {})[seed] = tuple(float(m.group(i)) for i in (3, 4, 5))
    if not data:
        print("no per-seed rows found; is this the right log?")
        return 1

    print(f"arms found: {', '.join(f'{a} (n={len(v)})' for a, v in sorted(data.items()))}\n")

    def paired(a, b, axis, axis_name):
        """Paired difference on one axis, dropping the -1.0 'no levels' sentinel."""
        common = sorted(set(data.get(a, {})) & set(data.get(b, {})))
        d = [data[a][s][axis] - data[b][s][axis]
             for s in common
             if data[a][s][axis] >= 0.0 and data[b][s][axis] >= 0.0]
        if len(d) < 6:
            print(f"  {a:9s} - {b:9s} {axis_name}: n={len(d)}, too few to read")
            return
        m, se = jackknife(d)
        pos = sum(1 for x in d if x > 0)
        sigma = m / se if se > 0 else 0.0
        print(f"  {a:9s} - {b:9s} {axis_name}: {m:+7.2f} +- {se:5.2f} Hz "
              f"({sigma:+.1f} SE)  {pos}/{len(d)} positive")

    print("PRE-REGISTERED, recomputed from the per-seed rows as a cross-check on dF1:")
    for a, b in (("SEQ-TOTAL", "SIMUL"), ("SEQ-AXIS", "SIMUL"), ("SEQ-TOTAL", "STRETCH")):
        paired(a, b, 0, "dF1")

    print("\nPOST-HOC -- budget at MATCHED schedule (3x teaching, both sequential):")
    for axis, name in ((0, "dF1"), (1, "dF2"), (2, "df3")):
        paired("SEQ-AXIS", "SEQ-TOTAL", axis, name)

    print("\nPOST-HOC -- the 3x sequential arm against the 1.125x stretched arm:")
    paired("SEQ-AXIS", "STRETCH", 0, "dF1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
