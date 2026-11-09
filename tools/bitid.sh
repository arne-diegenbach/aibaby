#!/usr/bin/env bash
# Prove an experiment is BIT-IDENTICAL to its archived log, on CONTENT lines.
#
#   tools/bitid.sh <new-log> [<baseline-log>]
#
# WHY THIS EXISTS. Every section since §50 has proved additivity by re-running an
# older experiment on the new binary and diffing. That has been done by hand each
# time, and it has gone wrong twice in two different ways:
#
#   1. THE RAW DIFF FAILS OPEN THE OTHER WAY. `time` appends real/user/sys and a
#      blank line, so a correct run prints DIVERGED over lines that cannot be
#      equal. §51 and §54 both hit this. The comparison is CONTENT lines.
#   2. §55 DIFFED AGAINST THE WRONG ARCHIVE. `n53-slotdose-450k.log` predates the
#      §53b block later added to the SAME experiment, so the check printed
#      DIVERGED with 33 extra lines that were all one contiguous known block. The
#      correct baseline was `n53b-450k.log` -- 127 of 127.
#
# So this script does not only strip the timing block: it finds EVERY archived log
# of the same experiment, orders them, and REFUSES if the baseline handed to it is
# not the newest. Choosing an older one then requires saying so out loud, which is
# exactly the step that was missing.
#
# Three INCONCLUSIVE states are kept from `tools/cmp.sh`, because a missing file,
# an empty file and an unfinished run must never read as a pass.
set -u

new=${1:-}
base=${2:-}
force=${BITID_ALLOW_OLD:-0}

die() { echo "INCONCLUSIVE: $*" >&2; exit 2; }

[[ -n $new ]] || die "usage: $0 <new-log> [<baseline-log>]"
for f in "$new" ${base:+"$base"}; do
  [[ -f $f ]] || die "$f does not exist"
  [[ -s $f ]] || die "$f is empty"
done

# The experiment's own name, from the log's first line, not from the filename --
# the archive names (n53b-450k.log) do not carry it.
expname() { sed -n '1s/^experiment: *//p' "$1"; }
exp_new=$(expname "$new")
[[ -n $exp_new ]] || die "$new has no 'experiment:' header -- is it a log?"

# AND THE BUDGET, because the same experiment at a different tick count is a
# different measurement and would diverge for a correct reason -- the same class
# of mistake as the stale archive, one step along. `n53b-450k.log` and
# `n53b-900k.log` differ only in this line.
budget() { grep -m1 -E '^  session' "$1" | sed 's/^ *session *//'; }
bud_new=$(budget "$new")
[[ -n $bud_new ]] || die "$new has no 'session' line -- cannot establish its budget"

# Every archive of THIS experiment AT THIS BUDGET, newest mtime last.
mapfile -t archives < <(
  for f in results/*.log; do
    [[ -f $f && -s $f ]] || continue
    [[ $f == "$new" ]] && continue
    [[ $(expname "$f") == "$exp_new" ]] || continue
    [[ $(budget "$f") == "$bud_new" ]] || continue
    printf '%s\t%s\n' "$(stat -c %Y "$f")" "$f"
  done | sort -n | cut -f2-
)
(( ${#archives[@]} )) ||
  die "no archived log of '$exp_new' at budget '$bud_new' in results/"
newest=${archives[-1]}

if [[ -z $base ]]; then
  base=$newest
  echo "baseline chosen automatically: $base (newest of ${#archives[@]})"
elif [[ $base != "$newest" ]]; then
  echo "archives of '$exp_new' at '$bud_new', oldest first:"
  printf '    %s\n' "${archives[@]}"
  if (( force )); then
    echo "*** BASELINE IS NOT THE NEWEST ($newest) -- allowed by BITID_ALLOW_OLD=1"
  else
    die "baseline $base is NOT the newest archive of '$exp_new'.
       The newest is $newest. An experiment EXTENDED since an archive was taken
       diverges from it for a correct reason -- that is §55's defect verbatim.
       Re-run against the newest, or set BITID_ALLOW_OLD=1 to say so on purpose."
  fi
fi

exp_base=$(expname "$base")
[[ $exp_base == "$exp_new" ]] ||
  die "different experiments: '$exp_new' vs '$exp_base' -- this is not a bit-identity check"
bud_base=$(budget "$base")
[[ $bud_base == "$bud_new" ]] ||
  die "different budgets: '$bud_new' vs '$bud_base' -- the same experiment at a
       different tick count is a different measurement, not a divergence"

# CONTENT lines: drop blanks and the timing block `time` appends.
content() { grep -vE '^[[:space:]]*$|^(real|user|sys)[[:space:]]' "$1"; }
a=$(mktemp) b=$(mktemp)
trap 'rm -f "$a" "$b"' EXIT
content "$new" > "$a"
content "$base" > "$b"

na=$(wc -l < "$a")
nb=$(wc -l < "$b")
same=$(comm -12 <(cat "$a") <(cat "$b") 2>/dev/null | wc -l || true)

echo "bit-identity: $new"
echo "  baseline        $base"
echo "  experiment      $exp_new"
echo "  budget          $bud_new"
echo "  content lines   $na new, $nb baseline"

if (( na != nb )); then
  echo "  *** LINE COUNTS DIFFER by $((na - nb)). The first differing lines:"
  diff "$b" "$a" | head -12 | sed 's/^/      /'
  echo "BITID: *** FAILED ***"
  exit 1
fi

if diff -q "$a" "$b" >/dev/null; then
  echo "  identical       $na of $nb content lines"
  echo "BITID: PASS"
  exit 0
fi

d=$(diff "$b" "$a" | grep -cE '^[<>]')
echo "  *** $d content lines DIVERGED of $nb (ordered comparison, $same shared)"
diff "$b" "$a" | head -16 | sed 's/^/      /'
echo "BITID: *** FAILED ***"
exit 1
