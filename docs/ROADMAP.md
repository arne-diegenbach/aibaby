# Where this project stands, and what is prepared next

Written 2026-10-09, revised 2026-10-12. The *record* is the root `README.md`, which is chronological and long;
the *curated findings* are `docs/discoveries.pdf`. This file is the short forward-looking
one, and it exists because the working lead queue has always lived in a memory store
**outside the repository** — so a reader of the repo could see everything that had been
done and nothing that was planned.

Every number below is from a run gated on `--experiment verify` reporting `PASS` with the
pinned hash `ad96f882becbee92`. Sample sizes are creatures, not trials.

## The one-paragraph state

The creature hears, babbles, hears itself, is taught a vowel by praise alone, repeats a
word, and answers. It **names**: four words in two formant dimensions, on a context index it
derives from its own ear, and nine words name as well as six. It says two things in a row
unprompted — a population half-centre gives it a free-running two-phase vocal gesture that
reaches the formant at 30–44 SE with no reward at all. **And it teaches another creature what
it knows**, faithfully enough that a lineage loses nothing over four generations.

**And it is now small and cheap, measured rather than asserted.** A third of the modules were
deleted — vision, somato, expression — leaving **782 neurons and 21,213 synapses**, and naming
survived at n=72 pooled with the difference at **−0.0010 ± 0.0255**, stated as a bound (no loss
larger than ~39% of the effect at 2 SE, not "free"). Its compute is **1.412×10⁶ operations per
simulated second**, 83% of that per-neuron integration, and the shape matters more than the
total: the per-tick path contains **no pass over synapses at all**, so the dense term is bounded
by neurons rather than by synapses. Against a chip's active power floor that cost sits between
293× and 276,000× below it, so **operations were never the power bill — being awake is.**

Power-gating is then free for **perception** (the derived index survives a freeze between words
and its second bit improves, replicated, with a monotone dose-response in silence processed) and
free for a **fixed-target lesson** measured on a creature with no conditional machinery. For
the **conditional** task it is not settled: what an index buys falls by 81%, but decomposition
shows that is mostly the no-index control *converging* with more trials, while the indexed arm's
own fall stays unresolved at 2.1 SE. What *is* clean there is a **fixed 0.153 deficit in index
consistency** (10.3 SE), level not trajectory, with six accounts eliminated and the mechanism
still unknown.

What it cannot do is be *taught* two things in a row. A phase-targeted lesson produces no
learning at any credit window or reward delay, nor with a delay line, a synaptic tag, a
faster-than-human teacher, or a **perfect phase index** — that last at 0.5 SE with the
per-phase bias tables provably diverging at 70–100 SE, which locates the obstacle in
*expression* rather than in credit assignment. **It composes in space and not in time**, and
the reason is now a measured property of the architecture rather than a tuning problem: a
scalar reward writing into a rate-weighted centroid cannot move what a mutual-inhibition
attractor is already holding at one of its two ends. Four sweeps across both axes, the
crossing between them and the corners put the two requirements on disjoint stretches with a
resolved gap — 3.7 SE at the closest approach.

**That bounds how general the creature can be; it does not bound what it is for.** Everything
refused above is an attempt to exceed a specification the creature already meets, and the
specification is written out in the next section. Read that way the project is not stuck
against a blocker — it has finished characterising its creature, and the characterisation is
a working design.

**And it can teach another creature what it knows.** A pupil hearing only its tutor's
recorded vowels, with no host target, acquires the tutor's word-to-vowel convention at
5.3 SE — faithfully enough that a lineage holding the width fixed shows no detectable loss
over four generations, and stubbornly enough to lock in an inverted convention that began as
a near-tie. So the ceiling on what a lineage can say is the ceiling on what *one* creature
can produce, which makes production width the highest-leverage quantity in the project
rather than the line of diminishing returns it looked like.

## The specification, which is what the refusals add up to

Added 2026-10-16, and it is a reframe rather than a new result. **Every refusal in this
project bounds *generality*; none of them bounds *function*.** Read together they stop being
a list of things that failed and become a design document for the creature this project set
out to build — a small one, trainable for one specific purpose, on low power. Each clause
below is a measured result with its own entry in `README.md`:

| the creature's native code | the measurement that fixes it |
|---|---|
| **≤4 signals, laid out on *both* formant axes** — not four levels on one | four levels on one axis read at chance even with a perfect index (0.279); the 2×2 grid reads 5.6 SE on F1 and 4.0 SE on F2 at n=36 |
| **each signal a held vowel, with no structure inside it** | a phase-targeted lesson is null on a delay line, a synaptic tag, a narrowed credit window, a faster-than-human teacher, and a *perfect* phase index — the last at 0.5 SE with the bias tables provably diverging |
| **the condition derived by the creature itself** | its own index matches a host-supplied perfect one on behaviour (−1.0 ± 6.0 Hz), and DNA v70 added +15.22 ± 3.56 more |
| **taught toward fixed targets, not imitated from heard ones** | +24% error reduction toward a fixed target against −0.1% toward a heard one |
| **~96 Hz per distinction, with the ask kept inside reach** | naming hits **0.958** when targets sit inside the learned reach; outside it, dF1 ~ aligned^0.61 makes every further hertz unaffordable |
| **rhythm available, at a price paid in lesson strength** | the gesture is free-running at 30–44 SE with no reward; at 3.2 Hz of depth the lesson keeps 94% of its no-gesture value, at 10 Hz it keeps 36% |
| **and the whole code inherited faithfully** | no detectable loss over four generations (1.4 SE), with arbitrary conventions locking in |

**Two framings in this file were too strong and are corrected here.** "They do not compose"
is about *time*, not about coexistence: a gesture and a taught identity do run together, and
the trade is on the gesture's *depth*. What cannot be done is make the two halves of a
gesture say different things, because that degree of freedom belongs to the oscillator and is
not plastic. And "turning the gesture on costs four fifths of the lesson" is the cost at the
operating point chosen for the gesture's swing, not a general one — at low inhibition the
gesture is nearly free.

**What the specification implies about the work.** The remaining open items are no longer
"find the mechanism that lifts the ceiling"; the ceilings are measured and the escape routes
are priced and closed. They are: *use the creature inside its specification*, and find out
where the specification itself binds. The first question of that kind is §0.

### 0. Does the specification hold end to end? — MET, and the limit is the INDEX

Each half is established and the composition has never been run. `ctxgrid` names four
signals on both axes with a **host** index; `tutorwide` transmits **two** between distinct
individuals with each pupil deriving its **own** index, at no loss over four generations.
`tutorgrid` runs the two together: a four-signal two-bit code, 18 lineages, three
generations, with generation 0 run twice — on a host index and a derived one — so a failure
says *which* unknown failed instead of leaving the run uninterpretable.

The bars are pre-registered in the source: generation 0 on the derived index clears 3 SE on
**both** bits (prerequisite); the widened chain clears 3 SE on both at generation 2
(primary); and the flipped chain **alternates sign** between generations (inheritance —
without which a positive primary is void, because the tape re-imposes the host's width on
both axes whether or not anything was inherited). The F2 bit's power limit is named in
advance: 18 lineages is `tutorwide`'s n, not the n at which ctxgrid's F2 side established,
so an F2 bit landing between 2 and 3 SE is **underpowered, not null**, and the follow-up is
a pre-committed larger n.

**It holds.** The first run refused and named its own lever: on a host index both bits are
strong (F1 +81.0 at 20.5 SE, F2 +148.8 at 14.4 SE), on the derived index the F1 bit forms
(+42.1, 9.9 SE) and the F2 bit is **8% of the oracle's** (+12.3 ± 8.9, 1.4 SE) — so
production was fine and the limit was the derived index at four contexts. Not underpowered
but absent: at n=144 it would read 3.9 SE while still being 8% of the magnitude, and a bar
cleared only by adding creatures is not a bit forming.

The second run switched on the one measured lever for that partition — DeSieno's conscience
at DNA v70's settled gain, two genome fields, nothing rebuilt. The oracle arm came back
**bit-identical** (max |diff| 0.0000), which is what licenses the paired comparison:

```
PRIMARY     bit F2, paired consc − base   +79.68 ± 15.43  (5.2 SE)   MET
SECONDARY   bit F1, paired consc − base    −4.44 ±  5.16  (0.9 SE)   did NOT fall
generation 2    F1 +25.31 ± 3.81 (6.6 SE)      F2 +75.27 ± 25.03 (3.0 SE)
INHERITANCE flip alternates 18 of 18 lineages on BOTH bits, z = +4.24 each
```

So: *a small low-power creature learns a four-signal conditional code, derives the condition
from its own ear, and passes it to the next individual intact* — demonstrated end to end
rather than assembled from separate runs.

**Three limits, stated because the bars were close.** The F2 bit at generation 2 is *exactly*
on the 3 SE bar. The decay improved from 4.3 SE to 2.6 SE and is no longer established, but
the improvement itself is 1.9 SE and is *also* not established. And this is **two**
generations where `tutorwide` ran five.

**Depth 5 was run, and it splits (2026-10-17).** Raising the chain to five generations put
four signals on `tutorwide`'s exact footing. Paired generation 4 minus generation 0:
**bit F1 −21.58 ± 5.56 (3.9 SE), a resolved loss of 57%; bit F2 +0.97 ± 27.91 (0.0 SE), no
detectable loss.** One bit decays and the other ends where it started.

What crosses is unambiguously **inherited** rather than re-imposed: the flipped chain
alternates across *every* consecutive pair — three at this depth against the single flip
depth 3 allowed — at 18 of 18 and 16 of 18 lineages. So a decaying bit is a channel-capacity
statement, not an artefact of the tape.

Against a two-signal code losing nothing over four generations, **four signals leak where
two did not, on one axis only.** As a hypothesis and not a conclusion: F1 has the smaller
measured reach (118 Hz against ~300) and the derived index recovers less of the oracle there
(46% against 62%), so the bit with less headroom relative to its noise erodes first — but
this run cannot separate "less reach" from "worse index" because they coincide on that axis.

Two things are *not* claimed. F2's "no detectable loss" is a **bound, not a hold**: at n=18
the smallest resolvable loss on that bit is ~84 Hz. And the F1 decay's shape is not
established — linear and geometric fits are indistinguishable on five points.

**Two follow-ups settled it, and the answer is the index.**

`ctxfactor` refused the first explanation. The decay looked like a parameter-sharing problem,
since every context owns independent parameters. It measured whether there is shared
structure to generalise *over* and found none: the three orthogonal contrasts hold equal
power (interaction 0.325 ± 0.006 against a derived chance of 1/3). **The creature stores a
factorial task non-factorially**, so there is nothing to share and the factored fix is
unavailable (§6.10 of the discoveries).

**Then the oracle chain found the cause.** Handing the pupils a host-written index — and
changing nothing else, with generation 0 returning bit-identical on all four of its numbers —
turns the decay around:

```
THE PRIMARY, cross-run paired:  F1 decay, oracle − derived   +62.61 ± 5.91  (10.6 SE)
  derived chain, bit F1   −21.58 ± 5.57  (−3.9 SE)  a resolved LOSS
  ORACLE  chain, bit F1   +41.03 ± 4.36  (+9.4 SE)  a resolved GAIN
```

Reach is ruled out: F1's 118 Hz is unchanged and only the index moved. And the oracle chain
lands at **+78.69 against the oracle arm's own generation-0 value of +81.01** — 97%, paired
difference 0.4 SE. **The lineage climbs to what a perfect-index creature manages in a single
session and stops there: the chain's level is set by how well a pupil can use its index, not
by cumulative transmission loss.**

That corrects this section's earlier framing. "Four signals leak where two did not" was wrong
about the *channel*; a four-way **derived** index is worse than a two-way one, and down a
lineage that compounds into apparent decay.

**The growth is not culture accumulating**, which is the limit to carry: the `wide` tape
re-imposes the host's spacing every step, so what is inherited is the *order* and not the
width. And a host-supplied index is not cultural transmission in the same sense — the pupil
is told its condition — so the result says **fix the index**, never "the code transmits".

**The next run is named by the result:** improve F1's derived index. The conscience moved
F2's recovery from 8% to 62% of the oracle's bit; nothing has tried F1's.

## What is settled, and therefore not to re-open

| line | state |
|---|---|
| Credit assignment via reward masks | **Closed by arithmetic.** Gain rises monotonically with how much of the motor group the reward reaches, so the cheapest mask is no mask. Nothing in the family is affordable against its retention price. |
| The geometry of the motor pool | **Settled in two independent families.** The readout is a rate-weighted centroid, so potency is *signed* distance from it; potency decides *whether* extent costs anything, and given nonzero potency extent costs until it saturates. |
| The derived context index | **Explained, and the repair is closed.** Its deficit was the word boundary: given a clean one, classification error goes to exactly zero and the creature's own index matches a host-supplied perfect index on behaviour (−1.0 ± 6.0 Hz). But four detectors built to supply that boundary are all beaten by the arm with no detector at all, and the leading arms sit at their occupancy ceilings — so **build no further boundary detector**. The gap itself was real (oracle − detector-free = +17.2 ± 5.1, 3.3 SE) and is **now mostly claimed** — not by a detector and not by balancing the table, both of which are closed, but by DNA v70's conscience gain, settled at a pre-committed n = 144: +15.22 ± 3.56 (4.3 SE) with 85% of it in the voice, taking the gap from +19.16 ± 3.18 to +6.15 ± 2.78 (§2). |
| The aim/feedback line | **Refused by its own control** — a blind controller beats feedback at this creature's loop delay. |
| Protocol variations | **Closed.** Longer, blocked, region-shaped, distance-aware and staircased rewards were each measured and none helps. |
| The transmission channel | **Faithful once width is held** — no detectable loss over four generations (1.4 SE), and a ratchet that locks in whichever convention it finds. Further chain runs measure the tutor, not the channel, so the quantity to move is production width (§1). |

### Closed 2026-10-17 to 2026-10-24, and each for a measured reason

- **The magnitude in the index feature.** Removing overall drive from the clustering feature
  pays +0.064/+0.161 **only with random distinct prototype seeds**, which this creature cannot
  produce; at its own zero initialisation it is 1.4 SE, and ART vigilance — the only mechanism
  here that makes distinct data-driven seeds — recovers at most 1.6 SE of it. **Do not build.**
- **ART vigilance, explained as well as closed.** At the shipped threshold **exactly one slot
  ever commits**, so the milestone built on it (0.737 → 0.594 at 96 creatures) was measuring a
  *constant index*. At thresholds where four do commit it still does not deliver. **Do not
  re-calibrate the threshold** — that sweep has been done.
- **The conscience-gain optimum.** Already a four-point sweep with the shipped value at the
  peak on both delivered Hz and the registered contrast, collapsing to chance at gain 8. "Not
  located" meant bracketed in [1,4] with the peak at 2, not unexplored.
- **The operating point for the `bias→rate` stage.** `stageprobe` names that stage as the
  compressive one (0.34 where pooling is 0.66) and shows it reaching 1.40 at higher drive, which
  invites a tonic pedestal. **Multiplying the stages out refutes the whole family:** the product
  end to end is 0.227, 0.260, 0.226 across the first three rungs — moving the operating point
  improves `bias→rate` 4.1× and costs `rate→centroid` 4.1×. Best available is **+15%**, not 4×.
  What remains is changing the transfer's *shape* without moving along it, which is a neuron
  model change and needs its own oracle.
- **And the shape change is now closed too, through all three intrinsic constants
  (2026-10-29).** v71 randomised the **threshold** and it cost **61%** of the lesson, monotone
  in sigma. v72 learned and randomised the **leak**: nothing either way, bound 31%. v73 did the
  **refractory period** and it is *structurally inert* — the creature runs **46–81× below the
  333 Hz ceiling** 3 ms sets, so the constant governs ~1.5% of each neuron's input, and a
  spread holds the operating point to sigma 0.60 where the threshold broke it at **0.04**. The
  pre-registered trend reads **−0.0053 ± 0.0060 (0.9 SE), 18/36 negative — exactly chance**,
  i.e. **−1.6% per 0.1 sigma against the threshold's −20%**, with the threshold's own
  dose-response **four times outside the 2 SE bound**. Each constant fails for a different
  reason, and the one thing that *does* move the 0.34 stage is not a constant at all:
  **intrinsic plasticity** (0.34 with IP on, 0.91 off; off destroys learning at −7.42 SE,
  half-relieved buys nothing). **So the target is the neuron's regulator, not its constants**,
  and `stageprobe`'s untried **group-mean homeostat** — holding the common mode while leaving
  the tilt a centroid readout reads — is now the remaining candidate rather than one of
  several. ~~**Struck 2026-10-29:** it was never untried. That is **DNA v57 `ip_pool`**, run at
  articulator-group granularity and refused (−0.90 SE, learned bias halved, threshold pushed 3×
  harder). See the v74 entry below for what was actually left and how it closed.~~
  `noise_amp` is still unvaried per neuron and still the harder one, not the better
  one: it is already the exploration drive.
- **And the regulator's coordinate system, the narrower lead that replaced it (2026-10-29).**
  The estimator and the readout already share a basis: `read_group` is a rate-weighted centroid
  over neuron index, `ctx_gain_mode = 1` writes along the group's zero-mean **ramp**, and
  `ctxgain` showed **18 ramp parameters match 252 per-neuron ones**. The regulator is the only
  part of the loop in a basis of its own, so **DNA v74 `ip_ramp_exempt`** exempts that one
  direction of fourteen — against v57's thirteen and v58's zero (a magnitude, not a direction).
  At full dose (`ramp cov` 100%, liveness 18/18, no dose warning) the pre-registered primary
  reads **aligned ×0.957 ± 0.146** — **−1.7 SE from v58's ×1.21** and not clear of ×1.000 —
  with the control ratio falling too (×0.860) and excess dF1 at **−27.0 ± 18.0 Hz, signs 5/18**.
  **The mechanism prediction was confirmed and bought nothing:** threshold push **×1.02**
  against v57 pooling's **×3.3**, so a projection is demonstrably not pooling in disguise, and
  `aligned` still did not move. The stopping rule's third branch fires — point estimate under
  the bar, so the pre-committed n=72 does **not** run. One result to keep out of it: the arm ran
  its larynx at **7.79 Hz**, *above* the 6.84 Hz at which `ipoff` collapsed learning at −7.42
  SE, yet lost only 1.50 SE — **group rate drift does not predict learning collapse**, so the
  refusal condition was using a poor proxy for what `ipoff` actually removed (per-neuron
  regulation along the other thirteen directions).
- **Further trimming, on power grounds.** Measured to buy nothing: the creature's dynamic cost
  is already orders of magnitude below any platform's active floor, so size is not the lever.
- **Prototype drift as the account for a four-word index.** The v53 line predicted the index
  *decays* across a session in a learning arm, and v53b's did (0.697 → 0.663). On the four-word
  conditional task **both arms rise** (+0.045 and +0.061, 3.4 and 4.6 SE). Do not cite drift here.

## What is open, in the order the measurements allow

### 1. How wide a distinction one creature can produce — and a lineage now inherits it

**One creature teaches another through the air.** A pupil hearing only a tutor's recorded
vowels, with no host target, acquires the tutor's word-to-vowel mapping at
**+53.3 ± 10.0 Hz (5.3 SE)**, and symmetrically: swapping which tutor vowel goes with which
word gives −27.7 against the faithful arm's +25.6, so the pupil takes whatever convention
its tutor holds rather than recovering the host's.

**The chain's apparent decay was width, not a leak.** Passed down five generations the
distinction decays to nothing — but hold the *width* fixed and pass on only the mapping, and
|dF1| at generation 4 is **85.6 ± 5.5** against generation 0's 96.1 ± 5.1: **1.4 SE, no
detectable loss over four generations**, where the plain chain lost 16.0 SE worth. Culture
does not decay here because the air loses information. It decays because each tutor produces
a narrower distinction than it was taught.

**And it is a ratchet.** 5 of 18 chains ended holding the *inverted* convention at full
strength, 18 of 18 never changed sign after generation 1, and the inverted ones are exactly
those whose first pupil nearly failed (|dF1| 5.1 vs 86.4 Hz, 10.5 SE). A near-tie is noise;
the next generation restores it to full width and locks it in. An arbitrary convention, held
stably down a lineage — and the sharpest reason to distrust any single chain.

**So the lead is production width, and it has a new argument behind it.** The host demands a
460 Hz distinction and a taught creature delivers 96.1. Every Hz added to one creature's
dF1 is inherited by its whole lineage without loss, which turns what looked like diminishing
returns into the highest-leverage quantity in the project. Transmission itself should not be
iterated on further.

**Read first, because the obvious routes are closed:** dF1 ~ aligned^0.61, so moving dF1
×1.68 needs ×2.9 the aligned bias; the transfer curve and the context-scale sweep put
asymptotes at 234 and ~118 Hz independently; and nothing downstream of the bias binds.
**Do not re-run a bias sweep.** The nearest unexplored thing is the creature's own f0 — the
tutoring tape carries formants only, and pitch is still the host's.

### 2. Table balance for the derived context index — CLOSED, the feature saturates at 0.89

This replaced the closed boundary line and has now closed too, in one night. The question
was whether the derived index's pinning to `1.5 − occupancy` meant *balance* was binding or
*accuracy* was, which no comparison at the ceiling could answer because there the two are
identically equal. **DNA v70 answered it by overshooting.** A gain on DeSieno's conscience
drives occupancy from 0.641 to 0.502 — a perfectly even split — taking the ceiling to 0.998,
and the index **saturates at about 0.89** while slack opens from 0.005 to 0.478.

**So the partition's balance was binding only while occupancy exceeded ~0.52, and past that
the limit is the feature.** The ~9 Hz still separating the detector-free arm from the oracle
is therefore **not reachable by balancing the table**, which closes the route rather than
leaving it open as an invitation.

Two results worth carrying. **A balanced partition is not an informative one:** at the
highest gain the table is balanced to 0.502 and the index reads 0.520 against a chance level
of 0.500 — slots assigned by turn-taking rather than by the word — with dF1 *below* the
no-index arm, −30.4 ± 5.6 (5.4 SE). And the best setting's behavioural gain was at first
**sub-bar and recorded as such**: pooled over 72 creatures (half of them never seen by the
hypothesis) the registered contrast was +12.7 ± 4.7, **2.7 SE** against a bar of 3, 45 of 72
positive. No creatures were added to close the 0.3 SE gap, because that is optional stopping.

**It was then settled at a pre-committed n, and it clears (2026-10-15).** The design was
fixed in the source before the run: two further blocks of 36 creatures, **n = 144 exactly**,
the 3.0 SE bar, the power limit stated up front (blind below +10.0 Hz, so a refusal would
mean "under about 10 Hz" and never "zero"), and a stopping rule declaring that a failure
*ends the line* with 2.5–3.0 SE counting as a failure.

| block | creatures | contrast |
|---|---|---|
| 1, discovery | 0–35 | +14.81 ± 5.85 |
| 2, fresh | 36–71 | +10.51 ± 7.36 |
| **3, new** | 72–107 | **+15.02 ± 6.71** |
| **4, new** | 108–143 | **+20.53 ± 8.50** |
| **pooled** | **144** | **+15.22 ± 3.56 — 4.3 SE, 94/144** |

All four blocks positive, so the sign-change refusal never fired; the two blocks the claim
had never seen are the **strongest** of the four; and the sign test agrees with no
distributional assumption at all (94 of 144, **z = +3.67**). Per-creature SD came in at
42.7 Hz against the 39.9 predicted beforehand, so the calculation that chose the sample size
was sound rather than lucky.

**85% of it is in the voice**, which the discovery half could not establish: `ema` rises
**+13.00 ± 3.10 (4.2 SE, 91/144)** while the matched-marginal control falls only
−2.21 ± 2.16 (1.0 SE, indistinguishable from zero). The voice arm clears the bar on its own.

**And it collects the gap §1 left unclaimed.** `oracle − ema` goes from +19.16 ± 3.18
(6.0 SE) to **+6.15 ± 2.78 (2.2 SE)** — the treatment closes **+13.00 ± 3.10, 68% of it** —
and what remains is no longer clearly resolvable. The specificity control is exact: the
oracle arm is **bit-identical across the two genomes on all 144 creatures**, so the
conscience acted on the derived index and nothing else. **The conscience gain ships.**

One audit is owed and is now safe to run, because the claim no longer rests on it: block 1's
shipped baseline is a twenty-arm log rather than a dedicated four-arm run. Blocks 2–4 alone,
all on dedicated baselines, pool to +15.35 ± 4.34 (3.5 SE, 70/108) and clear without it. The
swap was deliberately *not* folded into the settle, because changing a baseline after seeing
the number is the flexibility pre-registration exists to remove.

The mechanism, by contrast, replicated to three decimals: two families starting at occupancy
0.641 and 0.653 both land on 0.522/0.523 with the index at 0.890/0.892, and the treated arm
delivers 71.8 and 71.9 Hz against an oracle at 80.6 and 80.8. **The noisy quantity is the
untreated baseline, not the treatment.**

Two cautions carried forward from the run that closed §1's framing. The boundary prize is
**genome-dependent** — +11.1 ± 7.1 Hz on one family and +27.4 ± 7.0 on another, with no
shared creatures — so the +53.4 Hz figure quoted in the settled table is one sample of a
range. And the gate shapes built for that run were nearly refused on a short smoke that
mis-measured their gate rate by ~2×, because a rate-thresholded gate depends on a rate
distribution still settling early in a creature's life. **Only a handed window is
budget-invariant**, so do not refuse a thresholded mechanism on a smoke's event rate.

### 3. Myelination gated to sleep — prepared, not built

Bellesi and colleagues measured oligodendrocyte precursor proliferation roughly **doubling
in sleep** and correlating with REM, with myelination genes transcribed preferentially then
and differentiation higher in wake. This project's consolidation brake runs **continuously
and awake**, which is the state that biology says suppresses it. The machinery to fix that
exists: competitive pruning is already sleep-gated.

**The naive version is already refused, by arithmetic rather than by a run.** Sleep here is
fatigue-driven and automatic, and the genome fixes the sleep bout exactly: the fatigue swing
is 0.55 and recovery is a plain 0.0000030 per ms, so a bout is 183,333 ms ≈ 3.1 minutes.
Against the sleep model's recorded ~20 minute cycle that is **~15% of simulated time**, so a
12M-tick session already contains about ten sleep bouts. The brake runs through all of them
and costs nothing — a null on retention *and* on learning. **Gating it to sleep would make it
bind ~15% as often: less of a thing that already does nothing.**

So "gate the brake to sleep" is the wrong build, and the reason is worth stating because it
is a reading error in the metaphor. Bellesi and colleagues' result is that myelination
*happens* in sleep — proliferation doubles, myelin genes transcribe. That is a
**consolidation** step. The brake is the **throttle** half. A mechanism that *commits* during
sleep is the thing the biology points at, and the brake is the wrong starting point for it.

**And that collides with two settled negatives, which is why it is not next.** A slow store
was built and refused — it is a 21× brake on the quantity this project is short of — and
consolidation at a checkpoint was refused separately. A sleep-timed commit differs from both
only in *when* it fires, inside a ~15% duty cycle of about ten bouts. **Price it against
those two results before building anything**, and if it cannot be distinguished from them,
the lead should be closed rather than carried.

### 4. Two gestures in a row — the blocker, and they DO NOT COMPOSE

Everything above is about a *carrier*. Intelligent speech needs structure in time — and the
state of that is better than this file said on 2026-10-09, which was written from memory
rather than from the record.

**Structure in time already exists and is replicated.** DNA v56+v57's population half-centre
produces a two-phase alternation that *reaches the formant*: produced F1 follows it at
30–44 SE over a matched null with both single-population controls at or below the null, and
one setting gains **+65.9 Hz of free-running excursion at +52 SE with no caregiver and no
reward**. So the creature does make two gestures in a row, unprompted.

**What is missing is narrower than "no sequence".** Two things. The alternation is *not a
clock* — its period ignores the adaptation time constant it was built on — so the creature
cannot be asked to alternate at a particular rate. And a *taught* utterance is still a held
vowel: 1–8% of its movement is shared, and no module holds a kick for 10 ms. **The generator
exists, the lesson exists, and they have never been connected.** That is a coupling problem
rather than an existence problem, and this project has solved one of those before — the word
window was exactly "the signal is there and the gate on it is wrong".

**It was yoked, and refused — and the refusal is defensible, which took three runs.**
`phaseteach` rewards the alternation's two phases toward two *different* formant targets and
reads the difference between the identity and swapped mappings, so the oscillator cancels.
The contrast is null in every configuration tried: at the shipped credit window (void, the
window spans three phases), at a window narrowed twentyfold to fit inside one phase, and at
a **50 ms reward delay** — a caregiver faster than any human, run as an oracle because
resolution and delivery are otherwise opposed and 50 < 100 < 136 is the only way to satisfy
both. −0.1 ± 0.3 Hz at power to see 0.8 Hz, n=18.

**The sharper statement needs no contrast at all.** A phase-targeted lesson produces an
error drop indistinguishable from zero on every genome, window and delay — eight cells, none
past 2 SE, not one positive — while a **fixed**-target lesson in the same sessions on the
same creatures reaches 6.3 to 18.4 SE. Same reward machinery, same populations; the only
difference is what the target is. So reward can move this creature's vowel and cannot move it
*differently at different moments of one utterance*. **The gap is where the reward lands, not
when it arrives** — which refuses the delay-line and synaptic-tagging build, since both
deliver credit to a place with no phase-specific target to receive it.

**And the control that licensed all of that found the larger result.** Turning the gesture on
costs **four fifths of the lesson**: error drop +0.2197 ± 0.0120 with the half-centre off
against +0.0465 ± 0.0073 with it on, a difference of 0.1732 ± 0.0140 at 12.3 SE. The lesson
still lands at 6.3 SE, so this is attenuation and not a block. The direction had been
reported from the other side and called a success — *the gesture survives teaching*, phases
splitting rewarded time 0.50/0.50, the oscillator never silent. **The mirror image was never
measured.** The operating point was chosen for the gesture's F1 swing and never once checked
against the lesson.

**THE COMPOSITION QUESTION WAS THE NEXT RUN, AND IT IS NOW ANSWERED IN THE WHOLE PLANE —
THEY DO NOT COMPOSE.** Four generations of `hcsweep`, twelve cells, against a bar fixed
before the first of them: some point must give an error drop over half of 0.2197 (that is
0.1099) while keeping `sep` > 10 Hz and `share` within 0.15–0.85.

| axis swept | range | result |
|---|---|---|
| `halfcenter_gain` at drive 1.40 | 0.15–0.60 | lesson 0.2062 → 0.0465 as sep 3.2 → 19.5; none clears both |
| `halfcenter_drive` at gain 0.45 | 0.90–1.40 | lesson 0.0210 → 0.1243 as sep 32.1 → 8.0; none clears both |
| the crossing, gain at drive 1.40 | 0.45–0.54 | **at sep 10.0 the lesson is 0.0796, 3.7 SE under its bar** |
| the corner, as a 2×2 factorial | gain 0.15 × drive 0.90 | **lesson 0.1790 ✓ but sep 1.6 ✗ — dominated on both** |

**The two requirements are satisfied on disjoint stretches with a resolved gap between
them**, so it is not a knife-edge finer resolution could find. The control cell reproduced
to four decimals in all four runs. Along the way the oscillating window was mapped for the
first time — alternation present only between drive ≈0.9 and ≈1.6, with the shipped
operating point near its *upper* edge — and `sep` turned out to be **non-monotone in gain**
(8.0 at 0.45, 6.7 at 0.48, 10.0 at 0.51), which is why every interpolation across the
unsampled interval was wrong, the optimistic one by 39%.

So this is the real negative the bar was written to detect: **a scalar reward writing into
a rate-weighted centroid cannot move what a mutual-inhibition attractor is already holding
at one of its two ends.** The creature makes two gestures in a row unprompted and can be
taught a vowel; it cannot do both at once in this architecture.

**And the phase route is closed too, with the block finally named (2026-10-15).** The
refusal above was about *where reward lands*: it reaches the vocal module as a quantity with
no phase-specific target to write into. So the lesson was handed a **perfect** phase index —
the context pinned per tick to the creature's own half-centre phase, a separate bias table
per phase, a credit window inside one phase, a faster-than-human teacher. Result:
**+0.19 ± 0.35 Hz, 0.5 SE** against a 3 SE bar, with every precondition satisfied (presence
1.000 on treated arms and 0.000 on all five controls, the fixed-target lesson landing at
12.0 SE, 60.7% of credit surviving the delay).

**The tables did diverge, which is what makes it a finding.** A null looks identical whether
reward wrote phase-specifically and the formant ignored it or the write never happened, and
those license opposite conclusions. Table divergence reads **0.0216 on both treated arms at
70–100 SE, against exactly 0.0000 on the shared-bias arms.** Reward wrote per phase, in
opposite directions for the two mappings, and F1 did not follow.

**Naming prices that at ~40 Hz** — the same table on the same larynx carries 0.0288 → 80.1 Hz
with a word index — so this is a **~40× shortfall**. The validating control is the
matched-marginal arm, which diverges just as much (0.0268) and delivers nothing over no table
at all, proving divergence must be *aligned* with the distinction; here it is aligned by
construction. And the obvious objection fails: the 50 ms delay misdirects 37% of each write,
worth 3.8× even if all of it subtracts.

**So the block is expression, not credit assignment.** Mutual inhibition holds F1 at one of
its two ends, and a bias written into the *correct* per-phase table still cannot shift it.
A delay line, a synaptic tag, four word-boundary detectors and now a perfect phase index have
all been refused on the same quantity. **This also kills a build before it started:** the
songbird sparse-time-basis lead (a synfire chain supplying a sequential index, turning
temporal credit into a contextual bandit) exists to *derive* this index, and an oracle version
buys under 1 Hz. **What remains is the readout**, not the index — something other than a
rate-weighted centroid an attractor already pins.

**The corners were the named escape, and they are now measured and closed.** The surface is
*not* separable — low drive costs the lesson **5.1× less** when inhibition is weak, exactly
the interaction a pass needed — but it buys no gesture, because the oscillating window's
**lower edge moves with gain** and gain 0.15 × drive 0.90 lies outside it. The corner loses
the lesson against the cell above it (−0.0272 ± 0.0128, 2.1 SE) *and* the gesture (−1.6 Hz),
and sits 2.6 SE below the half-centre-off reference: **starving the oscillator does not hand
the lesson back.** At matched gesture there is no evidence the route matters (0.7 and 1.7 SE
on two cross-axis pairs), though the power only bounds a route difference to tens of per
cent — consistency, not a collapse onto one variable. **Do not re-sweep this plane.**
Anything further here is a change of readout, not of operating point.

One thing is owed alongside it. The 4/5 cost names the *configuration*, not the mechanism:
four genome fields differ and two are spike-frequency adaptation, so an adaptation-only and
an inhibition-only genome are needed to attribute it.

The 2.2 SE credit-window lead that stood here has been **powered up and refused**. Narrowing
the window twentyfold gives +0.0154 ± 0.0080 (1.9 SE) on 36 creatures, and
**+0.0052 ± 0.0135 (0.4 SE) on the 18 the original read had never seen**, against
+0.0256 ± 0.0084 (3.0 SE) on the 18 that produced it. A failure to replicate rather than a
proof of absence — the fresh interval reaches +0.032. **The method result outlives the lead
and is in §2.17 of the findings document: pairing a claim on its own discovery sample lifts
it exactly onto the bar, and pairing pays only in proportion to how much the units correlate
(r = +0.133 here moved the SE 0.0086 → 0.0080, so it was the extra creatures that decided
it, not the sharper statistic).**

Bohland, Bullock & Guenther's GODIVA names the layer above all of this, and Segawa and
colleagues say the stored unit is a *gestural score* rather than an acoustic target — which
is the shape the dormant `dictionary_*` genome block was cut for. Both stay on the shelf
until the composition question has an answer, because a score is a sequence of targets and
this creature cannot yet be taught two.

### 5. Carrier residuals, demoted and worth stating

- **The prototype learning-rate floor on the episode path.** With a clean boundary the index
  decays within a session, 0.919 → 0.858 at 46.5 SE — MacQueen's `1/wins` reaching zero
  under an input that is still moving. The floor exists (DNA v60) but is wired into the
  per-tick update path and not the episode path the decaying arms use. Demoted because the
  behaviour already reaches oracle level *while* decaying, which bounds what the decay can
  be costing.
- **An unexplained 8 Hz.** The index is identical under alternating and shuffled word
  orders while the behaviour is not. A predictable word sequence helps the voice and not the
  carrier, and nothing accounts for it.

### 6. A MULTIPLICATIVE homeostat — the strongest remaining lead, not built (2026-10-29)

Found by asking which literature this project had *not* cited: **Triesch** and **Toyoizumi**
were both absent while Zenke, Turrigiano, Masse, Kornfeld and Fiete were all present.
Toyoizumi, Kaneko, Stryker & Miller (2014) argue that slow homeostatic feedback cannot
stabilise fast Hebbian feedback **when both act on the same variable**, and their remedy is
to factorise: a synapse-specific Hebbian factor times a cell-wide homeostatic one, each with
its own stable inactive state.

That maps onto two facts in this code, and the mapping is algebra rather than analogy:

- **The readout is exactly scale-invariant.** `read_group` returns
  `sum(w_i·p_i)/sum(w_i)` with `w_i` the rate, so scaling every rate by a common factor
  cancels. The same function's `activity` output — the mean rate, which gates voicing — is
  *not* scale-invariant.
- **The homeostat is additive on the threshold**
  (`threshold_[i] = clampf(threshold_[i] + ip_rate*err, …)`), and a per-neuron offset changes
  *relative* rates, so it moves the centroid.

**So a homeostat regulating rate by a common multiplicative gain would be algebraically
invisible to the formant readout while still controlling the voicing gate.** That is why it
is not v57/v58/v74 again: all three kept the additive homeostat and relieved it on a subspace
(13 directions of 14, a magnitude, then 1 direction), trading regulation for expression. A
multiplicative homeostat keeps regulation at full strength and is unseen anyway, which
dissolves the "no setting between the horns" trade instead of bargaining inside it. The prize
would be recovering `bias→rate` from 0.34 toward the 0.91 measured with IP off, without the
−7.42 SE collapse that turning IP off causes.

**The blocker, which the code states itself.** The threshold path exists partly to lower the
bar and "keep a module alive when reward learning quietly weakens all of its inputs" — and a
gain cannot revive a silent neuron, since any gain times zero is zero. So the honest design is
a hybrid: multiplicative for regulation, with the threshold term retained as a floor near
silence. The floor is where it can go wrong, and that makes it a two-part mechanism rather
than a one-line change.

**Before building it, settle which quantity the exponent takes.** `dF1 ~ aligned^0.61` needs
`aligned` ×2.34 for the ×1.68 the ceiling asks. Transfer 0.34 → 0.91 is a larger change than
any knob found so far, but it is a change to the *transfer*, not to `aligned` — and the
synthesis already found that nothing downstream binds. Work that arithmetic out first;
v58's generation was lost to a gate that asked for a quantity the mechanism could not move.

Not costed. `norm_gain`, `ffi_gain` and `lateral_gain` are all 0.0 on vocal, so a gain pathway
exists in the kernel but would need a slow integrator driving it rather than instantaneous
control.

**Read in full (2026-10-30), the paper confirms the structure and qualifies the prize.** It
states the blocker itself, unprompted: *"A nonzero minimal weight value is important because
multiplicative homeostatic plasticity cannot potentiate synaptic strength if it reaches
zero."* So the hybrid-with-a-floor is the published design requirement rather than a patch
invented to rescue the idea. Two qualifications, though, and the second is serious:

- **It is not about intrinsic plasticity at all.** `grep -i excitability` over the full text
  returns nothing; the paper factorises *synaptic* weight, while this project's homeostat is
  intrinsic and writes a threshold. The structural lesson transfers; the IP antecedent is
  Triesch, not Toyoizumi.
- **Its precondition may fail here.** The requirement is two-part: *"Hebbian plasticity must
  arrive on its own at a stable steady state, and this stability must not be disturbed by
  slow, ongoing homeostatic plasticity."* Part (a) is an assumption about the learning rule,
  and `ipoff` is evidence against it — removing the homeostat does not make learning drift,
  it collapses it at −7.42 SE.

**And v57 is nearer prior art than the section above admits.** `ip_pool = 9` regulates each
group's mean and frees the other thirteen directions — already "regulate the common level,
leave the pattern alone", the same division of labour a group-wide gain implements — and it
*halved* `aligned`. The only honest distinction left is mechanical: v57 subtracted a common
offset, which does not preserve a centroid through a nonlinear rate transfer, whereas a gain
preserves it exactly. That is real but narrow, and it is the whole case. Cutting the same
way, v74 found 39% rate drift costing 1.50 SE where `ipoff`'s 23% collapsed learning — so
what learning needs is per-neuron *pattern* regulation, not rate-level regulation, and a
cell-wide gain supplies none of the former.

**So the pre-flight comes before the build, and it is cheap:** does the additive threshold
homeostat stabilise the rate only, or the learned pattern too? If the pattern, this is
`ipoff` with a rate clamp and it is refuted before a line is written. That is answerable from
existing logs or a read-only probe.

**The pre-flight is now answered, from three logs already on disk, and it closes the lead.**
Each `aligned` read against its own shipped arm, so the ratios hold even though the absolutes
do not compare across generations:

| mechanism | pattern directions freed of 14 | `aligned` ratio | rate drift |
|---|---|---|---|
| v74 `ip_ramp_exempt = 1` | 1 | **0.957** | +39% |
| v57 `ip_pool = 9` | 13 | **0.518** | +19% |
| `ipoff` | 14 | **0.346** | +48% |

`aligned` degrades monotonically in the number of pattern directions freed and **does not
track rate drift at all** — v74 has the second-largest drift and the best `aligned`, v57 the
smallest drift and the middle one. So the additive homeostat stabilises the *pattern*, not the
rate, which is what the `ipoff` collapse was about all along.

**And the scale-invariance argument does not rescue it, because it concerns the wrong
quantity.** Invariance protects the *readout* from the homeostat; `aligned` is the learned
bias pattern, and it degrades because the gradient estimator needs per-neuron regulation. A
centroid that cannot see the gain is no help to an estimator that never read the centroid.

**The closure is arithmetic.** A group-wide multiplicative homeostat regulates the group's
scale — one direction — and frees the other thirteen, which is v57's count exactly. So it
should land at v57's halving unless the multiplicative form avoids v57's damage mechanism,
the 3× threshold push. But v74 tested avoiding that push in isolation: it avoided it (1.02×
against 3.3×) and bought nothing. The multiplicative homeostat is the conjunction of two
things that have each now failed alone — free thirteen directions and `aligned` halves; avoid
the threshold push and nothing happens. **Do not build it without first naming why the
conjunction should beat its parts.**

Three points, two of which share a generation: strong enough to stop a build, not strong
enough to publish as a closure. What would re-open it is a mechanism that regulates rate while
leaving per-neuron pattern regulation intact, or evidence that the estimator's need for
per-neuron regulation is itself an artefact of the additive form — a read-only question, and
the cheap version is a `stageprobe` arm rather than a seven-hour paired run.

### 7. CLOSED 2026-10-30 — the lesson's rotation peaks and declines

`dirbudget`, four taught rungs x 36 paired creatures, ~7.1 h. The open half of the
magnitude/direction split: the magnitude's budget curve was closed by `ctxscale` (~118 Hz, 94%
of asymptote) and nothing had laddered the direction.

| doubling | ratio | paired step | SE | signs |
|---|---|---|---|---|
| 3.4M → 6.8M | ×1.41 | **+0.0524 ± 0.0106** | **5.0** | 30/36 up |
| 6.8M → 13.6M | ×1.07 | +0.0121 ± 0.0099 | 1.2 | 20/36 — chance |
| 13.6M → 27.2M | ×0.87 | **−0.0244 ± 0.0092** | **2.6** | **25/36 down** |

**The free control passed bit-identically on all 36 seeds** — rung 1 is `echoname`'s taught arm
argument-for-argument, compared as printed strings rather than to a tolerance, with zero
creatures dropped. **Budget is not the limit on the rotation:** it buys the first doubling
decisively, then stops, and past 13.6M it is probably harmful.

**The two halves of the split differ in shape, not only in mechanism.** The magnitude's ratios
are 1.98, 1.38, 1.23 — monotone and asymptotic. The direction's are 1.41, 1.07, 0.87 —
non-monotone. Reward's half approaches a ceiling; the lesson's half overshoots and comes back.

**The null rises monotonically with budget** (0.251 → 0.275), so of the top-rung decline about
two thirds is the mapping degrading and one third is the null inflating as the voice separates.
Read against a fixed 1/k the reversal would have been invisible.

**A defect in the harness, recorded because it is the useful part.** The run's own verdict reads
"THE ROTATION SATURATES TOO", which is wrong for a ratio of 0.87 resolved negative at 2.6 SE.
Three verdict branches were written — growing, turning over, flat — all assuming ratio ≥ 1, and
the refusal was registered as "flat across 8×". The outcome space was never enumerated past
flat, so a reversal fell through to the flat branch. A null and a reversal are different
results; pre-registering a threshold does not help when the shape was not enumerated. Fixed
with an explicit declining branch; the numbers stand and only the printed sentence was wrong.

**Cite at the stated confidence.** The 5.0 SE first doubling is solid. **The 2.6 SE decline is
a hypothesis**, as the log said before the numbers existed, and it is a population tendency
rather than a law: per-creature peaks are 4 at 3.4M, 12 at 6.8M, 12 at 13.6M and 8 still at
27.2M. The shipped 6.8M sits below the population peak, but the 6.8 → 13.6 step is 1.2 SE, so
this run does not license moving it.

### 8. CLOSED 2026-10-30 — the creature falls silent as it trains; the rotation's decline is still unexplained

`dlvry`, one arm x 12 of `dirbudget`'s own creatures at 27.2M, ~1 h, plus a free re-analysis of
the ladder log that cost nothing (`tools/dirbudget_decline.py`).

**The ladder's x-axis was ticks, read as experience.** Its `trials` column is
`utt_word.size()` — scored, voiced, labelled — against a nominal `ticks / kVLTrialTicks`, and
the delivered fraction falls 71.4% → 60.7% → 46.7% → 35.8%. **8x of ticks buys 4.02x of
trials.**

**The creature quiets, decisively and early.** Delivery per tenth of one session: 0.716, 0.540,
0.366, 0.318, 0.234, 0.233, 0.247, 0.255, 0.257, 0.261. Last minus first **−0.4554 ± 0.0230,
19.8 SE, 12 of 12 creatures down**, silence 28.4% → 73.9%. **Every non-delivered trial is
silent** — 116568 = 39963 delivered + 76605 silent + 0 degenerate, residual zero. At the
**shipped 6.8M budget 42% of trials are already silent**: a standing tax on every experiment
built on this session, not a property of this ladder.

**And it refuses the hypothesis it was built to support.** The top step covers tenths 6–10,
where delivery is 0.251 and trending **+0.027 — rising**. Delivery bottoms out before that step
begins, and the step bought **1218 delivered trials, the largest absolute increment in the
ladder**, against 713 for the step below it which *gained* direction. So the decline is not a
delivery failure. Together with the *positive* spread/direction correlation (grew minus shrank
+0.0523 ± 0.0174, 3.0 SE), which refuses the magnitude-eats-direction trade-off, **both
candidates are now refused from opposite directions and the decline is more sharply unexplained
than before.**

**The method worth copying.** Nothing that moves the creature depends on `n_trials` here, so the
ladder's rungs are **nested prefixes of one trajectory**; with `utt_trial` recorded, one long run
reproduced all four published columns as **integer equality on 48/48 creature-rungs**. A 3.4M
pre-run certified the genome for 10 minutes against a 70-minute risk, and the genome was
identified by arithmetic — the ladder's instrument seed `0x135fc89 ^ 0xDB40` = 20260809 — rather
than by guessing which `ctx*.toml` was current. The root `ctx.toml` is stale (pre-v66).

**Two instrument defects, both recorded.** `dl_degen` reads 0.000 because under
`kVLScoreFormant` it is *unreachable*, not because the fault is absent, and the log now says
which — an unfalsifiable counter printed like a measured one is the `Expect::kRead` lesson in a
new place. And `tools/dlvry_predict.py` v1 keyed rows by a bare leading integer, so the delivery
table's `tenth` 1..10 overwrote seeds 1..10 and it reported 10 of 12 creatures mismatched while
the log plainly showed them agreeing; it now matches a row by shape and aborts on a duplicate
key. **Had it failed the other way it would have agreed silently.**

### 9. CLOSED 2026-10-30 — the homeostat is what shuts the creature up, and the silence is not the bottleneck

`silence`, two arms x 12 paired creatures at 13.6M ticks = 4857 trials, ~75 min. `ipoff` sets
`homeo.ip_rate = 0` and changes nothing else.

**IP owns the quieting.** Delivered fraction, last tenth minus first: shipped **−0.5919 ± 0.0176
(33.7 SE)**, `ipoff` **−0.1688 ± 0.0158 (10.7 SE)**, difference **+0.4231 ± 0.0267, 15.8 SE**.
With the homeostat off the creature keeps only **29%** of the shipped decay. Dose check exact:
the shipped larynx threshold climbs **+2.164** across the session while `ipoff` holds **exactly
+1.0000 at all ten tenths**.

**The decay is two things, not one.** Of the shipped −0.5919, the first-tenth step is −0.2184 and
the sustained slide is −0.3735. IP owns **84% of the sustained slide** and only **50% of the first
step**, so a −0.11 early transient survives removing it and is still unexplained.

**The shape prediction held; its stated reason did not.** The hypothesis said delivery would floor
*where the threshold stops moving*. The threshold is still climbing **+0.138 in the last tenth**
while delivery has flattened at 0.234 → 0.233. The floor is where the threshold stops *mattering*.
Pre-registering a shape and pre-registering its cause are different commitments, and only the
first one was met.

**The knob is refused, and the price was already on file.** `results/ipoff-136m.log` at this same
budget: dF1 **103.9 → 17.7 Hz, −7.42 SE, signs 1/18**, larynx rate 4.62 → 6.84 Hz. **The arm that
answers ~2.7x more trials learns ~6x less.** That rate column had sat in the log since September
as a cost, without being read as "the creature answers more often".

**So delivery is not the bottleneck on learning, now from both directions.** `dlvry` found the
rotation declining in the ladder's best-fed *step*; `ipoff` finds the best-fed *arm* learning
least. The silence is a symptom of the regulator doing its job, and the regulator is what the
learning rides on. **This closes why the creature quiets. It does not touch the rotation's
decline**, which stays refused from both sides.

**Two free integer controls, and one instrument defect.** The `ip` arm reproduced `dirbudget`'s
published 13.6M column on 12/12 creatures exactly, and `silence`'s ten tenths sum in pairs to
`dlvry`'s first five — ten integers, all exact — so the two runs are one trajectory read at two
resolutions and their −0.5919 / −0.4554 are not independent measurements. The defect is the
**mirror image of section 8's**: `tools/silence_nest.py` v1 recognised a table row by "leading
integer counting 1,2,3.. in order" and so harvested `dlvry`'s **seed** column, reporting 5 of 5
pairs differing. Section 8 was bitten by a `tenth` column posing as a seed; this was the same
collision from the other side, in a tool written *after* documenting it. **Anchor on the table's
header, never on the shape of a row.**

### 10. CLOSED 2026-10-30 — the decline tracks ticks, not answered trials; and a third of it is the null

Free from the ladder log, no run: `tools/dirbudget_experience.py`. Possible only because section 9
established that delivery varies **1.54x between creatures at one budget** (1648-2538 delivered at
13.6M), which makes ticks and experience separable axes in data already on disk.

**The pre-flight withdrew the primary before it was read.** Within a rung, more delivered trials
buys a LOWER null — **−0.435 ± 0.148 (2.9 SE)** at 6.8M, −0.305 ± 0.143 (2.1 SE) at 13.6M — so
`excess` rises with delivery for reasons that are not learning. The pre-registered primary
(d(excess) vs d(trials), paired) reads +0.564 / +0.551 / +0.721, all resolved, all the sign the
artefact predicts. **Recorded, not quoted.**

**What survives is the column the artefact pushes the other way.** `direction` alone correlates
with delivered trials **+0.447 ± 0.139 (3.2 SE)** at 13.6M and **+0.583 ± 0.123 (4.7 SE)** at
27.2M. The experience account predicts the OPPOSITE sign there — the creature that answered most
should be furthest past the peak and show least direction. It shows most. **So the decline does
not go with answered trials: the third candidate is refused.** Caveat stated, not buried: this is
between-creature, so a quality confound could mask a true negative, and it would have to beat a
4.7 SE positive to do it. Settling it needs delivery moved WITHIN a creature without moving its
learning, which `ipoff` cannot do.

**A correction to section 7's reading.** Across rungs the null RISES +0.0239, opposite to the
within-rung effect. The top step's −0.0243 is direction **−0.0158** plus null **+0.0085**: about
**a third of the published decline is the creature's own chance level rising**, not its direction
falling. Subtracting the null stays correct; the two parts were simply never separated.

**Where it points.** The cause must keep moving after delivery stops changing. Section 9 found
exactly one such thing: the larynx threshold still climbing **+0.138 in the final tenth** with
delivery already floored at 0.234 → 0.233. Next candidate, with a characterised knob — but
`ipoff` destroys learning, so it needs a partial dose, and the regulator is marked closed for
other questions. **Flag before spending the run.**

### 11. OPEN 2026-10-30 — `verify-teach` cannot pass on `dna/default.toml`, and the suite cannot say why

Found while discharging an owed item: the `Expect::kRead` failure branch in `run_verify` had never
been exercised, only read.

**It is exercised now, and it is correct.** `silence` was temporarily moved to `Tier::kFast` at
100000 ticks, the fast suite run, and the branch produced exactly what it promises
(`results/kread-fail-branch.log`): `experiments 24 as expected, 1 not`, the instrument named with
"instrument did not complete — apparatus, not content", and `verify FAIL` while `determinism` and
`pinned hash` both still read PASS. The spec was reverted immediately afterwards and the gate
re-run on the reverted source. **The log in `results/` is a deliberate FAIL and not a real gate
failure** — it exists because a branch that has only been read is not known to work.

**But smoking it first turned up the real finding, which is not about today's additions.**
`silence` on `dna/default.toml` refuses for a reason nobody was testing for: that genome has no
`kContext` module. Checked across the tier, **seven teach-tier instruments refuse immediately on
the suite's own genome** — `boundprobe`, `ctxgrid`, `ctxvocab`, `dirbudget`, `dlvry`, `silence`,
`ctxgain`, all exit 1 before any simulation. `run_verify` counts a false `kRead` as `failed` and
`ok = det && hash_ok && failed == 0`, so **`verify-teach` cannot pass on `dna/default.toml`.**
There is no verify-teach log anywhere in `results/` (the `phaseteach-*` logs are a different
experiment), which fits: the teach tier has apparently never been run green. `return false` on a
missing context module is the established pattern across ~20 sites and `boundprobe` has been a
`kTeach` spec far longer than `dlvry` or `silence`, so this is latent, not new.

**The message is honest; the accounting is what breaks.** The branch already names "a missing
module" as one of the reasons and says to read the instrument's output rather than that line. What
it cannot do is distinguish *this genome is not my subject* from *the apparatus broke* — which is
this project's own `Expect::kRead` lesson arriving one level up, in the harness that counts the
instruments rather than in an instrument.

**And the project already prescribes the answer, which makes this a mis-registration rather than
a gap.** The rule is written down: *produces a number → `kPass`; refuses because a mechanism is
off or a module is absent → `kGuard`; has a real bar it does not yet clear → `kOpen`.* `duty` is
registered `kGuard` for exactly this reason — "`dna/default.toml` has no context module and that
experiment legitimately refuses there" — and `v1probe` is the same pattern for the absent visual
cortex. **So the seven are mis-registered: they refuse on the shipped genome for an absent
module, which is `kGuard`'s definition, and they are declared `kRead`.** Under `kGuard` the
refusal counts as the expected answer and the teach tier can reach a verdict.

**But `kGuard` is only correct for `dna/default.toml`, and that is the whole difficulty.** These
experiments exist to be run on a context genome, where they do produce measurements — and a
`kGuard` spec that starts passing is reported as *"the mechanism it watches is no longer off"*.
So a flat `kGuard` trades a tier that cannot pass for a false alarm on every context-genome run,
which is precisely the genome-dependent label this project already has a rule about: **a label
correct on only SOME genomes must be PRINTED.** The expectation here is a function of the genome,
and no single `Expect` value encodes that.

**So the fix is better specified than "a design call", but it is still not a cleanup**: register
the seven as `kGuard` *and* have the suite state which genome the expectation is relative to, so
the label is disclosed rather than silently wrong half the time. Until then, `verify PASS`
certifying the FAST tier only is load-bearing and should not be read as conservative caution.

### 12. CLOSED 2026-10-31 — the threshold is not the clock either, and ALL FOUR candidates are out

`silence` at **36 creatures** instead of 12, joining its per-creature larynx-threshold climb
against `dirbudget`'s per-seed `direction`. It needed **no partial `ip_rate` dose**: `cell.thr[d]`
was already per creature per tenth and merely pooled at print time, so the flagged run was not
the run required.

**The published primary replicated at triple n:** `ipoff − ip` **+0.3924 ± 0.0167 (23.5 SE)**
against +0.4231 ± 0.0267, `ipoff` keeping **31%** vs 29%. Dose exact: +1.984 against 0.00000,
**0 of 36** creatures moving it. Creatures 0–11 came back **identical** — the seed ignores the
rep count — so the published numbers are still the same creatures'.

**The primary: −0.136 ± 0.190 (0.7 SE)**, predicted sign, unresolved against the ~0.28 this join
can resolve.

**AND THE PRE-REGISTRATION HAD THE CONFOUND BACKWARDS, which outlives the null.** It asserted
delivery "points the OPPOSITE way to the prediction" — an assumption about `r(climb, delivered)`
made before measuring it. Measured: **−0.713 ± 0.087 (8.2 SE)**. So the chain
climb → delivery → direction is **negative** and runs *with* the hypothesis; the two links
multiply to **−0.416** and the observed −0.136 is **smaller than the confound alone predicts**.
Had the primary resolved negative, the registered text would have called it "surviving the
confound a fortiori" over an effect the confound produces unaided.
**DERIVE A CONFOUND'S SIGN FROM ITS MEASURED LINKS; never assert it in the registration.**

**Post hoc, labelled so:** with delivery held fixed the partial **flips positive**, +0.247 ±
0.198 (1.2 SE). A lead for a different design, not an answer.

**So all four candidates have NO SUPPORT** — magnitude trade, delivery starvation, experience (at
the wrong sign), threshold. "No support", not "refuted": ±0.19 at n=36 bounds the effect rather
than showing absence. **And every test on this line has been BETWEEN creatures with the same
quality confound.** What it needs is section 13.

### 13. PREPARED, NOT BUILT — `thrfork`: move the threshold WITHIN one creature

The design that answers section 12, and the reason section 12 is a stopping point rather than a
closure. A brain that both regulates harder and rotates less reads identically to one where the
regulator causes the decline; only a within-creature intervention separates them.

**THE DESIGN, and the machinery already exists.** `snapshot`'s own experiment is the precedent
(`experiments_milestones.cpp:2931`): run a creature to a leg, `save_snapshot`, then restore into
**two** sessions and diverge them. Here:

- run to **13.6M**, which is where `direction` peaks;
- snapshot once;
- restore twice — arm `free` continues to 27.2M as shipped, arm `frozen` continues with the
  **larynx's intrinsic homeostat off** while every other module keeps its own.

**THE KNOB IS NOT `ip_rate`, AND GETTING THAT WRONG WOULD HAVE VOIDED THE DESIGN.**
`ip_rate` lives at `dna_.header().homeo` (`network.cpp:3103`) — it is **global**, so zeroing it
freezes every module and the arm becomes a full late `ipoff` rather than a larynx-only
intervention. The per-module knob is `ip_scale = asleep ? dm.ip_sleep_scale : dm.ip_wake_scale`
(`network.cpp:3111`), multiplying `ip_rate` per module (`:3121`). So the arm sets **vocal's
`ip_wake_scale` and `ip_sleep_scale` to 0**, which ships at 1.0/1.0. That also leaves
`scaling_rate` alive, so synaptic scaling keeps running and only the threshold path stops —
which is exactly the narrow intervention wanted, and `:3120` shows the module is skipped
entirely only if the syn and isp terms are zero too.

Both arms share a **bit-identical** first half by construction, so this is paired within the
creature and not across creatures. **Primary:** `direction(27.2M) − direction(13.6M)` in `frozen`
minus the same in `free`. **Prediction:** if the climbing threshold causes the decline, `frozen`
declines LESS, so the difference is **positive**. **What refuses it:** no difference at 2 SE, or
`frozen` declining more.

**THE PRE-FLIGHT THAT MUST PASS FIRST, AND IT IS A CODE QUESTION, NOT A SCIENCE ONE.**
`load_snapshot` "rebuilds `brain` from the snapshot's own genome", so a modified genome cannot be
passed to it. The genome is embedded **in place** in a buffer the caller owns and
`snapshot_genome()` returns a pointer into it, so the route is to **patch vocal's
`ip_wake_scale`/`ip_sleep_scale` inside the embedded blob** before restoring. That may trip
`kBadDna`, `kLayoutMismatch` or `kStateMismatch`. Both are floats on a `DnaModule` and not
structural — unlike `n_max`, which re-rolls wiring — so layout should hold, but **this is a
pre-flight and not an assumption.** The probe must do two things, because loading is not the
same as taking effect: (1) read the `SnapshotStatus`, and (2) run both restores a short leg and
show the larynx threshold climbing in `free` and **flat** in `frozen`. A patch that loads and
does nothing would otherwise look like a pass — the same shape as the `kGuard` lesson in §11 and
the liveness-is-not-dose rule. If it refuses, the fallback is a runtime per-module scale hook,
which is a kernel change and needs its own gate.

**THE CONFOUND THIS DESIGN STILL CARRIES, stated before the run.** Freezing the larynx threshold
is a *partial, late* `ipoff`, and `ipoff` destroys learning at −7.42 SE. It is far narrower —
one module, second half only, after learning has peaked — but narrower is not clean. **So
`aligned` is a mandatory co-primary:** if `frozen` declines less *and* `aligned` holds, the
threshold was the clock. If `aligned` collapses, the arms differ in learning and the result is
the same confound in a new place. Also mandatory: a dose check that `frozen`'s threshold really
stayed put and `free`'s really climbed.

**THE NEAREST PRECEDENT IS A WARNING, AND IT IS ABOUT A DIFFERENT MODULE.** The genome records
(`results/ctxgrid.toml:866-873`, from `g3probe`) that relaxing **central's** intrinsic plasticity
sparsens it — population sparseness 0.91 → 0.85 → 0.79 as `ip_wake_scale` goes 0.25 → 0.05 → 0 —
and **separability collapses to chance at zero, "because nothing is left holding the neurons in
their dynamic range."** That is the same thing `network.cpp:3219` says the threshold path is for,
so it is the failure mode to expect.

**BUT IT IS CONTRADICTED FOR VOCAL BY A MEASUREMENT ALREADY ON FILE, which is why this design is
worth running.** `ipoff` at 13.6M did not make the larynx fall out of range: rate went
**4.62 → 6.84 Hz** and the arm **delivered 2.7× MORE** trials than the shipped one
(`results/ipoff-136m.log`, and §9). With the homeostat off the voice gets *louder and more
answerable*, not silent — it learns 6× less, but it stays alive. So the central result is a
reason to **read the dose check and `aligned` before the primary**, not a reason to expect an
uninterpretable arm.

**Cost:** 18 × (13.6M + 2 × 13.6M) = **734M ticks, ~2.5–2.7 h** — the snapshot saves re-running
the shared half, which a two-full-arm version would spend 979M on.

### 14. ~~PREPARED, NOT BUILT — `f1decode`: is 13.7% the creature's, or the centroid's?~~

> **CLOSED 2026-10-31 BY MEASUREMENT — DO NOT BUILD. READ §23.** The question was whether the
> blind directions hold word-specific structure the centroid cannot reach. They do not. Against
> an index-randomised arm running the same rule on the same trials, the blind component is
> **never above the floor** and is **3.8 SE below it** at the largest budget, by a gap that
> widens at every rung — randomising the word makes the creature write *more* into the blind
> subspace. `seen` exceeding its null says only that the off-ramp write is **anisotropic**,
> which is a property of the write rule and the wiring, not word identity.
>
> Two of the reasons this section gave are themselves withdrawn, and are struck below rather
> than deleted so the error stays visible: `seen` does **not** rise (+0.0521 ± 0.0997 over 4×,
> flat), and the residual is **not** transient (same measurement). The decoder is closed because
> there is no word in there — not because the structure is draining away.

~~**READ §15 AND §20 FIRST. §20 RAN THE DECIDER AND `f1decode` CAME OUT WEAKER, NOT STRONGER.**
The residual IS large (it exceeds `aligned` at every budget) but `seen` RISES +0.074 per
doubling, so the lesson is concentrating onto the readout's own coordinate as it learns and the
residual is transient rather than a reservoir.~~ A decoder would claim a quantity the creature is
already giving up. With §15's arithmetic that is two independent reasons — one algebraic, one
measured — not to build this yet. ~~**The fork that would settle it is one more doubling** (see
§20), not a decode.~~ It can say how much the centroid discards; it
cannot promise that recovering it raises the asymptote, and §15 is the reason.

~~The build `ctxscale`'s new `seen` column decides whether to make. **Run it only if `seen` comes
back near its derived null of 1/√13 = **0.277**; if `seen` ≈ 1.0 the lesson writes along the
centroid's own coordinate, the blind directions are empty, and this is not worth building.~~
**That gate was answered and it was the wrong gate.** `seen` came back at 0.57–0.63, between the
null and 1.0 — the case the gate read as "worth building". It took a second control, not a
bigger `seen`, to show the blind directions are word-free. **A magnitude above its isotropic
null is not evidence of content; the control for content is an index-free arm.**

**THE QUESTION.** Keemink & van Rossum (2025) find the population-vector estimate is most biased
exactly when few neurons are active — this creature's fourteen — and that ML/Bayesian decoders
carry substantially less. Three results written up as facts about the creature are candidates for
being facts about the **estimator**: the centroid using **13.7%** of its range, output potency
being **signed distance** from it, and the readout **pinning at one end** under the half-centre.

**WHAT IT IS NOT.** Not read-only. I proposed it as a read-only re-decode of archived logs and
that was wrong: **no log carries trial-level per-neuron rates**, only the early/late means at
`experiments_milestones.cpp:6282`. It needs an instrumented run that dumps, per scored trial, the
F1 group's 14 rates with the produced F1 and the live word.

**THE DESIGN THAT ISOLATES THE ESTIMATOR.** Decode F1 from the same dumped trials two ways — the
shipped centroid, and a maximum-likelihood or Bayesian decoder — and compare on **identical
data**. That is what makes it a test of the readout and not of the creature. **Primary:** the
decoders' F1 separation between words, paired per creature. **What refuses it:** the ML decoder
recovering the same separation, which means the centroid was never the constraint.

**AND THE PUBLISHED CAUTION THAT DECIDES WHETHER A GAIN MATTERS.** Chase, Schwartz & Kass (2009)
report that an OLE's open-loop advantage **need not survive closed loop, because the subject
adapts to whatever decoder it is given.** This creature *learns against its own readout*, so an
offline decoding gain is **not** a prediction that changing the readout would help — it bounds
the information present, nothing more. Any build that follows has to re-learn under the new
readout before claiming anything. **Do not reach for the OLE specifically:** Xu et al. (2019)
found OLE and vector reconstruction *worse* than most decoders and most sensitive to violated
assumptions.

**Cost:** the dump rides on an existing teach-tier session, so one 13.6M run at n=18 is
~250M ticks (**~1 h**); the decoding itself is offline and free.

### 15. CLOSED 2026-10-31, FREE — readout sensitivity is a SPEED knob by arithmetic

No run. Two logs on disk and the readout's own algebra, and it prices §14's ceiling prize before
§14's decider has reported.

`read_group` is `Σ rᵢ^β pᵢ / Σ rᵢ^β` with uniform `pᵢ`. With `rᵢ = r̄(1+εᵢ)` and homeostasis
holding `Σεᵢ ≈ 0`, the excursion is `β · (1/n) Σ εᵢ (pᵢ − ½)` — and that sum **is** `aligned`.
So β multiplies `aligned`'s effect linearly, and with `dF1 ~ aligned^0.61` the composition is
`dF1 ~ (β·aligned)^0.61`.

| budget | % of 234 Hz asymptote | predicted dF1 ratio | observed | miss |
|---|---|---|---|---|
| 3.4M | 27% | 1.280 | 1.310 | **−2%** |
| 13.6M | 47% | 1.324 | 1.078 | **+23%** |

**Accurate far from saturation, overpredicting near it — and NOT because the loop fights
harder:** fitting `aligned ~ β^−c` gives c = 0.416 at 3.4M and **0.336** at 13.6M, so the
compensation is *weaker* at the larger budget. The composition fails because the 0.61 relation
flattens toward the asymptote.

**So a readout change buys speed, not ceiling**, which is what `poolbeta` concluded descriptively
and now has arithmetic for. The compensation's cause is in `poolbeta`'s own words — the drift
`−Cov(e, perturbᵢ)` factors through `dF1/d(drive)`, so a more sensitive readout needs less bias
to cancel the same error and less bias is what it learns. **That is Chase, Schwartz & Kass (2009)
measured here in September without the citation:** an open-loop decoder gain need not survive
closed loop because the subject adapts to its decoder, and this creature always does.

**AND IT KILLED AN EXTRAPOLATION OF MINE WITHIN MINUTES.** From the 3.4M point alone I derived
that sensitivity **k ≈ 4.3** would reach the ceiling — far more reachable than `aligned` ×2.34
against a best-ever ×1.2, and wrong, because it assumes the composition holds at the budget where
the ceiling is measured. The 13.6M row refutes it. **The "interpolated crossing quoted as a
prediction" prohibition below covers exactly this**, and it was one grep from being published as
a target. Two further signs it was never safe: β=3 was worse than β=2 on every column, so the
axis is not monotone, and a hard argmax should be untrainable (`centroid-is-steerability`).

**LIMITS, because this is enough to stop a build and not enough to be a law:** two budgets, one
β value, `dF1 ~ aligned^0.61` is itself fitted, and one point agreeing to 2% is one coincidence
by this project's own rule. What is established is the direction and rough size of the failure.

### 16. PREPARED, NOT BUILT — `aimaxes2`: seven of the nine axes have never been measured

**The largest cheap prize on record, and it needs no new learning mechanism.** This is pure
code space.

`senses.cpp:382-384` maps the larynx's nine groups to **F0 (group 0), F1 (2), F2 (3), F3 (4)
and three bandwidths (5–7)**. `aimaxes` measured the steerability and cross-talk of **groups 2
and 3 only** — "a second steerable formant axis exists and nothing has used it" was about the
second of nine, and the remaining seven have never had either quantity measured.

**WHY THIS IS NOT REFUSED BY `aimaxes`'S OWN CONCLUSION, which is the first thing to check.**
That memory ends "F2's extra range is offset by proportionally more scatter, so it is
independence and not resolution — ~2–3 levels per axis = 4–9 words in 2-D." True, and it refuses
*stretching one axis*: more Hz on F2 buys no more levels on F2. **But levels MULTIPLY across
independent axes.** 2–3 levels on two axes is 4–9 words; on four axes it is **16–81**, with the
same per-axis resolution and no change to the learning rule. The quantity to grow is the number
of axes, not the range of any one — and `ctxvocab` already supports that direction: nine words
(3×3) name as well as six (3×2 or 2×3), with the whole cost paid in F1 *reach* and naming excess
**conserved at 0.6 SE**.

**THE RUN, and it reuses an existing read-only protocol.** `aimaxes` was 6 creatures × 2 groups
× 5 rungs, fresh creature per rung, read-only. Extend it to all nine groups: per group, the
own-axis swing, the cross-talk into every other axis, and the scatter. ~4.5× `aimaxes`'s cost and
still cheap because nothing learns.

**THE GATING QUESTION IS THE EAR, NOT THE LARYNX, and it is a second read-only measurement.**
Naming needs the distinction to be *heard* and *scored*, not merely produced. `vocab` shows the
ear handles F2 — /i/ vs /u/ differ almost purely in F2 and score **0.900** — but whether it
resolves F3, F0 or bandwidth is unmeasured. `kVLScoreFormant` also scores F1/F2, so the scorer
and reward need extending before any naming claim. **So `aimaxes2` is stage one of two, and a
steerable axis the ear cannot resolve is worth nothing for naming.**

**WHAT WOULD REFUSE IT:** cross-talk. The prize is a product of *independent* axes, so an axis
that moves F1 or F2 while moving itself adds no words — it reallocates. `aimaxes` set the bar by
measuring 0.01 ratio between groups 2 and 3, so there is a precedent for what independence looks
like here.

**ONE CAVEAT THAT MATTERS FOR ONE USE AND NOT THE OTHER:** the transmission tape carries
**formants only** and f0 stays the host's, so an F0-based distinction will not survive
creature-to-creature teaching. Irrelevant to a device taught by a human; fatal to a lineage.

## Standing prohibitions

These are places where fitting has already cost this project a result, and they are written
down so the next person does not pay again.

- **Do not read a magnitude above its isotropic null as evidence of content.** `seen` sat at
  0.63 against a null of 0.28 and was written up as the lesson writing structure the readout
  cannot see — which it is. But an index-**randomised** arm, same rule and same trials, wrote
  *more* into those same directions (3.8 SE more at the top rung). The null answers *is the
  write shaped?*; only a control with the label destroyed answers *is the shape about the
  label?*. Those are different questions and the first one was being quoted for the second.
  **And the trap is worse than a forgotten control.** `ctxscale`'s own headline gate is
  `excess = dF1(ema) − dF1(ema-rnd)` — the index-free arm was wired to that statistic from the
  start. The *same run* therefore carried one correctly-controlled quantity and two
  uncontrolled ones (`aligned`, `residual`), and the uncontrolled pair got read as evidence.
  The control's existence is exactly what made the omission invisible: the run looked
  controlled, because it was, just not on the columns being quoted. **Check the control is
  wired to the statistic you are about to quote, not merely present in the experiment.**
- **Do not compare `excess` between creatures that delivered different amounts.** It is
  `direction − its null`, and the null is an estimate that tracks how much was said: within a
  rung, more delivered trials buys a null lower by −0.435 ± 0.148 (2.9 SE). Any between-creature
  statistic built on `excess` inherits that, and the contamination points the same way as the
  obvious hypothesis — more experience, more apparent learning. Use `direction` with the null
  reported beside it, or match on delivery.
- **Do not fit the magnitude of the lever.** The direction is established on three targets;
  a distance law was fitted to two points, matched them to 0.1%, and died on the third.
- **Do not fit the extent curve.** Three points, a one-parameter family, and the exponent is
  not constant across them.
- **Do not price the matched cluster prior.** It was argued from the index's own *output*
  occupancy, where the result it rests on is about the *data's* class distribution — and the
  words here are exactly balanced, so a uniform prior is the correct one.
- **Do not subtract one bound from another.** A decoder's held-out accuracy is a *lower bound*
  on what a signal carries, never an estimate of it, so the gap between two decoders measures
  the decoders. One run's pre-registered primary read 5.4 SE and had to be retired: applying
  the same reader to a normalised copy of the identical feature moved one bit by 0.204 and the
  "asymmetry" *grew*. To compare content across two quantities you need a ceiling.
- **Do not match on trials when the intervention changes how much the creature runs.** A gated
  creature lives 61% of the ticks, so trial-matched arms differ in experience as well as in
  gating. Match what the per-tick processes consume — and note that matching ticks un-matches
  reward, so the two designs bracket rather than settle.
- **Do not interpret the direction of a difference-between-arms before decomposing it.** A
  gated collapse of 81% turned out to be the *control* converging, not the treatment failing,
  and the columns that showed it were in the same log throughout.
- **Do not quote an effect size from an `--allow-short` run.** Those print under a banner
  because their numbers are not measurements. A two-seed smoke showed an index decaying; at
  n=36 and full budget the sign reversed at 4.6 SE. Smokes are for vacuity — does the mechanism
  engage, do the arms differ, is the control live — and never for magnitude or direction.
- **Do not trust an arm's label over its `ctx_match`.** A genuine host-written index reads
  1.000; `ctxgrid`'s "oracle" arm reads 0.7498 because the experiment sets `context_slots` and
  inherits `context_source`, so on a source-4 genome the drive it injects is invisible to the
  index. Five experiments share that shape and `tools/audit_inherited.py` lists them.
- **Do not price a loop without naming its enclosing function.** A cost claim reported
  bookkeeping beating delivery by 226× by assuming a sweep ran per tick; it runs per reward
  cash-in, and the run that priced it never applied reward, so the loop executed zero times.
- **Do not chain build, gate and launch in one command.** A skipped rebuild sent a three-hour
  run at a stale binary while a waiter that only asked "is anything running" declared the
  machine free. Verify the binary is newer than the source, verify the *invocation* reports the
  configuration you intended, and make the launch conditional on `verify PASS`.
- **Do not refuse a rate-thresholded mechanism on a smoke's event rate.** A gate keyed on a
  rate threshold ran at 0.55 events per trial on a 17×-short smoke and 1.76 at full budget,
  because the rate distribution is still settling early in a creature's life. Two mechanisms
  were nearly discarded on that number. Only a handed window is budget-invariant.
- **Do not quote a gap between two logs before checking the creatures match.** Appending a
  module re-rolls the noise, so two genomes are two different populations; `off` and `oracle`
  are the cheap test, since no mechanism edit can move them. A prize quoted across families
  as a point estimate was a range (+11.1 ± 7.1 against +27.4 ± 7.0).
- **Do not settle an old lead by re-analysing it more sharply.** Spend the compute on units
  the claim has not seen first. A paired re-read of the discovery sample reproduced a 2.2 SE
  lead's point estimate to four decimals and landed it on exactly 3.0 SE, where 18 fresh
  creatures gave 0.4 SE. Report the discovery half and the fresh half separately, because
  the pooled number contains the half that generated the claim.

### Prepared and runnable, as of 2026-10-31 — read this before the 2026-10-24 list below

The list below is still right about the demonstration. These are the three things that are
**specified, costed and gated**, in the order their dependencies allow:

| # | run | gated on | cost | what refuses it |
|---|---|---|---|---|
| — | `ctxscale` with `seen` | **running now** | ~3.0 h | — (it is the decider for `f1decode`) |
| 13 | `thrpatch` pre-flight | nothing; build + gate | **minutes** | `load_snapshot` refusing a patched genome, or the patch loading without taking effect |
| 13 | `thrfork` | `thrpatch` passing | ~2.6 h | no `frozen`−`free` difference at 2 SE, or `aligned` collapsing |
| 14 | `f1decode` | `seen` ≈ 0.277, **not** ≈ 1.0 | ~1 h + offline | the ML decoder recovering the centroid's own separation |

**`thrpatch` is the cheapest and should go first** whatever else happens: it costs minutes, it
gates a 2.6 h run, and it answers a code question (can a snapshot be restored with a patched
per-module genome, and does the patch take effect) rather than a science one. The two
prohibitions it protects against are already on record — a mechanism that is wired is not a
mechanism that is dosed, and a check only the thing being tested performs is not evidence.

**`f1decode` may not need building at all**, which is the point of putting `seen` on the
currently running `ctxscale`: if the lesson writes along the centroid's own coordinate, the
twelve blind directions are empty and the within-group half of that lead closes for free.

### What is actually next, as of 2026-10-24

1. **The demonstration, and most of it already exists.** The project's stated goal is a small
   creature trainable for one specific purpose, and the capabilities for that are each
   established separately: it repeats, it is taught, it answers, it names four words
   conditionally, it transmits a two-bit code with 18/18 inheritance. **`tutorgrid` already is
   the assembly for two of those** — one creature learns a four-word conditional code and
   passes it to another. The missing piece is the **echo** on the *same* creature, which is a
   read-only property `imitate` currently measures on a freshly hatched one. The composition
   question is not vacuous: this project finds composition fails in time (two gestures) and
   holds in space (orthogonal targets keep 0.84 against 0.22), and the 2×2 grid *is* the
   orthogonal case. Cost: lifting `imitate`'s windowed classifier into a trained creature.

   **BUILT 2026-10-27 as `echoname`. Its first pre-flight refused it; that refusal was a
   bug in the harness and is withdrawn, and the corrected pre-flight CLEARS.** The lift is
   done and verified: `imitate_on_session` reads the
   windows off a `Session` the caller owns, bit-identical on all five windows and all five
   columns against `results/delay-battery-off.log`, with `run_vocallearn_session` carrying
   an optional echo buffer so the echo comes off the live creature after its lesson. Four
   arms — `fresh` (no session), `none` (a session, no reward), `taught` (the lesson),
   `nodrive` (a session with no CtxDrive).

   **The first pre-flight read the echo at chance on all three session arms, and the cause
   was the hook passing `silent = true` into `imitate_on_session`** — a flag with exactly
   one meaning in that function, *the caregiver never speaks*. It is `imitate`'s own
   negative control. Every arm that lived a session was scored on a creature that was never
   spoken to, while `fresh` reached the same reader through the defaulted wrapper and
   played words. **A positive control that reaches the measurement by a different code path
   from the treatment arms is not a control** — it certifies the reader and says nothing
   about the protocol. The two "corroborating diagnostics" (auditory drive halved, voiced
   fraction down two thirds) are both what a word that never plays produces, so they were
   one fault counted three times rather than independent confirmation.

   **Corrected, the echo survives a teaching session** — still vacuity readings and not
   effect sizes, since the log keeps its `--allow-short` banner:

   ```
   arm      n    echo          its EAR  shuffled  aud WHILE  voiced
   fresh    12   0.896+-0.007  0.626    0.508     352.6      0.772   <- bit-identical across the fix
   none     12   0.876+-0.007  0.690    0.513     281.1      0.700
   taught   12   0.920+-0.005  0.713    0.507     279.4      0.655
   nodrive  12   0.895+-0.011  0.685    0.492     280.9      0.799
   ```

   `fresh` returning bit-identical is what licenses reading the rest. Every vacuity
   condition clears — positive control live, four arms distinct, shuffled control inside
   [0.40, 0.60], the ear below the voice in the scored window on every arm. **The primary's
   one open leg is the 4th-rung guard**, naming above its own per-creature null at 3 SE,
   which reads 1.3 SE because 142 teaching trials cannot form four-word naming against a
   6,800,000-tick minimum. A budget artefact, not a content failure — the state in which a
   pre-flight says *launch*.

   **Withdrawn with the refusal:** the caution this put on M1b. Nothing here ever showed
   that a creature which has lived a teaching session stops repeating. And the vision lead
   it named is real in the code — `run_vocallearn_session` never drives vision while
   `imitate_on_session` does every frame — but there is now **no deficit for it to explain**,
   so it is not a lead.

   **THE FULL-BUDGET RUN LANDED AND MET BOTH LEGS** (`results/echoname-full.log`, n=36,
   2 h 14 m): echo on the taught arm **0.955** against the 0.75 bar, naming **+0.1790 ±
   0.0197 (9.1 SE)** over its own per-creature null, on the same 36 creatures. Paired
   `taught − none` on the echo is **+0.0823 ± 0.0101 (8.1 SE)** — teaching four-word
   naming does not cost the echo, it raises it — and living a session at all costs
   −0.0156 ± 0.0070.

   **Two things the verdict did not account for, and the second is why this is not yet
   called composition.** The arms are not the same sample size: `taught` scored a mean of
   159 of 200 echo trials against a full 200 on the two unrewarded arms, because an asleep
   echo trial is skipped rather than scored and reward drives the creature into sleep
   during the echo phase. Now printed as a `DIFFERENTIAL SAMPLING` line; absent at 400,000
   ticks, so it is a consequence of a *long* lesson. And **the excess may be the naming
   response** — the scored window is 200–600 ms after the word stops, exactly when a
   taught creature answers, so read off the vocal module the two may be one mechanism
   measured twice. The echo exists at 0.867–0.888 with no lesson, so naming does not
   create it; it is the +0.0823 excess that is unseparated.

   **THE DISCRIMINATOR RAN AND SETTLED IT** (`results/echoname-yoke-full.log`, n=36,
   5 h 00 m, all four original arms **bit-identical**). A `yoked` arm takes reward in the
   same proportions on the same schedule, half a trial out of phase, with no word→vowel
   mapping; the reading rule was fixed in the source before the run.

   ```
   yoked echo 0.940+-0.007        (none 0.873, taught 0.955)
   paired yoked  - none    +0.0673 +- 0.0107  (6.3 SE)   RESOLVED -- recovers 82%
   paired taught - yoked   +0.0151 +- 0.0082  (1.8 SE)   unresolved, 2 SE bound = 20%
   yoked naming over its own null   -0.0027   (taught +0.1790)
   ```

   **The yoke learned nothing and got almost all of the benefit**, so the excess is reward
   acting on the voice rather than the naming response: the echo and the naming are
   genuinely **two capabilities**, and the composition claim is earned. The remainder is a
   bound and not a hold — at most a fifth of the excess could be naming's own contribution.

   **And the yoke reproduces the whole syndrome** with zero learning: the voicing collapse
   (0.123 against 0.082 and `none`'s 0.765), the sleep (162 scored trials against 159 and
   190), the ear suppression (0.602 against 0.574 and 0.682). **Reward, not the lesson,
   moves this creature's operating point** — and the direction is the striking part: a
   quieter, sleepier creature reads its own vocal code *better*, which fits larynx→ear
   being negative feedback and the classifier reading vocal-module spikes per neuron.

   **The lead this opens**, stated as a lead: nothing has asked how far the quieting goes. A
   quieting knob that is not reward — the voicing gate, or scaling the larynx drive
   directly — would say whether the echo can be bought without a lesson at all.

   **THE LEAD RAN THE SAME DAY AND REFUSED THE ACCOUNT** (`quietecho`,
   `results/quietecho-ctxgrid.log`, n=36, 10 arms, ~30 min). Ten freshly hatched
   creatures-worth of arms, no lesson and no reward: a seven-rung `voicing_threshold`
   ladder, which silences the larynx while leaving the module's spikes untouched, plus
   `self_gain` 0.0 and 2.0, which leave the larynx alone and change the ear. Two unrelated
   code paths cutting the proposed chain in different places.

   ```
   voiced fraction 0.773 -> 0.000, and the echo does not move
   slope, echo  per unit voiced   +0.0001 +- 0.0077   (reward's own: -0.1048, committed)
   slope, ARTIC per unit voiced   -0.0152 +- 0.0087

   SAME BRAIN, OPPOSITE VOICE   deaf (94% voiced) vs thr 1.15 (0%): artic +0.0000 +- 0.0000
   THE VOICED-MATCHED PAIR      thr .70 vs loud, voiced matched +0.0002 +- 0.0028 (0.1 SE),
                                echo -0.0896 +- 0.0080 (11.2 SE)
   SELF-AUDITION REMOVED        shipped -> deaf, echo +0.0001 +- 0.0060
   ```

   **The masking account is refused, not merely unsupported.** The range is matched (the
   ladder brackets the rewarded arms' 0.082–0.123), so on the column reward's +0.0673 was
   measured, quieting accounts for **at most 15% at 2 SE** with a point estimate of zero.
   The ladder was even handed an advantage it failed to use — arrival *rose* 13% as the
   voice went silent, because a creature that stops phonating stops masking the caregiver.

   **A flat result that is a measurement, because `loud` is the sensitivity control.** The
   same instrument, same n, same reader resolves `loud`'s −0.09 at 11.2 SE. What `loud`
   cannot carry is a mechanism: its arrival fell with its gain, so masking and arrival are
   confounded there.

   **A trap worth remembering:** `m3_vocal_features`' feature 10 **is** the voiced fraction,
   and `holdout_accuracy` z-scores every dimension with the sd floored at 1e-9 — so an
   exactly constant feature is harmless while a *nearly* constant one is amplified and can
   dominate. The primary was therefore the **articulator** column (`scored_artic`, voicing
   and loudness dropped). Replicated on `dna/default.toml` as the `kPass` gate obligation.

   **BOTH ARTEFACTS IN `echoname`'s OWN NUMBER ARE NOW CLOSED** (2026-10-28,
   `results/echoname-audit-full.log`, one 5-hour re-run). The trap above was turned on the
   headline itself, and a second one found beside it.

   ```
   FEATURE 10          yoked - none, echo +0.0672 +- 0.0107 -> ARTIC +0.1153 +- 0.0136 (8.4 SE)
                       taught - yoked unresolved on BOTH, so the 20% bound is unchanged
   TRIAL SELECTION     reward thinned scored trials 200 -> 159; an asleep trial is SKIPPED,
                       and dropping uninformative rows RAISES held-out accuracy
                       slope it would need -0.004135 / trial
                       slope measured      +0.000570 +- 0.000863   5.5 SE THE WRONG WAY
                       both arms >= 180/200, n=6: +0.1213 +- 0.0360 (3.4 SE), FULL effect
   IDENTITY            180 rows x 11 published columns came back character-identical
   ```

   Three explanations for the echo excess are now closed — not the audible voice, not the
   classifier's voicing feature, not selection on which trials were scored. **Reward is
   still the cause and the mechanism is still unidentified.** The untested leg is the vocal
   **module**'s own activity (`target_rate_hz`), which voids the calibration invariant.

   **One correction and one gap, both recorded rather than left implicit.** The articulator
   contrast looks 1.6–1.7× larger raw; on log-odds it is 1.09–1.18×, because `echo` sits at
   0.955 with no room — the claim survived its audit, it was not understated. And the
   `shuffled` control is computed on the eleven vocal features only
   (`experiments_milestones.cpp:1603`), never on the seven articulator ones; the transfer is
   sound but is an argument, not a measurement. Add
   `holdout_accuracy(probe_timbre, shuffled, train)` next time that file is touched.

   **What is NOT closed, sharply:** neither knob makes the vocal **module** quieter —
   CONTROL B proves `voicing_threshold` leaves the brain's trajectory untouched. "A quieter
   module reads better" survives untested, and reaching it means moving vocal's
   `target_rate_hz`, which voids the calibration invariant and needs a recalibration pass.
   **And one number is owed:** `echoname` computes no articulator column, so whether its
   +0.0673 survives dropping voicing and loudness is open. One 5-hour re-run settles it.

   **The n was re-committed at 36**, in the source with its reasoning, before the full run:
   `ctxgrid`'s own n, the run that established four-word naming, which is the precedent the
   naming leg is held to. The old 12 was provisional — a pre-registration voided before it
   read a primary is not binding — and 36 borrows its power from a precedent rather than
   choosing it after seeing a number. Cost: ~20x the 7.8 min short run on 14 cores, so a
   **~2.5–3 h estimate**, and this project has missed a runtime prediction by 2.3x before.
2. **CLOSED 2026-10-26 — the baseline debt, and it found a defect.** `verify-long`
   completed on `dna/default.toml` in 37.3 h: **no regressions** (zero `kPass` failures,
   zero `kGuard` flips), but **21 discrepancies, every one `expected FAIL (open
   milestone), got PASS`**. 16 of the 21 functions end in an unconditional `return true;`,
   so `Expect::kOpen` on an instrument is unfalsifiable — the milestone line fires the
   moment the experiment runs. **The replacement item is the gate repair**: add the
   missing third `Expect` class for instruments, and promote the genuine criterion cases
   (`salience`, `blockwhere`, `partprobe`, `orthovocab`) one at a time on a reading of
   each. No runs, and the gate is unreadable until it is done.
2b. **CLOSED 2026-10-26 — the delay axis.** Found by reading the kernel, measured, and
   closed in an afternoon. §3.5 moves per-synapse delays and `delay_floor_frac` had never
   been measured. It is causally live (the off arm hashes differently) and behaviourally
   null: the pre-registered `g2` primary read **−0.0019 ± 0.0041** at n=36, and at **4×
   the range** — axis 2–7 ticks widened to 5–28, travel 1 tick to 3–8 — the **identical
   −0.0019 ± 0.0045**. The effect does not scale with its lever arm, which refutes the
   range account on the one statistic fixed before either run. **Do not build the signed
   delay rule.** The creature is insensitive to delay *changes* and at most mildly
   sensitive to the absolute *level*: rate-coded learning, not timing-coded — the fourth
   line to land there after synfire, burst plasticity and the sparse time basis.
3. **CLOSED 2026-10-27 — the inherited index source is disclosed, and now checked.**
   All five experiments that write `context_slots` and inherit `context_source`
   (`ctxfour`, `ctxvocab`, `areax`, `rpeprobe`, `ctxgrid`) print their source and what it
   means, from one shared helper; `ctxgrid`'s inline copy was folded into it. Both branches
   were smoked on real genomes, and `results/` holds 64 genomes at source 0 against 6 at
   source 4, so the ambiguity was live rather than theoretical. **`ctxfour`'s honesty is a
   property of the RUN, not of the experiment** — its `ctx_match` of 1.000 says that
   particular log was on a source-0 genome, and the same code on `results/ctxgrid.toml`
   would be derived. Disclosure only: no behaviour, no number, so the published logs stay
   comparable. `tools/audit_inherited.py` grew a `discloses` column and a non-zero exit, and
   its failing branch was exercised on a stripped copy rather than assumed.
4. **The `env2` baseline audit**, still owed. Cheap to *decide*, not free to *run*:
   block 1 of the n=144 conscience settle took its baseline from the twenty-arm
   `ctxself-env2.log` instead of a dedicated four-arm run, so the audit is 36 creatures on
   a dedicated baseline. §2 already records that blocks 2–4 clear without it
   (+15.35 ± 4.34, 3.5 SE), so this cannot overturn the claim — it can only tighten or
   annotate it, and that is why it keeps losing to everything else on this list.

## How to not waste a run

Collected because each line cost one.

1. **Grep the logs for the condition, not the name of the experiment.** **Four times now** the
   answer was already on disk. A question written down as a *lead* reads as not-yet-done — and
   the fourth time (2026-10-29) the lead was a sentence *this project had written seven weeks
   earlier*, carried forward as current without re-reading the later sections of the same file.
   A lead you wrote yourself is not evidence that it is open. One `grep results/` each time.
2. **Read the anchor of the log a number comes from before reading the number.** A search
   across runs is a join on the arm name, and the arm name does not carry the tick budget.
3. **Check the mechanism changed before measuring whether it helped**, at smoke length, and
   refuse the long run if it did not.
4. **Print the power of a test beside its result.** A criterion nothing can clear, one
   almost anything can, and one only the *best* outcome fails have all shipped here.
5. **Never quote a product or a mean of ratios.** Measure the ratio directly on both sides;
   a declined product-of-means was wrong by more than the deficit it was estimating.
6. **Derive every new constant** from something already measured, and say from what.
7. **Restore the default arm set in the commit that writes up a focused run**, and verify
   the restore by running the experiment rather than by reading the diff.

### 17. CORRECTION 2026-10-31 — `seen`'s null was wrong twice, and the estimator was the wrong one

Found by reviewing the code written the same day, while the run using it was still going. **The
run's data is unaffected** — `align` and `residual` are per-creature primitives and valid; what
was wrong is the statistic built from them and the null it was read against.

**THE ESTIMATOR.** `seen` was computed per creature and then averaged — a **mean of ratios**,
which this project has a standing rule against. Monte Carlo on an unstructured write, 40k draws
at n = 9: the mean of ratios reads **0.2254** where the pooled **ratio of sums** reads
**0.2703**. Biased low by **19%**, and low is the direction that reads a real effect as the
null. Now pooled from `align²` and `residual²` and jackknifed, which is the house estimator and
makes the SE comparable with every other column in that table.

**THE NULL, WRONG TWICE.** First answer `1/√gn = 0.2673`. But `seen` excludes the uniform
direction with `common`, so it lives in the **gn−1 centred** directions and the closed form is
**1/√(gn−1) = 0.2774**. Then neither closed form described the mean of ratios at all, which is
why the Monte Carlo was needed rather than a third derivation.

**AND THE SMOKE'S "CONFIRMATION" WAS A COINCIDENCE OF TWO ERRORS.** The commit for the gate
claimed the null was "derived analytically and confirmed on a creature with no structure",
reading 0.269 against 0.267 at 0.0 SE. Against the correct pooled null the same reading is
**~0.7 SE above** it, and against the mean-of-ratios null it is further still. **A measurement
agreeing with a wrong prediction is not a confirmation of anything**, and the agreement is
exactly why neither error was caught: two mistakes cancelled to four decimal places.

**WHAT THIS CHANGES FOR THE RUNNING `ctxscale`.** Its `aligned` and `residual` columns stand.
Its printed `seen` column and verdict are computed the old way and must not be quoted; the
corrected build has to be gated and the numbers re-derived from the squares. Since the log
prints only the *means* of `align` and `residual`, not their squares, the pooled ratio cannot be
recovered exactly from the published table — so the honest reading of that run is the direct
comparison of mean `residual` against mean `aligned`, which answers the real question anyway.

**THE RULE THIS EARNS:** when a new statistic is a RATIO, write down which estimator it is and
Monte Carlo its null before the run, not after. The closed form answers a question about
expectations; the printed number is a function of the sample.

### 18. ~~PREPARED, NOT BUILT — `hcpair`: the half-centre was never wired as a half-centre~~

> **ITS DIAGNOSIS IS WRONG — see §22, which locates the real defect and fixes it.** This
> section blames the v32 bump kernel's local term for the within-population coupling. That
> kernel ships at `lateral_gain = 0.0` on every module including vocal, so it contributes
> nothing and is innocent. The within-population coupling comes from the half-centre's own
> **normaliser**. The section is also wrong that the wiring does not exist: `halfcenter_gain`
> has been in the genome since DNA v57, and `halfcenter` alternates and reaches the formant
> at 30–44 SE. What fails is the **period**, and §22 says why.

**The strongest case for a build on the gesture line since it opened, and it is a WIRING change
inside one module rather than a new learning rule.** It aims at the biggest missing capability:
a sequence. `no sequence` found an utterance is a **held vowel**; `hcsweep`'s syllable band is
empty.

**THE DIAGNOSIS IS A PUBLISHED CONDITION, not an analogy.** Baruzzi, Lodi, Storace & Shilnikov
(2020, Phys Rev E 102:032406): the classic half-centre is recovered when **within-population
coupling is zero and between-population coupling is nonzero.** This larynx's lateral kernel is
**local-minus-field-mean** — the local term is nonzero within-population coupling, and there is
no distinct between-population term. **The half-centre condition is violated by construction.**

**SO WHAT `hcsweep` REFUTED IS WHAT THEORY SAYS SHOULD FAIL.** This README already recorded it:
"only fatigue was added… a bump kernel, not reciprocal inhibition between two populations."
Matsuoka (1985) is that mutual inhibition *alone* settles on a winner and adaptation is what
makes it release — **you need both.** `halfcenter` has the adaptation and the gesture (30–44 SE);
it has never had the topology.

**THE BUILD.** Athota et al. (2021, EMBC 6703–6706) give the motif in the needed form: "two
neurons, **or populations of identical neurons**, connected by reciprocal inhibition", spiking.
Split the F1 group's 14 neurons into two sub-populations with reciprocal inhibition between them
and the local term removed, keeping the adaptation already present.

**AND THE TIMESCALE IS ALREADY IN THE BODY.** `frame-is-a-body-part` measured a resonant jaw at
**~4.5 Hz**, inside the syllable band, and adaptation timescales in this literature run from tens
of ms to seconds. **The missing piece is the coupling topology, not the period** — so no new
constant has to be guessed, which is where four runs have been lost before.

**WHAT IT MUST NOT BE CONFUSED WITH.** `hcdrive` closed the **(gain, drive) plane of operating
points** and its prohibition stands: **do not re-sweep gain or drive.** This changes the
*coupling*, which that sweep never varied. Stating it because re-opening a closed plane under a
new name is exactly the failure this ROADMAP exists to prevent.

**THE BAR is `hcsweep`'s own** — the lesson surviving the gesture — because that is the trade
every gesture mechanism here has paid. **What would refuse it:** reciprocal inhibition buying a
syllable band while costing the lesson more than the bump kernel did, which is the monotone trade
`hcsweep` measured and has no reason to escape. **A second refusal, cheaper:** no syllable-band
power at any coupling strength, which would say the topology was never the limit.

**AND THE ORACLE TO AVOID, from the reviewers of this literature themselves.** Dogonasheva,
Giraud, Zakharov & Gutkin (2026, Neural Networks 195:108194) note that adaptive-frequency
oscillators in speech models often have "no learning or inference mechanism for period adaptation
beyond a hard-coded relation to stimulus rate." **That is an oracle in disguise**, of exactly the
kind `tutorgrid` and `ctxaxis` had to strip. Any period here must be derived or disclosed as
host-fed.

### 19. PREPARED, NOT BUILT — the chain as a CREDIT CLOCK, which the literature has now done

`no sequence` left "a chain as a CREDIT CLOCK" flagged as untried. It is untried **here**, not in
the literature. Maes, Barahona & Clopath (2020, PLOS Comput Biol 16:e1007606) build a recurrent
spiking network whose **chained cluster activity is a temporal backbone**, with a downstream
readout learning against that timing signal.

**WHY IT FITS THIS CREATURE SPECIFICALLY.** `chain TRAVELS` already showed the chains here
**propagate** — what was missing was something that *uses* the timing, and this is it. And
`credit-resolution` measured the credit window at **15× the gesture**, so a clock is aimed at a
quantity already known to be mis-scaled.

**THE RULE THE LITERATURE SAYS IS REQUIRED, AND IT AGREES WITH THIS PROJECT'S REFUSALS.** Cone &
Shouval (2021, eLife 10:e63751) learn the **order and duration** of sequence elements with
eligibility traces for both LTP and LTD, and argue pair-based STDP and rate-based Hebbian rules
**fail** at temporal credit assignment. This project reached the same place from the other side:
`oscillations v26` found "a code STDP could not use", the delay axis is closed, learning here is
rate-coded. **Agreement from opposite directions is a reason to take the prescription seriously,
not a new idea** — and it means the eligibility machinery, not the chain, is the expensive half.

**Order these two: §18 first.** It is a wiring change with a published condition it currently
violates; §19 needs a new plasticity path and its prize is credit resolution, which
`phasemask` and `credgate` have already closed from two directions.

### 20. RAN 2026-10-31 — the lesson concentrates onto the readout's own coordinate

`ctxscale` at 13.6M, nine creatures, four arms, 2 h 20 m, carrying the new within-group
decomposition. **Validity first:** the printed `seen` is a mean of ratios against a wrong null
(§17); `aligned` and `residual` are means of linear per-creature quantities and are valid.

| budget | `aligned` | `residual` (12 blind directions) | aligned/residual |
|---|---|---|---|
| 3.40M | 0.01507 ± 0.00406 | 0.02679 ± 0.00127 | 0.563 |
| 6.80M | 0.02229 ± 0.00427 | 0.03467 ± 0.00264 | 0.643 |
| 13.60M | 0.03298 ± 0.00523 | 0.04407 ± 0.00320 | **0.748** |

**THE VERDICT SURVIVES THE ESTIMATOR BUG, which is why it is quotable at all.** `seen` = 0.564 ±
0.049 sits **6.9 SE** above the Monte-Carlo null for the estimator used (0.2254), **5.8 SE** above
the closed form for the right one (0.2774), and 6.1 SE above the wrong printed null. All three
resolve the same way, so a bias that points LOW could not have produced it. Near the null the run
would have had to be repeated.

**FINDING 1: the blind directions are NOT empty** — `residual` exceeds `aligned` at every budget.
The centroid discards real learned structure, as Keemink & van Rossum (2025) predict for a
population vector over few neurons. First direct measurement here.

**FINDING 2, which reframes the line: `seen` RISES with budget** — 0.416 → 0.491 → 0.564,
**+0.075 and +0.073 per doubling**, linear in log-budget, not saturating. The lesson
progressively concentrates onto the one direction the readout can express. That is a closed loop
adapting to its decoder (Chase, Schwartz & Kass 2009), and it makes the residual **transient
inefficiency, not a reservoir**.

**THE PRE-REGISTRABLE FORK, and it decides a build either way.** Extrapolating +0.074/doubling
gives `seen` ≈ 0.64 at 27.2M and ≈ 0.71 at 54.4M. **Rising → the residual is transient and the
readout line closes for good. Saturating → the residual is permanent and `f1decode` is worth
building.** A single-arm extension of an existing experiment; no new mechanism.

**And ctxscale's ORIGINAL question is INCONCLUSIVE again, correctly:** excess +38.0 → +60.8 Hz
over 4×, ratio **1.60**, inside the **1.3–1.7 band declared before the run**. The 272M run read
1.70 and said the same. Its own "54× these trials to reach 236 Hz" is the more useful number.

### 21. RAN 2026-10-31 — the fork resolves AGAINST §20, and §20's slope was 65% estimator bias

`ctxscale` at 27.2M, nine creatures, four arms, **4 h 59 m** (1714M ticks, 344M ticks/h).
Rungs 6.80M / 13.60M / 27.20M, all computed with the corrected pooled estimator of §17.

**THE FREE CONTROL FIRST, AND IT PASSES EXACTLY.** The two rungs this run shares with §20's run
print identical `aligned` and `residual` to all five decimals *and both SEs* — 0.02229 ± 0.00427 /
0.03467 ± 0.00264 at 6.80M, 0.03298 ± 0.00523 / 0.04407 ± 0.00320 at 13.60M — across two runs
whose **total** budget differs 2×. So a rung depends only on its own budget, the new rung is on the
same trajectory, and at those two rungs the *only* difference between the runs is the estimator.

**THAT ISOLATION MAKES THE OLD RUNG RECOVERABLE WITH NO RE-RUN.** The house jackknife of a plain
mean reduces to the textbook SEM, so `sd = se·√n` and `Σx²/n = mean² + sd²(n−1)/n`. Since
`seen` pooled is `√(Σa²/(Σa²+Σr²))`, it is recoverable from the mean and SE already printed.
Checked against the three rungs whose corrected `seen` *was* printed before being used on the
fourth: worst error **0.00013**, inside a 4-dp print's own rounding.

| budget | `seen` corrected | `seen` as §20 printed it | the estimator's bias | step |
|---|---|---|---|---|
| 3.40M | **0.5740** | 0.4160 | +0.1580 | — |
| 6.80M | **0.5815** | 0.4910 | +0.0905 | +0.0075 |
| 13.60M | **0.6263** | 0.5640 | +0.0623 | +0.0448 |
| 27.20M | **0.6371** | — | — | +0.0107 |

**§20'S HEADLINE IS WITHDRAWN.** "+0.074 per doubling, linear in log-budget, not saturating" was
mostly the **mean-of-ratios bias decaying** as `seen` rose: the bias itself drifts −0.0478 per
doubling, **65% of the slope that was reported**. Corrected, the rise is **+0.021 per doubling over
an 8× span** with steps +0.0075 / +0.0448 / +0.0107 — non-monotone, the one large step between two
near-zero ones, against rung SEs of 0.046–0.079. **There is no resolvable trend.**

**THE ERROR WAS MINE AND IT WAS HALF-FIXED.** §17 found the estimator bug and corrected the
*level* at 13.6M, where the verdict survived at 5.8 SE. It never re-derived the **slope** — and the
slope was the entire statistic the fork rested on. A correction that fixes a level and leaves a
trend computed in the old units is not a correction. **Rule: when an estimator is replaced, every
statistic derived from it is re-derived, not only the one that motivated the fix.**

**WHAT SURVIVES UNTOUCHED.** `seen` = 0.574–0.637 at *every* budget, 4.5 SE and up over the 0.2774
null. The lesson **is** ramp-enriched, robustly, at every budget measured. And `residual` exceeds
`aligned` everywhere, so the centroid does discard real learned structure (§20's Finding 1).

**WHAT REPLACES IT: the split is a CONSTANT of the architecture, not a trajectory.** The blind
share of centred within-group variance, `1 − seen²`, runs 0.671 → 0.662 → 0.608 → 0.594: drifting
**−0.0255 per doubling**, which at that rate needs **12 more doublings (3250×) to halve it**. On
any budget this project can run, roughly **60% of the within-group pattern is permanently invisible
to the centroid.** So "transient inefficiency, not a reservoir" is withdrawn too.

**THE MECHANISM, and it is the sharpest number in the run.** Per-doubling growth exponents
(0.5 = √t): `aligned` **0.565 → 0.565 → 0.338**, `residual` **0.372 → 0.346 → 0.405**. The
*aligned* component is the one that decelerates; the blind directions keep filling at a steady √t.
Consistent with the rest of the run saturating — `excess` ratio 1.04 over 4× (exponent 0.03),
`gain` 1.85 → 2.26 → 2.35 — while `table divergence` (0.0345 → 0.0430 → 0.0523) keeps rising.
**The writing continues; what reaches the output stops.**

**A CAVEAT ON THE TOP RUNG, because it bears on the +0.0107.** `aligned`'s between-creature SE
nearly doubles at 27.2M (0.00523 → 0.00883; relative sd 0.48 → 0.64). A pooled ratio of sums
weights that variance in, so the last step is partly creatures **diverging** in how much ramp they
write rather than the mean concentrating. At the longest budget the creatures stop agreeing.

**WHAT THIS DOES TO `f1decode`.** Of the two reasons §20 gave for holding off, one is withdrawn
(transience) and one stands untouched (§15's algebra: readout sensitivity buys **speed**, not
ceiling). The deciding question is the one the log has printed as unmeasured all along: **the
residual is a MAGNITUDE, and an unstructured write produces one too.**

**AND THAT CONTROL WAS IN THE ARRAYS THE WHOLE TIME.** Every decomposition column ctxscale prints
reads the `ema` arm only, while `m_al`, `m_rs`, `a2` and `sn` are all indexed `[arm][budget]` —
so `ema-rnd`, which writes with the same rule, reward and trial count but a **randomised** context
index, has had its own `aligned`, `residual` and `seen` computed and discarded at every budget.
Added 2026-10-31, as prints only, no change to the run:

- **`ema − ema-rnd` per component, paired on creature id and jackknifed** — the word-specific part
  of the ramp and of the blind directions, which is what a decoder would be claiming.
- **`seen`'s null MEASURED rather than derived.** 1/√(gn−1) assumes the unstructured write is
  isotropic across the group's centred directions; `ema-rnd` is the creature's own unstructured
  write, carrying whatever anisotropy its wiring, homeostat and perturb clamp impose. Agreement
  vindicates the derivation; disagreement means every "SE above null" in §20 and §21 is against
  the wrong yardstick.
- **the fork's statistic PAIRED** — top-minus-bottom `seen`, jackknifed by deleting a creature from
  *every* rung at once. The rungs share creatures, so §20 and §21 both priced a paired difference
  with a between-creature SE, which is why a +0.0107 step sat inside a ±0.079 bar and decided
  nothing.

**NEXT RUN: `ctxscale --ticks 6800000`, ~1.25 h** (9 × 4 × (1.7+3.4+6.8)M = 428M ticks). Rungs
1.70M / 3.40M / 6.80M. Chosen over another doubling upward because the fork's statistic is a
**slope**, and rungs at the cheap end buy more slope leverage per hour than one more rung at the
expensive end — and 6.80M re-prints a value now known exactly (0.5815), so it carries its own
determinism check. Combined with the algebraic recovery above this gives a **five-rung corrected
series spanning 16×**, with the measured null and the paired slope.

**PRE-REGISTERED, BEFORE THAT RUN.** The recovery predicts corrected `seen`(3.40M) = **0.5740**,
and 3.40M is the rung the broken estimator put at 0.416. If the run prints ≈0.574 the recovery
algebra is confirmed end to end and the series is **flat at the bottom** (0.574 → 0.582 over 2×).
If it prints ≈0.49–0.52 the recovery is wrong and §20's rise survives. **Note which rungs are the
discovery sample:** 3.40M and 6.80M are, so the genuinely new level is **1.70M**, and the paired
slope and the measured null are new at all three.

### 22. DNA v75, WRITTEN NOT YET BUILT — the half-centre's normaliser *was* the within-population coupling

**Derived by hand from the kernel and one published condition. No run was spent finding it, and
`hcdrive` had already closed the whole (gain, drive) plane — which is the wrong search space for a
defect in a *derivative*.** §18 is withdrawn: it blamed the v32 bump kernel, which ships at
`lateral_gain = 0.0` on every module including vocal and therefore contributes nothing.

**WHAT IS ALREADY TRUE, so the target is narrow.** `halfcenter` (v56+v57) **does** alternate and
**does** reach the formant — produced F1 follows the alternation at **30–44 SE** over a matched
null, replicated on two seed families, with a +65.9 Hz free-running excursion and no caregiver.
The single failure is timekeeping: period against `tau_a` gave a four-point slope of **+0.548,
R² 0.448**, one of four arms inside the 2–4·tau band, series non-monotone, against a required
**1.0**. The memory's own summary: *"The depth is controlled by the fatigue constant; the frequency
is not."*

**THE DEFECT, AND IT IS EXACT.** Baruzzi, Lodi, Storace & Shilnikov (2020, Phys Rev E 102:032406):
the classic half-centre is recovered when **within**-population coupling is zero and
**between**-population coupling is nonzero. v57 inhibits each half by the other half's rate —
correct Matsuoka form — but divides by the **pair's own mean**. With `D = m_hi − m_lo` and
`S = m_lo + m_hi`:

```
hc_lo = g·(1 + D/S)        hc_hi = g·(1 − D/S)
∂hc_lo/∂m_lo = −2g·m_hi/S²        ∂hc_lo/∂m_hi = +2g·m_lo/S²
```

At balance those are **equal in magnitude**. The division makes each half's inhibition depend on
its own rate, so within-population coupling is as strong as between-population coupling and
Baruzzi's condition is violated — by the **normaliser**, not by any kernel.

**AND IT UNDOES THE SELF-LIMITING ARGUMENT THE KERNEL MAKES THREE TIMES.** "Suppress a half and
its mean falls, so the inhibition it exerts falls too" is true of the **absolute** term and false
once it is divided by the pair's own mean, because that converts *inhibited by the other half's
rate* into *inhibited by the other half's **share***. The `−g·D/S` on the quiet half and `+g·D/S`
on the busy one is **winner amplification**: the differential structure v57a and v57b were each
rewritten to remove, surviving inside the normaliser. The comment at `network.cpp` "the sign is
opposite across the two halves, so the loop is competitive rather than self-amplifying" describes
this same term and draws the opposite conclusion.

**WHY THAT PRODUCES EXACTLY THE OBSERVED FAILURE — stated as a mechanism, not a derivation.** A
positive self-feedback on each half, equal in magnitude to the mutual inhibition, deepens the
winner's well. The alternation then happens when fatigue climbs out of a well whose depth is set
by **g**, so the escape time is dominated by the depth rather than by the fatigue timescale. That
is "the depth is controlled by the fatigue constant; the frequency is not", and a non-monotone
series at slope 0.548 is what a depth-dominated escape looks like. Removing the self-excitation
returns the system to release-from-inhibition, where period ≈ 2–4·tau_a. **The derivative algebra
above is exact; this paragraph is an argument and is what the run tests.**

**THE FIX INTRODUCES NO CONSTANT**, which matters because four guessed constants have cost a run
each here. Divide by the module's own homeostatic setpoint `target_rate_hz`, which depends on
neither half:

```
∂hc_lo/∂m_lo = 0   exactly        ∂hc_lo/∂m_hi = g/T
```

Still dimensionless and still scale-free, so the v32 hertz-minus-hertz runaway cannot return. The
kernel **already takes this exact ratio** for divisive normalisation (`ms.mean_rate_fast /
norm_target_[m]`), so there is no unit question and no precedent to invent. The vocal module's
setpoint is **5.0 Hz**, non-zero, so the new normaliser is live rather than falling back — checked
before writing the change, not after.

**IT CANNOT MOVE THE PINNED HASH.** The whole block is behind `if (hc_gain_[m] > kZero &&
ms.count >= 2)` and `halfcenter_gain` ships at **0.0**, so the shipped creature is bit-identical
by construction. A genome with no vocal setpoint falls back to the v57 normaliser, and that
fallback is **disclosed** by a new `halfcenter_normaliser()` accessor rather than inferred — a null
measured under the fallback refutes nothing about v75, and a caller that cannot tell them apart
would report it as if it did.

**THE ARM DESIGN, AND THIS IS THE PART THAT MAKES IT A ONE-VARIABLE CONTRAST.** At the fixed point
v57's between-coupling is `g/(2T)` and v75's is `g′/T`, so **`g′ = g/2` matches the
between-population coupling exactly** and leaves the within term (`−g/(2T)` → **0**) as the only
thing that differs. Running v75 at the same gain instead would change both at once and confound
the two. It also answers the standing prohibition directly: **this is not re-opening `hcdrive`'s
closed (gain, drive) plane**, because at equal nominal gain the coupling is a *different quantity*
— twice the between-term and no within-term — and that plane never varied coupling at all.

**THE TEST ALREADY EXISTS AND ITS BAR IS ALREADY PUBLISHED**, which removes any chance of fitting
a verdict to the data: `halfcenter` pre-registers the frequency exponent at "Required ~1.0", plus
an anti-phase signature (≤ −0.20 and ≥ 3 SE below each single), a duty/rate kill switch, a
per-half replication and an argmax null. All of it was written before this fix was conceived.

**PRE-REGISTERED FOR THE v75 RUN.** Primary: the four-point slope of log(period) on log(tau_a)
moves from **+0.548 toward 1.0**, with more than one of four arms inside the 2–4·tau band and a
**monotone** series — the endpoint fit (+0.750) must not be quoted, per `halfcenter`'s own note.
**Refusal 1:** the slope stays ≤ 0.6, which says the normaliser was never what set the period.
**Refusal 2, and it is the informative one:** the alternation itself (currently 30–44 SE) is
**lost**, which would say the within-population self-excitation was load-bearing for the
competition and that the 30–44 SE gesture was bought by the very term Baruzzi forbids. Either
refusal is worth the run.

**STATUS: source written, NOT COMPILED.** `build/aibaby` is held by the `ctxscale` run, and
overwriting a running executable fails rather than silently working. Build, gate on the four
verify keys (the hash must be *unchanged*, which is the whole point of the branch guard), then
smoke `halfcenter` before spending the full run.

## §23 — THE BLIND DIRECTIONS HOLD NO WORD. The readout line closes on the information question (2026-10-31)

`ctxscale --ticks 13600000`, 9 creatures, 4 arms, rungs 3.40M / 6.80M / 13.60M, 2 h 19 m.
Exit 1, which is the declared-band `return false` on the **growth-ratio** question alone
(ratio 1.60 inside the pre-registered 1.3–1.7 dead zone). The run reached its own final verdict
and every one of the three questions this run was built to ask resolved.

### The recovery algebra is confirmed end to end, and §20 stays withdrawn

**PRE-REGISTERED in §21, before the run:** corrected `seen`(3.40M) = **0.5740**, against the
broken estimator's 0.416.

    rung      predicted   printed     role
    3.40M     0.5740      0.5742      the pre-registered test        (error 0.0002)
    6.80M     0.5815      0.5814      determinism, recovered value
    13.60M    0.6263      0.6263      determinism, recovered value

So the inversion of the archived log was right to four decimals, the two shared rungs re-printed
across runs whose total budget differs 2×, and §20's withdrawal is final: the corrected series
is **0.5742 → 0.5814 → 0.6263**, not the rise that was published. `residual`'s exponents also
re-print at 0.372 / 0.346 and `aligned`'s at 0.565 / 0.565, so the whole decomposition is
reproduced, not just the ratio.

### The fork resolves FLAT, so the split is architecture, not a trajectory

Paired on creature id over the 9 creatures scored at every rung: `seen` moves **+0.0521 ±
0.0997** over the 4× span, **+0.0260 per doubling**. Flat within 0.5 SE. Neither §20's rise nor
a decay. As a variance share, the blind part is **0.670 → 0.662 → 0.608**, a change of
**−0.063 ± 0.120** — equally unresolved.

**This retires the extrapolation as well as the claim.** §21 priced the blind share as draining
at −0.0255 per doubling, "3250× the budget to halve it." That number came out of a drift that
is **not distinguishable from zero**, so it was precision on noise. The licensed statement is
only: the blind share sits at 0.61–0.67 and does not move measurably over 4×.

### THE RESULT: the blind directions carry no word, and the ramp carries only half a word

The control was `ema-rnd` — the same write rule, the same trials, the same creatures, with the
word index randomised. It has no word to key on, so it is the floor a magnitude must clear to be
*about* the word at all.

**And it was not a missing control — it was a control wired to the wrong statistic.** This
experiment's headline gate has always been `excess = dF1(ema) − dF1(ema-rnd)`
(`experiments_milestones.cpp:38039`), so the index-free arm was attached to *that* number from
the first run and never to the decomposition columns. The same run carried one properly
controlled quantity and two uncontrolled ones, and the uncontrolled pair is what was read as
evidence. The control's presence is what hid the gap. §21 printed the missing comparison.

    rung      component  ema       rnd (floor)  word-specific (paired, jackknifed)
    3.40M     aligned    0.01507   0.00487      +0.01021 +/- 0.00513   ( 2.0 SE)
    6.80M     aligned    0.02229   0.01146      +0.01083 +/- 0.00643   ( 1.7 SE)
    13.60M    aligned    0.03298   0.01703      +0.01595 +/- 0.00761   ( 2.1 SE)
    3.40M     residual   0.02679   0.02813      -0.00134 +/- 0.00217   (-0.6 SE)
    6.80M     residual   0.03467   0.04146      -0.00679 +/- 0.00472   (-1.4 SE)
    13.60M    residual   0.04407   0.06050      -0.01643 +/- 0.00436   (-3.8 SE)

**The residual is never above the floor, and at the top rung it is 3.8 SE below it.** The gap
widens monotonically. Randomising the word makes the creature write *more* into the directions
the centroid cannot see — so keying the write to the word moves magnitude **out of** the blind
subspace and **into** the ramp. That is the opposite of a reservoir.

**`seen` > null was never evidence of a hidden code.** Both facts hold together: the off-ramp
write is anisotropic (`seen` = 0.6263 against a **measured** null of 0.3077, ≥5 SE once the
null's own spread is carried, and above it at all three rungs), and it is word-free (at or below
the index-free floor at all three rungs). Anisotropy is a property of the write rule and the
wiring. It is not word identity. The derived null `1/sqrt(n-1)` = 0.2774 is confirmed by the
measured one, which ran 0.254 / 0.338 / 0.308.

**And the ramp itself is about half floor.** The word-specific part of `aligned` is
0.01595/0.03298 = **48%** at the top rung, at only ~2 SE per rung. Every `aligned` number
published in this line is therefore a floor-contaminated quantity.

### What that costs the published mechanism story

§21 explained the ceiling by *the expressible component stalls*: `aligned`'s exponent holding
0.565 for two doublings then falling to 0.338, while `residual` held 0.372 / 0.346 / 0.405.
Two caveats now attach, and both land on the same single data point:

* the fall to **0.338 occurs only at 27.2M**, the top rung of the *other* run, where `aligned`'s
  between-creature relative spread already nearly doubles (0.48 → 0.64);
* `aligned` is ~50% index-free floor, and the floor's own exponent is **1.23 then 0.57** — it
  grows at least as fast. The word-specific part's exponent is **0.085 then 0.559**, which at
  this n is unresolvable.

So *the expressible component stalls* rests on one rung of one run, measured on a half-floor
quantity. It is not retracted — it is **unsupported**, and the honest reading of this run is
that both components grow with no separable exponent.

### CONSEQUENCE: the readout line is closed, both ways

`aibaby-readout-is-a-speed-knob` already priced the readout's **ceiling** prize at ~nothing by
algebra, leaving it an information question: is there word-specific structure the centroid
cannot reach? **Measured: no.** A decoder reading the blind directions inside the F1 group would
be reading something that falls *below* what an index-free write produces.

**DO NOT BUILD the within-group decoder.** Not because the gain is small, and not because the
structure is transient — the fork says it is permanent — but because the structure is **not
about the word**.

This matters beyond the readout. It says the naming ceiling (`dF1 ~ aligned^0.61`) is **not**
hiding behind an expression bottleneck. The write is already ramp-enriched; what it puts
elsewhere is interference. More F1 reach has to come from a **bigger or better-aimed write**,
not from a better reader.

### WHAT IS LEFT ON THIS LINE

One limit still bounds every branch: `seen` accounts only for the group's **centred**
directions. A decoder reading rates directly could also use the common-mode level and the other
eight groups, which the centroid's ratio discards by construction. That is a different
instrument, not a continuation of this one. The open leads that survive are `noise_amp` and the
nine-axes multiplication — not the readout.

## §24 — DNA v75 IS CORRECT AND MUST NOT SHIP. The period is the amplitude lever (2026-10-31)

Built, gated (**all four verify keys PASS, hash `ad96f882becbee92` unchanged**), and run at
`halfcenter`'s own 1.2M minimum, 8 arms × 12 reps, 16 min. The normaliser disclosure confirms it
ran **dosed**, not fallen back: `v75: inhibition / target_rate_hz = 5.00 Hz`.

### FIRST, A CORRECTION TO THE BASELINE THIS WAS GOING TO BE SCORED AGAINST

§18, §22 and the memory all quote **+0.548, R² 0.448, 1 of 4 in band** as *the* clock slope.
That is seed family **604553**. v75 runs on family **288559**, and the archived replication on
*that* family gives **+0.320, R² 0.214** (four-point; endpoint +0.489). Same code, same budget,
same arms — the slope moves **+0.320 → +0.548** on seeds alone.

**So the clock statistic is not well determined at 12 reps, and its between-family spread is
as large as any treatment effect measured against it.** Comparing v75 to +0.548 would have
been comparing across seed families. The matched baseline is +0.320.

The same audit found the printed exponent is the **endpoint** fit, while the project's standard
is the four-point slope — and the archived log says so itself ("the printed +0.750 is an
ENDPOINT fit and must not be quoted") while printing only the endpoint fit. **Both are printed
now, labelled, with R² on the four-point one.**

### THE SPECIFICITY CONTROL IS EXACT, WHICH IS WORTH MORE THAN THE HASH

The two arms with `halfcenter_gain = 0.00` re-print **bit for bit** across the kernel change:

    arm        v57 (fam 288559)              v75
    fat140     +0.092, rate 1.51, swing 71.5   +0.092, rate 1.51, swing 71.5
    ctrl140    +0.371, rate 4.94, swing 69.0   +0.371, rate 4.94, swing 69.0

The verify hash only tests the shipped genome. This tests the **experiment's own path** and
shows the edit is confined to arms with the gain on, to every printed decimal.

### REFUSAL 2 DID NOT FIRE — THE ALTERNATION SURVIVES AND DEEPENS

The informative refusal was that removing the within-population coupling would **lose** the
competition, which would have said Baruzzi's forbidden term was buying the 30–44 SE. It is not
lost. Every arm with the gain on gets **deeper**, against SEs of 0.001–0.004:

    arm        v57      v75      arm        v57                    v75
    tau083   -0.754   -0.872     drive200 -0.650                 -0.759
    tau167   -0.555   -0.671     inhib140 +0.075 (rate 121.75)   -0.162 (rate 17.79)
    tau250   -0.395   -0.580
    tau333   -0.297   -0.507

So **Baruzzi's condition is satisfiable here and it improves the competition**, exactly as
derived. `inhib140`'s runaway is also tamed 7× (121.75 → 17.79 Hz, still VOID) — predicted,
since v57 divides by the pair's own mean, which blows up as that mean falls, and v75 divides by
a constant.

### REFUSAL 1 FIRED — AND THE RUN FOUND A THIRD THING THE PRE-REGISTRATION DID NOT NAME

The clock is not fixed: four-point slope **+0.320 → +0.099** (R² 0.214 → 0.153). Given the
between-family spread above, the honest reading is **v75 did not fix the clock**, not that it
made it worse.

**But the produced F1 swing collapses, and that is the milestone's own statistic.** Against the
control's 69.0 ± 1.2 Hz:

    arm       v57 swing     v75 swing    excess v57   excess v75    alternation
    tau083   45.9 +/- 0.4  45.3 +/- 1.2    -23.1        -23.7      2.88 -> 3.25 Hz
    tau167   75.6 +/- 0.6  40.3 +/- 0.1     +6.6        -28.7      1.25 -> 2.25 Hz
    tau250  105.6 +/- 0.3  34.2 +/- 0.0    +36.6        -34.8      2.62 -> 2.65 Hz
    tau333  134.9 +/- 0.4  42.7 +/- 0.2   **+65.9**    **-26.3**   1.46 -> 2.83 Hz

**The milestone reverses sign.** `halfcenter`'s replicated result is "tau333 carries +65.9 Hz
of excursion over a creature with the mechanisms off." Under v75 tau333 produces **26.3 Hz
less** than that control. The effect moves 92.2 Hz against SEs under 1.3.

**The mechanism is the plant, and it is quantitative.** v75 roughly doubles the between-coupling
at equal nominal gain, which roughly doubles the alternation frequency; the vocal tract is a
low-pass with an 800 ms pole, `|H| = 1/sqrt(1+(2πfτ)²)`, which reads 0.135 at 1.46 Hz and 0.070
at 2.83 Hz. The pole model reproduces both runs' `|H|` exactly, so the **direction** is
explained; it accounts for about half the loss on a log scale (predicted ×0.52, observed ×0.32),
so something else — plausibly the half-difference amplitude — carries the rest.

### THE REFRAMING, WHICH IS THE ACTUAL RESULT

**Frequency and formant reach are coupled through the pole, so the period is the AMPLITUDE
lever and not a cosmetic failure.** Anything that speeds this oscillator costs swing at roughly
the pole's rate. That is why the clock matters: `tau_a` is supposed to buy slowness, slowness
buys swing, and the clock's failure is what stops it.

**And it inverts the optimisation target.** The arm that produces the gesture is the *worst*
oscillator — tau333 has the **shallowest** anti-phase (−0.297) and the **largest** swing
(134.9 Hz), while tau083 has the deepest (−0.754) and the smallest (45.9 Hz). Across the four
v57 arms, anti-phase depth and produced swing run in **opposite** directions. So "deepen the
competition" is not the objective, and a mechanism scored on anti-phase alone will keep
choosing the arm that speaks less. §22's pre-registration made exactly that mistake: its
primary was the slope and its refusals were about the alternation, with the swing nowhere in it.

**STATUS: v75 REFUSED AT `halfcenter_gain = 0.60`.** It is correct by derivation, better on the
neural measure, and worse on the creature. It stays behind the shipped gain of 0.0.

### WHAT IS RUNNING

The matched-coupling stage, which the derivation required from the start and which this result
makes mandatory: four appended arms at **hc 0.30**, where v75's between-coupling equals v57's
and the removed within-term is the only variable. Appending is safe — a cell's seed is
`seed + kHCSeedOffset + r*7919`, a function of the replicate only — so **the eight base arms
must re-print to every decimal**, which makes the run carry its own determinism check. Two
index constants had to be introduced first (`kHCCtrlArm`, `kHCBaseArms`): the control was being
found as `kHCArmCount - 1`, so appending would have silently made `mat333` the baseline for the
VOID bound and re-decided VOID for every arm.

**PRE-REGISTERED.** Primary is the **produced swing at `mat333` against the control's 69.0 Hz**,
because that is the statistic the milestone is stated in — not the anti-phase, and not the
slope. Halving the coupling should roughly halve the frequency and the pole then passes about
twice as much, so the prediction is a swing back in v57's range and an excess above zero at
≥3 SE. **The refusal that matters:** the swing stays below the control even at matched
coupling, which would say the forbidden within-term was buying the slow period the pole needs —
Baruzzi's condition and this creature's formant reach in direct conflict, and v75 dead at any
gain.

## §25 — THE FORBIDDEN TERM WAS BUYING THE PERIOD. v75 is dead at any gain (2026-11-01)

> **READ §26 FIRST. Both of this section's headline claims are WITHDRAWN there.** The "matched"
> arm halved the tonic along with the coupling — the two kernels are not a one-parameter family,
> so no gain matches v57 — and "dead at any gain" runs against its own monotone trend. What
> survives is the measurement at full gain and the across-arm dissociation.

The matched stage, gated on all four keys, 12 arms × 12 reps, 26 min. **Determinism first:** all
eight base arms re-print to every decimal — anti-phase and produced swing both — so appending
four arms re-rolled nothing and the comparison is attributable to the arms alone.

### THE PREDICTION WAS WRONG, AND WRONG IN THE DIRECTION THAT SETTLES IT

§24 pre-registered: halving the gain matches v57's between-coupling, so the frequency should
roughly halve and the pole should pass about twice as much, putting the swing back in v57's
range. At hc 0.30, with between-coupling matched to v57 exactly and the removed within-term the
only remaining difference:

    arm      anti     alternation  period    swing   vs ctrl 69.0
    mat083  -0.550     2.88 Hz     347.8      9.8    -59.3 +/- 1.2
    mat167  -0.518     2.27 Hz     440.4      9.9    -59.1 +/- 1.2
    mat250  -0.487     5.52 Hz     181.1     14.4    -54.6 +/- 1.2
    mat333  -0.463     4.58 Hz     218.2     19.3    -49.7 +/- 1.2   (-42.1 SE)

**The frequency went UP, not down**, and the swing fell *further* — below even the hc 0.60 arms.
`mat333` runs at **4.58 Hz** where v75 at double the coupling ran 2.83 and v57 ran 1.46, and it
produces **19.3 Hz** of swing where v57 produced 134.9. The refusal branch fired at **−42.1 SE**.

**So the loss was never the doubled between-coupling. It was the removed within-term**, and
halving the gain took away the last of what was holding the period open.

### THE DISSOCIATION, AT MATCHED BETWEEN-COUPLING

    within-term   competition      period        produced swing
    ON  (v57)     -0.297 shallow   1.46 Hz slow  134.9 Hz   (+65.9 vs ctrl)
    OFF (v75)     -0.463 deeper    4.58 Hz fast   19.3 Hz   (-49.7 vs ctrl)

Same between-coupling, same tau_a, same drive, same seeds, same budget. **The forbidden
self-coupling buys the slow period, and the slow period is what the 800 ms pole needs.** The
mechanism is the one the derivation identified and mislabelled as a defect: the `+g·D/S` on the
busy half is positive feedback inside a half, which sustains that half's dominance and lengthens
each half-period. Remove it and each half flips sooner.

**`halfcenter`'s kernel is not violating Baruzzi by oversight. The violation is load-bearing** —
and specifically load-bearing for the *plant*, not for the competition, which is measurably
better without it. **v75 must not ship at any gain.**

### THE SECOND LOSS IS CONSISTENT ACROSS BOTH STAGES, AND POINTS AT THE ASSUMPTION

The pole explains the direction and about half the magnitude in log terms in both stages —
predicted ×0.52 against observed ×0.32 at hc 0.60, predicted ×0.32 against observed ×0.14 at
hc 0.30. The residual factor is **1.76 and 1.71**, near enough identical.

That consistency points at the print's own assumption. The 375 Hz figure is a **full** alternation
between the two postures (438 and 813 Hz). At 4.58 Hz each half holds for 109 ms, which is short
against the postures' own settling, so the halves never reach their extremes and the true neural
swing is below 375 Hz. **The pole understates the loss because it assumes a full excursion** —
so speeding the oscillator costs twice: once in transmission, once in excursion. Hypothesis, not
a claim; what supports it is the near-identical residual across two independent stages.

### AND tau_a DOES NOT SET THE PERIOD IN EITHER KERNEL

    kernel                four-point slope   R^2
    v57 (fam 288559)          +0.320        0.214
    v75, hc 0.60              +0.099        0.152
    v75, hc 0.30 matched      -0.457        0.452

The matched arms give the **best-determined fit in the run and it is NEGATIVE** — the period
*shortens* as the fatigue constant lengthens. The series is non-monotone (2.88, 2.27, 5.52,
4.58), so do not read −0.457 as a law. The licensed statement is that `tau_a` does not set the
period under either normaliser, and under v75 at matched coupling the relationship, if anything,
runs backwards.

### PREPARED, NOT BUILT — `hcwithin` (v76): the within-term is the period knob

This run turned the within-coupling from a bug into **the only measured handle on the period**,
and the period is the amplitude lever (§24). So the derived move is not to remove it but to
**increase it past v57's level**.

**THE PARAMETERISATION, AND BOTH ENDS ARE ALREADY MEASURED.** Write each half's inhibition on
v75's constant normaliser with an explicit self-term:

    hc_lo = g*(m_hi/T) - w*g*(m_lo/T)      between = +g/T,  within = -w*g/T

v57 at gain `g` gives between `+g/(2T)` and within `−g/(2T)`, so **`g'' = g/2` with `w = 1`
reproduces v57 exactly** and `w = 0` is the `mat*` arms above. The sweep `w = 0, 1.0, 1.5, 2.0`
at hc 0.30 therefore has **two anchors already in hand**: `w = 0` must re-print the `mat*` row
and `w = 1` must re-print v57's archived tau333 (1.46 Hz, 134.9 ± 0.4 Hz swing, anti −0.297).
A re-parameterisation that fails either anchor is wrong, and says so before any extrapolation is
read.

**PRE-REGISTERED.** Primary is **produced swing at tau333 against the control's 69.0 Hz**, not
the anti-phase and not the slope. Prediction: period lengthens monotonically in `w`, swing rises
with it through the pole, anti-phase shallows — the §24 dissociation continued past its measured
end. **Refusal:** swing peaks at or below `w = 1`, which would say v57 already sits at the
optimum and the period cannot be bought this way.

**COST AND THE ONE RISK.** One new genome field. Appended to `DnaModule` so no existing offset
shifts, and `halfcenter_gain` ships at 0.0 so the branch is dead at the shipped genome and the
pinned hash should hold — but a genome-layout change is exactly what
`aibaby-shared-constants-audit` and `aibaby-nmax-rerolls-the-wiring` exist for, so the four
verify keys gate it before anything else, and KEY2 failing is the finding rather than a nuisance.

## §26 — THE MATCHED STAGE MATCHED NOTHING, AND THE TWO KERNELS ARE NOT A GAIN APART (2026-11-01)

Written **before** v76 ran, because it is what made v76 a different experiment from the one §25
prepared. No new ticks were spent: everything below is arithmetic over numbers already printed,
and two of the four things it checks came out clean.

### WHAT §25 CLAIMED AND WHY IT CANNOT BE TRUE

§25's design claim was that `g' = g/2` "matches v57's between-coupling exactly and leaves the
within-term as the ONLY difference." Write `T` for the vocal setpoint (5.0 Hz) and `m` for the
half-mean the module actually runs at. At the balance point:

    v57   tonic = g          between = +g/(2m)     within = -g/(2m)
    v75   tonic = g*m/T      between = +g/T        within = 0

v57's tonic is **scale-free** — `m_hi/mean` is ≈ 1 whatever the rate — while v75's is
proportional to `m/T`. So:

* **equal tonic** demands `g' = g*T/m`;
* **equal between-coupling** demands `g' = g*T/(2m)`.

**The two demands differ by exactly a factor of two.** No single gain satisfies both, and the
factor does not cancel because **the module does not run at its setpoint**: the measured half
means are 3.67 Hz (v57 tau333), 4.33 (v75 at hc 0.60) and 4.57 (`mat333`), against T = 5.0.
The kernels are not a one-parameter family of each other, so "the matched-coupling arms" were
not matched on anything:

| | tonic | between (per Hz) |
|---|---|---|
| v57 tau333, g = 0.60 | 0.600 | 0.0818 |
| v75 tau333, g = 0.60 | 0.519 | 0.120 |
| v75 `mat333`, g = 0.30 | **0.274** | **0.060** |

`mat333` had **less inhibition of every kind** than v57 — the tonic halved, the coupling 27%
lower. Less inhibition means a higher rate (4.57 vs 3.67 Hz, as printed), faster fatigue
accumulation, and a shorter half-period. **That is the sign of the effect §25 attributed to the
within-term**, so the attribution is not established. And the true between-match is
`g' = g*T/(2m) ≈ 0.41`, not 0.30 — a gain never run.

### A THIRD UNMATCHED VARIABLE: THE CLAMP STOPS BEING INERT

`m_hi/mean ≤ 2` **by construction** (m_hi ≤ m_lo + m_hi), so v57's clamp at 2 is unreachable and
the kernel's `clampf(·, 0, 2)` never fires. `m_hi/T` has no such bound: it passes 2 above 10 Hz.
`mat083`'s two half means are **11.188 and 11.197 Hz**, so its ratio is **2.24 — above the clamp
on the MEAN**, not merely on excursions, and both offsets sit pinned at 2g with the differential
flattened to zero for much of the record. `tau083` ran 9.37 with half-mean SDs of ±7.7, so it
clips part of the time too. Under v75 the clamp bites in exactly the fast arms — which are the
arms the clock slope is computed from, and that is a live candidate for the **−0.457** slope §25
reported and declined to read as a law.

It also means `mat083` reached anti-phase **−0.550 while its competition term was largely
constant**, which is the first hint that the differential term may not be what alternates this
oscillator. The run that settles it is v76's k = 0 arm: `fat140` cannot, because `halfcenter_drive`
is applied inside the `hc_gain > 0` branch, so the "fatigue only" arm has no tonic drive either
and is VOID at duty 0.00.

### WHAT THIS WITHDRAWS, AND WHAT SURVIVES

> **WITHDRAWN:** "the within-term is the only remaining difference" (§25). Three variables moved
> at once, and the two that were not meant to move both push the measured direction.

> **WITHDRAWN:** "v75 must not ship at any gain" (§25, and the commit message of `c941fe1`).
> Within v75 the swing **rises** with gain — 19.3 Hz at 0.30, 42.7 at 0.60 — so an extrapolation
> to all gains runs against its own monotone trend. The licensed statement is that **v75 is
> refused at both gains measured**, and the region above 0.60 is unmeasured. It is not worth
> measuring: the kernel is retired for the arithmetic above, not for the two refusals.

**SURVIVING, and unaffected by any of this:** at hc 0.60 the tonic is nearly matched (0.519 vs
0.600) and the coupling 1.47× v57's, so **§24's own comparison is sound as a measurement** — the
produced swing fell 134.9 → 42.7 Hz against a 69.0 control at SEs under 1.3, and the creature
got worse while the neural anti-phase deepened. The dissociation between competition depth and
formant reach (§24) is **across the four tau arms of a single kernel** and is untouched. So is
the pole arithmetic, and so is `tau_a`'s failure as the clock.

### TWO CONFOUNDS CHECKED AND CLEAN — DO NOT RE-CHECK THESE

**Budget.** The archived v57 logs are named `halfcenter-12m-*` and carry no instrument line, so
they looked like a 12M-tick comparison against v75's 1.2M. They are not. The archive's two
`hc = 0.00` arms print **identical** numbers to the v75 runs — `fat140` +0.092 ± 0.007 /
3.83/4.54 / 7.23 ± 1.18 / duty 0.00 / 1.51 Hz / swing 71.5 ± 0.9, and `ctrl140` +0.371 ± 0.011 /
2.12/4.63 / 5.69 ± 0.97 / 0.57 / 4.94 / 69.0 ± 1.2 — to every printed decimal including the SEs.
Two runs of different length cannot agree like that. Same budget; the filename is a legacy label.

**Seed family.** Both are 288559 (`offset 288559 -> first creature 0000000001398ef8`), and the
same hc = 0.00 identity proves it independently.

**A LABELLING BUG IN THE ARCHIVE, worth knowing before quoting it.** The hand-written header of
`halfcenter-12m-gesture-replicated.log` has a column headed "ANTI-PHASE corr" whose values
(tau333 −0.527 ± 0.004) are the **LAGGED coherence with the alternation**, not the anti-phase
correlation the machine table prints on the same arm (−0.297 ± 0.000). The milestone's "30–44
SE" figures come from that column, so they are SEs on the lagged coherence. The produced-swing
claims (+65.9 Hz, +52.1 SE) are correctly labelled and carry the milestone regardless.

### v76 `hcwithin` — BUILT, AND IT IS A DIFFERENT EXPERIMENT FROM §25'S

§25's `w` was defined on v75's constant normaliser, which the arithmetic above rules out. The
field that shipped instead weights the **differential term** on v57's scale-free normaliser:

    hc_lo = g * clamp((1 - k) + k*(m_hi/mean), 0, 2),   mean = (m_lo + m_hi)/2
          = g * (1 + k*D/S)

so the **tonic is g for every k**, the clamp keeps its `≤ 2` guarantee, and `k = 1` is v57 in
exact IEEE arithmetic — `(1 - k)` is exactly 0 and `k*x` is exactly `x`, so **the eight base arms
must re-print the archive to every decimal**. That is a stronger specificity check than v75's,
which only showed that two arms with the gain at zero were untouched: this one exercises the
half-centre branch itself. v75's normaliser is retired rather than re-tuned, and lives in
`c941fe1` if the unmeasured high-gain region is ever wanted.

**WHAT k CANNOT DO, stated before the result.** `within = −k*g/(2m)` and `between = +k*g/(2m)` at
every k — equal and opposite, because a term built from the **normalised difference** has no
other option. So this sweep measures the **strength of the competition**, not the within-coupling
alone, and a result here does not speak to Baruzzi's condition. Isolating within needs a
normaliser that tracks the rate without tracking the instantaneous balance — a slow filter of the
group's own mean, where a 1 s pole cuts the within-coupling ~9× at 1.46 Hz without touching the
tonic. That is the design v75 should have been, it needs no new constant, and it is **not built**.

**PRE-REGISTERED, and the statistic is unchanged from §25.** Primary is **produced swing at
tau333**, reported both against the control's 69.0 Hz and **paired on the same twelve seeds
against k = 1**, which is the increment the run exists to measure. Arms: k = 0, 1 (`tau333`
itself), 1.5, 2.0, 3.0 at hc 0.60 / drive 1.40.

* **Prediction:** the period lengthens with k and the swing rises with it through the pole,
  because a larger k suppresses the loser harder (toward 2g) and the winner less (toward 0), so
  the winner holds longer. The anti-phase **shallows** as it does — §24's dissociation continued
  past its measured end.
* **Refusal:** the swing peaks **at or below k = 1**, which says v57 already sits at the optimum
  of this axis. A plateau above k = 1 is expected rather than refusing, since the clamp caps the
  asymmetry at "loser 2g, winner 0".
* **The k = 0 arm is informative either way.** It is the tonic with no competition at all — the
  control the v75 pair never ran. If it still alternates at ≤ −0.20, the differential term is not
  what makes this oscillate and every anti-phase number on this line has been read as evidence
  for a mechanism that was not required.
* **Runaway cannot return.** The clamp never injects drive, and the two offsets sum to exactly
  2g whether clipped or not, so v57b's 125 Hz failure is unreachable at any k. The duty/rate
  kill switch still gates every arm.

**WHY THE SWING IS THE RIGHT PRIMARY HERE, AND IT IS NOT ONLY CONVENTION.** k has **no effect on
the mean drive at any setting** — the two offsets sum to exactly 2g clipped or not, so raising k
changes the competition's variance and not its level, and the rate should stay near tau333's 3.67
Hz rather than drifting toward the VOID bound. That leaves two ways for a large k to fail, and
the produced swing penalises both: too fast and the pole attenuates it; a **settled winner**
(v57c's failure, which a large k invites by suppressing the loser past recovery) holds F1
constant and so produces almost no swing at all, while showing up as near-perfect anti-phase. The
anti-phase column would score that failure as the best result in the run. The swing column cannot.

## §27 — k = 1 IS v57 TO THE LAST BIT, AND v57 IS ALREADY AT THE OPTIMUM OF THAT AXIS (2026-11-01)

Gated on all four verify keys with the **pinned hash unchanged**, which is the first thing a new
genome field has to survive. 12 arms × 12 reps at the 1.2M minimum, 25 min.

### THE ANCHOR HELD EXACTLY, WHICH IS THE STRONGEST SPECIFICITY CHECK THIS LINE HAS HAD

All eight base arms re-print the v57 archive **to every printed decimal, SEs included**:
anti-phase −0.754 / −0.555 / −0.395 / −0.297 / +0.075 / +0.092 / −0.650 / +0.371, rates 10.21 /
6.38 / 4.67 / 3.67 / 121.75 / 1.51 / 9.26 / 4.94, swings 45.9 / 75.6 / 105.6 / 134.9 / 2.6 / 71.5
/ 30.7 / 69.0. So `g*clamp((1-k) + k*(m_hi/mean), 0, 2)` at `k = 1.0f` **is** v57's
`g*clamp(m_hi/mean, 0, 2)`, as the IEEE argument said it must be.

Why that matters more than v75's check: v75's specificity rested on two arms with
`halfcenter_gain = 0`, which only shows the *dead* branch is dead. This exercises the half-centre
branch itself, on the arms the result is read from. **And the gesture milestone replicates a third
time** — tau333 carries +65.9 ± 1.2 Hz of free-running excursion over the matched control.

### THE DIFFERENTIAL TERM IS REQUIRED, WHICH SETTLES A DOUBT §26 RAISED

`k = 0` leaves the tonic and removes the competition entirely. It **does not alternate**: anti
+0.094 against v57's −0.297, and the produced swing collapses to 40.7 ± 2.7 Hz — **28.4 Hz BELOW
the control** and **−94.3 ± 2.7 Hz against v57, paired on the same twelve seeds**.

So Matsuoka's prediction holds here: the tonic drive and the fatigue current alone do not make
this oscillate. This also answers §26's suspicion that `mat083` alternating with a largely clipped
term meant the competition was dispensable — it was not. And note what `k = 0` is not: `fat140`
cannot ask this question, because `halfcenter_drive` is applied inside the `hc_gain > 0` branch,
so "fatigue only" has no tonic either and sits VOID at duty 0.00.

### THE PRIMARY: FLAT, AND THE NULL IS TIGHT ENOUGH TO CLOSE THE AXIS

Produced swing at tau333, paired on the same twelve seeds against `k = 1`:

| k | anti | swing | vs control (69.0) | vs v57, paired |
|---|---|---|---|---|
| 0.00 | +0.094 | 40.7 | −28.4 ± 2.9 | **−94.3 ± 2.7** |
| 1.00 (v57) | −0.297 | 134.9 | +65.9 ± 1.2 | — |
| 1.50 | −0.270 | 136.5 | +67.5 ± 1.4 | **+1.6 ± 0.7** (2.2 SE) |
| 2.00 | −0.272 | 136.3 | +67.2 ± 1.4 | +1.3 ± 0.8 |
| 3.00 | −0.278 | 135.5 | +66.5 ± 1.3 | +0.6 ± 0.4 |

**By the pre-registered rule this is NO VERDICT on the sign:** the best arm above k = 1 is +1.6 ±
0.7, inside the ±3 SE band, and neither the "under-driven" branch nor the "peaks at or below k =
1" refusal fires. But the *magnitude* closes the axis anyway. **The whole range of the effect is
+1.6 Hz on a 135 Hz quantity — 1.2%** — and it then declines monotonically with k. Tripling the
weight on the competition changes the creature's formant reach by about one part in a hundred.
This project needs ×1.68 on the naming line; 1.012 is not a lever.

**AND THE THREE ARMS ABOVE k = 1 ARE INDISTINGUISHABLE FROM EACH OTHER**, across a 2× range in
k: anti −0.270 / −0.272 / −0.278, swing 136.5 / 136.3 / 135.5, lagged coherence −0.519 / −0.518 /
−0.519. If the term were still responding to k, k = 3 would differ from k = 1.5 by a factor of
two in the differential. It does not — and the re-print says why, in one column:

| k | frames with the term CLIPPED |
|---|---|
| 1.00 (v57) | **0.0%** |
| 1.50 | 98.7% |
| 2.00 | 99.3% |
| 3.00 | 99.6% |

**v57 is not "near the cap" — it is exactly at the boundary where the cap becomes reachable, and
it never touches it.** That is not luck, it is the form: `D/S` is bounded to [−1, 1] because both
half-means are non-negative, so `1 + k·D/S` at `k = 1` lands exactly inside the clamp's [0, 2]
and the clamp is unreachable by construction. **v57's kernel is the unique member of this family
that uses the full range without ever saturating.**

So `k > 1` does not make a stronger competition. It makes a **saturated** one — bang-bang at
"loser 2g, winner 0" essentially always — and the whole value of saturation is +1.6 ± 0.7 Hz. The
axis is closed, and closed for a structural reason rather than an empirical one. The only route
to a genuinely stronger competition is to raise `g`, which raises the cap (it is 2g) — but `g`
also sets the tonic, which is exactly the entanglement §26 is about.

**The rate confirms the construction.** k changes the competition's variance and not its level —
the two offsets sum to exactly 2g clipped or not — and the measured rates are 3.50 / 3.67 / 3.70 /
3.70 / 3.69 Hz across the whole sweep. No arm drifted toward the VOID bound, and no arm settled
into a winner.

### WHAT IS NOT YET INTERPRETABLE, AND WHY THE RE-PRINT IS WORTH 26 MINUTES

The first pass printed the alternation frequency **without its spread**: 1.46 Hz at k = 1 against
1.00 at k = 1.5, which reads as a 46% longer period. The clock block on the same page prints
`tau 333 ms alternation 1.46 +/- 0.48 Hz`, so **that difference may be inside one SE** — an
averaged argmax, which is the exact statistic `s_hf` is maintained for and the exact failure this
project already has a rule about. The pole's prediction was about to be scored against it: 1.46 →
1.00 Hz should pass ~1.46× more swing, i.e. 134.9 → ~197 Hz, against the +1.6 measured.

That distinction decides the next experiment, so it was re-printed rather than guessed — and the
answer is the cautious one. **The period difference is −0.46 ± 0.51 Hz, 0.9 SE. Not resolvable.**

> So this run does **not** test the pole, and the "46% longer period bought nothing" reading that
> the unlabelled column invited is withdrawn before it was ever published. The licensed statement
> is only that **nothing measurable moved above k = 1.**

The re-print was **not a new experiment**: identical arms, seeds and budget, and the run proved it
by diffing all twelve arm rows against this log — *all twelve identical*, so both added columns
are read-only. All four verify keys passed again with the hash unchanged.

### THE ARGMAX IS THE WEAKER OF TWO INSTRUMENTS, AND IT IS THE ONE THE CLOCK WAS REFUSED ON

Printing the spread exposed something bigger than the k question. Over the four `tau` arms:

| tau_a | argmax alternation | produced swing |
|---|---|---|
| 83 ms | 2.88 ± 0.51 Hz | 45.9 ± 0.4 |
| 167 ms | 1.25 ± **0.00** Hz | 75.6 ± 0.6 |
| 250 ms | 2.62 ± 0.27 Hz | 105.6 ± 0.3 |
| 333 ms | 1.46 ± 0.48 Hz | 134.9 ± 0.4 |

The argmax series is **non-monotone**, carries SEs up to ±0.51, and at tau167 all twelve seeds
landed in a single bin — an SE of exactly 0.00 on an argmax is a pinned estimator, not agreement.
The swing series is **monotone with SEs of 0.4**. The plant is a low-pass, so swing falls as
frequency rises: **both series cannot be right, and the tight one says the period tracks tau_a.**

That matters because **"NOT A CLOCK" is this line's standing verdict across four runs and rests
entirely on that argmax** — four-point slope +0.319 at R² 0.213 against a required 1.000.
Inverting the plant's square-wave response on the swing column instead gives periods of 394 / 654
/ 926 / 1206 ms, a slope of 0.80 at R² 0.997. **That number is not claimed**: it is one
measurement expressed through a plant model which is itself miscalibrated, under-predicting the
measured swing by 1.7× even after the π/2 correction below. But it is enough to say the refusal
rests on the weaker instrument.

**A DERIVATION ERROR IN THE PRINTED PREDICTION, worth fixing wherever it is quoted.** The log's
"predicted" column applies the sinusoidal |H(f)| = 1/sqrt(1+(2πfτ)²) to the alternation. The
alternation is not a sinusoid — above k = 1 it is 99% clipped, and even at k = 1 it is strongly
square. A first-order pole driven by a square wave of period T passes `tanh(T/4τ)`, not |H(1/T)|,
and the ratio between them is **π/2 to within 1% across this entire frequency range** (1.5616 at
1.46 Hz, 1.5694 at 4.58 Hz). So every predicted figure on this line is low by that constant
factor: tau333's prediction is 79.1 Hz, not 50.7. **This does not rescue §25's residual**, which
was computed from a ratio between two arms where the π/2 cancels to 0.5% — that residual stays
open. What it does mean is that the pole arithmetic is good for a *direction* and not for a
magnitude, and the 1.7× that remains after correcting it is unexplained.

### WHAT RUNS NEXT, AND IT IS A THIRD READ-ONLY COLUMN

A period estimator with **no model and no spectrum**: a Schmitt trigger on the centred
half-difference, hysteresis at ±0.25 sd (derived from the signal, not chosen), period = **median**
up-to-up interval so a missed flip cannot halve it, and the turn count printed so a settled winner
cannot read as an infinitely long period. Both estimators print side by side — an instrument is
never silently replaced here — and the four-point slope is computed on each.

**PRE-REGISTERED.** Slope ≥ 0.80 at R² ≥ 0.90 where the argmax gave +0.32 → the clock was refused
on a broken instrument and `halfcenter` is a relaxation oscillator after all. Slope ≤ 0.60 →
the refusal is confirmed on the better instrument and the clock closes for good. Anything between
→ no verdict, and the estimator itself becomes the open question. A collapsed turn count in any
arm refuses the new instrument instead of the hypothesis.

**The pass/fail gate deliberately stays on the argmax exponent**, because four previous runs were
refused on it and the exit code has to stay comparable. What changes is that the text may no
longer assert "the period ignores tau_a" while the better instrument disagrees — a gate that
fails has to say which statistic failed it.

## §28 — THE CLOCK WAS REFUSED ON A TEST OF THE WRONG SHAPE (2026-11-01)

Gated on all four keys again, hash unchanged, and **all twelve arm rows re-printed identically**,
so the crossing estimator is read-only like the two columns before it.

### THE TWO INSTRUMENTS, SIDE BY SIDE

| tau_a | argmax period | **crossing period** | turns counted |
|---|---|---|---|
| 83 ms | 347.8 (2.88 ± 0.51 Hz) | **468.3 ± 1.7** | 2312 |
| 167 ms | 800.0 (1.25 ± 0.00 Hz) | **784.2 ± 1.5** | 1377 |
| 250 ms | 381.0 (2.62 ± 0.27 Hz) | **1070.8 ± 1.9** | 1008 |
| 333 ms | 685.7 (1.46 ± 0.48 Hz) | **1361.2 ± 2.3** | 791 |

    four-point slope   ARGMAX     +0.319   R^2 0.213
    four-point slope   CROSSINGS  +0.764   R^2 0.999

The crossing series is **monotone**, determined to **0.2%**, and counts 791–2312 turns per arm —
so the Schmitt trigger is measuring turns, not noise, and its own refusal condition did not fire.
The argmax series is non-monotone with one arm's twelve seeds in a single bin. **There is no
longer a contest between these two instruments.**

**AND THE NEW INSTRUMENT VALIDATES ITSELF, from numbers already printed.** `dt_ms = 1.0`, so a
tick is a millisecond and `settle = ticks/10` leaves a **1,080,000 ms** record. Multiply each
arm's turn count by its median interval:

    tau083   2312 x  468.3 = 1,082,710 ms   (+0.25%)
    tau167   1377 x  784.2 = 1,079,843      (-0.01%)
    tau250   1008 x 1070.8 = 1,079,366      (-0.06%)
    tau333    791 x 1361.2 = 1,076,709      (-0.30%)

All four recover the record length to **within 0.3%**, and that is not a tautology. A trigger
firing on noise would produce a **bimodal** interval distribution — many spurious short intervals
plus the real long ones — which pulls the MEDIAN down to the short mode while the count rises, so
`count × median` would fall far short of the record. A trigger missing flips would overshoot it.
Matching to 0.3% says the distribution is unimodal and tight: **every counted crossing is a
turn.** This is the check to run on any future crossing measurement before quoting it.

### BY THE PRE-REGISTERED BAR THIS IS STILL NO VERDICT, AND THAT STANDS

The bar was **slope ≥ 0.80 at R² ≥ 0.90** for "the clock was refused on a broken instrument."
It came in at **0.764**. That is below the bar I set before seeing it, so the printed verdict is
*the two instruments disagree without either winning* and **the clock is not established by this
run.** The threshold does not move after the fact.

But two independent routes now agree on the same number. §27's inversion of the plant's
square-wave response on the swing column gave **0.80 at R² 0.997**; counting zero crossings, with
no model and no spectrum, gives **0.764 at R² 0.999**. Those share no machinery. The argmax's
+0.319 is the outlier.

### AND THE REASON IT IS 0.76 AND NOT 1.0 IS THE SHAPE OF THE TEST

The four crossing periods are **linear in tau_a with a positive intercept**:

    period = 3.560 * tau_a + 179.8 ms      R^2 0.99962
    residuals  -7.0 / +9.9 / +1.1 / -4.0 ms   on periods of 468 to 1361 ms

A pure proportionality — the form this line's clock test has always assumed — fits the same four
points far worse, with residuals up to **113 ms**. **Any fixed additive latency makes the log-log
slope sub-unit no matter how good the clock is**, and 0.764 is exactly what a line with a 180 ms
offset looks like on log axes. So "period ∝ tau_a, exponent must be ~1.000" is a test the
mechanism **cannot pass**, and four runs of "NOT A CLOCK" have been reporting the intercept.

`aibaby-verdict-fitted-to-data` already carries this rule in general form: **a pre-registration
cannot save a statistic of the wrong shape.** It was written about someone else's statistic.

The offset is not a free parameter dressed as physics. It is the time for a released half to rise
and a fatigued winner to fall — set by the 50 ms rate filter and the membrane, not by `tau_a`. A
relaxation oscillator sitting on top of a finite rise time has exactly this form.

### THE FIT IS WORTH NOTHING UNTIL IT PREDICTS — SO IT IS BEING TESTED OUT OF SAMPLE

It was fitted **after** seeing those four points. Two arms are therefore appended, `tau042` and
`tau500`, and the law is refitted at run time on arms 0–3 only — whose numbers are deterministic
and published above — then asked to predict two periods it has never seen:

    tau  42 ms  ->  predicted  329.3 ms
    tau 500 ms  ->  predicted 1959.8 ms

**PRE-REGISTERED, BEFORE THE RUN.** Both arms within **10%** of prediction → the linear law holds
out of sample, the period tracks `tau_a` at 3.56 ms per ms with a measured ~180 ms latency, and
the clock question is settled affirmatively on the better instrument. **Either arm outside 10%** →
the fit was curve-fitting on four points and the clock stays refused. A VOID arm, or a collapsed
turn count, refuses **that arm** rather than the law — `tau500` in particular may leave the
oscillating window, which the duty/rate kill switch gates independently.

Note what the 10% band is not: the measurement SEs are ~2 ms (0.2%), so a 3-SE test would demand
the law hold to better than 1% at 1.5× beyond the fitted range. 10% is the band a physical law
with one fitted latency deserves; it is stated here before the numbers exist, and it will not be
widened afterwards.

## §29 — ONE ARM HELD AT 1.4%, THE OTHER LEFT THE REGIME, AND MY VERDICT CODE CONTRADICTED MY OWN PRE-REGISTRATION (2026-11-01)

Gated on all four keys, hash unchanged, **all twelve original arm rows re-printed identically**
after appending two — so the seed really is a function of the replicate only, for the third time.

### THE RESULT

    in-sample fit (unchanged, deterministic)   period = 3.560 * tau_a + 179.8 ms   R^2 0.99963

    OUT OF SAMPLE  tau 500   predicted 1959.8   measured 1986.7 +/- 3.1   +1.4%   543 turns
    OUT OF SAMPLE  tau  42   predicted  329.3   measured   60.0 +/- 0.0  -81.8% 17011 turns  VOID

**`tau500` held, and it is a real test**: 1.5× beyond the fitted range, duty 0.57, rate 2.52 Hz
(inside the kill switch), and the instrument's own consistency check passes at −0.11% — 543 turns
× 1986.7 ms recovers the 1,080,000 ms record almost exactly.

**`tau042` did not test the law; it left the oscillating regime.** Rate 18.30 Hz against the
control's 4.94 is **3.7×**, past the pre-registered 2.5× kill switch, and a short `tau_a` means
fatigue decays before it can brake anything. Its "60.0 ± 0.0 ms period" is the module's own firing
rhythm — 18.30 Hz is a 54.6 ms cycle — not a half-centre turn. The instrument's consistency check
catches the same thing independently: 17011 × 60.0 is **5.5% short** of the record, the signature
of a trigger firing on a bimodal interval distribution, where the four valid arms sit inside 0.3%.
An SE of exactly 0.00 across twelve seeds is the same pinned-estimator tell the argmax gave.

### THE PRINTED VERDICT WAS WRONG, AND IT WAS WRONG AGAINST A RULE I HAD ALREADY COMMITTED

The binary printed **"THE LAW DOES NOT PREDICT."** §28, committed as `0085f0f` *before* the run,
says: *"A VOID arm, or a collapsed turn count, refuses **that arm** rather than the law."* The
code set `held = false` on a VOID arm and so reported a refusal of the law from an arm that never
tested it. The pre-registration and the code disagreed, and the code won by default.

Fixed: `testable` is now separated from `passed`, gated on **both** the duty/rate kill switch and
the instrument's `turns × median` consistency check, and the verdict states how many arms were
testable at all. With that correction the licensed reading of these numbers is **1 of 2 arms
testable, 1 of 1 held** — which is *suggestive, not settled*, and the text now says exactly that.

> This is the fourth time in this session that a printed verdict and a written rule came apart.
> The pattern is always the same: the rule is stated in prose and the gate is written in code,
> and nothing checks them against each other. **Write the refusal clauses as the code's branch
> structure, not as a paragraph the code approximates.**

The log is not re-run for a text change; the corrected verdict is derivable from the numbers above
and will be re-printed by the next substantive run on this instrument.

### AND MY PRE-REGISTERED BAND WAS TOO LOOSE TO DISCRIMINATE — WHICH THE DATA SURVIVE ANYWAY

A 10% absolute band cannot separate the two competing laws at `tau500`, and I should have seen
that before setting it. The pure proportionality fitted on the same four arms is
`period = 4.2793 * tau_a`, which predicts **2139.6 ms** — an error of −7.1%, also inside 10%.
**Both laws would have passed my band.**

What does separate them is the comparison, stated here post hoc and derived from the pre-registered
numbers rather than from any refitting:

| | in-sample max residual | tau500 prediction | error |
|---|---|---|---|
| linear + offset | **9.9 ms** | 1959.8 | **+1.4%** |
| pure proportionality | 113.1 ms | 2139.6 | −7.1% |

The two predictions differ by 179.9 ms against a measurement SE of 3.1 ms — **58 SEs** — and the
linear law is 11× better in sample and 5× better out of sample. So the discrimination is strong
even though the band was not; but the credit goes to the measurement's precision, not to the test
I designed.

### WHERE THE CLOCK QUESTION NOW STANDS

* The **argmax is refuted as an instrument** (non-monotone, one arm's twelve seeds in a single
  bin, R² 0.213), and "NOT A CLOCK" rests on it across four runs. That much is settled.
* The **crossing estimator** is monotone, determined to 0.2%, self-consistent to 0.3%, and gives
  +0.764 at R² 0.999 — which no oscillator with a finite rise time can push to 1.0.
* The **linear law with a ~180 ms offset** fits in sample at R² 0.9996 and survives its one
  testable out-of-sample point at +1.4%, beating the proportional form by 58 SEs there.
* **It is not established.** One arm is one arm, and the low-`tau_a` end of the axis is closed by
  the kill switch, so the obvious second point cannot be bought.

### THE NEXT TEST IS THE OFFSET, AND IT IS A MECHANISTIC PREDICTION RATHER THAN MORE CURVE

The claim that carries risk: the 180 ms is **not** `tau_a`'s — it is the time for a released half
to rise and a fatigued winner to fall, set by the rate filter and the membrane. `rate_fast_tau_ms`
is already a genome field at 50.0, and 179.8/50 = 3.6. So:

    period = a * tau_a + b * rate_fast_tau_ms,   predicted a invariant, b ~ 3.6

**Doubling `rate_fast_tau_ms` to 100 should move the offset to ~360 ms and leave the `tau_a`
coefficient at ~3.56.** That is a prediction no curve fitted in `tau_a` can make, and an invariant
coefficient is much harder to produce by accident than one more point on a line.

*Sharpened in §30 before the run: the kernel reads `rate_fast_[...]` directly, so the filter is a
lag INSIDE the loop and the model collapses to `period = A·(tau_a + rate_fast_tau_ms)` — one
coefficient for both axes, which forces the intercept to 178.0 instead of fitting `b`.*

**THE CONFOUND, AND IT IS WHY THIS IS NOT LAUNCHED HERE.** `rate_fast_tau_ms` is a **shared
constant**: divisive normalisation, intrinsic plasticity and the envelope-onset detector all read
the same filter. Changing it changes far more than the half-centre's rise time, which is exactly
the class `aibaby-shared-constants-audit` and `aibaby-nmax-rerolls-the-wiring` exist to gate. The
run needs its own specificity argument first — at minimum a `hc_gain = 0` arm at each filter
setting, to show what the filter change does on its own before any of it is credited to the offset.

## §30 — THE OFFSET TEST, PRE-REGISTERED BEFORE THE RUN (2026-11-01)

§29 named the offset test and deliberately did not launch it, because `rate_fast_tau_ms` is a
shared constant and the run needed a specificity argument first. This section is that argument,
the sharpened prediction, and the decision rule — all written and committed **before** the 29-arm
run finished, so the verdict cannot be fitted to the numbers.

### THE PREDICTION IS SHARPER THAN §29 THOUGHT, AND IT HAS NO FREE PARAMETER

§29 wrote the model as `period = a·tau_a + b·rate_fast_tau_ms` with `a` invariant and `b ≈ 3.6`,
noting only that 179.8/50 = 3.6. Reading the kernel closes the gap. The half-centre's two
half-means are sums of `rate_fast_[...]` — [network.cpp:2240-2241](../core/src/network.cpp#L2240) —
so the filter is **not an instrument beside the oscillator, it is inside the loop**. Two cascaded
lags, `tau_a` and the filter, and if the period is set by their sum then

    period = A * (tau_a + rate_fast_tau_ms)

with **one coefficient for both axes**. That forces the intercept rather than fitting it:
`A · 50 = 3.560 · 50 = 178.0 ms` against the **179.8 ± 11.0** measured. The agreement is the
reason to run this; it is also exactly the kind of coincidence that has to be tested by moving the
quantity, not admired at one point.

### THE CONTRAST, NOT A TOLERANCE ON ONE MODEL

§28's band was 10% and **both candidate laws would have passed it** — the lesson is in
`aibaby-verdict-fitted-to-data`. So this is registered as a comparison:

| | model A — cascaded lags | model B — offset is elsewhere |
|---|---|---|
| form | `intercept = 3.560 · rft` | `intercept = 179.8` at every setting |
| rft 25 | 89.0 | 179.8 |
| rft 50 | 178.0 | 179.8 |
| rft 75 | 267.0 | 179.8 |
| rft 100 | 356.0 | 179.8 |

Every setting separates the two by ≥ 87 ms against an intercept SE of ~11 ms, so unlike §28 this
contrast can actually choose. **The deciding statistic** is the coefficient `c` of the intercept
**on** `rate_fast_tau_ms`, fitted across the four settings:

* `|c − a|` within 3 SE **and** residual offset within 3 SE of zero → **model A**: one law over
  both axes, and the 180 ms that four runs reported as a failed clock is the rate filter.
* `|c|` within 3 SE of zero → **model B**: moving the only other lag in the loop by 4× does not
  move the intercept. `rate_fast_tau_ms` is **closed** as its source.
* `c` resolvably non-zero but ≠ `a` → the filter contributes, the equal-weight form is refused,
  **no mechanism verdict**.

`c` and `a` are measured on **different axes** and have no reason to agree unless the lags
genuinely add. **Slope invariance is a necessary condition, not a bonus:** a sum of lags cannot
change the coefficient on `tau_a`, so a moving slope refuses the additive form however the
intercepts fall.

### THE SPECIFICITY ARGUMENT §29 ASKED FOR

Each setting carries its own `hc_gain = 0` control, and it does two jobs. It discloses what the
shared filter does with no competition present, and it is the baseline that setting's kill switch
gates against — via a new `hc_ctrl_for()` that resolves an arm's control by matching drive **and**
filter instead of a hardcoded index. Gating a 100 ms-filter arm on the 50 ms control's firing rate
would re-decide VOID for a reason unconnected to the mechanism, which is `aibaby-match-at-the-
operating-point` restated. Every pre-v77 arm still resolves to arm 7, so the archived rows re-print
unchanged — and five separate determinism checks assert exactly that.

### WHAT THE SMOKE CHANGED BEFORE ANYTHING RAN AT LENGTH

A 40k-tick smoke was run first, and it falsified two things I had already written down.

**The controls report a filter-scaled period.** Across rft 25/50/100/200 the no-mechanism controls
give 139 / 189 / 288 / 493 ms — `90 + 2.0·rft` to within a millisecond. My comment had argued the
period was "safe by construction" because a one-pole low-pass shifts phase without moving
crossings. True, and beside the point: a control has **no periodic signal at all**, so the trigger
rides noise, whose autocorrelation time is the filter's. The estimator was always going to report
that. **The inference I had attached to it was invalid** — "the controls track the filter, therefore
nothing above them is a mechanism" would condemn the instrument for behaving correctly on an empty
input. It is a per-arm **floor** (period ≥ 2× the matched control's), and the global veto is gone.

**An sd-ratio gate I derived refused all sixteen arms.** It required `sd_arm ≥ 4·sd_control`,
derived as "the band, 0.25·sd_arm, must sit at one noise sigma". The derivation assumes `sd_arm` is
the oscillation alone, but it is oscillation and noise **in quadrature**, so a 4× sd ratio demands
a 15:1 power ratio. Measured ratios are 2.5–4.6×, and the gate rejected the four anchor arms whose
fit has already predicted a fifth to +1.4% out of sample. **A gate that refuses a measurement which
has made a successful out-of-sample prediction is testing the wrong thing.** The ratio is now
printed as a column and gates nothing.

**rft 200 was dropped from the axis.** Wherever the filter is slower than the fatigue the
oscillation is gone — at rft 200 both `tau083` and `tau167` collapse to a fast rhythm (90 ms over
397 turns) that passes the turns × median consistency check, because a *uniformly* fast noise
rhythm recovers the record just as well as a slow real one. Arms above `rft ≈ tau_a/1.5` are not a
harder test of the law, they are no test of it. The axis is 25–100, which is 4× and keeps the whole
tau set alive at three of four settings.

### WHAT CANNOT BE CLAIMED FROM THIS RUN WHATEVER IT SAYS

The four taus are the same at every setting, so the intercepts are compared at matched leverage —
but the **tau_a axis itself is unchanged**, so this run cannot revisit whether the period tracks
`tau_a`. It asks one thing: whether the intercept is the other lag. A setting with fewer than three
live arms refuses **itself** and not the law, written as the code's branch structure rather than as
prose beside it — §29's rule, earned when this file printed "THE LAW DOES NOT PREDICT" for an arm
that never tested the law.
