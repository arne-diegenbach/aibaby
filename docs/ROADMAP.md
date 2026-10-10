# Where this project stands, and what is prepared next

Written 2026-10-09, revised 2026-10-10. The *record* is the root `README.md`, which is chronological and long;
the *curated findings* are `docs/discoveries.pdf`. This file is the short forward-looking
one, and it exists because the working lead queue has always lived in a memory store
**outside the repository** — so a reader of the repo could see everything that had been
done and nothing that was planned.

Every number below is from a run gated on `--experiment verify` reporting `PASS` with the
pinned hash `ad96f882becbee92`. Sample sizes are creatures, not trials.

## The one-paragraph state

The creature hears, babbles, hears itself, is taught a vowel by praise alone, repeats a
word, and answers. It **names**: four words in two formant dimensions, and nine words name
as well as six. It does say two things in a row,
unprompted: a population half-centre gives it a free-running two-phase vocal gesture that
reaches the formant. What it cannot do is be *taught* two — a phase-targeted lesson produces
no learning at any credit window or reward delay, while a fixed-target lesson on the same
creatures reaches 18 SE, and turning the gesture on costs four fifths of even that.
**It composes in space and not in time, and the generator and the lesson are in tension at
the only operating point where both are known to run.** That is the single blocker between
where it is and speech.

## What is settled, and therefore not to re-open

| line | state |
|---|---|
| Credit assignment via reward masks | **Closed by arithmetic.** Gain rises monotonically with how much of the motor group the reward reaches, so the cheapest mask is no mask. Nothing in the family is affordable against its retention price. |
| The geometry of the motor pool | **Settled in two independent families.** The readout is a rate-weighted centroid, so potency is *signed* distance from it; potency decides *whether* extent costs anything, and given nonzero potency extent costs until it saturates. |
| The derived context index | **Explained.** Its deficit was the word boundary. Given a clean one, classification error goes to exactly zero and the creature's own index matches a host-supplied perfect index on behaviour (−1.0 ± 6.0 Hz). |
| The aim/feedback line | **Refused by its own control** — a blind controller beats feedback at this creature's loop delay. |
| Protocol variations | **Closed.** Longer, blocked, region-shaped, distance-aware and staircased rewards were each measured and none helps. |

## What is open, in the order the measurements allow

### 1. A word-boundary detector that is not an oracle — IN FLIGHT

The prize is measured rather than argued: a clean boundary takes word-conditional formant
delivery from **+12.0 to +53.4 Hz** (9.6 SE) and closes the entire deficit to a perfect
index. So the last oracle in the context line is the word's *timing*, which is far cheaper
to require than its *identity*.

Three instrument passes refused every timing repair of the shipped larynx gate: episode
lengths are smooth (a fifth of episodes under two ticks, a fifth over 512, no gap between),
the off-gaps are smooth too, and it recovers only **0.68** of the caregiver's word — so it
misses a third of what it should see and no logic placed on top can recover that.

**Running now:** an envelope-onset gate after Nabé, Schwartz & Diard's COSMO-Onset — a
sustained *rise* in the auditory envelope rather than a level anywhere. No constant is
guessed: the derivative is the crossing of two filters the kernel already keeps, so the
threshold is zero; the persistence is the fast filter's own time constant in ticks.
**Bars, pre-registered:** episodes per trial must fall from ~2.3 toward 1.0 *and* coverage
rise from 0.68 toward 1.0 — a mechanism check independent of the outcome — and then the
conditional effect must move toward +53.4 Hz. Refused if segmentation reaches the oracle's
and delivery does not move.

### 2. Myelination gated to sleep — prepared, not built

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

### 3. Two gestures in a row — the blocker, and unbuilt

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

**So the next run is the composition question, not another mechanism.** The oscillating
regime is a bounded window in drive × gain (Shpiro & Rinzel) and the point inside it was
picked for swing alone. Sweep `halfcenter_gain` across the window, three or four points, and
measure *both* on every point — `sep` and `share` for the gesture, error-early-minus-late on
a fixed target for the lesson. The account to test is that mutual inhibition makes the
alternation a strong attractor that reward-driven weight changes cannot perturb, so weaker
inhibition should trade F1 swing for lesson purchase. **The bar, fixed in advance:** some
point must give an error drop over half of 0.2197 while keeping `sep` > 10 Hz and `share`
within 0.15–0.85. If none does, the gesture and the lesson do not compose at any operating
point — a real negative about this architecture rather than one more refused mechanism.

Two things are owed alongside it. The 4/5 cost names the *configuration*, not the mechanism:
four genome fields differ and two are spike-frequency adaptation, so an adaptation-only and
an inhibition-only genome are needed to attribute it. And one 2.2 SE lead is worth powering
up — narrowing the credit window twentyfold *improves* the fixed-target lesson under the
oscillator (+0.0256 ± 0.0117) while delivering strictly less total credit, which is what a
trace that stops crediting several phases at once should do.

Bohland, Bullock & Guenther's GODIVA names the layer above all of this, and Segawa and
colleagues say the stored unit is a *gestural score* rather than an acoustic target — which
is the shape the dormant `dictionary_*` genome block was cut for. Both stay on the shelf
until the composition question has an answer, because a score is a sequence of targets and
this creature cannot yet be taught two.

### 4. Carrier residuals, demoted and worth stating

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
