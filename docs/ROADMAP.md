# Where this project stands, and what is prepared next

Written 2026-10-09, revised 2026-10-12. The *record* is the root `README.md`, which is chronological and long;
the *curated findings* are `docs/discoveries.pdf`; the *conclusion* — what the creature does, what it
cannot, and the one reason behind both — is `docs/CONCLUSION.md`, added 2026-11-04 and the place to
start. This file is the short forward-looking
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

## §31 — THE OFFSET IS NOT THE RATE FILTER. THE INTERCEPT MOVED THE WRONG WAY (2026-11-01)

The §30 prediction is refused, and refused in the one manner that cannot be argued with: the
intercept moved **in the opposite direction** to what a sum of cascaded lags requires.

29 arms, 12 reps, 1.2M ticks. `results/halfcenter-v77.log`. Verify gated on all four keys with
`ad96f882becbee92` unchanged.

### THE ANCHOR REPRODUCED ITSELF EXACTLY, WHICH IS WHAT LICENSES THE REST

At `rate_fast_tau_ms = 50` the four taus give **slope 3.560 ± 0.048, intercept 179.8 ± 11.0 ms,
R² 0.99963** — the published fit to every decimal, re-derived by a binary carrying fifteen new
arms. Three of the five determinism checks were byte-identical (the fourteen main-table rows, the
kill-switch VOID flags, the regime profile's SE-vs-control column) and the v76 k-sweep block was
too. The periods are 468.3 / 784.2 / 1070.8 / 1361.2 ms, unmoved.

*Check 4 flagged and it was my script that was wrong, not the run.* It diffed the clock-law block
against a log written by the **pre-fix** binary, so it caught the §29 verdict correction and the
new consistency column — text I had deliberately changed. Every number in that block is identical
(`60.0 ± 0.0`, `1986.7 ± 3.1`, `+1.4%`, 543 turns), and the fit and all four residual lines diff
clean. The corrected verdict text is now in an archived log, which closes §29's loose end.

### THE RESULT

| | rft 50 (anchor) | rft 25 |
|---|---|---|
| slope on `tau_a` | 3.560 ± 0.048 | 3.372 ± 0.093 |
| intercept | 179.8 ± 11.0 ms | **227.6 ± 21.3 ms** |
| R² | 0.99963 | 0.99847 |
| live arms | 4 of 4 | 4 of 4 |

The intercept changed by **+47.8 ± 24.0 ms**. Model A — cascaded lags, `period = A·(tau_a + rft)` —
required it to change by **−89.0 ms**. That is **5.7 SE away and the opposite sign**, and a
wrong-signed prediction is not rescued by any choice of coefficient.

**Slope invariance HOLDS** at −1.80 SE (3.372 ± 0.093 against 3.560 ± 0.048). This matters: it
removes the escape that the additive form was broken and the test therefore meaningless. The
coefficient on `tau_a` is the same at both settings; it is the **intercept specifically** that
refuses to follow the filter.

**Model B — the offset is elsewhere — is unrefuted at 2.0 SE, and that is all.** It is not
established. An unrefuted null is a weaker thing than a confirmed model, and the +47.8 ms hints at
a weak *negative* dependence that neither candidate predicts.

**So `A·50 = 178.0` against `179.8 ± 11.0` was a coincidence.** That agreement is what the whole run
was built on. It is worth saying plainly: a one-parameter model that lands 1.8 ms from a measured
value, derived from reading the kernel rather than fitted, was still wrong.

### THE DECIDING STATISTIC WAS NOT MEASURED, AND I AM NOT LOOSENING A GATE TO GET IT

§30 named the coefficient of the intercept **on** the filter, fitted across four settings, as the
deciding statistic. **Only two settings were fitted, so it does not exist in this run.**

`rft 75` and `rft 100` each refused themselves with 2 live arms:

* `rf75t083` (113.3 ms over 6615 turns, −30.6% consistency) and `rf100t083` (100.0 ms over 10239
  turns, −5.2%) **collapsed** — the oscillation is gone where the filter approaches `tau_a`, which
  the 40k smoke predicted and §30 recorded in advance.
* `rf75t167` (+2.8%) and `rf100t250` (+2.3%) were dropped on the **±2% consistency gate alone**.

**Loosening that gate to ±3% would have given three settings plus the anchor and therefore the
deciding coefficient. I am not doing it.** The ±2% came from v76, where the anchor arms land within
0.3%, and it was in the binary before this run. Relaxing a pre-registered gate because it blocks
the statistic I wanted is the definition of fitting the verdict to the data, and this file already
carries four entries about doing exactly that. The right repair is a longer record or more reps to
make those two arms as regular as the others — not a wider band.

### WHAT THE CONFOUND CHECKS SAID, INCLUDING THE ONE I NEARLY MISSED

* **Firing rate held exactly.** All four controls sit at **4.94 Hz** — homeostasis pins it, so the
  shared filter does not move the rate.
* **Duty drifted, and my check would have missed it.** The controls go **0.53 → 0.57 → 0.60 → 0.62**
  across rft 25 → 100, a +17% spread. My operating-point check tested the *rate only*, which was
  always going to pass; it would have certified a matched operating point while the duty moved
  underneath. Both axes are printed now. The two fitted settings differ by 0.04 in duty, which is
  disclosed rather than dismissed.
* **The noise floor is real, linear, and a different number.** The no-mechanism controls report
  115.8 / 164.2 / 212.5 / 262.5 ms, i.e. `66.7 + 1.954·rft` at R² 0.9999. The claimed coefficient on
  `tau_a` is 3.560 — a ratio of 1.82, so the signal periods are not the floor being re-measured.
  Every live arm clears 2× its own setting's floor.

### AN ARCHITECTURAL CONSTRAINT FELL OUT OF IT

**The half-centre only works cleanly at `rate_fast_tau_ms ≤ 50`.** Above that the fastest fatigue
arm collapses: the loop cannot resolve an adaptation faster than the filter that measures it. The
shipped 50.0 sits near the top of the usable range, which is a constraint on the architecture and
not just on this test — any future change to that filter has to re-check the gesture.

### THE FIFTH TIME A PRE-REGISTRATION LIVED IN PROSE AND NOT IN A BRANCH

§30 committed a per-setting contrast table and wrote that "this contrast can actually choose". The
code evaluated only the four-point coefficient and, when two settings refused themselves, printed
*"report the per-setting intercepts and nothing more"* — withholding a committed test that **one**
fitted setting is enough to run, together with the floor and operating-point disclosures, because
all of it sat inside the `n ≥ 3` branch. The contrast above was therefore computed by hand from the
binary's own printed per-setting fits, against §30's table as committed in `21fa450` **before the
run**. The branch now exists and prints unconditionally, so the next run on this instrument states
its own verdict. **A pre-registration the code does not branch on is not a pre-registration.**

No re-run was spent on this. Re-running would have had the arithmetic done by a binary written
*after* the numbers were seen, which buys an arithmetic check and no integrity at all — the
integrity is in §30's commit timestamp, and the arithmetic was verified independently instead.

### WHERE THE 180 MS GOES NEXT

`rate_fast_tau_ms` is **closed** as its source in the equal-weight cascaded form, and has no support
in any form: the one usable test point moved the wrong way.

The next candidate is better-shaped than this one was. **`leak_tau_ms` is a PER-MODULE field**
(`dna/default.toml`, 20.0 on the vocal module) rather than a shared constant, so changing it on the
vocal module alone carries none of the specificity burden that made §29 hold this run back. The
membrane is also the physically obvious home for "the time for a released half to rise". Note
179.8/3.560 = 50.5 ms of effective lag to account for, against a 20 ms membrane and a 3 ms
refractory — so it is probably not a single field, and the test should be built to measure a
coefficient rather than to confirm a coincidence. **That is the lesson this run actually paid for.**

---

## §32 — THE TWO PER-MODULE LAGS, PRE-REGISTERED AS A COEFFICIENT AND NOT AS A PREDICTION (2026-11-01)

§31 closed `rate_fast_tau_ms` and named the lesson: **build the test to measure a coefficient, not
to confirm a coincidence.** This is that test. It is committed before the run.

59 arms, 12 reps, 1.2M ticks. The 29 arms of v77 re-print unchanged; 30 are appended.

### WHAT CHANGED IN THE SHAPE OF THE QUESTION

v77 asked whether a predicted number survived being moved. That shape has exactly two answers and
one of them is worthless: when the prediction failed, the run was left with no statement about how
much of the 180 ms that field *does* buy. This run measures `dIntercept/dField` on both remaining
lags and reports it with an error bar. Every outcome is then usable, including zero.

**And no outcome here can explain the offset.** 179.8/3.560 = **50.5 ms** of effective lag has to be
accounted for. The membrane ships at 20 ms and the refractory at 3 ms, so even if both land at the
full lag coefficient they sum to 23 ms — under half of it. A test built to confirm "the membrane IS
the 180 ms" would therefore have been built to fail. The committed deliverable is the **budget**:
what the two measured coefficients account for, and what is left over.

### THE TWO AXES, AND THEY ARE DELIBERATELY UNEQUAL

Both are per-module (`DnaModule::leak_tau_ms`, `DnaModule::refractory_ms`), patched on the vocal
module alone, so neither can reach the auditory module or the divisive normaliser the way the shared
`rate_fast_tau_ms` did. That removes v77's specificity burden. It does not make them equivalent:

| | `refractory_ms` | `leak_tau_ms` |
|---|---|---|
| in the loop as | **pure dead time** | a lag **and** an inverse gain |
| side effects | rate ceiling 1000/refr Hz | effective drive scales as `leak_tau/dt` |
| settings | 3 (anchor) / 12 / 22 / 35 | 20 (anchor) / 35 / 50 / 65 |
| a coefficient of 3.560 means | the offset is dead time in this loop | **ambiguous** — lag or gain |
| a coefficient of 0 means | this field is not in the loop | this field is not in the loop |

The membrane's confound is arithmetic, not a worry: the update is `v += leak_alpha*(v_rest − v) +
drive` with `leak_alpha = dt/leak_tau`, so a steady drive `d` holds `v` at `v_rest + d/leak_alpha`
and the effective drive in units of threshold scales with `leak_tau`. Halving the membrane halves
how far *any* drive — the half-centre's own inhibition included — pushes the neuron, while the
threshold does not scale with it. **So only a zero on that axis is unambiguous**, because then
neither the lag nor the gain moved the intercept. `refractory_ms` is the axis that can refuse
something on its own, and it is evaluated first for that reason.

### THE PRE-REGISTERED VERDICTS, AS THE CODE'S BRANCHES

Written as branches and not as prose beside the code — §29's rule, and §30 broke it a fifth time by
burying a committed contrast inside an `n ≥ 3` gate. Per axis, with `A` = that axis's own anchor
coefficient on `tau_a` and `c ± ce` = the measured coefficient:

| condition | verdict |
|---|---|
| the `tau_a` slope moves ≥ 3 SE across settings | **refused as a sum of lags** — the field interacts with `tau_a`, it does not add to it. Contributes nothing to the budget. |
| `\|c\| < 3 SE` and `\|c − A\| ≥ 3 SE` | **not a lag in this loop.** The field is CLOSED as a source of the 180 ms. |
| `\|c − A\| < 3 SE` and `\|c\| ≥ 3 SE` | **it is a lag** — the same coefficient on two axes with no reason to agree unless the lags add. On the membrane axis this still cannot claim the mechanism. |
| both inside 3 SE | **not separated, and UNDERPOWERED rather than null.** Needs more reps or a longer record. |
| both outside 3 SE | **contributes, but not as a simple lag.** A negative `c` is called out: no lag can be negative, and that is the filter axis's signature. |

The design aims for that separation rather than hoping for it: with per-setting intercept SEs of
11–21 ms, `Sxx` of 566 on the refractory axis and 1125 on the membrane gives a coefficient SE of
~0.76 and ~0.54, so **3.560 and 0 are separated by 4.7 and 6.6 SE.** Derived before the run, and
re-derived after the smoke moved the membrane range — see below.

**And which coefficients may enter the budget is fixed now, not after seeing them.** Only an axis
whose coefficient is measured *and* additive is summed. A refused additive form, an underpowered
non-separation, and a **resolvably negative** coefficient are each excluded and named with their
measured value, because a line that silently omits an axis reads as a zero and none of those three
is a zero. The negative case is the one that needs the rule written down in advance: a negative
`dIntercept/dField` cannot be a lag, so summing it would let it **cancel** a real contribution from
the other axis and shrink the unaccounted remainder by arithmetic rather than by mechanism. The
filter axis produced exactly that sign, so this is not a hypothetical.

### WHAT IS CARRIED OVER UNCHANGED, ON PURPOSE

* **The ±2% consistency gate is not widened.** It cost v77 the deciding coefficient — two settings
  each lost an arm at +2.8% and +2.3%, and 3% would have handed the statistic back. The licensed
  repair for a blocked statistic is more data, so this run buys a **fourth setting per axis as
  insurance** instead of a looser gate.
* **The floor is disclosed, never vetoed on.** The controls' reported periods track whatever sets
  the noise's autocorrelation time, because a Schmitt trigger handed a signal with no period in it
  rides noise. That is the instrument working on an empty input. Per-arm floor (period ≥ 2× the
  matched control), plus the floor's own slope printed beside the claimed coefficient.
* **The operating point is disclosed on BOTH duty and rate.** v77's first version checked the rate
  alone, and the rate is the quantity homeostasis actively defends: all four controls read 4.94 Hz
  while duty drifted 0.53 → 0.62 underneath. A regulated quantity cannot detect a shift.
* **Slope invariance is a necessary condition, not a bonus.** A sum of lags cannot change the
  coefficient on `tau_a`. On the filter axis this is what removed the "the additive form was broken,
  so the test was meaningless" escape, and it does the same job here.

### WHAT THE SMOKE CHANGED, BEFORE ANY OF THIS RAN AT LENGTH

Three 60k-tick smokes. None of their effect sizes is quoted anywhere — `--allow-short` certifies
nothing — but they changed the design twice, and the second change is the one that matters.

**1. The membrane axis was written straddling the shipped value and a quarter of it was dead.**
`leak_tau_ms = 8` came back with duty 0.00 on all four mechanism arms *and* on its own control,
which ran 1.34 Hz against 4.1–5.0 everywhere else. The arithmetic above says why: effective drive
scales with `leak_tau`, so 8 ms is 0.4× the shipped drive and the module cannot reach threshold.
**The cliff is below the shipped value and the direction is not symmetric.** At 35 and 50 the
controls run duty 0.55/0.54 and rate 5.04/5.02 against the anchor's 0.47/4.41 — a matched operating
point, so homeostasis absorbs the gain change going up and not going down. The failure modes differ
in kind, too: too low voids an entire setting including its control, too high can at worst cost the
single fastest arm and leave three live, which is still a fit. The axis is now **20 / 35 / 50 / 65**,
upward only. `Sxx` rises 781 → 1125 and the coefficient SE improves to ~0.54. A fourth smoke
confirmed 65 ms is alive and that its control is the *closest* of any leak setting to the anchor.

**2. The refractory axis moves the operating point, which I had asserted it would not.** "Pure dead
time, no gain term" is true of the single-neuron update and false of the module: its controls run
duty 0.47/0.43/0.41/0.36 and rate 4.41/4.14/3.93/3.71 across 3/12/22/35. Homeostasis does *not*
hold the rate against a refractory period the way it holds it against the filter. That matters
because the v76 normaliser divides by the **pair's own mean**, so a 1.19× lower module rate raises
the between-coupling — a competition-strength change of the size v76's k sweep probed and could not
resolve. The axis is kept, because **slope invariance separates the two**: a competition-strength
change scales the whole relaxation cycle and so moves the coefficient on `tau_a`, while a lag adds
and so moves the intercept alone. That check was already in the design as a necessary condition; it
now does a second job, and it is printed before any verdict.

**3. THE INTERCEPTS WERE NOT BEING COMPARED AT MATCHED LEVERAGE, AND THIS IS THE REAL FIND.**
§32 above inherited a sentence from v77 — *"the same four taus at each setting, so every fit has an
identical design matrix"* — and the code never enforced it. It fitted whatever survived the gates.
In the smoke, `tau083` fails the ±2% consistency gate at the **anchor** and passes it at refractory
12/22/35, so the anchor fitted on three taus and the others on four. The periods at the three
*shared* taus are identical to within a millisecond (759.2/1082.1/1415.0 against 758.3/1085.0/
1415.4) and the fitted intercepts read **97.7 against 12.3–18.0.** Refitting all four on the common
three turns that into **97.7 / 96.7 / 97.1 / 93.1** — flat.

So ~80 ms of apparent intercept shift was the design matrix: dropping the shortest tau removes the
leverage that pins the intercept. That is far larger than any effect this run is looking for, and it
would have arrived with a tight error bar on it.

The fix: every intercept entering a coefficient is refitted on the taus live at **every** fitted
setting. The per-setting law is still printed on its own live arms as `FIT (own)`; the balanced
refit is labelled and marked `<- REFITTED` where it differs. Fewer than three common taus and the
axis refuses itself rather than producing a coefficient. The OLS is factored out, because the block
now fits twice per setting and a third hand-inlined copy is how the two come to disagree.

**v77 escaped this by luck.** Both of its fitted settings happened to keep all four arms, so its
published coefficient would have been valid — but the hazard was live in that code and unnoticed.
The lesson is the §29 one again, in a new place: *an assumption stated in a comment is not enforced
by the comment.*

### 4. THE v78 ARMS WERE HIJACKING THE v77 VERDICT BLOCK

Caught by pre-testing determinism check 6 against a smoke log rather than waiting for the run — the
one habit §31 bought, where a check flagged and the script was wrong.

Every v78 arm carries `rft = 50.0` and `drive = 1.40`. Those are the two fields the **v77** block's
arm-resolution loop matched on, so at its anchor setting all thirty new arms passed both tests and
the last one to match each tau won the assignment. The block printed its anchor as
`lk50t083…lk50ctrl` with a fitted intercept of **142.5 ms where v77 published 179.8**, and
`A predicts 178.0 (off by −35.5)`. A published refusal, silently re-decided by arms appended below
it, in a block nothing in this run was supposed to touch.

The fix pins the two per-module lags at their shipped values in that loop too. **It is the same
failure `hc_ctrl_for` was written to fix, one level up:** a selector that names only the fields it
knows about silently absorbs every field added later. The rule this earns:

> **An arm selector must be stated positively over the whole dose vector** — every axis pinned,
> including the ones a block does not care about — rather than as whatever subset happened to
> distinguish the arms on the day it was written.

v78's own selector takes a `hold` parameter for exactly this reason and the older block still needed
the clause added by hand, which is the measure of how easy the omission is. The rest of the file was
audited: the clock-law and exponent blocks select by **hardcoded index** (`a < 4`, `a = 12; a < 14`)
and are immune to appending, and `hc_ctrl_for` already pins the full vector. Both field-matching
selectors now pin all eight dose fields, and determinism check 6 exists to prove the v77 block
re-prints unchanged at full length.

### TWO FREE CHECKS THIS DESIGN BUYS

1. **The anchor is shared.** Both axes pass through the same four arms at the shipped 20.0/3.0, so
   the two calls must print an identical anchor fit. If they do not, the arm resolution is picking up
   arms it should not, and the coefficients are not comparable. The code prints the cross-check.
2. **The range stops short of the fatigue constant.** v77 found the half-centre dies wherever a loop
   lag approaches `tau_a` (at rft 100 the tau 83 arm collapsed to a 100 ms noise rhythm over 10239
   turns). Both fields here stay well under 83 ms, and the refractory’s rate ceiling at 35 ms is
   28.6 Hz against the 10.8 Hz the fastest arm runs at.

### WHAT CANNOT BE CLAIMED, WHATEVER COMES BACK

* **Not** that the offset is explained. Neither field can be 50.5 ms of lag; the budget will have a
  remainder and the remainder is the honest output.
* **Not** a mechanism from the membrane axis on a non-zero reading — the gain confound is
  structural, and the verdict branch says so itself rather than leaving it to this paragraph.
* **Not** anything about `rate_fast_tau_ms`, which is closed. The anchor setting here is the shipped
  50.0 and no arm moves it.

---

## §33 — THE OFFSET IS NOT A LAG IN ANY OF THIS LOOP'S THREE TIME CONSTANTS (2026-11-01)

59 arms, 12 reps, 1.2M ticks. `results/halfcenter-v78.log`, verify gated on all four keys with
`ad96f882becbee92` unchanged. Pre-registered in §32, committed as `a694213` before the run.

**The answer is negative on both axes, and for the first time it is quantitative.** `refractory_ms`
accounts for **+0.5 ± 0.2 ms of the 179.8 ± 11.0 ms** this run fits at the shipped setting — 0.3%.
`leak_tau_ms` refuses the additive form outright. With v77's closure of `rate_fast_tau_ms`, **all
three time constants in the half-centre loop are now closed as the source of the 180 ms.**

### THE REFRACTORY AXIS — A CLEAN MEASUREMENT, AND IT SAYS NO

| `refractory_ms` | 3 (shipped) | 12 | 22 | 35 |
|---|---|---|---|---|
| slope on `tau_a` | 3.560 ± 0.048 | 3.572 ± 0.036 | 3.577 ± 0.027 | 3.572 ± 0.019 |
| intercept | 179.8 ± 11.0 | 179.6 ± 8.2 | 181.2 ± 6.3 | 185.4 ± 4.3 |
| R² | 0.99963 | 0.99980 | 0.99988 | 0.99994 |
| live arms | 4 | 4 | 4 | 4 |

All four taus live at all four settings, so the balanced design equalled each setting's own fit and
the correction cost nothing. **Slope invariance holds** at +0.2/+0.3/+0.2 SE. **The operating point
held** — rate 4.91–4.94 Hz (1.01×), duty 0.57–0.62 (+8%), both inside their disclosure thresholds.

**Coefficient +0.182 ± 0.054 ms per ms**, against the **+3.560 ± 0.048** a lag requires: they differ
by **46.4 SE**. The pre-registered branch that fired is *"contributes, but not as a simple lag"* —
3.4 SE from zero is just past the 3-SE line, so it did not take the "closed" branch. **See the
threshold caveat below; it does not change the reading.** Over the shipped 3.0 ms the field is worth
half a millisecond of a 180 ms offset.

### THE MEMBRANE AXIS — IT ROTATES THE LAW RATHER THAN SHIFTING IT

The raw periods say it without any fit:

| | leak 20 | leak 35 | leak 50 | leak 65 |
|---|---|---|---|---|
| `tau_a` 83 | 468.3 | 394.2 | 393.3 | 395.8 | **← falls 74 ms** |
| `tau_a` 167 | 784.2 | 797.5 | 815.0 | 830.8 | |
| `tau_a` 250 | 1070.8 | 1100.0 | 1130.0 | 1153.3 | |
| `tau_a` 333 | 1361.2 | 1406.7 | 1441.7 | 1476.7 | **← rises 115 ms** |

A slower membrane makes the **slow** arms slower and the **fast** arm faster. That is a rotation
about a pivot, not an additive shift, and the slope on `tau_a` duly climbs monotonically —
**3.476 → 3.670 → 3.775 → 3.891**, +11.9% across the range, on the common three taus. The
pre-registered branch **refused the axis as a sum of lags** and excluded it from the budget by name
rather than scoring its coefficient.

**This is exactly the discriminator §32 committed, firing on the axis it was predicted for.** The
membrane is a lag *and* an inverse gain; a gain change scales the whole relaxation cycle, so it moves
the coefficient on `tau_a`, while a lag adds and moves the intercept alone. The refractory period is
dead time and did not move the slope. The two axes dissociate along precisely that line. And the
membrane's intercept moved **down** (−19.0 / −17.9 / −22.0 ms) — the wrong sign for a lag regardless.

**The anchor cross-check DIFFERED, and it was right to.** `lk35t083` missed the ±2% consistency gate
at −2.3%, so the membrane axis's common set is {167, 250, 333} and its balanced anchor refits to
203.0 ± 3.4 against the refractory axis's four-tau 179.8 ± 11.0. The two `FIT (own)` lines are
identical (3.560 ± 0.048, 179.8 ± 11.0), which is what the check asks for. **Consequence:** the two
axes' coefficients rest on different designs — each internally balanced — so they are individually
valid and must not be compared with each other quantitatively.

### THE BUDGET

```
refractory_ms  +0.182 +/- 0.054 ms per ms  x   3.0 ms shipped  =    +0.5 +/- 0.2 ms
leak_tau_ms    NOT COUNTED: the tau_a slope moved: not an additive lag at all
               (measured -0.433 +/- 0.197 ms per ms)
accounted      +0.5 +/- 0.2 ms of the 179.8 +/- 11.0 ms (0%), leaving
               +179.2 +/- 11.0 ms with no account on 1 of 2 axes.
```

### WHERE MY OWN PRE-REGISTRATION WAS WRONG

**The 3-SE threshold was mis-specified for a four-point fit.** A fit on four settings has 2 dof,
where the two-tailed 5% critical t is **4.303**, not 3. So the refractory coefficient at 3.4 SE is
p ≈ 0.08 — *not* significant at 5%, and the branch that called it "resolvably non-zero" overstates
what four points support. The slope-invariance SEs on the membrane axis are worse: three-point fits
carry 1 dof, where the critical t is **12.706**, so of the three slope contrasts (9.9, 17.1, 31.0
SE) only two clear it, and a `± 0.003` SE on three near-collinear points is not a trustworthy
scale at all.

**Neither correction changes a conclusion, and I am not relabelling either verdict** — the
thresholds were committed before the run and moving them afterwards is the thing this project keeps
refusing. What carries both readings is magnitude, which is threshold-free: half a millisecond of a
180 ms offset, and a 74 ms fall against a 115 ms rise at opposite ends of the same axis. **The fix
for the next run is a dof-scaled t-threshold rather than a flat 3-SE z-rule.**

### WHAT THE SMOKE GOT WRONG ABOUT THE FULL RUN, AND WHY DISCLOSING BEAT GATING

* §32 recorded, from a 60k smoke, that the refractory axis moves the operating point — duty
  0.47→0.36, rate 4.41→3.71 — and argued slope invariance was needed to separate that confound from
  a lag. **At 1.2M the operating point holds** (1.01× on rate, +8% on duty). The drift was a
  short-run artefact. Had I *gated* on it instead of disclosing it, I would have thrown away the one
  clean axis in the run.
* The smoke also showed leak 65's slope moving 3.1 SE, and §32 said that setting stays because
  dropping an arm for producing an inconvenient verdict is the verdict-fitting this project
  refuses. **At full length the slope moves at every membrane setting, monotonically.** That signal
  was real and keeping the setting was right.
* The balanced-design correction, which the smoke forced, **cost nothing on the refractory axis**
  (4 of 4 taus common) and **was load-bearing on the membrane axis** (3 of 4, and it is what makes
  the rotation visible as a slope series rather than as intercept noise).

### WHERE THE 180 MS GOES NEXT

Three time constants are closed: the rate filter (v77), the refractory period and the membrane
(v78). The remaining structural candidates are **not** time constants of the loop:

**1. The tonic drive, and this run is what promotes it.** The textbook form this file has cited
since §28 (Shpiro/Curtu/Rinzel/Rubin) makes the period depend on the adaptation constant *and the
input strength*: the intercept is the **escape time**, how long a released half takes to climb back
to threshold, and that is set by the drive, not by `tau_a`. v78's own finding points the same way —
the membrane, which is an inverse **gain**, rotated the law, while the refractory period, which is
pure dead time, did essentially nothing. **The gain-like axes are where the dynamics live and the
drive is the pure gain axis.** `halfcenter_drive` is already an `HCArm` field (1.40 shipped) and
`drive200` already exists as a single arm, so this is four taus at two or three drives.

*Checked against what is closed:* `hcdrive` closed the **(gain, drive) plane for the lesson/gesture
trade** and says do not re-sweep it. That is a different measurement — and decisively, its archived
log predates the crossing estimator by two DNA versions, so **it contains no period data at all.**
The period-versus-drive relation has never been measured on the good instrument.

**2. The vocal frame.** [[the frame is a body part]] explains five refusals by a resonant jaw at
~4.5 Hz — a 222 ms period, the right order for a 180 ms offset, and the only candidate whose natural
scale matches without tuning.

**3. The estimator — and I had this argument backwards until I checked the code.** The draft of this
section called the Schmitt band "the cheapest candidate", reasoning that a finite dead band adds a
fixed traverse time per turn. **That is wrong.** The estimator takes **up-to-up** intervals
(`experiments_milestones.cpp`, `state <= 0 && x > band` → `state >= 0 && x < -band`), and for a
periodic signal whose amplitude exceeds the band the up-to-up spacing *is* the true period,
independent of band width. The band delays each detection in phase and does not change the spacing
between detections.

This is the same phase argument that v77 found **true but beside the point** for the *controls* —
and the reason it applies here is the reason it failed there: an arm has a genuine periodic signal
and a control has none. So the estimator is probably innocent, which demotes this from a candidate
to a **cheap confirmation**: sweep the 0.25 coefficient with no genome change, prediction that the
intercept does *not* move. Worth one arm block to retire an instrument question that has shadowed
four runs, and not worth a run of its own.

## §34 — THE DRIVE AXIS, PRE-REGISTERED AS A BOUND (2026-11-01)

Three axes are closed as the clock law's 180 ms offset and all three were **lags**: the shared rate
filter (§31), and both per-module time constants (§33). This run moves the **gain**.

`period = 3.560·tau_a + 179.8 ms`, R² 0.99963, intercept 16 SE from zero. The coefficient is
understood; the offset has no account.

### Why drive, and why *now*

v78 did not only produce two refusals. On the way it produced a **discriminator**, and that
discriminator is what selects this axis:

> A gain change scales the whole relaxation cycle and so moves the **coefficient** on `tau_a`; a lag
> *adds* and so moves the **intercept** alone.

The two v78 axes dissociated exactly along that line. `leak_tau_ms` is a lag *and* an inverse gain,
and it **rotated** the law — slope 3.476 → 3.891 across 20–65 ms, monotone, visible in the raw
periods before any fit (`tau_a` 83 *fell* 468.3 → 395.8 while `tau_a` 333 *rose* 1361.2 → 1476.7).
`refractory_ms` is pure dead time, and it moved the intercept 0.5 ms of 179.8 while leaving the
slope alone to within 0.3 SE.

So the dynamics of this oscillator live on the **gain-like** axes, and `halfcenter_drive` is the pure
gain axis — Matsuoka's tonic *S*, with no time constant in it at all.

And the textbook puts the offset here. Shpiro/Curtu/Rinzel/Rubin make a relaxation oscillator's
period the sum of two **escape times** — how long a released half takes to climb back to threshold —
and the escape time is set by the tonic input, not by the adaptation constant. **An escape time is
exactly the shape of a term that adds to `3.560·tau_a` without scaling it**, which is what the three
closed lags were asked to supply and could not.

### This axis is not shaped like the three before it

| | the three closed axes | `halfcenter_drive` |
|---|---|---|
| units | milliseconds | dimensionless |
| a lag predicts | coefficient = 3.560 ms/ms | *no such number exists* |
| zero of the field | a real reference (no lag) | a **dead module** (0.54 Hz at drive 0.50) |
| sweepable range | 4× (rft), 11.7× (refr), 3.25× (leak) | **1.52×**, and that is the whole window |
| moves the operating point | a confound to disclose | **it *is* the dose** |

Each row changes the test, and three of them change it in ways that make copying §32 wrong.

### The models, and why v78's pair does not transfer

There is no "a lag requires 3.560" to compare a coefficient against, so the two-model separation
that both §30 and §32 turned on is unavailable. What the textbook supplies instead is a **sign**:

- **model E — the intercept is the escape time.** More drive reaches threshold faster, so the escape
  time *shrinks*: `dIntercept/dDrive < 0`, resolvably.
- **model F — the offset is fixed.** `dIntercept/dDrive = 0`; the 180 ms comes from somewhere the
  drive does not reach.

### The deciding statistic is a SPAN, not a coefficient

§32 could write `coefficient × shipped value` because refractory 0 means no dead time — a real
reference point. **Drive 0 is not "the offset without drive", it is a dead module.** `hcdrive`
measured 0.54 Hz at drive 0.50, nine times below baseline, and alternation is already gone at 0.75.
So `c × 1.40` would be an extrapolation across a boundary the oscillator cannot cross, and it is not
the statistic.

The honest quantity is **how much of the 180 ms the drive moves across the window the oscillator
actually occupies**: `|c| × (highest fitted drive − lowest fitted drive)`.

**And the verdict is run on that span's 95% upper end, not on its point estimate.** This is the
design's one real idea. A span whose upper end is small closes the axis *whether or not the
coefficient is resolved* — an unresolved coefficient with a tight error bar is a **bound**, and a
bound is a result. An unresolved coefficient with a loose error bar correctly closes nothing. The
two cases are told apart by the code and not by prose.

### The threshold: 20%, fixed now

**A field that cannot move 20% of the offset across its entire usable range is not the principal
source of it.** The two closed lags moved 0.3% of it and the third moved it the wrong way, so 20% is
not a near-miss line — it is the floor for a claim worth making. Fixed before the run, and the
`0.20` is a literal in the verdict branch.

### Is this powered? — derived, not assumed, and my first guess was wrong

I first estimated the residual scatter of the per-setting intercepts at ~8 ms, from the size of the
*intercept error bars* (11.0 / 8.2 / 6.3 / 4.3 ms on the refractory axis). **That gives a dead run:**
it puts the bound's upper end at ≥ 45 ms even at `c = 0`, above the 20% line, so the BOUNDED branch
could never fire and every outcome reads "underpowered".

That estimate was the wrong quantity. What the bound depends on is how well the four intercepts lie
**on a line**, not how well each is individually pinned — and v78 measured that directly. Backing it
out of the achieved coefficient:

```
refractory settings {3,12,22,35}:  Sxx = 566,   sqrt = 23.79
v78 achieved  ce = 0.054  =>  residual sd = 0.054 x 23.79 = 1.29 ms
drive settings {1.05,1.20,1.40,1.60}:  Sxx = 0.1719,  sqrt = 0.4146
projected     ce = 1.29 / 0.4146 = 3.1 ms per unit drive
span SE = 3.1 x 0.55 = 1.7 ms        span_hi = 0.55|c| + 7.3 ms   (t = 4.303, 2 dof)
```

The four intercepts lie on a line to ~1.3 ms because they share taus and each period is measured on
12 reps of a 1.2M-tick record. So:

- **the BOUNDED branch fires while |c| < 52 ms per unit**, and
- **model E predicts ≈ −128 ms per unit** if the offset is entirely escape time scaling as 1/*S*
  (`−179.8 / 1.40`).

A **2.5× margin between the prediction and the detection floor**, in a design whose sweepable range
is only 1.52×. Both verdicts are reachable. *The caveat this derivation carries:* it assumes the
intercept-versus-drive relation is near-linear over the window. Curvature inflates the residual on 2
dof and would show up as a loose bound — which the UNDERPOWERED branch will say, rather than my
having to notice it afterwards.

### The vacuity guard: the dose has to land, and the rate cannot tell me

**This is the one way a null here would be worthless rather than informative.** IP defends the
firing rate. v77's four controls all read 4.94 Hz while the duty drifted 0.53 → 0.62 underneath
them — *a regulated quantity cannot detect an operating-point shift.* If the homeostat absorbs a
tonic change there is no dose, and a null measured on no dose is vacuous.

So: **the duty — the quantity nothing defends — must move ≥10% across the settings before this block
reads its own result.** A branch in the code, not a sentence here. 10% is the threshold the
operating-point print already calls a move; it is not a new number chosen for this test.

### The asymmetry, stated before the data

**A null closes this axis cleanly. A positive result only localises.** Drive *is* the operating
point — there is no version of this test in which the tonic moves and the firing rate does not — so a
large effect cannot distinguish *"the drive sets the escape time"* from *"the drive sets the rate and
the rate sets the escape time."* Separating those needs a follow-up that moves one without the other,
and this run cannot be quoted as having done it.

### The verdict branches, as the code's branches

In order, with the first that applies deciding:

1. **< 3 settings fitted, or < 3 taus common** → the axis refuses *itself*. §32's balanced-design
   flaw, kept: intercepts are comparable only on a common set of taus, and dropping the shortest tau
   moved a fitted intercept ~80 ms at 60k.
2. **duty moved < 10%** → **NO VERDICT, the dose was absorbed.** The repair is a dose the regulator
   cannot absorb, not a wider window.
3. **span's 95% upper end < 20% of the anchor intercept** → **DRIVE IS NOT THE PRINCIPAL SOURCE, as
   a bound.** Fires whether or not the coefficient is resolved, and says which.
4. **resolved and negative** → **the escape time is real** — model E on sign *and* size. Reports the
   zero-crossing and that this localises rather than explains.
5. **resolved and positive** → **it reaches the intercept the wrong way.** Model E refused on *sign*,
   which is a different finding from model F.
6. **otherwise** → **UNDERPOWERED, not null**, naming the narrow range as the likeliest cause. The
   repair is more reps or a longer record — not a wider gate, and not a setting outside the window.

**The slope moving is not in that list, and that is deliberate.** In §32 a moving `tau_a` slope
refused the additive-lag form, which was right because the field was a lag candidate. Drive is the
**gain** candidate and v78's own finding is that gain-like axes rotate the law, so a rotation here is
*expected*. It is disclosed with a consequence — if the slope rotates, the intercept is the
extrapolation of a line whose shape itself depends on drive, so intercept and slope stop being
independently interpretable and the verdict says so — but it does not zero the budget.

### The dof-scaled t, applied from here forward

§33 recorded a mis-specification against itself: a flat 3-SE rule on a **four-point** fit has 2 dof,
where the two-tailed 5% critical t is **4.303**, so v78's refractory coefficient at 3.4 SE was
p ≈ 0.08 and the "resolvably non-zero" label overstated it. This block uses `crit_t(n)` — 12.706 at
3 points, 4.303 at 4 — and prints the dof and the critical value beside every call.

**The v78 block is left exactly as it printed.** Retro-fitting the threshold would change a published
verdict after the fact, which is the same sin as widening one; the scaling applies from v79 forward
and the determinism check proves v78's numbers are untouched.

### The hijack audit, written out

This is the failure the v78 build earned the hard way: its appended arms carried `rft 50` and
`drive 1.40`, the only two fields the **v77** block matched on, so they were swept into a *published*
verdict and silently re-decided it — intercept 142.5 where v77 printed 179.8. Every consumer of
`kHCArms`, checked against 15 appended arms at drive 1.05 / 1.20 / 1.60:

| consumer | how it selects | safe? |
|---|---|---|
| v77 offset block | pins `drive == 1.40` | ✔ excluded |
| v78 `offset_axis` | pins `drive == 1.40` | ✔ excluded |
| clock-law block | hardcoded indices 0–3, 12–13 | ✔ immune |
| v76 competition block | hardcoded `kHCBaseArms`, `kV57 = 3` | ✔ immune |
| UPPER EDGE print | hardcoded index 6 | ✔ immune |
| `hc_ctrl_for` | matches drive/rft/leak/refr | ✔ each new arm finds its own control |
| `kHCCtrlArm`, `kHCBaseArms` | constants | ✔ unchanged |
| **the new v79 block** | **pins all eight dose fields** | ✔ by construction |

**And one trap found by doing the audit rather than after it: `drive200` has no control of its own
and falls back to `kHCCtrlArm` by design.** Adding a drive-2.00 control would have re-decided a
published arm's VOID flag through `hc_ctrl_for`. 2.00 is therefore *not* one of the four settings,
and the reason is written at the arm table.

### Two further notes on the build

**The anchor is not `set[0]` on this axis.** The shipped 1.40 sits near the window's *upper* edge, so
a monotone sweep puts it third. v78 could assume `set[0]` was the shipped value because both its
fields ship at their minima; assuming it here would have anchored every contrast on drive 1.05.

**The OLS lambda is hoisted one scope out**, so the v78 and v79 blocks share one definition. A fourth
hand-inlined copy is how two fits come to disagree, and the two blocks' anchor fits are a free
cross-check only while they are the same arithmetic. Identical body, identical call sites, and the
determinism check re-prints v78 byte for byte.

### What this run cannot claim

- **That the offset is explained**, on any outcome. Even model E at full strength localises the
  offset to a quantity that drive and the firing rate both move.
- **Anything about drive outside ~0.9–1.6.** The oscillator does not exist there; `hcdrive` measured
  alternation absent at 0.50, 0.75, 2.10 and 2.80.
- **A mechanism from a rotation.** If the slope moves, this axis has shown the gain reaches the
  dynamics — which v78 already showed on the membrane — and not that it reaches the *offset*.
- **A null if the duty does not move.** That is branch 2, and it is a refusal of the test.

### Where it leaves the remaining candidates

If drive is bounded out, the live list is short and one entry has a scale that matches untuned: the
resonant **jaw** at ~4.5 Hz = **222 ms** ([[the frame is a body part]]), the only candidate whose
natural period is the right order without fitting. Plus the cheap instrument confirmation from §33
(sweep the Schmitt band's 0.25 coefficient; prediction: the intercept does not move).

### What the pre-flight changed — three refutations before any data

**Everything above this heading is the record of the design I started with. The statistic,
the threshold and the settings below supersede it.** I am keeping the original rather than
editing it because what a pre-flight *refuted* is part of the audit trail: two of the three
refutations came from arithmetic I should have done first, and the third came from a 60k
smoke doing exactly what a smoke is for.

#### 1. Drive 1.05 is not in the window at all

The smoke ran the four taus at 1.05, 1.20 and 1.60. At 1.05: anti-phase **+0.000** on three
of four arms, signal sd **0.6–0.7× the control's own noise floor**, and 483–940 "turns" of
61–99 ms inside a 54k record. That is not the intermittency §34 predicted — it is **no
half-centre**. `hcdrive` read 1.05 as alternating because it was looking at an FFT argmax
two DNA versions ago, before the crossing estimator existed.

So the window's lower edge is between 1.05 and 1.20, and the usable span is **1.20–1.60,
a factor of 1.33** — narrower even than the 1.52 §34 budgeted for. 1.05 is not re-run; it
is recorded here as the boundary, the way `hcdrive` recorded 0.50 / 0.75 / 2.10 / 2.80.

#### 2. My vacuity guard was reading a field that cannot act

§34 required the **controls'** duty to move, on the reasoning that IP defends the rate so
the rate cannot detect a dose. The smoke returned all four controls **bit-identical** —
duty 0.47, rate 4.41 Hz, floor sd 3.3240, period 190.0 ms — and the guard dutifully
announced that the homeostat had absorbed the dose.

It had not. `halfcenter_drive` is read **only inside `if (halfcenter_gain > kZero)`**
([network.cpp:674](core/src/network.cpp#L674)), and a control is *defined* by having that
gain at zero. The controls never read the field. My guard could not fire for the reason it
was written for — the same shape as the mis-specified oracle guard in §29.

> **A matched control is the wrong dosimeter whenever the dose acts only through the
> mechanism the control switches off.** The control is dose-blind by construction. It is
> still the right *baseline* — an arm must be gated against its own both-mechanisms-off
> rate — but liveness has to be read on the mechanism arms.

Rewritten to read the mechanism arms' rate at matched `tau_a`. On the re-smoke it fires
correctly: **the dose LANDED, 5.13 → 7.65 Hz at tau 167, +49%.**

#### 3. The drive rotates the law, so the intercept is not an interpretable statistic

This is the one that mattered. §34's whole deciding statistic was `dIntercept/dDrive`, on
the premise that an escape time *adds* to `3.560·tau_a` without scaling it. **The premise is
false on this axis.** The smoke:

| drive | slope on `tau_a` | intercept |
|---|---|---|
| 1.20 | 7.460 | 294.5 |
| 1.30 | 5.286 | 77.9 |
| 1.40 (anchor) | 3.951 | 97.7 |
| 1.60 | 2.912 | 31.9 |

A factor of **2.6 in the gain**, and the raw periods say it without any fit — tau 167 runs
1542 ms at drive 1.20 against 759 at 1.40. So those four "intercepts" are the `tau_a = 0`
extrapolations of lines with entirely different slopes, and differencing them measures the
rotation. The smoke's coefficient came out **−656 ms per unit**: arithmetically real,
mechanistically empty. *A statistic that cannot be interpreted is not rescued by an error
bar.*

**The one form in which a rotating axis can still carry a fixed offset** is the cascaded one
— v77's model A, which that run could only test on a single field:

```
period = A(S) · (tau_a + k)        so    intercept B = A · k
```

The gain `A` moves with the drive by construction. The question is whether the **effective
lag k = B/A** is a constant of the loop. And `k` is in milliseconds, so it is directly
comparable to the **50.5 ms** (`179.8 / 3.560`) the offset has required since §28, and to
the three fields already eliminated against that number.

- **model K — the offset is a fixed effective lag.** `k` is the same at every drive; the
  gain scales it. The 50.5 ms becomes a real quantity with three candidates already gone.
- **model E — the offset is an escape time.** `k` *falls* as drive rises, because a
  harder-driven half climbs back to threshold sooner. Then the 180 ms is not a constant of
  the loop at all but a function of the operating point — and "the one missing time
  constant" was the wrong question for four runs.

**I have seen a hint of the answer and say so.** The smoke's `k` values fall — 39.5 / 14.7 /
24.7 / −23.4 / 11.0 ms across 1.20–1.60 — which is broadly model E's direction, though not
monotonically. Three things keep that from contaminating the test: those are **three-tau**
fits at 60k carrying the known anchor artefact (the anchor reads intercept 97.7 where the
full run gives 179.8 — the *same* 97.7 the v78 smoke produced), the 1.50 fit has R² 0.99167
against 0.9999 elsewhere so it is not even internally consistent, and the decider is a test
with a threshold fixed before the run. **What the smoke changed is which quantity is
interpretable, not where the line sits.**

#### And then the replacement statistic failed its own power check

The obvious decider — contrast `k` at the top of the window against `k` at the bottom — does
not work either, and this time arithmetic caught it rather than a smoke. Propagating v78's
own anchor numbers through `k = B/A`, **including the slope–intercept covariance**:

```
taus {83,167,250,333}:  mx = 208.25,  Sxx = 34695
from A's SE 0.048:  residual s = 8.94 ms   (reproduces B's printed SE of 11.0 exactly)
Cov(A,B) = -mx·s²/Sxx = -0.4798
Var(k) = Var(B)/A² + B²·Var(A)/A⁴ - 2(B/A³)·Cov(A,B) = 9.46 + 0.46 + 3.82 = 13.75
=> ke = 3.71 ms at the anchor,  contrast SE = 5.24 ms
```

The covariance term **adds** 3.82 of the 13.75 rather than cancelling. So:

- model E's predicted drift across this window is `50.5·(1.40/1.20 − 1.40/1.60)` =
  **14.6 ms → only 2.8 SE**, under the 3.182 a 3-dof jackknife requires. **Not separable.**
- and the bound would need `|dk| + 16.7 < 10 ms`, which is **unsatisfiable**.

The contrast is kept and printed because it is the quantity a reader can hold in their head.
It is explicitly labelled as *not* the decider, with this reason beside it.

### The design as built

**The decider is a nested model comparison over every period at once.** Collapsing each
setting to a single ratio throws away precisely the information that makes this answerable —
it turns `nfit × ncom` measured periods into `nfit` numbers.

```
FREE    nfit gains + nfit lags   (2·nfit parameters)
SHARED  nfit gains + ONE lag     (nfit + 1 parameters)      <- model K
F = ((RSS_shared - RSS_free)/(nfit-1)) / (RSS_free/(nfit·ncom - 2·nfit))
```

SHARED is linear in the gains given `k` (`A_s = Σp(t+k) / Σ(t+k)²`), so it is profiled by
scanning `k` and solving the gains in closed form — **bounded below at `−min(tau)`**, where
`A·(tau+k)` changes sign and the fit would start assigning a negative gain to the shortest
tau and still reduce the residual. That is a different and meaningless model, not a smaller
lag.

**Power, simulated before the run** against this axis's own measured gains and v78's own
8.94 ms residual scatter:

| taus per setting | residual dof | critical F | power vs model E | false-positive vs K |
|---|---|---|---|---|
| 4 | 10 | 3.478 | **72%** | 4% |
| **6** | **20** | **2.866** | **95%** | 5% |
| 7 | 25 | 2.759 | 95% | 4% |

**72% is a coin-flip dressed as an experiment**, so two taus were bought at every setting —
`tau 125` and `tau 290`, both safely interior to the usable band (`tau042` came back VOID at
18.3 Hz, `tau500` is outside at the other end). A seventh adds nothing measurable. **89 arms,
12 reps, 1.2M ticks.**

The critical F is a **tabulated** value for every `(nfit, ncom)` the design can reach —
`nfit` 3–5, `ncom` 3–6, giving `dn = nfit−1` and `dd = nfit·(ncom−2)`, twelve cases. A pair
off the grid returns an unreachable `1e9`, so **an unanticipated shape refuses rather than
passing on a threshold guessed at run time.** That is a branch in the code.

The **profile interval** on the shared `k` is the range whose residual stays inside one
critical `t²` of the minimum. It is what makes a constant `k` *usable* rather than merely
unrefuted.

### The verdict branches, as built

First match deciding:

1. **< 3 settings fitted, or < 3 taus common** → the axis refuses itself (§32's
   balanced-design rule: intercepts are comparable only on a common tau set).
2. **the mechanism arms' rate moved < 10%** → **NO VERDICT, the dose did not reach the
   oscillator.** Vacuous.
3. **`(dn, dd)` off the committed grid** → **NO VERDICT, no threshold this run is entitled
   to use.** F is printed and decides nothing.
4. **F < critical** → **model K: the offset is a fixed effective lag.** One shared `k` fits
   all periods as well as `nfit` free ones, across a 33% drive change that rotates the gain
   and that the arms confirm landed. A null with 95% power behind it.
5. **F ≥ critical and `k` falls with drive** → **model E: the offset is an escape time.**
   The 180 ms is not a constant of the loop. Localises; does not explain.
6. **F ≥ critical and `k` rises with drive** → **both models refused.** A harder-driven half
   cannot take *longer* to reach threshold.

**The rotation is not in that list and that is deliberate** — on a gain axis it is the
expected shape, and it is the premise for using `k`. But the code now prints a line that
**checks my own redesign**: if every slope sits within 3 SE of the anchor's, the rotation did
not survive the full record, the premise for demoting the raw intercept was *wrong*, and the
disclosure line should be read as a statistic after all. That refutation of my own reasoning
is printed rather than left for a reader to infer.

### Still carried over from §34 above

The **asymmetry** (a constant `k` is strong; a moving `k` only localises, because drive *is*
the operating point), the **hijack audit** (now against 30 appended arms at drive 1.20 /
1.30 / 1.50 / 1.60 plus two new taus at 1.40 — every consumer still pins or hardcodes, and
`drive200`'s fallback to `kHCCtrlArm` is still why 2.00 is not a setting), the **dof-scaled
t** applied from v79 forward with the v78 block left exactly as printed, the **anchor not
being `set[0]`**, and the **hoisted OLS lambda** with determinism check 7 to prove it a
no-op.

### One thing the smoke raised that this run will answer incidentally

**The law may not be exactly linear in `tau_a`.** The anchor's full-length residual of
8.94 ms sits well above its per-period SEs of 1.9–3.7 ms, and the smoke's three-point fits
show local slopes disagreeing within a setting (at drive 1.50: 4.82 between tau 167 and 250,
3.50 between 250 and 333). Four taus cannot see curvature; **six can.** If it is there it
inflates `RSS_free`, which makes the F-test *conservative* rather than wrong — but it would
qualify the "clock law" framing this line has used since §28, and it is worth watching for.

## §35 — THE DRIVE AXIS REFUSES ITSELF, AND THE DIAGNOSIS SUPERSEDES THE OFFSET'S PUBLISHED VALUE (2026-11-02)

`results/halfcenter-v79.log`, gate `results/verify-v79.log`. 89 arms, 12 reps, 1.2M ticks.
Gate 16m22s, run 3h02m18s. Pre-registered in §34 at commit `6c40df3`, before the run; the
source was untouched between that commit and this log, so the log provably matches the
committed binary.

**Gate: `verify PASS`, hash `ad96f882becbee92` as recorded, pinned hash PASS, 24 experiments
as expected / 0 not. Determinism 7 of 7 IDENTICAL** — the 59 archived rows, the VOID flags,
the regime profile, the published clock law, the v76 k-sweep, the entire v77 block, and the
entire v78 block. Check 7 is the one that was added for this run and it did its job: it
**proves** the OLS lambda hoisted out of the v78 scope to be a no-op on every number and
label v78 published, rather than asserting it.

### THE VERDICT: the axis refuses itself, by the branch §34 wrote for it

Taus live at all five fitted settings = **{167, 250}, two of six**, against the ≥3 the
balanced design requires. The run printed the refusal and declined to produce an effective
lag. Neither model K nor model E is tested.

| drive | live taus | dropped | own fit slope | own intercept | resid rms |
|---|---|---|---|---|---|
| 1.20 | 167 250 290 333 | 083 (−73.1%), 125 (−15.9%) | 4.532 ± 0.521 | 793.4 ± 139.2 | **63.8 ms** |
| 1.30 | 125 167 250 333 | 083 (−23.9%), 290 (+2.0%) | 4.356 ± 0.052 | 292.6 ± 12.1 | 8.30 ms |
| **1.40** | **all six** | — | 3.536 ± 0.038 | 186.7 ± 8.6 | 8.34 ms |
| 1.50 | 167 250 290 333 | 083 (+4.9%), 125 (−5.0%) | 3.546 ± 0.137 | 75.2 ± 36.5 | 16.7 ms |
| 1.60 | 125 167 250 290 | 083 (+10.6%, also under 2× floor), 333 (−3.4%) | 2.534 ± 0.013 | 121.8 ± 2.8 | 1.69 ms |

**`tau083` is live at one setting of five, and the shipped drive is the only setting where
all six taus are clean.** That is not bad luck, and it is the result:

- **At low drive the short taus go intermittent.** `dr120t083` logs 200 turns of a claimed
  1458 ms period in a 1.08M ms record — 27% of the record oscillating. The oscillator is
  near its lower boundary, which the §34 pre-flight had already located between 1.05 and
  1.20 by finding no half-centre at all at 1.05.
- **At high drive the short taus collide with the circuit's own noise band.** `dr160t083`
  runs a 249.2 ms period against the control floor's 164.2 ms — 1.52×, so it trips the
  `UNDER 2x FLOOR` gate. The arm is fast enough to be indistinguishable from the resonance.

**So the drive window and the usable tau window are coupled: the drive is what decides which
taus oscillate cleanly, and the leverage that gives `k` its precision cannot be held fixed
while the drive moves.** The dropped cells are missing for a reason correlated with the
period — the very quantity being measured — so relaxing the balance guard would not rescue
this design, it would fit a model to a period-selected subsample.

### A SECOND, INDEPENDENT REASON THE PRE-REGISTERED DECIDER COULD NOT HAVE FIRED

The residual rms per setting is **63.8 / 8.30 / 0.91 / 16.7 / 1.69 ms — heteroscedastic by a
factor of 70.** The nested F-test pools all of it into one `RSS_free` under a single common
σ. Even a perfectly balanced version of this design would have produced an F with no
calibrated null. §34 derived the power of this test and got the answer right for the error
model it assumed; it never asked whether the error model held across the axis. **Deriving
power is not the same as checking the error model — a power calculation assumes the noise it
is given.**

### THE ROTATION IS CONFIRMED, SO THE REDESIGN PREMISE WAS RIGHT

Threshold-free, no fit needed: across drive 1.20 → 1.60 the period at `tau_a` 167 falls
1587.5 → 543.3 ms (×0.342) while at `tau_a` 333 it falls 2350.4 → 970.0 ms (×0.413). The
short taus are affected more, so the coefficient on `tau_a` falls — 4.532 → 2.534 in the own
fits. The drive is a **gain**, as v78's discriminator predicted, and demoting the raw
intercept in favour of `k = B/A` was the correct call. That is the one pre-registered
expectation this run did meet.

### A DEFECT IN MY OWN REPORTING CODE: ONE FLAG DOING TWO JOBS

`fit_ok[]` meant both *"this setting produced a usable own fit"* and *"this setting may enter
the balanced lag"*. The balance refusal zeroed it, which **silenced three disclosures that do
not depend on balance at all**: the dose-liveness check, the slope-rotation table together
with the printed self-check on my own redesign, and the per-setting `k` contrasts. The
anchor's own fit was perfectly good — six live taus, R² 0.99954 — so the rotation table
should have printed, and the guard instead announced `THE DOSE CANNOT BE CHECKED` about the
single most live dose in the run (periods differ threefold).

This is a **reporting** defect and cannot have moved the verdict: the refusal branch is
upstream of everything it silenced, and the numbers it suppressed are recoverable from the
printed table, which is how the rotation above is quoted. But it is the third guard in three
runs to read the wrong quantity — §29's oracle regime, §34's control-as-dosimeter, and now
this — and all three have the same shape: **a guard that is correct about a condition it is
not actually measuring.** The fix is to split the flag, so that a balanced-design refusal
withdraws only the balanced statistic.

### THE REAL FINDING CAME FROM THE DIAGNOSIS: `tau083` DOES NOT OBEY THE LAW

Buying `tau125` and `tau290` for power at six settings also bought, for free, the first
chance this line has had to ask whether the law is linear *within* a setting. It is not — at
the published four taus — and the culprit is the shortest one.

| taus fitted | slope | intercept | resid rms |
|---|---|---|---|
| {83, 167, 250, 333} — **the published law since §28** | 3.5600 ± 0.0486 | **179.8 ± 11.1 ms** | 9.06 ms |
| {83, 125, 167, 250, 290, 333} — all six | 3.5358 ± 0.0380 | 186.7 ± 8.6 ms | 8.34 ms |
| {125, 167, 250, 290, 333} — `tau083` dropped | **3.4756 ± 0.0053** | **203.1 ± 1.3 ms** | **0.908 ms** |
| {167, 250, 333} — the v78 balanced set | 3.4752 | 203.3 ± 3.4 ms | 1.56 ms |

Dropping one point takes the residual from 9.06 ms to **0.908 ms, a factor of ten**, and
tightens the offset **8.5-fold**. `tau083` sits 23.3 ms below the line the other five define
— **25 times that line's own residual scale.** Adding `tau125` moves the intercept by 0.2 ms,
so the law really is linear on 125–333 and `tau083` is a lone outlier, not a trend.

**The whole ±11 ms uncertainty on the published offset was one arm's misfit.** The offset is
**203.1 ± 1.3 ms** and the effective lag `B/A` is **58.4 ms**, not 179.8 ± 11.0 and 50.5.
(The arithmetic was done with an independent OLS that reproduces the binary's published fit
to the last printed digit — 3.5600 / 179.8 ± 11.1 against 3.560 / 179.8 ± 11.0 — so the
instrument is cross-checked, not merely re-read.)

### AND IT SUPERSEDES ONE OF v78's PUBLISHED COEFFICIENTS — RE-DERIVED, NOT ASSUMED

`aibaby-re-derive-after-an-estimator-fix` says fixing a level does not fix a slope, so both
v78 axes were refitted on the clean range. **`tau083`'s misfit is not constant — it shrinks
monotonically with refractory**, which is exactly the way a bad point poisons a coefficient:

| refractory | clean intercept {167,250,333} | `tau083` resid vs that line |
|---|---|---|
| 3 ms | 203.1 ms | −23.3 ms |
| 12 ms | 196.9 ms | −17.3 ms |
| 22 ms | 194.4 ms | −13.1 ms |
| 35 ms | 194.3 ms | −8.9 ms |

- **v78 published `dIntercept/d(refractory) = +0.182 ± 0.054 ms/ms`. On the clean taus it is
  `−0.258 ± 0.107 ms/ms` — the sign reverses.** The published budget line, "+0.5 ± 0.2 ms
  over the shipped 3.0", becomes −0.8 ms.
- **v78's CLOSURE of `refractory_ms` stands, and is strengthened.** A lag requires the
  coefficient to equal the slope, +3.476; −0.258 ± 0.107 is **34.9 SE** from that, where v78
  claimed 46.4 SE from +3.560. Either number closes it. The budget verdict — 0% of the
  offset accounted — is unchanged, because both candidate values are under 1 ms of 203.
- **v78's `leak_tau_ms` result is fully intact, and for an instructive reason.** Its
  published rotation, slope 3.476 → 3.891, reproduces *exactly* on the clean taus — because
  the consistency gate had already dropped `tau083` at `lk35`, so the leak axis's balanced
  set was `{167,250,333}` all along. **The balance guard protected the axis where it had
  something to do and had nothing to say about the axis where `tau083` survived everywhere.**
  That asymmetry is why the defect went unseen for two runs.
- **A live effect nobody has looked at:** `tau083`'s deviation shrinks 2.6× across the
  refractory axis. The refractory period does nothing to the law's intercept and something
  resolvable to the fastest arm — consistent with v73's finding that the constant only bites
  near the rate ceiling. It is not the offset, but it is the first resolvable effect this
  field has shown in this line.

### THE 202 MS COINCIDENCE, DISCLOSED AND NOT PROMOTED

The controls' firing rate is pinned at **4.91–5.00 Hz on every control of every axis** —
across refractory 3→35 ms (11.7×), leak 20→65 ms (3.25×) and drive 1.20→1.60 — while the
control floor *period* swings 115.8→262.5 ms. The vocal module's `target_rate_hz` is 5.0.
One period of 4.94 Hz is **202.4 ms**, against a measured offset of **203.1 ± 1.3 ms**.

**This is numerology until tested, and two things already argue against the obvious reading.**
The oscillating arms' own rates at the anchor run **10.21 / 7.83 / 6.38 / 4.67 / 4.13 / 3.67
Hz** across the six taus — a 2.8-fold spread — while the offset is a single constant good to
0.91 ms. So **the offset is not an interval of the oscillating pool's own firing.** It could
still be an interval of the *unperturbed* rate, but that is a quantity the oscillating arm
does not possess. And the one free cross-check disfavours a strict 1:1: across the refractory
axis `1/rate` rises 202.4 → 203.7 ms while the clean intercept *falls* 203.1 → 194.3, a
coefficient of −0.258 ± 0.107 against the +0.039 that 1:1 predicts — 2.8 SE, below this
project's 3-SE bar, so suggestive rather than decisive. **Recorded as a coincidence with its
first test already failing, not as a lead.**

### WHAT THIS LEAVES

The offset's three loop time constants are closed (§31, §33) and the drive is now closed as a
*measurable* axis — not for want of power but because the design cannot exist on it. What
changed is the instrument, and that is worth more than the axis was: at **±1.3 ms instead of
±11 ms**, a candidate worth 4 ms is now a 3-SE effect where it used to be invisible. Every
small candidate that was dismissed as unmeasurable deserves re-reading against the new
precision — including the Schmitt-band confirmation v78 demoted, which predicts no intercept
move and can now be held to 1.3 ms. The jaw resonance at ~4.5 Hz = 222 ms remains the only
candidate whose scale matches untuned.

**Before any of that: the published law in this document and in memory reads 3.560 ·
`tau_a` + 179.8 ms. It should read 3.4756 · `tau_a` + 203.1 ms, on `tau_a` ∈ [125, 333],
with `tau_a` = 83 outside the law's range.** That correction lands before the next
experiment, not after it.

## §36 — THE FATIGUE RATE, PRE-REGISTERED AS A COEFFICIENT (2026-11-02)

The offset has now survived all three loop time constants (§31, §33) and the drive (§35). The
kernel says why, and it is not that the right lag has yet to be found.

### THE ARGUMENT FROM THE KERNEL, AND WHERE IT BREAKS

`network.cpp:2569` adds `adapt_jump` to a neuron's fatigue on every spike; `:689` decays it by
`dt/adapt_tau` per tick; `:2545` subtracts it from the drive. So the fatigue difference between
the two halves obeys `dD/dt = jump*R - D/tau_a` with asymptote `D_inf = jump*R*tau_a`, and the
switch at `D = delta` (the inhibition asymmetry `hc_lo - hc_hi`) comes at

    half-period = tau_a * ln[ (D_inf + D0) / (D_inf - delta) ]

Writing `a = D0/(jump*R)` and `b = delta/(jump*R)`, both times, this is
`tau_a * ln[(tau_a + a)/(tau_a - b)]`. **So the offset is a FATIGUE-ACCUMULATION TIME, not a
delay — and no time constant in the loop could ever have carried it.** That is the first account
this line has had of why four lag candidates failed in a row.

**AND THE CLOSED FORM DOES NOT REPRODUCE THE MEASURED LAW, which is stated here rather than
discovered later.** For `tau_a >> a, b` the expression tends to the constant `2(a+b)` — it
SATURATES, where the data is linear in `tau_a` with slope 3.4756 over [125,333] at a 0.908 ms
residual. So the regime is not the one the expansion assumes: `R` is not constant within a phase
(the drive falls as fatigue grows, so the winner slows as it fatigues) and IP moves on its own
timescale. **The mechanism is right in KIND and my closed form is wrong in REGIME, so no number
derived from it is pre-registered as a prediction.** What is pre-registered is the one thing that
does not depend on the regime at all.

### THE PRIMARY, AND IT IS PARAMETER-FREE

**`adapt_jump` is dimensionless.** It is an increment, not a time. **No lag account — the rate
filter, either per-module constant, a conduction delay, the Schmitt band, the jaw resonance —
has any mechanism by which a dimensionless fatigue increment changes an additive delay.** So:

    a LAG account requires      dB/d(jump) = 0
    a FATIGUE account requires  B falls as jump rises

**PRIMARY: `dB/d(jump)` across four settings {1.00, 1.40, 2.00, 2.80}, with an error bar**, `B`
being the intercept of the clean-tau law per setting. Reported as a coefficient and not as a
yes/no, which is §32's lesson. Four settings is an OLS on 4 points = 2 dof, so the 5% critical t
is **4.303**, not 3.0 — §33's dof lesson, applied in advance.

**SECONDARY, printed and explicitly NOT deciding: is `B * (jump * R)` invariant?** This is the
quantitative form of the mechanism, and it is demoted for the regime reason above.

### WHAT THE SMOKE ALREADY CHANGED, BEFORE ANY VERDICT

A 60k pre-flight was read for DESIGN VIABILITY only — which arms oscillate, the duty/rate gates
— and not for any period or fit.

1. **All 18 arms oscillate.** Anti-phase −0.177 to −0.659, all negative; SNR 5.3–11.3; **duty
   0.46–0.47 at every setting, so the KILL SWITCH holds even at 2.8x fatigue.** The decision to
   sweep UPWARD was the right one: raising `jump` raises `D_inf` and makes the switch easier, so
   nothing dropped out. Sweeping down to 0.5 would have reproduced §35's failure by
   construction, pushing `D_inf` under `delta` at short `tau_a`.
2. **THE HOMEOSTAT PARTIALLY COMPENSATES, AND IT WOULD HAVE DEFEATED THE OBVIOUS STATISTIC.**
   The rate FALLS as jump rises — at `tau_a` 83: 10.21 / 8.77 / 6.99 / 5.61 Hz — so `jump*R`
   goes 10.21 / 12.28 / 13.98 / 15.71. **A 2.80x sweep in `jump` buys only 1.53x in `jump*R`.**
   Pre-registering `B*jump = const`, which is what the derivation suggests at a glance, would
   have been testing a statistic the regulator defeats — [[match at the operating point]] in its
   exact documented form. The secondary therefore uses `jump*R`, with `R` read off the mechanism
   arms.

### THE REFUSAL CLAUSES, AS THE CODE'S BRANCHES AND NOT AS PROSE

Written before the run, each one a branch:

| condition | verdict |
|---|---|
| fewer than 3 taus common to all fitted settings | **REFUSED** — unbalanced leverage, as §35 |
| the slope on `tau_a` moves >= 3 SE from the anchor's | **the additive form is REFUSED on this axis** — B is then a `tau_a` = 0 extrapolation of differently-sloped lines, which is what made the drive axis uninterpretable |
| the mechanism arms' rate range < 10% across the sweep | **VACUOUS** — no dose |
| `\|dB/d(jump)\|` < 4.303 SE and the predicted fall is inside the bound | **UNDERPOWERED**, reported as a bound and not as a null |
| `dB/d(jump)` resolvably negative | **THE OFFSET IS A FATIGUE TIME** — every lag account is excluded as its sole source |
| `dB/d(jump)` resolvably positive | **both models refused** — printed, not explained away |

The per-setting residual rms and its ratio are printed beside the coefficient, because §35 met a
factor of 70 and a pooled statistic's null is not calibrated under that. **Reported, not gated:
a refusal on the ratio needs a factor pre-registered, and choosing one now would be choosing a
bar after seeing §35's.**

### POWER, DERIVED BEFORE THE RUN

`Sxx` over jump {1.0, 1.4, 2.0, 2.8} is 1.84. With the anchor's own ±1.3 ms on five clean taus,
a per-point scatter of 5 ms gives `se(slope)` = 3.69 ms per unit jump. The fatigue account's
rough fall — 203 ms to ~132 over the 1.53x in `jump*R` — is about −39 ms per unit jump, so
**10.6 SE against the lag account's zero.** At a scatter of 20 ms it is 2.7 SE and the run
reports a bound instead. **The power rests on B's precision holding at the clean tau range,
which is the one thing §35 bought and the reason this axis is worth running now and was not
before.**

### WHAT IS NOT CLAIMED

This cannot say the offset is *only* fatigue, nor pin `delta`, which is not independently
measured. And the 202.4 ms coincidence of §35 is NOT evidence for this: a fatigue time scales as
`1/(jump*R)` so matching one inter-spike interval is expected rather than surprising, but with
`delta/jump` unpinned the exact agreement remains a coincidence. **If this axis comes back null,
the offset is not a fatigue time either, and the honest position is that none of the quantities
the kernel exposes accounts for it.**

### AMENDMENT, WRITTEN AFTER THE BLOCK SMOKE AND BEFORE THE RUN — FULL DISCLOSURE

A 60k smoke of the finished block was read. **It shows the slope ROTATING on this axis**, which
would fire the refusal clause above:

    jump    1.00     1.40     2.00     2.80
    slope  3.9547   4.0791   4.1379   4.8094      (+3.6, +5.5, +30.5 SE vs the anchor)

**THE CLAUSE IS NOT BEING RELAXED.** It was committed before any data and it stands exactly as
written: if a slope moves >= 3 SE at full record, the run refuses and B is not differenced.
Moving that bar now, having seen which way the data leans, is the one thing
[[verdict fitted to data]] exists to forbid.

**AND I ALSO SAW THE PRIMARY, so it is recorded here rather than discovered in the result:** the
smoke gives `dB/d(jump) = -2.94 +/- 11.01` ms per unit, 0.3 SE, not resolved, R^2 0.035.

**WHY THE RUN GOES AHEAD ANYWAY, and it is not optimism.** A 60k record is known to distort this
law badly and in a documented direction: the anchor's intercept reads **97.2 ms here against the
203.1 the full record gives**, and v78's and v79's smokes produced the same artefact at 97.7 from
the same arms. With only 32–71 turns per arm the fits are not the fits. Worse, the bias is not
uniform — a 60k record hits the SLOWEST oscillators hardest, and `jump` 2.80 has the slowest arms
in the set (900–1700 ms against the anchor's 760–1415). **So the smoke's rotation is exactly what
a short-record bias would manufacture on this axis, and 60k cannot distinguish that from a real
one.** The full record can.

**WHAT THE RUN IS WORTH IF THE CLAUSE DOES FIRE,** which is the honest case to make in advance:

- Full-record periods at four fatigue settings on six taus is permanent data no future design
  has to re-buy, and it settles the rotation question outright.
- A refusal here is a RESULT, not a blank. It would make `adapt_jump` the **third gain-like
  axis** after the membrane (§33) and the drive (§35), and would mean **every quantity touching
  the drive/fatigue balance rotates the law and none of them shifts it.**
- That outcome argues AGAINST the fatigue account this section proposes, not for it: if the
  offset were a fatigue-accumulation time, the fatigue increment is the one thing that should
  move it. **So the clause firing is evidence against my own hypothesis, which is the shape a
  test should have.**

**The balance holds here where it did not on the drive** — four taus common to all four settings
against the drive axis's two — so unlike §35 this axis can at least produce comparable
intercepts. The residual rms ratio is 2x, not 70x. Whatever this run says, it says it on a sound
design.

## §37 — THE FATIGUE RATE ROTATES THE LAW TOO, AND THE LAW TURNS OUT TO BE A LOCAL CHORD (2026-11-02)

`results/halfcenter-v80.log`, 107 arms × 12 reps × 1.2M ticks, 3h33m. Gate `verify PASS`,
hash `ad96f882becbee92` as recorded, 24 experiments. **Determinism 8 of 8 IDENTICAL** — one
check fired and the firing was the check's own extraction scope, proven below before anything
else in this section was written. Pre-registered in §36, commit `0d53335`, before the run.

**The run refused itself at the clause §36 committed for it, and the refusal is the result.**
`adapt_jump` rotates the law. Three settings, three rotations, all one-signed:

| `adapt_jump` | slope on tau_a | intercept B | rms | vs anchor slope |
|---|---|---|---|---|
| **1.00** shipped, ANCHOR | 3.4666 ± 0.0112 | 205.0 ± 2.7 ms | 1.00 ms | — |
| 1.40 | 3.6967 ± 0.0460 | 165.8 ± 11.1 ms | 4.08 ms | **+4.9 SE** |
| 2.00 | 3.8490 ± 0.0298 | 104.0 ± 7.2 ms | 2.64 ms | **+12.0 SE** |
| 2.80 | 4.5897 ± 0.0189 | 110.6 ± 4.6 ms | 1.68 ms | **+51.1 SE** |

Balanced on the three taus live at all four settings, `{167, 250, 290}` — three of six, against
the drive axis's two, so the design §36 bought did hold. The 60k smoke had already shown the
rotation and §36 disclosed it rather than relaxing the clause; **the full record confirms it at
51 SE where the smoke read 30.5.** The one thing the smoke could not settle — whether a
short-record bias was manufacturing it — is settled against me.

### THE PRIMARY IS VOID TWICE OVER, AND THE SECOND REASON IS NOT THE CLAUSE'S

`dB/d(jump) = −53.75 ± 20.65 ms per unit`, R² 0.772. The sign is the fatigue direction, and it
means nothing, for two independent reasons:

1. **The clause.** B is a `tau_a = 0` extrapolation. Differencing the intercepts of lines with
   different gains is arithmetically real and mechanistically empty — the drive axis's exact
   failure, caught this time by a branch written beforehand rather than by a diagnosis after.
2. **It would not have resolved anyway.** 2.6 SE against the dof-scaled 5% critical t of 4.303
   (4 settings → 2 dof). And the raw pattern is not even monotone: 205.0 → 165.8 → 104.0 →
   **110.6**, rising at the top setting.

So the fatigue-time hypothesis §36 derived from the kernel is **not supported**, and §36 said in
advance that this is what the test would mean: *"the clause firing is evidence against my own
hypothesis, which is the shape a test should have."* The derivation's own disclosed defect — the
closed form saturates at large `tau_a` where the data is linear — was the warning, and it was
right.

### THE ROTATION'S SIGN IS THE KERNEL'S, WHICH IS WHY THIS IS A CLOSURE AND NOT A PUZZLE

**The drive rotates the coefficient DOWN (4.53 → 2.53) and the fatigue increment rotates it UP
(3.47 → 4.59).** `core/src/network.cpp:2544-2545` is the whole reason: `drive_a = drive −
adapt_[i]`, one subtraction, so the two axes enter the same quantity with opposite sign. Two
independent sweeps, opposite directions, kernel-predicted — no fitting required to read it.

**And `core/src/network.cpp:2294` says why neither can touch B.** The mutual inhibition is
divided by the pair's own mean, `inv = 1/((m_lo + m_hi)/2)`, and the kernel comment states the
intent: *"Normalised by the group's own mean so the term is scale-free."* So the competition
loop carries **no absolute timescale at all**. The decomposition follows:

- `A · tau_a` is the fatigue relaxation. Its coefficient is set by the drive/fatigue balance,
  with the sign the subtraction demands. Both sweeps are measurements of that one coefficient.
- `B` is what is left at `tau_a = 0` — the switch with no fatigue — and the scale-free loop
  cannot host it.

That is now five axes that fail to move B for one structural reason, not five coincidences:
`rate_fast_tau_ms` (§31), `refractory_ms` and the per-module axonal lag (§33), `leak_tau_ms`
(§33, rotates), `halfcenter_drive` (§35, rotates), `adapt_jump` (here, rotates). **Every
quantity in the drive/fatigue balance rotates the law and none of them shifts it.**

Two candidates were checked against the kernel and killed at the desk, before costing anything:

- **The Schmitt band's additive term is not free.** `kbase = kOne − kw` (`:2344`) — fully
  determined by the within-coupling `kw`, which §-v76's k-sweep already closed. There is no
  independent additive term to sweep.
- **There is no recompute cadence.** The half-centre block runs inside `Network::step()` with no
  sub-sampling, so a fixed update interval — which would have had exactly B's signature — does
  not exist.

### THE DETERMINISM CHECK THAT FIRED WAS ITS OWN SCOPE, AND THAT IS THE FOURTH IN FOUR RUNS

Check 8 printed `*** A NUMBER IN THE v79 BLOCK MOVED -- void.` It had not. The extraction ran
`/v79 -- THE DRIVE AXIS/` → `/UPPER EDGE/`, and **`UPPER EDGE` is a trailing section, not a block
boundary.** v79 was the last block when the check was written, so there was no successor marker
to close it with; appending v80 put the new block *inside* that range, so the archived log's
extraction stopped at v79 and the new log's ran on through v80.

Proven two ways before being believed:

- **Correctly scoped** — closing v79 at `/v80 -- THE FATIGUE RATE/` — both logs yield 41 lines,
  **IDENTICAL**.
- **The original firing's diff has one hunk, `41a42,74`**: a pure append. No `c` and no `d`
  hunk, so no pre-existing line changed or vanished.

Checks 6 and 7 close each block with the *next* version's marker and were immune. **The newest
block is the one with no end delimiter, so it is the one that inherits this trap every time** —
the check is now parameterised on its end marker and re-passes.

### TWO FREE FINDINGS FROM THE AUDIT, AND ONE OF THEM DEMOTES §35

Re-reading the law's own block with the corrected law in hand turned up something §35 should
have caught and did not.

**`tau500` departs the corrected law by +45.8 ms = 13.4 SE.** The out-of-sample arm measures
1986.7 ± 3.1 ms. The superseded law predicts 1959.8 ± 15.2 → **+1.8 SE, HELD**, which is what
the binary prints. The corrected law predicts **1940.9 ± 1.5 → +45.8 ms, 13.4 SE**. Same
measurement, a ruler ten times tighter, and the verdict reverses.

**So the §35 correction voided the law's only out-of-sample test, and §35 did not notice.** The
rule in `aibaby-re-derive-after-an-estimator-fix` was applied to v78's two coefficients and *not*
to the out-of-sample arm. §28's own text — *"On one arm this is SUGGESTIVE, not settled"* — turns
out to have been the operative caveat.

**And a pure power law is refuted by arithmetic, so the offset is not a linearization artefact.**
The obvious reading of a bent law is that there is no offset and the exponent is simply below 1.
Two free parameters either way, so the residuals compare directly:

| taus | line + offset | pure power law |
|---|---|---|
| {125…333} | **0.703 ms** | 8.080 ms (`p` = 0.7733) |
| {125…500} | **9.862 ms** | 31.048 ms (`p` = 0.8143) |
| {83…500} | **9.488 ms** | 35.768 ms (`p` = 0.7935) |

The line wins by 9–11× at every range. **B is real.** (The instrument reproduces the binary's
published fits — 3.5600 / 179.8 / 9.055 and 3.4756 / 203.1 / 0.908 — to the last digit.)

### THE LAW IS A LOCAL CHORD, AND ITS SHAPE IS A FLAT MINIMUM IN THE DERIVATIVE

Consecutive secant slopes at the shipped setting, which need no fit at all:

    tau_a    83 -> 125 -> 167 -> 250 -> 290 -> 333 -> 500
    ms/ms      4.029  3.493  3.453  3.500  3.498  3.746

**Flat to 1.3% across [125,333] and higher at both ends.** So `period = 3.4756·tau_a + 203.1` is
the chord of a broad flat minimum in `d(period)/d(tau_a)`, not a global law — exactly linear
where it was fitted, 0.908 ms over a 724 ms span of period, and bent outside. `B = 203.1` is a
property of that window.

**Both departures sit at a rate boundary, which is a lead and not a result.** `tau083` runs
10.21 Hz — the fastest arm on file, and `aibaby-refractory-sigma-v73` already has the refractory
constant biting only near the rate ceiling, which is why `tau083`'s misfit shrinks 2.6× as the
refractory rises. `tau500` runs 2.52 Hz, approaching the low-rate edge where `jp280t333` at 1.83
Hz went VOID. The window where the law holds spans 3.67–7.83 Hz around the 5.0 Hz setpoint.
**This is post-hoc on seven points and is recorded as a lead**: it predicts that the window is a
rate band rather than a `tau_a` band, which needs the rate moved at fixed `tau_a` to test and
cannot be read off this run, because `adapt_jump` moves the rate and rotates the law at once.

### WHAT IS LEFT, AND WHAT IS NOT CLAIMED

`halfcenter_gain` is the one dose dimension in the arm vector with only an on/off history —
**92 of 92 mechanism arms sit at 0.60 and the 15 controls at 0.00.** It multiplies the
*scale-free* inhibition term, so it is dimensionless in rate units and is the only remaining
quantity in the switch path that is neither a time constant nor a term in the drive/fatigue
subtraction. (`aibaby-hcdrive`'s "DO NOT RE-SWEEP THIS PLANE" is a closure on the *composition*
trade surface, measured on `lesson` and `sep` at gain 0.45 — a different experiment and a
different outcome. The clock law has never been scored against this axis.) `noise_amp` and the
~4.5 Hz resonant jaw remain as before.

Not claimed: that the fatigue account is wrong in kind, only that it is unmeasurable on this
axis and unsupported by this test. Not claimed: any value of `dB/d(jump)`. Not claimed: that the
rate-band reading of the law's window is more than a post-hoc pattern in seven points. Not
claimed: that `B = 203.1` means anything outside `tau_a ∈ [125,333]` — on the contrary, the one
arm outside it now says it does not.

## §38 — THE COMPETITION'S STRENGTH AT A FIXED TONIC, PRE-REGISTERED AS THE LAST FREE DIRECTION (2026-11-02)

§37 closed `adapt_jump` and gave the structural reason five axes had failed: the drive assembly
is one subtraction, and the competition loop is scale-free. This section pre-registers the one
direction in that assembly nobody has moved — and it begins by **refusing the experiment §37
proposed.**

### THE AXIS I NAMED IN §37 DOES NOT EXIST, AND THE KERNEL'S OWN ALGEBRA IS WHY

§37 named `halfcenter_gain` as "the one dose dimension with only an on/off history — 92 of 92
mechanism arms at 0.60." That is true and it is not an axis. Assembling
`core/src/network.cpp:2544-2554`:

    drive_a = drive_module − adapt_[i] − g·(1 ± k·D/S) + hc_drive
            = [drive_module + hc_drive − g] − adapt_[i] ∓ g·k·D/S

Four terms, **two quantities**:

- **net tonic** `T = drive_module + hc_drive − g`
- **asymmetry** `g·k`, the amplitude of the competition itself

So `g` moves `T` and `g·k` **together**, and `T` is `halfcenter_drive` — the axis v79 closed.
Sweeping `g` alone would have confounded the one unmeasured direction with a closed one and
produced a sixth rotation that proved nothing. **The v76 header states the same decomposition
independently** — *"the tonic is g for EVERY k"* — so this was derivable without any run, and
§37's proposal was wrong for a reason §37 itself had already published.

**And `k` is not the vehicle either.** It is the pure asymmetry direction, but v76's sweep ran at
**one tau** (tau333, on an FFT argmax) so it never fitted the law, and `k ≥ 1.5` **clips on
98.7–99.6% of frames** — the clamp binds when `k·|D/S| > 1`, which is the clip detector's own
condition at `host/src/experiments_milestones.cpp:21183`. Above the shipped 1.00 the axis is dead
by saturation; below it, `k = 0` does not alternate at all. The shipped value sits at the top edge
of a window that cannot be widened.

### SO THE DOSE IS A PAIR, AND THE COMPENSATION IS EXACT

Scale the asymmetry by `L` and let `halfcenter_drive` absorb the tonic:

| L | `halfcenter_gain` g | `halfcenter_drive` | asymmetry g·k | net tonic |
|---|---|---|---|---|
| **1.00** shipped, ANCHOR | 0.60 | 1.40 | 0.60 | **+0.80** |
| 0.83 | 0.50 | 1.30 | 0.50 | **+0.80** |
| 0.67 | 0.40 | 1.20 | 0.40 | **+0.80** |
| 0.50 | 0.30 | 1.10 | 0.30 | **+0.80** |
| *2.00, the locked bound* | *1.20* | *2.00* | *1.20* | *+0.80* |

**The compensation is exact in the cycle mean, not merely on average over arms.** A lower-half
neuron always receives `hc_lo = g·clamp(m_hi/mean)`; `mean` is the pair's own mean, so `m_hi/mean`
averages 1 over a cycle by the halves' symmetry, and its time-averaged tonic is exactly `g`. The
clamp cannot break this: it binds only as `m_lo → 0`, where the value is already 2, and at
`k = 1.00` the term `1 + k·D/S` is bounded by 2 because `|D| ≤ S`. **No clipping at any g.**

**And no new controls.** The guard at `:2549` skips the inhibition *and* the `hc_drive` addition
together when `hc_gain == 0`, so a control is blind to **both** swept fields and would be
bit-identical to `ctrl140` — which v79 measured directly as four identical controls. `hc_ctrl_for`
already resolves there. Adding none means no existing arm's control resolution can move, the same
argument §36 used.

**24 arms appended last**, 107 → 131: eighteen for the sweep and six holding the locked upper
bound. Seeds are a function of the replicate only, so nothing is re-rolled, and **all four
existing verdict selectors already pin `hc == 0.60`** — the pins added in `9a0d029` before this
axis was swept, which is the whole reason this append is safe.

### THE SWEEP GOES DOWN, BECAUSE UP LOCKS UP — AND THAT IS A FINDING

The first version of these arms went **up** (g = 0.90 / 1.20 / 1.50). A 60k pre-flight lost the
oscillation on **all eighteen**: periods collapsed to 40–107 ms against a 190 ms control floor at
0.6–1.1× the floor's own sd, and at g ≥ 1.20 every arm returned **`anti +0.000 ± 0.000` on all
twelve seeds** — which is `hc_corr`'s DEGENERATE flag, meaning **one half-series is constant.**

**This is a lock-up, and it follows from the normaliser with no fitting.** Once the loser is
silent, `m_lo = 0` makes `mean = m_hi/2` and therefore `m_hi/mean ≡ 2` *identically*, so
`hc_lo` pins at `2g` and stays there however much the winner fatigues. **The scale-free
normaliser — the very property that keeps the loop from carrying a timescale (§37) — is also
what removes the fatigue's ability to release a fully-suppressed half.** As the asymmetry
deepens the oscillator does not slow down; it stops. The same mechanism explains v76's `k ≥ 1.5`
clipping, which reaches the identical state through `k` instead of `g`.

So **the shipped 0.60 sits near the upper edge of the asymmetry window, exactly as the shipped
`k = 1.00` does**, and down is the only direction. This is the smoke doing the job §34 defined
for it — *choosing where to look, never where the line sits.* No threshold moved and no verdict
was touched; the dose range changed, and six arms at g = 1.20 now carry the boundary on the full
record so a structural claim is not left resting on a short run.

A second pre-flight defect, found the same way and worth naming because it was invisible in the
numbers: the block's **control resolution had no pin of its own.** The control clause must sit
*before* the drive pin, since the drive varies per setting, so the last control passing
rft/leak/refr won and every setting printed `dr160ctrl`. The values were right — v79 measured the
drive controls as bit-identical because `hc_drive` is unread at `hc_gain == 0`, and the smoke
re-confirmed it to four decimals — but **the name was wrong, which is exactly what gets read back
out of a log as a finding two runs later.**

### THE PRIMARY, AND THE CASCADE AS BRANCHES

`dB/d(g·k)` on the balanced design, with the standing clause set now proven over two runs:

| clause | branch |
|---|---|
| ≥3 taus live at all fitted settings | else withdraw **only** the balanced claim; own fits stand |
| slope within 3 SE of the anchor's | else **the additive form is REFUSED** |
| dose read on MECHANISM arms, ≥10% | else VACUOUS |
| dof-scaled critical t (4 settings → 2 dof → **4.303**) | else a BOUND, never a null |
| per-setting residual rms + ratio | reported, **not gated** |
| operating point (duty, rate, net tonic) | **disclosed, not gated** — §32 is why |

That last line is a deliberate choice against my own instinct. The compensation's whole purpose is
to hold the operating point, so a held rate is the design working rather than a dose failing —
and v78 showed a 60k duty/rate drift that vanished at full record, where gating would have
discarded the run's only clean axis. **A held rate also weakens the vacuity test**, so the verdict
code says so in the vacuous branch rather than leaving a reader to notice.

**Power.** `B`'s SE is ±2.7 ms on the anchor's balanced three taus and ±1.3 ms on the clean five.
Taking ±2.5 ms per setting and `Sxx = Σ(g−ḡ)² = 0.05` over the four swept gains gives
`SE(dB/dg) ≈ 11.2` ms per unit, so 4.303 resolves `|dB/dg| > 48` ms per unit — **14.4 ms across
the 0.30 of asymmetry swept, 7% of the 203 ms offset.** The sensitivity is unchanged by the
redirection, because it is set by `B`'s own precision against the *fractional* range, and the
range is 2× either way. This design cannot see a 1% contribution and says so in advance.

### EITHER OUTCOME IS A RESULT, AND THEY POINT OPPOSITE WAYS

- **Rotates** → the tonic (v79), the fatigue (v80) and the asymmetry all rotate the law and none
  shifts it. **`B` is not a quantity in this assembly at all**, and the search moves outside the
  vocal module's own drive.
- **Slope holds and `B` moves** → the first axis ever to shift it, with the net tonic fixed by
  construction so it is not the drive axis in disguise.
- **Slope holds and `B` does not move resolvably** → a bound, and still the first time the
  additive form has survived an axis in this assembly.

### FOUR THINGS THE PRE-FLIGHT SETTLED FOR FREE

1. **The instrument adds no constant.** The crossing estimator is a Schmitt trigger with a
   `±0.25·sd` hysteresis band and **no dead time and no minimum interval**
   (`experiments_milestones.cpp:21245-21258`). A threshold delays every crossing equally and
   leaves the up-to-up spacing alone, so it cannot manufacture an offset. The whole line is not
   chasing a measurement artefact.
2. **`B` survives zeroing every constant it could decompose into.** `dB/d(leak_tau_ms)` extrapolates
   to **+206.6 ± 9.0 ms at leak = 0**; the corrected refractory coefficient extrapolates to ~204 ms
   at refractory = 0; the law itself reads **203.1 ± 1.3**. An additive transition-time account
   needs `dB/d(leak) ≈ +1` ms/ms and measures **−0.433**, wrong sign and wrong size — so `B` does
   not even partially decompose into the membrane or the dead time.
3. **Noise is refuted by reproducibility, before being swept.** A noise-driven escape cannot give
   periods repeatable to ±0.2% across twelve independent seeds (468.3 ± 1.7, 637.5 ± 1.3,
   784.2 ± 1.5). `noise_amp` is also confounded — it scales a DC compensation term as well as the
   stochastic one (`network.cpp:2518`) — so it was never the clean axis the memory index implied.
4. **There IS one live, unaudited term in the vocal drive, and it is deferred, not dismissed.**
   `rebound_source = 1`, `rebound_gain = 0.02`: the ear drives the vocal drive, `self_gain = 0.5`,
   so the creature hears its own babble and a closed loop runs vocal → larynx → cochlea → auditory
   → drive. v78 patched three constants on the **vocal** module; the auditory module's were never
   touched, so this loop sits outside everything closed so far. Deferred because it needs a new
   `HCArm` field and therefore an explicit value on all 131 existing arms — a change worth making
   on its own rather than riding along with a sweep. Arithmetic says it is worth ~0–25 ms, which is
   only interesting because ±1.3 ms now resolves it.

Not claimed: that the asymmetry is more likely to shift `B` than to rotate it — the mechanism
argument, that a larger escape threshold costs time proportional to `tau_a`, predicts a rotation,
and this is pre-registered as a test I expect to lose.

### §38 AMENDMENT — THE SECOND SMOKE FIXED MY DOSIMETER AND SHOWED ME THE ANSWER

**The downward arms oscillate.** Three taus live at all four swept settings (`{250, 290, 333}`),
and the balance should improve at full record rather than degrade: every drop at 60k is an `UNDER
2x FLOOR` or `INCONSISTENT` against a 190.0 ms short-record floor that falls to 164.2 ms at full
length, lowering the bar from 380 to 328 ms.

**The compensation works, measurably.** At tau 250 across the whole sweep: duty 0.47 → 0.46, rate
4.62 → 4.23 Hz, net tonic +0.80 at every setting. A 2× change in the asymmetry moves the operating
rate by 9%. That is the (tonic, asymmetry) coordinate doing exactly what the algebra said it would.

### AND THAT IS WHY THE DOSE GATE WAS WRONG — THE FOURTH GUARD IN FIVE RUNS

The pre-registered liveness gate read the **firing rate** at a matched tau, copied from v79 and
v80 where the dose arrives as a drive change and the homeostat's failure to absorb it *is* the
dose landing. **This axis holds the rate on purpose.** So the smoke printed

    THE DOSE WAS ABSORBED ... Under 10%, so any null is VACUOUS

over a sweep whose slopes had moved **52 SE** — and because `!dose_live` sits first in the
cascade, that would have been the published verdict. A held rate is the design working, not the
dose failing.

**Fixed before the run, and the fix is a quantity the dose actually moves.** `anti` — the
correlation between the two half-rate series, the competition's own depth, pre-registered by v76
as the half-centre's signature. It is not circular: it is computed from the raw series and shares
no code with the crossing estimator or with the fit. The smoke moves it **−0.299 → −0.353 →
−0.405 → −0.397** at tau 333 with SEs of 0.001–0.015, and pins it at exactly **0.000** at the
locked setting — so one statistic reads both the dose and the bound. The rate range is still
printed, now as the compensation's own check rather than as the gate.

This is the fourth guard in five runs to be confidently correct about a condition it was not
measuring, and **the first one caught before the run instead of after.** The pattern is now
explicit: *a liveness guard inherited from another axis measures that axis's dose, not this one's.*

### WHAT THE SMOKE SHOWED, AND WHY IT IS NOT THE RESULT

The slopes rotate, hard and monotonically: **4.01 → 3.13 → 2.43 → 1.76**, i.e. −21.2, −36.6,
−52.3 SE. A 2× change in the asymmetry changes the coefficient on `tau_a` by **2.3×** — the
largest rotation any axis in this loop has produced.

**Three reasons that is not quotable and the run goes ahead anyway.** The anchor's own 60k fit is
4.0441 / 71.5 ms against 3.4666 / 205.0 at full record, so this record length distorts the law on
the very arms the comparison rests on. §37 established the distortion is *not* common-mode — it
hits the slowest arms hardest — and these settings have very different period distributions (1415
vs 621 ms at tau 333), so the rotation's magnitude at 60k is exactly what a non-common-mode bias
would manufacture. And the balanced set is a bare 3 taus here against the 5–6 expected at full
length. **What a smoke changes is which quantity is interpretable, not where the line sits** — the
clause is untouched, and I expect it to fire.

If it does, the reading is the one §38 pre-registered: the tonic (v79), the fatigue (v80) and the
asymmetry all rotate the law, none shifts it, **and `B` is not a quantity in the vocal module's
drive assembly at all.** Together with the lock-up bound above — the asymmetry cannot be raised,
and the shipped value sits near the window's upper edge — that closes the (tonic, asymmetry) plane
in both directions. The remaining candidates are then all outside this assembly: the self-audition
rebound loop through the auditory module, and the plant.

## §39 — THE ASYMMETRY ROTATES THE LAW, AND THE GATE THAT REFUSED TO SAY SO WAS READING A CORRELATION (2026-11-02)

The run §38 pre-registered landed: 131 arms, 12 replicates, 1.2M ticks, 4h29m. Its own verdict
line reads **`NO VERDICT — THE DOSE DID NOT REACH THE COMPETITION … VACUOUS`**, printed over
slopes that had moved 60 to 170 SE and a `dB/d(g·k)` resolved at 27 SE. That sentence is
self-contradictory, and this section is mostly about why it got printed.

### WHAT REPRODUCED

Gate clean on all four keys (`ad96f882becbee92`). Eight of nine determinism checks identical,
including the whole v77, v78, v79 and v80 blocks — so appending 24 arms and a new analysis block
moved nothing that had been published.

**Check 5 fired and had not failed.** Its diff is a single hunk, **`0a1,109`** — a pure prepend,
no `c` and no `d`, which is arithmetically incapable of being a determinism failure. The cause is
**v80's check-8 defect at the other end**: §37 fixed every unanchored *end* marker and left the
*start* markers alone. `/THE COMPETITION.S STRENGTH/` matches **v81's own heading** as well as
v76's, and v81 prints *before* v76, so the range swallowed the new block. Re-scoped to
`/v76 -- THE COMPETITION.S STRENGTH/`: **20 lines, identical, on both logs.** The rule is now
stated once and generally — **anchor both ends of a block range to a `vNN --` marker**, never to
prose, at either end.

**And a free, independent confirmation.** v81's anchor setting re-fits the same six archived arms
and prints `slope 3.5359 ± 0.0378, intercept 186.7 ± 8.6, R² 0.99954` — **byte-identical to v80's
print of the same arms.** Its balanced refit on the four common taus gives **3.4758 ± 0.0090 /
203.1 ± 2.4**, reproducing the published law `3.4756·tau_a + 203.1` from a different tau subset
and a different code path.

### THE MEASUREMENT

Four settings of the asymmetry `g·k`, with `halfcenter_drive` holding the net tonic at **+0.80**
at every one, on the four taus live everywhere (167, 250, 290, 333):

| `g·k` | `halfcenter_drive` | slope on tau_a | vs anchor | intercept `B` |
|---|---|---|---|---|
| **0.60** anchor | 1.40 | 3.4758 ± 0.0090 | — | 203.1 ± 2.4 ms |
| 0.50 | 1.30 | 2.8051 ± 0.0065 | **−60.5 SE** | 164.4 ± 1.7 ms |
| 0.40 | 1.20 | 2.2175 ± 0.0085 | **−101.6 SE** | 114.9 ± 2.3 ms |
| 0.30 | 1.10 | 1.5988 ± 0.0064 | **−170.0 SE** | 67.2 ± 1.7 ms |
| *1.20* | *2.00* | *locked* | — | *0 live arms* |

Per-setting residual rms 0.79–1.10 ms — **×1.4, the first homogeneous error model this line has
produced** (§35 met ×70). The six arms at `g·k = 1.20` returned `anti +0.000 ± 0.000` on all
twelve seeds at every tau, with periods of 40–76 ms at 0.6× the control floor's own sd: **the
lock-up §38 derived from the normaliser, confirmed on the full record.**

This is the strongest lever on the period the line has found. At tau 333 the period runs
**1361 → 600 ms** across a 2× change in the asymmetry.

### THE PRINTED VERDICT WAS WRONG IN TWO INDEPENDENT WAYS, AND BOTH ARE STRUCTURAL

**(1) The cascade's order was pre-registered twice, in two places that disagreed.** §38's clause
table lists slope-invariance *above* the dose clause; the committed code put the dose clause
first. **Order is part of a specification**, and it had never been testable — v79 and v80 both
ended in refusals where the two clauses agreed. v81 is the first run in which they disagree.

**(2) A vacuity gate cannot guard a rejection, and that is a logical fact rather than a numerical
one.** A vacuity clause's entire content is *if the independent variable did not perturb the
mechanism, then a **failure** to see an effect is uninformative.* It is a precondition for
trusting a **null or a bound**. It has no bearing on a **rejection**: an outcome that moved by
170 SE is itself proof the dose arrived, whatever a dosimeter says. Putting the gate first let it
void exactly the case it was never meant to address — and the sentence it printed asserts, in one
breath, that the dose never arrived and that it changed the outcome by tens of SE.

This was derivable at the desk and is independent of which way the data went: re-ordering changes
nothing in any run where the clauses agree, which is every run so far.

### AND THE DOSIMETER WAS A CORRELATION — THE FIFTH GUARD IN SIX RUNS

§38 caught the rate dosimeter before the run and replaced it with `anti`. **The replacement was
wrong for a new reason.** `anti` is `corr(lower-half rate, upper-half rate)` — a **correlation**,
hence **scale-free by construction**, hence incapable of metering an amplitude. It sits near
−0.55 for *any* pair that genuinely alternates and collapses to 0.000 only at lock-up, which
makes it a **liveness indicator** — the one thing [[aibaby-liveness-is-not-dose]] forbids as a
dose. At full record it ran **0.543–0.559 across the whole 2× sweep: a 3% range.**

The 60k smoke made it look graded (−0.299 → −0.397) only because the oscillation was marginal
there, so part of what it was reading was *whether the creature oscillated at all*. **So the fix
that caught the fourth guard installed the fifth**, and the general form is now:

> **A NORMALISED STATISTIC CANNOT METER A DOSE.** Before using a quantity as a dosimeter, ask
> what it is divided by. A correlation, a duty cycle, a ratio to a control and an R² are all
> blind to the magnitude of the thing they describe.

### THE RIGHT DOSIMETER WAS PRINTED UNDER ALL 131 ARMS AND NEVER READ

Three unnormalised quantities were already in the log, one of them under every single arm:

| quantity | range at tau 167 | as a gate |
|---|---|---|
| rate | 5.98–6.38 Hz (**7%**) | held **by design** — meters the compensation |
| \|anti\| | 0.543–0.559 (**3%**) | a **correlation** — blind to amplitude |
| produced F1 5–95% swing | 75.6 → 28.0 Hz (**170%**) | past the 800 ms pole, so **inflated** by the shorter period |
| **applied inhibition swing `g·k·sd(D)`** | **≈2.93–7.16 (144%)** | **unnormalised, the kernel's own term** |

`hc_lo = g·(1 + k·D/S)` at `network.cpp:2344`, so the swing of the term the kernel actually
applies is `g·k·sd(D)/S`, with `S` the pair mean that the homeostat holds. `m_ds[]` is `sd(D)` in
the estimator's own rate units — **unnormalised, and an amplitude where the outcome is a
timing.** It is falsifiable rather than tautological: were `sd(D)` to scale as `1/g` the applied
swing would be flat and the dose genuinely absorbed. It does not — it falls *with* `g`
(3.3 → 2.7× the control floor), so the two compound. This is the same shape as
[[aibaby-null-is-not-a-control]]: the quantity that settled it was already in the arrays.

### THE READING, BY §38's TABLE ORDER

The slope clause fires, at a threshold pre-registered identically in both documents:
**the additive form is REFUSED on this axis.** The asymmetry rotates the law, as the tonic (v79)
and the fatigue (v80) did. With the lock-up bound — the asymmetry cannot be raised, and the
shipped 0.60 sits near the window's upper edge — **that closes the (tonic, asymmetry) plane in
both directions, and `B` is not a quantity in the vocal module's drive assembly at all.** Exactly
the outcome §38 pre-registered and said it expected to lose.

### `dB/d(g·k) = +457 ms PER UNIT` IS NOT A SHIFT, AND THE PIVOT IS WHY

With the slope refused, the intercept is **the slope's shadow**. A family of lines pivoting about
a common point `(tau*, P*)` moves its intercept by exactly `−tau*` per unit of slope with no
additive term anywhere, so regressing `B` on `A` recovers the pivot directly:

    dB/dA = +73.4 ms per unit of slope   ->   tau* = -73 ms   (R^2 0.994)

**The pivot sits 73 ms to the *left* of the origin**, where no adaptation constant can live: the
four lines fan out from a virtual point rather than crossing inside the swept range. Nor is it a
pure scaling — `B/A` runs 58.4, 58.6, 51.8, 42.0 and is not invariant. Both coefficients move and
neither is separable from the other on this axis. **Do not quote +457 ms per unit as "the
asymmetry shifts `B`".** The verdict code now prints the pivot immediately under the refusal, so
the log itself declines the misreading.

### WHAT §39 CHANGES, AND THE RE-RUN THAT PRINTS IT

A verdict argued for in prose is precisely what *"write refusal clauses as the code's branches"*
exists to prevent, so both fixes are in the verdict code and the same 131 arms run again:

1. **The cascade is re-ordered** — rotation, then a resolved shift, then the dose gate, then the
   bound. The dose clause now guards **only** the branch whose interpretation depends on it.
2. **The dosimeter is the applied inhibition swing** `g·k·sd(D)`, with the rate, `|anti|` and the
   produced F1 swing all **disclosed beside it**, each labelled with the reason it is not the
   gate. Three refused dosimeters in three runs have earned their own line in the log.
3. **The pivot is computed and printed** inside the refusal branch.

**Nothing else changes: no arm, no seed, no genome field.** That is what makes the re-run
readable rather than a second look at a result that did not suit — **check 10 compares the v81
block's measured lines and every one of them must come back bit-identical**, while the prose
changes by intent. If a single period, fit or coefficient moves, the run is void and the fix was
not confined to the verdict. The threshold stays at 10%; the quantity changed for a reason that
holds whichever way the data had gone, and at 144% it would pass any threshold this project has
used, so this is not threshold-shopping.

Pre-registered and committed before the run, as every refusal clause on this line has been.

### §39 ADDENDUM — THE SMOKE CONFIRMED THE DIAGNOSIS FROM THE OTHER SIDE

The 60k pre-flight on the corrected code exercises all four new paths — the applied-swing
dosimeter, the three disclosed refusals, the re-ordered cascade reaching
`THE ADDITIVE FORM IS REFUSED`, and the pivot print. It also measured the artefact §39 blames:

| | 60k smoke | full record |
|---|---|---|
| `\|anti\|` range across the sweep | **28%** | **3%** |
| applied inhibition swing | 128% | ≈144% |

**`anti` looked like a dose meter at 60k and stopped being one at full length**, which is exactly
the mechanism §39 names: at 60k part of what it read was *whether the creature oscillated at all*,
and at 1.2M ticks every fitted setting oscillates cleanly, so the correlation saturates. The
quantity a smoke validates a guard against must be measured **in the regime the run will use** —
a liveness indicator impersonates a dose meter precisely where liveness is marginal.

### §39 CONFIRMED — THE RE-RUN CHANGED THE VERDICT AND MOVED NOT ONE NUMBER

`results/halfcenter-v81b.log`, the same 131 arms at the 1.2M minimum, 18:42 → 23:25 (**4h43m**)
behind a clean gate (all four keys PASS, hash `ad96f882becbee92`, 24 experiments).

**All ten determinism checks came back IDENTICAL**, including the one the re-run exists for:

| check | scope | result |
|---|---|---|
| 1 | all 131 main-table rows, the 24 v81 arms included | IDENTICAL |
| 2 | the kill-switch / VOID lines | IDENTICAL |
| 3 | the regime profile, 131 rows | IDENTICAL |
| 4 | the published linear clock law | IDENTICAL |
| 5 | the v76 k sweep, **start marker now anchored** | IDENTICAL |
| 6–9 | the entire v77, v78, v79, v80 blocks | IDENTICAL |
| **10** | **the v81 block's 54 measured lines** | **IDENTICAL** |

Check 5 passing is the second proof that v81's fire was the extraction and not the data, now from
a run rather than from a re-scoped `sed` on an archive. And the free cross-check closes it: the
anchor's own six-tau fit prints `slope 3.5359 +/- 0.0378  intercept 186.7 +/- 8.6  R^2 0.99954`,
**byte-identical to v80's print of the same arms** through a different block of code.

So the cascade changed, the measurement did not, and the verdict below is the same data read
through a gate that meters the dose it claims to meter.

### THE DOSE LANDED, ON THE QUANTITY THE KERNEL ACTUALLY APPLIES

At the reference tau the applied inhibition swing `g·k·sd(D)` runs **2.96–7.16 rate units, a 142%
range** across the 2.00× sweep — against the 10% threshold pre-registered before the run. The
three refused dosimeters now print beside it, each with the reason it is not the gate:

| quantity | range over the same sweep | why it is not the gate |
|---|---|---|
| **`g·k·sd(D)`** | **142%** | **the inhibition the kernel applies, unnormalised** |
| rate | 7% | held by design here, so it meters the compensation |
| `\|anti\|` | 3% | a correlation, scale-free, blind to amplitude |
| produced F1 swing | 170% | past the 800 ms pole, inflated by the shorter period |

`sd(D)` falls *with* `g`, so the applied swing was falsifiable rather than tautological: had it
scaled as `1/g` the swing would have been flat and the dose genuinely absorbed. It did not.

### THE VERDICT, IN THE BRANCH THE CLAUSE TABLE PRE-REGISTERED

The re-ordered cascade reaches the rotation clause first, and it fires hard:

| `g·k` | slope `A` | vs anchor | intercept `B` |
|---|---|---|---|
| 0.60 (anchor) | 3.4758 ± 0.0090 | — | 203.1 ± 2.4 ms |
| 0.50 | 2.8051 ± 0.0065 | **−60.5 SE** | 164.4 ± 1.7 ms |
| 0.40 | 2.2175 ± 0.0085 | **−101.6 SE** | 114.9 ± 2.3 ms |
| 0.30 | 1.5988 ± 0.0064 | **−170.0 SE** | 67.2 ± 1.7 ms |

`dB/d(g·k) = +457.06 ± 16.92 ms per unit` — **27.0 SE, resolved** against the 2-dof critical t of
4.303, R² 0.99727, per-setting residual rms 0.79–1.10 ms (a 1.4× spread, disclosed and not
gated). And it is **not a shift**: the pivot regression prints `dB/dA = +73.4 ± 4.1 ms per unit of
slope`, R² 0.99391, so **τ\* = −73.4 ms** — 73 ms to the *left* of the origin, where no adaptation
constant can sit. The lines fan out from a virtual point; `B` is a `tau_a = 0` extrapolation of
lines with different gains, and the log says so one line under the refusal.

**THE ADDITIVE FORM IS REFUSED ON THIS AXIS, AND THAT CLOSES THE DRIVE ASSEMBLY.** The tonic
(§35/v79), the fatigue rate (§37/v80) and the asymmetry (§39/v81) all **rotate** the law and not
one of them **shifts** it. `B` is not a quantity in the vocal module's own drive assembly, and the
search belongs outside it.

### AND THE UPPER BOUND HOLDS ON THE FULL RECORD

The six `g·k = 1.20` arms print `ONLY 0 LIVE ARMS -- this setting is not fitted`: periods of
40–76 ms against taus of 83–333 ms, every one `INCONSISTENT  UNDER 2x FLOOR`, with turn-to-turn
sd at **0.6× the control floor's own** — i.e. below the noise of a creature that is not
oscillating at all. The 60k pre-flight's lock-up reproduces at twenty times the record, so the
`(tonic, asymmetry)` plane is closed from above by measurement and not only by the normaliser
arithmetic: once `m_lo = 0`, `m_hi/mean ≡ 2` identically, `hc_lo` pins at `2g`, and no amount of
fatigue can release the loser. **Do not sweep the asymmetry upward again.**

## §40 — THE SELF-AUDITION LOOP, THE FIRST AXIS OUTSIDE THE DRIVE ASSEMBLY (2026-11-02)

**Pre-registered before the run, clause order included.** v79, v80 and v81 closed the vocal
module's drive assembly: the net tonic, the fatigue rate and the competition's asymmetry all
**rotate** the clock law `period = 3.4756·tau_a + 203.1 ms` and not one of them **shifts** it, so
`B` is not a quantity in that assembly. What is left inside the creature is the loop that runs
*out* of it.

### THE LOOP HAS BEEN CLOSED IN EVERY ARM EVER RUN, AND NOBODY HAD LOOKED AT IT

Verified at five lines, not inferred:

| what | where |
|---|---|
| the module carrying the rebound is `vocal` | `dna/default.toml:1870` |
| `rebound_source = 1`, `rebound_gain = 0.02`, `rebound_mean_tau_ms = 5000` | `:1935-1937` |
| module **1 is `auditory`** — so the ear drives the vocal drive | `:1472` |
| `drop = mean_rate[src] − pool_fast[src]`, rectified, own 5 s mean subtracted | `network.cpp:2363-2381` |
| `rebound` is a term of **`drive_module`** | `network.cpp:2520` |
| the ear mixes the creature's own larynx at `self_gain = 0.5`, through a **sample-accurate ring-buffer delay** of `self_delay_ms` | `host/src/audio.cpp:52-56, 89-104` |

And this experiment's caregiver renders **silence** on every tick, so the auditory module's entire
input is the creature's own babble — at the 57% duty the log reports. `vocal → larynx → cochlea →
auditory → vocal drive` has been a closed loop in all 131 arms of every run on this line.

### WHY §37's CLOSURE ARGUMENT DOES NOT REACH IT

§37 closed five axes with one observation: the drive assembles as a **single subtraction**, so a
**DC** contribution to the net tonic rotates the law and cannot shift it. The rebound is not DC
**and is designed not to be** — the genome's own comment says the slow mean is subtracted *"or it
is a tonic lift"*. With `rebound_mean_tau_ms = 5000` against periods of 0.47–1.36 s, `use = raw −
mean(raw)` is zero-mean over 3.5–10 cycles: its DC contribution is ~0 by construction and its
whole contribution is **within-cycle and phase-locked**.

That is the one functional form this line has never tested, and the only one that can host `B`:
**a kick whose arrival time is set by a transport latency rather than by `tau_a`.**

### WHAT v77 ALREADY CONSTRAINS — STATED AGAINST THIS TEST, NOT FOR IT

v77 swept `rate_fast_tau_ms` and refused it. That constant is **shared**, and `pool_fast_[src]` is
one of the two terms in the rebound's own `drop` — so v77 already moved the loop's **filter leg**
and saw no shift. **The prior is against this axis.** It also says where to aim: the untested legs
are the **gain** and the **transport delay**.

### THE TWO DOSES, AND THE FOUR CONTROLS

28 new arms, 131 → **159**.

**(1) THE LOOP-OFF BOUND — 6 arms.** `rebound_gain` 0.02 → **0.00** at the six taus. A bound and
not a slope: it removes the term rather than scaling it, so it measures the **entire** contribution
the loop can make. It is also **self-dosimetering**, which no axis here has been — the dose is a
structural switch, so bit-identical rows would prove the term is exactly zero on every tick and
close the candidate with no appeal to a vacuity gate.

**(2) THE LATENCY TEST — 18 arms.** `self_delay_ms` **25 / 50 / 100** at the six taus; the 0 ms
setting *is* the anchor and is not re-run.

**(3) AND FOUR CONTROLS OF THEIR OWN.** Every dose since v77 carries per-setting controls, and
here that pin does real work for the first time. On the v79 and v81 axes the controls were
dose-*blind* (`halfcenter_drive` is unread when `halfcenter_gain == 0`), so every setting resolved
to the same arm and only its *name* was at stake. **This dose reaches the control:** a creature
with the half-centre off still babbles and still hears itself, so its rebound and its delay are
live and its floor is its own. `ctrl140` would have been the wrong baseline for all 24 arms.

### THE PREDICTION HAS NO FREE SCALE

If `B` is the loop's transport time, adding transport **adds** to it: **`dB/d(self_delay) = +1.0
ms/ms`, exactly.** This is the discriminator shape that refuted the membrane in v78 — an additive
transition-time account needed `dB/d(leak) ≈ +1` and measured **−0.433**, wrong sign and wrong
size.

**Power.** `B`'s SE is ±2.4 ms per setting on the balanced design. Over delays {0, 25, 50, 100}
(mean 43.75), `Sxx = 5469`, so `SE(dB/d(delay)) = 2.4/√5469 = 0.032 ms/ms`. At 2 dof the critical
t is 4.303, so the run **resolves `|dB/d(delay)| > 0.14 ms/ms` — seven times finer than the
prediction it is testing.** This is the first axis on this line with that much headroom over its
own hypothesis, and it is what makes a **null informative**: a bound that excludes +1.0 is a
rejection, not a shrug.

### THE DOSIMETER, AND WHY THIS AXIS NEEDS A DIFFERENT KIND OF GATE

**A transport delay cannot be absorbed.** `self_delay_ms` is applied by a ring buffer of
`round(delay·sample_rate/1000)` samples in the **ear**, outside the brain, with no adaptive element
anywhere between the genome field and the delay line. A drive or a gain can be compensated by the
homeostat — which is exactly what made v79's and v81's dose gates hard — but nothing in this
creature can give the delayed samples back. **So a vacuity gate on the delay would guard a failure
mode that cannot occur**, and asserting one would be a guard measuring a condition it does not
check for the sixth time in seven runs.

**What genuinely can be vacuous is the loop itself**, and that is what is gated. The dosimeter is
`sd(rebound)` — the term's own within-cycle swing, **recorded by the kernel at the point it is
applied** and reported in the drive's own rate units, unnormalised. Note what is *not* used: the
amplitude of the delayed signal, which a pure delay leaves unchanged **by construction** and which
would therefore read as *"the dose did not land"* on every arm however well it landed. That is the
§34 error in a new dress, and it was refused at the desk rather than after a run.

**The threshold is 1% of the net tonic of +0.80**, i.e. 0.008 rate units. A ratio is legitimate
here and the distinction matters: the tonic is a **fixed constant of this design**, held at +0.80
by construction in every arm of the block, so dividing by it is a *unit conversion* and not a
normalisation that can absorb the dose — which is precisely what went wrong in §39. The reasoning
behind the number holds either way: a term contributing under 1% of the drive cannot plausibly
carry a 203 ms intercept when the drive's **own** 2× changes rotate the law rather than shift it.

**And the DC level is itself a pre-registered check, not a decoration.** The v45 mean-subtraction
exists so this term is *not* a tonic lift, and §37 closed every DC axis in this assembly. If
`|rebound_mean|` comes back comparable to the swing, the term is partly a tonic after all and that
much of it is **already refuted** — so the log prints both and says which case it is in.

### THE CASCADE, IN ORDER, WITH THE ORDER JUSTIFIED

First match decides. The order is §39's rule applied from the start rather than after a run: a
vacuity clause is a precondition for trusting a **null** and has no bearing on a **rejection**.

| # | clause | verdict |
|---|---|---|
| 1 | fewer than 3 delay settings fitted | NO VERDICT; the loop-off bound stands alone |
| 2 | a slope moved ≥3 SE from the anchor's | THE ADDITIVE FORM IS REFUSED — plus the pivot |
| 3 | `dB/d(delay)` resolved **and** within 3 SE of +1.0 | **THE OFFSET IS THE LOOP'S TRANSPORT LATENCY** |
| 4 | `dB/d(delay)` resolved but far from +1.0 | B moves with the delay but **not as a latency** |
| 5 | the loop is inert (`sd(rebound)` < 1% of tonic) | NO VERDICT — VACUOUS |
| 6 | 95% upper limit on \|`dB/d(delay)`\| **< 1.0** | **THE LATENCY ACCOUNT IS REFUTED** by a bound |
| 7 | otherwise | UNDERPOWERED, reported as a bound and not a null |

Clause 2 before 3 and 4 because a rotated law makes `B` an extrapolation and not an offset. Clause
5 before 6 because 6 *is* a rejection derived from a null and so is the one branch the dose gate
legitimately guards. Clause 6 before 7 because the pre-registered resolution (0.14 against a
prediction of 1.0) is what licenses reading this particular null as a rejection — and that
licence was computed before the run, not granted after seeing the width.

A **transport delay that rotated the law** would be a genuinely new finding — it would mean the
loop's timing enters the *gain* on `tau_a`, which no term in the drive assembly does — so the
pivot prints under clause 2 as it does in v81.

### THE PINS, AND ONE KERNEL CHANGE

Two new `HCArm` fields mean **every field-based verdict selector must pin both**. Five do: v77,
v78, v79, v80, v81, now through a single named helper (`hc_v82_shipped`) so the next dose extends
one function instead of auditing six call sites. **The sixth, v76, is immune by construction** — it
selects its arms by fixed index (3, 7, 8–11), so appending cannot reach it. All 131 existing arms
carry the shipped `0.02 / 0.0` explicitly; aggregate initialization would have **zero-filled
`rebound_gain` and switched the loop off on every one of them**, which is the appended-arm hijack
in its most destructive form.

The kernel gains **one recorder and one accessor**: `rebound_last_[m]`, written where the term is
computed and read by nothing inside the kernel, plus `Network::rebound_term(m)`. No arithmetic and
no RNG draw moves, and it is **deliberately not serialized** — it is recomputed every tick, so
adding it to the snapshot would change the save format for nothing. The pinned determinism hash
`ad96f882becbee92` is therefore expected to stand, and the gate will say so rather than be assumed.

**Determinism expectation:** writing `0.02f` over a genome that parsed `0.02`, and `0.0f` over
`0.0`, is a byte no-op, so **all 131 existing arms must re-print identically**. That is check 1 and
it is not assumed — if it fails, the patch offsets are wrong.

### WHAT COULD GO WRONG, NAMED IN ADVANCE

- **A delay shifts arrival *phase*, not only latency.** 100 ms is 7% of a cycle at tau 333 and 21%
  at tau 83, so the phase shift differs per tau. That confounds a *rotation* reading and leaves
  the `+1` prediction on `B` intact; clause 2 separates them.
- **Startup transient.** With a delay the ear's first *n* samples are zeros — at most 100 ms of a
  1.2M ms record, and the first 10% is discarded as settling anyway. Disclosed, not gated.
- **The loop-off arm removes a term rather than scaling one**, so it is a bound on the loop's total
  contribution and must not be read as a slope. It gets its own sentence in the verdict, and it is
  refitted on the common taus so its `B` is comparable with the anchor's at matched leverage —
  while being excluded from the delay regression, so the bound cannot silently re-weight the sweep.

**Cost:** 159 arms against 131 at 4h43m ≈ **5h44m**, plus the 16-minute gate.

### §40 PRE-FLIGHT — THE LOOP IS LIVE, AND THE CONTROLS WERE NOT OPTIONAL

A 60k pre-flight on the 159-arm set, run before any verdict code existed, to answer the one
question that decides whether this axis is worth five hours. **It is a vacuity check and a
bit-identity test, not a measurement** — nothing below is quoted as an effect, which the standing
rule forbids for `--allow-short`.

**(1) The loop is live and reaches the oscillator.** `rb000t167` (loop cut) against `tau167` (loop
at the shipped 0.02), same twelve seeds, one genome float apart: the voiced duty moves **0.46 →
0.38**. A term that was identically zero on every tick would have produced a bit-identical row, and
it did not. **(2) The transport delay reaches it too** — `sd100t167` differs from the anchor, and
across 25/50/100 ms the duty declines monotonically (0.46 / 0.45 / 0.43).

**(3) And the four controls earned their place, which is the pre-flight's real finding:**

| control | `anti` | duty | rate |
|---|---|---|---|
| `ctrl140` (the shipped loop) | +0.572 | 0.47 | **4.41 Hz** |
| `rb000ctrl` (loop cut) | +0.178 | 0.38 | **3.82 Hz** (−13%) |
| `sd025ctrl` | +0.666 | 0.45 | 4.53 Hz |
| `sd050ctrl` | +0.697 | 0.43 | 4.58 Hz |
| `sd100ctrl` | +0.759 | 0.43 | **4.70 Hz** (+6.6%) |

**The dose reaches the control, and the control's rate is what the kill switch gates on.** Had v82
reused `ctrl140` — as v79 and v81 legitimately do, because `halfcenter_drive` is unread when the
gain is zero — all 24 arms would have been scored against a floor measured on a creature that
heard itself differently. That is the §34 lesson arriving the right way round for once: the
question *"can the arm I am measuring reach the field?"* was asked of the **control** this time,
before the run rather than after it.

**(4) The five pins were verified empirically, not just by reading them.** Appending 28 arms left
the arm-name sequence of every published block identical — v77 (20 names), v78 (40), v79 (36), v80
(29), v81 (37), **162 in all**. Arm names do not depend on record length, so a short run is a
valid instrument for exactly this check. The hijack class of defect is closed by measurement.

### AND ONE TRAP THE PRE-FLIGHT WALKED INTO, RECOGNISED IN ADVANCE THIS TIME

`rebound_mean_tau_ms = 5000`. At 60k ticks the settling period is 6 s — **1.2 time constants** — so
the term's own mean is still converging while the record is taken, which leaves a slow drift in
`use = raw − mean(raw)` that inflates **both** the DC level and the swing. At 1.2M ticks the settle
is 120 s, twenty-four time constants, and the residual drift is negligible.

**So the pre-flight's dose reading is an UPPER BOUND on the full-record swing, not an estimate of
it.** This is structurally the same trap as §39's: a quantity that looks larger in a short record
than it is at full length, read as evidence the dose landed. The difference is that this time the
bias was derived from the genome's own time constant before the number was looked at, rather than
diagnosed from a vacuous verdict afterwards. **Clause 5 reads the dose at full record and nowhere
else.**

### §40 ADDENDUM — A SIXTH GUARD DEFECT, FOUND BY READING MY OWN CASCADE

Clause 1 of the table above was written as *"fewer than 3 delay settings fitted"*. That left a
gap, found before the run by re-reading the branch rather than the result:

`slope_held` **starts true** and is only ever set false by a comparison against the anchor's slope.
So had the anchor itself failed to fit, the flag would still have read **held** — having compared
nothing — and the cascade would have fallen through to a clause that presumes a held slope. A
guard asserting a condition it never checked is the defect this experiment has now shipped five
times (§35, §37, §39), and this would have been the sixth.

The anchor has fitted in every run on this line, with six live taus and R² 0.99954, **which is
precisely why the branch would never have been exercised in a smoke** — the same reason §35's
refusal path had to be forced to run before its own defect showed. Clause 1 now requires the
anchor to be balanced and names which of the two conditions failed. It is written as a refusal
branch in the code, not as a caveat in this document.

**The general form, which is the part worth keeping:** a boolean initialized to the *permissive*
value and narrowed only inside a conditional block carries two meanings — *"the check ran and
passed"* and *"the check did not run"* — and those are the same two meanings that `fit_ok` carried
in §35. **Initialize such a flag to the value that means "unestablished", or require, in the
branch that consumes it, the precondition that makes it meaningful.**

### §40 SMOKE — ALL SEVEN CLAUSES EXERCISED, AND TWO HONEST DISCLOSURES BEFORE THE RUN

The 60k smoke reaches every new path: the five per-setting blocks with their own controls, the
balanced design, the dosimeter with its spread, the DC check, the loop-off bound, the rotation
table, the primary, and the verdict. **Nothing below is an effect** — 60k distorts the law in a
documented direction (the anchor fits 4.0441 / 71.5 against the published 3.4756 / 203.1) and the
standing rule forbids quoting an `--allow-short` result. What a smoke *can* establish is that the
code computes what it claims and that the design's premises hold.

**(1) THE v45 MEAN-SUBTRACTION WORKS, WHICH NO RUN HAD EVER CHECKED.** The rebound's DC level is
**+0.00027** against a swing of **0.01290** — **2% of the swing.** The term is predominantly AC,
exactly as the genome's comment intends, and that is the load-bearing premise of this whole
section: it is *why* §37's one-subtraction closure does not reach this axis. Had the DC come back
comparable to the swing, most of §40 would have been refuted before it ran, by a result already on
file. The premise is now measured rather than argued.

**(2) THE DOSE SITS AT 1.61% AGAINST A 1% GATE, AND THE 60k READING IS BIASED UPWARD.** Disclosed
now, in advance, because it is the one way this run can come back without a latency verdict: if the
full-record swing falls below 1% of the tonic, clause 5 fires and the delay branch is VACUOUS. The
threshold **stays at 1%** — moving it after seeing a number is the thing this project has a memory
file about, and the direction of the bias is known, which makes a post-hoc adjustment exactly the
unjustifiable kind.

The bias is also smaller than the pre-flight note feared, and the measured DC says so: at 60k the
*scored* window is 54 s, which is **10.8** time constants of the 5 s mean, not the 1.2 the settling
period alone suggested — so convergence is largely complete within the record and the DC of 0.00027
confirms it. The caveat erred conservative, which is the right direction, and is now quantified.

**AND IT MATTERS LESS THAN IT APPEARS, BY DESIGN.** The decisive arm is threshold-free. The
loop-off bound measures the loop's *entire* contribution to `B` directly — at 60k `dB = +8.5 ±
18.3 ms` (0.5 SE), bounding the loop below 22% of the offset even at a twentieth of the record. At
full length the SEs shrink by roughly √20, which should bound it near **5%**. So whatever clause 5
does to the delay branch, **the question "can the self-audition loop host `B`?" is answered by an
arm that needs no dosimeter at all** — which is what "self-dosimetering" was pre-registered to
mean, and the reason the bound was put in the same run rather than after the sweep.

**(3) AND THE SMOKE SHOWS THE SHAPE THE ANSWER IS LIKELY TO TAKE.** All three delay settings hold
the slope (−0.4, +0.2, +0.5 SE), so this would be **the first axis on this line that does not
rotate the law** — and `dB/d(delay)` comes back **−0.1566**, wrong in sign and 13.9 SE from the +1.0
a transport account requires. That is the same signature that refuted the membrane in v78 (needed
≈+1, measured −0.433). Stated as a prediction of the run's reading, not as the reading.

### §40 AMENDMENT, WRITTEN WHILE THE RUN IS IN FLIGHT AND BEFORE ANY RESULT EXISTS

Recorded now, from reading the genome, **not** from reading an outcome — the run is ~1 h into
~6 h and its verdict is not yet visible. The measurement is unaffected; what is wrong is the
**scope of one verdict sentence** and the **reach of the dose gate**.

**THE REBOUND IS NOT THE LOOP. IT IS THE WEAKEST OF THREE ROUTES FROM THE EAR TO THE VOICE.**
`dna/default.toml` declares two synaptic `auditory -> vocal` projections as well:

| route | strength |
|---|---|
| the rebound (`rebound_source = 1`) | `rebound_gain` = **0.02** |
| `auditory -> vocal`, `source = "either"` (line 2656) | density 0.15, weight **0.12** |
| `auditory -> vocal`, `source = "inhibitory"` (line 2906) — the listening reflex | density 0.25, weight **0.25** |

So `rebound_gain = 0.00` cuts **one** path, and the *smallest*. The verdict block's sentence
*"THE WHOLE LOOP CONTRIBUTES NOTHING TO B ... THAT CLOSES THE AUDITORY CANDIDATE"* **overclaims and
is withdrawn.** What that arm bounds is the **rebound path's** contribution, and it must be read as
exactly that. This is the seventh instance of this project's recurring defect — a guard or a label
confidently describing something it does not measure — and it is mine, in a running experiment,
found by a five-minute genome read that belonged *before* the pre-registration.

**THE TWO DOSES SWAP ROLES, AND THE BROAD ONE IS THE DELAY.** `self_delay_ms` delays the audio
**before the cochlea**, upstream of all three routes, so the latency sweep probes the entire
self-audition loop while the loop-off arm probes a twentieth of it. §40 framed these the other way
round. Clause 6's wording happens to be correctly scoped already — *"the offset is not the time the
creature's voice takes to return through its ear"* is a claim about the delay, and the delay delays
everything.

**AND THE DOSE GATE THEREFORE DOES NOT BEAR ON THE DELAY BRANCH.** Clause 5 reads `sd(rebound)`,
which meters the weakest route; a delay acting through the two synaptic paths would be real whatever
the rebound's size. **Pre-registered reading, stated before the outcome: if clause 5 fires, the
delay branch is governed by clause 6 or 7 on the coefficient alone, and the "VACUOUS" label applies
only to the rebound path.** The threshold is not moved and the code is not touched mid-run — what is
fixed is the *scope* of a label, derived from the genome and committed before the number.

**WHAT THIS DOES NOT WEAKEN.** The delay sweep's power arithmetic, its one-parameter prediction, the
rotation check and the balanced design are all untouched; they were never functions of which route
carries the signal. If anything the run got **stronger**: its broad dose turns out to be the one
with 7x headroom over its own hypothesis.

**AND IT MAKES v82 A TERMINUS RATHER THAN A WAYPOINT.** The period is measured on **neural** rates —
Schmitt crossings of the half-difference of `rate_fast` within vocal group 2 — so the acoustic plant
(the 800 ms formant pole, the jaw) sits **downstream of the instrument** and cannot reach the
oscillator except back through the ear. The delay sweep is exactly that path. So v82 closes the
plant by the same measurement that closes the loop, and the candidate list for `B` inside this
creature is then empty rather than one short.

### §40 RESULT — THE OFFSET IS NOT THE TIME THE VOICE TAKES TO RETURN THROUGH THE EAR (2026-11-03)

`results/halfcenter-v82.log`, 159 arms, 12 reps, 1.2M ticks. Gate `results/verify-v82.log`.
Pre-registered in §40 with the clause order, and **amended mid-run before any result existed** —
that amendment governs the reading below wherever it differs from the printed verdict text.

**THE GATE HELD, INCLUDING THE ONE KEY THIS RUN COULD GENUINELY HAVE BROKEN.** All four keys PASS
and the pinned hash is still `ad96f882becbee92`. v82 is the first axis on this line to touch the
**kernel** (`rebound_last_[m]` plus an accessor), and the hash standing is the measurement that the
recorder is as inert as the reasoning claimed — written where the term is computed, read by nothing
inside the kernel, deliberately not serialised.

**ELEVEN DETERMINISM CHECKS, ALL IDENTICAL OR CLEAN.** The 131 pre-v82 main-table rows (131/131
matched, byte-identical), their kill-switch lines, the regime profile's first 131 rows, the published
clock-law block, and the v76/v77/v78/v79/v80/v81 blocks entire at 30/69/164/94/79/116 lines — v81
diffed with its prose for the first time, which is what `END v81` was added for. Check 11: **no v82
arm name appears anywhere between the v76 and v81 markers**, so the five `hc_v82_shipped` pins held
at full record exactly as the 60k pre-flight said they would. The anchor cross-check agrees byte for
byte with v81's same-arms anchor for the **fourth** consecutive run (v80, v81, v81b, v82).

#### THE MEASUREMENT

Balanced: all **6 of 6** taus live at all four delay settings, so each setting's balanced fit equals
its own fit.

| self_delay | B (intercept) | slope on tau_a | vs anchor |
|---|---|---|---|
| 0 ms (shipped) | **186.7 ± 8.6 ms** | 3.5359 ± 0.0378 | anchor |
| 25 ms | 186.4 ± 8.5 ms | 3.5385 ± 0.0376 | +0.0 SE — HELD |
| 50 ms | 185.6 ± 8.8 ms | 3.5410 ± 0.0388 | +0.1 SE — HELD |
| 100 ms | 187.0 ± 8.2 ms | 3.5349 ± 0.0363 | −0.0 SE — HELD |
| loop cut (`rebound_gain` 0) | 187.4 ± 9.0 ms | 3.5342 ± 0.0398 | — |

**THE PRIMARY: `dB/d(self_delay)` = +0.0026 ± 0.0093 ms per ms** (0.3 SE; 2 dof, 5% critical t
4.303, so unresolved), R² 0.038. **The prediction is +1.0000 ms/ms exactly, with no free scale** —
if `B` is the loop's transport time then adding transport adds to it. The measurement is **107.0 SE
from +1**, and bounded above by **+0.0427 ms/ms at 95%, which excludes +1.0 by a factor of 23.4.**

**THIS IS A NULL THAT REJECTS, and it may be read that way only because of what was fixed in
advance:** the resolution was pre-registered at 0.14 ms/ms against a prediction of 1.0 — seven times
finer than the thing it tests. A null at this resolution against a parameter-free prediction is a
refusal, not an absence of power.

**THE REBOUND-PATH BOUND** (the threshold-free arm — it removes the term rather than scaling it):
`dB` **+0.7 ± 12.4 ms** (0.1 SE), `dA` −0.0018 ± 0.0549. So `|dB| < 30.4 ms at 95%`, under **15.0%**
of the 203.1 ms offset.

**SCOPE, PER THE AMENDMENT AND AGAINST THE LOG'S OWN WORDING.** The verdict block prints *"THE WHOLE
LOOP CONTRIBUTES NOTHING TO B ... THAT CLOSES THE AUDITORY CANDIDATE"*. **That sentence was withdrawn
in `87bfe35`, before this number existed, and the log preserves the overclaim permanently.**
`rebound_gain = 0` cuts one of three ear→voice routes and the *smallest* (0.02 against synaptic
weights 0.12 and 0.25). The bound is on the **rebound path**. The **delay** sweep is the broad test,
because `self_delay_ms` delays the audio before the cochlea and therefore upstream of all three.

#### THE DOSE, AND WHY THE AMENDMENT PAID FOR ITSELF

The rebound the kernel applies to the vocal drive swings **sd 0.00843 ± 0.00002** rate units over 12
seeds — **1.05% of the +0.80 net tonic** — on a DC level of **−0.00001**, i.e. 0.1% of the swing.
Two things follow. The term is **predominantly AC as designed**, which confirms the premise §40 rests
on: §37's one-subtraction closure genuinely does not reach this axis, a thing never previously
checked. And the liveness gate was pre-registered at 1%, so **it cleared by 5% of relative margin** —
a knife edge.

**That knife edge does not load-bear, and the reason is the amendment.** It pre-registered, before
the number, that *if clause 5 fires the delay branch is governed by clause 6 or 7 on the coefficient
alone, and "VACUOUS" applies only to the rebound path* — because the dosimeter meters the weakest of
three routes while the delay acts on all of them. So the delay rejection holds whichever side of 1%
the rebound's swing had landed on. **The amendment's value is now measurable: it converted a coin-flip
gate into an irrelevance, and it was written from the genome rather than from an outcome.**

**AN INDEPENDENT LIVENESS SIGNAL THE DOSIMETER DOES NOT READ — ordered, and UNBARRED.** The *control*
arms move monotonically with the dose: duty **0.57 / 0.56 / 0.55 / 0.54** across 0/25/50/100 ms, rate
4.94 / 4.93 / 4.93 / 4.92 Hz, floor period 164.2 / 165.0 / 165.0 / 165.0 ms — and the loop-off control
sits outside that ordering on all three (duty **0.61**, rate 4.97 Hz, period **160.8 ms**). No error
bars are printed on those columns, so this is corroboration and **not** a measured effect. But it is
an ordered dose–response in a quantity independent of the dosimeter, and note the direction:
**increasing the delay moves duty AWAY from the loop-off value, not toward it.** A delayed loop is not
a weaker loop, it is a differently-timed one. The loop does something to the non-oscillating creature
while doing nothing whatever to `B`.

#### WHAT IS CLOSED, AND THE ONE RESIDUAL

**CLOSED: the latency account, for the entire self-audition loop.** All three ear→voice routes see the
delayed signal and `B` did not move at 23.4x the resolution the hypothesis needs.

**CLOSED BY THE SAME MEASUREMENT: the plant.** The period is read on **neural** rates (Schmitt
crossings of the half-difference of `rate_fast` in vocal group 2), so the formant pole and the jaw sit
**downstream of the instrument** and can only reach the oscillator back through the ear — which is the
path just swept flat.

**THE ONE RESIDUAL, STATED AND NOT CHASED.** A *non-latency* contribution through the two synaptic
`auditory -> vocal` projections is not bounded: the delay sweep refutes the latency form for all three
routes, and the loop-off arm bounds only the rebound. Bounding the other two would mean cutting those
projections, which v82 did not do.

**AND IT WILL NOT BE CHASED, BY THE LINE-LEVEL STOPPING RULE.** The question *what capability would
naming `B` buy?* has the answer **none** — the period is set by `tau_a`, so `B` is neither a lever nor
a blocker. That rule, recorded the day before this run landed, exists precisely because "this axis is
refuted" otherwise generates "try the adjacent axis" forever. **§40 is the terminus of the offset line.
Twelve DNA versions, v76 through v82, and the honest summary is that the quantity was never worth
naming.** The next work is on the input side — recognition, instruction, accumulation.

## §41 — THE NUMBER OF ORTHOGONAL AXES, PRE-REGISTERED (2026-11-03)

**The first line of the input-side work, and it is not an input-side line** — it is the
**accumulation** lever, which is the row the goal names as the real wall. Written before any build.

### WHY, AS A CHAIN OF FOUR MEASURED RESULTS WITH NO GAP

1. **Retention is FREE across an orthogonal axis.** A second lesson *silent* about F1 leaves lesson
   A at **0.948 ± 0.126** — not distinguishable from 1.0 — while one that *demands* F1 wipes it to
   **−0.057/−0.167**. `axisfree`, **+5.7 SE** between them.
2. **Two orthogonal lessons coexist**: 0.84 kept against 0.22 for a conflicting one (`capacity`).
3. **A second axis is genuinely independent**: biasing vocal group 3 swings F2 by **961 Hz with
   0.00 cross-talk into F1** (`aimaxes`).
4. **Levels MULTIPLY across independent axes**, and `aimaxes` measured **2 of 9** groups. 2–3 levels
   on 2 axes = 4–9 words; on 4 axes = **16–81**, same per-axis resolution, **no new learning rule**.

**So "capacity is pure overwrite holding ~2 items" is the wrong summary.** Overwrite is what happens
*on one axis*. The item count is **the number of orthogonal axes**, and seven are unmeasured.

### THE BUILD, WHICH IS SMALL BY DESIGN

`aimaxes` already loops over `kAXGroups[gi]` and `VocalParams` already exposes every output
(`f0 f1 f2 f3 bw1 bw2 bw3 amplitude voicing`). So: `kAXNGroups` 2 → 9, record all nine parameters
per rung instead of two, and print a **nine-by-nine normalised cross-talk matrix**. Read-only,
66666 ticks, `Expect::kRead`, nothing learns.

**The candidate set is four, not nine.** `larynx-reads-nine-scalars` gives the non-guessable
mapping — centroid 0→f0, centroids 2–7→f1,f2,f3,bw1,bw2,bw3, **activity** 1→voicing, **activity**
8→loudness. F1 and F2 are done. **F0 is excluded for lineage**: the transmission tape carries
formants only, so an F0 distinction cannot be taught creature-to-creature. That leaves **F3 and the
three bandwidths**, plus voicing/loudness which are not vowel-identity axes.

### THE CLAUSES, IN ORDER — AND ORDER IS PART OF THE PRE-REGISTRATION

**CLAUSE 1 — LIVENESS, AND IT COMES FIRST BECAUSE A DEAD AXIS SCORES AS A PERFECT ONE.** This is the
defect this project has installed seven times: *a guard measuring the wrong quantity.* Cross-talk is
a ratio, so **an axis that does not move at all has zero cross-talk trivially** and would be reported
as ideally independent. So for each group, the own-axis full-ladder swing must exceed the within-run
scatter by a factor stated **now**: `|swing_own| >= 3 * sd_own`. An axis failing this prints
**NO VERDICT — AXIS DEAD**, and is **never** counted as independent and never enters the product.

**CLAUSE 2 — CROSS-TALK, NORMALISED, BECAUSE RAW Hz CANNOT BE COMPARED ACROSS THESE AXES.** F0 rests
near 120, F3 near 2500, the bandwidths near 80–120, so a raw Hz off-diagonal is meaningless. Define
`X[g][p] = swing_p(bias g) / swing_p(bias p)` — each axis's response in units of **its own**
steerable range. The diagonal is 1 by construction. **Independence bar: `|X[g][p]| <= 0.10` for all
p != g**, an order of magnitude looser than `aimaxes`'s measured 0.01 between groups 2 and 3, chosen
loose on purpose so a *refusal* is unambiguous rather than marginal.

**CLAUSE 3 — RESOLUTION, which is what actually sets the prize.** Levels per axis
`= 1 + |swing_own| / (k * sd_own)`, with `k` fixed at the value `aimaxes` used to get its 2.6, so
this run's numbers are comparable to the one published result on this instrument. An axis that is
live and independent but resolves **< 2 levels adds nothing** and is reported as such.

**CLAUSE 4 — AND ONLY THEN THE PRODUCT.** The code-space claim is the product of level counts over
axes that passed clauses 1, 2 and 3. **It is a claim about CODE SPACE, not about words.** No naming
claim may be made from this run — see the gate below.

### WHAT THIS RUN MAY NOT CONCLUDE, STATED IN ADVANCE

**THE EAR IS THE GATE, NOT THE LARYNX.** Naming needs the distinction *heard and scored*. The ear
handles F2 (/i/ vs /u/ at 0.900) but **whether it resolves F3 or bandwidth is unmeasured**, and
`kVLScoreFormant` scores only F1/F2 — so the scorer and the reward both need extending before any
word count is claimed. **A steerable, independent, well-resolved axis the ear cannot hear is worth
exactly nothing for naming.** This is stage one of two.

**AND ORTHOGONAL IS FREE FOR RETENTION, NEVER FOR THE REWARD CHANNEL.** `capacity`: two dimensions
share one scalar reward, A keeps only **0.58** against A-then-A, and **sequential beats
simultaneous**. More axes multiply the code space; the teaching must still be sequential. Any
proposal to teach two axes at once is already refused.

### THE LINE'S OWN STOPPING RULE, WITH THE COUNT

1. **Which part of the goal:** accumulation *and* vocabulary, together.
2. **The cheapest thing that could refuse it:** cross-talk (clause 2) — the prize is a product of
   *independent* axes, so an axis that moves F1/F2 while moving itself **reallocates rather than
   adds**. One read-only run settles it.
3. **ABANDON AFTER 3 RUNS**, regardless of how interesting it has become: (i) this nine-group
   cross-talk matrix, (ii) the ear's resolution on whichever axes survive, (iii) one build if both
   pass. If (i) or (ii) refuses, stop at that run and write the refusal.

**Had this rule existed in September the offset hunt would have ended at v79.** It exists now
because §40 cost twelve DNA versions to establish that a quantity was never worth naming.

### §41 RESULT — SIX INDEPENDENT AXES, NOT TWO (2026-11-03, run 1 of 3)

`results/axisgrid-v1.log`, new experiment `axisgrid`, 9 groups × 5 rungs × 12 creatures at
aimaxes's pinned 66666-tick window, read-only, 4m40s. `aimaxes` left byte-identical.

**THE INSTRUMENT IS aimaxes's, AND IT SAYS SO.** F1 swing **372.2** against aimgain's published
378.3 and F1 rest **626.5** against its 629.7 — the reproduction guard passes. F2 swings **947.4**
against aimaxes's published **961**. Two independent anchors from a different experiment, both
reproduced by new code.

**AND A FREE DETERMINISM CHECK THAT COST NOTHING TO ADD.** No bias is applied at the zero rung, so
all nine groups run the *same* creature there. Largest disagreement across the nine: **0.000e+00**.
Any nonzero value would have been a harness bug, and this is the check that proves the nine slices
are indexed correctly.

#### CLAUSE 1 — LIVENESS, AND IT IS THE CLAUSE THAT DID THE WORK

| group → parameter | own swing | bar (3×sd) | verdict |
|---|---|---|---|
| 0 → f0 | 157.0 | 27.4 | LIVE |
| 1 → **voi** | 0.009 | 1.45 | **NOT TESTED** |
| 2 → f1 | 372.2 | 65.2 | LIVE |
| 3 → f2 | 947.4 | 173.3 | LIVE |
| 4 → f3 | 803.9 | 142.8 | LIVE |
| 5 → bw1 | 99.8 | 16.0 | LIVE |
| 6 → bw2 | 101.1 | 17.9 | LIVE |
| 7 → bw3 | 100.2 | 16.9 | LIVE |
| 8 → **amp** | 0.002 | 0.47 | **NOT TESTED** |

**THE TWO ZEROES ARE NOT A REFUTATION AND THE FIRST DRAFT LABELLED THEM AS ONE.** `set_bias_oracle`
is *"GRADED AND ZERO-MEAN on purpose"* — a ramp from −amp to +amp sampled so the mean over the
group is exactly zero, which **moves a centroid while adding no net drive.** Seven parameters are
read as centroids; **voicing and loudness are read as ACTIVITY** (activity 1 and activity 8, per
`m3probe`). A zero-mean ramp cannot move a net-drive readout **by construction**, so those two cells
were never a test. The run was re-run with the label corrected to **NOT TESTED**, and every measured
number came back byte-identical, which is what proves the fix touched only the label. Those axes may
well be steerable by a non-zero-mean bias; this says nothing about it.

#### CLAUSE 2 — CROSS-TALK: THE SEVEN LIVE AXES ARE INDEPENDENT

Worst off-diagonal into a teachable axis, per group: **0.002, 0.006, 0.003, 0.004, 0.007, 0.003,
0.002** — every one at or under **0.007**, against a pre-registered bar of 0.10 and aimaxes's
measured 0.01 between F1 and F2. **All seven pass.**

#### CLAUSE 3 — RESOLUTION, AND AN UNEXPECTED STRUCTURAL FINDING

| axis | levels | oracle swing/scatter |
|---|---|---|
| f0 | 2.73 | 17.2 |
| f1 | 2.72 | 17.1 |
| f2 | 2.60 | 16.4 |
| f3 | 2.68 | 16.9 |
| bw1 | 2.96 | 18.7 |
| bw2 | 2.69 | 16.9 |
| bw3 | 2.83 | 17.8 |

**swing/scatter spans 16.4 to 18.7 across seven axes whose absolute scales differ by 10× (f0 rests
at 246, f2 at 1670, the bandwidths near 150).** That near-constancy is a finding, not a diagnostic:
**the ~2.6 levels per axis is a property of the GROUP ARCHITECTURE** — identical neuron count and
`noise_amp` per group — **and not of any parameter.** aimaxes's 2.6, replicated seven times. It is
also why the level counts look suspiciously uniform, which was the first thing to check.

#### CLAUSE 4 — THE PRODUCT

Six teachable axes qualify (f1, f2, f3, bw1, bw2, bw3; **f0 excluded for lineage** — the tape
carries formants only). **Code space = 424.9 distinguishable signals, against aimaxes's 2 axes and
4–9 words, with no new learning rule.**

#### §41's OWN POWER ARGUMENT WAS WRONG, AND THAT IS WORTH MORE THAN THE RESULT

§41 named **cross-talk** as *"the cheapest thing that could refuse it"*. **It could not have.** The
vocal groups are **disjoint index slices** and each parameter is a centroid over *its own* slice, so
a ramp inside one slice can only reach another through lateral dynamics. Near-zero was
*structurally* near-certain, and a 0.10 bar against a measured 0.007 was never going to bind. **A
pre-registered refusal clause is only as good as its probability of firing**, and I did not compute
that probability — the same omission as a power calculation, one level up. The clause that actually
worked was **clause 1**, which caught the two readouts the instrument cannot drive at all, and it
was written for a different reason (a dead axis scoring as a perfect one).

**New rule: when pre-registering a refusal clause, state what would have to be true of the
ARCHITECTURE for it to fire.** If the answer is "nothing could have made it fire", it is decoration.

#### WHAT THIS DOES NOT ESTABLISH — THE SAME LIST §41 WROTE IN ADVANCE

1. **IT IS NOT A WORD COUNT.** The ear is the gate, not the larynx. Whether the ear resolves F3 or
   bandwidth is **unmeasured**, and `kVLScoreFormant` scores only F1/F2, so the scorer and the
   reward both need extending before any naming claim.
2. **THE RULER IS WITHIN-RUN JITTER**, which is aimaxes's convention and therefore comparable to its
   2.6 — but a held vowel is scored over many ticks, so it *overstates* the noise a scorer sees,
   while the quantity that actually bounds naming is **between-trial spread**, which this run does
   not measure. Wrong ruler in both directions, conservative in the one that matters.
3. **THE LEARNED/ORACLE RATIO IS ASSUMED, NOT MEASURED.** `frac = 0.317` comes from ctxscale's
   118 Hz F1 asymptote over this run's F1 oracle swing, and is applied to six other axes.
4. **TEACHING MUST STILL BE SEQUENTIAL.** `capacity`: two dimensions share one scalar reward channel,
   A keeps 0.58 against A-then-A. More axes multiply the code space, not the teaching rate.

**RUN 1 OF 3 PASSED. Run 2 is the ear**, per §41's abandonment count: whether the auditory module
resolves F3 and bandwidth distinctions. If it does not, the prize collapses to the two axes already
known and the line stops there.

### §41 RUN 2 — REFUSED AT THE DESK, AND IT CORRECTS RUN 1 (2026-11-03, zero runs)

**Run 2 was to measure whether the ear resolves F3 and bandwidth. It cannot be run, and the reason
invalidates part of §41 RESULT published an hour ago.**

#### THE PRE-FLIGHT THAT WAS SUPPOSED TO PRICE THE EAR FOUND THE RENDERER INSTEAD

The new rule from §41 RESULT — *state what the architecture would have to make true for the clause
to fire* — was applied to run 2 before building it. First the cochlea, which looked **fine**:

| axis | rest | learned reach | Hz per mel channel | reach in channels |
|---|---|---|---|---|
| f1 | 626.5 | 118.0 | 128.7 | **0.92** |
| f2 | 1669.6 | 300.3 | 229.9 | **1.31** |
| f3 | 2613.7 | 254.9 | 321.6 | **0.79** |

24 mel channels, 50–7000 Hz, 8 neurons each. **F3's reach is 0.79 of a channel against F1's 0.92,
and F1 demonstrably works** — so the ear was never the likely blocker, and the test looked live.

**THEN THE SYNTHESISER.** `host/src/audio.cpp`:

```
void VowelSource::render(float f0, float f1, float f2, float amplitude, ...)
  const float bw = 90.0f;
```

**TWO RESONATORS AND A HARDCODED BANDWIDTH.** There is no F3 resonator, and the three commanded
bandwidths are replaced by a single constant 90 Hz. `struct Word { float f0, f1, f2; }` carries the
same three. This is the **only** audio path — the creature's own voice into its own ear and the
caregiver's speech both go through it. `f3`, `bw1`, `bw2`, `bw3` are computed by the vocal decoder,
exposed in `VocalParams`, written to the panel and the journal, **and never rendered.**

#### WHAT THIS DOES TO §41 RESULT

**STILL TRUE, and measured:** the larynx *commands* seven independent centroid axes with worst
cross-talk 0.007 and ~2.6 levels each; the level count is the group architecture (swing/scatter
16.4–18.7 across a 10× range of scales); retention is nearly free across an orthogonal axis, so the
item count is the axis count.

**NOW FALSE AS PUBLISHED: "six teachable axes, code space 424.9."** Four of those six — f3, bw1,
bw2, bw3 — **do not reach the air at all.** The axes that reach the air are f0, f1 and f2; of those,
f0 does not survive creature-to-creature teaching. **So the transmitted code space is f1 × f2 ≈ 2.72
× 2.60 ≈ 7.1 signals, which is exactly aimaxes's 4–9 and exactly where the project already was.**

The 425 is a property of the **motor representation**, and it is entirely untransmitted. That number
must not be quoted. §41 RESULT's own caveat said "this is not a word count, the ear is the gate" —
**the caveat was right and the layer was wrong. The gate is the RENDERER, one layer before the ear.**

#### AND RUN 2 WOULD HAVE BEEN VACUOUS, WHICH IS THE REUSABLE PART

Had this been built as specified, it would have presented F3-varying stimuli through a synthesiser
with no F3, measured chance, and concluded **"the ear cannot resolve F3"** — a false refusal of the
ear, from an instrument that could not have produced any other answer. That is
`aibaby-null-is-not-a-control` exactly: an arm whose result is fixed by construction.

**It was caught for zero runs by the rule written three hours earlier in this same section**, which
is the first time a method rule in this project has paid out on the very next run. The rule earned
its place: *before building, say what the architecture must make true for the measurement to be
capable of going either way.* I applied it to the cochlea, got a reassuring answer, and the habit is
what carried me one layer further to the renderer.

#### THE LINE'S STATUS UNDER ITS OWN STOPPING RULE

§41 said: abandon after 3 runs, and stop if run 1 or run 2 refuses. **Run 2 refuses.** By the letter
of the pre-registration the line stops here.

**But the refusal is of a different KIND than the one anticipated, and that must not be used as an
excuse to continue.** §41 anticipated *"the ear cannot hear these distinctions"* — a fact about the
creature. What was found is *"the simulation never implements them"* — a fact about the code. A real
vocal tract has an F3 and real formant bandwidths, and the decoder already computes all four, so
extending the renderer would be completing an acknowledged incompleteness rather than inventing a
capability.

**THAT IS A DECISION ABOUT THE CREATURE'S PHYSICS, NOT AN EXPERIMENT, AND IT IS NOT MINE TO TAKE.**
A third resonator and live bandwidths change what every creature has ever heard of itself
(`self_gain` is on), so they would almost certainly move the determinism hash and break
comparability with the entire archive of results. The line is therefore **HELD, not continued**,
pending that call.

### §41 RUN 2 RESULT — ONE OF THE FOUR SURVIVES, AND IT IS F3 (2026-11-03)

`results/axisear-v1.log`, new experiment `axisear`, plus **DNA v83** which made the measurement
possible. Read-only, seconds.

#### WHAT v83 IS, AND WHY IT SHIPS OFF

`VowelSource::render` gained a third resonator and the three commanded bandwidths, behind
`render_f3_gain` and `render_bw_gain`, **both shipping at 0.0.** The off-path is the *literal*
pre-v83 code — the third resonator is never evaluated and `bw` is the same literal `90.0f` — so
bit-identity is **by construction** rather than by a floating-point argument. `x + 0.0f * y` is not
reliably `x`; a branch is. Both are **gains, not flags**, because an axis needs a dose.

The off-by-default choice is not timidity: turning either on changes what every creature has ever
heard of **itself**, since `self_gain` is on. Any result compared across that boundary compares two
different creatures.

#### THE RESULT — d', unbiased, through the creature's own cochlea, at each axis's OWN reach

| axis | gain 0 (shipped) | gain 1 | × F1 |
|---|---|---|---|
| f1 | 44.822 | 36.398 | 1.000 |
| f2 | 91.805 | 94.569 | 2.598 |
| **f3** | **0.002** | **31.195** | **0.857** |
| bw1 | −0.043 | 0.399 | 0.011 |
| bw2 | −0.042 | 0.418 | 0.011 |
| bw3 | 0.036 | 0.367 | 0.010 |

Null floor `sqrt(d*(1/n0+1/n1)) = 0.346` at n=200 per class, **derived**. Positive control passes:
f1 and f2 clear the 2AFC bar at both settings.

**F3 IS A REAL AUDIBLE AXIS: 0.002 → 31.195, at 0.86× F1's own separability.** **THE THREE
BANDWIDTHS ARE NOT:** 0.37–0.42, barely clear of the 0.346 floor and **~1% of F1**. At the reach the
creature can actually command, a 21% bandwidth change does essentially nothing in the air.

**AND THE GAIN-0 COLUMN IS THE DIRECT MEASUREMENT OF THE DESK FINDING.** All four read *exactly null*
on the shipped creature — 0.002, −0.043, −0.042, 0.036. Those axes were acoustically absent, now
measured rather than inferred from reading `audio.cpp`.

#### THE CORRECTED PRIZE

**f1 × f2 × f3 ≈ 2.72 × 2.60 × 2.68 ≈ 19 signals, against ~7.1 before. About 2.7×, not the 60× run
1 implied.** One new axis, not four.

**AND `render_bw_gain` BUYS NOTHING WHILE COSTING SOMETHING:** f1's d' falls **44.822 → 36.398**
(−19%) with the gains on, because widening the resonators from 90 Hz to the commanded ~150 blurs
them. So if anything ever ships it is `render_f3_gain` alone. **Confounded, and stated as such:** this
run moves both gains together, so the f1 decrement cannot be attributed to bandwidth specifically. A
one-line follow-up separates them, and until it runs the attribution is a hypothesis.

#### THREE INSTRUMENT ERRORS, ALL MINE, EACH CAUGHT BY THE RUN REFUSING

1. **NO DENOMINATOR.** The first ruler scored per-**frame** cepstra of one held posture. A
   deterministic noiseless vowel renders near-identical frames, so the within-class spread was
   numerical dust and d' came out at **41,514** — for a statistic where 1.0 means 76% correct. Fixed
   by making each sample a posture perturbed by the creature's **own measured motor scatter** from
   run 1, which makes this d' and run 1's level count the same quantity in two domains.
2. **CLAUSE 2 WAS MIS-DERIVED, AND THE VOID BRANCH SAID SO.** §41 required f1/f2 to be
   gain-independent "because they never pass through the new code". **They do:** `bw1` and `bw2` ARE
   the f1 and f2 resonators' bandwidths. The run voided on f2 moving 168448 → 279153 and was right
   about the number while the clause was wrong about the reason. Withdrawn and recorded.
3. **n=24 PUT THE BANDWIDTH CELLS ON THE NOISE FLOOR**, where the unbiased estimator's own null is
   d' ≈ 1.0 — and the bar was 1.0. My dose test was `|dp1| > 2*|dp0| + 0.10`, a rule invented for the
   numbers in front of me. Both replaced with the **derived** floor. Also: §41 demanded a dose of
   bw3, which **cannot show one** — bw3 is the third resonator's own width, so at `f3_gain = 0` it
   has no resonator to act on. A clause that could only ever fail.

#### STATUS, AND THE COUNT IS NOW SPENT

**STILL NOT A WORD COUNT.** This says the distinction survives the **cochlea**, not that the
auditory module represents it, and `kVLScoreFormant` still scores only F1/F2. Stage **2b** is that
measurement.

§41 budgeted three runs. Run 1 was the cross-talk matrix; run 2 was the ear, which required a
renderer build the user authorised as a separate scope decision. **Stage 2b is the third and last
under the rule.** If it passes, extending the scorer and the reward to F3 is a new line and needs its
own pre-registration — not a continuation of this one.

---

### §41 RUN 2 CORRECTED — THE INSTRUMENT REPORTED d'², AND THE SHIP DECISION WAS BACKWARDS

`axisear` v2, 24.9 s, zero new brain runs. Two faults in run 2, one of units and one of design, and
the second one **reverses the conclusion run 2 published**.

#### FAULT 1 — THE UNITS. `unbiased` RETURNS d'², NOT d'

`cepstral_dprime(..., unbiased = true)` ends on `return acc - dims * (1.0/n[0] + 1.0/n[1]) *
inflate;  // signed d'^2`. Its own header says why: the bias correction is unbiased in d'² and not
in d'. **Run 2 printed every cell as `d'`.** The measured values were right; three things built on
them were not.

| | run 2 published | correct |
|---|---|---|
| the 2AFC bar | `d' ≥ 1` | **unchanged** — `d' ≥ 1` ⟺ `d'² ≥ 1`, the same threshold. Luck, not correctness. |
| the null floor | 0.347, "= sqrt(d·(1/n₀+1/n₁))" | the removed **bias** is 0.121 in d'² units; its **spread** — which is what a floor should be — is **0.049**. Over-strict by 2.9×, and the wrong quantity besides. |
| every ratio | a ratio of **squares** | the square of the ratio. f3 is 0.80–0.88× F1, not 0.857×; the bandwidths are **0.10–0.13× F1, not the ~1% published**. |

So the headline verdicts stand and **the sentence under them does not**. The bandwidths are not
inaudible. They read d' 0.78 / 0.88 / 0.59 at **11–16 SE above zero** — about 67% in a 2AFC against
50% chance. They stay off because they are under the bar, and they stay **open**, because a
tenth-of-F1 axis could clear the bar at a larger reach and the reach here is the one run 1 measured,
not a limit of the body.

**WHY IT HID, AND THE RULE.** Run 2 derived the floor at n = 24, where the bias is 1.02 — and
`sqrt(1.02) = 1.01`. **A square root is invisible at the one sample size where the quantity is 1**,
and that was the sample size I derived it at. Moving to n = 200 is what made 0.121 and 0.347
different numbers. *Derive a constant where its value is NOT 1, or you cannot tell a quantity from
its square root.*

#### FAULT 2 — ONE KNOB, TWO FACTORS. A 2×2 WAS NEEDED AND REVERSES THE ANSWER

Run 2 moved `render_f3_gain` and `render_bw_gain` **together**, then attributed F1's decrement to the
bandwidths and concluded *"`render_bw_gain` buys nothing and costs f1 19%, so ship f3 alone."* The
2×2 over the two gains, paired over 12 replicate streams:

| cell | f0 | **f1** | f2 | f3 | bw1 | bw2 | bw3 |
|---|---|---|---|---|---|---|---|
| f3=0 bw=0 — **shipped** | 7.24 | **6.88** | 9.47 | — | — | — | — |
| f3=1 bw=0 | 7.08 | **6.10** | 8.98 | **4.85** | — | — | — |
| f3=0 bw=1 | 8.59 | **6.64** | 10.18 | — | 0.78 | 0.88 | — |
| f3=1 bw=1 | 8.57 | **6.19** | 9.54 | 5.44 | 0.72 | 0.81 | 0.59 |

(d', each the root of the mean of 12 reps' d'² — the root taken once, at the end, as that estimator
requires.) The paired attribution of F1's cost, in d'²:

```
f3 alone      -10.127 +- 0.324   COSTS F1   (31 SE)
bw alone       -3.211 +- 0.245   COSTS F1   (13 SE)
both (corner)  -9.083 +- 0.284   real
interaction    +4.255 +- 0.204   present
```

**F3 IS THE LARGER COST, NOT THE BANDWIDTHS.** It takes 11.3% of F1's d' on its own, against the
bandwidths' 3.5%, and the corner is *smaller* than f3 alone because the interaction recovers +4.26.
"Ship f3 alone to avoid the cost" is exactly backwards: shipping f3 alone is the **worst** cell for
F1 of the three that have f3 in them. The corrected decision is that **the third formant is a trade
against the first**, and the `f1 × f2 × f3` arithmetic has to price it.

**AND THE BANDWIDTHS ARE NOT ONLY A COST.** Turning them on *improves* f0 by 18.7%, f2 by 7.5% and
f3's own separability by 12%, while costing f1 3.5%. "Buys nothing" was wrong in both directions.

#### WHAT IS CONFOUNDED, AND WHAT IS NOT

**The f3 cost is clean.** Cell 1 has `bw_gain = 0`, so the third resonator renders at the shipped
hardcoded 90 Hz: it is "add F3 and change nothing else".

**The bw column is not.** `render_bw_gain = 1` does two things at once — it makes bandwidth *live*
**and** moves its mean from the hardcoded 90 Hz to the genome's ~149 Hz. So every bw number above
conflates "bandwidth now varies" with "bandwidth is now 60% wider". Per the operating-point rule,
stated rather than quietly fixed; separating them needs a cell that jitters around 90.

**AND THE MECHANISM OF F3'S COST IS UNTESTED, WITH THREE CANDIDATES — THE THIRD FOUND AT THE DESK
AFTER THE RUN AND THE MOST LIKELY OF THEM.**

1. **LEVEL.** The renderer sums `0.25*(v1+v2+g*v3)` with no renormalisation. Against it: amplitude
   is constant across all samples in a cell, so a constant level shift moves both classes' C0
   equally and cannot change a *between*-class difference directly.
2. **SPECTRAL MASKING** — F3's energy at ~2.6 kHz diluting F1's cepstral signature. No
   renormalisation would fix it, because it is the real price of adding a formant. Against it: F1
   at 626 Hz is the axis *furthest* from F3 in frequency and it loses the most.
3. **JITTER INFLATION, AND THIS IS THE LEADING CANDIDATE.** Every posture is drawn with jitter on
   **all seven** parameters, f3 included. At `f3_gain = 0` that f3 jitter is **inaudible**, so it
   contributes nothing to the within-class spread. At `f3_gain = 1` it reaches the ear and **inflates
   the denominator of every other axis's d'** while leaving its numerator alone. It predicts what
   the table shows and the other two do not: **f0, f1 and f2 all fall together** (−2.2%, −11.3%,
   −5.1%) rather than only the spectrally-adjacent one.

**IF (3) IS RIGHT THE FINDING REFRAMES, AND NOT AS A DEFECT.** There would be nothing to fix in the
renderer. The statement becomes: **giving the creature a seventh commanded axis means its motor noise
on that axis now reaches the ear and adds to the confusion on every other axis.** That is an
intrinsic cost of adding an axis to a code, it is the same quantity as §41 run 1's level count seen
from the other side, and it would bound how many axes a code can carry — which is a far more useful
result than a renderer patch.

**THE DISCRIMINATING TEST IS CLEAN, CHEAP AND NEEDS NO GENOME CHANGE:** re-measure F1 in cell 1 with
f3 held **exactly at rest** instead of jittered. Return toward 6.88 ⇒ jitter inflation; stay at 6.10
⇒ the static presence of the third formant. **Named, not run** — it is a separate claim and gets its
own build and its own gate rather than riding on this one.

**What none of the three explains** is the **positive interaction**: switching the bandwidths on
*recovers* +4.26 of F1's d'² and *improves* f3's own separability. Stated as unexplained.

#### THE REPRODUCTION CHECK, WHICH IS WHY ANY OF THIS IS CREDIBLE

Rep 0 keeps run 2's exact seeding, so cells 0 and 3 at rep 0 **must be run 2**. All fourteen agree to
the printed 3 dp — f1 44.822 and 36.398, f3 0.002 and 31.195, every bandwidth. The rebuild changed
the presentation and the design and **not one measured value**, and the clause that says so is a
`VOID` branch rather than a remark.

#### STATUS

Unchanged where it counts: **F3 is a real audible axis** at ~0.8× F1, the air carries it, and stage
2b is still the open question and still the last run §41's count allows. What changed is the price
tag — f3 taxes f1 by 11% — and a published sentence about the bandwidths that was wrong by a square.

---

### §41 RUN 2c — F3'S TAX ON F1 IS 29% MOTOR NOISE AND 71% THE FORMANT ITSELF

`axisjit`, 37.5 s, zero brain runs. The mechanism question from the section above, answered — and the
candidate I had promoted to "leading" turns out to carry under a third of the effect.

#### THE MANIPULATION

Hold an axis **exactly at rest** instead of jittering it, and ask how much of f3's cost to F1 comes
back. The deviate is still **drawn and then discarded**, so the RNG stream and every other parameter
are untouched: skipping the `gauss(rng)` call would have shifted every subsequent draw and the arms
would have differed by their noise as well as by the freeze.

#### THE RESULT

```
the cost       gain 0 -> gain 1, nothing frozen   dd'^2  -10.127 +-  0.324
the recovery   at gain 1, freeze f3's jitter      dd'^2    2.934 +-  0.154
recovered fraction of the cost                        0.290
```

The recovery is **19 SE above zero** and **20 SE short of the whole cost**, so both ends are
resolved and the verdict is the middle branch: **jitter inflation is real and is 29% of it.** The
remaining **71% is the static presence of a third formant** — the creature pays most of the tax
simply for having F3 in the air, not for being unsteady on it.

**So my desk hypothesis was right in kind and wrong in size.** I had written it up as "the leading
candidate" on the strength of one qualitative prediction (that f0, f1 and f2 would all fall
together). They do, and freezing f3 helps all three — f0 +2.42, f1 +2.93, f2 +5.98 — which is the
shared-denominator signature. But **f2 is helped most, and f2 is the axis spectrally closest to f3**,
which is the masking signature. Both fingerprints are in the same table, which is exactly what a 29/71
split should look like. A qualitative prediction that discriminates between mechanisms does not
measure their proportions, and I treated it as though it had.

#### CLAUSE 1 COULD NOT FIRE, AND THAT IS THE SECOND TIME TODAY

I pre-registered the gain-0 column as "the built-in null, which is the point": freezing f3 where f3
is inaudible must do nothing. It came back **exactly `0.0000 +- 0.0000` on all six axes** — bit-exact,
because at `f3_gain = 0` the renderer **never reads f3 at all**, so the freeze cannot change a single
sample. **Near-certain was not the word; structurally impossible was.**

It is not worthless — it confirms the off-path genuinely ignores f3, which is a real property of the
v83 build. But it is a **wiring check**, not a test that the freeze is a clean manipulation, and I
sold it as the latter. This is the identical error to §41's cross-talk clause, **recorded as a rule
this morning and committed within the same day**, which says the rule as written is not yet doing its
job: asking "what must the architecture make true?" catches it only if asked of the clause one is
most pleased with.

**The clause that WOULD have fired:** freeze an axis that is *audible* at gain 0 — f2, say — and
measure F1. Jitter inflation predicts a recovery there with no third resonator involved at all. That
is a falsifiable positive control for the mechanism, and it is the one I should have written.

#### WHAT MUST NOT BE READ FROM THIS RUN

The **"all off-axis frozen"** column is near-degenerate and its magnitudes are unreliable. F1 reads
d' 15.0 +- 5.3 — an SE a third of the value — because freezing almost everything collapses the
within-class denominator toward numerical dust. That is the same failure family as `axisear` v1's
d' of 41,514. The direction is informative (off-axis jitter is by far the largest single burden on
every axis) and **the numbers 178.5 and 201.8 are not quotable.**

#### WHAT IS STILL OPEN

The **71%** is still unsplit between **level** and **spectral masking**. The amplitude-compensated
cell separates them and is still named, not run. Level would be fixable in the renderer; masking
would not.

#### RUN ACCOUNTING, STATED RATHER THAN ASSUMED

§41 budgeted three runs with an abandonment count. Run 1 was the cross-talk matrix; run 2 was the
ear, and it has now taken **three builds** — v1 (void at the desk), v2 (the units and the 2x2), and
this one. **All three are read-only desk instruments costing seconds and zero brain runs**, which is
why I have not counted them against a budget that was written about expensive stages. **Stage 2b, the
auditory module, remains the third and last stage under the rule and is still unspent.** If that
reading of my own pre-registration is too convenient, the conservative count says §41 is finished and
2b needs its own line.

---

### §41 RUN 2d — LEVEL REFUTED, JITTER CONFIRMED, AND THE MECHANISM IS CLOSED

`axislev`, 21.4 s, zero brain runs. Both leftovers from run 2c, and they belonged in one build
because one is the control for the other.

#### THE CONTROL I OWED, AND IT PASSED ITS ORDERING PREDICTION

`axisjit`'s gain-0 clause could not fire. The falsifiable version is to freeze an axis that is
audible in **both** conditions — f2 — and measure F1, with no third resonator in play anywhere.
Jitter inflation then predicts a recovery, and predicts it is **larger** than freezing f3 bought
(+2.934), because f2 is both more audible (d' 9.47 vs 4.85) and more jittered (sd 57.8 vs 47.6 Hz).

```
freezing f2 at gain 0:   dd'^2  +34.590 +- 1.169   RECOVERS, and 11.8x the f3 figure
```

**Jitter inflation is confirmed on an independent axis**, with the ordering prediction — the part
that could have failed — satisfied. It is a real property of the code and not an artefact of the v83
renderer change. Note the size: freezing one axis's jitter nearly **doubles** F1's d'², which makes
off-axis motor noise the largest single thing standing between this creature and a bigger vocabulary.

#### THE LEVEL ACCOUNT IS REFUTED, AND MORE STRONGLY THAN I EXPECTED TO BE ABLE TO

I predicted the ruler would be **amplitude-blind**, which would have refuted level by deduction (a
mechanism the ruler cannot see cannot have produced a change it did see). **The prediction was
wrong** — and the truth gives a better refutation:

```
amp 1.0000   gain0  47.345   gain1  37.218   COST  -10.127 +- 0.324
amp 0.8165   gain0  49.920   gain1  39.479   COST  -10.441 +- 0.336
amp 0.6667   gain0  52.486   gain1  41.886   COST  -10.600 +- 0.339
amp 0.5000   gain0  56.144   gain1  45.253   COST  -10.890 +- 0.350
```

F1's d' **does** move with amplitude (+8.799 ± 0.278 over 2×), so the instrument is sensitive to
exactly the quantity the level story turns on. And f3's cost **at matched amplitude does not shrink**:
|cost| *grows* by 0.764 ± 0.181 (4.2 SE) as the volume halves — the wrong sign for level. **Turning
the volume down buys back nothing the third resonator took.**

**So the 71% is spectral masking:** F3's energy in its own band diluting F1's cepstral signature, not
the loudness of the sum. **There is nothing for a renderer renormalisation to fix**, and the mechanism
question that opened with run 2c is closed: **29% jitter inflation, 71% masking, 0% level.**

#### CLAUSE 3 WAS MIS-SPECIFIED AND PRINTED A CONFIDENT WRONG VERDICT

The first build of this clause compared the **best** gain-1 cell against gain 0 at amplitude **1.0** —
two cells differing in the gain *and* the amplitude. Clause 1 had established, three lines above it,
that amplitude moves F1 on its own by +8.80. So the clause credited the amplitude effect to f3
compensation and printed:

> *"compensation RESTORES F1, so the cost was level and the renderer's unrenormalised sum is the
> defect — a one-line fix."*

That is false, and **the same run's own numbers refute it**: the amplitude ladder moves gain 1 by
+8.04 and gain 0 by +8.80, i.e. identically, which is an independent additive effect and not a
compensation. This is the operating-point rule exactly — **a dose compared across a knob that
independently moves the readout is not compared at all** — and the rule was already in the method
notes. Caught at the desk before publication; the fix is to score the **paired cost at matched
amplitude**, which is what the table above does.

#### AN INCIDENTAL FINDING THAT IS BIGGER THAN THIS LINE, AND IS ONLY A LEAD

**Quieter is more discriminable.** F1's d'² rises monotonically 47.3 → 56.1 as amplitude falls by 2×,
which says the cochlea is compressing or saturating at amplitude 1.0 — and 1.0 is the operating point
this instrument has used throughout. If the creature's own utterances sit near there, then **every
measurement in this project that goes through the ear is being taken above the cochlea's best
operating point**, which would affect naming, echoing and teaching alike.

**It is a lead and not a result**, for a reason that has to be checked first: this instrument sets
amplitude by hand, while the creature sets it from vocal group 8's activity. **Nobody has measured
what amplitude the creature actually produces.** If it is well below 1.0 the finding is irrelevant to
the live system. That measurement is the cheap first step and it is not this run.

#### THE RUN ACCOUNTING, AND A PATTERN WORTH MORE THAN THE RESULT

Seven instrument faults today, all caught before publication except the units bug, which corrected an
already-published number:

1. §41's cross-talk clause — unfalsifiable.
2. `axisear` v1 — no denominator (d' of 41,514).
3. `axisear` v1 — a clause mis-derived from a wrong belief about which code the change touched.
4. `axisear` v1 — reading inside the estimator's floor at n=24.
5. `axisear` v2 — **d'² printed as d'**, which is the one that had already shipped.
6. `axisjit` — the gain-0 clause unfalsifiable, a repeat of (1), *after* (1) was written down as a rule.
7. `axislev` — clause 3 compared across an operating point, against a rule already in the notes.

**The uncomfortable pattern is that (6) and (7) each broke a rule this project had already written
down**, and (6) broke one written the same morning. The rules are not the missing thing; applying
them to the clause one is most confident in is. That is now the standing instruction in
[[aibaby-power-is-not-the-error-model]].

---

### §41 RUN 2e — THE WHOLE ARCHIVE MEASURED THE EAR 3.3x TOO LOUD, AND A VERDICT FLIPS

`axisamp`, 39.6 s, zero brain runs. The lead `axislev` opened, closed by executing its own step 1 —
which took no new instrument at all, because `babble` had printed the answer all along.

#### THE CREATURE'S EAR OPERATES AT 0.30, AND EVERY INSTRUMENT RENDERED AT 1.0

```
mean amplitude      0.446      (babble, 1.2M ticks, all motor frames)
mean amp if loud    0.607      (the 60122 of 120000 frames above the amplitude floor)
self_gain           0.50       (genome)
```

`audio.cpp` passes `float(v.amplitude) * self_gain_` to the larynx, so the product **is** the
amplitude at the ear: **0.607 × 0.50 = 0.303** when the creature is producing. Every `Timbre`
instrument in this archive — `axisear`, `axisjit`, `axislev` and everything older — renders at
**1.0**, about **3.3× too loud**.

**THE CONDITIONING MATTERS AND IS DERIVED, NOT PICKED.** Half the motor frames sit below the
amplitude floor and carry no utterance, so an "is this distinguishable?" claim must condition on
frames that *are* producing. The all-frames mean gives 0.223 at the ear and **a different answer** —
two axes cross the bar there, one crosses at 0.303. I added the conditional mean to `babble`'s output
rather than keep estimating it from histogram bin centres, because the verdict turns on it. (The
estimate was 0.609 against the measured 0.607, so it would not have misled — but a verdict should not
rest on a number read off a bar chart.)

#### THE LEAD IS REFUTED AND A PUBLISHED VERDICT FLIPS

**The "overdriven ear" lead is dead:** the creature already lives at the quiet end, so nothing in the
live system is being saturated. But the same fact puts today's own verdict at risk, and it does not
survive:

| axis | d' at 1.0 (archive) | d' at 0.303 (the creature) | |
|---|---|---|---|
| f1 | 6.881 AUDIBLE | 7.976 AUDIBLE | same |
| f2 | 9.469 AUDIBLE | 9.445 AUDIBLE | same |
| f3 | 4.852 AUDIBLE | 4.487 AUDIBLE | same |
| bw1 | 0.781 sub-bar | 0.930 sub-bar | same, and *just* short |
| **bw2** | **0.875 sub-bar** | **1.101 AUDIBLE** | **CROSSES** |
| bw3 | 0.588 sub-bar | 0.755 sub-bar | same |

**`axisear` v2's "REAL BUT SUB-BAR" was partly an artefact of the instrument's loudness.** At the
amplitude the creature actually hears, **bw2 clears the same 2AFC bar that called it sub-bar**, so the
audible axis count is **four — f1, f2, f3, bw2 — not three.** bw1 at 0.930 is close enough that it
should be treated as unresolved rather than refused.

**That is three successive corrections to one claim in one day:** "the bandwidths are inaudible
(~1% of F1)" → "weak but real, 10% of F1, sub-bar" → "**bw2 is audible at the creature's own
amplitude**". Each correction came from a different fault: units, then the instrument's operating
point.

#### WHAT THIS DOES AND DOES NOT DO TO THE REST OF THE ARCHIVE

**Between-arm contrasts are untouched.** Both arms always rendered at the same amplitude, so every
"arm A beats arm B by N SE" in this project is unaffected. That is most of what the archive contains.

**Every ABSOLUTE threshold claim is exposed**, and `d' >= 1.0 is ~76% in a 2AFC` is exactly one. Any
archived "is X audible / distinguishable" verdict was decided at 1.0 and needs the same re-reading.

**AND THE SENSITIVITY IS AXIS-SPECIFIC AND NON-MONOTONE, so there is no single correction factor and
no best operating point.** Across 1.0 → 0.12 the ear *trades* axes against each other:

```
amp     f0      f1      f2      f3     bw1     bw2     bw3
1.000  7.238   6.881   9.469   4.852   0.781   0.875   0.588
0.500  5.418   7.493   9.964   5.331   0.840   1.001   0.694
0.305  4.844   7.976   9.445   4.487   0.930   1.101   0.755
0.223  4.392   8.403   8.564   3.766   1.032   1.182   0.797
0.120  3.432   9.129   6.122   1.702   1.130   1.214   0.714
```

f1 and the bandwidths rise monotonically as it gets quieter; f0 falls monotonically; **f2 and f3 turn
over**, peaking near 0.5. So a single "quieter is better" is false.

#### CLAUSE 1 EARNED ITS PLACE BY REFUTING MY OWN EXTRAPOLATION

I had reasoned from `axislev`'s 1.0 → 0.5 measurement to a *per-halving rate* (+8.9% in d') and used
it in this experiment's own header to predict bw2 would clear the bar. **f2 turns over inside the
ladder**, so that rate does not extrapolate and the clause withdrew the 18% figure in the run's own
output. The verdict stands on the **measured rung** instead — which is why the test was run at the
creature's amplitude rather than computed from a slope. A two-point slope is not a rate.

#### THE PRIZE IS NOT YET RE-DERIVABLE, AND I AM NOT GOING TO MULTIPLY

bw2 clears **audibility**, at d' 1.101 — which is ~78% in a 2AFC, i.e. about **two** distinguishable
levels at best, not the ~2.6 the motor-side measurement gave. And adding bw2 would impose the same
two taxes this line just measured on f3: **jitter inflation** (its own motor noise becoming audible)
and **spectral masking**. Neither has been measured for bw2.

**So: the candidate axis count is 4, and the prize is NOT `19 × 2`.** Quoting a number requires
pricing bw2's tax first, and that is a new measurement, not an arithmetic step.

### §41 RUN 2f — THE FOURTH AXIS LANDS ON THE BAR, AND THE BANDWIDTH GAIN PAYS ANYWAY

`axisamp` left four **candidate** audible axes and an explicit refusal to multiply them: bw2's
d' 1.101 had been measured in a creature with **no third formant**, and a four-axis code ships one.
This run asks whether bw2 survives f3, and what bw2 costs in return.

**Pre-registered, as a number, before the run:** if f3 taxes bw2 the way run 2c measured it taxing F1
(21.4% of d'²), then 1.101² = 1.212 becomes 0.953 — **d' 0.976, below the bar.** bw2 survives only if
that tax comes in under 17.5%.

**Measured: f3's tax on bw2 is 19.3%, and bw2 lands at d' 0.990 ± 0.039.** The prediction was within
one SE of the measurement.

| | as rendered | at matched output level |
|---|---|---|
| bw2, no f3 | d' 1.103 | 1.070 |
| **bw2, with f3** | **d' 0.990 ± 0.039** | 0.970 |
| f3's tax on bw2 | −0.235 ± 0.036 | **−0.204 ± 0.038** |

The bar of 1.0 sits **inside** the error, so the honest verdict is **UNRESOLVED, and the usable count
remains THREE**. An axis within noise of its own audibility threshold cannot carry a vocabulary
claim. `f1 × f2 × f3 ≈ 19 signals` stands uncorrected, and §41's accumulation prize is not doubled.

The tax **survives level matching** at 5.4 SE, so it is not f3's extra loudness — `f3_gain` adds a
third resonator to an unrenormalised sum and does make the voice louder, but renormalising the
renderer would **not** give bw2 back. That matters because it closes the cheapest available fix.

#### AND THE THING THIS RUN WAS NOT LOOKING FOR: `render_bw_gain` IMPROVES THE AXES THE CREATURE ALREADY HAS

`axisear` priced the bandwidth gain at **−3.211 ± 0.245** on F1 — in a creature with f3 **off**. With
f3 shipped the sign flips, which that run's own factorial implied (B + AB = −3.211 + 4.255 = +1.044)
and which this run's header pre-registered as the likely branch. The measured effect is far larger
than the implied +1.044, and it holds across every formant:

```
                        d' at matched level        dd'^2
  f0    4.318 ->  5.465   (+26.6%)    +11.219 +- 0.365   HELPED
  f1    7.189 ->  7.521   ( +4.6%)     +4.882 +- 0.413   HELPED
  f2    8.308 ->  9.601   (+15.6%)    +23.150 +- 0.593   HELPED
  f3    4.299 ->  5.431   (+26.3%)    +11.016 +- 0.269   HELPED
```

**4 of 4 formant axes helped, after level matching.** The mechanism is clause 3's: the **static
widening** (+8.085 ± 0.497 on F1), not the bandwidths' own jitter, which is a small *real cost*
(−0.480 ± 0.069). The reading is that **the renderer's hardcoded 90 Hz bandwidth was too narrow for
its own cochlea** — a 90 Hz resonance sits inside a single mel channel, so a formant shift barely
moves the spectrum, while a ~149 Hz one spans several and a shift moves them all.

**This is a candidate genome change on its own merits, independent of bw2**, and it is the first
thing in this line that would *raise* a capability rather than price one. It is **not free**: it
moves the determinism hash and voids every cross-genome absolute
(`aibaby-vocal-calibration-invariant`), and `kVLScoreFormant` scores only F1/F2, so an ear-side d'
gain is **not** a naming gain until a naming task shows it. Pre-registering that is the next run.

#### THE LEVEL CONTROL WAS NOT OPTIONAL, AND IT MOVED A NUMBER BY 36%

Widening a resonator lowers its peak gain, so `bw_gain = 1` comes out **quieter** at the same
commanded amplitude (rms 0.1319 → 0.1167, −11.5%) — and `axisamp` showed this ruler *improves* as
level falls. The uncompensated bandwidth effect was **+7.605 ± 0.451**; at matched output level it is
**+4.882 ± 0.413**. So **36% of the headline was loudness**, and without the control this run would
have overstated a real effect by half again. `rms_of` was added to `Timbre` so the compensation is
**measured over the posture distribution the d' is measured over**, not derived at the setpoint
(`aibaby-match-at-the-operating-point`).

The uncompensated grid remains the primary reading, because the creature really does hear the
uncompensated level — that is where an absolute bar applies. The matched grid is what **attribution**
may use, and nothing else.

#### THREE FAULTS IN MY OWN INSTRUMENT, AND ONE OF THEM RETRACTS A PUBLISHED READING

**1. A branch that could only see one sign printed a false statement.** v1's clause 2 tested
`cost < -crit*SE` only, so a **+7.605 ± 0.451** result — a benefit at 16.8 SE — fell into the branch
whose text reads *"not resolvable at 2.447 × SE"*. The instrument asserted the opposite of its own
measurement. **Rule: a pre-registered quantity that is not sign-constrained needs a three-way
branch.** `aibaby-verdict-fitted-to-data` says to write refusal clauses as the code's branches; this
is the companion failure — writing them as branches that cannot reach the data.

**2. The `all off-axis frozen` arm is an artefact, and now its exact arithmetic is known.** Freeze
every axis but the target and the within-class noise becomes **rank one and collinear with the
signal**. `cepstral_dprime` is a *diagonal* Mahalanobis, so it divides each coefficient's class
difference by that coefficient's own marginal spread and **sums** — and when signal and noise share
one direction, every coefficient reports the same ratio. The sum returns `dims × (reach/sd)²` instead
of `(reach/sd)²`: the estimator counts one number `dims` times.

```
  axis   measured    dims*(reach/sd)^2   ratio
  bw2     347.260         347.280        1.000   <- five significant figures
  bw1     409.029         419.274        0.976
  bw3     392.900         384.478        1.022
  f1      317.803         353.953        0.898
```

Run 2c called this column *"near-degenerate"* and refused to quote it — the right instinct with the
**wrong diagnosis**: it blamed an SE a third of the value, i.e. noise. The column is not noisy, it is
**deterministic**, and its value can be written down from the reach and the sd with no ear involved.
**That also retracts run 2c's statement that the column's *direction* was informative.** A saturated
estimator has no direction. The off-axis burden is still real, but only arms where **two or more**
axes still jitter can measure it — run 2d's f2-only freeze is one, and it stands.

**3. A difference of two means quoted without an error bar.** v1 printed the jitter part as
`cost − static` with no SE. It is now its own paired contrast, and it turns out to be a *resolvable*
−0.480 ± 0.069 rather than the rounding error it looked like.

#### WHAT HELD

- **The guard reproduced run 2c exactly**: +2.934 at amplitude 1.0, same seeds, same draw order. This
  instrument is the one that produced the archive.
- **The wiring check read exactly 0**, and was *labelled a wiring check in the source before it ran* —
  at `bw_gain = 0` the renderer uses a literal `90.0f` and never reads the commanded bandwidths, so
  freezing them is bit-exactly inert and cannot fail for a measured reason. That is run 2c's fault
  (`aibaby-power-is-not-the-error-model`) disclosed in advance rather than discovered afterwards.
- **Clause 4, the control that *could* fail with every sign correct, held.** Freezing f3 beats
  freezing bw1+bw3 on F1 by **+5.888 ± 0.197**, the ordering jitter inflation requires — f3 is both
  more audible and more jittered than the two weak bandwidths.
- **Four instruments' duplicated jitter calibration was hoisted** to one `kAESd`. Four copies of a
  constant that must move together is what `aibaby-shared-constants-audit` warns about; editing one
  would have silently voided only part of §41.
- **`axisjit`'s result table printed its ± in d'² beside a value in d'.** Published yesterday. It
  changes no verdict — every clause used paired d'² with matching bars — but the table's error bars
  were mislabelled by the very units slip `axisear` v2 was built to fix. Converted:
  `se(d') = se(d'²)/(2d')`.

#### WHERE §41 STANDS

Stage 2a — does the air carry the new axes — is **finished**. The answer is that it carries f3 well
(d' 5.43 in the four-axis cell) and bw2 to exactly the threshold and no further. **Stage 2b, whether
the auditory *module* represents f3, is the last stage the count allows and is still unspent.** The
new `render_bw_gain` lead is **not** part of §41: it is an input-side renderer change with its own
pre-registration owed, and it does not consume §41's budget.

#### CORRECTED THE SAME DAY — THE BANDWIDTH LEAD HAS NO ROUTE TO BEHAVIOUR, AND IT COST NO RUN

The section above calls `render_bw_gain = 1` "a candidate genome change on its own merits" and "the
first thing in this line that would RAISE a capability rather than price one." **Both are wrong, and
four greps were enough to show it.**

**1. The naming reward never consults the ear.** `experiments_milestones.cpp:5401` computes the
formant error as `formant_error_axis(double(v.f1), double(v.f2), ...)` — an **oracle on the larynx
command**. Every naming result in this archive is driven by a teacher-supplied scalar on the motor
parameters. Improving the cochlea's resolution of formants cannot move it by one bit.

**2. `render_bw_gain` reaches nothing else.** `VowelSource` has exactly one consumer outside the
experiment layer: `Ear::larynx_` at `audio.cpp:119`, the **self-audition** render, scaled by
`self_gain`. The field is read once, in `Ear::configure`. So its entire causal footprint is what the
creature hears of itself.

**3. And that path is a measured dead null.** `aibaby-selfloop` altered the creature's auditory
feedback and got a coherent response of **+0.01 Hz ± 0.23** at the shift frequency, at both
timescales. `aibaby-aimhold` handed over a *perfect* F1 error signal and bought reach but no aim,
concluding: *if a perfect controller cannot improve precision, no learned one can.*

So the +4.882 ± 0.413 is real, and it is a real improvement to the fidelity of a channel with **no
route to behaviour**. It is an ear-side fact, not a capability. The honest status is **the
measurement stands, the ship argument is withdrawn** — if the self-audition line is ever re-opened
this is a lever waiting there, and `aibaby-aimfloor` is the memory that would re-open it.

**WHAT THIS COST AND WHAT CAUGHT IT.** It cost nothing — no run was built. What caught it was asking
*what must be true of the architecture for the effect to reach behaviour*, which is the rule from
`aibaby-power-is-not-the-error-model` applied to a LEAD rather than to a clause. **I did not apply it
before writing the lead up as a capability candidate, which is the same omission in a new place.**
And the memory that refuted it — `aibaby-aimhold` — had been **unreachable from the memory index**
until an hour earlier.

#### AND IT SHARPENS WHAT §41 ACTUALLY MEASURED

`f1 x f2 x f3 ~ 19 signals` is a **motor** capacity, scored by an oracle that reads the motor
command. Today's runs measured the **cochlea's** resolution of the same axes. **Nothing has measured
whether the auditory MODULE can tell the creature's own words apart** — and "recognize WORDS" is what
the goal asks for. That, not another d', is what stage 2b should be aimed at.

### THE ABSOLUTE-THRESHOLD AUDIT — THE LIABILITY DOES NOT EXIST, AND THE ARCHIVE GOT THIS RIGHT FIRST

`axisamp` recorded that "every absolute threshold claim in this archive is exposed" because the
creature hears itself at 0.303 while "every `Timbre` instrument renders at 1.0". **Both halves of
that are wrong, and enumerating the call sites was enough to show it.** Every `Timbre` amplitude in
the archive:

| site | amplitude | what it is | exposed? |
|---|---|---|---|
| 1513, 3412 | `r.amplitude / n` | the creature's own recorded mean | **no** — correct by construction |
| 1587–1598 | `mamp`, plus [i]/[a]/[u] anchors at the *same* amplitude | own mean, self-calibrating | **no** |
| 6209 (`teachsound`, M1b's ruler) | `a_acc / n_voiced` | the **voiced-frame conditional mean** | **no** |
| 7463, 7467, 29440, 30793–4 | hardcoded `0.4` | contrasts only — taught vs control, first vs last | **no** — shared amplitude |
| 45994 (`axisear`), 46386 (`axisjit`) | **`1.0`** | §41 | **yes — and already re-measured** |
| 46622, 46888, 47145 | swept, including 0.303 | §41 runs 2d/2e/2f | no |

**Two instruments out of ten ever rendered at 1.0. Both are §41's, both were written in the last two
days, and `axisamp` already re-measured their entire ladder** — finding exactly one crossing, bw2,
which `axisbw2` then settled at 0.990 ± 0.039. There is no archive-wide re-reading to do.

**And the external regime was never at 1.0 either.** Caregiver words render at **0.5** in every
caller (`voice.render(w.f0, w.f1, w.f2, 0.5f, ...)`), and `struct Word` carries only `f0, f1, f2` —
no F3, no bandwidths. So no caregiver-word claim can touch the new axes, and at 0.5 the three it does
touch read **5.418 / 7.493 / 9.964** against a bar of 1.0. Five to ten times the margin.

**ONLY ONE PRE-§41 ABSOLUTE CLAIM EXISTS AT ALL** — `dpu >= 1.0` at line 1835, the cube-vs-ball
audibility call, and M1b's 1.37 at line 6302. Everything else in the archive is a contrast at a
shared amplitude, which was never at risk.

#### THE PART WORTH KEEPING: THE OLD INSTRUMENT IS BETTER THAN THE ONE I WROTE

The instrument making the oldest absolute audibility claim does two things §41's did not:

1. **It conditions on voiced frames** — `a_acc / n_voiced`. That is precisely the conditioning I
   "derived" in run 2e as though it were a new idea, complete with a new `babble` print to measure
   it. It was already there, in the instrument whose claim it protects.
2. **It carries vowel anchors at its own amplitude** — `[i]` vs `[a]` ("two vowels nobody would
   confuse") and `[a]` vs `[u]`. So its absolute bar is calibrated *in situ*: if the amplitude were
   wrong, the anchors would be wrong too and would say so. §41's instruments had a fixed 1.0 and no
   anchor, which is exactly why they could drift without anything noticing.

**So the fix for §41's instruments is not a different amplitude — it is an anchor.** An absolute bar
needs a reference measured through the same tract at the same level, or it is a number floating free.

#### AND THIS IS THE THIRD TIME TODAY

`aibaby-cochlea-operating-point` generalised from the two instruments in front of me to an archive I
had not enumerated. That is the same error as reading a memory's headline instead of its body
(twice today) and as asking the architecture question of a run's clauses but not of its conclusion
(once today). **One shape: asserting of the whole what was checked of the part.** The cost each time
has been a few greps; the cost of not doing them was a published liability that would have made every
future session distrust every threshold in the project.

### THE ROTATION-DECLINE LINE, PRICED BEFORE IT WAS RUN — AND THE PRICE REFUSES IT

Picked up as the live frontier. The pre-flight is free (both numbers are already published) and it
withdraws the run rather than scoping it.

#### STAGE 0: THE PEAK-LOCATION DATA CARRIES ALMOST NO INFORMATION

`dirbudget` reports per-creature argmax rungs of **4 / 12 / 12 / 8** at 3.4M / 6.8M / 13.6M / 27.2M.
Against uniform that is **χ² = 4.89 on 3 df, p = 0.18** — indistinguishable. **Eight of thirty-six
creatures still do best on the largest budget.**

Stated carefully, because this is a caution and not a refutation: a *true* common peak with noise
would also spread the argmax, so p = 0.18 does not show there is no peak. What it shows is that the
peak-location data cannot locate one, and the paired-step statistic is doing all the work.

#### STAGE 1: THE POWER, AND IT DEPENDS ON WHICH CLAIM IS BEING MADE

Two different effects are in play and they are not interchangeable:

| claim | effect | SE at n=36 | n for 2.447 SE | n for 4 SE |
|---|---|---|---|---|
| `excess` declines (published) | −0.0244 | 0.0092 | 31 (already there, 2.65 SE) | **82** (×2.27) |
| `direction` itself declines | **−0.0158** | not published | **74** (×2.0) | **196** (×5.4) |

The second row is the mechanistic claim — *the creature's mapping degrades* — and `decline-tracks-ticks`
established that **about a third of the published decline is the creature's own chance level rising,
not its direction falling.** Subtracting the null stays correct, so `excess` is the right statistic;
but anyone reasoning about a *mechanism* is reasoning about −0.0158, which at n=36 is not resolved
even by this project's lenient 2.447 convention.

At the predecessor's measured 6–7 h per sample, 4 SE on `excess` is **~15 h** and 4 SE on `direction`
is **~35 h**.

#### AND STAGE 2 HAS NO CANDIDATE AT ALL

Explaining the decline needs the larynx threshold moved **within** a creature without moving its
learning. `ipoff` cannot: it destroys learning at **−7.42 SE**. No other knob has been identified.
So the sequence is 15–35 h to find out whether a 2.6 SE hypothesis survives, and then a causal
question with **no known manipulation**.

#### THE REFUSAL

`dirbudget`'s own "how to cite" section says it first: *"the 2.6 SE decline is a HYPOTHESIS, not a
finding… do not promote it because it is interesting. Three 2-3 SE results have been retracted here."*
Spending 35 hours to resolve a −0.0158 effect that a quarter of creatures do not show, in order to
reach a causal question with no candidate manipulation, is poor value. **The line's own closing
paragraph was right: "that is a design problem, not a budget problem, and it is the honest end of
this line until someone has one."** This section is the arithmetic behind agreeing with it.

**What would re-open it:** a within-creature threshold knob that leaves learning intact. That is the
single missing piece, and it is worth more than any amount of n on the existing design.

### THE CONDITIONING CLOSURE IS RE-DERIVED ON TODAY'S CREATURE, 52 GENOME REVISIONS LATER

Two method notes in this archive say to re-derive a standing null rather than re-explain it, and
`g3-synthesis`'s own last buildable lead died because it was aimed at a cap that had closed
underneath it: *"before building against a bottleneck, re-measure that the bottleneck is still there
on the current creature."* The conditioning refusal — the number that closes this project's central
line — was from **2026-08-19**, and `dna/default.toml` has changed **52 times** since. `restate` pins
eight numbers and that is not one of them. Nobody had executed the instruction.

Executed now. 900k ticks, 5 creatures × 2 arms, 141 probes each, ~13 min per instrument.

| instrument | taught | random | paired | signs |
|---|---|---|---|---|
| `g3probe` (teaching: object only) | 0.5354 | 0.5070 | +0.0284 ± 0.0359 (0.8 SE) | 3u / 1d |
| `pairprobe` (teaching: object **and word**) | 0.5294 | 0.5266 | +0.0028 ± 0.0210 (0.1 SE) | 1u / 3d |
| **published 2026-08-19** | 0.637 | 0.639 | **−0.002** | — |

**`taught − random` is zero on both instruments.** The refusal replicates. An ideal teacher — dense,
immediate, naming the object — still cannot make this voice depend on what the creature sees.

**And the heard word as a pairing cue buys nothing.** That is the one thing the two instruments
differ in, and `pairprobe − g3probe` on the taught arm is **−0.0060** against a published noise floor
of **0.115**. So adding the word to the teaching window does not help the mapping form.

#### WHAT MAY NOT BE READ FROM THIS

**The LEVELS moved, 0.64 → 0.53 on both arms, and that is not readable.** The instrument prints its
own warning — two probe sequences differ by ~0.05 at 300 trials — and this ran 141, so the caveat is
larger than at 300. The comparable quantity is the **within-run paired contrast**, which is what the
table above compares and what the claim rests on. A level shift across instruments is exactly the
kind of thing this project has been burned by, so it is recorded and not interpreted.

#### THE LOCALISATION IS NOW TRUSTWORTHY, AND IT WAS NOT BEFORE

`g3probe` reads **0.54 against a 0.75 bar**, and the same features and classifier take the
**word-driven** voice at **~0.83** — so the readout is not the limit and the pipeline is not broken.
The condition table localises the loss **downstream**: vision 0.927 → central 0.724 (−0.203), then
central → voice (**−0.237**). The log states why this matters: *"at the default tick count this same
table read central at 0.475 and pointed upstream instead; that estimate was resolution-limited, not
a finding."* Running at 900k rather than the default is what makes the direction readable.

#### SO THE DECISION IS ARCHITECTURAL, AND THAT IS NOW A CURRENT STATEMENT

The creature recognises words at **0.981** (1.000 is the signal *ceiling*, not the reading —
corrected 2026-11-04), repeats them, is taught sounds, and names four words. It
cannot bind what it **sees** to what it **says**, under an ideal teacher, with the only learning
rules it has: node perturbation structurally cannot (a per-neuron bias is a constant, not a function
of the input) and R-STDP on central→vocal does not manage it even with ideal reward.

**`g2cond` was NOT run.** It is 3.4M ticks at teach tier, ~2–3 h, and it would measure the same null
a third time. The cheap probe already answered the question it would have asked — which is the
general form of today's lesson: **price the question before funding it.**

## THE OWED DEDUP, AND WHAT A SHORT RUN CANNOT GATE

Done 2026-11-04, after `docs/CONCLUSION.md`. `echoname` and `exsweep` each carried an inline copy
of the separability decomposition — the within/between variance split pooled over the articulator
dimensions. The source note said the dedup had to be gated by `tools/echoname_identity.sh`
against `results/echoname-audit-full.log`, and **that gate was already wrong when it was written**:
the baseline predates eight commits to `core/` and the genome (v73–v76, v82, v83), so a
character-identity check against it would have failed for reasons with nothing to do with the
change. The note had been corrected the day before to name a cheaper and stronger gate; this is
that gate executed.

**The helper also had to move.** `artic_spread` sat in an anonymous namespace **2,700 lines after
its caller**, so a plain call was not possible. It is now in `experiments_common.h` beside
`code_stats`, the other struct-and-function statistic, which removes the ordering problem instead
of relocating it. **157 lines of duplicated arithmetic left `experiments_milestones.cpp`.**

**THE GATE, AND IT IS TWO HALVES BECAUSE ONE OF THEM IS UNREACHABLE FROM A RUN.** Both
implementations are pure deterministic functions of (rows, labels, words), so what a dedup can
break is the **float accumulation order** — compared with `memcmp` rather than a tolerance, since a
tolerance would pass the one failure being looked for.

    bit-equality on real rows   51 of 51 cells, all four values to 17 significant digits
    structural edge cases        9 of 9  (tools/artic_spread_edges.cpp, self-checking)
    verify on the final tree     PASS, hash ad96f882becbee92, 24 as expected / 0 not

**Every one of those 51 cells took the same branch**, which is why the second half exists. The two
copies agreed on every ordinary creature and disagreed in exactly one place: when every scored row
is identical within its word, `within` is 0, the ratio is undefined, and `echoname` published the
partial decomposition with `sep` 0 while `exsweep` published nothing. **That is not a theoretical
branch** — the taught arm runs at a voiced fraction of 0.082 and an all-zero articulator row is
what silence looks like. So `ArticSpread` carries two flags, `decomposed` and `ok`, each caller
guards on the one that preserves its own published output, and the edge test pins all nine
contracts including that one.

**THREE THINGS THIS RUN GOT WRONG BEFORE IT GOT THEM RIGHT, all caught by reading output rather
than trusting it:**

1. **The gate run did not finish and said it had.** `timeout` killed the binary at 900 s after 51
   of 180 cells — **exit 124, masked by the compound command's own exit 0**, visible only in the
   `echo "exit: $?"` line. A truncated run that prints no table looks like a run with nothing to
   say. **Read the status of the binary, not of the pipeline.**
2. **`ctx.toml` was six weeks stale** and missing ten keys added since (`rate_fast_tau_ms`,
   `self_delay_ms`, the six `jaw_*` fields). Regenerated rather than repaired, to a scratch path
   so the owner's file was left alone — and this is the same staleness the note predicted for the
   published log, in a second place nobody had checked.
3. **A third copy was suspected and is not one.** The per-word mean accumulation near
   `experiments_milestones.cpp:31016` is a nearest-centroid one-of-eight classifier with a
   train/test split on halves, sharing only a generic idiom. The decomposition's distinctive
   denominators now appear **exactly once** in the codebase, with two call sites.

**What was NOT done, and why.** A full `echoname` would be 1–2 h and would produce a table with
nothing to compare it against, because the only published baseline is the stale one that made the
original gate invalid. Per-cell bit-equality is the stronger statement anyway: it is 51
independent comparisons at full precision rather than one aggregate.

## §42 — THE DENOMINATOR: WHAT SETS THE CREATURE'S OFF-AXIS MOTOR SCATTER? (2026-11-05)

**Pre-registered before the run. No build: the measurement already exists in `axisgrid`'s output
and has never been read as a dose-response.**

### WHY THIS AND NOT ANOTHER AXIS

§41 closed the axis *count*: three axes reach the air, `f1 x f2 x f3 ~ 19 signals`, and the fourth
(bw2) lands inside the error bars of its own audibility bar. The line's stopping rule is spent.

But §41 run 2d measured something larger **in passing**, and §41 never picked it up because it was
a positive control for a different question. Freezing **one** off-axis parameter — f2, held at rest
instead of jittered — recovers **+34.590 ± 1.169** of F1's d'², against F1's shipped d'² of ~47.
**Freezing one axis nearly doubles the discriminability of another.** Its own note says it:
*"off-axis motor noise is the single largest thing between this creature and a bigger vocabulary"*.

Levels per axis are `swing / scatter` and the code space is the product of levels, so there are two
ways to grow a vocabulary: add axes to the numerator, or take scatter out of the denominator. §41
spent three runs and a genome revision on the numerator and got 2.7x. **Nothing has ever asked what
sets the denominator.** This section asks.

### THE ROUTE TO BEHAVIOUR, ASKED OF THE LEAD AND NOT OF THE CLAUSES

`render_bw_gain` was measured and then **withdrawn as a ship argument the same day**, because its
only consumer is `Ear::larynx_` — self-audition, a path with a measured dead null to behaviour. The
rule that miss produced is to ask what the architecture must make true for the *lead* to reach
behaviour, before designing the run. Asked here:

Scatter is a property of what the creature **produces**, so it is read by every audibility
measurement in the project (`imitate` d' 1.37, `teachsound` d' 5.57), by every held-out classifier
on the voice (`echoname` echo 0.955, `orthoname` 0.958), and by the transmission tape a pupil hears
(`tutorgrid`, 18/18 on both bits). **It does not reach the naming reward**, which is an oracle on
`v.f1`/`v.f2` (`experiments_milestones.cpp:5401`) and never consults the ear. So the claim §42 can
support is about **what a listener or a pupil can recover from the voice**, not about the learning
signal. Written down now so it cannot be inflated later.

### THE INSTRUMENT, AND THE ONE COLUMN THAT IS SAFE TO READ

`axisgrid` already prints, at the **zero rung**, the rest point and the **within-run scatter** of
all nine larynx parameters. At that rung `kAXLadder[zero] == 0.0` and the code skips
`set_bias_oracle` entirely, so the column is **dose-free**. No build, no new instrument, no gate.

**AND THAT IS NOT A CONVENIENCE — IT IS THE ONLY SAFE COLUMN.** `axisgrid` sets its dose as
`kAXLadder[L] * namp`: **the ladder is denominated in `noise_amp` itself.** A `noise_amp` ladder
read off the *swing* columns would lower the dose in exact proportion to the knob, swing and scatter
would fall together, levels would read flat, and §42 would close on a purely mechanical artefact
that would not have looked wrong in the output. Caught at the desk for zero runs. The swing half of
this question therefore needs a **pinned absolute dose** and is stage 2, not stage 1.

### THE LADDER

Vocal `noise_amp` ∈ **{0.28, 0.22 (shipped), 0.16, 0.11, 0.06}**. **Two-sided**, because a
down-only ladder cannot distinguish *"scatter is `noise_amp`"* from *"scatter has a floor"*. Only
`dna/default.toml:1928` moves; the other five modules sit at 0.28 and must not.

### CLAUSE ORDER — LIVENESS FIRST, FOR `axisgrid`'S OWN STATED REASON

1. **LIVENESS.** The genome records a measurement at `exploration.sensitivity 500` where **five of
   nine creatures produced too few vocalisations to score**, because in this creature the motor
   module's spontaneous drive *is* its babble: closing exploration makes the larynx **silent, not
   stereotyped** — and a silent larynx has low scatter trivially. `amp` and `voi` are in the same
   table (shipped **0.444** and **0.475**). A rung whose `amp` falls below **0.222**, half the
   shipped value, is **NO VERDICT** and not a scatter result. `calibrate` must **PASS** at every
   rung: it is this project's pre-calibrated detector of a moved operating point, which broke at
   `threshold_sigma 0.04` and held to `refractory_sigma 0.60`.
2. **REPRODUCTION.** Rung 0.22 **is** the shipped genome, so it must return `results/axisgrid-v1.log`'s
   zero-rung table exactly — f1 sd **21.727**, f2 **57.751**, amp **0.444**. If it does not, the
   harness is wrong and nothing else in the run is readable.
3. **DOSE — THREE-WAY, because the question is not the sign but the exponent.** Fit
   `d log sd(f1) / d log noise_amp`:
   - **≈ 1** — scatter *is* `noise_amp`, proportionally. The lever is real, and stage 2 is justified.
   - **strictly between 0 and 1** — there is a floor: something else sets most of the scatter, and
     the exponent **bounds what the knob can ever buy**. This is the outcome that most changes the
     plan, because it prices the lever instead of opening or closing it.
   - **≈ 0** — `noise_amp` is not the scatter source. §42 closes here and names its successor: the
     dense ears→larynx tract, which the genome comment at `[[module]]` vocal already blames for
     swamping the reward signal at the motor module.
4. **SELECTIVITY.** `noise_amp` is **per module, not per group**: `network.cpp:633` fills
   `noise_amp_[i]` from `dm.noise_amp` for every neuron of the module, and the nine larynx
   parameters are **disjoint index slices of that one module**. So the prediction is that all seven
   axes move **together** — and that is what would make the interesting object a knob the kernel
   **does not have**: a per-group noise scale. If one axis moves differently from the others, this
   prediction is wrong and the group architecture is doing something unmeasured.

### ABANDONMENT COUNT: THREE RUNS

Stage 1 is this one. **Stage 2** (swing at a pinned absolute dose, so `levels` can be read) only if
clause 3 reads above 0. **Stage 3** (does a per-group scale buy audible separation on a real task)
only if stage 2's levels move. A refusal at any stage ends §42.

### HONEST DISCLOSURE BEFORE THE RUN

The scatter `axisgrid` measures is **within-run** — the variability of a held posture across ticks.
What a vocabulary is actually bounded by is **between-trial** spread, which is **unmeasured**; §41
note 2 already flags this as *"wrong ruler both ways"*. So a dose-response here is a statement about
the knob's effect on motor variability, and the step from that to a word count still needs the
between-trial measurement §41 never made. §42 does not claim that step.

### §42 AMENDMENT — `calibrate` IS NOT A LIVENESS GUARD HERE, AND THE SMOKE SAID SO

**Written after the smoke and the calibrate ladder, before any scatter number exists.**

Clause 1 named `calibrate` as a guard on liveness. **It is not one for this ladder, and the reason
is printed in its own output.** Run across all five rungs:

| vocal `noise_amp` | vocal free-running rate | amplitude floor | verdict |
| --- | --- | --- | --- |
| 0.28 | 6.55 Hz | 0.45 | `calibrate PASS` |
| **0.22 (shipped)** | **4.12 Hz** | 0.45 | `calibrate PASS` |
| 0.16 | 2.10 Hz | 0.45 | `calibrate PASS` |
| 0.11 | 1.20 Hz | 0.45 | `calibrate PASS` |
| 0.06 | **0.62 Hz** | 0.45 | `calibrate PASS` |

**A 10.6x collapse in the rate of the module being manipulated, and the guard passes every rung.**
`calibrate` compares each module's free-running rate against its genome target and prints, for
vocal and vocal only, `by design — holds the duty cycle down`. The one module whose rate this
section moves is the one module `calibrate` **excuses by construction**. The amplitude floor is
0.45 at every rung and is likewise blind to it.

This is [[aibaby-guard-measures-the-wrong-quantity]] again, and it is the shape that memory counts
as six in seven runs — **two of them installed by the previous fix.** Pre-registering a guard is not
the same as checking that the guard can fire. Cost here: one smoke, ninety seconds, before the
ladder rather than after it.

**THE SURVIVING GUARD IS THE ONE IN `axisgrid`'S OWN TABLE**, which clause 1 also named: `amp` rest
**0.444** and `voi` rest **0.475** on the shipped genome. A rung whose `amp` falls below 0.222 is
NO VERDICT. That guard is unaffected by this amendment, so clause 1 still has a firing branch —
but it now has exactly one, where it was written as if it had two.

**AND THE CALIBRATE LADDER IS A RESULT IN ITS OWN RIGHT, two ways:**

1. **The 0.22 rung returns 4.12 Hz**, which is the archive's published shipped vocal rate to the
   printed precision. Clause 2's reproduction check passes on the calibrate side before `axisgrid`
   has reported anything.
2. **The rate is supralinear in the knob**: `d log rate / d log noise_amp = 1.53` over the full
   ladder. That is the quantity clause 3 must be read **against**. If `sd(f1)` falls with an
   exponent at or below the rate's, the scatter may be falling because the creature is *firing
   less*, not because its motor noise per unit of behaviour is lower — **"less behaviour" and "less
   noise" are not the same finding**, and the shipped genome's own comment says which one to expect
   (*"in this creature the motor module's spontaneous drive is its babble"*). So clause 3's three-way
   split is now read as a **ratio of exponents**, and a scatter exponent near 1.53 is the *null*,
   not the result. This is a stricter test than the one pre-registered, and it is stricter in the
   direction that could refuse the lead.

### §42 PREDICTION, WRITTEN ON RUNG 1 OF 5 AND TESTABLE BY THE OTHER FOUR

**The first rung inverts the expected sign, so the prediction is recorded now, while four rungs are
still in flight and cannot be used to fit it.**

Rung `noise_amp = 0.28` — **above** the shipped 0.22 — against the archive's shipped zero rung:

| | f0 | f1 | f2 | f3 | bw1 | bw2 | bw3 | **amp** | **voi** |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| sd at 0.28 | 7.619 | **18.146** | 47.916 | 38.768 | 4.880 | 5.090 | 4.748 | 0.164 | 0.377 |
| sd at 0.22 (shipped) | 9.129 | **21.727** | 57.751 | 47.616 | 5.346 | 5.967 | 5.618 | 0.158 | 0.482 |
| rest at 0.28 | — | — | — | — | — | — | — | **0.588** | **0.812** |
| rest at 0.22 | — | — | — | — | — | — | — | **0.444** | **0.475** |

**MORE motor noise gives LESS scatter on all seven axes** — f1 down 16.5%, f2 down 17.0% — while
amplitude rises 32% and **voicing rises from 0.475 to 0.812**. The naive reading of §42 ("turn the
knob down to take scatter out of the denominator") has the sign exactly wrong, and the direction is
not a small effect.

**THE MECHANISM I THINK THIS IS, AND IT IS A SECOND CONFOUND IN THE SAME INSTRUMENT.** `axisgrid`
accumulates `p[k]` on **every tick** after the settle half, with **no voicing gate**
(`experiments_milestones.cpp` ~45624). So its "within-run sd" is the wander of a parameter across
voiced *and unvoiced* ticks. A creature voiced 47.5% of the time crosses in and out of voicing
constantly and its parameters wander more; one voiced 81.2% of the time holds a steadier posture.
**That makes this scatter a DUTY-CYCLE statistic as much as a motor-noise one** — and `noise_amp`
moves the duty cycle, which is why the sign inverts.

**It matters beyond §42 because the constant is shared.** `kAESd[7]` (in `experiments_milestones.cpp:46042` — §42 said `experiments_common.h`, corrected by §43) is
exactly this ungated scatter from §41 run 1, and **four instruments** read it — `axisear`,
`axisjit`, `axislev`, `axisamp`. If it is partly a duty-cycle number then the within-class
denominator of every d' in §41 is partly a duty-cycle number. §41 note 2 half-saw this — *"a held
vowel is scored over many ticks (overstates the noise)"* — without connecting it to voicing.

**THREE PREDICTIONS, and the four remaining rungs are a clean test because they are already running
and I cannot see them:**

- **P1.** `sd(f1)` is **monotonically decreasing in `noise_amp`** across all five rungs — i.e. it
  *rises* as the knob falls, the opposite of the naive expectation.
- **P2.** `voi` is **monotonically increasing in `noise_amp`** across all five rungs.
- **P3.** The rank correlation between `sd(f1)` and `voi` across the five rungs is **−1**. This is
  what a duty-cycle artefact predicts; a per-tick motor-noise mechanism predicts the *opposite* sign
  on P1 and no particular relation on P3.

**WHAT EACH OUTCOME MEANS, fixed in advance:**

- **P1 ∧ P2 ∧ P3 hold** → the instrument's scatter is a duty-cycle statistic. §42 stage 1 answers
  its question in the negative *and* turns up a defect in a constant four instruments share. Stage 2
  becomes a **voicing-gated** scatter, which is a small build on a read-only path, and §41's d'
  numbers need re-reading rather than re-running.
- **P1 fails (non-monotonic)** → neither story holds, and the 0.28-vs-0.22 inversion is something
  else; §42 reports it as unexplained rather than naming a mechanism.
- **P1 holds, P3 fails** → scatter does rise as the knob falls, but not with voicing, so there is a
  per-tick mechanism after all and it has the sign the naive reading wanted. §42 continues to
  stage 2 as originally written.

**AND CLAUSE 2 IS NOT YET SATISFIED.** The 0.22 rung of this ladder has not run; the comparison
above is against `results/axisgrid-v1.log`. The ladder's own 0.22 rung is byte-identical to
`dna/default.toml` (diff = 0 lines, checked), so it must return f1 sd **21.727** and amp **0.444**
exactly. **Until it does, every number in this section is provisional** and the inversion could still
be a harness difference rather than a creature difference.

### §42 CLAUSE 2 PASSES, AND THE CODE CORRECTS MY OWN MECHANISM CLAIM

**Clause 2 is satisfied.** The ladder's own `0.22` rung reproduces `results/axisgrid-v1.log`'s
zero-rung table **byte-identically** (`diff` empty over the whole block). So the 0.28-vs-0.22
inversion is a **creature** difference, not a harness difference, and the provisional flag on the
previous section is lifted.

**AND THE MECHANISM IS NOT QUITE THE ONE I COMMITTED TWELVE MINUTES AGO.** I wrote that the ungated
scatter was inflated by *"crossing in and out of voicing"*. Reading `VocalDecoder::update`
(`core/src/senses.cpp:655-666`) says otherwise:

    params_.f1 = target_f1;        // set on EVERY awake tick, unconditionally
    ...
    // Voicing is a gate, not a dial, so it is not smoothed.
    params_.voicing = group_activity_[1] > Scalar(cfg_.voicing_threshold) ? kOne : kZero;

**The formants are live on every awake tick and no branch freezes or resets them when unvoiced.**
On a silent tick `v.f1` is a genuine articulatory posture — the tongue is somewhere, it simply is
not phonating. So nothing is corrupted by a transition. The defect is simpler and worse:

**`axisgrid` counts posture wander that occurs during silence, which no listener ever hears.** A
creature voiced 0.475 of the time spends the other 0.525 moving its articulators **inaudibly**, and
that movement sits in the within-class denominator of every d' in §41. One voiced 0.812 of the time
spends less of itself wandering unheard. The quantity a listener actually experiences is the
variability of the posture **while it is audible**, and that is exactly what the ungated statistic
does not measure.

**`voi` IS THE DUTY CYCLE, CONFIRMED ARITHMETICALLY.** Voicing is binary per tick, so its mean *is*
the voiced fraction and its sd must be Bernoulli: predicted `sqrt(p(1-p))` = 0.499 at p = 0.475 and
0.390 at p = 0.812, measured **0.482** and **0.377**. The table is internally consistent with the
code.

**THIS CHANGES WHAT STAGE 2 MEASURES, AND GIVES IT A TWO-WAY ANSWER.** The gate is already an idiom
in this codebase — `v.voicing > 0.5f && v.amplitude > kAmplitudeFloor`, used at eight sites in
`experiments_milestones.cpp` — so stage 2 is a small read-only change, not a new instrument. And it
**separates two effects the ungated number conflates**:

- **gated scatter flat across the ladder** → the entire inversion was the gate, §41's shared `kAESd`
  is measuring unheard movement, and the fix is to re-read §41's d' numbers rather than re-run them.
- **gated scatter also falls as `noise_amp` rises** → there is a *real* steadiness gain on top of
  the gating artefact. The decoder applies inertia to the motor **command**, so a creature that
  phonates more continuously may genuinely hold postures longer. That would be a lever, with the
  sign the naive reading of §42 had backwards.

P1–P3 are unaffected by this correction: both stories predict all three. What changes is what a
confirmation **means**, which is why it is recorded before the four rungs land rather than after.

### §42 RESULT — THE SCATTER IS A DUTY CYCLE, AND THE LEAD THAT OPENED §42 IS NOT SUPPORTED (2026-11-05)

`results/n42-axisgrid-namp0{28,22,16,11,06}.log`, `results/n42-babble-seeded.log`,
`results/n42-calibrate-ladder.log`. No build, no genome shipped, pinned hash untouched.

| vocal `noise_amp` | f1 swing | **sd f1 ungated** | **levels ungated** | **sd f1 gated** | **levels gated** | `voi` | `amp` | guards |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0.28 | 363.6 | 18.146 | **20.04** | 22.33 ± 1.71 | **16.28** | 0.812 | 0.588 | 6/6 PASS |
| **0.22 shipped** | 372.2 | 21.727 | **17.13** | 25.17 ± 2.87 | **14.79** | 0.475 | 0.444 | 6/6 PASS |
| 0.16 | 377.9 | 27.675 | **13.66** | 25.67 ± 1.48 | **14.72** | 0.159 | 0.298 | 6/6 PASS |
| 0.11 | 368.0 | 38.784 | 9.49 | 23.83 ± 1.47 | 15.44 | 0.041 | 0.198 | **NO VERDICT** |
| 0.06 | 289.8 | 46.582 | 6.22 | 24.67 ± 2.97 | 11.75 | 0.012 | 0.133 | **NO VERDICT** |

Gated spread is `babble`'s voiced-gated F1 sd, **6 seeds per rung**; ungated is `axisgrid`'s zero
rung, 12 creatures. Levels are `swing / sd` in each column's own units.

**P1, P2 AND P3 ALL HOLD, STRICTLY AND ON ALL FIVE RUNGS.** Ungated `sd(f1)` is monotone decreasing
in `noise_amp` (18.1 → 46.6); `voi` is monotone increasing (0.012 → 0.812); their rank correlation
is **−1**. That is the branch whose reading was fixed in advance.

**AND THE GATED LADDER SETTLES IT.** Over the three scoreable rungs the **ungated** levels fall
**32%** (20.04 → 13.66) while the **gated** levels move **1.56 ± 1.51, i.e. 1.0 SE — flat.** The
audible resolution of the voice does not change across a ladder on which the ungated statistic
changes by a third.

**SO §42 ANSWERS ITS OWN QUESTION: NO.** `noise_amp` does not take scatter out of the denominator
that matters. What it moves is the **duty cycle** — 0.812 → 0.159 over the scoreable range, a 5.1x
change — and the ungated scatter follows the duty cycle, because it counts posture wander during
silence that no listener hears.

### AND THE LEAD THAT OPENED §42 IS NOT SUPPORTED IN THE FORM IT WAS WRITTEN

§42 was opened by `axislev`'s *"freezing one axis nearly doubles F1's d'²"* (+34.590 ± 1.169) and
its note *"off-axis motor noise is the single largest thing between this creature and a bigger
vocabulary"*. Read the instrument (`experiments_milestones.cpp`, `measure_f1`):

    for (uint32_t k = 0; k < 7; ++k) p[k] = kAERest[k] + kAESd[k] * gauss(rng);
    ...
    tb.frames_of(p[0], p[1], ..., amp, g3, 0.0f);

**The jitter magnitude is `kAESd` — the ungated scatter — and `frames_of` renders a vowel
unconditionally, so every sample is audible.** The magnitude is taken from a creature that is silent
half the time and applied to renders in which it never is. The two do not match, and §42 shows the
mismatched quantity is exactly the one that moves with the duty cycle.

**SCOPE OF THE DAMAGE, stated narrowly on purpose.** `kAESd` is shared by four instruments
(`axisear`, `axisjit`, `axislev`, `axisamp`), and it enters every arm of every comparison equally.
So §41's **sign and verdict** conclusions survive — F3 is audible, the bandwidths are weak, f3 taxes
F1, level is refuted, masking carries ~71%. What does **not** survive is any claim resting on the
jitter's **magnitude**, and *"off-axis motor noise is the largest thing between this creature and a
bigger vocabulary"* is precisely such a claim. **It is withdrawn.** Whether `kAESd` is too large or
too small in absolute terms cannot be said from this ladder: comparing `axisgrid`'s 12-creature
mean-of-within-run-sds against `babble`'s pooled voiced sd is a cross-instrument comparison of
absolute levels, which is not valid. Only each instrument's own slope is, and that is what was used.

**THE FIX IS A RE-DERIVATION, NOT A RE-RUN.** `kAESd` should come from a voiced-gated measurement,
and `babble` already computes one under the comment *"the spread is what any milestone downstream
has to hear"* — written before §41 built four instruments on the ungated number instead. Re-deriving
it voids §41's d' **absolutes** by [[aibaby-vocal-calibration-invariant]] and leaves its verdicts.

### FOUR METHOD RESULTS, THREE OF THEM AGAINST ME

1. **`calibrate` PASSES ALL FIVE RUNGS** while the manipulated module's rate collapses 10.6x,
   because it excuses vocal by construction. The guard that works was already in the tree:
   `babble`'s duty-cycle bar (0.10–0.85). It and the `amp < 0.222` bar fire on **the same two
   rungs**, independently — two guards agreeing on the scoreable set is worth more than either.
2. **CLAUSE 3'S THREE-WAY SPLIT WAS ALL ON ONE SIDE OF ZERO.** Measured:
   `d log sd(f1) / d log noise_amp = −0.75`, a sign the pre-registration did not contain. Three-way
   is not sufficient; it has to **span the sign**. Note which document got it right: the P1–P3 set
   written after seeing **one** rung spans the sign explicitly, and the clause written after seeing
   **zero** does not.
3. **THE BIAS ORACLE IS DOSE-SATURATED, so my desk refusal of the swing columns was unnecessary.**
   The dose is `±0.25 × noise_amp` and therefore varies 1.75x across the scoreable range, yet f1
   swing moves **3.9%** (363.6 / 372.2 / 377.9). Levels were readable all along. The refusal was
   right as procedure and wrong as physics; `set_bias_oracle` has no clamp, so the saturation is
   downstream and its mechanism is **untested**.
4. **ONE SEED WOULD HAVE PRODUCED THE OPPOSITE HEADLINE.** The single-seed gated ladder read
   23 / 25 / 26 / 27 — a tidy monotone rise that would have been reported as a real residual effect.
   At 6 seeds the same rungs read 22.33 / 25.17 / 25.67 / 23.83 with per-seed values spanning
   **16 to 37**, and the trend is not significant. Six `babble` runs cost about three minutes.

### §42 STATUS: CLOSED AT STAGE 1 OF THREE

The abandonment count allowed three runs. Stage 1 refuses the line, so stages 2 and 3 are not spent.
**§42 does not re-open**, and the denominator is not the lever the numerator wasn't either: the
audible levels per axis are ~2.6 and `noise_amp` does not move them. What §42 leaves behind is the
`kAESd` re-derivation, which is bookkeeping on §41's magnitudes rather than a new lead.

## §43 — RE-DERIVE THE JITTER FROM AUDIBLE TICKS, AND ASK WHETHER bw2 CROSSES (2026-11-05)

**Pre-registered before the build. Opened by §42's finding, with §42's own stopping rule respected:
§42 closed at stage 1 and this is a new line with its own count.**

### THE QUESTION, AND WHY §42 COULD NOT ANSWER IT

§42 established that `kAESd` — the jitter magnitude four instruments draw their within-class spread
from — is an **ungated** scatter, measured on a creature silent half the time and applied to renders
`tb.frames_of` makes **unconditionally voiced**. `axisear` left **bw2 at d' 0.990 ± 0.039 against a
1.0 bar: UNRESOLVED**, inside its own error. That d' is computed with the defective denominator.

**Does a correctly gated denominator move bw2 across the bar — making the usable axis count FOUR
instead of three?** Levels multiply, so a fourth axis is worth more than anything else still open.

§42 could not answer it. Its gated number came from `babble`: a pooled sd over one creature's voiced
frames, against `axisgrid`'s mean of twelve per-creature within-run sds. **Comparing those absolutes
is invalid** and §42 used only each instrument's own slope. To get a gated `kAESd` that is
comparable, the gate has to go into **`axisgrid`'s own accumulation** — same estimator, same window,
same twelve creatures.

### THE DESIGN, AND THE TWO THINGS IT MUST NOT DO

1. **The ungated column stays byte-identical.** The gated accumulator is added **alongside**, never
   replacing. Six reproduction checks exist: `results/axisgrid-v1.log` plus §42's five ladder logs.
2. **`axisear` keeps its old arm.** The jitter magnitude becomes a **paired arm** — ungated `kAESd`
   against gated `kAESdG`, on **identical posture draws** — so `results/axisbw2-v1.log` must come
   back exactly in the ungated arm. Replacing `kAESd` in place would void §41 with nothing left to
   check against: the hijack [[aibaby-control-on-a-different-code-path]] priced.

### CLAUSE ORDER

1. **REPRODUCTION FIRST, because every number downstream is a difference against it.** `axisgrid`'s
   ungated zero rung identical to `results/axisgrid-v1.log`; `axisear`'s ungated arm identical to
   `results/axisbw2-v1.log`. If either fails, nothing else in the run is readable.
2. **THE GATE MUST BITE AND MUST NOT EMPTY THE SAMPLE.** `kAmplitudeFloor = 0.45`
   (`experiments_common.h:128`) against a shipped mean amplitude of **0.444** — the gate sits almost
   exactly at the centre of the distribution it cuts, which is why it can fail in both directions.
   **Under 10% of ticks surviving on any axis is NO VERDICT; over 95% means the gate did not bite
   and the comparison is vacuous.** `babble` measures 51.7% at this genome, so both branches are
   reachable and neither is foregone.
3. **THE DIRECTION IS NOT CONSTRAINED — THREE-WAY, SPANNING THE SIGN.** This is §42's lesson applied
   on the next run rather than recorded and forgotten. Gated `kAESd` may be **larger** (every §41 d'
   falls, bw2 moves further below the bar, the count stays three, and §41's magnitudes were
   optimistic), **smaller** (d' rises, bw2 may cross, the count may be four), or **equal within
   error** (`kAESd` was adequate; §42's defect is real but immaterial to §41's magnitudes — a result
   worth having, and the one that costs the least).
4. **bw2's VERDICT, against the bar that already exists and is not being moved.** bw2 clears only if
   its gated d' exceeds 1.0 **with its lower error bound above 1.0**. 0.990 ± 0.039 did not. *An axis
   inside the error bars of its own audibility threshold cannot carry a vocabulary claim* — §41's
   rule, quoted unchanged so it cannot be relaxed to fit what comes back.

### THE PINNED HASH MUST NOT MOVE, AND THAT IS A CLAUSE TOO

Nothing in `core/` changes and no RNG stream is touched: the gated accumulator only reads
`VocalParams` that the ungated one already reads, and `axisear`'s new arm re-seeds identically per
arm. **So `verify` must return `ad96f882becbee92` unchanged.** If the hash moves, the change is not
what this section claims it is, and the result is void regardless of what it says.

### ABANDONMENT COUNT: TWO RUNS

Run 1 is the gated `kAESd` from `axisgrid`. Run 2 is `axisear`'s paired arm. **If run 1 fails
clause 2, §43 ends having spent one.**

### HONEST DISCLOSURE BEFORE THE BUILD

§42 withdrew a **magnitude** claim, and §43 is entirely about magnitudes — the same class of
quantity that just proved fragile. The protection is structural rather than a promise to be careful:
every number here is a **within-run difference on identical draws**, and the old value survives
beside the new one in the same table. What §43 cannot do is say which of the two is *correct* in an
absolute sense; it can only say that one counts ticks a listener hears and the other does not.

### §43 RESULT — THE CORRECTION IS REAL AND IMMATERIAL, AND bw2 IS STILL NOT A FOURTH AXIS (2026-11-05)

`results/n43-axisgrid.log`, `results/n43-axisbw2.log`, gate at `results/n43-verify.log`.

**CLAUSE 1 PASSES TWICE, which is what makes the rest readable.** `axisgrid`'s ungated zero rung is
identical to `results/axisgrid-v1.log`, and `axisbw2`'s diff against `results/axisbw2-v1.log`
contains **only added lines** — every published number, including `bw2 0.990 +- 0.039`, came back
unchanged after `measure` was rewired to take the jitter scale as a parameter at four call sites.

**CLAUSE 2 PASSES.** The gate left **0.288 of scored ticks** and **12 of 12 creatures** contributing
— inside the pre-registered 0.10–0.95 window, so it bit without emptying the sample.

**AND THE GATE IS PROVABLY APPLIED,** by a row that can only read one way: `voi` comes back with
rest **exactly 1.000** and audible sd **exactly 0.000**. Voicing is binary and the gate requires it
above 0.5, so those two values are forced — and they are the cheapest possible check that the
conjunction is really in the accumulation path.

| axis | sd ungated | sd audible | **jitter ratio** | d' ungated | d' audible | **d' ratio** |
| --- | --- | --- | --- | --- | --- | --- |
| f0 | 9.129 | 8.906 | 0.976 | 5.298 ± 0.076 | 5.489 ± 0.076 | 1.036 |
| f1 | 21.727 | 20.806 | 0.958 | 7.649 ± 0.091 | 7.857 ± 0.093 | 1.027 |
| f2 | 57.751 | 56.462 | 0.978 | 9.492 ± 0.093 | 9.722 ± 0.094 | 1.024 |
| f3 | 47.616 | 45.395 | 0.953 | 5.119 ± 0.044 | 5.272 ± 0.045 | 1.030 |
| bw1 | 5.346 | 5.253 | 0.983 | 0.859 ± 0.031 | 0.875 ± 0.030 | 1.019 |
| **bw2** | 5.967 | 5.696 | 0.955 | **0.990 ± 0.039** | **1.016 ± 0.040** | 1.025 |
| bw3 | 5.618 | 5.342 | 0.951 | 0.756 ± 0.021 | 0.777 ± 0.022 | 1.028 |

**CLAUSE 3 LANDS ON THE "~1" BRANCH — the cheapest of the three and the one worth having.** The
audible-tick jitter is **2–5% smaller** than the ungated one, so every §41 d' is **2–4% larger**.
`kAESd` was **adequate**. §42's defect is real and **immaterial to §41's magnitudes**, which stand
to within about four percent. **Nothing in the §41 archive needs re-deriving or voiding.**

**THE SELECTION EFFECT WAS MEASURED AND IS NEGLIGIBLE.** Gating on amplitude could have shrunk
formant variance merely by conditioning on a correlated variable. The gated rest points say it did
not: f1 **626.843** against 626.545, f2 **1667.849** against 1669.598, bw2 **150.551** against
150.579 — all under 0.11%. Meanwhile `amp` itself moves **0.444 → 0.610** with its sd ratio at
0.712, exactly as conditioning on it must. So the formants are essentially uncorrelated with
amplitude and the 2–5% narrowing is a real effect, not a subsetting artefact.

### CLAUSE 4: THE POINT ESTIMATE CROSSES AND THE VERDICT DOES NOT

**bw2 rises from d' 0.990 to 1.016 — above the bar of 1.0.** And it is **STILL UNRESOLVED**, because
the pre-registered rule is the lower bound, not the point estimate:

    1.016 - 2.447 x 0.040 = 0.918     which is below 1.0

**The usable axis count stays THREE and `f1 x f2 x f3 ~ 19 signals` is unchanged.** This is the
clause working as intended: a point estimate stepping over a bar is exactly the result that invites
a relaxed rule, which is why the rule was quoted into the output rather than rewritten there, and
why the instrument prints `SO THE CORRECTION DOES NOT CHANGE THE VERDICT` as a computed branch
rather than leaving it to be narrated afterwards.

**AND THE MORE USEFUL CLOSURE IS THAT STATISTICS IS NOT WHAT IS MISSING.** For 1.016 to clear by
replication, the SE would have to fall from 0.040 to under 0.0065 — a 6.1x reduction, so about
**450 creatures instead of 12**. That is affordable. It is also not worth doing: **a margin of 1.6%
over the bar is smaller than the systematic choices the number rests on** — which `kAESd`, which
amplitude, which reach — and §41's rule exists precisely to exclude an axis sitting on its own
audibility threshold. **bw2 is not a fourth axis in any useful sense, and §43 closes the question
rather than refining it.**

### A CORRECTION TO §42'S WRITE-UP

§42's result section and its memory both place `kAESd` in `experiments_common.h`. **It is in
`host/src/experiments_milestones.cpp:46042`**, in the `axisear` block, hoisted there from four
instruments that previously carried it by copy. The claim about what it *is* was right; the file was
wrong. `kAESdG` now sits beside it.

### AND §42'S FINDING IS REFINED, NOT WITHDRAWN

Both of these are true and they are about different things:

- **§42:** across a `noise_amp` ladder the **ungated** scatter's slope is steep (levels −32%) while
  the **gated** one is flat (1.0 SE). The ungated statistic tracks the duty cycle.
- **§43:** at the **shipped operating point** the two **levels** agree to within 5%.

A statistic can be a poor *estimator of a derivative* and a fine *estimator of a level*. §42
established the first, §43 the second, and together they say: the defect is real, it would have
mattered to any conclusion drawn from moving `noise_amp`, and it did not matter to the conclusions
§41 actually drew — which were all at the shipped point.

### §43 STATUS: CLOSED, BOTH RUNS SPENT, NOTHING OWED

The abandonment count allowed two runs and both were used: the gated `kAESd` and the paired arm.
`kAESdG` ships beside `kAESd` as documentation of a checked null rather than as a replacement. **The
axis count is three, §41's magnitudes stand, and bw2 does not re-open.**

## §44 — "ACCUMULATION", RE-DERIVED: THE PREMISE WAS SETTLED AND THE RESULT WAS LOST (2026-11-05)

**A desk exercise, no runs.** §41–43 closed the axis count and the denominator, and the memory
direction's two classical routes were already closed, so the question was: what else can
*"builds intelligence from what it hears and is told"* mean on this architecture? The answer is not
a new lead. **It is that the question was already answered, at 11.5 SE, by a run that was never
written into this document or into memory.**

### WHAT "ACCUMULATION" WAS OPERATIONALISED AS, AND EVERY READING'S STATUS

`aibaby-the-goal` (2026-11-03) named accumulation as the wall and picked the axis count as the
lead, on the premise *"retention is FREE off-axis (0.948 vs −0.167), levels MULTIPLY."* Checking
every reading of the word against the archive:

| reading of "accumulate" | status |
| --- | --- |
| more distinguishable utterances (vocabulary) | **capped**: `f1 x f2 x f3 ~ 19 signals`, §41–43 |
| lessons that persist | **works**: a taught sound is kept and keeps improving, 1.31 vs 0.999, and sleep does not erase it 3/3 |
| two lessons at once | **works at 0.84** on orthogonal targets; a conflicting pair collapses to 0.22 |
| across sleep (consolidation) | **closed**: replay as shipped does nothing; switching it off costs 0.3 SE |
| hold a word after it stops (working memory) | **closed**: 0.503, chance |
| a new conditional mapping | **the conclusion's refusal** — no input-specific factor exists |
| structure over time (sequences) | **closed**: composes in space, not in time |
| monotone improvement with exposure | **open and positive**, with a measured price: `dF1 ~ aligned^0.61` |

Nothing in that table is a new direction. But the *premise* under the second and third rows turned
out to be the interesting thing.

### THE PREMISE HAD A THREE-RUN HISTORY AND THE ARCHIVE RECORDS ONLY ITS FIRST TWO STEPS

1. **`axiscollide`** (2026-09-16, 20 seeds) refuted *"the wipe tracks distance"* cleanly — `f1-full`
   and `f2-full` are the same L1 distance and retain **−0.06 against 2.83** — and established that
   retention tracks the demand on A's **own** axis, with `corr(retention, gap dUPPER) = −0.9994`.
   **But its own write-up says the useful half is UNTESTED:** its "orthogonal" arms set the second
   target's F1 to A's own F1 target, so they *reaffirm* A's axis rather than being silent about it.
   Its memory still carries *"do not quote this file for the third claim"* and names the fix: an arm
   with `second_axis = 2`.
2. **`axisfree`** (5.6M ticks, 20 seeds) built exactly that arm. It **replicated the level** —
   `f2-axis` retention ~1.0 in two families — **but not the contrast: +5.7 SE in family 0, then
   +2.0 SE in family 1, against a pre-registered +3 SE gate. Its own verdict line reads `NO
   VERDICT: between the gates.`**
3. **`axispower`** (2026-09-17, `results/axispower-56m.log`) settled it, and **nothing in this
   document or in memory says so.**

### WHAT `axispower` MEASURED

Third independent seed family (offset 224737, *"a nonzero offset distinct from 104729 proves this is
not a rerun of family 0 or 1 on a stale binary"*), n=60, 180 jobs, with `do NOT pool this with
families 0 or 1 — they generated the hypothesis` written into the pre-registration.

| arm | B scored | F1-axis retention | gap dUPPER |
| --- | --- | --- | --- |
| `f2-axis` (silent about F1) | F2 only | **1.024 ± 0.062** | −1.941 ± 0.541 |
| `f1-axis` (pure demand on F1) | F1 only | **0.116 ± 0.048** | +6.428 ± 0.616 |

**Contrast +0.909 ± 0.079 = +11.5 SE** against a +3 SE gate, 60/60 seeds scored in both arms, and
`f2-axis` retention 0.39 SE *above* 1.0 where the gate allowed 2 SE below. Both pre-registered
conditions met: **a lesson silent about A's axis is FREE, at power.** The mechanism column tracks as
`axiscollide` predicted — arms that keep A deepen the suppression, arms that lose A restore it.

The pre-registration even priced itself: *"on a true difference of ~0.9 this should read about 4.3
SE."* It read 11.5, because the SE came in at 0.079 rather than the projected 0.21.

### SO THE ACCUMULATION ACCOUNT IS COMPLETE, AND IT IS A PRODUCT OF TWO MEASURED FACTORS

**Capacity = (how many axes can carry mutually silent demands) x (levels per axis).** Both factors
are now measured, and by different lines that never met:

- **The second factor is §41–43's:** three axes reach the air and the ear, at ~2.6 levels each.
- **The first factor is `axispower`'s:** a lesson silent about another's axis costs it nothing, 11.5
  SE. So the axes really do act as independent slots rather than as a shared store.

That is why nine words name as well as six, and why the ceiling is where it is. **It is a coherent,
closed account rather than a set of unexplained refusals** — which is the useful output of this
section, and it needed no run.

### THREE PLACES IN THE ARCHIVE CARRIED THE PRE-`axispower` STATE

1. **`axisgrid` printed it at runtime, in every log it has ever produced.** `run_axisgrid`'s
   `why it matters` block cited *"axisfree 0.948 vs −0.167, +5.7 SE"* as the stated rationale for
   the whole axis-count line — the discovery-sample number, whose own confirmation family returned
   NO VERDICT. **Seven archived logs carry it, five of them committed by §42 yesterday.** The
   registry's documentation string for `axisgrid` carried it too; that one is *not* in the
   user-facing `--experiment` listing, which shows a curated subset, so it was source documentation
   rather than displayed output — an earlier draft of this section overstated it as printed. Both
   are corrected. Every changed line is a comment or a string literal, so computation cannot move;
   gated regardless.
2. **`aibaby-axis-not-distance`** still says the claim is UNTESTED and *"do not quote this file for
   the third claim."* True when written; superseded by two later runs.
3. **`aibaby-the-goal`** cites the discovery number as *"THE REAL LEAD."* The lead was right and its
   evidence was the weaker of the two available.

**And this project already knows this failure mode by name** — `aibaby-pairing-on-discovery-sample`
is in the method rules. What it did not have was a check that a *superseding* result gets written
back to everything that cited the superseded one.

### ONE GENUINELY UNFILLED CELL, RECORDED WITHOUT ENTHUSIASM

`axisgrid` reports `amp` and `voi` as `NOT TESTED -- activity readout, and this bias is zero-mean`.
That is structural, not an oversight: `bias_oracle_at` returns `amp * (2*pos - 1)`, a zero-mean ramp
by construction, and `target_amp = group_activity_[8]` is a **mean** over the slice — so the oracle
*cannot* move a net-drive readout at any dose. Testing whether loudness is a steerable fourth axis
needs a constant-offset oracle mode, which the kernel does not have.

**It is recorded as unfilled, not as promising**, for three reasons written down before anyone builds
it: the transmission tape carries formants only, so a loudness distinction cannot be taught
creature-to-creature (`f0`'s caveat exactly); `aibaby-cochlea-operating-point` measured that d' rises
as level falls, so a loud/soft axis **modulates every other axis's discriminability** instead of
multiplying with it; and `kAmplitudeFloor = 0.45` gates "is it sounding" at eight sites, so a
deliberately quiet signal is scored as silence by this project's own instruments. A constant lift of
a group is also the *"you made the module louder"* confound that the zero-mean ramp was chosen to
avoid — and for an activity readout the confound and the signal are the same event.

### §44 VERDICT: NO NEW LINE, AND THAT IS THE RESULT

The accumulation question is **answered**, by three runs over two days in September, and the answer
is a product of two factors that are each closed. §44 opens nothing and spends no abandonment count.
What it changes is the archive: a settled 11.5 SE premise now has a memory, and the three places
that cited its unsettled predecessor now point at it.

## §45 — WHY F1 AND NOT F2: BOTH NAMED CAUSES ARE WEAK, AND Hz IS THE WRONG UNIT (2026-11-05)

**Pre-registered before the build.** The last open lead from `aibaby-open-leads` other than the
rotation's decline. `tutorgrid` run 3 (depth 5) found a split nobody predicted:

| gen | bit F1 | bit F2 |
| --- | --- | --- |
| 0 | +37.6 ± 3.0 | +91.9 ± 13.7 |
| 4 | +16.1 ± 5.5 | +92.9 ± 28.1 |

**gen 4 − gen 0, paired: F1 −21.58 ± 5.56 (3.9 SE), a RESOLVED LOSS of 57%. F2 +0.97 ± 27.91
(0.0 SE), nothing.** A two-signal code loses nothing over four generations, so four signals leak
where two did not — **on one axis only.**

### THE PREMISE SURVIVES ITS OWN SIBLING'S RETRACTION, WHICH HAD TO BE CHECKED FIRST

`tutorwide` retracted a *different* decay as an artefact: *"a signed mean is not a magnitude when
the sign can flip between units"* — 5 of 18 chains ended holding the inverted convention, and with
the width held there was no loss at all. **If F1's decay were the same artefact there would be
nothing to explain.** It is not: `tutorgrid` reports **inheritance at every consecutive pair, F1
18/18**, so no F1 lineage ever flips sign and its signed mean *is* a magnitude. (F2 is 16/18, two
flips, which would only *understate* F2 — and F2 shows no loss anyway.) The premise holds.

### CAUSE (b) IS ALREADY REFUTED, BY THE INSTRUMENT BUILT TO FIND IT

The lead says two causes coincide on the F1 axis: a smaller separation, and *a worse derived-index
recovery*. **The second points the wrong way.** `ctxaxis` measured the kernel's own clustering rule
against the best available reader:

| | kernel index | best reader | loss |
| --- | --- | --- | --- |
| bit F1 | 0.827 ± 0.011 | 0.850 ± 0.020 | **0.023** |
| bit F2 | 0.781 ± 0.023 | 0.965 ± 0.006 | **0.184** |

**The index recovers F1 nearly perfectly and loses 8x more on F2 — yet F2 is the bit that does not
decay.** `ctxaxis`'s own closing line says so: *"the opposite of the asymmetry the primary was
built to find."* So index recovery cannot be why F1 decays, and the lead's framing of "two
coinciding causes" is already down to one.

### AND CAUSE (a) IS MOSTLY A UNIT ARTEFACT — THE DESK CALCULATION THIS SECTION EXISTS TO TEST

§43 measured each axis's airborne d' at its **own reach**. `d' = Δ/σ` is linear in separation at
fixed jitter, so `d'(reach)/reach` is a per-Hz slope — and the two tables have never been combined:

| axis | reach | d' at reach (§43) | **d' per Hz** | delivered (tutorgrid) | **d' at delivered** |
| --- | --- | --- | --- | --- | --- |
| f1 | 118.0 | 7.857 ± 0.093 | **0.0666** | 37.6 | **2.50** |
| f2 | 300.3 | 9.722 ± 0.094 | **0.0324** | 91.9 | **2.98** |

**F1 is 2.06x MORE discriminable per Hz than F2.** So the separation gap is **2.44x in Hz and only
1.19x in airborne d'** — the unit the air and the pupil's ear actually work in. *"F1 is the weaker
signal"* is largely an artefact of quoting Hz.

**THIS IS AN EXTRAPOLATION ACROSS TWO INSTRUMENTS AND IS NOT A MEASUREMENT.** §43's d' comes from
synthetic jitter at `kAESd` magnitudes rendered at `kB2Amp`; tutorgrid's Hz are delivered values in
a lineage run. The combination is defensible — §42/§43 showed the jitter magnitude is right to
within 5% at the shipped point — but it is exactly the cross-instrument absolute comparison this
session has refused three times. **So §45 measures it in ONE run instead.**

### THE INSTRUMENT: `bitsnr`, READ-ONLY, NO GENOME FIELD, NOTHING SHIPPED

For each axis, the airborne cepstral d' at **tutorgrid's delivered separation**, with identical
deviates across axes, jittered on all seven parameters at `kAESdG` (§43's audible-tick values, the
correct denominator; by §43 the choice moves nothing material), at the amplitude the creature hears,
through its own cochlea.

### CLAUSE ORDER

1. **WIRING CHECK FIRST, because every number downstream is a difference against it.** Run at each
   axis's **own reach** and the instrument must return §43's values — f1 **7.857**, f2 **9.722**.
   An instrument that cannot reproduce the table it extends cannot extend it. This also proves the
   instrument *can* see a difference between the axes at all, so clause 2 is not asking a question
   it is structurally unable to answer.
2. **THE COMPARISON, THREE-WAY AND SPANNING THE SIGN** (the §42 lesson, applied on the next run
   rather than filed):
   - **d'(F2 at 91.9) > d'(F1 at 37.6) by the 1.19x the desk predicts** → the airborne-SNR account
     survives as *a* factor, and the open question becomes whether 1.19x can produce 57% against 0%.
   - **the two are EQUAL within error** → **the airborne-SNR account is REFUTED, model-free.** Two
     bits on the same creature with the same jitter, equally audible, one decaying 57% and the other
     not at all, cannot be explained by one being the weaker signal. This holds regardless of any
     transmission model, which is why it is the clause worth having.
   - **d'(F1) > d'(F2)** → the asymmetry is *inverted* relative to the decay, and the lead's
     remaining cause joins its first in pointing the wrong way.
3. **POWER, STATED AS A NUMBER BEFORE THE RUN.** §43 returned SE ~0.09 on d' at these sample sizes.
   The predicted gap is 2.98 − 2.50 = **0.47, about 5 SE** — so the instrument can tell 1.19x from
   1.00x, and a null will mean "equal" rather than "underpowered". If the realised SE is above
   **0.16** the comparison is NO VERDICT on power alone, and that is checked before the contrast is
   read.

### WHAT §45 CANNOT DO, STATED NOW

It cannot explain the decay. It can only remove, or fail to remove, the one surviving named cause.
If clause 2 lands on "equal", **the lead becomes a question with no candidate answers left** — which
is a worse-sounding and more useful position than one with two bad ones, and is where the next
design would have to start from scratch.

### ABANDONMENT COUNT: ONE RUN

§45 is a single read-only measurement. It opens no line. If it refutes the SNR account, what remains
is a *new* question and gets its own section.

### §45 RESULT — THE AIR IS NOT THE REASON, AND THE ANSWER WAS ALREADY IN tutorgrid's ORACLE ARM

`results/n45-bitsnr.log`. Read-only, no genome field, nothing shipped.

**CLAUSE 1 REPRODUCES §43 EXACTLY**, which is what licenses everything below:

| axis | own reach | d' measured | §43 says | difference |
| --- | --- | --- | --- | --- |
| f1 | 118.0 Hz | 7.857 ± 0.093 | 7.857 | **−0.000** |
| f2 | 300.3 Hz | 9.722 ± 0.094 | 9.722 | **−0.000** |

Bit-exact, because the seeding is `axisbw2`'s and ignores the separation. The same table is
the vacuity check: an instrument that resolves a known 1.24x difference between these axes is
demonstrably able to answer clause 2 either way.

**CLAUSE 3 PASSES BEFORE THE CONTRAST IS READ:** SE on the gap **0.079** against the
pre-registered ceiling of 0.16.

**CLAUSE 2 — AT THE SEPARATIONS tutorgrid ACTUALLY DELIVERED:**

| axis | delivered | the bit | airborne d' |
| --- | --- | --- | --- |
| f1 | 37.6 Hz | **decays 57%** | **2.576 ± 0.065** |
| f2 | 91.9 Hz | holds | **2.708 ± 0.045** |

**Gap +0.132 ± 0.079 (1.7 SE), ratio 1.051. EQUAL WITHIN ERROR.**

So **the airborne-SNR account is refused**, and on two independent grounds, which matters
because 1.7 SE is close to the 2 SE line the code tests:

1. **The significance test does not clear the bar** it was given in advance.
2. **The effect size cannot do the work regardless.** Even taking the point estimate at face
   value, a **5%** difference in airborne discriminability is being asked to produce **57%
   against 0%**. That argument does not depend on the p-value and is the stronger of the two.

Note the desk extrapolation predicted **1.19x** and the measurement returned **1.05x** — the
right direction, a smaller gap, and a different conclusion at the 2 SE line. Measuring instead
of extrapolating was the right call for once in the direction that mattered.

### MY PRE-REGISTRATION'S "WHAT THIS CANNOT DO" WAS WRONG, AND THE ARCHIVE SAYS SO

§45 predicted that an EQUAL verdict would leave the lead *"with no candidate answers left."*
**That was written before I had read `tutorgrid`'s fourth run**, and it is wrong: the oracle-index
arm (`results/tutorgrid-oracle.log`) already contains the mechanism.

Only the pupils' index source changed, derived → host-written, with generation 0 **bit-identical
on all four numbers** so the pairing is licensed:

    F1 decay, oracle - derived, cross-run paired   +62.61 +/- 5.91  (10.6 SE)
    gen 4 - gen 0:  derived chain F1  -21.58 +/- 5.57  (3.9 SE)  resolved LOSS
                    ORACLE  chain F1  +41.03 +/- 4.36  (9.4 SE)  resolved GAIN
                    both chains   F2  under 3 SE either way

**A perfect index turns a 3.9 SE loss into a 9.4 SE gain, and reach is ruled out** — F1's 118 Hz
is unchanged between the runs and only the index moved. That run's own correction reads: *"The
channel is fine; a four-way DERIVED index is worse than a two-way one, and down a lineage that
compounds into apparent decay."* And: *"the chain's level is set by how well a pupil can USE its
index, not by cumulative transmission loss."*

**So "why F1 and not F2" now has an answer, assembled from three places that had never been put
together:**

1. **It is not the channel.** §45, measured: the two bits are equally audible at the separations
   actually delivered, 1.05x.
2. **It is not the index's classification accuracy.** `ctxaxis`: the kernel's index loses **0.023**
   against the best reader on bit F1 and **0.184** on bit F2 — eight times worse on the bit that
   does *not* decay.
3. **It is the index, in MAGNITUDE rather than in accuracy.** The oracle arm makes the index
   causal at 10.6 SE, and the deficit costs **F1's delivered Hz** while leaving F2's alone.

**AND THAT IS THE PROJECT'S OWN RECURRING SPLIT.** `aibaby-magnitude-direction-split` established
that magnitude and assignment are separable here; `aibaby-storage-vs-expression` that a store and
its expression are different quantities. **`ctxaxis` measured whether a classifier can read the bit
out of the index. The lineage is limited by how many Hz the lesson writes given that index.** Those
are different statistics and this is the clearest case in the archive of the second not following
from the first.

### THE SUCCESSOR, POSED BUT NOT RUN

**Why does an index deficit cost F1's magnitude and not F2's, when that index classifies F1
better?** That is a magnitude-vs-accuracy question about the write, not about the air or the
partition, and it is where a new section would start. §45's abandonment count was one run and it is
spent.

**One honest caveat on the thing being explained:** the F1 decay has read **4.3 SE, then 2.6 SE
("no longer established"), then 3.9 SE** across three runs. At depth 5 with 18 lineages and 18/18
inheritance it is credible, but its *size* is not well pinned, and a successor should re-measure the
asymmetry before modelling it.

## §46 — SHIP THE THIRD FORMANT? MEASURE THE PRIZE BEFORE PAYING FOR IT (2026-11-05)

**Pre-registered before the build.** The owner picked this after §45, ordered *"first 2 then 1"*, and
the ordering is right for a reason worth stating: **shipping f3 moves the determinism hash**, so
every absolute measured after it stops being comparable with the archive. Anything cheap and
hash-neutral belongs first, because it is the last chance to measure on the current physics.

### WHAT IS ACTUALLY OFF, AND WHAT THAT COSTS THE CREATURE

`dna/default.toml:383` ships `render_f3_gain = 0.0`. The renderer therefore has **two resonators**,
and the transmitted code space is `f1 x f2 ~ 7 signals`. §41 measured F3, once rendered, as a real
audible axis — **d' 5.272 ± 0.045 at its own reach**, ~0.93x F1 — and the larynx already *commands*
it at **2.68 levels with cross-talk ≤ 0.007**. So the prize is `f1 x f2 x f3 ~ 19 signals`, **2.7x,
and it is switched off.**

Three things are needed before it is *usable*, and naming only the gain would be misleading:

| change | why it is required |
| --- | --- |
| `render_f3_gain` 0.0 → 1.0 | F3 reaches no ear at all until this is on |
| `struct Word` gains `f3` | it is `{float f0, f1, f2;}` — **a caregiver cannot currently say an F3 distinction**, nor a tutor transmit one |
| `formant_error_axis` gains axis 3 | it handles `axis == 1, 2` only, so **the reward cannot see F3** |

### THE SEQUENCING, AND IT IS THE WHOLE DESIGN

**STAGE 1 MEASURES THE PRIZE ON A VARIANT GENOME AND SHIPS NOTHING.** `dna/default.toml` is not
touched, the pinned hash `ad96f882becbee92` does not move, and the archive stays comparable. The
source changes are **additive**: `f3` last in `Word` leaves every `{f0, f1, f2}` initialiser
zero-filling it, and at `render_f3_gain = 0` the renderer ignores it — so existing behaviour is
bit-identical and `verify` must prove that.

**STAGE 2 SHIPS IT, AND ONLY IF STAGE 1 PAYS.** A genome revision, a **new pinned hash**, a fresh
`calibrate`, and an explicit note that every cross-genome absolute in the archive is void by
[[aibaby-vocal-calibration-invariant]]. **If stage 1 reads null, nothing was shipped and nothing was
voided** — which is the entire reason for doing it in this order rather than flipping the switch.

### STAGE 1'S TEST: THE MINIMAL DECISIVE FORM

Not a 2x2x2. **Two words differing ONLY in f3**, taught on the creature's own derived index, scored
as naming excess over the same creature's no-index control — `ctxgrid`'s design with the axis
swapped. If the creature cannot name on the third axis *at all*, a three-dimensional grid is
pointless; if it can, the grid is the obvious successor and gets its own section.

Separation follows the grid's own convention — each axis at its **own measured reach**, as `ctxgrid`
used 118 Hz on F1 (571/689) and 300 on F2 (1517/1817). F3 rests at **2613.7** with a reach of
**254.9**, giving **2486 / 2741**.

### CLAUSE ORDER

1. **BIT-IDENTITY FIRST, because it is what makes stage 1 free.** With `render_f3_gain = 0` the
   extended `Word` and scorer must change nothing: `verify PASS`, hash **`ad96f882becbee92`
   unmoved**, `experiments 24 as expected`. A hash move here means the change was not additive and
   stage 1's variant results are not interpretable against anything.
2. **CALIBRATION ON THE VARIANT, before any behaviour is read.** `self_gain` is on, so turning f3 on
   changes **what the creature hears of itself** and may move its operating point. `calibrate` is
   the pre-calibrated detector of exactly that and must PASS on the f3 variant. **A rate shift here
   would confound every naming number in the run**, and §42 is the reminder that `calibrate` can
   also pass while the thing you moved went silent — so the vocal rate is read as a number, not as a
   verdict.
3. **THE REWARD MUST DEMONSTRABLY SEE F3.** A wiring check, not an assumption: `formant_error_axis`
   at `axis == 3` must return finite values, and **the taught arm's F3 error must fall**. Written as
   its own clause because a null naming result with a reward that never moved would be a false
   refusal of the axis — the shape [[aibaby-null-is-not-a-control]] and §41's own vacuous run 2
   record. **A guard for the zero-filled f3: any word with `f3 <= 1.0` makes axis 3 unscoreable, and
   that is a refusal, not a zero.**
4. **THE NAMING TEST, THREE-WAY AND SPANNING THE SIGN.** Excess over the creature's own no-index
   control:
   - **positive** → the third axis names, the code space is real, and stage 2 is justified.
   - **zero** → **the most interesting outcome.** F3 is audible (d' 5.272), commandable (2.68
     levels) and now rewarded, so a null localises the failure in the **write** and not in the air,
     the ear or the reward — and it would say the 19-signal figure is a code-space claim the
     creature cannot cash. Stage 2 does **not** run.
   - **negative** → F3 interferes; the f3 tax on F1 (11.3% of d'², measured) is the named suspect.

### ABANDONMENT COUNT: TWO RUNS

Stage 1 is one run. Stage 2 is one genome revision plus its gate. **A null at stage 1 ends §46**, and
the shipped creature keeps two resonators with a written reason rather than an unexamined default.

### HONEST DISCLOSURE

The 19-signal figure has never been a behavioural claim. §41 measured that the air *carries* three
axes; nothing has ever asked whether the creature can be **taught** on the third. §46 asks that, and
the answer may well be no — in which case the value delivered is a closed question and an
unchanged genome, not a bigger vocabulary.

### §46 SIDE-FINDING, INDEPENDENT OF THE RESULT: `kVLTgtHeard` AT `nw = 1` RETURNS nan

Found while debugging my own instrument, and worth separating from §46's question because it is a
**latent trap in `run_vocallearn_session` for anyone else**, not a fact about f3.

`f3name`'s first version passed `target = -1`. Line 4957 resolves that to `kVLTgtHeard` for a
`kVLTaught` arm — **the conditional mode** — where clause 3 is the *fixed-target* question that
gates conditionality. So it asked the wrong one of the two. It also came back with
`err early nan, err late nan`, and changing the mode to `kVLTgtFixed` made the nan vanish:
**`kVLTgtHeard` with `words = 1` produces a non-finite joint error.**

**AND THE REASON IT PROPAGATES IS WORTH KNOWING EVERYWHERE, NOT JUST HERE: `nan < 0.0` IS FALSE.**
So a non-finite formant passes *both* guards on that path —

    if (f1 <= 1.0 || f2 <= 1.0) return -1.0;   // formant_error: nan fails both comparisons
    if (err < 0.0) { ++out.skipped; continue; }  // the degenerate-trial skip: nan is not < 0

— and lands in `err_sum[bin]`, poisoning `err_early`, `err_late` and every mean downstream of
them, while `scored` still counts the trial. **A guard written as `x <= bound` does not exclude a
nan; it admits one.** The corrected instrument therefore prints a **non-finite count** per
utterance vector rather than assuming the rows are clean, and `utt_f1/f2/f3` came back
**0/0/0** once the mode was right.

**AND MY OWN CLAUSE 3a WAS A GUARD MEASURING THE WRONG QUANTITY.** It read `err_early`/`err_late`
— which the session fills from `formant_error(f1, f2, w)` in every mode except amp and rate, i.e.
the **F1/F2 joint** error, *not* `kVLScoreF3`'s objective. It printed *"THE REWARD NEVER MOVED --
clause 4 is NOT readable"* for a run whose very next clause showed the voice moving **+33 Hz against
its yoke**. Two clauses on the same output contradicting each other is what caught it. It now
computes the objective from `utt_f3` directly.

**That is three instances in one instrument** of the shape this project keeps hitting — the wrong
target mode, a guard on the wrong quantity, and a comparison that admits nan — and all three were
found by *reading output that disagreed with itself* rather than by any gate.

### §46 STAGE 1 RESULT — F3 CAN BE TAUGHT, WEAKLY, AND THE GATE OPENS (2026-11-05)

`results/n46-f3name-s{0..8}.log`, gate at `results/n46-verify.log`. Nothing shipped:
`dna/default.toml` untouched, every run on a scratchpad variant with `render_f3_gain = 1.0`.

**CLAUSE 1 PASSES, WHICH IS WHAT MADE STAGE 1 FREE.** On the shipped genome the extended `Word`
(`f3` last and defaulted), the appended `kVLScoreF3`, the new `utt_f3` vector and
`formant_error_f3` change **nothing**: `verify PASS`, hash **`ad96f882becbee92` unmoved**, pinned
hash PASS, **experiments 24 as expected**. The changes really were additive.

**CLAUSE 2 PASSES WITH THE NUMBER READ AS A NUMBER.** `calibrate PASS` on the f3 variant, and vocal
sits at **4.12 Hz — identical to the shipped genome's 4.12**. Turning f3 on does not move the
operating point, so nothing below is confounded by a rate shift.

**CLAUSE 3 PASSES. Nine seeds, a fixed f3 target of +127.4 Hz against teachsound's yoke:**

| | value | |
| --- | --- | --- |
| taught moved | **+27.54 ± 15.11 Hz** (21.6% of the ask) | 6/9 positive — **1.8 SE, NOT significant alone** |
| yoked moved | **−20.33 ± 13.49 Hz** | 2/9 positive |
| **paired taught − yoked** | **+47.88 ± 9.47 Hz** | **5.1 SE, 9 of 9 seeds positive** (sign test p ≈ 0.002) |

Per seed, taught: +4.3, +87.0, +63.3, −7.5, +79.7, −20.6, −37.8, +50.9, +28.6. **Non-finite rows
0/0/0 in every arm of every seed.**

**THREE THINGS THE 5.1 SE DOES NOT SAY, stated so it cannot be quoted as more than it is:**

1. **The taught arm alone is 1.8 SE.** What is established is *taught differs from yoked*, not *the
   lesson moves f3 toward the target* standing on its own. The pairing is legitimate — it is
   `teachsound`'s design, the yoke replays the taught arm's own praise shifted half a trial, and
   pairing by seed removes the seed-level variance — but the two claims are different sizes.
2. **Part of the contrast is the control going the wrong way.** The yoke drifts **−20.33 Hz at 2/9
   positive**: directional, not symmetric noise. That is why the paired figure exceeds the taught
   arm's own movement. [[aibaby-lesson-silences-the-top]] and
   [[aibaby-silences-the-wrong-side]] make a directional drift under uncorrelated praise plausible,
   but it is **unexplained here** and the contrast is not all signal.
3. **It is weak in absolute terms** — 21.6% of the ask, against M1c's F1 lesson at d' 5.57 versus a
   yoked 1.08. **f3 is influenceable, not readily taught.** And conditionality always costs
   (`ctxgrid`'s excess is a fraction of its unconditional lesson), so **the prior on clause 4 is
   modest even though the gate opened.**

**AND I MISREAD THE EVIDENCE TWICE, IN OPPOSITE DIRECTIONS, OFF PARTIAL DATA.** After seed 0
(+4.3 Hz) I wrote that the evidence pointed at *not teachable*; after seeds 1–2 (+87.0, +63.3) I
nearly wrote the opposite. The spread is **+87.0 to −37.8**, which is exactly the variance the
instrument's own *"single seed is a gate and not a result"* line was pointing at — a caveat I had
written into it myself and then ignored twice. **The rule is not "run more seeds"; it is: do not
narrate a sweep while it is running.**

### WHERE §46 STANDS: STAGE 2 IS NOW A REAL DECISION, AND IT IS NOT MINE

The abandonment count allowed two runs; stage 1 spent one and **opened** rather than closed. Stage 2
— shipping `render_f3_gain = 1.0` — would be a genome revision with a **new pinned hash**, voiding
every cross-genome absolute in the archive by [[aibaby-vocal-calibration-invariant]]. Stage 1 was
designed so that this decision could be made on evidence rather than on the 19-signal arithmetic,
and the evidence is: **the third axis is audible (d' 5.272), commandable (2.68 levels), and
teachable at about a fifth of what is asked of it.**

**The honest recommendation is to build clause 4 BEFORE stage 2**, on the variant, and ship nothing
until a *conditional* f3 code is shown to work: a code space is only worth a hash move if the
creature can be taught to use it, and naming is the thing §46 actually cares about. Clause 4 is the
2-word f3-only grid against a no-index control, and it needs its own section and its own count.

### §46 CLAUSE 4 RESULT — F3 CARRIES A CONDITIONAL CODE, AND RUN 1 WAS VACUOUS (2026-11-06)

`results/n46-f3grid.log` (run 1, **VOID**), `results/n46-f3grid-v2.log` (the result), gate at
`results/n46c4-verify.log`. Nothing shipped: `dna/default.toml` untouched, hash
**`ad96f882becbee92` unmoved**, `experiments 24 as expected`.

**RUN 1 PRINTED A FALSE NULL AND IT HAS TO BE RECORDED, NOT QUIETLY REPLACED.** It read
`+0.0181 ± 0.0105 (1.7 SE)` with the verdict *"f3 carries no conditional code -- the refusal is in
the conditional WRITE."* **That null agreed with `docs/CONCLUSION.md`, which is precisely what made
it dangerous.** It was an artefact: the genome carried `context_source = 4` and
**`context_slots = 0`**, so there were no prototypes, `active_context()` never returned an index
inside the word range, and the contrast was between two arms **neither of which had a condition**.
`ctxgrid` patches **both** fields at runtime (`slots = 2u, src = 4u`) and only one was copied.

**AND THE PRE-FLIGHT WAS WRITTEN IN A COMMENT TWENTY LINES FROM THE EDIT:** *"the genome must carry
`context_slots >= 2`, and `ctx_present_frac` must be checked NONZERO on a smoke BEFORE any budget is
spent. That check is the pre-flight."* The budget was spent without it. The instrument's own vacuity
check tested `context_source != 0` — **the wrong one of the two fields.** It is now a *measured*
clause 4a(i) that suppresses the verdict entirely when the index is dead, and it discriminates
**1.0000 / 0.9395 live** against **0.0000 / 0.0000 dead**.

**THE RESULT, on a live index:**

| arm | p3 (chance 0.500 by construction) | ctx_match (best-of-2, null > 0.5) |
| --- | --- | --- |
| **conditional** | **0.786 ± 0.032** | 0.915 (early 0.930 → late 0.884) |
| random | 0.544 ± 0.043 | 0.943 (early 0.953 → late 0.914) |

**Excess +0.2427 ± 0.0492 — 4.9 SE, 16 of 18 seeds positive.** Index live (`ctx_present_frac`
1.0000, `ctx_match` 0.9288), control **at chance** (+0.044, ~1 SE), power adequate (SE 0.0492
against the 0.055 ceiling set beforehand).

**So a two-word distinction carried PURELY on f3, on an index the creature derives for itself, is
named. F3 is the first axis beyond F1 and F2 shown to carry a conditional distinction.**

### TWO WAYS THE INSTRUMENT OVERCLAIMED, CORRECTED IN THE SOURCE

1. **It said the 19-signal code space is "cashable". It does not follow.** This is **one bit on one
   axis at two contexts.** Nineteen signals requires f3 to **compose** with f1 and f2, and §41
   measured f3 **taxing F1 by 11.3% of d'²** — so a three-axis code may lose on F1 what it gains on
   f3. **The composition run is the one that decides shipping, and it has not been run.**
2. **It invited a like-for-like comparison with `ctxgrid` that is not one.** ctxgrid's +0.126 and
   +0.082 are per-axis bits inside a **four-word** task; this is **two words on one axis**.
   `tutorgrid` established the derived index is good at two contexts and is the **binding limit** at
   four — and `ctx_match` here is ~0.93, the easy case. **A larger excess here means an EASIER TASK,
   not a better axis.** Quoting "twice ctxgrid's F1" would have been flattering and wrong.

What the ctxgrid comparison *does* still license, and only this: a **null** here would have been a
statement about the axis rather than the method, because the design and the measure are ctxgrid's.

### WHAT STAGE 2 NOW NEEDS, AND WHY IT IS STILL NOT A SHIP DECISION

The axis is **audible** (d' 5.272), **commandable** (2.68 levels), **teachable** (clause 3, 5.1 SE,
9/9) and **nameable** (clause 4, 4.9 SE, 16/18). What is missing is **composition**: a 2x2 grid with
f3 as one axis against `ctxgrid`'s F1xF2, on the variant, hash-neutral. Either f3 **adds** an axis —
and the vocabulary claim is real — or it **trades** against the first two, which §41's measured tax
makes a live possibility. §46's abandonment count is spent, so that is a new section.

## §47 — SPREADING VERSUS STRETCHING: DOES A THIRD AXIS BUY REACH BACK? (2026-11-06)

**Pre-registered before the build.** §46 established f3 is audible, commandable, teachable and
nameable. What it did **not** establish is that 19 signals are cashable, because that needs f3 to
**compose** with f1/f2 and §41 measured f3 **taxing F1 by 11.3% of d'²**. §47 is that question.

### THE DESIGN, AND WHY IT IS NOT THE OBVIOUS ONE

The obvious run — *"does turning f3 on cost F1/F2 naming?"* — is **refused on power before
building.** f3's tax is 11.3% of d'², so ~5.5% of d'; against `ctxgrid`'s +0.137 ± 0.023 excess that
is an effect of **0.0075**, needing SE ≤ 0.0037, i.e. **~1,400 seeds.** Vacuous.

The resolvable question is the **shape** of the code at matched size:

| code | words | axes | levels/axis |
| --- | --- | --- | --- |
| **3x3** on F1xF2 | 9 | 2 | 3 — *stretched* |
| **2x2x2** on F1xF2xf3 | 8 | 3 | 2 — *spread* |

Matched word count (8 vs 9) and matched index load (8 vs 9 contexts), so the only difference is how
the code is **shaped**. Each axis at its own measured reach around rest: F1 567/686, F2 1520/1820,
f3 2486/2741; the 3x3 adds the midpoints 626.5 and 1669.6.

### THE PRIMARY IS DELIVERED Hz, NOT NAMING EXCESS, AND ctxvocab IS WHY

`ctxvocab` ran 9 words against 6 **paired within creature with trials per word matched**, and only
two columns cleared 3 SE:

    F1 delivered Hz        -11.8 +- 3.5   (3.3 SE)   <- THE RESOLVABLE ONE
    total naming excess    -0.012 +- 0.019 (0.6 SE)  CONSERVED, uninformative

**So the primary is F1 delivered Hz.** The prediction is sharp: the spread code asks F1 for **two**
levels where the stretched code asks for **three**, so if levels multiply across independent axes
the spread code should **deliver more F1 separation** — recovering some of ctxvocab's 11.8 Hz.
Naming excess is reported as a secondary and is **expected conserved**, because ctxvocab says it is.

### AND THE OBJECTION THIS RUN HAS TO ANSWER, STATED FIRST

**`ctxvocab` ALREADY FOUND SHAPE NULL WITHIN TWO AXES: 2x3 minus 3x2 = −0.032 ± 0.027 (1.2 SE), and
"shape beats count" is WITHDRAWN.** So why should shape matter here? Because that comparison
**redistributed levels between the same two axes**, while this one **recruits a third**. §41
measured f3's cross-talk into the teachable axes at **≤ 0.007**, and the whole product argument is
that levels multiply across *independent* axes, not that they rearrange within dependent ones.
**If §47 is null, the honest reading is that recruiting an axis behaves like reshaping one — which
would retire the product argument behaviourally**, and that is a result worth the run.

It also means a null here must NOT be read as "f3 fails": f3 names at 4.9 SE. It would be read as
"a third axis buys no reach back", which is a statement about composition.

### CLAUSE ORDER

1. **INDEX LIVENESS AT 8 AND 9 SLOTS, MEASURED, BEFORE ANY BUDGET.** The pre-flight §46's clause 4
   skipped and paid for: `context_slots` must be set (**not just `context_source`** — `ctxgrid`
   patches both and copying one is what produced a false null), and `ctx_present_frac` must be
   **nonzero on a smoke**. Reported as a number per arm.
   **`ctx_match` IS REPORTED AND NOT GATED:** it is the best of k! permutations, so at 8 and 9
   contexts its null is far above 1/k and stating a naive bar against it would be
   [[aibaby-direction-null]]'s trap in the diagnostic column.
2. **TRIALS PER WORD MATCHED,** as `ctxvocab` did explicitly. 8 words against 9 means an unmatched
   design gives the smaller vocabulary more practice per word, which would move delivered Hz on its
   own and in the direction this run is looking for.
3. **POWER, STATED AS A NUMBER FIRST.** ctxvocab's SE on delivered Hz was **3.5 at n=36**, so n=36
   here. The effect to beat is its own **11.8 Hz**. Above SE **5.5** the contrast is NO VERDICT on
   power alone, read before the contrast.
4. **THE CONTRAST, THREE-WAY AND SPANNING THE SIGN.** Spread − stretched on F1 delivered Hz:
   **positive** → a third axis buys reach back, the product argument cashes, and shipping f3 is
   worth a hash move; **zero** → recruiting an axis behaves like reshaping one, and the product
   argument does not cash behaviourally; **negative** → f3 costs more than it saves, and §41's
   measured tax is the named suspect.

### ABANDONMENT COUNT: ONE RUN

§47 is one paired comparison. It decides stage 2 either way and opens nothing by itself.

### §47 AMENDMENT, BEFORE THE BUILD: BOTH ARMS GET THE SAME THREE-TERM SCORER

Written while implementing, before any run. The pre-registration implied the 3x3 arm would use the
existing two-formant joint error and the 2x2x2 arm a three-formant one. **That would confound the
primary.** `formant_error` is a SUM of per-axis log terms, so a three-term error gives F1 one third
of the gradient where a two-term error gives it one half — and the primary is **F1 delivered Hz**.
The spread arm would deliver less F1 for a reason that has nothing to do with axis count, and the
bias runs **against** the prediction, which makes it a conservative confound but a confound.

**So both arms use the SAME three-formant joint error**, and the stretched arm simply never varies
the third axis: all nine of its words carry `f3 = 2613.7`, the measured rest. Identical physics
(one genome with `render_f3_gain = 1.0`), identical scorer, identical term count, identical gradient
share per axis. **The only difference between the arms is whether the code USES the third axis** —
which is the question.

This needs one new scorer (`formant_error3`) and one new score mode (`kVLScore3`), both **additive**:
appended enumerator, new function, every existing caller untouched, and `verify` asked to prove the
hash does not move.

**AND `render_f3_gain = 1.0` IN BOTH ARMS IS DELIBERATE.** The question is not *"is f3-on better than
f3-off"* — §46 already showed f3-on gives a nameable axis. It is *"given a creature that has three
audible axes, is it better to spread a vocabulary across them or stretch it over two?"* Running the
stretched arm on the same physics removes the between-genome contrast
[[aibaby-vocal-calibration-invariant]] would otherwise void.

### §47 SECOND AMENDMENT: `ctxgrid`'s `sp1` IS NOT COMPARABLE ACROSS GRID SHAPES

Also before any run, and also found by reading the existing estimator instead of assuming it.
`ctxgrid`'s delivered-separation statistic is

    for every word PAIR (x,y):  s1 += |mean F1(x) - mean F1(y)|;  sp1 = s1 / pairs

— a mean over **all** pairs, including pairs that share an F1 target and should contribute ~0. In a
2x2x2 the fraction of pairs differing on F1 is 4/7 of the 28 pairs; in a 3x3 it is 6/9 of the 36,
at **two different asked separations** (59 and 118 Hz). **So `sp1` mixes "pairs that should differ"
with "pairs that should not" in different proportions in the two arms, and a difference in it would
be partly the grid's pair structure rather than the creature's delivery.**

**THE PRIMARY IS THEREFORE THE EXTREME-LEVEL CONTRAST:** mean delivered `|ΔF1|` between the words at
the **lowest** and **highest** F1 target. Both arms ask for the same **118 Hz** there (567.5 vs
685.5), so it is one asked separation against one delivered separation, dimensionally identical in
both. Reported in Hz and as **delivered/asked**, with the same pair of numbers for F2 (asked 300 Hz
in both) as the secondary.

**AND THE CONSEQUENCE FOR THE PRE-REGISTRATION'S "11.8 Hz TO BEAT":** that figure is `ctxvocab`'s on
`sp1`-style accounting, so it is an **order-of-magnitude expectation here and not a bar**. §47's
verdict rests on its own within-run paired contrast, which is the only sound comparison available.

### §47 RESULT — STRETCHING WINS, AND §47 VIOLATED THE PRECONDITION IT WAS TESTING (2026-11-06)

`results/n47-f3comp.log`, gate at `results/n47-verify.log`. Nothing shipped.

**CLAUSE 1 PASSES:** index live in both arms, `ctx_present_frac` **1.0000** at 8 and at 9 slots.
`ctx_match` reads **0.627 / 0.544** there against **0.93 at two slots** — a sharp fall that matches
`tutorgrid`'s finding that the index is the binding limit past four contexts. Reported, not gated.

**CLAUSE 3 PASSES:** SE on the paired contrast **1.43 Hz** against the 5.5 ceiling.

| arm | delivered \|ΔF1\| | of the 118 Hz ask | delivered \|ΔF2\| |
| --- | --- | --- | --- |
| SPREAD (2x2x2, 3 axes) | **5.95 ± 0.75** | **5.0%** | 18.38 ± 2.21 |
| STRETCHED (3x3, 2 axes) | **11.14 ± 1.27** | **9.4%** | 25.16 ± 3.47 |

**Paired: −5.19 ± 1.43 Hz, −3.6 SE, 11 of 36 seeds positive.** F2 the same sign at −1.6 SE.
**The two-axis code delivers nearly twice the F1 separation of the three-axis one.**

### THE SUSPECT I PRE-REGISTERED IS THE WRONG ONE

Clause 4 named §41's **11.3% of d'²** tax on F1. It cannot do this work: the delivery drop is
**−47%**, four times too large, and a d'² tax on a rendered posture is a different quantity from how
much separation a lesson delivers. **Naming a suspect in advance does not make it the cause**, and
the pre-registration should have required the magnitudes to match.

### THE REAL MECHANISM IS DOCUMENTED, AND §47 VIOLATED ITS OWN PRECONDITION

`aibaby-nine-axes-lead` note 4 — in the memory §47 was built to test — reads:

> **TEACHING STAYS SEQUENTIAL**: two dimensions share one reward channel (0.58 vs A-then-A).
> **More axes multiply the CODE SPACE, never the teaching rate.**

**§47 taught three axes SIMULTANEOUSLY.** `aibaby-capacity` prices that at **0.58 simultaneous
against 0.84 A-then-A**. So §47 measured the known cost of a shared reward channel on the naming
task — a real quantification, and **not** a refutation of the product argument, whose stated
precondition it broke. The product argument is about **code space**; it never claimed three axes
could be taught at once.

**AND THE SECOND AMENDMENT'S "IDENTICAL GRADIENT SHARE" CLAIM WAS WRONG.** Both arms use the same
three-term scorer, but **the stretched arm's f3 term is constant across all nine of its words**, so
it contributes no gradient — effectively **two** varying terms against the spread arm's **three**.
The scorer's *form* was matched and its *effective* share was not. That confound pushes in the same
direction as the result, so **in this design it is not separable from it.**

### WHAT SURVIVES, AND IT DECIDES STAGE 2: DO NOT SHIP f3

The actionable conclusion holds regardless of which of the two accounts dominates, because both
describe the protocol the caregiver actually uses. **Under simultaneous teaching a three-axis code
delivers LESS than a two-axis one**, so turning on `render_f3_gain` would make the creature worse at
the task it is scored on, whatever the code-space arithmetic says. **Stage 2 is decided: f3 does not
ship**, `dna/default.toml` keeps two resonators, and the reason is measured rather than inherited.

That leaves §46's results standing and unembarrassed: f3 **is** audible, commandable, teachable and
nameable. What fails is the step from *"an axis exists"* to *"a vocabulary spread across it is
better"*, and it fails on the **teaching channel**, not on the axis.

### THE SUCCESSOR, AND IT IS THE PROPER TEST OF THE PRODUCT ARGUMENT

**Sequential teaching**: the F1 bit, then F2, then f3, against the same stretched control.
`aibaby-capacity`'s 0.84-against-0.58 is the size of the prize, and it is the arm §47 should have
run beside the simultaneous one. It needs its own section and its own count. **Until it is run, the
product argument is neither cashed nor refuted — it is untested at its own stated precondition.**

## §48 — SEQUENTIAL TEACHING: THE PRODUCT ARGUMENT AT ITS OWN PRECONDITION (2026-11-06)

**Pre-registered before the build.** §47 found a three-axis code delivers half the F1 separation of a
two-axis one — and then found it had **broken the precondition of the argument it was testing**.
`aibaby-nine-axes-lead` note 4: *"TEACHING STAYS SEQUENTIAL: two dimensions share one reward channel
(0.58 vs A-then-A). More axes multiply the CODE SPACE, never the teaching rate."* §47 taught three
axes simultaneously. §48 teaches them one at a time.

### WHY SEQUENTIAL SHOULD WIN, AND THE MECHANISM IS ALREADY MEASURED

`axispower` established at **+11.5 SE** that a lesson **silent about another's axis costs it
nothing** (1.024 ± 0.062 against 0.116 ± 0.048). Per-axis scoring makes each lesson silent about the
others by construction. **So sequential teaching is the protocol that turns three competing lessons
into three mutually silent ones**, and `aibaby-capacity` prices the difference at **0.58 simultaneous
against 0.84 A-then-A**. If the product argument cashes anywhere, it cashes here.

### FOUR ARMS, AND TWO OF THEM ARE REPRODUCTION CHECKS

One genome, `render_f3_gain = 1.0`, `context_slots` patched to the word count per arm.

| arm | code | schedule | budget |
| --- | --- | --- | --- |
| **SEQ-TOTAL** | 2x2x2, 8 words | axis rotates by session third: F1 → F2 → f3 | x1 |
| **SEQ-AXIS** | 2x2x2, 8 words | same rotation | **x3** |
| **SIMUL** | 2x2x2, 8 words | all three axes every trial | x1 — **reproduces §47's spread arm** |
| **STRETCH** | 3x3, 9 words | all axes every trial, f3 at rest | x9/8 — **reproduces §47's stretched arm** |

**SIMUL and STRETCH must return §47's whole-session numbers — 5.95 ± 0.75 and 11.14 ± 1.27 Hz — or
nothing else in the run is readable.** Two arms paying for themselves as a gate.

### THE MATCHED-WHAT QUESTION HAS NO CLEAN ANSWER, SO BOTH ARE RUN

Sequential teaching at a fixed total budget gives each axis **one third** the trials the simultaneous
arm gives it. Scaling the budget by three equalises trials per axis but triples the **total**
teaching. Neither is "the" control:

- **SEQ-TOTAL vs SIMUL** asks *given the same total teaching, is it better to sequence or interleave?*
  A win here is strong, because sequencing wins **despite** a third the trials per axis.
- **SEQ-AXIS vs SIMUL** asks *given the same teaching per axis, does sequencing help?* A win here is
  weaker, because the arm also received three times the total teaching.
- **A loss in SEQ-TOTAL with a win in SEQ-AXIS** is the interesting middle: it would say sequencing
  helps but does not pay for the trials it costs, which is a statement about the budget rather than
  about the schedule.

Stating all three readings now is the point; picking the favourable one afterwards is
[[aibaby-verdict-fitted-to-data]].

### THE MEASUREMENT WINDOW, AND WHY IT IS THE FINAL THIRD

Delivered `|ΔF1|`, `|ΔF2|` and `|Δf3|` between each axis's **extreme levels**, over the **final third**
of scored utterances — the state the creature ends in. For the sequential arms that makes F1 the
**oldest** lesson and f3 the freshest, so **the primary is also a retention test**, which is exactly
the capacity question: `axispower` predicts the later lessons do not wipe F1 because they are silent
about its axis. The **whole-session** window is reported beside it, because that is the one §47
measured and the one the reproduction check needs.

### CLAUSE ORDER

1. **REPRODUCTION.** SIMUL and STRETCH against §47 on the whole-session window.
2. **INDEX LIVENESS AT 8 AND 9 SLOTS, measured** — `ctx_present_frac` nonzero per arm. The pre-flight
   §46 skipped and paid for. `ctx_match` reported, **not** gated: best-of-k! at 8-9 contexts.
3. **THE ROTATION MUST ACTUALLY ROTATE.** A wiring check with a firing branch: the scored axis per
   session third, printed, plus the count of feedback events in each third. A schedule that silently
   never switched would make SEQ identical to a one-axis lesson and its null a false refusal.
4. **POWER before the contrast.** §47's SE on the paired dF1 contrast was **1.43 Hz** at n=36, so
   n=36. Above SE **5.5** it is NO VERDICT on power alone.
5. **THE CONTRASTS, THREE-WAY AND SPANNING THE SIGN**, in the order above, each against SIMUL paired
   by seed.

### ONE LIMITATION DISCLOSED NOW

**The order is fixed at F1 → F2 → f3.** So "sequential" is confounded with "f3 taught last", and f3
being freshest at the measurement window flatters its own delivery while F1's must survive two
intervening lessons. Randomising the order would triple the arms; instead **all three axes are
reported separately**, so an ordering effect is visible as an asymmetry rather than hidden in a
single number. If the result hinges on it, the permuted order is the successor.

### ABANDONMENT COUNT: ONE RUN

### PRE-FLIGHT, 240 000 ticks, `--allow-short` (2026-11-06)

Run before the real budget, exactly as the clause order demands. **Clauses 1-3 all pass:**

| clause | reading |
|---|---|
| 1 REPRODUCTION | SIMUL **7.29 ± 0.96** vs §47's 5.95 (+1.34, inside 3 SE); STRETCH **11.48 ± 1.38** vs 11.14 (+0.34). Both reproduce. |
| 2 INDEX LIVE | `ctx_present_frac` **1.0000** in all four arms. `ctx_match` 0.51-0.65, reported not gated. §46's false null cannot recur here. |
| 3 ROTATION | SEQ-TOTAL 136/124/125 feedback events on axes 1/2/3; SEQ-AXIS 388/387/381 on 1/2/3. **The schedule rotates.** |

The contrasts at a fifth of the budget are all null, with SEQ-TOTAL − STRETCH at **−3.45 ± 1.95 Hz
(−1.8 SE, 11/36 positive)** — stretching still ahead, the same direction §47 found. Not a result: a
short run cannot pass whatever it reads, and the binary exits 1 by design to say so.

### TWO BUGS THE PRE-FLIGHT CAUGHT IN THE INSTRUMENT ITSELF

**1. Every standard error in the final-third table printed `0.00` over 36 varying seeds.** The cause
is argument evaluation order:

```cpp
std::printf("%7.2f +- %5.2f", v1.empty() ? 0.0 : ctx_mean_se(v1, &e1), e1);   // WRONG
```

C++ does not order `printf`'s arguments, so `e1` was read before the call that writes it. The means
were right and the paired contrasts — which compute into a local first — were right, which is what
made it survive a reading: **an SE of zero beside a plausible mean is the quietest wrong number this
file can print**, and it would have been quoted as "the arms do not differ, with no scatter at all".
A scanner over all 14 host sources found this as the **only** instance in the codebase, and it was
three hours old. Fixed by computing each mean into a local first. I then wrote the identical bug
again inside the fix, which is recorded where the fix is.

**2. A sentinel was being averaged as Hz.** `sep()` returns `-1.0` for "this arm's table has no two
extreme levels on that axis", which is true of f3 in STRETCH by construction. Averaged over 36
STRETCH cells it printed **`-1.00`** under a column headed `ask 255` — indistinguishable from a
measured collapse. `d1`/`d2`/`w1` cannot carry it because the cell is dropped earlier, so only f3 was
exposed, but the filter is now by value in `col()` **and in `paired()`, asked of both sides**, which
also catches the case where only *some* seeds fail to reach an extreme. An empty column now prints
`none scored` rather than `0.00 ± 0.00` — the smoke printed that silent zero for three of four arms
and it reads the same wrong way.

Both are display defects, not measurement defects: no published number changes. They are written
down because both produced a **plausible, quotable, wrong** number rather than an error.

### CLAUSE 3 IS HALF A TAUTOLOGY — DISCLOSED BEFORE THE RESULT IS READ

Re-reading the implementation while the run was in flight: clause 3 prints two things, and only
one of them can fail.

```cpp
if (seq_phase < 3u) {
  ++out.seq_fb[seq_phase];
  out.seq_axis[seq_phase] = seq_phase == 0 ? 1u : (seq_phase == 1 ? 2u : 3u);
}
```

- **The feedback counts per third are a real check.** A phase that received zero feedback events
  would print `0` and fire `*** DID NOT ROTATE`. That is what the pre-flight's `136 / 124 / 125`
  and `388 / 387 / 381` establish, and it is the thing that matters: every third was taught.
- **The axis triple `1 / 2 / 3` cannot fail.** `seq_axis[k]` is a pure function of `seq_phase` by
  the same constant mapping, so it reports that the code says what the code says. It is
  **decoration, not evidence** — exactly the `Expect::kRead` failure mode: a check that cannot
  fail for a measured reason is not a check.

What *is* established independently is better than the tautology anyway: **the feedback objective
and the trial error are the same rotating expression**, verified by reading both — the feedback
selection and the `err =` assignment use an identical `seq_phase` ternary over an identical axis
mapping. That is the property §46 clause 3a lost when `err_early`/`err_late` stayed the F1/F2 joint
error under an f3 objective and printed "THE REWARD NEVER MOVED" about a run that had moved +33 Hz.

**So §48's verdict may not claim "clause 3 confirmed the objective rotated".** It may claim that
all three session thirds received feedback, and that the scored and optimised quantities are the
same by construction. A falsifiable version — tracking the measured error on all three axes per
phase and checking that the optimised one is the one that declines — is the successor's to build,
and it is strictly more informative than what is printed here.

### §48 RESULT: THE PRODUCT ARGUMENT DOES NOT CASH, AND SEQUENCING IS NOT WHAT WAS MISSING

Full run, 1 200 000 ticks, n=36 paired seeds per arm. **Verify gate held** on all four keys, checked
by grep rather than by eye: `verify PASS`, `hash ad96f882becbee92 as recorded`, `pinned hash PASS`,
`experiments 24 as expected, 0 not` (`results/n48-verify.log`). The gate ran AFTER these numbers were
written down, and the sentence it replaces said so rather than claiming a pass in advance.

| contrast | what it isolates | result | verdict |
|---|---|---|---|
| SEQ-TOTAL − SIMUL | schedule, **matched budget** | **+0.83 ± 1.39 Hz** (0.6 SE), 19/36 | **NULL** |
| SEQ-AXIS − SIMUL | schedule **+ 3× budget** | +5.44 ± 1.47 Hz (3.7 SE), 23/36 | wins, confounded |
| SEQ-TOTAL − STRETCH | **the pre-registered question** | **−4.53 ± 1.78 Hz (−2.5 SE)**, 15/36 | **SEQUENCING LOSES** |

**The answer is no.** A three-axis code taught one axis at a time still delivers less F1 than a
two-axis code stretched, by −2.5 SE — the same direction as §47's −3.6 SE under simultaneous
teaching. §47's finding survives the repair of the precondition it broke. `nine-axes` note 4's
"teaching stays SEQUENTIAL" is a true statement about the teaching *rate* and does **not** rescue
the axis count.

### THE DECOMPOSITION (POST-HOC, `tools/n48_decompose.py`)

The one positive contrast differs from its control in two ways at once, so it was decomposed. Note
that paired differences on common seeds telescope, so the two parts **sum to the joint effect by
arithmetic, not by evidence** — what is informative is the allocation, not the sum.

| | isolates | dF1 | dF2 | df3 |
|---|---|---|---|---|
| SEQ-TOTAL − SIMUL | schedule | +0.83 ± 1.39 (0.6 SE) | — | — |
| SEQ-AXIS − SEQ-TOTAL | **budget, matched schedule** | **+4.61 ± 1.74 (2.7 SE)** | **+20.94 ± 3.69 (5.7 SE)** | **+19.31 ± 4.11 (4.7 SE)** |

So of SEQ-AXIS's +5.44 Hz over SIMUL, the schedule contributes +0.83 (null) and the budget +4.61.
**What buys delivered reach is teaching time, not how the lessons are ordered.**

And the sharpest line in the run, also post-hoc:

> **SEQ-AXIS − STRETCH = +0.09 ± 2.25 Hz (0.0 SE), 18/36.** A three-axis sequential arm with **3×
> the teaching** exactly ties a two-axis stretched arm with **1.125×**. Stretching reaches the same
> F1 separation for roughly **2.7× less teaching**.

### WHAT THIS MEANS, AND WHY `axispower` WAS NOT WRONG

The retention half of the primary came out as `axispower` predicted. F1 is the **oldest** lesson in
the sequential arms, two intervening lessons behind the measurement window, and it did not decay:
SEQ-TOTAL **8.83 ± 1.07** against SIMUL's **8.00 ± 0.92**. A lesson silent about another axis really
does cost that axis nothing.

**It also buys nothing.** That is the mechanism, and it closes the line more honestly than a tally:

> **Silence between axes is free, and free is not the same as profitable.** Adding an axis multiplies
> the code *space* and divides the teaching *time*. Reach is bought with time. So the product in
> §41's arithmetic is real about how many signals the air could carry and says nothing about how far
> the voice will move on any of them.

### CLAUSE 1 WAS A REFACTOR CHECK, NOT A REPLICATION — AND THE ±0.00 IS WHY

SIMUL came back at **5.95 ± 0.75** against §47's 5.95, and STRETCH at **11.14 ± 1.27** against 11.14
— equal in both the mean *and* the standard error, to the last printed digit. That is not striking
agreement; it is the expected output, and reading it as replication would have been an error. The
two arms are **bit-identical computations** to §47's: same genome file, same creature seeds
(`header().seed + r*7919`), same word tables via `fc_table`, same `kVLScore3`, same `×1` and `×9/8`
budgets. The differing "trial seed" in the two headers (`135db8e` vs `135d281`) is
`header().seed ^ <per-instrument constant>` — **a label that does not drive these arms**, so the
boilerplate note about "another probe is another trial sequence" does not apply to them and the
`3.0 × e` tolerance could never have been a real test.

What the check *is* worth is considerable, and it is the reason to keep it: §48 added `f3_sum`,
`seq_phase` and a new enum value into the **shared** `run_vocallearn_session`. Had any of that
perturbed the existing `kVLScore3` path — one extra RNG draw would do it — SIMUL and STRETCH would
have moved. They did not move at all. **It is a refactor-invariance proof on a shared code path**,
which is exactly what this project needed after §46's bugs and should be labelled as such.

An independent recomputation of all three pre-registered contrasts straight from the per-seed rows
(`tools/n48_decompose.py`, separate parser and estimator) returns +0.83 ± 1.39, +5.44 ± 1.46 and
−4.53 ± 1.78 — agreeing with the instrument to rounding on two code paths.

### STATUS OF THE AXIS-COUNT LEAD: CLOSED BEHAVIOURALLY

Three sections asked whether more axes buy more behaviour. §46: f3 is audible, teachable and
nameable. §47: spreading loses to stretching under simultaneous teaching. §48: it loses under
sequential teaching too, and sequencing at matched budget is null. **`render_f3_gain` stays 0.0.**
The remaining open question is not about axes at all — it is why teaching time converts to reach so
inefficiently, which is the `occupancy-ceiling` exponent, already measured at `dF1 ~ aligned^0.61`.

## §49 — CAN THE CREATURE INDEX ITS VOICE ON WHAT IT SEES? (2026-11-06, pre-registration)

### THE REDIRECTION, STATED BEFORE THE DESIGN

The direction given was "the vision front end — make it understand and speak the object it sees".
**The front end was already rebuilt and it is not the lever.** `shapeprobe` priced a radial mass
profile outside the brain (0.960 pooled against the shipped retina's 0.570, 4/4 seeds) and said
BUILD; **DNA v46 built it**; the outcome was *delivery moves a LOT — central +0.160, vocal +0.115,
oracle ×3, all 4/4 — and the milestone does not* (+0.085 ± 0.071 on one arm of four). The seen
object reaches the larynx, the `vision→vocal` tract **ships**, and G2 is 9 of 9. G3 is settled
negative for a reason measured elsewhere: **conditionality**. `g2cond` made G2's own contingency
conditional and got +0.022 ± 0.153 against a positive control of +0.369 ± 0.031.

So the goal is right and the target moves one stage: not *a better visual code*, but **an index read
off the visual code**.

### THE MECHANISM, AND WHY IT IS NOT THE ROUTE THAT FAILED

`CONCLUSION.md`'s spine: **this architecture can learn a bias per context and cannot learn the
context.** Where the index is *supplied* the creature names — four words **+0.160 ± 0.032**, nine as
well as six, and the ear's own rate EMA reaches **100.1% of a perfect index's lift**. DNA v53 supplies
it as `context_source = 4`: a **competitive partition of the auditory code**, formed while the word
plays and **latched** across the silence.

**There is no visual source.** The kernel's role selector offers `kContext` (0), `kVocal` (1) and
`kAuditory` (≥2), and `dna.cpp` refuses `context_source > 4` and `== 3`.

Every failed G3 route **transmitted** the object through a tract, and the object code is
**pooling-fragile** — a random sparse tract is a low-pass filter whose capacity *falls* with density,
while the word code survives the same tract. **The context source transmits nothing.** The kernel
says so itself:

> *"this source needs no projection anywhere, which is the whole reason this costs the larynx nothing
> where DNA v47's tract cost it everything."*

It reads a population **in place** and indexes a bias table. That is a different operation from
every route in the G3 cluster, and it is the operation that already works for hearing.

### ONE ASYMMETRY IN VISION'S FAVOUR, WHICH IS WHY THIS IS WORTH A RUN

`ctxsrc` named persistence **THE LAST PIECE**: the word is at ceiling **1.000** while it plays and at
**chance 0.533** by the reward window, which is the whole reason source 4 needs a latch. **A SEEN
OBJECT DOES NOT STOP.** It is still there when the reward arrives. Vision starts ahead on precisely
the dimension that was audition's final blocker.

### THE PROBE: `visprobe`, READ-ONLY, NO LEARNING OF ANY KIND

Does a fixed partition of a visual population recover **which object is being seen**, in the reward
window? Three populations × three rules.

| | |
|---|---|
| populations | `kVision` (retina), `kVisualCortex` (V1), `kVisualForm` (v46's curve-radius stage) |
| the shippable rule | v53's competitive partition — unsupervised, fixed, latched |
| the upper bound | a supervised held-out readout |
| the null | shuffled labels |

### CLAUSE ORDER, EACH A BRANCH IN THE CODE

1. **VACUITY, MEASURED PER POPULATION.** Each module must exist and be active in the window. A
   missing source module sets `ctx_module_ < 0` and the index then *silently never fires* — `areax`
   and `ctxself` check for this rather than trusting it, and so does this.
2. **THE NULL MUST SIT AT CHANCE.** A nearest-prototype estimator gains from width alone: `vocab`'s
   126-dimension shuffled control read **0.149** against a chance of 0.125. Printed, not assumed.
3. **THE SUPERVISED CEILING IS AN UPPER BOUND, NOT A PREDICTION.** `partprobe`'s lesson: the
   supervised ceiling needs the labels the creature is trying to infer — a circularity, not a tuning
   problem. Reported as a bound and never as the shippable number.
4. **THE SHIPPABLE NUMBER** is the unsupervised fixed partition, against the auditory partition's own
   `ctx_match` at matched object count (§48 measured 0.5442–0.6590 at eight and nine words; its null
   is above 1/k because it is a best-of-k! match, so it is reported and never gated on).
5. **THE DISPLACEMENT REGIME, STATED NOT ASSUMED.** The object code **dies at 2.6 px — one foveal
   cell** — and position tolerance is **not learnable** (`invprobe`: jittered minus centred −0.025
   over 3 seeds). So the same measurement runs at 0 px and at ≥2.6 px. Every visual number in this
   project was measured inside that window; this one prints its window.

### THE THREE-WAY, SPANNING THE SIGN

- **At or above the auditory level** → build `context_source = 5`: one role in the selector, one
  validation bound. Everything downstream (`ctx_proto_`, `ctx_acc_`, the latch, the per-context
  table) is role-agnostic and walks whichever module was selected.
- **At chance** → the **representation** is the loss, exactly as `partprobe` found for audition, and
  the front end *is* the lever after all — which vindicates the original instinct and points back at
  the radial profile.
- **In between** → report where, and price the build against the `dF1 ~ aligned^0.61` transfer curve
  **before** building it. `ceiling-is-an-exponent`'s rule: compute a gate's power against the
  transfer curve first, or the null is guaranteed by the gate's own arithmetic.

Both outer outcomes point at a *different* lever, so this is a real gate and not one that can only
say "build it" — `shapeprobe`'s own standard.

### ABANDONMENT COUNT: ONE PROBE

If `visprobe` refuses, the index route to naming what it sees is closed and the remaining question is
the representation, not the mechanism.

### §49 AMENDMENT, BEFORE THE BUILD: THE VISUAL CODE IS ALREADY MEASURED AT 0.927

Applying this project's own rule — grep the registry for the STAGE before calling a quantity
unmeasured — `g3probe` already carries a `vision` row with a shuffled control, and
`experiments_probes.cpp` states the cascade in prose:

> *"At 141 probes they are vision 0.927 -> central 0.724 (-0.203) and then central -> voice
> (-0.237), so the larger loss is the downstream one."*

**So clause 4's premise is half-answered and the answer is good.** A supervised per-neuron readout of
the visual population recovers which object is being shown at **0.927**, against a 0.700 bar the
instrument already checks. The object is legible *before* any tract. What costs 0.44 is carrying it:
`vision 0.927 → central 0.724 → voice ~0.487`.

**This strengthens the design rather than retiring it, and it sharpens what `visprobe` must ask.**
The mechanism proposed here is the one operation in the architecture that does **not** cross a
tract — it reads a population in place and indexes a bias table. The measured cascade says that is
exactly where the 0.44 goes.

**What is still genuinely unmeasured** is the gap `partprobe` found for audition: a *supervised*
ceiling is not the *shippable* number, because v53's partition is **unsupervised, fixed and
latched**. For the motor state that gap was 0.740 supervised against 0.561 per-neuron, and
`partprobe`'s verdict was that the representation, not the cut, carried the loss. So:

| | measured | still unknown |
|---|---|---|
| visual population | supervised per-neuron **0.927** | what v53's unsupervised partition gets on it |
| auditory population | **1.000** while the word plays, **0.533** by the reward window | — (shipped, 100.1% of a perfect index's lift) |

**And the derived expectation, which is the reason to run it.** The auditory route reaches 100.1% of
a *perfect* index's lift from a code that is at **chance by the reward window** and has to be latched
to survive. Vision's code is at **0.927 in that window and does not need a latch, because the object
does not stop.** If index quality is what the per-context table consumes, vision enters this with a
better signal than the route that already works.

**Clause 4's bar is therefore set here, in advance:** the unsupervised partition on a visual
population must reach the auditory partition's own `ctx_match` at matched object count (§48: 0.5442
to 0.6590 at eight and nine words, reported never gated, since best-of-k! puts its null above 1/k).
Below that and the cut is the loss; at or above it and the build is licensed. **The 0.927 is quoted
as a bound on what the cut could recover and never as the expected result** — that is `partprobe`'s
lesson and it applies to its own successor.

### §49 DESIGN CHANGE, DISCLOSED AS SUCH: TWO ARMS ADDED AFTER SEEING SMOKE DATA

The pre-registration above describes two arms, RETINA and EAR. The probe that will run has
**seven**, and five of those were added after looking at output. A smoke is not a measurement — the
binary exits 1 and says so — but the design it produced was still shaped by data, and recording that
is the difference between a pre-registration and a story.

**What the first smoke showed, and why it changed the instrument.** RETINA read `ctx_match` exactly
**0.5000** against a null of exactly **0.5000**, and clause 5 printed *"the cut is the loss, the
front end IS the lever after all"*. **That verdict was wrong.** With the labels alternating by
construction at k=2, an exact 0.5000 is the signature of an index **pinned to one slice**. Adding an
occupancy column showed it: RETINA **1.0000**, every probe in a single slice, against EAR's 0.6800.

`ctx_present_frac` could not see it, and that is the general lesson rather than a detail: **a
constant index is perfectly live.** This is `liveness-is-not-dose` landing on its own author. The
occupancy measure is now part of clause 1 with its own refusal branch.

**The second smoke, seven arms, is a 2×2 of {conscience} × {first-sample seeding}** — gates the
kernel already carries, which source 5 inherits through `context_source_is_ema()`:

| arm | occupancy | ctx_match | null | excess |
|---|---|---|---|---|
| EAR g0 | 0.6800 | 0.7800 | 0.5653 | **+0.2147** |
| RET g0 | **1.0000** collapsed | 0.5000 | 0.5000 | +0.0000 |
| RET g2 | 0.6500 alive | 0.5300 | 0.5788 | −0.0488 |
| RET g3 | 0.6700 alive | 0.5100 | 0.5784 | −0.0684 |
| RET g4 | **1.0000** collapsed | 0.5000 | 0.5000 | +0.0000 |

**THE CONSCIENCE IS WHAT KEEPS A VISUAL PARTITION ALIVE, and the seeding is neither necessary nor
sufficient.** g2 and g3 carry the conscience and balance; g0 and g4 do not and collapse totally.
That differs from the auditory case, where the kernel's own note says the conscience *"cannot RESCUE
one that died at initialisation"* and gate 3's seeding was the proposed fix. On the retina the
conscience alone suffices.

**And with the partition demonstrably alive, `ctx_match` is still at its null** while a supervised
readout of the same runs reads 0.76–0.84. Note how much work the label-destroyed null is doing: it
*rises* to 0.578 once the index varies, because best-of-k! has more to optimise over. Against a
naive 0.5000 the g2 arm would have been read as a +0.03 effect.

**WHY TWO MORE ARMS, AND WHAT THEY DECIDE.** An unsupervised split of retinal **rates** takes the
largest variance direction. `m3_toy` supplies two that dwarf disc-versus-square — position
**±1.28 px** and half-width **0.095–0.115 (~20%)** — while the shapes are deliberately
**area-matched** by `kAreaMatch` so brightness carries no signal at all. So a partition that is alive
and uninformative is exactly what splitting on *size* would look like. `RET g2 H` and `RET g3 H`
hold position and size fixed; `m3_toy_fixed` is new and the jittered draw still runs in both, so the
rng stream is identical and the arms differ only in where the toy is.

**This is the discriminator the pre-registration already promised** under "one interpretation limit,
disclosed before the result" — it named `m3_toy_at` as the tool. It is being run as part of the same
section rather than deferred, because a null without it cannot distinguish *"the object is not in
the rate code"* from *"the partition split on size"*, and those point at opposite next steps.

### §49 RESULT: THE CREATURE DERIVES A VISUAL INDEX, AND IT IS BETTER THAN ITS AUDITORY ONE

Full run, 900 000 ticks, 5 creatures × 9 arms, **705 probes per arm**. Verify gate held on all four
keys (`verify PASS`, `hash ad96f882becbee92 as recorded`, `pinned hash PASS`, `experiments 24 as
expected, 0 not`) — `results/n49-verify.log`.

| arm | occupancy | supervised `vision` | `ctx_match` | null | excess |
|---|---|---|---|---|---|
| EAR g0 | 0.7262 | 0.496 | 0.7674 | 0.5233 | **+0.2441** |
| RET g0 | 0.7901 | 0.927 | 0.5092 | 0.5243 | −0.0150 |
| RET g2 | 0.5348 | 0.949 | 0.6099 | 0.5330 | +0.0769 |
| RET g3 | 0.5291 | 0.932 | 0.6326 | 0.5354 | +0.0972 |
| RET g4 | 0.9277 | 0.938 | 0.5050 | 0.5153 | −0.0104 |
| **RET g2 H** | 0.5092 | **1.000** | **0.8624** | 0.5346 | **+0.3278** |
| RET g3 H | 0.5248 | 1.000 | 0.8383 | 0.5328 | +0.3055 |

**A competitive partition of the retina recovers which object is being shown at `ctx_match` 0.8624
against a measured null of 0.5346 — and the ear, the mechanism that already names, reads 0.7674
against 0.5233.** The visual index is the better of the two.

### IT SURVIVES THE CONTROL THAT WOULD HAVE VOIDED IT

Probe labels alternate upstream by construction, and this codebase already warns that an alternating
sequence lets a readout score off session time alone (`holdout_guess_time_corr`). A label-shuffled
null **cannot** catch that, because shuffling breaks a spurious parity correlation exactly as it
breaks a real one. So the probe order was drawn instead, and the question asked of **both** senses:

| | alternating | order drawn | retained |
|---|---|---|---|
| RET g2 H | +0.3278 | +0.3182 | **97%** |
| EAR g0 | +0.2441 | +0.2002 | **82%** |

The effect is the stimulus, not the schedule. (The first version of this clause paired `RET g3 H`
against `RET g2 HS` — arms differing in gate as well as order. Same verdict, but a control that is
not like-for-like is not a control, so it now pairs on source, gate and hold with the order as the
only difference.)

### TWO CONDITIONS, BOTH MEASURED

1. **DeSieno's conscience is necessary.** Without it the partition drifts to collapse and carries
   nothing: g0 reads −0.0150 at occupancy 0.7901 and g4 −0.0104 at 0.9277, while the conscience arms
   sit at occupancy ~0.53 and carry +0.077 to +0.097. **The first-sample seeding is neither necessary
   nor sufficient** — g4 (seeding alone) collapses, and g3 ≈ g2 throughout. That is the *opposite*
   weighting from the auditory case, where the kernel's own note says the conscience "cannot RESCUE
   one that died at initialisation" and gate 3's seeding was the proposed fix.
2. **Position and size must be held, and this is the dominant term: 4.3×** (+0.0769 → +0.3278). It is
   also the regime this project already lives in — every visual number here was measured inside a
   1.4 px window and the code dies at 2.6 px — but it is a real scope limit and it is the reason
   `m3_toy_fixed` exists. A caregiver holding a toy still is the setting this result is about.

### WHAT IT IS NOT

**This is not naming the seen object.** The index is the *input* to the per-context bias table. The
conclusion's spine — *this architecture can learn a bias per context and cannot learn the context* —
is not broken by this: what §49 shows is that the **context can be supplied from vision by the
creature itself**, unsupervised, which is the premise that clause said was missing. Whether the voice
then learns a per-object bias off that index is the next run, and the instrument already names it:
teach a two-object vocabulary on `context_source = 5` and score the voice.

### I WAS WRONG TWICE IN THIS SECTION, IN OPPOSITE DIRECTIONS, AND BOTH TIMES OFF A SMOKE

Worth recording because the pattern is the lesson, not the slips.

1. The first smoke had RETINA at `ctx_match` exactly 0.5000 against a null of exactly 0.5000 and
   clause 5 printed *"the cut is the loss, the front end IS the lever after all"*. The index had
   merely **collapsed onto one slice**. `ctx_present_frac` read 1.0000 throughout, because a constant
   index is perfectly live — `liveness-is-not-dose`, landing on its own author. Occupancy is now
   clause 1.
2. Having fixed that, the seven-arm smoke showed the held arms still at their null and I wrote into
   this notebook that §49 would be *"a refusal with a mechanism"*. At full budget the nuisance
   hypothesis those arms tested is **confirmed** and the effect is +0.33. The smoke had **100**
   probes; the run has **705**.

**The rule both slips share: a smoke can show the right sign of nothing.** It exists to walk the
branches, and this one did its job — every clause fired. It is not evidence about the magnitude of
anything, and the only reason §49 has a result is that the pre-registered budget was run anyway
rather than stopping at output that looked conclusive.

---

## §50 — PRE-REGISTERED: does the voice learn a per-object bias off the visual index?

Written and committed **before the instrument is built**. §49 supplied the index; this asks the other
half of the owner's instruction — *make it understand and speak the object it sees*. §49 is the
understanding. This is the speaking.

### THE MECHANISM, CONFIRMED IN THE KERNEL FIRST

Not assumed. `network.cpp` learns the per-context bias table by **node perturbation keyed on the
derived slice**:

```cpp
} else if (ctx_slots_ > 0 && ctx_present_) {
  Scalar& b = bias_ctx_[size_t(i) * ctx_slots_ + active_ctx_];
  b = clampf(b + u * gate_meta, -perturb_max_, perturb_max_);
}
```

and reads it back into the drive at `bias_ctx_[i * ctx_slots_ + active_ctx_]`. So **§49's arms already
had a retina-indexed bias table running**; `visprobe` simply never read the voice off them. §50 does
not need a new mechanism, only a new column — which is the same shape as §49 and the reason it is
cheap.

### THE HYPOTHESIS, AND WHY THE ARCHIVE MAKES IT A STRONG ONE

`ctxsrc` named the obstruction that stood between `areax` v51 and naming, and it was not the readout
and not the reward: **persistence.** The word is at ceiling (1.000) *while it plays* and at chance
(0.533) *by the reward window* — the index has to survive 800 ms of silence to still be there when
reward lands, and nothing in the creature holds it. Its own closing rule:

> *"before wiring a signal into a mechanism, check it is still there WHEN THE MECHANISM NEEDS IT.
> `coderprobe`'s 0.981 and this file's 0.533 are the same module measured 400 ms apart."*

**An object in view does not stop.** A word is a transient; a toy held in front of the creature drives
the retina continuously, through the vocalisation and through the reward window. The persistence
problem that the auditory index has to fight **does not arise for vision** — structurally, not as a
matter of degree. That is the prediction, and it is derived from the archive rather than invented.

**This is a prediction, so it gets measured, not assumed** — clause 2b below asks `ctxsrc`'s question
of vision directly instead of reasoning about it.

### THE ARMS — the full dose vector, pinned per arm

`control-on-a-different-code-path` requires an arm selector to pin every field it varies, so all five
are named on every row. Each index arm runs **both** target arms (idealised teacher / random target),
paired by seed, exactly as `g3probe` does.

| arm | `context_source` | `context_slots` | `ctx_proto_gate` | toy | index |
|---|---|---|---|---|---|
| `BASE` | 0 | **0** | 0 | held | none — bias table off |
| `RET` | 5 | 2 | 2 | held | **the creature's own, off the retina** |
| `ORACLE` | 5 | 2 | 2 | held | host-pinned to the **true** object |
| `SHUF` | 5 | 2 | 2 | held | host-pinned to a **drawn** object |

`ORACLE` and `SHUF` use `pin_context_oracle`, which overrides every source *after* it has run and
forces `ctx_present_ = true`. So all three indexed arms are **the same genome on the same code path**,
differing only in what the index contains. That is the one design choice that makes §50 readable:

- **`ORACLE` is the ceiling** on what the bias table can express with a *perfect* object index. Without
  it, a null on `RET` cannot distinguish *"the visual index is too weak"* from *"the bias table cannot
  make this voice object-specific at all"*. `null-is-not-a-control` cost this project four weeks for
  exactly this omission.
- **`SHUF` is the label-destroyed control.** It is equally live, equally bimodal and equally present —
  it just carries no object information. It is what says any `RET` effect is about the *label* and not
  about having a second bias table to write into. (`sparse-switch` found a benefit from mere dropout
  once; that is why this arm is not optional.)

All four arms hold position and size, including `BASE`. **`BASE` is therefore not bit-comparable to the
published `g3probe` 0.535 / 0.507**, which jitters the toy; it is the slots-0 anchor under §50's own
protocol. The tie back to the published protocol already exists and is already verified — §49
confirmed `g3probe` bit-identical to `results/g3probe-2026-11-04.log` — so it is cited, not re-run.

### POWER, COMPUTED BEFORE THE RUN, AS A NUMBER

The published per-creature spread of (taught − random) is **SD 0.080** over 5 creatures. That is almost
entirely *probe-count* noise, not biology: 70 held-out test probes at p = 0.5 give a binomial
SE of √(0.25/70) = 0.0598, and a difference of two such is 0.0598·√2 = **0.0846**, which matches the
observed 0.080. Between-creature biology is therefore a small term and **total test probes is the
lever** — more creatures and more ticks buy the same power per tick spent.

At **n = 12 creatures, 900 000 ticks**: SE(taught − random) = 0.0846/√12 = **0.0244**.

- Primary comparison `RET − SHUF` is a difference of two such paired estimates. Treating them as
  independent is the conservative bound: SE ≈ 0.0244·√2 = **0.0345**.
- **2 SE = 0.069. 3 SE = 0.104.**
- The available dynamic range is small and must be stated up front: the calibration echo — the voice
  while a word plays, with no learning at all — reads **0.725**, so the headroom above chance is about
  **0.23**. A 2 SE effect is **30% of that range**; a 3 SE effect is **45%**.

**So §50 can resolve an effect of ~0.07 and cannot resolve one of ~0.03.** If `RET − SHUF` lands inside
±0.069 the pre-registered reading is **inconclusive with a stated bound**, not a refusal
(`power-is-not-the-error-model`). Written as a number now so it cannot be re-read later: an outcome of
`RET − SHUF = +0.04 ± 0.035` is **not** evidence of naming and **not** evidence against it.

Cost: 4 index arms × 2 target arms × 12 creatures = **96 sessions**.

**The cost figure here was wrong when first written and is corrected before the run.** It said "~26 s
per session = ~42 min", taken from a half-remembered figure. The archived baseline log records its own
wall time — `real 12m49.955s` for 5 creatures × 2 arms at 900 000 ticks — which is **77 s per session**,
so 96 sessions is **~2 hours serial**, not 42 minutes. That is nearly 3× the stated budget, and it is
the kind of error that gets noticed only after the machine has been busy for an hour.

It also makes a standing rule bite: *a serial long run is a mistake on this box.* `objname`'s rep loop
was therefore converted to `parallel_reps` over all 48 arm × creature jobs in **one flat pool** (a
per-arm pool pays an idle tail four times), which the measured ceiling of 3.2× puts at **~38 min**. Per
that rule the conversion is proved, not assumed: the smoke was captured **serially before the edit**
and the parallel build must reproduce it exactly.

### THE CLAUSES, IN ORDER — order is part of a pre-registration

1. **Vacuity and occupancy.** Realised `ctx_slots`, `ctx_present_frac` and **occupancy** (share of
   probes in the busiest slice) per arm. Expected by construction: `BASE` slots 0; `ORACLE`/`SHUF`
   occupancy ≈ 0.50; `RET` ≈ 0.51 from §49. **Refusal, as a number: if `RET` occupancy > 0.90 the
   partition has collapsed and every `RET` row below is void** — this is the §49 trap, where a constant
   index read `ctx_present_frac` 1.0000 and looked perfectly healthy.
2. **The index, re-measured on this instrument.** `ctx_match` against its label-destroyed null for
   `RET`. §49 read **+0.3278**. **Refusal: if this comes back under +0.15, §50 is not running on the
   index §49 found and nothing below is interpretable** — a different instrument invocation is exactly
   where this project has found harness divergence before.
3. **`ctxsrc`'s question, asked of vision.** The same `ctx_match`, restricted to the **teaching**
   windows where reward actually lands, not pooled over probes. This is the clause that tests the
   hypothesis above rather than asserting it. A gap between clause 2 and clause 3 is the auditory
   failure reappearing in the visual path.
4. **The voice.** taught − random per creature, paired by seed, mean ± SE over 12 creatures, for all
   four arms. This is the primary table.
5. **The verdict, three-way and spanning the sign**, with all three branches written now:
   - **(a) `ORACLE` > `SHUF` and `RET` − `SHUF` > 2 SE** → the creature's voice carries what it sees,
     driven by an index it derived itself. Report `RET`/`ORACLE` as the fraction of achievable lift,
     and quote the margin in SE units, never as a bare inequality.
   - **(b) `ORACLE` ≈ `SHUF`** → **the ceiling itself is null**: the bias table cannot make this voice
     object-specific even with a perfect index, so `RET`'s null says nothing about vision. This
     indicts *expression*, not the index, and §49's own cascade predicts it is possible
     (central → voice is the larger loss at −0.237). §49 would stand unchanged and G3's block would be
     downstream of the index — `CONCLUSION.md`'s spine re-confirmed with the input side now supplied.
   - **(c) `ORACLE` > `SHUF` but `RET` ≈ `SHUF`** → the table works with a perfect index and the
     retina's is not good enough to drive it. That outcome is *surprising* against `ctxsrc`'s derived
     bar — an index right with probability p scales the signal as (2p−1), so p ≥ 0.75 keeps half, and
     `RET` measured **0.86**. Pre-registered response: do **not** conclude the index is weak; read
     clause 3 against clause 2, because (c) with clause 3 ≈ clause 2 means the failure is neither
     accuracy nor timing and the next measurement is the *write*, not the index.

### WHAT WOULD MAKE THIS UNREADABLE

Stated now so it cannot be discovered as a convenience later. The run is void if `RET`'s partition
collapses (clause 1), if clause 2 fails to reproduce §49 (clause 2), or if the **random-target arm
beats the taught arm in `BASE`** — which would mean the idealised teacher is not teaching under the
held-toy protocol and no arm's contrast means what its name says.

### §50 DESIGN CHANGE, DISCLOSED BEFORE THE RUN: THE PRIMARY ESTIMATOR WAS WRONG

The pre-registration above named *held-out accuracy on the probe voice, taught minus random* as the
primary comparison. **The smoke showed that estimator is confounded, and the confound is a mechanism
rather than noise.** Changed before the budget was spent, and recorded here rather than quietly.

The serial smoke (150 000 ticks, 12 creatures, every clause walked):

| arm | taught | random | taught − random |
|---|---|---|---|
| BASE | 0.567 | 0.517 | +0.050 |
| RET | 0.483 | 0.583 | −0.100 |
| **ORACLE** | **0.900** | **0.875** | +0.025 |
| SHUF | 0.533 | 0.600 | −0.067 |

**`ORACLE`'s RANDOM-target arm reads 0.875 against `BASE`'s 0.517.** Nothing object-specific is taught
in that arm — `target_for` is a coin — so a voice that is 87.5% decodable by object cannot be learning.
The mechanism is visible in the kernel: node perturbation writes to `bias_ctx_[i * slots + active_ctx_]`,
so pinning the index to the object creates **two tables that each random-walk under reward and
diverge**. A classifier then reads *which object* off the divergence. That is decodability without
naming.

**And `SHUF` cannot catch it** — which is the part worth keeping. A drawn pin decorrelates the slot
from the object, so the divergence stops being readable off the label and the label-destroyed arm sits
near chance by construction. The control was correctly chosen for the hypothesis and is simply blind to
*this* artefact. Had the run gone ahead on the pre-registered estimator, the headline available from
it was **"the voice carries which object at 0.90"**, which is false.

**The replacement, `axis_sep`:** the projection of (mean voice | object 1 − mean voice | object 0) onto
the teacher's **own axis**, divided by |axis|², so it reads in multiples of the separation the teacher
asked for — 1.0 being full compliance. Three properties the accuracy column lacks:

1. **Expectation zero under drift.** In the random arm each slot is rewarded toward +axis and −axis
   equally often, so divergence has no systematic component along the axis.
2. **Signed.** A creature that learns the mapping backwards reads negative instead of scoring as a
   success, which an accuracy column cannot express at all.
3. **No fitted split.** The axis is fixed in Phase A before any teaching, so it uses every probe
   rather than a held-out half.

This is the archive's own distinction and it already has a name: `magnitude-direction-split` —
*reward buys the separation, the lesson buys the direction.* The accuracy column is **kept as a
secondary** and explicitly excluded from the verdict, because a high 4b with a zero in 4a is itself a
finding: the two voices differ without differing as taught.

Clause 5's scale statement changes with it. The old "headroom 0.23 above an echo of 0.725" was in
accuracy units; in 4a's units 1.0 is full compliance and the printed 2 SE says directly what share of
the asked-for gesture the run can resolve.

**What the smoke does NOT say.** `RET` read occupancy 0.8917 and clause 2 refused it at −0.0216, and
**that is not a finding about the visual index.** §49 measured occupancy 0.509 and `ctx_match` +0.3278
at full budget, and §49's own second smoke had the held arms at their null before the real run reversed
them to +0.33. A smoke can show the right sign of nothing. The 150 000-tick partition has had a
fraction of the time the conscience needs to balance two prototypes, so clause 2 at full budget is the
measurement and this is not evidence about it in either direction.

### §50 GATES, BOTH PASSED BEFORE THE BUDGET WAS SPENT

1. **`g3probe` is bit-identical** to `results/g3probe-2026-11-04.log`, number for number
   (`results/n50-g3probe-bitidentity.log`). The new `pin_mode` parameter, the two teaching-window
   columns and the private `pin_rng` cost nothing to `g3probe`, `condprobe`, `pairprobe` or §49's
   `visprobe`: the pin block is gated on `pin_mode != 0`, and the label-destroyed arm's coin is drawn
   from a **separate** stream so the session's own `rng` never shifts. Drawing that coin from `rng`
   would have silently de-published two results.
2. **The parallel conversion is bit-identical.** `results/n50-smoke-serial.log` was captured serially
   *before* the edit and `results/n50-smoke-parallel.log` after it; clauses 1–3 and the whole accuracy
   table match exactly, the only difference being the clause-4 heading that was deliberately rewritten.

Parallel wall time is **170 s** for 96 sessions at 150 000 ticks, so the 900 000-tick run is ~17 min.
**No speedup factor is quoted**, because the serial reference ran concurrently with the `g3probe`
bit-identity check and a contended baseline is exactly how the parallel-run note's first speedup table
came out flattering.

### §50 RESULT: THE CREATURE SPEAKS THE OBJECT IT SEES

> **AMENDED BY §51 THE SAME DAY, AND THE HEADING ABOVE OVERSTATES IT.** §51 calibrated what this
> section's estimator is worth in the units a listener hears, which nothing here had done. The two
> **echo postures** that define `axis_sep = 1.0` are **the same vowel** — 630/1691 Hz against
> 632/1698 Hz, for words at 780/1180 and 320/2500 — only **19%** of `axis` lies in F1 and F2, and
> full compliance is therefore worth **+0.0008** of an available ≈1.64 formant gap.
>
> Read everything below as: **the creature's motor state separates by object, along the direction
> that hearing the two words produces.** The mechanism claim stands and `g3probe`'s null is still
> corrected. **The creature is not shown to say the two words differently** — on §51's audible
> estimator this section's own teacher reads **−0.0059**, slightly the wrong way. See §51's ruler
> calibration, and note that the calibration needed no teaching and was available before §49.

Full run, 900 000 ticks, 12 creatures × 4 index arms × 2 target arms = 96 sessions, ~21 min
parallel (`results/n50-objname.log`, per-creature rows in `results/n50-objname-perseed.log`,
genome `results/n50-genome.toml`). Re-run with per-creature output added reproduced clauses 1–3
and clause 5 **identically**, which doubles as a determinism check.

#### CLAUSE 3 CONFIRMED THE PREDICTION, AND IT IS THE WHOLE RESULT

| index | while presented | **in the reward window** |
|---|---|---|
| the ear — `ctxsrc`, published | 1.000 | **0.533, at chance** |
| **the retina — §50** | 0.8517 | **0.8400, intact** |

`ctxsrc` named *persistence* as what stood between `areax` v51 and naming: the auditory index is at
ceiling while the word plays and gone by the time reward lands 800 ms later, and nothing in the
creature holds it. **A toy in view does not stop.** The visual index is as good in the reward window
as at the probe — the obstruction does not arise, structurally rather than by degree. This was
derived from the archive before the run and clause 3 measured it instead of asserting it.

#### CLAUSE 2 REPRODUCED §49 ON A DIFFERENT INSTRUMENT

`ctx_match` 0.8517 against a measured null of 0.5352 = **+0.3165**, against §49's +0.3278. Occupancy
0.5177 — balanced, no collapse. ORACLE 1.0000 by construction, SHUF −0.0063 as a drawn pin must.

#### CLAUSE 4a — THE VOICE, IN MULTIPLES OF THE SEPARATION THE TEACHER ASKED FOR

| arm | taught | random | taught − random | beat own ctl | taught > 0 |
|---|---|---|---|---|---|
| `BASE` — table off | +0.0149 ± 0.0245 | −0.0051 | +0.0200 ± 0.0250 | 7/12 | 9/12 |
| **`RET` — self-derived** | **+0.5875 ± 0.1500** | −0.0659 | **+0.6534 ± 0.2236** | **10/12** | **11/12** |
| `ORACLE` — perfect index | +0.9798 ± 0.2340 | +0.1989 | +0.7809 ± 0.2842 | 10/12 | 10/12 |
| `SHUF` — label destroyed | +0.0194 ± 0.0362 | +0.0458 | −0.0264 ± 0.0264 | 5/12 | 5/12 |

**`RET − SHUF` = +0.6798, paired SE 0.2282, 3.0 SE** — clears the pre-registered 2 SE bar. `ORACLE`
works, so the ceiling is real; `RET` reaches **84%** of the achievable lift and **60%** of `ORACLE`'s
taught column. Every *random* arm sits at zero, which is the estimator validating itself.

**The effect is not bimodal**, which a mean and an SE cannot tell you. Per creature, `RET` taught:

```
-0.244  +0.119  +1.071  +0.696  +1.738  +0.398  +0.876  +0.099  +0.359  +0.429  +0.750  +0.758
```

Eleven of twelve positive, ten of twelve beating their own control, a continuous spread from 0.10 to
1.74 with one negative. `SHUF` reads 5/12 and 5/12 — an exact coin flip with every value under 0.27.
And `BASE`'s +0.0200 ± 0.0250 is the `axis_sep` rendering of `g3probe`'s published 0.535 vs 0.507,
which cross-validates the new estimator against a known number.

#### WHAT THIS CORRECTS IN THE PUBLISHED CONCLUSION

`g3probe`'s verdict — *"a perfect teacher cannot make this voice depend on what the creature sees"*,
0.535 against a 0.75 bar — was measured with `context_slots = 0`. **That null was a statement about a
missing index, not about the learning rule.** With the bias table on and indexed off the creature's
own retinal partition, the same idealised teacher moves the voice to **59% of the asked-for
separation** against **2% with the table off**. Node perturbation *can* learn a conditional mapping
from what is in view to what the larynx does — it needs somewhere to put the condition.

`CONCLUSION.md`'s spine was *this architecture can learn a bias per context and cannot learn the
context.* For vision, under held nuisances, **both halves now hold**: the creature learns the context
(§49) and then names off it (§50).

#### THE SCOPE LIMITS, WHICH ARE REAL

1. **The teacher is idealised** — dense, immediate, and it says which object this is. The creature
   supplies the *question* (which object, from its own retina, unsupervised); the teacher still
   supplies the *answer* (which sound). `g3probe`'s own note names what remains: the
   picture→sound→voice teaching signal. **This is not autonomous word learning.**
2. **Position and size are held** in all four arms. §50 inherits §49's measured conditions rather
   than re-testing them: the conscience is necessary (`ctx_proto_gate` 2) and holding nuisances is
   worth 4.3×. A caregiver holding a toy still is the setting this result is about.
3. **Two objects.** The vocabulary ceiling and axis-count work is about a different question.

#### THE ESTIMATOR CHANGE WAS NOT COSMETIC — IT REVERSED THE VERDICT

On the **pre-registered** primary estimator, this same run reads `ORACLE` **−0.133** and `RET`
**−0.028**: taught *losing* to random. That fires branch (b) — *"the ceiling itself is null, the block
is expression"* — and closes the line **in exactly the wrong direction**, on a run whose honest
estimator says 3.0 SE the other way.

The accuracy column is contaminated in *both* arms because it measures table divergence rather than
teaching, so which way `taught − random` lands is arbitrary: at the smoke it flattered `ORACLE`
(+0.025), at full budget it buries it (−0.133). **The danger was described as a false headline when
the smoke found it; the full run shows the worse failure mode was a false refusal.** A confounded
estimator does not merely inflate — it has no reliable sign.

#### THE VERIFY GATE, ALL FOUR KEYS

`results/n50-verify.log`:

```
verify PASS
hash              ad96f882becbee92  as recorded
pinned hash       PASS
experiments       24 as expected, 0 not
```

The pinned hash is unmoved, so DNA v84's `context_source = 5` and §50's `pin_mode` remain inert for
the shipped genome (`context_slots = 0`, `context_source = 0` — confirmed in `dna/default.toml`). The
experiment count stays 24 because `objname` is `Tier::kLong` / `Expect::kRead`, exactly as `visprobe`
was.


## §51 — PRE-REGISTERED: WHO SUPPLIES THE ANSWER? A CAREGIVER THAT NEVER LOOKS AT THE TOY (2026-11-07)

§50's first scope limit is the whole of §51. The teacher there is **idealised in two separate ways**,
and only one of them has ever been argued about:

1. It **knows which object is present** and signs the reward by that label.
2. Its target is `±axis`, where `axis` is **this creature's own echo posture difference**, measured in
   Phase A before teaching. A real caregiver cannot compute that: it is a per-creature, perfectly
   reachable, perfectly aligned direction in the creature's own motor space.

So §50 established that *the creature can supply the question*. The answer was still handed over. The
lead `g3probe` itself named, and the one §50's result makes the next one rather than one of several,
is the **picture→sound→voice teaching signal**: a caregiver who *says a word while the object is in
view*, with no per-object reward supplied.

### FIRST, A REFUSAL BY ARITHMETIC, BEFORE ANY BUDGET IS SPENT

The strongest reading of that lead is **constant, object-blind, word-blind approval** — the caregiver
simply approves of naming, the same approval for both words, and the *binding* comes from the innate
echo (M1b: the voice carries the heard word at 0.890 after the word stops) being reinforced into
whichever slot the retina has selected. The m3 naming milestone's own code says this out loud:

> *"Approval accompanies naming, and it is the same approval for both words. It is what opens the gate
> on three-factor learning (§3.1); it is not what says which object this is."*

**That version cannot work, and the kernel says so without a run.** `network.cpp:3021` writes

```
b = clampf(b + u * gate_meta, -perturb_max_, perturb_max_)      with  u = perturb_rate_ * gate * r̃ * perturb_[i]
```

where `r̃` is the centred reward. The written direction is **the neuron's own random perturbation**,
not the posture the larynx is currently holding. There is no Hebbian term on the current output. So
if `r̃` carries no information about the voice, `E[write] = E[r̃] · E[perturb_[i]] = 0`, because
`perturb_[i]` is zero-mean. A reward that does not evaluate the voice writes **nothing in
expectation**, whatever it is correlated with in the world.

The only thing such a protocol can learn is a *timing* correlation — praise arriving while the
creature happens to be vocalising credits perturbations that increased vocalisation. That is "say
more", which is G2, already measured, and it is not naming.

This is why the m3 naming milestone needed `set_reward_mask` with `CtxMask::kByObject` — **an oracle
mask is how the answer got in there**, and it is the same oracle §51 is trying to remove. So the
question is not whether the reward can be information-free. It is **how little the reward has to know**.

### THE TEACHER THAT IS LEFT, AND IT IS A PUBLISHED ONE

M1c's caregiver (`teachsound`, d' 5.57, 3/3 seeds): *praise when a moment lands closer than this
creature usually gets, a mild no when further.* In code, at
`experiments_milestones.cpp:6305`, binary `kPraiseValue` / `kScoldValue` on
`formant_error(produced f1, f2, target word)` against a **per-word EMA baseline**.

Point that teacher at **the word the caregiver is currently saying** and it has these properties:

* It **evaluates the voice**, so `r̃` correlates with the perturbation and the arithmetic above does
  not kill it.
* It **never looks at the object.** It compares two sounds. A caregiver who says "cube" and warms when
  the babble comes out closer to "cube" needs no knowledge of what is on the table.
* Its target lives **in the world** — `kWords[1] = {250, 320, 2500}` — not in the creature's motor
  space. It is not fitted per creature and is not guaranteed reachable. This removes §50's *second*
  idealisation, the one nobody has argued about.
* The **conditionality is never told to the reward.** Object→word is never a term in it. The only
  route from sight to sound is that the caregiver happens to say "cube" when the cube is in view.

**That is the claim §51 tests.** `vocallearn` is the reason it is worth testing and the reason it might
fail: it ran exactly this target — "a target that depends on what was heard" — and read **flat on 3/3
seeds** (+0.7, −1.4, +0.3) against a fixed target's **+24.0**, and concluded *reward cannot reshape
the conditional route; node perturbation moves a constant.* **That conclusion was reached at
`context_slots = 0`.** It is the same missing index that made `g3probe` null, and §50 is what makes
`vocallearn`'s wall worth re-measuring rather than re-asserting.

`vocallearn` also could not have run this: it has no retina and no toy. §51 goes in
`run_g3probe_session`, which already has the retina, the toy, the caregiver's voice, the context index
and the silent object-only probe.

### THE THREE TEACHERS, AND WHAT EACH ONE KNOWS

| teacher | what it says | does it see the toy? | does it evaluate the voice? |
|---|---|---|---|
| **T0** — §50's | dense signed projection of the voice onto `±axis`, signed by **the object** | **yes** | yes, in its own motor space |
| **T1** — M1c's, re-pointed | praise/scold on whether the produced formants came closer than usual to **the word being spoken** | **no** | yes, in the world's units |
| **T2** — yoked | praise/scold at the same clock, **drawn by a coin** | no | **no** |

T2 is both the arithmetic floor measured rather than asserted (`power-is-not-the-error-model`: write
the firing output as a number) and the established reward-arrival control — it is `vocallearn`'s
`kVLYoked`. Its expected write is zero by the derivation above, and an arm that scores above zero on
it voids the whole run.

**T1's clock is T0's clock**, every `plasticity_interval_ticks` (10) over `t ∈ [500, 1400)`, not
`kFeedbackPeriodTicks`. That is a deliberate change from M1c and it removes a confound rather than
adding one: `vocallearn`'s own power note records that reward density flipped its sign (−1.2 at one
reward per trial, +1.0 at 19× denser, both at 560k ticks), so a sparse T1 against a dense T0 would
confound *what the reward knows* with *how often it arrives*. The one legitimate dose difference that
remains is that **T1 can only fire on a voiced frame** (`voicing > 0.5 && amplitude > 0.45`), because
a silent larynx has no formants. `delivery-fraction` says the creature falls silent as it trains, so
that difference could be large. It is measured as clause 0 and not assumed.

The **per-word baseline** is kept from `vocallearn`'s design-trap note ("against one global mean the
creature is rewarded for saying the easy word"). It sets the praise/scold *threshold*, never a
direction, and it is per-word in every arm including the decorrelated one, so it is symmetric. A
caregiver tracking "how close does this baby usually get to *this* word" needs no sight of the toy.

### THE ARMS. THE TABLE PINS THE WHOLE DOSE VECTOR ON EVERY ROW

`control-on-a-different-code-path`: an arm selector that sets some fields and inherits others is how a
control ends up on a different code path from the arm it controls.

| # | arm | index | teacher | word spoken | n | what a null here would mean |
|---|---|---|---|---|---|---|
| 1 | `T0·RET` | retina, source 5, gate 2 | T0 | — (silent) | 12 | **the run is void** — §50 did not reproduce |
| 2 | **`T1·RET`** | retina, source 5, gate 2 | T1 | **= the object** | **36** | **THE CLAIM** |
| 3 | `T1·RET·D` | retina, source 5, gate 2 | T1 | **drawn, independent of the object** | **36** | the pairing is doing the work |
| 4 | `T1·ORACLE` | **pinned to the true object** | T1 | = the object | 12 | T1 cannot teach even a perfect index |
| 5 | `T1·BASE` | **table off**, `slots = 0` | T1 | = the object | 12 | the bias table is the route, not an association |
| 6 | `T2·RET` | retina, source 5, gate 2 | **T2** | = the object | 12 | reward *arrival* is not the effect |

Position and size are **held** (`m3_toy_fixed`) in all six, inheriting §49's measured conditions
rather than re-testing them — the conscience is necessary and holding nuisances is worth 4.3×. Two
objects. The probe is unchanged from §50: **the object alone, in silence.**

Three arms earn their place against specific published failure modes:

* **Arm 5 is not redundant with arm 1.** If the caregiver says "cube" whenever the cube is in view,
  vision→auditory association could form and the sight could come to *evoke* the sound at probe,
  driving the innate ear→larynx route with the bias table doing nothing. That is the classical
  conditioning route, and `g2cond`/`pairprobe` say it is closed — so arm 5 should be null. If it is
  **not**, that is a larger result than the one being asked for and the honest response is to report
  it as the headline and re-run `pairprobe` before believing it.
* **Arm 4 separates two different nulls on arm 2.** "The teacher cannot teach" and "the index is not
  good enough for a weaker teacher" are different findings with different next steps, and arm 2 alone
  cannot tell them apart. `null-is-not-a-control` cost four weeks for this exact omission.
* **Arm 3 is the label-destroyed arm, and `control-blind-to-its-artefact` is why it is enough *here*
  and was not enough in §50.** §50's `SHUF` was blind to table divergence because divergence is not
  about the label. Arm 3 is not blind to it on this estimator: two per-slot tables random-walk in
  arms 2 and 3 alike, and `axis_sep` has **zero expectation under drift in both**. The artefact is
  present in the control and the estimator cannot read it. That is the property §50's accuracy column
  lacked, and it is why the primary is not being re-chosen.

### THE ESTIMATOR IS §50's, UNCHANGED, AND DELIBERATELY NOT T1's OWN CURRENCY

Primary: **`axis_sep`** — the projection of (mean voice | object 1 − mean voice | object 0) onto the
Phase A echo axis, divided by |axis|², so 1.0 is full compliance with the separation the two words
ask for. Signed; zero expectation under drift; no fitted split. It cross-validates against a known
number — §50 read `BASE` at +0.0200 ± 0.0250, which is `g3probe`'s published 0.535 vs 0.507 in these
units.

**The same ruler for every arm, including the arms whose reward uses a different target.** Scoring T1
on T1's own formant error and T0 on T0's own projection would be fitting the verdict to the arm. The
cost of this choice is stated up front: `axis` is the creature's *echo* axis and T1's target is the
*world* formant pair, so a success could be under-read if the two are misaligned. M1b (echo 0.890)
says they should be positively aligned, and `axis_sep` on arm 1 is the within-run check that the ruler
is pointing the right way. The probe formant error toward the paired word versus the other word is
reported as a **secondary** in T1's own currency, explicitly excluded from the verdict — the
`estimator-change` lesson from §50 run forward rather than relearned.

### CLAUSES, IN ORDER, FIXED BEFORE THE RUN

**Order is part of a pre-registration** (`vacuity-gate-cannot-guard-a-rejection`), and every bar is
asked of the controls too.

**Clause 0 — DOSE, NOT LIVENESS.** Per arm: `ctx_slots` as the genome asked, `ctx_present_frac`,
occupancy, and the reward dose — evaluations fired / eligible, praise fraction, and mean |effective
reward|. `liveness-is-not-dose`. **Any arm whose fired-count is below 50% of arm 1's is reported as
UNDERPOWERED, not as a null**, and the reason is named. T1 and T2 should both land near a praise
fraction of 0.5, T1 because an EMA baseline is straddled by construction and T2 because it is a coin;
a T1 praise fraction outside [0.35, 0.65] means the baseline is not tracking and the arm is void.

**Clause 1 — THE INDEX UNDER A SPEAKING CAREGIVER.** §49 and §50 both measured the retinal index with
the caregiver **silent**. Source 5 reads `kVision`, so added auditory drive should not touch it, but
that has never been measured. `ctx_match` excess over its own 64-shuffle null, arms 2 and 3. **Bar
+0.20**, against §50's +0.3165. Below it, §51 says nothing about teaching and reports an index finding
instead.

**Clause 2 — THE PROBE IS SILENT, AND THIS ONE IS FATAL.** If the word is audible at probe, M1b's
innate route produces the right sound with nothing learned at all. Required:
`probe_heard / probe_ticks == 0.0000` exactly, in every arm. And for the T1/T2 arms the teaching
window must read `teach_heard / teach_ticks = 0.444` — the word sounds for `t < 900` and the window
opens at `t = 500`, so 400 of 900 ticks. **A number, pre-computed, not an eyeball.** Any deviation
voids the run.

**Clause 3 — THE PRIMARY.** `axis_sep` per arm, mean ± SE across creatures. Primary contrast
**arm 2 − arm 3**, paired by creature identity (on `ax_who`, never on position — a cell can yield an
accuracy and no axis value). Bar **2 SE**, as §50. Secondary contrasts: arm 2 − arm 6, arm 2 − arm 5.

**Clause 4 — THE 4TH RUNG.** `lesson-landed-guard`. Arm 1 must read **≥ +0.30** — half of §50's
+0.5875 — for a null on arm 2 to be interpretable at all. A flat arm 2 beside a flat arm 1 is an
underpowered run, not a finding about teachers.

**Clause 5 — THE VERDICT. Seven branches, and the sign is not constrained.**

1. **Arm 1 < +0.30 → VOID.** Report, stop, claim nothing about the teacher.
2. Arm 6 ≥ +2 SE above zero → **VOID**: the effect is reward arrival, not content.
3. Arm 5 ≥ +2 SE above zero → **the association route works and the table is not needed.** Bigger
   than the question asked; report as the headline and re-run `pairprobe` before believing it.
4. Arm 1 passes, arms 5 and 6 null, **arm 2 − arm 3 ≥ +2 SE and arm 2 > 0 → THE CLAIM HOLDS.** A
   caregiver that never looks at the toy and only compares sounds suffices; the binding comes from
   co-occurrence. Report arm 2 as a fraction of arm 1 and of arm 4.
5. Arm 1 passes, arm 2 ≈ arm 3 ≈ 0 **and arm 4 ≈ 0 → THE TEACHER IS THE BLOCKER, NOT THE INDEX.**
   §50's scope limit 1 is a real wall, `vocallearn`'s conclusion survives the index being supplied,
   and the next lead is the teacher rather than the retina.
6. Arm 1 passes, arm 2 ≈ 0 **but arm 4 ≥ +2 SE → the index is not good enough for this weaker
   teacher.** A dose statement, not a wall: the next step is the index, and §49's 4.3× nuisance term
   is where to look.
7. **arm 2 − arm 3 ≤ −2 SE → A REVERSAL.** The mapping learned backwards. `axis_sep` can express
   this and an accuracy column cannot; the candidate mechanism is T1's scold half dominating, and it
   would be reported as a finding rather than folded into a null. **The sign is not constrained in
   advance, so this branch exists** (`verdict-fitted-to-data`).
8. Otherwise **INCONCLUSIVE**, with the SE printed.

### POWER, WRITTEN AS A NUMBER, AND IT SET n

§50's published spread is the prior: `RET` taught +0.5875 ± 0.1500 over 12 creatures, so per-creature
**SD ≈ 0.52**; the paired `RET − SHUF` contrast read SE 0.2282 over 12, so the **paired SD ≈ 0.79**.

T1 knows strictly less than T0, so assume it delivers **half** of §50's effect, +0.29. Then:

| n | paired SE | +0.29 reads | clause 5.4 fires? |
|---|---|---|---|
| 12 | 0.228 | **1.3 SE** | no — INCONCLUSIVE |
| 24 | 0.161 | 1.8 SE | no — INCONCLUSIVE |
| **36** | **0.132** | **2.2 SE** | **yes** |

**So n = 12 would have had roughly even odds of printing INCONCLUSIVE on a real half-sized effect**,
and that is exactly the outcome this project pays for twice (`ctxsrc` at 3 creatures printed a licence
on four thousandths; `smoothing-sweep-closed` on 3 seeds). The primary contrast therefore runs at
**n = 36** and the reference and null arms at **n = 12**, where the quantities being asked for are
"reproduces ≈ +0.59" and "is not above +0.30" and n = 12 resolves both. Creature r is the same
creature in every arm (`seed + r * 7919`), so the paired contrast between arms 2 and 3 uses all 36 and
contrasts against arms 1/4/5/6 use the shared first 12.

**Ticks: 1 800 000, double §50's.** T1 fires only on voiced frames and `delivery-fraction` says the
creature falls silent as it trains, so T1's realised dose is below T0's by an unknown factor; and
`vocallearn` needed 3.4M at a 19× sparser clock. Halving this to save ~25 minutes on a run whose
refusal clause was just shown to have ~50% power at the smaller size is false economy.

**Cost: 2·36 + 4·12 = 120 sessions at 1.8M ticks.** §50 ran 96 sessions at 900k in 1249.59 s wall
parallel, so this is `120/96 × 2 × 1250 ≈ 3100 s ≈ 52 min`. Quoted from the archived wall time of a
measured run, not from a remembered per-session figure — §50's pre-registration got that wrong by 3×
and the correction is the reason this sentence names its source.

### THE GATES, BOTH BEFORE THE BUDGET

1. **`g3probe`, `condprobe`, `pairprobe`, `visprobe` and `objname` stay bit-identical.** Every new
   path is gated on `teacher != 0`; the decorrelated word reuses the `coin` that
   `run_g3probe_session` **already draws unconditionally** on every teaching trial, so no new random
   stream is touched and no existing stream shifts. `pin_rng`'s lesson, applied before it could cost
   anything.
2. **`verify` holds all four keys** and the pinned hash `ad96f882becbee92` is unmoved. §51 adds no
   genome field at all — `context_source = 5` already shipped inert in DNA v84 — so there is nothing
   for the shipped genome to pick up.

### §51 GATES, BOTH PASSED BEFORE THE BUDGET WAS SPENT

1. **`g3probe` is bit-identical** to `results/n50-g3probe-bitidentity.log`, all 75 lines including the
   instrument seed line (`results/n51-g3probe.log`). The new `teacher` and `pairing` parameters, the
   dose counters, the probe-formant accumulator and the moved `formant_error` cost `g3probe`,
   `condprobe`, `pairprobe` and §49's `visprobe` nothing: every new path is gated on `teacher != 0`,
   and the decorrelated word reuses `coin`, which the teaching loop **already draws unconditionally on
   every trial**. No new random stream is touched by the paired or decorrelated arms at all, so no
   existing stream can shift. `pin_rng`'s lesson applied before it could cost anything rather than
   after.
2. **`objname` is bit-identical**, 110 lines number for number, against a reference captured *before*
   the edit (`results/n51-objname-bitidentity.log`). §50's published result therefore still stands on
   this binary.

The pre-edit reference also measures the rate the cost estimate rests on: **185 s wall for 96 sessions
at 150 000 ticks**, so ~78 000 ticks/s aggregate, against §50's full run at ~69 000. 120 sessions at
1.8M is ~51 min, which is the pre-registered figure confirmed by a measurement rather than a memory.

**And all eight clause-5 branches plus the four VOIDs are present in the shipped binary**, checked by
string. The first form of that check used `grep` on the executable and reported every branch missing,
including one the smoke had just printed; `strings | grep` found them all. **That is §50's wrong-case
grep again — the check tool was wrong, not the thing being checked** — and the only reason it did not
become a conclusion is that a result claiming twelve-for-twelve missing was too clean to be true.

### TWO THINGS THE SMOKE CHANGED, DISCLOSED BEFORE THE RESULT EXISTS

Neither is a response to an outcome. The smoke's verdict was a pre-registered **VOID** — clause 1 read
−0.0216 excess at 150 000 ticks, which is *the same number* §50's own smoke read before the full budget
reversed it to +0.3165, and clause 4 read +0.0583 against its +0.30 bar. Both changes come from
quantities clause 0 was built to print.

**1. THE BASELINE RATE WAS RE-DERIVED, AND CARRYING IT OVER WOULD HAVE BEEN `ctxgain`'S ERROR.**

`kTSBaselineAlpha = 0.02` is a rate **per evaluation**. M1c evaluated every `kFeedbackPeriodTicks`
over an 800-tick reward window — 5.33 evaluations a trial — so its baseline carried `1/0.02 = 50`
evaluations, which is **9.4 trials** of memory. That span is what *"closer than this creature usually
gets"* means.

§51 evaluates on the plasticity clock to match T0's dose: 90 evaluations a trial, **16.9× denser**
(90 ÷ 5.33; the first draft of this paragraph said 18× from rounding 5.33 to 5, and the instrument
prints the ratio it actually used rather than the one that was typed).
Inheriting 0.02 would have left the baseline with **half a trial** of memory and silently turned
"closer than usual" into "closer than a moment ago" — a different teacher wearing M1c's name, and a
null that says nothing about what a teacher has to know.

This is the shape of error the kernel already documents on `ctxgain`, where a normaliser 12× too small
made an arm run under-driven by exactly that factor: *"a learning-rate CUT wearing the costume of a
parameterisation test."* So the **invariant preserved is the baseline's memory in trials**, and the
ratio is computed from the named constants rather than typed in: α = 1/(9.375 × 90) = **0.001185**.

**The smoke could not have told the two regimes apart, which is why this is an argument and not a
measurement.** Both straddle, so both read a praise fraction near 0.5 — the first smoke read 0.500,
0.518, 0.554, 0.513, 0.498 across the five speaking arms, comfortably inside the pre-registered
[0.35, 0.65]. And **the opposite reading is defensible**: a fast baseline is a high-pass, and
crediting a perturbation against the error a moment ago is ordinary RPE and arguably sharper credit
assignment. It is not settled here. **If T1 nulls, this constant is the first thing to vary**, and
that is written down now rather than discovered later.

**2. T1 DRIVES THE REWARD CHANNEL 6.4× HARDER THAN T0, AND IT IS NOT BEING RETUNED.**

Clause 0 measured `mean|r|` at **0.0781 for T0 and 0.5000 for T1 and T2**. Neither is a bug: T0's
reward is a *correlation*, bounded by 1 and typically small, while T1's is M1c's published
`±kPraiseValue`. The two teachers differ in how hard they drive the reward channel as well as in what
they know.

* **The primary contrast is unaffected and is magnitude-matched by construction.** `T1.RET` and
  `T1.RET.D` run the same rule on the same clock and differ only in which word is spoken, so both read
  0.5000.
* **Every comparison with T0 is affected.** "% of T0" in the result is a descriptive ratio and
  explicitly **not** an efficiency statement.
* **It is not retuned, and the direction is why.** Scaling T1's praise to match T0's measured output
  would be fitting a constant to the very arm T1 is compared against. And the asymmetry runs in the
  conservative direction for a null: T1 is given *more* drive than T0, so **a T1 null cannot be blamed
  on want of drive.**

The ratio is now printed in clause 0 on every run, so the result cannot be read without it.

### ONE CONTROL THE SMOKE WEAKENED, AND THE PRE-REGISTRATION ALREADY HANDLES IT

`T2.RET` fired at **0.42× T0's rate**, below clause 0's pre-registered 0.50 bar, so the dose refusal
fired on it. The cause is not a harness fault and is worth naming: T1's teacher *requires* voicing to
score, so it reinforces vocalising as a side effect and lands at 0.77×, while a coin does not and
lands at 0.42×. **The content of the reward raises its own dose.**

That weakens branch 2 in the direction that matters — a yoked arm with less drive is *less* likely to
show the spurious effect branch 2 exists to catch — so the clause-0 refusal is printed beside every
verdict rather than only once. The arithmetic argument stands independently of dose: a reward
uncorrelated with the perturbation writes nothing *in expectation* at any density. But **T2 is
reported as underpowered rather than as a clean floor**, and that was pre-registered as the response
before the number existed.

### §51 RESULT: THE RULER WAS THE WRONG RULER, AND IT WAS §50'S

Full run, 1 800 000 ticks, 120 sessions, **WALL 57m28s** (`results/n51-objword.log`, genome
`results/n51-genome.toml`). Clause 5 fired branch **(5)**, and **the branch's own text is wrong**. The
finding is not about this teacher. It is about the estimator both sections were scored on.

| arm | index | teacher | `axis_sep` | > 0 | dose |
|---|---|---|---|---|---|
| `T0.RET` | retina | §50's, object-signed | **+0.5295 ± 0.1056** | **12/12** | 1.00× |
| `T1.RET` | retina | sound-only, word = object | −0.0686 ± 0.0606 | 17/36 | 0.60× |
| `T1.RET.D` | retina | sound-only, word drawn | +0.0193 ± 0.0618 | 17/36 | 0.56× |
| `T1.ORACLE` | pinned true object | sound-only, word = object | +0.0702 ± 0.2334 | 7/12 | 0.63× |
| `T1.BASE` | table off | sound-only, word = object | +0.0170 ± 0.0092 | 9/12 | 0.53× |
| `T2.RET` | retina | yoked coin | +0.0386 ± 0.0969 | 5/12 | 0.39× — refused |

Primary `T1.RET − T1.RET.D` = **−0.0879, paired SE 0.0741, −1.2 SE**, with **17/36 in both arms** —
the same coin flip, which is the distributional statement a mean and an SE cannot make.

The three gates held: **`T0.RET` reproduced §50 at 12/12** (+0.5295 against §50's +0.5875 on half the
budget), the retinal index **survived a speaking caregiver** at +0.2269 excess against a +0.20 bar —
which neither §49 nor §50 measured, both having run the caregiver silent — and clause 2 was exact,
`probe heard 0.0000` and `teach heard 0.4444` against a derived 0.4444, in all six arms.

#### THE RULER CALIBRATION, WHICH IS THE RESULT

The pre-registration named this risk before the run:

> *"`axis` is the creature's echo axis and T1's target is the world formant pair, so a success could be
> under-read if the two are misaligned."*

It did not merely fire. It dominates. `axis_sep = 1.0` is defined as the full separation between the
two **echo postures** — what this larynx does on hearing each word — and nothing in §49, §50 or
`g3probe` had ever asked what that separation is worth in the units a listener hears:

| quantity | value |
|---|---|
| share of \|`axis`\|² lying in groups 2 and 3 (F1, F2) | **0.1909** |
| echo posture formants, hearing "ball" | **630 / 1691 Hz** |
| echo posture formants, hearing "cube" | **632 / 1698 Hz** |
| the two words themselves | 780 / 1180 and 320 / 2500 Hz |
| **formant gap at `axis_sep` = 1.0 (`echo gap`)** | **+0.0008** |

**The two echo postures are the same vowel.** They differ by **2 Hz on F1 and 7 Hz on F2**, against
words that differ by 460 Hz and 1320 Hz. Four fifths of the echo axis lies in f0, the bandwidths and
the rest; the fifth that is formant is, given those numbers, **noise**.

So **full compliance with §50's teacher — `axis_sep` = 1.0 — is worth `+0.0008` of an available
≈1.64 formant gap, which is 0.05%.** The primary estimator has essentially no formant content, and
what little it has points in a direction set by a 2 Hz and 7 Hz difference. **A real formant
separation would project onto it with random sign**, which is exactly what `T1.RET` −0.0686 and
`T1.RET.D` +0.0193 at 17/36 and 17/36 look like.

This is not a new fact about the creature so much as a quantification of one the archive already had:
`vowel-space` ("one vowel: the centroid never dwells") and `audibility-ruler` (cube vs ball at **0.40**
on a bar of 1.0, while M1b's word repetition is **1.37**). The creature repeats a word by moving f0
and the bandwidths, **not by moving the vowel**. M1c's d' 5.57 shows reward *can* move F1 and F2 —
609→564, 1685→1789 — so this is a property of the innate echo route, not of the larynx.

**The calibration is a Phase A property and was checked rather than asserted.** `T0.RET`,
`T1.ORACLE` and `T2.RET` differ in teacher *and* in pin and print byte-identical ruler rows; and all
six rows are byte-identical between a 150 000-tick and a 300 000-tick run. It is a function of the
creature seed alone.

#### WHAT THIS DOES TO §51's OWN VERDICT

**`T1`'s null on the primary is uninformative about `T1`.** The teacher aimed at formants; the
estimator cannot see formants. Branch (5)'s *"a teacher whose target lives in the world cannot teach
this mapping however good the index is"* is **not supported**, on two counts:

1. The estimator could not have detected success in either direction.
2. `T1.ORACLE` was underpowered anyway: ±0.2334 means a 2 SE interval of ±0.467, **which spans
   `T0.RET`'s entire +0.5295**. That arm excludes nothing.

And the **secondary — the only estimator in the run that can see a formant change — inverts the
ordering**, with a scale that is now interpretable rather than suggestive:

| arm | `axis_sep` | `perr(other) − perr(own)` | as a multiple of the `echo gap` ceiling |
|---|---|---|---|
| `T0.RET` | **+0.5295** | −0.0059 | — |
| `T1.ORACLE` | +0.0702 | **+0.1889** | **236×** |
| `T1.RET` | −0.0686 | +0.0121 | 15× |
| `T1.RET.D` | +0.0193 | −0.0678 | — |

`T1.ORACLE` moved the formants **236 times further than full compliance with §50's teacher would**,
and ~12% of the whole available gap. It read +0.1071 and +0.1049 in the two smokes, so it has
**reproduced at three budgets.**

**It is still not a result, and promoting it would be §50's error in the opposite direction.** The
secondary has no SE, `T1.ORACLE` has no decorrelated counterpart (no `T1.ORACLE.D` was run), and
n = 12. What has changed is that it is no longer a hint against a null — it is the one arm that moved
the audible quantity, measured against a ceiling that makes its size meaningful.

**§51's honest status is therefore INCONCLUSIVE, on the ruler that matters**, reached by a route the
pre-registration anticipated but whose magnitude it underestimated by three orders.

#### WHAT STANDS, AND WHAT §50 HAS TO BE RESTATED AS

Unaffected and solid:

* **`T1.BASE` is null at +0.0170 ± 0.0092**, the tightest number in the table, so the association
  route stays shut and `g2cond`/`pairprobe` are re-confirmed.
* **The retinal index survives a speaking caregiver** (+0.2269 vs +0.2494 silent on the same budget),
  so added auditory drive costs the visual index ~0.02. New, and it is a real measurement.
* **The protocol teaches**: `T0` at 12/12.
* **`T2` is underpowered, as pre-registered.** It fired at 0.39× T0 because T1's teacher *requires*
  voicing and so reinforces it while a coin does not — **the content of a reward raises its own
  dose** — and it reads +0.0386 ± 0.0969, consistent with the arithmetic's zero but not a clean floor.

And the correction, which is to a result written up and pushed earlier the same day:

> **§50 MUST NOT BE READ AS "THE CREATURE SAYS THE TWO WORDS DIFFERENTLY".** Its +0.5875 is
> compliance with the separation its teacher asked for, and that separation is **0.05% of a formant
> separation**. What §50 established is that *the creature's motor state separates by object, along
> the direction that hearing the two words produces*. It did **not** establish that the two objects
> sound different, and on the one arm of §51 that measured the audible quantity, §50's own teacher
> moved it **−0.0059** — slightly the wrong way.
>
> The §50 section and `CONCLUSION.md` are amended accordingly. The mechanism claim survives intact:
> node perturbation *can* learn a conditional mapping from what is in view to what the larynx does,
> which is what corrected `g3probe`'s null. What does not survive is the headline's implication that
> the mapping reaches the vowel.

#### §52 IS NOW OBVIOUS, AND IT IS CHEAPER THAN THIS RUN WAS

Score the voice on the quantity a listener hears, not on the echo axis, and give the load-bearing
null enough creatures:

1. **Primary becomes the formant gap** with an SE and a paired label-destroyed contrast, the way
   `axis_sep` was built for §50.
2. **`T1.ORACLE` at n = 36 with a `T1.ORACLE.D` beside it** — `null-is-not-a-control`, and this run
   is why.
3. **Keep `T0.RET` as the 4th rung and `axis_sep` as a reported secondary**, so the dissociation is
   measured on both rulers in one instrument rather than inferred across two sections.

The pre-registration's other escape hatch also remains open and is now owed: the **baseline memory**
was derived rather than inherited, and the alternative reading — a fast baseline is ordinary RPE and
sharper credit — was written down as *"the first thing to vary if T1 nulls."* T1 nulled on a ruler
that could not see it, so that line is deferred rather than discharged, and it is deferred for a
stated reason.

**And the method lesson, which cost an hour of compute and would have cost nothing to avoid:
CALIBRATE THE ESTIMATOR IN THE UNITS OF THE CLAIM BEFORE SPENDING THE BUDGET.** The calibration is
~20 lines, runs in Phase A, needs no teaching at all, and is a function of the creature seed. It was
available before §49, let alone §50. Three sections were scored on a ruler whose full-scale reading
was never converted into the units the milestone is written in.

## §52 — PRE-REGISTERED: THE SAME QUESTION ON THE AUDIBLE RULER (2026-11-08)

§51 established that its own primary cannot see what its teacher aimed at: `axis_sep = 1.0` is worth
**+0.0008** of a formant gap, so `T1`'s −0.0686 is a statement about an estimator, not about a
teacher. §52 re-asks §51's question with the **formant gap as the primary**, the arms §51's `ORACLE`
lacked, and an `n` that is finally derived from a measured spread.

### THE PRE-FLIGHT, AND IT INVERTS §51's BUDGET ARGUMENT

§51 reported the formant gap as a bare mean, so the quantity §52's verdict rests on had **no
published SD** — which is the error §51 itself recorded. Two pilot runs at different budgets now
measure it (`results/n52-pilot-150k.log`, `results/n52-pilot-450k.log`, both `--allow-short`):

| quantity | 150k | 450k | ratio |
|---|---|---|---|
| `T1.RET − T1.RET.D` paired **SD** | 0.0451 | **0.1689** | **×3.7** |
| `T1.ORACLE` per-creature SD | 0.1080 | 0.1752 | ×1.6 |
| ticks | — | — | ×3.0 |

**THE SD GROWS WITH THE BUDGET, ROUGHLY IN PROPORTION TO IT.** That is not estimation noise
shrinking as probes accumulate — it is the opposite, and it is what `ctxscale` already describes:
`bias_ctx_` is *"a pure clamped accumulator"* with a near-diffusive growth exponent, so the per-slot
tables random-walk further apart the longer they run, and the formant gap inherits that spread.

**So §51's reasoning — "double the budget, it can only help" — is wrong for this estimator, and
more ticks can buy negative power.** Projecting the trend, at 1.8M the paired SD would be ≈0.68, so
`n = 36` would give SE ≈ 0.113 and a 2 SE bar of **0.225** — which **would not detect `T1.ORACLE`'s
own +0.1889**, the very effect §52 exists to pin down. At 450k the same `n` gives SE 0.0282 and a bar
of **0.056**.

### THE BUDGET IS CHOSEN ON POWER, AND THE PILOT'S OWN CONTRAST IS RING-FENCED

**This matters and is stated before the run.** The 450k pilot also happens to read
`T1.RET − T1.RET.D` = **+0.0627 ± 0.0282 = 2.2 SE**, and that is **not** the reason 450k was chosen
and **must not be cited as a result.** M1b's lesson is explicit: *selecting the window on the outcome
is a search, not a milestone.* The budget argument above uses only (a) the measured **SD**, which is a
variance and not an effect, and (b) a target effect of **+0.19** taken from **§51's completed full
run**, not from either pilot.

And because a single budget is itself a selection when the SD moves with it, **§52 runs TWO budgets
as a pre-registered dose axis — 450 000 and 900 000** — so the result is not a one-budget artefact.
A finding that appears at one and not the other is reported as exactly that.

### THE ARMS, WITH THE TWO §51 LACKED

| # | arm | index | teacher | word | n |
|---|---|---|---|---|---|
| 1 | `T0.RET` | retina | §50's, object-signed | silent | 12 |
| 2 | **`T1.RET`** | retina | sound-only | **= object** | **36** |
| 3 | `T1.RET.D` | retina | sound-only | drawn | **36** |
| 4 | **`T1.ORACLE`** | pinned true object | sound-only | = object | **36** |
| 5 | **`T1.ORACLE.D`** | pinned true object | sound-only | **drawn** | **36** |
| 6 | `T1.BASE` | table off | sound-only | = object | 12 |

* **`T1.ORACLE` rises from 12 to 36**, because it is the arm whose *null* was load-bearing in §51 and
  whose 2 SE interval spanned the whole effect being compared against.
* **`T1.ORACLE.D` is new and is the reason §51 could not finish the argument.** `null-is-not-a-control`:
  without a decorrelated counterpart, `ORACLE`'s +0.1889 cannot be separated from whatever a pinned
  two-slot table does to the formant gap on its own. **This arm is the one §52 is really for.**
* **`T2.RET` is dropped.** It was underpowered in §51 at 0.39× dose, and it is now redundant: the two
  `.D` arms are proper label-destroyed controls on the actual primary, which a yoked-reward arm is
  not. Dropping it pays for `ORACLE.D`.

168 sessions per budget. From §51's measured rate (120 sessions × 1.8M in 57m28s), 450k is ≈20 min
and 900k ≈40 min: **≈60 min for both**, against §51's 57 for one.

### CLAUSES

**Clause 0 — dose and baseline**, unchanged from §51 and still gating: fired/eligible ≥ 0.50 of T0's,
T1 praise fraction inside [0.35, 0.65]. **Clause 1 — the index** at ≥ +0.20 excess. **Clause 2 — the
probe is silent**, `probe heard` exactly 0.0000 and `teach heard` 0.4444. All three passed in §51 and
are re-asked rather than assumed.

**Clause 3 — THE RULER, FIRST THIS TIME.** The calibration block prints before any verdict:
`echo gap` and the share of `axis` in F1+F2. If `echo gap` is again ≈+0.0008, then `axis_sep` is
reported as a **secondary only** and no verdict may rest on it. **This clause exists because §51
spent an hour discovering it after the fact.**

**Clause 4 — the 4th rung, and its prediction is INVERTED.** `T0.RET` must still reach ≥ +0.30 on
`axis_sep` (its own ruler) to show the protocol teaches. But on the **formant** primary §51 measured
T0 at **−0.0059**, so T0 is predicted **NULL OR NEGATIVE** here. A large positive T0 on the formant
gap would contradict §51's calibration and voids the run pending re-measurement of the ruler.

**Clause 5 — the verdict.** Primary `T1.RET − T1.RET.D`, paired by creature, bar 2 SE, sign
unconstrained. Ceiling contrast `T1.ORACLE − T1.ORACLE.D`. Branches:

1. Clause 0, 1, 2 or 4's rung fails → **VOID.**
2. `T1.BASE` ≥ 2 SE above zero → **the association route works**; report as the headline and re-run
   `pairprobe` before believing it.
3. **`T1.ORACLE − T1.ORACLE.D` ≤ 2 SE → §51's +0.1889 WAS THE TABLE, NOT THE TEACHING.** The hint
   that motivated §52 dies, and the sound-only teacher is refused on both rulers. This is the branch
   that would close the line, and it is written first on purpose.
4. Ceiling clears 2 SE **and** primary clears 2 SE at **both** budgets → **THE SOUND-ONLY CAREGIVER
   TEACHES, AND §51's NULL WAS THE RULER.** Report T1 as a fraction of the ceiling.
5. Ceiling clears, primary clears at **one** budget only → reported as budget-dependent, with the
   accumulator's SD growth as the named candidate mechanism. **Not** promoted to a result.
6. Ceiling clears, primary null at both → **the index is not good enough for this teacher** on the
   audible ruler; next step is the index.
7. Primary ≤ −2 SE → **a reversal**, reported as a finding, with `T1.RET.D`'s persistent negative
   drift (−0.0064 at 150k, −0.0691 at 450k, −0.0678 at 1.8M) as the thing to explain first. **That
   drift is unexplained today**: the resting voice sits much closer to "ball" (perr 0.574) than to
   "cube" (1.069), but averaged over both objects a resting voice scores exactly zero, so a
   systematic −0.07 in a label-destroyed arm is not yet accounted for and `ORACLE.D` is the arm that
   will say whether it is general.
8. Otherwise **INCONCLUSIVE**, with the SE printed.

### WHAT THIS DOES NOT DO

It does not revisit §51's **baseline-memory** constant, still deferred and still owed. It does not
address the larger thing §51 exposed — **the innate echo route does not move the vowel**, which is
why the two echo postures are 2 Hz apart and which no amount of re-scoring will change. If §52
confirms that a sound-only teacher moves the formants, then **the echo route's silence on vowels
stops being an obstacle and starts being the finding**: reward reaches the vowel where the reflex does
not.

### §51's VERIFY GATE, ALL FOUR KEYS

`results/n51-verify.log`:

```
verify PASS
hash              ad96f882becbee92  as recorded
pinned hash       PASS
experiments       24 as expected, 0 not
```

The pinned hash is unmoved, so §51's `teacher`/`pairing` parameters, the dose counters, the probe
formant accumulator, the ruler calibration and the moved `formant_error` are all inert for the
shipped genome. The count stays 24 because `objword` and `wordruler` are both `Tier::kLong` /
`Expect::kRead`, exactly as `objname` and `visprobe` were.

**And the log was re-run to get it.** The first two attempts collided: `--verify` is not an argument
(the flag is `--experiment verify`), and then two runs were launched redirecting to the *same* path,
so they interleaved. **The interleaved log still ended in `verify PASS` and looked entirely clean**,
which is why it was discarded rather than read — `smoke-the-exact-command` is about the invocation,
and this is the same lesson for the output file.

### §52 RESULT: THE SOUND-ONLY CAREGIVER TEACHES — AT THE CEILING. THE RETINA'S INDEX DOES NOT CARRY IT

Two budgets, 168 sessions each, **17m49s + 37m22s** (`results/n52-wordruler-450k.log`,
`results/n52-wordruler-900k.log`). Clause 5 fired branch **(4)** at *both* budgets. **The branch's
printed text is too generous, and the arm-by-arm decomposition is why.**

Every gate passed at both budgets: the rung (T0 on `axis_sep` at 2.5 and 3.9 SE), the index (+0.20
bar), the probe silent to 0.0000, dose 0.71–0.80× T0, praise fractions 0.51–0.56. And clause 3, which
prints the ruler *first* this time, flagged **every arm** as `NOT A FORMANT RULER` — `echo gap` 0.05%
to 1.39% of the gap available — so `axis_sep` was demoted to a secondary before any verdict, which is
§51's correction applied in advance instead of an hour late.

#### THE PRIMARY AND THE CEILING, BOTH REPLICATED

| contrast | 450k | 900k |
|---|---|---|
| `T1.RET − T1.RET.D` (**pre-registered primary**) | +0.0627 ± 0.0282, **2.2 SE** | +0.1101 ± 0.0375, **2.9 SE** |
| `T1.ORACLE − T1.ORACLE.D` (**ceiling**) | +0.1675 ± 0.0408, **4.1 SE** | +0.1613 ± 0.0493, **3.3 SE** |

Both clear 2 SE at both budgets, which is what the pre-registered dose axis was for. But a paired
contrast says only that the *pairing* matters; it does not say which arm moved. **It must be
decomposed, and the two pairs behave completely differently.**

#### ARM BY ARM, AGAINST ZERO — THE WHOLE RESULT IS HERE

| budget | `T1.RET` | `T1.RET.D` | **`T1.ORACLE`** | `T1.ORACLE.D` |
|---|---|---|---|---|
| 150k | −0.0002 ± 0.0073 | −0.0064 ± 0.0052 | **+0.0819 ± 0.0177 (4.6 SE)** | −0.0220 ± 0.0203 |
| 450k | −0.0064 ± 0.0175 | −0.0691 ± 0.0242 | **+0.1572 ± 0.0263 (6.0 SE)** | −0.0103 ± 0.0288 |
| 900k | +0.0233 ± 0.0193 (1.2 SE) | −0.0868 ± 0.0304 | **+0.1824 ± 0.0328 (5.6 SE)** | +0.0211 ± 0.0327 |

**THE CEILING IS CLEAN, STRONG AND MONOTONE.** `T1.ORACLE` rises +0.082 → +0.157 → +0.182 across a
6× budget range at **4.6, 6.0 and 5.6 SE above zero**, 30 of 36 creatures positive at the two larger
budgets, while its decorrelated twin sits at zero throughout (−0.022, −0.010, +0.021). The effect is
**in the claim arm** and the control is where a control should be.

> **So this is established: a caregiver that only ever compares two sounds, and never looks at the
> toy, CAN teach this creature a conditional mapping from what it sees to the vowel it produces.**
> The reward never contained the object. The target was `{320, 2500}` in the world's units, not a
> direction fitted in the creature's motor space. The binding came from **co-occurrence**, and
> destroying the co-occurrence while keeping the rule, the clock, the dose and the index removes it.
>
> **§51's headline was a ruler artefact and is now overturned by measurement, not by argument.**
> `vocallearn`'s wall — *"reward cannot reshape the conditional route"* — does fall once the
> condition has somewhere to live, exactly as §50's correction to `g3probe` suggested it might. It
> just could not be seen on `axis_sep`.

#### AND THE PART THAT IS **NOT** ESTABLISHED, WHICH BRANCH (4) OVERSTATES

**`T1.RET` — the creature's own index — is not significantly above zero at any budget.** Its best
reading is +0.0233 ± 0.0193, **1.2 SE**. The pre-registered primary clears 2 SE almost entirely
because its *control* goes negative: at 900k, **+0.0233 of the +0.1101 contrast comes from the claim
arm and +0.0868 — 79% — from `T1.RET.D` being below zero.**

That is a materially different claim from "the voice moved toward the word", and the instrument's own
branch text does not make the distinction. **Corrected here: §52 does not show that the creature's
self-derived visual index drives a sound-only teacher.** It shows that a *perfect* index does.

#### THE UNEXPLAINED NUMBER, NOW SHARPENED INTO A SPECIFIC QUESTION

`T1.RET.D` drifts negative and the drift **grows with budget** — −0.0064, −0.0691, −0.0868 here, plus
§51's −0.0678 at 1.8M, across four budgets. A label-destroyed arm should sit at zero: each slot
receives both words' targets, and averaged over the two objects a uniform drift cancels by symmetry.

**The pre-registration named `T1.ORACLE.D` as the arm that would say whether this is general. It
answered, and the answer is no** — ORACLE.D reads −0.022, −0.010, +0.021, i.e. zero, at the same three
budgets. So the drift is specific to the **derived** index paired with a **drawn** word, and it is not
a property of drawn-word arms as such.

**I cannot derive its sign and am not going to invent one.** `retention` says conflicting lessons in
one slot are pure overwrite, which predicts a random-per-slot residue averaging to zero, not a
consistent negative. What this means practically: **the pre-registered primary is inflated by an
artefact in its control**, which is `control-blind-to-its-artefact` turned inside out — there the
control was blind to the arm's artefact, here the control *carries* one. **This is the next question,
and it is a cheap one**: it needs a `RET` arm with the index derived but the slot-to-object
correspondence broken downstream, not another hour of the same six arms.

#### TWO REPRODUCTIONS WORTH RECORDING

1. **`T0.RET` on `axis_sep` at 900k reads +0.5875 ± 0.1500 — §50's published number to four
   decimals**, from a *different driver* (`wordruler` rather than `objname`). Same genome, same
   `seed + r·7919`, same budget, same held toy. A cross-instrument reproduction of a headline figure.
2. **T0's formant prediction held.** §51 measured T0 at −0.0059 on the formant gap and §52
   pre-registered it as null-or-negative; it reads +0.0099 and +0.0235, both inside 1 SE of zero.
   **The teacher that moves `axis_sep` by +0.59 moves the audible quantity by nothing**, and the
   teacher that moves the audible quantity by +0.18 moves `axis_sep` by +0.04. **The dissociation is
   now established at 3–6 SE from both directions**, on three budgets, which is far past where §51
   could only flag it.
3. `T1.BASE` is null at both budgets (−0.0046, −0.0069), so the **association route stays shut** and
   `g2cond`/`pairprobe` are re-confirmed a second time.

#### WHAT IS LEFT

* **The `RET.D` artefact**, above — the only thing standing between §52 and a clean claim about the
  creature's own index.
* **The baseline-memory constant**, still deferred from §51 and now less urgent: the teacher
  demonstrably works at the ceiling under the derived rate, so that rate is no longer a candidate
  explanation for a null.
* **The echo route still does not move the vowel** — the two echo postures remain 2 Hz apart on F1,
  and nothing here changes that. §52 reframes it: **reward reaches the vowel where the innate reflex
  does not.** That is a cleaner statement of this architecture's division of labour than either
  section had on its own, and it belongs beside `CONCLUSION.md`'s finding rather than inside §52.

### §52's VERIFY GATE, ALL FOUR KEYS

`results/n52-verify.log`: `verify PASS`, `hash ad96f882becbee92 as recorded`, `pinned hash PASS`,
`experiments 24 as expected, 0 not`. The count stays 24 because `wordruler` is `Tier::kLong` /
`Expect::kRead` like `objword`, `objname` and `visprobe` before it, and the pinned hash is unmoved
because §52 added **no shared code at all** — `pin_mode` and `pairing` already composed into the one
arm it needed.

### §53 PRE-REGISTRATION: WHY THE CONTROL MOVED — THE PRIMARY IS ALGEBRAICALLY DECOMPOSABLE

§52 established the ceiling and left exactly one thing in the way of a clean claim about the
creature's own index: **`T1.RET.D` drifts negative and the drift grows with budget** — −0.0064,
−0.0691, −0.0868 at 150k/450k/900k, plus §51's −0.0678 at 1.8M. **Four budgets, and 25 of 36
creatures negative at 900k** (binomial p = 0.0057 against a coin), so it is a consistent
displacement and not a spread. A label-destroyed arm should sit at zero, and `T1.ORACLE.D` — the
same destroyed label with the index *pinned* — does: −0.022, −0.010, +0.021.

The obvious next move is another arm. **It is the wrong move, and the reason is §51's lesson at full
cost: the estimator was never calibrated.** This primary is not an opaque score. It decomposes
exactly, into one term the bias table can explain and one it cannot, and the decomposition is
measurable on the arms already built. **§53 measures the estimator. The arm, if one is still needed,
is designed afterwards from what it says.**

#### THE ALGEBRA, WRITTEN DOWN BEFORE ANYTHING IS BUILT

Per probe, the creature is shown object `o` ∈ {0,1} and produces a posture `p`. Define the posture's
**word preference**

> `G(p) = perr(p, kWords[1]) − perr(p, kWords[0])`, positive meaning the posture sits nearer /a/ "ball".

`G` is a property of the posture alone and does not mention the object. The primary's per-probe
quantity `perr(other) − perr(own)` is `+G` when object 0 is shown and `−G` when object 1 is, so with
`σ₀ = +1`, `σ₁ = −1` the measured gap over `N` probes is

> `gap = (1/N) · Σ_{o,s} n_{o,s} · σ_o · Ḡ_{o,s}`

where `n_{o,s}` is the probe count in cell (object `o`, modal slot `s`) and `Ḡ_{o,s}` that cell's
mean `G`. Writing `Ḡ_s` for slot `s`'s mean over all its probes, this splits **exactly**:

> `gap = SLOT TERM + OBJECT RESIDUAL`
> `SLOT TERM      = (1/N) · Σ_s (n_{0,s} − n_{1,s}) · Ḡ_s`
> `OBJECT RESIDUAL = (1/N) · Σ_{o,s} n_{o,s} · σ_o · (Ḡ_{o,s} − Ḡ_s)`

This is an identity, not a model. What it buys:

* **The SLOT TERM is the only part a per-context bias table can produce.** It is non-zero only when
  the slot counts differ between the objects — only when the index carries the object — and it is
  then proportional to the **difference in word preference between the two slots**.
* **The OBJECT RESIDUAL is the same slot producing a different posture depending on what is in
  view.** `bias_ctx_` is the only per-context store in the kernel, so a two-slot table predicts this
  is zero. If it is large, the lesson is living somewhere other than the bias table —
  `storage-vs-expression` is the archive's name for that distinction, and it would be a finding
  rather than a nuisance.
* **With a pinned index the cells are exactly diagonal, so `OBJECT RESIDUAL` must be EXACTLY 0.000
  and `SLOT TERM` must equal the measured gap to the last digit.** That is a self-test of the
  bookkeeping, not a result, and it is the reason `T1.ORACLE` is in the arm table at all
  (`plausible-wrong-number`: the dangerous defect is the number that looks right).

#### THE HYPOTHESIS, AND ITS SIGN IS DERIVED

Label slot **A** = the slot object 0 most often occupies, **B** the other. Then `n_{0,A} − n_{1,A} >
0` and `n_{0,B} − n_{1,B} < 0`, so `sign(SLOT TERM) = sign(Ḡ_A − Ḡ_B)`. **The negative drift
therefore says: the slot object 0 occupies sits nearer /i/ than the slot object 1 occupies.** In the
arm where the taught word is a coin. That needs explaining and the symmetry argument does not supply
it, so here is one that makes a signed, falsifiable prediction.

**H — THE UNEQUAL-DOSE ARTEFACT.** Under a drawn word each slot receives a 50/50 mixture of both
words' lessons, so both are pulled from the untaught posture toward the *same* compromise. The two
slots are exchangeable — and therefore `E[Ḡ_A − Ḡ_B] = 0` — **only if they are written equally
often.** They are not: §52 measured probe occupancy at **0.5347** against 0.5000 for balance, a
~53/47 split, because MacQueen clustering under a conscience does not partition a stimulus set
evenly. A slot written more often is pulled **further**.

And the pull has a known direction, because **this creature does not start out between the two
words.** At the echo postures §51 measured (630/1691 Hz), `perr` to "ball" is 0.5734 and to "cube"
1.0684, so

> **`G(untaught) = +0.4950` out of an available 1.6417 — the untaught voice already sits 30% of the
> way toward /a/ "ball".**

This asymmetry has been in every section since §49 and has never been quoted. Being pulled toward a
50/50 compromise therefore **lowers** `G`. Hence H's prediction:

> **`sign(Ḡ_A − Ḡ_B) = −sign(dose_A − dose_B)`**, i.e. the more-written slot has the lower word
> preference, giving a negative `SLOT TERM` whose magnitude grows with the number of lessons — which
> is the budget dependence. **`T1.ORACLE.D` is immune because the pin makes the two doses exactly
> equal and the cells exactly diagonal**, which is the dissociation §52 measured and could not
> explain.

#### WHAT IS MEASURED — AND THE ORDER IS PART OF THE PRE-REGISTRATION

Instrumentation only; no new mechanism, no new genome field, no core change (`active_context()` and
`context_present()` already exist). Per creature:

1. the four cells `n_{o,s}` and `Ḡ_{o,s}` at the silent probes, hence `gap`, `SLOT TERM`,
   `OBJECT RESIDUAL`, `Ḡ_A`, `Ḡ_B`;
2. **the teaching dose per slot** — the live slot at each firing of the caregiver, so `dose_A` and
   `dose_B` are counted where the credit is actually delivered rather than inferred from the probes;
3. **the taught word per slot**, which tests H's premise directly: the mixture must read 50/50 per
   slot in the drawn arms and ~pure in the paired arms;
4. `Ḡ` at the untaught reference, so the +0.4950 above is re-measured in the run rather than carried
   in from §51.

**The three tests, in this order:**

* **T-A, the bookkeeping self-test.** `T1.ORACLE`'s `OBJECT RESIDUAL` is 0.000 and its `SLOT TERM`
  equals its measured gap. **If this fails nothing else in §53 may be quoted**, whatever it says.
* **T-B, the primary: the paired sign test.** In `T1.RET.D`, per creature, does `sign(Ḡ_A − Ḡ_B)`
  oppose `sign(dose_A − dose_B)`? Bar **≥26 of 36 opposing** (one-sided binomial p = 0.0057); **≥26
  agreeing refutes H with its own sign**; anything between is undecided. No SD is assumed and the
  test is within-arm, so no between-arm confound reaches it.
* **T-C, the share.** Does the dose route account for **≥50%** of the mean gap? Reported as the mean
  `SLOT TERM` against the mean measured gap, plus the per-creature regression slope of `gap` on
  `(dose_A − dose_B)`.

#### REFUSAL CLAUSES — FOUR-WAY, BECAUSE THE SIGN IS CONSTRAINED BUT THE SHARE IS NOT

1. **T-A fails** → `§53 VOID — THE BOOKKEEPING IS WRONG.` Nothing is quoted; the identity is
   arithmetic and a disagreement is my error, not the creature's.
2. **T-B ≥26 opposing and T-C ≥50%** → `THE CONTROL'S DRIFT IS AN UNEQUAL-DOSE ARTEFACT, EXPLAINED.`
   §52's pre-registered primary is inflated by it; the honest statement of the derived arm is its own
   gap against zero, and the dose term's size says how much of `T1.RET`'s +0.0233 was *suppressed*
   by the same term. **That cuts against §52's correction, which is the point of deriving the sign
   before looking.**
3. **T-B ≥26 opposing but T-C <50%** → `THE DOSE ROUTE IS REAL AND PARTIAL.` Half the displacement is
   still unaccounted for and the next suspect is named, not invented: the caregiver's **shared**
   per-word baseline `t1_base[word]`, which couples the two slots through one EMA.
4. **T-B ≥26 agreeing, or undecided** → `H REFUTED` / `NO VERDICT ON H.` In either case the
   decomposition still stands on its own: the split of §52's primary into a slot-mediated term and an
   object residual is reported, and **if the OBJECT RESIDUAL is large the finding is that the lesson
   is not in the bias table**, which redirects the whole line.

#### VACUITY, AND THE GUARD IS ASKED OF EVERY ARM

* A creature using ≠2 live slots is dropped from the cell columns and the count is **disclosed**;
  §52 read `slots 2 (ask 2)` and `present 1.0000` on every derived arm, so this should drop none.
* **The `OBJECT RESIDUAL` is unidentified unless the off-diagonal cells are populated.** Each of the
  four cells must hold ≥5% of a creature's probes or that creature is dropped from the residual
  column only. §52's derived index excess was +0.3201 against a pinned +0.4660, so off-diagonal
  probes exist by construction — but it is measured, not assumed.
* Clause 0's dose, index-excess and probe-silence gates are carried over unchanged, and
  `vacuity-gate-cannot-guard-a-rejection` applies: **every gate is read on `T1.ORACLE.D` too**, the
  arm whose null is load-bearing.

#### BUDGET, POWER AND WHAT WOULD MAKE THIS UNDERPOWERED

`slotdose`, a **new driver**, so §51's and §52's logs stay byte-reproducible. Arms: `T1.RET` (36),
`T1.RET.D` (36), `T1.ORACLE.D` (36), `T1.ORACLE` (12, the self-test) = **120 sessions**, at **450k and
900k** — the two budgets where the drift is −0.0691 and −0.0868, so the effect under test is at its
largest and the budget axis doubles as a dose axis for H's "the pull grows" clause.

**The primary needs no variance estimate**: T-B is a sign test on 36 paired comparisons. What *could*
make it underpowered is a dose imbalance too small to resolve — if `|dose_A − dose_B|` is under a few
percent for most creatures the signs are coin flips whatever the mechanism. **That is a measured
quantity and it is read before T-B**, with the median `|dose_A − dose_B|` printed; under 2% on more
than half the creatures the arm is declared **UNDERPOWERED, NOT NULL** (`liveness-is-not-dose`).

**Disclosed before the run:** the dose is counted at the caregiver's firing, while the kernel's
perturbation write consumes that reward one plasticity interval (10 ticks) later. Within a trial the
live slot is constant — §52 read `present 1.0000` — so the lag mis-attributes only writes that
straddle an episode boundary. It is a known, bounded, one-interval error in the dose column and not
in the probe cells.

### §53 ADDENDUM, WRITTEN FROM THE SMOKE AND BEFORE EITHER REAL BUDGET RETURNED

The 150k smoke (`results/n53-smoke-150k.log`) passed T-A for real — identity 8.3e-17, `T1.ORACLE`'s
residual **exactly** 0.0000, and both pinned arms' `SLOT` equal to their `gap` to the last digit —
and it reproduced §52's archive on four independent columns **including the standard errors**:
`T1.RET` −0.0002 ± 0.0073, `T1.ORACLE` +0.1049 ± 0.0312, `T1.ORACLE.D` −0.0220, occupancy 0.6875 and
0.6847. The instrument adds only unprinted accumulators and the session did not move. A
`REPRODUCTION COLUMN` was then added so the log carries that proof itself rather than my checking it
against the archive by hand.

It also raised a quantitative problem with H, and the honest place to put it is **before** the
result:

> **THE DOSE IMBALANCE SHRINKS WITH BUDGET WHILE THE DRIFT GROWS.** The smoke measures `T1.RET.D`'s
> median `|Δdose|` at **0.383** with probe occupancy **0.6847** at 150k, where §52 measured occupancy
> **0.5347** at 900k — the conscience equalises the partition as the budget grows. But the drift runs
> the other way: −0.0064 at 150k, −0.0868 at 900k. **H's sign can pass while H's magnitude fails.**

So a fifth clause, pre-registered now:

* **T-D, THE COEFFICIENT.** H says `slot term ≈ k · Δdose · pull`. Fitting `k` at each budget, the two
  must agree. The smoke gives slot term −0.0116 at `Δdose` 0.383. **If the implied `pull` has to grow
  by more than ~10× across a 6× budget to reconcile 450k and 900k, H is quantitatively inadequate
  even with its sign test passing**, and the verdict must say so rather than claiming the artefact is
  explained. `bias_ctx_` is a clamped accumulator, so a random walk affords √t ≈ 2.4× and a drift
  affords 6×; the smoke's arithmetic projects a requirement near 40×, which is outside both.

And the smoke measured something no earlier section had, which may matter more than H:

> **THE DERIVED INDEX BARELY SEPARATES THE LESSONS.** In the paired arm `T1.RET`, slot A receives
> **46.9%** of its lessons teaching /i/ and slot B **55.3%** — against a pinned index's 0% and 100%.
> The condition is almost absent *at the moment the bias table is written*, which is a different and
> earlier failure than a noisy index at probe time. §52 could not see this: it measured the index on
> the probes, where `excess` read +0.3201. **If the mixture is still near 50/50 at 900k, `T1.RET`'s
> weak +0.0233 is explained without H at all** — the lessons never got sorted into slots, so there
> was nothing for the probe to read back. The mixture column is printed for every arm and this
> reading is now pre-registered as a finding in its own right.

### §53's THIRD READING, PRE-REGISTERED WHILE THE 450k RUN WAS STILL EXECUTING

The mixture column does not merely diagnose the derived index — it **predicts the claim arm's
magnitude with no free parameter.** `mix_s` is the fraction of slot `s`'s lessons that taught /i/, so
`mix_B − mix_A` is the share of the available conditional signal that actually reaches the bias
table, on a scale where a pinned index scores 1.000 (measured: 0.000 and 1.000 in `T1.ORACLE`). If
the only thing wrong with the derived arm is that its lessons arrive mixed, then

> **predicted `T1.RET` gap = (mix_B − mix_A) × `T1.ORACLE` gap**

a pure transfer with nothing fitted. At 150k the smoke gives `mix_B − mix_A` = 0.553 − 0.469 =
**0.084** and `T1.ORACLE` = +0.1049, predicting **+0.0088** against a measured −0.0002 ± 0.0073 —
consistent, though at 150k every term is small. **The 450k and 900k budgets are where this is worth
anything**, and the prediction is recorded here before either returned.

The three accounts now make *different* predictions, which is the point of writing them down
together:

| | predicts `T1.RET.D` < 0 | predicts `T1.RET` weak | predicts the budget trend |
|---|---|---|---|
| **H, unequal dose** | yes, sign derived | no — it is a control effect | **wrong way** (imbalance shrinks) |
| **impure write-time index** | **no** | yes, with a magnitude | yes, if purity improves slowly |
| **both** | yes | yes | needs T-D to adjudicate |

So the mixture reading and H are **not** rival explanations of the same number: H is about the
control, purity is about the claim arm, and a log can support both, one, or neither. The verdict must
say which, and the instrument prints all four columns needed to tell.

### §53 RESULT: THE DECOMPOSITION WORKS, THE REGISTRATION IS PERFECT, AND MY DERIVED SIGN WAS WRONG

Two budgets, 120 sessions each, **12m + 26m36s** (`results/n53-slotdose-450k.log`,
`results/n53-slotdose-900k.log`). The pre-registered verdict clause fired **(4b) NO VERDICT ON H** at
both budgets. That is the honest reading of the test I nominated as primary, and it understates what
the log contains in one direction and overstates it in another. Both are set out below.

#### T-A AND THE REPRODUCTION COLUMN, BEFORE ANY RESULT

The identity holds to floating point at every creature (worst **1.1e-16** over 120), `T1.ORACLE`'s
object residual is **exactly 0.0000**, and both pinned arms' slot term equals their gap to the last
digit. The decomposition is arithmetic and the arithmetic is right.

And §52's primary reproduces **exactly, SEs included, at both budgets** — `T1.RET` −0.0064 ± 0.0175
and +0.0233 ± 0.0193, `T1.RET.D` −0.0691 ± 0.0242 and −0.0868 ± 0.0304, `T1.ORACLE.D` −0.0103 ±
0.0288 and +0.0211 ± 0.0327. §53 added only unprinted accumulators and the session did not move.

#### THE REGISTRATION HYPOTHESIS IS REFUTED, AND IT IS THE FIRST TIME ANYTHING MEASURED IT

| arm | aligned | **swapped** | degenerate |
|---|---|---|---|
| `T1.RET` | 36 | **0** | 0 |
| `T1.RET.D` | 36 | **0** | 0 |
| `T1.ORACLE.D` | 36 | **0** | 0 |
| `T1.ORACLE` | 12 | **0** | 0 |

**At 900k the lesson is read back from exactly the slot it was written to, in every creature of every
arm.** The pinned arms at 100% aligned are the self-test; the derived arms at 100% aligned are the
result. So the sound-gated-latch story — teaching window not silent, probe silent, source 5 sharing
source 4's latch — **is dead**, and `T1.RET`'s negative word-preference difference is not a readout
swap.

This is worth recording as a method point regardless: **`vis_ctx_match` maximises over slot
permutations, so every index `excess` published since §49 is registration-blind by construction.**
§50's reassuring "probe +0.3165, teaching +0.3200" could not have detected a total swap. It now has
an answer, and the answer is favourable — but it was an assumption for four sections.

At 150k the same column reads **85–97% DEGENERATE** (both objects sharing one modal slot), which is a
far starker statement of the low-budget index failure than `excess −0.0409` was.

#### H: THE MECHANISM IS CONFIRMED AND THE SIGN I DERIVED IS WRONG

The pre-registered primary T-B, a paired sign test, reads **14 oppose / 22 agree** at 450k and **15 /
21** at 900k — neither side reaching the bar of 26, hence clause (4b). **But the secondary carries an
SE and the primary does not, and the secondary is unambiguous:**

| regression of gap on … | 450k | 900k |
|---|---|---|
| signed **firing** dose imbalance | +0.2374 ± 0.1061 (**+2.2 SE**) | +0.3534 ± 0.0848 (**+4.2 SE**) |
| signed **occupancy** imbalance | — | +0.4934 ± 0.0956 (**+5.2 SE**) |

**H's mechanism is right — unequal dose does drive the slot term — and H's direction is backwards, at
4.2 and 5.2 SE, replicated across two budgets.** The busier slot sits *nearer* /a/, not further.

The error is locatable and it was in the derivation, not the code. I assumed a 50/50 lesson mixture
pulls a slot toward the *midpoint* of the two words, which lowers `G` from the untaught +0.4950. The
run says otherwise: **`mean G` exceeds `echo G` in every arm** — +0.5192 and +0.5555 against +0.5021
in the derived arms, +0.5892 in `T1.ORACLE`. **Teaching moves this voice toward /a/ on average, even
under a 50/50 mixture**, because /a/ = {780, 1180} is reachable and /i/ = {320, 2500} is not, so a
baseline-EMA teacher banks progress on the reachable word and not on the other. More dose therefore
means *higher* `G`, which is the measured sign. **Asymmetric reachability, not a symmetric
compromise** — and `vowel-space` already said this creature has one vowel.

**DISCLOSED, AND IT IS THE DISCIPLINE POINT OF THIS SECTION.** Had I pre-registered the regression as
the primary, H would read REFUTED-WITH-SIGN at 4.2 SE. I pre-registered the sign test, which discards
magnitude and does not reach its bar. **The pre-registered verdict therefore stands at NO VERDICT ON
H, and I am not promoting the secondary after seeing which way it read** — `verdict-fitted-to-data`
is exactly this move. What the log supports is stated here and the clause is left as it fired.

#### THE BUDGET TREND IS EXPLAINED, AND T-D DISSOLVES

The §53 addendum worried that H needed a ~40× growth in the pull across a 6× budget, outside what a
clamped accumulator affords. It needs none:

> **`G_A − G_B` IS FLAT ACROSS BUDGETS — −0.1367 at 450k and −0.1358 at 900k — WHILE THE GAP GROWS
> FROM −0.0691 TO −0.0868.** The underlying slot asymmetry does not grow at all. What grows is how
> much of it the probe can *see*: §52's index excess goes 0.2286 → 0.3201 over the same budgets.
> **The drift grows with budget because the index gets better at revealing a fixed artefact.**

#### WHAT §52's PRIMARY ACTUALLY DECOMPOSES INTO

| budget | §52 primary | **slot route** | residual route |
|---|---|---|---|
| 450k | +0.0627 | **+0.0629** | −0.0002 |
| 900k | +0.1101 | **+0.1028** | +0.0073 |

**93–100% of §52's contrast travels through the per-context bias table**, so the route is the one §50
and §52 claimed. The object residual is small, and most of what there is appears **with the label
destroyed** (+0.0079 in `T1.RET.D` against +0.0152 in `T1.RET`), so it is not a lesson.

And the write-purity number nothing had measured: the derived index delivers **0.084 / 0.490 / 0.658**
of the available conditional signal to the bias table at 150k / 450k / 900k, against a pinned index's
1.000. The parameter-free prediction `purity × pinned contrast` gives **+0.1212 against a measured
+0.1138 at 900k — 6%** — and **+0.0753 against +0.0435 at 450k, off by 1.7×.** It holds at one budget
and not the other, and is reported as that rather than as a law.

#### THE ONE THING STILL OPEN, NOW A SINGLE NUMBER

`T1.RET`'s **slot term is +0.0081 ± 0.0170 at 900k — zero.** But its dose imbalance is 0.1324 where
`T1.RET.D`'s is 0.2863, **so the two arms carry different amounts of the confound and it does not
cancel in their difference.** That confirms §52's correction rather than overturning it: the
pre-registered contrast is not a clean estimate of the lesson in the derived arm.

Two readings of `T1.RET` are now consistent with the same data and they differ by 3×, which is
exactly why the next step is arithmetic and not another arm:

* if the artefact in `T1.RET` is the **same** as in `T1.RET.D` (−0.136), the lesson is +0.114 — **66%
  of the pinned ceiling, matching write purity**;
* if it scales with dose imbalance (half of `T1.RET.D`'s, −0.068), the lesson is +0.046 — **21%**.

**The regression already in hand settles it**: with slope +0.4934 ± 0.0956 measured, the artefact can
be regressed out *within* each arm and the slot term read at zero imbalance. That column was not in
this run because the confound's sign was not known when it was built. §53b adds it.

### §53b PRE-REGISTRATION: READ THE SLOT TERM AT ZERO IMBALANCE

§53 measured the confound's slope (+0.4934 ± 0.0956 on signed occupancy, **+5.2 SE**) and established
that the two derived arms carry **different amounts of it** (median |Δdose| 0.1324 against 0.2863), so
it does not cancel in their difference. With the slope in hand the correction is arithmetic: regress
each arm's per-creature slot term on its signed occupancy imbalance and **read the intercept at zero
imbalance** — the slot term a creature with evenly-occupied slots would show.

**Added columns**, all from data the session already returns: the **signed mean** of both imbalances
(§53 printed only the median of the absolute value, which is why its sign had to be inferred from a
regression rather than read off), the per-arm slope, and the intercept with its SE.

**PRE-REGISTERED PREDICTIONS, EACH WITH A DIRECTION:**

1. **`T1.ORACLE.D`'s intercept is within 2 SE of zero.** A pinned, label-destroyed arm has nothing to
   express at equal occupancy. **This is a self-test, not a result** — a non-zero value means the
   adjustment is mis-specified and §53b may not be quoted.
2. **`T1.RET.D`'s intercept is within 2 SE of zero** *if* the control's whole −0.0947 displacement is
   the occupancy confound. **A surviving negative means the confound is partial** and a second term is
   needed; the magnitude that survives is the size of that second term and is reported as such.
3. **`T1.RET`'s adjusted intercept is the lesson estimate**, and §53 already bracketed it between
   **+0.046 (21% of the pinned ceiling)** and **+0.114 (66%, matching write purity)**. The two
   readings differ by 3× and this number chooses between them. A value near 0.00 says the derived
   index's bias-table route carries nothing; a value near +0.11 says it carries the lesson at the
   strength its write purity predicts and a near-equal artefact was masking it.

**DISCLOSED BEFORE THE RUN, AND IT BOUNDS WHAT THIS CAN CLAIM:** the imbalance is a **measured
covariate, not a randomised one**. This is a covariate adjustment, so it removes a confound under a
linearity assumption it cannot itself verify, and it is not a causal estimate. It is reported as an
adjusted reading beside the raw one, never instead of it. The raw primary stays `T1.RET`'s slot term
against zero, **+0.0081 ± 0.0170**, which needs no model at all.

**Also disclosed:** the need for this column was established *by* §53's regression, so §53b is a
second look at the same data. The predictions above are fixed before the run, and the arms, budgets,
seeds and drop rules are unchanged — but it is a second look, and a reader should weigh it as one.

### §53b RESULT: THE DERIVED INDEX DOES NOT DRIVE THE LESSON INTO THE VOICE, AND BOTH OF MY BRACKETS WERE WRONG

`results/n53b-450k.log`, `results/n53b-900k.log`. Every pre-registered prediction returned, the
self-test passed where it was identified, and **the answer is below both ends of the bracket §53
offered** — which is the most useful thing a pre-registered bracket can do.

#### THE CONFOUND IS ARM-INDEPENDENT, WHICH IS WHAT MAKES IT A CONFOUND

Slope of the per-creature slot term on signed occupancy imbalance:

| arm | 450k | 900k |
|---|---|---|
| `T1.RET` | +0.3327 ± 0.1104 | +0.4465 ± 0.0908 |
| `T1.RET.D` | +0.3292 ± 0.1235 | +0.5223 ± 0.0925 |
| `T1.ORACLE.D` | +0.6247 ± 0.1717 | +0.5113 ± 0.1057 |
| `T1.ORACLE` | +0.7928 ± 0.5355 | +0.5566 ± 0.2230 |

**Eight estimates, all positive, clustering at 900k around +0.5 regardless of arm.** A quantity that
takes the same value in the claim arm, the label-destroyed arm and both pinned arms is a property of
the *measurement*, not of any lesson. Linearity holds: splitting `T1.RET.D` at its median gives
half-slopes differing by **0.8 SE** (450k) and **0.1 SE** (900k).

#### THE THREE PREDICTIONS

| | 450k | 900k | |
|---|---|---|---|
| **1. `T1.ORACLE.D` at zero imbalance** | +0.0006 ± 0.0249 (0.0 SE) | +0.0198 ± 0.0255 (0.8 SE) | **PASS** |
| **2. `T1.RET.D` at zero imbalance** | −0.0540 ± 0.0243 (−2.2 SE) | −0.0559 ± 0.0234 (−2.4 SE) | **survives** |
| **3. `T1.RET` at zero imbalance** | −0.0176 ± 0.0128 (−1.4 SE) | **−0.0053 ± 0.0135 (−0.4 SE)** | **zero** |

**Prediction 1 passed as a real test**, on a regressor with SD 0.146 and 0.245 — it could have failed.
*(Disclosed: the first version of this block asserted this test was void by construction, because the
150k smoke read SD 0.0126 on that arm. At the real budgets the pinned arms carry genuine occupancy
variation from dropped probes alone, so the log was printing "the regressor does not vary" next to an
SD of 0.2450. The branch now measures instead of asserting, and both budgets were re-run.)*

**Prediction 2: the confound is PARTIAL.** It accounts for 33% of the control's displacement at 450k
(−0.0802 → −0.0540) and 41% at 900k (−0.0947 → −0.0559), and **what survives is −0.055 at both
budgets** — stable across a 2× budget, so it is a real second term and not noise. §52's unexplained
control is now 40% explained and 60% open.

**Prediction 3, and it is the section's answer: ZERO.** −0.0053 ± 0.0135 at 900k. §53 bracketed this
between +0.046 (21% of the ceiling) and +0.114 (66%, matching write purity). **The measurement is
below both.** So:

> **THE CREATURE'S OWN RETINAL INDEX DOES NOT DRIVE THE SOUND-ONLY CAREGIVER'S LESSON INTO THE
> VOICE.** The bias-table route in the derived arm reads zero at both budgets, raw and adjusted,
> while the pinned route reads +0.1118 ± 0.0400 on the identical correction.

#### THE ADJUSTMENT'S POSITIVE CONTROL, WITHOUT WHICH THE NULL SAYS NOTHING

An adjustment that shrinks every arm toward zero would produce Prediction 3's reading whether or not
a lesson was there. What rules that out is the ceiling **surviving** the same correction:

* **900k: the pinned arm retains +0.1118 of its raw +0.1293 — 86%, at 2.8 SE above zero.** The
  correction keeps a real lesson where there is one and removes the derived arm's. **The null is
  informative.**
* **450k: +0.0683 of +0.0827 — 83% retained, but only 1.4 SE.** It does **not** clear 2 SE, so at
  that budget the run cannot separate a removed confound from a removed effect, and the instrument
  says so itself rather than leaving me to notice.

**So this conclusion rests on the 900k budget, and 450k corroborates its direction without licensing
it.** That is stated here because the guard is in the code and fired.

#### THE PUZZLE THIS LEAVES IS SHARPER THAN THE ONE IT CLOSED

Three measurements that should not sit together:

1. **write purity 0.658** — the derived index delivers two thirds of the available conditional signal
   into the correct slot (`mix` 0.154 / 0.811 against a pinned 0.000 / 1.000);
2. **registration 100% aligned** — every creature reads back from the slot it wrote to;
3. **expressed lesson 0.000** — and the pinned arm, given the same teacher, expresses +0.112.

**The conditional information arrives in the right slot and the voice does not express it.** That is
`storage-vs-expression` exactly, and the archive already holds a mechanism that would do it:
**`retention` measured conflicting lessons in one slot as PURE OVERWRITE, not averaging.** Under
averaging, a 19% wrong-slot rate dilutes an accumulation by 19%. **Under overwrite it erases it** —
each wrong lesson resets what the right ones built, so 0.658 purity is ample for an averaging store
and may be worth nothing in an overwriting one. That predicts expression collapses non-linearly in
the wrong-write rate, which is a quantitative claim about a mechanism already in the archive.

**NAMED AS THE NEXT QUESTION, NOT AS A FINDING.** §53 has refuted two of my own hypotheses by their
signs and found both ends of its own bracket wrong; this one gets measured before it gets believed.

### §53's VERIFY GATE, ALL FOUR KEYS

`results/n53-verify.log`, exit 0: `verify PASS`, `hash ad96f882becbee92 as recorded`, `pinned hash
PASS`, `experiments 24 as expected, 0 not`. The count stays **24** because `slotdose` is `Tier::kLong`
/ `Expect::kRead` like `wordruler`, `objword`, `objname` and `visprobe` before it, and the pinned hash
is unmoved because §53 touched **no shared code and no core file at all** — the session gained two
unprinted accumulators (`cell_*`, `dose_*`) and everything else is driver-side arithmetic over data
`run_g3probe_session` already returned.

**And a correction to how this gate gets checked.** §51 was fooled by two verify runs redirected to
the same path: the interleaved log still ended in `verify PASS` and read entirely clean. The lesson
was recorded as "smoke the exact command, including the output file" — but the *check* recorded with
it was the line count, and **this clean log is also 936 lines**, exactly like the corrupted one. Line
count is not the check. **The check is that each key appears exactly once**, which is what was run
here: `verify PASS` 1, `pinned hash PASS` 1, `as expected, 0 not` 1. A second interleaved run doubles
every marker and cannot hide in a count of one.

### §54 PRE-REGISTRATION: WHAT WRONG-WRITE RATE CAN THIS ARCHITECTURE TOLERATE?

§53 closed the derived index as an expression route and left one mechanism named: **the conditional
information reaches the correct slot — write purity 0.658, registration 100% aligned — and the voice
does not express it.** `retention` holds the candidate: conflicting lessons in one slot are **PURE
OVERWRITE, not averaging.**

**§53's evidence for this is correlational and that is its limit.** The derived arm has a wrong-write
rate *and* everything else that differs about deriving an index from a retina. §54 makes the
wrong-write rate the **only** manipulated variable: it takes the **pinned** index — a perfect index,
whose ceiling §52 and §53 both measured — and corrupts the slot the lesson is written to **at a known
rate**, leaving the read, the teacher, the clock, the dose and the probe untouched.

#### THE TWO MECHANISMS, DERIVED BEFORE ANYTHING IS BUILT

Let `w` be the fraction of a slot's lessons that teach the *other* slot's word.

* **AVERAGING** (`bias_ctx_` as an accumulator): increments sum, so the net is
  `(1−w)·(+Δ) + w·(−Δ) = (1−2w)` of full. **Expression is linear in `w` and reaches zero at
  `w = 0.5`.**
* **OVERWRITE** (a conflicting lesson *replaces* rather than subtracts): what survives is only the
  run of same-direction lessons since the last switch, of expected length `1/w`, against the `N`
  lessons a session delivers. **Expression collapses as roughly `1/(wN)` — i.e. it is gone as soon as
  `w` exceeds `1/N`, and `N` is in the thousands.**

**The derived arm's measured rate is `w = 0.1712`** (`mix` 0.1538 and 1 − 0.8113, averaged over the
two slots). There, averaging predicts **0.657 of the ceiling** — which is *exactly* §53's write-purity
figure of 0.658, because they are the same arithmetic — and overwrite predicts **~0.00**.

> **So the single most useful number here is not a p-value: it is the wrong-write rate at which this
> architecture stops being able to speak.** Averaging puts that half-collapse point at `w½ = 0.25`.
> Overwrite puts it near `1/N`. **`w½` is the primary and averaging's value for it is parameter-free.**

#### THE SWEEP, AND IT PASSES THROUGH THE MEASURED OPERATING POINT

`pin_mode = 3`: pin the true object, but on each **teaching** trial, with probability `w`, pin the
other slot instead. Per *trial* and not per interval, because §53 measured the derived index stable
within a trial (`present 1.0000`), so a per-interval coin flip would be a different noise process.
**The probe is never corrupted**, which reproduces the derived arm's measured 100% registration — the
lesson is read back from where it was written, and only the writing is wrong.

| `w` | n | why this level |
|---|---|---|
| 0.000 | 48 | **the ceiling, and the self-test**: must reproduce §52/§53's pinned arm |
| 0.020 | 24 | resolves a fast collapse — overwrite's `w½` should be near here |
| 0.050 | 24 | " |
| 0.085 | 24 | half the operating point: the convexity probe |
| **0.171** | **48** | **the derived index's MEASURED rate — `match-at-the-operating-point`** |
| 0.340 | 24 | twice it; averaging predicts 0.32 of ceiling and still clearly non-zero |

**192 sessions at 900k**, the budget where §53's ceiling is informative (its 450k positive control
cleared only 1.4 SE, so 450k is not run).

#### THE DOSE VERIFIES ITSELF, WHICH IS WHY THIS KNOB IS CHECKABLE

The mixture column already measures the **realised** wrong-write rate. At `pin_mode = 3` with the word
paired, a corrupted trial sends the object's own word into the other slot, so **`mix_A` must come back
≈ `w` and `mix_B` ≈ `1 − w`.** That is a dose measurement, not a dose assumption
(`liveness-is-not-dose`), and it is read **before** any expression number. A knob whose realised rate
does not match its nominal rate is a broken knob, and §53's own history —
`control-on-a-different-code-path`, "an arm selector must pin the whole dose vector" — says to check.

#### REFUSAL CLAUSES

1. **`w = 0` fails to reproduce the pinned ceiling** (outside 2 SE of §52's +0.1824 ± 0.0263 and
   §53's +0.1293 ± 0.0478) → **§54 VOID.** The harness differs from the one those sections ran and
   nothing may be quoted.
2. **Realised `mix` departs from nominal `w` by more than 0.03 at any level** → **§54 VOID on the
   knob**, whatever the expression column says.
3. **`w½ ≥ 0.20`, and expression at `w = 0.171` within 2 SE of `0.657 × ceiling`** → **AVERAGING
   HOLDS. Wrong writes are NOT sufficient to explain §53's null**, and the derived index must carry a
   second defect that §53 did not isolate. This is the branch that refutes the lead.
4. **`w½ ≤ 0.05`, and expression at `w = 0.171` within 2 SE of zero** → **OVERWRITE, AND §53's NULL IS
   FULLY EXPLAINED BY THE WRONG-WRITE RATE ALONE.** Then the derived index has exactly one defect, it
   is quantified, and **what to build is a better index rather than a different architecture** — with
   the tolerance curve saying how much better.
5. **Expression is flat in `w`** → the write slot does not matter, which contradicts §49–§53
   wholesale. **Suspect the harness first** and re-run clause 2 before reading anything else.
6. **`w½` between 0.05 and 0.20** → an intermediate store, neither pure accumulator nor pure
   overwrite. Report `w½` with its interval and do not name a mechanism.

#### POWER, STATED HONESTLY, AND IT IS THE WEAK POINT

Using §52's measured spread (SD 0.158 at 900k), an arm at n = 48 has SE 0.023, so the decisive
single-point contrast — `0.657 × ceiling` against zero — runs at **2.6 SE if the ceiling is §53's
+0.129 and 3.7 SE if it is §52's +0.182.** That is adequate but not generous, and **it is why `w½`
across six levels is the primary rather than any one contrast**: the curve uses all 192 creatures
where a single comparison uses 96. A result resting only on the `w = 0.171` point is to be reported
as 2–3 SE and not more.

**Disclosed:** pairing by creature is *not* assumed to help. §52's paired SE for the pinned contrast
(0.0493) was larger than its per-arm SE (0.0328), so these arms are not positively correlated across
seeds; both the paired and unpaired contrasts are printed and the unpaired one is the primary.

### §54 RESULT: AVERAGING HOLDS, THE OVERWRITE LEAD IS REFUTED — AND THE DERIVED INDEX HAS A SECOND DEFECT, NOW QUANTIFIED

`results/n54-writerate-900k.log`, 192 sessions, **40m8s**. Clause 5 fired branch **(3) AVERAGING
HOLDS, AND THE LEAD IS REFUTED** — the branch pre-registered as the one that kills it.

#### THE SELF-TESTS, BOTH BEFORE ANY READING

**Clause 2, the dose.** The realised wrong-write rate matches its nominal value to within **0.0005 to
0.0072** at all six levels. The knob does exactly what it claims.

**Clause 1, the ceiling.** `w = 0` reads **+0.1772 ± 0.0292** against §52's **+0.1824 ± 0.0263** — a
cross-driver reproduction of the pinned ceiling from a third instrument.

**And bit-identity, proved not asserted:** re-running `slotdose` at 450k on the §54 binary reproduces
`results/n53b-450k.log` with **127 of 127 content lines identical** (`results/n54-bitid-450k.log`).
`pin_mode = 3` left §53's arms untouched. *(The first diff printed `*** DIVERGED ***` over a single
blank line — the one preceding the `real/user/sys` block my filter stripped from one side only. The
same capture artefact as §51's. The check is the content lines, not the raw diff.)*

#### THE TOLERANCE CURVE

| `w` | expressed slot term | averaging wants `(1−2w)·ceiling` |
|---|---|---|
| 0.000 | **+0.1772 ± 0.0292** (6.1 SE) | +0.1772 |
| 0.020 | +0.1101 ± 0.0337 | +0.1701 |
| 0.050 | +0.1309 ± 0.0226 | +0.1595 |
| 0.085 | +0.1049 ± 0.0321 | +0.1471 |
| **0.171** | **+0.1046 ± 0.0241** (4.3 SE) | **+0.1166** |
| 0.340 | +0.0695 ± 0.0408 | +0.0567 |

> **`w½ ≈ 0.248` against averaging's parameter-free 0.250.** And at the derived index's *measured*
> rate, expression is **4.3 SE above zero** and **0.5 SE from the averaging line**. Overwrite wanted
> ~0.00 and is inconsistent.

**`w½`'s precision should not be oversold.** The interior points are separated by less than their
SEs and the curve is not monotone (+0.1101 at w = 0.020 sits below +0.1309 at w = 0.050), so `w½` is
bracketed only between 0.171 and 0.340. **The load-bearing number is the operating point, which needs
no interpolation**: 4.3 SE above zero.

**So `bias_ctx_` integrates conflicting trial-length lessons like an accumulator, not like an
overwriting store.** And **§54 does not contradict `retention`**, which is worth stating precisely
because it could easily be read that way: `retain` measured a **sequential** conflict — teach one
sound to criterion, then teach a conflicting one, and the first is erased (0.22 mean, 0.03 and 0.06 on
two seeds). §54 measured **interleaved** conflicts, alternating within a single session. **A clamped
accumulator does both without contradiction**: interleaved opposite increments average out, while a
long second block walks the same store to the opposite clamp. The two results bound the same mechanism
at two timescales, and only the interleaved one bears on §53's null.

**Wrong writes at the derived index's own rate are NOT sufficient to explain §53's null.**

#### THE SECOND DEFECT, AND IT IS THE WHOLE EFFECT

Same budget, same ceiling, same teacher, same wrong-write rate — one with a perfect index corrupted to
`w = 0.171`, the other the retina's own index measured at `w = 0.171`:

| | expressed |
|---|---|
| perfect index, corrupted to `w = 0.171` | **+0.1046 ± 0.0241** |
| §53's derived index, raw | +0.0081 ± 0.0170 |
| §53's derived index, adjusted | −0.0053 ± 0.0135 |

> **The deficit is +0.0965 ± 0.0295 (3.3 SE) against the raw figure and +0.1099 ± 0.0276 (4.0 SE)
> against the adjusted one.** The derived index performs significantly *worse* than its own
> wrong-write rate accounts for. §53 said its route expresses nothing; §54 says that is **not**
> because of how often it writes to the wrong slot.

#### AND A HOLE IN MY OWN PRE-REGISTRATION, WHICH IS WHERE THE NEXT MEASUREMENT COMES FROM

I justified corrupting **per trial** rather than per interval by citing §53's `present 1.0000` as
evidence the derived index is "stable within a trial". **`ctx_present_frac` measures LIVENESS — is any
slice driven this tick — and says nothing about whether the slot is the SAME one tick to tick.** That
is `liveness-is-not-dose` applied to the wrong quantity, in my own pre-registration, after writing
that rule into memory.

It matters precisely here, because **run length is the one dimension the two models differ on** — I
wrote that sentence in the pre-registration and then chose the run length from a number that does not
measure it:

* **trial-length runs** (~90 consecutive wrong lessons, what §54 imposed) → averaging-like, **which is
  what §54 measured**;
* **interval-length runs** (isolated wrong lessons) → a different regime entirely, and untested.

`mix_A = 0.1538` is computed over *lessons*, so a within-trial flicker and per-trial blocks produce
**identical mixture columns with completely different run lengths.** The derived index's switch rate
within a window has never been measured.

**So §54's scope is exact: wrong writes delivered in trial-length blocks, at the derived rate, do not
explain §53's null. Wrong writes delivered in short runs remain open**, and they are the leading
candidate for the 3.3 SE deficit above.

**The measurement is nearly free and the data is already collected.** `ctx_hist` is accumulated per
window and both `ctx_slot` and `teach_slot` take only its **argmax** — the histogram's *concentration*
has been discarded every time. The modal slot's share of live ticks per window is a direct
within-window stability measure, available from the session as it stands. **§55 reads it before
imposing anything.**

### §54's VERIFY GATE, ALL FOUR KEYS — CHECKED THE NEW WAY

`results/n54-verify.log`, exit 0: `verify PASS`, `hash ad96f882becbee92 as recorded`, `pinned hash
PASS`, `experiments 24 as expected, 0 not`. The count stays **24** because `writerate` is
`Tier::kLong` / `Expect::kRead` and `verify` runs the fast tier only — the same reason `slotdose`,
`wordruler`, `objword`, `objname` and `visprobe` never appeared in it.

**And this is the first gate run with §53's correction**: each key was counted with `grep -c` against
an expected value of **one** rather than tested with `grep -q`, because the defect this guards against
is two interleaved runs *duplicating* every marker, not a marker going missing. `verify PASS` 1,
`pinned hash PASS` 1, `as expected, 0 not` 1.

The pinned hash is unmoved because §54's only shared-code change is one extra `else if` arm on the
experiment-only context pin, plus a new RNG stream drawn from **only** when `pin_mode == 3` — and that
was verified by re-running §53 and diffing, not by reasoning about it.

## §55 (2026-11-08) — HOW LONG IS A RUN? THE PRE-REGISTRATION

§54 closed pure overwrite and left one candidate standing for the derived index's **3.3–4.0 SE
deficit**: wrong writes delivered in **short runs** rather than the trial-length blocks §54 imposed.
This section measures the run length before imposing anything, because the data is already collected
and has been thrown away every time since §49.

### THE QUANTITY, AND WHY IT HAS NEVER BEEN READ

`ctx_hist[kMaxContextSlots]` is zeroed per `observe` call and incremented on every scored tick the
index is live. Both consumers — `teach_slot` at the teaching window and `ctx_slot` at the probe —
take only its **argmax**. The histogram's *concentration* is discarded, and with it the only
within-window stability measure the session already produces.

Three columns per window kind, pooled as a **ratio of sums** across windows and creatures (not a mean
of per-window ratios — `mean-of-ratios`):

* **`runlen` = Σlive / (Σswitches + Nwindows)**, in ticks. **THE PRIMARY.** A switch is a live tick
  whose slot differs from the previous live tick's, so a dark gap that returns to the same slot is not
  a switch. Per creature this is a ratio of sums over that creature's windows; **across creatures the
  verdict quantity is the mean of those per-creature ratios with its SE**, because a CI needs an error
  bar and a figure pooled over every window of every creature has none. Both are printed — if they
  disagree that is creature heterogeneity, which is worth seeing rather than hiding.
* `pure1` = fraction of windows with **zero** switches. Secondary, reported, not verdict-bearing —
  it is the directly interpretable companion ("§54's manipulation was faithful for this fraction of
  writes").
* `purity` = Σmodal / Σlive, the modal slot's share. Secondary. Note it **cannot** separate a fast
  flicker from two long blocks: both read 0.5. That is why `runlen` is primary and not this.

Both window kinds are scored over exactly **900 ticks** — teaching is `kM3TrialTicks − kM3SettleTicks`
= 1400 − 500 and the probe is `kM3ProbeTicks − kM3ProbeSettleTicks` = 2000 − 1100. One ceiling, 900,
for both. **The estimator is bounded above by 900 by construction and the log prints that bound beside
every reading.**

### THE DECISION BOUNDARIES, IN CLAIM UNITS, BEFORE ANY NUMBER EXISTS

§54 imposed a run length of **900 ticks — one whole teaching window, by construction**: the pin is set
once per `observe` call, above the tick loop. So the question is whether the derived index does the
same.

The boundaries are the protocol's own timescales, not round numbers. `kFeedbackPeriodTicks = 150` is
the interval over which a reward delivery integrates, and `plasticity_interval_ticks = 10` is how
often the write lands. A run much longer than 150 means every reward delivery sees one stable slot; a
run at or below 150 means reward and slot **decouple inside a single window**, which is a credit
misattribution and a different mechanism from a wrong-write *rate*.

Verdict on the **95% CI** of `runlen` across creatures on the teaching windows (mean ± 1.96 SE), three
ways:

* **(A) lower bound ≥ 450** (half the window; at most one switch per window): the derived index writes
  in trial-length runs, **§54's manipulation was faithful in run length, and short runs are REFUTED as
  the explanation for the deficit.** The log prints `VERDICT A -- REFUTED: runlen 95%% CI [%.1f, %.1f]
  ticks, lower bound >= 450`.
* **(B) upper bound ≤ 150** (at or below the feedback period): **the hypothesis SURVIVES**, §54
  imposed runs at least 6x too long, and §56's manipulation is defined by the measured value. The log
  prints `VERDICT B -- SURVIVES: runlen 95%% CI [%.1f, %.1f] ticks, upper bound <= 150`.
* **(C) anything else** — a CI straddling either boundary, a calibration arm off its derived
  prediction by more than 15%, or an exact self-test failing: **NO VERDICT**, and the log names which
  of the three fired. A CI that straddles is case C and gets said, not rounded to whichever side is
  closer.

`pure1` and `purity` are reported in every arm and bear on no verdict. Pre-registering one primary is
the point.

### THE SELF-TESTS, AND THEY CAN ALL FAIL

**Two are exact.** `PINNED` (pin_mode 1) and `W171` (pin_mode 3 at the derived rate) both set the pin
once per call, so both must read **switches exactly 0, `pure1` exactly 1.0000, `purity` exactly
1.0000, `runlen` exactly 900.0**. Anything else means the counter does not count what this section
says it counts. `W171`'s reading is also the evidence for §54's scope claim — that §54 imposed
trial-length runs — which until now has been an argument from reading the code rather than a
measurement.

**Three are calibrations with derived predictions**, and this is the part that makes the instrument's
units falsifiable rather than asserted (`calibrate-the-estimator-in-claim-units`). `pin_mode = 4`
re-draws the pinned slot from two slots every `flicker` ticks, counted from `t = 0` of the call. Over a
scored window `[s, e)` the number of internal block boundaries is `floor((e-1)/f) - floor(s/f)`, each
changes the slot with probability 1/2, so `E[nruns] = boundaries/2 + 1` and

**`runlen_expected = 900 / (boundaries/2 + 1)`**

which for `f` = 10, 50, 150 is **19.78, 94.74 and 225.00 ticks** — identical on the teaching and probe
windows, since both hold 89, 17 and 6 boundaries respectively. The code computes these from the
protocol constants rather than carrying them as literals (`derive-the-constant`).

Two things this discriminates, which is why it is worth running:

* An estimator that counted **redraws** instead of **changes** would read 10, 50 and 150 — a clean 2x
  miss at every point. The factor of two is the whole content of "a fair coin over two slots leaves the
  slot alone half the time", and `fifty-fifty-does-not-pull-to-the-midpoint` is the note that says
  derive that rather than assume it.
* An estimator that mishandled the window edge would miss at `f = 150` (6 boundaries, so the
  truncation term is 4/3) and hit at `f = 10` (89 boundaries, truncation 1.011). The three points are
  spread deliberately so one bad edge case cannot pass.

### THE LIVENESS GATE RUNS FIRST

§53's T-A **passed on an empty set**: at 60k ticks no creature scored a probe, every column read
0.0000 and the identity self-test printed PASS twice because the identity is trivially zero when no
cell was filled. So before any test here: if an arm's `Nwindows` is zero, or the index never went live
(`ctx_slots == 0`, the missing-source-module case `areax` and `ctxself` check rather than trust), the
log prints **VOID** for that arm and no test reads it. A test that cannot fail is not a test
(`vacuity-gate-cannot-guard-a-rejection`).

### WHAT IS DELIBERATELY NOT IN HERE

**The auditory index as a natural positive control** (`context_source = 4`, which `ctxsrc` measured at
1.000 while the word plays and 0.533 by the reward window, so it should switch a great deal). It needs
a different genome, and `ctx-genome-somato` records that context genomes fail `calibrate` — a failure
mode that would arrive mixed in with the result. The flicker arms give a positive control with a
*derived* prediction instead of a plausible one, which is strictly stronger, so the natural arm buys
nothing it does not already have.

### BIT-IDENTITY, PROVED NOT ASSERTED

Every change is additive and gated: a new `flicker` argument defaulting to 0, a new `pin_mode == 4`
arm, a new RNG stream seeded unconditionally but **drawn from only inside that arm**, and accumulation
into new struct fields. So §53's `slotdose` at 450k must reproduce its archived log on the §55 binary,
**compared on content lines** — the raw diff has twice now printed DIVERGED over one blank line before
the stripped timing block. That re-run is part of this section, not an afterthought.

**Arms, 84 sessions, every argument held at §52/§53/§54's values except `pin_mode` and `flicker`:**

| arm | `pin_mode` | `flicker` | reps | role |
|---|---|---|---|---|
| `DERIVED` | 0 | — | 24 | **THE MEASUREMENT** — the retina's own index |
| `PINNED` | 1 | — | 12 | exact self-test: 0 switches, `runlen` 900.0 |
| `W171` | 3, `w` = 0.1712 | — | 12 | **§54's OWN ARM** — the evidence for §54's scope claim |
| `FLICK.150` | 4 | 150 | 12 | calibration, predicted **225.00** |
| `FLICK.50` | 4 | 50 | 12 | calibration, predicted **94.74** |
| `FLICK.10` | 4 | 10 | 12 | calibration, predicted **19.78** — the short-run end, where the open hypothesis lives |

Run at **450k and 900k**, as §53 did, because a drift in the index's own stability as it trains is
worth seeing and costs one more launch.

### §55's RESULT — NO VERDICT, AND THE PRE-REGISTERED PRIMARY WAS THE WRONG ESTIMATOR

`results/n55-switchrate-900k.log`, 84 sessions, exit 0. **The pre-registered test returns VERDICT C at
both budgets**, and it returns it for a reason worth more than either of the verdicts it was designed
to pick between.

| | 150k (smoke) | 450k | 900k |
|---|---|---|---|
| `runlen` across creatures | 668.38 ± 29.86 | 417.34 ± 19.58 | **455.81 ± 14.43** |
| 95% CI | [609.9, 726.9] | [379.0, 455.7] | **[427.5, 484.1]** |
| verdict | *would have been A* | **C** | **C** |
| `pure1`, teaching | 0.9583 | 0.7805 | 0.7432 |
| switches per window | 0.413 | 1.281 | 1.025 |

The CI straddles the 450-tick boundary at both registered budgets, so the log says C and names the
straddle rather than rounding to the nearer side.

#### THE INSTRUMENT IS SOUND, WHICH IS WHAT MAKES THE C INFORMATIVE

Everything that could have failed, did not:

* **The two EXACT self-tests pass at both budgets.** `PINNED` and `W171` read **0 switches, `pure1`
  1.0000, `runlen` 900.0000** on teaching and probe windows alike. So **§54's scope claim is now
  measured rather than argued: §54 imposed its wrong writes in runs of 900 ticks, one whole window.**
* **The three CALIBRATIONS land within 0.4% at 450k and 0.3% at 900k**, against predictions of 19.78,
  94.74 and 225.00 derived from the protocol constants. An estimator counting *redraws* rather than
  *changes* would have read 10, 50 and 150 — a clean 2x miss at every point, and it did not.
* **`pure1` calibrates too**, on a prediction it was not gated on: `0.5^6 = 0.0156` expected for
  `f = 150`, **0.0163 measured** at 900k. And `sw/sww` reads 3.04 against `3/(1 - 1/64) = 3.048`.
* **Bit-identity proved:** §53's `slotdose` at 450k on the §55 binary reproduces its archived log,
  **127 of 127 content lines**. (The first diff printed DIVERGED because I compared against
  `n53-slotdose-450k.log`, which *predates* the §53b block — my archive-selection error, not a
  divergence. The correct baseline is `n53b-450k.log`, and the older archive's first 90 content lines
  match too.)

#### THE SUBSTANTIVE ANSWER IS A MIXTURE, AND A MEAN CANNOT CARRY IT

At 900k, on the teaching windows:

* **74.3% of windows are perfectly single-slot** — 900-tick runs, exactly what §54 imposed.
* **25.7% switch at least once.** They hold **25.7% of all live teaching ticks**, carry **3.99
  switches each**, and the run length *inside them* is **180.26 ticks** — just above
  `kFeedbackPeriodTicks = 150`, which is the interval a reward delivery integrates over.

So neither pre-registered boundary describes the index. It is not "at most one switch per window"
(1.025) and it is not "runs at or below the feedback period" (456). **It is two populations: three
quarters of writes delivered in trial-length blocks and a quarter delivered in ~180-tick runs across
which reward and slot decouple inside a single window.**

**And this is a defect in my pre-registration, not in the creature.** A mean tested against a
threshold cannot represent a mixture, and the diagnostic is that *more power would make it worse*:
the mean sits essentially ON the 450 boundary, so shrinking the SE from 14.4 to 3 would deliver
**VERDICT A — "the derived index writes in trial-length runs"** for a quantity where a quarter of
writes demonstrably do not. A better-powered run would have produced a cleaner and more misleading
answer. The right primary was the **fraction of writes delivered in short runs** — `live%`, which
reads **0.2568** directly — and I would have pre-registered that had I known the shape. **I am not
switching to it now.** The pre-registered verdict stands at C and the fraction is reported as the
descriptive answer, because `verdict-fitted-to-data` is about exactly this temptation.

#### THE SMOKE WOULD HAVE DELIVERED A FALSE VERDICT

The 150k smoke read **668.38 ± 29.86 and VERDICT A — REFUTED**. The registered budgets read 417 and
456. The 150k-to-450k drop is **251 ± 35.7, seven SE**, and it is not noise: `pure1` falls 0.9583 →
0.7805 → 0.7432, so the index **destabilises substantially over the first 450k ticks** and is flat
between 450k and 900k (417.3 ± 19.6 against 455.8 ± 14.4, and those two runs share seeds so even that
contrast's SE is not the right one).

`verify-command` already records that **a smoke is never good for sign**. §55 extends it: the smoke
was *perfectly* good for the instrument — every self-test and all three calibrations landed at 150k
exactly as they did at 900k — and worthless for the measurement, because at 150k the minority
population was only 4.2% of windows instead of 25.7%. **A smoke validates the instrument and measures
nothing.** I stated the smoke's verdict as the result before the budgets returned, which was wrong.

#### WHAT THIS DOES TO THE DEFICIT, BY ARITHMETIC AND NOT BY MEASUREMENT

Granting the hypothesis the most generous possible reading — that every write in a switching window
loses its credit assignment entirely — scales §54's measured expression by 0.743:

* §54 at `w` = 0.171 with 900-tick runs: **+0.1046 ± 0.0241** (0.59 of a +0.1772 ceiling)
* scaled by the 74.3% of writes that are trial-length: **+0.0777 ± 0.0179**
* §53's derived index, raw: **+0.0081 ± 0.0170**
* **remaining deficit +0.0696 ± 0.0247, 2.8 SE**

So short runs on a quarter of the writes can buy **at most about a quarter** of the 3.3 SE deficit,
and roughly 2.8 SE of it survives the most generous accounting available. **This is arithmetic on two
measured arms, not a measurement**, and the generous assumption is doing the work — the honest
statement is that short runs are now a *partial, bounded* contributor rather than the explanation.

#### THE READ IS STABLE AND THE WRITE IS NOT

The probe window is far steadier than the teaching window in the same sessions: **`runlen` 690.38 ±
34.24 against 455.81 ± 14.43, `pure1` 0.9572 against 0.7432, 0.411 switches per window against
1.025.** That asymmetry matters for how §53 should be read — **§53's 100% registration was measured on
the probe, the stable window, while the lesson is written in the unstable one.**

Three things differ between those windows and this run cannot separate them: position in the trial
(1100–2000 of 2000 versus 500–1400 of 1400), the caregiver speaking versus silence, and reward and
plasticity running versus not. Named as confounds rather than resolved.

#### WHAT §56 IS, AND WHY IT IS BETTER MOTIVATED NOW

**These arms run with `hold_nuisance = true`, so the scene rendered for a given object is identical on
every trial** — and that was checked rather than assumed. `m3_toy_fixed` takes **no RNG argument at
all** (fixed centre 0.5/0.5, fixed half-width 0.105, shapes area-matched by `kAreaMatch` so brightness
cannot give the answer away), and the session **never calls `look_at` and never drives
`gaze_command`**, so the fovea does not move either. The retinal feature vector presented for a given
object is therefore identical on every trial of the session.

And the index still switches within a quarter of windows, and still writes to the wrong slot 17% of
the time. **It cannot be failing on appearance ambiguity, because there is none** — so the partition
is moving for internal reasons: prototype adaptation, or network state, or both.

That makes the next question sharp and cheap. `teach_label` and `teach_slot` are already returned per
window, so the **modal assignment's drift across the session** is measurable without new machinery: cut
the session into blocks, score the slot-to-object assignment per block, and ask whether it flips.
A session-scale flip is the one timescale neither §54 nor §55 has touched, and it is precisely where
`retention` measured **erasure** (0.22) rather than averaging — so if the derived index's errors arrive
in session-scale blocks, `retention` and §54 bound one mechanism at two timescales and the deficit
lands at the erasing end. **§55 does not test that and makes no claim about it.**

### §55's VERIFY GATE — AND THE KEYS ARE NOW A SCRIPT, BECAUSE I GOT ONE WRONG AGAIN

`results/n55-verify.log`, exit 0: `verify PASS`, `pinned hash PASS`, `experiments 24 as expected, 0
not`, and the determinism hash **`ad96f882becbee92` as recorded**, with `final hash A == final hash B`.
The count stays 24 because `switchrate` is `Tier::kLong` / `Expect::kRead` and `verify` runs the fast
tier only.

The pinned hash is unmoved because every §55 change is additive and gated — a `flicker` argument
defaulting to 0, a new `pin_mode == 4` arm, a new RNG stream drawn from only inside it, and
accumulation into new struct fields — and that was **proved** by re-running §53's `slotdose` at 450k
and diffing, 127 of 127 content lines, not by reasoning about it.

**But my hash key reported a passing line as missing.** I wrote `grep -oE "hash [0-9a-f]{16} as
recorded"` and the log reads `hash              ad96f882becbee92  as recorded` — several spaces, not
one — so the pattern matched nothing and the gate printed three keys instead of four.
`smoke-the-exact-command` already records this exact defect and its exact fix (*"use `\s+` for column
padding, and write gate keys against a pasted line from a real log"*). **Recording it was not enough:
this is the second occurrence, in the session where that note is in the index.**

A rule that has now failed twice at the same spot is not a knowledge problem, so the fix is mechanical
rather than another note. **`tools/verify_gate.sh` holds all four keys, typed once**, with the two
preliminary assertions that must come first — a negative exact-match result is the absence of
evidence, not evidence of failure, and a usage error, a crash, an empty file and a real determinism
regression are indistinguishable without them.

**And it was exercised on all four branches before being committed**, because
`deferral-note-is-untested-code` says a gate's command is untested code until it has run and *real rows
cannot test a branch they never take*:

| input | result |
|---|---|
| the real `n55-verify.log` | **PASS**, exit 0 |
| the log concatenated with itself — §51's interleaving defect | **FAILED**, exit 1: all three counts read 2, and `final hash A == B` reports four hashes |
| the real log with a wrong expected hash | **FAILED**, exit 1, and it *prints the hashes it actually found* |
| a one-line file containing `error: unknown argument` | **FAILED**, exit 1 on every check including both preliminaries |

The second row is the one that matters: it is the defect the `wc -l` key was invented for and could
never catch, since a clean log and a doubled log were both 936 lines. A count of exactly one catches
it, and now catches it without anyone retyping the pattern.
