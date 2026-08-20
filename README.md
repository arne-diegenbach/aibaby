# aibaby

A creature you raise. It is born from a compact DNA file that defines only its
initial brain structure; everything else it becomes comes from what happens to
it. See [requirements.md](requirements.md) for the specification.

There is no pretrained data anywhere in this repository. Nothing is downloaded.

## Build and run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/aibaby                       # then open http://localhost:8080
```

The host serves the panel and the WebSocket on the same port, so there is
nothing to install and nothing else to start.

```
--dna <file.toml>     genome to compile and hatch (default dna/default.toml)
--journal <file>      interaction log (default journal.aibj)
--port <n>            http+ws port (default 8080)
--speed <x>           simulation speed multiplier
--snapshot <file>     the creature's saved state: resumed from if it exists,
                      written back on sleep, on the interval, and on shutdown
--snapshot-every <s>  wall-clock seconds between snapshots (default 300)
--fresh               hatch from birth even though the snapshot file exists
--experiment <name>   run headless and exit
--ticks <n>           experiment length
--wav <prefix>        render the creature's voice to disk (babble, m3)
--save <file>         snapshot the creature an experiment raised (babble, m3)
--verbose             more detail from experiments
```

## Putting the baby down and picking it up again

Raising a creature takes hours of wall clock, so §8's brain snapshot exists to
make that survivable:

```sh
./build/aibaby --snapshot babies/ada.aibs      # first run: hatches, then saves
./build/aibaby --snapshot babies/ada.aibs      # every run after: resumes
```

One flag does both. The file is written when the creature falls asleep, every
five minutes, and on shutdown, always via a temporary and a rename — the moment
a snapshot matters is the moment something went wrong, and a half-written file
would take hours of a life with it. It is about 19 MB for the default genome,
almost all of it the arena.

What comes back is the same creature, not an approximation of it: weights,
eligibility traces, thresholds, the delay line still holding spikes in flight,
drives, the critic's error windows, the noise generator's position in its own
stream, and whatever the transducers were half way through. Verified by
`--experiment snapshot`, below.

Three things are worth knowing:

- **A snapshot only loads against the genome it was grown from.** The saved
  arena was laid out and wired by that genome, and resuming across an edited
  `.toml` is refused by name rather than reinterpreted. It is the same rule the
  recalibration below runs into: a genome edit makes a different creature, and
  nothing measured on the old one carries across. Use `--fresh` or a new path
  to start over.
- **The journal restarts.** G1 is *genome plus journal reproduces the brain*,
  and a resumed session did not start at birth, so the log of what follows has
  to be replayed against the snapshot rather than against a new-born. The host
  says so at startup.
- **The host's DSP is not in the file.** The cochlea's analysis window and the
  retina's frame are the room's state, not the creature's, so they start clean.

## Experiments

In the browser: **enable microphone** grants permission and opens the device —
click it again to release it, which also clears the browser's recording
indicator. With the device on, audio only reaches the baby while **hold to
talk** is held. **enable camera** works the same way and releases the device
when clicked again; there is no push-to-look, because a visual field is simply
there rather than something you do to the baby in bursts. **speaker: on** lets
you hear it. `g` praises, `b` scolds.

## Experiments

The measurable goals in §2 of the requirements cannot be checked by looking at
a canvas, so each has a headless experiment that prints a number and a verdict.

```sh
./build/aibaby --experiment determinism --ticks 20000   # G1
./build/aibaby --experiment audio       --ticks 30000   # sound reaches B2
./build/aibaby --experiment vision      --ticks 30000   # an object reaches B3
./build/aibaby --experiment babble      --ticks 120000  # output has variety
./build/aibaby --experiment calibrate   --ticks 120000  # genome at its operating point
./build/aibaby --experiment sleep       --ticks 1500000 # fatigue cycles, senses shut
./build/aibaby --experiment m2          --ticks 200000  # M2
./build/aibaby --experiment m3          --ticks 200000  # M3/G3
./build/aibaby --experiment m3probe     --ticks 600000  # M3's microscope
./build/aibaby --experiment m3sweep     --ticks 200000  # M3 against praise strength
./build/aibaby --experiment g2          --ticks 200000  # G2
./build/aibaby --experiment g2probe     --ticks 200000  # G2's diagnostic ceiling
./build/aibaby --experiment g3probe     --ticks 600000  # G3's ceiling, and why
./build/aibaby --experiment g4          --ticks 1500000 # M4/G4
./build/aibaby --experiment v1probe     --ticks 120000  # is V1 tuned as its map says
./build/aibaby --experiment pcprobe     --ticks 240000  # what a module knows about the sound
./build/aibaby --experiment audprobe    --ticks 240000  # which word, and how fast each module knows
./build/aibaby --experiment eligprobe   --ticks 300000  # can R-STDP tell the two objects apart
./build/aibaby --experiment dwprobe     --ticks 300000  # what reward actually writes onto a tract
./build/aibaby --experiment snapshot    --ticks 2400000 # §8: resume is exact
```

`v1probe` exits non-zero on the shipped genome by design — it has no visual
cortex to probe. It is there for a genome that turns one on.

`pcprobe` asks whether a prediction is worth anything before anything is built on
it. It scores the curiosity critic's forward model against two baselines — the
per-channel mean and *persistence*, "the next frame looks like this one" — and
then fits an offline ridge ceiling: the best a linear readout of a module's own
firing rates could manage, held out, for the critic's 32 bins, for every central
neuron, for every B2 neuron, and for the previous frame. That separates "the
model is badly fitted" from "the information is not there", which is the
distinction the number it was built for turned on. Three of its rows exist only
to stop it lying — a shuffled-rows control, a positive control asking each module
for the frame it is hearing *now*, and a self-test that fits the target from a
copy of itself and must read ~1.

The host DSP layer has three objects: `Cochlea` (PCM -> normalised log-mel
bands), `Mfcc` (the DCT of those bands), and `Ear`, which owns a Cochlea plus the
creature's own larynx so that self-hearing goes through one shared path. **`Mfcc`
is measurement-only and deliberately does not feed the brain** — a cochlea does
not perform a DCT, and more practically the auditory encoder maps mel channel c
onto a contiguous slice of B2, so cepstral coefficients would destroy tonotopy
while still looking like they worked. It exists because `pcprobe` fits linear
readouts to the mel frame and mel bands are collinear enough to cost the fit's
self-test 3%; against an MFCC target that self-test reads exactly 1.000.

`eligprobe` opens up one tract. It samples the **eligibility trace** on
`central->vocal` at the same phase of every trial and asks a held-out classifier
which object the creature was looking at. That is the quantity R-STDP actually
spends, and the question it settles is not "is there a trace" but "is the trace
*different* for the two objects" — because reward does not create eligibility,
it only cashes it. If the pattern is the same either way, no reward schedule can
make the voice conditional; the rule can scale the tract and nothing else. The
size-matched arcuate is the control: same readout, same feature count, a tract
known to carry a conditional signal.

`dwprobe` is the one that decides whether R-STDP can build a conditional
mapping at all. `eligprobe` reads the eligibility *trace*, one trial at a time,
and that trace is shot-noise limited — about 2.4 coincidences per synapse per
trial — so per-trial discriminability is the wrong bar for a rule that
accumulates over hundreds of trials. This runs blocked teaching sessions, one
object throughout, and compares the weight change against **the same object run
twice on a different placement draw**. That control is the whole design:
correlating cube against ball means nothing read against 1.0, and everything
here turns on reading it against the reproducibility ceiling instead. Three
caregiver arms — praise only, praise and scold balanced, and no reward at all —
because the object-independent part of what gets written has to come from
somewhere, and the third arm is what proves reward is writing it.

`audprobe` is the measurement G3 has always needed and never had. Every other
number this project has about the association module scores *cube versus ball*,
which the creature can answer from vision alone — so none of them says whether
the **word** arrives. This one plays the two words with **nothing in view**, in
shuffled order (never alternating: alternation makes the label equal to the
trial's parity), and asks a held-out classifier which word was said, from each
module's spikes, at three integration windows.

The windows are the point. B2 has the word within 50 ms; central needs more than
a second to reach the same confidence. **The association module is slow, not
deaf** — and reading a single fast-timescale probe as though it meant "deaf" is
a mistake this repo has already made once, which is why the window sweep is
printed rather than a single number.

Each exits non-zero if its criterion is not met, so they work as a check.
The experiments live in four files: `experiments.cpp` is the dispatcher,
`experiments_common.h` the shared scaffolding, `experiments_milestones.cpp` the
numbered goals, `experiments_probes.cpp` the diagnostics that say why a goal
failed.

`tools/` holds two genome-editing helpers that assert their edit matched, and
`tools/README.md` the sweep pattern they belong to — read it before running one,
because three of this project's sweeps have had to be thrown away and repeated.
It also holds the two harnesses for the parts no experiment can reach:
`eye_wire_test.py` drives the eye port over a real socket, and
`eye_panel_test.py` checks the panel in real Chrome, down to the pixels.

`m2` runs five creatures and takes about twenty seconds; `g2` runs nine
creatures against nine yoked controls and takes a couple of minutes; `m3` runs
five creatures against five differently-raised controls and takes about two.
`m3probe` and `m3sweep` are diagnostics rather than criteria and always exit
zero. `g4` needs the long run because nothing structural happens until the
creature has tired itself out, and it raises three creatures — a normal one, a
forced one, and that one's twin — so it takes about four minutes. `snapshot`
runs at any length and is honest about what that length covers: below about
1.2M ticks it says outright that sleep and replay were never reached.

### Is this genome still calibrated?

```sh
./build/aibaby --experiment calibrate --ticks 120000    # ~10 s
```

The genome sits at a hand-measured operating point, and editing any part of it
invalidates measurements elsewhere without producing an error anywhere. The
rules are not hard; remembering to apply all of them, in order, after every
edit is what fails. So they are one command now:

- **free-running rates against genome targets**, measured with `ip_rate` and
  `scaling_rate` at zero — *and growth disabled*, because since DNA v5 a
  creature left alone for two minutes grows, and a rate measured across a
  change of shape is not the genome's operating point;
- **the amplitude floor** against the vocal module's operating point;
- **in-degree caps across the nine seeds** `g2` and `m3` actually sweep, not
  just the default one — an overrun in seed 7 of 9 is four synapses missing
  from one creature of an experiment's nine, and the warning scrolls past
  mid-table.

**Its first run found two modules out of calibration.** `central` free-ran at
8.48 Hz against a target of 8.05, and `expression` at 8.64 against 8.04 — so
intrinsic plasticity had been spending every session hauling both toward a
number the genome asked for and the wiring did not produce. Both targets are
now set to what they measure. This is the most likely explanation for the small
drifts against the numbers recorded above (M2 reading 0.95/0.82 against a
recorded 0.94/0.79, the babble duty cycle 0.83 against 0.81).

**One check reports and does not fail, and the reason is a real finding.** The
amplitude floor sits at 0.45 against an operating point of 0.50 — a margin of
0.05 where 0.10 is wanted. It cannot be fixed by retuning, because rule 4 and
the `babble` ceiling are the same constraint pulling opposite ways:

> Loudness is `mean group rate / rate_norm`, and §3.1 pins that mean rate at
> the module's setpoint — so amplitude is a rescaled constant. Rule 4 wants the
> floor *below* the operating point so vocalisation counts do not drift to
> zero. But a floor below a constant is a floor the creature sits above
> essentially always, and that is the 0.83 duty cycle against babble's 0.85
> ceiling. Raise the floor and the counts collapse; lower it and the drone gets
> worse. **You can have a reliable vocalisation count or a quiet creature, not
> both**, for as long as amplitude does not vary.

A check that can only ever be red teaches you to stop reading the output, so
`calibrate`'s verdict covers what an edit can actually put right and this one
is printed with its explanation instead.

### Why the creature always sounds the same

Two histograms out of `babble --verbose`, over 12000 motor frames:

```
F1 group  0     0     0     0  4298  7702     0     0     0     0
amplitude 3    19   186   881  2323  2975  2587  1862   977   187
```

**The first formant never leaves the middle fifth of its range.** Not once in
twelve thousand frames. That is the acoustic measurement from the recordings
restated from the inside: the baby's vowel colour shifts by about a third of a
standard deviation where the caregiver's two words are separated by hundreds of
hertz, because F1 is pinned near 0.5 and cannot go anywhere else.

The cause is the population code. A group's value is a centroid,
`Σ rate_i × preferred_i / Σ rate_i`, where `preferred_i` is the neuron's index
within the group — **and a centroid over an undifferentiated group is the
centre of that group, whatever the input does.** §3.1 drives every neuron in a
group toward the same rate, and the projection into the module is random, so
nothing makes index correlate with what a neuron responds to.

**Reading it against each neuron's setpoint instead of its absolute rate was
tried and reverted.** It is the same fix that worked for the growth trigger, it
is well motivated, and it changes nothing measurable: echo 0.808 against 0.812,
M2 96%/80% against 95%/82%, and an F1 histogram identical to the one above. It
cannot work, and the reason is worth stating because it rules out a whole family
of attempts — a readout cannot recover structure the group never had. Whatever
weights the centroid is computed from, an input pattern excites a random subset
of an untuned group and the answer averages back to the middle.

That argues for one of two things upstream. **The second is now built** (DNA
v6), and the first is still open:

1. **Topography** — wiring that makes a neuron's index mean something to the
   projection driving it, so the population vector has a map under it. Not
   done. This is what would actually unpin F1.
2. ~~**Closing the loop from the larynx to the ears.**~~ **Done — see below.**

### The creature can hear itself (DNA v6)

Until v6 the only path into `brain.hear()` was the microphone: the synthesised
voice went to the panel and never came back. So babbling had no sensory
consequence at all, and nothing in the system carried a signal an articulatory
map could be learned from. Note what §5.3 says babble *is* — *"motor noise
shaped by the curiosity drive"* — and curiosity is prediction error on the next
mel frame. **The mechanism the requirements name could not operate**, because
the creature's own voice never reached a mel frame.

It does now. `Ear` (host/include/host/audio.h) owns the cochlea plus the path
back from the larynx, and **every caller shares it** — the live loop and all
five experiments that drive the brain's ears. Self-hearing wired separately
into six places would be six chances for a headless result to stop predicting
the creature.

**The expected risk did not happen, and the opposite did.** A larynx → ear →
dorsal tract → larynx loop should howl; the tract is the densest projection in
the genome. Swept against `babble`:

| `self_gain` | duty cycle | vocalisations | auditory rate (target 4.50) |
|---|---|---|---|
| 0.0 — deaf, every measurement before v6 | 0.84 | 537 | 4.55 |
| 0.35 | 0.73 | 448 | 4.65 |
| **0.5 — shipped** | **0.67** | **410** | **4.62** |
| 0.7 | 0.59 | 357 | 4.31 |
| 1.2 | 0.46 | 277 | 6.18 |
| 2.0 | 0.39 | 233 | 3.31 |

**It is negative feedback, and the negative sign comes from homeostasis.** The
creature's own voice drives the auditory module, the dorsal tract carries that
to the larynx, vocal rises above its setpoint, and intrinsic plasticity answers
by raising thresholds. The louder the creature is to itself, the quieter it
becomes — monotonically, across a 6x range of gain.

**That is the drone problem's missing dial.** The duty cycle had sat at 0.83
against `babble`'s 0.85 ceiling with nothing able to move it, which is what
blocked a denser ears→larynx tract and made the amplitude floor unfixable. It
now sits at 0.67. `self_gain = 0.5` is where the headroom is bought without
knocking the auditory module off its calibrated operating point — at 0.6 it
falls to 4.07 Hz and `calibrate` calls it stale.

**What it did not fix.** F1 is still pinned in the middle fifth of its range,
which is expected: self-hearing supplies a learning *signal*, and the centroid
problem is that there is no map for that signal to shape. At the shipped 0.5
the echo reads **0.829** against 0.812 deaf and the milestone 0.537 against a
control of 0.463 — unchanged, i.e. chance. (At 0.35 the same experiment read
0.846 and 4 of 5 beating control, which looked like a small win until the
shipped setting read 0.829 and 3 of 5. Five creatures cannot separate these;
the honest summary is that self-hearing has not yet moved either number.)

### Listening to it

A held-out classifier settles whether two vocalisations differ, and it is
completely mute about what the creature sounds like. `--wav` renders the vocal
tract to disk alongside a run, and `--save` keeps the creature that run raised.

```sh
./build/aibaby --experiment babble --ticks 120000 --wav babble   # 2 minutes of voice, 4 s
./build/aibaby --experiment m3 --ticks 200000 --wav m3 --save trained.aibs
```

`babble` writes one mono take: the creature alone in a quiet room, which is
what that experiment is. `m3` writes four files, because the milestone is a
comparison and a single timeline is a poor way to hear one:

| file | what it holds |
|---|---|
| `m3.wav` | the whole session in stereo — **baby left, caregiver right** |
| `m3.labels.txt` | an Audacity label track: `name cube`, `probe ball`, … |
| `m3.ball.wav`, `m3.cube.wav` | every probe the classifier scored, split by which toy was in view |

The last two are the listening test. The milestone claims a cube and a ball
should make the baby sound different; those two files are the evidence, and at
0.507 they sound the same, which is what the number says. **They share one
gain** — loudness is among the features being classified, so normalising them
separately would erase a real difference and manufacture the impression of
one. Probes the baby slept through are dropped from the audio exactly as they
are dropped from the score, so both cover the same trials.

Only the first of `m3`'s ten sessions is recorded — the first creature raised
with the names attached, the top row of the table. Recording all ten would
write about two gigabytes to answer a question the first one answers.

Three things worth knowing about these files:

- **Recording changes nothing.** The synthesiser has its own filter state and
  draws no random numbers, so a recorded creature is bit-identical to the same
  creature unrecorded — verified by running `m3` both ways and diffing the
  table. It is an observation, not a condition, and G1 would not survive it
  being anything else.
- **It is not what the baby hears itself say.** Nothing rendered here feeds
  back into the brain; the creature has no ear on its own larynx (§5.3).
- **It sounds like the panel rather than identical to it.** The file is
  rendered through `VowelSource`, the same two-formant synthesiser the
  caregiver speaks through, so f0, F1, F2 and loudness are faithful and the
  third formant and the bandwidths the browser's three-resonator voice also
  has are not.

`--save` writes a §8 snapshot of the creature as the run left it, so a baby
raised headless in ninety seconds can be resumed in the browser and talked to:

```sh
./build/aibaby --snapshot trained.aibs        # speaker: on, and it is the same creature
```

That is the way to hear a *taught* creature rather than a fresh one — the
experiment does 96 named presentations in the time it takes to read this
paragraph, and doing that by hand through the microphone would take a quarter
of an hour.

## Where the project stands

| Milestone | State |
|---|---|
| **M0** skeleton | done |
| **M1** closed audio loop | **done — G2 met.** Rewarded vocalisations rise within the session (×1.35) and praise beats its own yoked control in **23 of 27 creatures** across three seed families, and 9 of 9 at 420 s. What closed it was directional exploration (DNA v10), which was not aimed at reward at all |
| **M2** vision | **done** — camera → retina → B3 → B1, discriminates present from absent at 98%, and 86% with firing rate divided out |
| **M3** cross-modal association | **built and measured, G3 not met — and now settled rather than open.** The baby echoes a word it hears (0.75) and the seen object *does* now reach the larynx (0.654, up from 0.515, via `vision→vocal`), yet naming still reads taught−random **−0.014** with 0 of 5 creatures over the 0.75 bar. Delivery is no longer the limit; conditioning is |
| **M4** growth and sleep | **done** — **G4 passes**: the creature never grows while it is still learning, grows only on a detected plateau, and never passes the DNA cap; myelination, pruning and replay all run |
| M5 embedded | not started |

Every number on this page comes from the genome in [dna/default.toml](dna/default.toml)
as it currently stands. **The whole suite above passes except `m3`** — G1,
calibrate, babble, audio, vision, M2, G2, sleep, G4 and snapshot are all green,
and G3 is the one milestone still open. Shipped hash `23c4eb2c7c45d05c`;
`--experiment verify` reads 14 of 14 as expected.

**G3 is open but no longer a live line of work.** The `vision→vocal` tract
below raised delivery to the larynx by 27% and moved the milestone by −0.014,
which is the third independent measurement saying the same thing: the object
arriving is not what G3 was ever waiting on. Both of the creature's learning
rules are ruled out as ways to supply the missing conditionality — node
perturbation writes a per-neuron constant rather than a function of the input,
and reward-modulated STDP's object-specific share of what it writes is about
8%. Clearing 0.75 needs a third learning rule, which is a new architecture and
not a fix.

Two mechanisms carry the change since M4, and neither was aimed at the goal it
hit. **Per-module homeostasis (DNA v9)** came from asking why praise did not
survive its own session, and it stopped the decay. **Directional exploration
(DNA v10)** came from asking why babble never converges, and it is what met G2.
Three other mechanisms were built, measured and either reverted or shipped
inert — a visual cortex, a curvature stage, and a scalar version of that same
exploration — and each of those sections below records what it ruled out.

### G1 — determinism: **passing**

Two brains from the same genome, given the same scripted touches, praise and
sound, agree bit-for-bit at every checkpoint. `-ffast-math` was removed from
the build for this reason and `-ffp-contract=off` added: determinism is the
only real testing lever the project has, and fast-math lets the compiler
reassociate float arithmetic differently at different optimisation levels.

### §8 snapshots — a resumed creature is the same creature: **passing**

A creature saved at tick 1,221,652 and restored into a fresh process runs
**bit-identical to the one that never stopped for 1.2M further ticks** — 2400
checkpoints, no divergence — and the save deliberately lands while it is
**asleep and mid-replay**, which is the state most likely to be dropped: the
replay cursors are the only thing in the creature pointing into an episode it
is half way through re-living, and they exist for a few hundred ticks in every
hundred thousand. The window after the resume contains 13 sleep passes and 112
replays, so consolidation is running on restored state rather than merely being
survived by it.

The restore works by rebuilding the creature from the genome inside the file and
then overwriting the arena, so nothing in the format is a pointer and nothing is
fixed up on load. That is also why the genome check is absolute rather than
advisory: the saved bytes only mean anything against the layout that produced
them.

Two controls make the result more than an assertion:

- **A perturbed twin must diverge.** A second creature restored from the same
  bytes and given one hundredth of a praise it never received diverges at the
  first checkpoint. Without that arm, "the hashes agree" would only establish
  that the hashes are insensitive.
- **Mutation testing.** Deleting the restore of the noise generator, of the
  critic's error windows, or of the auditory encoder each turns the experiment
  red. The third one is the interesting one: it *passed* until the save point
  was moved off a round number. A mel frame arrives every 10 ticks and a camera
  frame every 100, so a creature saved on that boundary has its encoders
  overwritten on the first tick after it resumes, and everything they were
  holding stops mattering. The save is now at `ticks/2 + 7` for that reason.

A second arm covers growth, because a normally raised creature never grows and
would otherwise leave the structural half of the format untested. It lowers the
saturation guard exactly as G4's non-vacuity control does, so what runs is the
shipped growth path: the creature is saved with **32 grown neurons over 5 growth
events** and comes back identical. Both arms are one `--experiment snapshot
--ticks 2400000`, about seven minutes.

### The audio loop: **working**

Microphone → 512-sample Hann windows at 50% overlap → FFT → 24-channel mel
filterbank → log compression → intensity-population-coded into B2 → B1 → B5 →
nine population-coded motor groups → eight vocal-tract parameters at 100 Hz →
formant synthesiser in the browser. A vowel raises the auditory module from
2.0 Hz to 7.7 Hz and it settles back afterwards; silence reads as silence.

### M2 — vision: **done**

Camera → 64×64 grayscale → foveated difference-of-Gaussians sampling → 88
centre-surround cells, ON and OFF → latency-coded into B3 → B1.

The retina is foveated because the data budget says so: a uniform 64×64 grid
would be 4096 numbers per frame for a creature whose whole education is a few
thousand interactions. Full acuity across the middle 16×16, halving with each
ring outward, describes the field in 176 numbers.

Where the cochlea spends population on intensity — a louder band recruits more
of its neuron group — the retina spends **time**. Each cell fires once per
frame, early if it responded strongly and not at all below the contrast floor,
so a frame arrives as a volley whose *shape* is the image rather than as an
amount of activity. That matters because "object present" must not be a synonym
for "B3 is busier", which any bright wall would also produce.

Scored the way G3 will be scored, with a held-out classifier:

| | held-out accuracy |
|---|---|
| B3 vision (the input module — plumbing, not the claim) | 0.98 |
| **B1 association — the milestone** | **0.94** |
| B1 with each trial's overall firing rate divided out | 0.79 |
| B1 with the labels shuffled — the control | 0.46 |

Trials are balanced, the object moves and changes size and shape between them
so a readout cannot pass by memorising one picture, and the split is by time
rather than at random: the classifier is fitted on the first half of the
session and tested on a creature that has since kept learning. Stable across
developmental seeds — five creatures span 0.88 to 0.96, and all five clear the
0.75 bar.

The third row is the one that matters, and it is where most of the work since
has gone. It asks how much B1 knows about the object once you stop letting it
answer "there is a lot going on" — and it has moved twice: **0.60** originally,
**0.70** when M3 widened vision→central from 0.04 to 0.06, and **0.79** once
central's membrane constant was cut from 20 ms to 5 ms so it reads the retina's
volleys as coincidences rather than integrating them into a sum (that finding
is below, and it came out of M3's diagnostics). Two thirds of what B1 holds
about the object is now *where* rather than *how hard it is working*.

Two findings from getting there, both of which cost real time:

**A sensory projection can be too strong — but "too strong" depends on the
question.** Raising vision→central from 0.02 to 0.10 made present-versus-absent
steadily *worse*: B1 reads "busier" instead of "different" and the pattern
score collapses. Scored on *shape* instead, the same sweep runs the other way —
0.52 at 0.02, 0.73 at 0.04, 0.80 at 0.06, 0.85 at 0.10 — because a busyness
answer is exactly what present-versus-absent rewards and shape cannot use. The
genome sits at 0.06, where both are still healthy; it was 0.04 for M2 alone.

**That sweep was wrong the first time, in a way worth writing down.** Changing
a projection into a module changes that module's free-running rate, and if
`target_rate_hz` is not re-measured to match, intrinsic plasticity spends the
whole session hauling the network somewhere and swamps the effect. Before
re-measuring, every density looked bad and the best was the one already
shipped. After, the same experiment reads 0.83–0.95. Central's true
free-running rate turned out to be 7.31 Hz against a genome that claimed 6.00 —
so the shipped M1 genome was already slightly miscalibrated, and M2 was
measuring that rather than vision.

Fixing the *rest* of the calibration is not free, and this is the interesting
part: vocal free-runs at 7.0 Hz against a genome target of 5.0, and setting it
honestly to 7.0 pushes the babble duty cycle to 0.93 and the baby drones. That
target is deliberately below free-running to hold the duty cycle down. The
invariant "targets must equal free-running rates" is about measurement
validity; it collides here with a behavioural requirement, and the behavioural
one wins.

### The eye points itself, and there is a port for a real one

DNA v31 gave the retina a controller: once per frame it re-aims at the centroid
of the cells responding above 0.70 of the peak. Inside the fovea it recovers
**67%** of what a perfect eye would gain (vision 0.520 → 0.840, gaze 3.8 → 1.3
px); outside the fovea it recovers nothing, and peripheral acquisition is the
named next problem. It was switched on at v1.0.1 for the live path rather than
for a milestone — a real camera does not hand the creature a pre-centred object
and every experiment here does.

What the retina owns is a **crop window over a fixed camera**. A real eye is
different in kind: it belongs to whoever built it, it takes a command, it moves
at its own pace, and it answers late. So the seam is drawn between the two
halves that were never one thing — **where to look** stays in the creature,
**how the eye gets there** leaves — and the port is data in and data out with no
callback, the same shape as `present()`/`features()`:

```
     controller (DNA v31, in the retina)
              │  gaze_command()  — px, and fractions of the frame
              ▼
        [ your actuator ]        — pan/tilt head, upstream cropper, robot arm
              │  report_gaze()   — where it actually is, echoing the command seq
              ▼
     sampling, and the panel's crosshair
```

`Retina::EyeMount` says which side is aiming. `kInternal` slides the sampling
window; `kExternal` means the frame already arrives aimed and the window must
not slide as well. The same three operations exist over the WebSocket as `eye`,
`gaze` and `look`, so the device can be out of process — `tools/eye_wire_test.py`
drives all of it against a live host, and §9.1 of the requirements is the
message reference.

Four things this cost a measurement to learn, in the order they matter to
somebody wiring up hardware.

**A fast actuator is more dangerous than a slow one.** One frame of reporting
lag with an instant actuator flings the eye into the frame rails — 26 px on a
64 px frame — while the *same* lag at half slew is fine at 1.6 px. The slew is
acting as the low-pass that holds loop gain under one across the dead time,
because the controller re-aims every frame with no model of its own motion and
so keeps commanding movement it cannot yet see. **Readback precision is nearly
free**: 2.0 px of encoder noise costs nothing. Spend on reporting latency, not
on encoder resolution.

**Applying the aim twice is silent.** Leaving the mount internal while a device
is also aiming is the obvious integration mistake, and the expectation was that
it would tear the loop apart. It does not: at gain 0.70 the doubled loop is
1.4, inside the stability bound of 2, so it converges on the toy and scores like
a working eye — 1.3 px from the toy while the host reports 2.6. On the
pessimistic motor it is not punished either, because slew 0.5 was already
halving the loop and the doubling cancels it. **No arm in `gazeprobe` detects
the mistake from performance.** The only symptom is that the two ends disagree
about where the eye is, which is why `mount` is an enum and not a comment, and
why the panel now draws the position and the command as separate marks.

**A correctly wired external eye is the same creature.** `gazeprobe`'s `device`
arm runs the whole loop through the port with the harness acting as the eye, and
reads 0.840 / 1.3 px against the internal controller's 0.840 / 1.4 px. That arm
fails the probe if the two ever diverge, because a port that quietly changes the
answer is worse than no port.

**The freeze is worth 0.3 px, and it is kept anyway.** When a device stops
answering, the controller stops commanding — built from the servo sweep's
divergence, on the reasoning that steering blind is what the sweep punished.
Measured, it buys almost nothing: 1.8 px of command drift held against 2.1 px
open, in the one arrangement that leaves the loop wanting anything (an eye lost
on the first frame, before it ever reaches the toy). The reason is structural
and worth knowing rather than fearing — **the loop is proportional with no
integrator**, so a command is one gain-step off a frozen belief and cannot wind
up however long the device is gone. The runaway it was built against needs a
device that answers *late* rather than one that stops. What it is kept for is
the interface guarantee and an explicit `held` counter; the timeout is settable
if you want the other behaviour.

### M3 — cross-modal association: **built, G3 not met**

The headline goal, and the one the project is currently stuck on. Everything
M3 needs is wired and measured; what does not happen is the learning.

**The protocol.** A caregiver holds up a cube or a ball and says its name. The
two names are an open /a/ and a close /i/ — far apart in the one dimension a
24-band mel filterbank resolves well — and they are *spoken*, through the same
cochlea a microphone would drive, so the baby meets a word as sound and never
as a label. Praise accompanies the naming and is **identical for both words**,
so reward carries no information about which object this is. Then a probe: the
same object, in silence, no praise. The baby's own vocalisations during the
probe are the only thing the classifier ever sees.

Four things make the number mean something, and each of them costs accuracy:

- **The shapes are area-matched** — the disc is drawn at 2/√π times the
  square's half-width. Without that the two differ by 27% in lit area and a
  brightness meter could pass a milestone about form. What is left is corners
  against curvature.
- **A control upbringing**, not just a control label: a second creature sees
  the same objects and hears the same two words in the same proportions, paired
  at random. A baby whose two shapes already drove its voice apart before any
  teaching scores the same in both, and has learned nothing.
- **Split by time**, as in M2 — fitted on the first half of the session, tested
  on a creature that has gone on learning.
- **Probes the baby slept through are dropped.** Its eyes were shut and its
  larynx closed; scoring those hands the classifier a coin flip labelled as
  data.

**The result**, five creatures against five controls, 200 s each:

| | held-out accuracy |
|---|---|
| **the baby's voice, named consistently — the milestone** | **0.54** |
| the same, named at random (the control upbringing) | 0.50 |
| timbre only, loudness and voicing dropped | 0.60 |
| labels shuffled — the control | 0.55 |
| **its voice while the word is playing — the echo** | **0.83** |

Against a 0.75 bar, that is a fail, and the fourth row is the honest way to
read the first: with 16 held-out probes the score is quantised to 1/16, and the
shuffled control lands *above* the milestone.

**A longer, wider run settles what that means.** Four independent seed
families, twenty creature pairs, 600 000 ticks each — 96 probes per creature
instead of 32:

| over 20 creature pairs | mean | 95% CI |
|---|---|---|
| **the milestone — voice, named consistently** | **0.507** | [0.477, 0.537] |
| control upbringing — named at random | 0.512 | [0.478, 0.545] |
| labels shuffled | 0.494 | [0.464, 0.523] |
| **the echo — voice while the word plays** | **0.827** | [0.785, 0.869] |

Named beat its own control in **9 of 20** (sign test p = 0.82), **0 of 20**
creatures clear the 0.75 bar, and the alignment cosine is +0.055 with a
confidence interval from −0.28 to +0.39 — which is what "no direction at all"
looks like. The four families agree rather than averaging out: they read 0.525,
0.525, 0.525 and 0.454 on the milestone and 0.88, 0.85, 0.81 and 0.77 on the
echo, so no family is quietly carrying an effect the pool has cancelled.

**The last row is why this is a conclusion rather than a disappointment.** The
same classifier, over the same creatures, at the same number of trials, reads
the audio route at 0.83 — 16 of 20 creatures clear the bar on the echo. The
measurement is not blunt, underpowered or broken; it detects exactly the kind
of effect G3 asks for, in the same session, and there is none on the visual
route to detect. The interval is tight enough to bound it: whatever cross-modal
association exists is smaller than about four points of accuracy, against the
twenty-five it would need.

Reproducing it means changing `seed` in the genome — the five creatures of one
run are `seed + r × 7919`, so a new seed is a new family — and running each at
length. Four families take about seven minutes in parallel:

```sh
./build/aibaby --dna dna/family.toml --experiment m3 --ticks 600000
```

**But the echo works.** 0.83 while the caregiver is talking says the baby
repeats a word it hears — and that is new, and it was the prerequisite. M3
cannot bind a picture to a sound the creature has no way to make, and before
the ears were wired to the larynx a word was legible in B1 at 0.93 and in the
voice at 0.50, which is chance. That route is the dorsal stream, and it is the
densest projection in the genome at 0.15, because nine population-coded motor
groups are a lossy thing to push a pattern through: at 0.08 the word was still
legible in B5's spikes at 0.93 and in the voice at only 0.57. Only when most of
a motor group is driven does its centroid move. The anatomy this argues for is
a large tract rather than a few fibres, which is what the arcuate fasciculus is.

**Where it actually breaks.** `m3probe` walks both routes with no learning at
all and reads every module with the same classifier, which turns "it fails"
into a location:

| | vision | central | vocal | the voice |
|---|---|---|---|---|
| **a word**, to an empty field | 0.38 | 0.82 | 0.92 | **0.88** |
| **a shape**, in silence | 0.98 | 0.66 | 0.54 | **0.42** |

The word arrives at the larynx intact. The shape is vivid in the retina and
reaches the voice at chance. This is M2's caveat coming due exactly where it
was predicted to: B1 knows *that* it is looking at something far better than it
knows *what*.

**Reading the same spikes by *when* they happened.** Those columns count
spikes over a window hundreds of ticks long, and that is a strange thing to do
to this particular brain: the retina deliberately spends *time*, firing each
cell once per frame and early if it responded strongly, so the picture is
carried by the order of a volley rather than by how many spikes it holds
(§5.1). A module could represent the shape perfectly in its timing and still
score at chance here. So `m3probe` now also reads every module resolved by
phase within the retinal frame — four slices, `kFeatureBins / 4` spatial bins
each, so both readouts get **the same 32 numbers per trial** and differ only in
whether those numbers describe *where* or *where and when*.

Over three seed families at 300 trials each:

| a shape, in silence | per-neuron | 32 spatial | 8×4 space+time | shuffled |
|---|---|---|---|---|
| **retina (B3)** | 0.94 | 0.56 | **0.70** | 0.47 |
| **association (B1)** | 0.77 | 0.57 | 0.56 | 0.50 |

**The retina's timing carries shape; B1's does not.** At the same feature
budget, asking *when* buys the retina +0.15 — consistently, in all three
families — and buys the association module nothing at all. The volley structure
that holds the shape is real, and it does not survive the first synapse. (The
readout itself is fine: on the word condition the same phase-resolved feature
reads auditory at 1.00, so a strong signal passes through it undamaged.)

What *does* reach B1 is the fine-grained part: 0.77 per neuron, but only 0.57
once pooled into 32 bins, and its own shuffled control sits at 0.50. So the
shape arrives in a form that needs 400 dimensions and 150 training trials to
extract — and the thing downstream of B1 is a sparse, coarse, rate-driven
projection with neither. The information is *present* and *unreadable by the
rest of the brain*, which is a different problem from the one we thought we
had.

**The fix that followed from it helped everything except G3.** If the volley is
being integrated into a sum, the cure is to stop integrating: central's
membrane constant went from 20 ms to 5 ms, swept against a threshold re-tuned
at every step to hold the module's free-running rate at 8 Hz, so the experiment
varies *how long it integrates* and not *how much it fires*. That is written up
below as a design finding, because it moved M2 and G2 substantially. It did
nothing at all for G3:

| leak_tau_ms | M2 | M2, rate divided out | G2, praise beat its control | **G3** |
|---|---|---|---|---|
| 20 (before) | 0.89 | 0.69 | 2 of 9 | 0.52 |
| 10 | 0.91 | 0.73 | 3 of 9 | — |
| **5 (now)** | **0.94** | **0.79** | **6 of 9** | **0.52** |
| 3 | 0.97 | 0.84 | 5 of 9 | 0.46 |

Faster integration recovers a great deal of *where the object is* and none of
*which object it is*. That is worth stating plainly: the two questions had
looked like the same question at different difficulties, and they are not. The
cheap end of the fix list is now spent, and what remains for G3 is the
expensive one — a stage that makes a corner and an arc different *kinds* of
thing rather than different arrangements of the same one.

**One finding about the protocol, from the reward trace.** Learning is gated on
reward minus its running expectation, and the two phases of a trial are not
symmetric: mean R−E[R] is **+0.007 while naming and −0.013 while probing**. The
probe is quiet by construction, so it sits below the session's own expectation,
and whatever the baby does during the phase being *measured* is gently
unlearned. A protocol can do this to itself with nobody noticing, which is why
both phases are now accounted for separately. It is a real effect and it is
probably not the whole story: the alignment cosine — does the picture drive the
voice the way the word does — swings between −0.94 and +0.91 across the
quarters of a session with no trend in it. That is noise, not a learning curve
being undone, and a mechanism that was working and then being erased would
look like the latter.

**Praise is not the variable.** `m3sweep` re-runs the milestone with the
caregiver's approval at 0.5, 0.2 and 0.0 — the last leaving curiosity as the
only reward in the creature's life. The milestone reads 0.59, 0.63 and 0.53,
none of them separated from its own control, and at praise 0.0 the shuffled
control comes in at 0.66, above the milestone. Two creatures per arm makes
these noisy and none of the differences should be read as real; the flat,
uninformative shape of the sweep is the point. Turning the reward gate off
entirely does not make cross-modal association worse, which is what you would
expect when the thing being gated never arrives. The echo ceiling does move —
0.896, 0.958, 0.969 as praise falls — so reward-driven weight motion is, if
anything, currently degrading the one route that works.

**What the wiring cost.** The ears→larynx tract drives the vocal module hard
enough to more than double the babble duty cycle: 0.34 without it, 0.81 with,
against a ceiling of 0.85 in the `babble` criterion. (It is wired last on
purpose, so deleting it leaves the rest of the brain bit-identical and the
comparison is clean.) The creature is close to droning, and that ceiling is now
the binding constraint on making the tract any denser.

**The obvious ceiling experiment is not available.** For G2, `g2probe` bounds
what reward can do by making feedback dense and immediate. The analogue here
would be to switch homeostasis off so nothing erases what is learned — and it
does not work: with `ip_rate` and `scaling_rate` at zero the creature stops
producing usable probes at all (11 in a session that normally gives 32) and the
echo falls from 0.83 to 0.57. That is the project's oldest finding restated —
intrinsic plasticity is what keeps the network in a regime where weight changes
still turn into rate changes — and it means a G3 ceiling has to be built the
way `g2probe` was, with homeostasis intact and the reward schedule idealised
instead.

So M3 stands at: the voice route works and is stable, the picture route now
delivers *where* well and *what* not at all, and there is no cross-modal effect
of any size to preserve. Three candidate explanations have been measured and
spent — praise strength, session length, and integration speed — which is what
makes the remaining one worth building rather than guessing at.

### M4 — growth, myelination and sleep consolidation: **done, G4 passes**

G4 is the only goal in the document phrased entirely as things the creature
must *not* do — "neuron count stays flat while error is improving; grows only
on a detected plateau; never exceeds the DNA budget cap". A brain with the
growth code deleted satisfies all three. So `g4` carries its own non-vacuity
control, the same way `m2` carries shuffled labels: a second arm in which the
saturation guard is lowered until growth is unavoidable, proving the path being
restrained is a path that works.

```
as raised (re-measured on DNA v35)         forced (guard lowered)
  windows    75 (4 improving, 70 plateau)     neurons  1102 -> 1150 (cap 9216)
  neurons    1102 -> 1190 (cap 9216)          growth   6 events, 48 neurons
  growth     11 events, 88 neurons            determinism  31 checkpoints, identical
  ledger     w4 w7 w10 w13 w16 w19
             ... 42 windows of nothing ...
             w61 w64 w67 w70 w73
  saturation rate 8.21 Hz vs 20.0 bar
             weight 0.146 vs 0.300 bar
  crowding   0.4% at 20 s -> 0.0% thereafter
  error      0.0263 at 20 s -> 0.0057 at 1320 s
  sleep      7 passes, 25 synapses pruned
  replay     8 episodes held, 56 replayed
  myelination  mean per-edge rate 0.654 x eta
```

The forced arm is kept because `require_saturation = 1` restores the old
behaviour, and that arm is what proves the path still works when it is.

**Until DNA v5 a normally raised creature never grew, and the numbers say why.**
It reaches 68% of the rate bar and 51% of the weight bar and stops there. That
is not the guard being set out of reach by accident — §3.4 asks for "mean
firing rate high, weights near bounds, little headroom", and §3.1 installs a
mechanism whose entire purpose is to stop exactly that from happening. **The
two clauses of the requirements contradict each other**, and the contradiction
is invisible until you measure: a regulated module cannot have a high rate,
because regulation is what a setpoint means.

Two further measurements say the literal reading is unreachable rather than
merely strict, and they are the reason it was replaced rather than retuned:

- **The rate is a setpoint, not a slow climb.** The association module's peak
  over 25 minutes was 13.54 Hz against a 20 Hz bar when this was written, and
  its genome target is 8.05 Hz. It is being *held* well below the bar, and a
  longer life reads the same. (On the shipped genome the peak is now **8.21 Hz**
  — the argument is unchanged and the gap is wider.)
- **Crowding decays.** The share of incoming edges within 3/4 of their own
  ceiling reads 2.0% after the first 20 s and **0.0% for the remaining 25
  minutes** — sleep downscaling pulls weights away from their bounds faster
  than learning presses them into it. Nothing in this creature accumulates
  toward "full", so no bar placed there can ever be crossed. (Now 0.4% at 20 s
  and 0.0% after, same shape.)

**What replaced it, in DNA v5.** "Little headroom" now means the module cannot
explain any more of what happens next with the structure it has: the creature
grows while it is plateaued *and still wrong*. Prediction error is the one
quantity here that homeostasis does not regulate to a setpoint — it falls to
0.0084 in the first window and sits flat, which is precisely the "stuck with
work left to do" state §3.4 is describing.

```
error_floor = 0.004       # grow while the critic is still this wrong
patience    = 6           # ...and stop if six events in a row do not help
require_saturation = 0    # 1 restores the literal §3.4 reading, and every
                          # measurement taken before DNA v5
```

`patience` is §3.4's own warning made mechanical — *"growth without limits
masks bugs: the network expands instead of revealing that learning is broken"*.
If adding neurons stops helping, growth stops, and a flat neuron count goes
back to being evidence.

**What a normally raised creature does — corrected 2026-08-20.** This section
used to say "six growth events, 48 neurons, 1102 → 1150, and then it stops".
That is now the *forced* arm. Re-measured on the shipped genome, growth comes in
**bursts**: six events at windows 4–19, then **840 s of simulated life with
growth switched off**, then five more from window 61. `patience` is not
terminal — an improving window clears the counter and growth re-arms.

The conclusion underneath it is unchanged and is the honest result about this
brain: **more neurons in the association module do not help it predict the next
sound.** Eleven events and 88 new cells move the error from 0.0070 to 0.0057,
while the fall that actually matters — 0.0263 → 0.0070 — happens in the first
280 s, before growth has added anything. Growth is reachable; growth being
*useful* is still open, and the crowding curve above is the first place to look.

Three variants of the re-arm rule were measured at 1.5M ticks before settling on
the one that ships (see `Brain::try_grow`): the reference frozen at the last
event gave 11 events, moved forward on every judgement 12, and the improving-
window verdict 11 with a **bit-identical ledger** to the frozen one. The
suspicion that motivated looking — that a stale absolute reference lets the error
*drift* across it and re-arm growth for a reason growth had nothing to do with —
is **not supported by that pair of numbers**; the two rules agree at every
judgement here. What shipped is a simplification, not a fix: one definition of
"improving" in the creature instead of two differently-scaled ones.

**A panel readout was fixed at the same time, because it caused this question.**
The structure card read `grown / cap` — "40 / 9216" — which parses as
live-over-capacity and made a healthy creature look like a network stuck at 0.4%
of its size and refusing to grow. It is a *cumulative growth counter* over the
*arena ceiling*: 40 is five events' worth of new cells, 9216 is the sum of the
six modules' `n_max`, fixed when the arena is allocated at hatch and unable to
move. The card now reads `1,142 / 9,216 (40 grown)`.

**Fixing the trigger exposed a bug in G4's own checker.** Growth records are
written at window boundaries, after the tick's growth decision has run, so an
increase between record i-1 and record i belongs to window *i*. Both violation
predicates blamed window i-1. With the count permanently flat, both readings
were vacuously true and nothing could tell them apart; the first run that grew
reported a violation at window 3 while the ledger showed all six events landing
on plateaus. The predicates now read window i, and `g4` prints the ledger so
the alignment is checkable rather than assumed.

**And it exposed a latent buffer overrun in three experiments.** `m2`, `m3` and
`m3probe` each sized a feature buffer from a live `ModuleState&` and then
indexed it with the same field read later — safe only while modules never
changed size. The first growing m3 run died with a heap corruption. All three
now fix their feature width at session start, which a classifier needs anyway:
a neuron that did not exist when the session began has no column. Both the host
and the core are clean under `-fsanitize=address,undefined` after the fix.

**The forced arm is what makes that a result rather than an absence.** With the
same code and a lowered guard, 63 growth events insert 504 neurons at the
pressure-weighted centroid of the association module, wire them to their local
neighbours, and stop at the cap — and the twin raised on the identical script
agrees bit-for-bit at all 31 checkpoints. **G1 survives structural plasticity**,
which the `determinism` experiment cannot tell you: at 20k ticks nothing has
changed shape yet.

**Myelination (§3.5) is the piece that acts continuously**, and it is the only
part of M4 that touches an already-passing measurement. Each edge keeps a leaky
traffic counter; a busy edge's axonal delay falls toward half its birth value
and its learning rate falls toward 0.3 of eta. After 1500 s the mean per-edge
learning rate across the brain is **0.64 x eta** — pathways carrying the
creature's behaviour are measurably protected from the next two seconds of
reward. It is a saturating hyperbola of traffic rather than an exponential, so
it reverts on its own as traffic decays and nothing has to remember that an
edge was once busy.

Three things about the implementation are not obvious and are load-bearing:

- **Growth is confined to association modules**, and that is structural rather
  than a policy choice. Every transducer reads its module by slicing the live
  range into equal contiguous pieces — one per mel channel, retinal feature,
  caregiver action or motor group. Changing the neuron count moves every slice
  boundary at once, so growing the cochlea by one neuron renumbers all
  twenty-four channels and every weight downstream is suddenly about the wrong
  frequency. That is a catastrophic-forgetting event caused by the mechanism
  meant to add capacity. The body plan agrees: a cochlea has as many channels
  as it has.
- **Pruned neurons are tombstoned, not compacted.** Compaction would renumber
  neurons, and every synapse, every reverse entry and every transducer's
  channel map is keyed by neuron index. A dead slot is skipped by the tick loop
  and reused by the next growth event, which is the same thing at a fraction of
  the risk.
- **Sleep downscaling needs a floor, and it is not obvious that it does.**
  §3.6 asks for uniform downscaling, which is what makes pruning selective —
  multiply everything by 0.98 and the survivors are the edges reward and
  traffic were holding up. But nothing awake puts the weight back: synaptic
  scaling is silent inside its band and reward-modulated STDP is signed, so the
  multiplier compounds over every sleep of a long life and the creature quietly
  fades. Each neuron therefore keeps what its *structure* entitles it to —
  birth, plus growth, minus pruning — and downscaling may not erode past 60% of
  it. This is a bound rather than a setpoint, for the same reason
  `scaling_band` is.

**Pruning is very conservative and the reason is the same homeostasis.** Seven
sleep passes removed 21 synapses and no neurons. §3.4 asks for edges that are
weak *and* idle; intrinsic plasticity guarantees nothing is ever idle, so the
traffic half of the test almost never fires. This is under-tuned rather than
wrong, and the number to move is `prune_traffic` — deliberately not moved here,
because every existing measurement is calibrated against the current genome.

To show the path is right rather than merely unused, a genome that sleeps ten
times as fast and prunes hard (`prune_weight` 0.09, `prune_traffic` 400,
`downscale` 0.90) was run through the same experiment: **13,485 of about 22,000
synapses removed and 32 neurons tombstoned across 35 consolidation passes, with
growth running at the same time — and the twin still agrees bit-for-bit.** That
build is also clean under `-fsanitize=address,undefined`, which matters here
more than anywhere else in the project: pruning compacts every neuron's slice
of the synapse pool and rebuilds the reverse index from scratch, and the reverse
index is the most index-dense structure in the codebase. The sanitised binary
produces the same state hash as the optimised one.

**Growth watches the critic's prediction error, so it is silent without sound.**
The plateau detector reads the one quantity that means "how well do I
understand what happens next", and that model predicts the next mel frame. A
creature raised entirely on vision, or one nobody speaks to, never completes a
plateau window at all — the panel shows `plateau windows 0` with the microphone
off, which is correct and worth knowing before it looks like a bug.

### Sleep: gating and consolidation both done

Fatigue was write-only — it accumulated with activity and had no way back
down, so it pinned at 1.0 and the creature babbled through it forever. Sleep is
now a state with two thresholds (0.90 to fall asleep, 0.35 to wake, so it
cannot flutter at the boundary): sensory input is gated off, the larynx closes,
and rest is the only thing that discharges fatigue. A baby babbling at ~5 Hz is
awake for about fifteen minutes and then sleeps for about three.

The experiment now keeps an object in front of the creature for the whole
session, so the visual half of that gate is checked too: B3 runs at 5.14 Hz
awake and 2.98 Hz asleep with the same object in view, and the retinal drive
falls to 2e-5. The eyes close as well as the ears.

The `sleep` experiment is the regression test, and it needs a long run because
the cycle is long:

```sh
./build/aibaby --experiment sleep --ticks 1500000    # ~35 s of wall clock
```

The half of §3.6 that matters for learning now runs in that window too. Every
30 s of a sleep bout the creature downscales, prunes, and replays: high-reward
episodes are re-presented to the encoders from the inside — the room stays shut
out, `hear()` and `see()` still drop their frames — and the reward each one
earned is paid out at the end of it, against an eligibility trace holding the
replayed activity rather than the original. In the live app a baby left alone
for fifteen minutes falls asleep and re-experiences 32 episodes over four
passes.

That replay drives the same encoders the gate is supposed to shut is why the
`sleep` experiment skips its retinal peak measurement during replay ticks. The
measurement asks whether the *room* reaches a sleeping baby; counting replay
would read a working memory as a leaking gate. The object stays in front of the
creature throughout, so a real leak still shows up on every other tick.

### G2 — rewarded vocalisations increase: **met**

The honest result. Nine creatures — same genome, different developmental seeds
— each paired with its own yoked control that received the same praise and
scolding, in the same proportions, time-shifted so it followed nothing the baby
did. Praise arrives 500 ms after the sound that earned it.

| session | mean rate advantage | mean share advantage | praise beat its control |
|---|---|---|---|
| 200 s, before M2 | +0.098× | +0.044 | 5 of 9 |
| 420 s, before M2 | −0.002× | +0.012 | 3 of 9 |
| 200 s, after M2's recalibration | +0.125× | +0.069 | 7 of 9 |
| 200 s, after M3's wiring | −0.134× | −0.077 | 3 of 9 |
| **200 s, after the 5 ms membrane constant** | **+0.102×** | **+0.051** | **6 of 9** |
| 200 s, with M4's consolidation | +0.089× | +0.059 | 5 of 9 |

**Consolidation was the plan of record for G2, and G2 did not move.** M4
built the mechanism §3.5 describes — the mean per-edge learning rate really
does fall to 0.64× eta — so the prediction was testable for the first time.
Run as a paired comparison, the same nine seeds with the M4 mechanisms switched
off in the genome and nothing else changed:

| seed family | arm | rate advantage | share advantage | praise won |
|---|---|---|---|---|
| 20260809 | off | +0.102× | +0.051 | 6 of 9 |
| 20260809 | **on** | +0.089× | +0.059 | 5 of 9 |
| 20360812 | off | +0.143× | +0.070 | 3 of 8 |
| 20360812 | **on** | +0.010× | +0.006 | 6 of 9 |

The off arm reproduces the previously recorded numbers exactly, so the
comparison is sound. **The result is that it does not resolve.** The two
families disagree about which arm wins, and they disagree in opposite
directions on the two statistics: on win count consolidation looks better in
one family and worse in the other; on magnitude it looks worse in both, but the
gap between the two *off* runs is itself larger than the gap between arms
within either family. Five of nine creatures changed which side they fell on
when M4 was switched on — the per-creature verdict is noise, not signal.

What this does establish is a bound. Whatever consolidation contributes to G2
is smaller than the noise floor of a nine-creature run, and G2's shortfall is a
factor of about 1.4 in the absolute clause (×0.70 against a bar of 1.0). A
mechanism that cannot be detected at this scale is not what closes that gap.
Settling the sign would need the treatment the M3 null got — twenty pairs
across four seed families at 600k ticks — and that is worth doing only if
something else makes consolidation look promising again.

**The row for the 5 ms membrane constant recovers the regression above it**, and it was
not aimed at G2 at all — it came from asking why B1 could not hold a shape.
Cutting central's integration window stops the association module smearing its
inputs into a level, and one consequence is that the vocal module free-runs at
6.6 Hz instead of 7.0 against its 5.0 Hz target. Less distance to its target
means intrinsic plasticity spends less of the session hauling vocal thresholds
around, which is precisely the drift that was burying the reward effect. Every
controlled number is positive again, with M3's tract still in place.

**Why the row before it was a regression, and M3 caused it.** The ears→larynx tract is
dense by necessity and it drives the vocal module hard — the babble duty cycle
goes from 0.34 to 0.81 — so the module now sits much further above its 5.0 Hz
target, and intrinsic plasticity spends the session pushing its thresholds up.
That is a large, reward-independent force acting on exactly the quantity G2
measures, and it swamps a transient effect that was never bigger than +0.125×
in the first place. Every controlled number M2's recalibration had improved
went back down. Whether M3's route to the larynx and G2's measurable reward
effect can coexist under the current homeostasis is now an open question, and
it is one the answer to consolidation would settle: a shift that consolidates
does not have to out-shout homeostasis every session to survive.

**Sleep-gated homeostasis identifies the eraser and cannot remove it.** DNA v8
adds `wake_scale` and `sleep_scale`, two multipliers on `ip_rate` and
`scaling_rate` chosen by whether the creature is awake. At 1.0/1.0 the brain is
bit-identical to one built before the fields existed, which is what makes the
sweep readable. Nine creatures at 200 s:

| wake | rate advantage | share advantage | won | mean ratio |
|---|---|---|---|---|
| 1.00 | +0.100× | +0.0629 | 7/9 | ×0.72 |
| 0.75 | +0.135× | +0.0681 | 6/9 | ×0.79 |
| 0.50 | +0.055× | +0.0253 | 5/9 | ×0.82 |
| 0.25 | +0.221× | +0.0980 | 6/9 | **×1.07** |
| 0.00 | +0.261× | −0.0131 | 3/8 | ×1.36 |

The mean ratio is monotonic; the controlled advantage is not, and 0.50 sitting
below both its neighbours is the reminder that nine creatures cannot resolve
that column. **The decay is the result worth keeping**, because it is what the
hypothesis is actually about. Re-run at 420 s — the duration at which the effect
was recorded as having vanished — the share advantage goes 0.0629 → **0.0012**
at wake 1.0 and 0.0980 → **0.0518** at 0.25. Baseline loses 98% of its effect
and the sleep-gated creature keeps half. *Awake homeostasis is the eraser, and
this is the direct measurement of it.* The compensating sleep dose (4.0, matching
the 14 minute-units of regulation a sleep/wake cycle used to deliver) was checked
over 1.2M ticks with `sleep`: the cycle is intact, nothing saturates or falls
silent.

**And it still ships at 1.0, because `babble` refuses it.** Duty cycle by
wake_scale: 0.25 → **0.93 (FAIL)**, 0.50 → 0.81, 0.75 → 0.71, 1.00 → 0.67. At
the setting that preserves the reward effect the creature drones, and a creature
vocalising 93% of the time inflates the very quantity G2 counts. The settings
that still babble show no advantage distinguishable from noise.

That collision is the finding, and it is the open question two paragraphs below
answered in the negative: **the larynx's route and G2's reward effect cannot
coexist under a single global regulation rate**, because the vocal module
already sits 1.8 Hz above its target and the one dial moves both in opposite
directions.

**Per module they come apart, and DNA v9 is that.** `wake_scale` and
`sleep_scale` moved off `[homeostasis]` and onto each module, which is also how
neuromodulation actually works — it is delivered to some regions and not others.
Rewarded-share advantage at both durations, with the duty cycle beside it:

| scheme | 200 s | 420 s | won | duty |
|---|---|---|---|---|
| all 1.0 (as before) | +0.0629 | +0.0012 | 5/9 | 0.67 |
| all 0.25 / 4.0 | +0.0980 | +0.0518 | 5/9 | **0.93 FAIL** |
| vocal held, rest relaxed | +0.0560 | +0.0071 | 6/9 | 0.63 |
| central relaxed only | +0.0165 | +0.0279 | 7/9 | 0.65 |
| **vocal + auditory held** | +0.0372 | **+0.0564** | 7/9 | 0.65 |

The shipped row keeps more of the effect at 420 s than the droning creature did,
and it still babbles. It does not *start* higher — it simply does not decay:
0.0372 → 0.0564, where the baseline goes 0.0629 → 0.0012. Preservation rather
than amplification is exactly what the hypothesis predicts, and it is not what a
louder creature would look like.

**Why auditory has to be held alongside vocal is the surprise**, and the row
above it is the evidence: relaxing auditory alone costs almost all of the
preservation (+0.0071). The ears→larynx tract is the densest in the genome, so
an under-regulated auditory module drives the larynx hard enough to swamp the
reward signal in the very place it has to land.

It is not free. Relaxed modules run hotter under drive — vision sits at 6.5 Hz
awake against its 4.6 target, where it used to sit at 5.2 — and **`calibrate`
cannot see this**, because it measures with homeostasis switched off. `babble`
is the check that catches it. M2 moved the other way, 0.91 → 0.98. And the win
counts are 5–7 of 9 across every arm, which is not significant on its own: G2's
absolute clause is still failing at ×0.64, and settling the controlled column
needs the treatment the M3 null got, twenty pairs across four seed families.

In the runs before M3 the effect was present at 200 s and gone by 420 s. That
direction is the informative part: the longer run is not noisier, it is *later*, and homeostasis
eventually re-centres what reward moved. Reward-modulated STDP here produces a
transient shift, not a consolidated one — which is exactly what §3.5 says
myelination is for. **That was the argument for building it, and M4 built it,
and it did not help.** The transient/consolidated story may still be the right
diagnosis; per-edge learning rates that fall with traffic are evidently not the
cure.

Which clause fails has moved twice, and that history is the useful part.
Originally the rewarded class grew in absolute terms (×1.08) while praise beat
its yoked twin in only five of nine creatures. After M2's recalibration it won
seven of nine, but the absolute rate fell to ×0.90 — the controlled comparison,
which is the one that isolates learning from drift, got better while the
uncontrolled one got worse. After M3's wiring both are bad: ×0.71 absolute and
three of nine. None of the three is a pass, and consolidation was the missing
piece in all of them.

The machinery is not in doubt. With dense, immediate feedback the praised and
yoked brains separate enormously (F1 motor group +0.09 versus −0.48); the
`g2probe` experiment exists to measure that ceiling. What is missing is
signal-to-noise under realistic sparse, delayed praise, and a mechanism that
makes a learned change stick.

### What G2 and G3 were waiting on — and why they were not the same thing

> **G2 has since been met** (see above, and the DNA v10 section below). This
> section is the reasoning that preceded it, kept because the G3 half of it
> is still live and because it records what was ruled out on the way.

It is tempting to file G2 and G3 as one problem, and the tripled M3 session is
what says they are not. **They fail differently, and the difference decides the
order of the work:**

- **G2 has a real effect that does not last.** Praise moves the rewarded class,
  the controlled comparison has been positive in three separate configurations,
  and homeostasis re-centres it within the session. Something exists to
  preserve. **What preserves it is no longer known to be consolidation —** M4
  built it, and G2 did not move by an amount this experiment can see.
- **G3 has no effect at any duration.** Twenty creature pairs over four seed
  families put the milestone at 0.507, CI [0.477, 0.537], winning 9 of 20
  against its own controls — while the echo, measured by the same classifier
  over the same trials, sits at 0.827. Nothing exists to preserve.

Consolidation was therefore filed as the answer to G2 and **not** the answer to
G3 — per-edge learning rates that fall with traffic would faithfully preserve a
binding of zero. The G3 half of that still holds. **The G2 half is now
untethered**: M4 built the mechanism, the mean per-edge learning rate really
does fall to 0.64× eta, and the goal sits exactly where it was. In order now:

1. **A selective stage between B3 and B1** — orientation and curvature, the
   thing that actually separates a corner from an arc — so the distinction
   survives coarse pooling instead of depending on which individual retinal
   cells happened to fire. This is now the only candidate left standing for G3.
   Faster integration was the cheap alternative, it was tried, and it recovered
   *where* without touching *what*; density was tried before that and buys shape
   only by turning B1 into a busyness meter. What is left is the hypothesis that
   B1 cannot represent a distinction its input never made explicit — that a
   corner and an arc arrive as two arrangements of the same kind of thing, and
   no amount of downstream machinery will turn one into a category.
   **Half of this is now built — the orientation half — and it did not work.
   See "DNA v7" below: the cortex is real and measurably tuned, M2 improved
   sharply, and the shape route got *worse*.**
2. ~~**Consolidation** — per-edge learning rates that fall with accumulated
   traffic (§3.5 myelination, M4), so G2's transient shift survives
   homeostasis.~~ **Built for M4, and it does not move G2 at any scale this
   project can currently measure** — see the paired comparison in the G2
   section. This was the plan of record; it is not refuted, but it is bounded
   below the noise floor, so the next idea for G2 should come from somewhere
   else.
3. **A G3 ceiling experiment**, built like `g2probe` — idealised reward, but
   homeostasis left intact, since switching it off silences the creature. Until
   this exists there is no bound on how much of G3 is learnable in principle,
   and step 1 has no target to aim at.
4. **A probe phase that does not carry negative reward prediction error**
   (currently −0.013 against +0.007 while naming), so the behaviour being
   measured is not gently unlearned while it is measured.
5. **Real recurrent persistence in the vocal module**, rather than the readout
   time constant that currently approximates it — the honest version of the fix
   that articulator inertia only stands in for.

### DNA v7 — a visual cortex: **built and tuned, and it does not help G3**

The retina used to project straight into the association module. That is the
optic nerve wired into the hippocampus, and it was the standing explanation for
G3: a corner and an arc arrive as two arrangements of the same kind of thing,
because nothing in the creature had ever represented an orientation.

**What was built.** `ProjectionKind::kGabor`, the first *structured* wiring rule
in the genome — every projection before it drew a random `density` of pairs — and
a `visual_cortex` module of 512 neurons carrying it. The path is now
retina → v1 → central, and `vision→central` is gone. A simple cell's receptive
field is read off its own coordinates: x and y retinotopic through a power-law
cortical magnification, and **z is its preferred orientation**. Sigma and lambda
are in *cell pitches at the field's own eccentricity*, because the fovea is
sampled eight times finer than the outer ring and one fixed field size is either
blind in the periphery or a blur in the middle.

**It is a real cortex, and `v1probe` is what says so.** The probe scores each
cell's preferred orientation against the one its position predicts, with a
shuffled control that pairs it with a different cell's prediction (chance is 45°,
since orientations wrap at 180). Two independent readings:

| reading | own map | shuffled |
|---|---|---|
| field axis, from the wiring alone — no spikes | **24.0°** | 43.6° |
| orientation preference, to a drifting grating | **29.9°** | 45.2° |

The first is the structural ceiling: ~35 afferents on a discrete lattice cannot
specify an orientation more finely than that. The spiking readout sits just
under it, so the cortex recovers most of what its map encodes.

**M2 improved, and improved in the way that matters.** Object-present rose 0.89 →
0.92 — but the honest column, with each trial's firing rate divided out, went
**0.79 → 0.92**. Before, most of what B1 knew about an object was how hard it was
working; now raw and rate-divided are identical, so it is all pattern and none of
it is amount. That is exactly what a selective stage is supposed to buy.

**And the shape route got worse.** `m3probe`, 300 trials, cube against ball in
silence, per-neuron readout:

| | retina | selective stage | central | voice |
|---|---|---|---|---|
| DNA v6 (no V1) | 0.94 | — | **0.77** | 0.42 |
| DNA v7 | 0.993 | **v1 0.567** | 0.473 | 0.487 |

The retina sees the two shapes essentially perfectly. V1 keeps almost none of it,
and central ends up below where it was when the retina wired into it directly.
The word condition is the control and it is untouched — auditory 1.000, vocal
0.992, voice 0.870 — so this is not a broken creature, it is a lossy stage.

**Why, and it is not a bug.** V1 is sparse, high-threshold and coincidence-driven
by construction; it compresses 176 retinal features into an orientation code at
~30° precision and discards the fine per-neuron detail that the retina's 0.993
was made of. A cube and a ball do not differ in local orientation statistics —
an area-matched square and disc present the same edges in the same amounts. They
differ in the **conjunction**: a corner is two orientations meeting at a point,
and curvature is orientation changing smoothly along a contour. V1 makes
orientation explicit and says nothing about how orientations are arranged, which
is precisely the half of item 1 above that reads "and curvature".

**DNA v8 built that second layer, and it does not work either.** `kCurvature`
and a `visual_form` module of 256 cells: each one asks whether the oriented
cells around it are tangent to a common circle, with x,y its centre and **z the
radius**. Tangency is rotation-invariant, so three coordinates are enough and a
curve is detected however it is turned. A disc's whole boundary answers at once;
an area-matched square can be tangent at four points and nowhere else. This is a
radial-frequency cell — a V4 property at a V2 position in the hierarchy.

It calibrates, it is deterministic, and it carries nothing:

| | retina | v1 | v2 | central | voice |
|---|---|---|---|---|---|
| DNA v6 — no cortex | 0.94 | — | — | **0.77** | 0.42 |
| DNA v7 — V1 | 0.993 | 0.567 | — | 0.473 | 0.487 |
| DNA v8 — V1 + V2 | 0.980 | 0.567 | **0.533** | 0.433 | 0.440 |
| DNA v8, readout ×4 | 0.993 | 0.533 | 0.513 | 0.507 | 0.513 |

The last row is a control worth keeping. The obvious explanation for a sparse
code reaching central as nothing is that `v1→central` at density 0.015 samples
too few of the few active cells — so the tract was widened four-fold, breaking
the constant-afferent discipline on purpose. Central went to 0.507. **It is not
a sampling problem.** The distinction is gone before the readout, and adding a
stage that provably computes what it was designed to compute does not bring it
back.

**Where that leaves G3.** Three controlled attempts now say the same thing: on
this substrate, inserting hand-designed feedforward selectivity between the
retina and the association module *attenuates* the shape distinction rather than
reformatting it. Each stage is a lossy spiking transform, and the retina's 0.98
is already linearly decodable — there is nothing for a compression to add, and
plenty for it to lose. What a hierarchy is supposed to buy is invariance and
composability for *learning*, and `m3probe`'s offline linear readout cannot see
either. So one of two things is true, and they need different work:

- the metric is wrong, and G3 should be scored on what central's plastic
  synapses can latch onto rather than on what a classifier can extract — which
  means the **G3 ceiling experiment** (item 3 in the list above) is now a
  prerequisite rather than a nicety; or
- the approach is wrong, and selectivity on this substrate has to be *learned*
  from the retina's own statistics rather than specified in the genome.

Either way the shipped genome now trades G3's shape route for M2's honesty, and
that trade should probably not be kept: the DNA v6 visual wiring is one edit
away, and every mechanism this work added — `kGabor`, `kCurvature`, the two
roles, `v1probe`, the shared retinal geometry — survives the revert.

**One trap, recorded because it cost most of the work.** The first version
scored 46.3° against a 45.8° shuffle — pure chance — with 430 of 512 cells
preferring the *same* orientation. The wiring was correct the entire time. Three
things were wrong with the operating point, and every one of them is invisible
downstream: `noise_amp` at 0.28 left the retina supplying a tenth of V1's
activity; `target_rate_hz` set to the free-running rate meant intrinsic
plasticity **dragged the threshold back**, which is why the first threshold sweep
read flat; and a simple cell's selectivity is *entirely* the threshold sitting
near the top of its input distribution, because all its weights are positive —
ON and OFF both excite — so a cell that fires on two afferents is a summer, and a
summer reports how much light there was. **V1 is therefore the one module whose
target is deliberately not its free-running rate**, since it is silent in the
dark by construction; `calibrate` was taught to expect the gap and reports it the
way it reports vocal's.

### DNA v10 — reward-modulated motor variability: **this is what met G2**

The songbird argument for it is strong. Babble came from a fixed `noise_amp`,
and a fixed exploration rate is the one thing that cannot converge — the
creature was exactly as random after a hundred praises as on its first babble.
LMAN *injects* variability into the motor pathway and gates it by how well
things are going; lesion it and a finch loses the variability and the learning
together. And it aims at a quantity homeostasis does not regulate: rate is
regulated, *which posture the larynx returns to* is not.

It went in in two halves. The first — a *scalar gain* on the noise, driven by
two windows on total reward — measured worse than nothing, and the reason it
failed is exactly why the second half works. Both are in the genome, the first
switched off.

**Motor noise is motor drive here.** Neurons fire on the positive excursions of
a zero-mean noise term, so shrinking the amplitude does not make the larynx
stereotyped — it makes it silent. Five of nine creatures scored inconclusive at
sensitivity 500. A songbird avoids this because the two come from different
places: HVC drives RA reliably and LMAN adds variance on top. `drive_compensation`
is that missing term, handing back as steady depolarisation what the closing
variance removes. **Anything that modulates `noise_amp` in future needs it.**

**And a global scalar gain measures worse than nothing.** With the HVC term in
place: mean rewarded ratio **x0.35 and x0.15 against a x0.72 baseline**, three
of nine still too quiet. Partly the signal is one-sided — `fast - slow` trends
positive as the drives settle, so exploration closes monotonically and the
creature narrows early, before there is anything to narrow onto, where a bird
goes wide early and stereotyped late. But the real error is the shape of the
mechanism: **a scalar gain makes the larynx less varied in every direction at
once**, which is neither what LMAN does nor how node perturbation works. Real
exploration is *directional* — each neuron's own recent perturbation is
correlated with the reward that followed and the ones that helped are kept.
That steers variability instead of shrinking it, and it is the difference
between narrowing onto something and going quiet.

**Directional exploration is the second half, and it carries G2.** Each neuron
keeps a decaying trace of the random numbers it was actually given, and when
the centred reward arrives its excitability moves along that trace: a push that
preceded a better-than-expected outcome is kept, one that preceded a worse
outcome is undone. Averaged over many perturbations that is an unbiased
estimate of the reward gradient with respect to each neuron's excitability —
node perturbation, which needs no path back through the vocal tract to know
which way to move, and which is the standard account of LMAN's instructive
signal to RA. It is a second learning rule beside STDP.

Note the reward it multiplies is the *centred* one. Uncentred, every bias would
drift together, which is motion and not learning — the exact opposite
requirement to the two windows above, where centring was what made the signal
useless.

Swept at 200 s with the scalar gain held at zero:

| perturb_rate | rate advantage | won | mean ratio |
|---|---|---|---|
| 3e-6 | +0.312× | 7/9 | ×0.95 |
| 1e-5 | +0.580× | 9/9 | ×1.20 |
| **3e-5** | **+0.936×** | **9/9** | **×1.35** |
| 5e-5 | +0.888× | 7/9 | ×1.34 |
| 1e-4 | +0.667× | 5/9 | ×1.46 |
| 1e-3 | −0.123× | 4/9 | ×1.01 |

A plateau between 1e-5 and 3e-5 falling away on both sides: too slow and the
gradient estimate never accumulates against the noise, too fast and the
creature chases single lucky perturbations. Across independent seed families:

| session | advantage | won | mean ratio | |
|---|---|---|---|---|
| 200 s, seed 20260809 | +0.936× | 9/9 | ×1.35 | PASS |
| 200 s, seed 20360812 | +0.593× | 6/9 | ×1.30 | PASS |
| 200 s, seed 20451117 | +0.504× | 8/9 | ×1.39 | PASS |
| **420 s, seed 20260809** | +0.672× | 9/9 | **×1.11** | **PASS** |

Both clauses, at both durations, in three seed families — 23 of 27 creatures
at 200 s, and 9 of 9 at 420 s. The 420 s row is the one that matters most: that
is the duration at which every previous version of this creature had lost the
effect entirely.

**One correction to read the advantage column with.** Switching this on takes
praised creatures from ×0.742 to ×1.347 and their yoked twins from ×0.686 to
×0.411 — so about a third of the "advantage" is the control being *harmed*
rather than the creature being helped. Random praise times a perturbation trace
is a random walk in excitability, and that damages a creature; arguably it
should, since a baby praised at random ought to learn nonsense. But the
milestone does not rest on it. ×1.347 is the praised arm alone, and G2's
absolute clause is met without reference to any control.

Everything else still passes: G1, calibrate, babble (duty 0.65), M2 at 0.98,
sleep, G4, and `snapshot` — which matters here, because the mechanism adds two
per-neuron arrays and a resumed creature has to carry both.

### G3 after G2 — where it breaks, and two fixes that do not work

Re-measured on the current genome, because DNA v9 and v10 changed the creature
and every earlier G3 number was taken on a different one. `m3probe`, 300 trials,
per-neuron, cube against ball in silence:

| | retina | central | vocal | voice |
|---|---|---|---|---|
| shape | 1.000 | **0.773** | 0.533 | 0.513 |
| word | — | 0.837 | **1.000** | 0.923 |

Central holds the shape. The larynx does not receive it — while the *same*
module receives a word perfectly. The asymmetry is the two tracts: the arcuate
runs at density 0.15 and `central→vocal` at 0.03.

**Widening it is not the answer, and the ceiling is why.** At 0.06 central
improves (0.773 → 0.880) and the larynx does not move at all (0.533 → 0.513);
0.10 and 0.15 drive the duty cycle to 0.91 and 0.95 and the creature drones.
Enlarging the larynx instead — 14 neurons per motor group to 30 and 50 — makes
it monotonically worse (echo 0.745 → 0.697 → 0.628), so the population vector
is not group-size limited either.

**A warning about reading any of this from a default-length run.** `m3` at its
default 120 s × 5 creatures gives about nineteen probes per creature — the
per-creature milestone column comes out in multiples of 0.1 because it is
computed on **ten held-out trials**, and the echo on about twenty-nine. At that
size the echo reads 0.70–0.79, which looks like a ceiling sitting exactly on
G3's 0.75 bar and is not one: `m3probe`, using the same feature vector over 207
word trials, reads the voice at **0.923**. The low figure was sample size. The
README already carried this warning before it was written down again here.

**At 600k ticks the alignment signal also disappears.** On a default-length run
the alignment statistic — does a picture drive the voice the way its word does —
looked like it separated the arms cleanly, +0.370 named consistently against
−0.080 named at random. With five times the data it reads **−0.083 against
−0.082**. There is no binding to preserve; that apparent signal was the same
small-sample effect as the echo figure above. The milestone itself at 600k is
**0.467 against a 0.458 control**, consistent with the 0.507 over twenty pairs
already recorded.

**What is left, and it is a real asymmetry.** The echo is 0.923 when the word is
spoken *to an empty field* (`m3probe`, 207 trials) and 0.775 when it is spoken
*while the object is in view* (`m3`, 600k). Same feature vector, ample trials in
both. The difference between those two conditions is the picture — so on the
evidence available the visual input is acting as **noise on the vocal pathway
rather than as a signal that can bind to it**. If that holds it is G3's actual
obstacle, and it is a different problem from every one attacked so far: not that
the shape fails to arrive, but that its arrival degrades the very channel the
naming has to travel through.

Testing it needs a third `m3probe` condition — a word *and* a shape together —
which does not exist yet.

### The G3 ceiling: **0.51 against a 0.75 bar**

`g3probe` is the experiment the list above has wanted since M4, built the way
`g2probe` is: an idealised teacher. `m3`'s praise is deliberately
class-uninformative, so when the milestone fails it cannot say whether the
creature lacked the teaching signal or the capacity to comply with one. This
supplies a perfect signal and measures what the creature does with it.

Reward becomes dense, immediate, and class-informative: at every plasticity
interval it reports how far the voice sits along the axis between the two
target postures, toward the one the visible object calls for. Two things make
the number a bound rather than a wish. **The targets are the creature's own** —
each session opens by playing both words to an empty field and recording what
*that* larynx does, so the teacher only ever asks for postures this body has
been observed to produce. And **homeostasis is left running**, because switching
it off silences the creature and would bound a different animal. The control arm
gets reward just as dense but shaped toward a target drawn at random each trial,
which separates "a teacher can shape this voice per object" from "dense reward
makes any voice more classifiable".

Five creatures, 600k ticks, 93 probes each:

| | |
|---|---|
| idealised teacher | **0.515** |
| random target (dense reward, no mapping) | 0.511 |
| labels shuffled | 0.540 |
| **calibration echo** | **0.825** |
| at or above 0.75 | 0 of 5 |

**The echo is what licenses the conclusion.** The same feature vector and the
same classifier read the word-driven voice at 0.825, so the readout is not the
limit and the null is not a broken pipeline — it is the internal positive
control that makes this a result instead of a missing measurement.

So G3 is **not** waiting on the picture→sound→voice teaching signal. A perfect
teacher cannot make this voice depend on what the creature sees. What is missing
is a mechanism that can learn a **conditional** mapping from what is in view to
what the larynx does. Node perturbation cannot supply one: a per-neuron bias is
a constant, not a function of the input — it can teach the creature to vocalise
more like posture X in general, never X-for-cube and Y-for-ball. That leaves
reward-modulated STDP on `central→vocal` as the only conditional mechanism in
the creature, and this bound says it does not manage it even under ideal reward.

> Read the two sections below before taking that last sentence at face value.
> The teacher is ideal; the *condition* the mapping has to key on is not, and
> until it was measured this paragraph was charging the learning rule for a
> deficit it only partly owns.

Three suspects followed, all on the conditional pathway itself: `central→vocal`
sparsity at 0.03, its eligibility being swamped by an arcuate five times denser
and always active, or **vocal's full-strength synaptic scaling erasing the
weights as they form** — the same eraser DNA v9 found for G2, still at 1.0 on
the larynx because relaxing it makes the creature drone. That last one was
directly testable, and testing it is what the next section is about. It is
wrong, and the measurement built to test it says the paragraph above claims
more than it is entitled to.

### DNA v11 — the regulation knob splits, and the suspect it was built for dies

§3.1 has two mechanisms and v9 gave each module one dial for both. On the
larynx they were thought to be in tension: intrinsic plasticity holds the duty
cycle, synaptic scaling erases a rewarded weight change, and v9 could only ask
for both or neither. **v11 splits the dial** — `ip_wake_scale`/`ip_sleep_scale`
multiply `ip_rate`, `syn_wake_scale`/`syn_sleep_scale` multiply `scaling_rate`.
It is a more honest layout regardless of the result: threshold regulation is a
cell-intrinsic conductance change and synaptic scaling is receptor trafficking
at the synapse. They share a purpose, not a machine.

Shipped at the v9 values, so the brain is bit-identical — same determinism hash,
`7b2e385d6ef2faf5`, with and without the split.

Then vocal's `syn_wake_scale` was swept, 600k ticks, five creatures per arm:

| vocal `syn_wake_scale` | g3probe ceiling | calibration echo | babble duty |
|---|---|---|---|
| 1.0 (as shipped) | 0.515 | 0.825 | 0.61 |
| 0.5 | 0.515 | 0.825 | 0.61 |
| 0.25 | 0.515 | 0.825 | 0.61 |
| **0.0 — scaling off entirely** | **0.515** | **0.825** | **0.61** |

Not "a small effect". **Not one digit moves**, in any column, with the mechanism
switched off completely. The instrumentation added to find out why gives the
answer in one line: `vocal sum|w| / setpoint 0.93, outside the band 0.0% of
samples`. Synaptic scaling has a dead band of a factor of three, reward learning
moves the larynx's afferent total by about 7%, and so **scaling never runs on
the vocal module at all**. It was never erasing anything.

And the drone it was blamed for is intrinsic plasticity's alone: with
`ip_wake_scale` at 0.25 and scaling left at full strength, the duty cycle goes
0.61 → 0.83. There was never a trade-off on the larynx to resolve.

**The general lesson is worth more than the null.** The dead band was a
deliberate, well-argued decision — §3.1 asks for a *bound* and a bound is not a
setpoint, and the band is what let G2's rewarded and yoked babies separate at
all. That same decision made the mechanism inert on the larynx, and for a year
of experiments it stayed on the suspect list anyway. *A mechanism that is
present in the genome, defensible on paper, and never actually executing looks
exactly like a mechanism that is doing harm.* Before relaxing a regulator,
measure whether it is running.

### What the same instrumentation found, which matters more

`g3probe` idealises the *teacher*: reward tells the creature which object it is
looking at. It does not hand the creature that fact — the condition side of a
conditional mapping still has to be read off the creature's own activity. That
was never measured. It is now, with the same nearest-centroid classifier and the
same held-out split that scores the voice, and each row against a shuffled
control computed on **its own** feature set.

Three seed families, five creatures each, 600k ticks:

| | vision | central | the voice |
|---|---|---|---|
| held-out cube vs ball | 0.872 | 0.604 | 0.505 |
| matched chance | 0.516 | 0.521 | 0.497 |
| **margin above chance** | **+0.356** | **+0.083** | **+0.008** |

Stable where it matters: the vision margin reads +0.349, +0.349, +0.371 across
the three families; the central margin is positive in all three (+0.106, +0.029,
+0.115) but varies threefold, so treat +0.083 as "small and real" rather than as
a precise quantity.

**The distinction is attenuated about fourfold at each synapse, and by the
larynx there is nothing left.** The single largest absolute loss is not on the
conditional pathway at all — it is `vision→central`, which throws away 0.27 of
the 0.36 the retina delivered.

This does not rescue any of the three visual-route attempts; they were measured
and they failed. But it does mean the closing claim of the section above —
*improving the visual route cannot be what fixes G3* — **is more than the
evidence supports**. A 0.75 bar was never reachable through an association
module that carries the distinction at +0.083 above chance, whatever the
learning rule downstream does. The ceiling is a joint statement about the
conditional mechanism *and* the representation it has to key on, and it cannot
be charged entirely to the first.

What it does still establish, and this is unchanged: the voice picks up almost
none of even the +0.083 that *is* there. So both halves need work, and the
honest next question is which is cheaper — and this table is the first thing in
the project that can tell one from the other.

### DNA v12 — divisive normalisation: the condition improves, G3 does not

The cascade above says the largest single loss is `vision→central`, so the next
question is *how* the distinction is carried, which a single accuracy cannot
answer. Same three families:

| | \|d'\| > 0.5 | mean \|d'\| | sparseness |
|---|---|---|---|
| vision | 22.3% | 0.324 | 0.826 |
| central | 4.2% | 0.177 | 0.913 |

Central's code is **denser** than the sensory module feeding it (sparseness → 1
means every neuron equally active) and carries cube-versus-ball in a fifth as
many neurons. That reads as a sparse-coding problem, and the obvious fix is to
stop forcing every neuron to one target rate — which v11 now makes testable on
its own. It does not work:

| central `ip_wake_scale` | sparseness | mean \|d'\| | cube vs ball |
|---|---|---|---|
| 0.25 (shipped) | 0.913 | 0.177 | 0.621 |
| 0.05 | 0.848 | 0.170 | 0.664 |
| 0.0 | **0.788** | 0.154 | **0.498 — chance** |

Sparseness tracks intrinsic plasticity exactly as predicted, and separability
*collapses* at the sparse end: with nothing holding the neurons in their dynamic
range, a sparse code is a code about nothing. **Sparser is not better on its
own.**

So v12 adds what cortex actually has and this creature did not: **divisive
normalisation**, each module's synaptic drive divided by how active the module
currently is, relative to its own target rate. Pooled on the *fast* rate
estimate (tens of ms), because pooled over a second it would just be a second
rate regulator and intrinsic plasticity is already that — it holds the slow mean
*at* the target, so a slow pool would find nothing to divide by.

**And the sparse-coding story it was built on is wrong.** Sparseness *rises*
with gain, 0.913 → 0.939. What actually improves is per-neuron discriminability
— mean |d'| 0.177 → 0.202 — because the division removes a common-mode "how busy
is this module" term that was riding on every neuron at once. Sparseness was the
wrong variable; shared gain was the right one.

Three seed families, `norm_gain = 1.0` on central against 0:

| | off | on | per family |
|---|---|---|---|
| mean \|d'\| at central | 0.168 | **0.194** | +0.025, +0.006, +0.045 |
| M2 | 93 / 97 / 94% | **96 / 98 / 95%** | up in 3 of 3 |
| cube vs ball, margin | +0.083 | +0.159 | +0.039, −0.008, **+0.196** |
| **G3 ceiling** | 0.505 | **0.488** | unchanged |

Read the first two rows, not the third: the margin's mean is carried almost
entirely by one family, while mean |d'| (400 neurons × 93 probes) and M2 (a
criterion, not a diagnostic) move consistently. Everything else passes — G1,
audio, vision, babble 0.62, calibrate, sleep, G4, snapshot, and G2 at ×1.37 with
8 of 9, against ×1.35 and 9 of 9 before.

**Shipped on for `central` only**, gain 1.0. Provably inert at 0 — all gains
zero reproduces v11's hash `7b2e385d6ef2faf5` exactly — and provably live when
on, gains 1.0 and 4.0 giving distinct hashes. The shipped genome now hashes
`0ebf3dad6155254b`, so every number recorded before v12 is stale.

**The result that matters most is the row that did not move.** The condition
side improved and the voice did not. That is the two-problem claim above,
confirmed by intervention rather than inferred from a table.

### The density follow-up: refuted, and it strengthened the case for v12 anyway

v12 set up an obvious next move. `vision→central` was capped at density 0.06
because past that "the association module reads *busier* rather than
*different*" — and busyness is exactly what normalisation divides out. So:
raise the density with normalisation on, and collect the shape the old cap was
paying for. Densities 0.06 / 0.12 / 0.20 crossed with the gain, one family:

| density | norm | cube vs ball | margin | mean \|d'\| | M2 | duty |
|---|---|---|---|---|---|---|
| 0.06 | off | 0.621 | +0.106 | 0.177 | 93% | 0.61 |
| 0.06 | **on** | 0.736 | +0.145 | 0.202 | 96% | 0.62 |
| 0.12 | off | 0.634 | +0.208 | 0.171 | 76% | 0.60 |
| 0.12 | **on** | 0.689 | +0.149 | 0.212 | 84% | 0.65 |
| 0.20 | off | 0.519 | +0.030 | 0.124 | 61% | 0.56 |
| 0.20 | **on** | 0.668 | +0.183 | 0.202 | 71% | 0.67 |

The `0.06 / on` row is the shipped genome exactly — the fan-out caps this sweep
raises are provably wiring-neutral — so it reproduces 0.736 and 0.202 to the
digit. That is the sweep's internal control.

**The hypothesis is wrong.** With normalisation on, mean |d'| reads 0.202, 0.212,
0.202 across the three densities — flat. Density buys nothing, and M2 falls
monotonically whether normalisation is on or not. The 0.06 cap was right and
stays.

**But read down the columns rather than across them.** Without normalisation
mean |d'| *degrades* as the tract thickens, 0.177 → 0.171 → 0.124; with it, it
holds flat. So normalisation's contribution grows monotonically with how hard
the module is being driven:

| | 0.06 | 0.12 | 0.20 |
|---|---|---|---|
| what normalisation adds to mean \|d'\| | +0.025 | +0.041 | **+0.078** |

That is a better argument for v12 than the one it shipped on. Normalisation is
not a way to buy accuracy — it is what stops an association module from being
swamped by a loud input, and its value shows up exactly when something tries to
swamp it. **Insurance against a loud input, not a way to afford a louder one.**
It is worth reaching for whenever a module is about to be driven harder, which
is not the same question as whether a module needs to be more accurate.

**Two verification failures worth recording, because between them they cost
three runs of this sweep.**

*The check that could not fail.* Central's `max_out_degree` caps its **incoming**
count too — the reverse index shares the per-neuron slicing — and at density 0.20
that silently dropped 2369 reverse entries. I did check for this, with
`--experiment audio`, saw nothing, and reported an all-clear. `audio` does not
print the dropped-synapse warning at all. **A check that cannot fail is not a
check**; the sweep now runs `babble`, which does print it, and aborts on any
warning rather than measuring a brain the genome does not describe.

*The nuisance parameter that was not one.* The second run raised vision's
`n_max` 2048 → 256 alongside the cap, on the reasoning that a transducer can
never grow into that capacity so the slots are dead weight. They are — but
`n_max` sets each module's global neuron index base, so changing it re-rolls the
RNG draw order and rewires everything downstream. Measured directly: vision
`n_max` at 2048 / 1024 / 256 gives three different determinism hashes, while
`max_out_degree` at 80 / 256 gives one. That shifted the sweep's 0.06 baseline
from 0.736 to 0.630 — **a nuisance larger than the effect being measured**.
Raising a fan-out cap is free; touching `n_max` inside a sweep is not.

### DNA v13 — a hippocampus: built, measured three ways, does not separate

The condition side is weak because cube and ball overlap at the association
module. Pattern separation is the one computation whose entire purpose is to
pull overlapping cortical codes apart, so: the fast half of a complementary
learning system. v13 adds a `kHippocampus` role and a per-module **`eta_scale`**
— how fast synapses *onto* a module learn, applied postsynaptically because that
is where a real synapse gates plasticity. Ships at 1.0 everywhere and is inert
there, hash `0ebf3dad6155254b` unchanged.

The module is 1200 neurons off central's 400 (expansion), fed by a sparse random
projection (decorrelation), with a threshold set so only the best-matched cells
fire (sparsification) and `eta_scale = 20` (fast learning). Three configurations,
each measured *within* one brain — the comparison has to be within-subject,
because adding a module re-rolls every projection's wiring:

| | hippocampus mean \|d'\| | central, same brain | sparseness |
|---|---|---|---|
| central-fed, `noise_amp` 0.10 | 0.080 | 0.086 | 0.249 |
| vision-fed, `noise_amp` 0.10 | 0.074 | 0.129 | 0.330 |
| vision-fed, `noise_amp` 0.01 | **0.129** | 0.153 | 0.282 |

**It never beats its own input.** Held-out cube vs ball reads 0.473 against a
0.533 shuffled control — chance. The sparsification half works every time
(0.25–0.33 against central's 0.84), so the module is doing what it was built to
do; the information simply is not surviving it.

Two things were learned on the way, and the first was a wrong explanation of my
own.

**"It inherits central's weakness" — refuted by the second row.** The obvious
reading of the first row is that a random projection cannot create information
it was not given, so a hippocampus downstream of a weak central is capped by it.
That is true as far as it goes, and re-pointing the perforant path at *vision*
(mean |d'| 0.329, the strongest signal in the creature) is both the fix it
implies and the correct anatomy — the dentate is fed by entorhinal cortex, not
by a generic association hub. It made things **worse**, 0.080 → 0.074. The
hippocampus was destroying the signal regardless of what it was handed.

**The code was noise-locked, not stimulus-locked.** The tell was in the table
all along: 0.0% of hippocampal neurons exceeded |d'| 0.5. A sparse code that is
locked to the stimulus has a *few* strongly selective cells; one locked to noise
has none. The arithmetic agreed — threshold 0.42 against an afferent weight of
0.10, while ±0.10 of noise integrated over a 5 ms leak contributes about 0.22,
over half the threshold. Confirmed directly: at the same threshold, dropping
`noise_amp` 0.10 → 0.01 cut the firing rate 1.46 → 0.24 Hz, so most spikes had
been noise. Fixing it nearly doubled mean |d'|, 0.074 → 0.129 — the largest
single improvement of the three, and still not enough.

**Then the readout itself was put on trial, and acquitted.** Pattern separation
is defined as a reduction in *overlap* between representations, not as an
increase in linear discriminability — and a nearest-centroid classifier on rate
vectors measures the latter, so a hippocampus could be decorrelating exactly as
intended and this readout would never show it. Two further reasons to suspect
the measurement rather than the creature: `holdout_accuracy` divides each
dimension by its own training standard deviation floored at `1e-9`, which
amplifies a near-silent neuron's noise enormously, and `mean |d'|` divides by
every neuron including the silent ones. **Both penalties fall hardest on exactly
the codes a hippocampus is built to produce, and the vision positive control
cannot catch either, because vision is dense.**

So `g3probe` now also reports the unbiased readings: the fraction of neurons
that vary at all, mean |d'| over only those, the correlation between probes of
the same and of different classes, and a scale-free correlation classifier.

| hippocampus arm | active | \|d'\| active | r within | r between | **separation** | corr-clf |
|---|---|---|---|---|---|---|
| vision | 1.00 | 0.357 | 0.455 | 0.382 | **+0.073** | 0.867 |
| central | 1.00 | 0.153 | 0.115 | 0.127 | −0.012 | 0.493 |
| hippocampus | 1.00 | 0.129 | 0.120 | 0.142 | **−0.022** | 0.500 |

**The hippocampus separates negatively** — patterns from different classes are
*more* alike than patterns from the same class. And the bias that motivated the
whole exercise did not apply: `active` reads 1.00 for all three modules, so no
neuron was silent enough to trigger the penalty, and the scale-free classifier
agrees with the z-scored one (0.500 against 0.473). The verdict is unchanged,
but it is now measured on the metric that defines the claim with the readout
excluded as a cause, rather than assumed.

The same columns give a second reading of the attenuation cascade, in the
currency of overlap rather than accuracy: separation is **+0.073 at vision and
+0.006 at central** on the shipped genome. Vision→central is where the
distinction dies, whichever way it is measured.

v13 stays in the genome unused, on the same terms as `kGabor` and `kCurvature`:
inert, free, and available to a genome that asks. `eta_scale` is worth keeping
on its own — per-module learning rate is useful well beyond hippocampi.

### The cascade, re-measured: **the bottleneck has moved**

Everything above was aimed using a cascade measured on DNA v11. Four versions
later that map is out of date, and re-measuring it changes where the next work
should go. Three families, five creatures, 600k ticks, current shipped genome:

| margin above matched chance | v11 | **v14 (now)** |
|---|---|---|
| vision | +0.356 | +0.399 |
| central | **+0.083** | **+0.198** |
| the voice | +0.008 | −0.038 |
| loss `vision→central` | **−0.273** | −0.200 |
| loss `central→voice` | −0.075 | **−0.237** |

**Central's margin has more than doubled, and the largest single loss is now
`central→vocal`.** The per-family readings are +0.149, +0.246, +0.200 —
consistent, where the v11 measurement scattered +0.106, +0.029, +0.115. The
credit belongs to DNA v12's divisive normalisation; v14 left central alone.

**Which corrects a framing error of mine.** "Four mechanisms failed to move
`vision→central`" was wrong — v12 substantially fixed the condition side. It got
under-credited because G3 itself did not move, and *"did not move G3"* quietly
became *"did not move central"*. Re-measure the cascade after anything that
touches a module in it; steering by a stale map is how a line of work ends up
aimed at the wrong synapse.

It also makes G3 look far more tractable than it did. Central now reads 0.729,
so a downstream learner that used what it is given could approach the 0.75 bar.
The live suspects are the two parked ones on the conditional pathway —
`central→vocal`'s sparsity at 0.03, and its eligibility being swamped by an
arcuate five times denser and always active — and no longer the visual route.

### Both `central→vocal` suspects, tested and refuted — and they fail the same way

The re-measured cascade puts the largest loss on `central→vocal`, and the two
suspects parked there have been on the list since G2 was met. Both are genome-only
tests. Both are wrong, and the way they are wrong is the useful part.

**Suspect 1 — the tract is too thin (0.03 against the arcuate's 0.15).** Worth
retesting rather than trusting the old answer: it was blocked pre-v12 by the
creature droning past density 0.10, and v14 has since made the creature quieter
(duty 0.51), so the headroom might have appeared.

| `central→vocal` | voice margin | central margin | **echo** | duty |
|---|---|---|---|---|
| 0.03 (shipped) | +0.013 | +0.207 | **0.825** | 0.51 |
| 0.06 | −0.053 | +0.180 | 0.675 | 0.63 |
| 0.10 | −0.027 | +0.287 | **0.550** | 0.79 |

**Suspect 2 — the arcuate swamps its eligibility.** Five times denser and
active whenever the creature hears anything, including itself.

| `auditory→vocal` | voice margin | central margin | **echo** | duty |
|---|---|---|---|---|
| 0.15 (shipped) | +0.013 | +0.207 | **0.825** | 0.51 |
| 0.075 | +0.040 | +0.207 | 0.700 | 0.47 |
| 0.03 | −0.006 | +0.113 | **0.525** | 0.39 |

**The voice margin never leaves zero in either direction, and in both the echo
collapses.** Widening the weak tract costs the echo; thinning the strong one
costs the echo. The two tracts compete for one larynx and the balance between
them determines how well a *heard word* survives into the voice — but no setting
of that balance makes the voice depend on the *object*. Whatever `central→vocal`
is failing to do, it is not failing for want of synapses, and it is not being
crowded out.

That leaves the mechanism itself rather than its wiring. The next measurement is
narrow and specific: **is eligibility even accumulating on `central→vocal`?**
Reward-modulated STDP can only potentiate a synapse that has a trace to cash in,
and central at 8 Hz against a larynx driven mostly by its own noise may simply
not produce enough pre-post coincidence at 0.03 density to leave one. That is a
different claim from any tested so far, and it is measurable directly.

*(Density 0.15 on `central→vocal` was not measured: the sweep's drop guard
aborted it rather than report a brain the genome does not describe. The guard
was added after the last time that happened silently.)*

### DNA v14 — top-down feedback: **shipped**, and the sign is the whole story

Until v14 this creature's projection graph was **entirely feedforward** apart
from one weak `vocal→central` return. Cortex sends about as many fibres back
down a hierarchy as up it, so that was a large architectural gap, not a tuning
choice — and it is the one thing on the brain-realism list that was missing
outright rather than merely simplified.

v14 lets a projection say which presynaptic neurons it may recruit: `either`
(the pre-v14 rule, bit-identical), `excitatory`, or `inhibitory`. A **source
filter rather than a forced weight sign**, because `apply_reward` takes a
synapse's clamp bounds from its presynaptic neuron's `is_inhib_` flag — a
negative weight hanging off an excitatory cell would be clamped to `[0, ceil]`
and driven back across zero by the first reward that arrived. The filter is
applied *after* the density coin so that changing it does not consume different
random numbers and silently re-roll every later projection.

**Then the sign decided everything.** Two arms, `central→vision` and
`central→auditory`, matched for synapse count and for total |w| so that only
the sign differs:

| | central | vision | auditory | outcome |
|---|---|---|---|---|
| no feedback | 8.48 | 4.60 | 4.50 | — |
| **excitatory** | 17.48 ↑ | 15.90 ↑ | 14.52 ↑ | **diverges** |
| **inhibitory** | 7.01 | 2.37 | 2.34 | settles in 2 rounds |

The excitatory arm is a positive feedback loop with a gain above one: five
rounds of re-measuring its operating point took central 8.48 → 17.48 Hz and it
was still climbing. Not a tuning failure — chasing it makes it worse. **That is
the textbook argument for why cortical feedback is not net-excitatory, arrived
at here by measurement.**

What the stable version buys, three seed families:

| | off | on | per family |
|---|---|---|---|
| vision mean \|d'\| | 0.357 | **0.372** | +0.015, +0.009, +0.021 — up in 3 of 3 |
| **M2** | 96% | **98%** | +3, +0, +1 — never negative |
| central | 0.658 | 0.649 | −0.006, +0.014, −0.034 — no effect |
| **G3 ceiling** | 0.527 | 0.522 | unmoved |

**Shipped**, at weight 0.024. It sharpens the module it lands on, and
`vision→central` throws the gain away — exactly as every other thing aimed at
that bottleneck has. It is shipped for the sensory improvement and for the
architecture, not for G3.

The cost is that the distance senses free-run at about half their old rate,
which is why their `target_rate_hz` came down. **That is not a regression: it is
the signature.** Reduced sensory firing under feedback is what "explaining away"
means in predictive coding, and a model that predicts its input well should
respond to it less. G2 still passes with a *larger* effect (×1.86 against
×1.35), though on fewer creatures (7 of 9 against 9 of 9), which is what a
quieter creature looks like. New shipped hash `2594458a1939c28e`.

One caveat with teeth, discovered while building this. The first version pooled
central's 400 neurons into 16 bins and read 0.506 — apparent proof that central
carries nothing. The positive control is what caught it: **the same pooling
reads 0.532 on the vision module**, where `m2` gets 0.98. The pooling, not the
creature, was destroying the signal. A negative result about a population code
needs a positive control run through the identical readout, or it is a
measurement of the readout.

### `vision→vocal` — the seen object finally reaches the larynx, and G3 still does not move

**Shipped 2026-08-18.** New hash `ca3234c61439b538`. This is the first change in
this project that puts the *seen object* at the larynx, and it is worth being
precise about what that did and did not buy.

The genome had nine projections and `vision` reached only `central`. `projprobe`
pushes a module's own activity through a random sparse binary matrix — the
linear part of a tract, nothing else driving the target — and says why that
mattered:

| arm | at source | d=0.03 | d=0.15 | d=0.40 | shuffled |
|---|---|---|---|---|---|
| object from `central` | 0.760 | 0.620 | 0.560 | 0.600 | 0.460 |
| **object from `vision`** | 0.960 | **0.980** | 0.840 | 0.640 | 0.540 |
| word from `auditory` (control) | 1.000 | 1.000 | 1.000 | 0.660 | 0.400 |

Vision's object code crosses a sparse tract intact and central's does not, so
the one source whose code survives a tract had no route to the larynx.

**Density was the wrong axis.** Matching `central→vocal` exactly delivered
nothing — vocal 0.600 against a 0.500 baseline, +8% drive. The linear model has
nothing else driving the target, and the real vocal fires ~971 spikes/trial on
its own, so a tract that does not compete with that is invisible. Weight is the
axis: at d=0.03 the vocal row goes 0.600 → 0.760 → 0.880 as weight goes 0.14 →
0.40 → 0.80, with the shuffled control flat at 0.520.

**Then `audio` failed, and that is the interesting part.** At weight 0.80 the
creature babbles hard enough to hear itself — `self_gain` mixes its own voice
into the room — so mel energy during "silence" went 0.0499 → 0.0830 and an
external vowel no longer lifted the auditory module by the 10% that experiment
requires. The baby was babbling over you. It shipped at **weight 0.30**, the
largest that leaves the creature able to hear, and that costs nothing in
delivery: 0.760 either way, because a gentler tract is not fighting the arcuate
for the same larynx.

**What it buys — G2, on the same nine seeds:**

| | before | after |
|---|---|---|
| praise won | 7 of 9 | **9 of 9** |
| F1 motor group advantage | +0.0255 | **+0.0556** |
| rewarded rate advantage | +0.777× | +0.707× |

The F1 advantage is the largest recorded here, above DNA v32's +0.0559 and
without v32's `rate_norm` fragility. The rewarded-rate advantage slips, which is
an honest cost rather than a rounding error.

**What it does not buy.** G3 does not move. On this genome, with `g3probe`'s echo
control passing at 0.750, the object arrives at vocal at **0.654 against 0.515**
before — and taught minus random is **−0.014**, with 0 of 5 creatures over the
0.75 bar. Delivery up 27% and the milestone went nowhere. That is the third
independent confirmation that **delivery was never G3's limit**; the blocker is
conditioning. No further delivery mechanism should be built for G3.

`central→vocal` is left exactly as it was and is now labelled in the genome as a
known non-participant: deleting it and recalibrating leaves every G3 number
unchanged, its density is flat at chance over a 5× range, and the eligibility
trace it carries is object-weak at 0.654 against the arcuate's 0.892. It stays
because it is the only descending path from the association module to the
larynx, because every G3 result on record was measured with it in place, and
because deleting a projection silently re-rolls the delay jitter of every
projection after it.

**A trap worth recording.** A 300k `--allow-short` screen of an earlier arm read
teacher 0.618 against random 0.536, a +0.082 gap that looked like G3 moving. The
900k run reversed it to 0.518 / 0.563. Screens select candidates; they never
conclude.

### The listening reflex — and the single-seed run that nearly shipped a deaf baby

The tract above did not ship alone, and the reason is the most useful thing on
this page.

`verify` went green and `audio` passed at ratio 1.15. Then the same check across
the **nine seeds the experiments actually sweep** said `audio` passed on **4 of
9** — down from 8 of 9 before the change. The default seed was simply lucky. The
tract drives the larynx harder, the creature babbles more, and it *hears itself*:
`self_gain` mixes its own voice into the room before the cochlea, mel energy
during silence went 0.0499 → 0.0830, and an external vowel stopped lifting the
auditory module by the 10% the experiment needs. **The baby was babbling over
you, and one seed hid it.**

The fix is an **inhibitory `auditory→vocal` projection**: the ears quiet the
larynx. Nothing in the creature can tell its own voice from the room, so this
suppresses babble whenever anything is loud — which is exactly the negative
feedback already described beside `self_gain`, now with its own weight. The
shape is right rather than merely quieter:

| gate weight | none | 0.15 | 0.25 | 0.40 | 0.80 |
|---|---|---|---|---|---|
| audio ratio | 1.15 | 1.34 | **1.46** | 1.73 | 2.13 |
| silence rate | 5.42 | 4.44 | | 3.54 | 2.88 |
| vowel rate | 6.22 | 5.70 | | 5.96 | 6.14 |
| babble duty | 0.68 | 0.57 | 0.52 | 0.41 | 0.29 |

The response to real sound is flat while the silence collapses.

**Strong gates make the creature too quiet to measure.** G2 needs baseline
vocalisations to establish a rate; at 0.40 two creatures returned
"inconclusive: 7 baseline / 43 test vocalisations", at 0.80 five did and G2
failed outright. It ships at **0.25**, which is a peak rather than a plateau —
score any change to it over a neighbourhood.

**The prediction that was wrong.** This should have broken the echo, since
echoing a word means vocalising *while* hearing it. It improves the echo: the
voice carries the heard word at 0.880 against 0.840 without the gate. A quieter
creature has a cleaner readout, and inhibition does not stop it answering.

**Where the pair lands, all nine seeds:**

| | pre-change | tract only | tract + gate |
|---|---|---|---|
| `audio` | 8 of 9 | **4 of 9** | **9 of 9** |
| G2 creatures scored | 9 | 9 | 9 |
| G2 praise won | 7 of 9 | 9 of 9 | **9 of 9** |
| G2 rewarded rate | +0.777× | +0.707× | **+1.361×** |
| G2 F1 advantage | +0.0255 | +0.0556 | +0.0349 |

One more catch on the way, and it is rule 6 of the calibration invariant: with
the gate in, `calibrate` reported **0 modules off target and 8 of 9 seeds wiring
badly** — the new tract overran `auditory`'s `max_out_degree` and was silently
dropping up to 21 synapses per creature. Invisible on the default seed. Raised
72 → 256, which draws no random numbers and was verified inert on its own.


### DNA v34 — peripheral acquisition: the eye was not blind, it was refusing to look

**Shipped 2026-08-19.** New hash `23c4eb2c7c45d05c`. `gazeprobe` had said for a
long time that the v31 reflex recovers 72% of the oracle gain **inside** the
fovea and nothing outside it — vision 0.440 against a `fixed` 0.540, i.e. the
controller actively made things worse — and the standing explanation was that
the retina cannot see out there.

Three things were tried against that explanation and two of them were wrong.

**DNA v33, a spatial aim radius: refuted.** v31 averages every cell above
`gaze_peak_frac` × the peak *wherever it sits*, so peripheral acquisition
failing looked like foveal cells clearing the bar on noise and dragging the aim
back to the centre. Making the neighbourhood spatial does nothing outside the
fovea at any radius, and hurts inside it. It ships off at 0.0, kept with its
table.

**The diagnosis that worked** was to stop scoring the controller and ask what it
believes. Release the eye from the centre, put one fixed-size toy at a known
offset, let it converge:

| toy at | 0 | 2 | 4 | 6 | 8 | 12 | 16 | 24 px |
|---|---|---|---|---|---|---|---|---|
| eye ends | 0 | 1.4 | 3.1 | 5.9 | 7.3 | 11.4 | 15.6 | **REFUSED** |

Acquisition was never broken out to 16 px. At 24 px whole-frame contrast falls
to 0.0225 and the controller **refuses to move**, because `contrast_floor` was
one number doing two jobs: the encoder's per-cell silence floor *and* the
gaze controller's "is anything worth looking at". Perception should be
conservative; acquisition should be twitchy, because a wasted saccade costs one
frame and not looking costs the object entirely. Splitting them is what a
retinotectal pathway is for — the colliculus drives saccades from signals the
geniculate pathway cannot yet resolve into a shape.

`gaze_contrast_floor`, at scatter 0.25 (toy 12.1 px out):

| floor | OUTSIDE vision | gaze err | INSIDE | empty-field drift |
|---|---|---|---|---|
| 0.060 | 0.440 | 11.4 px | 0.870 / 1.3 px | 0.0 px |
| 0.030 | 0.700 | 7.5 px | 0.870 / 1.3 px | 0.0 px |
| 0.020 | 0.740 | 3.1 px | 0.870 / 1.3 px | 0.0 px |
| **0.015** | **0.860** | **2.0 px** | 0.870 / 1.3 px | 0.0 px |
| 0.010 | 0.900 | 1.3 px | 0.870 / 1.3 px | 0.0 px |

Monotone with a plateau, and the INSIDE column is **flat across the whole
range** — this costs nothing where the reflex already worked.

**Why 0.015 rather than the better-scoring 0.010.** The control this lives on is
an empty field, because the floor exists to stop the eye chasing grain.
Empty-field contrast across the nine seeds is 0.0052–0.0064, so 0.015 sits 2.3×
above the worst of it and 0.010 sits 1.6×. The gain between them is inside the
instrument's noise at 100 trials; the safety margin is not, and a real camera is
noisier than this renderer.

**Replicated on all nine seeds:** OUTSIDE vision 0.760–0.940 (mean 0.847 against
`fixed` 0.540 and `oracle` 1.000 — **67% of the available gain, where it was
−22%**), gaze error 1.3–2.0 px, 0.0 px of empty-field drift on every one. No
regression anywhere else: `audio` 9/9, `babble` 9/9, and G2 is bit-identical
because its protocol keeps the toy centred, where the floor never binds.

### Node perturbation on synapses — the last untried rule, and what it closes

**Built, measured and removed on 2026-08-19.** No genome field survives it; the
hash is still `23c4eb2c7c45d05c`. The tables live in `[exploration]` in
`dna/default.toml`.

For months the G3 position had a one-sentence summary: **this creature has two
learning rules and neither can make the voice a function of what the eye is
looking at.** R-STDP can in principle and does not — `dwprobe` splits what
reward writes onto `central→vocal` into ~61% irreproducible noise, ~31%
reproducible but object-independent, ~8% object-specific. Node perturbation
cannot even in principle, because it moves `bias_`, a per-neuron *constant*, and
a constant is not a function of the input.

That sentence names its own repair, and it had never been built: cash the same
perturbation trace onto **synapses** under a presynaptic gate.

    e_ij += k * perturb_j        on every spike of i

This is Fiete & Seung's rule written on weights instead of on excitability, and
in the birdsong model this project borrows from it is the HVC→RA synapse — HVC
says *when*, LMAN supplies the exploratory push, and the synapse active during a
push that paid off is the one that grows. It is conditional by construction:
credit lands only on synapses whose source was firing, so a cube and a ball
write onto different ones without the perturbation knowing anything about
either. It was sampled on the *presynaptic* spike rather than the postsynaptic
one deliberately — reading the target's perturbation only when the target fired
conditions on the very thing the perturbation caused, and turns a zero-mean
exploratory credit into a plain Hebbian one.

**It does something large, and on the wrong column.** `dwprobe`, 3 creatures,
120k ticks, with `c` the fraction of the source module's mean firing subtracted
back off the gate:

| `k` | `c` | mean\|dw\| | corr(A,A′) | corr(A,B) | noise | obj-indep | obj-spec |
|---|---|---|---|---|---|---|---|
| 0 | — | 2.99e-02 | 0.349 | 0.322 | 65% | 32% | **2.7%** |
| 1e-4 | 0 | 5.98e-02 | 0.815 | 0.794 | 19% | 79% | **2.1%** |
| 1e-4 | 1.0 | 3.61e-02 | 0.460 | 0.449 | 54% | 45% | **1.1%** |
| 3e-4 | 0 | 9.30e-02 | 0.917 | 0.910 | **8%** | **91%** | **0.7%** |
| 3e-4 | 1.0 | 5.52e-02 | 0.687 | 0.644 | 31% | 64% | **4.3%** |
| 1e-3 | 1.0 | 9.28e-02 | 0.688 | 0.658 | 31% | 66% | **3.0%** |

The rule cuts the irreproducible share of learning from **65% to 8%** — nothing
in this project had moved that column at all — and every point of it arrives in
the object-*independent* one, 32% → 91%. The reason is one line of arithmetic:
the gate is a presynaptic spike count, a spike count is a neuron's baseline rate
plus a few percent of object, so the credit factorises into a shared term and a
differential one and the shared term is far larger. The same common-mode
swamping as everywhere else in this creature, arriving in one more place — but
here it is arithmetic rather than anatomy, so the mean can be subtracted
*exactly* rather than approximated by an interneuron. Centring does exactly
that, 91% → 64%, and it also gives back the calibration echo the common mode was
costing (g3probe echo 0.700 → 0.925 at `k`=1e-4, against 0.850 shipped) — which
was the falsifiable prediction made before the run, on a quantity other than the
milestone.

**And the object-specific column never leaves the 1–4% band in any of eight
settings.** G3's margin over its own random-target control: +0.017 shipped,
+0.022 at the best arm, mean −0.006 across the grid, 0 of 5 creatures at the bar
everywhere. Above `k`=3e-4 the echo falls under g3probe's 0.700 readout floor
and those arms are *unreadable* rather than negative.

**The control that says why, and it is the part worth keeping.** Run G2 with the
bias half switched off, so the synaptic rule is the only exploration there is:

| arm | rewarded rate | won | G2 |
|---|---|---|---|
| bias half only (shipped) | **+1.742 ×** | 9/9 | PASS |
| synaptic half only | **−0.031 ×** | 5/9 | FAIL |
| neither half | −0.081 × | 3/9 | FAIL |
| both | +1.700 × | 7/9 | PASS |

The synaptic rule sits at the no-exploration floor. **It cannot carry G2 — the
milestone the bias version met outright — so this was never a failure of
conditionality.** Both rules estimate the same gradient from the same reward
stream; the bias version estimates one number per *neuron* and the synaptic
version one per *synapse*, and the variance of the second is hopeless at these
session lengths. Note the last row, too: bolted onto the working rule it makes
G2 slightly worse, 9/9 → 7/9.

Two things that closes, and they are both corrections to standing beliefs:

- **Expressiveness was never the problem.** The reason node perturbation cannot
  make this voice conditional is not that a bias is a constant. Given the
  expressive power, it still does not learn.
- **The SNR framing of G3 is refuted.** `dwprobe`'s 61/31/8 split invited "raise
  the signal or lower the noise". This lowers the noise by 8× and moves the
  object-specific share not at all — so the object-specific part is not being
  *hidden* by anything, and there is no amount of denoising that will surface
  it.

Removed rather than shipped-off because DNA v30 set that policy and this is
exactly the case it was written for: two required TOML keys and two kernel
branches, forever, for a rule that cannot carry the milestone its own ancestor
met. The measurement is the asset; the field is the tax.

### Why G3 does not happen — the synthesis, and the one thing it specifies

Written 2026-08-19, after the last structurally untried learning rule was
refuted. Everything below is already in this file in pieces; what was missing
was the sentence that connects them, and it turns out to name something
buildable.

#### There are three caps, not one, and each is independently sufficient

| cap | what is measured | status |
|---|---|---|
| **delivery** | the object at vocal in the *shipped* creature: **0.660** per neuron across three seed families (0.740 / 0.700 / 0.540), against central's 0.600 | **closed** — see the correction below |
| **conditioning** | condition at vocal 0.732, idealised teacher: taught 0.637 vs random-target 0.639 | five mechanism classes fenced, below |
| **expression** | cube vs ball as a *sound*: d′ **1.20** against a shuffled null of **1.57** | below the instrument's floor |

They are independent. Fixing any one leaves the other two, which is why every
intervention that improved one of them left the milestone where it was.

> **Correction, 2026-08-19, and it is a stale-number bug of the kind this file
> keeps paying for.** The delivery row above used to read "0.500 native", which
> is what `m3probe` measured *before `vision→vocal` shipped*. That tract is in
> the genome now, so "native" has meant something different since. Re-measured
> on the shipped creature: vocal reads the seen object at **0.660** per neuron
> (0.740 / 0.700 / 0.540 across three seed families, chance 0.500, SE ≈ 0.05),
> and central reads it at **0.600** — **the larynx now knows the object better
> than the association module does**, because the direct tract bypasses central
> entirely. Any argument that starts "the object never arrives at the larynx" is
> about a creature two versions old.

#### What is fenced inside the conditioning cap

| class | representative | result |
|---|---|---|
| reward composition | separate neuromodulators (v20), drives zeroed | 0.502 vs 0.498 |
| eligibility distribution | v16 subtract, v17 scale, v18 select | all null; "the distribution is not the problem" |
| reward-independent Hebbian | v19 global, v23 per-pathway | paired never separates from unpaired |
| representation upstream | v12, v13, v14, denser tracts | every one improved central, none moved the voice |
| **exploration on synapses** | node perturbation under a presynaptic gate | **cannot carry even G2** |

#### Two accounts of *why* were offered, and both are now refuted

**The coherence account** — "a random tract preserves coherent codes and cancels
balanced ones, so the tonotopic word survives and the object does not" — is
contradicted by `projprobe`'s own coherence column. Coherence is
`|Σdᵢ| / Σ|dᵢ|`, the share of the discriminative signal that is common-mode:

| | coherence | at source | through a tract |
|---|---|---|---|
| word from auditory | **0.007** | 1.000 | 1.000 |
| object from vision | 0.038 | 0.960 | 0.980 |
| object from central | **0.152** | 0.700 | 0.560 |

The word is the *least* coherent code in the creature and survives perfectly;
central's is the most coherent and does not. Coherence does not predict
survival. Source strength does.

**The SNR account** — "R-STDP differentiates and its output is buried, so raise
the signal or lower the noise" — has now been refuted twice. `a_minus = 0.020`
removes the 5× cancellation penalty and nearly halves the noise; the
object-specific share moves 8% → 10%. Node perturbation on synapses cuts the
irreproducible share from 65% to **8%**; the object-specific share does not move
at all. The object-specific part is not being *hidden* by noise, so no amount of
denoising surfaces it.

#### What the measurements actually leave

Central's object code is **balanced** — coherence 0.024–0.152 across three seed
families, i.e. 85–98% of the discriminative signal is some neurons up and others
down, not a shared rise. And `central→vocal` is an **all-positive random
projection**: every synapse is excitatory, so each vocal neuron computes a
positive-weighted sum of a random subset of central.

That is the one combination that destroys information. A positive random sum of
a balanced pattern averages toward zero — the differential falls as ~1/√n while
the common mode adds as ~n, which is exactly the monotonic decline the density
sweep measured (74% more drive, zero object, and a voice that gets worse). The
word escapes not because it is tonotopic but because it arrives saturated: at
1.000 it can pay the tract's fixed cost and still be legible.

**So the object's code is not destroyed by the tract. It is written in a form the
tract cannot read**, and there is a standard fix for that which this creature
does not have.

#### The specification, and it is buildable with no kernel change

A *signed* random projection preserves a balanced code where a positive one
destroys it. `projprobe` already measures this, label-free, as its `E-I` arm —
two independent random subsets per target, one added and one subtracted, which
is what balanced cortical feedforward inhibition physically is:

| seed | plain | **E-I** | oracle sign-flip |
|---|---|---|---|
| 20260809 | 0.560 | 0.580 | 0.740 |
| 20360812 | 0.560 | **0.760** | 0.600 (control leaking at 0.640) |
| 20451117 | 0.560 | **0.700** | 0.720 |
| mean | 0.560 | **0.680** | 0.687 |

**3 of 3 seeds, mean +0.12, and it matches the label-derived oracle** — which
says the label-free transform extracts essentially everything a supervised
per-neuron sign assignment could. That is the same evidential standard as v24's
in-degree-weighted subtraction (+0.077, 3/3), the one mechanism in this project
that ever measurably worked.

**And it is not what v24 built.** `ffi` subtracts `ffi_gain × pool_fast_[src]` —
the source module's population mean, *one shared scalar for every target
neuron*. That removes the common mode and leaves every target with the same
positive-weighted sum of the remainder. The E-I transform gives each target its
**own independent inhibitory sample**, so 126 vocal neurons form 126 different
signed projections of the balanced pattern rather than 126 copies of one.

The genome can express it today, with no C++: an inhibitory relay module between
central and vocal — `central → relay` and `relay → vocal`, both random and
sparse — gives every vocal neuron an inhibitory input that is an independent
random sample of central, which is the structure the arm measures.

**Two caveats, and they are real.** `projprobe` is the *linear* part of a tract:
no threshold, and nothing else driving the target. The live larynx has a
threshold, 33.9% intrinsic noise, and the arcuate competing for it — and the
lesson of `delivery was never the limit` is that getting the object to vocal is
not sufficient. This attacks the delivery cap, which is the cap already known
not to be binding on its own. It should be built and measured because it is the
only *measured, label-free, buildable* lead on the board, not because it is
expected to close G3 by itself.

#### It was built, and the cap it targeted was already closed

**DNA v35 adds `ModuleRole::kInterneuron`** — a relay population that exists to
be sampled. A role rather than a field: no TOML has to mention it, so unlike a
dead mechanism it costs nothing to carry. `Network::growable()` admits
kAssociation and nothing else, so a relay can never grow, which is the property
it exists to hold fixed. Roles are unique per genome except kAssociation, and
kInterneuron joins that exception: a relay owns no hardware channel, so a limit
of one per creature would be a restriction nothing asked for.
`tools/genome_add_relay.py` builds one in a single command.

The arms are **paired**: the module and both projections append last, and
projection weight is applied *after* the RNG draw, so `out_w = 0` is the same
creature with the same wiring and a silent relay. Both arms were recalibrated to
convergence and both pass `calibrate` and `babble`.

| `out_w` | babble duty | central | vocal per neuron | vocal interleaved | voice |
|---|---|---|---|---|---|
| 0.00 (control) | 0.51 | 0.720 | 0.760 | 0.840 | 0.520 |
| 0.05 | 0.44 | 0.760 | 0.740 | 0.760 | 0.500 |
| 0.10 | 0.38 | 0.760 | 0.760 | 0.700 | 0.500 |
| 0.20 | 0.28 | 0.780 | 0.780 | 0.680 | 0.440 |

**Delivery is flat across a 4× range** — 0.740 to 0.780, every value inside one
standard error of the control — while the interleaved readout and the voice
decline monotonically and the larynx quietens from 0.51 to 0.28. The relay
subtracts; it does not sharpen.

**And the reason is the correction at the top of this section.** `projprobe`'s
E-I arm measures what a signed projection recovers from *central's* code, and
that mattered when `central→vocal` was the object's only route to the larynx. It
no longer is. `vision→vocal` ships, the shipped creature already reads the
object at vocal at 0.660 — above central's 0.600 — and there is no buried
central code left at the larynx for a signed projection to recover. The lead was
real, measured and label-free, and it was aimed at a cap that had closed
underneath it while the notes still said otherwise.

**What survives.** The projprobe result itself stands: a signed projection does
recover a balanced code where a positive one destroys it, 3/3 seeds. What is
refuted is that `central→vocal` is where this creature needs it. If a future
architecture ever has a module whose only route out is an all-positive random
tract, the mechanism and the role are both here and both measured.

### The audibility ruler could not resolve its own question — now it can

**Instrument only, 2026-08-19.** Hash unmoved at `23c4eb2c7c45d05c`, `verify`
byte-identical.

`m3` renders each probe posture through the creature's own tract and cochlea and
reports a d-prime against the creature's own within-word scatter. It also
reported a **shuffled-label null of 1.57 against a signal of 1.20** — a floor
higher than the thing being measured, which means the instrument could not
resolve the question at all. It said so honestly and left it there.

That floor is not a property of the creature and does not need to be measured.
A squared Mahalanobis distance built from two *sample* means is positively
biased: for two identical distributions each coefficient still contributes
`E[z²] = 1/n₀ + 1/n₁`, so

    E[d'² | identical sounds] = D · (1/n₀ + 1/n₁)

which at 12 coefficients and ~20 probes is 2.4, i.e. **d′ = 1.55** — against the
1.57 that was measured. The floor was arithmetic all along, and it subtracts.

Three things were needed to make that work, and two of them were only visible
once the first was tried:

- **Subtract the bias analytically.** One expression.
- **Aggregate in d′², not d′.** Clamping at zero and rooting *per creature*
  before averaging over creatures puts the bias straight back — measured, it
  left the null reading 0.43 instead of ~0. The corrected quantity is carried as
  a signed square and rooted once, at the end. Negative is a real answer: it
  means "no separation, and the sample says so".
- **Estimate the null over 32 permutations, not one.** A single shuffle is one
  draw from the null distribution rather than an estimate of it, and with five
  creatures that was five draws holding up the only reference on the table. It
  read 0.00 at 300k ticks and 0.50 at 900k — *more* data giving a worse null,
  which is the signature. Re-permuting costs nothing: the cepstra are already
  computed and no creature is simulated again.

A fourth was found the day after, by watching the ruler inside `verify-long`
rather than at the length it was developed at. σ is itself *estimated*, and
`E[1/σ̂²] = (1/σ²)·dof/(dof−2)`, so every z² is inflated by that factor before it
is summed. At the ~200 probes of a long run that is a 1% correction and
ignorable — which is why it was waved away — but at the ~20 of a short one it is
12.5%, and the corrected null read **0.58** in the one place the instrument
actually runs most often. Including the term makes the correction exact at every
length:

| corrected null | 300k | 600k | 900k |
|---|---|---|---|
| before | 0.22 | — | 0.00 |
| after | **0.01** | **0.00** | **0.00** |

The null now behaves at any run length, which is what a null must do. And the
reading changes with it:

| | before | after |
|---|---|---|
| cube vs ball | 0.68 | **0.40** |
| shuffled null | 0.71 | **0.00** |
| verdict | unresolvable — signal under the floor | a real separation, 2.5× under the audibility bar |

**The conclusion is unchanged and its status is not.** Cube versus ball was
"below the instrument's floor"; it is now *measured* at d′ ≈ 0.4 against a bar
of 1.0 — the two utterances genuinely do differ, and no listener could use the
difference. The expression cap now has a ruler that can register progress
instead of one that can only report failure.

### The smoothing sweep on a creature with something to hold: null

The experiment this ruler was built for. `[[aibaby-vowel-space]]` parked the
`smoothing_ms` sweep with an explicit trigger — the 800 ms filter is only
destructive because it is blurring noise, so re-run it "on a creature with
something worth holding" — and DNA v32's lateral competition is that creature
(it drops F1 attenuation from 4.2× to 1.6×).

Six arms, corrected d′:

| `lateral_gain` | 800 ms | 400 ms | 200 ms |
|---|---|---|---|
| 0.0 (shipped) | 0.40 | 0.59 | 0.42 |
| 0.020 (v32) | **0.76** | 0.49 | 0.36 |

The v32 row looks monotone and looks like a result. It is not. Three fresh seed
families, both ends, v32 on:

| seed | 800 ms | 200 ms | Δ |
|---|---|---|---|
| 20260901 | 0.58 | 0.51 | +0.07 |
| 20260902 | 0.41 | 0.58 | **−0.17** |
| 20260903 | 0.40 | 0.29 | +0.11 |

**Mean +0.003 and the sign flips.** The 0.76 does not replicate — fresh seeds
give 0.58 / 0.41 / 0.40 at the same setting. Fourth single-seed high to
evaporate in this project, after v32's own acoustic result,
`invariance-not-learnable`'s +0.060 and `critical-period`'s spurious +11.

**What it settles.** Articulator inertia is not what is holding the vowel space
shut, on the shipped creature *or* on one with a bump to hold — so the parked
decision to leave `smoothing_ms` at 800 was right, and now for a measured reason
rather than a cautious one. And no arm of the six is audible: every corrected d′
lands between 0.29 and 0.76 against a bar of 1.0. The expression cap does not
open by making the voice steadier, which leaves it where the v32 work already
pointed — nothing controls *where in the vowel space* each word sits, and that
is the conditioning blocker wearing acoustic clothes.

### The last untested combination — and the noise floor that ends the search

**Measurement only, 2026-08-20.** Hash unmoved at `23c4eb2c7c45d05c`.

One combination had never been run. DNA v19 and v23 built a reward-independent
Hebbian term so a CS could bind to a US without reward naming the object, and
both were refuted — but look at what they write:

```
syn_weight_[syn] += eta_h * credit * sign;
```

`credit` is the **STDP eligibility trace**, the quantity `eligprobe` measured as
object-*weak* on `central→vocal`, and whose conditionality rises a long way when
`a_minus` is moved off its shipped value. So v19 and v23 were classical
conditioning driven by a signal already measured to be nearly blind to the
condition. Each half had been tested; the combination had not.

**The precondition re-measured, and it held** — `eligprobe` at 600k on the
current creature, which matters because `vision→vocal` has shipped since the
original table:

| `a_minus` | central→vocal | shuffled | arcuate (control) | corr(A,B) |
|---|---|---|---|---|
| 0.012 shipped | 0.742 | 0.482 | 0.942 | **+0.938** |
| 0.020 | 0.818 | 0.486 | 0.998 | +0.701 |
| 0.030 | **0.832** | 0.506 | 0.998 | **+0.575** |

Controls lit throughout, and `calibrate` and `babble` are untouched at all three
(duty 0.50, ~300 vocalisations, amplitude 0.479) — so `a_minus` is free at the
operating point, which is not what a global STDP constant usually is.

**The grid, `pairprobe` against `g3probe` at the identical settings** — the
absolute ceiling, because a reward-independent term lifts both arms and the
usual margin is flat by construction:

| arm | `a_minus` | hebb on | rate | PAIRED | unpaired | gap | echo |
|---|---|---|---|---|---|---|---|
| A1 | 0.012 | — | 0 | 0.524 | 0.527 | −0.003 | 0.825 |
| **A2** | 0.012 | vision→vocal | 0.15 | 0.617 | 0.532 | **+0.085** | 0.925 |
| B1 | 0.030 | — | 0 | 0.558 | 0.501 | +0.057 | 0.675 ✗ |
| B2 | 0.030 | vision→vocal | 0.15 | 0.496 | 0.504 | −0.008 | 0.650 ✗ |
| B3 | 0.030 | central→vocal | 0.15 | 0.513 | 0.532 | −0.019 | 0.750 |
| B4 | 0.030 | vision→vocal | 0.05 | 0.479 | 0.487 | −0.008 | 0.675 ✗ |

**The hypothesis is refuted directly**: at `a_minus = 0.030`, where the trace is
most conditional, the echo falls under g3probe's 0.700 floor in three arms of
four and nothing is positive anywhere. A more conditional trace does not buy
conditioning; it buys an unreadable creature.

And A2 — v23's own configuration at the *shipped* `a_minus`, on a creature where
`vision→vocal` now ships — did not replicate. Three fresh seed families, each
with its own hebb = 0 control:

| seed | hebb 0 gap | hebb 0.15 gap | paired contrast |
|---|---|---|---|
| 20260901 | −0.008 | −0.009 | −0.001 |
| 20260902 | +0.045 | −0.039 | **−0.084** |
| 20260903 | −0.070 | +0.008 | **+0.078** |

Mean **−0.002**, sign flipping. **Fifth single-arm high to evaporate here.**

#### The number worth keeping: this metric's noise floor

Read the `hebb 0` column. Those three arms have the mechanism switched off *by
construction*, so their gap is zero by definition — and they measure −0.008,
+0.045 and −0.070, a spread of **0.115**. That is the arm-to-arm noise on
"pairprobe minus g3probe" at five creatures and 141 probes, measured directly
rather than assumed.

It settles three things at once. A2's +0.085 was inside the noise before it was
ever run. Any future gap on this pair under about **0.12** is unreadable at this
sample size. And v23's original ±0.017 table, which was called null on judgement,
was called correctly — the instrument could never have shown anything smaller
than seven times it.

**This was the last named, untested mechanism against the conditioning
blocker.** It is now measured and negative, on a re-verified precondition, with
a replication and a noise floor. G3 is closed under this architecture.

### `restate` — a test for the numbers, not just for the creature

**Built 2026-08-20**, after two documented numbers sent work in the wrong
direction inside three days. Hash unmoved at `23c4eb2c7c45d05c`.

- *"The object never reaches the larynx; vocal is at chance, 0.500."* True when
  written, false since `vision→vocal` shipped. An entire mechanism — the
  interneuron relay — was designed, built, calibrated and measured against a
  bottleneck that had already closed.
- *`eligprobe`'s central→vocal conditionality is 0.654.* It reads 0.742, which
  changed the premise of the experiment it was quoted to justify.

Both were correct when recorded. Nothing noticed when they stopped being
correct, because **a README number has no test attached to it.** `verify` pins
exactly one quantity this way — the determinism hash — and that pin has paid for
itself repeatedly. `restate` is the same idea for the numbers that decide what
gets built next:

```
  quantity                               expected  measured  drift    verdict
  object at vocal, per neuron            0.733     0.733     +0.000   ok
  object at central, per neuron          0.653     0.653     +0.000   ok
  object at vision, per neuron           0.947     0.947     -0.000   ok
  word at vocal, per neuron              0.847     0.847     -0.000   ok
  word at auditory, per neuron           1.000     1.000     +0.000   ok
  central->vocal trace conditionality    0.813     0.813     +0.000   ok
  arcuate trace, size-matched            0.973     0.973     +0.000   ok
```

Four things make it worth having rather than decorative:

**It reuses the probes' own arithmetic.** `m3probe` gained an optional
structured output filled from the same locals its table prints, so the audit and
the table can never disagree about what was measured. A second implementation of
one measurement would drift apart — which is the exact disease this exists to
catch.

**It is pinned to what it reads, not to what the prose says.** The first version
compared the README's three-seed-*family* means against this experiment's three
within-family replicates and carried a systematic offset of up to 0.08 before
anything had drifted at all, spending the tolerance budget on a units mismatch.
The expectations are now the instrument's own readings — the `kPinnedHash` model
— and a `recorded` column says where the prose claim lives so a human reconciles
the two when either moves.

**It is deterministic.** Same genome, same seeds, same trial RNG: two runs are
byte-identical, so a row that moves means the creature moved, not that the dice
did. That is what lets the tolerances mean "how much change is worth hearing
about" instead of "how noisy is this".

**And it can fail.** Zero the `vision→vocal` weight — the exact change whose
*arrival* made the old number stale — and `object at vocal` falls to **0.507**,
the historical 0.500, red on that row alone with everything else green. An audit
nobody has watched fail is a decoration.

It runs in the long tier, and its minimum is 600k because it uses `eligprobe`'s
session, which is blind below that — a drift detector with a blind control would
agree with anything.

### The one rule that did not read the trace — and where the search stops

**Built and removed 2026-08-20.** Hash unmoved at `23c4eb2c7c45d05c`.

The sharpest remaining observation about this creature's learning was that
**every write goes through `syn_elig_`**, a quantity assembled from spike timing
inside ±20 ms windows — and `hebb` is no exception, it multiplies the same trace
without waiting for reward. Central codes the object as a **rate** difference
over hundreds of milliseconds. So every rule ever tried here has been a timing
rule asked to read a rate code, and `eligprobe`'s `a_minus` sweep is that
mismatch showing up as a number: the shipped balance nearly cancels precisely
the rate component.

So: `covar`, a per-pathway **rate covariance**, reward-independent, reading no
eligibility at all.

    dw_ij = covar * (r_i - mean_r(src module)) * (r_j - mean_r(dst module))

It also predicted the one result nothing else explains — the decile test, where
a synapse hanging off central's *most* object-discriminative neuron carries no
more conditional eligibility than one off its least. Inexplicable for a rule
that reads rates; expected for one counting 2.45 coincidences per synapse per
trial.

**It is neither inert nor unstable.** `babble` PASSes from 1e-7 to 1e-1 — six
orders of magnitude — and `dwprobe` shows it writing hard, mean |dw| per synapse
**2.99e-02 → 1.64e-01**. What it writes is the problem:

| `covar` | mean\|dw\| | corr(A,A′) | corr(A,B) | noise | obj-indep | obj-spec |
|---|---|---|---|---|---|---|
| 0 | 2.99e-02 | 0.349 | 0.322 | 65% | 32% | 2.7% |
| 1e-4 | 1.64e-01 | 0.630 | 0.616 | 37% | 62% | 1.4% |
| 1e-2 | 1.82e-01 | 0.349 | 0.308 | 65% | 31% | 4.1% |
| 1e-1 | 1.89e-01 | 0.246 | 0.275 | 75% | 28% | **−2.9%** |

Large, reproducible and **object-independent**. Then the milestone test — both
CS tracts on, three seed families, each against its own `covar = 0` control,
read against the **0.115** noise floor measured the day before:

| seed | paired contrast |
|---|---|
| 20260901 | −0.009 |
| 20260902 | −0.011 |
| 20260903 | +0.050 |

Null, every arm readable, nothing near the floor.

**The diagnosis, and it is the reason to stop rather than iterate.** Centring on
the population mean removes the *population's* offset and not each neuron's own.
`r_i − mean_r(module)` is dominated by the fact that some cells simply fire
faster than their neighbours — a static property of the wiring — and the product
of two static offsets is a fixed pattern: reproducible and object-blind, which
is exactly what `dwprobe` measured. A true covariance would centre each neuron
on *its own* running mean, which needs a second per-neuron array and a snapshot
format bump.

That refinement is named and deliberately not built. This was the **seventh**
mechanism to hit the same wall — a small differential riding on a large common
component — and the seventh time the differential did not move. The stopping
rule was set before the run: clear the measured noise floor on three seed
families or it is dead. It did not, so it is.

Removed rather than shipped off, per DNA v30: `covar` was a required key on
every one of the eleven projections, forever, for a rule that does not learn.
The table is the asset and it is kept on `DnaProjection::hebb`, where anyone
reaching for reward-independent per-pathway learning will land on it.

### M1b — the creature repeats what it hears: **met**

**Built and met 2026-08-20.** Hash unmoved at `23c4eb2c7c45d05c`.

The spec asks one question about the voice — G3, cube versus ball — and this
creature has been failing it for months while doing something else nobody ever
scored. `m3probe` reads the heard word out of the *voice* at 0.86 and the seen
object at 0.58, and the notes have carried the sentence "this creature can
repeat and cannot name" since August. Repeating is a real developmental
milestone. It had no criterion, no control and no bar, so it was never a result.

**What had to be settled first.** `m3probe`'s auditory sweep scores ticks
500–1999 while the word plays 0–899, so **27% of its scored window is concurrent
with the stimulus**. A voice that differs while the sound is still playing is
the arcuate transmitting — a reflex, and an interesting one, but calling it
imitation would be overclaiming. Repetition is what survives the sound stopping.

So the voice is scored in four disjoint windows, five creatures, trial order
shuffled rather than alternating (an alternating sequence lets a classifier
score well on session time alone — this project has been caught by that once):

| window | voice | articulators | shuffled | audible d′ | **EAR still knows** |
|---|---|---|---|---|---|
| WHILE the word plays | 0.896 | 0.828 | 0.499 | 1.84 | 1.000 |
| 0–200 ms after | 0.930 | 0.864 | 0.497 | 1.85 | 0.972 |
| **200–600 ms after** | **0.890** | **0.778** | 0.498 | **1.37** | **0.534** ← scored |
| 600–1400 ms after | 0.704 | 0.660 | 0.497 | 0.71 | 0.482 |

**The EAR column is what makes this a claim about repeating rather than about
hearing.** It is the auditory module on the same trials in the same window. The
caregiver stops at 900 ms and the cochlea and B2 take a few hundred more to let
go — at 0–200 ms the ear still classifies the word at 0.972, so that row is the
stimulus finishing its arrival, not memory. By 200–600 ms the ear is at **0.534**
and the voice is still at **0.890**. The stimulus is gone from the ear and
present in the voice.

**M1b PASS: 5 of 5 creatures at or above 0.75**, the same bar G3 is scored
against, with the shuffled control at chance in every window. Two things stop it
being a technicality:

- **Articulators alone read 0.778.** That column drops loudness and voicing
  entirely, so this cannot pass on "one word makes it louder" — it is a claim
  about two *sounds* rather than two amounts of sound.
- **Audible d′ 1.37, against the 1.0 a listener needs.** This is the first time
  anything this creature does has cleared the audibility bar. Cube versus ball
  is 0.40 on the same ruler.

**The window is fixed a priori, and the first version got that wrong.** It
picked each creature's best window subject to the ear being at chance, and two
of five then "failed" only because their ear decayed slightly slower and the
rule fell through to a later window. Choosing the window once, in advance, for
everyone is the difference between a milestone and a search. The first version
also took its shuffled control from a *single* permutation, which failed the
whole experiment on one window at 2 SE — the same one-draw-is-not-an-estimate
error the audibility ruler needed fixing for two days earlier. Both are now 16-
and 32-permutation averages.

**Read it beside G3, same creature, same classifier, same bar: it repeats at 89%
and names at 53%.** The object reaches the larynx (0.66, above central's 0.60)
and does not reach the voice. That contrast is sharper than either number alone,
and it is the honest headline for what this architecture built.

#### Is it repeating, or transmitting one loud spectral axis?

M1b is scored on /a/ versus /i/, which differ hugely on **both** formants — a
creature transmitting nothing but "how bright was that" would pass it. So two
more words were appended to `kWords` (appended, never reordered: every other
experiment indexes 0 and 1 by name-of-object) and all six pairs scored off *one*
simulation in the same window:

| pair | voice | EAR |
|---|---|---|
| /a/ ball – /i/ cube | 0.933 | 0.567 |
| /a/ ball – /u/ boot | 0.880 | 0.540 |
| /a/ ball – /e/ bed | 0.753 | 0.593 |
| **/i/ cube – /u/ boot** | **0.900** | 0.540 |
| /i/ cube – /e/ bed | 0.873 | 0.573 |
| /u/ boot – /e/ bed | 0.880 | 0.473 |

**All six clear the 0.75 bar.** The decisive row is /i/–/u/: their F1s are 30 Hz
apart and their F2s 1600 apart, so it can only be answered on F2 — and it reads
0.900, as high as the easiest pair. **It is not one axis.** The weakest pair is
the adjacent-vowel one, /a/–/e/ at 0.753, which is where a weakest pair should
be. Scoring every pair from the same trials means differences between rows are
about the two vowels rather than about the run.

#### Does it survive a microphone?

Everything above plays a synthesised vowel straight into the cochlea. This adds
broadband noise and per-trial level variation — a measurement-layer model, not a
genome one, for the same reason `Retina::Servo` is not in the genome: it
describes the world the creature is measured in, not the creature.

| condition | voice | shuffled | EAR | audible d′ |
|---|---|---|---|---|
| clean | 0.937 | 0.508 | 0.560 | 1.54 |
| SNR 20 dB | 0.910 | 0.511 | 0.623 | 1.78 |
| SNR 10 dB | 0.913 | 0.506 | 0.583 | 1.06 |
| **SNR 0 dB** | **0.807** | 0.508 | 0.530 | 1.07 |
| ±6 dB level | 0.870 | 0.500 | 0.603 | 1.41 |
| 10 dB & ±6 dB | 0.847 | 0.501 | 0.567 | 1.09 |

**It survives, and degrades gracefully.** At 0 dB SNR — broadband noise as loud
as the word itself — the voice still carries which word it was at 0.807, above
the bar, with the shuffled control at chance and the audible d′ still over 1.0.
The noise is referenced to the word's own amplitude rather than to the buffer's,
so it keeps playing through the silent tail this experiment scores; referencing
it to the buffer would have made the room go quiet exactly when the talker did,
which is the flattering version and not the real one.

#### The bug this found, which was not in the new code

The four-word session crashed in `free()`, three frames away from its cause.
`holdout_accuracy` is a **two**-class classifier that indexes a size-2 array with
the label and never checked it — so a four-way label set walked off the end of
`centroid` and corrupted the heap. It has been that way for the whole project
and nothing had ever passed it a label outside {0,1}. Both classifiers now
refuse out-of-range labels rather than corrupting memory: silently clamping
would have been worse, because it would score a four-way problem as a two-way
one and report a number.

## Design decisions that were not obvious

These were all discovered by measurement, and each one is the difference
between "learning does not work" and "learning works".

**A module's neuron indices are its channel map, so growth cannot touch a
transducer.** Every encoder and decoder slices the live range into equal
contiguous pieces — mel channel *c* is whichever neurons currently fall in the
*c*-th slice. That makes `count` load-bearing in a way nothing about it
advertises: adding one neuron to the cochlea renumbers all twenty-four channels
and invalidates every weight downstream of them. Growth adds capacity where
capacity means something and cannot represent anything, which is association.
The same aliasing is why pruned neurons are tombstoned rather than compacted
out.

**Uniform downscaling needs a floor, because nothing awake undoes it.** §3.6
asks sleep to downscale, and it is what makes pruning selective. But synaptic
scaling is silent inside its band and reward-modulated STDP is signed, so no
waking mechanism restores total weight and a 0.98 multiplier per sleep bout
compounds over a life. Each neuron therefore tracks what its *structure*
entitles it to — birth, plus growth, minus pruning — separately from the
scaling setpoint, and downscaling stops at 60% of it. Sleep can weaken what
experience did not reinforce; it cannot erase the creature.

**Synaptic scaling must be a bound, not a setpoint.** §3.1 asks for scaling
"to keep total input weight bounded". Implemented as a proportional pull
toward a setpoint it regulates precisely the quantity reward-modulated
learning has to move, and it wins: with a setpoint the praised and yoked
babies were indistinguishable. With a dead band — silent inside
`[birth/3, birth×3]` — they separate. Intrinsic plasticity, by contrast,
*helps*: it keeps the network in a regime where weight changes still translate
into rate changes.

**Homeostasis must be slow relative to learning**, acting over minutes rather
than seconds, and each module's target rate should sit at its free-running
rate so homeostasis idles at the operating point instead of driving it.
Before this, the entire network was silent without homeostasis (central module
at 0.05 Hz) and every rate you could see was homeostasis pumping thresholds
down.

**Learning follows reward prediction error, not reward.** Without subtracting
a running expectation, the mean reward multiplies the mean eligibility and
every synapse in the brain drifts together. That is motion, not learning.

**The motor readout needs a fast rate estimate and homeostasis needs a slow
one.** They are different questions and one EMA cannot answer both: reading
the motor groups through the one-second homeostatic average low-passes the
creature's entire output to about 1 Hz.

**A vocal tract has two inertias.** The articulators are heavy — a posture is
held for the length of a syllable — and that persistence is also what lets a
delayed reward find the behaviour that earned it. The glottis is a valve and
opens in tens of milliseconds. Giving both the same time constant makes the
creature either drone continuously or lose its posture between the sound and
the praise.

**Reward-modulated STDP can find degenerate solutions.** Scored on the
*share* of vocalisations that were of the rewarded kind, the baby learned to
fall silent: three sounds, all of them correct. G2 is scored on frequency for
exactly this reason, and going quiet is the one thing that cannot satisfy it.

**Inhibitory synapses are not plastic by default.** Potentiating an
inhibitory synapse means more inhibition, so if inhibitory activity correlates
with reward the module shuts itself down with no restoring force.

**Changing one projection's density rewires the ones after it.** Every synapse
that lands draws its own delay jitter, so widening vision→central shifted the
shared RNG stream for every projection wired later, and four expression neurons
ended up with more incoming synapses than the reverse index could hold. A
dropped reverse entry is the worst kind — that synapse still depresses but can
never potentiate — and the only symptom was a line in the startup banner.
Expression's `max_out_degree` went from 32 to 48.

**And it does it again, in one seed out of nine.** M3 widened the same
projection to 0.06 and the same failure landed somewhere new: the busiest
somato neuron overran its 32 outgoing slots and lost four synapses, in seed 7
alone, with nothing about touch having changed. It survived into a full G2 run
— the warning is printed per session, so it scrolled past in the middle of a
results table, and two of the eighteen creatures in that table were not the
brains the genome describes. Finding out *which* module cost an hour of
instrumenting a build, so the warning now names it: both banners print the
offending module, its current cap, and how many synapses it lost, because a
total on its own sends you to read the projection you changed last and the cap
that overflowed belongs to a module you did not touch. Raising a cap draws no random numbers, so the fix
changes the one overflowing brain and leaves the other eight bit-identical;
`somato.max_out_degree` is now 48. The general lesson is that this class of bug
is *seed-dependent*, so a genome edit that looks clean on the default seed can
still be quietly wrong on one creature in nine — check the banner across the
seeds an experiment actually uses, not just the one you ran by hand.

**The panel used to die whole.** `app.js` wired `$('camera').onclick` at load
time; on any page without that element it threw before `connect()` ran, so the
socket never opened and *every* control was dead — and the one you happen to
reach for is the one you report as broken. It surfaced as "the microphone no
longer works". A missing element now costs you that panel and nothing else, and
an uncaught error is painted into the status line rather than swallowed: a
panel that fails silently is worse than one that fails loudly.

**A control you hold must not resize under your hand.** Push-to-talk closed
about four milliseconds after it opened, and holding the button sent no sound
at all. Pressing it relabels it from "hold to talk" to "listening…", which
makes it 16 px narrower, which let the wrapped row of buttons fit on one line,
which moved the button up beside its neighbour and out from under a finger that
had not moved. `mouseleave` fired and muted the microphone again. The release
now hangs on pointer capture rather than on the pointer still being over the
button, so the button's geometry stops mattering, and the buttons that relabel
themselves are pinned at load to the width of their widest label. The general
shape of it: a widget whose own state change alters its hit area cannot use
"the pointer left me" to mean "you let go".

Worth noting how it hid. The headless test dispatched a synthetic `mousedown`
at the element, which engages push-to-talk and leaves it engaged — there is no
real pointer, so nothing ever leaves. It reported PASS throughout. Only
injecting input through `Input.dispatchMouseEvent` at the button's screen
coordinates reproduces it, because only then does the browser re-run hit
testing when the layout moves. Synthetic events test your handlers; they do not
test whether a person can reach them.

**A membrane time constant is a decision about what the senses are.** Central
integrated with a 20 ms constant, inherited from the other modules and never
questioned. But the retina does not send levels, it sends volleys — each cell
once per frame, early if it responded strongly, spread across a 60 ms latency
window — and integrating for 20 ms of that window converts the volley's *order*
into an *amount*. The module was being handed a pattern and computing a sum.
Cutting it to 5 ms turns those neurons into coincidence detectors and moves M2's
honest score, the one with firing rate divided out, from 0.69 to 0.79; it also
takes G2 from 2-of-9 to 6-of-9, which nothing aimed at G2 had managed. The
sweep is only readable because the threshold was re-tuned at every step to hold
the module's free-running rate at 8 Hz — otherwise "integrates for less time"
and "fires less" are the same experiment. The general shape: a time constant
copied between modules is an assumption about what arrives at each of them, and
two senses that encode differently should not integrate identically.

**A display fade constant is a frame-rate constant.** The retina reused the
cochlea's 0.9-per-tick fade. The cochlea gets a frame every sixteen ticks and
the retina every hundred, so between one camera frame and the next the eyes
read as shut, and any jitter in browser frame delivery made the retina view
flicker. It is now derived from the frame period, so "fades over about three
missed frames" means the same thing at either rate.

**A drive that only accumulates is not a drive.** Fatigue rose with activity
and had nothing to discharge it, so it saturated and the creature carried on
exactly as before. The bug was invisible to every experiment — they all run
well under the ~17 minutes it takes to reach the threshold — and was found by
someone leaving the thing running. Both other drives have a restoring term;
fatigue's is a state, not a constant, which is why it was the one that got
missed.

**A save point on a round number hides half of what a snapshot has to carry.**
The resume test passed with the auditory encoder's restore deleted outright.
Mel frames arrive every 10 ticks and camera frames every 100, and the save was
landing at exactly `ticks/2` — so the first tick after the resume overwrote
every level the encoder had been holding, and the fading, the hold countdown and
the retina's half-finished latency volley never got a chance to matter. Nothing
about the snapshot was wrong; the test simply could not see that part of it. The
save is at `ticks/2 + 7` now. Anything sampled on a schedule needs its tests
taken off that schedule's beat.

**Restoring a brain is a rebuild, not a parse.** The arena is a bump allocator
fed only by sizes from the genome, so hatching the creature again lays every
array back at the same offset. That makes the snapshot the arena's bytes plus
the few fields outside it, with no pointers in the file and no fixups on load —
and it makes the format's rule ("only against the genome it came from") a
consequence of the design rather than a policy that has to be enforced by hand.

## Layout

```
core/     libaibaby — C++17, no exceptions, no RTTI, no STL, no I/O, no threads.
          Arena allocated once at birth; nothing allocates afterwards.
host/     Desktop only: TOML→DNA compiler, cochlea, journal, snapshot files,
          HTTP+WebSocket, headless experiments.
web/      Panel, microphone worklet, formant synthesiser, avatar.
dna/      Genomes.
```

The core never sees a socket, a file, or a thread. The host is thin and the
browser is a peripheral, which is what keeps the ESP32 port (M5) a build
question rather than a rewrite.
