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
| **M3** cross-modal association | **built and measured, G3 not met — and now settled rather than open.** The baby echoes a word it hears (0.75) and the seen object *does* now reach the larynx (0.654, up from 0.515, via `vision→vocal`), yet naming still reads taught−random **−0.014** with 0 of 5 creatures over the 0.75 bar. Delivery is no longer the limit; conditioning is |
| **M4** growth and sleep | **done** — **G4 passes**: the creature never grows while it is still learning, grows only on a detected plateau, and never passes the DNA cap; myelination, pruning and replay all run |
| M5 embedded | **started, and the first measurement moves the goalposts.** `--experiment footprint` reports what nothing printed before: the shipped genome needs **108.70 MB**, against an ESP32's 520 KB of SRAM and an ESP32-S3's 8 MB ceiling with PSRAM — **13.6× too large**. 99.1% of that is the synapse pool, sized `n_max × max_out_degree`, and the creature wires **0.96%** of it. Tightening `max_out_degree` is free and verified bit-identical (108.70 → 50.70 MB); the rest is `n_max`, which is also the growth cap and re-rolls the wiring |

Every number on this page comes from the genome in [dna/default.toml](dna/default.toml)
as it currently stands. **The whole suite above passes except `m3`** — G1,
calibrate, babble, audio, vision, M2, G2, sleep, G4 and snapshot are all green,
and G3 is the one milestone still open. Shipped hash `ad96f882becbee92`;
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

**That makes M5 not a rewrite. It does not make it a build question, which is
what this paragraph claimed until 2026-08-29.** The portability half is real —
core includes `stddef.h`, `stdint.h`, `math.h` on the build path and
`<type_traits>`, with no STL container, no allocator, no I/O and no thread. The
*fit* was never measured, and the creature is **13.6× larger than the biggest
part it was meant to run on**. See `--experiment footprint`, and the M5 row in
[Where the project stands](#where-the-project-stands). M5 is a provisioning
question: the arena is sized for a worst case the genome permits and the
creature uses under one percent of it.

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

**Everything else.**

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
- **Oriented receptive fields (DNA v7).** Hubel, D. H. & Wiesel, T. N. (1962).
  *Receptive fields, binocular interaction and functional architecture in the
  cat's visual cortex.* Journal of Physiology 160, 106–154.
  <https://doi.org/10.1113/jphysiol.1962.sp006837> — the curvature stage of DNA
  v8 is V4's computation at V2's position in the hierarchy.

[bp]: https://www.nature.com/articles/s41593-021-00857-x
