# Where the vocal line stands after the aim experiments

Written 2026-09-30, after `aimgain`, `aimpool`, `aimfloor`, `selfcode`, `sensefit`,
`aimsense`, `aimslice`. No new run: this is arithmetic over measurements already on
file, and its purpose is to stop the next one being wasted.

## The aim line measured everything downstream of the bias, and none of it binds

| stage | measured | binding? |
|---|---|---|
| actuator swing | +-190 Hz, symmetric to 0.03 (`aimgain`) | no |
| actuator backlash | 107 Hz, and it is per-neuron IP granularity; pooling gives 7 Hz and returns 30 Hz of reach (`aimpool`) | no |
| controller gain | 0.001 k/Hz derived, and it is the OPTIMUM for the shipped 905 ms delay (`aimfloor`) | no |
| sensor | `observed = b + 0.150*F1 + eta`, eta 56 Hz input-referred at tau 62 ms, replicated on two genomes (`sensefit`) | no |
| the loop itself | REFUSED: a blind feed-forward controller beats a PERFECT sensor by 10.4 Hz at the shipped delay (`aimsense` + `aimslice`) | n/a |

**The best controller found is the one with no sensor at all** — a constant bias per
target, delay-immune because it has no loop. It lands 37.2 Hz from targets spread
+-150 Hz around rest.

## And that is the same controller `areax` already built

A constant bias per target IS a context-indexed bias, which DNA v51/v53 supplies and
which works. So the aim line's apparatus does not add a mechanism; **it prices the one
already there.** Putting both on the same axis, which nobody had done because the
naming line measures MOVEMENT (dF1) and the aim line measures ERROR (|F1 - target|):

    the ORACLE bias on a +-150 Hz task   delivers ~150 Hz, lands 37.2 Hz out
    the LEARNED bias (ctxscale)          delivers  118 Hz asymptote, creature at 94% of it
    the NAMING task (shipped vowels)     demands  ~230 Hz to cross the midpoint of a
                                                  460 Hz gap

**The learned bias is roughly sufficient for a +-150 Hz ask and roughly HALF of the
shipped naming ask.** Not an order of magnitude — a factor of two.

## Which is what three independent results already said

1. `ctxscale`: asymptote ~118 Hz, creature at 94% of it. Doubling trials buys 6 Hz.
2. `ceiling-is-an-exponent`: `dF1 ~ aligned^0.61`, so dF1 x2.00 needs aligned x3.12,
   and every knob this project has found moves aligned x1.1-1.2.
3. `orthoname`: **naming works at 0.958** when the targets sit 0.29 log units from
   rest. The shipped vowels demand 0.67 on one word alone.

Three measurements, three framings, one fact: **the bias path reaches about half as
far as the shipped vocabulary asks.**

## So the direction with headroom is the ASK, not the bias

Fourteen routes have tried to raise the bias and returned x1.1. One result — the
`orthoname` 0.958 — shows the machinery already names *well* when the ask is inside
its reach. Nobody has asked whether that scales: it was two words, and the milestone
wants several.

**THE NEXT BUILD, and its cost is honest:** teach a vocabulary whose F1 contrasts sit
inside the measured reach (~0.29 log units, not 0.67) across four or more words, and
score it on `direction-null`'s k-word statistic rather than on a two-word margin.
WHAT WOULD REFUSE IT: naming falling toward chance as k rises, which `ctxfour` already
warns about — a perfect index holds four distinctions but not four TARGETS.
Cost: a teaching protocol, hours rather than minutes.

**What NOT to spend a run on**, each already measured and null:
- pooling the larynx homeostat under teaching (`ippool`: -0.90 SE on 18 creatures)
- a sliced inhibitory auditory->vocal tract (`aimslice`: 0.1 Hz over blind)
- a learned feedback controller (`aimsense`: loses to no controller at all)
- salience/transient reward (`salience-reward`: effect SHRANK from n=12 to n=16)
