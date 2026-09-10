# A diff that CANNOT fail open. Three ways to be wrong -- missing file, empty
# file, unfinished run -- and each is reported as its own state, never as a pass.
a="$1"; b="$2"; want="$3"   # want = same | differ
for f in "$a" "$b"; do
  [ -f "$f" ] || { echo "INCONCLUSIVE: $f does not exist"; exit 2; }
  [ -s "$f" ] || { echo "INCONCLUSIVE: $f is empty"; exit 2; }
done
if diff -q "$a" "$b" >/dev/null; then got=same; else got=differ; fi
[ "$got" = "$want" ] && echo "PASS (files $got, wanted $want)" || echo "FAIL (files $got, wanted $want)"
