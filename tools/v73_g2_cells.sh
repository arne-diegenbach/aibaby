#!/usr/bin/env bash
# DNA v73: does an UNEARNED spread on the refractory period cost the lesson, as the same
# spread on the threshold did?
#
#   control  dna/default.toml                refractory_sigma 0      calibrate PASS
#   s030     results/refrac-clean-030.toml    0.30, NOT retuned      calibrate PASS
#   s060     results/refrac-clean-060.toml    0.60, NOT retuned      calibrate PASS
#   s100     results/refrac-clean-100.toml    1.00, NOT retuned      calibrate FAIL (central, -0.28)
#
# s060 IS THE CELL THAT CAN ACTUALLY REFUSE THE MECHANISM, and it nearly went unbuilt.
# 0.30 is there to be like-for-like with v71's headline sigma, but 0.60 is the LARGEST
# spread that still leaves the operating point matched -- CV 0.655 against 0.320, 78% of
# neurons off the median, blackout spanning 0.5%-12% of ticks. If a refractory spread
# costs the lesson at any legal dose with a clean control, it costs it here.
#
# WHY THERE IS NO RETUNE, AND WHY THAT MAKES THIS CLEANER THAN v71. v71 needed a third
# cell because a log-normal threshold spread RAISES free-running rates, so holding the
# operating point required raising the median too and no two-cell design could separate
# spread from median. Here the spread does not move the operating point at all: `calibrate`
# PASSES un-retuned at 0.05, 0.15, 0.30 and 0.60, where the threshold broke it at 0.04.
# So control and s030 carry BYTE-IDENTICAL thresholds (verified) and differ in exactly one
# bit -- whether the refractory period is spread across neurons. v71 could not have that.
#
# THAT SAME FACT IS THE CAVEAT. Equal sigma is NOT equal dose. Measured before this ran:
# an 8x change in refractory_ms moves vocal 4.12 -> 3.47 Hz, an elasticity of -0.083, and
# the constant caps a neuron at 333 Hz while the creature runs at 4-7 Hz -- 46-81x below
# its own ceiling, controlling ~1.5% of each neuron's input. So a null here does NOT mean
# "this constant is irrelevant to the lesson"; it means a perturbation at least 15x weaker
# in sigma did less. The dose table is the headline, not this.
#
# PRE-REGISTERED, written before the first cell ran:
#   PRIMARY    the per-creature SLOPE of the F1 MOTOR column against sigma across the three
#              calibration-clean cells {0, 0.30, 0.60}, jackknifed over creatures, 3 SE.
#              A slope and not a single contrast for the reason v71's own dose-response
#              gave: every creature is measured at every sigma on identical wiring, so the
#              slope is the better-powered statistic AND it spends the multiplicity once
#              instead of inviting the best of three contrasts to be promoted afterwards,
#              which is precisely how `exsweep` manufactured an improvement that withdrew.
#   SECONDARY  s030 - control, the like-for-like row against v71's sigma 0.30; s060 -
#              control, the largest clean dose; and s100 - control, which is a BOUND and
#              not a cell -- s100 is off calibration on `central` by 0.03 Hz past
#              tolerance, so the invariant voids it as a matched comparison. Also
#              secondary: `rate`, `share`, praise-won counts, g2's own per-family verdicts.
#   PREDICTION the slope is indistinguishable from zero, and s030's cost is under 20% of
#              the control's own effect, against the threshold's -61% (3.5 SE) and the
#              leak's +0.0007 +/- 0.0070. If it instead reads anything like -61%, the
#              duty-cycle account is WRONG and a constant controlling 1.5% of each neuron's
#              input carries the lesson, which is worth far more than the parameter.
#   n=36 per cell as four non-overlapping families of nine (offset 9*7919), the same
#   families v71 and v72 used, so the control is directly comparable to both. Verified
#   before running: `tools/v73_g2_stats.py` reproduces nine published v71/v72 numbers
#   exactly, including both generations' controls at +0.0338 and 30/36.
set -u   # NOT -e: a non-zero exit from an experiment is DATA.
cd "$(dirname "$0")/.."
BASE=20260809
STRIDE=$((9 * 7919))
run_cell() {
  local tag=$1 genome=$2
  local out="results/v73-g2-$tag.log"
  : > "$out"
  for k in 0 1 2 3; do
    local seed=$((BASE + k * STRIDE))
    local g="results/v73-g2-$tag-$seed.toml"
    sed "s/^seed = .*/seed = $seed/" "$genome" > "$g"
    echo "===== family $k seed $seed =====" >> "$out"
    ./build/aibaby --dna "$g" --experiment g2 --ticks 120000 >> "$out" 2>&1
    echo "[exit $?]" >> "$out"
    rm -f "$g"
  done
  echo "$tag done: $out"
}
run_cell control dna/default.toml
run_cell s030    results/refrac-clean-030.toml
run_cell s060    results/refrac-clean-060.toml
run_cell s100    results/refrac-clean-100.toml
