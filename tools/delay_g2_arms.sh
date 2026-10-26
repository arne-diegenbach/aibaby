#!/usr/bin/env bash
# g2 at n=36, paired, for a delay-ON and delay-OFF pair built from any base genome.
#
#   usage: tools/delay_g2_arms.sh <base.toml> <tag>
#
# Generalises tools/delay_g2_n36.sh, which was hard-wired to dna/default.toml. The
# widened genome (projection delays x4, axis 5-28 ticks instead of 2-7) needs the same
# test, and the two tags give a 2x2: narrow/wide against delays moving/frozen.
#
# PRE-REGISTERED, unchanged from the narrow run: the primary is the F1 MOTOR column,
# paired per creature, difference of differences (frozen minus moving), jackknifed,
# 3 SE bar. `rate` and `share` are secondaries. Four non-overlapping families of nine
# (offset 9*7919, so creature blocks cannot collide) make n=36.
set -u   # NOT -e: an experiment returning non-zero is DATA, not an error. set -e
         # killed the first wide run the moment family 3 read G2 FAIL, before it
         # could record the exit or start the off arm.
cd "$(dirname "$0")/.."
BASE_GENOME=$1
TAG=$2
BASE=20260809
STRIDE=$((9 * 7919))
for arm in on off; do
  frac=0.5; [ "$arm" = off ] && frac=1.0
  out="results/delay-g2-$TAG-$arm.log"
  : > "$out"
  for k in 0 1 2 3; do
    seed=$((BASE + k * STRIDE))
    g="results/delay-g2-$TAG-$arm-$seed.toml"
    sed -e "s/^seed = .*/seed = $seed/" \
        -e "s/^delay_floor_frac = .*/delay_floor_frac = $frac/" \
        "$BASE_GENOME" > "$g"
    echo "===== family $k seed $seed =====" >> "$out"
    ./build/aibaby --dna "$g" --experiment g2 --ticks 120000 >> "$out" 2>&1
    echo "[exit $?]" >> "$out"
    rm -f "$g"
  done
  echo "$TAG/$arm done: $out"
done
