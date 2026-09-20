#!/usr/bin/env bash
# Regenerate the standard context genome from dna/default.toml.
#
# WHY THIS EXISTS. Every experiment in the naming line needs a kContext module,
# built with NO output weight because the index is READ and never driven. That
# genome is generated from default.toml, so ADDING ANY GENOME FIELD STALES IT --
# and the loader then refuses every module by name, which is correct behaviour and
# costs a run launch each time. It happened three times on 2026-09-12 alone
# (replay_credit, pool_beta, ip_bias_gain).
#
# Usage:  tools/ctxgenome.sh [output path]
# Default output is the first argument or ./ctx.toml.
set -euo pipefail
cd "$(dirname "$0")/.."
out="${1:-ctx.toml}"
python3 tools/genome_add_context.py dna/default.toml "$out" vocal out_w=0
# Source 4 is the ear's rate EMA: the creature's own index, and the only source
# that has replicated out of sample. Every naming experiment expects it.
sed -i 's/^context_source = 0/context_source = 4/' "$out"
echo "wrote $out"
# A genome that does not load is the failure this script exists to prevent, so it
# checks rather than assuming.
# THE CHECK IS THAT IT LOADS, which is the failure this script exists to prevent.
# It is deliberately NOT "calibrate passes", because a context genome does not:
# appending a module re-rolls the per-neuron noise stream (see the
# appending-rerolls-noise note), which drifts every module's free-running rate a
# little, and `somato` lands ~0.4 Hz off its genome target. That has been true of
# every context-genome experiment in this project. somato is not in the vocal path
# so it is very probably harmless -- but the calibration invariant exists to stop
# "very probably", so the state is PRINTED rather than swallowed.
if ! ./build/aibaby --dna "$out" --experiment babble --ticks 120000 >/dev/null 2>&1; then
  echo "WARNING: $out was written but does not load or run. Run:" >&2
  echo "  ./build/aibaby --dna $out --experiment babble --ticks 120000" >&2
  exit 1
fi
echo "loads and runs"

# RECALIBRATE somato, rather than printing that it is stale and moving on.
#
# Appending a module re-rolls the per-neuron noise stream (see the
# appending-rerolls-noise note), which drifts every module's free-running rate.
# somato lands ~0.4 Hz below its declared target, and `calibrate` says exactly what
# to do about it: "reset the genome to the left column" -- the DECLARED target is
# what has gone stale, not the creature.
#
# This was left as a printed warning for a long time on the grounds that somato is
# not in the vocal path and so it is "very probably harmless". But the calibration
# invariant exists precisely to stop "very probably", and a genome that fails its own
# calibration check blocks every experiment that would need it -- which is how the
# context index ended up untestable.
#
# MEASURED, not hard-coded: the free-running rate is read back from calibrate and
# written in, so this stays correct as the genome changes instead of pinning 4.16.
# `calibrate` EXITS NONZERO when it fails -- which is the whole reason this block
# exists -- and under `set -euo pipefail` that kills the command substitution and the
# script with it, silently. The first version of this did exactly that: it printed
# "loads and runs" and then nothing at all.
somato_hz="$(./build/aibaby --dna "$out" --experiment calibrate --ticks 120000 2>&1 |
  awk '$1 == "somato" { print $2; exit }' || true)"
if [ -n "$somato_hz" ]; then
  before="$(grep -c '^target_rate_hz = 4.56' "$out" || true)"
  if [ "$before" = "1" ]; then
    sed -i "s/^target_rate_hz = 4.56\$/target_rate_hz = $somato_hz/" "$out"
    echo "recalibrated somato target 4.56 -> $somato_hz Hz (measured on this genome)"
  else
    echo "WARNING: somato target line not found exactly once ($before); left alone" >&2
  fi
fi

./build/aibaby --dna "$out" --experiment calibrate --ticks 120000 2>&1 |
  grep -E "STALE|by design|off target|calibrate (PASS|FAIL)" | sed 's/^/  calibrate: /' || true
