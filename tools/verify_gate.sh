#!/usr/bin/env bash
# Check a `--experiment verify` log against all four gate keys.
#
#   tools/verify_gate.sh results/n55-verify.log [<expected-hash>]
#
# WHY THIS EXISTS. The four keys have been hand-typed at every gate since §49 and
# have now been got wrong twice in the same way: the log pads its columns with
# SEVERAL spaces, so a pattern written with single spaces reports a PASSING line
# as missing. `aibaby-smoke-the-exact-command` records the rule (`\s+` for column
# padding, and write the key against a pasted line from a real log) and recording
# it was not enough -- §55 repeated it on the hash key. A retyped pattern is
# untested code every time; this file is typed once.
#
# TWO ASSERTIONS COME FIRST, because a negative exact-match result is the absence
# of evidence and not evidence of failure: a usage error, a crash, a typo'd flag
# and an empty file all look identical to a determinism regression.
#
# COUNTS, NOT PRESENCE. The §51 defect was two verify runs redirected to the same
# path INTERLEAVING, which doubles every marker while still reading clean and
# still ending in `verify PASS`. §53 then measured that the corrupted log was 936
# lines and a clean one is ALSO 936, so line count never distinguished them.
# `grep -c` against an expected count of exactly one does.
set -u

log=${1:-}
want_hash=${2:-ad96f882becbee92}

if [[ -z $log || ! -f $log ]]; then
  echo "usage: $0 <verify-log> [expected-hash]" >&2
  exit 2
fi

fail=0
note() { printf '  %-34s %s\n' "$1" "$2"; }
bad() { fail=1; note "$1" "*** $2 ***"; }

echo "verify gate: $log"

# --- did the run happen at all? ---------------------------------------------
lines=$(wc -l < "$log")
if (( lines < 800 )); then
  bad "the run happened" "ONLY $lines LINES -- a real verify log is ~936"
else
  note "the run happened" "$lines lines"
fi
if (( $(grep -c "error: unknown argument" "$log") != 0 )); then
  bad "no usage error" "the binary rejected an argument"
else
  note "no usage error" "ok"
fi

# --- the four keys, each exactly once ---------------------------------------
# label|pattern, so the report shows the key a reader recognises rather than the
# regex -- and the pattern uses `[[:space:]]+` for every column gap.
for pair in "verify PASS|verify PASS" \
            "pinned hash PASS|pinned hash[[:space:]]+PASS" \
            "as expected, 0 not|as expected, 0 not"; do
  label=${pair%%|*}
  pat=${pair#*|}
  n=$(grep -cE "$pat" "$log")
  if (( n != 1 )); then
    bad "$label" "COUNT $n, EXPECTED 1"
  else
    note "$label" "count 1"
  fi
done

# The hash key, with `\s+` for the column padding that has broken it twice.
if grep -qE "hash[[:space:]]+${want_hash}[[:space:]]+as recorded" "$log"; then
  note "hash $want_hash" "as recorded"
else
  bad "hash $want_hash" "NOT FOUND AS RECORDED"
  grep -iE "^[[:space:]]*(final )?hash" "$log" | sed 's/^/      got: /'
fi

# A and B must agree with each other as well as with the record: a pinned hash
# that matches while the two runs differ would mean the record is stale.
mapfile -t ab < <(grep -oE "final hash [AB][[:space:]]+[0-9a-f]{16}" "$log" |
                  grep -oE "[0-9a-f]{16}")
if (( ${#ab[@]} == 2 )) && [[ ${ab[0]} == "${ab[1]}" ]]; then
  note "final hash A == B" "${ab[0]}"
else
  bad "final hash A == B" "GOT ${ab[*]:-nothing}"
fi

# --- the experiment count ----------------------------------------------------
cnt=$(grep -oE "experiments[[:space:]]+[0-9]+ as expected, [0-9]+ not" "$log" | head -1)
if [[ -z $cnt ]]; then
  bad "experiment count" "LINE NOT FOUND"
else
  note "experiment count" "$cnt"
fi

if (( fail )); then
  echo "VERIFY GATE: *** FAILED ***"
  exit 1
fi
echo "VERIFY GATE: PASS"
