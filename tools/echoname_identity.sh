#!/bin/bash
# THE CONTROL ON THE CHANGE, WRITTEN 2026-10-28 BEFORE THE AUDIT RUN FINISHED.
#
# `echoname` gained one column (`artic`) and nothing else. If that is true, then
# every number the published log printed must come back CHARACTER-IDENTICAL: same
# creatures, same seeds, same trials, same reader. A single differing digit means
# the change was not inert and the published result is not what this run audits.
#
# The per-seed table gained `artic` as field 4, so the new row's published fields
# are 1,2,3 then 5..12 -- old 1..11 with one insertion. Nothing else may move.
#
# usage: en_identity.sh OLD_PUBLISHED_LOG NEW_AUDIT_LOG
set -u
old="$1"; new="$2"
row='^  [0-9]+ +(fresh|none|taught|nodrive|yoked) '

# Per-seed rows: 11 published columns out of the old log, the same 11 out of the new.
awk -v r="$row" '$0 ~ r { s=""; for (i=1;i<=11;i++) s = s " " $i; print s }' "$old" > /tmp/en_old_rows.txt
awk -v r="$row" '$0 ~ r { s="";
  for (i=1;i<=3;i++)  s = s " " $i;
  for (i=5;i<=12;i++) s = s " " $i; print s }' "$new" > /tmp/en_new_rows.txt

no=$(wc -l < /tmp/en_old_rows.txt); nn=$(wc -l < /tmp/en_new_rows.txt)
echo "per-seed rows   old $no   new $nn"
if [ "$no" -ne "$nn" ] || [ "$no" -ne 180 ]; then
  echo "ROW COUNT WRONG -- expected 180 in each. IDENTITY NOT ESTABLISHED."; exit 2
fi
d=$(diff /tmp/en_old_rows.txt /tmp/en_new_rows.txt | wc -l)
if [ "$d" -eq 0 ]; then
  echo "per-seed table  IDENTICAL on all 180 rows x 11 published columns"
else
  echo "per-seed table  DIFFERS -- $d diff lines. The change was NOT inert:"
  diff /tmp/en_old_rows.txt /tmp/en_new_rows.txt | head -20
fi

# Summary table: the same arms, published columns only (artic inserted at field 4).
echo
printf "%-9s %-28s %-28s %s\n" arm "published" "audit run" verdict
for a in fresh none taught nodrive yoked; do
  o=$(awk -v a="$a" '$1==a && $2==36 { print $3, $4, $5, $6, $7, $8, $9 }' "$old")
  n=$(awk -v a="$a" '$1==a && $2==36 { print $3, $5, $6, $7, $8, $9, $10 }' "$new")
  if [ "$o" = "$n" ]; then v="same"; else v="DIFFERS"; fi
  printf "%-9s %-28s %-28s %s\n" "$a" "$o" "$n" "$v"
done
