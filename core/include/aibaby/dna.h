// The genome: a compact, POD, little-endian binary blob.
//
// The core deliberately cannot read TOML. The host compiles the readable
// genome into this layout (see host/src/dna_toml.cpp) and the core just
// validates and points into it — on a microcontroller the blob can live in
// flash and never be copied to RAM at all.
//
// Every field here is a decision that is meant to be heritable. If something
// is tunable at runtime rather than born with the baby, it does not belong
// in this file.

#ifndef AIBABY_DNA_H
#define AIBABY_DNA_H

#include "aibaby/config.h"

namespace aibaby {

constexpr uint32_t kDnaMagic = 0x44424941;  // "AIBD"
// v2 adds module roles, homeostasis, and the sensory/motor transducer
// parameters that M1 needs; v3 adds the retina (§5.1) for M2; v4 adds
// structural plasticity (§3.4 growth, §3.5 myelination, §3.6 consolidation)
// for M4. Older blobs are rejected rather than upgraded: a silently
// reinterpreted genome would break G1 in the least visible way possible.
// 5: growth gained require_saturation / error_floor / patience, because the
//    literal §3.4 saturation test is unsatisfiable under §3.1's homeostasis and
//    no creature had ever grown outside the forced arm of `g4`.
// 6: audio gained self_gain — the creature can hear its own voice, without
//    which §5.3's account of babble cannot operate.
// 7: projections gained a `kind`, and with it the first *structured* wiring
//    rule (kGabor) and the visual-cortex role it exists to build. Every
//    projection before this one was "connect a random `density` of pairs"; the
//    retina→V1 map is a receptive field, and a receptive field is geometry, not
//    a probability.
// 8: kCurvature and the form-cortex role. v7 made orientation explicit and
//    measurement said that was not enough: area-matched, a cube and a ball
//    present the same orientations in the same amounts and differ only in how
//    those orientations are *arranged*.
// 9: wake_scale/sleep_scale moved from [homeostasis] onto each module. One
//    global rate cannot buy G2's reward effect without buying the drone: the
//    dial that stops homeostasis erasing a rewarded shift is the same dial that
//    stops it holding the larynx down. Neuromodulation is delivered to some
//    regions and not others, and so is this.
// 10: [exploration] and per-module explore_scale — motor variability that the
//    reward signal opens and closes, after LMAN in the songbird anterior
//    forebrain pathway. Babble was a fixed noise_amp before this, and a fixed
//    exploration rate cannot converge on anything.
// 11: v9's one dial per module became two, because §3.1's two mechanisms want
//    opposite things of the larynx. Intrinsic plasticity is what holds the duty
//    cycle — relax it and the creature drones — while synaptic scaling is what
//    erases a rewarded weight change before it can compound. v9 could only ask
//    for both or neither.
// 12: divisive normalisation, per module. Cortex divides a neuron's drive by
//    how active its neighbourhood is and this creature did not, which showed up
//    as an association module carrying cube-versus-ball in 4% of its neurons
//    where the vision module feeding it carries it in 22%.
//
//    Built on a sparse-coding argument that the measurement then refuted, and
//    the correction is the interesting part. It does not sparsen the code —
//    population sparseness *rises* with gain, 0.913 to 0.939 — and sparsening
//    the code directly does not help either: relaxing central's intrinsic
//    plasticity to zero gives the sparsest code in the project (0.79) and reads
//    chance, because nothing is left holding the neurons in their dynamic
//    range. What normalisation actually does here is raise per-neuron
//    discriminability, mean |d'| 0.177 -> 0.202, by dividing out a common-mode
//    "how busy is this module" component that was riding on every neuron at
//    once. Sparseness was the wrong variable; shared gain was the right one.
// 13: the fast half of a complementary learning system — a kHippocampus role
//     and a per-module eta_scale, so synapses onto one module can learn at a
//     rate the rest of the brain could not survive. Motivated by measurement,
//     not tidiness: cube-versus-ball sits at only +0.083 above chance at the
//     association module, and pattern separation is the one computation whose
//     entire purpose is to pull overlapping cortical codes apart.
// 14: a projection can say which presynaptic neurons it may recruit. Before
//     this every tract drew from both excitatory and inhibitory cells in the
//     source module, so there was no way to express a *subtractive* pathway —
//     and the creature's whole projection graph was feedforward apart from one
//     weak return, where cortex sends about as many fibres back as forward.
//     Implemented as a source filter rather than a sign flip because the
//     learning rule takes a synapse's clamp bounds from its presynaptic
//     neuron's is_inhib_ flag; flipping the weight instead would leave reward
//     driving it straight back across zero on the first update.
// 15: the critic's prediction is subtracted from what the ears deliver, so the
//     auditory module carries the *residual* rather than the signal. v14 added
//     top-down inhibition, but generic top-down inhibition is gain control: it
//     turns a module down without telling it what it was supposed to be
//     hearing. Predictive coding is the specific version — one number per mel
//     channel, cancelling that channel and no other — and the creature already
//     had the forward model needed to produce it, wired to nothing but a scalar
//     reward. See DnaCuriosity::predict_gain.
// 16: the eligibility trace carries a per-synapse baseline that is subtracted
//     before reward cashes it in, turning R-STDP from a correlation rule into a
//     covariance one. Motivated by measurement, not theory: `eligprobe` shows
//     the trace on central->vocal is large but object-blind, correlating +0.93
//     between the two conditions. See DnaStdp::elig_baseline_tau_ms.
// 25: the apical compartment — a neuron is two electrically separated
//     integration sites, and a tract says which one it lands on. Motivated by
//     the one thing v14 could not express: top-down feedback that modulates
//     without driving. See DnaModule::apical_threshold and
//     DnaProjection::apical.
// 26: subthreshold theta and nested gamma, per module. Aimed at the one thing
//     measurement says is wrong with the learning rule rather than at the fact
//     that cortex oscillates: STDP is a timing rule and central codes the
//     object as a rate, and an oscillation is what turns a rate difference into
//     a timing difference. See DnaModule::theta_hz.
// 27: the fovea can be pointed. REMOVED in v30 — see below.
// 28: the developmental arc. A tract can be born exuberant and pruned back by
//     experience, and a module's learning rate can close with its own age. The
//     first aims at projprobe's second named candidate — a structured tract
//     built by selection rather than by hand — and the second is the
//     developmental form of the fix that met G2. See DnaProjection::exuberance
//     and DnaModule::critical_tau_ms.
// 29: plateau-gated plasticity. Eligibility onto a module's neurons only
//     accumulates while that neuron's apical tuft is in a plateau. The first
//     mechanism aimed at the conditioning blocker itself rather than around it:
//     R-STDP's third factor is a global scalar and cannot steer a tract, but a
//     plateau is per-neuron and input-specific, and v25 measured that it
//     discriminates the object where the soma does not. See
//     DnaModule::plateau_gate.
// 30: the first version that *removes* things, and the removals are the point.
//     Eleven mechanisms now ship off, and each one costs a genome field, a
//     kernel branch and a required key in every TOML — the no-defaults rule
//     means a stale scratch genome fails to load once per dead mechanism. The
//     policy this sets: measured-and-refuted with no plausible revival gets
//     deleted, and its measurement table moves into the memory notes, because
//     the table is the valuable part and the field is the tax.
//
//     Deleted: DnaStdp::elig_pre_gate (v18, measured flat, no mechanism
//     proposed since); DnaStdp::hebb_rate (v19, superseded by v23's per-pathway
//     `hebb`, which solved the stability problem the global term failed); and
//     DnaVision's four saccade fields (v27's reflexive gaze controller, which
//     landed the eye ~4.4 px from the toy where the code needs ~1.4 — a
//     centroid over cells cannot be sharper than the cells). The retina keeps
//     its gaze and the edge-extend bug fix; `Retina::look_at` now drives it
//     directly.
//
//     Replacing that controller with an oracle immediately overturned what the
//     deletion was expected to confirm. With the fovea placed exactly on the
//     toy, `vision` reads 0.980 at a scatter that leaves the fixed eye at 0.540,
//     and `central` 0.860 against 0.520. **The visual code is not fragile, it is
//     retinotopic** — it is a perfect template in the eye's own coordinates and
//     displacement only breaks it because nothing moves the eye. Deleting the
//     controller was right; concluding that active vision has nothing to buy
//     here was wrong, and the open problem is how to aim, not whether aiming
//     helps.
//
//     NOT deleted, against an earlier plan that said otherwise: v22's
//     DnaInterneuron::tau_ms. It is not "uniform pooling" — it is the time
//     constant of the *live* v21/v22/v24 pooling mechanism, the one thing in
//     this project that measurably worked.
//
//     Added: DnaProjection::birth_weight, the companion without which v28's
//     exuberance is inert by arithmetic.
// 31: the fovea can be pointed again, by a controller built from what killed
//     the last one. v27 aimed at the response-weighted centroid over every
//     ganglion cell and landed ~4.4 px out where the code needs ~1.4; v30
//     deleted it and, in deleting it, measured that a *perfect* eye restores
//     the code completely (0.980 against 0.540 at 12 px of displacement). So
//     the mechanism was always worth having and only the aim was wrong. See
//     DnaVision::gaze_rate_hz.
//
// Built and removed inside one change, so it never took a version and the
// format below is unchanged: node perturbation cashed onto *synapses* rather
// than onto excitability, under a presynaptic gate. It was the last
// structurally untried learning rule here — R-STDP's output is buried and a
// per-neuron bias is a constant, so neither of this creature's two rules can
// make the voice a function of what the eye sees. The synaptic version can
// express that mapping and cannot estimate it: with the bias half switched off
// it scores -0.031x on G2 against the bias half's +1.742x and a no-exploration
// floor of -0.081x, which is the floor. So expressiveness was never what was
// missing. The tables are in `[exploration]` in dna/default.toml, under "the
// third half that did not work".
// 35: ModuleRole::kInterneuron — a relay population that exists to be sampled.
//     The genome language gains a role, not a field: no TOML has to mention it,
//     so unlike the mechanism above it costs nothing to carry. Built to test
//     `projprobe`'s E-I result in a spiking creature, and that test came back
//     negative for a reason worth more than the test: **the cap it targeted was
//     already closed.** `vision->vocal` ships, so the shipped creature already
//     reads the seen object at vocal at 0.660 across three seed families —
//     better than central's 0.600 — and there is no buried code left at the
//     larynx for a signed projection to recover. See DnaModule::ffi_source and
//     the README.
// 36: dynamic synapses — a tract's synapses deplete with use and recover on
//     their own time constant, after the model Barbara Webb's group used to
//     make a cricket robot recognise a calling song (Webb & Scutt 2000; Reeve &
//     Webb 2002). See the References section of the README.
//
//     The reason to want it is that this brain has nothing on this timescale.
//     Every plasticity mechanism it owns — R-STDP, the eligibility trace,
//     myelination, scaling, intrinsic plasticity — runs on seconds to minutes,
//     and a synapse's *transmission* has been a constant since M1. Recognition
//     of a temporal pattern needs neither learning nor a recogniser if the
//     synapse itself is a filter: Webb's BN1 fires efficiently only when the
//     gap between sound bursts is long enough for it to have recovered from
//     depression, and BN2 only when the onsets BN1 reports arrive close enough
//     together for facilitation to still be standing. The bandpass on syllable
//     rate is a side effect of two synapses with different time constants, and
//     no part of it is learned.
//
//     The second reason is closer to what this project keeps hitting. A
//     depressing synapse transmits its *changes* and not its steady traffic —
//     it is a per-edge common-mode remover, which is the same job DNA v21-v24's
//     pooling interneurons do at population level. That is the one thing here
//     that measurably worked (+0.077, 3/3 families) and its documented failure
//     mode is common-mode *swamping*: one shared scalar subtracted from every
//     target. A synapse can only ever deplete in proportion to what it
//     individually carries, so it has no shared term to swamp with.
//
//     Two decisions worth knowing before reading a number off it.
//
//     **Release is normalised by `stp_use`,** so the first spike after a
//     silence delivers exactly the genome's `weight`. Without that, switching
//     the mechanism on would scale a whole tract down by U and every number
//     taken afterwards would be about the recalibration rather than about the
//     filter. What depression then means is "less than the resting weight under
//     sustained traffic", which is the semantics the mechanism is for.
//
//     **Synaptic scaling still holds the nominal weight.** §3.1 regulates the
//     sum of `syn_weight_`, which is now the resting delivery rather than the
//     mean one, so a hard-depressing tract delivers less than the regulator
//     believes. That is a real interaction and not a bug: the resting weight is
//     the only rate-independent thing there is to regulate.
//
//     See DnaProjection::stp_use and the `stpprobe` experiment.
// 37: burst-dependent plasticity (Payeur, Guerguiev, Zenke, Richards & Naud,
//     Nature Neuroscience 2021). A neuron's postsynaptic **burst** rather than
//     its single spike carries the learning signal, and whether it bursts is
//     controlled by its apical dendrite — so a feedback signal arriving at the
//     tuft steers plasticity at that neuron's basal synapses. See the
//     References section of the README.
//
//     **This is the one structurally untried class in the conditioning cap.**
//     All five classes fenced there keep R-STDP's architecture: a *global
//     scalar* third factor multiplying a local trace. Reward composition
//     changed the scalar (v20), eligibility distribution changed the trace
//     (v16/v17/v18), the Hebbian arms removed the scalar (v19/v23),
//     representation changed the input (v12/v13/v14), perturbation changed the
//     exploration. Not one of them changed the fact that every synapse in the
//     brain is multiplied by the same number. What has never existed here is a
//     **per-neuron, input-specific** learning signal, which is what e-prop
//     (Bellec et al., Nature Communications 2020) says a spiking network needs
//     and what a burst code is a biological way to deliver.
//
//     **It is the "selects" version of v29, which attenuated.** The plateau
//     gate multiplies eligibility by (1 - gate), and its recorded post-mortem
//     is that the gate *attenuates instead of selecting*. A burst signal is
//     signed: a neuron bursting above its own baseline potentiates its
//     afferents and one bursting below depresses them. That is selection, and
//     it is the precise difference the v29 failure names.
//
//     **Two of the three ingredients the paper names were already here.** It
//     asks for regenerative apical activity (v25), plasticity in feedback
//     pathways (v14), and short-term synaptic dynamics (v36, built the same
//     day). What was missing is the burst itself: nothing in this kernel had
//     ever distinguished two spikes 5 ms apart from two spikes 500 ms apart.
//
//     See DnaModule::burst_ms, DnaModule::burst_refrac_scale,
//     DnaProjection::burst_learn, DnaStdp::burst_baseline_tau_ms, and the
//     `burstprobe` experiment.
// 38: competitive pruning. §3.6 removes a synapse that is weak AND idle, and
//     the exuberance post-mortem (v28/v30) measured that the second half is the
//     binding constraint: "in an exuberant tract every synapse carries traffic
//     because the source fires. An absolute traffic floor cannot express what
//     development actually does, which is COMPETITION — a synapse is removed
//     because its neighbours on the same target won, not because it fell below
//     a fixed bar." That file named the change and declined to build it, which
//     is what this is.
//
//     A synapse is now also prunable when it is weak *relative to the other
//     afferents of its own target*, with no idle test at all. Four arms on
//     `vision->central` had measured at most 58 of ~18,400 exuberant synapses
//     removed over seven sleep cycles, 0.3%, with no trend across a 10x range
//     of birth weight — selection had no gradient because nothing born could
//     ever become idle. A relative bar has a gradient by construction: half of
//     any distribution is below its own mean.
//
//     See DnaConsolidate::prune_compete.
// 39: a per-module eligibility time constant, which is e-prop's one concrete
//     experimental prediction made testable here (Bellec, Scherr, Subramoney,
//     Hajek, Salaj, Legenstein & Maass, Nature Communications 2020: "the time
//     constant of the eligibility trace for a synapse is correlated with the
//     time constant for the history-dependence of the firing activity of the
//     postsynaptic neuron"). This creature has one global tau_elig for every
//     synapse in the brain, which asserts that the larynx and the association
//     module credit the past over the same window. They do not: the larynx has
//     800 ms of articulator inertia and central carries an object code over
//     hundreds of ms.
//
//     **Honest placement: this lands inside a fence.** "Eligibility
//     distribution" is one of the five refuted conditioning classes — v16
//     subtracted a baseline, v17 scaled by the presynaptic mean, v18 selected,
//     and the recorded verdict is "the distribution is not the problem". A
//     per-module timescale is another knob on the same quantity, and it is
//     built because it is cheap and named in the literature rather than
//     because the fence has a gap in it. See DnaModule::elig_tau_scale.
// 40: the dendritic error microcircuit (Sacramento, Ponte Costa, Bengio &
//     Senn, NeurIPS 2018). The pooling interneuron moves onto the apical tuft
//     and its weight LEARNS to cancel what arrives there, so the compartment
//     carries a prediction *error* rather than a teaching signal. When the
//     lateral prediction is right the tuft reads zero and nothing is written;
//     when it is wrong, the residual is what drives learning.
//
//     **Why this and not another third factor.** v37 put a per-neuron learning
//     signal on the tuft and `burstprobe` measured what it cost: the plateau
//     carries the object at 0.913 and the burst derived from it at 0.673, on
//     3 of 3 seeds. A nonlinearity applied to a signal that was already there
//     loses a quarter of it. An error is not a nonlinearity on the signal — it
//     is the signal with the predictable part subtracted, which is the one
//     transform this project's standing diagnosis actually asks for: *a small
//     differential riding on a large common mode*, eight times over.
//
//     **Why it can be built out of parts that already exist.** v24 gave every
//     neuron its own weight on the pooled signal (`ffi_w_`, mean 1 within a
//     module) and v25 gave it an apical compartment. What was missing is that
//     `ffi_w_` is fixed at birth. v24's recorded failure is that `ffi`
//     subtracts *one shared scalar from every target*; a weight that learns
//     until the residual is zero is per-target by construction, and it is the
//     difference between a fixed gain and a cancellation.
//
//     It also needs no reward, which puts it outside the conditioning cap
//     rather than inside it — every one of the five classes fenced there is
//     about what reward multiplies.
//
//     See DnaModule::ffi_apical and DnaModule::ffi_learn, and `errprobe`.
constexpr uint32_t kDnaVersion = 41;

// What a module is wired to the world through. The host looks modules up by
// role, never by name or index, so renaming a module in the genome cannot
// silently disconnect a microphone.
enum class ModuleRole : uint32_t {
  kAssociation = 0,
  kAuditory = 1,
  kVision = 2,
  kSomato = 3,
  kVocal = 4,
  kExpression = 5,
  // Early visual cortex: the selective stage between the retina and the
  // association module. It owns no hardware channel — nothing in the host looks
  // it up — but it is a distinct role rather than a second association module,
  // because the wiring rule that fills it is retinotopic and growth must not
  // touch it. See the note on Network::growable().
  kVisualCortex = 6,
  // The stage after V1: form rather than edges. Named for what it does rather
  // than for a brain area, because the computation it carries — selectivity for
  // the radius of a curve — is documented in V4 while its position in the
  // hierarchy is V2's. Like kVisualCortex it owns no hardware channel and
  // cannot grow.
  kVisualForm = 7,
  // The fast half of a complementary learning system (McClelland, McNaughton &
  // O'Reilly). Cortex here learns slowly and represents things in overlapping,
  // distributed codes — good for statistics, bad for keeping this cube apart
  // from that ball. The hippocampus is the opposite: a sparse, expanded,
  // pattern-separated code learned at a rate cortex could not survive.
  //
  // It is a role rather than a second association module for the same reason
  // kVisualCortex is: experiments have to be able to find it, and what makes it
  // a hippocampus is a *combination* of genome settings — expansion, a high
  // threshold, strong normalisation, a large eta_scale — which is worth being
  // able to name and check in one place.
  kHippocampus = 8,
  // A relay of interneurons between two other modules — a population that
  // exists to be *sampled*, not to represent anything. It owns no hardware
  // channel and nothing in the host looks it up.
  //
  // It is a role rather than an unlabelled module because of what must not
  // happen to it. `Network::growable()` admits kAssociation and nothing else,
  // so a relay can never grow: adding neurons to it would change how every
  // downstream cell samples its source, which is the one property it is built
  // to hold fixed. The same argument as kVisualCortex, for the same reason.
  //
  // Why the creature wants one. `central->vocal` is an all-positive random
  // projection and central's object code is *balanced* — coherence 0.024-0.152
  // across three seed families, so 85-98% of the discriminative signal is some
  // neurons up and others down. A positive random sum of a balanced pattern
  // averages toward zero; a *signed* one preserves it. `projprobe`'s E-I arm
  // measures that, label-free, at 0.560 -> 0.680 on 3 of 3 seeds.
  //
  // The signed projection cortex actually builds is balanced feedforward
  // inhibition, and the thing that makes it *signed* rather than merely
  // subtractive is that each target draws its own independent inhibitory
  // sample. That is exactly what DnaModule::ffi_source is not — `ffi` subtracts
  // one shared scalar from every target, which removes the common mode and
  // leaves every target with the same positive sum of the remainder. A relay
  // gives each target its own draw. See the README synthesis.
  kInterneuron = 9,
  kRoleCount,
};

// How a projection decides which pairs to connect.
//
// kRandom is every projection the genome had before DNA v7: draw `density` for
// each (source, target) pair and wire the winners. It can make a module louder
// or quieter, and it can carry a pattern, but it cannot make a *distinction*
// explicit that its input did not already contain.
//
// kGabor is a receptive field. The target's position in its own module volume
// is read as a position on the retina and a preferred orientation, and the
// source cells it draws from are chosen by where they fall in that field.
// kCurvature is the conjunction stage. Its target pools *oriented* cells rather
// than retinal ones, and it asks a question about their arrangement: are these
// orientations tangent to a common circle, and how big is that circle? A disc
// drives such a cell with every point on its boundary at once; a square of the
// same area can be tangent to a circle at four points and nowhere else. That
// difference is the whole of what separates the two toys, and it is invisible
// to any amount of local orientation.
// Which presynaptic neurons a projection may recruit (DNA v14). kEither is
// what every projection did before and is bit-identical to it.
//
// Top-down feedback in cortex is largely excitatory *axons* that terminate on
// local interneurons, so a descending pathway can be net-inhibitory. Selecting
// inhibitory sources is the cheap way to express that here, and unlike a forced
// negative weight it keeps every sign invariant in the learning rule intact.
enum class ProjectionSource : uint32_t {
  kEither = 0,
  kExcitatory = 1,
  kInhibitory = 2,
  kSourceCount,
};

enum class ProjectionKind : uint32_t {
  kRandom = 0,
  kGabor = 1,
  kCurvature = 2,
  kKindCount,
};

struct DnaSim {
  float dt_ms;                        // simulation timestep
  uint32_t max_delay_ticks;           // ring-buffer depth for axonal delay
  float conduction_velocity;          // spatial units per ms
  uint32_t plasticity_interval_ticks; // how often eligibility is cashed in
};

// The plasticity interval is a real design constant, not a performance hack:
// decaying every eligibility trace on every tick is O(synapses) at 1 kHz,
// which is the one loop that would stop this scaling. Reward is integrated
// over the interval and applied once. Keep it well under tau_elig.

// DNA v20. When enabled, each reward term keeps its own running baseline and
// each module weights the four channels itself (DnaModule::nm_*). When
// disabled the four are summed against one baseline and one scalar reaches
// every synapse, which is exactly the v19 behaviour and bit-identical to it.
// DNA v22. Time constant of the pooling interneurons' estimate of their source
// module's activity. v21 reused `mean_rate_fast`, a one-second EMA, and that is
// the reason it did not work: the common mode that buries the object code moves
// trial-to-trial, and a constant offset is invisible to the classifier anyway
// because holdout_accuracy z-scores every dimension. Only the fast component
// matters. Real feedforward inhibition acts within about ten milliseconds.
struct DnaInterneuron {
  float tau_ms;
  uint32_t pad;
};

constexpr uint32_t kModulatorChannels = 4;

struct DnaNeuromod {
  uint32_t enabled;
  uint32_t pad;
};

struct DnaStdp {
  float a_plus, a_minus;        // trace increments
  float tau_plus_ms, tau_minus_ms;
  float tau_elig_ms;            // ~2000ms: the human-feedback latency window
  float eta;                    // reward-gated learning rate
  float reward_clip;            // |R| ceiling, so one poke cannot wipe a brain
  // DNA v16. Time constant of a slow per-synapse running mean of the
  // eligibility trace, subtracted before the trace is cashed in. 0 disables it
  // and reproduces v15 exactly.
  //
  // This is the reward baseline's trick applied one level down, and for exactly
  // the same reason. R(t) already has its running mean subtracted, because
  // otherwise the common-mode part of reward "walks every eligible synapse in
  // the same direction". The trace has the same disease and no such cure: on
  // `central->vocal` the eligibility pattern correlates **+0.93** between the
  // two objects, so almost all of what reward multiplies is a constant of the
  // wiring rather than anything about what the creature is looking at.
  //
  // Subtracting a per-synapse mean turns the correlation rule into a covariance
  // rule. What is left is the part of the trace that is unusual *for that
  // synapse*, which is the only part that can carry a condition.
  //
  // Why this rather than the `a_minus` route, which was measured first and
  // does raise the trace's conditionality a long way (0.576 -> 0.912): making
  // STDP more depression-dominant buys that by pushing the whole weight
  // distribution down, and the calibration echo falls 0.825 -> 0.675 with it.
  // A baseline subtraction is sign-neutral by construction — it removes a
  // common mode without biasing the drift — so it should not cost the drive.
  float elig_baseline_tau_ms;
  // DNA v37. The timescale of the per-neuron baseline burst rate that
  // burst-dependent plasticity reads its learning signal as a deviation from.
  // 0 disables the mechanism brain-wide, because without a baseline the signal
  // is a rate — non-negative at every neuron at every moment — and a rule that
  // can only potentiate is the failure mode v19 already paid for.
  //
  // Slow relative to the burst rate itself, which follows the 50 ms motor
  // readout constant: the baseline has to describe what this neuron ordinarily
  // does, not what it is doing now, or the deviation is always zero.
  float burst_baseline_tau_ms;
  // DNA v17. How much of the presynaptic module's *instantaneous population
  // mean* trace to subtract before the potentiation term reads it. 0 disables
  // it and reproduces v16 exactly; 1.0 subtracts the mean entirely.
  //
  // The rule this fixes is a product of two rates. Split each into its
  // population mean and that neuron's deviation from it:
  //
  //   e_ij ~ (mean_i + d_i)(mean_j + d_j)
  //        = mean_i*mean_j + mean_i*d_j + d_i*mean_j + d_i*d_j
  //
  // Only the last term is conjunctive — the one that says *this* input
  // together with *this* output. The first is pure common mode and it
  // dominates, which is why `dwprobe` finds 31% of what reward writes is
  // reproducible but object-independent, and why the eligibility pattern on
  // central->vocal correlates +0.93 between the two objects.
  //
  // Why the presynaptic *population* mean and not the v16 baseline. v16
  // subtracted a slow per-synapse running mean of the eligibility itself, and
  // it was refuted — credit fell to chance (0.503 against 0.570 raw), because
  // the object-dependent part is slow too and went with it. `projprobe` then
  // measured which centring actually recovers a code that pooling has buried:
  // subtracting each *trial's own population mean across neurons* restores
  // vision from 0.510 to 0.940 and central from 0.500 to 0.690, and it is
  // label-free so it cannot be leaking. This is that operation moved from the
  // measurement into the learning rule.
  float elig_pre_centre;
};

// Rate regulation. §3.1: "Homeostasis is mandatory" — a spiking net without it
// falls silent or saturates within minutes, and every result after that is
// noise.
struct DnaHomeo {
  float ip_rate;                // threshold step per Hz of rate error
  float threshold_min;
  float threshold_max;
  float w_max;                  // |weight| ceiling for excitatory synapses
  float scaling_rate;           // fraction of the excess corrected per pass
  // Synaptic scaling has a dead band: it acts only when a neuron's total
  // incoming weight leaves [birth/band, birth*band]. §3.1 asks it to keep
  // total input weight *bounded*, and a bound is not a setpoint. Implemented
  // as a setpoint it pins the exact quantity reward learning has to change,
  // and it wins — measurably: with a setpoint the rewarded and yoked babies
  // are indistinguishable; with a band they separate.
  float scaling_band;
  uint32_t interval_ticks;      // how often intrinsic plasticity + scaling run
  // Whether reward may retune inhibitory synapses as well as excitatory ones.
  // Potentiating an inhibitory synapse means *more* inhibition, so if
  // inhibitory activity happens to correlate with reward the module quietly
  // shuts itself down — a runaway with no restoring force. Off by default.
  uint32_t inhib_plastic;
};

// Growth (§3.4). Every field here is a guardrail: the point of the section is
// that a network which grows whenever it feels like it hides the fact that
// learning is broken, so each of the three trigger conditions has a number and
// none of them is optional.
struct DnaGrowth {
  uint32_t plateau_window_ticks;
  float epsilon;                // improvement below this counts as a plateau
  uint32_t insert_k;            // neurons added per growth event
  float saturation_rate_hz;     // module counts as saturated above this
  // The second half of "saturated": rate alone is not enough, because a module
  // can be busy and still have room to learn. A module is only saturated when
  // its incoming weights are also pressed against the ceiling — that is what
  // "little headroom" means, and it is the difference between a module that
  // needs more neurons and one that is merely loud.
  float saturation_weight;      // mean |w| / w_max above which headroom is gone
  // Whether the two bars above are required at all.
  //
  // They describe an *unregulated* network, and §3.1 makes regulation
  // mandatory: intrinsic plasticity drives every module's rate back to its
  // genome setpoint, and sleep downscaling pulls weights away from their
  // ceilings. Measured over 25 minutes of life, a healthy creature peaks at
  // 13.5 Hz against a 20 Hz bar and its share of edges near the ceiling *falls*
  // from 2% to nothing. So a literal reading of §3.4 condition 2 can never be
  // satisfied by a creature that §3.1 is working on, and the two clauses of the
  // requirements contradict each other.
  //
  // The resolution kept here: "little headroom" means the module cannot
  // explain any more of what happens next with the structure it has — which is
  // a plateau that persists while the creature is still wrong, not a hot
  // module with full weights. Set this back to 1 for the literal reading, and
  // for the behaviour every measurement before DNA v5 was taken on.
  uint32_t require_saturation;
  // How wrong the creature must still be for more neurons to be the answer.
  // Below this the forward model already explains what happens next, and a
  // brain that keeps growing after that is padding, not developing.
  float error_floor;
  // Consecutive growth events that fail to reduce prediction error before
  // growth gives up. This is §3.4's "growth without limits masks bugs" made
  // mechanical: if adding neurons is not helping, the flat neuron count goes
  // back to being evidence that learning is broken rather than hiding it.
  uint32_t patience;
  float new_weight;             // mean |w| of a grown neuron's synapses
  // Minimum gap between two growth events anywhere in the brain. A plateau
  // lasts as long as it lasts, and without this one flat stretch would grow
  // every module to its cap in a few seconds of simulated time.
  uint32_t refractory_ticks;
  uint32_t enabled;
};

// Myelination (§3.5) and sleep consolidation (§3.6). One struct because they
// are one mechanism seen at two timescales: traffic decides which edges are
// load-bearing, myelination speeds and protects those, and consolidation
// deletes what traffic never reached.
struct DnaConsolidate {
  // --- §3.5 myelination, continuous ---
  float traffic_tau_ms;         // leak of the per-edge traffic counter
  // Traffic at which an edge is half-myelinated. Traffic settles at roughly
  // (presynaptic rate in Hz) x (traffic_tau in seconds), so this is quoted in
  // the same units: at tau 10 s, a half of 50 means a 5 Hz axon.
  float traffic_half;
  float delay_floor_frac;       // shortest delay, as a fraction of the birth delay
  // Per-edge learning rate floor. A fully myelinated edge learns at this
  // fraction of eta: the pathway is consolidated and protected from being
  // overwritten, which is §3.5's defence against catastrophic forgetting.
  float eta_floor_frac;

  // --- §3.6 sleep, discrete passes ---
  float downscale;              // every weight is multiplied by this per pass
  // How far downscaling may erode a neuron's afferent budget, as a fraction of
  // what its structure entitles it to. Without a floor the multiplier compounds
  // over every sleep bout of a long life and the creature quietly fades: there
  // is no waking mechanism that restores total weight, because synaptic scaling
  // is silent inside its band and reward-modulated STDP is signed. This is the
  // restoring force, and it is a bound rather than a setpoint for the same
  // reason scaling_band is.
  float downscale_floor;
  float prune_weight;           // |w| below this, and quiet, is prunable
  float prune_traffic;          // traffic below this counts as negligible
  // DNA v38. Competitive pruning: a synapse whose |w| is below this fraction of
  // the MEAN |w| over its target neuron's afferents is removed, whether or not
  // it is idle. 0 is off and is the pre-v38 rule exactly.
  //
  // The absence of the idle test is the mechanism, not an oversight. Pruning
  // has always been "weak AND idle", and an exuberant tract fails the second
  // test everywhere: every synapse carries traffic because its source fires.
  // Development does not wait for a synapse to fall silent — it removes the
  // ones that lost to their neighbours on the same postsynaptic cell, which is
  // a comparison and not a threshold.
  //
  // Read it as a fraction: 0.5 removes afferents under half their target's
  // mean, which for a symmetric distribution is a large minority, and 1.0 would
  // remove everything below the mean and is almost certainly too much. It runs
  // once per sleep pass, so the effect compounds across a life.
  float prune_compete;
  // How many afferents a target needs before competition applies to it at all.
  // Without this, a neuron down to two inputs would keep losing the weaker of
  // them every sleep and end up disconnected — which the neuron pruner would
  // then remove, so a mechanism meant to sharpen a tract would quietly delete
  // it. Competition is only meaningful among enough candidates to have a
  // meaningful mean.
  uint32_t prune_compete_min_in;
  uint32_t replay_episodes;     // high-reward episodes kept and replayed
  uint32_t replay_ticks;        // how long each replayed episode is presented
  float replay_threshold;       // effective reward above which one is worth keeping
  uint32_t interval_ticks;      // how often a sleeping brain runs a pass
  uint32_t enabled;
  uint32_t pad;
};

struct DnaDrives {
  float hunger_rate, comfort_decay, fatigue_rate;
  float curiosity_weight;
  float w_external, w_hunger, w_comfort;
  // Time constant of the running reward expectation. Learning is driven by
  // R - E[R], not by R: a creature that is praised constantly should learn
  // nothing from being praised again. Without this, the mean reward multiplies
  // the mean eligibility and every synapse in the brain drifts together —
  // motion, but not learning.
  float reward_baseline_tau_ms;
  // Sleep (§3.6). Fatigue is the only drive with no way back down on its own:
  // rest is what discharges it, and rest is a state, not a decay constant.
  float fatigue_recovery;       // per ms while asleep
  float sleep_threshold;        // fatigue at which the baby drops off
  float wake_threshold;         // and the lower level at which it wakes again
  float pad;
};

// The cochlea (§5.2). Window and hop live here rather than as host constants
// because they set the frame rate the auditory module is born expecting.
struct DnaAudio {
  uint32_t sample_rate;         // 16000
  uint32_t window;              // 512 samples
  uint32_t hop;                 // 256 = 50% overlap
  uint32_t mel_channels;        // 24
  float mel_low_hz, mel_high_hz;
  float floor_db;               // log-compression floor
  // How loud the creature is to itself. §5.3 makes babble "motor noise shaped
  // by the curiosity drive", and curiosity is prediction error on the next mel
  // frame — so without a path from the larynx back to the cochlea that
  // sentence describes nothing. 0 restores the deaf creature every measurement
  // before DNA v6 was taken on.
  float self_gain;
  float gain;                   // injected current per unit normalised energy
};

// The retina (§5.1). As with the cochlea, the sampling itself is host code and
// only the *shape* of the result is heritable: a foveated field of
// centre-surround cells, ON and OFF, arriving at a fixed frame rate.
//
// Foveation is not a saving here, it is the encoding. A uniform 64x64 grid
// would be 4096 numbers for a creature whose whole data budget is a few
// thousand interactions; spending the acuity where the baby is looking and
// coarsening outward is what makes the field describable in ~176.
struct DnaVision {
  uint32_t frame_size;      // the grayscale square the host delivers
  uint32_t fovea_size;      // central region sampled at full acuity
  uint32_t fovea_grid;      // ganglion cells across the fovea
  uint32_t ring_grid;       // cells across each peripheral ring
  float center_sigma;       // DoG centre, in cell pitches
  float surround_sigma;     // DoG surround, likewise
  float contrast_gain;      // DoG response that reads as full scale
  float contrast_floor;     // below this a cell has nothing to say
  // DNA v34. The same threshold, but for AIMING rather than for seeing.
  //
  // `contrast_floor` does double duty: `AuditoryEncoder`'s visual twin uses it
  // as the per-cell silence floor, and the gaze controller uses it to decide
  // whether anything is worth looking at. Those are different jobs and they
  // want different numbers. Perception should be conservative — a cell with
  // nothing to say should say nothing. Acquisition should be twitchy, because
  // the cost of a wasted saccade is one frame and the cost of not looking is
  // never seeing the object at all.
  //
  // Splitting them is what a retinotectal pathway is for: the colliculus drives
  // saccades from signals the geniculate pathway cannot yet resolve into a
  // shape. Set equal to `contrast_floor` for the pre-v34 creature exactly.
  float gaze_contrast_floor;
  float gain;               // injected current for one latency spike
  float frame_hz;           // camera frames the module is born expecting
  // Latency coding (§5.1): a cell at full contrast fires at the top of the
  // frame and a barely-responding one fires this much later. The intensity is
  // in *when* the spike happens, which is the quantity STDP is built to read —
  // a rate code would hand the same information to the module as a number of
  // spikes, and the timing structure reward has to bind to would be gone.
  float latency_ms;
  // DNA v31. Pointing the eye, second attempt. `gaze_rate_hz` 0 never moves it
  // and is the v30 creature exactly.
  //
  // Two changes from v27, and each is aimed at a specific reason that one
  // missed by an order of magnitude.
  //
  // **Aim at the peak-responding cell, not the response-weighted centroid.** A
  // centroid over every cell averages the object with everything else in view,
  // and the periphery is where that hurts: a coarse ring cell that merely
  // overlaps the object still contributes, and it contributes *its own centre*,
  // which can be half a frame away. The peak cell is a worse estimator of where
  // the object's middle is and a far better one of where the object is at all.
  //
  // **Iterate.** v27 fired one open-loop jump every saccade period. This
  // corrects a fraction `gaze_gain` of the estimated error every re-aim, so
  // three re-aims at 0.7 leave 2.7% of the initial error — 0.3 px from a 12 px
  // start. Precision then comes from repetition rather than from any single
  // estimate being good, which is what real oculomotor control does and is why
  // a coarse peripheral estimate is enough to start with.
  //
  // The two together buy the third thing v27's post-mortem asked for without
  // needing a field for it: once the object is inside the fovea, the
  // peak-responding cell *is* a foveal cell, so the final correction is made at
  // 2x2 px resolution automatically.
  float gaze_rate_hz;   // how often the eye re-aims; 0 = never
  float gaze_gain;      // fraction of the estimated error corrected per re-aim
  // The peak cell alone is the wrong target and the first run said so in one
  // number: the eye settled ~5.4 px from the toy at *every* scatter level,
  // including 1.0 px, where it made things worse. A fixed error independent of
  // where the object is is not a convergence failure, it is convergence onto
  // the wrong point — and the wrong point is the object's **edge**. A
  // difference-of-Gaussians cell reports contrast, so the strongest response to
  // a disc is on its boundary, one radius (6-7 px here) from its middle.
  //
  // So aim at the centroid of the cells responding at least this fraction of
  // the peak. That is the synthesis of the two failures: v27's centroid was
  // over *every* cell and got diluted by a periphery that merely overlapped the
  // object, while a single peak cell is local but sits on an edge. A centroid
  // over the peak's neighbourhood is local *and* centred.
  //
  // 1.0 is the pure-peak behaviour and 0.0 is v27's whole-field centroid, so
  // this one number spans both previous attempts and the space between them.
  float gaze_peak_frac;
  // DNA v33. How far from the peak cell a cell may sit and still join the aim
  // centroid, in pixels. 0 = unlimited, which is the v31 rule exactly, and it
  // SHIPS AT 0 — this mechanism was built against a hypothesis and the
  // hypothesis was wrong. Kept because the refutation is worth more than the
  // field costs, and because the measurement below is the one that says where
  // peripheral acquisition actually breaks.
  //
  // The hypothesis. v31's neighbourhood is a MAGNITUDE test and nothing else:
  // every cell above `gaze_peak_frac` x the peak is averaged in, wherever it
  // sits on the retina, even though the comment above calls it "the peak's
  // neighbourhood". So with the toy outside the fovea, foveal cells clearing
  // the same bar on noise should drag the aim back to the layout centre — and
  // the symptom fits, because at scatter 0.25 the eye re-aims 133 times and
  // moves 0.7 px.
  //
  // The measurement, and it is a clean no:
  //
  //   radius   INSIDE the fovea      OUTSIDE it
  //   0 (v31)  0.870, gaze 1.3 px    0.440, gaze 11.4 px
  //   6        0.740, gaze 4.5 px    0.480, gaze 12.7 px
  //   10       0.640, gaze 2.5 px    0.460, gaze 11.4 px
  //   16       0.830, gaze 1.3 px    0.440, gaze 11.4 px
  //   24       0.870, gaze 1.3 px    0.440, gaze 11.4 px
  //
  // Outside the fovea the gaze error does not move at ANY radius, and inside
  // it a tight radius makes aiming worse — cutting the neighbourhood down
  // throws away the cells that centre the estimate on a DoG edge response.
  //
  // What it rules out, which is the useful part. The eye is not being dragged
  // off a good estimate; there is no estimate. `fixed` reads vision 0.540 at
  // 12.1 px, which is chance, so the retina does not localise the toy out
  // there at all and no aiming rule can help. Peripheral acquisition is a
  // RETINA problem — coarser rings that actually respond, or a separate
  // low-acuity channel for acquisition the way a retinotectal pathway is —
  // and not a controller problem. Do not tune the controller for it again.
  float gaze_peak_radius;
};

// Reward-modulated motor exploration (LMAN).
//
// §5.3 makes babble "motor noise shaped by the curiosity drive", and until now
// the noise half of that was a constant: every module drew its spontaneous
// drive from noise_amp and never varied it. A constant exploration rate is the
// one thing that cannot converge — the creature is exactly as random after
// being praised a hundred times as it was on its first babble.
//
// A zebra finch does not work that way. LMAN *injects* variability into the
// motor pathway rather than tolerating it, and the injection is gated by how
// well things are going: undirected practice is variable, directed song is
// stereotyped, and lesioning LMAN abolishes both the variability and the
// learning. Exploration that collapses on success is what turns a lucky
// vocalisation into a habit.
//
// That is also why this is worth building *after* per-module homeostasis rather
// than instead of it. Homeostasis regulates a module's firing *rate*, and
// collapsing variability changes *which posture* the larynx keeps returning to.
// Those are different quantities, so the memory stops being a weight that has
// to out-argue the regulator every session.
struct DnaExploration {
  // Two windows on total reward, and their difference is the performance
  // signal. It has to be a difference: the creature's reward baseline chases
  // its own reward with an 8 s time constant, so the prediction error the
  // synapses see averages to zero by construction and cannot tell "doing well"
  // from "doing badly". Fast above slow means things are going better than they
  // have been lately, which is the moment to stop experimenting.
  float fast_tau_ms;
  float slow_tau_ms;
  float sensitivity;            // how hard that difference moves exploration
  float floor;                  // most stereotyped the creature may become
  float ceiling;                // ...and most exploratory. 1.0 is plain noise_amp
  // What replaces the variability when exploration closes, as a fraction of
  // noise_amp — and without it the mechanism does not model LMAN, it models a
  // lesion.
  //
  // In this creature the motor module's spontaneous drive *is* its babble:
  // neurons fire on the positive excursions of a zero-mean noise term, so
  // shrinking the amplitude does not make the larynx stereotyped, it makes it
  // silent. Measured, and unambiguously: at sensitivity 500 five of nine
  // creatures produced too few vocalisations to score at all.
  //
  // A songbird does not have this problem because the variability and the drive
  // come from different places — HVC drives RA reliably and LMAN adds variance
  // on top. This is the HVC term. As exploration closes, the variance it
  // removes is handed back as steady depolarisation, so what falls is how
  // *unpredictable* the larynx is and not how much it moves. Around 0.5,
  // because that is the mean positive excursion of the uniform noise being
  // replaced.
  float drive_compensation;

  // --- directional exploration: node perturbation ---------------------------
  // The scalar gain above shrinks variability in every direction at once, which
  // is neither what LMAN does nor how a creature converges on anything. This is
  // the part that *steers* it.
  //
  // Each neuron keeps a decaying trace of its own recent perturbation — the
  // actual random numbers it was given — and when reward arrives its
  // excitability moves along that trace, scaled by the reward. A perturbation
  // that preceded a better-than-expected outcome is kept; one that preceded a
  // worse one is undone. Averaged over many perturbations this is an unbiased
  // estimate of the reward gradient with respect to each neuron's excitability,
  // which is why it works with no model of the vocal tract and no gradient
  // through the synthesiser (Fiete & Seung; it is also the standard account of
  // LMAN's instructive signal to RA).
  //
  // The reward it multiplies is the *centred* one — reward minus its running
  // expectation — and that is not a detail. Uncentred, every neuron's bias
  // would drift in the same direction at once, which is motion rather than
  // learning. Note this is the opposite requirement to the two windows above,
  // where centring was exactly what made the signal useless.
  float perturb_tau_ms;         // how long a perturbation stays creditable
  float perturb_rate;           // learning rate; 0 disables the whole mechanism
  float perturb_max;            // |bias| ceiling, or one lucky moment runs away

  // --- DNA v41: metaplastic consolidation -----------------------------------
  // The rule above is an unbiased gradient estimate, and `driftprobe` measured
  // what that costs. A neuron with no effect on the current lesson's reward has
  // a covariance of exactly zero with it — the rule is already telling it "you
  // get nothing" — but it cannot say so on any single sample. It says it as
  // zero-mean NOISE, and zero-mean noise applied to a standing bias is a random
  // walk. Measured on the larynx: while one vocal group is being taught, the
  // OTHER group diffuses just as hard (0.0164 against 0.0160) with nine times
  // less directional motion, and that diffusion is what erodes the older lesson.
  //
  // So the missing machinery is not a better third factor. Every attempt at one
  // here has been refuted — burst plasticity (v37), the dendritic microcircuit
  // (v40), plateau gating (v29) — and if the covariance is already right, they
  // were all answering a question the learning rule had answered. What is
  // missing is a way to stop moving what has already been decided.
  //
  // Each neuron keeps two running moments of its OWN bias updates, and their
  // ratio is a purely local estimate of how much of what it is being told is
  // signal:
  //
  //   snr = E[u]^2 / E[u^2]        1 = every update agrees, 0 = pure noise
  //
  // Plasticity is gated on that, so a neuron receiving a consistent gradient
  // learns at full rate and one receiving noise is quieted toward `meta_floor`.
  // This is metaplasticity in Fusi's sense — the plasticity of a synapse
  // depending on its own history of change rather than on any signal from
  // elsewhere (Fusi, Drew & Abbott 2005; Benna & Fusi 2016) — and it is the
  // per-neuron version of exactly the statistic `driftprobe` reads per group.
  //
  // It needs no credit assignment, no third factor and no teacher, which is why
  // it is a different class from everything in the conditioning fence.
  // Counted in REWARD EVENTS, not milliseconds, and that is not cosmetic. The
  // moments may only advance when reward actually arrived: updating them on
  // every plasticity interval would decay E[u]^2 faster than E[u^2] through a
  // quiet stretch, so the SNR would fall to zero from silence alone and the
  // gate would quiet every neuron almost all the time. The question this
  // statistic asks is "of the rewards I have had, how consistently did they
  // point the same way", and silence is not evidence either way.
  float meta_window;            // reward events in the window; 0 disables
  float meta_floor;             // plasticity left to a neuron whose updates are noise
  float meta_ref;               // the SNR that already counts as fully consistent

  // The SECOND way of asking the same question, and the cheaper one. The bias
  // IS the accumulated evidence: a neuron driven consistently walks away from
  // zero, one fed noise stays near it, and the distance is integrated over the
  // whole lesson rather than over a window. That matters, because the moment
  // ratio above has a noise floor of 1/meta_window and a lesson holds only a
  // few thousand reward events, so it has almost no headroom to work in.
  //
  // Plasticity falls with how far this neuron has already committed:
  //
  //   gate = 1 - (1 - meta_floor) * meta_commit * |bias| / perturb_max
  //
  // A neuron that has learned something is hard to move; a fresh one is free.
  // That is Fusi's cascade in its simplest form and it costs no state at all,
  // because the quantity it reads is one the kernel already keeps. It is also
  // the standard soft-bound on a weight, arrived at from the other direction.
  //
  // Independent of the moment gate: a genome may run either, both or neither.
  float meta_commit;            // 0 disables; 1 is the full brake at |bias| = max

  // The THIRD way, and the one that is actually Benna & Fusi's. Both gates
  // above work by refusing updates, which is why the commitment brake costs 14%
  // of the learning rate: a gate cannot tell "this neuron should stop" from
  // "this neuron is still working", so it slows both.
  //
  // This one refuses nothing. The bias becomes the fast variable of a
  // two-compartment chain, coupled to a slow one that stores:
  //
  //   bias  += flow * (slow - bias)
  //   slow  += flow * ratio * (bias - slow)
  //
  // A random walk injected into `bias` becomes an Ornstein-Uhlenbeck process
  // with bounded variance instead of one that wanders without limit, so the
  // DIFFUSION `driftprobe` measured decays away on its own. Sustained drive is
  // not damped the same way: it drags `slow` with it, the pair settles at the
  // driven value, and with no drive at all the two converge and stay — so what
  // a lesson bought is kept rather than leaked back to zero.
  //
  // The learning rate is never touched, which is the whole point.
  // The exchange CONSERVES `meta_ratio * bias + slow`, which is what makes this
  // a store rather than a leak — nothing drains to zero and a lesson is kept.
  // It is also what makes `meta_ratio` the parameter that matters, and the one
  // this project first got wrong. Starting from an empty store the pair settles
  // at `bias = ratio * B / (1 + ratio)`, so the readout keeps `r/(1+r)` of what
  // was learned: at 0.05 that is FIVE PERCENT and the creature looks frozen no
  // matter how `meta_flow` is set. `meta_flow` only sets how fast that happens.
  //
  // The trade is therefore between damping and readout: larger `meta_ratio`
  // keeps more of the lesson and shares less of the noise away.
  float meta_flow;              // fast-to-slow coupling per cash-in; 0 disables
  float meta_ratio;             // the store's capacity ratio; keeps r/(1+r) of the signal



  uint32_t enabled;
};

// Divisive normalisation (§3.1, DNA v12). The per-module strength lives on
// DnaModule::norm_gain; these are the guardrails that keep the division from
// becoming a runaway.
//
// The factor is 1 / (1 + gain * (mean_rate / target_rate - 1)), so a module at
// its target rate is untouched whatever the gain, a busier one is divided down,
// and a quieter one is multiplied up. That reference to the target is what
// makes gain = 0 exactly inert rather than approximately so.
struct DnaNormalisation {
  // Bounds on the factor. The lower one matters most: mean_rate is an EMA and
  // the loop through it is delayed by a tick, so an unbounded divisor can
  // oscillate. The upper one stops a module that has fallen silent from
  // amplifying its own noise back up.
  float floor;
  float ceiling;
  uint32_t enabled;
};

struct DnaTouch {
  float gain;                   // injected current per unit intensity
  float duration_ms;            // how long a touch keeps driving B4
};

// The vocal tract (§5.3). These ranges are body plan: two babies with
// different formant ranges are physically different creatures.
struct DnaVocal {
  float f0_min, f0_max;
  float f1_min, f1_max;
  float f2_min, f2_max;
  float f3_min, f3_max;
  float bw_min, bw_max;
  float voicing_threshold;      // normalised group activity that opens the glottis
  // Two inertias, because a vocal tract has two. The tongue and jaw are heavy:
  // an articulatory posture is held for the length of a syllable, and that
  // persistence is also what lets a delayed reward find the behaviour that
  // earned it. The glottis is a valve — voicing and loudness start and stop in
  // tens of milliseconds. Giving both the same time constant makes the
  // creature either drone continuously or lose its posture between the sound
  // and the praise.
  float smoothing_ms;           // articulator inertia (formant targets)
  float gate_smoothing_ms;      // glottal inertia (voicing and loudness)
  float rate_norm_hz;           // group rate that reads as full scale
};

// Curiosity (§3.3) is a forward model of the next sensory frame plus two
// error windows. Reward is the *reduction* in error — learning progress —
// which is what keeps the baby off the noisy TV.
struct DnaCuriosity {
  float learn_rate;             // LMS rate of the forward model
  float fast_tau_ms;            // short error window
  float slow_tau_ms;            // long error window
  float gain;                   // progress -> reward
  // Predictive coding (DNA v15). How much of the predicted frame is taken off
  // the frame the ears deliver, before it is encoded. 0 is v14 exactly: the
  // forward model still runs and still pays curiosity reward, it just does not
  // reach the network. 1 subtracts the whole prediction, which makes B2 a pure
  // error module — it fires for the part of the sound the creature did not see
  // coming, and falls silent for the part it did.
  //
  // Two reasons to expect this to be worth more than the generic top-down
  // inhibition of v14. It is *specific*: the prediction for mel channel 7
  // cancels channel 7 and nothing else, where a random inhibitory tract turns
  // the whole module down. And the loudest predictable thing in this creature's
  // world is its own voice, which reaches its own ears (DNA v6) — so a forward
  // model driven by the same association module that drives the larynx is in a
  // position to cancel it, which is what reafference suppression is.
  //
  // The residual is rectified: a channel quieter than predicted reads as zero
  // rather than as negative energy, because the encoder's intensity population
  // code has no way to express "less than nothing". That is the standard
  // single-population approximation to predictive coding, and it does throw the
  // negative half of the error away. Splitting each channel's slice into ON and
  // OFF error units — the same trick the retina already uses here — is the
  // follow-up if this one measures well and the rectification is what limits it.
  float predict_gain;
  // What the forward model predicts. 0 is the model this creature has had
  // since M1: the absolute mel frame, from a coarse binning of association
  // activity and nothing else. 1 predicts the *change* from the frame before
  // it, so the model starts from "the world stays as it is" and has only to
  // learn the correction.
  //
  // Measured before it was built, in `pcprobe`. The absolute model loses to
  // persistence — predicting that the next frame looks like this one — in
  // every condition, and it is not learning its way out: R^2 goes 0.546 ->
  // 0.517 over four minutes while persistence holds flat. A prediction that
  // loses to a one-line baseline is not one to subtract from a sensory input,
  // and it is also a poor thing to pay curiosity reward for and a poor thing
  // for the growth detector to watch, which is why this is a switch on the
  // model rather than a detail of predictive coding.
  //
  // Persistence as a *prior* is not a hack bolted on to flatter the number.
  // Sensory adaptation is exactly this prior implemented in wetware: a neuron
  // that has been driven at one level stops reporting it, and what reaches the
  // next stage is the departure. Predicting the change and correcting it with
  // cortical state is the standard shape of a predictive-coding model.
  uint32_t persistence_base;
};

struct DnaModule {
  char name[kMaxNameLen];
  uint32_t role;                // ModuleRole
  uint32_t neurons;             // at birth
  uint32_t n_max;               // hard cap; the arena is sized for this
  uint32_t max_out_degree;      // axonal branching limit — see note below
  float extent[3];              // spatial volume
  float conn_radius;            // hard cutoff for intra-module wiring
  float conn_density;           // connection probability at zero distance
  float threshold;
  float v_rest;
  float leak_tau_ms;
  float refractory_ms;
  float target_rate_hz;         // homeostatic setpoint
  float inhib_fraction;
  float inhib_gain;             // inhibitory weights scale by this
  float weight_init;            // mean initial |weight|
  float noise_amp;              // spontaneous drive; also the babble source
  // The synaptic homeostasis hypothesis (§3.1 meets §3.6), per module: how
  // strongly this module's intrinsic plasticity and synaptic scaling act while
  // the creature is awake, and while it sleeps.
  //
  // Global versions of these were tried first and the result is why they are
  // here instead. Weakening regulation everywhere does preserve G2's reward
  // effect — measured directly, the rewarded-share advantage at 420 s goes from
  // +0.0012 to +0.0518 — and it also drives the babble duty cycle to 0.93,
  // because the vocal module already sits 1.8 Hz above its target and the same
  // dial was holding it down. Two demands on one number, in opposite
  // directions.
  //
  // A real neuromodulator is not a global constant either: it is released into
  // some regions and not others, which is exactly the freedom the single dial
  // was missing.
  //
  // v11 splits each of those two numbers again, because §3.1's two mechanisms
  // are not one mechanism with two names and the larynx is where that shows.
  // Intrinsic plasticity moves a threshold to hold a firing rate: that is what
  // keeps the babble duty cycle where the genome asked, and relaxing it is what
  // made the creature drone. Synaptic scaling multiplies a whole afferent set
  // back inside a band: that is what erases a rewarded weight change before it
  // can compound. Under v9 the larynx had to keep both at full strength to
  // avoid the drone, and so it kept the eraser too.
  //
  // Biologically the split is the more honest layout anyway. Threshold
  // regulation is a cell-intrinsic conductance change; synaptic scaling is a
  // receptor-trafficking process at the synapse. They share a purpose, not a
  // machine, and they run on different clocks.
  float ip_wake_scale;
  float ip_sleep_scale;
  float syn_wake_scale;
  float syn_sleep_scale;
  // How much this module's spontaneous drive answers to the exploration signal
  // (see DnaExploration). 0 leaves it at the fixed noise_amp above, which is
  // what every module did before DNA v10; 1 hands it the full range between
  // floor and ceiling.
  //
  // Per module because LMAN projects to RA and not to the whole brain. The
  // creature that explores is the one making the movement, and adding noise to
  // the ears in proportion to how badly the larynx is doing would be a
  // different and much stranger animal.
  float explore_scale;
  // Divisive normalisation (DNA v12). How strongly this module divides each
  // neuron's *synaptic* drive by how active the module currently is, relative
  // to its own target rate. 0 leaves the drive untouched, which is what every
  // module did before v12 and is bit-identical to it.
  //
  // Per module because normalisation pools are local: a cortical neuron is
  // divided by its own neighbourhood, not by the whole brain. Applied to the
  // synaptic drive only and not to noise or bias — normalisation is a circuit
  // phenomenon, and dividing the babble source by it would make the larynx
  // quieter rather than more selective.
  //
  // What it buys that intrinsic plasticity cannot: IP acts on one neuron over
  // seconds and pushes every neuron toward the *same* rate, which is a force
  // for a uniform code. Normalisation acts across the population within a tick
  // and lets the best-driven neurons keep their drive while the rest lose
  // theirs. It is also what separates "this module is busy" from "this module
  // is saying something different" — the confound that capped the
  // vision->central density at 0.06.
  float norm_gain;
  // How fast synapses *onto* this module learn, as a multiple of [stdp].eta
  // (DNA v13). 1.0 is what every module did before and is bit-identical to it.
  //
  // Postsynaptic, not presynaptic, because that is where plasticity is gated in
  // a real synapse — the NMDA receptor is on the receiving side. So this says
  // "this module learns fast", not "this module teaches fast", and a
  // hippocampus can learn quickly from a cortex that is still learning slowly
  // from everything else. That asymmetry is the whole point of a complementary
  // learning system: one rate would either wash out cortical statistics or make
  // episodes too slow to catch.
  float eta_scale;
  // DNA v20. Which neuromodulator channels this module's plasticity listens to.
  // Only read when [neuromod] enabled is true; otherwise the single summed
  // scalar is used and these are ignored.
  //
  // The creature's reward has four terms — the caregiver, hunger, comfort and
  // curiosity — and until v20 they were summed into one number delivered to
  // every synapse in the brain. `dwprobe` measured what that costs: with the
  // caregiver silent the weights still move 76% as much, because three of the
  // four channels are object-blind and none of them knows which toy is in view.
  // A real neuromodulator is not global either; it reaches some regions and not
  // others, which is the freedom the single dial never had.
  float nm_external;   // the caregiver
  float nm_hunger;
  float nm_comfort;
  float nm_curiosity;
  // DNA v21. A pooling interneuron class: this module subtracts
  // `ffi_gain` x the *population mean rate* of module `ffi_source` from every
  // one of its neurons' synaptic drive. -1 for ffi_source disables it.
  //
  // This is the one operation the measurements actually call for and the
  // genome could not express. `projprobe` showed that what a random sparse
  // tract delivers is dominated by an object-independent common mode, and that
  // subtracting each moment's population mean recovers the buried code —
  // vision 0.510 -> 0.940, central 0.500 -> 0.690, label-free. Divisive
  // normalisation cannot do it: `norm_gain` computes one scalar per module and
  // *multiplies*, which rescales the common mode and the signal together.
  // Subtraction changes their ratio; multiplication cannot.
  //
  // Why a dedicated class rather than the existing inhibitory neurons. A
  // `source = "inhibitory"` projection was tried on central->vocal and read
  // flat, because it draws a random subset of the source's own 20% inhibitory
  // pool — a poor estimator of the population mean, and one that carries its
  // own selectivity. A pooling interneuron samples the source broadly and
  // inhibits uniformly, which is what a PV basket cell does and what makes it
  // a common-mode estimator rather than another feature detector.
  // v24: the subtraction is now weighted per neuron by how much of the source
  // module that neuron actually receives, normalised to mean 1 so this gain
  // keeps the scale it had in v21/v22. A uniform term (v21-v23) does not match
  // what `centred` computes unless every target has identical fan-in, and at
  // density 0.03 the spread is about 18%.
  int32_t ffi_source;   // module index to pool from, -1 for none
  float ffi_gain;
  // DNA v40. Where the pooling interneuron lands, and whether its weight
  // learns. Both 0 is the pre-v40 module exactly.
  //
  // `ffi_apical` moves the subtraction from the soma's synaptic drive onto the
  // apical compartment. That is the whole architectural change: at the soma a
  // pooled inhibition is gain control, and on the tuft it is a prediction being
  // cancelled. The module needs an apical compartment for it to land in —
  // `apical_threshold` above zero and a tract marked apical — or the genome is
  // refused, for the same reason v29's gate is.
  //
  // `ffi_learn` is the rate at which each neuron's own `ffi_w_` moves to make
  // the residual zero:
  //
  //     ffi_w_[i] += ffi_learn * v_apical_[i] * ffi
  //
  // Positive residual means the tuft is receiving more than the interneuron
  // predicts, so the weight rises until it does not. It is clamped at zero
  // from below: an inhibitory pooling weight that went negative would stop
  // being inhibitory and the microcircuit would run away rather than settle.
  //
  // At `ffi_learn` 0 the weight stays at v24's in-degree-weighted value, which
  // is the mechanism that measurably worked (+0.077, 3/3 families) and is worth
  // keeping as the control arm rather than replacing.
  uint32_t ffi_apical;
  float ffi_learn;
  // DNA v25. The apical compartment. Until now a neuron here has been a point:
  // every synapse, from every source, summed into one membrane. A real
  // pyramidal cell is at least two electrically separated compartments — a
  // perisomatic one that feedforward input drives directly, and an apical tuft
  // 500 um away that top-down input arrives on and that cannot make the soma
  // fire on its own. What the tuft does instead is generate a slow regenerative
  // calcium plateau, and a soma that is *already* being driven responds to that
  // plateau with a burst. Larkum's coincidence detector: feedforward alone
  // gives regular spiking, feedback alone gives nothing, both together give a
  // burst.
  //
  // Why this creature needs the distinction rather than merely deserving it.
  // DNA v14 added top-down feedback and had nowhere to put it, so it lands on
  // the same membrane as the sensory input and is indistinguishable from it —
  // which is why it had to be built as *inhibition* to keep the loop from
  // diverging (see the v14 note above). A segregated compartment is the reason
  // real cortex can afford excitatory feedback: it modulates rather than
  // drives, so there is no positive feedback loop to run away.
  //
  // The gain is multiplicative on the synaptic drive, not additive to it, and
  // that is the whole content of "segregated". An additive plateau would let
  // apical input fire the soma by itself, which is exactly the property a
  // two-compartment neuron exists to deny. Multiplying preserves it: with no
  // feedforward drive there is nothing to amplify.
  //
  // apical_threshold <= 0 disables the compartment on this module, and no
  // projection marked apical means no input reaches it in any case, so a v24
  // genome is bit-identical under a v25 core.
  float apical_tau_ms;      // tuft integration constant; tens of ms, not the soma's 5
  float apical_threshold;   // plateau trigger; <= 0 disables the compartment
  float apical_gain;        // somatic gain during a plateau: drive x (1 + gain)
  float apical_plateau_ms;  // how long one plateau lasts
  // DNA v26. Subthreshold oscillations: a theta rhythm, a gamma rhythm nested
  // inside it, added to every neuron in the module as a current. Both
  // amplitudes at zero is off and is bit-identical to v25.
  //
  // The motivation is a specific measured failure, not the fact that cortex
  // oscillates. `eligprobe` shows the eligibility trace on central->vocal is
  // large but **object-blind** — it correlates +0.93 between the two
  // conditions. The reason is structural: pair-based STDP adds
  // `+a_plus * trace_pre` and subtracts `-a_minus * trace_post`, both scaling
  // with the same firing rates, so the rule is very nearly rate-balanced by
  // construction — and central codes the object as a *rate* difference. STDP
  // is a timing rule being asked to read a rate code.
  //
  // An oscillation is the standard way that conversion happens in cortex. On a
  // rising subthreshold ramp, a neuron with more synaptic drive crosses
  // threshold *earlier in the cycle* than one with less: phase-of-firing
  // coding. The rate difference the rule cannot see becomes a spike-timing
  // difference, which is the only thing it can see. The falsifiable prediction
  // is therefore not about accuracy anywhere — it is that the trace on
  // central->vocal becomes object-conditional, measured by the probe that
  // found it blind.
  //
  // Added as a current rather than as a gain, because that is what the
  // mechanism has to be for the argument above to hold. A multiplicative term
  // scales each neuron's own drive and cannot reorder threshold crossings; a
  // shared additive ramp is what makes crossing time a function of drive.
  //
  // Gamma nests in theta the way it does in cortex: `gamma_theta_coupling` at
  // 1 gives gamma full amplitude at the theta peak and none at the trough, at 0
  // gives a flat gamma independent of theta phase.
  float theta_hz;
  float theta_amp;
  float gamma_hz;
  float gamma_amp;
  float gamma_theta_coupling;
  // DNA v28. The critical period: how much this module can still learn, as a
  // function of its own age. The reward-gated learning rate onto this module is
  // multiplied by
  //
  //     critical_floor + (1 - critical_floor) * exp(-age / critical_tau_ms)
  //
  // so it starts at 1 and settles at the floor. `critical_tau_ms` <= 0 disables
  // it and is the pre-v28 creature.
  //
  // Per module because that is the one thing everybody knows about critical
  // periods: they do not close together. Ocular dominance shuts within months
  // and phonology stays open for years, in the same head. A single global
  // schedule would be the version that cannot express the interesting case.
  //
  // Why a brain would want plasticity to close at all is the part worth stating,
  // because it is not obvious that less learning is ever better. A rate that
  // stays high forever is a brain that keeps overwriting what it knows, and this
  // creature has the sharpest possible demonstration of that: G2's reward effect
  // was being *erased* by its own regulation until DNA v9 turned the regulation
  // down per module. Closing the period is the developmental version of the same
  // move — learn while the input is worth learning from, then protect it.
  float critical_tau_ms;
  float critical_floor;
  // DNA v29. Plateau-gated plasticity: how much the eligibility written onto
  // this module's neurons depends on their apical tuft being in a plateau.
  //
  //     factor = (1 - plateau_gate) + plateau_gate * (in a plateau ? 1 : 0)
  //
  // At 0 the rule is the v28 one exactly. At 1 a synapse becomes eligible only
  // during a plateau on its *postsynaptic* neuron.
  //
  // This is the one idea left that attacks the blocker head-on instead of
  // routing around it, and it is motivated by a measurement rather than by
  // theory. The blocker is that three-factor learning's third factor is a
  // global scalar: it can scale a whole tract up or down, but it is the same
  // number for every synapse, so it cannot make one tract do different things
  // for two different inputs. Eight variants of that scalar have now been
  // refuted, and the eligibility on central->vocal is object-*weak* (0.654
  // against the arcuate's 0.892 on the identical readout).
  //
  // A plateau is not a scalar. It is per-neuron, it is input-specific, and DNA
  // v25 measured that vocal's plateaus discriminate cube from ball at **0.835**
  // where the same neurons' spikes manage 0.524 — with an object-blind control
  // at 0.529, so the tuft reports what its input carries rather than being an
  // easier thing to classify. Gating eligibility on it makes the learning rule
  // conditional **by construction**: during a cube the plastic neurons are the
  // ones the cube's tuft input plateaus, during a ball they are a different
  // set, and a single global reward then writes two different things.
  //
  // Both halves of the STDP pair are gated, and on the postsynaptic neuron's
  // state in both cases. Gating potentiation alone would leave depression
  // running whenever the tuft is quiet, which is a net downward drift dressed
  // up as a gate rather than a gate.
  //
  // A gate > 0 on a module with no apical compartment is rejected rather than
  // silently ignored: it would multiply every eligibility by (1 - gate) and
  // switch learning off, which is a mechanism that looks like it ran.
  float plateau_gate;

  // DNA v32. Lateral competition inside a module, so a population code can
  // have a *place* in it.
  //
  // The problem it is built for, measured rather than assumed. The vocal
  // readout is a centroid over neuron index — value = sum(rate_i * i) / sum
  // (rate_i) — and every parameter read that way is pinned near the middle of
  // its range while every parameter read as a firing rate uses all of its. The
  // instantaneous centroid is not stuck: it ranges 0.068 to 0.880, which on the
  // formant map is F1 from 300 Hz to 910 Hz, real vowel territory at both ends.
  // What it never does is DWELL. An untuned group excited by an arbitrary input
  // produces a wandering average, and an average that wanders is noise however
  // wide it wanders — there is no posture for a reward to find twice.
  //
  // So this is not about widening the range. The acoustic yardstick says a
  // +/-1sd swing of the F1 the creature ALREADY delivers is d' = 3.2, which a
  // listener hears. It is about making a position in the group stable enough to
  // be aimed at.
  //
  // The mechanism is local excitation against the field mean: each neuron's
  // drive gains `lateral_gain` x (smoothed rate over its neighbourhood minus
  // the field's own mean rate). Locally-active regions reinforce themselves and
  // suppress the rest, which is the standard continuous-attractor arrangement
  // and produces a bump whose position is set by whichever part of the field
  // the afferents happen to favour. Index then means something — to a neuron's
  // neighbours rather than to its afferents, which is enough for the centroid
  // to have a metric under it.
  //
  // Rate-based rather than spike-based, exactly as `ffi_source` above already
  // is: this is the same interneuron approximation the pooling class uses, on
  // the same `rate_fast` the decoder reads.
  //
  // `lateral_fields` tiles the module into that many competitive fields using
  // the same `slice_begin` the decoders use, because a bump has to form inside
  // ONE readout group. A single field spanning the vocal module would light one
  // group and starve the other eight, taking voicing and loudness with it.
  //
  // gain 0 is off and is bit-identical to not having it.
  float lateral_gain;    // strength; 0 disables the whole mechanism
  float lateral_sigma;   // excitation width, as a fraction of one field
  uint32_t lateral_fields;  // competitive fields per module; 0 or 1 = whole module

  // DNA v37. What counts as a burst in this module, and what the apical
  // compartment does about it.
  //
  // `burst_ms` is the inter-spike interval under which a spike is scored as
  // part of a burst rather than as a lone event. 0 means this module has no
  // burst code at all and is bit-identical to the pre-v37 creature. A real
  // pyramidal burst is two to four spikes at 100-200 Hz, so the interval that
  // means "burst" is a handful of milliseconds and not a tenth of the
  // refractory period — set it too wide and every spike in a busy module is a
  // burst spike, which is a rate code with a new name.
  //
  // `burst_refrac_scale` is how the tuft gets a say. Larkum's BAC firing is a
  // dendritic calcium spike turning a somatic single spike into a burst, and
  // the cheapest faithful version of that here is to shorten the refractory
  // period while the tuft is in a plateau: the same drive then produces a
  // doublet where it produced one spike. 1.0 is no effect and is the pre-v37
  // creature exactly; below 1 an apical plateau makes bursting easier.
  //
  // This is the whole causal chain the mechanism needs — feedback arrives on
  // the tuft (v14 + v25), the tuft raises burst probability (here), and the
  // burst signs the weight change (DnaProjection::burst_learn) — and each link
  // is separately measurable by `burstprobe`, which is the point of splitting
  // it this way rather than hiding it in one number.
  float burst_ms;
  float burst_refrac_scale;

  // DNA v39. This module's eligibility time constant, as a multiple of the
  // genome's global tau_elig. 1.0 is what every module did before and is
  // bit-identical to it.
  //
  // Read on the POSTSYNAPTIC side — a synapse decays at its target module's
  // rate, not its source's — because that is what e-prop's prediction is
  // about: the trace's timescale tracks the history-dependence of the neuron
  // whose firing the credit is for.
  float elig_tau_scale;
};

// max_out_degree is what makes growth allocation-free: every neuron owns a
// fixed slice of the synapse pool, so adding a neuron in M4 never has to move
// anyone else's synapses. It costs memory we may not use — that is the trade
// for never calling an allocator after birth. The same slicing is reused for
// the reverse (incoming) index, so it bounds in-degree as well.

struct DnaProjection {
  uint32_t src, dst;            // module indices
  // For kRandom, the probability that any given pair is connected. For kGabor,
  // a thinning factor applied *after* the geometry has chosen the pair, so that
  // a genome can build a sparse map without changing the shape of the field.
  float density;
  float weight;
  float delay_ms;
  float delay_jitter_ms;
  uint32_t kind;                // ProjectionKind
  uint32_t source;              // ProjectionSource; kEither is the pre-v14 rule
  // DNA v23. A reward-INDEPENDENT Hebbian rate for this pathway alone. 0 is off
  // and is bit-identical to v22.
  //
  // DNA v19 showed a *global* Hebbian term cannot work: it potentiates every
  // tract at once — including the dense auditory->vocal arc — and the creature
  // drones at rates as low as 0.002, with no window between stable and useless.
  // But the classical-conditioning route only ever needed it on **one** pathway:
  // vision->vocal, the conditioned stimulus. The word already drives the right
  // vocal pattern natively (echo 0.825, above G3's 0.75 bar) and the object now
  // reaches vocal at 0.720, so pairing the two and binding only the CS side is
  // the one route to G3 that never needs reward to name the object.
  //
  // Stored as a (source module, target module) pair rather than per synapse,
  // because the two tracts that must be separated differ in their source and
  // the kernel already knows both ends. The consequence is that two projections
  // sharing a src/dst pair share this value; nothing in the shipped genome does.
  //
  // MEASURED 2026-08-16. It does what it was built to do and the conditioning
  // still does not happen. With the CS tract at d=0.03 / w=0.30 and `hebb` on
  // it alone, scored by `pairprobe` (teach object+word, probe the object alone)
  // against `g3probe` at the same rate as the unpaired control:
  //
  //   hebb    cal    duty   PAIRED   unpaired  shuffled  echo
  //   0.0     PASS   0.69   0.538    0.541     0.485     0.700
  //   0.01    PASS   0.84   0.575    0.577     0.485     0.650
  //   0.05    PASS   0.81   0.541    0.558     0.470     0.775
  //   0.15    PASS   0.79   0.561    0.549     0.468     0.650
  //
  // PAIRED never separates from unpaired: -0.003, -0.002, -0.017, +0.012.
  //
  // The mechanism itself is a success and worth keeping. DNA v19's global
  // Hebbian term droned the creature at 0.002 with no viable window at all;
  // confined to one sparse tract it is stable at **0.15**, fifty times that
  // rate, still calibrated and still babbling. Per-pathway confinement solves
  // the stability problem exactly as intended. What it does not do is make a
  // paired CS come to evoke the response.
  //
  // --- and the one rule that did NOT read this trace: also refuted -----------
  // Built and removed 2026-08-20 as `covar`, a per-pathway **rate covariance**
  // applied once per plasticity interval and multiplying no eligibility at all:
  //
  //     dw_ij = covar * (r_i - mean_r(src)) * (r_j - mean_r(dst))
  //
  // The motivation was the sharpest available. *Every* learning write in this
  // creature, `hebb` above included, multiplies a quantity assembled from spike
  // timing inside +-20 ms windows — while central codes the object as a rate
  // difference over hundreds of ms. Every rule tried had been a timing rule
  // asked to read a rate code. It also predicted the one result nothing else
  // explains: the decile test, where a synapse off central's most
  // object-discriminative neuron carries no more conditional eligibility than
  // one off its least, which is inexplicable for a rule that reads rates.
  //
  // It is not inert and it is not unstable. `babble` PASSes across 1e-7..1e-1,
  // six orders of magnitude, and `dwprobe` shows it writing hard — mean |dw|
  // per synapse 2.99e-02 -> 1.64e-01. What it writes is the problem:
  //
  //   covar   mean|dw|    corr(A,A')  corr(A,B)   noise  obj-indep  obj-spec
  //   0       2.99e-02    0.349       0.322       65%    32%        2.7%
  //   1e-4    1.64e-01    0.630       0.616       37%    62%        1.4%
  //   1e-2    1.82e-01    0.349       0.308       65%    31%        4.1%
  //   1e-1    1.89e-01    0.246       0.275       75%    28%        -2.9%
  //
  // Large, reproducible and **object-independent**, with the object-specific
  // share bouncing around zero. Then the milestone test, both CS tracts on,
  // three seed families, each against its own covar = 0 control and read
  // against the measured 0.115 noise floor of pairprobe-minus-g3probe:
  // **-0.009, -0.011, +0.050.** Null, and every arm readable.
  //
  // **The diagnosis, and it is why this file stops here.** Centring on the
  // population mean removes the *population's* offset and not each neuron's
  // own: `r_i - mean_r(module)` is dominated by the fact that some cells simply
  // fire faster than their neighbours, which is a static property of the
  // wiring. The product of two static offsets is a fixed pattern — reproducible
  // and object-blind, which is exactly what dwprobe measured. A true covariance
  // would centre each neuron on *its own* running mean, which needs a second
  // per-neuron array and a snapshot format bump.
  //
  // That refinement is named and not built, deliberately. This was the seventh
  // mechanism to hit the same wall — a small differential riding on a large
  // common component — and the point at which the search was stopped rather
  // than the point at which it ran out of ideas.
  float hebb;
  // DNA v25. Does this tract terminate on the target's apical tuft rather than
  // near its soma? See DnaModule::apical_threshold for what the compartment
  // does. Per pathway because that is what the anatomy is: feedback from a
  // higher area arrives in layer 1 on the tuft, feedforward from a lower one
  // arrives in layer 4 near the soma, and the same two cells are involved
  // either way. Nothing about a synapse's source or its weight says which; only
  // the tract does.
  //
  // Carried as a (source, target) module pair inside the kernel for the same
  // reason `hebb` is, with the same consequence: two projections sharing a
  // src/dst pair share this flag.
  uint32_t apical;
  // DNA v28. Synaptic exuberance: this tract is *born* at `density x
  // exuberance` and pruned back by experience. 1.0 is the pre-v28 genome.
  //
  // The developmental arc every mammal runs and this creature has never had.
  // A human cortex overproduces synapses through the first years and then
  // removes roughly half, and the half that survives is the half that carried
  // traffic. This creature does the opposite: it is born at its adult density
  // and M4 *grows* into a plateau.
  //
  // Motivated by a named open candidate rather than by the developmental
  // biology. `projprobe` measured why the object dies crossing a sparse tract —
  // a random projection pools, and pooling buries a fine distributed code under
  // an object-independent common mode — and listed "make the tract structured
  // rather than random, so each target samples a feature-defined group instead
  // of a uniform random subset" as the second thing to try. Activity-dependent
  // pruning from an exuberant start is how biology builds exactly that, and it
  // is the version that needs no hand-designed hierarchy: three of those have
  // been built here and all three attenuated the code.
  //
  // The pruning rule already exists and needs no changes — sleep removes a
  // synapse that is weak AND idle (DnaConsolidate::prune_weight and
  // prune_traffic). Exuberance only supplies the surplus for it to select from.
  //
  // Costs fan-out at birth: a tract at exuberance 3 needs three times the
  // out-degree headroom, so `max_out_degree` on the source module has to be
  // raised with it or the surplus is dropped rather than pruned, which is the
  // opposite of the experiment. Raising that cap is wiring-neutral; raising
  // n_max is not.
  float exuberance;
  // DNA v30. The companion that makes exuberance mean something: every synapse
  // of this tract is born at `weight` x this. 1.0 is the pre-v30 creature.
  //
  // v28 shipped `exuberance` alone and it was **inert by arithmetic** — tripling
  // the born density changed the number of synapses pruned from 58 to 52.
  // Pruning takes a synapse that is weak AND idle, `prune_weight` is 0.004, and
  // the tract is born at 0.12 +/- 30%. Nothing born could ever become weak
  // enough to qualify, so every synapse was equally established from birth,
  // selection had no gradient, and exuberance was a permanent density increase —
  // worse than neutral, since projprobe measures a random tract's capacity
  // *falling* with density.
  //
  // Development does not overproduce adult-strength contacts. It overproduces
  // *weak* ones and keeps the ones traffic strengthens, and that is the whole
  // difference between a surplus that can be selected from and a surplus that
  // cannot.
  //
  // Two arithmetic facts make this cheap, both checked before it was built:
  //
  //   - Set `exuberance * birth_weight = 1` and the module's total input drive
  //     at birth is unchanged, so the arm needs no recalibration. At E=3,
  //     B=1/3 a synapse is born at 0.04 against a 0.004 floor: a 90% reduction
  //     for R-STDP to achieve rather than 97%, and the same absolute distance
  //     ten times smaller.
  //   - Sleep's downscale floor does not fight it. `w_in_struct_` is captured
  //     as the *actual* sum of birth weights, not the genome's nominal
  //     `weight`, so a weak-born tract gets a proportionally lower floor — and
  //     the floor is on the per-neuron sum while downscaling multiplies all of
  //     a neuron's inputs equally, so it can never single a synapse out anyway.
  //     Differential weakening comes only from R-STDP's signed write.
  //
  // MEASURED AND REFUTED, 2026-08-17, and the reason is the useful part. Four
  // arms on `vision->central`, 1.6M ticks so sleep consolidates seven times,
  // `max_out_degree` raised on both ends so nothing is dropped:
  //
  //   exuberance  birth_weight  born at  synapses pruned  G4
  //   1.0         1.0           0.12     58               PASS
  //   3.0         1.0           0.12     52               PASS
  //   3.0         0.333         0.04     40               PASS
  //   3.0         0.100         0.012    47               PASS
  //
  // No trend. Born at a tenth of adult strength — three times *below* the
  // downscale floor's reach and only 3x above the prune floor — the surplus is
  // still not selected against. The counts are brain-wide, which is an
  // instrument limit worth fixing, but it is an *upper bound* on the tract and
  // the bound already settles it: at most 58 of ~18,400 exuberant synapses,
  // 0.3%, in a creature that lived seven sleep cycles.
  //
  // So v28's diagnosis was right that born-strong blocks selection, and wrong
  // that it was the only thing blocking it. The binding constraint is the other
  // half of the rule: pruning takes a synapse that is weak AND **idle**, and in
  // an exuberant tract every synapse carries traffic because the source fires.
  // An absolute traffic floor cannot express what development actually does,
  // which is **competition** — a synapse is removed because its neighbours on
  // the same target won, not because it fell below a fixed bar.
  //
  // The requirement for a third attempt is therefore not another genome field:
  // `consolidate()` would have to rank a target neuron's afferents against each
  // other and prune the losers. Both fields stay, because that change would
  // make them live; they are inert-but-revivable rather than refuted outright.
  float birth_weight;

  // DNA v36. Dynamic synapses (Tsodyks & Markram), the mechanism Webb's cricket
  // model puts song recognition in. 0 is off and is bit-identical to the
  // pre-v36 creature — no state is allocated, no table is built, and the
  // delivery loop keeps its old cost behind one branch.
  //
  // `stp_use` is U, the fraction of a synapse's available resources released by
  // one presynaptic spike. It is both the depth of the effect and its switch:
  // at 0 the synapse is the constant-weight one it has always been, and at 1 a
  // single spike empties it.
  //
  // Per spike, with `gap` the interval since this synapse last transmitted:
  //
  //   u <- U + u_prev (1 - U) exp(-gap / stp_facil_ms)     utilisation
  //   R <- 1 + (R_prev (1 - u_prev) - 1) exp(-gap / stp_recover_ms)  resources
  //   delivered <- weight * u * R / U
  //
  // Both stay inside [0, 1] by construction rather than by clamping, which is
  // worth stating because a clamp would hide a genome that had gone unstable.
  //
  // The two time constants are what make one synapse a low-pass and another a
  // high-pass, and that is the whole mechanism:
  //
  //   depressing   large U, long stp_recover_ms, stp_facil_ms 0. Passes the
  //                first spike of a burst and little of the rest. Webb's BN1,
  //                and the per-edge common-mode remover described above.
  //   facilitating small U, short stp_recover_ms, long stp_facil_ms. Passes
  //                almost nothing at rest and builds under a fast train.
  //                Webb's BN2, which is what turns "a burst started" into
  //                "bursts are arriving at the conspecific rate".
  //
  // Carried as a (source, target) module pair inside the kernel for the same
  // reason `hebb` and `apical` are, and with the same consequence: two
  // projections sharing a src/dst pair share these three numbers. Webb's pair
  // needs two different *targets* rather than two tracts onto one, which is
  // what the anatomy is anyway.
  float stp_use;          // U: released per spike. 0 = the mechanism is off
  float stp_recover_ms;   // tau_rec: how fast resources come back
  float stp_facil_ms;     // tau_facil: how long utilisation stays raised. 0 = none

  // DNA v37. The rate at which this pathway learns from the POSTSYNAPTIC
  // neuron's burst signal, rather than from the global reward scalar. 0 is off
  // and bit-identical to the pre-v37 tract.
  //
  //   dw = burst_learn * eta_scale * credit * (burst_rate_post - burst_base_post)
  //
  // Signed by construction: a target bursting above its own running baseline
  // potentiates whatever fired into it, one bursting below depresses. The
  // baseline is what makes it a *deviation* rather than a rate, and without it
  // the term could only ever potentiate — the same argument as v16's
  // eligibility baseline, one level up.
  //
  // Per pathway rather than global for the reason v23's `hebb` is, and with
  // the same measured motivation: v19's global Hebbian term potentiated every
  // tract at once, including the dense arcuate, and the creature droned.
  //
  // Carried as a (source, target) module pair inside the kernel, so two
  // projections sharing a pair share this rate.
  float burst_learn;

  // --- kGabor only -----------------------------------------------------------
  // A simple cell in the Hubel–Wiesel sense: an oriented envelope over the
  // retina, with the ON channel driving its positive lobe and the OFF channel
  // its negative one.
  //
  // Sigma and lambda are in *cell pitches* at the receptive field's own
  // eccentricity, the same unit DnaVision's centre and surround already use.
  // Fixed retinal units would not work: the fovea is sampled eight times finer
  // than the outer ring, so one field size is either blind in the periphery —
  // three sigma falls between two cells and the neuron has no afferents at all
  // — or a blur in the middle. Quoting them in pitches makes every simple cell
  // the same shape in the sampling lattice it actually sits in, which is what
  // an eccentricity-invariant field means.
  float rf_sigma;               // envelope width across the preferred orientation
  float rf_lambda;              // wavelength: one full ON-OFF-ON cycle
  float rf_aspect;              // elongation *along* the orientation (gamma < 1)
  // Cortical magnification. The target module's x and y are uniform over its
  // volume, but the retina is not uniform: 64 of the default body plan's 88
  // ganglion cells are inside the fovea, which is a sixteenth of the field.
  // Mapping position straight through would spend 94% of the visual cortex on
  // the coarse periphery and leave the fovea — the only place a held-up toy
  // ever is — with thirty neurons.
  //
  // s -> sign(s)*|s|^k about the centre of the field, so k = 1 is no
  // magnification and larger k pulls the map inward. This is the one place the
  // model is unapologetically a caricature of the real thing: cortex warps
  // roughly logarithmically, and a power law is the same shape with one
  // heritable number instead of three.
  float rf_magnification;
  // Minimum |lobe| a cell must sit in, as a fraction of the peak, before it is
  // wired at all. This is what keeps the field a field: without it every
  // retinal cell connects to every V1 neuron with a vanishing weight, and the
  // out-degree caps eat the map before the tick loop ever sees it.
  float rf_floor;

  // --- kCurvature only -------------------------------------------------------
  // The preferred radius of curvature, in normalised retinal units, spread over
  // the target's z axis exactly as orientation is spread over V1's. A cell at
  // z = 0 prefers the tightest curve its body plan can represent and one at
  // z = 1 the broadest; between them the module tiles the range of sizes a toy
  // held up to the creature can subtend.
  //
  // rf_sigma is reused here as the *radial* tolerance — how far off its
  // preferred circle a contributing edge may sit — since that is already what
  // it means, a distance in cell pitches. rf_lambda and rf_aspect are not read
  // by this kind, and rf_floor and rf_magnification mean what they do above.
  float rf_radius_min;
  float rf_radius_max;
  // How closely an edge's orientation must match the tangent of the circle at
  // its position, in radians. Wide, and every edge in the neighbourhood counts
  // and the cell is a contrast meter again; narrow, and the discrete retina
  // cannot supply enough matching edges to fire it at all.
  float rf_tangent_sigma;
};

struct DnaHeader {
  uint32_t magic;
  uint32_t version;
  uint64_t seed;
  uint32_t module_count;
  uint32_t projection_count;
  DnaSim sim;
  DnaStdp stdp;
  DnaHomeo homeo;
  DnaGrowth growth;
  DnaConsolidate consolidate;
  DnaDrives drives;
  DnaAudio audio;
  DnaVision vision;
  DnaTouch touch;
  DnaVocal vocal;
  DnaCuriosity curiosity;
  DnaExploration exploration;
  DnaNormalisation normalisation;
  DnaNeuromod neuromod;
  DnaInterneuron interneuron;
};

// The retina's shape, derived from the genome by one formula that the host
// sampler, the core encoder, and the genome validator all call. Three
// independent copies of this arithmetic would be three chances to disagree
// about how many numbers a frame contains, and the symptom would be a brain
// quietly reading the wrong cells.
//
// Ring 0 is the fovea. Each further ring doubles the window it covers and is
// sampled by the same small grid, so acuity halves with every step outward;
// the middle of each ring is dropped because the ring inside it already
// covers that ground at higher resolution.
uint32_t vision_rings(const DnaVision& v);
uint32_t vision_cells(const DnaVision& v);
inline uint32_t vision_features(const DnaVision& v) { return vision_cells(v) * 2; }

// Where a ganglion cell sits and how wide its field is, in normalised [0,1]
// frame coordinates. The host sampler used to own this arithmetic and the core
// had no opinion about it, which was fine for as long as nothing in the core
// needed to know *where* a retinal cell was looking. A receptive field does,
// so the layout moved here and the host now reads it back rather than
// recomputing it — the alternative is two descriptions of the same retina that
// are free to drift apart, and the symptom of that is a visual cortex wired to
// a picture of the world that nothing is sending it.
struct VisionCell {
  float u, v;      // centre, normalised to the frame
  float pitch;     // cell spacing at this eccentricity, likewise
};
VisionCell vision_cell(const DnaVision& v, uint32_t cell);

// A module's neurons split into `groups` contiguous, near-equal slices. This is
// the channel map every transducer uses — mel channel c, or retinal feature f,
// is whichever neurons currently fall in the c-th slice — and now also what
// lets a structured projection ask which retinal cell a source neuron carries.
//
// The two must stay exact inverses of each other: slice_of() is the unique g
// with slice_begin(g) <= index < slice_begin(g+1). A rounding disagreement here
// is a brain wired to the neighbouring cell, which is a difference no test
// looks at directly.
inline uint32_t slice_begin(uint32_t count, uint32_t groups, uint32_t g) {
  return uint32_t(uint64_t(count) * g / groups);
}
inline uint32_t slice_of(uint32_t count, uint32_t groups, uint32_t index) {
  if (count == 0 || groups == 0) return 0;
  // ceil((index+1) * groups / count) - 1
  const uint64_t n = (uint64_t(index) + 1) * groups;
  const uint32_t g = uint32_t((n + count - 1) / count) - 1;
  return g < groups ? g : groups - 1;
}

enum class DnaStatus {
  kOk = 0,
  kTooSmall,
  kBadMagic,
  kBadVersion,
  kBadCounts,
  kSizeMismatch,
  kBadModule,
  kBadProjection,
  kBadRole,
  kBadAudio,
  kBadVision,
  kBadPlasticity,
  kBadGrowth,
};

const char* dna_status_string(DnaStatus status);

// A non-owning view over a validated blob.
class Dna {
 public:
  DnaStatus load(const void* blob, size_t size);

  const DnaHeader& header() const { return *header_; }
  uint32_t module_count() const { return header_->module_count; }
  uint32_t projection_count() const { return header_->projection_count; }
  const DnaModule& module(uint32_t i) const { return modules_[i]; }
  const DnaProjection& projection(uint32_t i) const { return projections_[i]; }

  // Index of the module carrying a role, or -1. Roles are unique: the loader
  // rejects a genome with two auditory modules rather than picking one.
  int32_t module_with_role(ModuleRole role) const;

  // Total neurons at birth, and the ceiling the arena must be sized for.
  uint32_t total_neurons_at_birth() const;
  uint32_t total_neurons_max() const;

  bool valid() const { return header_ != nullptr; }

 private:
  const DnaHeader* header_ = nullptr;
  const DnaModule* modules_ = nullptr;
  const DnaProjection* projections_ = nullptr;
};

}  // namespace aibaby

#endif  // AIBABY_DNA_H
