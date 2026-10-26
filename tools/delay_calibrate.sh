#!/usr/bin/env bash
# Does turning the delay term OFF move the creature's operating point further than
# changing its wiring seed does?
#
# `delay_floor_frac = 1.0` makes move_delays false, so delays stay at their birth
# values; the shipped 0.5 lets a busy axon halve its delay. A single-seed comparison
# showed module rates shifting by up to 0.18 Hz with MIXED signs, which is unreadable
# without a noise floor -- seed-to-seed spread could be larger than that.
#
# So: the same seeds in both arms, paired. Seeds follow this project's convention,
# base + r*7919. Each arm is one `calibrate` run, which measures free-running rates
# with ip_rate and scaling_rate at 0, so nothing is regulating them back.
set -u
cd "$(dirname "$0")/.."
BASE=20260809
REPS=${1:-6}
OUT=${2:-/tmp/delay_calib.tsv}
: > "$OUT"
for r in $(seq 0 $((REPS - 1))); do
  seed=$((BASE + r * 7919))
  for arm in on off; do
    frac=0.5; [ "$arm" = off ] && frac=1.0
    g="results/delay-cal-$arm-$seed.toml"
    sed -e "s/^seed = .*/seed = $seed/" \
        -e "s/^delay_floor_frac = .*/delay_floor_frac = $frac/" \
        dna/default.toml > "$g"
    ./build/aibaby --dna "$g" --experiment calibrate 2>/dev/null |
      awk -v S="$seed" -v A="$arm" '
        /^  (central|auditory|vision|somato|vocal|expression) / {
          printf "%s\t%s\t%s\t%s\n", S, A, $1, $2 }' >> "$OUT"
    rm -f "$g"
  done
done
echo "wrote $OUT: $(wc -l < "$OUT") rows"
