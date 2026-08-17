# AI Baby — Requirements

**Version:** 0.2
**Status:** Draft for implementation

---

## 1. Vision

Build a creature you raise. It is born from a compact **DNA** that defines only its initial
brain structure — a few hundred neurons, some wiring rules, and a handful of drives. Everything
else it becomes comes from what happens to it: what it sees through your camera, what it hears
through your microphone, and whether you tell it that it did well.

Think of it as the next generation **Tamagotchi**. The original one faked an inner life with a
state machine. This one actually has a brain that changes shape while you're not looking, and
two babies raised differently will end up genuinely different.

There is no pretrained data anywhere in the system. Nothing is downloaded. A fresh baby knows
nothing and can do nothing except twitch, babble, and want things.

### Non-goals

Stating these plainly so nobody is disappointed later:

- This is **not** a language model and will not converse. It learns to associate sounds with
  what it sees, at roughly the level of a pre-verbal infant.
- It will **not** reach human-level anything. The realistic ceiling is cross-modal association
  and a handful of learned vocalizations.
- No pretrained weights, no transfer learning, no cloud inference. If it can't be learned from
  the user's own interactions, it isn't in scope.

---

## 2. Measurable goals

Replacing "emulate the human brain as well as possible" with things we can actually test.

| # | Goal | Success criterion |
|---|------|-------------------|
| G1 | Brain structure derives entirely from DNA + experience | Same DNA + same interaction journal ⇒ bit-identical brain state |
| G2 | The baby learns from reward | Within one session, rewarded vocalizations increase in frequency measurably above baseline |
| G3 | Cross-modal association | After ≤2000 labeled presentations, a held-out classifier distinguishes the baby's vocal output for object A vs object B at ≥75% (chance = 50%) |
| G4 | Structure grows only when needed | Neuron count stays flat while error is improving; grows only on a detected plateau; never exceeds the DNA budget cap |
| G5 | Portable and small | Core compiles for ESP32-S3 and runs frozen inference within 8 MB PSRAM |

G3 is the headline result. If we hit it, the project succeeded.

---

## 3. Resolved design decisions

These were open in v0.1. They are now settled; changing any of them is a redesign, not a tweak.

### 3.1 Learning rule — reward-modulated STDP with eligibility traces

Spike-timing-dependent plasticity does **not** write to the weight directly. It accumulates into
a per-synapse **eligibility trace**:

```
pre fires before post  →  e_ij += A₊ · exp(-Δt / τ₊)     (potentiating)
post fires before pre  →  e_ij -= A₋ · exp(-Δt / τ₋)     (depressing)
e_ij decays continuously with τ_e
```

The weight only changes when a reward signal arrives:

```
Δw_ij = η · e_ij · R(t)
```

where `R(t)` is total reward (external + intrinsic, §3.3). This is three-factor learning.

**Why this rule:** `τ_e` is set to ~2 s, which is roughly how long it takes a human to say
"good baby!" after the baby did the thing. That delay is fatal for plain backprop and is handled
natively here. It also works with arbitrary topology and recurrence, which the growth model
requires.

**Homeostasis is mandatory.** Spiking networks without rate regulation either fall silent or
saturate within minutes. Every neuron runs intrinsic plasticity toward a DNA-specified target
firing rate, plus synaptic scaling to keep total input weight bounded.

### 3.2 "Multi-dimensional" = spatial

Neurons occupy real coordinates `(x, y, z)` in a per-module volume. This gives us, for free:

- **Connection probability** falls off with distance — sparse wiring without hand-tuning
- **Axonal delay** proportional to distance — real temporal structure for STDP to work on
- **Growth** = inserting a neuron at a coordinate, near where the error is
- **Myelination** = a physical property of a path, not an abstraction

### 3.3 Drives and intrinsic reward

Without internal motivation the baby sits at a fixed point and never explores. External reward
is far too sparse to bootstrap anything. Four drives generate reward continuously:

| Drive | Behaviour |
|-------|-----------|
| **Hunger** | Rises on a timer; feeding resets it. Sustained hunger = negative valence |
| **Comfort** | Tickle/stroke = positive. Poke = mildly negative. Sustained poking = strongly negative |
| **Curiosity** | Reward ∝ **reduction** in prediction error over a window — i.e. learning *progress* |
| **Fatigue** | Rises with activity. Triggers a sleep state (§3.6) |

> Curiosity rewards learning progress, **not** raw prediction error. Rewarding raw error makes
> the baby stare at noise forever (the "noisy TV problem") — static is maximally unpredictable
> and teaches nothing.

Total reward: `R(t) = w_ext·R_external + w_hunger·ΔHunger + w_comfort·ΔComfort + w_curiosity·ΔProgress`
with all weights from DNA.

### 3.4 Growth — with guardrails

Growth without limits masks bugs: the network expands instead of revealing that learning is
broken. Every growth rule is paired with a decay rule.

**Trigger** — all three must hold for a module:
1. Reward/error has **plateaued** over window `W` (no improvement beyond ε)
2. The module is **saturated** — mean firing rate high, weights near bounds, little headroom
3. Neuron count is below the DNA budget cap `N_max`

**Action:** insert `k` neurons at the spatial centroid of the highest-error region, wired to
local neighbours with small random weights.

**Pruning:** synapses whose `|w|` falls below threshold *and* whose traffic is negligible decay
to zero and are removed. Neurons left with no surviving connections are removed. Runs during
sleep, never mid-interaction.

### 3.5 Myelination — usage-based consolidation

Each edge keeps a leaky traffic counter.

- High traffic → axonal **delay decreases** toward a floor (the signal gets faster)
- High traffic → per-edge **learning rate decreases** (the pathway consolidates and is protected
  from being overwritten)
- Traffic decays → both revert, and the edge becomes prunable again

This is what turns a frequently-used route into a highway, and it doubles as our defence against
catastrophic forgetting.

### 3.6 Sleep

When fatigue crosses threshold the baby enters a sleep state: sensory input is gated off, and
the system runs pruning, synaptic downscaling, and replay of high-reward episodes from the
journal. Biologically apt, and it gives us a safe window for structural surgery.

---

## 4. Brain modules

Each module is an independent population with its own spatial volume. Modules connect to each
other — output neurons of one project into input regions of another, with the same
distance-based rules applied across module boundaries.

| # | Module | Direction | Description |
|---|--------|-----------|-------------|
| B1 | **Central** | assoc. | Receives already-processed activity from all other modules. No direct sensor access. Where cross-modal association happens |
| B2 | **Auditory** | input | Microphone → FFT → mel filterbank → spike encoding |
| B3 | **Vision** | input | Camera → foveated sampling → center-surround features → spike encoding |
| B4 | **Somatosensory** | input | Touch events from the UI: feed, poke, tickle, stroke. *(Was miscategorised as an output module in v0.1 — the user acts on the baby, so this is afferent.)* |
| B5 | **Vocal motor** | output | Drives the formant synthesiser (§5.3) |
| B6 | **Expression** | output | Drives the visible avatar state — the actual output channel for how the baby feels |

---

## 5. Sensory and motor encoding

The data budget is the hard constraint on this project. A real infant gets years of dense
multimodal input plus enormous evolutionary priors; we get a few thousand interactions from one
person at a laptop. Every channel below is deliberately shrunk so that learning is *visible*
and we can tell a bug apart from insufficient data.

### 5.1 Vision

- 64×64 grayscale, **foveated**: center 16×16 at full resolution, periphery progressively
  downsampled
- Difference-of-Gaussians (center-surround) ON/OFF filters — retina-like, not raw pixels
- Latency coding: stronger response ⇒ earlier spike

### 5.2 Audio

- 16 kHz mono, 512-sample windows, 50% overlap
- FFT → **24-channel mel filterbank** → log compression

  > The mel/log spacing is what the cochlea actually does and cuts dimensionality by ~50×
  > versus raw FFT bins. Non-negotiable given the data budget.

- Per-channel rate coding into B2

### 5.3 Vocal output — source-filter, not samples

The network **must not** generate audio samples. It emits ~8 continuous parameters at 100 Hz,
population-coded from B5 motor neuron groups:

`F0 (pitch) · voicing on/off · F1,F2,F3 formant frequencies · F1,F2,F3 bandwidths · amplitude`

These drive a glottal-pulse-plus-formant-filter synthesiser on the host. Eight parameters versus
16,000 samples/second is the difference between learnable and impossible. Babble emerges
naturally from motor noise shaped by the curiosity drive.

---

## 6. Architecture

```
Browser  ──WebSocket (binary)──►  Host process (C++)  ──►  libaibaby (core)
  camera, mic, UI                  sensors, journal,          LIF sim, STDP,
  avatar, synth                    persistence, WS             growth, drives
```

### 6.1 Core — `libaibaby`

- **C++17**, no exceptions, no RTTI, no STL in the simulation kernel
- **Arena allocator**; zero heap allocation in the hot path
- Scalar type templated so we can swap float → fixed-point for the ESP32 build
- Event-driven leaky integrate-and-fire neurons (spiking is required for STDP)
- Completely headless — no I/O, no threads, no platform calls

### 6.2 Host layer

Sensors, WebSocket server, journal and snapshot persistence. Thin.

### 6.3 Browser client

`getUserMedia` for camera and mic, Web Audio for the synthesiser, canvas for the avatar and
brain telemetry. Sends downscaled frames and PCM as binary WebSocket frames; receives synth
parameters and state.

> **Node is a relay, not a host.** Keeping the core in its own process rather than an N-API
> addon means no event-loop blocking, and the exact same binary runs headless on device.

### 6.3.1 The eye port — a moving eye that is not the browser's

The retina points itself (DNA v31), and the body it points is not necessarily a crop window
over a webcam. The seam is drawn between the two halves that were never one thing: **where to
look** stays in the creature, **how the eye gets there** belongs to whoever owns the hardware.

- `Retina::gaze_command()` publishes the wanted position after every frame, in pixels and in
  fractions of the frame. Fractions are the device unit: multiply by field of view for degrees
  and never learn the retina's resolution.
- `Retina::report_gaze()` takes the position back, optionally echoing the command sequence
  number it reflects, from which the host reports the loop's dead time in frames.
- `Retina::EyeMount` says which side aims. `kInternal` slides the sampling window; `kExternal`
  means the frame already arrives aimed and the window must not slide as well.

Data in and data out, with no callback: an integrator polls it from a loop they already have,
and nothing in the retina calls into their code at a moment they did not choose. The same three
operations exist over the WebSocket as `eye`, `gaze` and `look`, so the device may equally be
out of process — see §9.

### 6.4 Portability discipline

The ESP32-S3 has ~512 KB SRAM and 8 MB PSRAM. It **cannot** host camera + mic + FFT + a growing
network + WiFi at a scale where learning is observable. Therefore:

- **Train on desktop. Deploy frozen inference to the S3.**
- The core is built against ESP-IDF with stubbed sensors **from week one**, in CI. Discovering
  portability rot at month six is the failure mode we're avoiding.

---

## 7. DNA

A versioned, human-readable declarative file (TOML) that compiles to a compact binary blob for
embedded use. It specifies:

- **Seed** for the PRNG (xoshiro256++)
- Per-module: neuron count, spatial extents, connection radius and density
- Neuron parameters: threshold, leak time constant, refractory period, target firing rate
- STDP: `A₊`, `A₋`, `τ₊`, `τ₋`, `τ_e`, `η`
- Growth: plateau window `W`, tolerance `ε`, insertion count `k`, budget cap `N_max`
- Drives: rate constants and reward weights (§3.3)

**Determinism is a hard requirement.** Same DNA + same journal ⇒ bit-identical brain. This means
a single-threaded core by default, strictly-ordered accumulation, and a seeded PRNG. It is our
only real testing lever, and it's easy to lose by accident.

---

## 8. Persistence

Two artefacts, both required:

1. **Brain snapshot** — versioned binary, full structural + weight state. Written on sleep and
   on shutdown.
2. **Interaction journal** — append-only log of every sensory frame, touch event, and reward,
   timestamped.

Without the journal we cannot re-run an experiment and would be reduced to tuning by vibes. It
also feeds sleep replay (§3.6). Cheap to build now, painful to retrofit.

---

## 9. Interface

- Live camera and microphone, with an explicit permission and mute state
- An avatar reflecting B6 expression output and current drive levels (hunger, comfort, fatigue)
- Interaction: **feed**, **poke**, **tickle**, **stroke**
- Explicit feedback: good / bad, mapped to `R_external`
- Push-to-talk for speaking to the baby
- **Telemetry panel** — live neuron count per module, firing rates, reward trace, growth and
  prune events, and where the eye is pointing. This is a debugging necessity, not a
  nice-to-have; without it a failure to learn is indistinguishable from a crash.

### 9.1 The eye, over the wire

Three text commands and one telemetry block, so a moving eye can live outside this process
(§6.3.1). Positions travel in either unit and either may be omitted: `fx`/`fy` are fractions of
the frame and are what a device should speak; `x`/`y` are pixels from the frame centre.

| message | meaning |
|---|---|
| `{"cmd":"eye","mount":"external"\|"internal","timeout":5}` | which side owns the actuator, and how many frames of silence count as a lost eye |
| `{"cmd":"gaze","fx":0.12,"fy":0,"seq":41}` | feedback: where the device says the eye actually is, echoing the command it was acting on |
| `{"cmd":"look","fx":0,"fy":0}` | steering from outside: an attention system or a caregiver overriding the reflex |

Telemetry carries `gaze`: `x`/`y` where the eye is, `cx`/`cy` and `fx`/`fy` where it has been
told to be, `seq`, `moves`, `mount`, `stale`, `lag` in frames, `reports`, and `stalls` — re-aims
suppressed because the device went quiet.

Two rules the implementation enforces rather than documents. A report to an internal eye is
**refused and not counted**, so a client that forgot to set the mount sees `reports` stay at
zero instead of quietly losing a fight with the servo model. And staleness is counted in
**retinal frames**, not wall-clock: with the camera stopped the controller is not acting, so
there is nothing to be late for.

---

## 10. Milestones

| # | Deliverable | Done when |
|---|-------------|-----------|
| **M0** | Skeleton | DNA parses, LIF network simulates, journal records, browser connects, telemetry renders |
| **M1** | **Closed audio loop** | Mic → mel → B2 → B1 → B5 → formant babble → user reward → weights change. Rewarded vocalizations measurably increase within a session (**G2**) |
| **M2** | Vision | Camera → B3 → B1. Baby discriminates object present vs absent |
| **M3** | **Cross-modal association** | Cube vs ball produce distinguishable vocalizations (**G3** — the headline result) |
| **M4** | Growth and sleep | Plateau-triggered growth, pruning, sleep consolidation all validated against **G4** |
| **M5** | Embedded | Frozen brain runs inference on ESP32-S3 (**G5**) |

M1 is the smallest slice that exercises the entire architecture — encoding, spiking, eligibility
traces, delayed reward, and motor output. If M1 works, everything after it is extension rather
than redesign.

---

## Appendix A — Assumptions carried into this revision

Flagged for review; each was an ambiguity in v0.1 resolved in one direction.

1. **"Multi-dimensional" was interpreted as spatial coordinates** (§3.2). It is the reading that
   makes growth, distance-based delay, and myelination coherent as one mechanism.
2. **The physical feedback module was reclassified as sensory input** (B4), and a separate
   expression module (B6) was added to carry the output role it seemed to be reaching for.
3. **Slower learning was accepted** as the cost of STDP over backprop, on the grounds that
   delayed human reward and arbitrary topology are both load-bearing requirements.
4. **ESP32-S3 was scoped to inference only.** Training on-device is not achievable at a useful
   scale, and pretending otherwise would distort every other decision in the document.
