#!/usr/bin/env bash
# Watch a running experiment and emit a timestamped progress line with an ETA.
#
# WHY THIS EXISTS. Progress notifications without a clock make a long run
# impossible to plan around: "94/144" does not say whether to wait or go away.
# Every line here carries the wall time now, the elapsed time, and a PROJECTED
# FINISH extrapolated from the observed rate, so the next check is a decision
# rather than a guess.
#
# Usage: tools/runwatch.sh <pid> <logfile> <total-jobs> [interval-seconds]
set -uo pipefail
pid="$1"; log="$2"; total="$3"; every="${4:-900}"
pstart=$(ps -o lstart= -p "$pid" 2>/dev/null | xargs -I{} date -d "{}" +%s)
[ -z "${pstart:-}" ] && { echo "runwatch: pid $pid is not running"; exit 1; }
while [ -e "/proc/$pid" ]; do
  n=$(grep -c "^  \[" "$log" 2>/dev/null || true); n=${n:-0}
  now=$(date +%s); el=$(( now - pstart ))
  if [ "$n" -gt 0 ]; then
    eta=$(( (total - n) * el / n ))
    printf '%s  %s  %d/%d  elapsed %dm  ETA %s (~%dm)\n' \
      "$(date '+%H:%M:%S')" "$(basename "$log" .log)" "$n" "$total" \
      "$(( el / 60 ))" "$(date -d "+$eta seconds" '+%H:%M')" "$(( eta / 60 ))"
  else
    printf '%s  %s  0/%d  elapsed %dm  ETA unknown (no jobs finished yet)\n' \
      "$(date '+%H:%M:%S')" "$(basename "$log" .log)" "$total" "$(( el / 60 ))"
  fi
  sleep "$every"
done
printf '%s  === %s ENDED after %dm ===\n' \
  "$(date '+%H:%M:%S')" "$(basename "$log" .log)" "$(( ( $(date +%s) - pstart ) / 60 ))"
