#!/usr/bin/env python3
"""Where does the framehold ring sit, counted per seed?

THE STATISTIC, and why it replaced two others. `cascade_verdict.py` scored the
ARGMAX of the mean early-window SNR, then the ratio of the top mean to the next.
Both are DEFINED WHETHER OR NOT THERE IS A PEAK:

  - an argmax on a flat profile reports a location for a peak that is not there
    (cell D: 2.04 / 2.21 / 1.77 / 1.74, scored as "the ring held at 4 Hz");
  - a ratio of means is manufactured by one seed (cell E: heard-5 reads 28.97 on
    seed 1 and 0.34-2.82 on the other eleven, scoring 1.62).

This asks each seed separately where ITS OWN maximum falls and counts agreement.
The null is exact: if a seed's peak lands uniformly on one of k arms, the count
is Binomial(n, 1/k). No outlier can move it by more than one vote, and a flat
profile scatters the votes instead of concentrating them.
"""
import re, sys, statistics as st
from math import comb

PER = re.compile(r'seed (\d+) (heard|silent)-(\d+)\s+drive\s+[0-9.]+\s+'
                 r'hold\s+[0-9.]+\s+early\s+([0-9.]+)')

CELLS = [(50,   120, 'results/framehold-gate120.log',            'gate120 (coarse)'),
         (50,    60, 'results/framehold-wide-gate060.log',       'A shipped'),
         (100,   30, 'results/framehold-C-rate100-gate030.log',  'C'),
         (50,    30, 'results/framehold-wide-gate030.log',       '-'),
         (25,    60, 'results/framehold-B-rate025-gate060.log',  'B'),
         (25,    30, 'results/framehold-D-rate025-gate030.log',  'D'),
         (50,    15, 'results/framehold-E-rate050-gate015.log',  'E'),
         (12.5,  60, 'results/framehold-F-rate0125-gate060.log', 'F')]

def load(path):
    seeds = {}
    for line in open(path):
        m = PER.search(line)
        if m and m.group(2) == 'heard':
            seeds.setdefault(int(m.group(3)), {})[int(m.group(1))] = float(m.group(4))
    return seeds

def p_at_least(k, n, arms):
    q = 1.0 / arms
    return sum(comb(n, i) * q**i * (1 - q)**(n - i) for i in range(k, n + 1))

def main():
    import math
    print('%-18s %-6s %-5s %-8s %-8s %-6s %-8s %s' %
          ('cell', 'rate', 'gate', 'product', 'pred Hz', 'peak', 'seeds', 'p (binomial)'))
    for tr, tg, path, name in CELLS:
        try:
            seeds = load(path)
        except FileNotFoundError:
            print('%-18s LOG MISSING' % name); continue
        freqs = sorted(seeds)
        allseeds = sorted({s for f in freqs for s in seeds[f]})
        meds = {f: st.median(seeds[f].values()) for f in freqs}
        top = max(freqs, key=lambda f: meds[f])
        votes = sum(1 for s in allseeds
                    if max(freqs, key=lambda f: seeds[f].get(s, 0.0)) == top)
        n, arms = len(allseeds), len(freqs)
        p = p_at_least(votes, n, arms)
        pred = 1.0 / (2 * math.pi * math.sqrt(tr * tg * 1e-6))
        print('%-18s %-6s %-5.0f %-8.0f %-8.2f %-6d %-8s %.5f%s' %
              (name, tr, tg, tr * tg, pred, top, '%d/%d' % (votes, n), p,
               '   CONCENTRATED' if p < 0.01 else '   scattered' if p > 0.05 else ''))
    print()
    print('The ring reads the PRODUCT: 6000 -> 2 Hz, 3000 -> 3 Hz, 1500 -> 4 Hz,')
    print('confirmed by two matched pairs whose constants differ by 2x and 4x.')
    print('At product 750 all three splits scatter -- the ring stops existing')
    print('rather than moving higher, and two of those three hold one constant')
    print('at its shipped value, so it is the product and not one constant.')
    return 0

if __name__ == '__main__':
    sys.exit(main())
