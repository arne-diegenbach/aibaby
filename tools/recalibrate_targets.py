#!/usr/bin/env python3
"""Reset every STALE module target in a genome to its measured free-running rate.

WHY. Appending a context module re-rolls the per-neuron noise stream, which drifts
every module's free-running rate a little. `calibrate` then reports those modules as
STALE and says exactly what to do -- "reset the genome to the left column" -- because
the DECLARED target has gone stale, not the creature.

ctxgenome.sh used to do this for `somato` alone, by matching the literal target 4.56
because that value happened to be unique in the file. That works for one seed and
fails for any other: on family 288559 it is `vision` and `expression` that drift, and
the genome fails its own calibration check, which blocks every experiment needing it.

Takes the module name from calibrate rather than a hard-coded list, and edits the
target inside that module's own [[modules]] section rather than by value, so it stays
correct as the genome changes.

Usage: recalibrate_targets.py <genome.toml> <calibrate-output-file>
"""
import re
import sys


def stale_modules(text):
    """(name, measured_hz) for every module calibrate marked STALE."""
    out = []
    for line in text.splitlines():
        m = re.match(r"\s*(\w+)\s+([\d.]+) Hz\s+([\d.]+) Hz\s+[+-][\d.]+\s+STALE", line)
        if m:
            out.append((m.group(1), m.group(2)))
    return out


def retarget(lines, module, hz):
    """Set target_rate_hz inside `module`'s own section. Returns (lines, ok, note)."""
    start = None
    for i, ln in enumerate(lines):
        if re.match(r'\s*name\s*=\s*"%s"\s*$' % re.escape(module), ln):
            start = i
            break
    if start is None:
        return lines, False, "no section named %r" % module
    # The section ends at the next [[modules]] header, so the search stays inside it.
    end = len(lines)
    for j in range(start + 1, len(lines)):
        if lines[j].lstrip().startswith("[["):
            end = j
            break
    hits = [j for j in range(start, end)
            if re.match(r"\s*target_rate_hz\s*=", lines[j])]
    if len(hits) != 1:
        return lines, False, "%d target_rate_hz lines in %r, expected 1" % (len(hits), module)
    old = lines[hits[0]].rstrip()
    lines[hits[0]] = re.sub(r"(target_rate_hz\s*=\s*)[\d.]+", r"\g<1>%s" % hz, lines[hits[0]])
    return lines, True, "%s -> %s Hz  (was: %s)" % (module, hz, old.strip())


def main():
    if len(sys.argv) != 3:
        print(__doc__, file=sys.stderr)
        return 2
    genome, calib = sys.argv[1], sys.argv[2]
    stale = stale_modules(open(calib).read())
    if not stale:
        print("  no STALE modules; nothing to recalibrate")
        return 0
    lines = open(genome).read().splitlines(keepends=True)
    failed = False
    for name, hz in stale:
        lines, ok, note = retarget(lines, name, hz)
        print("  recalibrated %s" % note if ok else "  REFUSED %s: %s" % (name, note))
        failed = failed or not ok
    open(genome, "w").writelines(lines)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
