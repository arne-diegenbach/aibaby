// Leaky integrate-and-fire network with spatial wiring, axonal delay, and
// reward-modulated STDP.
//
// Layout is struct-of-arrays throughout: the tick loop is a streaming pass
// over neurons and the plasticity pass is a streaming pass over synapses, and
// neither wants to drag the other's fields through cache.
//
// Every array is sized for n_max at birth, so growth (M4) is a bump of
// module.count, never an allocation.

#ifndef AIBABY_NETWORK_H
#define AIBABY_NETWORK_H

#include "aibaby/arena.h"
#include "aibaby/config.h"
#include "aibaby/dna.h"
#include "aibaby/rng.h"

namespace aibaby {

class SnapshotReader;
class SnapshotWriter;

struct ModuleState {
  uint32_t begin;      // first neuron index in the global arrays
  uint32_t count;      // slots in use: live + tombstoned. Grows in M4.
  uint32_t capacity;   // n_max from DNA
  uint32_t spikes;     // spikes emitted this tick
  uint32_t dead;       // tombstoned slots inside [begin, begin+count)
  Scalar mean_rate;       // Hz, one-second EMA across the module
  Scalar mean_rate_fast;  // Hz, tens of ms — the divisive-normalisation pool
  // The divisive-normalisation factor in force this tick (DNA v12). Exposed on
  // the state rather than kept private because a module being quietly divided
  // to a third of its drive is invisible in every rate and weight, and it is
  // the first thing to check when normalisation is on and a number moves.
  Scalar norm;

  uint32_t live() const { return count - dead; }
};

// What structural plasticity has done so far. §9 asks the telemetry panel to
// show growth and prune events, and it is right to: a brain that quietly grew
// to its cap and a brain that never grew look identical from every other
// number, and only one of them means learning is broken.
struct StructuralStats {
  uint32_t growth_events;      // times the three §3.4 conditions all held
  uint32_t neurons_grown;
  uint32_t consolidations;     // sleep passes run
  uint32_t synapses_pruned;
  uint32_t neurons_pruned;
  uint32_t replays;            // episodes re-experienced during sleep
  uint64_t last_growth_tick;
  uint64_t last_prune_tick;
};

// Everything the host needs to render a frame, with no core internals leaking.
struct Telemetry {
  uint64_t tick;
  uint32_t total_spikes;
  uint32_t live_neurons;
  uint32_t live_synapses;
  Scalar mean_rate_hz;
  Scalar mean_weight;      // mean |w| over live synapses
  Scalar mean_eligibility; // mean |e|: shows the trace filling and draining
  Scalar last_reward;
};

// DNA v36. How many distinct dynamic-synapse parameter sets one genome may
// have, and the shape of the decay tables each of them owns. Eight pathways is
// more than the shipped body plan has tracts; the table spans
// kStpCoarse * kStpCoarseSteps = 2048 ticks, which is two seconds at the
// default dt and past the point where any recovery constant still matters.
constexpr uint32_t kMaxStpPaths = 8;
constexpr uint32_t kStpCoarse = 32;
constexpr uint32_t kStpCoarseSteps = 64;

class Network {
 public:
  // Exact arena size this genome needs, so the host can allocate once and
  // know a build cannot fail. Kept adjacent to the allocations in build() —
  // the two must be edited together.
  static size_t required_bytes(const Dna& dna);

  // Sizes and wires the brain from the genome. Returns false only if the
  // arena is smaller than required_bytes().
  bool build(const Dna& dna, Arena& arena, Rng& rng);

  // One simulation timestep: integrate, spike, accumulate eligibility, and
  // deliver spikes into the delay line.
  void step();

  // Cashes the eligibility traces in against a reward. Called by Brain once
  // per plasticity interval; `reward` is the reward *integrated over that
  // interval*, not an instantaneous rate.
  void apply_reward(Scalar reward);
  // DNA v20: the same pass, but each module weights the four channels itself.
  void apply_reward_channels(const Scalar* channels);

 private:
  void apply_reward_impl(const Scalar* per_module, bool any);
  void capture_ffi_weights();

 public:

  // Intrinsic plasticity and synaptic scaling. Called on the homeostasis
  // interval. Without this the network saturates or falls silent (§3.1).
  //
  // `asleep` is how §3.6 gets a say in §3.1: each module carries a wake and a
  // sleep multiplier for *each* of the two mechanisms — ip_*_scale and
  // syn_*_scale — and this picks the pair that applies. Every module at 1.0 is
  // the unscaled rule.
  void homeostasis(bool asleep);

  // --- M4: structural plasticity ---

  // Myelination (§3.5). Reads each edge's traffic counter and moves its axonal
  // delay toward the floor; the matching per-edge learning rate is applied
  // inside apply_reward(), where it belongs. Called on the homeostasis
  // interval — the effect accumulates over minutes, so nothing needs it faster,
  // and it is a full pass over the synapse pool.
  void myelinate();

  // Is this module saturated in the sense §3.4 means — firing high *and* out of
  // weight headroom? Exposed because the growth decision is split across two
  // layers: Brain owns the plateau detector (it owns the critic), Network owns
  // saturation and the budget cap.
  bool saturated(uint32_t module) const;

  // Mean |w| over the incoming synapses of a module's live neurons — the
  // "weights near bounds" half of that test. Exposed separately so an
  // experiment can report how far a module sits from the bar, rather than only
  // whether it crossed it: a growth trigger that never fires and one that
  // nearly fires are very different states of the same creature.
  Scalar mean_in_weight(uint32_t module) const;

  // What fraction of a module's incoming synapses sit at or above
  // `frac` of the weight ceiling.
  //
  // This exists because the mean cannot answer the question §3.4 is asking.
  // Synaptic scaling holds each neuron's *total* input weight inside a band,
  // so the mean is pinned by homeostasis no matter how much the module has
  // learned — it is a number about the regulator, not about the module. What
  // learning does under that constraint is redistribute: some edges are driven
  // to the ceiling and others toward zero. The count at the ceiling is
  // therefore the honest reading of "this module has run out of ways to tell
  // its inputs apart", and it is free to move while the mean is not.
  Scalar in_weight_fill(uint32_t module, Scalar frac) const;

  // Does §3.4's third condition hold: is there room under the DNA budget cap?
  bool below_cap(uint32_t module) const;

  // May this module grow at all? False for every transducer module, and the
  // reason is not a policy choice — see the note in the .cpp. Growth is only
  // ever attempted where it is meaningful.
  bool growable(uint32_t module) const;

  // Insert up to `k` neurons at the spatial centroid of the module's
  // highest-error region and wire them to their local neighbours (§3.4).
  // Returns how many were actually inserted, which is less than `k` when the
  // budget cap intervenes.
  uint32_t grow(uint32_t module, uint32_t k);

  // One sleep consolidation pass (§3.6): synaptic downscaling, then pruning of
  // synapses that are both weak and idle, then of neurons left with nothing
  // connected to them. Never call this awake — it rebuilds the reverse index,
  // and §3.4 is explicit that structural surgery happens in the safe window.
  void consolidate();

  const StructuralStats& structural() const { return structural_; }

  // DNA v38. How many synapses the last prune pass removed for losing to their
  // neighbours rather than for being weak and idle. Separate from
  // structural_.synapses_pruned because the two answer different questions: a
  // pass that removed 40 synapses tells you nothing about whether competition
  // is what removed them, and a mechanism that ships off must be visibly off.
  uint32_t competed_out() const { return competed_out_; }

  // Replay is the Brain's business — it owns the episodes and the encoders —
  // but the count belongs beside the other structural events, because it is
  // the third of the three things §3.6 says sleep does and the panel shows
  // them together.
  void note_replay() { ++structural_.replays; }

  // Per-edge learning rate after myelination, as a fraction of eta, averaged
  // over live synapses. One number for "how consolidated is this brain" — 1.0
  // at birth, falling toward eta_floor_frac as pathways establish themselves.
  // Cached: refreshed by myelinate(), not recomputed per call.
  Scalar mean_plasticity() const;

  // Inject current into a neuron before the next step. This is how the
  // sensory encoders drive the brain.
  void inject(uint32_t neuron, Scalar current);

  // Scale a module's spontaneous drive (DnaExploration). 1.0 is the genome's
  // noise_amp unchanged; the Brain moves it as performance changes. Multiplying
  // the amplitude does not change how many random numbers a tick draws, so at
  // 1.0 everywhere the creature is bit-identical to one built before this
  // existed — which is what makes any sweep over it readable.
  void set_exploration(uint32_t module, Scalar mult) {
    if (module < kMaxModules) explore_mult_[module] = mult;
  }
  Scalar exploration(uint32_t module) const {
    return module < kMaxModules ? explore_mult_[module] : kOne;
  }

  // DNA v39. Mean |eligibility| over the synapses TERMINATING on one module.
  // By target and not by source because that is the side v39's time constant is
  // read on, and a probe measuring the mechanism has to slice it the same way
  // the mechanism does.
  // DNA v38. One neuron's mean afferent weight, for `pruneprobe`. The
  // per-module mean_in_weight() above cannot answer a question about
  // selection: pruning is a comparison *within* one target's inputs, and a
  // module-wide mean averages over every such comparison at once.
  Scalar mean_in_weight_of(uint32_t neuron) const;

  Scalar mean_eligibility(uint32_t module) const;

  Telemetry telemetry() const;

  // FNV-1a over live neuron and synapse state. The G1 lever: two runs of the
  // same genome and the same inputs must agree here on every tick.
  uint64_t state_hash() const;

  // --- Snapshot seam (§8) ---
  //
  // Only what a tick can change and the arena does not already hold: every
  // array here is arena memory, which the snapshot copies wholesale, and every
  // remaining field is derived from the genome and comes back with build().
  // Keep this pair next to the fields it walks — a field added above and not
  // added here is a creature that resumes almost right.
  //
  // The pair is not versioned itself: the number of bytes it writes is the
  // snapshot's layout fingerprint, so changing what goes in here invalidates
  // old files loudly instead of silently.
  void save_state(SnapshotWriter& w) const;
  void load_state(SnapshotReader& r);

  uint64_t tick() const { return tick_; }
  uint32_t module_count() const { return module_count_; }
  const ModuleState& module(uint32_t i) const { return modules_[i]; }
  const DnaModule& module_dna(uint32_t i) const { return dna_.module(i); }

  // Spikes emitted during the most recent step, as global neuron indices.
  const uint32_t* spikes() const { return spike_list_; }
  uint32_t spike_count() const { return spike_count_; }

  // Sampled membrane potential, for the telemetry trace.
  // DNA v36. Does this genome have a dynamic synapse anywhere, and what is one
  // pathway currently delivering?
  //
  // `stp_gain` is the mean of the release factor u*R/U over the live synapses
  // of one (source, target) pair, so 1.0 is "delivering the genome's weight"
  // and 0.3 is "delivering three tenths of it". It is read as of each synapse's
  // most recent transmission, which is the only moment the quantity exists —
  // a synapse that has not fired for a second holds the value it last
  // delivered with, and a sampler has to be reading during traffic for the
  // number to mean anything.
  //
  // Exposed for the same reason ModuleState::norm is: a tract quietly
  // delivering a third of its weight is invisible in every rate, every weight
  // and every hash, and it is the first thing to check when this is on and a
  // number moves.
  // DNA v37. The burst code, for `burstprobe`. `burst_rate` is the neuron's
  // fast running rate of burst spikes and `burst_base` the slow baseline the
  // learning signal is a deviation from — their difference IS the third factor
  // burst-dependent plasticity substitutes for the global reward scalar, so a
  // probe that cannot see both cannot tell a silent mechanism from a balanced
  // one.
  bool burst_active() const { return any_burst_; }
  Scalar burst_rate(uint32_t neuron) const {
    return any_burst_ ? burst_rate_[neuron] : Scalar(0);
  }
  Scalar burst_base(uint32_t neuron) const {
    return any_burst_ ? burst_base_[neuron] : Scalar(0);
  }

  bool stp_active() const { return any_stp_; }
  Scalar stp_gain(uint32_t src, uint32_t dst) const;

  Scalar membrane(uint32_t neuron) const { return v_[neuron]; }
  Scalar threshold(uint32_t neuron) const { return threshold_[neuron]; }

  // DNA v25. Is this neuron's apical tuft in a plateau right now? Exposed
  // because the question the compartment was built to answer is measurable
  // only from here: a plateau is not a spike, so it appears in no rate, no
  // weight and no hash, and a tuft that never fires and a tuft that fires
  // constantly look identical from outside.
  //
  // Read after step(), which has already advanced tick_, so this answers for
  // the tick about to be simulated rather than the one just finished. The
  // difference is the final tick of each plateau, about 2% of a 50 ms one, and
  // it falls on every neuron equally — it cannot manufacture a difference
  // between conditions, only shorten every plateau by the same tick.
  bool in_plateau(uint32_t neuron) const {
    return any_apical_ && tick_ < plateau_until_[neuron];
  }
  Scalar apical_membrane(uint32_t neuron) const {
    return any_apical_ ? v_apical_[neuron] : Scalar(0);
  }

  // DNA v29. What fraction of the potentiation events offered to the gate got
  // through it, since birth. This is the whole did-it-run guard for plateau
  // gating: 0.0 is a gate that never opens and 1.0 is a gate that is never
  // shut, and both are inert while producing the same null as a gate that runs
  // and does not help — the lesson of DNA v11's synaptic scaling, which was
  // tuned for a week before a direct measurement showed it never executed.
  bool plateau_gated() const { return any_plateau_gate_; }
  double plateau_pass_rate() const {
    return plateau_events_ ? double(plateau_hits_) / double(plateau_events_) : 0.0;
  }
  uint64_t plateau_gate_events() const { return plateau_events_; }

  // DNA v26. This module's gamma phase for the tick just simulated, in turns
  // [0,1). Exposed for the same reason `in_plateau` is: an oscillation that
  // fails to entrain and an oscillation that entrains and does not help
  // produce the same null everywhere else, and only a phase readout separates
  // them. Subtracts one tick because step() has already advanced tick_.
  Scalar gamma_phase(uint32_t module) const {
    if (!any_osc_ || module >= module_count_) return Scalar(0);
    const uint64_t t = tick_ ? tick_ - 1 : 0;
    const uint32_t ph = uint32_t(t * osc_gamma_inc_[module]);
    return Scalar(double(ph) / 4294967296.0);
  }
  bool oscillating() const { return any_osc_; }

  // Where a neuron sits in its module's volume (§3.2). Read-only, and exposed
  // because a structured projection makes position *meaningful* rather than
  // merely a wiring input: a V1 neuron's coordinates are its receptive field,
  // so checking that the cortex is tuned the way its map says it should be
  // means comparing a measured preference against a position. See `v1probe`.
  void position(uint32_t neuron, Scalar& x, Scalar& y, Scalar& z) const {
    x = pos_x_[neuron];
    y = pos_y_[neuron];
    z = pos_z_[neuron];
  }

  // How many synapses terminate on this neuron. A receptive field that wired
  // nothing is the one failure mode of a structured projection that every
  // firing rate in the system would hide.
  uint32_t in_degree(uint32_t neuron) const { return in_count_[neuron]; }

  // What a neuron listens to: the source and weight of its k-th afferent.
  //
  // Reading the structure directly is the only way to tell a map that was
  // wired wrong apart from a map that was wired right and is not showing up in
  // the spikes — and those two have identical symptoms in every rate, every
  // classifier and every downstream milestone in this project.
  void in_synapse(uint32_t neuron, uint32_t k, uint32_t& source, Scalar& weight) const {
    const uint32_t syn = syn_in_[syn_base_[neuron] + k];
    source = syn_source_[syn];
    weight = syn_weight_[syn];
  }

  // What synaptic scaling is aiming this neuron's afferent set at — the birth
  // total, lowered by sleep downscaling. Exposed because "scaling erased the
  // weights" and "scaling never fired" produce the same flat sweep, and the
  // dead band is wide enough (a factor of `scaling_band`) that the second is a
  // live possibility. Comparing sum|w| against this says which happened.
  Scalar scaling_setpoint(uint32_t neuron) const { return w_in_target_[neuron]; }

  // Two firing-rate estimates, deliberately on different timescales.
  //
  // `rate` is a one-second average: that is the right window for homeostasis,
  // which must not chase a burst, and for telemetry.
  //
  // `rate_fast` is tens of milliseconds: that is the right window for reading
  // a motor population, which has to articulate at 100 Hz. Reading the motor
  // groups through the slow estimate would low-pass the creature's entire
  // output down to about 1 Hz — the vocal tract could not form a syllable, and
  // reward would have nothing fast enough to shape.
  Scalar rate(uint32_t neuron) const { return rate_ema_[neuron]; }
  Scalar rate_fast(uint32_t neuron) const { return rate_fast_[neuron]; }

  uint32_t total_capacity() const { return capacity_; }
  uint32_t live_neurons() const;
  uint32_t live_synapses() const;

  // Reports how many synapses the wiring rules had to discard because a
  // neuron's max_out_degree was full — either as a source (forward) or as a
  // target (the reverse index reuses the same slicing). Non-zero means the
  // genome is over-subscribed and the built brain is not the brain the DNA
  // describes; a dropped reverse entry is worse than a lost synapse, because
  // that synapse would then depress but never potentiate.
  uint32_t dropped_synapses() const { return dropped_synapses_; }
  uint32_t dropped_reverse() const { return dropped_reverse_; }

  // Which module ran out of slots. A total alone sends you reading the genome
  // for whichever projection you changed last, and that is the wrong place
  // twice over: the cap that overflows belongs to the *source* module of some
  // projection you did not touch, and it overflows in some developmental seeds
  // and not others. Counted per module so the banner can name it.
  uint32_t dropped_synapses(uint32_t module) const {
    return module < kMaxModules ? dropped_by_module_[module] : 0;
  }
  uint32_t dropped_reverse(uint32_t module) const {
    return module < kMaxModules ? dropped_reverse_by_module_[module] : 0;
  }

  // --- One tract, opened up (2026-08-14) ------------------------------------
  //
  // Everything above reports a module or the whole brain. This reports the
  // synapses running from one module to another, which is the grain the G3
  // question is actually asked at: `central->vocal` has been the prime suspect
  // for months and nothing outside the kernel has ever been able to see it.
  //
  // The order is stable — presynaptic neuron ascending, then slot ascending —
  // so two calls at different times index the same synapses, which is what
  // makes it meaningful to compare eligibility vectors across conditions. Pass
  // `out = nullptr` to count first, then size a buffer and call again.
  uint32_t tract_synapses(uint32_t src_module, uint32_t dst_module, uint32_t* out,
                          uint32_t max) const;
  Scalar synapse_eligibility(uint32_t syn) const { return syn_elig_[syn]; }
  Scalar synapse_weight(uint32_t syn) const { return syn_weight_[syn]; }
  // Which neuron drives this synapse. Needed to ask whether a synapse's trace
  // inherits the conditionality of the cell behind it, which is the difference
  // between "the tract is too thin" and "the source code is too sparse".
  uint32_t synapse_source(uint32_t syn) const { return syn_source_[syn]; }
  uint32_t synapse_target(uint32_t syn) const { return syn_target_[syn]; }
  // What reward actually multiplies under DNA v16: the trace minus this
  // synapse's own slow mean. Equal to the raw trace when the baseline is off.
  // Exposed because `eligprobe` was measuring `syn_elig_` and therefore could
  // not see the baseline at all — the sweep over it came back flat by
  // construction, which is a check that cannot fail.
  Scalar synapse_credit(uint32_t syn) const {
    return elig_mean_alpha_ > kZero ? syn_elig_[syn] - syn_elig_mean_[syn] : syn_elig_[syn];
  }

 private:
  void wire_intra_module(uint32_t m, Rng& rng);
  void wire_projection(const DnaProjection& p, Rng& rng);
  // ProjectionKind::kGabor — a retinotopic receptive field rather than a
  // random draw over pairs. See the long note at the definition.
  void wire_projection_gabor(const DnaProjection& p, Rng& rng);
  // ProjectionKind::kCurvature — tangency to a common circle, over the oriented
  // cells a kGabor projection built. See the note at the definition.
  void wire_projection_curvature(const DnaProjection& p, Rng& rng);
  bool add_synapse(uint32_t src, uint32_t dst, Scalar weight, uint16_t delay);
  void build_reverse_index();
  void rebuild_reverse_index();
  void capture_scaling_setpoints();
  uint16_t delay_ticks(Scalar ms) const;
  void accumulate_eligibility();
  // DNA v14: may this projection recruit this presynaptic neuron?
  bool source_allowed(const DnaProjection& p, uint32_t src_neuron) const;

  Scalar weight_ceiling(uint32_t src) const;

  // M4 internals.
  Scalar myelination(uint32_t syn) const;
  Scalar pressure(uint32_t neuron) const;
  bool connect_grown(uint32_t src, uint32_t dst, Scalar weight, Scalar distance);
  uint32_t claim_slot(uint32_t module);
  void init_neuron(uint32_t i, uint32_t m, Scalar x, Scalar y, Scalar z);
  uint32_t prune_synapses();
  uint32_t prune_neurons();

  Dna dna_;

  // --- Neuron state (indexed 0..capacity_) ---
  Scalar* v_ = nullptr;
  Scalar* v_rest_ = nullptr;
  Scalar* threshold_ = nullptr;   // per-neuron: intrinsic plasticity adapts it
  Scalar* target_rate_ = nullptr;
  Scalar* leak_alpha_ = nullptr;  // dt / tau, precomputed
  Scalar* noise_amp_ = nullptr;
  Scalar* rate_ema_ = nullptr;
  Scalar* rate_fast_ = nullptr;
  Scalar* pos_x_ = nullptr;
  Scalar* pos_y_ = nullptr;
  Scalar* pos_z_ = nullptr;
  // Node perturbation (DnaExploration). `perturb_` is a decaying trace of the
  // random numbers this neuron was actually given; `bias_` is the standing
  // excitability those perturbations have earned it. Both are arena arrays, so
  // both travel with a snapshot.
  Scalar* perturb_ = nullptr;
  Scalar* bias_ = nullptr;
  Scalar* trace_pre_ = nullptr;   // presynaptic STDP trace
  Scalar* trace_post_ = nullptr;  // postsynaptic STDP trace
  Scalar* w_in_target_ = nullptr; // scaling setpoint; sleep downscaling lowers it
  // What this neuron's *structure* entitles it to: the birth total, plus what
  // growth added and minus what pruning removed, and nothing else. It is the
  // reference the sleep downscale is not allowed to erode past — see
  // DnaConsolidate::downscale_floor.
  Scalar* w_in_struct_ = nullptr;
  uint32_t* refrac_until_ = nullptr;
  uint32_t* last_spike_ = nullptr;
  // DNA v36. The tick this neuron spiked on *before* the one it is spiking on
  // now. Dynamic synapses need the interval since the last release, and by the
  // time the delivery loop runs, `last_spike_` has already been advanced to
  // this tick by the integrate loop. Per neuron rather than per synapse
  // because every synapse of a source transmits on every spike of that source,
  // so a per-synapse copy would hold the identical number at ten times the
  // cost. Allocated only when a genome asks for the mechanism.
  uint32_t* prev_spike_ = nullptr;
  // DNA v37. Burst-dependent plasticity's two per-neuron traces: a fast rate of
  // burst spikes on the motor readout's 50 ms constant, and the slow baseline
  // it is read as a deviation from. Allocated only when a module asks for a
  // burst code.
  Scalar* burst_rate_ = nullptr;
  Scalar* burst_base_ = nullptr;
  uint32_t* syn_base_ = nullptr;
  uint16_t* syn_count_ = nullptr;
  uint16_t* syn_cap_ = nullptr;
  uint16_t* in_count_ = nullptr;
  uint16_t* refrac_ticks_ = nullptr;
  uint8_t* module_of_ = nullptr;
  uint8_t* is_inhib_ = nullptr;
  // Tombstone rather than compaction (§3.4's "neurons left with no surviving
  // connections are removed"). Compacting would mean renumbering neurons, and
  // every synapse, every reverse entry and every transducer's channel map is
  // keyed by neuron index. A dead slot is skipped by the tick loop and reused
  // by the next growth event, which is the same thing at a thousandth of the
  // risk.
  uint8_t* dead_ = nullptr;

  // --- Synapse pool ---
  uint32_t* syn_target_ = nullptr;
  uint32_t* syn_source_ = nullptr;  // owner of the slot; the reverse pass needs it
  Scalar* syn_weight_ = nullptr;
  Scalar* syn_elig_ = nullptr;
  // DNA v16. A slow per-synapse running mean of the eligibility trace, so the
  // weight update can be driven by the *deviation* from what this synapse
  // usually accumulates rather than by the raw trace. Measured motivation: the
  // trace on central->vocal correlates +0.93 between the two objects, so 93% of
  // what reward multiplies is a constant of the wiring.
  Scalar* syn_elig_mean_ = nullptr;
  Scalar* syn_traffic_ = nullptr;  // §3.5 myelination
  uint16_t* syn_delay_ = nullptr;
  // The delay this edge was born with. Myelination shortens syn_delay_ toward
  // a floor and §3.5 requires that it *reverts* when traffic decays, so the
  // unmyelinated value has to survive somewhere.
  uint16_t* syn_delay0_ = nullptr;
  // DNA v36. The dynamic synapse's two state variables, as they stood at this
  // synapse's most recent transmission: `syn_res_` is R, the resources it had
  // available, and `syn_use_` is u, the fraction of them it released. Both are
  // per synapse and not per pathway, because R depends on this synapse's own
  // history and that is the whole point — a shared pool would be exactly the
  // common mode the mechanism exists not to have. Allocated only when a genome
  // asks, so a creature with it off gets an arena byte-for-byte the size it
  // was before v36 existed.
  Scalar* syn_res_ = nullptr;
  Scalar* syn_use_ = nullptr;

  // Reverse index: for each neuron, the synapse slots that terminate on it.
  // STDP potentiation is driven by the *post*synaptic spike, and the forward
  // pool is sliced by source, so without this the potentiation half of the
  // rule has no way to find its synapses. It reuses syn_base_/syn_cap_ — the
  // slicing is identical, and one pool is exactly as large as the other.
  uint32_t* syn_in_ = nullptr;

  // --- Delay line: ring of `delay_slots_` frames, each `capacity_` wide ---
  Scalar* inbox_ = nullptr;

  uint32_t* spike_list_ = nullptr;
  uint32_t spike_count_ = 0;

  ModuleState modules_[kMaxModules];
  uint32_t module_count_ = 0;

  Rng* rng_ = nullptr;
  uint64_t tick_ = 0;
  uint32_t capacity_ = 0;
  uint32_t synapse_pool_ = 0;
  uint32_t delay_slots_ = 0;
  uint32_t dropped_synapses_ = 0;
  uint32_t dropped_reverse_ = 0;
  // Indexed by the module that ran out: the source module for a forward drop,
  // the target's module for a reverse one.
  uint32_t dropped_by_module_[kMaxModules] = {};
  uint32_t dropped_reverse_by_module_[kMaxModules] = {};
  Scalar dt_ms_ = Scalar(1);
  Scalar rate_alpha_ = Scalar(0.001);
  Scalar rate_fast_alpha_ = Scalar(0.02);
  Scalar spike_rate_unit_ = Scalar(1000);

  // Precomputed decay multipliers. Computed once at build with expf, so the
  // tick loop never touches a transcendental.
  Scalar pre_decay_ = kOne;
  Scalar post_decay_ = kOne;
  Scalar elig_decay_ = kOne;
  // DNA v39. The same decay, per module, read on the postsynaptic side. All
  // entries equal elig_decay_ unless a genome scales one, so the arithmetic is
  // bit-identical when the mechanism is unused.
  Scalar elig_decay_mod_[kMaxModules] = {};
  Scalar elig_mean_alpha_ = kZero;  // 0 disables the baseline entirely
  // DNA v17. 0 disables presynaptic centring entirely.
  Scalar elig_pre_centre_ = kZero;
  // DNA v18. Negative disables the potentiation gate.
  // DNA v19. 0 disables reward-independent Hebbian consolidation.
  // Population mean of trace_pre_ per module, from the previous tick. The
  // one-tick lag matches what divisive normalisation already does and for the
  // same reason: reading this tick's own value would need a second pass.
  Scalar pre_trace_mean_[kMaxModules] = {};
  // DNA v22. A fast pool of each module's activity in Hz, for the pooling
  // interneurons. Separate from mean_rate_fast because that one is a
  // one-second EMA and this has to follow the common mode trial-to-trial.
  Scalar pool_fast_[kMaxModules] = {};
  Scalar pool_alpha_ = kOne;
  // DNA v24. Per-neuron pooling weight, mean 1 within each module.
  Scalar* ffi_w_ = nullptr;
  // DNA v23. Per-pathway Hebbian rate, indexed [source module][target module].
  // Built once from the projections; zero everywhere unless a genome asks.
  Scalar hebb_pair_[kMaxModules][kMaxModules] = {};
  bool any_hebb_pair_ = false;
  // DNA v25. The apical compartment. `inbox_apical_` is a second delay ring of
  // the same shape as `inbox_` — apical synapses have axonal delays like any
  // other, and folding them into the somatic ring would be exactly the merge
  // the compartment exists to undo. `v_apical_` integrates on the tuft's own
  // slow constant; `plateau_until_` is the tick a triggered plateau ends on.
  //
  // Which tract lands where, indexed [source module][target module], built once
  // from the projections. False everywhere unless a genome asks, and
  // `any_apical_` then keeps the whole mechanism behind one branch.
  Scalar* inbox_apical_ = nullptr;
  Scalar* v_apical_ = nullptr;
  uint32_t* plateau_until_ = nullptr;
  bool apical_pair_[kMaxModules][kMaxModules] = {};
  bool any_apical_ = false;
  // DNA v36. Which dynamic-synapse parameter set a (source, target) pair uses,
  // or -1 for the constant-weight synapse every pair had before v36.
  // DNA v37. Per-pathway burst learning rate, indexed [source][target], and
  // the per-module burst parameters. `burst_refrac_` is 1 wherever the tuft is
  // not allowed to change bursting, which is every module by default.
  Scalar burst_pair_[kMaxModules][kMaxModules] = {};
  bool any_burst_pair_ = false;
  bool any_burst_ = false;
  uint32_t burst_ticks_[kMaxModules] = {};
  Scalar burst_refrac_[kMaxModules] = {};
  Scalar burst_base_alpha_ = kZero;

  // DNA v38. Competitive pruning's scratch: the mean |w| over each neuron's
  // afferents, recomputed at the top of every prune pass. Arena-allocated only
  // when a genome asks, and never read outside consolidate().
  Scalar* in_mean_ = nullptr;
  Scalar prune_compete_ = kZero;
  uint32_t prune_compete_min_in_ = 0;

  int8_t stp_id_[kMaxModules][kMaxModules] = {};
  bool any_stp_ = false;
  uint32_t stp_paths_ = 0;
  Scalar stp_u_[kMaxStpPaths] = {};      // U
  Scalar stp_inv_u_[kMaxStpPaths] = {};  // 1/U, so delivery never divides
  // exp(-gap / tau), split so that the kernel can look one up for any gap
  // without calling expf. gap is decomposed as hi*kStpCoarse + lo, and the
  // exponential of a sum is the product of the exponentials, so two lookups
  // and one multiply are exact rather than interpolated. Beyond what the two
  // tables span the factor is taken as zero — at the shortest time constant a
  // genome can usefully ask for that is a rounding error, and at the longest
  // it is the difference between "recovered" and "recovered".
  Scalar stp_rec_lo_[kMaxStpPaths][kStpCoarse] = {};
  Scalar stp_rec_hi_[kMaxStpPaths][kStpCoarseSteps] = {};
  Scalar stp_fac_lo_[kMaxStpPaths][kStpCoarse] = {};
  Scalar stp_fac_hi_[kMaxStpPaths][kStpCoarseSteps] = {};
  Scalar apical_leak_[kMaxModules] = {};
  Scalar apical_thresh_[kMaxModules] = {};
  Scalar apical_gain_[kMaxModules] = {};
  uint32_t apical_ticks_[kMaxModules] = {};

  // DNA v29. Plateau-gated plasticity, indexed by the *postsynaptic* module.
  // `plateau_hits_` / `plateau_events_` count how many potentiation events
  // passed the gate against how many were offered, which is this mechanism's
  // did-it-run guard: a gate that never opens and a gate that is always open
  // are both inert, and neither shows up in any rate, weight or hash.
  Scalar plateau_gate_[kMaxModules] = {};
  bool any_plateau_gate_ = false;
  uint64_t plateau_hits_ = 0;
  uint64_t plateau_events_ = 0;

  // DNA v26. Subthreshold oscillations.
  //
  // Phase is an integer that wraps, and it is derived from `tick_` on every
  // tick rather than accumulated. Two reasons, both of which a float phase
  // would get wrong. A float accumulator drifts over the 1.6M-tick runs the
  // long-horizon suite uses, so the rhythm a creature is on would depend on how
  // long it had been awake; and a phase carried as its own state would have to
  // be saved, whereas one computed from the tick is exact across a snapshot and
  // resume for free. `uint64 * uint32` truncated to 32 bits wraps at exactly one
  // cycle with no rounding anywhere.
  //
  // The table is built once with sinf() at construction. The tick loop stays
  // free of math.h, which is a standing invariant of this file.
  static constexpr uint32_t kSinBits = 10;
  static constexpr uint32_t kSinSize = 1u << kSinBits;
  Scalar sin_table_[kSinSize] = {};
  uint32_t osc_theta_inc_[kMaxModules] = {};
  uint32_t osc_gamma_inc_[kMaxModules] = {};
  Scalar osc_theta_amp_[kMaxModules] = {};
  Scalar osc_gamma_amp_[kMaxModules] = {};
  Scalar osc_coupling_[kMaxModules] = {};
  bool any_osc_ = false;

  // DNA v32. Lateral competition. `lateral_` holds this tick's per-neuron
  // local-minus-field term and is scratch, not state: it is written before it
  // is read on every tick, so it is not snapshotted and a resumed creature does
  // not need it. Allocated only when some module asks for the mechanism, which
  // is what keeps a v31 genome's arena byte-for-byte what it was.
  Scalar* lateral_ = nullptr;
  Scalar lateral_gain_[kMaxModules] = {};
  Scalar lateral_sigma_[kMaxModules] = {};
  uint32_t lateral_fields_[kMaxModules] = {};
  bool any_lateral_ = false;
  // DNA v28. Does any module close its critical period? Keeps the whole
  // computation behind one branch on a genome that does not ask for it.
  bool any_critical_ = false;
  Scalar sin_at(uint32_t phase) const { return sin_table_[phase >> (32 - kSinBits)]; }
  Scalar traffic_decay_ = kOne;
  Scalar last_reward_ = kZero;
  bool inhib_plastic_ = false;

  // Per-module multiplier on noise_amp. Derived state, not saved: the Brain
  // recomputes it from its own saved reward windows on the first plasticity
  // interval after a resume, and sets it directly in load_state so that even
  // the ticks before that one are right.
  Scalar explore_mult_[kMaxModules] = {};
  // DnaExploration::drive_compensation, cached from the genome at build.
  Scalar drive_comp_ = kZero;
  Scalar perturb_decay_ = kZero;
  Scalar perturb_rate_ = kZero;
  Scalar perturb_max_ = kZero;
  // Per-module gate on node perturbation, from DnaModule::explore_scale. LMAN
  // projects to the motor pathway, so only the larynx learns this way.
  Scalar perturb_scale_[kMaxModules] = {};

  // Divisive normalisation (DNA v12), cached from the genome at build. All
  // derived state — nothing here is saved, because all of it is recomputed
  // from the genome the moment a brain is constructed.
  Scalar norm_gain_[kMaxModules] = {};
  Scalar norm_target_[kMaxModules] = {};  // target_rate_hz, the reference point
  // DnaModule::eta_scale, cached. Indexed by the *postsynaptic* module.
  Scalar eta_scale_[kMaxModules] = {};
  Scalar norm_floor_ = kOne;
  Scalar norm_ceiling_ = kOne;

  StructuralStats structural_ = {};
  uint32_t competed_out_ = 0;  // DNA v38: removed by competition, not by the floor
  // Refreshed by myelinate(), which walks the pool anyway. See mean_plasticity().
  Scalar mean_plasticity_ = kOne;
};

}  // namespace aibaby

#endif  // AIBABY_NETWORK_H
