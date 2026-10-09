# Where this project stands, and what is prepared next

Written 2026-10-09. The *record* is the root `README.md`, which is chronological and long;
the *curated findings* are `docs/discoveries.pdf`. This file is the short forward-looking
one, and it exists because the working lead queue has always lived in a memory store
**outside the repository** — so a reader of the repo could see everything that had been
done and nothing that was planned.

Every number below is from a run gated on `--experiment verify` reporting `PASS` with the
pinned hash `ad96f882becbee92`. Sample sizes are creatures, not trials.

## The one-paragraph state

The creature hears, babbles, hears itself, is taught a vowel by praise alone, repeats a
word, and answers. It **names**: four words in two formant dimensions, and nine words name
as well as six. What it cannot do is say two things in a row — an utterance is a held vowel,
and no module holds a kick for 10 ms. **It composes in space and not in time**, and that is
the single blocker between where it is and speech.

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

Everything above is about a *carrier*. Intelligent speech needs structure in time, and the
project's own measurement is that there is none: autocorrelation of a produced kick falls
0.92 → 0.03 within 10 ms. Until a creature can hold one gesture and then make another, a
dictionary of gestures has nothing to sequence and consolidation has nothing to consolidate.

**The bar is the existing measurement:** hold a kick for 10 ms. Bohland, Bullock & Guenther's
GODIVA names the layer above it, and Segawa and colleagues say the stored unit is a
*gestural score* rather than an acoustic target — which is the shape the dormant
`dictionary_*` genome block was cut for. **Price it with an oracle first**, as the credit
mask, the bias route and the word window were all priced: hand the creature a
host-supplied two-gesture score and measure what it converts into before building anything
that has to produce one.

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
