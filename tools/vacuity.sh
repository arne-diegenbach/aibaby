#!/usr/bin/env bash
# Does this genome field DO anything in this configuration?
#
# Run this before every sweep. Twice in one day a field was swept for hours and
# turned out to be inert -- `meta_commit` read a variable that is zero under
# contexts, and `meta_floor` sits behind `meta_window = 0.0` which disables the
# whole gate. Both were invisible in the result: the arms simply agreed, which
# reads as "no effect" rather than "never executed".
#
# Two minutes against four hours. A field that changes nothing at smoke length
# will usually change nothing at full length. The exception is a CEILING, which is
# slack until something grows into it -- `perturb_max` reads dead here and is not
# inert, just not yet binding. So a DEAD verdict sends you to the code path, not
# straight to the bin.
#
#   tools/vacuity.sh <genome> <field> <lo> <hi> <experiment> [ticks]
#
# Exits non-zero if the two settings produce identical output.
set -u
G=$1; FIELD=$2; LO=$3; HI=$4; EXP=$5; TICKS=${6:-200000}
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT

# THE BINARY, AND WHY IT IS CHECKED RATHER THAN ASSUMED (2026-09-29).
# This script used to hardcode one build path. Run after building somewhere else
# -- build-par, say -- it tested a STALE binary that had never heard of the field,
# reported DEAD, and pointed at case (a), "find the gate". There was no gate. A
# guard that answers confidently off the wrong binary is worse than no guard, so
# the path is now overridable and is REFUSED when the binary cannot possibly know
# the field. Checked by looking for the field name INSIDE the binary, which is
# direct, rather than by comparing timestamps, which is circumstantial.
BIN=${AIBABY_BIN:-"./build/aibaby"}
if [ ! -x "$BIN" ]; then
  echo "REFUSED: no executable at ${BIN}. Set AIBABY_BIN to the binary you built."
  exit 2
fi
if ! strings "$BIN" | grep -qx -- "${FIELD}"; then
  echo "REFUSED: ${BIN} contains no reference to '${FIELD}'."
  echo "  That binary cannot read the field, so it would report DEAD whatever the"
  echo "  field does. Rebuild it, or point AIBABY_BIN at the build you just made."
  exit 2
fi
echo "  binary: ${BIN}"

n=$(grep -c "^${FIELD} *=" "$G")
if [ "$n" -ne 1 ]; then
  echo "REFUSED: '${FIELD}' appears ${n} times in ${G} (need exactly 1)."
  echo "  Zero means the field is absent and takes its DNA default -- sweeping the"
  echo "  file will do nothing. More than one is the shared-constant trap."
  exit 2
fi
echo "  field line: $(grep "^${FIELD} *=" "$G")"
for v in "$LO" "$HI"; do
  sed "s/^${FIELD} *=.*/${FIELD} = ${v}/" "$G" > "$T/g_$v.toml"
  "$BIN" --dna "$T/g_$v.toml" --experiment "$EXP" --ticks "$TICKS" \
      --allow-short > "$T/out_$v.txt" 2>/dev/null
done
if diff -q "$T/out_$LO.txt" "$T/out_$HI.txt" >/dev/null; then
  echo "  DEAD at ${TICKS} ticks: ${FIELD} = ${LO} and ${HI} give byte-identical output."
  echo
  echo "  This does NOT always mean the field is inert. Two cases, and they need"
  echo "  different responses:"
  echo "    (a) GATED OFF -- something upstream disables the path, as meta_window = 0"
  echo "        does to meta_floor. Find the gate; sweeping is pointless at any length."
  echo "    (b) NOT YET BINDING -- a CEILING that nothing has reached. perturb_max"
  echo "        reads dead at smoke length because the bias is at 28% of it, and it"
  echo "        would read dead at full length too until something grows into it."
  echo "    (c) THE BINARY PREDATES THE FIELD -- refused up front now, and the"
  echo "        case that actually happened on 2026-09-29."
  echo "  Tell them apart by reading the code path, not by running longer: a gated"
  echo "  field is unreachable, a ceiling is merely slack."
  exit 1
fi
echo "  LIVE: ${FIELD} changes ${EXP} output. $(diff "$T/out_$LO.txt" "$T/out_$HI.txt" | grep -c '^[<>]') lines differ."
