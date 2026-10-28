# The paired SE on d log sep, which the experiment prints as a bare difference
# because its two terms are CORRELATED within a creature and their SEs cannot be
# combined. Computed per creature from the per-seed columns, where the pairing
# handles the correlation for free.
import sys, math
ARMS = ("fresh", "none", "taught", "nodrive", "yoked")
rows = {}
for line in open(sys.argv[1]):
    f = line.split()
    if len(f) >= 16 and f[0].isdigit() and f[1] in ARMS:
        try:
            # drive, within, between, sep are the last four fields
            rows.setdefault(f[1], {})[int(f[0])] = tuple(float(x) for x in f[-4:])
        except ValueError:
            pass
seeds = sorted(rows["none"])
def mse(v):
    n = len(v); m = sum(v) / n
    sd = math.sqrt(sum((x - m) ** 2 for x in v) / (n - 1))
    return m, sd / math.sqrt(n)
print(f"{'contrast':16} {'n':>3} {'d log sep':>18} {'SE u':>6}  {'sep ratio':>10}"
      f"  {'d log drive':>18} {'SE u':>6}")
for hi, lo in (("taught", "none"), ("yoked", "none"), ("taught", "yoked")):
    use = [s for s in seeds if rows[hi][s][3] > 0 and rows[lo][s][3] > 0
                            and rows[hi][s][0] > 0 and rows[lo][s][0] > 0]
    ds = [math.log(rows[hi][s][3] / rows[lo][s][3]) for s in use]
    dd = [math.log(rows[hi][s][0] / rows[lo][s][0]) for s in use]
    m, se = mse(ds); md, sed = mse(dd)
    print(f"{hi+' - '+lo:16} {len(use):3d} {m:+10.3f}+-{se:.3f} {abs(m)/se:6.1f} "
          f"{math.exp(m):9.2f}x  {md:+10.4f}+-{sed:.4f} {abs(md)/sed:6.1f}")
print("\nper-arm drive, as a fraction of the predicted 0.50:")
for a in ARMS:
    d = [rows[a][s][0] for s in seeds]
    m, se = mse(d)
    print(f"  {a:9} {m:.4f} +- {se:.4f}   {100*(m-0.5)/0.5:+6.2f}% off 0.50")
