#!/usr/bin/env python3
"""Which `Expect::kOpen` experiments can actually FAIL on their content?

THE DEFECT. `verify` decides PASS from the single bool `run_experiment` returns, and
`Expect::kOpen` prints "a milestone this project has never met just did -- READ THIS ONE"
on a true. The 2026-10-26 long-tier baseline printed that 21 times with ZERO regressions,
because those functions end in an unconditional `return true;`. An experiment that prints
a reading has no pass/fail content; filing it under kOpen makes the gate's loudest line
unfalsifiable.

THE CLASSIFIER READS ONLY TOP-LEVEL RETURNS -- indentation exactly 2, the function's own
control flow. A first version scanned every `return` in the body and called 79 of 80
"has a criterion", because it was picking up early-outs from nested lambdas and helper
closures; the returns that decide the verdict are the ones at the function's own level.

    {return true;}                 -> INSTRUMENT: cannot fail on content
    contains a `return <expr>;`    -> CRITERION: hands back something measured
    contains both true and false   -> CRITERION: the branch is the verdict
    anything else                  -> AMBIGUOUS, read by hand; guessing here would
                                      repeat the error being fixed
"""
import re
import sys

SRCS = ["host/src/experiments_milestones.cpp", "host/src/experiments.cpp"]
names = sorted(set(re.findall(r'\{"([a-z0-9_]+)", *[A-Za-z0-9_]+, *Expect::kOpen',
                              open("host/src/experiments.cpp", errors="replace").read())))
text = {p: open(p, errors="replace").read().split("\n") for p in SRCS}


def top_returns(name):
    pat = re.compile(r"^bool\s+run_%s\s*\(" % re.escape(name))
    for p, lines in text.items():
        for i, l in enumerate(lines):
            if pat.match(l):
                out = []
                for j in range(i + 1, len(lines)):
                    if lines[j] == "}":
                        return out
                    if re.match(r"^  return\b", lines[j]):
                        out.append(lines[j].strip())
    return None


inst, crit, amb, gone = [], [], [], []
for n in names:
    r = top_returns(n)
    if r is None:
        gone.append(n)
    elif r == ["return true;"]:
        inst.append(n)
    elif any(x != "return true;" and x != "return false;" for x in r):
        crit.append((n, [x for x in r if x not in ("return true;", "return false;")][:1]))
    elif "return true;" in r and "return false;" in r:
        crit.append((n, ["branches on both"]))
    else:
        amb.append((n, r))

print("%d kOpen specs, %d located\n" % (len(names), len(names) - len(gone)))
print("=== INSTRUMENT: only top-level return is an unconditional `return true;` (%d) ===" % len(inst))
print("   " + "  ".join(inst))
print("\n=== CRITERION: can fail on content (%d) ===" % len(crit))
for n, ex in crit:
    print("   %-13s %s" % (n, ex[0][:66]))
if amb:
    print("\n=== AMBIGUOUS, read by hand (%d) ===" % len(amb))
    for n, r in amb:
        print("   %-13s %s" % (n, r))
if gone:
    print("\n=== not located in these files (%d): %s" % (len(gone), ", ".join(gone)))
