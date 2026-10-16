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

### 0. Does the specification hold end to end? — MET, over two generations

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

**The next run is depth, and it is cheap:** raise the chain from 3 generations to 5 on the
conscience genome and ask whether the four-signal code holds where the two-signal one did.
The experiment, the genome and the analysis all exist; nothing needs building.

## What is settled, and therefore not to re-open

| line | state |
|---|---|
| Credit assignment via reward masks | **Closed by arithmetic.** Gain rises monotonically with how much of the motor group the reward reaches, so the cheapest mask is no mask. Nothing in the family is affordable against its retention price. |
| The geometry of the motor pool | **Settled in two independent families.** The readout is a rate-weighted centroid, so potency is *signed* distance from it; potency decides *whether* extent costs anything, and given nonzero potency extent costs until it saturates. |
| The derived context index | **Explained, and the repair is closed.** Its deficit was the word boundary: given a clean one, classification error goes to exactly zero and the creature's own index matches a host-supplied perfect index on behaviour (−1.0 ± 6.0 Hz). But four detectors built to supply that boundary are all beaten by the arm with no detector at all, and the leading arms sit at their occupancy ceilings — so **build no further boundary detector**. The gap itself was real (oracle − detector-free = +17.2 ± 5.1, 3.3 SE) and is **now mostly claimed** — not by a detector and not by balancing the table, both of which are closed, but by DNA v70's conscience gain, settled at a pre-committed n = 144: +15.22 ± 3.56 (4.3 SE) with 85% of it in the voice, taking the gap from +19.16 ± 3.18 to +6.15 ± 2.78 (§2). |
| The aim/feedback line | **Refused by its own control** — a blind controller beats feedback at this creature's loop delay. |
| Protocol variations | **Closed.** Longer, blocked, region-shaped, distance-aware and staircased rewards were each measured and none helps. |
| The transmission channel | **Faithful once width is held** — no detectable loss over four generations (1.4 SE), and a ratchet that locks in whichever convention it finds. Further chain runs measure the tutor, not the channel, so the quantity to move is production width (§1). |

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

## Standing prohibitions

These are places where fitting has already cost this project a result, and they are written
down so the next person does not pay again.

- **Do not fit the magnitude of the lever.** The direction is established on three targets;
  a distance law was fitted to two points, matched them to 0.1%, and died on the third.
- **Do not fit the extent curve.** Three points, a one-parameter family, and the exponent is
  not constant across them.
- **Do not price the matched cluster prior.** It was argued from the index's own *output*
  occupancy, where the result it rests on is about the *data's* class distribution — and the
  words here are exactly balanced, so a uniform prior is the correct one.
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

## How to not waste a run

Collected because each line cost one.

1. **Grep the logs for the condition, not the name of the experiment.** Three times the
   answer was already on disk. A question written down as a *lead* reads as not-yet-done.
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
