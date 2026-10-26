#!/usr/bin/env bash
# Is the heterogeneity cost MONOTONE in sigma, or is there an optimum at small spread?
#
# At sigma 0.30 the pre-registered primary resolved AGAINST heterogeneity: F1 motor
# -0.0207 +/- 0.0059 (3.5 SE), a 61% loss of the praise effect, with praise won falling
# 30/36 -> 19/36 and g2's own verdict failing 2 of 4 families. The decomposition cleared
# the median (C - A null on all three columns) and put the effect on the SPREAD.
#
# "Heterogeneity is bad" and "too much heterogeneity is bad" are different findings, and
# only the second leaves anything to build on. So: a dose-response.
#
# Each sigma needs its OWN retune, because the spread's effect on free-running rate is
# what the median compensates -- a single retuned genome cannot serve two sigmas. The
# control is the shipped genome, already measured at n=36 as cell A.
#
# PRE-REGISTERED: primary stays the F1 MOTOR column, paired per creature against cell A,
# jackknifed. The dose-response claim is monotonicity across the three sigmas, not any
# single cell clearing 3 SE.
set -u   # NOT -e: a non-zero exit from an experiment is DATA.
cd "$(dirname "$0")/.."
BASE=20260809
STRIDE=$((9 * 7919))
for s in 0.05 0.15; do
  tag="s${s/./}"
  genome="results/het-retuned-$tag.toml"
  echo "=== retuning at sigma $s ==="
  python3 -I tools/het_retune.py "$s" threshold_sigma "$genome" || { echo "RETUNE FAILED at $s"; continue; }
  out="results/het-g2-$tag.log"
  : > "$out"
  for k in 0 1 2 3; do
    seed=$((BASE + k * STRIDE))
    g="results/het-g2-$tag-$seed.toml"
    sed "s/^seed = .*/seed = $seed/" "$genome" > "$g"
    echo "===== family $k seed $seed =====" >> "$out"
    ./build/aibaby --dna "$g" --experiment g2 --ticks 120000 >> "$out" 2>&1
    echo "[exit $?]" >> "$out"
    rm -f "$g"
  done
  echo "sigma $s done: $out"
done
