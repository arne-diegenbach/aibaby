#!/usr/bin/env python3
"""Inject `refractory_sigma` into every module of a genome and change NOTHING else.

WHY THIS EXISTS RATHER THAN REUSING `het_retune.py`. That tool injects the sigma field
AND retunes thresholds, and it writes the genome at the top of every iteration -- so when
it fails to converge it leaves behind a genome carrying however many iterations of
threshold feedback it managed before giving up. Reading that file as "the genome at this
sigma" silently measures a retuned creature and calls it an un-retuned one. That happened
here: the first sigma-1.00 row in this sweep was measured on a genome with fourteen
iterations baked in, and it disagreed with the clean measurement.

The dose question -- does the spread move the operating point at all? -- can only be
asked of a genome whose thresholds are the shipped ones. So this does the injection and
nothing else, and asserts the insertion count, because a sweep whose regex silently
misses measures the same genome N times and reports a flat line.

Usage: refrac_sigma_genome.py <sigma> <out.toml> [in.toml]
"""
import re
import sys

SIGMA = float(sys.argv[1])
DST = sys.argv[2]
SRC = sys.argv[3] if len(sys.argv) > 3 else "dna/default.toml"
FIELD = "refractory_sigma"

src = open(SRC).read().split("\n")
if any(re.match(r"^%s\s*=" % FIELD, ln) for ln in src):
    sys.exit("%s already present in %s -- refusing to inject it twice" % (FIELD, SRC))

# Same insertion point `het_retune.py` uses, so the two tools produce the same layout:
# one field per module block, right after its `leak_tau_ms`.
out = []
inserted = 0
for line in src:
    out.append(line)
    if re.match(r"^leak_tau_ms\s*=", line):
        out.append("%s = %.4f" % (FIELD, SIGMA))
        inserted += 1

modules = sum(1 for ln in src if ln.strip() == "[[module]]")
assert inserted == modules, "inserted %d but the genome has %d modules" % (inserted, modules)

open(DST, "w").write("\n".join(out))
print("wrote %s: %s = %.4f in %d of %d modules, thresholds untouched"
      % (DST, FIELD, SIGMA, inserted, modules))
