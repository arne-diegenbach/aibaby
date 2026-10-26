#!/usr/bin/env bash
# How much per-neuron heterogeneity can this creature actually CARRY?
#
# sigma 0.3 was chosen from Perez-Nieves's learned distributions and it FAILS
# calibration hard: central 6.60 -> 10.16 Hz, vocal 4.12 -> 10.15 Hz. A log-normal
# spread raises the MEAN threshold only 4.6% at that sigma (exp(s^2/2)), so the blow-up
# is not the mean -- it is that the rate response to threshold is CONVEX, and the
# neurons whose thresholds fell fire far more than the ones whose rose fire less, with
# recurrent excitation amplifying the asymmetry.
#
# That makes the comparison VOID until the arms share an operating point (the
# calibration invariant). Rather than retuning six thresholds per arm, DERIVE the usable
# sigma from the creature: sweep it and find where `calibrate` stops passing. If the
# answer is "almost none", that is itself the finding -- this project already has
# homeostasis on record as the named opponent of differentiation.
set -u
cd "$(dirname "$0")/.."
FIELD=$1          # threshold_sigma | leak_tau_sigma
shift
printf "%-16s %-9s %s\n" "$FIELD" "verdict" "free-running rates (central/auditory/vision/somato/vocal/expression)"
for s in "$@"; do
  g="results/het-sweep.toml"
  awk -v S="$s" -v F="$FIELD" '
    { print }
    /^leak_tau_ms[ \t]*=/ { printf "%s = %s\n", F, S }
  ' dna/default.toml > "$g"
  line=$(./build/aibaby --dna "$g" --experiment calibrate 2>/dev/null)
  v=$(echo "$line" | grep -oE "calibrate (PASS|FAIL) — [0-9]+ modules?" | sed -E 's/calibrate //; s/ modules?//')
  rates=$(echo "$line" | awk '$1 ~ /^(central|auditory|vision|somato|vocal|expression)$/ {printf "%s ", $2}')
  printf "  sigma %-10s %-9s %s\n" "$s" "$v" "$rates"
  rm -f "$g"
done
