#!/usr/bin/env bash
# Does the delay term change what the creature LEARNS?
#
# The operating point said no: paired across 6 seeds, turning delay movement off
# (delay_floor_frac 0.5 -> 1.0) moved every module's free-running rate by less than the
# seed-to-seed spread -- largest `vocal` +0.098 +/- 0.035 Hz against a seed sd of 0.682.
# But `calibrate` measures RESTING rates with regulation off, and the delay-learning
# literature's claim is about task performance, which is a learning quantity.
#
# So: the cheap learning and timing experiments, both arms, same genome seed so each
# experiment's internal creatures are PAIRED. Chosen for being cheap in the 2026-10-26
# baseline AND plausibly timing-sensitive: g2/m2 are the core learning milestones,
# imitate repeats a HEARD sound, turntake is explicitly about timing, eligprobe probes
# the plasticity trace the delay feeds.
#
# EVERY EXPERIMENT CARRIES ITS DECLARED MINIMUM FROM kSpecs. The first version of this
# script omitted --ticks and six of eight REFUSED at the default 120000, which is the
# exact failure `aibaby-smoke-the-exact-command` records: verifying the binary is not
# verifying the invocation, and smoking only the CHEAPEST experiment (m2, which has a
# low minimum) generalised from the one case that could not fail.
set -u
cd "$(dirname "$0")/.."

# name:minimum ticks, straight from kSpecs in host/src/experiments.cpp
BATTERY="g2:120000 m2:120000 eligprobe:600000 restate:600000 turntake:560000 \
         vocab:1200000 g4:1500000 imitate:560000"

for arm in "$@"; do
  out="results/delay-battery-$arm.log"
  : > "$out"
  for pair in $BATTERY; do
    e=${pair%%:*}; t=${pair##*:}
    echo "===== $e --ticks $t =====" >> "$out"
    ./build/aibaby --dna "results/delay-$arm.toml" --experiment "$e" --ticks "$t" \
      >> "$out" 2>&1
    echo "[exit $?]" >> "$out"
  done
  echo "$arm done: $out"
done
