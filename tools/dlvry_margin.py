# WHAT THE DELIVERY CURVE DOES AND DOES NOT EXPLAIN.
#
# `dlvry` establishes that delivery decays within a session: 0.716 -> 0.261 across the
# tenths, 19.8 SE, 12 of 12 creatures down, and every non-delivered trial is SILENT.
# That accounts for the ladder's falling delivered FRACTION, which is one decaying curve
# averaged over ever longer windows.
#
# It does not follow that it accounts for the ROTATION's decline. That decline is a step
# between two particular rungs, so what matters there is not the session mean but the
# MARGINAL delivery rate in that window -- the trials the extra budget actually bought.
# If delivery has already bottomed out before the top step, the step is not starved and
# the decline needs a different explanation.
#
#   python3 tools/dlvry_margin.py results/dlvry.log
import sys


def read(path):
    """(arms, trial counts, {seed: [prefix per arm]}, [(nominal, kept) per tenth])"""
    arms, nominal, pre, tenths = None, None, {}, []
    for line in open(path):
        f = line.split()
        if len(f) > 2 and f[0] == "seed" and f[-2:] == ["nominal", "was"]:
            arms = f[1:-2]
            continue
        if arms and len(f) == len(arms) + 2 and f[0].isdigit() and "/" in f[-1]:
            pre[int(f[0])] = [int(v) for v in f[1:1 + len(arms)]]
            nominal = [int(v) for v in f[-1].split("/")]
            continue
        # tenth nominal delivered silent degenerate frac
        if len(f) == 6 and f[0].isdigit() and all(x.isdigit() for x in f[1:5]):
            tenths.append((int(f[1]), int(f[2]), int(f[3]), int(f[4])))
    return arms, nominal, pre, tenths


if len(sys.argv) != 2:
    print("usage: dlvry_margin.py <dlvry log>")
    sys.exit(2)

arms, nominal, pre, tenths = read(sys.argv[1])
if not arms or not pre or not nominal:
    print("VOID: no prefix table in %s" % sys.argv[1])
    sys.exit(2)

n = len(pre)
tot = [sum(pre[s][q] for s in pre) for q in range(len(arms))]
print("creatures %d   rungs %s" % (n, ", ".join(arms)))
print("\nDELIVERED FRACTION AT EACH BUDGET, this run's own creatures")
print("  rung     nominal   delivered   fraction")
for q, a in enumerate(arms):
    print("  %-8s %7d   %9.1f   %8.3f" % (a, nominal[q], tot[q] / n, tot[q] / n / nominal[q]))

# THE NUMBER THE TOP STEP TURNS ON.
print("\nMARGINAL DELIVERY, the trials each doubling actually bought")
print("  step                 extra nominal   extra delivered   marginal rate")
prev_nom, prev_got = 0, 0.0
for q, a in enumerate(arms):
    got = tot[q] / n
    dn, dg = nominal[q] - prev_nom, got - prev_got
    label = ("0 -> %s" % a) if q == 0 else ("%s -> %s" % (arms[q - 1], a))
    print("  %-20s %13d   %15.1f   %13.3f" % (label, dn, dg, dg / dn if dn else 0.0))
    prev_nom, prev_got = nominal[q], got

# Which tenths each rung's window covers, so the curve can be read against the step.
if tenths:
    per = nominal[-1] / float(len(tenths))
    print("\nWHICH TENTHS EACH WINDOW COVERS (a tenth is %.0f trials)" % per)
    prev = 0
    for q, a in enumerate(arms):
        print("  up to %-8s trials 0..%-5d = tenths %.2f .. %.2f"
              % (a, nominal[q], prev / per, nominal[q] / per))
        prev = nominal[q]
    lo = nominal[-2] / per
    sel = [t for i, t in enumerate(tenths) if i + 1 > lo]
    if sel:
        nm = sum(t[0] for t in sel)
        kp = sum(t[1] for t in sel)
        print("\n  THE TOP STEP'S OWN WINDOW, tenths %d..%d pooled:" % (int(lo) + 1, len(tenths)))
        print("    delivered fraction %.3f   first %.3f  last %.3f"
              % (kp / nm, sel[0][1] / sel[0][0], sel[-1][1] / sel[-1][0]))
        trend = sel[-1][1] / sel[-1][0] - sel[0][1] / sel[0][0]
        print("    trend across that window %+.3f -- %s"
              % (trend, "STILL FALLING" if trend < -0.01 else
                        ("RISING" if trend > 0.01 else "FLAT")))

# Silence is the only failure mode, so say whether the other counter ever fired.
if tenths:
    deg = sum(t[3] for t in tenths)
    sil = sum(t[2] for t in tenths)
    nm = sum(t[0] for t in tenths)
    kp = sum(t[1] for t in tenths)
    print("\nFAILURE MODES over the whole session")
    print("  nominal %d = delivered %d + silent %d + degenerate %d  (residual %d)"
          % (nm, kp, sil, deg, nm - kp - sil - deg))
    if deg == 0:
        print("  DEGENERATE NEVER FIRES. Under kVLScoreFormant a trial is only scored when")
        print("  n_voiced > 0, and a voiced window's mean formants are not <= 1 Hz, so this")
        print("  counter is STRUCTURALLY INERT here rather than informatively zero. It is")
        print("  kept because the rate and amplitude score modes can reach it.")
