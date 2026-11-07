# What this creature does, what it cannot, and the one reason why

Written 2026-11-04. This is the **conclusion**: the shortest honest account of what the project
established. The other three documents in this repository have different jobs — the root
`README.md` is the chronological *record* (26,000 lines, the full reasoning for every entry),
`docs/discoveries.pdf` is the curated *findings* with an auditable inclusion criterion, and
`docs/ROADMAP.md` is the *forward* view and the production specification. None of them states the
conclusion, and the one document that tried — a page titled *"Repeats But Cannot Name"* — is now
wrong in its title. The creature names.

Every number below comes from a run gated on `--experiment verify` reporting `PASS` against the
pinned determinism hash `ad96f882becbee92`. Sample sizes are creatures, not trials. Where a
number has a sibling in the record that reads differently, the difference is a genome revision or
a corrected estimator, not a disagreement; the record carries both.

## The goal

A creature that learns speech, recognises words, and builds intelligence out of what it hears and
what it is told — not at an assistant's level. It is a spiking network of **six modules** — central,
auditory, vision, somato, vocal, expression — seeded at **1,102 neurons** and growing from there,
with a cochlea, a larynx that reads nine scalars, an eye, and a caregiver. It learns by
reward-modulated plasticity and node perturbation. There is no gradient descent anywhere in it.

## What it does

Six capabilities, each with a pre-registered bar, a control, and an instrument in the record.

**It recognises words.** The auditory module reads one of eight words at **0.981** against a 1.000
signal ceiling (`coderprobe`). The association module reaches **0.940** on which-word at 1.2 s of
integration and 0.700 at 200 ms, chance 0.500 — but it *integrates*, it does not hold: after the
word stops it is at **0.503**, chance. Recognition is strong and momentary.

**It repeats.** 200–600 ms after a word stops, with the ear already down at 0.534, the voice still
carries which word at **0.890**, 5 of 5 creatures against a 0.75 bar, at an audible **d' 1.37**
against the 1.0 a listener needs (`imitate`, M1b). Articulators alone — loudness and voicing
dropped — read 0.778, so it is not passing on "one word is louder". All six vowel pairs clear the
bar, including /i/–/u/ at 0.900, which differs almost purely in F2 and so cannot be answered by a
brightness meter. At 0 dB SNR, noise as loud as the word, it still reads 0.807.

**It can be taught a sound.** The caregiver says /a/ and praises toward /i/ — a vowel the creature
never hears — and the voice moves toward the target on 3 of 3 seeds at an audible **d' 5.57**,
against a yoked arm's 1.08 and a −0.012 null (`teachsound`, M1c). The yoked arm is the point: the
voice drifts on its own, and on one seed that drift alone clears the audibility bar. The claim is
that the taught change is 3–4× the drift on every seed, with the error moving toward the target
only in the taught arm.

**It answers.** Corrected answering burst **+0.349 on 6 of 6 seeds** (range 0.315–0.366) against a
+0.05 bar, measured above the creature's own quiet tail with a silent arm to remove the trial clock
(`turntake`, M1d). The first version of this milestone read −0.021 and was published as a negative;
the design lesson it produced — *ask what would pass this milestone if the creature were inert* —
is why the positive is believable.

**It names, conditionally.** Four words laid out on both formant axes, on a context index the
creature derives from its own ear, beat the same creature's no-index control by **+0.160 ± 0.032
(5.0 SE)** at n=18 (`ctxgrid`). Nine words name as well as six — total naming excess conserved at
0.6 SE, with the whole cost paid in F1 reach (−11.8 Hz, 3.3 SE) (`ctxvocab`). When the targets sit
inside the creature's measured reach, naming reads **0.958** (`orthoname`). And one individual does
both halves in one session: names four words at **+0.1790 ± 0.0197 (9.1 SE)** while still echoing
which word it just heard at **0.955**, n=36 (`echoname`) — with the caveat its own yoked arm forced,
that what raises the *echo* is reward rather than the lesson, a yoke that learned nothing recovering
82% of the excess.

**It teaches another creature what it knows.** A pupil hearing only its tutor's recorded vowels,
with no host target, acquires the tutor's word-to-vowel convention at 5.3 SE, and a lineage shows
no detectable loss over four generations (1.4 SE), with arbitrary conventions locking in even when
they began as a near-tie. A four-signal two-bit code crosses the air on an index each pupil derives
for itself, inherited in **18 of 18 lineages on both bits**, z = +4.24 each (`tutorgrid`). So the
ceiling on what a lineage can say is the ceiling on what one creature can produce.

It is also small, and measured rather than asserted: deleting vision, somato and expression — three
of the six modules, a third of the seeded neurons — leaves a grown creature of **782 neurons and
21,213 synapses** with naming intact (−0.0010 ± 0.0255 at n=72, stated as a bound), at
1.412×10⁶ operations per simulated second, with **no pass over synapses in the per-tick path**.
Operations were never the power bill; being awake is.

## What it cannot do

**It cannot name what it sees.** Binding a *perceived object* to a spoken name — the project's
original third goal — does not happen, and this is the one capability that resisted everything.
Read out of the voice, the heard word classifies at 0.86 and the seen object at 0.58. As a *sound* —
on a ruler whose own bias is subtracted analytically, because the first version of it had a null
above its signal — cube versus ball is **d' 0.40** against a 0.00 null and a 1.0 audibility bar: a
real separation that no listener could use.

Three facts make this a finding rather than a failure to tune.

**The object arrives.** In the vocal module's own activity it is present at **0.660** per neuron
across three seed families — better than the association module's 0.600, because `vision→vocal`
bypasses it. Every note in this project's history saying "the object never reaches the larynx" was
measured before that tract shipped. Delivery is not the blocker.

**An ideal teacher does not help.** With the condition present at vocal and a dense, immediate
teacher that says which object this is, reward-modulated STDP learns exactly nothing: taught 0.637
versus random 0.639. Delivering the condition by *ear* instead of by eye raises its presence at
vocal to 0.732 and changes the outcome not at all. Re-derived on 2026-11-04, 52 genome revisions
later, because a standing null is worth re-measuring rather than re-explaining: **+0.0284 ± 0.0359
(0.8 SE)** and **+0.0028 ± 0.0210 (0.1 SE)** on the two instruments. It replicates.

**And the blocker is conditionality itself, not the readout.** This is the tightest measurement in
the project, and it is the one that closes the line. Take G2 — *rewarded vocalisations increase* —
which is a **met** milestone (×1.35, 23 of 27, 9 of 9 at 420 s), so "reward can shape this" is a
result and not a hypothesis. Copy its contingency exactly: the act is a voiced frame, the criterion
is the creature's own baseline median so both signs occur by construction, the control has matched
marginals, and the readout is a binary class boundary rather than the population centroid that
three earlier mechanisms had been built to fix. Change one line — *which class is praised depends
on the word just heard*.

| the same contingency | what reward buys |
| --- | --- |
| unconditional (G2's own measure) | **+0.369 ± 0.031**, hit rate 0.500 → 0.88, twelve sigma |
| made conditional on the heard word | **+0.022 ± 0.153** |

Per seed the conditional arms read +0.252 / −0.335 / +0.097 and −0.124 / +0.352 / −0.074: both
signs, no consistency. Reward moves this readout from half to nine tenths and still cannot be made
to depend on the input.

## The finding

**This architecture can learn a bias per context. It cannot learn the context.**

That single sentence accounts for both sections above, and it is why the capability list and the
refusal list can both be long.

> ### AMENDED 2026-11-07 — THE SECOND CLAUSE IS FALSE FOR VISION
>
> This is a correction to the finding above, not a second synthesis. ROADMAP §49 and §50:
>
> * **§49 — it learns the context.** DNA v84 points the shipped competitive-partition rule at the
>   retina (`context_source = 5`). An **unsupervised** partition recovers which object is shown at
>   `ctx_match` **0.8624 against a measured null of 0.5346**, where the ear — the sense that already
>   names — reads 0.7674 against 0.5233. **The visual index is the better of the two.** It survives a
>   drawn probe order (97%), and needs DeSieno's conscience plus position and size held.
> * **§50 — it then names off it.** With that self-derived index driving the per-context bias table,
>   an idealised teacher takes the voice to **59% of the separation it asked for (+0.5875 ± 0.1500)**
>   against **2% with the table off** and −0.03 with the index label-destroyed. RET − SHUF = +0.6798,
>   **3.0 SE**, 11 of 12 creatures positive.
>
> **The structural impossibility argument above survives, but it is narrower than it reads.** "A
> per-neuron bias is a constant, not a function of the input" is true of the *shared* bias `bias_`.
> It is **not** true of `bias_ctx_[i * slots + active_ctx_]`: the delivered bias is a function of the
> index, and with `context_source = 5` the index is a function of the input. The composition is
> conditional, and no new mechanism was added to make it so — §49's arms were already running this
> table and nothing had read the voice off them.
>
> **`g3probe`'s null is therefore not a fact about the learning rule.** *"A perfect teacher cannot
> make this voice depend on what the creature sees"* was measured at `context_slots = 0`; it was a
> statement about a **missing index**. Cite §50 beside it.
>
> **Why vision and not hearing.** `ctxsrc` named the obstruction and it is **persistence**: a word is
> at ceiling (1.000) while it plays and at chance (0.533) by the reward window 800 ms later. **A toy
> in view does not stop** — the retinal index reads **0.8400 in the reward window against 0.8517 at
> the probe.** The old lesson "there is nowhere to hold a context" should be carried forward as "a
> **transient** context has nowhere to be held."
>
> **What is still missing, and it is not the index.** The teacher is **idealised** — dense, immediate,
> and it says which object this is. The creature now supplies the *question*; the teacher still
> supplies the *answer*. Two objects, position and size held. **This is not autonomous word
> learning**, and the open lead is the picture→sound→voice teaching signal.

*Where the conditioning variable is supplied, conditional production works.* The shipped context
mechanism computes an index from the ear's own rate EMA — no gate, no episode, no latch, and no
word-boundary oracle — and reaches **100.1%** of the lift a host-supplied perfect index gives. The
creature then learns one bias per index, which is a *fixed-target* lesson repeated, and that it can
do. Nine words, two formant axes, inherited across generations.

*Where the creature must learn which input goes with which output, it returns zero.* Node
perturbation cannot do it **structurally**: a per-neuron bias is a constant, not a function of the
input, so there is no parameter setting at which it becomes conditional. Reward-modulated STDP on
the tract that carries the object is measured at zero with an ideal teacher. And `g2cond` shows the
obstacle is not the readout, the nine scalars, or the motor plant, because it reproduces on a
readout reward demonstrably can move.

Read that way, "Repeats But Cannot Name" was the wrong title. The creature repeats *and* names.
What it cannot do is *acquire a new conditional mapping by reward* — and when the architecture hands
it the condition, naming is one of the things it does best.

## What is closed, so it is not proposed again

Each of these was built or measured, not argued away; the record carries the run.

- **The readout.** Three mechanisms (a shape bank, a posture dictionary, a categorical policy
  gradient) were spent on the hypothesis that the population centroid was the problem. `g2cond`
  reproduces the failure on a binary class boundary and retires the line.
- **Delivery.** The object is at vocal at 0.660. A signed-projection relay, correctly derived and
  label-free, was built anyway: delivery stayed flat across a 4× sweep while the voice declined
  monotonically. There was no buried code left to recover.
- **Noise.** Two independent denoisings — halving the depression term, and moving perturbation onto
  synapses (irreproducible noise 65% → 8%) — leave the object-specific part unmoved. It is not
  being hidden.
- **Coherence.** The word is the *least* coherent code in the creature (0.007) and survives a random
  tract perfectly. Source strength predicts survival; coherence does not.
- **The naming ceiling,** which is an exponent rather than a tally of failures: the learned bias
  reaches the voice through `dF1 ~ aligned^0.61`, so doubling the delivered movement needs 3.12× the
  bias, and every knob found in this project gives 1.1–1.2×. Fourteen routes read as exhaustion and
  invite a fifteenth; one exponent tells you which routes are excluded.
- **Composition in time.** A phase-targeted lesson is null at every credit window and reward delay,
  with a delay line, a synaptic tag, a faster-than-human teacher, and a *perfect* phase index — the
  last at 0.5 SE while the per-phase bias tables provably diverge at 70–100 SE, which locates the
  obstacle in expression rather than credit assignment. The creature composes in space, not in time.
- **The size of the vocabulary, from both ends.** The code the air carries is
  `f1 x f2 x f3 ~ 19 signals`. The larynx *commands* seven independent centroid axes (cross-talk
  ≤ 0.007), but four never reached the air — the synthesiser had two resonators and a hardcoded
  bandwidth — and once a third resonator was added, only F3 survives the cochlea as a usable axis at
  ~0.93x F1. The fourth candidate, bw2, reads **d' 1.016 ± 0.040** against a 1.0 audibility bar: the
  point estimate above it, the lower bound at 0.918, so it cannot carry a word. The *denominator* is
  closed too — levels per axis are `swing / scatter`, and `noise_amp`, the only knob on the scatter,
  turns out to move the **duty cycle** rather than the audible spread, leaving voiced-gated levels
  flat within 1.0 SE across the ladder it is scoreable on. So nine words naming as well as six was
  never evidence that the grid could keep growing: the limit is how many distinctions the air and
  the ear carry, and that is three axes at roughly 2.6 levels each.
- **Spending the vocabulary on more axes, which is the behavioural half of the same question.** The
  entry above says the air carries three axes; this one says using the third one *loses*. With a
  third resonator switched on, f3 is audible (d' 5.272 at its reach), commandable (2.68 levels),
  teachable (+47.88 ± 9.47 Hz, 5.1 SE) and nameable on the creature's own derived index (excess
  +0.2427 ± 0.0492, 4.9 SE, 16/18). It still does not pay. A 2×2×2 code spread over three axes
  delivers **5.95 ± 0.75 Hz** of F1 where a 3×3 code stretched over two delivers **11.14 ± 1.27** —
  paired −5.19 ± 1.43, −3.6 SE. Taught one axis per session third instead, so that the three lessons
  are mutually silent by construction, it is still **−4.53 ± 1.78 Hz (−2.5 SE)** behind the stretched
  code, and sequencing at matched budget buys **+0.83 ± 1.39 (0.6 SE)** — nothing. The retention
  prediction that motivated the whole line held exactly: F1, taught first and measured two
  intervening lessons later, delivered 8.83 ± 1.07 against an interleaved 8.00 ± 0.92. So **silence
  between axes is free and also worthless.** Adding an axis multiplies the code space and divides the
  teaching time, and reach is bought with time — a three-axis arm given 3× the teaching exactly ties
  a two-axis stretched arm given 1.125× (+0.09 ± 2.25). `render_f3_gain` ships at 0.0 for that reason
  and not for an audibility one.
- **Accumulating on the input side instead.** The ear is not a bottleneck and cannot be made into
  one: it carries one-of-eight at **0.981 against a 1.000 signal ceiling**, so a better auditory
  front end has nothing to occupy. Nor is it a memory — on the same trials it reads 1.000 while the
  word plays, 0.992 for 200 ms after, 0.624 by 200–600 ms and 0.520 by 600 ms, which is chance. The
  creature's only persistent record of a word it heard is its own **motor echo**, which still carries
  the word at 0.706 where the ear is already at chance. Central carries nothing: deleting
  `central→vocal` entirely leaves every number unchanged. This is why the exponent above bounds
  everything rather than only production, and why four weight-retention stores were the wrong shape
  — there is no non-motor trace for them to consolidate.

## What would have to be different

The clearest way to state it is to count the factors the kernel actually has. Learning here is
three-factor: STDP writes to an eligibility trace and never to the weight, so a synapse's write is

    e += A+ * trace_pre(src)      on a postsynaptic spike
    e -= A- * trace_post(dst)     on a presynaptic spike

cashed in later against a **scalar** reward. The three factors are therefore *presynaptic
activity*, *postsynaptic activity*, and *a global scalar*. **None of them is the context.** A
synapse can see what arrived at it and what its target did; it cannot see which word was heard,
and the reward that gates it is one number shared by the whole creature. Node perturbation has the
same gap from the other side: it adds a per-neuron constant, and a constant cannot be a function of
the input at any parameter setting.

So what is missing is a factor that is **input-specific**. And the distinction matters, because a
multiplicative gate on eligibility was built here and refused — DNA v29 multiplies both halves of
the pair by a plateau gate — but it keys on the **target neuron**: it selects *which neurons are
plastic now*, not *which of their inputs*. The same is true of the other gating mechanisms in the
refuted list (an apical compartment, a dendritic error signal, bursts, metaplasticity, dynamic
synapses, inhibitory plasticity). Every one of them gates a neuron or a module. None gates a
pathway by the identity of what is on it. It is also **not** the group-wide multiplicative gain,
which is closed by arithmetic — it reduces to an existing mechanism exactly.

That is where the project ends: with the missing ingredient named at the level of the learning
rule rather than at the level of a parameter. It is a research programme and not a run, and the
honest summary is that this architecture's ceiling has been found and characterised, not that the
next mechanism is waiting to be tried.

Worth keeping in view: the creature already *has* an input-specific conditioning path that works —
the context index, which computes a discrete context from the ear and selects among learned biases.
It is not plastic and not learned, which is exactly why it works. A mechanism that *learned* such
an index is the same question in a different place.

## How to check any of this

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j
    ./build/aibaby --dna dna/default.toml --experiment verify --ticks 120000

That must print `verify PASS`, the hash `ad96f882becbee92` as recorded, and all experiments
expected. Every instrument named above is a value of `--experiment`; the record gives the tier and
tick budget each one needs, and several want hours. The experiments are also the test suite — a
genome change is not verified until they have run.
