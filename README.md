# aibaby

## AI Baby

A creature you raise. It is born from a compact DNA file that defines only its
initial brain structure; everything else it becomes comes from what happens to
it. See [requirements.md](requirements.md) for the specification.

There is no pretrained data anywhere in this repository. Nothing is downloaded.

## History

A few years ago (2023) I decided to spent my time during a nice vacation in 
Corsica (France) to write a modern version of the Tamagotchi. I watched all videos
on YouTube on how a brain is formed and develops, very interesting but I had limited results.
(Yes, I am a very bad holiday partner as I bring my laptop, learning something gives my
brain the best long term reward).
Now I thought it would be a good moment to see if modern LLM's could help and do better so
I gave Claude Code my code. It was not easy to get it to help. It kept pointing at
the Claude API I should use.
The words ***Tamagotchi*** and ***offline*** are the magic word to get it to
help. Here is the first attempt, it works better than I expected!
Update: By now this has become more of a science experiment than a project with a purpose ;-).

## Build and run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/aibaby                       # then open http://localhost:8080
```

The host serves the panel and the WebSocket on the same port, so there is
nothing to install and nothing else to start.

![The panel at milestone 2: counters, structural growth, the spike raster, the
baby and its buttons, ears, eyes, voice and reward, a central neuron's membrane
potential, and the drives.](screenshot.png)

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

## Driving the baby by hand

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
./build/aibaby --experiment stpprobe    --ticks 120000  # DNA v36: is a dynamic synapse a filter here
./build/aibaby --experiment burstprobe  --ticks 600000  # DNA v37: is there a burst code, and does the tuft steer it
./build/aibaby --experiment pruneprobe  --ticks 120000  # DNA v38: is competitive pruning selective, or just large
./build/aibaby --experiment tauprobe    --ticks 240000  # DNA v39: does a per-module eligibility tau do anything
./build/aibaby --experiment ipprobe     --ticks 240000  # what §3.1's regulator costs the rate code at the ear
./build/aibaby --experiment mechverify                   # a pinned hash for every mechanism that ships off
./build/aibaby --experiment errprobe    --ticks 600000  # DNA v40: does the tuft learn to carry an error
./build/aibaby --experiment relayprobe  --ticks 240000  # Webb's two-stage circuit; needs a genome with a relay
./build/aibaby --experiment vocallearn  --ticks 3400000 # does the echo improve with feedback (OPEN)
./build/aibaby --experiment teachsound  --ticks 3400000 # M1c: teach it a vowel; --wav to hear it
./build/aibaby --experiment retain      --ticks 5600000 # does it keep the lesson; does sleep erase it
./build/aibaby --experiment capacity    --ticks 5600000 # can it hold two lessons at once
./build/aibaby --experiment credit      --ticks 5600000 # would per-neuron reward remove the interference
./build/aibaby --experiment driftprobe  --ticks 3400000 # is the interference credit, or variance
./build/aibaby --experiment metaprobe   --ticks 5600000 # does DNA v41 buy what the oracle bought
./build/aibaby --experiment trajprobe   --ticks 600000  # does an utterance have a shape
./build/aibaby --experiment seqprobe                    # can any module hold a sequence
./build/aibaby --experiment snapshot    --ticks 2400000 # §8: resume is exact
```

`v1probe` exits non-zero on the shipped genome by design — it has no visual
cortex to probe. It is there for a genome that turns one on.

### Experiments run across the cores, and the numbers do not change

An experiment raises nine creatures, sometimes across six arms, and until
2026-09-06 it raised them one at a time on one core of fourteen. `parallel_reps`
in `host/src/experiments_common.h` runs them together.

The point is that this is **bit-identical**, not merely equivalent, and that
rests on three properties rather than on hope:

1. `core/` has no mutable globals — the only `static`s in it are three functions
   in `brain.cpp`. Every creature is an `Arena`, a `Network` and an `Rng` passed
   in explicitly, and `Session` owns its arena as a member. This is what the
   `-fno-exceptions -fno-rtti` contract has been buying all along.
2. A rep's seed is `header.seed + r * 7919` — a pure function of the rep index,
   so execution order cannot reach the numbers.
3. Distinct elements of a `std::vector<T>` may be written concurrently.

So a converted experiment still prints its rows and pools its statistics
**serially** from the returned cells. Sharding across processes instead would
need something outside the binary to pool the rows into a verdict — a second
implementation of the verdict, which is the trap `restate` and the
fitted-verdict rule exist to prevent. `ctxfour` is converted, and its whole
output is byte-for-byte the serial log's.

**The ceiling is 3.2x, not 14x.** `ctxfour`'s 18 sessions at 200k ticks, measured
on an idle machine — the first attempt at this table was taken while a 6.8M run
was consuming cores throughout, which inflated the serial baseline to 166 s and
the ceiling to a flattering 3.7x:

| threads | wall | speedup |
|---|---|---|
| 1 | 99.0 s | 1.00x |
| 6 | 41.7 s | 2.38x |
| 14 | 32.2 s | 3.08x |
| 18 (default) | 30.9 s | 3.20x |

**Capacity is not the constraint and never was** — peak RSS is 2.0 GB of 61 GB,
so there is no reason to throttle job counts to save memory. What is scarce is
memory *bandwidth*: a session streams a ~113 MB arena past a 12 MB L3 on a mobile
hybrid part, so cores wait on DRAM and threads stop paying at about three.

**And running separate processes hits the same wall** — asserted first, then
measured, because it is the kind of claim that follows plausibly from a bandwidth
limit and could still have been wrong. Three concurrent processes at six threads
each did 54 sessions in 93.4 s, against 92.8 s for the same 54 sessions done as
three sequential single-process runs at 18 threads. **3.18x against 3.20x**: the
machine does the same work per second however the work is divided.

That still takes `ctxfour` from 75 minutes to 20, and a six-arm run from 3h40 to
about an hour.

The default deliberately over-subscribes — one round of `n` threads whenever
`n <= 2 * hardware_concurrency` — because on a memory-bound job an idle tail
costs more than over-subscription does: 18 jobs on 14 threads is a full round
plus a tail of four, and it lost to 18 threads by 20%. `AIBABY_JOBS` caps the
pool for running several experiments at once.

The other rep loops are unconverted. Convert on demand, and diff a short run
against a serial reference captured **before** the edit every time — that diff
is the whole warrant.

**Spend it on `n`, not on arms.** The smoothing sweep closed on three seeds with
unanimous signs collapsing at n=6. More arms at fixed `n` makes the
multiple-comparisons problem worse exactly where this project has already been
burned; `kReps` 9 -> 24 now costs what one run used to.

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
`vision->vocal` — the tract that carries the seen object; it read
`central->vocal` alone until that was found to be a non-participant — at the
same phase of every trial and asks a held-out classifier which object the
creature was looking at. That is the quantity R-STDP actually
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
| **M1c** taught vocalisation | **met** — praise alone moves the creature's vowel toward a target it never hears: error down **+19.0 points** against its own yoked control and the change is audible at **d′ 7.50** (null −0.03), 3 of 3 seed families — re-measured after M1d shipped; it was +15.9 and d′ 5.57 before. The first taught change to *what* this creature says rather than how often |
| **M1** closed audio loop | **done — G2 met.** Rewarded vocalisations rise within the session (×1.35) and praise beats its own yoked control in **23 of 27 creatures** across three seed families, and 9 of 9 at 420 s. What closed it was directional exploration (DNA v10), which was not aimed at reward at all |
| **M2** vision | **done** — camera → retina → B3 → B1, discriminates present from absent at 98%, and 86% with firing rate divided out |
| **M3** cross-modal association | **CLOSED, NEGATIVE** on the vision→voice form. Eleven mechanisms across four structurally different families were measured against it and it never moved outside its own noise floor: the shipped creature reads taught−random **+0.060 ± 0.040 SE**, and the control genome — mechanism absent by construction — swings ±0.060 across seed families. **Its "does not acquire a new conditional map" reading is superseded**, see the naming row below: with a context-indexed bias the creature *does* acquire one, from the ear rather than the eye |
| **Naming (G3)** heard word → spoken word | **DIRECTIONAL NAMING MET; ABSOLUTE NAMING CEILINGED.** On the axis measure the creature moves its voice the right way for the word it just heard — **0.824 and 0.738 across two seed families, pooled +0.276 (7.6 SE) over echo-only and +0.244 (6.0 SE) over a matched-marginal control** — on a measure an echo scores at 0.503 and an arbitrary-but-consistent mapping at 0.406, both closed by construction rather than by argument. It also partly arrives: 0.670 and 0.604 on the strict nearest-of-actual-targets score against 0.495–0.518 for both controls. What is **not** met is absolute naming: delivered dF1 tops out near **137 Hz** against the ~230 needed, and that ceiling now has a cause rather than a shrug — see the row below. **AND CONDITIONALITY IS NO LONGER THE BLOCKER** (2026-09-14, `orthoname`, 36 seeds, contexts on): on a two-word vocabulary sitting 0.29–0.41 log units from rest, the creature lands nearest the **right** target **0.958** of the time against matched marginals at 0.389 and 0.542 — excess **+0.569 ± 0.078 (7.3 SE)** and **+0.417 ± 0.068 (6.2 SE)**, paired on seed. It says the right thing for the word it heard. That reconciles what looked like a contradiction on this page: `g2cond`'s 12σ closure (unconditional +0.369, conditional +0.022) was measured **without** the context-indexed bias; with v51/v53 contexts on, conditional steering works. **This is not the milestone**, and the reason names the wall precisely: these targets are *symmetric* about rest and 0.58 apart, while the shipped vowel vocabulary is 0.89 apart and asymmetric, demanding a 0.67 move on one word alone. Naming works here because the required **travel** is small. **THE REWARD IS BLIND TO HOW FAR THE TARGET IS — AND THAT IS NOT WHY NAMING CEILINGS** (2026-09-14, `travelsweep` then `absbar`, 36 seeds). Sweeping target separation produces **bit-identical creatures** past ~0.6 log units: `0.89 == 1.20` on **0/36 seeds differing**, because the reward is judged against an EMA of the creature's *own* error, so while it never crosses its target a further target adds only a **constant** — which cancels exactly. Past that point **the creature is not refusing to travel; it is never asked to.** *I concluded from this that the ceiling was the teaching protocol rather than the larynx, and then tested it and it was wrong.* `absbar` built the one replacement rule the algebra leaves standing — an absolute **quadratic** bar, whose drift scales with distance — and it did exactly what was derived: distance visible **36/36**, still learning at **11.5 SE**. It was also **worse**: 0.180 delivered against the blind rule's 0.291 near, and **−9.4 SE** at the far separation. The blindness is real and it is **not the binding constraint**. See the section below. *Two retractions from that run, both now settled off its own per-seed log:* it first reported the colliding encoding beating the orthogonal one at 8.4 SE, which was an artefact of scoring by **margin** — collinear targets inflate the margin ~2× for identical learning (predicted 1.8–1.9×, observed 2.03); on the geometry-free fraction that gate reads **−1.5 SE**, i.e. no difference, which confirms the rewritten verdict that was never re-run. And this page carried **~16 SE** for the excess above: that was the *margin* column's SE, attached to a quantity it was not computed on. The fraction's own paired SEs are the 7.3 and 6.2 now quoted — decisive either way, but not 16 |
| **The naming ceiling** | **CLOSED — it is one exponent, not a tally of dead ends.** `dF1 ~ aligned^0.61`: the larynx compresses what it is given, and the same curve does two jobs — it limits delivery, and its derivative limits learning, since the drift is `−Cov(e, perturb)` which factors through `dF1/d(drive)`. **DNA v58 settled it by dissolving the trade it was supposed to be stuck on.** Exempting the learned bias from the rate homeostat (`err = rate_ema − (target + gain·delivered_bias)`) raised the aligned bias **+21%**, raised `gain` 0.90 → 1.08, and kept the larynx regulated (5.87 Hz vs 6.23) — where v57 pooling *halved* the bias and `ipoff` collapsed learning outright (103.9 → 17.7, −7.42 SE). So regulation and learning are separable after all. **And it delivered +0.6 ± 8.6 Hz**, because `^0.61` turns +21% of aligned into **+4 Hz** — a bar needing ~316 seeds to resolve, against the 18 that ran. The arithmetic is the result: dF1 ×1.68 needs aligned **×2.34**, ×2.0 needs ×3.12, and every knob this project has found moves aligned ×1.1–×1.2. No amount of bias buys past an exponent below 1; only a change to the **readout** can, and the readout is a rate-weighted centroid by design. Naming stops here |
| **M4** growth and sleep | **done** — **G4 passes**: the creature never grows while it is still learning, grows only on a detected plateau, and never passes the DNA cap; myelination, pruning and replay all run |
| **Memory and interference** | **THE WIPE IS A COLLISION, NOT A MEMORY FAILURE** (2026-09-14). A conflicting second lesson wipes a taught sound to **0.22** (`retain`) — but that is two lessons steering one scalar, not a failure to remember. With lesson A taught identically in every arm and only the *second* lesson's **axis** differing at the same log-distance, a second lesson on **F1** costs +0.1603 (16.8 SE) and drops retention to 0.23; one on **F2** costs +0.0364 (3.2 SE) and leaves retention at 1.12 with the store intact — **77% less damage, +0.1238 ± 0.0116 (10.7 SE)**. The control is *structural*: `err taught` is **+0.00000 ± 0.00000** across arms because the teaching phase is the same code path, and `collide` reproduces the long-established 0.22 at 0.23. The effort check runs **against** the finding — ortho's lesson learned *better* by 14.9 SE and still damaged A less, so a confound could only have manufactured this had it learned *less*. (ortho is not harmless: 3.2 SE is real damage, just much less.) **THERE IS NO HIDDEN MEMORY.** Store and behaviour, measured on one creature and both as a fraction of what was gained: **0.25 ± 0.05 and 0.30 ± 0.05, difference −0.05 ± 0.05 (−1.0 SE)**, 36 seeds none dropped. They decay together, so the conflict is a **pure overwrite** and retrieval work is refused — including the per-neuron-homeostat account. **AND IT CANNOT BE DEFENDED.** Replay reinforces nothing (`E[u]=0` — during sleep `perturb_i` is fresh noise; switching it off costs 0.3 SE); interleaving is refused with a *perfect* oracle (−1.1 SE); stabilisation is a clean null (`meta_commit`); and **write separation by restricting plasticity is refused** — blocking even a *quarter* of the F1 group costs 59% of the lesson, because a blocked neuron still perturbs and is still read by the centroid. A pooled readout cannot be partitioned by restricting plasticity alone. **BUT THE COLLISION RESULT IS ABOUT SEQUENTIAL LESSONS ONLY.** `orthoname` tested the representational implication — spread the vocabulary across articulators — and it **does not transfer** to simultaneous conditional naming: orthogonal and colliding encodings name identically (0.958 each). Interference between lessons taught one after another, and conditioning on the heard word, are different problems. *Corrections on this row:* an early pass reported the store keeping 0.65, a **raw** `gap/teach` ratio rather than a fraction of what was gained (the same correction takes 0.58 → 0.25 on baseline); and the "a perfect index does not protect it" verdict rests on retention at **1.99 SE against a 2.0 bar** while `err after` favours the index at 14.56 SE — prefer `err after`, since retention's denominator (before − taught) collapses and printed 6.12 ± 2.74 there |

### Reading this page

It is long, and it is a working journal rather than a narrative: sections below
supersede earlier ones **in place**, and several claims were published and then
retracted within a day. That history is kept deliberately — a refuted idea with its
refutation attached is worth more than a tidy page, and more than one conclusion
here was reached twice in opposite directions.

So if you want the state rather than the story:

- **the table above is current**, and is the only part of this page guaranteed to
  be — everything below it is dated and some of it is superseded
- **what is closed and why** — the naming ceiling is *one exponent*,
  `dF1 ~ aligned^0.61`, not a tally of dead ends; see the ceiling row
- **what to read before touching G3** — that exponent and its two jobs. It limits
  delivery, and its derivative limits learning, since the drift is
  `−Cov(e, perturb)` and factors through `dF1/d(drive)`. Everything that failed,
  failed for that reason
- **what the memory chapter settled** — the wipe is a *collision*, not a memory
  failure; see [The memory chapter](#the-memory-chapter-and-what-closed-it)
- **what is still open** lives in the memory index, not here, because it changes
  faster than this page does

Where a later section contradicts an earlier one, **the later one is right** and
usually says so explicitly. Claims here are retracted often enough that the
retractions are part of the record: the 2026-09-13/14 session alone retracted six,
four of them by the run queued to test them. In every case the retraction sits in
the commit that superseded the claim rather than somewhere quieter, and the
corrected number replaces the old one *in place* rather than being appended after
it.

Every number on this page comes from the genome in [dna/default.toml](dna/default.toml)
as it currently stands. **The whole suite above passes except `m3`** — G1,
calibrate, babble, audio, vision, M2, G2, sleep, G4 and snapshot are all green,
and G3 is closed negative rather than open — see the row above and
[Why G3 is closed](#why-g3-is-closed-and-what-would-have-to-be-different).
Shipped hash `ad96f882becbee92`;
`--experiment verify` reads 20 of 20 as expected on the fast tier and
`verify-long` 40 of 40 and `verify-teach` the seven hour-scale ones — see [The suite, both tiers](#the-suite-both-tiers).

**What teaching can and cannot do is now measured rather than guessed.** Three
experiments bound it. `teachsound` (M1c) says praise alone moves the voice toward
a target it never hears, audibly. `retain` says the lesson is kept, keeps
improving, and **sleep does not erase it** — the first time any part of §3.6 has
been shown harmless to a learned behaviour, against a hostile prior. `capacity`
says the creature holds **two** lessons at once on orthogonal output dimensions
(0.84 of the first kept while the second lands), which **corrects** this page's
earlier "one lesson at a time" reading of `retain` — that collapse was two
lessons competing for the same formants. The remaining limits are real and
narrow: the two dimensions share one reward channel and compete for it (0.58
against a continued single lesson), teaching both at once is worse than teaching
them in turn, and every target is still *fixed* rather than conditional on what
the creature heard — which is `vocallearn`'s +24 against −0.1 and the reason G3
is closed.

**DNA v36–v39 are four mechanisms built since, and all four ship off**, so none
of the numbers above moved: dynamic synapses after Webb's cricket (v36),
burst-dependent plasticity after Payeur et al. (v37), competitive pruning (v38),
and a per-module eligibility timescale after e-prop (v39). Each has its own
probe. The two that produced findings worth acting on are **v37**, where the
apical tuft steers bursting 2.6× and the resulting third factor discriminates
the object at 0.673 — the first one here that does anything but — and **v38**,
which prunes 20,731 synapses where the shipped rule prunes 32. The two that
produced corrections are recorded as such.

**G3 is open but no longer a live line of work.** The `vision→vocal` tract
below raised delivery to the larynx by 27% and moved the milestone by −0.014,
which is the third independent measurement saying the same thing: the object
arriving is not what G3 was ever waiting on. Both of the creature's learning
rules are ruled out as ways to supply the missing conditionality — node
perturbation writes a per-neuron constant rather than a function of the input,
and reward-modulated STDP's object-specific share of what it writes is about
8%. Clearing 0.75 needs a third learning rule, which is a new architecture and
not a fix.

### Borrowing the pathway that works — measured, and it does not transfer

The one idea left that was not another learning rule: **stop trying to build a
second conditional pathway and route the object through the one that already
works.** The creature repeats a heard word at 0.890 (M1b). If the seen object
could evoke the *word's* activity in the auditory module, the innate arcuate
would say it, and G3 would need no conditional reward at all — only Hebbian
association between two co-active sensory codes, which is the one learning
problem this architecture has never been asked to solve.

`m3probe` states the case in two rows of one table — same creature, same
larynx, same 100-trial instrument:

| the condition is delivered to | that module reads | → **the voice reads** |
|---|---|---|
| `auditory` (a word, empty field) | 1.000 | **0.920** |
| `vision` (an object, in silence) | 0.980 | **0.460** |

and it also shows the missing edge: in the object block `auditory` sits at
**0.480**, chance. The seen object never reaches the ear's module, and no probe
had ever read that row — the genome has `vision→central`, `vision→vocal` and
`central→auditory`, and no `vision→auditory` at all.

So it was built (`tools/genome_add_tract.py`, genome-only, appended last so a
weight-0 arm is the same wiring). **Three independent manipulations, all
negative:**

| manipulation | object in `auditory` | **voice** |
|---|---|---|
| no tract (control) | 0.420 / 0.520 / 0.380 | 0.580 / 0.460 / 0.500 |
| random tract, w 0.35 | 0.880 / 0.500 / 0.680 | 0.620 / 0.460 / 0.440 |
| topographic, σ 0.05–0.20 | 0.500–0.800 | 0.500–0.560 |
| Hebbian (`hebb` 0.05–1.20), `m3` named − control | — | 0.000 / −0.113 / −0.038 / −0.012 |

The tract delivers: on one seed the object goes from chance to **0.880** in
auditory, driving it *harder than a real word does* (1601 spikes/trial against
the word's 932). The voice moves **+0.04 / 0.00 / −0.06** across three seeds.
The Hebbian arms are read against a mechanism-**off** arm that scored +0.112 —
this instrument's noise floor showing, and the reason none of the four counts.

**Why, and it is the part worth keeping.** The word's auditory code survives
32-bin spatial pooling at **1.000**; the vision-driven code at the same
per-neuron legibility sits at **0.520**, chance. One is coarse and coherent, the
other fine-grained and balanced.

> **The arcuate is a transcoder, not an amplifier.** It converts a pattern
> already in the ear's coordinates into the larynx's. M1b's 0.890 is a
> measurement of a fixed innate identity map — *not* a ceiling the visual route
> could inherit by being poured into the same module. `m3`'s own comment and
> its printed label claimed otherwise; the comment is corrected.

**One number fell out of it that prices the whole delivery programme.** The 14
`m3probe` runs above each give a matched object/word pair on the same creature
in the same run, and the tract moves vocal's object legibility over a useful
range — so for the first time the two routes *overlap* in how well `vocal`
itself knows which stimulus it is:

| | n | `vocal` (input) | **`voice` (output)** |
|---|---|---|---|
| object, `vocal` in [0.64, 0.82] | 7 | 0.749 | **0.523** |
| word, same band | 12 | 0.752 | **0.867** |

**+0.344 at matched input legibility**, and paired within-run +0.360 ± 0.018 SE,
14/14 positive, where the input differs by only +0.111. The step is not a lossy
channel whose output tracks its input: **the within-route slope is +0.12**, and
the object route's voice sits at 0.513 ± 0.013 — one SE off chance — while
vocal's own spikes read 0.661. Taking delivery to `vocal` from today's 0.66 to a
perfect 1.00 would buy about **+0.04** at the voice against a bar needing +0.24.
A future delivery mechanism can now be refused by arithmetic rather than by
another run. (The match is on a 126-feature classifier's *score*, not on the
underlying code — which is the point, same score and different form, but worth
quoting with it.)

### The object IS in the slice — index order is what hides it

The table above leaves two readings, and they point opposite ways. Either the
object is not in a vocal group at the granularity a centroid sees — in which
case nothing downstream of `vocal` is fixable and a coherent front-end would
not help either — or it is there and the centroid cannot see it, because a
*balanced* delta leaves a centre of mass where it found it.

`m3probe`'s oracle arm separates them without touching the genome. Within each
of the nine slices the neurons are **reordered by their own training-set
delta**, so the ones that fire more for a cube end up at one end of the slice
and the ones that fire more for a ball at the other. Rates are untouched — it
is a permutation, not a reweighting, so what comes out is still a physically
realisable readout. The order comes from training trials and d′ is scored on
the held-out half; `oracle-shuf` takes the order from **shuffled** training
labels and must not rescue anything.

Object route, mean d′² over the nine groups, 50 held-out trials each:

| seed | as wired | oracle | oracle-shuf | oracle − shuf |
|---|---|---|---|---|
| shipped | +0.005 | +0.630 | +0.030 | **+0.600** |
| 20260901 | +0.024 | +0.214 | +0.020 | **+0.194** |
| 20260902 | +0.157 | +0.653 | +0.074 | **+0.579** |
| 20260903 | +0.197 | +0.247 | +0.111 | **+0.136** |
| 20260904 | −0.021 | +0.305 | +0.040 | **+0.265** |
| 20260905 | +0.057 | +0.228 | +0.156 | **+0.072** |
| 20260906 | +0.018 | +0.288 | +0.090 | **+0.198** |
| 20260907 | +0.037 | +0.367 | −0.009 | **+0.376** |

**+0.303 ± 0.070 SE, 8 of 8 positive**, against an as-wired margin of −0.005 —
the shipped readout sits exactly on its own control. Eight families rather than
three on purpose: the smoothing sweep was unanimous at n=3 and null at n=6, and
this survived that test.

> **The object is in the slice. The index order is what hides it.** The
> centroid is not too coarse and the object is not too weak; the neurons that
> carry it are simply scattered through the slice with no axis, and a centre of
> mass over a scattered balanced code is a constant. That is the same coherence
> failure as the tract, now measured at the readout with a control.

**What this does and does not license.** It does not say "build it and G3
falls out" — the ordering carries a bit per neuron of label information, and
finding it without labels is exactly what eight learning rules could not do. It
says the target is an **ordered map into `vocal`**, and that the ordering has to
arrive from a coherent upstream code rather than from reward. DNA v43 built a
topographic tract and measured F1 ×1.00, which is consistent rather than
contradictory: it ordered `vocal` by *vision's* coordinates, and vision's code
is a balanced template with no object axis to inherit.

So the two open threads join. A coherent visual front-end would supply both
halves at once — a code that survives the tract, and an axis a topographic map
can align `vocal`'s index to. That is the one build this measurement supports,
and the offline pooling check gates it before any genome edit.

One caution for anyone re-running it: `oracle-shuf` is flat for the object
everywhere (≤ +0.156) but reaches **+0.721** on the word for seed 20260903.
Sorting ~113 neurons on 50 noisy training trials can find structure. Read the
object rows, which is what the arm was built for.

### The rate substrate, priced and refused

`vision->auditory` closed the topology. The remaining structural question was
the *substrate*: every learning rule here — R-STDP, `hebb`, v37's burst term,
v16's baseline — is `syn_elig_` times a different third factor, and `syn_elig_`
is a ±20 ms coincidence trace while the object is a rate difference over
hundreds of ms. `covar` was the one rule that read no eligibility, and its
refutation named its own fix: centre each neuron on its **own** running mean
rather than its module's. It was deferred for needing "a new per-neuron array
and a snapshot bump".

**That cost was out of date** — `rate_ema_` and `rate_fast_` both exist and were
already exposed. So `eligprobe` gained a rate arm, on the same tract, trials,
split and estimator as the trace, and the answer is **do not build it.**

Two criteria were written down before the run, and they fired in opposite
directions:

| | |
|---|---|
| rate product `corr(A,B)` | **+0.303 / +0.277 / +0.273** vs the trace's +0.945 |
| the product itself | **0.482 / 0.500 / 0.482**, shuffled 0.514 / 0.516 / 0.496 |
| `vision` raw rate | **0.795** |
| `vision` minus each neuron's own mean | **0.514** (+0.281 ± 0.012 SE, 15/15) |

The `corr(A,B)` row is a **trap**. It fires positively because the residual is
mostly noise, and uncorrelated noise has a low common-mode share by
construction. `fast − slow` is a change detector and the object sits in the
field for the whole trial, so both EMAs equilibrate and the object cancels.
**Centring does not fail to help — it removes the signal.**

> **A low common-mode share is necessary, not sufficient.** Any mechanism
> motivated by "remove the common mode" can maximise its own metric by
> destroying the thing it was meant to isolate. Pair it with a row showing the
> residual still carries the signal.

That closes the dilemma on both sides: centred loses the object, population-
centred multiplies static wiring offsets, and uncentred is a rule that can only
potentiate — which is v19, already refuted.

**The finding worth keeping is the positive one.** `vision`'s raw rate reads the
object at **0.795, above the coincidence trace's 0.726** on the same tract and
trials. The substrate mismatch is real — every rule here reads coincidences and
the object is a tonic rate. What is refuted is the specific repair the notes had
named for it.

This closes the last structurally different escape. "G3 is closed under this
architecture" was a statement about learning rules — eight of them. It now
covers **rewiring** as well: you cannot reach the milestone by feeding the
object into the one pathway that works, because that pathway is an identity map
rather than a channel.

### The larynx reads nine scalars, and the object is in none of them

The strangest number on this page is that at *matched* input legibility —
`vocal` read at 0.749 for the object and 0.752 for the word — the voice comes
out at 0.523 and 0.867. The voice is not a population read of `vocal`. It is
**nine scalars**, and `VocalDecoder::update` says exactly which nine: group
centroids 0 and 2–7 become f0, three formants and three bandwidths, group
*activity* 1 opens the voicing gate, and group *activity* 8 is loudness. The
other nine readings are computed every frame and discarded.

`m3probe` now prints a bias-corrected d′ for all eighteen against a
32-permutation null, on the same trials as the accuracy table above it. The
correction matters: a d′ from two sample means is positively biased by both a
noisy mean difference and an estimated σ, which is what the
audibility ruler below cost a whole verdict to learn.

**The hypothesis that motivated it was that the object hides in the discarded
half. It does not, and neither does anything else.**

| word route, mean d′² | heard (9) | discarded (9) |
|---|---|---|
| shipped seed | +0.574 | **+1.118** |
| 20260901 | **+0.438** | +0.286 |
| 20260902 | **+0.537** | +0.338 |
| 20260903 | **+2.943** | +0.747 |

Heard beats discarded on 3 of 4. The shipped seed's dissent rests on one cell —
centroid 8 at d′ **2.859** — which reads 0.217 / 0.831 / 1.705 on fresh seeds.
**Sixth single-arm high to evaporate here.** Routing the discarded readings to
the larynx would buy nothing, and the open question in `senses.h` — *"if the
object rides on rates rather than positions, the vocal tract is discarding
eight numbers that have it"* — is answered no.

**What the same table says about the object is the finding.**

| object route | `vocal` per-neuron | voice | d′² heard | d′² discarded |
|---|---|---|---|---|
| shipped | 0.540 | 0.460 | −0.032 | −0.026 |
| 20260901 | **0.760** | 0.460 | −0.009 | +0.003 |
| 20260902 | **0.860** | 0.540 | +0.020 | +0.034 |
| 20260903 | 0.660 | 0.620 | +0.066 | +0.083 |

`vocal` knows which toy it is at 0.860 — as well as it knows which *word* it is
(0.740) — and **not one of the eighteen group readings carries it.** All
eighteen sit at the permutation null on all four seeds. The word, at the same
population legibility, loads d′² of +0.44 to +2.94.

> **The decoder is a second pooling stage, and it fails the way the tract
> fails.** A group reading is a centroid over an index-slice of `vocal`, and a
> centroid is exactly the statistic a *balanced* delta vector leaves unmoved —
> `projprobe` measures the object's coherence at **0.065** against the word's
> **0.130**. The object survives the tract into `vocal`'s population and then
> dies at the readout, for the same reason and a second time.

This is the eighth time a small differential riding on a large common mode has
stopped something here, and the first time it has been located in a **fixed,
non-plastic, genome-specified readout** rather than in a learning rule. It does
not reopen G3 — nine scalars that cannot see a balanced code cannot be taught
to, and the centroid/rate/density readouts were all refuted separately when
the vowel space was measured. What it removes is the last version of "the object is at the
larynx and something downstream is throwing it away".

### `shapeprobe` — the gate on building a new sense, and it says build

Three measurements now point at the same repair: the object's code is balanced
where the word's is coherent, it dies in the tract, and it dies again at the
larynx. All of them say *give vision a code shaped like the cochlea's*. That is
a new sense, and senses are expensive, so `shapeprobe` is the gate. It runs
**entirely outside the brain** — no network, no ticks, no genome edit. It
renders the same toys `m3probe` renders, computes candidate channel banks from
the same noisy frames, encodes each into the same 256-neuron population the way
mel is encoded into `auditory`, and pools it through the same random sparse
projection a tract applies.

| bank | chans | population | d=0.15 | **d=0.40** | shuffled | coherence | delta PR |
|---|---|---|---|---|---|---|---|
| WORD (control) | 24 | 1.000 | 1.000 | **1.000** | 0.420 | 0.207 | 181.8 |
| retina (shipped) | 176 | 0.920 | 0.680 | **0.560** | 0.540 | 0.157 | 34.7 |
| retina, 12 coarse | 12 | 0.820 | 0.740 | **0.580** | 0.540 | 0.057 | 29.7 |
| **radial profile** | 12 | 0.920 | 0.980 | **0.940** | 0.420 | 0.050 | 42.7 |
| angular spectrum | 8 | 0.960 | 0.760 | 0.740 | 0.460 | 0.361 | 79.4 |
| radial+angular | 20 | 0.980 | 0.860 | 0.760 | 0.460 | 0.151 | 55.9 |

The **radial mass profile** — what fraction of the segmented object sits in each
normalised annulus, the direct analogue of a mel bank over an ordered physical
axis — goes into a d=0.40 tract at 0.920 and comes out at **0.940**. Pooling
costs it nothing. The shipped retina goes in at 0.920 and comes out at 0.560.

Four seed families, d=0.40:

| | shipped | 20260901 | 20260902 | 20260903 |
|---|---|---|---|---|
| WORD control | 1.000 | 1.000 | 1.000 | 1.000 |
| radial profile | **0.940** | **1.000** | **0.980** | **0.920** |
| retina (shipped) | 0.560 | 0.620 | 0.480 | 0.620 |
| retina, 12 coarse | 0.580 | 0.480 | 0.460 | 0.460 |

4 of 4, no overlap between the distributions.

**Two things had to be controlled and both were.** The **word row** is an
in-instrument positive control rather than a number quoted from `projprobe` —
same encoder, same projection, same classifier — and it survives at 1.000 every
time, so the model of a tract is the one that works in the creature. The
**channel-count control** is the retina's own features averaged into 12
contiguous groups and encoded identically: every candidate has ~12 channels
where the retina has 176, and the encoder gives each channel a contiguous slice,
so a coarse bank gets 21 neurons per channel and block structure the retina
never gets. That control pools at **0.495 mean** — no better than the full
retina. **The win is the features, not the channel count.**

> **Two mechanistic statistics were tried and neither orders the banks.**
> Coherence, which this page has treated as the operative variable, is *lowest*
> on the winning row (0.050) and middling on the losing one (0.157). The delta's
> participation ratio does no better: the channel-count control has a *higher*
> PR than the radial profile and pools worse. The pooled accuracy is the
> measurement; the statistic that explains it is not in hand. "Coarse and
> coherent" was a story, and the part of it that survives is "coarse".

**The first version of this probe could not fail, and that is worth recording.**
It projected the 12-number banks straight onto 126 units without the population
encoding — which is very nearly invertible, so every candidate "survived
pooling" for a reason with nothing to do with the code. It printed SAYS BUILD.
The encoding step is the whole instrument.

**What it licenses.** A build and a measurement, not a claim. Two things favour
the candidates: it compares feature banks rather than what a brain makes of
them, and it lets them **segment the frame** — at 0.02 noise against a 0.40
luminance step the segmentation is essentially perfect, where the retina's
centre-surround responses carry the pixel noise. So a REFUSE here would have
been decisive and a BUILD is permission to spend the sense and find out.

### DNA v46 — the shape bank, built: delivery moves a long way, the voice does not

`shapeprobe` said build, so it was built. `radial_bins` on `[vision]` is 0 in
the shipped genome and bit-identical to v45 there; non-zero replaces the
retina's ON/OFF responses with that many **radial mass channels** — the host
segments the frame, finds the object's own centroid, and reports the fraction of
it in each normalised annulus, radius scaled by the equivalent-circle radius.
Ordered, coarse, translation-free and scale-free. The DoG cells are still built
and still sampled, so `contrast`, the gaze controller and the panel's meters
keep working; what leaves the eye is the profile.

**A prediction that was wrong, and the bug it hid.** The genome comment said M2
would not survive a shape-only eye. M2 reads **100%**, against the retinotopic
eye's 98% — but only after a real bug was fixed. Thresholding a blank wall at
the midpoint of its own noise puts half the frame above the line, so an empty
field produced a confident profile of nothing and M2 read **0.57**. The guard
now reuses `contrast_` and `contrast_floor`, which are the retina's and the
genome's existing statements of "is anything there", rather than a new constant.

`m3probe`, four seed families, object route:

| seed | arm | vision | central | vocal | **voice** | oracle |
|---|---|---|---|---|---|---|
| 20260901 | retina | 1.000 | 0.640 | 0.760 | 0.460 | +0.214 |
| 20260901 | **radial** | 1.000 | **0.920** | **0.840** | 0.600 | **+0.866** |
| 20260902 | retina | 1.000 | 0.760 | 0.860 | 0.540 | +0.653 |
| 20260902 | **radial** | 1.000 | **0.800** | **0.920** | 0.580 | **+1.360** |
| 20260903 | retina | 0.920 | 0.560 | 0.660 | 0.620 | +0.247 |
| 20260903 | **radial** | 1.000 | **0.820** | **0.860** | 0.580 | **+0.909** |
| 20260904 | retina | 0.980 | 0.760 | 0.760 | 0.660 | +0.305 |
| 20260904 | **radial** | 1.000 | **0.820** | **0.880** | 0.540 | **+0.904** |

**`central` +0.160 and `vocal` +0.115, both 4 of 4, and the oracle roughly
triples.** The shape bank does exactly what `shapeprobe` said it would: the
object now survives the tract and arrives in the larynx's slices with an axis.

**And the milestone does not move.** `m3` on four families, each genome against
its own random-order control:

| | shipped seed | 20260901 | 20260902 | 20260903 | mean |
|---|---|---|---|---|---|
| retina, taught − random | +0.060 | −0.060 | +0.060 | −0.060 | **0.000** |
| radial, taught − random | +0.080 | 0.000 | **+0.260** | 0.000 | **+0.085** |

**+0.085 ± 0.071 SE — 1.2 SE, and it rests on one arm of four.** Seventh
single-arm high in this project's history. Read it against the row above it:
the control genome has the mechanism absent by construction and still swings
**±0.060, spread 0.120** — an independent re-measurement of the same noise floor
`pairprobe` reported at 0.115, on a different metric. Audibility agrees: cube
versus ball on the shape eye is corrected d′ **0.00** against a 32-permutation
null of 0.48, so nothing here is a sound anyone could hear.

> **This is the sixth intervention to improve what the association module
> represents and leave the voice where it was**, after divisive normalisation,
> top-down feedback, the hippocampus, the denser vision tract and the faster
> auditory tract. What is new is that this one was **priced in advance and the
> arithmetic held**. The within-route vocal→voice slope is +0.12, so `vocal`
> moving +0.115 predicts a voice change of about **+0.014**. Observed: +0.005 on
> the four paired families. The delivery programme can now be refused by
> arithmetic, and this is the measurement that shows the arithmetic works.

So `shapeprobe`'s gate was **correct about what it measured and did not decide
the milestone**. It asked whether a coherent front-end survives a tract; the
answer was yes and it is still yes. The tract was never the binding constraint —
the step from `vocal`'s population to nine scalars is, and DNA v46 does nothing
about that. Ships off, pinned in `mechverify` at `9945f7d9b1d66af8`.

### The write is not object-blind any more — and the old number was on the wrong tract

`dwprobe` decomposed what reward writes and found ~8% object-specific, which is
one of the two measurements that closed the credit-assignment family. It watched
**`central→vocal`** — and this page's own next section shows that tract is a
**non-participant**: delete it, recalibrate, and every G3 number is unchanged.
So the number that closed a family was measured on a pipe connected to nothing.
The probe now names its tract and runs both.

`corr(cube vs ball)` read against `corr(same object, redrawn)`, which is the
reproducibility ceiling and the only thing it may be read against:

| creature | tract | arm | ceiling | cube vs ball | ratio |
|---|---|---|---|---|---|
| retina | `vision→vocal` | praise | 0.458 | 0.473 | **1.03** |
| retina | `vision→vocal` | balanced | 0.436 | 0.444 | 1.02 |
| retina | `vision→vocal` | no reward | 0.448 | 0.433 | 0.97 |
| **v46 shape bank** | `vision→vocal` | praise | 0.639 | 0.454 | **0.71** |
| **v46 shape bank** | `vision→vocal` | balanced | 0.654 | 0.498 | 0.76 |
| **v46 shape bank** | `vision→vocal` | no reward | 0.666 | 0.472 | 0.71 |

**On the shipped retina the tract that reaches the larynx does not
differentiate at all** — cube-vs-ball sits *at* its own ceiling, 1 of 3 seeds.
**On the shape-bank creature it does, 3 of 3 seeds.** A session of
cube-teaching writes a measurably different weight change than a session of
ball-teaching, on the tract that participates.

**Two things this is not.** It is not reward doing it: the no-reward arm gives
the same 0.71, so this is the activity difference driving STDP and not credit
assignment. And it is not behaviour: `dwprobe` blocks one object per session,
while `m3` alternates them, so nothing here shows two mappings coexisting.

> **What it changes is which question is open.** "The rule writes the same thing
> whichever object is present" was the diagnosis, and on this creature and this
> tract it is false. What remains is that the two different things it writes
> land on the same synapses and overwrite each other — which is not a new guess
> but the thing three existing experiments already measure. `retain`: a
> conflicting second lesson wipes the first, 0.22. `capacity`: two lessons
> coexist on *orthogonal* output dimensions at 0.84 but compete for one reward
> channel. `credit`: a per-neuron reward mask removes the interference
> **completely**, targeted retention ~1.0 on 3 of 3.
>
> So the standing hypothesis is **interference, not credit** — and unlike every
> mechanism in the closed families, it already has a measured upper bound and an
> oracle that reaches it. G3 stays filed as a closed negative, because none of
> this is behaviour yet. But it is the first genuinely different question this
> project has had to ask in some time, and it exists because a probe was pointed
> at the tract that matters instead of the one it was written for.

### Routing the shape bank through the arcuate — and poolability was not the missing piece

The alignment principle says the nine scalars carry whatever arrives in their
coordinates, and exactly one pathway does that: the arcuate. `vision→auditory`
was built weeks ago to exploit it and failed, with a measured explanation — *the
word's auditory code survives 32-bin pooling at 1.000, the vision-driven code at
matched per-neuron legibility sits at 0.520.* The retina's code was the wrong
shape.

DNA v46 supplies a code of the right shape: `shapeprobe` puts the radial bank
through the same pooling at **0.960** where the retina reads 0.570. So the
experiment was re-run with the shape bank as the source — the one ingredient the
first attempt was diagnosed as missing.

**Ten configurations, then a replication.** Weight first, at density 0.15:
auditory delivery peaks at 0.640 (w 0.05) and collapses to 0.500 by w 0.20,
with `central`, `vocal` and the voice all degrading — the signature of a tract
that is too loud, which the shape bank is prone to because 12 channels drive 21
neurons each in lockstep where the retina drove about 1.5. Then density, which
runs the other way: **0.06 → 0.720, 0.15 → 0.640, 0.40 → 0.500**, the same
"a random tract's capacity falls with density" this project measured in
`projprobe`.

At the best setting (d 0.06, w 0.10), against a weight-0 arm with identical
wiring, four seed families:

| seed | auditory off → on | voice off → on |
|---|---|---|
| shipped | 0.520 → **0.720** | 0.580 → 0.620 |
| 20260901 | 0.480 → **0.780** | 0.560 → 0.560 |
| 20260902 | 0.540 → **0.660** | 0.580 → 0.700 |
| 20260903 | 0.600 → **0.700** | 0.560 → 0.520 |
| mean | **+0.180, 4 of 4** | **+0.030, sign flips** |

**The tract delivers and the voice does not follow** — the same result the
retina-sourced version gave (+0.04 / 0.00 / −0.06), reproduced with a source
chosen specifically to fix the diagnosed fault.

> **This retires my own explanation of the first failure.** "The visual code
> does not survive pooling" was the reason given for why the arcuate route did
> not transfer. A code that *does* survive pooling — measured at 0.960, against
> the word's 1.000 — does not transfer either. Poolability was not the missing
> ingredient.
>
> Two independent ways of failing at the same step now exist: a well-delivered
> badly-shaped code (retina, auditory 0.880) and a moderately-delivered
> well-shaped code (shape bank, auditory 0.720). Both leave the voice at its
> control. **The arcuate is an identity map on the ear's own coordinates, and no
> visual code inherits it regardless of how it is shaped.** That is a stronger
> statement than the one this page made before, and it was bought by trying the
> repair the earlier diagnosis called for.

Genome-only: nothing shipped, no code changed, the pinned hash is untouched.

### `vocabcurve` — the vocabulary does not collapse, and the readout is not the limit

With G3 closed, the open question moved to the pathway that works. M1b reads
**0.890 for two words** and `vocab` reads **0.371 for eight against a chance of
0.125**, and nothing had measured what happens in between.

**One session, nested subsets.** Each creature is run once on all eight words
and the N-way score is taken over the trials whose label is below N, so trials
*per word* are identical at every N. Running the creature separately per
vocabulary size would confound capacity with sampling — more words in a fixed
session means fewer trials each, and accuracy would fall for no reason about the
creature. Total trials do fall with N, which shrinks the training set at the
small-N end and so biases *against* it; the count is printed.

`voice` accuracy divided by chance, three seed families:

| N | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|
| shipped | 1.49 | 2.00 | 2.48 | 2.60 | 2.63 | 2.68 | **2.88** |
| 20260901 | 1.44 | 1.77 | 2.12 | 2.08 | 2.33 | 2.47 | **2.55** |
| 20260902 | 1.52 | 1.97 | 2.43 | 2.49 | 2.62 | 2.85 | **3.01** |

**The advantage over chance grows as words are added.** Raw accuracy falls from
0.747 to 0.360 only because chance falls faster, from 0.500 to 0.125. Shuffled
controls sit at chance at every N on 3 of 3. `vocab`'s "the vocabulary is full
below eight" remains true and is a *different* statement — it is about pairwise
separation against a 0.75 bar, 15 of 28 pairs under it. Both hold; say which.

**And the column that matters is the third one.** `vocal` read *per neuron* is
**below** the nine scalars at every N on 3 of 3 — 0.237 against 0.360 at eight
words, with `vocab` reporting the same independently (0.234 against 0.371).

> **The nine scalars are a bottleneck only for information that does not arrive
> in their coordinates.** A heard word comes through the arcuate, which is an
> identity map onto the readout's own coordinates, and there the scalars carry
> **more** than the population does — each group's centroid averages away noise
> the raw counts keep. A seen object arrives with no such alignment, and there
> the population carries 0.860 while the scalars carry nothing. Same readout,
> opposite verdicts, and the difference is alignment rather than capacity. This
> completes the per-knob finding above from the other side, and it means *more
> motor groups* is not the fix for either problem.

**One hypothesis of mine was refuted by its own control.** I expected `vocab`'s
number to be depressed by its first-half/second-half split — the documented
`projprobe` bug. The naive-split control column reads **0.371** against the
interleaved split's 0.360. The split is not a confound here.

### `ctxprobe` — pricing the interference hypothesis, and it does not pay

`dwprobe` on the participating tract left one hypothesis standing: the two
writes are different, but the objects alternate, so they land on the same
synapses and undo each other. `ctxprobe` prices that with an oracle. The naming
protocol runs three ways and the milestone's own readout is scored:

- **unmasked** — what the creature does today;
- **masked by object** — reward reaches only the lower half of the F1 group on
  ball trials and only the upper half on cube trials, so the two lessons cannot
  overwrite each other;
- **masked at random** — the same two halves on a coin flip.

**The first version of the mask was wrong and is worth recording.** It split
`vocal` in half by neuron index, which splits it *across* the nine articulator
groups — a cube lesson could only touch the bandwidths and a ball lesson only
f0 and the low formants. That is `capacity`'s orthogonal case, not naming. It
read −0.080 and cost the echo. Naming needs both objects driving the **same**
dimension to different values, so the mask now lives inside the F1 group, whose
centroid is the knob that would carry the answer.

Four seed families, 5 creatures × 3 arms each:

| family | unmasked | by object | at random | oracle − unmasked | **oracle − random** |
|---|---|---|---|---|---|
| shipped | 0.600 | 0.480 | 0.440 | −0.120 | **+0.040** |
| 20260901 | 0.520 | 0.480 | 0.380 | −0.040 | **+0.100** |
| 20260902 | 0.720 | 0.620 | 0.620 | −0.100 | **0.000** |
| 20260903 | 0.560 | 0.640 | 0.500 | +0.080 | **+0.140** |
| mean | 0.600 | 0.555 | 0.485 | **−0.045** | **+0.070 ± 0.031 SE** |

**Read the two right-hand columns together.** Against no mask at all the oracle
is **negative** — masking reward costs more than the condition buys, which is
consistent with `credit`'s note that the mask runs at about 30% of the learning
rate. Against a random mask of the same size it is **positive on 4 of 4 with no
negative, +0.070 ± 0.031 SE**. So the condition is doing *something*: gating by
which object is present beats gating by a coin flip.

**It is still not a result, for two reasons, and the second is the interesting
one.** It sits below m3's own cross-family floor of ~0.12. And the random arm
re-rolls its mask every trial, so it thrashes — a lesson's synapses get reward
on half its own trials *and* on half the other object's. **"Oracle beats
random" therefore confounds "the condition matters" with "a consistent mask
beats an inconsistent one",** and this instrument cannot separate them. The
control that would is a mask that is consistent but uncorrelated with the
object, which is awkward to build in a protocol where the objects alternate.

> **Where that leaves it.** Interference is not the blocker in the strong form:
> removing it by oracle does not make naming work, it makes it slightly worse.
> The residue is a +0.070 that has a live confound and sits under the floor.
> G3 stays closed. What this adds is that the last standing hypothesis was
> priced rather than argued, and the price is on the record.

### The two protocol axes, and neither was the missing piece

Eleven mechanisms were built against G3 and every one was measured under the
same protocol. Two things in this project's own notes said that protocol was the
wrong thing to hold fixed, and neither had been tested.

**Length.** Every teaching result that *works* runs at 3.4M–5.6M ticks;
`teachsound` (M1c), the one milestone where praise demonstrably moves the voice,
needs **3,400,000**. `m3` runs at **120,000** — 28× shorter, and `g3probe`'s
900k is still 3.7× under it. So `m3` was run on the v46 creature at
`teachsound`'s own length:

| | 120k (all prior G3 work) | **3.4M** |
|---|---|---|
| taught − random | +0.060 ± 0.040 | **−0.035 ± 0.013** |
| beat its own control | 2 of 5 | 1 of 5 |
| at or above 0.75 | 0 of 5 | 0 of 5 |

The error bar *tightens* at length and the estimate sits below zero. Against the
±0.060 cross-family floor it is a null rather than a negative, but there is no
sign of a gap opening with time. **"We never gave it time" is answered: no.**

One number did move, and it is the only thing 28× the training ever bought:
cube versus ball reads **corrected d′ 0.49 against a 0.07 null**, where every
shorter run read ~0.00. The two utterances genuinely differ at teaching length —
2.5% of an [i]/[a] contrast — and that is still five times under the audibility
bar and invisible to the classifier.

**Schedule.** `capacity` says in as many words that *sequential beats
simultaneous*: two lessons taught at once interfere where the same two taught in
turn do not. `m3` interleaves cube and ball from the first trial and always has.
`curriculum` adds a blocked-fade arm — blocks of sixteen of one object, halving
until the schedule *is* m3's interleaved one — each arm carrying its own
random-order control:

| arm | taught − random |
|---|---|
| interleaved (m3's schedule) | **+0.061 ± 0.014**, 3 of 3 positive |
| block-fade | **−0.146 ± 0.046**, 3 of 3 negative |

**Blocking is actively harmful, by more than the floor.** That is not a null and
it is worth understanding: `capacity`'s result is about lessons on *orthogonal*
output dimensions, and naming's two lessons are on the *same* dimension. So a
block of ball-teaching is overwritten by the block of cube-teaching that follows
it — which is `retain`'s 0.22 showing up in behaviour rather than in weights.
m3's interleaved schedule was already the right one, now for a measured reason.

> **Both protocol axes are closed alongside the four mechanism families.** The
> interleaved arm also reads +0.061 at 1.2M against +0.060 at 120k, so the
> milestone is stable across a 10× length range and sits exactly on its own
> noise floor at both ends. There is no protocol under which the existing
> machinery reaches the bar.

## Why G3 is closed, and what would have to be different

> **PARTLY SUPERSEDED, and the distinction is the point.** G3's own bar is
> *vision* → voice, and it remains closed negative — nothing below is retracted on
> that. But this section also says *"the creature has exactly one conditional
> pathway and it is innate"*, and **that sentence is now false.** DNA v51's
> context-indexed bias built a **learned** conditional pathway off the *ear*:
> `areax` reads 112.9 Hz of conditional dF1, the creature derives its own index
> from the ear's rate EMA (+23.6 +/- 6.6 Hz, 3.6 SE, 16 of 18 creatures,
> replicated on a fresh seed family), and directional naming is met at 0.82. The
> reasoning below is kept because it is what produced v51 — the parameter-count
> argument in it is the argument that worked — but read the naming rows in the
> table above for where this ended up.

G3 asked for a **held-out classifier to tell cube from ball off the baby's own
vocalisations at 0.75**, paired beating unpaired. It is not met, and after
eleven mechanisms in four structurally different families it is filed as a
settled negative rather than an open problem. The distinction matters: an open
milestone is an invitation to keep trying the same shape of thing, and this one
is not.

**The statement of the limit, in one measurement.** `vocallearn`, same
instrument, same session: reward cuts formant error **+24 points** toward a
*fixed* target and **−0.1** toward a target that depends on what the creature
heard. The creature has exactly one conditional pathway and it is **innate** —
the arcuate, `auditory→vocal`, which M1b measures at 0.890 and which the
`vision→auditory` work showed is an identity map rather than a channel.

**What was ruled out, by family:**

| family | what was built | outcome |
|---|---|---|
| learning rules | R-STDP, `hebb`, node perturbation on biases and on synapses, plateau gating (v29), burst plasticity (v37), rate covariance, metaplasticity (v41) — eight, every one `syn_elig_` × a different third factor | the object-specific share of what reward writes is ~8% |
| rewiring | `vision→vocal` (shipped), `vision→auditory` (built), topographic variants (v43, v46) | delivery reaches 0.880 and the voice moves +0.04 / 0.00 / −0.06 |
| substrate | per-neuron rate covariance, the repair the notes had named | centring is a high-pass and the object is tonic: it removes the signal |
| readout | the per-knob table, the oracle arm, the shape bank + topographic map | the object is in `vocal`'s slices, in none of the eighteen readings, and the ordering that would expose it is **learned**, not geometric |

**The arithmetic that now refuses future delivery work.** The within-route
`vocal→voice` slope is **+0.12**. DNA v46 moved `vocal` +0.115 and the voice
moved +0.005 — predicted +0.014. Perfect delivery to the larynx would buy about
+0.04 against a bar needing +0.24. A delivery mechanism can be declined on paper.

**What would have to be different.** Not a ninth learning rule. The creature
would need a way to make plasticity *itself* depend on which object is present —
a per-context gate on which synapses are eligible, rather than a scalar reward
scaling a population-wide trace. Everything here multiplies one eligibility
trace by one third factor, and a scalar cannot carry a condition. Whether that
is buildable in this architecture is genuinely open; what is closed is the idea
that any refinement of the existing machinery gets there.

**What the creature does do**, and these are the milestones that are met: it
repeats a heard word at 0.890 (M1b), it is taught a vowel by praise alone and
the change is audible at d′ 7.50 (M1c), it answers when spoken to (M1d), and the
answer carries the word +0.215 above ambient babble. Those are the interesting
results and none of them was aimed at the milestone it hit.

### The oracle's ordering cannot be wired — and that reading of it was mine

The oracle arm said the object is in `vocal`'s slices and index order hides it,
worth +0.303 ± 0.070 in centroid d′². This page then said that names the
target: *"an ordered map into `vocal`, with the ordering arriving from a
coherent upstream code rather than from reward."* DNA v46 supplied exactly that
coherent upstream code, and DNA v43's `kTopographic` supplies the map. So the
composition was built and measured, and **it does not work.**

`vision→vocal`, `kTopographic`, source range the whole shape bank, destination
the F1 group — annulus number onto position within the slice, which is the
permutation the oracle applies by hand. Weight is explicitly not a derived
constant, so it was swept.

On the shipped seed the dose-response is as clean as anything in this project:

| weight | 0.0 | 0.14 | 0.35 | 0.70 |
|---|---|---|---|---|
| F1 d′ | −0.199 | +0.400 | +0.804 | **+1.242** |
| voice | 0.580 | 0.620 | 0.700 | **0.740** |

Monotone over a 6× range in both the targeted knob and the milestone quantity.
**It is the seventh single-seed high in this project's history.** Two of the
other three seeds in that sweep collapse above 0.14, and at eight families
against a weight-0 control that has the same wiring:

| | F1 d′ delta | voice delta |
|---|---|---|
| mean over 8 seeds | **+0.172 ± 0.225 SE** | **+0.048 ± 0.040 SE** |
| sign | 6 of 8 positive, two large negatives (−1.087, −0.013) | 6 of 8 |

0.8 SE and 1.2 SE. Null on both, and the two biggest numbers in the F1 column
point in opposite directions (+1.003 on the shipped seed, −1.087 on 20260907).

> **The correction, and it is about how the oracle should have been read.** The
> oracle derives its ordering from *that creature's own training deltas* — it is
> a **learned** ordering, not a geometric one. A fixed map from an ordered
> source cannot supply it, because which of `vocal`'s neurons carry the object
> is a fact about that creature's random wiring and not about any axis outside
> it. So the oracle's +0.303 is not a route. It is another statement of the same
> wall: the ordering would have to be learned, and learning an ordering is what
> eight learning rules could not do.
>
> Reading it as "names the target" was mine and it was wrong. The arm is still a
> correct measurement of an upper bound; it was never a design.

**And `m3` now prints its own floor**, because this page quoted the shipped
creature's taught−random as a number for months. It reads **+0.060 ± 0.040 SE
over 5 paired creatures** — 1.5 SE, not a result — with a note that the control
genome swings ±0.060 across seed families with the mechanism absent by
construction, so a cross-family claim needs about 0.12.

Two mechanisms carry the change since M4, and neither was aimed at the goal it
hit. **Per-module homeostasis (DNA v9)** came from asking why praise did not
survive its own session, and it stopped the decay. **Directional exploration
(DNA v10)** came from asking why babble never converges, and it is what met G2.
Three other mechanisms were built, measured and either reverted or shipped
inert — a visual cortex, a curvature stage, and a scalar version of that same
exploration — and each of those sections below records what it ruled out.

## Stage 0 of the audio rewrite — DNA v47, and the wall it found is a new one

The rewrite this is the first step of drops vision and rebuilds the audio path.
Its argument, in one paragraph. Every conditional-learning failure here has the
same arithmetic, and the synaptic-perturbation post-mortem writes it out: the
presynaptic gate is a spike count, a spike count is a neuron's baseline rate
plus a few percent of condition, so the credit factorises into a shared term and
a differential one and the shared term is far larger. That shared term has been
subtracted (v24), signed (v35), gated (v29, v37, v40), re-timed (v26, v42),
re-routed (v43, v46) and re-rewarded (v20) — seven common-mode walls and eleven
mechanisms. Its **cause** had never been touched: §3.1 makes rate homeostasis
mandatory, so no population in this creature has ever had a baseline of zero.

`vocallearn` states the consequence in two numbers from one instrument: **+24.0
points toward a fixed target and −0.1 toward a conditional one.** The +24 is a
conditional-learning result with a context layer of size **one**. Give the
shipped rules a context layer of size *n* whose slices are disjoint and exactly
zero when absent, and two conditions share no presynaptic unit, therefore no
synapse.

**DNA v47 is that substrate.** `ModuleRole::kContext` — no noise, no homeostatic
setpoint, no recurrence, cut into slices the host writes into. It needs **no new
learning rule**: `trace_pre_` on an unwritten unit is exactly zero, so the
shipped R-STDP already has the property and had never had a substrate to run it
on. The role is inert on the shipped creature (`verify` 21 of 21, hash
`ad96f882becbee92`). `tools/genome_add_context.py` appends it;
`--experiment ctxlearn` is `vocallearn` with a third target — `fixed` (no
conditionality, the positive control), `heard` (the innate map, refined), and
`swap` (the *other* word's formants: arbitrary, and the arcuate pulls against
it, so anything it earns was learned). Each carries its **own** yoke, because a
yoke scored against a different target is not a control.

**With the oracle mute it reproduces `vocallearn` exactly**: +35.9 / +0.5 / +0.2.

### The substrate does what was predicted, at the synapse

Mean |Δw| written by reward onto the oracle's tract, as a fraction of what is
written onto the larynx's other afferents:

| oracle | out_w 0.005 | 0.010 | 0.020 |
|---|---|---|---|
| mute | 0.01 | 0.04 | 0.07 |
| 13 Hz | 0.42 | 0.28 | 0.44 |
| 63 Hz | **0.76** | **0.79** | **0.62** |

A zero-baseline tract goes from 1–7% of the larynx's plasticity to 28–79% of
it, against the ~8% object-specific share a dense tract manages. **It is the
most learnable input in the creature.** That much of the argument is confirmed.

### And that is exactly why it cannot be used

The positive control — one fixed formant target, no conditionality, the act G2
proved reward can shape — **dies whenever the oracle fires.** Not the difference
metric: the taught arm's own error reduction, over three seeds per cell:

| vocal noise | oracle mute | oracle at 31 Hz |
|---|---|---|
| 0.22, out_w 0.005 | +24.6 / +39.1 / +38.1 | +6.1 / +2.2 / −5.9 |
| 0.22, out_w 0.010 | +24.6 / +39.1 / +38.1 | −8.8 / +0.8 / +7.3 |
| 0.22, out_w 0.020 | +24.6 / +39.1 / +38.1 | −13.8 / +12.5 / −3.1 |
| 0.16, out_w 0.010 | +21.7 / +31.0 / +35.5 | +21.5 / +6.7 / −10.7 |
| 0.10, out_w 0.010 | +13.1 / +31.0 / +31.1 | +15.4 / −12.5 / −31.2 |

**Five genomes, twelve driven levels, and the control never survives once.** The
collapse does **not** scale with `out_w` over a 6× range, so it is not the tract
shoving the larynx; and paying for the tract out of vocal's own noise — rule 1
of the calibration invariant, the lever this experiment prints — does not save
it either, at 0.16 or at 0.10.

> **The wall, and it is a different kind from the seven before it.** Those were
> *the condition does not arrive*. This one is **arriving costs more than it
> pays**: this larynx cannot host a learnable conditional input and a
> reward-driven exploratory pathway at the same time, and adding the first
> abolishes the second. The presynaptic baseline was a real variable — the
> substrate delivers exactly what the arithmetic predicted — and the delivery is
> not usable on a single shared motor population.

**What that specifies, and it is not another learning rule.** In the birdsong
circuit this project borrows from, HVC→RA and LMAN→RA are separate afferent
populations onto RA under separate rules; RA is never asked to be a shared
exploratory pool and a conditional readout at once. `vocal` here is 126 neurons
in nine groups read as centroids, and it is asked to be both. So the motor side
has to be rebuilt **before** a conditional input can be tested at all — a sparse
dictionary of articulatory postures selected by competition, where a conditional
pathway *selects among discrete units* rather than competing for the same
continuous posture node perturbation is exploring. That was Stage 2 of the
rewrite plan for an unrelated reason (the `vocal→voice` slope is +0.12 because a
centroid over a dense module is a second pooling stage); it is now a
prerequisite rather than a follow-on.

### Two retractions, and both are the instrument working

**A +20.3 that did not replicate.** At `out_w` 0.030, gain 0.06, `swap` read
**+20.3 ± 2.7, 3 of 3 positive** against +0.2 with the oracle mute, with the
non-conditional control *down* — conditional-up-and-control-down being the one
pattern extra drive cannot produce. It was written up here as a lead. Across the
weight and noise series the same cell reads:

| | w 0.005 | w 0.010 | w 0.020 | **w 0.030** | noise 0.16 | noise 0.10 |
|---|---|---|---|---|---|---|
| `swap` at 13 Hz | +7.6 ± 7.5 | +3.2 ± 2.3 | +6.1 ± 14.8 | **+20.3 ± 2.7** | +7.6 ± 11.9 | +1.4 ± 5.0 |

Five of six under +8 with error bars over zero. **It is the eighth single-point
high in this project's history and it behaved like the other seven.** Retracted.

**A diagnostic that could not fail.** The collapse was first blamed on the
oracle winning vocal's *synaptic-scaling* competition. `syn_wake_scale` and
`syn_sleep_scale` were set to zero on vocal and three of the four rows came back
**byte-identical** — because DNA v11 measured in August that **synaptic scaling
never executes on the larynx at all** (`sum|w| / setpoint 0.93, outside the band
0.0% of samples`). The mechanism being relaxed was not running. That memory
carries the rule verbatim — *before relaxing a regulator, measure whether it is
running* — and it was read after the run rather than before it. The remaining
suspect is intrinsic plasticity, which v11 showed **is** the half that runs on
vocal, and which responds to rate rather than to weight — which fits a collapse
that does not scale with `out_w`.

### Three instrument corrections `ctxlearn` now carries

1. **A mute level is not a driven level.** Threshold is 1.0 over a 20 ms leak at
   1 kHz, so the steady state is 20× the gain and anything under 0.05 never
   fires. The first sweep ran gain 0.04, got a byte-identical copy of the silent
   arm, and *chose it as the best driven level*. Readability now checks the
   measured rate, not the index.
2. **A collapsed control cannot report a null.** `fixed` gates every row: a level
   is scored only if the control clears 5 points and keeps 60% of what it reads
   with the oracle mute. Otherwise the row prints UNREADABLE. The first version
   asked only for `> +5`, passed a control at a third of its own reference, and
   printed a negative it was not entitled to.
3. **A lead must clear its own error bar.** The LEAD branch tripped once on
   +6.1 ± 14.8; it now requires the mean to exceed 2 SE.

And one that belongs to the tooling rather than to this experiment: **appending a
module is not wiring-neutral.** Noise is `rng_->signed_uniform()` once per neuron
per tick, so 64 extra neurons re-roll the stream — `calibrate` reads somato
4.45 → 4.16 (STALE), and reads *the same at `out_w` 0.0 as at 0.30*, which is
what proves it is the stream and not the tract. `genome_add_relay.py` and
`genome_add_hvc.py` both say "bit-identical"; their paired arms are sound, their
docstrings are not. Never quote an appended genome's number against
`dna/default.toml`.

### DNA v48 — a dictionary of postures, and the centroid is not what was wrong

`ctxlearn` above ends by naming the motor side as a prerequisite: one shared
population cannot host a learnable conditional input and a reward-driven
exploratory pathway, so the larynx has to stop being one shared continuous
posture before a conditional pathway can be tested. The proposal was a
dictionary — a sparse inventory of articulatory postures selected by
competition, where reward *selects among discrete alternatives* rather than
steering a continuous posture. It was independently motivated: the
`vocal→voice` slope is +0.12 and the per-knob table finds the object at the null
in all eighteen group readings, both of which say the centroid is a second
pooling stage.

**It is built and it does not work, and the reason inverts the argument.**

`dictionary_units` slices of the *same* `vocal` neurons compete; the field names
a posture on a grid over (F1, F2), softmax-blended and held for a dwell. The
nine groups still set F0, the bandwidths, amplitude and voicing. Neurons, drive,
homeostasis and every learning rule are untouched, so the only thing under test
is how the larynx is read. Ships **off**: `dictionary_units = 0` is bit-identical
(hash `ad96f882becbee92`, `verify` 21 of 21), and switching it on moves the hash
to `fcda2eace06fcf36`, so it is live rather than inert.

#### First, the number the shipped larynx had never been asked for

| readout | produced F1 | produced F2 |
|---|---|---|
| **centroid (shipped)** | 638 **± 25 Hz** | 1620 **± 51 Hz** |

*Why the creature always sounds the same*, in Hz at the output. `babble` now
prints it, because it is what every milestone downstream has to hear and no
other number in that experiment is about it.

#### And then the trade-off, which is absolute

| readout | smoothing | F2 spread | `fixed − yoked` |
|---|---|---|---|
| centroid | 800 (shipped) | 51 Hz | **+36.5** |
| centroid | 200 | 86 Hz | **+17.6** |
| dictionary, argmax | 800 | 406 Hz | +2.3 |
| dictionary, blend 0.20 | 800 | 271 Hz | +4.2 |
| dictionary, blend 0.35 | 800 | 292 Hz | −1.2 |
| dictionary, blend 0.20 | 200 | 266 Hz | +2.4 |

**Five dictionary configurations, none teachable; two centroid configurations,
both teachable.** The last row is the control that matters: at *matched* 800 ms
of articulator inertia the comparison is confounded by the dictionary's dwell,
so the pair was re-run at 200 ms with everything but the readout identical. The
centroid clears the bar at +17.6 with 86 Hz of spread; the dictionary has 3.1×
the spread and cannot be taught at all.

> **The centroid is not a lossy readout to be engineered away. It is what makes
> the larynx steerable.** A centroid gives each of fourteen neurons a distinct,
> monotone lever on a continuous scalar, which is exactly what a per-neuron bias
> rule can push on. A dictionary pools those fourteen into one slice activity:
> the effective parameter count falls from 126 to 9, F1 and F2 are coupled
> through shared posture anchors, and the output is quantised to three levels per
> formant. The gradient node perturbation needs is gone.
>
> So the +0.12 slope is not a defect. **It is the price of a readout this
> creature's learning rule can steer at all**, and the shipped genome sits at the
> steerable end of a real tension.

#### What that refutes, and it is this page's own proposal

The audio rewrite argued that the motor readout was the load-bearing repair. Two
of its three stages are now measured and neither does what was claimed:
`ctxlearn` showed a zero-baseline conditional input starves the exploratory
pathway, and the dictionary — the proposed fix for exactly that — is not
steerable. **Making a discrete selection learnable needs policy gradient over a
categorical distribution, which is a new learning rule**, and needing no new
learning rule was the whole basis on which the rewrite was preferred to another
mechanism. That is a decision rather than a tuning step, and it is not taken
here.

What survives is two measurements worth having: a zero-baseline code is the most
learnable input this creature can be given (28–79% of the larynx's plasticity,
against ~8% for a dense tract), and the shipped voice has a standard deviation of
25 Hz in F1.

#### Four failures on the way, each of which named the next one

Recorded because three of them looked like the mechanism failing and were the
instrument or the design instead.

1. **The hard argmax.** 5.9× the F1 spread, positive control +36.5 → +2.3. Node
   perturbation writes a per-neuron *bias*, and under an argmax the voice is a
   **step function** of those biases — zero gradient almost everywhere, undefined
   at the boundaries. Replacing a pooling stage with a non-differentiable one is
   not an improvement.
2. **The softmax fix, on raw activities.** Spread collapsed to 24–30 Hz at every
   usable temperature, i.e. back to the centroid's own 25. The units' activities
   differ by **one or two percent of their mean**, because `vocal` is
   homeostatically regulated — the same small differential on a big common mode
   this creature has everywhere else, now arriving at the readout. An absolute
   temperature is either far below those differences or far above them, with no
   regime in between.
3. **Z-scoring the activities**, to make the temperature scale-free — and the
   sweep came back flat *including at temp 0.20, which is very nearly an
   argmax*. **A difference that survives at temperatures where the two rules
   agree was never about the temperature.** The softmax path was bypassing the
   hysteresis: it followed the instantaneous leader, which flips every frame, and
   the 800 ms inertia averaged a flickering target down to the mean of the whole
   inventory. The argmax path got the dwell for free through `winner_`. Holding
   the *blend* for the dwell is what opened the regime in the table above.
4. **The dwell itself.** 120 ms was a guess and it left the tract hovering near
   the inventory mean, because `smoothing_ms` is 800 and a posture replaced
   before it is reached is never produced. Swept to 1600 ms, where the spread
   peaks and the effective inventory has not yet collapsed. **The usage histogram
   looked perfect throughout** — all nine postures, effective 8.4 of 9 — which is
   why `babble` reports the produced spread and not only the usage: a dictionary
   that is selecting and a dictionary that is selecting *and being heard* are the
   same histogram.

### DNA v49 — a categorical policy gradient, and discrete selection becomes learnable

v48 above ends by naming what it would take: the dictionary is not steerable by
anything this creature had, because node perturbation writes a per-neuron **bias**
and estimates `Cov(R, xi)`, which needs the output to be a smooth function of
those biases. A discrete selection is a *step function* of them. The gradient is
not buried — it does not exist. Making a categorical choice learnable needs the
estimator a categorical choice actually has, and that is a new learning rule,
which is a decision rather than a tuning step.

**It was taken, and it works.**

For postures sampled from a softmax policy `pi` over the unit activities,

    d log pi(a) / d score_u  =  [u == a] - pi(u)

so the chosen posture's score rises and every other falls in proportion to how
likely it was. Three properties matter:

**Sampling is not optional.** At `dictionary_temp` 0 the policy is deterministic,
`pi` is one-hot, and the term is identically zero — a deterministic policy has no
REINFORCE gradient. Sampling is also where the variability comes from, which for
this readout replaces the role LMAN's motor noise plays for the centroid.

**It is cashed onto synapses, not onto postures, and that is the part that
decides whether it can ever be conditional.** A learned per-posture score is a
*constant*: it can learn "always say /a/" and never "say /a/ when you hear A",
which is precisely the wall node perturbation hits. So the term is written onto
the synapses onto that posture's neurons, gated by the presynaptic trace —
credit lands only on synapses whose source was firing, so two contexts write to
different synapses without the rule knowing anything about either.

**It rides `syn_elig_`**, and therefore the reward cash-in that already bridges
the caregiver's two-second delay. No second trace.

Ships **off**: `dictionary_policy_rate = 0` is bit-identical, hash
`ad96f882becbee92`, `verify` 21 of 21.

#### The dose-response, against a control that reads exactly zero

`vocallearn`'s positive control, at `dictionary_units` 9 and temp 0.35:

| rate | 0.0 | 0.03 | 0.08 | **0.2** | 0.4 | 0.8 | 1.5 |
|---|---|---|---|---|---|---|---|
| `fixed − yoked` | **−0.0** | +10.1 | +10.4 | **+16.4** | −14.2 | −10.0 | −22.3 |
| err early | 0.598 | 0.575 | — | 0.579 | 0.458 | 0.450 | 0.476 |
| err late | 0.604 | 0.521 | — | **0.485** | 0.524 | 0.484 | 0.581 |

Every readout-only configuration in v48 sat at +2.3 / +4.2 / −1.2 / +2.4. Here
the control — sampling with the rule switched off — reads **−0.0**, and the rule
lifts it monotonically to +16.4. **Discrete selection is learnable in this
creature once it is given the right estimator.**

The rate was sized rather than guessed: `a_plus` is 0.010 and the policy writes
once per dwell where STDP writes about eight times, so a comparable rate is
~0.08, and the sweep brackets it.

**Read the two error rows with the `change` row, because they disagree and both
are informative.** `change` is improvement between the first and last third, so
a creature that finishes learning inside the first third has no headroom left
for it to see — which is exactly what the err-early column shows at rate 0.4 and
above (0.458 against 0.598 at rate 0). But those arms then *drift back up*, which
is a learning rate past its stability limit rather than only a metric artefact.
In absolute terms 0.2 is the operating point: final formant error **0.604 →
0.485**, a 20% reduction.

**What is still open, and it is the whole point.** The conditional arm is noise
at every rate (+5.5, −3.5, +2.9): `taught` asks for a map that depends on what
was heard, and nothing supplies the condition to the larynx yet. The rule is
*built* to be conditional — credit lands only on firing sources — but that is a
property it cannot exercise without a zero-baseline conditional input. The
combination this has been building toward, and which this project has never had,
is v49's estimator on v47's substrate. That run is the one that matters and it is
not reported here yet.

### `pgprobe` — the control vocallearn never had, and DNA v49 does not survive it

The v49 section above reports that the categorical policy gradient made discrete
selection learnable: a control reading exactly −0.0 with the rule off, rising to
+16.4 with it on. **That is retracted.** It was measured against a mismatched
yoke, and under a correct one the effect is +2.2 ± 6.0 — indistinguishable from
every readout-only configuration v48 already refuted.

**What was wrong.** `vocallearn` scores its `fixed` arm against a yoke built from
the **taught** arm's reward stream rather than from its own. On the shipped
creature that is harmless — the two streams have near-identical statistics — but
the dictionary creature *samples* its postures, so its yoked arm wanders too, and
its yoke reads +9.6 / +0.2 / −1.5 where the centroid creature's reads −3.7 /
−6.4 / −2.5. A control that moves is a control that can manufacture a
dose-response: as `dictionary_policy_rate` rises the taught arm's reward stream
changes, so the yoke changes, so `fixed − yoked` changes for reasons that have
nothing to do with the fixed arm. **The dose-response was the control moving.**

`pgprobe` gives every arm its own yoke, and adds the control this project has
used for `m3` since the beginning and `vocallearn` never had: a **matched-marginal
random target** — the same two words in the same proportions, drawn independently
of what was heard. A creature that learns nothing conditional and simply sits at
the midpoint of two alternating targets scores identically on `swap` and on
`random`; only a creature whose output depends on its input separates them. So
the conditional quantity is `swap − random`, and `vocallearn`'s `fixed` arm was
never a matched comparison for it.

#### The instrument is not the problem, which is the good news

| creature | `fixed` (positive control) | per seed |
|---|---|---|
| **centroid (shipped)** | **+38.2 ± 3.8** | +40.2 / +30.9 / +43.5 |
| dictionary + policy gradient | **+2.2 ± 6.0** | +6.6 / +9.5 / −9.6 |

The shipped creature reads **+38.2 under the strict instrument against
`vocallearn`'s +36.5 under the loose one.** So the yoke mismatch was inflating
nothing on the centroid, M1c and the +24.0 stand as measured, and the collapse
belongs to the dictionary rather than to the method. **DNA v49 is refuted:
17× worse than the readout it was built to rescue, same seeds, same instrument,
everything but the readout identical.**

#### And the conditional quantity, measured against a matched control at last

| | `swap` | `random` | **`swap − random`** |
|---|---|---|---|
| centroid | −0.7 ± 0.5 | −0.6 ± 0.5 | **−0.1 ± 0.7** |

`heard − random` reads −0.2 on the same run. **Both conditional arms are exactly
zero against a target with the same marginals, with error bars under one point** —
the tightest statement of the conditional wall this project has, and the first
one where the control has the same target distribution as the arm it controls.

> The eleven mechanisms, the seven common-mode walls, `vocallearn`'s +24 against
> −0.1: all of them compared a conditional arm to a *fixed* one. This compares it
> to a target drawn from the same distribution and uncorrelated with the input,
> which is the only comparison that isolates conditionality itself. It reads
> −0.1 ± 0.7.

#### Two bugs this run caught, and one of them printed a verdict

`ctxlearn` printed **CONDITIONAL LEARNING** on `swap` +11.6 ± 25.8 — per seed
+53.4 / −35.5 / +16.7, at a voiced fraction of 0.13 against 0.61 with the oracle
mute. The 2 SE requirement had been added to the LEAD branch and not to the
positive branch, which is the third time this experiment printed through a hole
of that exact shape; it now applies to every verdict, and a level where the
creature has stopped vocalising is unreadable whatever its control reads.

And `pgprobe`'s own summary printed `heard − random` as **+38.8** beside a
`heard` arm reading −0.8, because the derived line indexed `fixed` instead of
`heard`. It was caught only because the two numbers were visibly incompatible —
which is the argument for printing the arms and the derived quantity together
rather than the conclusion alone.

### `g2cond` — the wall is not the readout. It is conditionality.

Every conditional test in this project has been scored on **formant error**
through a population centroid. `pgprobe` puts that at −0.1 ± 0.7 against a
matched-marginal control, and DNA v48 spent a whole mechanism on the theory that
the centroid was the problem. **The question one level up had never been asked:
is the wall the readout, or is it conditionality?**

G2 is the place to ask it, and for one reason. It is a met milestone —
rewarded vocalisations rise ×1.35 within a session, 23 of 27 creatures, 9 of 9 at
420 s — so *reward can shape this* is a result rather than a hypothesis. `g2cond`
copies G2's contingency exactly and changes one line.

| taken from G2 verbatim | |
|---|---|
| the act | a voiced frame, so reward always follows something the creature **did** |
| the class | `vocal_groups()[2]`, the F1 motor group — the thing the brain controls, not the audio it makes |
| the criterion | **this creature's own baseline median** over the first 20%, so hits start at half and **both signs occur by construction** |
| the schedule | feedback over the middle 60%, scoring over the last 20% |

The one change: **which class is praised depends on the word just heard.**
`fixed` praises a hit always, which *is* G2 with a caregiver in the room; `heard`
praises a hit after word A and a miss after word B; `swap` inverts it; `random`
draws the direction independently of the word, in the same proportions.

**And the measure needs no yoke.** Conditionality is scored *within* an arm: hit
rate on trials that wanted a hit, minus hit rate on trials that wanted a miss. A
creature that ignores the word scores zero on that difference no matter what else
it does — which removes the entire class of fault that cost `pgprobe` a run.

#### The result

| | 3 seed families |
|---|---|
| **G2 measure, `fixed`** — test hit − baseline hit | **+0.369 ± 0.031** |
| `heard` — hit\|want − hit\|not | +0.005 ± 0.176 |
| `swap` | +0.051 ± 0.151 |
| `random` — matched-marginal control | +0.029 ± 0.025 |
| **conditional** (best of heard/swap, minus random) | **+0.022 ± 0.153** |

The positive control takes hit rate from **0.500 to 0.913 / 0.885 / 0.811** — a
twelve-sigma effect, inside this instrument, on this contingency. Making the
identical contingency depend on the word produces nothing: per seed `heard` reads
+0.252, −0.335, +0.097 and `swap` reads −0.124, +0.352, −0.074. Both signs, no
consistency, and the matched control at +0.029 ± 0.025.

The two conditional arms and the control also receive **matched feedback
balance** — roughly 50/50 praise and scold, against `fixed`'s 2363/439 — because
half their trials ask for a miss. That is inherent to the design and it is
exactly what `random` controls for.

> **This closes the question the readout work was asking.** DNA v48 was built on
> the theory that the centroid is a second pooling stage and the place the object
> is lost; DNA v49 on the theory that a discrete selection needed its own
> estimator. Both were aimed at the readout. Here the readout is a **binary class
> boundary derived from the creature's own median**, reward moves it from half to
> nine tenths, and it *still* cannot be made to depend on the input. The wall is
> not the formant centroid, not the nine scalars, and not the motor plant. **It
> is conditionality itself, and it is the same on a class boundary as on a
> centroid.**

That is the firmest statement of G3's closure this project has, because for the
first time all three of these hold in one instrument: a positive control that is
a **met milestone** reproducing at 12 SE, a control with **matched marginals**,
and a readout that is **not a centroid**. Every earlier framing had at most one.

### `coderprobe` — the gate that says do not build it

The audio rewrite's third stage was an unsupervised competitive sparse auditory
coder, and its stated premise was that `vocab` reads one-of-eight at **0.210**
against chance 0.125, so the creature has no real word categories. It was the
one part of the proposal that needed no conditionality and it survived
`ctxlearn`, `pgprobe` and `g2cond` untouched.

**The premise was wrong, and one thirty-second experiment says so.** `vocab`'s
0.210 is measured off the creature's **voice** — what a classifier reads from the
echo 200–600 ms after the word stops. It was never a measurement of what the
creature *hears*. Nobody had read the ear.

| readout, one-of-eight, 3 seed families | |
|---|---|
| **mel** — the cochlea's own output, before any neuron | **1.000** |
| **auditory** — the B2 population, binned as every probe here bins it | **0.981** (0.981 / 0.962 / 1.000) |
| shuffled control | 0.122, against chance 0.125 |

> **There is no headroom.** The ear carries the word at 0.981 against a signal
> ceiling of 1.000. A better auditory representation has nothing to occupy, and
> `vocab`'s 0.210 is a fact about what this creature can **say**, not about what
> it can hear. Building the coder would have been improving the one stage that is
> not the bottleneck.

`coderprobe` is `Tier::kFast` and runs in seconds. It is written so that its
only interesting outcome is a refusal — a gate that can only say *build it* is
not a gate — and it follows `shapeprobe`, which licensed DNA v46 by measuring a
front end outside the brain before any of it existed.

#### All three stages of the audio rewrite, priced

| stage | premise | measured |
|---|---|---|
| **0** — a zero-baseline conditional code | the presynaptic baseline is the binding variable | the substrate works at the synapse (1–7% → 28–79% of the larynx's plasticity) and **starves the larynx**: control gone at every level, 5 genomes, 12 driven levels |
| **1** — an unsupervised sparse auditory coder | the ear has no word categories | **the ear reads one-of-eight at 0.981**; no headroom exists |
| **2** — a motor dictionary, then a categorical policy gradient | the centroid readout is where the object is lost | `g2cond`: reward moves a **binary class boundary** from 0.500 to 0.88 and it *still* cannot be made conditional. **The wall is not the readout** |

Three premises, three measurements, and none of the premises survived. What the
programme produced instead is four instruments — `ctxlearn`, `pgprobe`, `g2cond`,
`coderprobe` — and a much sharper statement of the one thing that was always in
the way.

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

### DNA v36 — dynamic synapses, after Webb's cricket: **built, and it is a gain knob here**

The one timescale this brain has never had. Every plasticity mechanism it owns —
R-STDP, the eligibility trace, myelination, synaptic scaling, intrinsic
plasticity — runs on seconds to minutes, and a synapse's *transmission* has been
a constant since M1. Barbara Webb's cricket model puts song recognition nowhere
else: her BN1 fires efficiently only when the gap between sound bursts is long
enough for it to have recovered from synaptic depression, and BN2 only when the
onsets BN1 reports arrive close enough together for facilitation to still be
standing. The bandpass on syllable rate is a side effect of two synapses with
different time constants, and no part of it is learned (Webb & Scutt 2000; Webb,
Reeve, Horchler & Quinn 2003 — see [References](#references)).

Two things about the name, because they change what is worth building. There is
no novel *cell* in that model — the neuron is an ordinary leaky
integrate-and-fire, which this creature has had since M1. And the localisation
half of it has no substrate here: Webb's directional response is a latency race
between two ears, and this creature has one mono cochlea. What ports is the
recognition half, and the recognition half lives in the synapse.

So v36 is a Tsodyks–Markram synapse on any tract that asks for one — `stp_use`,
`stp_recover_ms`, `stp_facil_ms`, all three 0 in the shipped genome, which keeps
the hash at `23c4eb2c7c45d05c`. Release is normalised by U, so the first spike
after a silence delivers exactly the genome's `weight` and switching the
mechanism on is not secretly a recalibration.

**The second reason to want it** was the standing G3 diagnosis. A depressing
synapse transmits its changes and not its steady traffic — it is a per-edge
common-mode remover, the same job DNA v21–v24's pooling interneurons do at
population level. That is the one thing in this project that measurably worked
(+0.077, 3/3 families) and its documented failure mode is common-mode
*swamping*: one shared scalar subtracted from every target. A synapse can only
deplete in proportion to what it individually carries, so it has no shared term
to swamp with.

#### What `stpprobe` measured

Three arms on `auditory→central` — the constant-weight synapse, a depressing
corner (U 0.5, τ_rec 300 ms) and a facilitating one (U 0.1, τ_rec 50 ms,
τ_facil 300 ms) — against amplitude envelopes at a **fixed 50% duty cycle**, so
every rate carries identical total sound and only the timing differs. `gain` is
what the tract delivered as a fraction of its genome weight; `vs off` is
central's spikes-per-auditory-spike against the off arm at the same envelope,
which is where an envelope filter would show up.

```
arm           envelope   aud/tick  transfer  vs off   gain
off           silence    0.70      3.8969    1.000    1.000
              2 Hz       1.12      2.5020    1.000    1.000
              4 Hz       0.86      3.2302    1.000    1.000
              8 Hz       0.78      3.5301    1.000    1.000
              12 Hz      0.69      3.9455    1.000    1.000
              shuffled   0.68      4.0493    1.000    1.000
depressing    silence    0.72      3.7589    0.965    0.686
              2 Hz       1.14      2.3705    0.947    0.643
              4 Hz       0.88      3.0832    0.954    0.664
              8 Hz       0.75      3.5990    1.020    0.680
              12 Hz      0.68      4.0221    1.019    0.695
              shuffled   0.68      4.0267    0.994    0.689
facilitating  silence    0.68      4.2723    1.096    1.670
              2 Hz       1.11      2.8396    1.135    1.808
              4 Hz       0.85      3.5132    1.088    1.749
              8 Hz       0.75      3.9217    1.111    1.732
              12 Hz      0.72      4.0143    1.017    1.739
              shuffled   0.65      4.3147    1.066    1.681
```

The kernel is right: the off arm delivers 1.0000 exactly, the two corners do
opposite things (0.64–0.70 against 1.67–1.81), and the delivered gain correlates
**−0.985** with the traffic the synapse actually carried. That correlation is the
PASS criterion, because it is a statement about arithmetic and a failure there is
a bug rather than a surprise.

**And the `vs off` column is flat.** 0.95–1.02 down the depressing arm, 1.02–1.14
down the facilitating one, with no peak anywhere — including at `shuffled`, which
holds the mean rate of the 4 Hz row and destroys only its regularity. On this
tract a dynamic synapse is a gain knob and not an envelope filter.

> These numbers replace an earlier table taken with a 1500-tick settle, which
> read silence at 1.30 spikes/tick instead of 0.70. The `vs off` conclusion is
> unchanged; the explanation built on the old silence row was not. See the
> section below.

The first explanation offered for that was wrong and is worth recording as such:
it blamed §3.1 for holding `auditory`'s rate flat, on a 2.8% silence-versus-speech
gap that turned out to be a settling artefact. The module's rate moves +60% with
sound. See the section above.

**The real reason is that one dynamic synapse cannot be a bandpass.** Depression
scales everything a synapse transmits by a single number that follows its own
mean rate — a high-pass with no upper corner. `gain` tracks the traffic
faithfully (−0.907) and every spike gets the same multiplier, so the transfer
ratio cannot peak at any envelope. **Webb's bandpass is two stages**: BN1
depressing, feeding BN2 facilitating, with the tuning living in the mismatch
between their time constants rather than in either synapse. v36 built one stage.

The genome can express the second today with no kernel change — a relay module
between `auditory` and `central`, depressing on the way in and facilitating on
the way out. It has not been built, and it is the named next step.

Two instrument limits worth keeping. The cochlea's window is 32 ms with a 16 ms
hop, so an envelope faster than ~15 Hz arrives as steady energy and the probe
stops at 12 Hz — Webb's crickets work at 20–30 Hz syllables through an ear with
microsecond resolution, and the band this ear resolves is 1–12 Hz, which is
where the syllable rate of speech sits anyway. And synaptic scaling still
regulates the *nominal* weight, so a hard-depressing tract delivers less than
§3.1 believes it does; the resting weight is the only rate-independent thing
there is to regulate.

#### A bug this found, which was older than the mechanism

Adding two per-synapse arrays meant reading every place a synapse's state is
moved, and `consolidate()`'s pruning pass moves them: survivors are compacted
down into the vacated slots. It copies target, source, weight, eligibility,
traffic and both delays — and it never copied `syn_elig_mean_`. A synapse that
slid down a slot inherited **DNA v16's eligibility baseline from the synapse
that used to be there**, so reward cashed its trace against another edge's
history.

It has been that way since v16 and nothing caught it, for two reasons that
compound. It needs a sleep prune, so nothing under ~1.04M ticks can reach it at
all. And `elig_baseline_tau_ms` is 0 in the shipped genome, which leaves that
array all zeroes — on the creature everyone actually runs, the bug copies 0 over
0. `determinism` at 1.8M is bit-identical before and after the fix, and the
pinned hash does not move.

On a genome with v16 switched on it moves: `41d3a15bdb42cb47` →
`b163ef30730320f1` at 1.8M ticks. That pair is the evidence the fix is a fix
rather than a no-op, and it is the general shape of this class — a mechanism
that ships off cannot be checked by any test run against the shipped genome.

### The regulator costs dynamic range — and the first measurement of it was wrong

`stpprobe` originally reported that `auditory` emits 1.30 spikes/tick in silence
and 1.33 during speech — 2.3% — and concluded that §3.1's intrinsic plasticity
holds the module so flat that no rate-reading mechanism can see anything. A whole
explanation of DNA v36's null was built on that number and **it was an artefact**.

`ipprobe` was written to make the claim a standing test, and it disagreed by an
order of magnitude. The cause is in the first probe, not the second: `stpprobe`
ran its silent block **first, on a just-hatched creature, after 1500 ticks of
settle**. That is nowhere near enough for intrinsic plasticity, so "silence" was
measured on an unregulated brain at 1.30 while every later block was measured on
a settled one. A settled creature reads silence at **0.72**. With the settle
raised to 20000 ticks the two instruments agree.

What is actually true, from `ipprobe` — silence and speech each read after the
regulator has settled into them:

| `auditory.ip_wake_scale` | silence | speech | gap |
|---|---|---|---|
| 1.00 (shipped) | 0.72 | 1.08 | **+51%** |
| 0.25 | 0.89 | 1.61 | +82% |
| 0.10 | 1.07 | 1.84 | +72% |
| 0.00 | 1.11 | 2.11 | +89% |

So the regulator **narrows** the rate signal — it does not erase it. Relaxing it
returns about half as much again, and the nine-seed guards pass on both arms
(`audio` 9/9, `babble` 9/9, silence→vowel ratio up on 9/9, mean 1.90 → 2.41).
Still not shipped: changing a module's regulation invalidates every calibrated
number downstream of it.

The correction matters more than the finding. A mechanism measured against the
shipped `auditory` module is measured against a signal a regulator has cut in
half — worth knowing — but *"a rate code is not available here"* was never true,
and `ipprobe` exists so that the next person gets the number instead of the
story.

### DNA v37 — burst-dependent plasticity: **built, the tuft steers it, and the burst loses what the plateau had**

The only structurally untried class in the conditioning cap. All five mechanisms
fenced there keep R-STDP's architecture — a *global scalar* third factor
multiplying a local trace — and change something else about it. What has never
existed here is a **per-neuron, input-specific** learning signal, which is what
e-prop argues a spiking network needs ([Bellec et al. 2020][ep]) and what a burst
code is a biological way to deliver ([Payeur, Guerguiev, Zenke, Richards & Naud
2021][bp]).

The rule: a postsynaptic *burst* rather than a single spike carries the learning
signal, and whether a neuron bursts is controlled by its apical dendrite. So

```
dw = burst_learn · eta_scale · credit · (burst_rate_post − burst_base_post)
```

is **signed by the baseline subtraction** — a target bursting above what it
ordinarily does potentiates its afferents, one bursting below depresses them.
That is the precise difference from DNA v29, whose recorded failure is that *the
gate attenuates instead of selecting*: a gate can only change how much is
written, never what.

Two of the paper's three named ingredients were already here — regenerative
apical activity (v25) and plasticity in feedback pathways (v14) — and the third,
short-term synaptic dynamics, shipped the same day as v36. What was missing was
the burst itself: nothing in this kernel had ever distinguished two spikes 5 ms
apart from two spikes 500 ms apart. The tuft gets its say through Larkum's BAC
firing, expressed as a shorter refractory period during a plateau, which is the
same statement about the soma and cannot make a silent module fire.

#### What `burstprobe` measured

The architecture under test is Payeur's mapped onto this body plan:
`vision→vocal` moved onto the tuft, `central→vocal` learning. Three seed
families, 100 trials per arm, 32-permutation nulls.

```
arm          burst%  plat%  burst|plat  burst|no  obj|burst  obj|plat  shuffled
off          0.0     0.0    0.0         0.0       0.500      0.500     0.503
burst        8.2     0.0    0.0         8.2       0.680      0.500     0.491
burst+tuft   13.2    40.5   16.8        6.5       0.800      0.900     0.486
```

**The tuft steers bursting, on 3 of 3 seeds**: 16.8/16.7/16.5 % inside a plateau
against 6.5/6.3/6.5 % outside, a factor of 2.6. The chain the mechanism needs is
real and every link is measurable.

**Why the window is 20 ms and not 5.** The probe tracks every interval at the
larynx, so it can report what any window would have scored:

```
fraction of spikes following another within:
arm           5 ms   10 ms  20 ms  40 ms  80 ms
off           0.1    1.8    8.2    18.9   34.6
burst         0.1    1.8    8.2    18.7   34.3
burst+tuft    1.2    4.9    13.2   24.7   39.3
```

A pyramidal burst in the literature is 100–200 Hz, i.e. 5–10 ms — and **this
larynx does not do that**: 0.1% of its spikes follow another within 5 ms. A code
scoring 1 spike in 1000 is a learning signal that is zero almost everywhere. 20 ms
is the shortest window at which the code is live in this creature.

Note where the *tuft's* effect is largest, though: **11× at 5 ms** (0.1 → 1.2)
against 1.6× at 20 ms. BAC firing produces genuine short-latency doublets, and
the wide window that makes the code usable is also the window that dilutes the
tuft's contribution into it. That trade is the honest reading of this table, and
it is why the whole curve is printed rather than one number.

**And the burst signal discriminates**, which no third factor in this project
ever has. Per-neuron burst deviation at the larynx classifies cube against ball
at **0.673 mean across three seeds (0.800 / 0.600 / 0.620)** against a
32-permutation null of 0.494 — above chance on 3 of 3. `eligprobe` reads the
trace R-STDP multiplies as object-*blind*, +0.93 correlated between the two
conditions.

**The control is what settles it, and it goes the other way.** The plateau the
burst is derived from reads **0.913 (0.900 / 0.960 / 0.880)**. Turning a plateau
into a burst rate is a nonlinearity applied to a signal that was already there,
and it costs 0.24 of object specificity on 3 of 3 seeds. So v37's claim reduces
to: *a signed signal at 0.673 is worth more than an unsigned one at 0.913*. That
is not absurd — v29 had the 0.913 and could not teach with it — but it is not
settled by this probe, and `m3` is what would settle it.

This is the eighth mechanism to meet the same shape of wall, and the first to
carry a discriminating signal into it.

### DNA v38 — competitive pruning: **the change v28/v30 named and declined to build**

§3.6 removes a synapse that is weak **and** idle, and the exuberance post-mortem
measured that the second half is the binding constraint:

> in an exuberant tract every synapse carries traffic because the source fires.
> An absolute traffic floor cannot express what development actually does, which
> is **competition** — a synapse is removed because its neighbours on the same
> target won, not because it fell below a fixed bar.

v38 adds exactly that. A synapse is prunable when its |w| is below
`prune_compete` × the mean |w| over its **own target's** afferents, *with no idle
test at all*. The absence of the idle test is the mechanism. A relative bar has a
gradient by construction — half of any distribution sits below its own mean —
where the absolute one had none: four arms across a 10× range of birth weight
removed at most 58 of ~18,400 exuberant synapses, 0.3%, with no trend.

The mean is computed in a pre-pass, before anything is removed, so every synapse
is judged against the same distribution rather than one its own removed
neighbours had already lowered. `prune_compete_min_in` stops a neuron down to two
inputs from losing the weaker of them every sleep until the neuron pruner deletes
it.

#### It works, and at 0.5 it works far too hard

On the exuberant tract v28/v30 could never prune — `vision→central` at
exuberance 3, born-weak at 0.333, seven sleep passes over 1.5M ticks:

| `prune_compete` | synapses pruned | G4 |
|---|---|---|
| 0.0 (the pre-v38 rule) | **32** | PASS |
| 0.5 | **20,731** | PASS |

648×, and growth still passes its milestone. The requirement the exuberance
post-mortem named is met: selection now has a gradient.

`pruneprobe` measures whether the removal is *selective* rather than merely
large, and its null is not a guess — removing k of a target's afferents at random
leaves the surviving mean |w| unchanged in expectation, so the off arm (same
sleep downscaling, no competition) is the control:

```
arm       pruned  competed  % of  mean|w| in  mean|w| out  change   orphans
off       0       0         0.0   0.14881     0.13451      -9.61    6
compete   8801    8801      26.5  0.14881     0.15266      +2.59    6
```

The survivors are **12.2 points stronger** than the population competition
selected from, and no neuron lost connectivity the off arm kept — the six
orphans are there at birth in both arms. But 26.5% of the brain in five passes
says 0.5 is a decimation rate, not a selection rate. It ships at 0, and anything
that turns it on should start an order of magnitude lower and watch `g4`.

### DNA v39 — a per-module eligibility timescale, and it lands inside a fence

e-prop's one concrete experimental prediction is that *the eligibility trace's
time constant tracks the history-dependence of the postsynaptic neuron*
([Bellec et al. 2020][ep]). This creature has a single global `tau_elig` for
every synapse in the brain, which asserts that the larynx and the association
module credit the past over the same window — and they do not: the larynx has
800 ms of articulator inertia and central carries its object code over hundreds
of ms. `elig_tau_scale` is per module and read on the postsynaptic side, which is
what the prediction is about.

**It is built with its verdict already stated.** "Eligibility distribution" is
one of the five refuted conditioning classes: v16 subtracted a baseline, v17
scaled by the presynaptic mean, v18 selected, and the recorded finding is *"the
distribution is not the problem"*. A per-module timescale is another knob on the
same quantity. It is here because it is cheap, because it makes a named
prediction testable in this creature, and so that the fence covers it explicitly
rather than by analogy — not because the fence was thought to have a gap.

`tauprobe` checks the only thing that can be silently wrong about a four-line
mechanism — a scale read on the wrong side of the synapse, or folded into a
decay already computed, would leave every number unchanged and the field would
look enabled forever. A leaky accumulator's steady state is proportional to its
time constant, so:

| `elig_tau_scale` on central | mean \|e\| central | vs 1× | mean \|e\| vocal | vs 1× |
|---|---|---|---|---|
| 1 | 0.008647 | 1.00 | 0.006279 | 1.00 |
| 2 | 0.013744 | 1.59 | 0.006185 | 0.99 |
| 4 | 0.022824 | 2.64 | 0.006276 | 1.00 |
| 8 | 0.038117 | 4.41 | 0.006297 | 1.00 |

Monotone on the scaled module and pinned at 1.00 on the untouched one, which is
the half that would catch a scale applied globally by mistake. Sub-linear
because the interval is not negligible against tau. **PASS**, and it says the
mechanism runs — not that a longer window buys anything.

### `mechverify` — a pinned hash for every mechanism that ships off

`verify` pins exactly one number, the determinism hash of the shipped genome,
and that pin has paid for itself repeatedly. It also has a blind spot big enough
to hide a bug for four DNA versions: **it is taken on one genome, and a dozen
mechanisms are switched off in it.** Nothing they do is hashed, so nothing about
them can go red.

That is not hypothetical, and the cost is on this page. `syn_elig_mean_` was not
carried through the pruning compaction from DNA v16 until 2026-08-23, so after
every sleep prune a surviving synapse inherited another edge's eligibility
baseline. The pin stayed green throughout, because reaching the bug needs **both**
a sleep prune and `elig_baseline_tau_ms` above zero, and the shipped genome has
neither. Proving the fix was real required a genome nobody runs.

So `mechverify` pins fifteen more hashes, one per off-by-default mechanism, each
on a genome where that mechanism alone is switched on:

```
mechanism                  dna   ticks     expected           measured           verdict
predictive coding          v15   120000    5246e218f7c2b8d6   5246e218f7c2b8d6   ok
eligibility baseline       v16   120000    2ef07ecfd3c1756d   2ef07ecfd3c1756d   ok
presynaptic centring       v17   120000    12c4c9061cc62019   12c4c9061cc62019   ok
per-pathway Hebbian        v23   120000    1fc86fcdc2b06e2d   1fc86fcdc2b06e2d   ok
pooling interneurons       v24   120000    633dcd0314e81925   633dcd0314e81925   ok
apical compartment         v25   120000    73653dbad8466837   73653dbad8466837   ok
oscillations               v26   120000    e77ded57ffba8e6c   e77ded57ffba8e6c   ok
critical period            v28   120000    7dcf64323b9e6ae3   7dcf64323b9e6ae3   ok
plateau-gated plasticity   v29   120000    d0a3b887d3dfd855   d0a3b887d3dfd855   ok
lateral competition        v32   120000    feb08e20bf42c877   feb08e20bf42c877   ok
dynamic synapses           v36   120000    ed756042e4167e0c   ed756042e4167e0c   ok
burst plasticity           v37   120000    d4159bcbf1dddfe2   d4159bcbf1dddfe2   ok
competitive pruning        v38   1300000   8bc54c9948268aed   8bc54c9948268aed   ok
per-module elig tau        v39   120000    6037b59ae289c878   6037b59ae289c878   ok
dendritic error            v40   120000    457a17d1af252433   457a17d1af252433   ok

15 mechanisms, 0 drifted, 0 vacuous, 0 unpinned, 0 broken
```

**The second condition is what makes it worth having.** A pin that matches is
only evidence if the variant differs from the shipped creature at all. An
enabled-but-inert mechanism hashes identically to the off genome, and pinning
*that* locks in a test that cannot fail — which is the failure this project has
paid for under half a dozen names: v18 measured flat, v28 inert by arithmetic,
v35 aimed at a cap that was already closed. So every variant must **match its
pin and differ from the baseline**, and one that does not is reported VACUOUS and
fails.

That check earned its keep before the experiment ever passed once. The v24
variant set `ffi_gain = 0.5` and hashed **identically** to the shipped creature,
because `ffi_source` is a module index defaulting to −1 and the gain alone does
nothing. Without the vacuity test that number would have been pinned and v24
recorded as covered.

Two design notes. `mechverify` ignores `--ticks`; each variant declares its own
length, because a pin taken where the mechanism cannot run is vacuous by
construction. That is why v38 gets **1.3M** and not 120000: competitive pruning
only executes inside a consolidation pass, and the creature does not fall asleep
until ~1.04M ticks.

### The suite, both tiers

Run 2026-08-26 on the shipped genome, with DNA v36–v41 all switched off.

```
--experiment verify           determinism PASS   pinned hash PASS   19 of 19 as expected
--experiment verify-long      determinism PASS   pinned hash PASS   38 of 38 as expected
--experiment verify-teach     the seven hour-scale teaching experiments
--experiment mechverify       15 mechanisms, 0 drifted, 0 vacuous, 0 unpinned, 0 broken
                              2 open milestones still failing, which is what
                              "as expected" means for them
```

Both tiers green, and the pinned hash is still `23c4eb2c7c45d05c` — four
mechanisms and a bug fix later, the creature everyone runs is bit-identical to
the one before them. `mechverify` runs inside the long tier, so a green
`verify-long` now also means every mechanism that ships *off* is unchanged.

**The fast tier cannot see any of this work, and that is the point of running
the long one.** `burstprobe` is `kLong` at a 600000-tick minimum because 200000
gives it 33 trials, and v38's pruning path only executes inside a sleep
consolidation at ~1.04M ticks. A green fast tier says nothing about either. What
the long tier actually reached:

| experiment | what it exercised |
|---|---|
| `burstprobe` | v37's whole chain — burst detection, BAC firing, the object columns |
| `g4` | **7 sleep passes, 25 synapses pruned** as raised and 31 forced — the compaction loop v38 changed, and the loop the `syn_elig_mean_` fix is in |
| `sleep` | fatigue reaching consolidation at 1040 s and waking at 1224 s |
| `snapshot` | the arena arrays v36 and v37 added, saved and restored |

Three of the four new probes are `Expect::kPass` even though the mechanisms they
test ship **off**. That is deliberate and it is what makes them tests rather than
guards: each patches its own arms into a copy of the genome blob, so a green run
means the mechanism was built, hatched and measured — not skipped. `stpprobe`,
`burstprobe`, `pruneprobe` and `tauprobe` all carry an explicit off arm patched
the same way, because an arm that reads its setting from the genome silently
stops being a control the day someone ships the mechanism on.

### Exuberance and competition: the candidate is refuted, and it was the tract all along

DNA v38 gave selection the gradient v28/v30 lacked, which made `projprobe`'s
second named candidate testable for the first time — *"make the tract structured
rather than random, so each target samples a feature-defined group instead of a
uniform random subset"*, built by activity-dependent pruning from an exuberant
start rather than by hand.

Three arms, `m3probe` at 1.6M ticks so sleep consolidates seven times, all three
sharing 4× `max_out_degree` so nothing is dropped and the arms differ in one
thing:

| arm | `vision→central` | central, per-neuron | vision | shuffled |
|---|---|---|---|---|
| **A** | random, no competition | **0.893** | 0.997 | 0.477 |
| **B** | exuberance 3, born weak 0.333 | 0.664 | 0.994 | 0.480 |
| **C** | the same, `prune_compete` 0.5 | **0.613** | 0.994 | 0.492 |

**Refuted, and each step made it worse.** Exuberance alone costs 0.229. Adding
competition costs a further 0.051. Vision is flat across all three at 0.994–0.997,
so the loss is in the tract and not in what feeds it.

The diagnosis is one sentence and it is not about pruning working — v38 removes
20,731 synapses where the old rule removes 32, so it works exactly as built.
**Competition selects on weight, and weight is not a measure of what a synapse
contributes to a distributed code.** Central's object code is *balanced* —
coherence 0.024–0.152, so 85–98% of the discriminative signal is some neurons up
and others down — and in a balanced code the informative synapses are not the
strong ones. Pruning the weak prunes thedifferential as readily as the noise.

That closes the candidate. What it does not close is v38, which is a working
mechanism looking for a criterion: any future use of it needs to rank afferents
by something other than |w|.

### DNA v40 — the dendritic error microcircuit: **built, settles, and finds nothing to cancel**

[Sacramento, Ponte Costa, Bengio & Senn (NeurIPS 2018)][sac]. Lateral
interneurons learn to *predict* the top-down input arriving at a cell's apical
tuft, and the tuft computes the **mismatch**. Right prediction, zero residual,
nothing written; wrong prediction, the residual drives learning. No separate
phases, errors local and continuous in time.

The reason to want it here is v37's specific loss. `burstprobe` reads the raw
plateau at **0.913** and the burst derived from it at **0.673** — a nonlinearity
applied to a signal that was already there costs a quarter of it. An error is not
a nonlinearity on the signal; it is the signal with the predictable part
subtracted, which is the transform this project's standing diagnosis has asked
for eight times: *a small differential riding on a large common mode*.

It needed two genome fields, because the parts existed. `ffi_apical` moves v24's
pooling interneuron from the soma onto v25's compartment; `ffi_learn` lets each
neuron's own `ffi_w_` move until the residual is zero:

```
ffi_w_[i] += ffi_learn · v_apical_[i] · ffi        (clamped at 0 from below)
```

#### What `errprobe` measured

```
arm              resid    plateau%  ffi w     obj|resid  shuffled
soma             0.126    40.6      1.0000    0.940      0.508
tuft, fixed      70.23     0.0      1.0000    0.680      0.503
tuft, learning   0.145    27.8      0.0018    0.940      0.494
```

**A fixed pooling weight cannot land on a tuft at all.** v24's in-degree weight
is calibrated for somatic drive, and on a compartment it over-subtracts so hard
that |apical| sits at 79 and the tuft **never plateaus once**. That is why the
learning arm now starts at zero and grows into its prediction, which is what the
biology does and what the first run of this probe forced.

**And against the arm that is actually a rival, it buys nothing.** `obj|resid`
is 0.940 learning against 0.940 with the interneuron left at the soma. The
converged weight is **0.0018** — the circuit finds almost nothing to cancel, and
the residual is the raw apical signal with a rounding error taken off it.

**The reason is a body-plan limit, not a tuning one.** `ffi` is a pooled rate in
hertz, order 1; the apical input is a sparse tract's per-tick arrival, order 0.1.
One scalar per neuron multiplying a smooth rate cannot track a bursty sparse
input, and the small weight it converges on is the best such a predictor can do.
Sacramento's circuit has an interneuron **population sampled per target** —
which is exactly what `DnaModule::ffi_source` is not, and exactly the shape of
v24's own recorded failure (*one shared scalar for every target*) one level up.
Building that is a body-plan change, not a genome field.

Ninth mechanism into the same wall, and the second in two days to arrive there
by a different road and find the same thing waiting.

### Webb's two-stage circuit, built: **no bandpass, and both controls say so**

`stpprobe` closed v36 with a named next step: one dynamic synapse is a high-pass
with no upper corner, and Webb's bandpass is **two** stages — BN1 depressing
feeding BN2 facilitating — with the tuning living in the *mismatch* between their
time constants rather than in either synapse. That was expressible in the genome
with no kernel change, so it was built: `tools/genome_add_relay.py` grew an
`inhib=0.0` knob and the six v36 fields, and now inserts a Webb pair.

Four arms, because *two stages* and *the right two stages* are different claims,
on the same 50%-duty envelopes `stpprobe` uses:

```
arm          envelope  aud   relay  cen   transfer  vs off  gain in  gain out
off          2 Hz      1.10  0.99   3.63  3.2938    1.000   1.000    1.000
             8 Hz      0.80  0.66   3.27  4.0620    1.000   1.000    1.000
             shuffled  0.72  0.55   3.12  4.3400    1.000   1.000    1.000
dep -> dep   2 Hz      1.12  0.43   3.02  2.6923    0.817   0.639    0.670
             8 Hz      0.84  0.42   2.99  3.5606    0.877   0.670    0.680
dep -> fac   2 Hz      1.08  0.42   3.58  3.3069    1.004   0.652    1.725
             8 Hz      0.78  0.41   3.49  4.4735    1.101   0.685    1.698
             shuffled  0.68  0.40   3.39  4.9711    1.145   0.688    1.691
fac -> dep   8 Hz      0.83  1.87   3.13  3.7893    0.933   1.767    0.369
```

**The stages work exactly as designed.** `gain in` sits at 0.65–0.69 and
`gain out` at 1.69–1.73, moving in opposite directions, which is two mismatched
time constants doing what they should.

**And the mismatch buys nothing.** Three seed families:

| | seed 1 | seed 2 | seed 3 |
|---|---|---|---|
| `dep → dep` spread | 1.087 | 1.098 | **1.210** |
| `dep → fac` spread (Webb) | 1.097 | 1.052 | 1.179 |
| Webb's peak vs its shuffled row | 1.101 / **1.145** | 1.077 / **1.138** | **1.159** / 1.111 |

Two stages with **no** mismatch spread as much as Webb's order — more, on 2 of 3
seeds. And on 2 of 3 the circuit responds *more* to the shuffled train than to
the regular one at the same mean rate, which is the opposite of an interval
filter. Whatever small spread exists is a rate effect the ear already had.

**One correction to this probe's own first criterion.** It offered the off arm's
spread as a noise floor. That is 1.000 *by construction* — the column is that arm
divided by itself — so it is an identity and measures nothing. The two honest
controls were already in the table: `dep → dep` for whether the mismatch matters,
and the shuffled row for whether the interval does. Both were needed; either
alone would have left the result arguable.

**What this closes.** v36's named next step is now measured and negative, which
means the ear is not what stood between this creature and a temporal filter — the
band Webb's circuit needs here (4–8 Hz, speech syllable rate) is well inside what
the 32 ms cochlea already resolves. **So a better cochlea would not have helped**,
and the case for rebuilding the auditory front end is weaker than it looked
rather than stronger.

### Where eleven mechanisms leave it, and the number that redirected the search

Three mechanisms were built and measured in one day — a dendritic error
microcircuit, developmental selection, and Webb's two-stage temporal filter —
by three unrelated routes, and all three found the same wall. That brings the
count to eleven. It seemed worth asking, before building a twelfth, exactly
where the object stops.

`m3probe` at 600000 ticks, 300 trials, the whole chain in one column:

```
vision 0.993  ->  central 0.773  ->  vocal 0.747  ->  voice 0.600   (shuffled 0.440)
```

**The object arrives.** It reaches the larynx at 0.747 and it survives the motor
decoder into the sound at 0.600, against a shuffled null of 0.440. The word does
the same: vocal 0.845, voice 0.741.

This corrected a hypothesis before it became work. A 200000-tick run had shown
the object at 0.740 in vocal and 0.440 in the voice — a total collapse at the
decoder — and the obvious next move was to attack the nine-scalar motor readout.
At 300 trials instead of 100 the collapse is not there: the decoder costs 0.147
on the object and 0.104 on the word, which is a similar toll on both and not an
object-specific one. **The 100-trial version of that table has an accuracy step
of 0.06 and should not be read for differences of 0.10.**

So delivery works, expression is weak but real, and what does not happen is the
creature producing the sound *in response to* the object. That is conditioning,
and conditioning is now fenced eleven times.

**G3 is closed under this architecture, and the useful move is a different
question rather than a twelfth mechanism.** M1b is the result that works — the
creature repeats a heard word at 0.890 with an audible d′ of 1.37, the only
number here that clears the audibility bar — and nothing has been built on it.
The open question it raises has never been asked: **does the echo get better
with practice?** The machinery for that already exists and is the one thing in
this project that has ever worked — node perturbation met G2, so reward can
shape a motor act here. It has only ever been scored on how MUCH the creature
vocalises, never on how well.

### `vocallearn` — does the echo get better with practice? Built, and honest about its own power

Eleven mechanisms have been aimed at G3, which is settled negative. None has ever
been aimed at the thing that works. M1b measures the creature repeating a heard
word at **0.890** with an audible d′ of **1.37** — the only number here that
clears the audibility bar — and nothing has been built on it.

The question it raises: **the creature imitates, does it get better at it?** That
is vocal learning, and it is what the songbird literature this project already
borrows from (LMAN, DNA v10) is actually about — motor variability selected by
how close a rendition lands to a template.

**Why this is a fair question where G3 is not.** Improving the echo does not ask
the creature to *learn* a conditional mapping; M1b measured that it already has
one, delivered by the ear-to-larynx route it was born with. It asks for that
mapping to be refined toward the heard formants. And the rule that would do it is
the one thing here that has ever worked: node perturbation met G2, so reward can
shape a motor act in this creature. It has only ever been scored on how *much*
the creature vocalises, never on how well.

The design is G2's with accuracy in place of rate. Error is
`|log(f1/heard f1)| + |log(f2/heard f2)|` over the voiced frames 200–600 ms after
the word stops — M1b's window, where the ear reads at chance and the voice still
carries the word. Four arms: **taught**, **yoked** (the same praise and scolding
in the same proportions, shifted, for nothing it did), **none**, and **fixed
target** — a positive control that rewards one formant target with no
conditionality at all, which is exactly the act G2 proved reward can shape here.

Two design points that are not details. The reward baseline is **per word**:
against one global mean, a word whose natural posture sits closer would earn
praise every time and the creature would be rewarded for word identity rather
than accuracy — it would learn to say the easy word. And a trial in which the
creature said nothing is **skipped, not scored as maximum error**: silence is not
a wrong answer, and scoring it as one makes "say less" the winning strategy.

#### Reward moves this larynx a long way, and cannot move it conditionally

Three seed families, 3.4M ticks, ~900 scored trials per arm:

| seed | `fixed − yoked` | `taught − yoked` |
|---|---|---|
| 20260809 | +18.3 | +0.7 |
| 20360812 | **+36.3** | −1.4 |
| 20451117 | +17.4 | +0.3 |
| **mean** | **+24.0** | **−0.1** |

```
arm         scored  err early  err late  change   rewards   voiced
taught      913     0.8138     0.8074    +0.8%    3612      0.37
yoked       889     0.8180     0.8172    +0.1%       0      0.32
none        902     0.8051     0.8140    -1.1%       0      0.34
fixed tgt   894     0.4989     0.4070   +18.4%    3231      0.31
```

**The positive control is the largest effect anything in this project has ever
produced on the voice** — an 18–36% reduction in formant error, far past G2's
×1.35 on rate. And the conditional arm is flat on 3 of 3, mean −0.1.

The two arms differ in exactly one thing. Same readout, same rule, same session,
same reward density, same praise/scold balance (1800/1812 against 1723/1508).
Only whether the target the creature is rewarded toward depends on what it just
heard.

**So the wall is not about vision, or naming, or the association module.** M1b
already measured that this creature HAS a conditional route — the ear-to-larynx
pathway delivers word-specific postures at 0.890. What this says is that reward
**cannot reshape that route**. Node perturbation moves a per-neuron bias, which
is a constant: it can shift an entire posture in one direction superbly, and it
has no way to shift it *differently depending on the input*. This creature has
exactly one conditional pathway and it is innate.

That is the same boundary G3 keeps meeting, reached from the one direction that
was supposed to avoid it — and it is the sharpest statement of it on this page,
because for once the positive control sits inside the same instrument.

> **SUPERSEDED on its last sentence.** "Exactly one conditional pathway and it is
> innate" was true when written and is not now: the diagnosis in this section —
> that a per-neuron bias is *a constant* and so cannot be conditional — is exactly
> what DNA v51 fixed by indexing that bias, `bias_[i]` -> `bias_[i][c]`. `areax`
> reads 112.9 Hz of **learned** conditional dF1. The paragraph above is the reason
> v51 exists and is kept for that, but the boundary it names was crossed.

#### The instrument's own power curve, and why the minimum is 3.4M

`fixed − yoked`, in points, is what the positive control buys over its own
control. The experiment refuses to report on the taught arm until this clears 5:

| session | feedback | `fixed − yoked` | `taught − yoked` |
|---|---|---|---|
| 560k | one reward per 2800-tick trial | **−1.2** | −1.8 |
| 560k | G2's clock, every 150 ticks | **+1.0** | −1.4 |
| 1.6M | G2's clock | **+4.0** | −1.8 |
| 3.4M | G2's clock | **+18.3** | +0.7 |

The first version delivered one reward per trial and its positive control moved
*backwards*. G2 delivers every 150 ticks while the creature is making the sound —
nineteen times denser, and the regime this creature's one working learning rule
was actually measured under. That change alone moved the control 2.2 points, and
tripling the session moved it 3 more.

**The control scales by a factor of fifteen and the taught arm does not move at
all.** That curve is why the minimum is 3.4M ticks and why it is set by the
positive control rather than by trial arithmetic: below it the experiment reports
UNDERPOWERED rather than a null, because a creature that will not move on one
fixed target will not move on anything here.

It also nearly produced a wrong answer twice. At 560k with one reward per trial
the taught arm read −1.8 and the control read −1.2 — a null that looked like a
finding and was a fact about reward density. G2 delivers every 150 ticks while
the creature is making the sound, nineteen times denser, and that is the regime
this creature's one working learning rule was measured under.

### M1c — the creature can be TAUGHT a sound, and you can hear it: **met**

`vocallearn`'s positive control was only ever meant to be a control, and it
produced the largest effect this project has had on the voice. It had never been
asked the question everything else here is held to: **is it audible?** That bar
exists because "cube and ball produce distinguishable vocalisations" was true at
0.75 in a readout and inaudible to any listener.

`teachsound` asks it. The caregiver says **"ball"** (an open /a/) and praises the
creature toward **"cube"** (a close /i/) — deliberately a *different* vowel from
the one it hears, so a shift toward the target cannot be the innate ear-to-larynx
pathway doing its job. Praise when a moment lands closer to the target than this
creature usually gets, a mild no when further. Nothing else.

Three seed families, 3.4M ticks, ~930 scored trials per arm:

| seed | error vs yoked | d′ taught | d′ yoked | null |
|---|---|---|---|---|
| 20260809 | +22.8 | **8.669** | — | −0.011 |
| 20360812 | +18.8 | 5.665 | — | −0.065 |
| 20451117 | +15.4 | 8.164 | — | −0.011 |
| **mean** | **+19.0** | **7.50** | — | **−0.029** |

**Re-measured 2026-08-29, after DNA v44/v45's rebound shipped.** These replace
the pre-M1d numbers, which were +12.9 / +23.5 / +11.3 and d′ 5.298 / 7.093 /
4.326, mean +15.9 and 5.57 — taken on a creature that babbled through the
caregiver. The mean rose on both measures and **2 of 3 families moved that way,
not 3 of 3**: seed 20360812 fell on both. Against a family spread of d′ 4.3 to
8.7 that is suggestive and not established, so the claim here is that M1d cost
M1c nothing, not that it improved it. The milestone is met on all three either
way, every d′ far above the audibility bar of 1.0.

And the formants move the right way on 3 of 3 — toward a target of F1 320,
F2 2500:

```
F1   609 → 564    605 → 535    605 → 560
F2  1685 → 1789   1707 → 1864  1695 → 1776
```

**d′ 5.57 mean.** At d′ = 1 a listener gets about 76% right in a two-alternative
forced choice; M1b cleared the bar at 1.37. This is four times that. `--wav`
writes the early utterance, the late one and the target, half a second each, so
the number is checkable by ear rather than taken on trust.

**One caveat, and it is the reason the yoked arm exists.** The creature's voice
drifts on its own: the yoked arm reads d′ 0.585 / 1.836 / 0.833, and on one seed
that drift is itself above the audibility bar. So the claim is not "the voice
changed" — it is that the taught change is **three to four times the drift on
every seed**, with the error moving toward the target only in the taught arm
(+15.9 points against −2.8 to +0.8).

**What is new about it.** G2 was taught behaviour too, but it changed how *much*
the creature vocalises. This changes **what it says**, and a listener can tell.
It is the first taught change to the content of this creature's voice, and it
rests entirely on the one learning rule here that has ever worked — node
perturbation, which shifts a whole posture in one direction and, as
`vocallearn` established on the same day, cannot be made conditional.

So the creature's abilities divide cleanly. **What it says in response to what it
hears is innate** and cannot be reshaped by reward. **What it says can be taught**,
as long as you are teaching it one thing.

### `retain` — the creature keeps what it was taught, sleep does not erase it, and a second lesson does

M1c gave this project its first taught behaviour that a listener can hear, which
made three questions answerable that never had been. Sleep, replay, downscaling
and myelination all ship and all pass G4, and **none had ever been shown to do
anything for learning** — there was nothing learned to test them against. Worse,
the prior was hostile: DNA v9 exists because awake homeostasis was measured to be
**G2's eraser**, and sleep downscaling is the same shape of mechanism run harder,
over every synapse, once a bout.

One creature, three phases, one continuous life: teach toward /i/, intervene,
re-measure with no reward. The arms differ only in the middle.

```
retention = (err before teaching - err after intervening)
            / (err before teaching - err after teaching)
```

Three seed families, 5.6M ticks each:

| arm | seed 1 | seed 2 | seed 3 | mean |
|---|---|---|---|---|
| **quiet** (sleeps when tired) | 1.21 | 1.23 | 1.50 | **1.31** |
| **no sleep** (`fatigue_rate` 0) | 1.14 | 1.22 | 1.41 | **1.26** |
| **relearn** (a conflicting lesson) | 0.56 | **0.03** | **0.06** | **0.22** |
| **never taught** (`settle` control) | 1.000 | 0.995 | 1.002 | **0.999** |

**What is taught is kept, and keeps improving.** Retention above 1 on 3 of 3 —
the creature drifts slightly further toward the target after the praise stops.

**Sleep is not an eraser.** Six bouts cost +0.06, +0.01, +0.09 — nothing, on
3 of 3, and the sleeping arm keeps marginally *more*. That is the first time any
part of §3.6 has been shown to be harmless to a learned behaviour, let alone
helpful.

**A conflicting second lesson erases the first.** Mean retention 0.22, and on two
of three seeds it is **0.03 and 0.06** — back to baseline as if never taught. The
formants move exactly as interference predicts: F1 564 → 580 (back up, away from
/i/'s 320 and toward the new target's 850) and F2 1784 → 1727 (down, away from
2500).

**"One lesson at a time" was the wrong reading of this, and `capacity` below is
what corrects it.** Both lessons here move the *same thing*: the reward is
|log(f1/target)| + |log(f2/target)|, one scalar over both formants, and the two
vowels pulled F1 and F2 in opposite directions. The interference is real and the
generalisation from it was not. On orthogonal targets the same creature holds
**two** lessons at once.

#### The control refuted the hypothesis it was built to measure

The `never taught` arm exists because the first version of this experiment
returned retention above 1.0 in every arm and I did not believe it. The proposed
explanation was an artefact: reward drives node perturbation, perturbation is
trial-to-trial scatter, the formant error is convex, so switching reward off
should lower the measured error without anything being learned.

It reads **0.999**. `err before 1.0851 → taught 1.0858 → after 1.0861`, flat on
3 of 3. There is no such artefact, the hypothesis was wrong, and retention above
1 means what it says.

#### Three faults this experiment had before it had a result

Each was found by the data rather than by inspection, and each would have
produced a publishable-looking number.

**The interference arm did not interfere, twice.** The second lesson was first
"boot" /u/, whose F1 is 350 against /i/'s 320 — the two lessons *agree* on F1, so
teaching the second improved the first one's score and the relearn arm came back
retaining 1.46, *more* than quiet. Replacing it with "bed" /e/ (550, 1850) was
worse in a more interesting way: that is almost exactly where the creature sits
after learning /i/ (564, 1784), so the second lesson was "stay where you are".
The target is now stated outright — (850, 1100), far from /i/ on both formants,
far from where teaching leaves the creature, and never heard.

**The control measured nothing.** The never-taught arm's scoring window was gated
on the same flag as its reward, so `err_taught` came back 0.0000 and its `settle`
of 1.000 was a default rather than a measurement. The phase a trial is in and
whether that arm is being taught are two different things.

**And one seed told a different story from three.** On the first seed the relearn
arm retained 0.56, which reads as "half survives, so there is more capacity here
than `vocallearn` implied". On the other two it is 0.03 and 0.06. The one-seed
reading was wrong and the three-seed one agrees with everything else on this page.

### `capacity` — teaching has at least two degrees of freedom, and they compete

`retain` found that a second lesson erases the first and this page read it as
"one teachable scalar". That reading was confounded, and the confound was ours:
both of its lessons moved the same two formants through one summed error, so a
collapse could equally have meant *the targets overlapped*. The two readings have
opposite consequences — one says every future milestone is a single setpoint,
the other says orthogonal lessons would have coexisted — so the question was
worth an experiment rather than a paragraph.

The larynx makes the test possible. F1 and F2 are read from two **separate**
population-coded groups (§5.3, `group_value_[2]` and `[3]`), so independent
control is structurally available even if learning cannot use it. Lesson A is
scored on F1 alone, lesson B on F2 alone, and their joint target (320, 2500) is
"cube" /i/ — the vowel `teachsound` already proved reachable. One known-reachable
target, split into two orthogonal halves.

Five arms, and most of the value is in the three controls:

| arm | teach phase | second phase | what it is for |
|---|---|---|---|
| `A only` | F1 | — | A undisturbed; its F2 column is the **yoke check** |
| `A then A` | F1 | F1 | **reward kept running** on the same lesson |
| `A then B` | F1 | F2 | the test |
| `A+B` | both | — | **reachability** of the pair |
| `never` | — | — | the scatter control `retain` needed and lacked |

Every arm is scored on both errors in every window against the fixed targets,
whatever it was taught; scoring each arm against its own lesson would measure a
different quantity in each arm. Three seed families, 5.6M ticks each:

| | seed 1 | seed 2 | seed 3 | mean |
|---|---|---|---|---|
| A undisturbed | +0.335 | +0.344 | +0.310 | **+0.330** |
| A with more A | +0.511 | +0.459 | +0.495 | **+0.488** |
| **A after B** | +0.289 | +0.371 | +0.176 | **+0.279** |
| **B landed** | +0.206 | +0.129 | +0.197 | **+0.177** |
| yoke check | +0.069 | −0.067 | +0.013 | **+0.005** |
| A retained | 0.86 | 1.08 | 0.57 | **0.84** |
| A against more A | 0.57 | 0.81 | 0.36 | **0.58** |
| interference d′ | 4.30 | 4.83 | 7.34 | (nulls −0.14, −0.06, −0.03) |

**Both lessons coexist.** A keeps a mean **0.84** of its gain while B lands on
3 of 3. On the same footing `retain`'s *conflicting* second lesson left **0.22**.
The difference between 0.84 and 0.22 is the whole finding: interference in this
creature is about competing for the same output dimension, not about a single
teachable scalar.

**The second degree of freedom is real and is not free.** Against `A then A` —
reward running in both arms, so the only difference is *which* lesson it went to
— A keeps only **0.58** of what continuing A would have bought. Two teachable
dimensions, one reward channel, and they compete for it.

**Teaching both at once is worse than teaching them in turn.** The `A+B` arm gets
+0.132 / +0.244 / +0.182 on F1 where `A only` gets +0.335 / +0.344 / +0.310 on
the same teaching budget. One scalar reward split across two dimensions does each
of them worse than spending the whole budget on one and then the other.

**The formants really are independent here.** The yoke check — F2 in the arm that
was never taught B — averages **+0.005** across seeds (+0.069, −0.067, +0.013).
Seed 1's positive reading alone would have looked like coupling; three seeds say
it is noise. Without this the orthogonality the experiment rests on would be an
assumption rather than a measurement.

#### The verdict line was fitted to the data, and that is why it is gone

The first version printed a **binary verdict** thresholded at −0.25 of A's gain.
The three seeds straddled it — costs of −0.14, +0.08 and −0.43 — and it returned
"two degrees of freedom" twice and "one" once from what is plainly one
distribution. The quantity is continuous and its seed spread is wider than any
line drawn through it.

The fix was **not** to move the threshold until the seeds agreed. That is fitting
the verdict to the data, it would have worked, and nothing in the output would
have shown it had happened. The readout now prints the two ratios and states
outright that one seed cannot settle the question; the reasoning sits in a
comment at the decision so the next person cannot quietly undo it. This is the
same failure mode as `retain`'s 0.56 seed, caught one stage earlier.

`capacity` refuses rather than nulls in three ways: **UNDERPOWERED** if lesson A
never landed, **VOID** if the `A+B` arm cannot hold both targets even when taught
both at once — which would make a sequential collapse anatomy rather than
interference — and **YOKED** if F2 moves toward B's target in the arm that never
learned it.

### `credit` — pricing a mechanism before building it, and the answer is that the prize is real

`capacity` left one number pointing somewhere specific. The two lessons compete
— the first keeps only **0.58** of what continuing it would have bought — and
reading `apply_reward_impl` says why. Node perturbation, the rule that actually
shapes this larynx, nudges `bias_[i]` for **every** neuron in the motor module,
scaled by one broadcast scalar. While lesson B is being taught, the F1 group's
neurons keep receiving updates driven by a reward uncorrelated with anything
they did, so what A taught them random-walks away. The two groups are disjoint
populations; the *reward* is what they share.

The mechanism that would fix that is a neuromodulator reaching some neurons and
not others. DNA v20 already splits reward into four channels — and **cannot do
this**, because its gains are per *module* and both lessons live in `vocal`.
Per-neuron gating is kernel surgery, so it is worth knowing whether it would pay
before writing it.

`Network::set_reward_mask` hands the creature the credit assignment it cannot
compute: while lesson A runs, reward reaches only the F1 group; while B runs,
only F2. **This is an oracle, not a mechanism.** It is not learnable, no genome
field reaches it, the shipped creature never sets it, and the pinned hash does
not move. What it measures is the ceiling — if perfect targeting existed, how
much of the interference goes away? A "none" would have closed the entire line
for the price of one experiment. That is the move the oracle fovea made for
foveation, and it reversed that decision.

Three seed families, 5.6M ticks, retention read **within** each condition
(`A then B*` against `A only*`) because a mask removes plasticity and a masked
arm may simply learn less:

| | seed 1 | seed 2 | seed 3 | mean |
|---|---|---|---|---|
| A retained, **broadcast** | 0.86 | 1.08 | 0.57 | **0.84** |
| A retained, **targeted** | 1.06 | 1.09 | 0.95 | **1.03** |
| change | +0.20 | +0.01 | +0.39 | **+0.20** |
| A gain, broadcast to targeted | 0.335 / 0.281 | 0.344 / 0.228 | 0.310 / 0.183 | 0.330 / **0.231** |
| B landed, broadcast to targeted | 0.206 / 0.159 | 0.129 / 0.129 | 0.197 / 0.143 | 0.177 / **0.144** |

**Targeted retention lands at ~1.0 whatever the broadcast arm read** — 1.06,
1.09, 0.95 from starting points of 0.86, 1.08 and 0.57. The interference does
not shrink by some fixed amount; it goes to zero. And the size of the gain
tracks how much interference there was to remove (+0.39 where broadcast read
0.57, +0.01 where it already read 1.08). A mechanism that removed something
other than the broadcast would not produce that correlation.

**It is a trade, not a free win.** Lesson A lands 30% weaker under the mask
(0.330 to 0.231) and B 19% weaker, because node perturbation searching **14
neurons instead of 126** is a weaker search. Confining the reward and confining
the exploration are the same act here. So the honest statement is that perfect
credit assignment converts a *broadcast-interference* problem into a *slower
learning* problem — and that is the bar a real DNA v41 has to clear while also
**discovering** the assignment rather than being handed it.

**What this does not show.** Nothing here says the creature could learn which
neurons deserve the reward. That is exactly what burst plasticity (v37) and the
dendritic error microcircuit (v40) were attempts at, and both are recorded above
as refuted. `credit` says the prize is worth another attempt; it does not say an
attempt would succeed. Two experiments with the same shape — an oracle that
works and a learnable mechanism that does not — is the pattern this project has
hit repeatedly, and it is the reason the oracle is labelled as one everywhere it
appears.

### `driftprobe` — the interference was never a credit-assignment problem

Everything above calls `capacity`'s interference a credit-assignment failure.
That framing is wrong, and this is the correction. Node perturbation already
assigns credit correctly **in expectation**:

```
d bias_i  ~  R * perturb_i        so       E[d bias_i]  ~  Cov(R, perturb_i)
```

`perturb_[i]` is the neuron's own injected noise, independent across neurons and
independent of the reward except through that neuron's causal effect on
behaviour. For a neuron with no effect on the current lesson's reward the
covariance is **exactly zero** — the rule is already telling it "you get
nothing". What it cannot do is deliver zero on any single sample. It delivers
zero-mean *noise*, and zero-mean noise applied to a standing bias is a random
walk.

So the prediction is specific: during lesson B the F1 group should DIFFUSE while
the F2 group DRIFTS. The decomposition has to respect what the larynx reads — a
group's output is a population centroid, so a uniform shift of every bias in the
group moves nothing — and the change vector is split along the axis that moves
the readout and perpendicular to it.

| seed | taught group drift/rms | untaught group drift/rms | untaught total motion |
|---|---|---|---|
| 1 | 0.744 | 0.083 | 69% |
| 2 | 0.860 | 0.117 | 61% |
| 3 | 0.919 | 0.353 | 48% |
| **mean** | **0.841** | **0.184** | **59%** |

The untaught group's *diffusion* is essentially identical to the taught group's
(0.0164 against 0.0160 on seed 1) with nine times less drift, and teaching B
raises it **5.6x** over the quiet arm. Same noise, no signal.

**This retires three refutations at once.** Burst plasticity (v37), the
dendritic error microcircuit (v40) and plateau gating (v29) were all attempts to
build a better *third factor* — a richer signal about who is responsible right
now. If the covariance is already right, they were answering a question the
learning rule had answered. The probe carries the falsifier in its own output:
had the untaught group drifted as directionally as the taught one, it prints
`IT IS A CREDIT PROBLEM AFTER ALL` and this section would not exist.

### DNA v41 — metaplastic consolidation, and one of its two gates works

If the problem is variance rather than credit, the mechanism to build is one
that stops what has already been decided from moving. v41 offers two gates that
ask that question different ways. Both ship off; both were measured in the same
`metaprobe` session against the same creature and seed, so nothing else can
differ between them.

**The moment ratio (S).** Each neuron gates its own plasticity on
`E[u]^2 / E[u^2]` over its own updates — 1 if every update agrees, 0 if pure
noise. Local, no teacher, two floats per neuron.

**The commitment brake (C).** The bias IS the accumulated evidence: a neuron
driven consistently walks away from zero, one fed noise stays near it, and the
distance integrates over the whole lesson rather than over a window.

```
gate = 1 - (1 - meta_floor) * meta_commit * |bias| / perturb_max
```

It costs **no new state at all**, because the quantity it reads is one the
kernel already keeps. That is Fusi's cascade in its simplest form (Fusi, Drew &
Abbott 2005; Benna & Fusi 2016), and the standard soft weight bound arrived at
from the other direction.

Six seed families, 5.6M ticks, against the `credit` oracle's 1.03 retention and
0.231 A-gain:

| seed | off | **commitment (C)** | change | C's A gain | moment ratio (S) |
|---|---|---|---|---|---|
| 1 | 0.86 | 1.22 | +0.36 | 0.159 | 0.94 |
| 2 | 1.08 | 0.85 | **-0.23** | 0.368 | 0.38 |
| 3 | 0.57 | 0.86 | +0.29 | 0.257 | 0.54 |
| 4 | 0.94 | 1.00 | +0.07 | 0.329 | 0.86 |
| 5 | 0.74 | 0.86 | +0.12 | 0.087 | 1.18 |
| 6 | 1.09 | 1.78 | +0.68 | 0.114 | 0.58 |
| **mean** | **0.88** | **1.10** | **+0.21** | **0.219** | **0.75** |

**The commitment gate reaches the oracle without being one.** Retention 1.10
against the oracle's 1.03 and 0.88 for doing nothing, at an A-gain of 0.219
against the oracle's 0.231. A purely local rule, reading a quantity that already
existed, lands where perfect targeting landed.

**The qualifications are not small.** It is 5 of 6 by sign, not 6 of 6. Seed 2
LOST 0.23, and it is the seed that had no interference to fix — the brake stops
the spontaneous post-lesson improvement as well as the erosion, so what it
really does is clamp retention toward ~1 from both directions. Seed 6's +0.68 is
an outlier pulling the mean up; the median is +0.205. A's gain falls 14% on
average and ranges from 0.087 to 0.368 across seeds.

**Benna-Fusi's two-compartment store is refuted too, and it was meant to be the
improvement.** Both gates above work by refusing updates, which is why the
commitment brake costs learning rate, so the obvious fix was a mechanism that
refuses nothing: make the bias the fast variable of a chain coupled to a slow
store. The prediction stated in advance was that it would cost no learning rate
at all. **That prediction was wrong, and the reason is one line of algebra that
should have been done before the first run.**

The exchange conserves `ratio * bias + slow`. That is the property that makes it
a store rather than a leak — nothing drains to zero. It also means that starting
from an empty store the pair settles at `bias = ratio * B / (1 + ratio)`, so the
readout keeps `r/(1+r)` of the lesson and the effective learning rate is scaled
by the same factor. `meta_flow` sets only how fast that happens; it cannot
change it.

| ratio | keeps | seed 1 | seed 2 | seed 3 |
|---|---|---|---|---|
| **0.3** | 23% | froze (-0.045) | froze (-0.056) | froze (-0.038) |
| **1.0** | 50% | ret 1.31, gain 0.075 | ret 1.05, gain 0.102 | ret 0.98, gain 0.037 |

At r = 0.3 the creature freezes on 3 of 3. At r = 1.0 it does not freeze, and it
posts the best retention numbers of anything tried — and fails the joint bar
outright: A's gain falls from a mean of 0.330 to **0.071**, a 78% cost, and
lesson B is blocked on 2 of 3 seeds (+0.002 and +0.003). It bought retention by
learning less, which is exactly what that bar exists to catch. The measured cost
is worse than the 50% the algebra predicts, because the leak damps the
systematic drift during the gap as well as the noise.

So the cost is **structural to the formulation**, not a tuning failure: two
compartments cannot separate the noise from the signal when both arrive through
the same variable. The crude brake it was built to improve on is better here.

**And it does not ship, because the gain does not transfer to a milestone.**
`metaprobe`'s +0.21 is measured on its own internal retention ratio, so the
commitment gate was run against `retain` — a milestone experiment — on three
seeds:

| arm | brake off | brake on |
|---|---|---|
| quiet | 1.21 1.23 1.50 = **1.31** | 1.24 0.89 1.37 = **1.17** |
| no sleep | 1.14 1.22 1.41 = **1.26** | 1.05 0.91 1.02 = **0.99** |
| relearn | 0.56 0.03 0.06 = **0.22** | 0.45 -0.32 0.65 = **0.26** |

It **costs** the headline retention that `retain`'s result rests on, and does not
reliably buy interference resistance — 0.22 to 0.26 is nothing against seed
swings of 0.45, -0.32 and 0.65. It also drops the babble duty cycle from 0.78 to
0.50. So it stays off, with a measured reason rather than caution.

The general lesson is worth more than the mechanism: **a gain measured on the
probe built to show a mechanism working is not a gain.** Test against a milestone
before shipping. And one seed here read relearn 0.06 -> 0.65, which looks exactly
like the mechanism doing its job; the other two read 0.56 -> 0.45 and
0.03 -> -0.32.

**The moment ratio is refuted**, at a mean of -0.13 and swings from +0.44 to
-0.70. Its measured SNR separation is 0.0025 against 0.0024 — no separation at
all — and that is a design flaw with a name rather than bad luck: the statistic's
noise floor is `1/meta_window`, a lesson holds only a few thousand reward
events, and there is no headroom to work in. Two guesses at its threshold (0.15,
then 0.02) pinned every neuron to the floor and froze the creature before the
third was measured from the data.

#### Four constants guessed instead of derived, at a full-length run each

This mechanism cost more compute in wrong constants than in wrong code, and all
four were one line of arithmetic away from being right the first time.

| constant | guessed | what the system said |
|---|---|---|
| `meta_ref` | 0.15, then 0.02 | measured SNRs live near 0.002; both pinned every neuron to the floor |
| `meta_flow` | 0.02 | cash-ins run at 100 Hz, so that is a **half-second** store against a 3360 s lesson |
| `meta_ratio` | 0.05 | the pair conserves `r*bias + slow`, so the readout keeps `r/(1+r)` = **5%** |

The pattern is the same each time: a number was picked from intuition when the
system's own arithmetic determined it. The `meta_flow` and `meta_ratio` errors
are the worst of them, because each produced a *frozen creature* — a result that
looks like a refutation of the idea rather than of the constant, and would have
been recorded as one if the probe had not been built to say `FROZE THE CREATURE`
instead of printing a retention ratio computed from a near-zero denominator.

#### Three bugs, and two of them looked like success

The v41 arrays were allocated inside `if (any_burst_)`, so they came back null on
any genome without a burst code and the probe segfaulted. `meta_alpha_` was
assigned *after* the allocation that tested it. `required_bytes` did not count
the new arrays, so the arm failed to hatch.

The first of those was invisible twice, and the reason is worth keeping: the run
was piped to `tail`, so `$?` reported **tail's** exit status and the segfault
read as success — and stdout was block-buffered, so the crash discarded every
line the experiment had printed and it looked like a silent clean exit. Run it
unpiped, and use `stdbuf -o0`, before believing a fast quiet finish.

### `trajprobe` and `seqprobe` — an utterance is a held vowel, and nothing here can hold a sequence

> **Read the second half of this heading with the travelling-wave section
> below.** "Nothing here can hold a sequence" was measured with population-vector
> correlation, which demands the same neurons in the same bin across repeats and
> is therefore structurally blind to a wave whose SPEED jitters. Measured
> per-repeat instead, `central` carries a travelling wave at travel r +0.583
> against a +0.035 null. The claim about the utterance — the first half —
> stands: it is about the larynx's output, not about the instrument.


Every taught result on this page teaches a **setpoint**. M1c moves the creature's
vowel toward a target; `capacity` teaches two formants to two values;
`vocallearn` scores the distance from a fixed pair of numbers. Nothing has ever
asked the larynx for a trajectory, and these two probes are why that was never
an oversight.

**An utterance has no shape.** `trajprobe` records every voiced run the creature
produces alone, mean-subtracts each one so the question is about SHAPE and not
about which vowel was said, resamples it to eight bins, and asks whether
different utterances share a time course. Three seed families, ~1000 utterances
each:

| | one utterance ranges | shared with the others |
|---|---|---|
| F1 | ~10 Hz | 1.2% / 5.1% / 8.1% |
| F2 | ~26 Hz | 1.0% / 3.7% / 1.9% |

The formants barely move *within* an utterance — 10 Hz against the 750 Hz of F1
the body plan allows — and almost none of that little movement is shared between
utterances. There is variability and no sequence, and **reward cannot select a
trajectory that is never repeated.**

§5.3's own comment named the missing piece years before this was measured: "HVC
drives RA reliably and LMAN adds variance on top. This is the HVC term." The
creature has an LMAN — node perturbation, DNA v10 — and an RA, which is `vocal`.
What it has in place of HVC is `drive_compensation`: a **scalar** steady
depolarisation. A constant cannot carry a sequence.

**And no module can hold one.** The substrate looked promising and nobody had
looked at it: `wire_intra_module` gives every module dense local recurrence
(density 0.5 inside radius 0.4, weight 0.12, 20% inhibitory). `seqprobe` kicks a
fixed 5% of `central`, removes the kick, and correlates repeats of the same kick
against each other — so the creature's own spontaneous activity is uncorrelated
by construction and acts as the null.

| w_rec | during the kick | 10 ms later | seized? |
|---|---|---|---|
| 0.120 (shipped) | 7 → **33 Hz**, r = **0.92** | r = 0.034 | no |
| 0.240 | 9 → 39 Hz, r = 0.89 | 0.057 | no |
| 0.480 | 13 → 46 Hz, r = 0.81 | 0.051 | no |
| 0.960 | 13 → 45 Hz, r = 0.78 | 0.060 | no |

The kick lands hard — five times baseline in a pattern reproducible at 0.92
across 24 repeats — and one bin later it is at 0.03. The trace does not decay,
it **vanishes**. Eight times the recurrent weight moves the rate from 6 Hz to
11 Hz and the reliability not at all: the loop is not near an interesting
operating point, it is nowhere near one. This is DNA v14's finding about
feedback *between* modules, now measured *inside* one.

So a generator needs **new structure, not a new constant**. Distance-based
symmetric wiring cannot carry activity forward; a sequence needs asymmetry — a
chain where one population drives the next — which is something this
architecture has never had anywhere.

#### Both probes first printed a confident number computed on absent data

`trajprobe`'s split-half correlation read **-0.940** on one seed, which looks
like a strong finding until you notice the shared shape it is correlating spans
**0.3 Hz**: two noise vectors normalised against each other, and the value can
come out anywhere. The probe now leads with the amplitude ratio and prints the
correlation as confirmation only.

`seqprobe`'s `changes r` read **1.000** at every weight, which reads as "a frozen
attractor". It was comparing bin 0 with bin 0, because nothing had persisted. It
now prints `-`.

Neither number was wrong arithmetic; both were statistics computed where there
was no data to compute them on, and both looked like results. The same shape of
mistake as the frozen-creature retention ratios above.

**`seqprobe` also shipped without its positive control and was rewritten to have
one.** "No persistence" and "the kick never landed" are the same table of zeros.
The control — the pattern DURING the kick must be loud and reproducible — is
what turns this from a shrug into a measurement, and it is checked per weight
rather than once.

### `vocab` — the creature recognises four words and not eight

`imitate` scores four words in all six pairs, 200-600 ms AFTER the word stops, and
every pair clears 0.75. The decisive one is /i/ against /u/: their F1s are 30 Hz
apart and their F2s 1600, so it can only be answered on F2 — and it reads
**0.900**, against 0.933 for the maximally-separated pair. The creature is not
running a brightness meter. It carries which vowel it heard, in its voice, after
the sound is gone, and it survives a microphone at 0 dB SNR (0.807, shuffled
control 0.508).

`vocab` asks the recognition twin of what `capacity` asked about teaching: how
many? Four vowels are appended to `kWords` that CROWD the original four rather
than filling the gaps between them — /o/ 50 Hz from /u/ on F1, /ae/ 140 from /a/,
/^/ between /a/ and /e/, /I/ 550 below /i/ on F2 — because a vocabulary that only
grows into empty space measures the size of the space and not the creature. All
28 pairs are scored off one simulation in the same window.

| hardest pairs | voice | dF1 | dF2 |
|---|---|---|---|
| /e/ bed - /ae/ bat | **0.569** | 140 | 150 |
| /e/ - /I/ bit | 0.664 | 150 | 100 |
| /a/ - /^/ but | 0.679 | 160 | **20** |
| /e/ - /o/ boat | 0.691 | 100 | 1000 |
| /u/ - /o/ | 0.701 | 100 | 50 |

Mean 0.786, but **12 of 28 pairs fall below 0.75** and the worst is 0.569 against
a chance of 0.5. At four words every pair cleared 0.753.

**The number that settles it is one-of-eight: 0.210 against a chance of 0.125.**
A pairwise table saying every pair is separable does not say a word can be picked
out of eight — that needs every boundary to hold at once. The gap between 0.786
pairwise and 0.210 eight-way is the difference between "these two sounds are
different" and "the creature has a vocabulary", and it is large.

#### The verdict line passed a saturated table, and had to be tightened

The first version asked whether MORE THAN HALF the pairs cleared 0.75, and duly
printed `EIGHT WORDS HOLD APART` over a table whose worst row was 0.569 against a
chance of 0.5, with twelve of twenty-eight failing. Four words clear every pair,
so "most of them" was never the standard the smaller vocabulary already met. The
bar is now every pair AND a one-of-eight score well clear of chance.

The return value then contradicted the verdict: `vocab` printed
`THE VOCABULARY IS FULL BELOW EIGHT` and returned **true**, so `verify` reported
"a milestone this project has never met just did". That message is the loudest
one the harness has and it exists for exactly this — had `vocab` been marked
`Expect::kPass` instead of `kOpen`, the same bug would have been completely
silent and this page would now claim eight words hold apart directly above a
table showing twelve failing pairs.

Appending to `kWords` also came within one constant of silently rewriting a
milestone: `kWordCount` was the only thing bounding `imitate`'s six-pair loop, so
growing the table from four to eight would have quietly turned that milestone
into a thinner 28-pair test. `kWordCount` stays pinned at 4 with a comment saying
why, `vocab` uses its own `kVocabCount`, and `imitate` was re-run to confirm it
reproduces its six numbers exactly. It does.

**Scored on the ARTICULATORS.** The first version used the unguarded feature set
— the nine motor groups **plus loudness and voicing** — when
`m3_timbre_features` sits directly beneath it in the same header for exactly this
reason: "Loudness and voicing are dropped entirely, so this cannot pass on 'the
cube makes it noisier'. It is the difference between two SOUNDS rather than two
amounts of sound." Guarded, the mean falls from 0.786 to **0.760** and 14 of 28
pairs clear the bar instead of 16. The conclusion is unchanged, and it had been
resting partly on loudness.

#### The creature says almost the same thing whatever it hears

`vocab` also reports what the creature ITSELF said after each word, which turns
out to be the most informative row in the experiment:

| heard | said |
|---|---|
| /a/ 780/1180 Hz | 632/1656 Hz |
| /i/ 320/2500 Hz | 627/1643 Hz |
| /u/ 350/900 Hz | 628/1630 Hz |
| /e/ 550/1850 Hz | 636/1654 Hz |
| /o/ 450/850 Hz | 633/1646 Hz |
| /ae/ 690/1700 Hz | 634/1646 Hz |
| /^/ 620/1200 Hz | 629/1675 Hz |
| /I/ 400/1950 Hz | 633/1644 Hz |

**F1 spans 9 Hz across the eight echoes** — 627 to 636 — against stimuli spanning
460 Hz. F2 spans 45 against 1650. The creature repeats every word by saying very
nearly the same thing.

That does not contradict M1b, because discriminability is a RATIO: small
differences with small variance are still discriminable, and the audibility ruler
agrees at d' 1.33, which is a listener at about 76% correct. But it puts every
other number on this page into one frame:

| | |
|---|---|
| echo spread across eight words | **9 Hz** of F1 |
| one utterance's range, shipped larynx | ~10 Hz |
| the chain's best trajectory (DNA v42) | 4 Hz |
| what teaching moves a formant by (M1c) | ~70 Hz |

**The voice lives in a band of tens of Hz inside a nominal range of hundreds**,
and the trajectory ceiling, the utterance range, the echo spread and the
vocabulary limit are not four walls but one. Teaching is the only thing that
moves it by more than a few tens of Hz, and it does that by pushing a setpoint
rather than by using the range.

**Two hypotheses tested here and refuted.** That the discriminable axis is the
cochlea's own — this creature hears through a mel filterbank, and the vowel
literature is consistent that Bark- or mel-scaled distance models confusion
better than F1xF2 in Hz. Over all 28 pairs the Spearman rank correlation with the
score is **+0.622 for Hz and +0.689 for mel**, using the creature's own
`hz_to_mel`. That is not a distinction, and the probe says so rather than
reporting +0.689 as a win: mel is a monotonic transform of Hz, so for vowels
whose formants differ in the same direction the orderings largely agree, and
these eight were chosen to CROWD rather than to make the metrics disagree.
Deciding it needs vowels picked so the two rankings conflict.

And that pairs are hard when the creature cannot SAY the two words differently.
The distance between the two ECHOES predicts discrimination **worse** than the
distance between the two words (+0.421 against +0.689), because the echo barely
moves for any word.

**What none of it explains.** /i/-/u/ has a 30 Hz F1 gap and scores 0.900;
/e/-/o/ has the largest gap in the hardest ten on BOTH scales — 1005 Hz, 569 mel
— and scores 0.691. A resolution limit on either axis cannot produce that, so
"where it runs out" still describes the table rather than explaining it.

### DNA v42 — a synfire chain, and the first structure across time this creature has had

`trajprobe` and `seqprobe` above say the same thing twice: an utterance is a held
vowel, and no module can carry its own activity forward for 10 ms. The wiring
every module has is distance-based and therefore **symmetric** — if i drives j
then j drives i just as hard — and a symmetric loop has no direction to carry
anything along. v42 adds the asymmetric pass: neurons are cut into consecutive
groups of `chain_group`, and every excitatory neuron in group k drives group
k+1. Nothing wraps, so a chain is a syllable and not a loop. **Ships off** at
`chain_weight = 0`.

**In `central` it carries a travelling wave.** `seqprobe` at 20 links of 8 ms:

```
reliability:  0.75 0.67 0.74 0.76 0.74 0.65 0.63 0.54 0.53 0.51 0.56 0.58 | 0.01
centre:        107  120  142  152  175  195  224  241  272  299  325  330 |  154
```

The centre of activity climbs monotonically for **96 ms** and then the wave runs
off the end of the chain and reliability collapses in one bin. Reproducible
across 24 repeats, moving, and finite — with divisive normalisation left ON and
the module at 22 Hz, so this is not a saturation artefact. The same creature with
no chain reads reliability 0.022 and a centre pinned flat at ~200 forever.

**At the larynx it imposes a reproducible trajectory, and an inaudible one.**
`trajprobe` with the chain in `vocal`, three seed families:

| seed | no chain | chain in `vocal` | shape spans | d' (null) |
|---|---|---|---|---|
| 1 | 1.2% | **53.8%** | 4.1 Hz | 0.461 (-0.014) |
| 2 | 5.1% | **66.2%** | 6.2 Hz | 0.641 (-0.010) |
| 3 | 8.1% | **57.1%** | 4.3 Hz | 0.549 (-0.030) |

Utterances go from agreeing about 1-8% of their own movement to **53-66%**, and
the mean shape goes from flat noise to a clean monotonic ramp. But it spans
**4-6 Hz** at d' ~0.5, and teaching moves a formant by ~70 Hz. The generator
works; the coupling into the vocal groups is far too weak to hear.

The chain in `central` does nothing at the voice (3.3-5.8%, no better than
baseline) even though `seqprobe` measures 96 ms of travelling wave in exactly
that module. The `central->vocal` tract at density 0.03 does not carry it, which
is the fourth independent measurement saying that about that tract.

So the open problem has moved from "there is no sequence anywhere" to "the
sequence does not reach the vocal groups loudly enough", which is a route and
not a generator. This chain is also entirely **innate** — reward could select
where in it to sing, but the sequence is genome-specified. The interesting
version self-organises, and it inherits exactly this route problem.

#### A dedicated HVC is WORSE, and it walks into this project's oldest wall

The chain inside `vocal` spans only 4-6 Hz for a structural reason: `vocal` is
126 neurons in nine groups of fourteen, a group's output is a CENTROID over its
fourteen cells, and sweeping that centroid would need links of three or four
neurons — which gives each target about three inputs and cannot fire anything.
Convergence and readout resolution are irreconcilable at that size. The songbird
answer is not to put HVC inside RA, so `tools/genome_add_hvc.py` appends a
dedicated 400-neuron nucleus (20 links, 160 ms) projecting into `vocal`.

The prediction, stated in advance: shared shape stays ~55% and the span rises
from 4 Hz toward 50+. **It fell to 1.0 Hz.**

| | chain in `vocal` | HVC -> vocal | HVC disconnected |
|---|---|---|---|
| shared shape | 53.8% | **17.5%** | 3.0% |
| span | 4.1 Hz | **1.0 Hz** | 0.3 Hz |
| d' | 0.461 | **0.032** | -0.147 |

The chain does run in the nucleus — `seqprobe` retargeted at it reads persist
24 ms and reliability 0.59 -> 0.26 -> 0.18, weaker than the 96 ms it manages in
`central` because `central` has sensory tracts feeding it background
depolarisation and `hvc` has only its own noise. So this is a real negative and
not a chain that never fired.

**Why it fails is the wall G3 hit.** At `out_w = 0.30` the creature DRONES —
duty cycle 1.00. A random all-positive projection into `vocal` excites every
group roughly equally, voicing and amplitude included: it delivers *drive*, not
*pattern*. Calibrating it down until the creature behaves at all (0.08, with
`vocal`'s own noise cut 0.22 -> 0.16 to pay for it, which is rule 1 of the
calibration invariant) leaves too little to shape anything.

That is what `genome_add_relay.py` already says about `central->vocal`: "an
all-positive random projection, and central's object code is balanced, which is
the one combination that averages a code away." So the route problem is sharper
than "too weak": **a chain's pattern cannot cross an unsigned random projection
at all** — more weight makes the creature drone before it makes the trajectory
audible.

#### The signed route is worse, and the ranking is the finding

`genome_add_relay.py` exists because an all-positive random projection is the one
thing that averages a balanced code away, and a relay of interneurons — where
each target draws its OWN inhibitory sample — preserves it. So the chain was
routed `hvc -> relay -> vocal`, calibrated to a duty cycle of 0.38, against a
control with the relay present and its output weight at zero.

| route | shared shape | span | d' |
|---|---|---|---|
| chain **inside** `vocal` | **53.8%** | 4.1 Hz | 0.461 |
| HVC -> vocal, direct | 17.5% | 1.0 Hz | 0.032 |
| HVC -> relay -> vocal | **5.1%** | 0.6 Hz | -0.148 |
| relay silent (control) | 2.1% | 0.2 Hz | -0.141 |

5.1% against a 2.1% control, with d' indistinguishable from its own null. The
signed route carries essentially nothing — worse than the unsigned one it was
built to fix.

**The ranking is what matters.** The only arrangement that works is the one with
NO TRACT AT ALL. The chain inside `vocal` reaches 53.8% precisely because there
is no projection between the sequence and the readout; every attempt to route it
through one loses almost everything, unsigned (it drones before it shapes) and
signed (it delivers nothing).

That is a sharper statement of this project's oldest result than it had before.
It is not only that `central->vocal` is thin, or that pooling swamps a balanced
code: **a projection into `vocal` cannot carry a temporal pattern by either
sign.** Three routes, two mechanisms of failure, one conclusion.

What is left is not another route. `vocal` is 126 neurons in nine groups of
fourteen and everything downstream reads a centroid over fourteen cells, which is
what caps the trajectory at 4-6 Hz. The next thing worth trying is a BIGGER
LARYNX — groups of a hundred rather than fourteen, where a chain inside the
module could sweep a centroid with real convergence. Changing a module's neuron
count re-rolls its wiring, so that is a different creature needing calibration
from scratch, which is why it is a separate piece of work and not a knob.

#### A bigger larynx makes it worse, and for the opposite reason to the one expected

`vocal` is 126 neurons in nine groups of fourteen, and a group's output is a
centroid over its fourteen cells — so the reasoning was that groups of a hundred
would let a chain sweep a centroid with real convergence and lift the 4-6 Hz
ceiling. `vocal` grown to 900 neurons calibrates cleanly (duty 0.73 at the
shipped noise), so the test is clean.

| 900-neuron `vocal` | shared shape | F1 range | span | d' | voiced |
|---|---|---|---|---|---|
| no chain | 3.4% | **4.8 Hz** | 0.2 Hz | -0.071 | 62% |
| chain 0.02, matched duty | 5.0% | 4.7 Hz | 0.2 Hz | -0.124 | 58% |
| chain 0.08, duty 0.14 | 49.2% | 4.7 Hz | **2.3 Hz** | 0.419 | **11%** |
| *(126-neuron original)* | *53.8%* | *10.4 Hz* | *4.1 Hz* | *0.461* | *40%* |

**The baseline is the finding.** One utterance in the big larynx ranges 4.8 Hz of
F1 where the small one ranges 10.4 Hz. Growing the module made the voice LESS
mobile, not more — the readout is a centroid over a population and a larger
population averages harder. The change made to give a chain something to sweep
is the same change that flattened the thing being swept.

**CORRECTION, measured the next day: size was not the problem, unopposed
background was.** Switching DNA v32's lateral competition on in the same
900-neuron larynx (`lateral_gain = 0.06`, `lateral_fields = 9` — one competitive
field per motor group, which is what it was built for) takes the F1 range from
4.8 Hz to **18.5 Hz**, above even the shipped 126-neuron larynx's 10.4 Hz. So
"a larger population averages harder" is the right mechanism and "bigger is
worse" was the wrong conclusion drawn from it: the extra neurons are only a
liability while nothing suppresses the ones the wave is not in.

That does not rescue the chain, and the reason is a third instance of the knob
v32 already documents:

| 900-neuron `vocal` | F1 range | shared shape | d' |
|---|---|---|---|
| no lateral, no chain | 4.8 Hz | 3.4% | -0.071 |
| no lateral, chain | 4.7 Hz | **49.2%** | **0.419** |
| lateral 0.06, no chain | **18.5 Hz** | 6.0% | -0.228 |
| lateral 0.06 + chain | 12.1 Hz | 10.3% | -0.289 |

Competition restores the mobility and **destroys the reproducibility** — 49.2%
down to 10.3%, with d' back below its own null. Winner-take-all makes which
subset wins depend on competition dynamics, and those amplify small differences,
so each utterance's wave settles somewhere else. Mobility and reproducibility sit
on opposite sides of one gain, exactly as v32's notes say its bimodality bar and
G2 do.

The strong-chain arm confirms it from the other side: it recovers the shared
shape (49.2%, d' 0.419, close to the small larynx's 53.8% and 0.461) but its span
is SMALLER at 2.3 Hz and it costs most of the voice, 11% voiced against 40%.

There is also no usable operating point. With a real chain weight the duty cycle
pins at ~0.15 and **no amount of noise lifts it** — 0.22, 0.40 and 0.55 all read
0.14 to 0.16. Only weakening the chain to 0.02 restores normal vocalisation, and
at 0.02 the chain does nothing. Strong enough to matter is too strong to speak.

**So four routes have now been tried and the picture is consistent:**

| | result |
|---|---|
| chain inside `vocal` (126) | **works**, capped at 4-6 Hz by fourteen-neuron groups |
| direct projection | drones before it shapes |
| signed relay | delivers nothing |
| bigger larynx | averages the movement away, and will not tolerate a real chain |

The only arrangement that carries a temporal pattern to the voice is the one
with no tract at all, and the ceiling on it is set by a population code that gets
*smoother* as you enlarge it. That is a structural statement about this vocal
architecture rather than a tuning result, and it is where this line stops without
a different readout at the larynx — one that reads something other than a
centroid.

#### `n_max` is not the live neuron count, and reading it as one wasted the first three runs

`central` hatches at **400 neurons**. `n_max = 4096` is the arena ceiling M4
growth may one day reach. Every conclusion drawn before that was checked was
wrong in the same direction:

- "the chain propagates two links and stops dead" — at 64 per group there are
  six links and, at 3 ms each, a chain **18 ms long end to end**. The measured
  20 ms of persistence was the chain running to its end.
- "more drive makes it die sooner" — higher `chain_weight` compresses a
  completed chain, it does not truncate one.
- "divisive normalisation is the brake" — switching v12 off gives 90 ms of
  persistence on an 18 ms chain, which is reverberation. v12 is not stopping a
  wave; it is stopping the module saturating, which is its job.

The instrument was wrong too: 10 ms bins cannot resolve a 3 ms link, so the
centre of activity read flat while the wave was real. And the first chain built
was a per-neuron window — each neuron driving about three downstream — which has
no **convergence** and therefore cannot fire anything. Four separate
plausible-looking negatives, none of them about the mechanism.

#### The verdict said "the creature has a syllable" about a 4 Hz event

`trajprobe`'s first chain verdict gated on the shared **fraction** and never on
the amplitude, so 53.8% printed as a syllable. It is a real statistic about
something no listener could hear — and it is the same mistake this very probe's
null was rewritten to avoid earlier the same day, made again in the same file.
It now gates on both and runs the project's audibility ruler on the start of an
utterance against its end.

#### The self-organising alternative: what removing divisive normalisation does

DNA v42's chain is wired at birth. Fiete's result says it should not have to be:
STDP **plus heterosynaptic competition** organizes a network into long sequences
with no structured input. The reason that looked cheap to test is that this
README claimed *"both ingredients already exist here — STDP ships, and DNA v38's
competitive pruning is the same family."* That sentence is wrong about v38,
which is structural and sleep-gated: it deletes synapses, in bursts, offline,
where Fiete needs one neuron's afferents trading strength continuously while the
sequence forms.

`seqprobe` grew a `--ticks` soak so a chain that must FORM has time to form in,
and a weight control, because a flat table cannot otherwise be told apart from a
rule that never ran. Three million ticks of spontaneous activity in `central`
with a `central->central` Hebbian tract at `1e-3`.

**Synaptic scaling is not the suppressor, and is exactly CV-neutral.** The band
was swept 1.02 / 1.2 / 3.0: mean unchanged to two digits across a 3x range, and
the *narrow* band came out most uniform, backwards from the prediction. With
`hebb = 0` it multiplies every weight by 0.67 and leaves CV at 4.823 -> 4.823, a
ratio of 1.00 — it rescales the distribution and has no opinion about which
synapse inside it wins. That is the design, stated where the rule lives:

> The band is the whole point. Pulling toward an exact setpoint regulates
> precisely the quantity that reward-modulated learning moves. Bounding it
> instead stops runaway without having an opinion about anything inside the
> bounds.

**`norm_gain` is the knob that moves weights apart.** Arms 4 and 6 are the same
wiring — 12 links, 8 ms, 11246 synapses, identical starting spread — one knob
apart:

| arm | mean \|w\| | sd | population CV | within-cell CV | cell total vs global drift |
|---|---|---|---|---|---|
| `12 links, 8 ms` (norm ON) | x1.72 | x1.23 | 0.831 -> 0.594 (x0.71) | x0.79 | 23% |
| `12 links, no norm` | x0.90 | x1.41 | 0.831 -> **1.309 (x1.58)** | **x2.28** | **69%** |
| same, `hebb = 0` | x0.66 | x0.66 | 0.831 -> 0.820 (x0.99) | x1.00 | 0% |

**But it is not Fiete's mechanism, and the control that says so had to be built.**
Population CV rising is equally consistent with heterosynaptic competition and
with every synapse growing independently at its own rate. Fiete's is specifically
per-postsynaptic-neuron: the afferents of one cell separate *while that cell's
total holds*. So the probe now decomposes it — within-cell spread, and each
cell's total measured against the **global drift** so that a uniform rescale
reads zero rather than 33%. Row 2 separates its afferents hard (x2.28) and moves
its cell totals 69%. That is differential growth. The `hebb = 0` row reads x1.00
and 0%, so the instrument is not inventing either number.

**What removing normalisation bought is rate, and persistence follows rate.**
The norm-OFF arm looked like the best result this probe has produced: persist
16 ms -> 144 ms, reliability above 0.6 for 56 ms and above 0.1 out to 160 ms,
first bin uncorrelated with last so the pattern *evolves* rather than sitting in
a static attractor — against this project's standing finding that no module
holds a kick for 10 ms. But it also ran at 55.5 Hz, eight times `central`'s
normal rate, and population correlation inflates when activity is dense.
`runaway` stayed silent only because its threshold is 200 Hz.

Two controls settle it, and neither did what it was built to do. A `no chain, no
norm` arm was meant to supply rate without structure; instead the module sits at
**7.3 Hz**, so removing normalisation is not what makes `central` hyperactive —
it is the **chain's own recurrence**, which normalisation had been holding down.
Rate-matching from the other side then failed too: `inhib_gain` from its default
2.5 up to 20 moved the rate only 55.5 -> 37.8 Hz, and non-monotonically, since
inhibition 5 *raised* it to 61.0.

A failed match that spans the variable beats a successful one, because it gives
a slope instead of a point:

| arm | rate | persist | **ms per Hz** |
|---|---|---|---|
| `no chain, no norm` | 7.3 Hz | 8 ms | 1.10 |
| `12 links, 8 ms` (norm ON) | 6.7 Hz | 16 ms | **2.39** |
| `no norm, inhib 20` | 37.8 Hz | 104 ms | **2.75** |
| `no norm, inhib 10` | 39.8 Hz | 104 ms | **2.61** |
| `12 links, no norm` | 55.5 Hz | 144 ms | **2.59** |
| `no norm, inhib 5` | 61.0 Hz | 176 ms | **2.89** |

Every chain-bearing arm sits between 2.4 and 2.9 ms of persistence per Hz across
a **9.1x rate range**, through changes of inhibition, normalisation and chain
length. Persistence here is not a fact about structure; it is a fact about
density. `seqprobe` now prints the `ms/Hz` column, because a probe that reports
only the numerator invites exactly the reading this section first gave it.

There is a second reason the norm-OFF configuration is not a candidate
mechanism: at 55.5 Hz the module is running eight times its own homeostatic
target of 6.81 Hz, which is outside the range its regulator restores. That is a
broken operating point, not a discovered one.

Three lessons, each of which cost a rerun. **A claim about what the code already
does is not evidence about what the code already does** — the v38 claim sat here
as settled fact and priced a whole line of work. **`| head -3` on a probe that
prints six arms is a measurement error, not a display choice**: it hid the arm
that reversed the conclusion, and a write-up built on the truncated output
declared this architecture unable to do something it had just been measured
doing. And **a population statistic cannot answer a per-neuron question** — the
first version of the weight control stopped at population CV, which would have
shipped "COMPETITION" for what the decomposition then showed to be ordinary
differential growth.

A fourth, from the two controls that missed: **a control can test a different
question than the one it was built for, and still read as an answer.** Both
assumed a rate/structure relationship that turned out to be backwards. Neither
failed loudly — they returned plausible tables. What caught it was checking
whether the arm had actually produced the condition it was supposed to produce
(high rate, matched rate) rather than reading its verdict column. Ratios like
`ms/Hz` exist to make that check part of the output.

#### The chain does carry a travelling wave, and only the per-repeat readout sees it

The section above closes with persistence explained away as density, which left
one thread: at a matched rate the chain still did something the no-chain arm did
not. `seqprobe` already printed a `centre of activity` line and nobody had read
it. The no-chain arms pin at ~210 — the centre of mass of diffuse activity is
just the population mean. Every chain arm instead collapses to the chain head
and sweeps monotonically away from it:

```
no chain           196 221 217 207 203 215 216 212 208 214 216 ... 208 214
12 links, 8 ms      65  74  14  15  15  16  16  20  21  23  31 ... 265 256 255 232
6 links, 16 ms      95 130 155 196 173  45  38  35  32  43  41 ...  238 248 286
```

**That line pools all 24 repeats**, so it cannot say whether any single repeat
sweeps — one loud repeat dominates a weighted mean, and this project has read an
aggregate as a per-trial fact before. So the probe now reports two per-repeat
numbers instead. `travel` is centre against bin WITHIN one repeat, averaged over
repeats: does activity move each time? `agree` is one repeat's trajectory against
another's: do they move the same way? A chain needs both, and neither alone is
enough.

On the shipped genome, 24 repeats per arm:

| arm | rate | travel r | agree r |
|---|---|---|---|
| `no chain` | 6.0 Hz | **+0.035** (sd 0.197) | **+0.005** |
| `no chain, no norm` | 6.6 Hz | **-0.007** (sd 0.201) | **+0.000** |
| `6 links, 3 ms` | 4.0 Hz | +0.373 | +0.247 |
| `6 links, 16 ms` | 8.7 Hz | +0.294 | +0.660 |
| **`12 links, 8 ms`** | 5.8 Hz | **+0.583** (sd 0.205) | **+0.525** |
| `20 links, 8 ms` | 5.7 Hz | +0.521 | +0.406 |

Both nulls sit at zero on both measures; every chain arm is well clear of them.
The strongest is `12 links, 8 ms` — one chain link per 8 ms bin, which is what
the bin width was chosen for.

**Why `reliab r` misses it.** Population-vector reliability collapses to 0.02-0.06
in exactly the bins where the sweep happens, and that is not a contradiction: it
demands the same neurons fire in the same bin across repeats, so jitter in wave
SPEED destroys it while leaving the trajectory intact. A probe that only measured
pattern correlation would have reported no sequence here, and did for months.

**The discriminator against "activity just spreads from the kick site"** is that
the chain arms leave the null's resting centre in *both* directions: they start
at the chain head (14-35, far below the null's ~210) and end past where diffuse
activity sits (265-286). Spreading with uniform decay converges on the mean from
one side; it does not start below it and finish above it.

**One result is genome-dependent and is reported as such.** Removing
normalisation destroys travel on the shipped genome and does not on a genome
carrying a potentiated `central->central` Hebbian tract:

| no-norm arm | shipped | + Hebbian tract (mean \|w\| x5.64) |
|---|---|---|
| `12 links, no norm` | -0.069 | **+0.435** |
| `no norm, inhib 5` | -0.079 | **+0.480** |
| `no norm, inhib 10` | -0.379 | **+0.342** |
| `no norm, inhib 20` | -0.357 | **+0.209** |

Four arms each, consistent within genome and opposite between them, so this is
systematic and not one arm wobbling. Extra potentiated recurrence appears to
substitute for normalisation in holding a wave together. The cause is not
isolated — that genome also sets `scaling_band = 1.02` — but the band sweep
above moved nothing to two digits, which leaves the tract as the candidate.

Note also that `agree` stays positive (+0.35 to +0.53) on the shipped genome's
no-norm arms while `travel` is negative. Those repeats do agree — on a
rise-then-fall that is not travel. Either number alone would have been read as a
sequence.

**And the bin label was wrong.** The line printed "reliability by 10 ms bin"
while `kSqBinTicks` was 8 at `dt_ms = 1.0`, so every duration computed off it
came out 25% long, including one in this README that called seven bins above 0.6
"70 ms" when it is 56. The label is now derived from the constant.

### DNA v43 — a topographic tract to the larynx, and the seventh common-mode wall

The travelling wave gave this project its first spatially organised signal, and
the larynx has always read a **place code**: `VocalDecoder` takes each motor
parameter as the centroid of firing rate WITHIN one 14-neuron group, so F1 is
the centre of mass of `vocal[28..42]`. Moving F1 means differentially activating
positions inside one slice. Every tract into the larynx has been
`kind = "random"`, which touches every position equally, so its centroid sits at
0.5 however loud it is — the measured HVC failure, where more weight drones
before it articulates.

So v43 adds a `kTopographic` projection: source position picks destination
position, with a destination sub-range so a genome can target one motor group,
and `topo_dst_lo > topo_dst_hi` reversing the map. The reversal is not a
flourish — [i] -> [a] raises F1 while F2 falls, so a forward map into group 2 and
a reversed one into group 3 turn ONE wave into a diphthong instead of sliding
both formants together. `tools/genome_add_topographic.py` wires it. Rule 1
holds: unused, the hash does not move.

**The oracle that justified building it.** Mapped onto one group, central's
measured wave (centre sweeping 14 -> 265 of 400) would sweep F1 by **471 Hz**,
against a null arm's 47 Hz, the 400 Hz two vowels need, and the **20 Hz the
larynx actually delivers**. That is the largest voice number this project has
produced, and it is an upper bound.

**It does not survive contact.** Three measurements, in the order they were made:

| | F1 raw sd |
|---|---|
| chain on, no map | **0.1098** |
| + topographic map, w 0.14 | 0.0951 |
| + w 0.30, d 0.60 | 0.0845 |
| + w 0.60, d 0.60 | 0.0792 |

Monotonically *down*: more topographic drive makes F1 **less** variable. That is
what a stationary localized input does — it pins the centroid rather than moving
it — and it says the map was delivering a stationary bump.

**Why: the shipped creature has no wave.** `chain_weight = 0.0` in
`dna/default.toml`; `seqprobe` sets it per arm. The chain ships OFF, so the map
had nothing to carry. With it on, `babble` now prints central's own centre of
mass free-running: **sd 0.0170 without the chain, 0.0438 with it, against the
kicked wave's 0.035 .. 0.66**. The chain roughly doubles the excursion and it is
still a wobble, not a sweep. Nothing kicks the chain head in free behaviour.

**`topoprobe` supplies the trigger as an oracle**, the way the credit oracle
priced per-neuron reward before it was built: two arms on one genome differing
only in whether central's chain head is kicked. It refuses on the shipped genome
— no chain, no map — and that refusal is the point, since running it there would
print a clean null about the genome rather than about the route.

| topographic weight | map r | central under trigger | **F1 gain** |
|---|---|---|---|
| 0.14 | 0.494 | x2.29 | **x1.00** |
| 0.40 | 0.613 | x2.31 | x1.26 |
| 1.00 | 0.613 | x2.32 | x1.06 |

Both preconditions pass — the kick moves the source, and the synapses landing in
group 2 correlate source position with target position at r ~0.5-0.6, so the map
is a map. The wave arrives, and F1 does not move. **Seventh appearance of the
common-mode wall:** the group's other drive — the auditory arc, `vision->vocal`,
its own recurrence, noise — dominates a centroid that one tract cannot move.

**Two instrument notes, because both preconditions had to be built and one was
wrong first.** The kick initially used a single tick at 1.5 and its own source
check caught it (central moved x1.09), so the F1 columns would have been a null
about the stimulus; it now matches `seqprobe`'s 2.0 held for 10 ticks. And the
delivery check first asked whether the trigger changed group 2's firing RATE —
the wrong quantity, since a travelling wave *redistributes* activity rather than
adding it, so a correctly working map holds the rate flat at ~1.00 and moves the
centroid. That check would have refused exactly the success it was built to
detect. It is now structural.

**What this leaves.** The route is built, correct, and measured, and the thing
in the way is not the route. Making it work needs the map to dominate its
target's activity, and raising the weight does not do that — homeostasis pulls
group 2's rate back (7.67 at w 1.00, 5.70 at w 2.50). The untried direction is
the one thing that has measurably worked against this wall before:
**in-degree-weighted subtraction** (DNA v21-v24's pooling interneurons, +0.077
on 3/3 families) and v32's lateral competition, applied *within* the target
group, so the map chooses where a bump sits and competition makes it sharp
rather than adding to a common mode.

#### Pooling subtraction and lateral competition: not the route, but +35% of vowel

v43's negative pointed at the one thing that has measurably moved the
common-mode wall before — in-degree-weighted subtraction (v21-v24's pooling
interneurons, +0.077 on 3/3) and v32's lateral competition — applied *within*
the target group, so the map chooses where a bump sits and competition sharpens
it rather than adding to a common mode. `vocal` is already built for the second
half: `lateral_fields = 9`, one competitive field per motor group. Both ship OFF.

**The constant trap, for the fourth time this project.** `ffi_gain = 0.5` is
what `mechverify`'s variant uses, and carried over to `vocal` pooling from
`central` it silences the larynx outright — 0.00 Hz, duty cycle 0.00. The scale
is derivable and was not derived: `pool_fast_` is in **Hz**, `central` runs at
~5.5, so a gain of 0.5 subtracts ~2.75 of drive from a module whose net drive is
on the order of 0.05. The usable range is **0.0005 to 0.001**, five hundred
times smaller, and 0.002 already fails `babble`.

**On the question it was built for, the answer is no.** Four arms, same genome,
trigger oracle:

| arm | F1 gain under trigger | F1 smoothed sd |
|---|---|---|
| topographic only | x1.26 | 0.0297 |
| + pooling subtraction | x1.28 | 0.0344 |
| + lateral competition | x1.16 | 0.0445 |
| + both | x1.18 | **0.0538** |

The trigger effect is flat at x1.16-1.28 across every arm. Neither mechanism
makes the travelling wave reach the voice, and the two together do not either.

**What they do instead replicates.** The *delivered* F1 spread rises with them,
present with and without the trigger, on three fresh seeds:

| seed | topographic only | + both | ratio |
|---|---|---|---|
| 20260901 | 0.0186 | 0.0248 | x1.33 |
| 20260902 | 0.0193 | 0.0255 | x1.32 |
| 20260903 | 0.0201 | 0.0284 | x1.41 |

3/3, same sign, x1.35 mean — which matters because the previous `lateral_gain`
result in this README did NOT replicate, coming out at mean +0.003 with the sign
flipping. The first seed's x1.81 was optimistic; x1.35 is the number.

**And the mechanism is legible.** Raw F1 sd rises only ~10% (x1.11, x1.09,
x1.10) while *smoothed* rises ~35%. The combination barely widens the centroid;
it makes the width **survive the 800 ms articulator inertia**. That is exactly
what the vowel-space note predicted would change the smoothing calculus — the
filter is destructive only while it is blurring noise, and a group holding a
stable bump is a posture rather than jitter. Anyone re-running the
`smoothing_ms` sweep should do it on this creature.

**Not shipped.** In Hz this is ~14.5 -> ~20 against a range of 750 and a bar of
~400 for two distinguishable vowels: a 35% gain on a quantity that needs about
twenty times. Switching it on is a genome change that moves the hash and forces
a re-baseline of every vocal number, which is the same trade `smoothing_ms` was
parked on. Both knobs are one line each when there is something worth
re-baselining for.

#### The smoothing sweep, re-run on the creature it was parked for: null again

`[[aibaby-vowel-space]]` parked the `smoothing_ms` sweep with a condition: the
800 ms filter is only destructive while it is blurring noise, so re-run it "on a
creature with something worth holding". The pooling + lateral pair above is that
creature — its F1 width survives the inertia where a bare one's does not. So the
sweep was run: two arms, three smoothing values, on the audibility ruler.

**At three seeds it looked like a result, and it was not.**

| arm | 800 ms | 400 ms | 200 ms | 800 -> 200 | signs |
|---|---|---|---|---|---|
| topographic only, **n=3** | 0.34 | 0.39 | 0.58 | **+0.24** | + + + |
| topographic only, **n=6** | 0.34 | 0.40 | 0.37 | **+0.03** | 4/6 |
| + pooling + lateral, n=3 | 0.45 | 0.26 | 0.31 | -0.14 | 1/3 |
| + pooling + lateral, n=6 | 0.48 | 0.44 | 0.35 | -0.13 | 2/6 |

Three seeds with the sign agreeing on **all three** gave +0.24; three more seeds
took it to +0.03. **This is the second time this exact sweep has produced an
effect that evaporated**, and it is worth being precise about how the bar moved:
the first time, a single seed read 0.76 and three fresh seeds killed it, and the
lesson recorded was "use three seeds". Three seeds with unanimous signs was still
not enough. The per-cell scatter here is larger than any effect being looked for
— arm A's 800 ms column alone runs 0.00, 0.27, 0.36, 0.39, 0.39, 0.62.

**Nothing is audible in any of the 36 runs.** Corrected d′ averages 0.398 and
peaks at 0.79 against the bar of 1.0. The smoothing knob does not produce two
distinguishable utterances on either creature, which is the question the sweep
existed to answer. That thread is now closed rather than parked.

**And the wider vowel does not buy audibility.** The pooling + lateral pair
raises delivered F1 spread ×1.35 on 3/3 seeds, but at the shipped 800 ms its
audibility gain is +0.14 on **4/6** — the same weak level as everything else
here. A wider centroid and a more audible creature are not the same measurement,
and only the first of them replicated.

**A correction about the null, stated because it was overstated first.** On 18
runs the corrected null looked misbehaved — mean 0.08, max 0.25, against a
README table saying 0.00 at 600k ticks — and that read as the bias correction
failing away from the shipped operating point. At 36 runs the picture is milder:
**median 0.01, mean 0.060, max 0.25, with 7 of 36 at or above 0.15.** The
correction does work; it has a tail. The usable conclusion is narrower than
"the ruler is broken" and more useful: a *single* run's corrected d′ carries a
floor of roughly 0.25, so no difference below that is readable without many
seeds — which is exactly what the n=3 table above got wrong.

#### The G3 eligibility diagnosis was measured on the wrong tract

`eligprobe` asks whether the eligibility trace distinguishes the two objects,
because if it does not then no reward schedule can make the voice conditional.
It has always read two tracts: `central->vocal` and the arcuate. Neither is the
right one to ask.

`central->vocal` was **later shown to be a non-participant** — delete it,
recalibrate, and every G3 number is unchanged. The arcuate carries the heard
**word**, not the seen object. And `vision->vocal` — the tract that actually
delivers the seen object to the larynx, reading it at 0.660 — shipped *after*
that diagnosis was written and was never in the probe. So `eligprobe` now reads
it too:

| tract | object, from the trace | shuffled null | corr(A,B) |
|---|---|---|---|
| **`vision->vocal`** | **0.820** | 0.474 | **+0.945** |
| `central->vocal` | 0.752 | 0.496 | +0.938 |
| arcuate, size-matched | 0.944 | — | **+0.689** |

**"The trace is object-blind" is too strong**, and this project has been quoting
it as settled since. On the tract that carries the object the trace classifies
it at 0.820 against a 0.474 null — above the non-participant the claim was
measured on.

**And it still cannot be used, for a reason the first column hides.** The mean
trace under cube and under ball are **94.5% identical**. The arcuate — the one
tract here whose conditionality is not in doubt, since the creature repeats
words — sits at 68.9%. A classifier with 1027 features can find a 5.5%
differential; R-STDP multiplies the whole trace by one scalar, so what it can
act on is the common mode. That is the same shape as everything else on this
page: a small differential riding a large common mode.

**What it changes.** The recorded diagnosis was "no conditional signal exists at
the larynx, so conditioning cannot work". The measurement says the signal exists
and the *rule* cannot separate it. Those have different fixes: the first needs a
new pathway, which is what `vision->vocal`, the HVC nucleus, the signed relay
and DNA v43's topographic map all tried; the second needs the common mode
removed **from the trace**.

**That second operation is DNA v16, and it is already in the genome.** It was
written for exactly this reason — "reward can therefore only scale that tract,
which is exactly the G3 symptom" — measured on 2026-08-15, refuted, and kept at
`elig_baseline_tau_ms = 0` with a note saying the mechanism is correct and cheap
and that the next covariance-flavoured idea should start there rather than
rebuild it. An earlier draft of this section called that operation untried. It
is not, and the genome comment anticipates the reasoning for proposing it,
including why it fails: a classifier is already invariant to per-feature means,
so the 0.820 above *is* the centred signal and subtracting the mean online only
adds the lag and noise of a running estimate.

**Its refutation was measured on the same wrong tract, and re-testing does not
rescue it.** v16 was killed on `0.570 -> 0.503`, which are `central->vocal`
numbers. On `vision->vocal`, across three genome seeds:

| seed | credit, v16 off | credit, v16 ON | Δ |
|---|---|---|---|
| 20260911 | 0.852 | 0.868 | +0.016 |
| 20260912 | 0.880 | 0.868 | **-0.012** |
| 20260913 | 0.830 | 0.868 | +0.038 |
| mean | 0.854 | 0.868 | **+0.014**, 2/3 |

+0.014 on 2 of 3 is not a result; a single first observation read 0.864 and
looked like one. A fourth seed takes it to **+0.011 on 3 of 4**, against a
per-run standard error of about 0.013 — one SE, which is nothing.

**The variance gate, run because that ON column read 0.868 on all three seeds
while OFF spanned 0.830-0.880.** That is the signature this project has a
standing warning about, and it was worth stopping to check rather than building
on. `eligprobe` now prints the per-creature vision credit and synapse count
instead of only the five-creature mean. The gate comes back clean on every
count: per-creature credit spans **0.77-0.93** (sd ~0.03), so nothing is stuck
and 0.868 was a coincidence on a 0.002 grid; and the tract's synapse counts are
identical across `tau` within a seed and different between seeds, so wiring
tracks the seed and an STDP timescale does not touch it. If anything the
baseline *raises* per-creature variance — 0.16 of spread against 0.07 — which is
the opposite of the collapse that was suspected.

**One of those readings was briefly a false alarm, and the cause is worth
recording.** The first gate run went through `xargs -P 4`, so all four labels
printed immediately while the tables arrived minutes later in completion order.
Pairing them by position said the genome seed did not change the wiring and that
`elig_baseline_tau_ms` did — two impossible things — when in fact the tables
pair by seed and the behaviour is correct. Never pair a label with output under
`-P`; print the label from inside the job, or run serially. This is the same
class as the `| head -3` truncation elsewhere on this page: a shell-level
artifact that produced a confident, wrong claim about the creature.

#### A trigger for the chain: the wave needs synchrony, and no pathway carries it

DNA v42's chain produces a travelling wave when `seqprobe` kicks it, and nothing
kicks it in life. The obvious next move is a trigger, and it needed no new
mechanism: v43's topographic kind can aim a projection at a *sub-range* of a
module, so `auditory -> central[0 .. 0.08]` — the first chain link — is a genome
edit. `tools/genome_add_topographic.py` grew `--dst/--dst-lo/--dst-hi` for it.

**A first correction, because the claim it rests on was over-read.** The v43
section reports central's free-running centre-of-mass spread (sd 0.0438 against
the kicked wave's 0.63 range) as showing no wave occurs without a kick. That
statistic cannot show it. `seqprobe` sees the wave by *aligning to a known
onset and averaging 24 repeats*; in free running, waves launching at random
times with several in flight at once average to the middle and produce a SMALL
spread. Small sd means "no waves **or** many overlapping waves".

So `topoprobe` gained the measurement that can answer it: align to the
creature's own auditory onsets and average, which is `seqprobe`'s analysis on a
natural trigger instead of an injected one. **It confirms the conclusion by a
route that could have refuted it** — travel r **+0.060** (sd 0.457, 200 onsets)
where the kicked wave reads +0.583 and the no-chain null +0.035. With 200 onsets
the standard error is ~0.032, so this sits on the null.

**And the trigger does not work.**

| trigger onto the chain head | travel r |
|---|---|
| none (chain only) | +0.060 |
| depressing, weight 0.30 | +0.086 |
| depressing, weight 1.0 | +0.104 |
| depressing, weight 3.0 | +0.101 |
| *(`seqprobe`'s injected kick)* | *+0.583* |
| *(no-chain null)* | *+0.035* |

A ten-fold range of trigger weight moves nothing, and making the projection
**depressing** — DNA v36's synapse, which passes the first spike of a burst and
little of the rest, i.e. an onset detector already in the genome — does not
either.

**Why, and it is structural.** A synfire chain propagates a *synchronous
volley*. `seqprobe`'s kick is 2.0 of current onto 32 neurons held for 10 ticks,
arriving together. A synaptic projection delivers asynchronous spikes, and
asynchronous drive of any magnitude raises the head's firing rate without ever
forming a volley. Drive is not synchrony, and no sensory pathway in this
creature carries synchrony. A working trigger would need a mechanism that
manufactures one — a burst generator, or a gate that releases the head all at
once — which is a different thing from any tract.

**An instrument bug found on the way.** The onset detector first compared
`mean_rate` against its own running mean and found **zero** onsets in 400k
ticks. `mean_rate` is a one-second EMA and cannot rise through 1.3x of itself.
The module already exposes the right pair — `mean_rate_fast` is tens of ms, the
timescale an onset lives in. Reading a slow average as an instantaneous one is
the same mistake as reading a smoothed formant for a raw one. The probe refused
rather than reporting a trajectory on zero onsets, which is why the bug surfaced
as a refusal instead of as a null.

### M1d — the creature takes turns: **met, and SHIPPED**

G3 has now absorbed eight mechanisms against one wall without moving, so this
asks a question shaped to what the creature demonstrably does. It echoes a heard
word (M1b), learns a fixed vowel from praise (M1c), keeps it across sleep, and
falls silent while listening. Proto-conversational turn-taking needs none of the
conditionality that is blocked: the creature does not have to know *what* it
heard, only that it heard something and that the speaker stopped.

**M1b is a claim about content and this is a claim about rate.** M1b showed the
voice CARRIES the word 200-600 ms after it ends. A creature babbling at a
constant rate, whose babble happens to resemble what it just heard, passes M1b
and is not taking turns.

**The trap this is built around.** The listening reflex suppresses babbling while
the creature hears something, so "quiet during the word, vocal afterwards" is
true *by construction* — a milestone defined on alternation passes without the
creature doing anything. So the silence is not scored at all. The only quantity
is whether the post-word rate exceeds the creature's own quiet baseline, taken
1400-2300 ms into the same trial once the reflex has long released. The null is
the same creature on the same trial clock hearing **nothing**, which removes any
rhythm of its own.

| window | word | silence | word - silence |
|---|---|---|---|
| while the word | 0.276 | 0.382 | **-0.105** |
| 0-200 ms after | 0.289 | 0.379 | -0.090 |
| 200-600 ms after | 0.408 | 0.368 | +0.040 |
| 600-1400 ms after | 0.432 | 0.380 | +0.052 |
| 1400-2300 ms (quiet) | 0.440 | 0.379 | +0.061 |

**Answering burst -0.032, the same contrast in silence -0.011, corrected
-0.021.** The creature does not answer. The profile is suppression and recovery:
deeply quiet during the word, still below its own baseline at 200-600 ms (0.408
against 0.440), rising monotonically back with no overshoot. **M1b's content
match at 200-600 ms is an echo riding on babble that was going to happen
anyway.**

And the guard did its job: on alternation alone that -0.105 would have read as a
pass.

**One unplanned positive.** The word arm sits above the silent arm at every late
window — +0.061 at the quiet tail. Hearing words raises the creature's overall
babble rate. That is arousal rather than turn-taking, it is not time-locked to
anything, and it had not been measured before.

#### DNA v44 — the rebound, and the signature appears

An answering burst needs the post-stimulus rate to **overshoot** baseline rather
than recover to it: a transient excitability rise when input stops, opposite in
sign to the listening reflex and outlasting it. Both halves of an offset
detector already exist in the kernel — `pool_fast_` is tens of ms and
`mean_rate` is a one-second EMA — so v44 is one rectified difference:

```
drive += rebound_gain * max(0, mean_rate[src] - pool_fast_[src])
```

Rectified on purpose: unrectified, a module whose source was RISING would be
suppressed, which is a second listening reflex nobody asked for. Rule 1 holds —
at gain 0 the hash does not move.

**The signature appears, and by a route worth understanding.**

| window | shipped | rebound 0.02 |
|---|---|---|
| while the word | 0.276 | **0.018** |
| 0-200 ms after | 0.289 | 0.454 |
| 200-600 ms after | 0.408 | **0.841** |
| 600-1400 ms after | 0.432 | 0.699 |
| 1400-2300 ms (quiet) | 0.440 | 0.553 |
| **corrected burst** | **-0.021** | **+0.296** |

+0.296 at gain 0.02 and +0.257 at 0.05, against a bar of +0.05. But read the
first row: the creature is now nearly **silent** while the word plays. That is
the rectification. During quiet, fluctuations rectify to a tonic lift — which is
why the silent-arm baseline rises 0.379 to 0.45 — and while the word plays the
fast pool sits above the slow one so the term is exactly zero. The word removes
a tonic drive *and* applies the reflex, and the burst is the release from both.

**M1b survives, and its audibility improves.** Same table, same conditions:

| condition | shipped voice / d′ | rebound voice / d′ |
|---|---|---|
| clean | 0.937 / 1.54 | 0.897 / **1.97** |
| SNR 20 dB | 0.910 / 1.78 | 0.900 / 1.74 |
| SNR 10 dB | 0.913 / 1.06 | 0.893 / **1.62** |
| SNR 0 dB | 0.807 / 1.07 | 0.823 / **1.59** |
| ±6 dB level | 0.870 / 1.41 | 0.910 / 1.43 |
| 10 dB & ±6 dB | 0.847 / 1.09 | 0.883 / **1.18** |

Discrimination is essentially unchanged and audible d′ rises in five of six
conditions, most in noise.

**And a confound, which is in the control column.** `EAR` goes 0.560 -> 0.700.
That column exists because an after-window in which the auditory module still
classifies is not memory but a stimulus that has not finished arriving. A
creature silent while listening does not mask the caregiver with its own babble,
so it hears the word better and more of it persists into the scored window.
**Part of the M1b gain is a cleaner stimulus rather than a better echo**, and
this table cannot say how much.

#### DNA v45 — the rebound's own baseline, and both gates pass

Two things stood between the signature and a claim, and both are now settled.

**The mechanism was doing something other than advertised.** v44's term is
rectified, and rectifying a fluctuating difference has a positive mean even at
rest — it lifted the silent-arm baseline 0.379 to 0.45. While a word plays the
term is exactly zero, so the word removed that lift *as well as* applying the
reflex, and the burst was partly the restoration of a baseline the creature had
been deprived of. v45 subtracts the term's own slow mean, which costs one field
and leaves v44 byte-identical at tau 0.

| | v44 (rectified) | v45, tau 5 s | v45, tau 20 s |
|---|---|---|---|
| silent-arm baseline | 0.45 | **0.370** | 0.372 |
| corrected burst | +0.296 | **+0.348** | +0.346 |

Removing the tonic lift **raises** the burst. So it was not baseline
restoration. Worth noting the mechanism is now *biphasic* rather than rectified
— during a word `raw` is 0 while the mean is positive, so the term goes negative
— which means the near-silence while listening (0.010) is partly v45's own doing
and not the reflex alone. That was not the design intent and is worth stating.

**And it is not a stimulus arriving late.** The `EAR` control rises when the
mechanism works, because a creature silent while listening stops masking the
caregiver and hears it better. So the burst is scored again on only those trials
whose auditory activity at 200-600 ms has fallen back within 20% of that same
trial's quiet window — per trial, so the criterion does not depend on a
creature's auditory gain.

| arm | burst, all trials | burst, ear back at baseline | trials qualifying |
|---|---|---|---|
| v44 | +0.296 | **+0.349** | 196/200 |
| v45 | +0.348 | **+0.425** | 192/200 |

Restricting to trials where the word has genuinely gone **strengthens** the
effect. A late stimulus would shrink it. The confound is excluded rather than
bounded, and 96% of trials qualify, so it was small to begin with.

**Six seeds.**

| seed | corrected | ear-separated |
|---|---|---|
| 20260921 | +0.333 | +0.358 |
| 20260922 | +0.315 | +0.283 |
| 20260923 | +0.357 | +0.308 |
| 20260924 | +0.364 | +0.318 |
| 20260925 | +0.366 | +0.318 |
| 20260926 | +0.358 | +0.389 |
| **mean** | **+0.349** | **+0.329** |

6/6 positive, minimum +0.315, spread ~0.02, against a +0.05 bar and a shipped
baseline of -0.021. This is a different shape from the `smoothing_ms` result
that evaporated on the same page: there the per-cell scatter was 0.6 and the
effect 0.03; here the scatter is 0.05 and the effect 0.35.

**Status: MET and SHIPPED.** `vocal` now carries the rebound in
`dna/default.toml` — `rebound_source = 1`, `rebound_gain = 0.02`,
`rebound_mean_tau_ms = 5000`. The pinned hash moves
`23c4eb2c7c45d05c -> ad96f882becbee92`.

**What shipping cost, and what it did not.** `calibrate` passes **unchanged** —
no `target_rate_hz` moved, `vocal`'s free-running rate going 3.81 -> 4.12 Hz
against a 5.00 target and every other module inside 0.21 Hz. That is the
cheapest re-baseline this project has had; the fixed-point hunt the calibration
invariant warns about never started.

`mechverify` loses its v44 and v45 rows and returns to 17. It exists for
mechanisms invisible to `kPinnedHash`, and a mechanism that ships ON is covered
by the determinism hash directly — left in, those rows would patch the shipped
values onto themselves, and the experiment's own vacuity check would correctly
fail a variant that had stopped doing anything.

**The behavioural cost is real and is not only an added burst.** The
mean-subtracted term is NEGATIVE while a word plays, so the creature is now
nearly silent when it listens: voiced fraction 0.276 -> 0.010. That is plausible
for an infant and it is a bigger change than "answers afterwards", and it was
not the design intent — v45 was built to remove a tonic lift and turned out
biphasic.

**And shipping it broke `ipprobe`, which turned out to be two bugs in the
probe.** It gates on whether relaxing the regulator opens `auditory`'s rate gap
— a question about the REGULATOR — while measuring a creature that hears its own
voice through `self_gain`. With vocal output barely tracking the stimulus that
was tolerable; the rebound makes it track hard, and the contamination reversed
the result (shipped +80.3%, relaxed +66.1%: relaxing appeared to NARROW the
gap). Muted, the same creature reads +141.1% and +247.3%, a clean 1.75x.

The second bug was found while fixing the first: `ipprobe` configured its ear
from the ORIGINAL blob rather than from each arm's variant, so every per-arm
edit to an audio field was silently ignored. The mute produced output
byte-identical to the unpatched sweep, which read as "the mute does nothing"
rather than "the mute never happened". Any future audio sweep in this probe
would have measured the same creature N times.

Both are fixed: the sweep is muted and built from its variant. **Numbers from
`ipprobe` are not comparable with any recorded before 2026-08-29**, which were
all taken with self-hearing on.
Meeting M1d means shipping it on, which moves the hash and forces a re-baseline
of every vocal number under the calibration invariant — and it should not be
switched on until the EAR confound is separated, because the honest headline
today is "a mechanism exists that makes the creature answer", not "the creature
answers". The separation is cheap: score the burst on trials where the ear's own
classification in the scored window is at chance.

**What it would take.** An answering burst needs the post-stimulus rate to
*overshoot* baseline rather than recover to it — a transient excitability rise
when input stops, which is the opposite sign to the listening reflex and would
have to outlast it. That is a genome-level change to one module rather than a
new pathway, which makes it cheap to try; it is not attempted here.

**A bug worth recording.** Adding the fifth window overflowed `ImitateRun`'s
four-wide per-window classification arrays and segfaulted. The fifth is a *rate*
baseline, not a content window — asking which word the voice carries 1400-2300 ms
after it ended is not a question — so the scoring loop is now bounded by its own
`kScoredWindows` rather than by `kWindows`. Same hard-coded-count class as the
shared-constants audit. It first appeared as *no output at all*, because the run
was piped to `tail`, which reports its own exit status and swallowed the crash —
the trap this project has already recorded once.

#### The answer is informative, not just louder

M1b says the voice CARRIES the word 200-600 ms after it ends. M1d says the
creature SPEAKS MORE then. Those two together read as "it answers with what it
heard" — but that is a conjunction of two separate measurements, and the
alternative is that the burst is simply more babble whose echo is incidental.

`imitate` now scores the fifth window for content as well as rate. That window
is the creature's own ambient babble, 1400-2300 ms after the word, with the
reflex long released and nothing left arriving — the same trials, the same
creature, no stimulus. If the burst carries the word no better than that, M1d
added volume and nothing else.

Read on **articulators**, which drops loudness and voicing: the burst is by
construction louder, so the wide `voice` readout could separate two words on
amount of sound alone. This is a claim about two sounds.

| seed | burst (200-600) | ambient (1400-2300) | difference | shuffled |
|---|---|---|---|---|
| 20260809 | 0.752 | 0.546 | **+0.206** | 0.500 |
| 20360812 | 0.812 | 0.580 | **+0.232** | 0.501 |
| 20451117 | 0.850 | 0.644 | **+0.206** | 0.502 |
| **mean** | **0.805** | **0.590** | **+0.215** | 0.501 |

3 of 3, and every shuffled control sits at chance, so the instrument works in
the ambient window on every seed. The burst carries about five times the
above-chance information that ambient babble does — 0.305 against 0.090 — and
ambient babble is only slightly above chance itself.

**So M1b and M1d compose.** The creature goes quiet while you speak, answers
when you stop, and the answer carries which word it heard. That is a different
statement from either milestone alone, and it was not guaranteed by having both.

**A bug that nearly buried it, and the tell that caught it.** `double sum[4][5]`
was still four wide after the scoring loop widened to five, so the ambient row
came out of bounds: **0.000 voice against a 0.174 shuffled**. A two-class
holdout accuracy cannot be 0.000 and its shuffled control must sit at 0.5 — the
impossible number is the only reason it was caught rather than believed. Had the
garbage landed anywhere plausible the conclusion would have been right by
accident.

That is the third hard-coded four this window count has broken: `ImitateRun`'s
per-window arrays (a segfault), `kScoredWindows` (the bound), and now this. They
should be sized from one constant rather than repeated as literals; a fourth
instance is likely.

#### The vocabulary ceiling is upstream of the larynx, not in its nine knobs

`m3probe` reads the heard word out of `vocal` at **0.980 per neuron** and
**0.380 on the centroid**, so the module plainly carries more than the nine
motor groups express. That suggests an obvious explanation for the vocabulary
ceiling: the brain holds eight words and the knobs cannot express them. If true
it would be a *readout* limit — a different problem from G3's wall, and a
tractable one.

`vocab` now scores one-of-eight on `vocal`'s own 126 neurons alongside the motor
groups. Identical estimator, identical trials, identical split; the only change
is which vector a trial is.

| readout | rebound OFF | rebound ON |
|---|---|---|
| `vocal` per-neuron (126) | 0.129 | 0.234 |
| **articulators (9, guarded)** | **0.325** | **0.344** |
| unguarded (11) | 0.210 | 0.371 |
| 126-dim shuffled control | — | 0.149 |

**The module does not hold what the knobs miss.** Per-neuron scores *below* the
articulators in both arms, and at 0.129 with the rebound off it is at the
shuffled control's own level. The nine knobs are the better readout, not the
bottleneck — each group's centroid averages noise that raw counts carry. So the
ceiling is upstream of the larynx and the readout hypothesis is refuted.

**And a 76% improvement that is not one.** Unguarded one-of-eight reads 0.210
with the rebound off and 0.371 with it on, which looks like M1d substantially
enlarging the vocabulary. Guarded, it is **0.325 -> 0.344**: nothing. The jump
lives entirely in loudness and voicing.

The reasoning that nearly published it is worth recording. Seeing guarded 0.344
against unguarded 0.371 in the ON arm, the conclusion drawn was "the guard costs
only 0.027, so the gain is not loudness" — comparing guarded against unguarded
*within one arm*, when the question requires guarded *across* arms. With the
rebound off the guarded readout **beats** the unguarded one, 0.325 against
0.210: adding loudness and voicing actively hurts there. The rebound did not add
word information, it changed how the unguarded channels behave. Only the
same-binary A/B could show that.

The A/B was run for a different reason — the recorded 0.210 predated a window
addition and an out-of-bounds fix, so the comparison crossed a code change. The
OFF arm reproduced 0.210 **exactly**, so the record was sound and the control
was unnecessary for its stated purpose. It caught the real error anyway.

### Three verify tiers, because the second one had become unrunnable

The teaching experiments each raise a creature through several phases of a life
at 3.4M-5.6M ticks, and there are now seven of them. Left in `kLong` they turned
a half-hour suite into a five-hour one, which does not mean the suite is
thorough — it means it stops being run before a commit.

```
--experiment verify        determinism, hash, 19 fast experiments      seconds
--experiment verify-long   + the minute-scale ones, 35 total           ~30 min
--experiment verify-teach  + the seven hour-scale teaching ones        hours
--experiment verify-all    both
```

### What this round taught about probes, which cost more than the mechanisms did

Four mechanisms went in and five probes came out, and the probes found more
errors in themselves than in the kernel. Each of these produced a number that
looked entirely normal.

**Settle before the *first* block, not just between them.** `stpprobe` ran its
silent control first, on a just-hatched creature, after 1500 ticks. Silence read
1.30 spikes/tick where a settled creature reads 0.70, so the control was measured
on a different brain from the conditions — and the 2.3% gap that produced became
a published explanation of a null. `ipprobe` disagreed by an order of magnitude,
which is the only reason it was caught. When two instruments disagree that far,
the purpose-built one is usually right: find the confound, do not reconcile.

**The null is whatever the control arm does, not zero.** `pruneprobe` asks
whether surviving synapses are stronger than the population competition selected
from. Random removal leaves that mean unchanged, so 0% looks like the null — but
a consolidation pass also downscales every weight, and the off arm reads −9.61%.
Against 0% the mechanism looks marginal at +2.59%; against its actual control it
is +12.2 points.

**Nor is zero the null for a count.** The same probe first failed for orphaning
six neurons. The shipped genome hatches with six neurons nothing projects onto,
and the off arm has them too.

**A trace is not a flag.** `burstprobe`'s first version asked "was this spike part
of a burst" by reading the kernel's burst rate, which is a 50 ms one-pole meaning
"has bursted recently". It reported 80.9% where the honest answer is 8.2%. A probe
that reads the kernel's own derived quantity cannot tell a correct implementation
from a self-consistent one — it should re-derive from the spike train it can see.

**One shuffle is a draw, not a null.** `burstprobe` at one permutation reported a
null of 0.380 against a chance of 0.500 and an "effect" that was partly that. At
32 permutations the nulls sit at 0.486–0.503.

**Count the trials before reading the column.** 200000 ticks across three arms is
33 trials, an accuracy step of 0.06, and every object column was inside its own
noise. The minimum is 600000.

**A mechanism that ships off was invisible to `verify`.** The `syn_elig_mean_`
pruning bug survived since v16 because it needs both a sleep prune and a genome
with DNA v16 enabled, and the shipped genome has neither. The pinned hash could
not see any of the off-by-default mechanisms, and the fix had to be proved on a
genome nobody runs. That is what `mechverify` above is for, and it is the only
one of these lessons that turned into code rather than a rule.

### `ipctx` — the regulator was measured before it was relaxed, and it saturates

`ctxlearn` left one named suspect. Its positive control dies whenever the
context oracle fires, the collapse does not scale with `out_w` over a 6x range,
and `dst_noise` compensation does not save it — so the lever is rate-side, and
intrinsic plasticity is the only rate regulator that runs on the larynx (DNA
v11 measured that synaptic scaling never executes there at all).

The obvious move is `ip_wake_scale = 0`. Two things already in this file say
not to. DNA v9 measured that relaxing IP on vocal makes the creature **drone**
(duty 0.61 -> 0.83), which moves the voiced fraction the formant readout
depends on — and `ctxlearn`'s voiced gate is one-sided, so it would not catch
it. And relaxing a regulator without first checking it runs is the
`syn_wake_scale` mistake, which cost a 3.4M-tick arm and returned byte-identical
rows. So `ipctx` measures rather than intervenes: twelve sessions, no yoke,
because reading a regulator does not need one.

| gain | ctx Hz | d threshold | **pinned** | change |
|---|---|---|---|---|
| 0.00 | 0.0 | -0.071 +/- 0.007 | **0.00** | **+34.0** |
| 0.06 | 13.4 | +1.233 +/- 0.318 | **0.25** | +14.7 |
| 0.10 | 31.4 | +1.263 +/- 0.371 | **0.31** | +16.6 |
| 0.20 | 62.6 | +2.781 +/- 0.028 | **0.83** | +1.5 |

**IP does not re-regulate the larynx. It runs out of range.** The threshold
climbs +1.2 to +2.8 into a [0.20, 4.00] clamp, and every driven level ends the
session with a quarter to five-sixths of the module sitting at `threshold_max`.
The derived rate error reads +0.60/+0.62 Hz driven but is invalid at all of
them, because once a neuron is on the clamp the drift stops tracking the error.
The graded re-regulation story is refuted, and `ctxlearn`'s last named suspect
is closed.

What it raises and does not claim: `pinned` tracks the collapse better than
anything else in the table. The narrower hypothesis is that IP pushes a large
share of the larynx onto its ceiling, where a per-neuron bias perturbation has
less purchase on the output — a **different** experiment, predicting the pinned
share rather than the learning score. The `ip_wake_scale` arm is not licensed.

**At 200000 ticks this probe reads `pinned 0.00` everywhere and grants the
licence.** The threshold needs a full session to walk to the clamp, so a short
run of this experiment reports the opposite of a long one. `--allow-short`
exists for exactly that and fails the run regardless of what it prints; the
short numbers were still quoted as directional before the long ones landed.

Three verdict bugs, each caught after printing something plausible: the two
conditions were tested independently across levels and passed one at gain 0.20
and the other at 0.10, calling two regimes one licence; a saturated level was
reported rather than refused, and refusing it is what exposed the first bug;
and it printed "no rate error to act on" when the error was *unmeasurable
because clamped*, which is the opposite conclusion. The rate error itself was
derivable rather than sampleable — IP steps the threshold by
`ip_rate * (rate - target)`, so the drift is the integral of the error and the
mean follows by division: +3.12 +/- 0.21 Hz where the endpoint EMA gave
8.18 +/- 2.48 for the same quantity.

### DNA v50 — inhibitory plasticity, and the pinned-share hypothesis dies with it

`ipctx` ended by naming its own successor. It had refuted the graded
re-regulation story and raised a narrower one in its place: not that intrinsic
plasticity re-regulates the larynx away from the context tract, but that it
pushes a large share of the larynx onto its threshold ceiling, where a
per-neuron bias perturbation has less purchase on the output. It said in terms
that this needed a *different* experiment — one predicting the pinned share
rather than the learning score — and that no `ip_wake_scale` arm was licensed
until something else was tried.

The something else is in the literature and this creature did not have it.
**Outside sleep downscaling, no inhibitory weight in this brain has ever
changed.** Both of its rate regulators act on the excitatory side: intrinsic
plasticity moves a threshold, synaptic scaling rescales the whole afferent set,
and v11 measured that the second never executes on the larynx at all. A
threshold is a bounded quantity, so threshold regulation has a finite budget by
construction, and `ipctx` had just watched a sustained new afferent spend it.

Vogels, Sprekeler, Zenke, Clopath and Gerstner (*Science*, 2011) is the
regulator with the other budget: a rule on inhibitory synapses whose mean drift
is `eta * nu_pre * (nu_post - rho0)`, which holds a neuron at a target rate by
growing the inhibition that matches the excitation arriving. Where a threshold
attenuates *every* afferent a neuron has, inhibition grows against the one that
caused the error.

**The constant is derived, not guessed, and deriving it caught a real bug.**
`isp_gain` is a fraction like `scaling_rate`: the step is normalised by the
presynaptic rate vector so that one pass moves a neuron's steady-state membrane
by exactly what one pass of IP moves its threshold, leaving `ip_rate` as the
only calibrated constant either mechanism uses. The conversion between the two
is not one-for-one. The membrane integrates — `v += leak_alpha * (v_rest - v) +
drive` holds a steady drive `d` at `v_rest + d / leak_alpha` — so a drive change
is worth `leak_alpha` times a threshold change, a factor of twenty at a 20 ms
leak and a 1 ms tick. Matching the two step sizes naively would have made ISP
twenty times weaker than the mechanism it is quoted against, and it would have
read as a mechanism too slow to matter rather than as one that was mis-scaled.

**It also has an instrument for its own failure mode**, because inhibitory
weight is as bounded as a threshold is and "saturated" and "nothing to do" would
otherwise read identically — which is exactly the confusion that cost `ipctx` a
second run. `inh sat` is the share of a module's inhibitory afferents sitting at
their own ceiling, and `ipctx` now prints it, along with the voiced fraction,
because relaxing regulation on the larynx makes the creature drone (v9: duty
0.61 -> 0.83) and a `change` read off a droning creature is a different
measurement wearing the same name.

**The prediction was registered before the run: the pinned share falls toward
zero at the same context drive.** Not the learning score — five upstream
improvements in this project have left the milestone still, and the mechanism's
own claim is about range.

#### The 2x2, 3 seed families, 48 sessions of 3.4M ticks

`ip_wake_scale` 1.0 / 0.25 crossed with `isp_gain` 0 / 1. The IP-relaxed cells
are the control that makes the test fair: with IP at full strength it nulls the
rate error before ISP can act, and in Vogels' network ISP *is* the regulator.

| ctx drive | | pinned | inh sat | voiced | change |
|---|---|---|---|---|---|
| mute | IP 1.0, no ISP | 0.00 | 0.00 | 0.66 | **+34.0 +/- 4.7** |
| | IP 1.0, **ISP** | 0.00 | 0.00 | 0.66 | **+15.4 +/- 4.3** |
| | IP 0.25, no ISP | 0.00 | 0.00 | 0.66 | +21.8 +/- 10.4 |
| | IP 0.25, **ISP** | 0.00 | 0.01 | 0.67 | +21.1 +/- 4.8 |
| 13.4 Hz | IP 1.0, no ISP | 0.25 | 0.00 | 0.49 | +14.7 +/- 7.0 |
| | IP 1.0, **ISP** | 0.20 | 0.06 | 0.46 | +4.2 +/- 3.8 |
| | IP 0.25, no ISP | 0.20 | 0.00 | 0.45 | -10.4 +/- 22.6 |
| | IP 0.25, **ISP** | 0.11 | 0.01 | 0.45 | -6.7 +/- 12.7 |
| 31.4 Hz | IP 1.0, no ISP | 0.31 | 0.00 | 0.65 | **+16.6 +/- 1.8** |
| | IP 1.0, **ISP** | 0.47 | 0.09 | 0.60 | -5.9 +/- 5.2 |
| | IP 0.25, no ISP | **0.00** | 0.00 | 0.60 | **-2.3 +/- 1.1** |
| | IP 0.25, **ISP** | 0.46 | 0.24 | 0.64 | -6.2 +/- 5.6 |
| 62.6 Hz | IP 1.0, no ISP | 0.83 | 0.00 | 0.42 | +1.5 +/- 6.6 |
| | IP 1.0, **ISP** | 0.40 | 0.24 | 0.37 | -4.2 +/- 13.0 |
| | IP 0.25, no ISP | 0.90 | 0.00 | 0.38 | +1.2 +/- 3.3 |
| | IP 0.25, **ISP** | 0.56 | 0.25 | 0.37 | -5.7 +/- 16.0 |

The no-ISP, IP-1.0 rows reproduce the recorded `ipctx` table exactly — pinned
0.00/0.25/0.31/0.83, change +34.0/+14.7/+16.6/+1.5 — so the arms are paired and
the instrument is intact.

**The registered prediction fails in every cell.** The threshold climbs +1.1 to
+2.9 into the clamp whether or not inhibition is plastic, and whether IP runs at
full strength or at a quarter of it. Relaxing IP fourfold does not slow the walk
to the clamp *at all*, which kills the mechanism's one excuse: it was not that
IP won the race and ISP never got the job. ISP got the job and could not do it.

**And it is not the inhibitory budget.** `inh sat` peaks at 0.25 — three
quarters of the range unspent while the threshold saturates beside it. That
column was added to tell "saturated" apart from "nothing to do", and it says
neither: the rule has room and is not using it.

#### What this actually settles, which is worth more than the mechanism

> At 31.4 Hz of context drive the shipped larynx reads **pinned 0.31** with
> `change` **+16.6 +/- 1.8**, positive on 3 of 3 seeds. The IP-relaxed larynx at
> the **same** drive reads **pinned 0.00 on 3 of 3 seeds** with `change`
> **-2.3 +/- 1.1**, negative on 3 of 3. Unpinning the larynx completely does not
> lift `ctxlearn`'s collapse. It deepens it.

That is `ipctx`'s narrower hypothesis, tested and answered with the sign
reversed. **Pinned share and learning score come apart**, unanimously and with
tight bars on both sides, so the pinned share is not the operative variable and
the `ip_wake_scale` arm that was never licensed is now not merely unlicensed but
spent and negative. Whatever kills the positive control when the context tract
fires, it is not the larynx sitting on its threshold ceiling — the same way it
was already not intrinsic plasticity re-regulating, not synaptic scaling, not
`out_w` over a 6x range, and not `dst_noise`.

**ISP also costs the positive control on its own.** With the context tract
silent, `change` goes +34.0 +/- 4.7 -> +15.4 +/- 4.3, and *not* through the
drone v9 measured — the voiced fraction is 0.66 in both. So it is paid out of
the exploratory pathway rather than out of the duty cycle, which is the same
currency `ctxlearn`'s wall is denominated in: a single motor population cannot
host a conditional afferent and a reward-driven exploratory search at once, and
it turns out it cannot host a second *regulator* either.

Ships off. `isp_gain = 0` is bit-identical to v49, hash unmoved at
`ad96f882becbee92`, `verify` 22/22. Kept rather than removed on the precedent of
v29, v40 and v43 — a refuted mechanism whose absence would invite the proposal
again is worth one field and one branch — and `mechverify` now carries an
eighteenth pin, `c7067352b374d80d`, so it cannot rot into a no-op unnoticed.

#### Where it points, and the next thing to run

Mehaffey and Doupe (*Nature Neuroscience*, 2015) measured what this creature is
being asked to do without: HVC's premotor input and LMAN's exploratory input
arrive at RA as *separate* afferents under *separate* rules, and pairing them
drives the two in opposite directions. `vocal` here is one population asked to
be both. The v47 diagnosis — a single shared motor population cannot host a
learnable conditional input and a reward-driven exploratory pathway at once —
has now been reached from three directions, and every one of them was an
addition to the larynx that the larynx charged for:

| what was added to `vocal` | what it cost the exploratory pathway |
|---|---|
| a conditional afferent (v47) | `fixed` +24.6/+39.1/+38.1 -> ~0, 5 genomes |
| a second regulator (v50) | +34.0 -> +15.4 with the tract silent |
| *less* of the first regulator (v50) | +34.0 -> +21.8 with the tract silent |

Fee and Goldberg (*Neuroscience*, 2011) describe the arrangement that does not
require the larynx to host anything: the conditional map is learned in a
basal-ganglia stage that receives the timing signal and a *collateral* of the
exploratory signal, and its output biases the motor population from outside.

**Do not build that yet — price it.** It is the largest build this project has
considered, and DNA v35 is the standing warning: a lead that was real,
label-free and correctly derived, built against a bottleneck that had closed
underneath it while the notes still said otherwise. The oracle that prices it
costs an afternoon and can only be interesting if it refuses, which is the shape
`shapeprobe` and `coderprobe` both had.

> **This was built and run — see `ctxbias` below, which LICENSES the route.**
> The gate is kept here as written, because what it predicted and what it found
> are different documents and the first should not be quietly edited into the
> second.


> **Area X's output is a bias onto the motor population.** So deliver one — a
> per-neuron bias offset on `vocal` that depends on the context, injected
> directly, bypassing every tract, the way `set_reward_mask` injects a credit
> assignment the creature cannot compute. Then ask `ctxlearn`'s only question:
> does the positive control survive?
>
> If an oracle bias *also* kills `fixed`, then no upstream architecture can
> help, because a bias onto `vocal` is exactly what any of them delivers — and
> the whole basal-ganglia route is refused for one run instead of one month. If
> it survives, the route is licensed and the cost is worth paying, with the
> oracle's own number as the bar the built version has to reach.

The second question that run should answer, because the same session can carry
it — **answered below, and the answer is that it cannot**: this creature has
**one** population that can affect the voice. The arcuate
carries the word innately, `vision->vocal` ships, and `central->vocal` is a
measured non-participant — deleting it leaves every G3 number unchanged. An
Area X analogue has to project *somewhere*, and the only association-to-motor
tract this creature has is the one that was shown to deliver nothing. Whether
that is the tract's fault or `central`'s is not known, and it decides whether
the architecture is buildable here at all.

### `ctxbias` — pricing the last architecture, and it is licensed

v47, v48 and v50 all failed the same way, and the shape they share is that
every one of them added something to `vocal` and `vocal` charged for it. Fee and
Goldberg's arrangement is the one that does not ask it to: the conditional map
is learned in a basal-ganglia stage that receives the timing signal and a
*collateral* of the exploratory signal, and its output biases the motor
population from outside.

That is the largest build this project has considered, and DNA v35 is the
standing reason not to start it — a lead that was real, label-free and correctly
derived, built against a bottleneck that had closed underneath it while the
notes still said it had not. So it gets priced first, the way `credit` priced
v41 and the oracle fovea priced v34.

**The last step of that architecture is a bias onto `vocal`, and a bias onto
`vocal` can be handed over directly.** `Network::set_bias_oracle` is a graded,
zero-mean ramp across an articulator group, added to the drive beside `bias_`.
Experiment-only, no genome field, on the `set_reward_mask` precedent; the hash
is unmoved at `ad96f882becbee92`. Three things make it an oracle rather than
another input: it moves the centroid the decoder actually reads while adding no
net drive, it bypasses every tract and synapse, and its amplitude is a multiple
of the module's own `noise_amp` — the scale node perturbation's bias lives on —
rather than a constant somebody typed.

#### 8 arms, 3 seed families, 24 sessions of 3.4M ticks

| arm | dF1 (Hz) | vocal Hz | change |
|---|---|---|---|
| off | +1.0 +/- 9.6 | 5.10 | **+34.0 +/- 4.9** |
| cond k=1 | **+235.9 +/- 9.6** | 5.81 | +3.9 +/- 0.8 |
| cond k=2 | **+302.5 +/- 4.0** | 8.06 | -0.1 +/- 0.3 |
| toward k=1 | -4.3 +/- 5.4 | 5.22 | +55.0 +/- 1.8 |
| away k=1 | +4.2 +/- 4.2 | 4.93 | +19.0 +/- 1.6 |
| **offaxis k=1** | -4.3 +/- 9.2 | 5.60 | **+31.2 +/- 8.6** |
| **offaxis k=2** | -0.8 +/- 7.7 | 7.02 | **+39.7 +/- 4.2** |
| central k=2 | +15.3 +/- 16.2 | 5.18 | +26.1 +/- 5.6 |

`dF1` is the voice's own F1 separation between the two words, over every voiced
frame — the vacuity guard and the ceiling measurement at once.

**Arriving as a bias is free.** The `offaxis` arms put the identical ramp — same
module, same amplitude, same conditional sign — on the first two *bandwidth*
groups, which `formant_error` does not read. The oracle arrives in full and
contributes exactly zero to the score, and the positive control does not move:
+34.0 -> +31.2 at k=1 and +39.7 at k=2, with k=2 carrying *more* drive
(7.02 Hz against 5.10) and reading *higher*. Against v47's tract, which killed
the same control outright on five genomes, and v50's regulator, which took it to
+15.4, that is the contrast the run exists for. **The wall those two hit is not
inherent to delivering something to this module**, so an upstream structure
whose output is a bias has somewhere to land.

**The bar is 236 Hz of dF1, which is 51% of the 460 Hz separating the two
words** — 302 Hz and 66% at k=2, at the cost of a rate that is no longer
neutral. That is the ceiling on how conditional this voice can be made by
anything, with credit assignment removed entirely. An upper bound and not a
behaviour, the same caution `credit`'s reward-mask oracle carries.

**And the second question, which the same session answers.** The identical
oracle on `central`, at a *larger* amplitude than the one that moves the voice
302 Hz from `vocal`, moves it +15.3 +/- 16.2 Hz — indistinguishable from the
+1.0 +/- 9.6 baseline. The association-to-motor route cannot carry a steering
signal at all, which is `central->vocal` being a non-participant seen from the
other side and with an oracle in place of a code. **An Area X analogue here has
to project to the larynx directly.**

#### Three verdicts, two of them wrong, and why that is in the file

This experiment printed a confident answer twice before it printed a defensible
one, and both retractions came from a control that was already in it.

1. **It first scored the conditional arm** and printed REFUSED on
   +34.0 -> +3.9. But the oracle moves F1 by 236 Hz per word while `fixed`
   rewards one target and teaching moves about 70, so that collapse is the
   arithmetic of a disturbance three times larger than the effect being scored.
   It is not a finding about credit assignment.
2. **It then scored a constant bias pointing away from the target**, and printed
   REFUSED again on +34.0 -> +19.0. `change` is a *ratio*, `1 - late/early`, and
   a constant off-target bias adds an error offset that compresses it whether or
   not learning was harmed — which `away k=2` shows by driving it to +1.0 on its
   own. The tell was already on the page: `toward` and `away` sit
   near-symmetrically around `off` at +55.0 and +19.0, which is an oracle moving
   the creature *along the scored axis*, not damaging it.
3. **Only `offaxis` prices arrival**, because it is the one arm that arrives
   without touching the quantity the score is computed from.

**The limitation that leaves, stated rather than buried.** The cost of a
*steering* bias on the scored axis is not measurable with a score computed from
that axis — that is true by construction, not for want of an arm. So what is
licensed is precisely: a bias arriving on neurons other than the ones carrying
the lesson is free. That is what an Area X output is, since it is a targeted
bias rather than v47's broadcast tract, but it is narrower than "a bias anywhere
in `vocal` is free" and should not be quoted as the wider claim.

### DNA v51 — a context-indexed bias, which is Area X reduced to its core

**Built from five papers, and the argument for it is a parameter count.**

`ctxbias` split the problem: delivery works, computation is missing. v51 is the
smallest thing that can supply the missing half. `bias_[i]` — node
perturbation's learned excitability, one scalar per neuron since DNA v10 —
becomes `bias_[i][c]`, indexed by the active slice of a `kContext` module.
`context_slots = 0` ships, is bit-identical to v50, and the hash is unmoved at
`ad96f882becbee92`.

**The index is read, never driven.** The context module needs no projection to
anything — v51 takes the argmax over its slices by rate and nothing else. That
is the whole reason this costs the larynx nothing where DNA v47's context tract
cost it everything: `ctxbias` measured that a bias arriving off the lesson's own
neurons is free, and an index is cheaper than a bias. The genome for the
experiment is built with `out_w = 0`.

**Where each paper enters, and what it decides.**

| paper | what it decides here |
|---|---|
| Werfel, Xie & Seung 2005 | *which parameterisation.* Learning time scales with parameter count, so one bias per neuron per context is 2x, where node perturbation on synapses was ~16x |
| Fee & Goldberg 2011 | *what it is anatomically.* HVC's code is sparse and near-one-hot, so a BG stage driven by it and biasing the larynx **is** a context-indexed table |
| Heald, Lengyel & Wolpert 2021 | *why an index is the fix.* Memory creation, updating and expression are one computation — which context the learner infers |
| Gadagkar et al. 2016 | *what is still missing after this.* Area X's reward is a performance prediction error; this creature delivers raw, object-blind R |
| Miconi 2017 | *that the rule class can be conditional at all*, so `g2cond` is a fact about parameters and not about reward-modulated learning |

**It reconciles two of this project's own contradictory conclusions.**
`vocallearn` concluded node perturbation cannot be conditional *because a
per-neuron bias is a constant*. The post-mortem on node perturbation cashed onto
synapses retracted that — *"expressiveness was never the problem, variance is"* —
after the synaptic version cut irreproducible learning noise 65% -> 8% and still
could not carry G2. Both are right. Werfel's scaling law says the synaptic
version bought expressiveness at sixteen times the parameter count and paid for
it in variance, and **the parameterisation that is expressive and cheap was
never tried.**

**The cash-in goes to the active context's table, or to the shared bias when
there is no context**, so the two lessons never touch the same parameter. That
is the point: `retain` measured a conflicting lesson wiping a taught sound to
0.22 while `capacity` measured two orthogonal lessons coexisting at 0.84, and
Heald, Lengyel and Wolpert say those are one computation seen with one context
and with two. Naming is the conflicting case by construction — both lessons
drive the same formant to different values — and an index is what converts it
into the other case.

**`areax` measures it against a bar that already exists**, which is unusual here
and is the reason `ctxbias` was run first: a perfect conditional bias reaches
236 Hz of dF1 on this readout. Four arms — `off`, `on`, a matched-marginal
`random` control, and `fixed+on` as the positive control. That last one is not
optional: splitting the table halves the trials each context gets, and
`vocallearn`'s own power curve reads +1.0 at 560k against +18.3 at 3.4M, so
without it a flat conditional result could be a null or simply half a session.
Two further gates fire before any of it is read — the creature has to have been
in a context, and the two tables have to have diverged, because a table that was
never indexed and a table that was indexed and learned nothing are the same flat
dF1 from outside.

#### `areax` — and it learns

Four arms, 3 seed families, at 3.4M ticks and again at 6.8M. `off` and `on`
differ in one genome field; `random` keeps the mechanism on and draws the target
independently of the word with the same marginals.

| arm | dF1 (Hz) | table div | change |
|---|---|---|---|
| off | 27.1 +/- 13.9 | 0.0000 | +1.7 +/- 3.1 |
| **on** | **112.9 +/- 3.8** | 0.0368 | **+22.5 +/- 4.5** |
| random | 33.5 +/- 19.0 | **0.0368** | -0.3 +/- 0.2 |
| fixed+on | 18.1 +/- 7.5 | 0.0439 | -1.7 +/- 3.1 |

**The voice becomes conditional.** dF1 112.9 against 27.1 with the mechanism off
(6.0 SE) and 33.5 against the matched-marginal control (4.1 SE), **unanimous on
3 of 3 seeds with no overlap in either comparison** — every `on` creature is
above every `off` and every `random` one. That is 48% of the 236 Hz a perfect
oracle bias reaches on the same readout.

**The line that matters most is the `table div` column.** `on` and `random` have
**identical divergence, 0.0368 both**. The estimator wrote the same *amount*
into the two contexts in both arms; only when the target tracked the word did
that writing become conditional behaviour. A creature that had merely become
more variable, or that sat between two targets, would score the same in both —
which is the control `pgprobe` exists to demand, and it is passed here on the
mechanism's own internal quantity as well as on the behaviour.

**It grows with trials, as a real effect should.** From 3.4M to 6.8M ticks, dF1
goes 82.0 -> 112.9 and the conditional arm's own error reduction goes
+14.5 -> +22.5. That was a prediction registered before the longer run.

#### The gate that was wrong, and how it was caught

`areax` first shipped with `fixed+on` as its power gate: keep the mechanism on,
make the target unconditional, and refuse the run if a split table cannot learn.
It refused at 3.4M (+5.3 against an +18.3 bar) and its own remedy was "re-run at
2x --ticks". **At 6.8M it read -1.7. Doubling the session made it worse, and a
power problem cannot do that.**

So the arm was falsified as a power measurement on its own terms, rather than
retired because the numbers underneath it were attractive — which is the
distinction that matters, because changing a gate after it refuses is exactly
the failure this project keeps a note about. The a priori reason was available
before either run and should have been seen: with an unconditional target both
tables must learn the *same* bias, so the split doubles the parameters needed
for one lesson while halving the data for each. **It is the split table's worst
case**, where the conditional task is its best, and "if this fails the
conditional arm is unreadable" never followed.

The gate is now `on` against `random` on the error reduction — identical
structure, identical marginals, identical split, differing only in whether the
target tracks the word. `fixed+on` is still reported, because what it measures
is real and worth knowing: **a split table is worse at an unconditional lesson**,
which is the price of the split and the reason `context_slots` should stay 0 in
a genome with nothing to condition on.

### `rpeprobe` — Gadagkar's mechanism, priced and refused

DNA v51 works, and the same literature names what is still missing. Gadagkar and
colleagues (*Science*, 2016) recorded dopamine in Area X during singing and found
a **performance prediction error**: suppressed after worse-than-predicted,
activated after better-than-predicted.

This creature is closer to that than it looks, and the difference is exact.
`Brain::update_drives` already computes `reward.effective = reward.total -
reward_baseline_`, so node perturbation is driven by a prediction error already.
**The baseline is one global EMA.** Gadagkar's is per performance context. That
is the whole gap — one array where this has a scalar.

Which is exactly why it wanted measuring first. A per-context baseline can only
buy something if the contexts differ in mean reward, and `vocallearn` already
sets its praise criterion per word — deliberately, because against one global
mean the creature is simply rewarded for saying the easier word.

**The decomposition**, on `areax`'s own protocol, read-only, 3 seed families:

    R - b_global  =  (mean_c - b_global)  +  (R - mean_c)

| arm | between share | R gap ctx0-ctx1 | external share |
|---|---|---|---|
| v51 off | 0.0000 +/- 0.0000 | -0.00008 +/- 0.00019 | 0.995 |
| v51 on | 0.0000 +/- 0.0000 | -0.00015 +/- 0.00006 | 0.995 |

**DO NOT BUILD IT.** Effectively none of the reward variance is between
contexts, against a bar of 10% set before the run — the order at which this
project already judged the object-specific share of a weight change (~8%) too
small to be the mechanism. The delivered reward is already balanced across
contexts, so a per-context prediction error would be subtracting a term this
creature does not have. **The protocol's own per-word criterion anticipated the
mechanism**, which is a pleasant way to lose a hypothesis.

**And a number nobody had measured: the caregiver's term is 99.5% of the reward
variance.** The drives contribute almost none of it. That is not a contradiction
of the standing finding that weights still move 76% as much with the caregiver
SILENT — that was a different condition, and the two together say something
sharper than either alone. The drives supply a large and nearly *constant*
reward, so the baseline removes almost all of it, and what reaches node
perturbation in a teaching session is overwhelmingly praise and scold. Any
future claim that the drives are drowning the teaching signal has to be made
against this number.

### `ctxsrc` — the last piece, and it is not a readout problem

`areax` has the **host** write the context slice from the word label. That is an
oracle, and it is the honest limit on v51's result. For naming, the creature has
to derive the index from what it heard.

The information exists — `coderprobe` reads one-of-eight off the auditory module
at 0.981 against a 1.000 ceiling. But that is measured *while the word is
playing*, and v51 needs the index at the moment reward lands, which in
`vocallearn`'s trial is ticks 900-1700: the 800 ms **after** the word stops.
So the question is timing, not legibility, and it needs no learning and no new
mechanism to answer. Decode the word from each module in bins across
`vocallearn`'s own trial — written beside those constants rather than copying
them, because two copies of 900 and 2800 that have to agree is the
shared-constant bug class this project has already swept once.

**The bar is derived and was stated before the run.** With two contexts, an
index right with probability `p` writes the *other* context's table `1-p` of the
time, so the conditional signal scales as `(2p - 1)`. Keeping half of `areax`'s
112.9 Hz needs `p >= 0.75` in the reward bins.

Nine creatures, and the sample size is not incidental — see below.

| bin | auditory | central | vocal spikes | **articulators** |
|---|---|---|---|---|
| 0-200 word | **1.000** | 0.729 | 0.872 | 0.510 |
| 200-900 word | **1.000** | 0.971 | 0.991 | 0.742 |
| 900-1300 **reward** | 0.841 | 0.503 | 0.602 | **0.818** |
| 1300-1700 **reward** | 0.541 | 0.514 | 0.588 | **0.742** |
| 1700-2800 after | 0.498 | 0.482 | 0.538 | 0.632 |
| 1100-1500 (M1b's window) | - | - | - | 0.789 |

Shuffled controls, taken in a *reward* bin rather than where the signal is
loudest: 0.469, 0.468, 0.486, and 0.497 for the articulators.

**M1b said to look at the voice, and the voice is by far the best carrier.**
The creature repeats what it hears 200-600 ms after a word stops with the ear
already at chance, so the persistence a context index needs might already exist
in the motor system — the echo as a memory rather than as an imitation. It
does: the articulators read 0.742 in the window where the ear reads 0.541 and
central 0.514. And they carry the word BETTER AFTER THE WORD STOPS (0.818,
0.742) than while it plays (0.510, 0.742), which is what a delayed copy looks
like and is the signature of a memory rather than a relay.

**It is still not enough.** The decisive number is the worst reward bin, and it
is **0.740 +/- 0.040 across nine creatures** against a 0.75 bar. The mean is
below the bar and the spread straddles it.

**And the sample size is the lesson.** At three creatures this read 0.754 and
the probe printed a licence. At nine it reads 0.740 and refuses. That is the
same failure `smoothing-sweep-closed` records — three seeds with unanimous signs
was not enough there either — and the only reason it was caught is that a
verdict resting on four thousandths is visibly a coin flip rather than a result.
The probe now runs nine and reports a spread on the number the verdict turns on.

**Two instrument errors, both found by checking rather than by the answer
changing.** The first version of this probe decoded the larynx from SPIKE COUNTS
and read 0.589 — but `vocab` had already measured that the articulator centroids
beat the per-neuron readout on this module, so it used the instrument this
project's own notes call inferior. And its bins straddled M1b's window
(1100-1500) rather than aligning to it. Fixing both is what turned 0.589 into
0.742, and neither fix was chosen because of what it did to the answer.

The third error was in the comparison itself: the corrected readout was first
reported from M1b's narrow window against the ear's *worst of two wider bins* —
a different window and a different readout, both moved in the favourable
direction at once, which is not a comparison. Every column above is now the same
bins and the same worst-of-both rule.

> **The word is at ceiling while it plays and marginal at best by the time
> reward lands.** The ear is at chance (0.541); the voice, which is the best
> carrier anything here has, reaches 0.740 +/- 0.040 against a 0.75 bar. And
> these are *held-out linear readouts* — upper bounds on any index the creature
> could compute — so the shortfall is not "the decoder was weak".

**`central` does not hold it at all, which refutes the obvious hypothesis.**
`audprobe` established that B2 classifies the word within 50 ms while central
needs 1200 ms to reach 0.940 — central is slow because it *integrates*, and
integration looked like exactly what a context needs to survive silence. It
reaches 0.978 during the word and then collapses to 0.551 and 0.480, if anything
faster than the ear. **Central integrates up; it does not hold.**

**So what stands between v51 and naming is a context that does not survive long
enough to be used.** The creature does have a persistent trace of the word and
it is in the motor system, which is worth knowing and was not known before — but
at 0.740 it would carry roughly half of `areax`'s effect, and half is what the
bar was set to protect.

That leaves two honest options and the file does not pretend otherwise. **The
cheap one** is to wire v51's index to the articulators anyway and measure
`areax` directly against its own 112.9 Hz: at p = 0.740 the predicted signal is
`(2p - 1) x 112.9` = ~54 Hz against an `off` baseline of 27, which is still
detectable, and the build is a change of where the index is read rather than a
new mechanism. The proxy is marginal enough that the direct measurement is now
the cheaper instrument. (The 27 is a *floor*, not a pedestal to add: dF1 is an
absolute difference, so the `off` arm's 27 Hz is `E|noise|` with no signal under
it, and scaling the difference and then adding it counts the noise twice.)
**The expensive one** is persistent activity — a mechanism class rather than a
tuning knob, and one this creature has resisted before.

> **The cheap one was taken: DNA v52 below.** It is worth reading this section's
> conclusion against what that measured. A *fixed* cut of the larynx reaches
> only p = 0.569 where the supervised readout above reaches 0.740, so the
> readout loses more of the effect than the persistence shortfall does — and the
> next thing to build is a partition that is learned, not persistence.

(On the expensive option: no module here holds a kick for 10 ms — `seqprobe`,
r 0.92 -> 0.03 at every recurrent weight up to 8x — and an utterance is a held
vowel rather than a trajectory.)

What is not in doubt is where the obstruction is. It is not credit assignment,
delivery, expressiveness or reward composition — all four are now measured and
none of them is it.

### DNA v52 — the creature indexes itself, and it is not accurate enough

`ctxsrc` left two options and this is the cheap one, built and measured. DNA v51
works, but `areax` has the **host** write the context slice from the word label,
and that oracle is the honest limit on the result. `ctxsrc` said where a
self-derived index would have to come from — the larynx, which carries the word
at 0.740 in the reward window where the ear is at 0.541 — and said 0.740 was
just under the 0.75 the bar needs.

**Why build it after a proxy said no, and why that is not fitting a verdict to
data.** `ctxsrc`'s 0.740 is a *held-out supervised* readout: it fits centroids
using the word labels and reports the best any linear decoder could manage. What
v52 installs is a **fixed, unsupervised partition** — the same argmax v51 already
runs, pointed at the larynx — which can only do worse. So the proxy was not a
prediction that got ignored; it was an upper bound that came out marginal, and
once an upper bound is marginal the direct measurement is the cheaper instrument
and the only one that can settle it.

The change is one genome field, `context_source`. `0` is the `kContext` module
(v51, and bit-identical to it); `1` reads the same argmax over the larynx's own
slices, restricted to the neurons **outside** articulator groups 2 and 3. Those
two are F1 and F2, the pair the bias table steers and the score is computed
from; an index read from them would be read from the very quantity the mechanism
is changing. `ctxbias` had already separated those populations for this reason —
its `offaxis` arm put the same ramp on groups the score does not read, and it
arrived in full and scored zero. No new rule, no new state, no new constant: the
whole change is which neurons the loop walks.

**`ctxself` runs `areax`'s arms plus two, and every arm gets the same host drive
on the context module**, so `self` and `oracle` differ in exactly one field and
`self` is not also a creature that was deprived of something.

Nine creatures, 3.4M ticks each:

| arm | dF1 (Hz) | p(index) | busiest slice | table div | change |
|---|---|---|---|---|---|
| off | 31.2 +/- 7.4 | — | — | 0.0000 | +3.7 |
| **oracle** | **89.0 +/- 8.9** | **1.000** | 0.500 | 0.0286 | +15.2 |
| **self** | **37.1 +/- 7.1** | **0.569 +/- 0.018** | 0.521 | 0.0183 | +2.2 |
| self-rnd | 17.8 +/- 4.2 | 0.525 | 0.512 | 0.0167 | +0.4 |

**The instrument checks itself and passes.** The oracle arm's index agrees with
the word 1.000 of the time, which is arithmetic if the kernel is reading the
condition the host wrote and a bug otherwise, and its dF1 reproduces `areax`
in-run (89.0 against 82.0 there at the same tick count, +57.8 over `off` at 3.5
SE). The reference is measured here rather than quoted, because a `self` result
means nothing beside an oracle arm that did not work either.

**The creature's own index does carry the word.** p = 0.569 +/- 0.018 is 3.8 SE
above chance, and the busiest slice takes 0.521 of the reward window — it is a
real index and not a constant dressed up as one. It even **sharpens over the
session, 0.564 -> 0.607**, which is the feedback loop closing: the bias table
cashes into the same off-axis neurons the index is read from, so the mechanism
can steer its own index. That loop cannot raise p, which is agreement with the
caregiver rather than with itself, and `self-rnd` runs the identical loop with a
target that does not track the word.

**And the effect is the size the index's accuracy predicts.** An index right
with probability `p` writes the other table `1-p` of the time and the wrong write
*cancels*, so the conditional part scales as `(2p - 1)`. At p = 0.569 that
predicts 31.2 Hz; the creature delivered 37.1.

> **It does not clear the bar, on two seed families and on both tests.** On
> these creatures `self` beats the matched-marginal control by +19.3 Hz at 1.7
> SE unpaired (8 of 9 positive); on a fresh family with the correct paired test
> fixed in advance it is +9.9 +/- 7.4, 1.4 SE, 6 of 9. See the replication
> section below — the gate does not move.

**Two shortfalls, and the second is the larger.** This is what the run adds to
`ctxsrc`, and it changes the prescription:

| stage | p | keeps |
|---|---|---|
| the host's oracle | 1.000 | 100% |
| the best readout of the larynx there is (`ctxsrc`, supervised, held-out) | 0.740 | 48% |
| **a fixed unsupervised cut of the same population (this run)** | **0.569** | **14%** |

Persistence costs just over half the effect. **The fixed cut then costs 71% of
what was left** — more, as a multiplier, than persistence did. `ctxsrc` concluded
that what stands between v51 and naming is nowhere to *hold* a context; that is
true and it is not the whole story. The information that survives into the
reward window is largely there — 0.740 of it — and a fixed equal-sized slicing of
the larynx recovers only 0.569 of it. **The cheaper target is a partition that is
learned rather than fixed**, not more ticks and not persistence.

#### The gate that could not have passed, which was my error and not the run's

`ctxself` shipped requiring `self` to beat **both** `off` and the
matched-marginal control at 2 SE. The `off` half cannot pass — not "did not",
cannot, on numbers available before it ran:

```
detection threshold vs off, n=3     2 x (11.7 + 11.6)  =  46.6 Hz
largest lift ctxsrc's bound allows        55.3 - 30.7  =  24.6 Hz
```

The gate demanded a lift twice the maximum its own pre-stated upper bound
permits, and seeds do not close the gap: n=6 needs 33.0 Hz and n=9 needs 26.9,
both above 24.6. At n=9 with this run's own numbers it is 29.1 against 11.5.
**A gate a perfect result would also fail is not a gate.** That is the same test
`areax` used to retire `fixed+on` — the control falsified a prediction it makes
itself, rather than being dropped because of the numbers underneath it — and the
arithmetic is in the source beside the gate.

There is a structural reason too, not only a power one. `off` carries no split
table, so its dF1 is incidental spread between two words with nothing
suppressing it; a split table averages opposing writes toward zero, which is why
`self-rnd` sits **below** `off` (17.8 against 31.2) rather than beside it. They
are not two measurements of the same zero. `self-rnd` is the zero of a creature
carrying identical machinery and differing in one thing — whether the target
tracks the word. `off` stays in the table and in the report as a diagnostic.

#### The paired re-analysis, and the replication that refused it

`ctxself` is a **paired design** — every arm runs the same creature, differing
only in genome fields — and its first two runs gated on an **unpaired** SE.
That is not a second valid choice; it discards the design's whole advantage,
since the between-creature spread (`off` ranges 3.1 to 75.7 Hz) is common to
both arms and cancels. On the nine creatures already run, the two tests
disagree:

| comparison | unpaired | paired |
|---|---|---|
| self vs self-rnd | +19.3, **1.7 SE** | +19.3, **3.4 SE**, 8/9 positive |
| self vs off | +5.9, 0.4 SE | +5.9, 0.6 SE |

**The error was noticed only after the gate refused**, which is when a change of
statistic is least trustworthy. So the verdict was not flipped. Instead the
paired test was written into the source as the gate, and the run that decides it
used a **fresh seed family** — genome seed `20260902` against `20260809`, sharing
no creature seed, so it is out-of-sample rather than a re-analysis.

> **IT DID NOT REPLICATE.** On fresh creatures with the paired test
> pre-registered: **+9.9 +/- 7.4 Hz, 1.4 SE, 6 of 9 positive.** The gate asks
> for 2 SE and does not get it. The 3.4 SE above was a post-hoc statistic doing
> what post-hoc statistics do.

The oracle arm reproduced in that run (82.4 Hz against 17.4, +65.0 at 5.5 SE),
so the refusal is a fact about the derived index and not about a dead reference.

Pooling both families gives +15.8 +/- 4.5, 3.5 SE across eighteen creatures.
**That number is recorded and not claimed**: family A is what motivated the
choice of test, so pooling it back in reintroduces exactly the selection the
replication was run to remove. The out-of-sample estimate is family B alone.

**Had the verdict been flipped on the re-analysis, this project would now carry
a false positive.** That is the whole return on the fresh-seed run, and the
reason to keep paying for it.

#### Three seeds said 0.603 and nine say 0.569

The first run of this experiment used three creatures and read p = 0.603 +/-
0.051 with `self` at 44.6 Hz and +31.0 against the control. At nine it is 0.569
+/- 0.018 and +19.3. **Same direction and same size of drift as `ctxsrc`'s 0.754
-> 0.740**, three days earlier, and the reason the run was nine this time is
that lesson rather than anything about these numbers. It kept going: the fresh
family reads **0.540**, so the sequence across every run of this measurement is
0.603 (n=3) -> 0.569 (n=9) -> 0.540 (n=9, fresh), keeping 21% -> 14% -> 8% of
the conditional effect. The quantity the verdict
turns on is a difference of tens of Hz between arms whose per-creature spread is
tens of Hz, which is exactly the regime where three seeds decide nothing.

One detail worth recording and not worth leaning on: at n=3 the single seed
where `self` fell below `off` was the seed whose index read p = 0.506, chance.

**`context_source = 0` is bit-identical**: hash `ad96f882becbee92`, `verify`
22/22, `mechverify` 18/18. Like v51 it cannot be pinned in `mechverify` — it is
a scalar field, so a row would patch cleanly, but it does nothing without the
`kContext` module the shipped genome has not got, so the variant would hash
identically and read VACUOUS. What covers it instead is `ctxself`'s own gate
requiring the oracle arm to reproduce `areax` inside the same run, which is a
per-run check that the index path is live.

### `partprobe` — pricing the learned partition, and it refuses

`ctxself` refused v52 at p = 0.540 where `ctxsrc`'s supervised readout of the
same seven off-axis groups reaches 0.740. The obvious reading was that the loss
is the **cut** — v52 slices the population into two equal contiguous halves, and
a partition drawn from the data would recover it. Lateral competition (v32)
already runs on that module, so it would not even be a new mechanism class.

This project prices a mechanism before building it — `credit`'s reward mask,
`ctxbias`'s bias oracle, `rpeprobe`'s variance decomposition — and two of those
three came back saying don't. So the same question, read-only, on `ctxsrc`'s own
trials: **if the partition were learned, how good would the index get?**

**Three things could be losing the signal, and naming one without the others is
how this project's last three instrument errors happened.**

1. **The cut.** Fixed equal halves against a boundary drawn from the data.
2. **The representation.** `ctxsrc`'s 0.740 is measured on the seven
   ARTICULATOR GROUP VALUES — the knobs. v52's rule reads NEURON SLICE RATES.
   Different feature spaces, and the 0.740 -> 0.540 gap was being attributed
   entirely to (1) when part of it is this.
3. **The per-tick argmax.** v52 argmaxes every tick; every column here argmaxes
   the bin average once.

`run_ctxsrc_session` now hands back its raw features, so every column runs on
identical trials, identical reward bins and an identical held-out split — only
the rule differs. Only the supervised column sees labels when it draws its
boundary; k-means picks its restart by within-cluster sum of squares and never
by accuracy.

**The bar, derived and stated first.** The effect scales as `(2p - 1)` against
the 65 Hz an oracle index buys, so p = 0.65 keeps 30% — about 20 Hz, ~2.8 SE on
the paired test at n=9, and a third of the way to naming. **Build only at
p >= 0.65.** Below that the mechanism is too small to be a route to naming
however significant it is, which is the reasoning that retired v52's residual.

Nine creatures, worst of the two reward bins:

| features | supervised | k-means (z) | k-means (raw) | fixed cut |
|---|---|---|---|---|
| articulator groups | **0.740 +/- 0.040** | 0.616 +/- 0.055 | 0.626 +/- 0.055 | 0.607 +/- 0.036 |
| off-axis neurons | 0.561 +/- 0.016 | 0.512 +/- 0.024 | 0.507 +/- 0.024 | 0.475 +/- 0.016 |

Shuffled controls: 0.474 and 0.461. **The supervised column reproduces
`ctxsrc`'s 0.740 to three decimals**, which is the gate that says this probe is
looking at what that number was measured on — without it, no comparison drawn
against 0.740 would be valid.

> **DO NOT BUILD IT. A learned boundary buys +0.019 over the fixed one on the
> same features.** That is the entire case for the mechanism and it is not
> there. The cut was never what was losing the signal.

**The largest single term is the one that had not been separated.** Supervised
reads 0.740 on the articulator groups and 0.561 on the neuron slice rates — a
loss of 0.179, against 0.124 for the cut — and neuron slice rates are what v52
actually reads. Going straight to the build would have replaced the cut and left
the term costing three times as much untouched.

**And the ceiling is unreachable in principle rather than in practice.** 0.740
is what a decoder achieves *by being told the answer*. Unsupervised on the same
features is 0.626. **The gap is the labels, and the labels are exactly what the
creature is trying to infer.** That is a circularity, not a tuning problem, and
it is the finding this probe exists to have produced.

**One thing this probe does NOT decompose, stated rather than buried.** Its
fixed cut on neurons reads 0.475 where `ctxself` reads p = 0.540, and those two
cannot be subtracted to isolate the per-tick argmax: this runs read-only on the
shipped genome with no context module for 600k ticks, `ctxself` runs a taught
creature carrying an active bias table for 3.4M. Genome, regime and session
length all differ. The first version of this write-up printed that difference as
a third term in the decomposition, which it is not; it is now reported side by
side and labelled.

**What it leaves as the lead.** Not a better clustering of the motor state —
that is now measured and refused. A partition **supervised by something the
creature actually has**: reward, or the caregiver's own timing. Those are
signals present at the moment the index is needed and independent of the word
the creature is trying to name, which is the one property an unsupervised
clustering of its own voice cannot have.

### DNA v53 — a competitive index off the ear, and the oracle the probe kept

`partprobe` said build it: the word is separable in the auditory code without
labels and without restarts, at 1.000, where the best unsupervised partition of
the motor state reaches 0.670. So v53 is `context_source = 2` — a competitive
partition of the auditory code, learned online with no labels, formed while the
word plays and **latched** across the silence.

Three design choices, each forced by a measurement rather than picked:

- **One update per word, not per tick.** The rate vector accumulates while the
  creature is listening and the competition runs once at the end, which is what
  `partprobe` scored. Per-tick wins would drive MacQueen's `1/wins` to zero
  inside a single word and freeze the prototypes on noise.
- **The conscience**, which is the whole difference between 0.584 and 1.000. Its
  strength is derived: the penalty is in the data's own distance units and the
  fair share is `1/K`. It also solves initialisation for free — prototypes start
  at zero, the first episode is a tie, and the share penalty is what makes the
  second unit claim a word.
- **The latch.** `ctxsrc` reads the ear at 1.000 during the word and 0.541 by the
  reward window, and that decay looked fatal only under the assumption that the
  index is *recomputed* when reward lands. It is not. The index is formed while
  the word plays and held until the next word, so the context is "the last thing
  I heard" — and `ctx_present_` now stays true once latched, so the cash-in
  reaches that table rather than the shared bias.

#### The oracle `partprobe` quietly kept

**`partprobe` scored the ear inside `ctxsrc`'s word bins, and those bins are
written by the host from the trial structure.** So "1.000" always presupposed
*knowing where the word is* — and the creature does not. Between caregiver words
it babbles and hears itself, so auditory activity marks SOUND, not the
caregiver. A mechanism can price at 1.000 and still have nowhere to start.

That is the same class as `areax`'s host-written slice and `ctxsrc`'s supervised
ceiling: an oracle one level below the one that had just been removed.

**Four boundary detectors failed before one worked, and each died against a
counter rather than against judgement** — `ev/tri`, competitions per trial,
where the design is one:

| boundary | ev/trial | why it failed |
|---|---|---|
| ear vs a fixed 1 Hz floor | 0 or ∞ | that constant was derived for a `kContext` module, which rests at *exactly* zero. The ear never does |
| ear fast vs ear's own 1 s EMA | 26–38 | a one-second reference **catches up to a 900-tick word mid-word** |
| ear fast vs its setpoint | 20–28 | a tens-of-ms EMA of a spiking response crosses any threshold repeatedly |
| ear slow vs its setpoint | ~0 | the creature's own babble keeps the ear above setpoint |

The one that works is **the larynx, not the ear**: M1d's listening reflex is
shipped and measured — the creature falls nearly silent while it hears something,
voiced fraction 0.276 -> 0.010 — so *quiet is listening*. Each module then does
what it is good at: the ear supplies the feature, the larynx supplies the
boundary, both against their own genome setpoints. Still no new constant. That
reads `ev/tri` ≈ 1.2 against a design of 1.

#### The result: better than v52, and it still refuses

Six arms, nine creatures, 3.4M ticks, on a fresh seed family.

| arm | dF1 (Hz) | p(index) | busiest | table div |
|---|---|---|---|---|
| off | 17.4 +/- 5.5 | — | — | 0.0000 |
| oracle | 82.4 +/- 6.3 | 1.000 | 0.500 | 0.0280 |
| self (v52) | 23.3 +/- 5.6 | 0.540 | 0.513 | 0.0179 |
| **ear (v53)** | **36.0 +/- 5.5** | **0.643 +/- 0.016** | 0.772 | 0.0239 |
| ear-rnd | 22.6 +/- 5.6 | 0.729 | 0.664 | 0.0242 |

Paired, which is this design's correct test:

- **`ear` − `ear-rnd`: +13.4 +/- 7.8 Hz, 1.72 SE, 8 of 9** — the gate asks 2 SE.
  **Short.**
- `ear` − `off`: +18.6 +/- 5.2 Hz, **3.6 SE**, 8 of 9.
- v52 for comparison: +9.9 +/- 7.4, 1.34 SE, 6 of 9.

> **v53 nearly doubles the index (0.540 -> 0.643, keeping 8% -> 29% of the
> conditional effect) and lifts the voice from 23.3 to 36.0 Hz. It beats the
> no-mechanism arm at 3.6 SE and its own matched-marginal control at 1.72. The
> gate is the control, and the gate does not move.**

**Three things the diagnostics say that dF1 alone would not.**

1. **`ear-rnd` has a HIGHER index accuracy than `ear`** — 0.729 against 0.643.
   The arms are not matched on `p`, because the index is read from the ear and
   the ear hears the creature's own voice: changing the target changes the
   behaviour changes the index. The matched-marginal control is less clean here
   than its name suggests, and that is a property of reading a context off a
   modality the creature also drives.
2. **It does not sharpen** (0.641 -> 0.634 across the session). The prototypes
   converge early and stay, so more trials buy nothing.
3. **`busiest` is 0.772, where `partprobe`'s clean windows gave exactly 0.500.**
   Roughly three-quarters of reward windows land in one context. That lopsidedness
   is the visible cost of the creature finding its own window.

### DNA v53 source 4 — the creature derives its own context, and it holds up

Eight boundary detectors failed before this one, and the ninth attempt was not a
detector. `partprobe`'s fragmentation counters said why:

    gate episodes per trial       4.27   (the kernel latches the LAST)
    of ALL gated ticks, word      0.55
    of the LAST episode, word     0.20   <- what the creature latches on

The gate fragments, and the fragment the creature commits to is **80% silence**.
Over a whole trial 55% of gated ticks are word, which is why a probe that
averages the trial into one vector reads 0.980 where the creature read 0.68: the
creature throws that average away and latches the last piece.

**So the fix removes the machinery rather than repairing it.** An accumulator
RESETS, so a fragment holding 80% silence carries almost nothing. `rate_ema_` is
already a per-neuron EMA over roughly the last second and never resets, so at
reward time it still carries a word that ended 400 ticks earlier. **The memory
belongs in the feature, not in a latch.** Source 4 reads the index straight off
that EMA every tick: no gate, no accumulator, no episode, no latch. The 4.27
episodes stop mattering instead of having to be fixed.

`partprobe` priced it first, on the same trials and the same split as everything
else, under the exact rule the kernel runs:

| features | supervised | batch | online | onl+conscience | AS KERNEL | fixed cut |
|---|---|---|---|---|---|---|
| EAR, host window | 1.000 | 1.000 | 0.584 | 1.000 | 1.000 | 0.466 |
| EAR, self window | 0.999 | 0.999 | 0.689 | 0.990 | 0.980 | 0.474 |
| **EAR ema @ reward** | **1.000** | **1.000** | 0.520 | **1.000** | **1.000** | 0.472 |

This is **not** a contradiction of `ctxsrc`'s 0.541 for the ear in this window.
That counted spikes INSIDE the bin, which remembers nothing before it. An EMA is
a different measurement of the same module, not a better decoder of the same
quantity — and the fixed cut still sits at chance, so nothing here is a leak.

#### The result, replicated out of sample

| arm | family two | family three |
|---|---|---|
| off | 17.4 +/- 5.5 | 23.1 +/- 6.9 |
| oracle | 82.4 +/- 6.3 | 65.0 +/- 8.4 |
| `ear` (source 2) | 23.1 +/- 8.7 | 33.3 +/- 8.6 |
| **`ema` (source 4)** | **49.2 +/- 10.0** | **56.1 +/- 10.3** |
| `ema-rnd` | 28.2 +/- 7.4 | 23.0 +/- 7.5 |

Paired against its own matched-marginal control — the gate, unchanged since it
was written down before the run that first used it:

| family | `ema` - `ema-rnd` | seeds | oracle lift recovered |
|---|---|---|---|
| two (`20260902`) | **+21.0 +/- 7.4, 2.8 SE** | 8 of 9 | 49% |
| three (`20260903`) | **+33.1 +/- 14.2, 2.3 SE** | 8 of 9 | 79% |
| **pooled, 18 creatures** | **+23.6 +/- 6.6, 3.6 SE** | **16 of 18** | |

> **The creature derives its own context index and the conditional effect on the
> voice is real.** The last oracle in the Area X architecture is gone: nothing in
> the `ema` arm is written by the host. Source 2 on the same two families reads
> +9.6 (1.0 SE) and +11.9 (1.2 SE) and is refused both times.

**Why the replication was run rather than the first result claimed.** Four
mechanisms have now been tested against this gate — v52's `self`, v53's `ear`,
the `adapt` rate cap and `ema` — and the more candidates you test the cheaper a
2.8 SE becomes. The rule never moved and each mechanism faced its own
matched-marginal control, but a result selected from four candidates has to
survive on creatures the selection never touched. This project has the
cautionary case on file: a paired re-analysis at 3.4 SE came back at 1.4 on
fresh seeds. Pooling is legitimate HERE and was not there — both families were
measured with the gate already fixed, and neither was used to choose it.

**The `(2p - 1)` model held a fourth and fifth time**: p = 0.758 predicts 42.5 Hz
against 49.2 measured, and p = 0.883 predicts 49.8 against 56.1.

#### At twice the session, the creature's own index MATCHES the oracle

`areax` established that the oracle index's effect grows with trials — 82.0 ->
112.9 Hz from 3.4M to 6.8M — and predicted that before measuring it. So the
question for a self-derived index is whether it composes over a long session or
degrades, which the decay (-0.036 per session third) made a live worry.

Family three, the same nine creatures at 3.4M and 6.8M, so growth is measured
within-creature:

| arm | dF1 @ 3.4M | dF1 @ 6.8M |
|---|---|---|
| off | 23.1 +/- 6.9 | 22.5 +/- 5.3 |
| oracle | 65.0 +/- 8.4 | **93.0 +/- 7.2** |
| **`ema`** | 56.1 +/- 10.3 | **93.1 +/- 10.1** |
| `ema-rnd` | 23.0 +/- 7.5 | 27.4 +/- 8.7 |
| `ear` (source 2) | 33.3 +/- 8.6 | 38.8 +/- 8.2 |

    ema - ema-rnd   +65.6 +/- 16.5 Hz, 4.0 SE, 8 of 9    <- the gate
    ema - off       +70.6 +/- 12.7 Hz, 5.6 SE, 9 of 9
    oracle - off    +70.5,             5.7 SE

> **The creature's own index recovers 100.1% of the oracle's lift on this
> family** — 70.6 Hz against 70.5 — and it did not plateau: doubling the session
> took the oracle from 65.0 to 93.0 and `ema` from 56.1 to 93.1.
>
> **CORRECTION, from the replication below: that 100% is family-specific.** On
> family two at the same length the oracle is stronger (109.3) and `ema` weaker
> (72.6), which is **60%**. The honest figure is 60-100%, family-dependent. What
> replicates is the gate — `ema` - `ema-rnd` reads +65.6 (4.0 SE) here and +42.2
> (2.9 SE) there, 8 of 9 creatures both times — not the claim that the index has
> caught the oracle outright.

**And the `(2p - 1)` model finally broke, in the direction that matters.** It had
held five times; here p = 0.873 predicts 69.4 Hz and the creature delivers 93.1.
The model assumes a mis-indexed write CANCELS an correct one, and evidently they
cancel less than that — so it is a lower bound, not an estimate. It is recorded
as broken rather than dropped, because it stopped being conservative in the
convenient direction.

**Where that leaves the goal.** The two words are 460 Hz apart in F1:

| | dF1 | of the word gap |
|---|---|---|
| an oracle BIAS with credit assignment removed (`ctxbias`) | 236 Hz | 51% |
| a LEARNED bias on an oracle index (`areax`, 6.8M) | 112.9 Hz | 25% |
| a LEARNED bias on the creature's OWN index (this run) | 93.1 Hz | 20% |

Naming is not achieved. But the remaining distance is **not** in the context any
more — that now matches the oracle — it is in how large a bias reward can build,
which is `areax`'s territory and a different problem from the one this thread
was about.

### Does it NAME? — the milestone question, and one control that cannot answer it

Every number in this thread was scored on `dF1`, the gap between the MEAN F1 for
one word and the other. That was the right instrument while the question was
whether a context reaches the voice. **It is not the milestone.** A mean shift
smaller than the within-word scatter buys a listener nothing, and node
perturbation works by injecting variance — so a creature can move dF1 and still
be unnameable. `vocab` asks the real question: can a listener tell which word
was said?

So utterances are kept per trial and scored the way `vocab` scores them — a
held-out one-of-two readout over what the creature actually produced. **F1 and F2
only**: `vocallearn` also asks for a different AMPLITUDE and rate per word, and a
readout given those would score loudness as naming.

**The confound the smoke run found first, before the long one.** With no
mechanism at all the readout already reads 0.70 at 200k. That is M1b — the
creature repeats what it just heard — and in this protocol **the context IS the
word just heard**, so imitation and naming are confounded by construction. No
readout over these trials can separate them; the most that can be claimed is the
increment over the imitation baseline, which is why `off` matters more here than
in any earlier measurement.

Both families, 6.8M:

| | `ema` | `off` (echo only) | `ema-rnd` | `oracle` |
|---|---|---|---|---|
| family three | **0.895 +/- 0.018** | 0.610 | 0.650 | 0.973 |
| family two | **0.780 +/- 0.052** | 0.575 | 0.732 | 0.973 |

    PAIRED  ema - off       +0.205 +/- 0.069,  3.0 SE,  6 of 9   PASSES
    PAIRED  ema - ema-rnd   +0.048 +/- 0.046,  1.0 SE,  7 of 9   FAILS

Every shuffled control sits at chance (0.484-0.507), so the readout is honest.
And the echo baseline FALLS with training, 0.704 at 200k to 0.575-0.610 at 6.8M
— the increment is not an artefact of imitation getting stronger.

> **The 93 Hz survives as discriminability**: a listener tells the words apart
> from the creature's own utterances at 0.78-0.90, against an imitation baseline
> of 0.575-0.610, paired at 3.0 SE. That was the thing no earlier measurement
> could predict, because dF1 is a shift in a mean and discriminability is a shift
> against the scatter.

**But `ema-rnd` cannot answer this question, and the reason is structural.** Its
index still tracks the word — p = 0.835 — because the index is read from the EAR,
which hears the word whatever the target is. So the control arm's bias table is
also indexed by the word, and also makes the voice word-dependent, just not in
the direction reward asked for. **A discriminability readout cannot separate
"named correctly" from "named arbitrarily but consistently."** `ema-rnd` is the
right control for *did reward teach the mapping* and the wrong one for *is the
voice word-dependent*.

The evidence that the direction is the taught one is elsewhere and is not
ambiguous: **`change` reads +11.4 and +18.3 for `ema` against +0.1 for
`ema-rnd`**, and dF1 against the matched-marginal control passes on both
families (+42.2 at 2.9 SE, +65.6 at 4.0 SE).

**So the composite claim, and it needs both halves.** The voice becomes
word-dependent above the imitation baseline, AND the dependence is in the taught
direction. Neither number alone is naming: discriminability alone would count an
arbitrary-but-consistent mapping, and dF1 alone would count a mean shift no
listener could use.

**What this is not.** It is TWO words, which `partprobe` shows are trivially
separable in the ear (1.000), and `vocab` already reports four words working and
eight not. A listener telling two vowels apart is not a vocabulary.

#### The measure that answers it: did the voice move the RIGHT WAY?

The held-out readout above fits centroids to the creature's own output, so it
cannot tell **named correctly** from **named arbitrarily but consistently** — and
`ema-rnd`'s index tracks the word too, so that arm's voice IS word-dependent,
just pointed nowhere. Nor can it separate naming from M1b's echo.

So each utterance is projected onto the axis joining the two words' targets,
centred on **the creature's own grand mean**, and the question is whether the
SIGN matches the word it heard. Nothing is fitted to the answer: the centring
uses no labels and the axis comes from the protocol. Chance is 0.500, and an
arbitrary-but-consistent mapping scores chance **by construction**, because its
sign is uncorrelated with the word.

| axis score | family three | family two |
|---|---|---|
| `oracle` | 0.875 | 0.901 |
| **`ema`** | **0.824** | **0.738** |
| `off` (echo only) | 0.503 | 0.561 |
| `ema-rnd` | 0.406 | 0.593 |

    POOLED  ema - off       +0.276 +/- 0.036,  7.6 SE   (9/9 and 7/9)
    POOLED  ema - ema-rnd   +0.244 +/- 0.041,  6.0 SE   (9/9 and 9/9)

> **THE IMITATION CONFOUND DISAPPEARS ON THIS MEASURE.** `off` reads 0.503 on
> family three while the SAME utterances are 0.610 discriminable: M1b's echo
> makes the voice tell the words apart, and does **not** move it in the correct
> direction. So this score cannot be passed by echoing, and it cannot be passed
> by arbitrary consistency — `ema-rnd` lands at 0.406. Both loopholes that made
> the fitted readout unusable are closed by construction rather than by argument.

**And it does not merely point the right way — it partly arrives.** The strict
score, nearest of the two words' ACTUAL targets with nothing fitted:

| | family three | family two |
|---|---|---|
| **`ema`** | **0.670** | **0.604** |
| `off` | 0.514 | 0.518 |
| `ema-rnd` | 0.495 | 0.499 |

    POOLED  ema - off       +0.121 +/- 0.021,  5.9 SE
    POOLED  ema - ema-rnd   +0.145 +/- 0.020,  7.4 SE

**A prediction of mine was wrong here, in the generous direction.** I expected
this score to sit at chance for every arm — a 93 Hz shift against a 460 Hz gap
should rarely flip a nearest-target decision. It reads 0.60-0.67, because the
creature's baseline sits nearer the midpoint than that reasoning assumed.

**What is honest about the size.** The baselines are cleaner on family three
(`off` 0.503, `ema-rnd` 0.406) than on family two (0.561, 0.593), so the
increment varies by family — +0.32 against +0.18. Both gates pass on both
families and every paired comparison is 7/9 or better, but the effect is not the
same size everywhere. And `oracle` reaches 0.875-0.901 where `ema` reaches
0.738-0.824, so a perfect index is still worth something.

> **At two words, the creature names.** It moves its voice the correct way for
> the word it heard, on a measure that fits nothing, that an echo scores at
> chance, and that an arbitrary mapping scores at chance — replicated on two
> fresh seed families, 6.0 SE pooled against the matched-marginal control.
>
> **It is two words.** `partprobe` shows the ear separates them at 1.000, and
> `vocab` already reports four words working and eight not. A listener telling
> two vowels apart is not a vocabulary, and the creature points the right way
> more reliably (0.74-0.82) than it arrives (0.60-0.67).

#### The index scales to four words. The LEARNING at four words is untested.

Before writing a four-word protocol, the cheap gate: the same competitive rule
with the same conscience, on the same feature v53 actually reads, asked for four
clusters instead of two. The four-word set is the hard case by design — `/i/` and
`/u/` sit within 30 Hz on F1 and 1600 Hz apart on F2, a nearly pure F2
discrimination, and `/e/` lands between `/a/` and `/i/` on both.

    two words, k=2    1.000 +/- 0.000   (chance 0.500)
    FOUR words, k=4   0.895 +/- 0.028   (chance 0.250)
    shuffled control  0.271 +/- 0.008

**0.895 against a chance of 0.250, so the partition is not a two-word trick.**
The shuffled control earns its place: a best-of-24-permutations assignment has
room to flatter itself, and it inflates by 0.021.

**What this does NOT license.** It measures the INDEX at four words, not the
LEARNING. Four contexts means four conditional mappings competing for one reward
channel, and `capacity` measured this creature holding TWO orthogonal lessons at
once (0.84) while a conflicting pair collapses to 0.22. Whether node perturbation
can hold four is a separate question this gate says nothing about.

**The four-word build, scoped.** `kWords[8]` already exists and `kVLWords` is one
constant, but four things assume two: `ctx_match`'s best-assignment (2
permutations, needs 24 — the code pattern now exists), `holdout_accuracy` which
**refuses labels above 1** by design, the amplitude and rate targets which are
`word == 0 ? loud : quiet`, and **the axis measure, which does not generalise** —
there is no single axis between four targets. The strict nearest-target score
does generalise for free, at chance 0.250.


#### What is still unexplained, and is not being smoothed over

- **The control's index is sometimes better than the mechanism's.** `ear` is
  worse than `ear-rnd` on 18 of 18 creatures across two builds; for `ema` it goes
  the other way on family three (0.883 against 0.844) and the same way on family
  two. The one explanation offered — that learning perturbs the larynx and
  corrupts its own gate — was refuted by its own data at **+0.72**, the wrong
  sign. Source 4 reads no gate at all, which may be why the asymmetry stops being
  consistent for it.
- **The index still decays slightly across a session** in both `ema` arms
  (-0.036 and -0.046 on family three). Feature-space drift is already refuted as
  the cause: injected drift out to two standard deviations costs the frozen rule
  0.002.
- **The oracle arm itself varies by family** (82.4 against 65.0), so fractions of
  "the oracle lift" are family-relative and the pooled Hz figure is the sounder
  number.

#### The drift hypothesis: confirmed in the creature, refuted as the cause

`ctxself` predicted it and then measured it, per arm, with the frozen rule:

| arm | early third -> last third | |
|---|---|---|
| `ear` (target tracks the word) | 0.697 -> 0.663 | **-0.034** |
| `ear-rnd` (target independent) | 0.763 -> 0.764 | +0.001, flat |

**Decay only in the arm whose voice actually changes.** That is exactly what a
frozen prototype under a moving input looks like, and exactly what a read-only
probe cannot show, because nothing in a read-only session drifts.

**So the fix was derived and it is refuted.** MacQueen's `1/wins` reaches zero,
so the first attempt capped the averaging window at the point where a prototype
is already accurate relative to the gap it resolves: `n_eff = 4*dscale/gap^2`,
both terms already maintained by the conscience, the 2 being where a two-way
boundary sits. In the creature it cost **-0.146 of index on 8 of 9 seeds** and
made *both* arms decay (-0.074 and -0.072) — the signature of a prototype
chasing noise rather than tracking drift. The flaw was stated before the run and
is in the arithmetic: `n_eff` **shrinks as the gap grows**, so it adapts hardest
when the clusters are furthest apart and tracking matters least. It answers "how
long to average to resolve the gap" when the question is "how fast is the input
moving".

A second rate was derived to answer the right question — random errors cancel
and averaging is correct, systematic errors add and the prototype must follow,
and the parameter-free test of which is which is the signal fraction
`|mean error|^2 / mean|error|^2`, 0 when errors cancel and 1 when they align.
**It was priced before building, and the pricing refuses the whole line of
work.** Drift is injected into the creature's own gated features at known sizes,
in a random direction favouring no rule, and scored as a curve rather than at
one guessed magnitude:

| learning rate | 0.0 SD | 0.5 SD | 1.0 SD | 2.0 SD |
|---|---|---|---|---|
| frozen (`1/wins`) | 0.980 | 0.979 | 0.978 | 0.979 |
| `n_eff` cap (refuted in the creature) | 0.997 | 0.997 | 0.997 | 0.997 |
| signal fraction | 0.998 | 0.998 | 0.998 | 0.998 |

> **Every rule is flat out to two standard deviations of drift.** Frozen
> prototypes lose 0.002 where the creature is missing 0.30. The reason is on the
> same page: the two vowels are so far apart that the supervised readout is
> 1.000, so a prototype can be badly stale and still classify correctly.
> **Feature-space drift is not what costs the creature its index**, and the
> adaptive rate that would have fixed it buys 0.018 against a 0.30 gap.

What survives is the asymmetry itself — the learning arm's index really is worse
(18 of 18 creatures across two builds) and really does decay while its control
does not. What is refuted is that drift in the feature space explains it.

**The surviving candidate is episode fragmentation, and the evidence is already
recorded.** `ev/tri` reads **1.2 to 1.5** where the design is one competition per
word. The probe forms ONE vector per trial and scores 0.980; the creature takes
the LAST of 1.2-1.5 episodes, so on 20-50% of trials the latch is set from a
fragment — plausibly one holding only post-word silence. It fits the arm
asymmetry too: the learning arm babbles differently, so its listening gate opens
and closes differently. The test is to take the episode with the most
accumulated ticks rather than the most recent one, which is a change to *which*
episode wins and not to any rate.

#### Two bugs this found in its own instrument

**A collapsed v53 is invisible in dF1.** A context table with one live slice is
arithmetically a shared bias, so the first build printed `ear` and `off` dF1s
that were *identical to the decimal*. Only `busiest = 1.000` showed why. Without
that column the whole 70-minute run would have read as "the ear index does
nothing" when a threshold was wrong.

**And the summary printed the wrong arm.** After the experiment was retargeted
from v52 to v53, two `printf` argument lists still pointed at the old arm, so the
line labelled "THE GATE" showed v52's `+9.9, 1.4 SE, 6 of 9`. The *decision* used
v53's numbers correctly and the verdict text below it was right, which is how the
contradiction became visible. Fixed; the table was always authoritative.

**`context_source = 0` stays bit-identical**: hash `ad96f882becbee92`, `verify`
22/22, checked after v53 touched both `step()` and the arena budget.

#### Isolating the gap: the window is not it, and neither is the rule

`ctxself` measured v53's index at 0.643 where `partprobe` priced the ear at
1.000, and **two things differ between those numbers** — the window (the host's
tick bins against the creature's own listening gate) and the setting (a
read-only probe against a taught creature). Two variables at once is `ctxsrc`'s
third instrument error, so `partprobe` was extended to score the window on its
own: same trials, same labels, same split, same rules, only the window changed.

| configuration | p |
|---|---|
| host window, z-scored, seeded from data rows | 1.000 +/- 0.000 |
| **self window** (v53's own larynx gate), z-scored, seeded | 0.990 +/- 0.006 |
| **self window, raw features, zero init — exactly what the kernel runs** | 0.980 +/- 0.008 |
| **v53 in the creature** | **0.643** |

**The window costs -0.010 and the kernel's own rule costs a further -0.010.**
The creature's gate is even *over-inclusive* — it calls 1522 of 2800 ticks a word
against the host's 900, swallowing ~600 ticks of silence — and still separates
the words at 0.980. The boundary was never the problem, and the four detectors
it took to find it were fixing something that was not broken by much.

That ladder is worth more than the mechanism it was built for: a five-minute
read-only instrument that prices a kernel change before a seventy-minute run
does.

#### The one difference left, fixed, and it changed nothing measurable

The probe accumulated on the larynx's SLOW gate; the kernel accumulated on the
FAST one and competed per fragment. That is the same fast-EMA chatter this
project had just diagnosed on the ear, reintroduced one edit later on the
larynx — the reasoning ("each signal for the job it suits") felt principled and
was wrong in a way already on the page.

Aligning it moved the index 0.643 -> 0.681 and the voice 36.0 -> 23.1 Hz.
**Neither is significant at n=9** (1.1 SE and 1.3 SE), so the two builds cannot
be told apart and the fix is UNVALIDATED. It is kept because it makes the kernel
match the configuration that priced at 0.980, not because it demonstrably helped.

| arm | dF1 (Hz) | p(index) | busiest |
|---|---|---|---|
| off | 17.4 +/- 5.5 | — | — |
| oracle | 82.4 +/- 6.3 | 1.000 | 0.500 |
| ear (v53b) | 23.1 +/- 8.7 | 0.681 +/- 0.030 | 0.585 |
| ear-rnd | 13.5 +/- 4.9 | 0.769 +/- 0.008 | 0.603 |

`ear` - `ear-rnd` paired: **+9.6 +/- 9.4, 1.0 SE, 6 of 9. Refused**, against
v53a's 1.72 SE. Both builds refuse.

#### The finding: the index is WORSE exactly when the target tracks the word

| build | p(ear) | p(ear-rnd) | paired difference | seeds |
|---|---|---|---|---|
| v53a, fast gate | 0.643 | 0.729 | **-0.086 +/- 0.018** | 9 of 9 |
| v53b, slow gate | 0.681 | 0.769 | **-0.088 +/- 0.027** | 9 of 9 |

**18 of 18 creatures, two independent builds, 4.8 and 3.3 SE.** The arm whose
target tracks the word derives a worse context than the arm whose target is
drawn independently of it. The mechanism's own input degrades precisely when the
mechanism has something to learn.

**The obvious explanation is refuted by its own data.** v53 injects bias into
`vocal` and the listening gate is `vocal` below setpoint, so learning should
perturb the gate, corrupt the window, and spoil the index — which predicts that
seeds learning MORE have WORSE indices. The within-arm correlation between how
much an arm learned and how good its index is comes out **+0.72 on 18
creatures**: the opposite sign. Better index -> more learning, which is the
mechanism working rather than undermining itself.

So the asymmetry is real, reproduced, and **unexplained**. The leading remaining
hypothesis is drift: MacQueen's `1/wins` means the prototypes FREEZE — after N
words the learning rate is 1/N — and the taught arm is the one whose own voice
changes under them. The read-only probe cannot see this, because nothing in it
drifts. It predicts the index should DECAY across a session in the learning arm,
and v53b's does: **0.697 -> 0.663**. The test is to report that split per arm,
and the fix would be a learning rate that does not go to zero — which needs
deriving rather than guessing, on this project's own record with constants.


### `ctxfour` — the bias holds four DISTINCTIONS, not four TARGETS

The pre-registered gate passed and the write-up it printed is too generous, so
the number goes in here with the correction attached rather than as a milestone.

`partprobe` had cleared the *index* at four words (0.895 against chance 0.250)
and said nothing about the *learning*. Four contexts is four conditional
mappings competing for one reward channel, and `capacity` measured this creature
holding two orthogonal lessons at 0.84 while a conflicting pair collapses to
0.22. So this ran the **oracle arm and its baseline only**: with the host
writing a perfect index, can the bias hold four mappings at all? The creature's
own index cannot beat the oracle, so a failure here would have made the six-arm
run pointless.

Nine creatures per arm at 6.8M ticks:

Re-run after the instrument was fixed. The first pass's numbers, and why they
moved, are in the section below.

| arm | F1 spread (Hz) | direction | its own null | dir - null | nearest | change |
|---|---|---|---|---|---|---|
| off | 34.1 +/- 4.7 | 0.283 +/- 0.037 | 0.251 +/- 0.004 | +0.032 +/- 0.034 | 0.161 +/- 0.020 | +1.2 +/- 3.1 |
| oracle | 71.6 +/- 8.2 | 0.469 +/- 0.033 | 0.267 +/- 0.006 | **+0.201 +/- 0.029** | 0.279 +/- 0.024 | **-20.0 +/- 3.5** |

`ctx_match` read 1.000 on every oracle row, which is the k-way assignment's own
check and the refactor's last outstanding claim.

**The fix strengthened the result while dissolving the anomaly, which is the
pattern a correct fix makes.** The off arm's unexplained 0.321 became +0.032
+/- 0.034 above its own null -- at its floor, where a mechanism-off arm belongs.
The oracle's null came out at 0.267 rather than 0.250, so its raw score was
inflated too. The gap between the arms went from +0.163 at 2.2 SE on the raw
numbers to **+0.169 at 2.7 SE** on the excesses: the correction took a confound
out of the baseline rather than shaving the effect.

**The prediction was half right, and the half it missed is the half that
matters.** From Werfel/Xie/Seung, learning time scales with parameter count, so
four contexts at 504 parameters against 252 should need ~2x the trials: at 6.8M,
what two words gave at 3.4M — ~65 Hz of spread and ~0.80 direction. The spread
came in at 71.6 Hz, near enough to call a hit. Direction came in at **0.469**
against a predicted 0.80, and +0.201 above its own null. It clears (+0.169 at
2.7 SE) and it is a third of the predicted headroom, not the whole of it.

**And two numbers the verdict text did not read.** `nearest` — which asks
whether an utterance actually lands closest to the right target, the measure
that corresponds to naming — is **0.279 against a chance of 0.250**, under one
SE. And the formant error in the oracle arm *grew 20% over the session* at 5.7
SE, while the off arm was flat. That is not "learning, but slower": slower would
sit near zero. The bias is producing larger, correctly-signed excursions that
land in the wrong places.

So the honest statement is that a context-indexed bias separates four words
along the axis and does not put them on their targets. It holds four
distinctions, not four mappings. **The gate as written asked only about
direction, and direction was the wrong thing to gate on** — a lesson that
belongs with `verdict-fitted-to-data`, except the failure here is choosing the
generous measure in advance rather than after the fact.

One loose end needed an answer before any of this could be built on: the **off**
arm reads 0.321 on direction against a chance of 0.250, +0.071 at 1.8 SE, where
at two words the corresponding arm sat at chance (0.503 and 0.561 against 0.500).
That was either the echo reappearing at four words or a bias in the measure.

### It was the measure: chance on `direction` is not 1/k

Checked the way it should have been checked before the run — by feeding
`direction_accuracy` a voice that carries **no information at all** and seeing
what it reads. Utterances drawn around one operating point, labels drawn
independently of them, the real four targets:

| labels | shipped | normalised |
|---|---|---|
| balanced | 0.243 / 0.258 / 0.258 | 0.242 / 0.250 / 0.258 |
| skewed 40/30/20/10 | **0.288 / 0.302 / 0.304** | 0.251 / 0.263 / 0.266 |

Two separate faults, and the first is a plain bug.

**The dot product was unnormalised.** The measure picks the target whose
direction the utterance best *aligns* with, but `p · d` is alignment times
magnitude, and the four targets' deviations differ by more than two to one —
`|d|` is 0.270 for /e/, which sits near the centroid of the four, against 0.643
for /i/. So targets far from the centroid win on length alone. On its own that
only skews *which* target is predicted; combined with an uneven label
distribution it moves the accuracy off 1/k. **At k=2 this is provably a no-op**,
because two targets give `d0 = -d1`, both are scaled by the same constant and
the argmax is untouched — which is why it was invisible for as long as the
project only had two words, and why every published two-word number stands.

**And a residue that is not a bug but geometry.** Normalising leaves the argmax
carving the plane into one wedge per target, and four vowels do not sit 90
degrees apart: the shipped four lie at −24.5°, +126.3°, −120.0° and +53.5°, so an
informationless voice is assigned to them 0.241 / 0.259 / 0.290 / 0.210 of the
time. There is no constant to correct that to. The floor depends on the target
geometry *and* on how many utterances carry each label.

So the floor is now **measured on the same utterances instead of assumed**:
shuffle the labels, which preserves their marginal distribution exactly and
destroys only their relation to the voice, and score again — `pgprobe`'s
matched-marginal control applied to the naming score. The mean over 64 shuffles
is used as an offset, not to read a p-value off the rank of the observed score,
which a 64-permutation null cannot resolve.

Two facts make the shuffle the right null here rather than a block-preserving
one. Words are presented strict round-robin (`label = trial % nw`), so each label
is spread evenly across the session and slow drift cannot align with it. The
skew that produces the artefact is therefore not in the *trial* counts, which are
balanced by construction, but in the *utterance* counts — the creature vocalises
more in some trials than others — and shuffling utterance labels preserves
exactly that.

**The gate moved because of this, and the defence is that the null was measured
with no creature in it.** Changing a gate after seeing the data is what the
fitted-verdict rule forbids; the change here would have been made identically had
the run gone the other way, and `ctxfour` now prints the retired raw number
beside the corrected one so the move is visible rather than tidied away.

### `ctxscale` — the bias SATURATES at ~118 Hz (and my first read of it was wrong)

**Read this heading before the section below it.** On three points the excess
looked like it was still growing and this experiment printed COMPUTE-LIMITED. A
fourth point at 27.2M bent the curve, and a model comparison across all four says
the bias is at **94% of an asymptote of ~118 Hz** — against the 460 Hz the two
words are apart. The original three-point reading is kept underneath, because the
way it failed is the useful part.

Two-word naming works and the distance left is how large a bias reward can build:
`ctxbias` showed the route carries 236 Hz of F1 when a bias is handed to the
larynx, and learning builds 93. That is either a matter of trials or a matter of
architecture, and the two remaining leads point opposite ways depending on which.
So it was measured rather than argued. Nine creatures, three arms, three budgets,
each budget a separate session so the points do not share a noise draw.

| budget | trials | off | ema | ema-rnd | excess (ema − rnd) |
|---|---|---|---|---|---|
| 3.4M | 1214 | 23.1 +/- 6.9 | 56.1 +/- 10.3 | 23.0 +/- 7.5 | +33.1 +/- 17.8 |
| 6.8M | 2428 | 22.5 +/- 5.3 | 93.1 +/- 10.1 | 27.4 +/- 8.7 | +65.6 +/- 18.8 |
| 13.6M | 4857 | 25.9 +/- 2.5 | **119.2 +/- 8.2** | 28.9 +/- 6.4 | **+90.3 +/- 14.6** |

**The controls carry the result.** Across a 4x span in trials the `off` arm moves
23.1 -> 25.9 and the matched-marginal `ema-rnd` moves 23.0 -> 28.9 — both flat —
while the taught arm goes 56.1 -> 119.2. Growth this specific to the arm that is
being taught is hard to get from a drift, a warm-up or a measurement artefact,
all of which would lift the controls too. And the 6.8M point reproduces the 93 Hz
already on record from a different experiment, which is a free replication.

Excess grew 2.73x over a 4x span, against the 2.0 that sqrt(t) predicts —
**implied exponent 0.72, between diffusion and linear.**

**The concern this was built to test is refuted by the table-divergence column.**
The one hint on record was that divergence grew like sqrt(2) when the session
doubled, and the worry was that a magnitude can grow while the useful component
does not — an unbiased random walk grows as sqrt(t) too. Divergence here goes
0.0275 -> 0.0363 -> 0.0452, an exponent of **0.36**, while the useful effect grows
at 0.72. The tables are getting *better aligned*, not merely bigger. That is the
opposite of the random-walk story.

**What it does not license.** The exponent rests on three points with standard
errors of +/-17.8, +/-18.8 and +/-14.6 — the shortest budget's error is over half
its value. "Compute-limited rather than saturated" is safe, because saturation
would need the excess to stop moving and it nearly tripled. The *exponent* is
soft, so the extrapolation it prints — 4x more trials to reach 236 Hz — is a
direction, not a plan.

### The fourth point, and the reversal

Extending to 27.2M (9714 trials) was launched with a prediction on record:
exponent 0.72 put the excess at **~149 Hz**, with "at or below ~100" declared as
the falsifier.

| budget | trials | off | ema | ema-rnd | excess |
|---|---|---|---|---|---|
| 6.8M | 2428 | 22.5 | 93.1 | 27.4 | +65.6 +/- 18.8 |
| 13.6M | 4857 | 25.9 | 119.2 | 28.9 | +90.3 +/- 14.6 |
| 27.2M | 9714 | 20.3 | **141.4 +/- 7.6** | 30.0 | **+111.5 +/- 14.7** |

**It came in at 111.5.** The pre-registered ratio landed on 1.70, the exact edge
of the inconclusive band declared before the first run, and the experiment
printed INCONCLUSIVE — the gate doing its job rather than me choosing.

But a ratio between the two extreme points throws away the middle ones, and with
all four the picture is not ambiguous at all. The per-doubling ratios are
**1.98, 1.38, 1.23** — the growth exponent halves every doubling:

| model | fit | chi-squared |
|---|---|---|
| power law `A·t^k` | k = 0.47 | 0.72 |
| saturating `A(1 − e^−t/τ)` | **A = 118 Hz**, τ = 3300 trials | **0.08** |

The saturating model fits an order of magnitude better, and it puts the creature
at **94% of its asymptote already**. Doubling the trials again buys 6 Hz; ten
times the trials buys nothing at all.

**Choosing a model after seeing data is the fitted-verdict trap, so the defence
has to be stated.** These two models are not a fishing expedition — they are the
two named hypotheses from the experiment's own header, written before it ran:
"compute-limited" *is* the power law and "saturated" *is* the asymptote. What the
data chose between was declared in advance; only the discriminating statistic
changed, from a two-point ratio to a four-point fit, and it changed because the
ratio provably ignores half the evidence.

**One thing I mis-sold.** I called the repeated 6.8M and 13.6M points an
out-of-sample replication. They are not: same genome, same seeds, so those are
bit-identical re-executions. They are a determinism check — a good one, they
reproduce exactly — but they add no independent evidence.

**The two columns disagree, and that is the actual finding.** Fitting the
`ema` arm's delivered dF1 and its table divergence separately, over the same four
budgets:

| | per doubling | power law | saturating | verdict |
|---|---|---|---|---|
| delivered dF1 | 1.66, 1.28, 1.19 | k=0.37, chi2 2.50 | A=144 Hz, chi2 **0.38** | **saturates** |
| table divergence | 1.32, 1.25, 1.25 | k=0.34, chi2 **0.27** | chi2 9.09 | **does not** |

The creature goes on learning at a perfectly steady rate — divergence grows 1.25x
per doubling with no deceleration at all — while the voice stops responding. So
the ceiling is not how much it learns.

**And it is not the route either**, because `ctxbias` handed a bias straight to
the larynx and got 236 Hz through the same path. The route carries more than
144 Hz; the learned bias cannot get there.

**What is left is SHAPE.** Additional learned magnitude stops converting into
delivered separation, which means it is increasingly going into directions that
do not move F1. That also inverts something claimed one section above: on the
first three points the useful effect grew faster than the table magnitude and I
called it "the tables are getting better aligned". With the fourth point the
exponents cross the other way — 0.19 for delivery in the last doubling against a
steady 0.32 for divergence — so the alignment is getting *worse*, not better. The
random-walk story I said was refuted is back, in a sharper form: the walk is in
the part of the table that does not reach the voice.

**This does not refuse Kornfeld — it aims it.** A mechanism whose whole content is
*gating which parameters may change* is pointed directly at a bias that grows in
unproductive directions. The next measurement is the cheap one that discriminates:
project the learned table difference onto the direction that actually moves F1,
and watch that component against the orthogonal one across budgets. If the aligned
part saturates while the orthogonal part grows, the diagnosis is settled without
building anything.

**So the architecture IS the binding constraint, and the earlier conclusion
inverts.** 118 Hz is half the 236 Hz the route can carry and a quarter of the
460 Hz gap, so `nearest` — which needs an utterance to cross the midpoint at
~230 Hz — cannot come off chance by spending trials. Kornfeld's compartments stop
being "not yet needed" and become the live lead.

**One prediction worth having on record.** `nearest`, the absolute naming score,
only flips when an utterance crosses the midpoint between two targets 460 Hz
apart, which is ~230 Hz of dF1. So if the curve holds, the absolute measure
should come off chance at about the same place the extrapolation reaches 236 Hz —
two independent quantities meeting at one number, which is the kind of
coincidence that is worth being wrong about publicly.

### The alignment split — my prediction was refuted, and the readout is the wall

Pre-registered before the run: if the ceiling were the *shape* of what reward
writes, `gain` should fall across budgets. The falsifier declared alongside it was
gain flat or rising, which would mean the ceiling is a delivery nonlinearity and
send the work somewhere other than Kornfeld.

**Gain rose.**

| budget | aligned (F1) | common-mode | outside group | gain (1.0 = structureless) |
|---|---|---|---|---|
| 6.8M | 0.03529 | 0.01103 | 0.04425 | 2.97 +/- 0.19 |
| 13.6M | 0.05261 | 0.01426 | 0.05444 | 3.57 +/- 0.18 |
| 27.2M | 0.07017 | 0.01456 | 0.06840 | **3.82 +/- 0.28** |

Growth exponents per doubling make it sharper still:

| | exponents |
|---|---|
| aligned (moves F1) | **0.58, 0.42** |
| outside group | 0.30, 0.33 |
| delivered dF1 | 0.36, 0.25 |

The component that can move F1 is growing **fastest of the three** — faster than
the useless directions, and faster than the formant separation it produces. The
table is nearly four times better aligned than a structureless one and getting
more so. **Reward is writing the right direction, and writing it increasingly
well.** The shape hypothesis is dead.

**So the loss is downstream of the bias.** Plotting delivered dF1 against the
only part of the bias that can produce it:

| aligned | delivered | Hz per unit |
|---|---|---|
| 0.03529 | 93.1 | 2638 |
| 0.05261 | 119.2 | 2266 |
| 0.07017 | 141.4 | 2015 |

`dF1 ~ aligned^0.61`. The larynx compresses what it is given.

**And that corrects "SATURATED at ~118 Hz" from the section above.** A saturating
fit whose asymptote lands 2% past the last data point is the classic way an
asymptote gets fitted to a decelerating power law, and that is what happened. The
underlying quantity shows *no* deceleration — aligned bias keeps growing at
exponent ~0.5 — so the bend in dF1 is the readout compressing, not learning
stopping. A compressive transfer is not a wall; it is an expensive slope.

**The suspect is the pooling rule itself.** `read_group` is a ratio,
`Σ(r_i·p_i)/Σ(r_i)`, and a ratio's sensitivity falls as the rates feeding it
grow — which `ipctx` already saw from the other side, with 25-83% of the larynx
pinned at `threshold_max`. This is the same common-mode pooling wall this project
has now hit from eight directions.

**Next, and it discriminates cleanly.** At 2x more trials the two readings
predict **174 Hz** (compressive power law) against **144 Hz** (true asymptote),
against a per-arm SE of about 8. Running it before choosing.

### The discriminator, and one transfer curve that two routes agree on

Pre-registered before the 54.4M run: the compressive reading predicted **174 Hz**,
the asymptote reading **144**. Actual: **137.4 +/- 7.5** — 4.9 SE from mine, 0.9
from the other. My reading is refuted.

| budget | trials | off | ema | ema-rnd | excess |
|---|---|---|---|---|---|
| 13.6M | 4857 | 25.9 | 119.2 +/- 8.2 | 28.9 | +90.3 |
| 27.2M | 9714 | 20.3 | 141.4 +/- 7.6 | 30.0 | +111.5 |
| 54.4M | 19428 | 14.7 | **137.4 +/- 7.5** | 33.7 | +103.8 |

One thing not to over-read: 141.4 -> 137.4 is 0.4 SE. It is not a fall, it is a
flat line, and describing it as the readout "switching off" would be reading noise.

**Meanwhile everything upstream keeps growing, exactly as before** — aligned
0.0526 -> 0.0702 -> 0.0847, outside 1.254x per doubling, table divergence 1.251x,
gain steady at 3.57 / 3.82 / 3.76. Learning does not stop. Delivery does.

**And the aligned axis lets the learned bias and the hand-supplied one be put on
one curve for the first time.** `ctxbias`'s oracle is a ramp of `k x noise_amp`
across the F1 group; converting it into the same aligned units gives 0.507 at
k=2, where it delivered 236 Hz. Fitting all five points — four learned, one
oracle — to a saturating transfer:

| aligned | delivered |
|---|---|
| 0.0353 | 93.1 |
| 0.0526 | 119.2 |
| 0.0702 | 141.4 |
| 0.0847 | 137.4 |
| 0.507 (oracle) | 236.0 |

**Asymptote 234 Hz.** `ctxbias` measured 236 independently. Two routes, one
number, and neither was fitted to the other.

So the position is now precise, and it is not the one the verdict text used to
print:

- **The readout is not broken.** It reaches 236 Hz when driven hard enough.
- **The shape is not the problem.** Gain holds at 3.8x a structureless table.
- **Credit assignment is not the problem.** The aligned component grows fastest.
- **The learned bias is simply too small** — 0.085 against the 0.326 that curve
  says is needed for 230 Hz, about 3.8x — and it grows at exponent 0.27 per
  doubling, so trials would need **147x**. Dead by compute, and dead for a reason
  that is now measured rather than inferred.

**The target is therefore the equilibrium magnitude of the learned bias.** It
comes with a payoff already priced: 3.8x more aligned bias is 230 Hz, and 230 Hz
is where `nearest` crosses the midpoint and absolute naming becomes possible.

**And this paragraph used to say the equilibrium was "a balance between what
reward writes and what leaks away", which is wrong: nothing leaks.** `bias_ctx_`
has no decay term at all. It is a clamped accumulator, so under a constant drift
the aligned component would grow **linearly** in trials — and it grows at
exponent 0.27 to 0.58. There is no equilibrium in the rule; there is a bound
coming from somewhere else, and naming that is what `boundprobe` is for. The
second clause was loose too: "not clamped, because the RMS is 0.085 against a
`perturb_max` of 0.30" is precisely the inference `ipctx` demolished for
thresholds, where a mean nowhere near the clamp sat alongside a quarter to
five-sixths of the module pinned AT it. A mean is not a share, so the share is
now measured.

### The slow store, tested at last — refuted, and it validates the transfer curve

DNA v41's third gate had never executed under a context: it relaxed `bias_[i]`,
which v51 never writes. Fixed on 2026-09-09, and the prediction was derived and
committed **before** the run, because the coupling is closed-form.

Per cash-in the pair is `fast += flow*(slow-fast)`, `slow += flow*ratio*(fast-slow)`.
For a coherent drift `u` the gap settles at `u / (flow*(1+ratio))`, after which the
whole pair advances at `u * ratio/(1+ratio)` per event instead of `u`. At
`meta_ratio = 0.05` that is a **21x brake on exactly the quantity we are short of**,
and `meta_flow` does not appear in it at all — it sets only how fast the gap
reaches equilibrium.

Two arms, `meta_flow` 0.02 and 0.1, against the `meta_flow = 0` baseline:

| 13.6M budget | off | ema | ema-rnd | excess | aligned (F1) | gain |
|---|---|---|---|---|---|---|
| **0 (baseline)** | 25.9 | **119.2 +/- 8.2** | 28.9 | **+90.3 +/- 14.6** | 0.05261 | 3.57 |
| **0.02** | 12.7 | 39.0 +/- 6.1 | 16.5 | +22.5 +/- 10.0 | 0.00421 | 4.57 |
| **0.1** | 24.6 | 35.8 +/- 6.4 | 13.0 | +22.8 +/- 9.3 | 0.00422 | 4.40 |

**Refuted, and every clause of the prediction held.** Aligned bias falls 12.5x and
delivered excess 4x. The measured level ratio is short of the derived 21x because
baseline aligned itself grows sublinearly (exponent ~0.5), so a 21x cut in *rate*
shows as less than 21x in *level*.

**The fingerprint that confirms the derivation rather than merely the sign:**
`aligned` reads 0.00421 and 0.00422 across a **5x** change in `meta_flow` — equal
to three significant figures. The accumulation rate is set by `meta_ratio` alone,
exactly as the algebra says. This also corrects the looser phrasing in my own
launch note, which said the loss would be "roughly in proportion to `meta_flow`".
It is not. It is independent of it.

**And the other half of the prediction held too, which is why this is not a
tuning failure.** The store was expected to *improve* the drift-to-diffusion
ratio while cutting absolute drift — and `gain` rises, 3.57 -> 4.57. It does the
thing it is for. This creature is short of **magnitude**, not of SNR: `gain` was
already 3.8x a structureless table, and forcing `outside` to zero bought nothing.
A mechanism that trades magnitude for cleanliness is aimed at a problem the
creature has not got, which is the same verdict `retention` reached from the
other side.

**No `meta_ratio` sweep will be run, and the reason is a bound rather than a
budget.** `ratio/(1+ratio) <= 0.5` for every `ratio` in [0,1], so Benna-Fusi can
at best halve the accumulation and can never raise it. The launch note's "raising
`meta_ratio` toward 1.0 should recover it" is true only up to 50% of the
no-store case. There is no setting that wins, so the question closes analytically.

**The unplanned payoff: a refuted mechanism is a clean probe.** The alignment
split fitted `dF1 ~ aligned^0.61` over a 2.4x range of aligned. These two arms sit
**12.5x below** that range, which no earlier run reached, and the law was never
fitted to them:

| readout model | predicted excess | measured |
|---|---|---|
| compressive, `aligned^0.61` | **19.3 Hz** | 22.5 +/- 10.0 and 22.8 +/- 9.3 |
| linear | 7.2 Hz | 1.5 SE high |
| saturated (flat) | 90.3 Hz | 6.8 SE high |

Within 0.3 SE, a decade below where it was fitted. The compressive readout is now
the only model standing across a 12.5x span of drive, and "the larynx compresses
what it is given" stops being a fit and becomes a measured property.

**So the magnitude question is closed on this architecture.** Every route to a
larger aligned bias has now been priced: more trials saturate near 137 Hz, more
rate grows `outside` 3x and halves `gain`, selectivity buys nothing, the
commitment brake moves it 1.07x where 6.2x is needed, and the slow store moves it
the wrong way by 12.5x. What remains is not a knob on the bias but a different
place to put it.

### `boundprobe` — learning has two phases, and the second one is structureless

`bias_ctx_` has **no decay term**. It is a clamped accumulator, so under a
constant drift the aligned component would grow **linearly** in trials. It grows
at exponent 0.27-0.58. Something bounds it, and after the slow store was refuted
that was the last unpriced thing between this creature and the 3.8x more aligned
bias absolute naming needs.

Three fingerprints were named in advance. 27.2M ticks, 9 seeds, 16 checkpoints
**inside** one session so the points are paired:

| | pre-registered test | measured | fires |
|---|---|---|---|
| H1 drift decays | second-half exponent < 0.40 | **0.29** | **yes** |
| H2 pure diffusion | exponent in [0.40,0.60] **and** gain flat | gain 1.37 -> 3.35 | no |
| H3 clamp binds | pinned share rises, last > 2x first | **0.000 throughout** | no |

**H3 is dead on the number that matters, not on an inference.** Not one table
entry out of 9 seeds x 16 checkpoints ever reached `perturb_max`. The earlier
"RMS 0.085 against a clamp of 0.30 so it is not clamped" was the right conclusion
reached by the wrong argument — a mean is not a share, and that is exactly the
inference `ipctx` demolished for thresholds.

**But the verdict line understates what the rows say.** The session splits into
two phases with a sharp character change at about 4900 trials:

| phase | trials | aligned exponent | outside exponent | gain |
|---|---|---|---|---|
| **1** | 614 -> 4857 | **0.83** | 0.40 | 1.37 -> 3.31 |
| **2** | 4857 -> 9714 | **0.290** | **0.286** | 3.31 -> 3.35 |

In phase 1 aligned grows nearly linearly — a real drift into a leakless
accumulator — while the useless directions grow diffusively, and gain climbs from
structureless to 3.3x. **In phase 2 the two exponents are equal to 0.4%** (ratio
of growths 1.002) and gain is frozen. The table keeps getting bigger and stops
getting better. Delivered dF1 goes 144.6 -> 138.8 Hz, x0.96, where the
compressive law predicts x1.13 off that much more aligned bias.

**So the drift does not decay toward some smaller drift. It goes to zero, and
the rest is isotropic growth the readout cannot use.**

**Why, and it is the reward criterion rather than anything in the plasticity
rule.** Praise is `e < baseline[bucket]`, where the baseline is an EMA
(`kVLBaselineAlpha` 0.02) of the creature's *own recent error*. Praise therefore
means "closer than you usually get", so the expected drift is proportional to the
**rate of improvement**, not to the remaining error. Once improvement stalls the
covariance between reward and perturbation goes to zero on its own, no matter how
far the voice still is from the target.

**And that makes one column of my own instrument unable to answer the question it
was put there for.** Praise share was included to test "the teacher runs out". It
cannot: a self-normalising baseline pins praise near 0.5 by construction. It
reads 0.548 at the first checkpoint and 0.536 at the sixteenth, never leaving
0.43-0.68 — which is not evidence that the teaching signal is healthy, it is what
this criterion prints in every possible world. What it *does* show is that the
creature was at its own baseline from the very first checkpoint.

**The reframing, and it is the sharpest statement of the limit so far.** The
ceiling is not that reward cannot build a bigger bias. The readout is
demonstrably not saturated at 145 Hz — `ctxbias`'s oracle reaches 236 Hz through
that same readout at aligned 0.507. The creature stops at aligned 0.061 because
**its own success criterion stops asking for more.** Compression and the baseline
close a loop: `dF1 ~ aligned^0.61` means each further unit of bias buys less error
reduction, and an EMA baseline converts "diminishing improvement" into "no
gradient at all".

That is a different object from every mechanism tried since v41. It is not in the
genome, not in the plasticity rule, and not in the architecture — it is in the
protocol, which is the one place this project has not looked for the ceiling.

### `baseprobe` — the gate fired, and I do not believe it

`boundprobe`'s phase 2 is node perturbation on its variance floor. That floor is a
ratio of systematic drift to diffusive spread, and everything tried so far
attacked the diffusive half — the commitment brake (1.07x where 6.2x is needed),
the slow store (12.5x the wrong way), a selectivity mask (nothing). The shipped
criterion delivers **one bit per trial**: praise if this trial beat the creature's
own running error, scold if not, magnitude discarded. Graded arms keep the same
comparison and hand the magnitude over, which raises the drift without touching
the step size — the thing raising `perturb_rate` cannot do, since that grows
`outside` 3x and halves `gain`.

**First, `boundprobe` re-measured with the checkpoint bug fixed, because its
`binary` arm is bit-for-bit the arm `boundprobe` ran:**

| | buggy | fixed |
|---|---|---|
| final aligned | 0.06060 | 0.06101 |
| phase-2 aligned / outside exponent | 0.26 / 0.31 | 0.25 / 0.32 |
| gain, first to last | 1.37 -> 3.35 | 1.38 -> 3.34 |
| pinned share | 0.000 | 0.000 |

Unchanged. The selection effect was real and it was small on this arm, so
`boundprobe`'s two phases stand as reported.

**The pre-registered gate fired.** Phase-2 separation, aligned exponent minus
outside exponent, jackknifed over seeds:

| arm | separation | vs binary | final aligned |
|---|---|---|---|
| binary | -0.07 +/- 0.11 (-0.6 SE) | — | 0.06101 |
| graded | +0.13 +/- 0.15 (0.9 SE) | +0.20 | 0.07014 |
| **graded-1** | **+0.23 +/- 0.08 (2.8 SE)** | **+0.30 (2.2 SE)** | 0.06841 |

That clears both pre-registered conditions, and my committed prediction was that
it would not. **The prediction is refuted as stated. The gate is also wrong, and
the second thing matters more than the first.**

**Three numbers say the graded arms are not escaping the floor, they are behind
it.** An arm that escaped a variance floor should end *better*. Both graded arms
end **worse**:

| | binary | graded | graded-1 |
|---|---|---|---|
| gain, first -> last | 1.38 -> **3.34** | 1.65 -> 3.16 | 0.99 -> 2.75 |
| delivered dF1, last window | **146.7 +/- 12.8** | 91.8 +/- 28.1 | 112.7 +/- 27.4 |
| final aligned, **paired** on the same 9 seeds | — | +0.86 SE, 6/9 | +0.61 SE, 5/9 |

Lower final gain, lower delivered dF1 (wrong sign, 1.8 and 1.1 SE), and the
paired test on the quantity the whole thing is about — how much aligned bias the
creature ends with — reads under 1 SE with the signs near even.

**The defect is in what "phase 2" means.** It is the second half of the *trials*,
which is a fixed index and not a fixed stage of learning. An arm that simply
learns slower is still inside its own phase 1 during that window, and will show
exactly the signature the gate rewards — aligned still outgrowing outside —
while finishing behind. `graded-1` starts at gain 0.99 against binary's 1.38 and
ends at 2.75 against 3.34: it is behind at both ends. The gate measured a
learning-rate difference and called it an escape.

**So the gate is replaced by the two things that actually have to be true, each
paired by seed since the arms run on identical creatures: more aligned bias at
the end, and not less delivered dF1.** The second clause is what the old gate
lacked. `aligned x gain` was already refuted as a causal model once — a bias can
grow larger and arrive smaller — so a magnitude win that does not arrive is not a
win. dF1 is now the mean of the last four windows rather than one, because one
window carries an SE of about 27 Hz against an effect of about 30.

**And the run is repeated longer**, at 40.8M, so that an arm which learns more
slowly still reaches its own plateau inside the session. Comparing a converged arm
with an unconverged one at a fixed trial count is the confound above; the only
clean fix is to let both converge.

**The corrected gate, at 40.8M so a slower arm still reaches its own plateau:
no escape.**

| budget | arm | final aligned, paired | delivered dF1, paired |
|---|---|---|---|
| 27.2M | graded | +0.86 SE | (-54.9, unpaired) |
| 27.2M | graded-1 | +0.61 SE | (-34.0, unpaired) |
| 40.8M | graded | +0.7 SE | -19.1 +/- 35.3 (-0.5 SE) |
| 40.8M | graded-1 | **+1.2 SE** | -13.9 +/- 33.4 (-0.4 SE) |

Aligned bias is positive in **4 of 4** comparisons and never reaches 2 SE.
Delivered dF1 is **negative in 4 of 4**. Graded reward buys a little more bias
magnitude and arrives with slightly less of it — which is `aligned x gain`'s
pattern for the third time. More seeds would only sharpen a null on the outcome,
so the lead closes rather than continues.

**And the old gate's instability is now visible directly.** Binary's own phase-2
separation reads -0.07 +/- 0.11 at 27.2M and **+0.13 +/- 0.09 at 40.8M** — the
same arm, the same creatures, a statistic that moved by 2 SE because the budget
changed. That is what a rate-confounded statistic does, and it is a cleaner
demonstration of the defect than the argument that caught it.

**So the floor is the rule's.** Node perturbation's variance floor is set by the
perturbation noise the estimator injects, not by the resolution of the signal it
reads — which is what Hiratani's analysis says, and handing the estimator the
magnitude it was discarding does not move it.

**The magnitude question is now closed on six routes**, each priced rather than
argued: more trials saturate near 137 Hz; more rate grows `outside` 3x and halves
`gain`; a selectivity mask buys nothing; the commitment brake moves it 1.07x where
6.2x is needed; the slow store moves it 12.5x the wrong way and is analytically
bounded from ever helping; and a graded criterion does not escape the floor. What
is left is not a knob on the bias, and not the information in the reward. It is a
different **place to put** what the bias has already learned.

### `bankprobe` — consolidation refused, and one scare about the error bars stood down

Andalman & Fee's songbird banks its AFP bias into the motor pathway daily, turning
a bounded store into an unbounded rate. `boundprobe` made that look like the right
medicine: the learned bias stops growing, so n bouts should hold n times one bout.

**I closed it analytically and the check refuted me inside the hour.** The argument
was three lines — the larynx reads `live + bank`, consolidation leaves that sum
exactly unchanged, so behaviour and the drift written next are unchanged, so
banking is a no-op. At 6 seeds it came back **-2.8 SE**, with banking making the
aligned bias *worse*. Something was wrong, and it took three controls to find out
what.

| arm | what it does | delivered aligned vs `hold` |
|---|---|---|
| `bank-0` | exercises the call, moves nothing | **+0.000000 +/- 0.000000** |
| `bank-eps` | moves 1e-6 — about one ULP of perturbation | +0.000158 +/- 0.000328 (0.5 SE) |
| `bank-4` | banks four times | -0.003770 +/- 0.003183 (-1.2 SE) |
| `bank-1` | banks sixteen times | -0.003763 +/- 0.003483 (-1.1 SE) |

**`bank-0` is byte-exact, so the call path is clean** and the effect is genuinely
in the split, not in my plumbing.

**`bank-eps` was the one that mattered, and it answered a question much bigger
than consolidation.** This creature is deterministic but chaotic — `verify` gets
bit-identical reruns only because everything is bit-identical. Banking changes
whether a value accumulates as `round(T + u)` in one place or `T + round(0 + u)`
across two, which differs by about one ULP, and one ULP can decorrelate a chaotic
trajectory. If that were the story, then "same seed, one change" would not be a
paired contrast at all — it would be two independent draws — and **every SE in
this project that leans on pairing by seed would be too small.** `bank-eps` moves
the answer by 0.00016 where real banking moves it by 0.0038, **24x more**. So
rounding-scale decorrelation is real but small, pairing still buys its variance
reduction, and the scare stands down. Worth the arm.

**And the effect shrank with the sample, exactly as this project has been caught
by before:**

| | `bank-4` | `bank-1` |
|---|---|---|
| n = 6 | -0.0109 (**-2.8 SE**) | -0.0101 (-1.9 SE) |
| n = 12 | -0.0038 (-1.2 SE) | -0.0038 (-1.1 SE) |

The effect size fell 2.9x when the sample doubled. The -2.8 SE was small-n, which
is the `smoothing-sweep` lesson (+0.24 at n=3 became +0.03 at n=6) arriving again
at twice the sample size.

**Where that leaves it. Consolidation is refused on the outcome, and the mechanism
stays open.** The split does change the dynamics — `bank-eps` rules out rounding
as the cause — but nothing in this genome reads the live table's magnitude (the
commitment brake and the slow store both ship off, and the clamp never binds at a
pinned share of 0.000), so *why* it changes them is unexplained. It does not
matter for the decision: banking is neutral at best and mildly negative at worst
in 4 of 4 measurements across two sample sizes, and it is never the accumulator
the songbird story promised. **A mechanism whose best case is "no worse" does not
get built**, so this closes on the number and the anomaly is logged rather than
chased.

**What the anomaly is worth, if anyone returns to it.** A 4x sample would settle
whether the -1.2 SE is real. The interesting version of the question is not
consolidation at all — it is that splitting one plastic quantity into a plastic
part and a frozen part measurably changes what reward can learn, with nothing in
the code reading the split. That is either a bug worth finding or a fact about
this estimator worth knowing.

### The verdict on naming: DIRECTIONAL naming is MET, absolute naming has a measured ceiling

This thread has been written as an open question for long enough that the answer
got lost inside it. Both halves are settled, and they settle differently.

**MET — the creature says the right word for what it heard, directionally.** On
the axis measure, which asks only whether the voice moved toward the target it was
taught for the word it just heard:

| | family three | family two | pooled |
|---|---|---|---|
| **`ema`** (the creature's own index) | **0.824** | **0.738** | — |
| `off` (echo only) | 0.503 | 0.561 | **+0.276 +/- 0.036, 7.6 SE** |
| `ema-rnd` (matched marginal) | 0.406 | 0.593 | **+0.244 +/- 0.041, 6.0 SE** |

Both loopholes that make a fitted readout unusable are closed **by construction,
not by argument**. Echoing cannot pass it: `off` reads 0.503 while the *same*
utterances are 0.610 discriminable, so M1b's echo makes the words tell apart
without moving the voice the right way. Arbitrary-but-consistent mapping cannot
pass it either: `ema-rnd`'s index tracks the word — it is read off the ear, which
hears the word whatever the target is — and it lands at **0.406**, below chance.

And it partly *arrives*, not just points: on the strict nearest-of-two-actual-
targets score with nothing fitted, 0.670 and 0.604 against 0.495-0.518 for both
controls, pooled at 5.9 and 7.4 SE. A prediction of mine was wrong in the generous
direction there — I expected chance on every arm.

**NOT MET, and now with a mechanism rather than an excuse — absolute naming.**
Delivered dF1 tops out near **137 Hz** where roughly 230 Hz is needed for
`nearest` to cross the midpoint on absolute targets. That is not a shortfall
waiting on compute:

- The readout is fine. `ctxbias`'s oracle reaches 236 Hz through it, and a
  saturating transfer fitted to four learned points plus that oracle gives an
  asymptote of **234 Hz** — two routes, one number, neither fitted to the other.
- The learned bias is simply **3.8x too small**, and `boundprobe` says why:
  learning has two phases, and after ~4900 trials the useful and useless
  directions grow at **identical** exponents (0.290 vs 0.286) with `gain` frozen.
  The table keeps getting bigger and stops getting better. That is node
  perturbation on its variance floor.
- **Seven routes to a bigger bias are priced and closed**: more trials saturate;
  more rate grows `outside` 3x and halves `gain`; a selectivity mask buys nothing;
  the commitment brake moves it 1.07x against 6.2x needed; the slow store moves it
  12.5x the *wrong* way and is analytically bounded from ever helping; a graded
  criterion does not escape the floor; and daily consolidation is neutral at best.

**So the honest milestone is the axis measure at 0.82, and the honest limit is
137 Hz.** Those are different claims about different quantities and both are
measured. What is NOT closed is the parameterisation: every one of those seven
routes tried to make the bias *bigger*, and none tried to make it *smaller in
parameters* — which is the one thing Werfel, Xie and Seung's scaling law actually
prescribes, and the same law that licensed v51's drop from ~4000 synaptic
parameters to 252 and produced the first conditional effect on the voice.

### `ctxgain` — 18 parameters match 252, and the ceiling is not variance

The eighth route, and the only one that did not try to make the bias bigger. DNA
v54 mode 1 replaces the 252-entry per-neuron table with **one gain per articulator
group per context — 18 parameters** — on the centred position ramp inside the
group. Same perturbations, same reward, same cash-in; only the projection differs.
Werfel, Xie and Seung's scaling law is already load-bearing here, since it is the
argument that took v51 from ~4000 synaptic parameters to 252 and produced the first
conditional effect on this creature's voice. This applies it once more.

| arm | dF1 (Hz) | aligned (F1) | outside | gain |
|---|---|---|---|---|
| `table` (252 params) | 112.7 +/- 9.8 | 0.04977 | 0.05481 | 3.33 |
| `table-rnd` | 31.3 +/- 6.3 | 0.01447 | 0.06229 | 0.90 |
| **`gains` (18 params)** | **108.6 +/- 18.6** | 0.04541 | **0.02162** | **7.47** |
| `gains-rnd` | 25.9 +/- 13.4 | 0.01513 | 0.01760 | 3.78 |

    PAIRED  gains - table        -4.1 +/- 21.2  (-0.2 SE)   <- the gate
    PAIRED  gains - gains-rnd   +82.7 +/- 20.2  (+4.1 SE)   mode 1 learns
    PAIRED  table - table-rnd   +81.4 +/- 12.2  (+6.7 SE)   mode 0 still does

**The parameterisation did exactly what it was designed to do.** `outside` falls
**2.54x** and `gain` rises **2.24x**, to 7.47 against a structureless table's 1.0.
Fourteen times fewer parameters write two and a half times less into directions a
centroid readout cannot see.

**And the thing that was supposed to buy did not move.** `aligned` reads 0.0454
against 0.0498 — 0.91x, not the predicted 3.74x — and delivered dF1 is 108.6
against 112.7. Dead even at **-0.2 SE**. The prediction of 186 Hz is refuted, and
not narrowly.

**That is a direct refutation of the variance-floor explanation, and it is the
real result here.** `boundprobe`'s phase 2 — aligned and outside growing at
identical exponents with the shape frozen — was read as node perturbation sitting
on its variance floor, following Hiratani. If diffusion set the ceiling, then
cutting diffusion 2.54x had to raise the aligned bias. **It did not raise it at
all.** The limit was never noise.

**So the account has to change, and `boundprobe`'s own second half already
supplies it.** Praise is `e < baseline`, an EMA of the creature's *own* recent
error, so the expected drift is proportional to the **rate of improvement** rather
than to the remaining error. When improvement stalls the drift goes to zero on its
own, however far the voice still is from the target — and it stalls because
`dF1 ~ aligned^0.61` means each further unit of bias buys less error reduction.
The ongoing isotropic growth in phase 2 is diffusion, and `ctxgain` shows that
diffusion is a *passenger*: remove 2.54x of it and the aligned level is unchanged,
because the aligned level was set by a drift that had already stopped.

That also retires the puzzle `baseprobe` left behind. A graded criterion did not
help because more information about a gradient that is zero is still zero.

**Nine routes are now closed**, and the last one closes the mechanism as well as
the option: more trials, more rate, a selectivity mask, the commitment brake, the
slow store, a graded criterion, daily consolidation, and now the parameter count —
which additionally proves the ceiling is **drift-limited, not variance-limited**.

**One thing worth keeping rather than discarding.** 18 parameters deliver what 252
deliver, on the same rule and the same creatures, while writing 2.54x less into
useless directions. That is not a win on naming, but it is a 14x cheaper
parameterisation at no measured cost, and Werfel's law predicts it should also be
*faster* to learn. Both arms are saturated at 13.6M so this run cannot see a speed
difference; a short-budget comparison would, and it is the cheapest open question
left on this thread.

### `interleave` — replay as shipped does nothing, and what fixes it is not interleaving

`retain` measured a conflicting second lesson wiping a taught sound to 0.22.
McClelland's complementary learning systems says the fix is **interleaving**:
replay the old item alongside the new one so the slow store does not overwrite
it. The replay machinery has shipped since M4, and DNA v13 already refuted the
other classical route — this creature's episodic module does not pattern-separate.

Two things had to hold, and they were run as separate arms rather than one bundle:
the buffer must still contain lesson A (`frozen`, an oracle, because reward
selection cannot keep it — A stops being rewarded the moment B starts), and replay
must actually reinforce something (`credit`, DNA v55).

| arm | err taught | err after | retention |
|---|---|---|---|
| `quiet` (no conflict) | 0.8282 | 0.8035 | **1.15 +/- 0.04** |
| `relearn` | 0.8282 | 0.9559 | **0.19 +/- 0.11** |
| `relearn-noreplay` | 0.8426 | 0.9610 | 0.22 +/- 0.10 |
| `frozen` | 0.8282 | 0.9682 | 0.14 +/- 0.12 |
| **`credit`** | **0.8036** | **0.9181** | **0.38 +/- 0.08** |
| `both` | 0.8036 | 0.9324 | 0.33 +/- 0.10 |
| `both-32` | 0.8234 | 0.9388 | 0.30 +/- 0.13 |

    PAIRED vs relearn        retention gain        err-after improvement
    relearn-noreplay      +0.028 (+0.3 SE)        -0.0051 (-0.4 SE)
    frozen                -0.056 (-1.1 SE)        -0.0123 (-1.3 SE)
    credit                +0.192 (+2.0 SE)        +0.0378 (+2.7 SE)   <- BOTH
    both                  +0.137 (+0.9 SE)        +0.0235 (+1.0 SE)
    both-32               +0.103 (+0.8 SE)        +0.0172 (+0.8 SE)

The baseline reproduces `retain` exactly — 0.19 against its 0.22 — so the protocol
replicates before anything is read off it.

**Replay as shipped does NOTHING, and that confirms a derivation rather than
merely failing.** Turning replay off entirely is indistinguishable from leaving it
on: +0.3 SE and -0.4 SE. The prediction was that it must be, because the cash-in
is `u = step * perturb_[i]` and during sleep `perturb_[i]` is fresh noise, so
`E[u] = step * E[perturb] = 0`. A stored scalar paid against unrelated noise
rehearses the cue and reinforces nothing. Six years of this buffer existing, and
switching it off costs zero.

**MY PREDICTION WAS REFUTED, and precisely.** I predicted `frozen` would not
rescue (right), `credit` alone would not rescue because it merely reproduces the
new lesson's updates (**wrong — it is the only arm that passes**), and `both`
would rescue (**wrong — it is weaker than `credit` alone**).

**And `err taught` says why, which is the column that makes this interpretable.**
It measures the teaching phase, before any conflict exists. `frozen` reads 0.8282,
identical to `relearn`, because the freeze only starts when teaching ends — so the
oracle did exactly what it was supposed to and nothing else. `credit` reads
**0.8036**: with the exploration stored, replay during the teaching phase makes
the creature **learn the lesson better in the first place**. Its advantage
afterwards is a stronger lesson going in, not protection during the conflict.

**So the CLS hypothesis is refused and a different mechanism is what worked.**
Interleaving — holding the buffer on the old lesson and rehearsing it while the
new one is taught — is the arm that *hurts* (-1.1 SE), and adding it to `credit`
costs 0.05 of retention. That is a perfect interleaving oracle, so no selection
rule built on this replay would do better.

**One conjecture about why freezing hurts, stated as a conjecture.** A frozen
buffer replays the same eight perturbation vectors over and over. With the
exploration stored, that re-applies eight fixed random directions repeatedly,
which is closer to overfitting eight samples than to rehearsing a lesson. A
free-running buffer at least keeps drawing fresh high-reward moments. `both-32`
is consistent with this — a bigger frozen buffer is slightly better than a small
one — but it is 0.8 SE and settles nothing.

**The metric trap is inverted here, which makes the result harder rather than
easier.** Retention is `(before - after)/(before - taught)`, so `credit`'s lower
`err taught` gives it a LARGER denominator and *deflates* its retention. It gained
+0.192 anyway, and `err after` improved by +0.0378 independently of the ratio. The
two columns agree, which is what the gate required.

**What this does not yet establish.** `credit` passes at 2.0 and 2.7 SE with nine
seeds. This project has watched -2.8 SE become -1.2 SE when the sample doubled,
today. It needs replication at a larger n before anything is built on it, and the
mechanism claim — that the gain is consolidation during teaching rather than
protection during conflict — predicts something testable and cheap: **`credit`
should help the `quiet` arm too**, where there is no conflict at all to protect
against.

### The replication, and the buffer is full of the WRONG lesson

`credit` was the only arm that passed at nine seeds, at 2.0 and 2.7 SE. This
project watched a -2.8 SE become -1.2 SE when a sample doubled on the same day, so
it was re-run at eighteen with two settled questions dropped to pay for the seeds.

| | n = 9 | n = 18 |
|---|---|---|
| retention gain | +0.192 (2.0 SE) | **+0.174 (2.5 SE)** |
| err-after gain | +0.0378 (2.7 SE) | **+0.0279 (2.6 SE)** |

The effect shrank slightly and the significance held, which is what a real effect
does when n doubles and is not what today's other 2.8 SE did. **Storing the
exploration is what makes replay work**, and it survives replication.

**MY MECHANISM ACCOUNT IS REFUTED.** I claimed the benefit was a stronger lesson
going in rather than protection during the conflict, and pre-registered the test
that follows from it: then it must help the arm with no conflict at all. It does
not — `quiet-credit` vs `quiet` reads **-0.7 SE** on retention and **+0.2 SE** on
err after.

| arm | err taught | err after | change |
|---|---|---|---|
| `quiet` | 0.8443 | 0.8166 | -0.0277 |
| `quiet-credit` | 0.8333 | 0.8134 | -0.0199 |
| `relearn` | 0.8443 | 0.9768 | **+0.1325** |
| `credit` | 0.8333 | 0.9489 | **+0.1156** |

The head start is identical in both protocols — 0.0110 of `err taught`. Without a
conflict it **shrinks** to 0.0032 by the end; with one it **grows** to 0.0279. And
the damage itself is smaller: the taught sound degrades by 0.1156 instead of
0.1325, **13% less**. So `credit` is not merely starting ahead. Under attack it
loses less.

**And the buffer is full of the wrong lesson, which was the other candidate.**
`episodes_recorded()` sampled at the phase boundaries gives the fraction of the
ring overwritten since teaching ended. It reads **1.00** on every taught arm: by
the time the conflict is underway, replay is rehearsing lesson B and nothing of
lesson A remains. So `credit` is not accidental interleaving with a naturally
mixed buffer either.

**That strengthens the earlier refusal rather than qualifying it.** `frozen` was
the fair test of interleaving — it is the only arm whose buffer actually held the
old lesson — and it failed at -1.1 SE. `credit`'s buffer holds the *new* lesson
and it works. Interleaving is refused, and what replaced it is not a variant of it.

**So the position is: a replicated effect with no established mechanism, and two
explanations refuted.** Replaying the *new* lesson's own successful explorations
during sleep makes the *old* lesson survive the new one better. The obvious next
question is whether sleep is absorbing some of the new learning — if `credit`
learns B better *and* damages A less, then consolidating B at night spares the
shared parameters by day, and that is measurable with the target the gap phase
already scores against.

### Absorption or stubbornness — and the answer is not the flattering one

Everything measured so far is scored against lesson A, which is correct for
retention and completely blind to *why* `credit` protects it. Two stories produce
identical A-scored numbers:

- **Absorption.** Sleep does some of the new learning, so fewer awake updates are
  needed and the shared parameters are spared. Predicts `credit` learns B
  **better** while damaging A less — a free gain.
- **Stubbornness.** `credit` resists the new lesson, so the old one survives
  because the new one lands less hard. Predicts `credit` learns B **worse** — a
  trade, and one that caps how much can ever be taught second.

Scoring against B's own target over the last third of the gap, paired on seed:

    credit - relearn   -0.0192 +/- 0.0104  (-1.9 SE)     positive = learned B BETTER

**Absorption is refuted directionally.** It predicted a positive number and the
measurement is negative at 1.9 SE. The free-gain story is gone.

**Stubbornness is suggested and not established**, and the pre-registered gate
correctly printed "neither at 2 SE" rather than rounding 1.9 up. But this is not a
null sitting at zero — it is 1.9 SE on the stubbornness side, with the only rival
account pointing the other way.

**So at face value the effect is a TRADE rather than a free gain:**

| | measured | status |
|---|---|---|
| gained on the OLD lesson | +0.0279 of `err after` | 2.6 SE, established |
| cost on the NEW lesson | -0.0192 of `err b` | 1.9 SE, suggestive |

Net +0.0087 in A's favour, but bought rather than free. **That changes what this
finding is worth.** A memory system that protects old lessons by learning new ones
worse has a ceiling on what can ever be taught second, and this creature's whole
naming problem is a second lesson landing on top of a first. It would be a poor
foundation to build a memory system on without knowing which it is.

**Three mechanism accounts have now been put to this effect and none survives
intact** — a stronger lesson going in (refuted at -0.7 SE), accidental
interleaving (refuted at `buf B` = 1.00), and absorption (refuted directionally).
The effect itself has replicated twice. What is missing is not evidence that it is
real; it is an explanation, and the one still standing predicts a cost this run
can see but cannot resolve.

### It does not survive 36 seeds, and "replicated" was my word too early

| n | retention gain | err-after gain |
|---|---|---|
| 9 | +0.192 (2.0 SE) | +0.0378 (2.7 SE) |
| 18 | +0.174 (2.5 SE) | +0.0279 (2.6 SE) |
| **36** | **+0.106 (1.6 SE)** | **+0.0189 (1.8 SE)** |

**The effect size shrinks monotonically and roughly halves from 9 seeds to 36, on
both measures.** A real effect holds its size and gains SE as the sample grows;
this did the opposite, and at 36 seeds it fails the pre-registered gate on both
columns.

**So the claim is retracted.** At n=18 this page said the effect "shrank slightly
and the significance held, which is what a real effect does when the sample
doubles." The shrink was not slight and it did not stop — doubling again took it
under the bar. **Two underpowered runs agreeing is not replication.**

**This is the third time.** `smoothing-sweep` went +0.24 at n=3 to +0.03 at n=6.
`bankprobe` went -2.8 SE at n=6 to -1.2 SE at n=12. Now `interleave` goes 2.0/2.7
SE at n=9 to 1.6/1.8 at n=36. The pattern is specific enough to act on: **a 2-3 SE
result at n <= 18 in this project is not a finding, it is a hypothesis**, and the
right response is more seeds before more mechanism.

**What still stands, because it was measured against its own control rather than
against a difference between arms:**

- **Replay as shipped does nothing.** Switching it off entirely costs +0.3 SE and
  -0.4 SE. That is a direct test of the `E[u] = 0` derivation and does not depend
  on `credit` at all.
- **Interleaving is refused.** `frozen` — the only arm whose buffer actually held
  the old lesson — failed at -1.1 SE, and `buf B` reads 1.00, so the free-running
  arms were never doing it accidentally.

**What does not stand:** that storing the exploration rescues a lesson from a
conflicting one. It may still be true at some size — 1.6 and 1.8 SE are not zero —
but it is not established, and three mechanism accounts were built on top of an
effect that has now failed its own gate.

**And a scope bug cost this run its second question.** The `err_b` block, which
was the entire reason for the previous run, had been nested inside the
`quiet-credit` guard — so dropping that arm silently skipped the measurement.
Nothing warned; the section simply did not print. It is now at function scope, and
the failure message no longer names arms it cannot know are present.

### `ctxretain` — contexts do not rescue it, and one arm shows why the gate has two columns

The 2x2: give the two lessons different cues, crossed with DNA v51's context table
to file them under. 36 seeds from the start.

| arm | err taught | err after | retention |
|---|---|---|---|
| `baseline` | 0.8615 | 0.9629 | 0.30 |
| `cue` | 0.8615 | 0.9529 | 0.34 |
| `ctx` | **0.9138** | 0.9897 | **0.60** |
| `cue+ctx` | **0.9138** | 0.9851 | 0.41 |

    PAIRED vs baseline    retention gain        err-after improvement
    cue                 +0.043 (+0.5 SE)       +0.0100 (+1.4 SE)
    ctx                 +0.303 (+1.6 SE)       -0.0268 (-1.5 SE)
    cue+ctx             +0.113 (+0.6 SE)       -0.0221 (-1.4 SE)

**No cell passes.** The interaction the whole design rested on is not there.

**And `ctx` is a textbook ratio artefact, caught by the gate.** It reads +0.303 on
retention at 1.6 SE — the best-looking number in the table — while its `err after`
is **-0.0268, i.e. WORSE**. The two columns disagree in sign. The reason is the
`err taught` column: turning contexts on made lesson A **harder to learn**, 0.8615
to 0.9138, and retention is `(before - after)/(before - taught)`, so a bigger
`err taught` shrinks the denominator and inflates the ratio. `ctx` and `cue+ctx`
have *identical* `err taught` because their teaching phases are identical, which is
the consistency check that pins the cost on the context field rather than on noise.

**That is the second ratio artefact this gate has caught tonight**, after
`credit`'s. Requiring retention and absolute `err after` to agree is doing real
work; either column alone would have produced a false positive here.

**Masse, Grant & Freedman predicted this**, and the prediction was written down
before the run finished. Context gating alone reads 61.4% across 100 tasks in the
paper that introduced it, against 95.4% when combined with synaptic stabilisation:
*"XdG alone does not support continual learning."* This run is gating alone. So the
right reading is **not** "contexts do not work" — it is that gating without
stabilisation is the insufficient half, exactly as the literature says, and here it
is worse than insufficient because the context machinery also costs the teaching
phase 0.05 of error.

**The stabilisation half already ships and has never been tested for this job.**
`meta_commit` gates plasticity by how far a neuron has already moved from where it
started, which is the EWC/SI idea. `brake-sweep` measured it as a decisive null on
the **magnitude** question — whether it grows the learned bias — and that is not
what the mechanism is for. Protecting an old lesson from a new one is, and nobody
has run it against `retain`.

### The index never separated the lessons, so the context result is VOID

The instrument added to rule this out found it. `slot0 teach/gap` reads
**0.23 / 0.23**.

The creature hears "ball" for the whole teaching phase and "boot" for the whole
gap. If the ear's rate-EMA index tracked the *word*, those two numbers would
differ. They are identical to two decimals. And 0.23 is not 0 or 1 either, so the
index **is** switching slots — 23% of trials to slot 0 — it is just switching on
something that varies *within* a trial rather than on which word is playing. In the
naming protocol that is harmless, because the word alternates every trial and the
reward window aligns with it. In this protocol, where one word plays for tens of
thousands of trials and then another does, it is useless.

**So the context arms were never a test of contexts.** The table was live — it cost
`err taught` 0.8615 -> 0.9138 by splitting one lesson across two slots — and the
index filed both lessons the same way, so there was nothing to separate.

**The sentence "contexts do not rescue it" in the section above is therefore
retracted.** What that run measured was the cost of carrying a table nobody
indexed, which is a fact about `retain`'s protocol and not about DNA v51. The 2x2
itself was sound; the cue reached the ear, and the arms differed. What failed is
the assumption that a cue reaching the ear becomes a context index — and nothing
printed before this column would have shown it.

**What stands, because it involves no contexts at all:** the commitment brake is a
clean null on the outcome.

| arm | retention gain | err-after improvement |
|---|---|---|
| `brake-0.5` | -0.038 (-0.6 SE) | -0.0035 (-0.3 SE) |
| `brake-1.0` | +0.069 (+0.9 SE) | **+0.0002 (+0.0 SE)** |

`brake-1.0` slows learning as predicted — `err taught` 0.8757 against 0.8615 — so
its retention gain is partly the denominator effect for the third time tonight. Its
absolute error after the conflict is **0.0002** better. Not a trade, not an
artefact: nothing. **`meta_commit` does not protect an old lesson from a new one**,
and that is now measured on the question the mechanism is actually for rather than
on the magnitude question `brake-sweep` asked.

**So one half of Masse's prescription is closed here and the other is untested.**
Synaptic stabilisation, in the form this creature has it, does not help. Context
gating has still not been tried, because making a cue audible is not the same as
making it an index — and `ctxsrc` said this two weeks ago in different words: the
word is at 1.000 while it plays and at chance when reward lands.

### The replay buffer has been decorative since M4

Stated plainly because it is the one solid structural finding of the memory work,
and because the code asserted the opposite for a month.

`drive_replay()` re-presents a stored cue and pays out the stored reward at the end
of the episode. Its comment claimed this was *"the same shape as the waking loop,
which is what makes this consolidation of the original episode rather than a new
and different lesson."* **For a value that is true. For a policy learned by node
perturbation it is not**, and the arithmetic is one line: the cash-in is
`u = step * perturb_[i]`, and during a sleep bout `perturb_[i]` is whatever noise
is live then — uncorrelated with the exploration that earned the reward being paid.
So `E[u] = step * E[perturb] = 0`. Replay rehearses the cue and reinforces nothing:
zero drift, non-zero variance.

**Measured, not merely derived.** `interleave` ran an arm with replay switched off
entirely against one with it on: **+0.3 SE on retention and -0.4 SE on absolute
error.** The buffer has shipped since M4 and turning it off costs nothing.

Three consequences, and the third is the one that matters:

- **Anything built on this buffer inherits it.** The `frozen` and `both` arms of
  `interleave` were testing interleaving *through* a mechanism that reinforces
  nothing, which is part of why they failed.
- **`replay_credit = 1` is the fix** — restore the stored exploration before paying
  out, so the cash-in credits what the creature actually did. It makes the original
  comment true again. But v55's own retention claim did **not** survive 36 seeds,
  so the mechanism is right and the benefit is unproven.
- **The comment is now corrected in `brain.cpp` and in the genome**, rather than
  left for someone to rediscover. A wrong comment on a mechanism that silently does
  nothing is worse than no comment: it is the reason nobody checked for a month.

### A perfect index recovers 15% of the damage — partial protection, as predicted

`ctxretain`'s previous generation never tested gating: `slot0 teach/gap` read
0.23/0.23, so the ear-EMA index filed both lessons the same way. This writes the
context module directly — slice 0 while lesson A is taught, slice 1 while B is —
and the slot column confirms it: **1.00 / 0.00**.

The control is `ctx-same`, which carries the identical table at identical cost and
writes slice 0 in both phases. `ctx-same` and `ctx-oracle` have identical
`err taught` (1.0344), so the difference between them is the index's *information*
and nothing else.

| arm | err taught | err after | slot0 teach/gap |
|---|---|---|---|
| `quiet` (no conflict, earlier run) | 0.8404 | **0.8116** | — |
| `baseline` | 0.8615 | 0.9629 | — |
| `ctx-same` (table, no information) | 1.0344 | **1.0393** | 1.00 / 1.00 |
| `ctx-oracle` (table + perfect index) | 1.0344 | **0.9404** | 1.00 / 0.00 |

**The retention column is degenerate here and must not be read.** Retention is
`(before - after)/(before - taught)`, and the context arms' `err taught` is 1.0344
against an `err before` of about 1.03 — the denominator is nearly zero. That is why
they print 6.12 +/- 2.74 and 8.16 +/- 7.24 against a baseline of 0.30. Those are not
large effects, they are divisions by almost nothing, and the table now suppresses
the ratio and prints the denominator instead when it collapses.

**On `err after`, which is absolute and has no denominator:**

- the conflict costs **0.1513** (0.8116 -> 0.9629)
- carrying a context table with an **uninformative** index costs a further
  **+0.0764** on top of that, at -6.9 SE. The machinery is expensive.
- a **perfect** index recovers **0.0225** against baseline, at +2.1 SE — about
  **15% of what the conflict destroys**.

**So gating helps, and only a little.** That is the outcome the temper set before
the run predicted: `ctxfour` found a perfect index at four words holds four
*distinctions* but not four *targets*, and this is the same shape — the index
carries which lesson it is, and most of the damage happens anyway. The `ctx-oracle`
vs `ctx-same` gap is large (+14.6 SE on `err after`) but most of it is the index
undoing the cost its own table imposes, not protection.

**And the gate refused on a knife-edge worth naming.** It requires 2 SE on both
columns; retention came in at **1.9893 SE** and printed as "+2.0 SE" because the
format rounded it. The verdict and the displayed number disagreed. The SE is now
printed to two decimals so a knife-edge reads as one.

**Where that leaves the standard account.** Stabilisation is a measured null
(`meta_commit`, +0.0002 on `err after`). Gating, given a perfect index, buys 15% of
the damage back and costs a great deal when the index is anything less than perfect.
Masse's combination — `ctx-orc+brake` — reads +1.1 SE and +1.4 SE, no better than
gating alone. The textbook decomposition applies here only weakly, and the honest
summary is that **catastrophic interference in this creature is mostly not a filing
problem**.

### `baseref` — "keep asking for more" is self-defeating, and that is structural

`boundprobe` located the 137 Hz ceiling as the drift dying, and named the cause:
praise is `e < baseline` with the bar an EMA of the creature's *own* recent error,
so expected drift tracks the **rate of improvement** rather than the remaining
error. Nine closed routes attacked the bias; `baseprobe` attacked the reward's
*resolution*. This attacked its **reference**.

| arm | dF1 (Hz) | aligned | gain | praise |
|---|---|---|---|---|
| `ema` (shipped) | **103.9 +/- 8.8** | 0.04581 | 3.09 | 0.546 |
| `slow` (bar lags 10x) | 44.6 +/- 8.1 | 0.03002 | 1.93 | 0.603 |
| `ratchet` (bar only tightens) | 40.0 +/- 6.8 | 0.01166 | 0.91 | **0.050** |
| `ema-rnd` (matched marginal) | 28.0 +/- 4.4 | 0.01475 | 0.87 | 0.505 |

    PAIRED vs ema    slow -59.2 (-5.94 SE)   ratchet -63.9 (-6.97 SE)

**The protocol is healthy**: `ema` beats its matched-marginal control by +75.9 Hz,
so the creature is genuinely learning and this is not a broken run.

**The ratchet is VOID, and it died the death written into the arm table before the
run.** Praise share **0.050** — 95% scold. A bar the creature can no longer beat is
a *constant* reward, which multiplies a zero-mean perturbation and gives zero
drift. `aligned` collapsed to 0.01166 against the shipped 0.04581 and `gain` to
0.91, which is **structureless**: it did not stall, it stopped learning. The
pre-registered praise-share guard refused the run rather than letting -63.9 Hz read
as "a demanding bar hurts."

**The `slow` arm is valid and it loses cleanly.** Praise 0.603 is healthy, and it
is **-59.2 Hz at -5.94 SE**. A bar that lags improvement is much worse than one
that tracks it.

**So the shipped EMA is not merely adequate, it is near-optimal on this axis** —
perturbing it in either direction costs about 60 Hz. And the reason is a conflict
that no tuning resolves:

> A bar must **track** the creature to keep praise informative at roughly 50%.
> A bar that tracks the creature **stops asking for more** the moment the creature
> stops improving. The two requirements are in direct opposition.

**That makes the dying drift a property of using a relative criterion at all, not
a tuning failure of this one.** It is the same shape as the slow store's
`ratio/(1+ratio) <= 0.5`: a bound rather than a budget.

**Which names the escape precisely, and it is not on the reference axis.** An
**absolute** criterion — praise proportional to how close the voice actually is to
the target, with no baseline subtracted — has no such conflict, because it keeps
asking for more without needing to track anything. `vocallearn` already hints that
it works in the unconditional case: reward cuts formant error +24 toward a FIXED
target. What it costs is that reward acquires a large constant component, and
`rpeprobe` measured the caregiver as 99.5% of the reward variance already. That is
the next test on this chain, and it is one arm.

### `poolbeta` — the readout was a constraint, and this one replicates

Nine mechanism routes attacked the bias. `baseprobe` attacked the reward's
resolution. `baseref` attacked its reference. All failed, and the algebra says all
of them had to: the drift is `-Cov(e, perturb_i)`, which factors through
`dF1/d(drive)`, and `dF1 ~ aligned^0.61` has a derivative falling as
`aligned^-0.39`. **The same curve that shrinks what arrives shrinks the gradient
that would build more.** Nothing had touched it, because `read_group` is a centroid
and `centroid-is-steerability` measured centroids as the only teachable readout
here.

DNA v56 sharpens the pooling to `sum(r^beta * p)/sum(r^beta)`, with 1.0 the shipped
centroid. Screened at 3.4M, confirmed at 36 seeds:

| arm | dF1 (Hz) | F1 scatter | aligned | gain |
|---|---|---|---|---|
| `b1.0` (shipped) | 63.5 +/- 4.4 | 61.5 | 0.01991 | 2.19 |
| `b1.0-rnd` | 24.9 +/- 2.8 | 39.7 | 0.00626 | 0.74 |
| **`b2.0`** | **83.2 +/- 5.8** | 80.0 | 0.01492 | 1.71 |
| `b2.0-rnd` | 26.4 +/- 3.7 | 48.2 | 0.00772 | 0.90 |

    EXCESS over its own control   b1.0 +38.6 (7.62 SE)   b2.0 +56.8 (10.13 SE)
    PAIRED difference             b2.0 - b1.0  +18.2 +/- 6.5  (+2.82 SE)

**It replicates, and it replicates the way a real effect does.** At 18 seeds the
paired difference read +18.7 +/- 9.29 (2.01 SE); at 36 it reads +18.2 +/- 6.50
(2.82 SE). The effect size is unchanged and the SE fell to 6.50 against the 6.57
that `1/sqrt(n)` predicts. Compare `credit`, which halved from n=9 to n=36 and
died — that is the contrast this project needed to see to trust anything in this
band.

**And the noise explanation dies on its own numbers.** Sharpening raises the F1
scatter in both arms, which is exactly why every beta carries its own
matched-marginal control:

- the **taught** arm gains **+19.7 Hz** (63.5 -> 83.2)
- its **control** gains **+1.5 Hz** (24.9 -> 26.4)

Thirteen times more. Pure scatter would have lifted both equally; it did not. The
creature is not producing a wider spread, it is **steering a wider range**.

**What is established and what is not.** Established: sharpening the pooling widens
the steerable range, at 2.82 SE, paired, replicated at two sample sizes. **Not**
established: the specific "more formant from less bias" framing — pooled it is
5576 against 3189 Hz per unit of aligned bias, 1.75x, but the paired per-creature
statistic is only +1.42 SE. The effect is real; that account of its mechanism is
suggestive.

**Note what it cost, because it is not free.** `gain` falls from 2.19 to 1.71 and
`aligned` from 0.01991 to 0.01492, so the bias table is *less* structured at
beta=2 and still delivers more. A hard argmax was expected to be untrainable, and
beta=3 was already worse than beta=2 on every column in the screen — so this is a
narrow optimum rather than a direction to push.

### The ceiling test says SPEED, not ceiling — and the previous section's headline is wrong

The confirmation run existed to ask one question: does sharpening move the
asymptote, or only the 3.4M value? It answers **only the 3.4M value.**

| budget | `b1.0` | `b2.0` | raw gap | paired excess difference |
|---|---|---|---|---|
| 3.4M | 63.5 | 83.2 | **+19.7** | **+18.2 +/- 6.5 (2.82 SE)** |
| 13.6M | 109.4 | 117.9 | +8.5 | +9.4 +/- 9.7 (**0.97 SE**) |

**The advantage halves as the budget grows and falls below significance.** That is
the signature of a speed advantage with a common asymptote: beta=2 gets there
sooner and does not get further.

**So the previous section's headline — "the first thing to move the naming
ceiling" — is wrong, and is retracted.** It moved the 3.4M value. The ceiling is
where it was. I wrote that headline after the 36-seed screen and before the
confirmation, which is exactly the order that makes a claim premature; the
confirmation was already queued for this reason and it did its job.

**What survives, and it is not nothing.** At 3.4M the effect is real and
replicated: 2.01 SE at n=18 and 2.82 SE at n=36, effect size stable, SE falling as
`1/sqrt(n)`. And it is genuinely steering rather than scatter — the taught arm
gained 19.7 Hz where its matched-marginal control gained 1.5. **Sharpening the
readout makes the creature learn faster. It does not let it learn further.**

**Which is a real answer to a question already on file.** `ctxgain` asked whether a
cheaper parameterisation buys convergence speed and could not see it, because both
its arms were saturated at 13.6M. This is that question answered from the other
end: an intervention that clearly buys speed shows up at 3.4M and has vanished by
13.6M. Any future speed claim in this project should be measured at a budget where
the arms are **not** saturated, and this pair of runs is the calibration for which
budgets those are.

**And the asymptote's cause is untouched.** `dF1 ~ aligned^0.61` gave 234 Hz, and
sharpening the pooling did not change where the curve ends up — only how fast it
gets there. The compression that sets the ceiling is not the smoothness of the
centroid.

### `stageprobe` — the compression is in the NEURON, not the pooling

Twelve routes failed against `dF1 ~ aligned^0.61` and not one of them knew which
stage of the pipeline produced it. The chain is four stages — `bias -> drive ->
rate -> centroid -> F1` — and only one can be the culprit. This injects a known
zero-mean ramp with `ctxbias`'s own oracle at six amplitudes and fits the local
slope of each stage separately. Read-only.

| k range | bias -> rate | rate -> centroid | centroid -> F1 |
|---|---|---|---|
| **0.25 -> 0.50** | **0.34** | 0.66 | 1.01 |
| 0.50 -> 1.00 | 0.46 | 0.56 | 1.01 |
| 1.00 -> 2.00 | 1.40 | 0.16 | 1.01 |
| 2.00 -> 4.00 | 1.19 | -0.01 | 1.01 |

**The learned bias lives in the first row.** `ctxscale` and `align-split` put
`aligned` at about 0.05, which is between k = 0.25 and k = 0.50 on this ladder.
There the compressive stage is **`bias -> rate` at 0.34**, against 0.66 for the
pooling.

**And that explains `poolbeta`.** Sharpening the pooling attacked `rate ->
centroid` — the stage that is *not* the bottleneck where reward operates — which is
exactly why it bought convergence speed and left the ceiling alone. It was aimed
one stage too late.

**It is not threshold saturation.** The pinned share is **0.000** at both of the
rungs that bracket the learned regime, and only reaches 0.357 at k = 4, a magnitude
only an oracle can produce. So the sublinearity at 0.34 is intrinsic to the
drive-to-rate transfer rather than a clamp being hit — which also retires the
`ipctx` pinned-share story as the explanation *at the magnitudes that matter*.

**Two ways this probe caught itself, and both earned their place.**

The self-check is `centroid -> F1`, which is a `lerp` and must measure 1.00. The
first version recomputed the centroid from a **one-tick snapshot** of `rate_fast`
and compared it against an equilibrated formant; the check read **0.74** and
refused the whole table. Reading the decoder's own smoothed `group_value_[2]` —
the quantity `target_f1` is actually a lerp of — puts it at 1.01.

The second was worse and would have been publishable. The first version fitted
**one exponent from the first rung to the last** and announced the compression was
in `rate -> centroid` at 0.21. That is the *high-k* regime: the centroid saturates
near its bound at 0.84 by k = 2 and drags the whole fit with it. Over the ladder
the answer **inverts**, and a single fit names the wrong stage. This project has
made that exact error before — three readings of one curve, the first two wrong —
so the table is per-interval and the regimes are visible.

**So there is a target for the first time.** Not "the larynx compresses" but: *the
drive-to-rate transfer is sublinear at 0.34 in the range reward can reach, with no
clamp involved.* What makes a spiking neuron's rate sublinear in drive at 5-6 Hz is
the next question, and the candidates are nameable — feedforward inhibition
scaling with the group's own drive is the obvious one, and would make this the
ninth appearance of the common-mode wall.

### The compression is intrinsic plasticity, and a per-neuron homeostat is the wrong shape

`stageprobe` put the compression in `bias -> rate` at 0.34. Every drive-dependent
inhibition on the larynx is already **off** in the genome — `norm_gain`,
`ffi_gain`, `lateral_gain` and `apical_threshold` are all 0.0 — so it is not any of
those. What is on is intrinsic plasticity, at `ip_wake_scale = 1.0` where most
modules run 0.25, which is v9's deliberate choice to hold the larynx.

Holding it off during wake:

| vocal `ip_wake` | `bias -> rate` | threshold | mean rate |
|---|---|---|---|
| 1.00 (shipped) | **0.34** | 1.025 -> 1.130 | 5.73 -> **5.18** |
| 0.00 | **0.91** | 1.000 -> 1.000 | 9.25 -> **16.29** |

**Doubling the drive with the homeostat off raises the rate 1.76x — essentially
proportional. With it on, the rate FALLS.** The homeostat is not damping the bias,
it is cancelling it and overshooting, entirely inside its range: the pinned share
is 0.000 at both of those rungs, so no clamp is involved.

**So the naming ceiling is the larynx regulating its own firing rate.** Twelve
routes failed upstream of that, and they had to: reward can write as much bias as
it likes and a homeostat downstream will absorb it.

**And the design point is sharper than "turn it down".** The readout is a centroid
— it reads the **tilt**, which neuron fires more across the group. A **per-neuron**
rate homeostat drives every neuron toward the *same* rate. It therefore does not
merely damp the bias; it actively **flattens the tilt**, which is the one thing the
centroid can see. A homeostat on the **group's mean** rate would hold the common
mode and leave the tilt alone.

That mechanism does not exist in this creature, and it is the first genuinely new
thing this thread has produced rather than another knob.

**Turning IP off is not the fix, and the record already says why.** DNA v50
relaxed the larynx's IP and learning got *worse* — `change` +16.6 to -2.3 on 3 of
3 seeds. A creature whose larynx does not regulate has a different problem. The
diagnosis licenses a *reshaped* homeostat, not an absent one.

**It also unifies nine walls.** Every "common-mode" result in this project —
pooling destroying the object code, the plateau gate attenuating instead of
selecting, the topographic tract measuring x1.00, `ipctx`'s saturation — is the
same shape: a signal that lives in *which* unit fires, meeting machinery that
regulates *how much* each unit fires. The ninth appearance is the one where the
mechanism is finally named rather than hit.

### DNA v57 — pooling keeps regulation, recovers half the transfer, and fails its own bar

`stageprobe` put the compression in intrinsic plasticity: `bias -> rate` runs at
0.34 with IP on and 0.91 with it off. The design point was that IP regulates each
neuron's **own** rate, so it flattens the tilt a centroid reads. v57 pools the rate
error over N equal slices of a module, so every neuron in a slice gets the same
step and the tilt inside it survives.

| homeostat | `bias -> rate` | threshold | mean rate |
|---|---|---|---|
| shipped (per neuron) | 0.34 | 1.025 -> 1.130 | 5.73 -> **5.18** |
| off (the diagnosis, not a candidate) | **0.91** | 1.000 -> 1.000 | 9.25 -> **16.29** |
| pooled x1 (module-wide) | 0.82 | 0.972 -> 1.012 | 10.14 -> 15.98 |
| **pooled x9** (`kVocalGroups`) | **0.51** | 1.124 -> 1.468 | 7.07 -> **8.90** |

**Regulation passes.** Pooled at the group the mean rate holds at 8.90 against the
shipped 5.18, where pooled module-wide ran to 15.98 — indistinguishable from
switching IP off. **The slice count is the design, exactly as predicted:** module-
wide dilutes the error ninefold because the larynx has nine articulator groups and
a bias steers one.

**The transfer bar fails.** 0.51 against the 0.59 the pre-registered gate required.
It recovers 0.17 of a possible 0.57. That bar was set before the run and moving it
now would be `verdict-fitted-to-data`, so **v57 does not pass.**

**And the threshold column says why the recovery is only partial.** Pooled x9
pushes back *three times harder* than the shipped rule — +0.344 of threshold
against +0.105. Per-neuron IP pushes each neuron in proportion to its **own** error,
so the high-firing ones get pushed most, which is what flattens the tilt. Pooled IP
pushes every neuron in a slice **equally**, preserving the tilt's *shape* — but the
slice's mean error is large, so it cuts the *amplitude* harder.

**Why the slice's mean error is large at all is rectification**, and that makes this
a trade rather than a knob set wrong. A zero-mean drive ramp does not produce a
zero-mean *rate* change, because a rate cannot go below zero. So any rate
homeostat, pooled or per-neuron, sees the mean rise and pushes back. A fully linear
transfer needs no rate regulation, and v50 measured what that costs: `change` +16.6
to -2.3.

**Two bugs of mine in this thread, and the second is the instructive one.** Pooling
module-wide was a design error I had reasoned past and then simplified back into.
Then the four-arm table shipped with a three-entry `ip_scales`, so arm 3 read one
past the end, got 0.0, and ran with IP **off** — printing numbers byte-identical to
the `off` arm, which is what gave it away. Two arms agreeing to three decimals
across four columns is never a coincidence. `ArmLiveness` checks exactly that and
`stageprobe` was the one experiment built without it; it is wired in now, and the
array lengths are a `static_assert` rather than a convention.

**What is left is the outcome, and it is a separate pre-registered question.** The
transfer bar was a proxy. Whether 0.34 -> 0.51 on the compressive stage buys
delivered dF1 is the thing that matters, and unlike the twelve closed routes this
one has a measured mechanism acting on the measured bottleneck. That deserves its
own gate, set before its own run.

### `ippool` — relieving the bottleneck does not buy dF1, and the reason is a real trade

`stageprobe` located the compression in intrinsic plasticity and DNA v57 relieved
half of it while keeping regulation. This asks the only question that matters:
does the delivered formant follow?

| arm | dF1 (Hz) | aligned | gain | larynx rate |
|---|---|---|---|---|
| `ip0` (shipped) | **103.9 +/- 8.8** | 0.04581 | 3.09 | 4.62 |
| `ip0-rnd` | 28.0 +/- 4.4 | 0.01475 | 0.87 | 4.74 |
| `ip9` (v57) | 87.5 +/- 13.8 | **0.02373** | **1.71** | 5.50 |
| `ip9-rnd` | 27.2 +/- 5.3 | 0.01393 | 0.84 | 5.30 |

    EXCESS DIFFERENCE, ip9 - ip0, paired on seed:  -15.6 +/- 17.5  (-0.90 SE), 8/18
    larynx rate drift 19%, well inside the 50% bar -- regulation held

**No.** And the point estimate is *negative*: v57 is slightly worse, not slightly
better.

**I predicted a small positive that fails the bar. It is a small negative, and the
`aligned` column says why — which I did not anticipate.** Pooled IP nearly halves
the learned bias, 0.04581 to 0.02373, and cuts `gain` from 3.09 to 1.71. So it
improves the **transfer** and degrades the **estimator** at the same time, and the
two offset.

**The mechanism is the same number that made it work.** `stageprobe` measured pooled
x9 pushing the threshold back *three times harder* than the per-neuron rule (+0.344
against +0.105), because a slice's mean error is large and every neuron in it takes
that step. A harder-pushed group is a noisier one to estimate a gradient in, and
`gain` falling to 1.71 is exactly that: the table is less structured, so more of
what reward writes is diffusion rather than drift. **The thing that preserves the
tilt also degrades the signal that builds it.**

**So the ceiling is a trade, not an unfound knob.** Three facts now hold together:

- the compression is **necessary** — it is intrinsic plasticity, measured at
  `bias -> rate` 0.34 against 0.91 with the homeostat off
- relieving **half** of it buys nothing, because the same intervention costs the
  estimator what it gains in delivery
- relieving **all** of it needs no rate regulation at all, and v50 priced that
  directly: `change` +16.6 to -2.3

**The larynx can be steerable or self-regulating and it has to be both.** That is a
different statement from "we have not found the right mechanism", and it is the
thirteenth route closed — the first one aimed at the measured bottleneck rather
than upstream of it, which is why its failure is informative rather than another
null.

### `ipoff` — the trade is real, and v50 was right for the wrong reasons

The trade rested on three legs and only two were solid. IP is the compression
(`bias -> rate` 0.34 on, 0.91 off) — solid. Relieving half buys nothing (`ippool`,
-0.90 SE) — solid. **"Relieving all of it costs learning" rested entirely on v50,
which was confounded**: its inhibitory-plasticity arm also charged the exploratory
pathway (+34.0 to +15.4 with the tract silent), so *the IP-relaxed larynx learns
worse* was never separated from *ISP charged for arriving*.

This relaxes the larynx's IP directly — no ISP, nothing else moved — and measures
delivered dF1 against its own matched-marginal control.

| arm | dF1 (Hz) | aligned | gain | excess over own control |
|---|---|---|---|---|
| `ip0` (shipped) | **103.9 +/- 8.8** | 0.04581 | 3.09 | **+75.9** |
| `ipoff` | **17.7 +/- 3.8** | 0.01585 | **1.00** | **+2.8** |

    EXCESS DIFFERENCE, ipoff - ip0, paired on seed:  -73.1 +/- 9.9  (-7.42 SE), 1/18

**Turning the homeostat off does not merely fail to buy dF1 — it destroys vocal
learning.** dF1 falls to a sixth, the learned bias to a third, and `gain` to
**1.00**, which is a structureless table. The excess over its own control collapses
from +75.9 Hz to +2.8. Seventeen of eighteen seeds negative, at **-7.42 SE** — not
marginal, and nowhere near the 2-3 SE band this project has had to retract from.

**So v50's conclusion survives its own confound.** It saw +16.6 to -2.3 on three
seeds through an intervention that moved two things; this is -7.42 SE on eighteen
through one. The confound did not change the answer, which is worth knowing both
ways: the August reading was lucky rather than sound, and it happened to be right.

**The three legs now hold:**

- the compression **is** intrinsic plasticity — 0.34 against 0.91
- relieving **half** buys nothing — the same intervention halves the learned bias
- relieving **all** collapses learning — dF1 103.9 to 17.7

**The larynx must regulate its own rate in order to learn at all, and regulating
its own rate is what compresses what it learns.** That is a tight trade with no
setting between the horns, and it makes 137 Hz structural rather than an unfound
knob. Thirteen routes, and the thirteenth is the one that closes the question
instead of adding to the list.

**What this does not say.** It does not say a *differently shaped* regulator is
impossible — v57 was one attempt and it recovered half the transfer at the cost of
half the bias. It says the two requirements are in genuine opposition in this
creature, and that anything which relaxes the rate constraint pays for it in the
estimator. Any future attempt has to measure both halves, and the machinery to do
that now exists.

### What the literature says to build next, and why it is the cheap option

> **This section is the argument that produced DNA v51, kept as the record of
> what was predicted before it was measured.** v51 and v52 above are what
> happened. What it leaves open is at the end.

`ctxbias` split the problem cleanly. **Delivery works**: a bias arriving off the
lesson's own neurons is free, and a perfect conditional bias steers the voice to
51% of the gap between the two words. **Computation is missing**: nothing in
this creature can work out what that bias should be.

Five papers, read together, name the same thing, and it is smaller than any
mechanism built here since v41.

**The parameterisation is the whole problem, and this project has said so twice
in contradictory ways.** `vocallearn` concluded that node perturbation cannot be
conditional *because a per-neuron bias is a constant*. Node perturbation on
synapses was then built to fix that, and its post-mortem retracted the
diagnosis: "expressiveness was never the problem, variance is". Both are right,
and Werfel, Xie and Seung reconcile them — learning time scales with the number
of parameters estimated, so the synaptic version bought expressiveness at
sixteen times the parameter count and paid for it in variance. **What was never
tried is the parameterisation that is expressive AND cheap.**

`bias_[i]` is one scalar per neuron. Make it one scalar per neuron *per context*
— `bias_[i][c]`, with `c` the active slice of a v47 `kContext` module — and:

- it is **conditional by construction**, which is the thing `g2cond` measured the
  absence of;
- it is **two node-perturbation problems, not one weight-perturbation problem**:
  252 parameters against 126, where the synaptic tract was ~4000;
- it is cashed **exactly as G2's rule already cashes**, and G2 is a met
  milestone at twelve sigma, so the estimator is not a hypothesis;
- it is delivered **as a bias**, which `ctxbias` has just shown costs the
  exploratory pathway nothing — where v47's tract and v50's regulator both
  charged for arriving;
- and it must project **to the larynx directly**, which the same run established
  by showing a bias on `central` reaches the voice not at all.

**It is Fee and Goldberg's Area X reduced to its computational core.** HVC's
timing signal is sparse and near-one-hot, so a basal-ganglia stage driven by it
and biasing the motor population *is* a context-indexed bias table. Kojima and
colleagues supply the variability that the perturbation term needs; the
efference copy of it is what lets the learning site evaluate an exploration it
did not itself produce, and it arrives at the striatal neuron **without driving
it** — gating plasticity rather than adding drive, which is the same shape
`ctxbias` just measured as free.

**Two things it also explains, which is the reason to believe it.** `retain`
found a conflicting lesson wipes a taught sound to 0.22 while `capacity` found
two orthogonal lessons coexisting at 0.84. Heald, Lengyel and Wolpert's account
says that is one computation, not two results: experiences assigned to one
context overwrite, experiences assigned to two do not. Naming is the conflicting
case by construction — both lessons drive the same formant to different values —
and an explicit context index is precisely what converts the first case into the
second. And Miconi's network learns delayed non-match-to-sample with a rule of
this family, so `g2cond`'s null is a fact about this creature's parameters
rather than about reward-modulated learning.

**What would refuse it.** The honest bar is `ctxbias`'s own ceiling: 236 Hz of
dF1. A context-indexed bias that learns nothing the shared bias did not already
learn, or that reaches a small fraction of that number, says the estimator
cannot find a conditional optimum even when one is representable — and that
would be a much firmer closure of G3 than anything currently on file, because
every earlier null had a delivery excuse and this one would not.

**What actually happened, and what is left.** It was not refused: `areax` reads
112.9 Hz, 48% of that ceiling, and the estimator finds the conditional optimum
when it is handed an index. Three of the architecture's four pieces are settled
— the indexed bias works, delivery as a bias is free, and a per-context
prediction error is unnecessary here (`rpeprobe`: 0.0% of the reward variance is
between contexts, because `vocallearn`'s per-word praise criterion had already
balanced it). The fourth is where the index comes from, and after v52 it is
**two** quantities rather than one: the word survives into the reward window at
0.740, and a *fixed* cut of the population holding it recovers 0.569 of that.
The second loses more than the first. A learned partition of the motor state was the obvious next
build — and `partprobe` priced it and **refused it**: a learned boundary buys
+0.019 over the fixed one, and the supervised ceiling that motivated it is only
reachable by seeing the labels the creature is trying to infer. What is left is
a partition supervised by something the creature has, which is a different
shape from anything tried here.

## The memory chapter, and what closed it

*2026-09-13/14 — six runs over two days.*

The headline is that **the conflicting-lesson wipe was never a memory failure.**
The chapter is worth reading as a sequence, because four of its six results were
things I had to retract or re-read.

**1. There is no hidden memory.** `retain`'s conflicting lesson drops a taught
sound to 0.22, and that is consistent with two very different mechanisms: the
parameters being overwritten, or surviving and no longer being expressed. Those
want opposite machinery — protection versus retrieval — so picking wrong is
expensive. Measuring the *stored* parameter beside the behaviour it drives, on one
creature and both as a fraction of what was gained: **store 0.25 ± 0.05, behaviour
0.30 ± 0.05, difference −0.05 ± 0.05 (−1.0 SE)**, 36 seeds none dropped. They decay
together. It is a **pure overwrite**, and retrieval work is refused.

**2. It cannot be defended either.** Replay reinforces nothing — the cash-in is
`u = step·perturb_i` and during sleep `perturb_i` is fresh noise, so `E[u] = 0`.
Interleaving is refused with a perfect oracle. Stabilisation is a clean null. And
**write separation by restricting plasticity is refused**: blocking even a quarter
of the F1 group costs **59%** of the lesson. The reason is not the one I first gave
— see below.

**3. So it needs avoiding, not defending.** `capacity` had long said this creature
holds two *orthogonal* lessons at 0.84 against the colliding case's 0.22, and that
had never been tested inside the retention protocol. With lesson A taught
identically in every arm and only the second lesson's **axis** differing at equal
log-distance: a second lesson on F1 costs +0.1603 (16.8 SE); one on F2 costs
+0.0364 (3.2 SE). **77% less damage, 10.7 SE.** The control is structural — `err
taught` is `+0.00000 ± 0.00000` because the teaching phase is the same code path —
and the effort check runs *against* the finding, ortho's lesson learning better by
14.9 SE and still damaging less.

**4. But it does not transfer to naming.** The representational implication —
spread the vocabulary across the nine articulator groups — was tested directly and
**refused**: orthogonal and colliding encodings name identically, 0.958 each.
Interference between sequential lessons and conditioning on a heard word are
different problems.

**5. What that run did show is larger.** Both encodings name *well*: **0.958**
against matched marginals at 0.389 and 0.542 — excess **7.3 SE** and **6.2 SE**,
paired on seed. (This page long quoted ~16 SE, which was the *margin* column's,
carried onto the fraction when the verdict was rewritten. Recomputed from the
run's own per-seed rows.) With contexts on,
the creature says the right thing for the word it heard — which reconciles
`g2cond`'s 12σ closure, measured *without* the context-indexed bias. Conditionality
is no longer the blocker. What remains is **travel**: these targets sit 0.29–0.41
log units from rest, where the shipped vowels demand 0.67 on one word alone.

### Four results from this chapter that were wrong before they were right

Kept because the corrections are the useful part.

- **"The store keeps 0.65"** was a raw `gap/teach` ratio, not a fraction of what
  was gained. The sampling boundary sits a third of the way into teaching, so a raw
  ratio keeps the already-learned part in both numerator and denominator. The same
  correction takes 0.58 → 0.25.
- **"Write separation is refused because the centroid partitions the range"** was
  refuted by its own curve. A range cap predicts loss tracking `1 − reachable`
  (38/50/62%) and a *steep* curve; observed was 59/58/68% — higher everywhere and
  flat. A quarter blocked costs nearly as much as three quarters. The real reason is
  that a blocked neuron still perturbs and **is still read by the centroid**: a
  pooled readout cannot be partitioned by restricting plasticity alone.
- **"The colliding encoding names better, 8.4 SE"** was an artefact of scoring by
  *margin*. Collinear targets gain on both distances and inflate the margin ~2× for
  identical learning (predicted 1.8–1.9×, observed 2.03). The geometry-free
  nearest-target fraction reads 0.958 for both.
- **"F2 is the more steerable axis"** is a **hypothesis, not a finding**: 2.9 SE
  once the SE is correctly *paired* (the arms share a seed, so they are the same
  creature at birth). It needs ~68 seeds for 4 SE. What is solid is that both axes
  respond strongly — 18.8 and 19.0 SE over flat untaught baselines — and that the
  axis gap is ~28%, not the 5.6× an earlier run appeared to show. Most of that gap
  was the collision.

### Method notes this chapter paid for

- A control must match the treatment on the **resource it spends** — plasticity,
  trials, updates — not merely on the treatment's presence. A mask control that
  flipped per tick wrote *both* halves over a phase and carried near-full
  plasticity, so the arms learned different amounts and `err after` merely
  reproduced `err taught`.
- **If you can spot a confound by hand afterwards, the experiment must refuse it
  automatically.** Gates added after the fact are retrospective; the catches that
  mattered came from checking a number against the shape it *should* have.
- `set_reward_mask` **allows** a range and freezes the rest of the network. That is
  right for "hand the creature perfect credit assignment" and wrong for "confine one
  lesson"; `set_reward_block` is the complement. Flat-across-a-varied-parameter and
  ending-worse-than-untaught are the two diagnostics that expose the confusion.
- A monitor that greps a results file it did not watch being created will report
  the **previous** run. Clear the log first, and require a marker string only the
  new build emits.

## The demand never arrived

`travelsweep` was launched to answer the question the row above leaves open —
where, between 0.58 and 0.89 log units of target separation, does naming fall
off? It was **killed after 30 of 288 jobs**, and the reason is a better answer
than the curve would have been.

**The top three rungs produce bit-identical creatures.**

| adjacent rungs | taught | matched marginal |
| --- | --- | --- |
| 0.29 → 0.58 | 3/3 differ | 3/3 differ |
| 0.58 → 0.89 | 1/3 differ | 0/3 differ |
| 0.89 → 1.20 | **0/3 differ** | **0/3 differ** |

The bottom rung differing on 6/6 is what proves the target table *does* reach
the reward — this is not a plumbing bug.

**It is algebra.** The caregiver's reward is binary against an EMA of the
creature's own error:

```
value = e < baseline ? praise : scold
e     = |log(f1/T1)| + |log(f2/T2)|
```

While the creature never crosses its target, `|log(f1/T)| = log T − log f1`, so
moving the target further away adds a **constant** to `e` — and a constant
cancels exactly against a baseline EMA of the same quantity. The reward sequence
is then identical no matter how far the target is, and so is the creature. The
graded arm does not escape it either: it divides by `dev`, an EMA of
`|e − baseline|`, in which the constant has already cancelled.

The boundary is where per-trial crossings stop. The *means* never cross a target
at any rung; what matters is fluctuation. At 0.29 the targets (539/720 Hz) sit
essentially **on** the voice's own span (553–702 Hz), so crossings are constant;
by 0.89 (399/972 Hz against 537–722 Hz) they never happen. The observed 3/3,
1/3, 0/3 gradient is exactly that.

**Why this matters more than the curve.** The curve would have come out flat
above 0.58, and there was every reason to read it as the travel ceiling — *the
voice separates two words by 0.30 log units no matter what is asked*. It is
never asked. This is `baseref` — a bar that tracks the creature stops asking —
arriving as an invariance rather than as an argument.

**`ArmLiveness` could not catch this**, and that is the method lesson. It
compares every arm against *one* control; the bottom rung differs from
everything, so it reports 7/7 live while the top three rungs are identical to
each other. A sweep needs an **adjacent-level** check, on the treatment and the
control side both. `travelsweep` now refuses on exactly that condition rather
than printing a ceiling.

### And then the obvious conclusion from it turned out to be wrong

This section first ended by concluding that the naming ceiling "has a cause in
the **teaching protocol** rather than in the larynx". `absbar` tested that
directly, on 36 seeds, and refused it.

Four candidate reward rules; three were settled by algebra before any compute.
Node perturbation's drift is `E[R·u]` with `E[u]=0`, so writing `e = C + h(u)`
with `C` the distance constant: the shipped binary-vs-EMA rule cancels `C` in the
comparison; an absolute **linear** rule `R = −k·e` has `E[R·u] = −k·E[h·u]`, so
`C` multiplies `E[u]=0` and drops — **provably invariant, never built**; an
absolute **quadratic** `R ~ −e²` gives `−k(2C·E[h·u] + E[h²·u])`, whose leading
term **scales with `C`**; and a hard bar makes `R` constant past it, which is
zero drift.

| | distance visible? | learns at 0.89 | far rung (1.20 − 0.89) |
| --- | --- | --- | --- |
| shipped | **0/36** | +19.0 SE | +0.0000 ± 0.0000 |
| absquad | **36/36** | +11.5 SE | **−0.1651 ± 0.0175 (−9.4 SE)** |
| abshard | 28/36 | +0.8 SE | −1.4 SE |

The quadratic rule does exactly what the algebra said — it makes the distance
visible on every seed and still learns at 11.5 SE. **And it is worse**: 0.180
delivered against the blind rule's 0.291 at the near rung, and backwards by 9.4
SE at the far one. Seeing the distance bought nothing and cost a third of the
lesson. **The invariance is real, replicates at 0/36, and is not what limits
naming.**

`baseref` is confirmed in its sharpest form by the third arm and was also too
strong: a *hard* bar placed where the shipped rule settles runs at praise 0.005
and 0.8 SE — dead, exactly as predicted — while the *graded* absolute bar learns
fine. The prediction holds for binary criteria, not for graded ones.

The likely account, untested: the tracking bar holds praise near 0.55, which is
the reward's maximum variance per unit magnitude — a free normalisation. An
absolute bar destroys it, and amplifying the reward's magnitude with distance
amplifies the gradient estimate's noise along with its mean.

**A method cost worth recording.** `ArmLiveness` refused this run after four
hours, before either gate printed, because its control was `shipped-0.89` —
against which `shipped-1.20` is byte-identical *by hypothesis*. The guard fired
on the result it exists to establish. Never make the liveness control an arm
whose identity to another arm is the finding. Every number above was recovered
from the run's own per-seed progress lines, the second time in one day that has
saved a re-run.

### The mask cost was a fact about *where the mask was* (2026-09-15)

`mask-unaffordable` closed write separation on a single number: blocking a
quarter of the F1 group costs **59% of the lesson**, so a pooled readout cannot
be partitioned by restricting plasticity. Three runs took that apart.

`compartprobe` split the 59% into the two causes that run had named — blocked
neurons still *perturb*, and they are still *read* — using a new
`set_explore_block` that silences exploration on a range without touching
anything else. **Both were refused.** Silencing recovers nothing (−0.8 SE), and
blocking twice as many costs no more (+1.0 SE), which dilution cannot survive
since dilution is a quantitative claim.

`blockfloor` then extended the ladder *down*, which nobody had done — every width
ever measured was 25–75%, and **the F1 group is only 14 neurons**, so "a quarter"
is four cells. One neuron costs 30%, two 36%, four 57%: steep, then saturating
onto the earlier plateau. `compartprobe`'s ladder had straddled the knee, which
is why it read flat.

`blockwhere` held the count fixed at **one** neuron and moved it:

| | position | lesson lost |
| --- | --- | --- |
| `top1` | 0.964 | **30%** (−5.8 SE) |
| `mid1` | 0.464 | 7% (−1.1 SE) |
| `bot1` | 0.036 | 9% (−1.4 SE) |

**The same cell costs 30% at the top and 7% in the middle** (`mid1 − top1` +3.6
SE, `bot1 − top1` +3.3 SE). `read_group` weights neuron *i* by
`(i − begin + 0.5)/n`, so index *is* articulator position and a weighted mean is
least sensitive near its own centre — and **every block this project ever
measured took the most expensive cells available.** Away from the top, freezing a
neuron costs nothing measurable.

So the 59% is a fact about *where the mask was put*.

`blockanchor` then ran the two **complementary halves** — exactly what a
separated pair of lessons would occupy:

| | blocked | lesson lost |
| --- | --- | --- |
| `top7` | upper half, 35–41 | **53%** (−11.1 SE) |
| `bot7` | lower half, 28–34 | **9%** (−1.5 SE) |
| `bot2` / `bot4` | lower 2 / lower 4 | 9% / 10% |

`bot7 − top7` is **+0.0737 ± 0.0086 (+8.6 SE)**, and `top7` reproduces
`compartprobe`'s 53% for the same seven neurons exactly. **Half the F1 group is
nearly inert**: freezing the lower seven costs nothing distinguishable from zero,
and the bottom ladder is flat — 2, 4 and 7 neurons all cost about the same.
Essentially all the steering authority sits in the upper half.

**That does not make separation affordable, and the sharper statement is the
point.** Separation needs both halves working *at once*, one lesson in each, and
the half without the leverage is crippled. "Confining a lesson is unaffordable"
becomes **"only one lesson can have the leverage."**

**This pointed at the decoder.** `read_group` is a rate-weighted mean over a
contiguous slice, so a neuron's influence is `(preferred_i − value)/Σw` — **zero
at the centroid, growing with distance from it.** A contiguous half therefore
cannot be equal to its complement, by construction. Uniform leverage would need a
position map that is not monotone in index, or a readout that is not a weighted
mean over a contiguous slice.

#### And the law behind that reasoning is refuted

I wrote the account above the same night, with a caution attached: one free
parameter — the centroid's position — explains all three one-neuron costs to 1.6
percentage points at 0.30, and that is a **fit, not a measurement**. The caution
was right and it was not enough. The fit was wrong, and the data that refutes it
was already in the table three lines above it.

The two **half-block** costs are out of sample for a fit made on the three
one-neuron costs. The law fails them:

| | top7 / bot7 |
| --- | --- |
| measured (53% / 9%) | **5.89** |
| law, at the fitted v = 0.299 | 3.48 |
| law, **maximised over every** v ∈ [0,1] | 4.31 |

**No centroid position reaches the measured asymmetry**, so the functional form is
wrong rather than its parameter. Saturation cannot rescue it either — the law
already predicts an impossible **142%** for `top7`, and compressing that down
makes the predicted ratio *smaller*. So `cost ∝ |preferred_i − v|` should not be
quoted, and neither should the 0.30 that came with it.

Two things it assumed rather than measured: the centroid's own position, and the
per-neuron weights — the true influence is `w_i·(p_i − v)/Σw`, and only
`(p_i − v)` was used. But the better question is not a better law. A block removes
a neuron from the **write**, so what should predict its cost is how much that
neuron's rate actually *moves* while the lesson is learned — which needs no fitted
constant at all. `leverprobe` measures that, and `stageprobe` predicts it comes
back **flat**, since intrinsic plasticity drives every neuron toward the same
rate. A flat profile would refuse the rate account too, and put the asymmetry in
the credit path rather than the readout.

The reusable part is the process failure, not the law: **a fit is not evidence
against the points it was fitted to, and the rest of the data was already in
hand.**

#### What the creature actually does: it silences the top

`leverprobe` measured the F1 group's rates directly, first third of teaching to
last. The anchor holds — the taught arm learned +0.1718 ± 0.0154 against
`blockanchor`'s +0.1676 — and the profile is not subtle:

| k | position | early | late | change |
| --- | --- | --- | --- | --- |
| 0–6 | 0.036 – 0.464 | ~4.8–5.7 | ~4.8–5.8 | **mixed sign, net −0.13 Hz** |
| 7 | 0.536 | 5.03 | 4.21 | −0.82 |
| 9 | 0.679 | 4.67 | 3.37 | −1.30 |
| 11 | 0.821 | 4.54 | 2.64 | −1.90 |
| 12 | 0.893 | 4.18 | 1.59 | **−2.58** |
| 13 | 0.964 | 4.11 | 1.67 | **−2.44** |

**Every upper-half change is negative and large; the lower half's changes are
small and cancel.** The lesson is learned by *silencing the top of the group*.
Blocking neurons that never move costs nothing; blocking the ones that move costs
everything. (This also refutes the prediction I'd drawn from `stageprobe` — that
intrinsic plasticity would flatten the profile. It refutes my prediction, not
`stageprobe`'s own measurement.)

**And the position law dies here without a free parameter left.** The centroid was
*measured* at **0.434** during teaching — the fit had assumed 0.299. At the
measured value, neuron 0 and neuron 13 sit almost equally far from it (0.399 and
0.530), yet one costs nothing and the other 30% at 5.8 SE. No law symmetric in
distance-from-centroid can do that at any scale.

**Most of the ladder I fitted was noise.** Checking the error bars I should have
checked the first time:

| | cost | | |
| --- | --- | --- | --- |
| `top1` | 30% | −5.8 SE | real |
| `top7` | 53% | −11.1 SE | real |
| `mid1`, `bot1`, `bot2`, `bot4`, `bot7` | 7–10% | −1.1 to −1.8 SE | **all consistent with zero** |

Only two of seven blocks have a measurable cost. The 7%, 9% and 10% quoted in the
`blockwhere` and `blockanchor` sections above are point estimates of something
indistinguishable from zero and should not be read as values — what is solid is
the *contrast* (mid1−top1 +3.6 SE, bot7−top7 +8.6 SE).

The pre-registered gate returned **no verdict** (0.738 against a 0.75 bar), and
the honest note is that the statistic was badly chosen: taking the absolute value
per seed *before* averaging turns per-seed noise into signal in the denominator,
biasing it toward 0.5 by construction. The conclusion above rests on the sign
pattern and the two real costs, not on that number.

#### This changes the fix, and opens a route

The first lesson targets /i/ at F1 320 Hz — position **0.093**, far below the
resting centroid of 0.499. To drag a rate-weighted centroid *down* you suppress
the high-position neurons, which is exactly what the profile shows. So the
asymmetry is set by **where the target sits relative to rest**, not by the
readout's geometry — and making the readout uniform, which is what `blockanchor`
proposed, would not address it.

It also opens something the old account had closed. Two lessons with targets on
**opposite sides** of the resting centroid would recruit *different halves* —
suppress-the-top for a low target, suppress-the-bottom for a high one. That is
write separation with no mask at all, and it is the F1-internal version of
`capacity`'s orthogonal case, where two lessons already coexist at 0.84.

The decisive test is pre-registered: **re-run the block ladder with a target above
the resting centroid**. If the asymmetry reverses, the direction account holds. If
the top stays expensive with the target reversed, then those neurons are
privileged in themselves — wiring, or an in-degree gradient — and that is a
different investigation.

#### It silences whichever half is on the wrong side

`blockflip` ran that test: two targets equidistant from the resting centroid, with
F2 held at 2500 in both so only F1's *direction* differs.

| | `top7` | `bot7` | `bot7 − top7` |
| --- | --- | --- | --- |
| **LOW** target (0.093, below rest) | **53%** (−10.7 SE) | 15% (−2.3 SE) | **+7.4 SE** |
| **HIGH** target (0.933, above rest) | 3% (−0.4 SE) | **47%** (−7.6 SE) | **−5.9 SE** |

The same two blocks, opposite signs. The anchors reproduce exactly — `lo-b0`
learned +0.1699 against `blockanchor`'s +0.1676, `lo-top7` cost 53% against its
53% — and the high target is genuinely learnable at +0.1542, so this is a real
comparison rather than one against a lesson that never happened.

**The rate profile is a mirror, and it is all suppression:**

| net change over teaching | LOW | HIGH |
| --- | --- | --- |
| lower half (k 0–6) | +0.062 Hz | **−10.272 Hz** |
| upper half (k 7–13) | **−10.258 Hz** | −0.431 Hz |
| centroid (rest 0.4993) | 0.4355 | 0.5611 |

−10.258 against −10.272: the same magnitude, the other half. And both are
*negative*. **The creature never learns by exciting anything — it silences
whichever half sits on the wrong side of the target**, and the centroid moves
because the other half is left alone.

So **"half the F1 group is inert" was an artifact of only ever teaching one
direction.** Nothing is special about the upper cells.

#### What this does not buy — and I had written that it did

The verdict I wrote into the experiment *before* the run said the reversal hands
us write separation with no mask: two lessons on opposite sides of rest recruit
different halves. That is not established, and data already in hand argues against
it. **`retain`'s two lessons are already on opposite sides of rest** — lesson A at
position 0.093, lesson B at 0.800 — and that is the exact protocol that produces
the 0.22 wipe. If opposite-side targets separated the write, retention would
already be high.

The likely reason is measurable. Lesson B is taught from where A left the
creature, not from rest. After A, the upper half is suppressed to ~1.6 Hz, so to
raise the centroid B can either suppress the lower half further or **restore the
upper half** — which is directly undoing A. From rest it suppresses; from A's
endpoint there is little room left to suppress and a great deal of room to
restore. The telemetry currently samples only the teach phase; extending it to the
gap reads which one B does, and that decides whether the separation route survives
or the 0.22 wipe finally has a mechanism.

#### The second lesson runs back through the first one's write

`gapwrite` extended the telemetry into the gap. The arm that decides it is
`A-keep` — a gap rewarded *toward A's own target*, so the reward, the plasticity
and the duration all match `AB` and the only difference is which target.

| gap phase, net Hz | upper half | lower half |
| --- | --- | --- |
| `AB` (A then B) | **+5.221** ± 1.210 | −5.170 ± 0.525 |
| `A-idle` (unrewarded) | −2.291 ± 0.744 | −2.515 ± 0.258 |
| `A-keep` (rewarded toward A) | **−11.109** ± 1.034 | −1.182 ± 0.490 |
| `AB − A-keep` | **+16.330 ± 1.732 (+9.4 SE)** | −3.987 ± 0.617 (−6.5 SE) |

**A gap rewarded toward A deepens A's suppression by a further −11.1 Hz.** It
restores nothing. That kills the alternative reading the first run couldn't
exclude — that reward simply pulls up whatever is lowest, since the neurons B
restores most (11, 12, 13) are exactly the ones A drove lowest. Same reward, same
plasticity, opposite direction.

Worth noting which way the correction went: **the proper control took the effect
from +7.5 to +16.3 Hz.** A better control is not always a smaller number. And
`A-keep` independently reproduces `retention`'s "a taught sound is kept and keeps
improving" at the level of rates.

**So the 0.22 wipe has a mechanism.** Lesson B raises the centroid by letting back
up the very neurons A silenced — recovering about 48% of A's write inside the gap.
It is not interference, decay, or competition for capacity; those were
descriptions. It is B actively reversing the specific thing A wrote, because after
A those suppressed cells at 1.6–2.6 Hz are the largest lever available for moving
the centroid the way B is asked to move it. `B-rest` confirms the asymmetry is
caused by A: the same lesson from rest moves the upper half −0.9, and in `AB` it
moves +5.2.

The write-separation route is therefore **refused** — two lessons on opposite sides
of rest do not occupy different halves, because the second one does not start from
rest.

**And it suggests one account covering two results.** If the wipe is B's route
running through A's write, retention should depend on the *angle between what the
two lessons require of the rates*, not the distance between their targets:
opposite changes to the same neurons collide (`retain`, 0.22), changes to different
neurons coexist (`capacity`, 0.84 on orthogonal targets). That predicts a second
lesson can be made retainable by choosing a target whose rate requirement doesn't
reverse the first — a property of the target *pair*, costing nothing to arrange.
The obvious way it fails is that almost every F1 target pair is collinear-opposed,
since one centroid has only one axis. Worth testing before believing.

#### Axis, not distance — and the rate signature predicted it

`axiscollide` moved the second lesson's target around the quarter-plane the ranges
allow, holding lesson A fixed and reading retention on the **F1 axis** so an
F2-only lesson wouldn't be scored as destroying A.

| arm | dlogF1 | F1-axis retention | gap dUPPER |
| --- | --- | --- | --- |
| `keep` | +0.000 | 2.738 ± 0.305 | −10.907 |
| `f2-full` | +0.000 | 2.829 ± 0.412 | −10.928 |
| `f1-half` | +0.501 | 1.631 ± 0.265 | −4.046 |
| `shipped` | +0.977 | −0.007 ± 0.127 | +5.021 |
| `f1-full` | +1.000 | −0.057 ± 0.143 | +5.799 |

**`f1-full` and `f2-full` are the same L1 distance from A in opposite axes, and
they retain −0.06 against 2.83** (+5.4 SE). Distance does not predict the wipe.
Retention falls monotonically with the demand on A's own axis while the F2 demand
shifts it by only 0.12–0.20. All seven arms learned A identically to four decimals,
which is the refusal check for a second target leaking into teaching.

**And the rate signature predicted it almost exactly: `corr(retention, gap dUPPER)
= −0.9994` across the seven arms.** That was written down before the run. Arms that
keep A deepen the suppression; arms that lose A restore it. `gapwrite`'s mechanism
generalises beyond the single pair it was measured on.

**But the useful half is not established, and the flaw is mine.** The `f2` arms set
their second target's F1 to 320 — *A's own F1 target* — on the joint metric, so the
gap's reward still contains an F1 term pulling toward 320. They don't ask *nothing*
of F1; they actively reaffirm it. So test B (`f2-full ≈ keep`, +0.4 SE) is close to
tautological, and it does **not** show that an orthogonal lesson is free. Test C
also failed (−2.1 SE): a large F2 demand on top of a fixed F1 demand costs a little
retention — about 7% of F1's effect, small but real.

So the three claims separate:

| | |
| --- | --- |
| the wipe tracks **distance** | **refuted**, cleanly |
| retention tracks the demand on **A's axis** | strongly supported, small residual F2 effect |
| a lesson **orthogonal** to A is free | **untested** — the arms meant to test it reaffirm A |

The run's own verdict is *partial*, which is the honest label. The fix is an arm
with `second_axis = 2`, so the gap rewards F2 alone and is silent about F1.

#### The orthogonal claim, and two ways of nearly fooling myself

`axisfree` built that arm. Across two independent seed families:

| arm | B scored | family 0 | family 1 |
| --- | --- | --- | --- |
| `keep` | joint | 2.738 | 2.254 |
| **`f2-axis`** | **F2 only** | **0.948** | **1.012** |
| `f1-axis` | F1 only | −0.167 | 0.281 |
| primary (`f2-axis − f1-axis`) | | **+5.7 SE** | **+2.0 SE** |

**`f2-axis` replicates tightly** — 0.948 then 1.012, both indistinguishable from
1.0. *A lesson silent about A's axis leaves A where teaching left it* is solid.
**The contrast does not:** family 0's +5.7 SE became +2.0 against a pre-registered
3 SE bar, because the comparison arm got less lethal, not because the orthogonal
arm got worse. So the orthogonal claim is **still unconfirmed**, and the fault is
power rather than design.

Two near-misses on the way there, both mine and both worth recording:

**A malformed gate.** Family 0's gate read "retention ≥ 1.0" — a point estimate
against a hard constant — and returned 0.948, which is 0.41 SE below the line
rather than below it. The correct form, *not significantly below 1.0*, is satisfied
comfortably; but choosing that form after seeing the number is verdict-fitted-to-data
wearing a statistical hat. It was the second malformed gate of the session, after
`leverprobe`'s per-seed |Δ| share, which is attenuated toward 0.5 by construction.
Rule 34: **state every gate as a contrast with an SE attached, and write down the
SE you expect, so a near-miss can be told from a refutation at the time.**

**A confirmation that confirmed nothing.** The first fresh-seed replication came
back *bit-identical* to the exploratory family, because the binary was stale: the
build had run with the shell's cwd left in another directory by an earlier `cd` in
the same command, cmake printed `Error:` where the check grepped for `error:`, and
`; echo built` reported success unconditionally. Three faults, each survivable
alone. The only reason it was caught is that the change was *supposed* to move the
numbers and they did not move at all — a subtler edit would have been accepted.
`axisfree` now prints its seed-family offset so a stale run cannot claim to be a
replication. Rule 35.

#### Confirmed at power, and what it is worth

`axispower` ran the contrast on a third seed family at n=60, with the expected SE
written down beforehand:

| arm | B scored | F1-axis retention | gap dUPPER |
| --- | --- | --- | --- |
| `keep` | joint | 2.455 ± 0.096 | −10.885 |
| **`f2-axis`** | **F2 only** | **1.024 ± 0.062** | −1.941 |
| `f1-axis` | F1 only | 0.116 ± 0.048 | +6.428 |

**`f2-axis − f1-axis` = +0.909 ± 0.079, +11.5 SE against a pre-registered bar of
+3.0.** A second lesson rewarded throughout — on F2 alone — leaves A exactly where
teaching left it. One rewarded on F1 destroys it.

| family | n | contrast | level | per-seed SD |
| --- | --- | --- | --- | --- |
| 0 exploratory | 20 | +1.115 ± 0.197 (5.7) | 0.948 | 0.88 |
| 1 confirmation | 20 | +0.731 ± 0.365 (2.0) | 1.012 | 1.63 |
| 2 at power | 60 | **+0.909 ± 0.079 (11.5)** | 1.024 | 0.61 |

The point estimates agreed all along (0.73–1.12); only family 1's *noise* was
anomalous — its two arms came out negatively correlated across seeds, which
inflates a paired SE above even the unpaired ones, and at n=20 that is well within
chance. **My power forecast was wrong in the safe direction for a real reason:** I
predicted SE ~0.21 and got 0.079, having derived the per-seed SD from family 1, the
one sample whose pairing was anomalous. Over-powering costs only time, but the same
reasoning would have under-powered the run had the anomaly pointed the other way.

**So the account closes.** `retain`'s 0.22 wipe and `capacity`'s 0.84 coexistence
are one mechanism seen at two angles: whether a second lesson costs the first is a
property of *what it asks*, not how far away its target sits. The rate signature
carries it the whole way — `f2-axis` leaves A's neurons suppressed (−1.94 Hz),
`f1-axis` restores them (+6.43), exactly as predicted before any of these runs
existed.

**And it buys nothing for naming.** One formant has one axis, so this is
coexistence *across* articulators and never *within* one — and two words needing
different F1 values are collinear by construction, which is precisely the case the
mechanism says is hopeless. `capacity` already knew orthogonal lessons coexist;
what is new is *why*, not *that*. Good as science, thin as progress. The exponent
ceiling is untouched.

*Two corrections from this sequence.* `compartprobe`'s verdict elected dilution
by elimination the moment silencing came back null — a mechanism claim it had not
tested. And `blockfloor`'s verdict accused `set_reward_block` of being a faulty
instrument, on a threshold it cleared by 1.5 points; the code skips exactly the
neurons in its range, and `blockwhere` exonerates it outright. **A pre-registered
threshold is not a safe verdict when the data lands on it.**

**And the protocol account is now closed** (2026-09-15). Three experiments asked
whether the ceiling is in *how* the creature is taught. All three failed, and two
made things worse:

| | | |
| --- | --- | --- |
| `absbar` | a bar that **sees** distance | **−9.4 SE** at the far separation |
| `staircase` | a **ramped** target | −1.0 SE, and the ramp outran the voice at trial 107/1214 |
| `chase` | a target that **tracks** the creature | **−4.6 SE** excess, **−5.2 SE** taught-only |

`chase` is decisive because it is the strongest form of the hypothesis: the
target sits at the creature's own delivered separation plus 0.06, **ratchets** so
it never retreats, and is free to climb past the milestone's 0.89. There is no
rate that could have been set wrong — which was `staircase`'s escape hatch.

| | target | delivered | shortfall |
| --- | --- | --- | --- |
| chase | climbed 0.15 → **0.2820** over 24 updates | 0.2067 | **0.075** — it *caught* it |
| jump | **fixed at 0.8900** | **0.2701** | 0.620 — never close |

**The arm that caught its target delivered less.** A target the creature can
catch stops pulling; one it cannot catch keeps pulling. The fixed far target wins
*because* it is unreachable — which inverts the framing above: the reward's
blindness means only that it cannot ask for **more than maximum**, and maximum is
already what it delivers. "A bar that tracks the creature stops asking" is true,
and is not a defect, because the alternative — a bar it can satisfy — asks for
less.

**So the ceiling is the voice.** `ceiling-is-an-exponent` stands: `dF1 ~
aligned^0.61`, delivered separation saturates near 0.27 against the 0.89 the
shipped vowels need, and no way of *asking* changes an exponent. The blindness is
real, replicates at 0/36, and is not what limits naming. Further runs on the
teaching protocol are not worth the time; the live leads are structural.

*One rule that collapsed, worth not rebuilding.* `chase`'s first version put the
targets at the 20/80 percentiles of F1 **pooled over both words**. That has a
downward fixed point — teaching two points shrinks the spread, the percentiles
move inward, the demand shrinks with them. The target fell to 0.13 and the voice
tracked it faithfully down to 0.105. Track the *between-word* quantity, and
ratchet it.

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
browser is a peripheral.

That boundary is kept for reasons that have nothing to do with where the code
runs: a core with no allocator and no I/O is deterministic, is testable without
a harness, and cannot hide a state change behind a syscall. **An embedded target
was a requirement here until 2026-08-29 and is not one any more** — the creature
needs 108.70 MB and the effort to shrink it was buying nothing the project
wants. Making it *work* comes first; memory is not a constraint on this
project.

## References

The mechanisms here are named after the work they come from, and every paper the
text leans on is listed with a link. Where a citation could not be verified to
author level it is given by title and URL rather than guessed at — an invented
author list is worse than an incomplete one.

**Sequence generation, and why it needs more than STDP.**

- Fiete, I. R., Senn, W., Wang, C. Z. H. & Hahnloser, R. H. R. (2010).
  *Spike-time-dependent plasticity and heterosynaptic competition organize
  networks to produce long scale-free sequences of neural activity.* Neuron
  65(4), 563–576. <https://doi.org/10.1016/j.neuron.2010.02.003> — states this
  project's `seqprobe` result from the other direction: STDP **alone** cannot
  organize a network to generate long sequences, and STDP **plus heterosynaptic
  competition** does, with no structured input. This project first recorded that
  both ingredients already existed here, naming v38's competitive pruning — which
  is wrong, since v38 is structural and sleep-gated. Measured directly, turning
  DNA v12's divisive normalisation off does spread the weight distribution, but
  per-neuron it is differential growth rather than the conserved-total
  competition Fiete's mechanism needs.
- Mackevicius, E. L., Gu, S., Denisenko, N. I. & Fee, M. S. (2023).
  *Self-organization of songbird neural sequences during social isolation.*
  eLife 12, e77262. <https://elifesciences.org/articles/77262> — sequences form
  in HVC **without tutor exposure**, so the generator does not need patterned
  input to exist; a tutor later binds pre-existing sequences to syllables.
- *Addition of new neurons and the emergence of a local neural circuit for
  precise timing.* <https://www.ncbi.nlm.nih.gov/pmc/articles/PMC8007041/> — new
  projection neurons are recruited to the **end** of a growing feedforward chain
  because immature cells are more excitable and more spontaneously active. The
  closest thing in the literature to what M4's structural growth could be for.
- Okubo, T. S., Mackevicius, E. L., Payne, H. L., Lynch, G. F. & Fee, M. S.
  (2015). *Growth and splitting of neural sequences in songbird vocal
  development.* Nature 528, 352–357. <https://doi.org/10.1038/nature15741> — the
  developmental account the two above build on.

**Node perturbation, which is this creature's motor learning rule (DNA v10).**

- Fiete, I. R. & Seung, H. S. (2006). *Gradient learning in spiking neural
  networks by dynamic perturbation of conductances.* Physical Review Letters 97,
  048104. <https://doi.org/10.1103/PhysRevLett.97.048104> — the rule
  `perturb_rate` implements. `driftprobe`'s reframing rests on its central
  property: the estimate is **unbiased**, so a neuron that cannot affect the
  reward has a covariance of exactly zero with it and receives noise rather than
  a wrong answer.
- Georgopoulos, A. P., Schwartz, A. B. & Kettner, R. E. (1986). *Neuronal
  population coding of movement direction.* Science 233, 1416–1419.
  <https://doi.org/10.1126/science.3749885> — the population vector. §5.3's
  larynx reads a centroid over each motor group, which is this idea, and
  `trajprobe` and `vocab` are both eventually limited by what a centroid can
  represent.

**Inhibitory plasticity, and the two afferents onto one motor population
(DNA v50).**

- Vogels, T. P., Sprekeler, H., Zenke, F., Clopath, C. & Gerstner, W. (2011).
  *Inhibitory plasticity balances excitation and inhibition in sensory pathways
  and memory networks.* Science 334(6062), 1569–1573.
  <https://doi.org/10.1126/science.1211095>,
  <https://pubmed.ncbi.nlm.nih.gov/22075724/> — the rule `isp_gain` implements.
  A symmetric spike-timing rule on inhibitory synapses whose mean drift is
  `eta * nu_pre * (nu_post - rho0)`: it holds a neuron at a target rate by
  growing the inhibition that matches the excitation arriving, rather than by
  moving a threshold every other afferent then has to climb. This creature had
  no inhibitory plasticity of any kind — outside sleep downscaling, an
  inhibitory weight here was fixed for life — and `ipctx` measured what that
  costs: its one rate-side regulator on the larynx runs into its clamp.
- Royer, S. & Paré, D. (2003). *Conservation of total synaptic weight through
  balanced synaptic depression and potentiation.* Nature 422(6931), 518–522.
  <https://doi.org/10.1038/nature01530>,
  <https://pubmed.ncbi.nlm.nih.gov/12673250/> — the experimental result behind the
  same idea stated as a budget: potentiation at one input is accompanied by
  depression at others, so a new input does not simply add. The property this
  creature's regulators do not have, and the reason a second afferent onto the
  larynx costs what it does.
- Mehaffey, W. H. & Doupe, A. J. (2015). *Naturalistic stimulation drives
  opposing heterosynaptic plasticity at two inputs to songbird cortex.* Nature
  Neuroscience 18(9), 1272–1280. <https://doi.org/10.1038/nn.4078>, free full
  text at <https://pmc.ncbi.nlm.nih.gov/articles/PMC5726397/> — the
  arrangement `ctxlearn` is asking one module to be. RA receives the premotor
  timing input from HVC and the exploratory input from LMAN as *separate*
  afferents under *separate* rules, and pairing them drives the two in opposite
  directions. `vocal` here is asked to host both at once, which is what
  "arriving costs more than it pays" is a description of.
- Fee, M. S. & Goldberg, J. H. (2011). *A hypothesis for basal
  ganglia-dependent reinforcement learning in the songbird.* Neuroscience 198,
  152–170. <https://doi.org/10.1016/j.neuroscience.2011.09.069>, free full text
  at <https://pmc.ncbi.nlm.nih.gov/articles/PMC3221789/> — the
  architecture the alternative reading points at. The conditional map is not
  learned at the motor population at all: Area X receives HVC's timing signal
  and a collateral of LMAN's own exploratory signal, learns there under
  dopamine, and biases the motor population from outside. Named here because it
  is what the ISP result licenses next if range is not the whole story — and
  because it is a much larger build than a plasticity rule, which is why it was
  not the thing tried first.

**Where the conditional map is computed, rather than where it is delivered.**
`ctxbias` licensed the delivery route and measured its ceiling; these are the
papers about the half it does not touch.

- Gadagkar, V., Puzerey, P. A., Chen, R., Baird-Daniel, E., Farhang, A. R. &
  Goldberg, J. H. (2016). *Dopamine neurons encode performance error in singing
  birds.* Science 354(6317), 1278–1282.
  <https://doi.org/10.1126/science.aah6837>, free full text at
  <https://pmc.ncbi.nlm.nih.gov/articles/PMC5464363/> — the reward signal in
  Area X is a *performance prediction error*: suppressed after worse-than-
  predicted, activated after better-than-predicted. This creature delivers raw
  R, and every reward term in the honest protocol is object-blind. The
  difference matters because a prediction error is the one form of reward whose
  sign can depend on context without anyone telling it the context.
- Kojima, S., Kao, M. H., Doupe, A. J. & Brainard, M. S. (2018). *The avian
  basal ganglia are a source of rapid behavioral variation that enables vocal
  motor exploration.* Journal of Neuroscience 38(45), 9635–9647.
  <https://doi.org/10.1523/JNEUROSCI.2915-17.2018>, free full text at
  <https://pmc.ncbi.nlm.nih.gov/articles/PMC6222063/> — the experimental
  counterpart to DNA v10. The variability that node perturbation depends on is
  generated in the basal ganglia and is *rapid* and *trial-to-trial*, which is
  what makes an efference copy of it worth anything.
- Werfel, J., Xie, X. & Seung, H. S. (2005). *Learning curves for stochastic
  gradient descent in linear feedforward networks.* Neural Computation 17(12),
  2699–2718. <https://direct.mit.edu/neco/article-abstract/17/12/2699>,
  <https://pubmed.ncbi.nlm.nih.gov/16212768/> — learning time scales with the
  number of parameters being estimated, so perturbing *weights* is slower than
  perturbing *nodes* by the ratio of synapses to neurons. This is the
  retroactive explanation of the one learning rule this project built and
  removed: node perturbation cashed onto synapses cut irreproducible learning
  noise 65% -> 8% and still could not carry G2, and the reason was never
  expressiveness. **It also says which parameterisation to reach for next, and
  it is the cheapest one, not the richest.**
- Miconi, T. (2017). *Biologically plausible learning in recurrent neural
  networks reproduces neural dynamics observed during cognitive tasks.* eLife 6,
  e20899. <https://doi.org/10.7554/eLife.20899>, free full text at
  <https://pmc.ncbi.nlm.nih.gov/articles/PMC5398889/> — the existence proof this
  project needs against its own null. A reward-modulated rule driven by
  exploratory perturbation, with a running reward baseline, learns
  *context-dependent* tasks including delayed non-match-to-sample. So `g2cond`'s
  result is not "this rule class cannot be conditional"; it is a fact about this
  creature's parameterisation.
- Heald, J. B., Lengyel, M. & Wolpert, D. M. (2021). *Contextual inference
  underlies the learning of sensorimotor repertoires.* Nature 600(7889),
  489–493. <https://doi.org/10.1038/s41586-021-04129-3>, free full text at
  <https://pmc.ncbi.nlm.nih.gov/articles/PMC8809113/> — the reframing, and the
  one that explains this project's own measurements rather than adding a
  mechanism. Memory creation, updating and expression are governed by a single
  computation: which context the learner infers it is in. `retain` found that a
  conflicting lesson wipes a taught sound to 0.22 while `capacity` found two
  orthogonal lessons coexisting at 0.84 — which is exactly the difference
  between two experiences assigned to one context and to two. Naming is the
  conflicting case by construction: both lessons drive the same dimension to
  different values.

- Kornfeld, J., Wang, Y., Januszewski, M., Rother, A., Schubert, P., Goldman, M.,
  Jain, V., Denk, W. & Fee, M. S. (2025). *An anatomical substrate of credit
  assignment in reinforcement learning.* bioRxiv 2020.02.18.954354.
  <https://doi.org/10.1101/2020.02.18.954354> — **a lead this project has not
  built, and it contradicts how v51/v53 are wired.** Serial EM of Area X finds the
  two inputs land on different structures: **78.8% of HVC input (the timing and
  context signal) is on dendritic SPINES, while 50% of LMAN input (the
  exploratory variability) is on dendritic SHAFTS.** Shaft depolarisation spreads
  into nearby spine heads and relieves the magnesium block, so variability GATES
  which context-carrying synapses are eligible rather than being added to them.
  The paper prices the alternative: targeting spines with both is about **4 times
  less efficient**. In this creature context and perturbation share ONE parameter
  — `bias_[i][c]` is both the context's memory and the thing node perturbation
  jitters — and the machinery for the separation already exists unused in DNA
  v25's apical compartment, which `apicalprobe` measured carrying the object at
  0.835 to the dendrites against 0.524 at the soma.

**Metaplasticity and memory consolidation (DNA v41).**

- Fusi, S., Drew, P. J. & Abbott, L. F. (2005). *Cascade models of synaptically
  stored memories.* Neuron 45(4), 599–611.
  <https://doi.org/10.1016/j.neuron.2005.02.001> — the commitment brake is this
  in its simplest form.
- Benna, M. K. & Fusi, S. (2016). *Computational principles of synaptic memory
  consolidation.* Nature Neuroscience 19, 1697–1706.
  <https://doi.org/10.1038/nn.4401> — the two-compartment store v41 built and
  refuted. Its conservation law is what makes the readout keep only `r/(1+r)` of
  a lesson.
- Abraham, W. C. & Bear, M. F. (1996). *Metaplasticity: the plasticity of
  synaptic plasticity.* Trends in Neurosciences 19(4), 126–130.
  <https://doi.org/10.1016/S0166-2236(96)80018-X> — the framing: plasticity that
  depends on a synapse's own history rather than on any signal from elsewhere.
- Zenke, F., Poole, B. & Ganguli, S. (2017). *Continual learning through
  synaptic intelligence.* ICML 70, 3987–3995.
  <https://proceedings.mlr.press/v70/zenke17a.html>
- Kirkpatrick, J. et al. (2017). *Overcoming catastrophic forgetting in neural
  networks.* PNAS 114(13), 3521–3526. <https://doi.org/10.1073/pnas.1611835114>
  — the machine-learning form of the same idea, and the reason `capacity` and
  `retain` are posed the way they are.

**Dynamic synapses and cricket phonotaxis (DNA v36).**

- Webb, B. & Scutt, T. (2000). *A simple latency-dependent spiking-neuron model
  of cricket phonotaxis.* Biological Cybernetics 82, 247–269.
  <https://link.springer.com/article/10.1007/s004220050024>
- Reeve, R. & Webb, B. (2003). *New neural circuits for robot phonotaxis.*
  Philosophical Transactions of the Royal Society A 361, 2245–2266.
  <https://doi.org/10.1098/rsta.2003.1188>
- Webb, B., Reeve, R., Horchler, A. & Quinn, R. (2003). *Testing a model of
  cricket phonotaxis on an outdoor robot platform.*
  <https://homepages.inf.ed.ac.uk/bwebb/publications/timr03.pdf> — the clearest
  short description of the circuit: BN1 recovering from synaptic depression at
  the right inter-burst gap, BN2 requiring those onsets close together, and the
  four-pair auditory circuit around them.
- Project overview and publication list:
  <https://homepages.inf.ed.ac.uk/bwebb/cricket/main.html>
- Tsodyks, M. & Markram, H. (1997). *The neural code between neocortical
  pyramidal neurons depends on neurotransmitter release probability.* PNAS 94,
  719–723. <https://doi.org/10.1073/pnas.94.2.719> — the u/R recursion in
  `DnaProjection::stp_use` is theirs; Webb's group's contribution is what to
  point it at.

**Per-neuron credit assignment (DNA v37, v39, v40).**

- Payeur, A., Guerguiev, J., Zenke, F., Richards, B. & Naud, R. (2021).
  *Burst-dependent synaptic plasticity can coordinate learning in hierarchical
  circuits.* Nature Neuroscience 24, 1010–1019.
  <https://www.nature.com/articles/s41593-021-00857-x> — the rule DNA v37
  implements, and the source of the observation that it wants short-term
  synaptic dynamics, apical regenerative activity and plastic feedback pathways
  alongside it. Two of those three were already here.
- Bellec, G., Scherr, F., Subramoney, A., Hajek, E., Salaj, D., Legenstein, R. &
  Maass, W. (2020). *A solution to the learning dilemma for recurrent networks of
  spiking neurons.* Nature Communications 11, 3625.
  <https://www.nature.com/articles/s41467-020-17236-y> — e-prop: eligibility
  traces times a *per-neuron* learning signal. The argument DNA v37 rests on, and
  the source of DNA v39's testable prediction.
- Larkum, M. (2013). *A cellular mechanism for cortical associations.* Trends in
  Neurosciences 36, 141–151. <https://doi.org/10.1016/j.tins.2012.11.006> — BAC
  firing: a dendritic calcium spike turning a somatic single spike into a burst,
  which is what `burst_refrac_scale` models.
- Sacramento, J., Ponte Costa, R., Bengio, Y. & Senn, W. (2018). *Dendritic
  cortical microcircuits approximate the backpropagation algorithm.* NeurIPS
  2018. <https://arxiv.org/pdf/1810.11393> — DNA v40. Errors originate at apical
  dendrites as a mismatch between predictive input from lateral interneurons and
  actual top-down feedback, continuously and without separate phases.

**Competitive learning, which is how DNA v53 derives a context without labels.**

- MacQueen, J. (1967). *Some methods for classification and analysis of
  multivariate observations.* Proceedings of the Fifth Berkeley Symposium on
  Mathematical Statistics and Probability, Volume 1: Statistics, 281–297.
  University of California Press.
  <https://projecteuclid.org/ebooks/berkeley-symposium-on-mathematical-statistics-and-probability/Proceedings-of-the-Fifth-Berkeley-Symposium-on-Mathematical-Statistics-and/chapter/Some-methods-for-classification-and-analysis-of-multivariate-observations/bsmsp/1200512992>
  — the source of the online update DNA v53 runs: each prototype is the running
  mean of what it has won, so the learning rate is `1/wins` and **no constant is
  guessed**. `partprobe` also scores the batch version beside it, because batch
  with restarts is what a probe can do and a creature cannot.
- Ding, C. & He, X. (2004). *K-means clustering via principal component analysis.*
  Proceedings of the Twenty-First International Conference on Machine Learning
  (ICML '04), 29. <https://doi.org/10.1145/1015330.1015408>
  — the ground for this project's `pcratio` account, **and the account was refused.**
  Their result is that the relaxed k-means cluster indicators span the top principal
  directions, so a prototype rule sees the distinctions that lie along the data's
  largest variance and is blind to ones that do not. The prediction was that the
  context index should separate exactly those word pairs whose class difference is
  large relative to the spread along the leading component. Across 28 pairs it
  correlates at +0.600 — and plain d′ correlates at +0.561, so the geometry adds
  essentially nothing over "this pair is easier to hear". The theory is not in
  question; what it predicted here was not distinguishable from the trivial
  predictor, and three pairs in the right order had made it look as though it was.
- Carpenter, G. A. & Grossberg, S. (1987). *A massively parallel architecture for a
  self-organizing neural pattern recognition machine.* Computer Vision, Graphics, and
  Image Processing 37(1), 54–115. <https://doi.org/10.1016/S0734-189X(87)80014-2>
  — **named the wall DNA v60 hit, forty years earlier.** The stability–plasticity
  dilemma is exactly this project's measurement: a prototype rate fast enough to
  acquire a word introduced late is fast enough to destroy the pair that already
  worked (0.999 → 0.480), and a rate slow enough to preserve it cannot learn the new
  one at all. Their answer is not a rate but a vigilance test — an input far enough
  from every existing prototype commits a NEW unit rather than dragging an old one.
  Not built here; it is the direction the refusal points to, and it would also turn
  the context module from k-means with a fixed slot count into something that decides
  when a word deserves a slot.
- Platt, J. (1991). *A resource-allocating network for function interpolation.*
  Neural Computation 3(2), 213–225. <https://doi.org/10.1162/neco.1991.3.2.213>
  — the same allocation idea for function approximation: allocate a new unit when the
  input is both novel and badly predicted, otherwise adapt existing ones. Listed
  beside ART because it is the cheaper of the two to build on this kernel.
- DeSieno, D. (1988). *Adding a conscience to competitive learning.*
  Proceedings of the IEEE International Conference on Neural Networks, San Diego,
  Vol. I, 117–124. IEEE Press.
  <https://www.inf.ufrgs.br/~engel/data/media/file/cmp121/conscience%20competitive%20learning.pdf>
  — **the difference between 0.584 and 1.000 in `partprobe`.** A unit that wins
  early becomes the running mean of what it won, and in high dimensions a mean is
  nearer every point than any single point is, so it keeps winning and the other
  starves. Penalising a unit for exceeding its share fixes it. **Measured again on
  2026-09-20 and it is the whole mechanism:** source 4 was built as "no episode at
  all" and skipped the conscience along with the episode, and adding it back takes
  the context index from 0.002 separation to 0.999 on a-vs-i, on every one of six
  creatures, under a shuffled presentation order. Removing the origin instead (DNA
  v59 gate 3) does nothing, and seeding without the conscience is inert to the bit —
  so it is the win-balance penalty carrying it and not the initialisation.
  This creature also runs the biological form of that fix — per-module homeostasis
  drives each unit toward a target rate and DNA v32's lateral competition is what
  makes
  them compete at all. The strength here is derived from the data's own distance
  scale rather than taken from the paper.

**Hearing and the vowel space.**

- Stevens, S. S., Volkmann, J. & Newman, E. B. (1937). *A scale for the
  measurement of the psychological magnitude pitch.* JASA 8, 185–190.
  <https://doi.org/10.1121/1.1915893> — the mel scale the cochlea is built on.
- Syrdal, A. K. & Gopal, H. S. (1986). *A perceptual model of vowel recognition
  based on the auditory internal representation of vowels.* JASA 79, 1086–1100.
  <https://doi.org/10.1121/1.393381> — Bark-scaled formant differences model
  vowel identification better than F1×F2 in Hz.
- *Auditory sensitivity to formant ratios: toward an account of vowel
  normalization.* <https://pmc.ncbi.nlm.nih.gov/articles/PMC2893733/> —
  sensitivity is heightened in densely populated regions of the vowel space,
  which is the shape `vocab`'s hardest pairs have.
- *Phonetic information in the vowel spectrum: the meaning of Mel-Frequency
  Cepstral Coefficients.*
  <https://www.sciencedirect.com/science/article/abs/pii/S0095447025000452> —
  what MFCCs actually carry, which matters because the audibility ruler is built
  on them.

- **The one-core regime, and three diagnostics worth stealing.** Diamos, G. &
  Claude Opus 5 (2026). *Outrageously Small Neural Networks: Emergent Basic
  Reasoning at 6,616 tok/sec on One Intel AMX Core.*
  <https://huggingface.co/gdiamos/amx-reasoning-v1-instruct> — a preprint on a
  model hub rather than a reviewed venue, and architecturally irrelevant to a
  spiking creature. Read for its Section 8: the permutation ablation that caught a
  branch computing a constant, the untrained-row attractor whose fix threshold had
  to be swept because the effect is graded rather than binary, and a budget law
  saying that adding experts divides a fixed token budget among them — which
  corrects how this project sized its own four-context run.

- **Infant brain volumes, and what they do NOT license.** Knickmeyer, R. C.,
  Gouttard, S., Kang, C., Evans, D., Wilber, K., Smith, J. K., Hamer, R. M.,
  Lin, W., Gerig, G. & Gilmore, J. H. (2008). *A structural MRI study of human
  brain development from birth to 2 years.* Journal of Neuroscience 28(47),
  12176-12182. <https://pmc.ncbi.nlm.nih.gov/articles/PMC2884385/> — read
  2026-09-08. Volumetric, so it says nothing about any mechanism this project can
  build, and nothing at all about the current bottleneck. Two things it does do.
  **Grey matter grows 149% in year one while white matter grows 11%**, so the
  period covering babbling and first words involves almost no long-range tract
  growth — an independent corroboration of this project's expensive finding that
  delivery was never the limit. And it sharpens rather than resolves the
  exuberance question: biology proliferates then prunes, while `exuberance`
  measured proliferation here as costing 0.229. The biological schedule does not
  transfer to this substrate, and that is now a measured disagreement rather than
  an oversight.

- **Why the ceiling is the learning rule, not the tuning.** Hiratani, N., Mehta,
  Y., Lillicrap, T. P. & Latham, P. E. (2022). *On the stability and scalability
  of node perturbation learning.* NeurIPS 2022.
  <https://proceedings.neurips.cc/paper_files/paper/2022/file/cf38eb1549024cce4b3d2c1bb87a6c27-Paper-Conference.pdf>
  — read to full text 2026-09-09. Under model mismatch node perturbation is
  *always* unstable, the instability is weight diffusion, the parameter norm
  "increases monotonically with time", and performance against that norm is
  U-shaped. The only fix they find is weight normalisation, which stabilises
  learning but biases the rule so "the error no longer goes to zero; instead, it
  saturates at a finite value". This creature's bias is node-perturbed and
  normalised twice over — `perturb_max` plus homeostasis — so it sits in the
  stabilised-but-floored regime by construction, and that is what every route
  closing at once was telling us.

- **What the biology does instead of holding it in the bias.** Andalman, A. S. &
  Fee, M. S. (2009). *A basal ganglia-forebrain circuit in the songbird biases
  motor output to avoid vocal errors.* PNAS 106(30), 12518-12523.
  <https://www.pnas.org/doi/10.1073/pnas.0903214106> — findings verified from two
  independent sources; full text paywalled. The AFP builds an error-reducing
  bias, inactivating its output makes the learned change regress *immediately*,
  and the bias is consolidated into the motor pathway within one day. So the bias
  is a **rate, not a store**: it stays inside the stable regime while the
  cumulative shift is unbounded. `bias_ctx_` here consolidates nowhere, which is
  why it has to hold everything and cannot.

**Everything else.**

- **The vowel targets themselves — the most load-bearing citation here and it was
  missing until 2026-09-09.** Hillenbrand, J., Getty, L. A., Clark, M. J. &
  Wheeler, K. (1995). *Acoustic characteristics of American English vowels.*
  Journal of the Acoustical Society of America 97(5), 3099-3111.
  <https://doi.org/10.1121/1.411872> — `kWords`' formants are its adult-male
  means, so every dF1 in this project, every formant error, and the 460 Hz gap
  the naming milestone is measured against all rest on this data set. It was
  credited only in a source comment.

- **The slow store (DNA v41's third gate).** Benna, M. K. & Fusi, S. (2016).
  *Computational principles of synaptic memory consolidation.* Nature
  Neuroscience 19(12), 1697-1706. <https://doi.org/10.1038/nn.4401> — the
  two-compartment fast/slow synapse that `meta_flow` and `meta_ratio` implement.
  Read alongside Andalman & Fee above: Benna-Fusi leaks a fast variable into a
  slow store that does not drive output, and the songbird banks its bias into the
  motor pathway that does. The two predict opposite signs here, which is what
  makes the distinction worth stating rather than assuming.

- **The bimodality coefficient, and its primary source is a manual.** Sarle,
  W. S., in SAS Institute Inc. (1990). *SAS/STAT User's Guide, Version 6*, the
  CLUSTER procedure, "Miscellaneous Formulas", p. 561 — `b = (skew^2 + 1) /
  kurtosis`, which DNA v32's lateral-competition gate is scored on. Worth being
  straight about the provenance: the coefficient is attributed to Warren Sarle and
  first appeared in software documentation rather than a paper, an asymptotic form
  shows up in Ellison (1987) citing the 1982 guide, and the citable modern
  discussion is Pfister, R., Schwarz, K. A., Janczyk, M., Dale, R. & Freeman,
  J. B. (2013), *Good things peak in pairs: a note on the bimodality
  coefficient*, Frontiers in Psychology 4, 700.
  <https://doi.org/10.3389/fpsyg.2013.00700>

- **Dale's law, which sets every weight ceiling in the kernel.** Strata, P. &
  Harvey, R. (1999). *Dale's principle.* Brain Research Bulletin 50(5-6),
  349-350. <https://doi.org/10.1016/S0361-9230(99)00100-8> — a neuron is
  excitatory or inhibitory and not both, which is why `weight_ceiling` is a
  property of the source and not of the synapse. Note that the name is Eccles's
  restatement rather than Dale's own claim, and that Lipton & Manning (2026),
  *What Dale never said*, Journal of Neurophysiology, argues the modelling usage
  overreaches the original — this project uses the modelling sense, so the
  citation is to that convention rather than to a historical claim.

- **Complementary learning systems (DNA v13, the hippocampus role).**
  McClelland, J. L., McNaughton, B. L. & O'Reilly, R. C. (1995). *Why there are
  complementary learning systems in the hippocampus and neocortex.*
  Psychological Review 102(3), 419–457.
  <https://doi.org/10.1037/0033-295X.102.3.419>
- **Song-system exploration (DNA v10, LMAN).** Ölveczky, B. P., Andalman, A. S. &
  Fee, M. S. (2005). *Vocal experimentation in the juvenile songbird requires a
  basal ganglia circuit.* PLoS Biology 3(5), e153.
  <https://doi.org/10.1371/journal.pbio.0030153> — the anterior forebrain
  pathway, which is where reward-modulated motor variability comes from.
- **Context gating is only half of it (`ctxretain`, and the case for `meta_commit`).**
  Masse, N. Y., Grant, G. D. & Freedman, D. J. (2018). *Alleviating catastrophic
  forgetting using context-dependent gating and synaptic stabilization.* PNAS
  115(44), E10467–E10475. <https://doi.org/10.1073/pnas.1803839115>,
  arXiv:1802.01569 — **gating ALONE reads 61.4% across 100 tasks and does not
  support continual learning; gating combined with Synaptic Intelligence or
  Elastic Weight Consolidation reads 95.4%.** That is a prediction about
  `ctxretain`, which tests gating alone. The stabilisation half already ships here
  as `meta_commit`, DNA v41's commitment brake — it gates plasticity by how far a
  neuron has already moved, which is the EWC/SI idea — and `brake-sweep` measured
  it against the wrong question, the magnitude of the learned bias rather than the
  protection of an old lesson from a new one.

- **Node perturbation, and what actually speeds it up.** Dalm, S., Offergeld, J.,
  Ahmad, N. & van Gerven, M. *Effective Learning with Node Perturbation in Deep
  Neural Networks.* arXiv:2310.00965 — decorrelating unit activities gives
  orders-of-magnitude faster NP convergence by removing the confounding that makes
  per-unit credit ambiguous. Recorded rather than acted on, because `ctxgain`
  established this creature's ceiling is drift-limited and showed the
  parameterisation does not matter, so the honest prior is that decorrelation buys
  SPEED and not ceiling — and speed is the one thing `ctxgain` could not measure,
  since both its arms were saturated.

- **Oriented receptive fields (DNA v7).** Hubel, D. H. & Wiesel, T. N. (1962).
  *Receptive fields, binocular interaction and functional architecture in the
  cat's visual cortex.* Journal of Physiology 160, 106–154.
  <https://doi.org/10.1113/jphysiol.1962.sp006837> — the curvature stage of DNA
  v8 is V4's computation at V2's position in the hierarchy.

[bp]: https://www.nature.com/articles/s41593-021-00857-x

### A note on where this project's record actually lives

Two stores, and they are not the same store:

- **In this repo** — `README.md`, `host/src/`, `core/`, and every run log under
  `results/`. Version-controlled, recoverable, and the authority for any number.
- **Outside it** — the assistant's memory directory
  (`~/.claude/projects/.../memory`), which holds the working notes, the refutation
  records and the literature citations. **It is not version-controlled.**

Commit messages in this history sometimes describe memory-file changes as though
they were part of the commit. They were not: those files sit outside the repo
tree, so `git add -A` never staged them. The content is real and on disk, but it
has no history and nothing recovers it if it is overwritten.

**So: any claim that matters belongs in the README or in a run log, not only in
memory.** That is why the scientific content of each chapter is duplicated here.

### The creature cannot hand a packet, and has never heard a glide (2026-09-17)

Two results aimed at the thing that makes this a vowel-holding creature rather
than a talking one.

#### The CPG route is refused at stage 0

Armstrong & Abarbanel model songbird HVC as **winnerless competition** rather than
a synfire chain — rate-coded, oscillatory sequences instead of precise spike
timing. That mattered here because `chain-trigger` had already diagnosed why
synfire failed: chains propagate synchrony, and every pathway in this creature is
rate-coded. So we built the wrong mechanism for our substrate.

Winnerless competition hands a *localised packet* between ensembles. `bumpwalk`
asked whether this creature can form one, using lag-1 spatial autocorrelation of
the F1 rate profile — distribution-free, unlike anything scored against a uniform
null:

| vocal lateral | width | lag-1 autocorrelation |
| --- | --- | --- |
| gain 0.000 | — | **−0.0708** (−54.6 SE) |
| gain 0.020, σ 0.10 | 1.4 neurons | **+0.1361** (+62.8 SE) |
| gain 0.020, σ 0.25 | 3.5 neurons | +0.0910 (+49.1 SE) |
| gain 0.020, σ 0.45 | 6.3 neurons | +0.0320 (+20.4 SE) |

Lateral competition genuinely flips the group from **anti-correlated to
correlated** — without it, neighbours form a checkerboard, which is what
competition without cooperation gives. But coherence **peaks at the narrowest
kernel and falls as it widens**, which is backwards for a bump, and the reason is
the kernel's own algebra: it computes *local excitation minus the field mean*, so
as the neighbourhood approaches the field the difference self-cancels. **A wide
coherent packet is not something this kernel failed to make by a margin — it is
something it cannot make by construction.**

Scope, so it isn't over-read: tested at gain 0.020 across a 4.5× range of width.
Gain wasn't swept jointly, but v32's own numbers cap the affordable range (0.035
fails G2, 0.045 makes babble drone), and the algebra above doesn't depend on gain.

#### And the instrument was wrong twice before it was right

The first version scored positional SD against an **analytic** uniform null of
1/√12 and read 79.6 SE below it — which looks decisive and is worthless, because
that null assumes every neuron fires at the same rate. They don't, so the gap
measured rate heterogeneity rather than spatial structure. The replacement then
carried a hard bar of 0.20 that the data missed at 0.136 while sitting +62.8 SE
from zero, and the verdict prose asserted "no more alike than distant ones" about
data showing the opposite.

The bar was picked with no model of the kernel producing the number — vocal's σ is
0.10, about 1.4 neurons of a 14-neuron group, and a packet that narrow *cannot*
give high lag-1 autocorrelation. The rule that would have caught it: **state what
the number would be if the mechanism worked, from the mechanism's own parameters,
before choosing a threshold.**

#### The finding that outranks both: it has never heard a glide

```
struct Word { float f0, f1, f2; };
```

A word in this project is three numbers. **A trajectory is not representable**, and
every caregiver render passes those constants for the full 900-tick word — no
render argument anywhere depends on the tick. Nothing this creature has ever heard
has changed while sounding.

This project has spent a long line of work asking whether the brain can *generate*
a sequence. It has never asked whether it can *follow* one, because it has never
presented one — while `M1b` is met at 0.890, so the ear-to-voice route demonstrably
carries content. It has only ever been asked to carry a constant.

**The creature is not failing to talk. It has never been asked to say anything that
takes time.**

#### The voice drones, and the result that said otherwise was my own fit

The sequence work had aimed at the wrong pathway the whole time. The genome
carries two time constants:

| | τ | usable to |
| --- | --- | --- |
| formants (`smoothing_ms`) | 800 ms | ~0.2 Hz |
| **glottis (`gate_smoothing_ms`)** | **60 ms** | **~2.7 Hz** |

Synfire waves, chain triggers, travelling bumps and the refused CPG route all
aimed at the formant path, which is *four syllables long*. And the split is exact
in the code: `amplitude` and `voicing` are smoothed with the **fast** constant,
only the formant centroids with the slow one. **Loudness and voicing are fast;
vowel identity is slow** — which is the order [MacNeilage's frame/content
theory](https://pubmed.ncbi.nlm.nih.gov/10097020/) says development takes. The
syllable *is* an open-close alternation, and babbling is mostly frame with little
content under independent control. This creature is accidentally built that way
round.

So: does its loudness already oscillate? `babble PASS — duty cycle 0.50` says how
*often* it is on, never how fast it alternates. A drone and a babble both read
0.50.

First reading: **+9.99 ± 0.29 dB** over the fitted aperiodic trend at **2.34 Hz**,
against a pre-registered 3 dB bar. It looked like the creature already babbles.

**It doesn't.** The gate's corner sits at 2.65 Hz and the peak appeared at 2.34 Hz
— and a first-order low-pass is flat below its corner and falls above it, so
fitting *one straight line* in log-log to that bend leaves a positive residual at
the knee. A gate sweep decided it:

| gate ms | corner Hz | peak Hz | per-seed SD | range | peak dB |
| --- | --- | --- | --- | --- | --- |
| 30 | 5.31 | 3.34 | **1.56** | **1.35 – 5.78** | 10.63 |
| 60 | 2.65 | 2.34 | — | — | 9.99 |
| 150 | 1.06 | 1.73 | 0.76 | 1.01 – 3.84 | 9.10 |

The peak tracks the corner; the amplitude stays flat at 9–11 dB across a 5× corner
change, where a genuinely filtered rhythm would change sharply; and decisively, **at
gate 30 the per-seed peaks span the entire search band.** A real 2.3 Hz rhythm
gives a tight per-seed distribution. This is "wherever the residual of noise
happened to be largest."

The pooled number looked overwhelming at *every* gate setting. Only the per-seed
rows showed the peak frequency was unstable — which is an argument for logging
per-creature values even when the summary looks decisive.

**What this leaves is sharper than what it took away.** The frame does not exist
and has to be built, and the glottal path is fast enough to carry one where the
formant path never was. Every oscillator this project has built (DNA v26) was
aimed at *learning* and refuted there; **none has ever been aimed at the voicing
gate**, which is the one target frame/content says matters. Note the ceiling
first: a 60 ms gate caps a frame near 2.7 Hz against human speech's 4–5 Hz.

#### A 3 Hz resonance — the first timekeeping structure here

If the frame can't come from inside, can it come from outside? `framecopy` said yes
and meant nothing: the creature's voice squares up to a caregiver's beat because a
*shipped inhibitory auditory→vocal tract stops the babbling while it hears
something*. An interrupt, not a beat.

The discriminator is persistence — the same logic that made M1b a real milestone
rather than a pass-through. So `framehold` pulses, then goes **silent**, and
measures the *f*-component of the voice during the silence.

| arm | family 0 | family 1 |
| --- | --- | --- |
| heard-1 | +0.1 SE | +0.9 SE |
| heard-2 | +2.4 | +0.1 |
| **heard-3** | **+3.5 SE** (SNR 5.01, early 5.23) | **+3.0 SE** (SNR 4.54, early 5.49) |
| heard-4 | +1.3 | +2.6 |

**Only 3 Hz holds across both seed families**, on the arm specified in advance —
the others scatter, which is one real effect plus noise. In both it is a *decaying*
ring-out: early hold ~5.2–5.5, late hold at baseline.

And it has a partial mechanism. Sweeping `self_gain`, the dial that mixes the
creature's own voice back into its ear:

| `self_gain` | early-hold SNR | vs silent |
| --- | --- | --- |
| 0.00 (deaf) | 1.79 | +1.94 (+3.1 SE) |
| 0.25 | 3.35 | +3.54 (+4.0 SE) |
| 0.50 (shipped) | 5.49 | +2.90 (+3.0 SE) |

**The ring scales threefold with self-hearing** — that loop is documented negative
feedback, and delayed negative feedback rings (3 Hz implies a ~167 ms loop delay).
**But a deaf creature still rings at +3.1 SE**, so self-hearing is a *gain* on the
resonance, not its origin. The next candidate is the other delayed negative
feedback that dominates this larynx: intrinsic plasticity, where a lagged homeostat
overshoots.

**What this is, precisely.** A ~3 Hz damped resonance exists in the voice, can be
excited from outside, decays over a few hundred ms, replicates, and is amplified
threefold by self-hearing. **It is not a frame** — a resonance that must be kicked
is not an oscillator that runs, and free-running the voice still drones. What it
adds is that the creature has a *preferred timescale in the syllable band*, on the
fast glottal pathway where a frame would have to live, rather than being flat
there.

It survived three separate attacks rather than one threshold: a transient fix that
selectively killed a rival arm, a fresh-seed replication on the pre-registered arm,
and a graded mechanism sweep.

**Intrinsic plasticity is not the oscillator.** Sweeping `ip_wake_scale` on vocal
gave 4.40 / 3.41 / 3.56 / 4.54 across 0.0 → 1.0 — flat, including fully off. The
duty cycle moved substantially (0.44 vs 0.57), so the creature changed while the
ring didn't, which is a stronger refusal than a flat curve alone. Both major
feedback loops are now eliminated as the source: self-hearing amplifies it, IP
doesn't touch it. **The origin is open** — something with a ~167 ms delay that is
neither.

**And the randomisation's imperfection turned into an accidental control.** Drive
length is fixed at 4000 ms, so only hold length varies, and whether a residual
transient stays coherent at *f* depends on each hold being a whole number of
cycles:

| f | holds in cycles | aligned | observed |
| --- | --- | --- | --- |
| 1 Hz | 3.0 4.5 3.5 5.0 4.0 | 3 of 5 | +0.1 / +0.9 SE |
| 2 Hz | 6.0 9.0 7.0 10.0 8.0 | **5 of 5** | +2.4 / +0.1 SE |
| **3 Hz** | 9.0 13.5 10.5 15.0 12.0 | 3 of 5 | **+3.5 / +3.0 SE** |
| 4 Hz | 12.0 18.0 14.0 20.0 16.0 | **5 of 5** | +1.3 / +2.6 SE |

2 Hz and 4 Hz are *fully* aligned, so a residual transient would be strongest
there — and they are the weakest. 3 Hz is only partially aligned and is strongest
twice. **The artifact account predicts the opposite of the data.**

**The mechanism hunt: four parameters swept, the frequency never moves.**

| parameter | swept | effect on the 3 Hz ring |
| --- | --- | --- |
| `self_gain` | 0.0 / 0.25 / 0.5 | **amplifies 3×** (early 1.79 → 5.49), but a deaf creature still rings (+3.1 SE) |
| vocal `ip_wake_scale` | 0.0 → 1.0 | **flat** (4.40 / 3.41 / 3.56 / 4.54) |
| `a_plus`, `a_minus` | 0 / half / shipped | **flat** — survives STDP fully off |
| `voicing_threshold` | 0.30 / 0.42 / 0.55 | **frequency pinned at 3 Hz** in all three |

So it is not learned, not either homeostat, and not a threshold relaxation
oscillator. It is a **fixed delay** — architecture rather than a dial. No genome
field sits at ~167 ms in the vocal path (`latency_ms` is the retina, `duration_ms`
is touch), so the delay is emergent from conduction and membrane constants.

And the *effect* is the stable quantity, not the contrast. Across nine independent
runs spanning different genomes and seed families, `heard-3` SNR averages 3.95
(SD 0.72) against `silent-3` at 1.25 (SD 0.61) — the control is what bounces, which
is why the contrast SEs ranged +1.4 to +4.2 while the effect barely moved.

*Caveat on "pinned":* the experiment samples only 1, 2, 3 and 4 Hz, so this means
"did not move to another **sampled** arm." A shift from 3.0 to 3.3 Hz would be
invisible, and resolving the frequency needs a finer sweep before anyone fits a
delay to it.

### The resonance stays a resonance

A damped ring becomes a running oscillator when loop gain rises. That is the
textbook route from the structure we found to the structure we want, and it had
one elegant property: **`framehold`'s `silent` arm already *is* the frame test.**
No caregiver, a free-running creature, its own voice measured at 3 Hz with the
local-SNR statistic that survived `babblerhythm`. It sits at 1.25. If the ring
self-sustained, that number would climb, and a voice alternating at 3 Hz with no
input at all is a frame by any definition.

So `self_gain` — the larynx→ear loop, the one dial that amplified the ring
threefold — was pushed from its shipped 0.5 up to 0.75, 1.0 and 1.5. The risk was
written down first: this loop is *negative* feedback, so more gain might simply
mute the voice and leave no envelope to measure. Duty cycle was the kill switch.

| `self_gain` | `heard-3` hold | early | `silent-3` | contrast | duty |
| --- | --- | --- | --- | --- | --- |
| 0.00 deaf | 2.62 ± 0.62 | 1.79 | 0.68 ± 0.11 | +1.94 (+3.1 SE) | 0.58 |
| 0.25 | 4.43 ± 0.87 | 3.35 | 0.89 ± 0.19 | +3.54 (+4.0 SE) | 0.59 |
| **0.50 shipped** | 4.54 ± 0.90 | 5.49 | 1.64 ± 0.34 | +2.90 (+3.0 SE) | 0.57 |
| 0.75 | 2.49 ± 0.57 | 3.72 | 2.07 ± 0.32 | +0.42 (+0.6 SE) | 0.56 |
| 1.00 | 2.57 ± 0.52 | 3.63 | 1.84 ± 0.35 | +0.73 (+1.2 SE) | 0.55 |
| 1.50 | 3.54 ± 1.17 | 4.43 | 1.22 ± 0.39 | +2.32 (+1.9 SE) | 0.51 |

**The kill switch never fired and the frame never appeared.** The manipulation
demonstrably took — free-running duty falls monotonically 0.58 → 0.51, the
negative-feedback dial behaving exactly as documented — and the voice never went
mute, so this is a null rather than an artifact of silence.

`silent-3`'s best case is 2.07 ± 0.32 at gain 0.75, which is **+2.2 SE** against
the nine-run control distribution: inside this project's own "hypothesis, not a
finding" band. Then 1.84 at 1.0. Then 1.22 at 1.5 — gone.

The detail that settles it is *which half of the silence moved.* In the `silent`
arm there is no drive, so early and late are two halves of the same silence, and a
sustained oscillation has to be in both.

| `self_gain` | `silent-3` early | `silent-3` late |
| --- | --- | --- |
| 0.50 shipped | 1.30 ± 0.49 | 1.60 ± 0.46 |
| 0.75 | 1.36 ± 0.34 | 1.72 ± 0.47 |
| 1.00 | 1.36 ± 0.42 | **2.52 ± 0.83** |
| 1.50 | 1.20 ± 0.29 | 0.97 ± 0.31 |

**The early window is pinned at 1.30 / 1.36 / 1.36 / 1.20 — a range of 0.16
across a threefold change in loop gain.** Everything that moved, moved in the
second half only, and not monotonically. A self-sustaining oscillation cannot hide
from the first half of a silence.

The sweep also cost us a claim we had already written down. The first three points
of this dial read as a clean threefold dose-response, and that is how it was
reported. Extended to six, it flattens: the three arms above shipped average
3.93 ± 0.49, and the apparent peak at 0.50 stands only +1.56 ± 1.20 = **+1.3 SE**
above them, so neither "threefold dose-response" nor "peaks at the shipped value"
survives. The gain saturates by 0.5. This is the third time here that a monotone
three-point curve has died on its fourth point, and the lesson is not getting any
cheaper.

What is refused, deliberately, is the follow-up: no finer sweep and no fresh seed
family chasing that late-window +2.2 SE. A late-only, non-monotone bump sitting
above a flat early window has exactly the shape of the +9.99 dB that `babblerhythm`
produced when it was really a fit to a low-pass knee, and of the spectacular
`framecopy` result that turned out to be the shipped listening reflex. Three of my
own positives died to that pattern in one night. The dial is closed.

Self-hearing loop gain is now the fifth parameter swept without changing the ring's
character. **A thing that has to be kicked is not a clock. A frame here needs a
mechanism this creature does not have, not a dial it does** — and the honest
version of tonight's progress is that we found a preferred timescale in the
syllable band and proved it cannot be talked into running on its own.

### The syllable band is empty

If a dial will not turn a damped ring into a limit cycle, the next question is not
which dial to try but what a limit cycle *needs*. So instead of running anything,
we read the neuron's state list and sorted every time constant in the genome.

The full per-neuron state is membrane voltage and rest, threshold, target rate,
leak, noise, two rate averages, position, the node-perturbation trace and its
earned bias, two STDP traces, two synaptic-scaling references, and the refractory
and last-spike bookkeeping. **Nothing in that list is a spike-triggered
hyperpolarising process with its own time constant.** The only two things that
make a neuron less excitable after it fires are `refractory_ms` = 3.0, which is
all-or-none and does not accumulate across spikes, and the intrinsic-plasticity
threshold, driven by a rate average whose alpha is hard-coded as `dt_ms / 1000` —
a time constant of exactly one second, at a gain of 0.0003 threshold-steps per Hz
of rate error.

Then the ladder:

| band | what lives there |
| --- | --- |
| 0.5–6 ms | axonal delays, `chain_delay_ms`, refractory |
| 5 ms | membrane leak |
| 15–20 ms | interneuron pool, both STDP windows |
| 30–60 ms | apical compartment, **the glottal gate** |
| **100–300 ms** | **nothing.** Only `duration_ms` = 250, and that is touch |
| 800 ms | **the formant smoothing** |
| 1000 ms | the rate average that drives intrinsic plasticity |
| 2 s – 60 s | eligibility, reward baselines, traffic, sleep |

**There is a hole between roughly 60 ms and 800 ms, and a syllable is 150–300 ms
long.**

A relaxation oscillator needs a fast variable and a slow one, with the slow one on
the order of the period. A half-center oscillator — Brown's 1911 result, formalised
by Matsuoka in 1985 — needs mutual inhibition *plus* a fatigue process that makes
the winner release; without the second ingredient, mutual inhibition settles on a
winner and stays there. **This project has always had the inhibition and has never
had the fatigue.**

That retro-explains four separate nulls, and none of them needed to be run again to
see it. Loop gain could not make the 3 Hz ring self-sustain because gain is not a
slow variable. The `voicing_threshold` sweep found it was "not a threshold
relaxation oscillator" — correct, because the threshold is a perfectly good fast
variable whose only slow partner sits at one second, three times too slow for a
333 ms period and running at a gain of 3×10⁻⁴. The CPG route was refused at stage 0
for a kernel that self-cancels as it widens, and this is a second and independent
reason the same route was dead. And "no module holds a kick for 10 ms" is what a
5 ms membrane and a 60 ms longest-fast-state predict.

So *this creature cannot keep time* is not a learning failure, a wiring failure, or
a tuning failure. **It is a missing state variable, and the ladder says exactly
where the missing one belongs.** Which also means the honest reading of the last
several experiments changes: they were not weak evidence against rhythm, they were
correct measurements of a system that structurally cannot produce one.

The constant for the next build is derived rather than guessed, which this project
has paid for before. A relaxation oscillator's period runs roughly two to four
times its adaptation constant, so a 3 Hz rhythm wants tau around 85–170 ms — inside
the empty band, reachable by nothing currently in the genome. The principled pick
is **≈167 ms, because that is the loop delay the existing 3 Hz resonance already
implies**: setting the fatigue constant equal to the delay the system already rings
at is the condition for that resonance to become a limit cycle rather than some
new and unrelated rhythm.

And the refusals go in before the run, including the one that killed the last
attempt. The rhythm has to appear with **no caregiver at all**, in `framehold`'s
`silent` arm, where a nine-run null distribution already exists — a rare luxury,
since the control was measured before the manipulation was conceived. It has to
appear in **both halves of the silence**, because that is precisely where loop gain
failed. It must not merely mute the voice, since adaptation lowers excitability and
a quieter creature would raise the statistic by shrinking its denominator. And
`tau` has to be **swept, with the frequency tracking it** — a rhythm that shows up
at 3 Hz whatever the constant is the old resonance being re-measured, not a new
oscillator.

Worth keeping past this build: **sort the timescales before proposing a dynamical
mechanism.** A rhythm at frequency *f* needs state at roughly 1/*f*, and a project
can carry a hole in its ladder for a very long time without noticing, because every
individual constant looks entirely reasonable on its own.

### The voice does not follow

The frame is the rhythm; the content is what rides on it. With the frame refused
four ways, the content half was still untested — and untested for a reason nobody
had noticed: `struct Word` is three scalars, so nothing this creature has ever
heard changes while sounding. A long line of work asked whether the brain can
*generate* a sequence. None asked whether it can *follow* one, because none had
ever presented one.

The obvious experiment is a diphthong. The arithmetic refuses it. The formant path
is one-pole with tau = `smoothing_ms` = 800 ms exactly, corner 0.199 Hz, so a
human-rate glide of 150–250 ms passes at |H| 0.06–0.10: a 460 Hz heard sweep would
arrive as 27–45 Hz of produced travel, under the noise floor. Run there, a flat
voice would have been written up as *the ear-to-voice route carries only steady
state* — a refusal of the wrong thing, because no amount of learning fixes a
filter. So the arms went at 4000, 1600 and 400 ms, and the caregiver's amplitude
was held **constant** so that only timbre moved. The instrument and its
pre-registration were committed before the run.

| arm | F1-SNR at f | amp-SNR at f | F1 mean | F1 sd | duty |
| --- | --- | --- | --- | --- | --- |
| glide-slow | 64.61 ± 48.02 | 328.59 ± 65.83 | 625.0 | 23.4 | 0.55 |
| glide-mid | 34.14 ± 15.84 | 466.93 ± 119.08 | 624.1 | 23.0 | 0.55 |
| glide-fast | 37.46 ± 19.73 | 553.88 ± 157.28 | 624.8 | 21.9 | 0.56 |
| static-mid | 1.37 ± 0.32 | 1.09 ± 0.42 | 624.6 | 21.7 | 0.58 |
| f2glide-mid | 13.26 ± 5.53 | 174.51 ± 46.13 | 626.1 | 22.3 | 0.57 |
| silent-mid | 0.91 ± 0.24 | 0.80 ± 0.28 | 624.4 | 22.4 | 0.57 |

**The voice does not follow, and the number that carries it is a plain standard
deviation.** A coherent sinusoid of amplitude A contributes A/√2 to the spread, so
following at the pole's allowance predicts 275.8, 174.7 and 50.9 Hz of produced
F1 spread. Observed excess over the static control: 8.76, 7.62 and 2.95 Hz —
**3.2%, 4.4% and 5.8%**. And produced-F1 variability in the glide arms (21.9,
23.0, 23.4) sits *inside* the scatter of the two arms with no formant motion at
all (21.7 and 22.4). A 460 Hz caregiver sweep leaves the articulator's excursion
unchanged, and its mean sits at 624–626 Hz in every arm including silence.

**The F1-SNR column, though, is contaminated, and it should not be quoted.** The
design held caregiver amplitude constant on the argument that there was then no
envelope at the glide frequency for the listening reflex to copy. That argument was
wrong. The reflex does not need a *rendered-amplitude* envelope, it needs an
*auditory-drive* envelope, and a formant sweeping across a 24-band mel filterbank
manufactures one by moving energy between bands of unequal width. Produced
amplitude came back carrying 300–550× the power at f that produced F1 did.

The trip-wire that caught this was pre-registered, and so was the control that
settles it: **`f2glide-mid` reads +2.1 SE over static, identical to `glide-mid`'s
+2.1 SE.** A caregiver that never moves F1 at all produces the same apparent
effect on produced F1. The instrument check failed too — a statistic measuring
transmission through an 800 ms pole cannot read 64.6 / 34.1 / 37.5 across a
fivefold change in that pole's attenuation.

**The 3 SE gate held the line.** `glide-mid` came in at +2.1. Had the bar been set
at 2 SE, this run would have printed *the voice follows* on contaminated numbers,
while its own axis control sat at the very same +2.1. The gate was written down
before the run, and that is the only reason this is a refusal rather than a
retraction. Two rules came out of it: *excluded by construction* is only as good as
your list of routes, so ship the trip-wire anyway; and a ratio statistic is the
wrong scale for a strongly driven signal — `glide-slow`'s 64.61 ± 48.02 is a 47×
effect that fails a gate purely on instability, where a standard deviation in Hz
answers the question and can be compared against a prediction instead of only a
control.

One thing fell out sideways. `static-mid` — a caregiver sounding *continuously* —
shows duty **above** the silent baseline, no reflex at all, where the pulsed
caregivers of the earlier experiments produced a clear gate-off. Only the arms
whose formant moves show the drop. **The listening reflex adapts to a steady sound
and re-triggers on change**, so it should be read as an onset response rather than
a sustained one.

What this closes is the more useful half. Hearing a trajectory does not install
one, so **a driver cannot be borrowed from the caregiver and the generator has to
be built** — which is what the empty syllable band says is missing, arrived at by
a completely independent route. The sequence problem is internal. That is a
refusal, not a null.

### Fatigue is not a clock — but only half the hypothesis was built

The timescale audit named a missing state variable, so we built it: DNA v56, a
spike-triggered adaptation current on the larynx. It ships off and bit-identical —
hash `ad96f882becbee92`, re-checked after every edit — and the vacuity check ran
*before* the experiment rather than after, since three distinct hashes at
`adapt_jump` 0.02, 0.10 and 0.50 are what distinguish a live mechanism from an
inert one. Forgetting to widen `required_bytes` from 19 to 20 Scalars per neuron
surfaced immediately as `brain init failed (2)`, which is the right way for that
mistake to arrive.

`adaptclock` then measured a **free-running** creature — no caregiver at any
point, because a kicked rhythm was already known to die with its driver. With no
drive there is no drive frequency, so every frequency scores from one run, which
freed the arms for `tau_a` itself.

| arm | jump | tau | peak f | peak SNR | duty | rate |
| --- | --- | --- | --- | --- | --- | --- |
| off | 0.00 | 167 | 1.90 ± 0.25 | 3.93 ± 0.43 | 0.57 | 4.93 |
| tau083 | 0.05 | 83 | 3.85 ± 0.40 | 5.33 ± 1.12 | 0.57 | 4.64 |
| tau125 | 0.05 | 125 | 3.75 ± 0.42 | 5.33 ± 0.72 | 0.51 | 4.44 |
| tau167 | 0.05 | 167 | 3.58 ± 0.29 | 6.35 ± 1.16 | 0.45 | 4.23 |
| tau250 | 0.05 | 250 | 3.00 ± 0.47 | 4.25 ± 0.48 | 0.30 | 3.67 |
| tau333 | 0.05 | 333 | 3.79 ± 0.40 | 3.70 ± 0.26 | **0.16** | 3.05 |
| jump15 | 0.15 | 167 | 2.96 ± 0.35 | 4.58 ± 0.74 | **0.06** | 2.76 |
| jump30 | 0.30 | 167 | 2.85 ± 0.47 | 4.38 ± 0.66 | **0.01** | 2.14 |

**The kill switch fired on three arms.** Duty falls monotonically 0.57 → 0.01 and
vocal rate 4.93 → 2.14 Hz, so the current is first and foremost a mute dial —
a considerably stronger one than `self_gain`, which spans only 0.58 → 0.51.
Intrinsic plasticity never compensated, which is what a gain of 3×10⁻⁴ predicts.

**The refusal rests on an exponent, not a correlation.** A relaxation oscillator
requires period *proportional* to its adaptation constant, so `period/tau` should
be flat. Instead it falls from 3.13 to 1.33 across the sweep, and on the three arms
that kept their voice, doubling `tau_a` moves the peak by 7% — an exponent of
**+0.104** against a required **1.000**. The peak never leaves 3.0–3.9 Hz, which is
exactly where the pre-existing resonance already sits. This is that resonance being
modulated, and it is the precise failure the run was pre-registered against.

There is a gate caution in here worth more than the result. The experiment printed
`corr(1/tau, peak f) = +0.493` computed over all five `tau` arms — including the
one whose duty had collapsed to 0.16. On the four arms that kept a voice the
correlation is **+0.853, which passes the pre-registered 0.80 bar.** It was refused
by the in-band count and then decisively by the exponent. So: compute the primary
over the arms the kill switch left alive, and prefer a statistic with a predicted
*value* over one with only a predicted *sign*. A correlation can pass on four
points while the slope is ten times too shallow.

And one null fell out by accident that will be useful repeatedly: the off arm's
peak is 3.93 ± 0.43, and that is an **argmax over 22 bins**. A raw local-SNR near 4
is unremarkable once a maximum has been taken, so any future argmax over this grid
must be compared against ~3.9 rather than against 1.0. The earlier 3 Hz result is
unaffected — it scored a pre-specified frequency against a matched silent control,
not an argmax.

**Now the correction, and it matters more than the numbers.** The experiment's own
closing text claims that mutual inhibition plus fatigue is the textbook half-center
and has now been tried. That is wrong, and the earlier chapter here stated the
reason without acting on it: **only fatigue was added.** Matsuoka's 1985 result is
that mutual inhibition *alone* settles into a winner and that adaptation is the
ingredient which makes it oscillate — you need both, wired as two mutually
inhibiting populations. The lateral kernel on this larynx is local-minus-field-mean,
a bump kernel, not reciprocal inhibition between two populations. So what this run
refutes is *adaptation alone suffices here*, which is what the theory predicts
should fail.

The empty-syllable-band account therefore splits. Its weak form is untouched and
isn't really in doubt: the rung genuinely is missing, and that is a fact about the
code rather than an inference. Its strong form — add the rung at the derived
constant and a clock appears — is refuted. **The half-center itself remains
untested**, and the next build is the wiring rather than another dial: two vocal
sub-populations with reciprocal inhibition between them, with the v56 current
already in place to release whichever one is winning.

### The queue, from the literature

Four leads came out of reading rather than running, and they are ordered by
cost-to-information rather than by how appealing they are.

**1. Sweep input strength to the larynx — first, because it is cheap and because it
gates the interpretation of the run already in flight.** Shpiro, Curtu, Rinzel and
Rubin analysed several mutual-inhibition-with-adaptation models and found the same
structure in all of them: as input rises, the circuit passes through fusion,
antiphase oscillation, **winner-take-all**, antiphase oscillation again, and finally
simultaneous high activity. There are two oscillation windows and they are not
adjacent. Two consequences matter here. Period is *proportional* to the adaptation
time constant, which means demanding an exponent of 1.0 was the right bar and
`adaptclock`'s measured +0.104 really does mean "not adaptation-driven." And the
regime boundaries **do not move with the time constants** — they are set by input
strength and cross-inhibition. `halfcenter` sweeps tau and inhibition and never
touches input strength, so it samples a single point on the only axis that selects
the regime. The handle is `noise_amp`, since motor noise is motor drive in this
creature. The sweep must be dense: a coarse bracket can step straight over a narrow
window, and there are two of them with a winner-take-all gap in between.

**2. A differential two-pool readout — the oldest wall here, and the smallest
build.** Seven instruments died to common-mode swamping in a pooled readout, and
`read_group` is a rate-weighted centroid over an all-positive rate vector, which is
exactly the shape common mode swamps: every neuron firing harder moves numerator and
denominator together. The textbook fix for common mode is to subtract two pools.
DNA v57 already split the vocal module in half and computes both means every tick,
so the pools exist, and it needs no new learning rule and no new state — the
opposite of the last eight attempts on this wall. It gets an arithmetic pre-flight
before any code: a difference of two half-means is far coarser than a 14-point
centroid, so count the distinguishable F1 levels first, and if it cannot resolve the
~230 Hz that absolute naming needs, that is the answer without a run. Its refusal is
a *pair* — delivered spread must rise **while steerability does not fall** — because
an earlier result found the centroid *is* the steerability, so replacing it risks
costing exactly the controllability reward depends on.

**3. Release versus escape — free, from data already being collected.** Two things
can end a dominance period: the dominant population's adaptation building until it
lets go, or the suppressed population's adaptation decaying until it breaks through.
The first makes dominance *lengthen* with input, the second makes it *shorten*, and
they are distinguishable by whether the dominant half declines into the switch or the
suppressed half rises into it. `halfcenter` already records both half-mean time
series, so this is pure analysis — and it says which window we are in, which says
which way to move the drive in lead 1.

**4. Noise-driven alternation — the version that needs no fatigue at all.**
Moreno-Bote, Rinzel and Rubin obtain alternation from noise alone, absent without
it, and the companion paper shows a model can be moved smoothly between
adaptation-driven and noise-driven rhythmogenesis with realistic behaviour requiring
a balance of the two. This creature already carries a large noise term, so it may sit
nearer the noise-driven end — which would be a clean explanation for why a fatigue
current at the derived constant did nothing. It composes with lead 1, and the two can
be separated because `threshold` changes drive without changing noise.

**5. Taming chaos — filed as a lead on the learning rule, not on the frame.** Laje
and Buonomano get seconds-scale timing out of a recurrent network by tuning its
weights until one trajectory is stable against perturbation. It is rate-based, which
is what this creature is, and it needs neither a CPG nor a fatigue current nor a
synfire chain — all three now refused here. But it requires a *trained* recurrent
matrix, and none of the rules in this project fits trajectories. So it is gated on a
measurement rather than attempted: taming chaos presupposes rich recurrent dynamics
to tame, and the finding that no module holds a kick for 10 ms suggests there may be
no trajectory here worth stabilising. Measure the module's autocorrelation time
first; if the recurrence is that weak, this is refused without a build.

### The blind spot: velocities, not positions

A fair challenge — thirty-odd mechanisms tried and the creature still does not
talk — deserves a check of what has not been read rather than another dial. So:
**DIVA appeared in zero of this project's 168 memory files.** It is the canonical
neural model of speech acquisition, and the goal here is a talking creature. That is
a gap, not a nuance.

DIVA stands for **Directions Into Velocities of Articulators.** It maps desired
movement *directions* in auditory space into movement *velocities* in articulator
space — explicitly not positions — inverting a one-to-many mapping with the
pseudoinverse of the Jacobian.

This project maps rate to **position**. `senses.cpp:365` reads
`target_f1 = lerp(f1_min, f1_max, group_value_[2])`, where `group_value_` is the
rate-weighted centroid behind a one-pole low-pass at 800 ms.

**That is precisely why the glide experiment measured trajectory-following at 3–6%
of what the filter permits.** A position command has to be re-issued continuously to
move, and the filter fights every re-issue. The refusal was real, but it was a
refusal of a positional decoder rather than of the creature. Under a velocity
decoder a sustained velocity *is* a glide; the 800 ms constant stops being the enemy
and becomes the integrator that produces trajectories; and a common-mode component
becomes a constant drift, which is removable in a way a swamped centroid is not —
the wall seven instruments died to.

The risk is equally concrete. An earlier result here found the centroid *is* the
steerability, so replacing the decoder threatens the controllability reward depends
on, and a velocity code drifts to a clamp unless something anchors it. So it gets an
arithmetic pre-flight first: work out the rate-to-velocity gain that would traverse
250–1000 Hz within a syllable, check it against the measured free-running vocal rate,
and if the achievable velocity cannot cover the ~230 Hz that absolute naming needs
within one dwell, that is the answer with no run at all. And it is a decoder change,
so every vocal number in the project is invalid until recalibrated.

**Two more things DIVA has that this project does not.**

Targets are **convex regions**, not points. Everything here has taught toward an
exact value, and a point target's reward never saturates — however close the creature
gets, the gradient still points inward, so the lesson never stops demanding. Four
separate results are about that pathology: a tracking bar making target distance
invisible past ~0.6 log units, a bar having to chase the creature to stay
informative, an unreachable target beating a reachable one, and a reward that *sees*
target distance performing 9.4 SE worse. A region has a natural zero — reward zero
inside, graded outside — so no EMA and no tracking bar are needed at all. And a
satisfied lesson stops pushing, which aims straight at the measured interference: a
second lesson restoring the very neurons the first silenced at +9.4 SE, with
retention tracking demand on the first lesson's own axis at +11.5 SE. That is the
0.22 wipe, and it is the thing actually standing between this creature and two words.

And the babbling phase learns three mappings *in order*: region targets, then the
directional mapping, then a forward model that lets feedforward control run
independent of feedback delay. A forward model was refuted here and ships off — but
it was not built third, after a directional mapping, and not for that purpose. Worth
checking before the refutation is treated as covering DIVA's.

Third, from a different literature: **articulatory phonology**, where an utterance is
a constellation of gestures, each its own dynamical system that forms *and releases*
its own constriction. Release being intrinsic rather than imposed by a clock is a
different answer to the frame than an oscillator, which matters given the oscillator
is now refused four ways. The genome's dormant `dictionary_*` block is half that
infrastructure. But each gesture needs intrinsic dynamics over ~150 ms, and the
timescale ladder here is empty between 60 ms and 800 ms — so this one sits downstream
of the timescale gap rather than routing around it, and the dormant dictionary makes
it look cheaper than it is.

### The larynx makes a gesture

The half-center was placed inside the F1 group on a specific argument: the two halves
code low-F1 and high-F1 postures, so their alternation should move the rate-weighted
centroid and F1 should sweep. Five runs measured neural anti-phase, within-half
coherence, the amplitude envelope and F1 *means* — and none of them recorded produced
F1 as a time series, which is the output the whole mechanism was built to make.

The means had been carrying the answer the entire time. The settled-winner arms read
F1 at **438** and **813 Hz**, and the centroid extremes for a 126-neuron, nine-group
module are 437.5 and 812.5. **The two competing postures are the ends of the F1
range**, so a complete alternation is a **375 Hz neural swing** — larger than the
~230 Hz that absolute naming has been short of for this project's entire history.

| arm | neural anti-phase | alternation | produced swing | excess over control | lagged coherence |
| --- | --- | --- | --- | --- | --- |
| tau083 | −0.753 | 2.83 Hz | 47.3 Hz | −20.6 | **0.699** |
| tau167 | −0.555 | 1.25 Hz | 74.8 Hz | +6.9 | **0.709** |
| tau250 | −0.395 | 2.31 Hz | 106.9 Hz | **+39.0** | **0.597** |
| tau333 | −0.297 | 1.00 Hz | **135.6 Hz** | **+67.7** | **0.522** |
| control, both off | +0.367 | — | 67.9 Hz | — | 0.312 |
| inhibition only | +0.069 | — | 2.7 Hz | — | 0.312 |
| fatigue only | +0.097 | — | 72.4 Hz | — | 0.228 |

**The formant follows.** All four combination arms sit well above the null while both
singles sit at or below it — the interaction again, on a measure independent of
everything that established it. The sign is a prediction rather than an observation:
the half-difference is (low − high) and F1 rises when the high half wins, so
transmission has to be negative, and all four are.

Three quantities pull in opposite directions across tau and all three hold. Deeper
neural alternation gives higher coherence, because there is a cleaner signal to
follow. Slower alternation gives a larger produced swing, because more of it survives
the 800 ms pole. And the two slowest arms land on that pole's own prediction — +39.0
against 32.1, +67.7 against 73.2.

**So `tau333`, free-running, produces 135.6 Hz of F1 swing against a control's 67.9.
An extra 67.7 Hz of articulatory excursion, with no caregiver and no reward.** That is
a repeated articulatory gesture arising from the dynamics rather than from teaching,
which is a fair description of what babbling is. It is two of four arms and carries no
standard error, so it wants a fresh seed family before it is banked.

**And the statistic that nearly buried it is the most instructive failure of the
night.** The first version measured coherence at zero lag and read +0.082 / +0.014 /
+0.069 / −0.094 — a flat null, apparently. It was nothing of the kind. A one-pole
filter phase-shifts by `atan(2πfτ)`, which at these rates is 79 to 86 degrees, so a
zero-lag correlation is *capped* at `cos(lag)` = 0.195 / 0.157 / 0.086 / 0.070. The
measured values were a large fraction of the maximum attainable; `tau250` reached 80%
of its ceiling. **The filter that attenuates a signal also rotates it** — and the same
pole was being used to compute the prediction while the measurement was taken in a way
its phase made impossible. Every other instrument repair in this line caught a false
positive after the fact. This one would have produced a false negative and discarded
the result.

The finding also sharpens the decoder question rather than settling it. The mechanism
generates 375 Hz of neural swing; the positional decoder delivers 68 Hz of it at best,
and only when the alternation is slow enough to creep under an 800 ms pole. A velocity
decoder would not be fighting that filter — it would be using it as the integrator.

### The pre-flight that refused a build

With the gesture replicated, the queue said the velocity decoder was next: DIVA maps
neural activity to articulator *velocities*, this creature maps it to positions, and
the half-center generates 375 Hz of neural swing of which the positional decoder
delivers about a fifth. The build looked obvious. It gets an arithmetic pre-flight
first, because four guessed constants have each cost a run here.

The pre-flight refused it, and in two directions at once.

**With a leak, a velocity decoder is a position decoder.** Write the system out:
`f1' = k(c − 0.5) − (f1 − rest)/τ`. That is a first-order low-pass with DC gain `k·τ`
and time constant `τ` — algebraically the same object as `lerp(min, max, c)` behind a
one-pole at `smoothing_ms`, with the gain and the constant renamed. And the leak has
to be long enough to preserve a 500 ms sweep, which means seconds — *slower* than the
800 ms pole it was meant to escape. At 1 Hz, τ = 2 s passes 0.079 where the current
pole passes 0.195.

**Without a leak, it is a random walk.** A pure integrator on a noisy centroid parks
F1 at a clamp. The excursion becomes bang-bang between the limits: acceptable for
babbling, useless for holding a target, and holding a target is exactly what absolute
naming requires.

**DIVA does not have this problem because its velocity command is not raw neural
activity.** It is computed from the *error* between the current and target auditory
state, through a forward model and convex region targets. The loop is closed. Velocity
coding without an error signal is not well-posed — it drifts, or it degenerates into
the decoder already in place.

So the ordering inverts. Region targets were filed as the cheaper sibling of the
velocity decoder; they are its **prerequisite**. They are also independently motivated,
and by the thing that actually blocks two-word naming: a point target's reward never
saturates, so a lesson never stops demanding, and the measured consequence is a second
lesson restoring the very neurons the first silenced at +9.4 SE. A satisfied lesson
that stops pushing is the direct attack on that.

None of this retracts the diagnosis. The decoder is positional, the 800 ms pole is why
the glide experiment measured 3–6% of allowance, and the half-center really does
generate 375 Hz of swing that arrives as 68. The bottleneck is real. The one-line fix
was not.

### The wipe is credit assignment, not a collision

With region targets refused, the obvious next question looked like: can two lessons be
made to occupy non-overlapping neurons? It turns out that question was answered some
time ago, and answered the other way.

**They already do occupy non-overlapping neurons, and it still wipes.** An earlier
capacity experiment found two lessons competing while driving *disjoint* groups, and
the credit oracle put the reason in one line: **the groups are disjoint; the reward is
what they share.** Node perturbation nudges `bias_[i]` for every neuron in the motor
module, scaled by a single broadcast scalar — so teaching B keeps updating A's neurons
with a reward uncorrelated with anything they did, and A's setting random-walks.

That also re-reads the axis result. Retention tracking demand on the first lesson's own
axis is exactly what a broadcast reward predicts: the arms that demand A's axis are the
ones whose reward signal is most correlated with A's neurons. It looked like a
representational collision and it is a credit-assignment failure wearing that shape.

The ceiling is already measured. Confining node perturbation to a neuron range gives
retention **1.03 against 0.84** broadcast — but at about 30% of the learning rate,
because the F1 group is 14 neurons of 126 and confining the reward also confines the
perturbation search. **Perfect credit assignment converts an interference problem into
a slower-learning one.** That is the trade, and it is worth knowing before anyone
designs for it.

And the best local rule already reached those numbers once without transferring. The
commitment brake hit retention 1.10 against the oracle's 1.03 with no new state at
all — then, against the actual milestone, moved relearn 0.22 → 0.26 against seed
swings of 0.45 / −0.32 / 0.65, cost the headline retention, and dropped the babble duty
cycle from 0.78 to 0.50. It does not ship, and that verdict stands.

So the open question is narrower than the one I posed. Not whether two lessons can use
different neurons — they do. Not whether targeted credit helps — it does. **It is
whether the creature can target its own reward without an oracle.** The standard
account is gating plus stabilisation, with gating alone at 61.4% and gating with EWC at
95.4%; here the stabilisation half is refused, and the gating half has never been tested
with an index that actually separates the lessons — the one attempt read 0.23/0.23
because the ear-EMA index switches on within-trial structure rather than on which word
is playing. The one buildable combination left is to drive the reward mask *from* that
derived index, which does work at 16/18 in its own right.

Worth recording the cheapest part of this: reading the existing notes before building
saved a redundant run, and corrected a direction I had already stated out loud.

### The index that never separated now separates almost perfectly (2026-09-20)

The last section ended on one buildable combination: drive the reward mask from the
creature's own derived context index. That required an index that separates the
lessons, and every measurement said it did not — `credgate` read separation at
0.012 ± 0.003 even with acoustically distinct words.

`ctxfeat` split the failure in two. The ear's per-neuron rate EMA — the feature the
index is built on, sampled at the end of the silent tail where reward actually lands —
classifies the two words at **97.8% held-out, d′ 3.51**, on a nearest-class-mean rule
scored on samples it never fitted. Even /i/ vs /u/, differing almost purely in F2 and
the hardest case for a rate code, reads 0.929. **The feature carries the word and the
classifier throws away 98.8% of it.** The source was never the problem.

So the target became the prototype learning. Three hypotheses, in order:

**Refused.** `ctx_proto_gate` 1 lets the prototypes learn wherever the index is read,
instead of only while the larynx is quiet. Separation stayed at ~0.002 on all three
vowel pairs. *When* the prototypes learn is not the story.

**Wrong, and the instrument caught it.** The next hypothesis was a dead second
cluster: prototypes are allocated zeroed, the feature is all-positive, so slot 0 walks
to the data while slot 1 stays at the origin and loses forever. The codebase documents
exactly this failure in the episode path's own comment — DeSieno's dead unit, which an
earlier probe measured killing 78% of random inits — and source 4, built as "no episode
at all", skipped the conscience along with the episode. Before running it, the
experiment was changed to *print* p(slot0) rather than infer it, on the grounds that
inferring had already gone wrong three times in this line. It reads **0.19–0.34 under
the shipped gate**: both slots live, splitting the ticks roughly 1:3, with separation
still 0.002. A live index that carries nothing about the word is a different disease
from a dead one, and the diagnosis asserted as fact in a code comment was simply false.

**It works anyway, and on one pair it works completely.** Adding DeSieno's
win-balance penalty — the conscience, which penalises a unit for exceeding its share
of wins — to source 4's winner selection takes a-vs-i separation from 0.002 to
**0.999 ± 0.001** across six seeds. The prototype update itself is MacQueen's, each
unit the running mean of what it has won at a `1/wins` rate, so no constant is
guessed anywhere in the rule.

The first read of that was 0.814 ± 0.163 on the alternating protocol, and it should
not have been believed: **a conscience balances wins, which makes the index alternate,
and the protocol alternates the words.** Two synchronised period-2 processes agree
perfectly with no information passing between them, and at a different phase lock they
alias to a constant — which is exactly what the i-vs-u arm's p(slot0) = 1.000 with
separation 0.000 ± 0.000 looked like. One confound, both numbers, no word involved.

The test was to shuffle which word comes first within each *pair*: classes stay exactly
balanced, the period-2 lock breaks, and an index that merely alternates now agrees on
only half the pairs. Pre-registered in the code — shuffled separation must clear 0.1
and its own 3 SE.

| arm (shuffled) | separation | p(slot0) | flip | wflip |
|---|---|---|---|---|
| a-i, shipped gate | 0.035 ± 0.020 | 0.191 | 0.028 | 0.755 |
| **a-i, conscience** | **0.999 ± 0.001** | 0.500 | 0.755 | 0.755 |
| i-u, conscience | 0.000 ± 0.000 | 1.000 | 0.000 | 0.755 |

`flip` — how often the index changes slot between samples — equals `wflip`, how often
the word changes, **to three decimals**, under an order the creature cannot anticipate.
That column settles alternation-versus-information directly rather than by argument,
and it is one counter. The alternating protocol had been *understating* the effect.

**The open failure is real and is being tested, not explained away.** i-vs-u goes to a
total monopoly under the very mechanism that exists to prevent monopolies. The account:
the penalty is scaled by the mean *achieved* distance, which is the within-class spread
and therefore small, while a prototype left at the origin sits at the full ‖x‖. So the
conscience can *keep* two live units balanced and cannot *rescue* one that died at
initialisation — a-i wins that race, i-u loses it. The test removes the origin rather
than strengthening the penalty, by seeding the prototypes from data.

That test found its own bug before it ran. The first version seeded "from the first
sample", and the vacuity check refused it: gate 3 hashed identically to gate 2. At tick
zero the rate EMA *is* the zero vector, so seeding from the first sample seeds from the
origin — the fix was the bug. A second arm, seeding without the conscience, still hashes
identically to plain nearest-prototype, and that one is not a bug but the point:
seeding every prototype to the same point breaks the tie to slot 0, slot 0 becomes the
running mean of what it wins, and a mean is nearer every point than any single point is.
**Seeding alone cannot rescue a unit**, and the identical hash is the proof.

**Temper all of this before quoting it.** This fixes a carrier, not a behaviour. The
credit oracle measured what a *perfect* index converts into: +4.8 SE of retention, no
more. An earlier four-word probe holds four distinctions at a perfect index but not
four targets. The question worth asking next is not whether the index separates — on
a-vs-i it now does, nearly perfectly — but whether retention moves when it does. And
retention is not currently quotable on the context genome at all: it divides by the
gain and blew up there. That statistic needs replacing before any retention contrast
from that genome is reported.

### The geometry account dies, and the thing it was explaining turns out to be a race (2026-09-20)

Three vowel pairs in the right order looked like an explanation: the conscience took
a-vs-i to 0.999 separation, a-vs-u to ~0.5 and i-vs-u to 0.000, and a quantity called
`pcratio` — the class gap measured in units of spread along the data's top principal
component — ordered them 2.25 / 2.08 / 1.87. The reasoning was sound as far as it
went, and it is Ding and He's result rather than an intuition: k-means and principal
component analysis are the same relaxation, so the cluster assignment a prototype rule
finds is a rotation of the top principal directions. A class distinction lying off
those directions is invisible to any prototype rule however cleanly the classes
separate. And `pcratio` was *stable*, varying by 0.02–0.04 within a pair
across five gates and six seeds, where d′ wobbled more than that.

It is also three points in the right order, which happens by chance one time in six,
and a fourth point has already reversed a three-point read once in this project. So
it went to all 28 pairs of the eight caregiver words, 168 creatures.

| predictor | r with separation | t |
|---|---|---|
| pcratio | +0.600 | 3.83 |
| d′ | +0.561 | 3.46 |
| held-out accuracy | +0.543 | 3.29 |
| pcalign | +0.522 | 3.12 |

**Refused.** `pcratio` barely beats plain d′, so the variance-direction story adds
essentially nothing over "this pair is easier to hear". The pre-registered bar —
across-pair r clearly positive — passed, and passing it was not sufficient. The
control that mattered was *does it beat the trivial predictor*, and it was not written
down beforehand. The apparent dissociation on three pairs (a-vs-u at d′ 3.08 reaching
0.514 while i-vs-u at d′ 3.34 reached 0.000) was noise.

**What the sweep established instead is worth more than what it refuted.** Every
pair-level separation lands on a sixth — 0.999, 0.664, 0.514, 0.167, 0.000 — because
per creature the outcome is *binary*. Each creature either locks its index onto the
word or never does, and the pair number is simply the fraction of six that won. The
conscience does not produce a graded index quality; it produces a race, won or lost
per creature. That is why a-vs-u read 0.514 ± 0.214 rather than a tight middling
value, and it means the SE on those middle pairs was never measurement noise.

The race is decided by neither factor alone. Pair fractions span the whole 0/6 to 6/6
range, so it is not a property of the seed; intermediate fractions are common, so it
is not a property of the words. Twelve of 28 pairs never succeed on any creature, and
three succeed on all six.

Those three — a-vs-O, i-vs-E, a-vs-i, at 0.999 on every creature — are the point. The
question this whole line exists to answer is whether retention moves when the index
separates, and until now there was no index that separated. Now there are three pairs
where it does, unanimously. More geometry would be the wrong next move.

### The carrier was fixed under the wrong protocol (2026-09-21)

The claim two sections up — that the conscience takes the context index from 0.002 to
0.999 and hands the retention question its prerequisite — is **protocol-specific, and
I stated it more broadly than the evidence supported.** It holds under alternating
presentation. It does not hold under the protocol that actually teaches.

`credgate` ran the derived reward mask on the conscience index, against the same mask
on the shipped index, at 3.4M ticks and 12 creatures per arm:

| arm | gate | GAINED | EROSION | separation | agree |
|---|---|---|---|---|---|
| derived-AB | 0 | −0.0069 ± 0.0096 | +0.0355 ± 0.0093 | 0.012 ± 0.003 | 0.501 |
| derived2-AB | 2 | +0.0062 ± 0.0109 | +0.0376 ± 0.0151 | 0.084 ± 0.039 | 0.633 |

Erosion is identical with `GAINED` matched, so the mask buys nothing — refused on the
criterion registered before the run. But the diagnostic column is the separation:
**0.084, not 0.999.** The conscience improves the index sevenfold and lands nowhere
near a usable carrier. Agreement rises from 0.501 to 0.633, which is the same story in
a different unit: better than chance, far from the 1.000 the oracle has by
construction.

The new `GAINED`/`EROSION` pair earned its place here. The ratio statistic on this
genome reported `oracle AB −5.166 ± 5.263` and `oracle keep −7.468 ± 10.419` — noise
wearing the costume of a measurement, because the denominator is a gain of 0.0135. The
undivided numbers are small, stable, and say plainly that nothing moved.

**The mechanism is one this project already wrote down and I did not connect.** An
earlier section names it while chasing a different asymmetry: *"MacQueen's `1/wins`
means the prototypes FREEZE — after N words the learning rate is 1/N."* credgate
teaches lesson A for roughly the first third of 3.4M ticks before lesson B ever
sounds. By then `ctx_wins_` is around 10⁶ and the update rate is about 10⁻⁶, so the
prototypes physically cannot move to accommodate a second word. Under `ctxpc` the two
words alternate from the start and both are learned while the rate is still large.

So the conscience fixes **who wins**, and cannot fix **when the prototypes are still
plastic**. Those are different failures and the conscience addresses only the first.
The result from last night stands exactly as measured and means less than I said it
did: an index that separates two words is not the same thing as an index that can
*acquire* a second word introduced late, and teaching only ever introduces words late.

That is a testable claim rather than a story, and testing it does not need another
2.7-hour run: playing word A alone for the first half of a 600k-tick session and then
alternating should reproduce credgate's collapse at a sixth of the cost. It is
pre-registered to refuse itself — if the late arm still separates near 0.999, the
freeze is not the reason and the two protocols differ for some other cause.

### DNA v60 refused, and the wall has a name (2026-09-21)

A floor under the prototype learning rate was the obvious fix for the freeze. It is
refused, and the way it fails is more useful than the fix would have been.

| arm | index separation |
|---|---|
| alternating, no floor | 0.999 ± 0.001 |
| late word, tau 3 s | 0.000 ± 0.000 |
| late word, tau 1 s | 0.000 ± 0.000 |
| late word, tau 300 ms | 0.011 ± 0.004 |
| **alternating, tau 1 s** | **0.480 ± 0.019** |

At every rate that demonstrably moves the prototype, a word introduced halfway
through is never acquired. And the floor does not merely fail to help: at tau = 1 s
it takes the protocol that *worked* from 0.999 to 0.480.

**The usable region is empty and both its walls are now measured.** From below, the
update is `proto += lr * (x − proto)`, so with `|proto|` of order 1 the increment
vanishes into float32 whenever `lr × |x − proto|` falls under eps = 1.2 × 10⁻⁷ — tau
30 s and 300 s are bit-identical to the floor being switched off, at 600k ticks, with
the field verified as loaded from inside the kernel. From above, a prototype whose
time constant approaches the feature's own 1-second EMA window stops being a class
mean and starts tracking the instantaneous feature, which is what the 0.480 is. The
one point between those walls does not rescue the late arm.

That makes this structural rather than a tuning failure. **A word introduced late
cannot be acquired by this prototype rule at any rate that also keeps the prototype a
class mean** — a rate fast enough to learn the new one is fast enough to erase the
old. That is Grossberg's stability–plasticity dilemma in its textbook form, and it
says the fix is not a rate.

Two directions follow from the literature, and both are about *allocation* rather
than speed. Carpenter and Grossberg's adaptive resonance answers it with a vigilance
test: an input far enough from every existing prototype commits a *new* unit instead
of dragging an old one, so learning something new never costs what is already stored.
Platt's resource-allocating network is the same idea for function approximation.
Either would mean the context module stops being k-means with a fixed slot count and
starts deciding when a word deserves a slot of its own — which is also, finally, a
mechanism for the vocabulary question rather than a mechanism for two words.

The measurement that got here looked like it might reach further, so it was audited
rather than left as a worry. Every `+= lr * (target − current)` update in the kernel:

| update | rate | |
|---|---|---|
| `rate_ema_` | 1/1000 | ~8000× eps |
| `rate_fast_` | 1/50 | safe |
| `pool_fast_` | 1/15 (interneuron tau) | safe |
| `burst_base_` | `burst_baseline_tau_ms = 0` | ships off |
| `syn_elig_mean_` | `elig_baseline_tau_ms = 0` | ships off |
| `meta_m1_`, `meta_m2_` | `meta_window = 0` | ships off |

**Clean: no existing result is invalidated.** And the reason is exact rather than
lucky. Every one of those is a *fixed* alpha, and the prototype rule is the only
update in this kernel whose rate **decays without bound** — MacQueen's `1/wins` walks
down through the danger zone as a run proceeds and then stays there. A constant alpha
is either always above eps or always below it, and the shipped ones are all far
above. So the hazard is specific to the one rule that has no floor, which is the rule
this section is about. It is still worth knowing for anything that later sets one of
those three taus long, or adds another decaying rate.

### The context machinery has been taxing teaching all along (2026-09-21)

The vigilance index was refused on the real teaching protocol — separation 0.001,
below even the conscience index it was meant to beat. But refusing it exposed a
baseline that had never been run, and the baseline is the finding.

Every `credgate` arm ever run set `context_slots = 2`. So the arm this experiment
calls its *broadcast baseline* is a creature whose learned bias is split across two
tables by an index that flickers on nothing — p(slot0) between 0.19 and 0.34, with a
measured lesson separation of 0.012. Running it with the machinery switched off
entirely, and alongside vigilance's near-constant index:

| arm | index | gained | AB−keep gap |
|---|---|---|---|
| broadcast | 2 slots, flickering | +0.0193 | +0.0933 |
| context off | none | +0.0553 | +0.1760 |
| **vigilance, unmasked** | **2 slots, near-constant** | **+0.0640** | **+0.0436** |
| oracle (masked) | perfect | +0.0135 | +0.0303 |

**The tax is real and large.** Both alternatives teach about three times better than
the flickering baseline (err_taught 0.96 against 1.01). Roughly two-thirds of the
teaching gain has been going to an index that carries no information about the
lesson — and every context-genome number in this project was measured through it.

**But "context is harmful" is the wrong conclusion**, which is why the third arm
matters. Switching the machinery off gives the *worst* interference gap of anything
measured here, nearly double the flickering baseline. Context is not a tax being paid
for nothing; it is buying interference resistance and being charged for it in
teaching gain.

**And vigilance gets both.** It gains more than context-off *and* loses four times
less — an ordering that rules out the obvious deflation, since a smaller gap is
usually just the mark of having learned less. This one learned more and kept more,
and its gap is the closest any unmasked arm has come to the oracle's.

**There is no mechanism for that yet, and the honest thing is to say so.** With index
separation at 0.001 it cannot be context-indexed credit assignment. If the index were
simply *constant* the creature should behave like context-off, and it emphatically
does not. Three accounts were proposed and refuted in this line already — a dead
cluster, a variance geometry, a learning-rate floor — each of which looked obvious
before it was measured. So the next run prints what the index actually did during
teaching rather than inferring it, which is the same correction that caught the dead
cluster: `p(slot0)` during the lesson against `p(slot0)` during the gap.

Also worth stating plainly: the masked arms are *worse* than the unmasked one on the
same brain. Vigilance with the derived mask gained 0.0211 where the identical brain
unmasked gained 0.0640. The mask costs learning rate, which the credit oracle priced
at ~30% long ago, and the low erosion it appeared to buy was substantially just
having learned less. That is the AB-minus-keep confound in a new costume, and it was
caught only because the unmasked control was run.
