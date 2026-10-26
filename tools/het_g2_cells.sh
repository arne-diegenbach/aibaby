#!/usr/bin/env bash
# g2 at n=36 for the three heterogeneity cells.
#
#   A  dna/default.toml              shipped: median m, sigma 0      calibrate PASS
#   B  results/het-thr-retuned.toml  median m', sigma 0.30           calibrate PASS
#   C  results/het-thr-median.toml   median m', sigma 0              calibrate FAIL (5)
#
# WHY THREE AND NOT TWO. A log-normal spread RAISES free-running rates (the drive-to-rate
# transfer is convex), so holding the operating point fixed REQUIRES raising the median
# threshold -- the median change is a consequence of the spread, not an independent knob.
# B vs A therefore differs in both, and no two-cell design can separate them: a cell with
# m' and no spread is off calibration by construction, which is exactly what C is.
#
#   B - A   spread + median, BOTH cells calibration-clean   <- the primary
#   C - A   median alone, and C is OFF CALIBRATION so this is a BOUND, not a clean cell
#   B - C   spread alone, at a mismatched operating point
#
# The two decompositions bracket the spread's contribution. Same logic as the
# tick-matched gating run, where matching experience un-matched reward and the pair
# bracketed the answer.
#
# PRE-REGISTERED: primary is the F1 MOTOR column, paired per creature, B - A, jackknifed,
# 3 SE. `rate` and `share` are secondaries, as is every comparison involving C. Four
# non-overlapping families of nine (offset 9*7919) make n=36.
set -u   # NOT -e: a non-zero exit from an experiment is DATA.
cd "$(dirname "$0")/.."
BASE=20260809
STRIDE=$((9 * 7919))
run_cell() {
  local tag=$1 genome=$2
  local out="results/het-g2-$tag.log"
  : > "$out"
  for k in 0 1 2 3; do
    local seed=$((BASE + k * STRIDE))
    local g="results/het-g2-$tag-$seed.toml"
    sed "s/^seed = .*/seed = $seed/" "$genome" > "$g"
    echo "===== family $k seed $seed =====" >> "$out"
    ./build/aibaby --dna "$g" --experiment g2 --ticks 120000 >> "$out" 2>&1
    echo "[exit $?]" >> "$out"
    rm -f "$g"
  done
  echo "$tag done: $out"
}
run_cell A dna/default.toml
run_cell B results/het-thr-retuned.toml
run_cell C results/het-thr-median.toml
