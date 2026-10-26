#!/usr/bin/env python3
"""Match a genome's operating point to the shipped creature's, by its median thresholds.

Generalises tools/het_retune.py, which was hard-wired to inserting a `threshold_sigma`
field. DNA v72's leak channel lives in the HEADER (`[exploration]`) rather than per
module, so the arm genome is built first and this only retunes it.

    usage: tools/retune_thresholds.py <in.toml> <out.toml>

WHY IT IS NEEDED HERE. The leak jitter disturbs free-running rates (`calibrate` FAIL on
2 modules: somato 4.45 -> 3.98, expression 7.24 -> 7.57), and the calibration invariant
voids any outcome comparison between arms that do not share an operating point.

AND WHY ONE RETUNE SERVES BOTH v72 ARMS. `calibrate` measures free-running rates with NO
REWARD, so the learning term never fires in it -- the jitter-only and learned arms return
identical calibration rates. That makes jitter-only an EXACTLY operating-point-matched
control for learned, differing in one bit: whether reward credits the jitter. v71 could
not have that, because there the spread itself caused the rate change and matching it
required moving the median, which is why that experiment needed three cells and a bracket.

Proportional feedback on all modules at once, because `calibrate` reports every module's
error in one run. `vocal` is off target BY DESIGN ("holds the duty cycle down") and is not
retuned. Moving `threshold` does not touch the divisive-norm setpoint
(`norm_target_[m] = target_rate_hz`), so this is not the fixed point the calibration
invariant warns about and it converges in one direction.
"""
import re
import subprocess
import sys

SRC, DST = sys.argv[1], sys.argv[2]
ITERS, GAIN = 14, 0.06
SKIP = {"vocal"}

src = open(SRC).read().split("\n")
mods, cur = [], None
for i, line in enumerate(src):
    m = re.match(r'^name\s*=\s*"([a-z]+)"', line)
    if m:
        cur = m.group(1)
    if re.match(r"^threshold\s*=", line) and cur:
        mods.append((cur, i))
        cur = None
thr = {n: float(re.match(r"^threshold\s*=\s*([0-9.]+)", src[i]).group(1)) for n, i in mods}
print("retuning %d module thresholds: %s -> %s\n" % (len(mods), SRC, DST))


def write():
    out = []
    for i, line in enumerate(src):
        hit = next((n for n, j in mods if j == i), None)
        out.append("threshold = %.5f" % thr[hit] if hit else line)
    open(DST, "w").write("\n".join(out))


for it in range(ITERS):
    write()
    r = subprocess.run(["./build/aibaby", "--dna", DST, "--experiment", "calibrate"],
                       capture_output=True, text=True)
    got = {}
    for line in r.stdout.split("\n"):
        m = re.match(r"\s+([a-z]+)\s+([0-9.]+) Hz\s+([0-9.]+) Hz", line)
        if m:
            got[m.group(1)] = (float(m.group(2)), float(m.group(3)))
    if not got:
        sys.exit("calibrate produced no module rows -- genome rejected?")
    worst = max((abs(v[0] - v[1]), n) for n, v in got.items() if n not in SKIP)
    ok = "calibrate PASS" in r.stdout
    print("  iter %2d  worst |err| %.3f Hz on %-11s %s"
          % (it, worst[0], worst[1], "calibrate PASS" if ok else ""))
    if ok:
        print("\nCONVERGED at iteration %d:" % it)
        for n, _ in mods:
            print("    %-11s %.5f%s" % (n, thr[n],
                                        "   (not retuned, off by design)" if n in SKIP else ""))
        sys.exit(0)
    for n, (free, target) in got.items():
        if n not in SKIP:
            thr[n] = max(0.05, thr[n] + GAIN * (free - target))

sys.exit("DID NOT CONVERGE in %d iterations -- report this rather than using the genome." % ITERS)
