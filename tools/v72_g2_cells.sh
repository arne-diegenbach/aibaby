#!/usr/bin/env bash
# DNA v72's three cells: does LEARNING the leak beat merely JITTERING it?
#
#   control  dna/default.toml         leak_perturb 0                 calibrate PASS
#   jitter   results/v72-jitter.toml  0.15, rate 0,   retuned        calibrate PASS
#   learned  results/v72-learned.toml 0.15, rate 0.1, SAME retune    calibrate PASS
#
# The learning rate is DERIVED, not guessed: `leakprobe` swept it over five decades and
# 0.1 is the largest rate at which the learned offset still grows in PROPORTION to the
# rate (linearity 0.981 against 1.000 and 1.024 at the two rates below it) without pinning
# against its ceiling. The bias-channel analogy would have given ~0.001, a hundred times
# smaller, which is why the analogy was refused.
#
# WHY THIS DESIGN HAS NO CONFOUND. `calibrate` measures free-running rates with NO REWARD,
# so the learning term never fires in it -- jitter and learned return IDENTICAL calibration
# rates (7.05 2.41 2.46 4.62 4.31 7.04, both). One retune therefore serves both, and the
# two arms differ in exactly one bit: whether reward credits the jitter. v71 could not have
# that, because there the spread itself caused the rate change.
#
# PRE-REGISTERED: primary is the F1 MOTOR column, paired per creature, LEARNED - JITTER,
# jackknifed, 3 SE. `rate` and `share` are secondaries, as is anything against `control`
# -- the retuned arms sit up to 0.45 Hz from the shipped operating point, so only
# learned-minus-jitter is exactly matched. n=36 as four non-overlapping families of nine
# (offset 9*7919).
set -u   # NOT -e: a non-zero exit from an experiment is DATA.
cd "$(dirname "$0")/.."
BASE=20260809
STRIDE=$((9 * 7919))
run_cell() {
  local tag=$1 genome=$2
  local out="results/v72-g2-$tag.log"
  : > "$out"
  for k in 0 1 2 3; do
    local seed=$((BASE + k * STRIDE))
    local g="results/v72-g2-$tag-$seed.toml"
    sed "s/^seed = .*/seed = $seed/" "$genome" > "$g"
    echo "===== family $k seed $seed =====" >> "$out"
    ./build/aibaby --dna "$g" --experiment g2 --ticks 120000 >> "$out" 2>&1
    echo "[exit $?]" >> "$out"
    rm -f "$g"
  done
  echo "$tag done: $out"
}
run_cell control dna/default.toml
run_cell jitter  results/v72-jitter.toml
run_cell learned results/v72-learned.toml
