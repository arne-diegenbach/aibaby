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
./build/aibaby --dna "$out" --experiment calibrate --ticks 120000 2>&1 |
  grep -E "STALE|by design|off target" | sed 's/^/  calibrate: /' || true
