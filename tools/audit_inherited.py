#!/usr/bin/env python3
"""Audit for the defect class `ctxgrid` turned out to have.

`run_ctxgrid` sets `context_slots` per arm and never sets `context_source`, so it inherits
whatever the genome carries. The genome carries 4 (the DERIVED index, bound to the auditory
module), which means the CtxDrive it injects into the CONTEXT module is ignored by the index
and the arm labelled "oracle" is nothing of the kind. Two commits were framed on that.

That is a class, not an instance: an experiment that varies one field of a coupled pair and
inherits the other is interpreting arms it does not fully control. This finds every such
case, so each can be checked by hand rather than discovered by accident.

The pair that matters most is (context_slots, context_source), because the SOURCE decides
which module the index is read from and the SLOTS only decide how many bins it has. Setting
slots without source means "I chose the resolution and let the genome choose the signal".
"""
import re
import sys

SRC = "/home/arne/development/aibaby/host/src/experiments_milestones.cpp"
text = open(SRC).read()
lines = text.split("\n")

# Function spans: a top-level definition starts at column 0 and runs to the next one.
starts = [(i, m.group(1)) for i, l in enumerate(lines)
          for m in [re.match(r"^(?:bool|VLRun|CtxSrc|SalRow|SRRow|\w+)\s+(run_\w+|\w+_arm|\w+_session)\s*\(", l)]
          if m]
spans = []
for k, (i, name) in enumerate(starts):
    end = starts[k + 1][0] if k + 1 < len(starts) else len(lines)
    spans.append((name, i, end))

FIELDS = ["context_slots", "context_source", "ctx_proto_gate", "ctx_conscience_gain",
          "ctx_vigilance", "ctx_param"]

print("Functions that WRITE context_slots. 'source' column = do they also write")
print("context_source, or inherit it from the genome?\n")
print("  %-26s %-8s %-8s %s" % ("function", "slots", "source", "verdict"))
flagged = []
for name, a, b in spans:
    body = "\n".join(lines[a:b])
    wrote = {f: ("offsetof(aibaby::DnaExploration, %s)" % f) in body for f in FIELDS}
    if not wrote["context_slots"]:
        continue
    src = wrote["context_source"]
    # A CtxDrive present and no source written is the exact ctxgrid shape: the host injects
    # into the context module while the index may be reading somewhere else entirely.
    drives = "CtxDrive" in body
    if src:
        verdict = "controls both"
    elif drives:
        verdict = "*** INHERITS SOURCE **and injects a CtxDrive** ***"
        flagged.append(name)
    else:
        verdict = "inherits source (no drive)"
        flagged.append(name)
    print("  %-26s %-8s %-8s %s" % (name, "yes", "yes" if src else "NO", verdict))

print("\n%d function(s) set the resolution and inherit the signal." % len(flagged))
if flagged:
    print("Each needs checking against the genome it is run on:")
    for f in flagged:
        print("  - %s" % f)
