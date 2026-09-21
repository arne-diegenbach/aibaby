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
# Usage:  tools/ctxgenome.sh [output path] [base genome]
# Default output is ./ctx.toml and the default base is dna/default.toml.
#
# THE BASE IS AN ARGUMENT so a FRESH SEED FAMILY can be built. Reseeding an existing
# ctx.toml by hand FAILS calibrate with three modules off target, because appending a
# module re-rolls the noise stream and the homeostatic targets are fitted to one
# draw -- the recalibration below is what makes a reseed legitimate, and editing the
# file directly skips it. On 2026-09-21 that cost a replication, which was then
# reported as "the script emits a stale template". It does not: it regenerates from
# the base every time and picks up new genome fields automatically. What it could not
# do was start from anything but default.toml.
set -euo pipefail
cd "$(dirname "$0")/.."
out="${1:-ctx.toml}"
base="${2:-dna/default.toml}"
if [ ! -f "$base" ]; then
  echo "base genome not found: $base" >&2
  exit 1
fi
python3 tools/genome_add_context.py "$base" "$out" vocal out_w=0
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

# RECALIBRATE EVERY STALE MODULE, rather than printing that they are stale.
#
# Appending a module re-rolls the per-neuron noise stream (see the
# appending-rerolls-noise note), which drifts every module's free-running rate.
# `calibrate` says exactly what to do about it -- "reset the genome to the left
# column" -- because the DECLARED target has gone stale, not the creature.
#
# THIS USED TO HANDLE `somato` ALONE, matching the literal target 4.56 because that
# value happened to be unique in the file. That works for one seed and fails for any
# other: on family 288559 it is `vision` and `expression` that drift, the genome fails
# its own calibration check, and every experiment needing it is blocked -- which is
# how a fresh-family replication was lost on 2026-09-21 and then misreported as "the
# script emits a stale template". It does not; it regenerates from the base every
# time and picks up new genome fields automatically.
#
# `calibrate` EXITS NONZERO when it fails -- which is the whole reason this block
# exists -- and under `set -euo pipefail` that kills the pipeline and the script with
# it, silently. The first version did exactly that: it printed "loads and runs" and
# then nothing at all. Hence the `|| true`.
calib="$(mktemp)"
./build/aibaby --dna "$out" --experiment calibrate --ticks 120000 > "$calib" 2>&1 || true
python3 tools/recalibrate_targets.py "$out" "$calib" || {
  echo "WARNING: a module could not be retargeted; see above" >&2
}
rm -f "$calib"

./build/aibaby --dna "$out" --experiment calibrate --ticks 120000 2>&1 |
  grep -E "STALE|by design|off target|calibrate (PASS|FAIL)" | sed 's/^/  calibrate: /' || true
