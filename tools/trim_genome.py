#!/usr/bin/env python3
"""Drop whole modules from an aibaby genome, with every projection that names them.

Hand line-ranges are the wrong tool for this: the blocks are not contiguous, two of
them abut, and a mis-counted boundary produces a genome that still parses while
describing a different creature. This parses the TOML into [[...]] chunks instead and
drops the ones that name a removed module, printing exactly what it dropped so the
diff can be checked against the intent.

A chunk is the [[header]] line plus everything up to the next [[header]], together
with the contiguous comment/blank lines immediately BEFORE it -- the usual convention
that a comment documents what follows, so dropping a module drops its own commentary
rather than orphaning it onto the previous one.

Usage: trim_genome.py <in.toml> <out.toml> <module> [<module> ...]
"""
import re
import sys


def chunks(lines):
    """Split into (leading_trivia, header_line_index) chunks.

    Returns a list of (start, end) half-open line ranges, the first being the
    preamble before any [[header]].
    """
    heads = [i for i, l in enumerate(lines) if l.startswith("[[")]
    if not heads:
        return [(0, len(lines))]
    out = []
    # Walk each header backwards over contiguous comment/blank lines to pick up the
    # commentary that documents it.
    starts = []
    for h in heads:
        s = h
        while s > 0 and (lines[s - 1].lstrip().startswith("#") or not lines[s - 1].strip()):
            s -= 1
        starts.append(s)
    # The preamble is everything before the first chunk's trivia.
    out.append((0, starts[0]))
    for k, s in enumerate(starts):
        e = starts[k + 1] if k + 1 < len(starts) else len(lines)
        out.append((s, e))
    return out


def block_names(lines, lo, hi):
    """The module name, or the (src, dst) of a projection, inside one chunk.

    Only assignments at column 0 count: a commented-out `# src = "vision"` inside the
    commentary must not make the chunk look like a vision projection.
    """
    name = src = dst = None
    kind = None
    for l in lines[lo:hi]:
        if l.startswith("[[module]]"):
            kind = "module"
        elif l.startswith("[[projection]]"):
            kind = "projection"
        m = re.match(r'^(name|src|dst)\s*=\s*"([^"]+)"', l)
        if m:
            key, val = m.group(1), m.group(2)
            if key == "name" and name is None:
                name = val
            elif key == "src" and src is None:
                src = val
            elif key == "dst" and dst is None:
                dst = val
    return kind, name, src, dst


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    src_path, out_path = sys.argv[1], sys.argv[2]
    drop = set(sys.argv[3:])
    lines = open(src_path).read().split("\n")
    kept, dropped = [], []
    for lo, hi in chunks(lines):
        kind, name, s, d = block_names(lines, lo, hi)
        gone = False
        if kind == "module" and name in drop:
            gone, why = True, 'module "%s"' % name
        elif kind == "projection" and (s in drop or d in drop):
            gone, why = True, 'projection %s -> %s' % (s, d)
        if gone:
            dropped.append((why, lo + 1, hi))
        else:
            kept.extend(lines[lo:hi])
    open(out_path, "w").write("\n".join(kept))
    print("dropped %d chunks:" % len(dropped))
    for why, lo, hi in dropped:
        print("  %-34s lines %5d-%5d (%d)" % (why, lo, hi, hi - lo + 1))
    print("kept %d of %d lines -> %s" % (len(kept), len(lines), out_path))


if __name__ == "__main__":
    main()
