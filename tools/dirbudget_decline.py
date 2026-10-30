# WHY THE ROTATION DECLINES at the top rung of `dirbudget`, read off the log that
# is already on disk. The ladder's per-seed table carries `direction`, `its null`,
# `trials` and `F1 spread` at every rung, so the obvious candidate explanations are
# testable for free -- before any rebuild.
#
# Four candidates for the -0.0244 +- 0.0092 top step:
#   (a) the null inflating      -- already decomposed, ~1/3 of it
#   (b) the magnitude eating it -- spread grows, assignment degrades: a TRADE-OFF
#   (c) the voice collapsing    -- spread FALLS on the creatures that lose direction
#   (d) the index degrading     -- ctx_match falls
# (b) and (c) make OPPOSITE predictions about the sign of corr(d_excess, d_spread),
# so one correlation separates them.
#
#   python3 tools/dirbudget_decline.py results/dirbudget.log
import sys


def fold(s):
    for a, b in (("its null", "its_null"), ("F1 spread", "F1_spread"),
                 ("F2 spread", "F2_spread")):
        s = s.replace(a, b)
    return s


COLS = ("trials", "direction", "its_null", "excess", "F1_spread", "F2_spread", "ctx")


def read(path):
    """{seed: {arm: {col: float}}} from the per-seed table."""
    idx, out = None, {}
    for line in open(path):
        f = fold(line).split()
        if len(f) > 4 and f[0] == "seed" and f[1] == "arm":
            idx = {n: i for i, n in enumerate(f)}
            continue
        if idx is None or len(f) < 9 or not f[0].isdigit():
            continue
        try:
            row = {c: float(f[idx[c]]) for c in COLS}
        except (KeyError, ValueError, IndexError):
            continue
        out.setdefault(int(f[0]), {})[f[1]] = row
    return out


def mean(v):
    return sum(v) / len(v)


def se(v):
    if len(v) < 2:
        return float("nan")
    m = mean(v)
    var = sum((x - m) ** 2 for x in v) / (len(v) - 1)
    return (var / len(v)) ** 0.5


def pearson(x, y):
    mx, my = mean(x), mean(y)
    sx = sum((a - mx) ** 2 for a in x) ** 0.5
    sy = sum((b - my) ** 2 for b in y) ** 0.5
    if sx == 0 or sy == 0:
        return float("nan")
    return sum((a - mx) * (b - my) for a, b in zip(x, y)) / (sx * sy)


if len(sys.argv) != 2:
    print("usage: dirbudget_decline.py <dirbudget log>")
    sys.exit(2)

tab = read(sys.argv[1])
arms = [a for a in ("3.4M", "6.8M", "13.6M", "27.2M") if any(a in v for v in tab.values())]
full = sorted(s for s, v in tab.items() if all(a in v for a in arms))
print("creatures with all %d rungs   %d" % (len(arms), len(full)))
print("rungs                        %s\n" % ", ".join(arms))

# THE BUDGET IS NOT THE TRIAL COUNT. Ticks per scored trial, rung by rung.
print("DOES THE BUDGET BUY TRIALS? ticks per scored trial, per rung")
ticks = {"3.4M": 3400000.0, "6.8M": 6800000.0, "13.6M": 13600000.0, "27.2M": 27200000.0}
base = None
for a in arms:
    tpt = [ticks[a] / tab[s][a]["trials"] for s in full if tab[s][a]["trials"] > 0]
    tr = [tab[s][a]["trials"] for s in full]
    if base is None:
        base = mean(tr)
    print("  %-7s trials %7.0f +- %-5.0f   ticks/trial %6.0f +- %-4.0f   x%.2f trials vs rung 1"
          % (a, mean(tr), se(tr), mean(tpt), se(tpt), mean(tr) / base))

# THE TOP STEP, per creature.
top, prev = arms[-1], arms[-2]
d = {c: [tab[s][top][c] - tab[s][prev][c] for s in full] for c in COLS}
print("\nTHE TOP STEP (%s -> %s), paired over %d creatures" % (prev, top, len(full)))
for c in ("excess", "direction", "its_null", "F1_spread", "ctx", "trials"):
    v = d[c]
    n_dn = sum(1 for x in v if x < 0)
    s = se(v)
    sig = abs(mean(v) / s) if s == s and s > 0 else float("nan")
    print("  d %-10s %+9.4f +- %-8.4f (%4.1f SE)   down on %d/%d"
          % (c, mean(v), s, sig, n_dn, len(v)))

# (b) vs (c): the sign of this correlation decides between them.
print("\nWHAT PREDICTS A CREATURE'S DIRECTION LOSS? Pearson r vs d excess (n=%d, SE~%.2f)"
      % (len(full), 1.0 / (len(full) - 3) ** 0.5))
for c in ("F1_spread", "ctx", "its_null", "trials", "F2_spread"):
    print("  r(d excess, d %-10s) %+.3f" % (c, pearson(d["excess"], d[c])))

# And the same split non-parametrically, since one collapsed voice could carry an r.
up = [i for i, x in enumerate(d["F1_spread"]) if x >= 0]
dn = [i for i, x in enumerate(d["F1_spread"]) if x < 0]
print("\nSPLIT BY WHETHER THE VOICE GREW OR SHRANK over the top step")
for name, g in (("spread grew", up), ("spread shrank", dn)):
    v = [d["excess"][i] for i in g]
    if not v:
        continue
    print("  %-14s n=%2d   d excess %+.4f +- %.4f   d spread %+6.2f Hz"
          % (name, len(g), mean(v), se(v), mean([d["F1_spread"][i] for i in g])))

# A voice too quiet to carry an assignment at all cannot show one.
for a in (arms[0], prev, top):
    q = [s for s in full if tab[s][a]["F1_spread"] < 20.0]
    print("\n  %-7s creatures under 20 Hz of F1 spread: %d/%d %s"
          % (a, len(q), len(full), sorted(q) if q else ""))

# A TWO-GROUP DIFFERENCE, because one collapsed voice could carry a Pearson r on 36.
if up and dn:
    a = [d["excess"][i] for i in up]
    b = [d["excess"][i] for i in dn]
    diff = mean(a) - mean(b)
    s = (se(a) ** 2 + se(b) ** 2) ** 0.5
    print("\n  grew MINUS shrank   %+.4f +- %.4f (%.1f SE)" % (diff, s, abs(diff / s)))
    print("  A POSITIVE sign means spread and direction move TOGETHER, which REFUTES a")
    print("  magnitude-eats-direction trade-off: the losers are the creatures whose")
    print("  voice SHRANK, not the ones whose voice grew.")

# THE LADDER'S X-AXIS. Nominal trials are ticks / kVLTrialTicks, and the column counts
# only SCORED trials that were VOICED and labelled -- so the gap is budget that bought
# no measurement at all.
print("\nDELIVERED FRACTION OF NOMINAL TRIALS (kVLTrialTicks = 2800)")
for a in arms:
    nom = ticks[a] / 2800.0
    got = mean([tab[s][a]["trials"] for s in full])
    print("  %-7s nominal %6.0f   delivered %6.0f   %5.1f%%" % (a, nom, got, 100.0 * got / nom))
