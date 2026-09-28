#!/usr/bin/env python3
"""Apply the pre-registered cascade test to a set of framehold logs.

Written BEFORE arms B, C and D were run (see results/framehold-cascade-prereg.md).
The verdict is a function of the logs, not of my reading of them.

Primary statistic: the EARLY-window SNR argmax over the `heard` arms -- the same
column the gate_smoothing_ms sweep was read on.
"""
import re, sys, math

# (tau_rate, tau_gate, predicted argmax set, log path). The two HAVE cells were
# run before the hypothesis existed and are baselines, not evidence.
CELLS = [
    (50.0,  120.0, {2},    'results/framehold-gate120.log',            'baseline'),
    (50.0,   60.0, {3},    'results/framehold-wide-gate060.log',       'baseline'),
    (50.0,   30.0, {4},    'results/framehold-wide-gate030.log',       'baseline'),
    (25.0,   60.0, {4},    'results/framehold-B-rate025-gate060.log',  'test'),
    (100.0,  30.0, {3},    'results/framehold-C-rate100-gate030.log',  'test'),
    (25.0,   30.0, {5, 6}, 'results/framehold-D-rate025-gate030.log',  'test'),
]

ROW = re.compile(r'^\s+heard-(\d+)\s+([0-9.]+)\s+([0-9.]+)\s+\+/-\s+([0-9.]+)\s+'
                 r'([0-9.]+)\s+\+/-\s+([0-9.]+)')

def read_early(path):
    """{freq: (early_snr, early_se)} from a framehold summary block."""
    out = {}
    try:
        text = open(path).read()
    except FileNotFoundError:
        return None
    for line in text.splitlines():
        m = ROW.match(line)
        if m:
            f = int(m.group(1))
            out[f] = (float(m.group(5)), float(m.group(6)))
    return out or None

def predicted(tr, tg):
    return 1.0 / (2 * math.pi * math.sqrt(tr * tg * 1e-6))

def peak_ratio(early):
    """Peak divided by the next-highest arm.

    ADDED AFTER SEEING THE DATA, and labelled so. The pre-registered statistic
    was the argmax alone, and an argmax is DEFINED on a flat profile -- it
    reports a location for a peak that is not there. Cell D exposed that: its
    argmax is 4 and its profile runs 2.04 / 2.21 / 1.77 / 1.74 across four
    arms, which is not a peak at any location.

    Reported as a QUANTITY and not against a threshold. A cut placed here after
    the fact would sit between B (1.49) and the rest (1.9-2.0), which is a
    threshold placed where the data is -- the thing this project already
    refuses. The numbers are given so the reader draws their own line.
    """
    order = sorted(early, key=lambda f: -early[f][0])
    return early[order[0]][0] / early[order[1]][0], order[1]

def main():
    print('%-8s %-9s %-9s %-9s %-7s %-7s %s' %
          ('cell', 'tau_rate', 'tau_gate', 'predicted', 'expect', 'argmax', 'verdict'))
    verdicts = []
    for tr, tg, expect, path, kind in CELLS:
        early = read_early(path)
        if early is None:
            print('%-8s %-9.0f %-9.0f %-9.2f %-7s %-7s LOG MISSING' %
                  (kind, tr, tg, predicted(tr, tg), sorted(expect), '--'))
            verdicts.append(None)
            continue
        arg = max(early, key=lambda f: early[f][0])
        ok = arg in expect
        ratio, runner = peak_ratio(early)
        print('%-8s %-9.0f %-9.0f %-9.2f %-7s %-7d %-14s (early %.2f +/- %.2f, '
              'peak/next %.2f over %d Hz)' %
              (kind, tr, tg, predicted(tr, tg), ','.join(map(str, sorted(expect))),
               arg, 'as predicted' if ok else 'REFUSES', early[arg][0], early[arg][1],
               ratio, runner))
        if kind == 'test':
            verdicts.append(ok)

    print()
    if any(v is None for v in verdicts) or len(verdicts) < 3:
        print('INCOMPLETE -- not every test cell has a log; no verdict.')
        return 2
    if all(verdicts):
        print('ALL THREE TEST CELLS LAND WHERE THEY WERE PREDICTED.')
        print('The decisive one is C: both of its constants moved, each alone')
        print('moves the argmax to 4, and their product is unchanged -- so a')
        print('return to 3 is not something a generic perturbation can produce.')
        print()
        print('DO NOT quote the absolute 2.91 Hz agreement; see the pre-registration.')
        return 0
    print('REFUSED AS PRE-REGISTERED. All three test cells were required to land')
    print('and D did not. Read the peak/next column before reading the argmaxes:')
    print()
    print('  product 3000   (50,60) peak 3  and  (100,30) peak 3   ratios 1.91, 1.95')
    print('  product 1500   (50,30) peak 4  and  (25,60)  peak 4   ratios 1.98, 1.49')
    print('  product  750   (25,30) argmax 4                       ratio  1.08')
    print()
    print('D did not move the ring to 6 and did not hold it at 4 -- at a ratio of')
    print('1.08 there is NO PEAK to locate. The ring did not move; it vanished.')
    print('So the product law is supported by two matched pairs whose constants')
    print('differ by 2x and 4x, and refused as an extrapolation past product 1500.')
    print('The ceiling is NOT part of the pre-registered hypothesis and is a new')
    print('claim needing its own test -- see the open lead.')
    return 1

if __name__ == '__main__':
    sys.exit(main())
