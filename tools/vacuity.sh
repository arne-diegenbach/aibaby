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
# will change nothing at full length, and the converse failure -- a field that
# matters only after millions of ticks -- is far rarer than a dead one.
#
#   tools/vacuity.sh <genome> <field> <lo> <hi> <experiment> [ticks]
#
# Exits non-zero if the two settings produce identical output.
set -u
G=$1; FIELD=$2; LO=$3; HI=$4; EXP=$5; TICKS=${6:-200000}
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT

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
  ./build/aibaby --dna "$T/g_$v.toml" --experiment "$EXP" --ticks "$TICKS" \
      --allow-short > "$T/out_$v.txt" 2>/dev/null
done
if diff -q "$T/out_$LO.txt" "$T/out_$HI.txt" >/dev/null; then
  echo "  DEAD: ${FIELD} = ${LO} and ${HI} give byte-identical ${EXP} output."
  echo "  Do not sweep it. Find what gates it first."
  exit 1
fi
echo "  LIVE: ${FIELD} changes ${EXP} output. $(diff "$T/out_$LO.txt" "$T/out_$HI.txt" | grep -c '^[<>]') lines differ."
