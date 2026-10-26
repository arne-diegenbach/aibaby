#!/usr/bin/env bash
# Tighten the g2 bound on the delay term from n=9 to n=36.
#
# At n=9 the paired difference was null at 0.1-0.3 SE, but the 2 SE BOUND was 40-60% of
# the praise effect -- loose enough that the delay term could change learning by a third
# and this test would miss it. A null is only as good as its bound (see aibaby-trim,
# where the same discipline put a 39% ceiling on the trim's naming null).
#
# g2's internal creatures are the genome seed plus r*7919 for r in 0..8, so a family
# offset of 9*7919 gives CONTIGUOUS, NON-OVERLAPPING blocks of nine: family k covers
# r = 9k .. 9k+8. Four families per arm is n=36.
#
# PRE-REGISTERED, before the run: the primary is the F1 MOTOR column, paired per
# creature, difference of differences (frozen minus moving), jackknifed, 3 SE bar.
# `rate` and `share` are reported as secondaries. The n=9 result stands as the
# discovery sample and is NOT pooled with this.
set -u
cd "$(dirname "$0")/.."
BASE=20260809
STRIDE=$((9 * 7919))
for arm in on off; do
  frac=0.5; [ "$arm" = off ] && frac=1.0
  out="results/delay-g2-$arm.log"
  : > "$out"
  for k in 0 1 2 3; do
    seed=$((BASE + k * STRIDE))
    g="results/delay-g2-$arm-$seed.toml"
    sed -e "s/^seed = .*/seed = $seed/" \
        -e "s/^delay_floor_frac = .*/delay_floor_frac = $frac/" \
        dna/default.toml > "$g"
    echo "===== family $k seed $seed =====" >> "$out"
    ./build/aibaby --dna "$g" --experiment g2 --ticks 120000 >> "$out" 2>&1
    echo "[exit $?]" >> "$out"
    rm -f "$g"
  done
  echo "$arm done: $out"
done
