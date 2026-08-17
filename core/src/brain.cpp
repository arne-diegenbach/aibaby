#include "aibaby/brain.h"

// Compile-time only, no code emitted, and freestanding in C++17 — the one STL
// header the core is allowed. It guards the transducers below, which are saved
// as whole objects rather than field by field.
#include <type_traits>

#include "aibaby/snapshot_io.h"

namespace aibaby {
namespace {

inline Scalar clamp01(Scalar v) {
  return v < kZero ? kZero : (v > kOne ? kOne : v);
}

// The vocal tract is read at 100 Hz (§5.3), independent of the simulation
// timestep. Ten motor frames per second of eligibility trace is plenty of
// resolution for a creature whose feedback arrives two seconds late.
constexpr Scalar kVocalUpdateMs = Scalar(10);

}  // namespace

// The replay buffer (§3.6). Sized from the genome, allocated at birth like
// everything else: a buffer that grew would be the one allocation after
// hatching, and §6.1 does not allow one.
static size_t replay_bytes(const Dna& dna) {
  const DnaConsolidate& c = dna.header().consolidate;
  if (!c.enabled || c.replay_episodes == 0) return 0;
  const size_t n = c.replay_episodes;
  const size_t per = size_t(dna.header().audio.mel_channels) +
                     size_t(vision_features(dna.header().vision)) + 1;
  return n * per * sizeof(Scalar) + 64;  // slack for alignment padding
}

size_t Brain::required_bytes(const Dna& dna) {
  return Network::required_bytes(dna) + Critic::required_bytes(dna) +
         replay_bytes(dna);
}

BrainStatus Brain::init(const void* dna_blob, size_t dna_size, void* memory,
                        size_t memory_size) {
  dna_status_ = dna_.load(dna_blob, dna_size);
  if (dna_status_ != DnaStatus::kOk) return BrainStatus::kBadDna;

  arena_.init(memory, memory_size);
  rng_.seed(dna_.header().seed);

  if (!network_.build(dna_, arena_, rng_)) return BrainStatus::kArenaTooSmall;

  const int32_t central = dna_.module_with_role(ModuleRole::kAssociation);
  const int32_t auditory = dna_.module_with_role(ModuleRole::kAuditory);
  const int32_t vision = dna_.module_with_role(ModuleRole::kVision);
  const int32_t somato = dna_.module_with_role(ModuleRole::kSomato);
  const int32_t vocal = dna_.module_with_role(ModuleRole::kVocal);
  const int32_t expression = dna_.module_with_role(ModuleRole::kExpression);

  // The association module is the one part of the body plan that is not
  // optional: it is where cross-modal association happens, and the curiosity
  // critic reads its activity.
  if (central < 0) return BrainStatus::kMissingModule;
  if (!critic_.build(dna_, arena_, uint32_t(central))) return BrainStatus::kArenaTooSmall;

  has_auditory_ = auditory >= 0;
  has_vision_ = vision >= 0;
  has_touch_ = somato >= 0;
  has_vocal_ = vocal >= 0;
  has_expression_ = expression >= 0;

  if (has_auditory_) auditory_.configure(dna_, uint32_t(auditory));
  if (has_vision_) vision_.configure(dna_, uint32_t(vision));
  if (has_touch_) touch_.configure(dna_, uint32_t(somato));
  if (has_vocal_) vocal_.configure(dna_, uint32_t(vocal), kVocalUpdateMs);
  if (has_expression_) {
    expression_.configure(uint32_t(expression), Scalar(dna_.header().vocal.rate_norm_hz));
  }

  const Scalar dt = Scalar(dna_.header().sim.dt_ms);
  plasticity_interval_ = dna_.header().sim.plasticity_interval_ticks;
  homeo_interval_ = dna_.header().homeo.interval_ticks;
  vocal_interval_ = uint32_t(kVocalUpdateMs / dt + Scalar(0.5));
  if (vocal_interval_ == 0) vocal_interval_ = 1;

  const Scalar interval_ms = dt * Scalar(plasticity_interval_);
  const Scalar baseline_tau = Scalar(dna_.header().drives.reward_baseline_tau_ms);
  baseline_alpha_ = baseline_tau > kZero
                        ? (interval_ms / baseline_tau > kOne ? kOne : interval_ms / baseline_tau)
                        : kOne;
  reward_baseline_ = kZero;

  // Same first-order approximation the reward baseline above uses, and for the
  // same reason: brain.cpp stays free of transcendentals, and at intervals far
  // shorter than the time constant dt/tau and 1 - exp(-dt/tau) agree to well
  // under a percent.
  const DnaExploration& ex = dna_.header().exploration;
  fast_alpha_ = ex.fast_tau_ms > 0.0f
                    ? (interval_ms / Scalar(ex.fast_tau_ms) > kOne
                           ? kOne
                           : interval_ms / Scalar(ex.fast_tau_ms))
                    : kOne;
  slow_alpha_ = ex.slow_tau_ms > 0.0f
                    ? (interval_ms / Scalar(ex.slow_tau_ms) > kOne
                           ? kOne
                           : interval_ms / Scalar(ex.slow_tau_ms))
                    : kOne;
  reward_fast_ = kZero;
  reward_slow_ = kZero;
  explore_ = kOne;
  apply_exploration();

  // Replay storage. Allocated after the network and the critic, so a genome
  // with replay switched off costs nothing.
  const DnaConsolidate& cons = dna_.header().consolidate;
  episode_capacity_ = cons.enabled ? cons.replay_episodes : 0;
  if (episode_capacity_ > kMaxReplayEpisodes) episode_capacity_ = kMaxReplayEpisodes;
  episodes_stored_ = 0;
  episode_next_ = 0;
  replay_index_ = 0;
  replay_left_ = 0;
  replay_queued_ = 0;
  if (episode_capacity_ > 0) {
    const uint32_t mel_n = dna_.header().audio.mel_channels;
    const uint32_t vis_n = vision_features(dna_.header().vision);
    episode_mel_ = arena_.alloc_zeroed<Scalar>(size_t(episode_capacity_) * mel_n);
    episode_retina_ = arena_.alloc_zeroed<Scalar>(size_t(episode_capacity_) * vis_n);
    episode_reward_ = arena_.alloc_zeroed<Scalar>(episode_capacity_);
    if (!arena_.ok()) return BrainStatus::kArenaTooSmall;
  }

  growth_ = GrowthWatch{};
  last_growth_tick_ = 0;
  last_consolidation_ = 0;

  drives_ = Drives{};
  sleep_ = SleepState::kAwake;
  reward_ = RewardBreakdown{};
  pending_external_ = kZero;
  pending_curiosity_ = kZero;
  prev_hunger_ = drives_.hunger;
  prev_comfort_ = drives_.comfort;
  plasticity_events_ = 0;
  return BrainStatus::kOk;
}

void Brain::step() {
  const bool awake = sleep_ == SleepState::kAwake;

  // Replay (§3.6) drives the encoders while the creature is asleep. That is
  // not a hole in the sensory gate, it is the point of the mechanism: the room
  // stays shut out — hear() and see() still drop their frames — and what
  // reaches the modules is an episode the creature is re-living from the
  // inside.
  if (replay_left_ > 0) drive_replay();
  const bool gate = awake || replay_left_ > 0;

  // Encoders inject into the slot this step is about to drain, so sensory
  // current reaches the neuron on the tick it was presented for. Asleep, they
  // do not run at all: §3.6 gates sensory input off, and a baby that keeps
  // hearing the room is not asleep.
  //
  // Touch is deliberately still connected, so a sleeping brain is not fully
  // cut off from its body. It does not wake the baby, though: waking is
  // fatigue-driven only. Wake-on-touch would need its own hysteresis, or a
  // poke at fatigue 0.9 would wake the creature and drop it straight back to
  // sleep on the following tick.
  if (has_auditory_) auditory_.drive(network_, gate);
  if (has_vision_) vision_.drive(network_, gate);
  if (has_touch_) touch_.drive(network_);

  network_.step();
  update_drives();

  // Re-read the state: update_drives() may have just flipped it, and the
  // larynx has to shut on the same tick the baby falls asleep rather than one
  // motor frame later. One frame is 10 ms of audible babble from a creature
  // that is supposed to be unconscious, and it is exactly the kind of thing
  // that shows up as "it doesn't stop talking" long before it shows up in a
  // measurement.
  const bool still_awake = sleep_ == SleepState::kAwake;

  const uint64_t tick = network_.tick();
  if (has_vocal_ && tick % vocal_interval_ == 0) vocal_.update(network_, still_awake);
  if (has_expression_ && tick % vocal_interval_ == 0) expression_.update(network_);

  if (tick % plasticity_interval_ == 0) cash_in_reward();
  if (tick % homeo_interval_ == 0) {
    // Each module is regulated at its own strength, and differently asleep
    // than awake. Read from `sleep_` rather than from `still_awake`, because
    // what matters is the state the creature is in for this whole interval.
    network_.homeostasis(asleep());
    // §3.5 on the same cadence as §3.1: both are slow regulation over the
    // whole synapse pool, and myelination has no reason to run faster than the
    // homeostasis it competes with.
    network_.myelinate();
  }

  // §3.4's first condition. Watched continuously — the point of G4 is that the
  // detector runs while error is still improving and declines to grow, so a
  // detector that only ran when growth was already plausible would beg the
  // question.
  if (check_plateau() && still_awake) try_grow();

  // Structural surgery, only ever in the safe window (§3.4, §3.6).
  const DnaConsolidate& c = dna_.header().consolidate;
  if (c.enabled && !still_awake && c.interval_ticks > 0 &&
      tick - last_consolidation_ >= c.interval_ticks) {
    last_consolidation_ = tick;
    network_.consolidate();
    begin_replay();
  }
}

// R(t) = w_ext*R_external + w_hunger*ΔHunger + w_comfort*ΔComfort
//        + w_curiosity*ΔProgress                                     (§3.3)
//
// Each term is an *impulse* — the reward integrated over the plasticity
// interval, not a rate. That keeps the total effect of one feed or one poke
// independent of how often we cash eligibility in, which is the property that
// makes the interval a performance knob rather than a learning parameter.
//
// The sign on hunger is the one reading the spec leaves implicit: rising
// hunger is aversive, so the reward is the *reduction* in hunger. Feeding is
// rewarding; being left hungry is a slow, constant negative.
// LMAN (§DnaExploration). Exploration is a function of how the creature is
// doing *relative to how it has been doing*, which is why two windows are
// needed rather than the reward prediction error the synapses already see:
// that one is defined as reward minus its own running mean and so averages to
// zero, and a signal that averages to zero cannot say "this is going well".
void Brain::apply_exploration() {
  const DnaExploration& ex = dna_.header().exploration;
  if (!ex.enabled) {
    explore_ = kOne;
    for (uint32_t m = 0; m < network_.module_count(); ++m) {
      network_.set_exploration(m, kOne);
    }
    return;
  }

  const Scalar doing_well = reward_fast_ - reward_slow_;
  Scalar e = kOne - Scalar(ex.sensitivity) * doing_well;
  if (e < Scalar(ex.floor)) e = Scalar(ex.floor);
  if (e > Scalar(ex.ceiling)) e = Scalar(ex.ceiling);
  explore_ = e;

  // Delivered per module, because LMAN projects to the motor pathway and not
  // to the whole brain. A module at explore_scale 0 keeps its plain noise_amp.
  for (uint32_t m = 0; m < network_.module_count(); ++m) {
    const Scalar scale = Scalar(dna_.module(m).explore_scale);
    network_.set_exploration(m, kOne + scale * (explore_ - kOne));
  }
}

void Brain::cash_in_reward() {
  const DnaDrives& d = dna_.header().drives;

  reward_.external = Scalar(d.w_external) * pending_external_;
  reward_.hunger = Scalar(d.w_hunger) * (prev_hunger_ - drives_.hunger);
  reward_.comfort = Scalar(d.w_comfort) * (drives_.comfort - prev_comfort_);
  reward_.curiosity = Scalar(d.curiosity_weight) * pending_curiosity_;
  reward_.total =
      reward_.external + reward_.hunger + reward_.comfort + reward_.curiosity;

  // Three-factor learning is driven by reward *prediction error*, not by
  // reward. The baseline is the difference between "this was better than
  // usual" and "something happened": subtracting it removes the common-mode
  // term that otherwise walks every eligible synapse in the same direction
  // whenever the average reward is non-zero.
  reward_.baseline = reward_baseline_;
  reward_.effective = reward_.total - reward_baseline_;
  reward_baseline_ += baseline_alpha_ * (reward_.total - reward_baseline_);

  // LMAN. The two windows and their difference are the performance signal;
  // doing better than lately closes exploration, doing worse opens it.
  reward_fast_ += fast_alpha_ * (reward_.total - reward_fast_);
  reward_slow_ += slow_alpha_ * (reward_.total - reward_slow_);
  apply_exploration();

  if (dna_.header().neuromod.enabled) {
    // DNA v20. Each term keeps its own baseline and travels as its own channel;
    // the module decides which ones it listens to. Summing them first, as the
    // branch below does, is what makes the caregiver's teaching signal share a
    // wire with three object-blind drives.
    aibaby::Scalar ch[aibaby::kModulatorChannels] = {
        reward_.external - nm_baseline_[0], reward_.hunger - nm_baseline_[1],
        reward_.comfort - nm_baseline_[2], reward_.curiosity - nm_baseline_[3]};
    const aibaby::Scalar term[aibaby::kModulatorChannels] = {
        reward_.external, reward_.hunger, reward_.comfort, reward_.curiosity};
    for (uint32_t c = 0; c < aibaby::kModulatorChannels; ++c) {
      nm_baseline_[c] += baseline_alpha_ * (term[c] - nm_baseline_[c]);
    }
    network_.apply_reward_channels(ch);
  } else {
    network_.apply_reward(reward_.effective);
  }

  // Recorded after the reward is final, and before it is cleared: an episode
  // is a sensory frame plus the surprise it turned out to be worth (§3.6).
  record_episode();

  pending_external_ = kZero;
  pending_curiosity_ = kZero;
  prev_hunger_ = drives_.hunger;
  prev_comfort_ = drives_.comfort;
  ++plasticity_events_;
}

// --- M4: growth (§3.4) -----------------------------------------------------

// The first of the three growth conditions: has reward/error plateaued over
// window W?
//
// The error watched is the critic's slow prediction error — the one quantity
// in the creature that means "how well do I understand what happens next", and
// the only one that improves when learning is working and flattens when it has
// extracted what the current structure can hold. External reward would be the
// other candidate and it is the wrong one: it is whatever the caregiver
// happened to do this minute, so a quiet stretch would read as a plateau and
// the creature would grow a module because nobody was in the room.
//
// Compared window-to-window on the *best* error in each, not the last sample.
// The slow error is still a noisy trace, and comparing endpoints makes the
// verdict depend on where the window boundary happened to land.
bool Brain::check_plateau() {
  const DnaGrowth& g = dna_.header().growth;
  if (!g.enabled || !critic_.ready()) return false;

  const Scalar err = critic_.slow_error();
  if (growth_.windows == 0 && growth_.window_start == 0 && growth_.best_error == kZero) {
    growth_.best_error = err;  // first sample of the first window
  }
  if (err < growth_.best_error) growth_.best_error = err;

  const uint64_t tick = network_.tick();
  if (tick - growth_.window_start < g.plateau_window_ticks) return false;

  // Window closed. The first one has nothing to compare against, so it only
  // establishes the baseline — a creature cannot be on a plateau before it has
  // had two windows to be flat across.
  const bool first = growth_.windows == 0;
  growth_.improvement = first ? kZero : growth_.last_best - growth_.best_error;
  growth_.plateaued = !first && growth_.improvement < Scalar(g.epsilon);
  growth_.improving = !first && growth_.improvement >= Scalar(g.epsilon);
  growth_.last_best = growth_.best_error;
  growth_.best_error = err;
  growth_.window_start = tick;
  ++growth_.windows;
  return growth_.plateaued;
}

// Conditions two and three, and the action. Called only on the tick a plateau
// is declared, so "grows only on a detected plateau" is true by construction
// rather than by tuning — which is the half of G4 that a measurement cannot
// really establish and a structure can.
void Brain::try_grow() {
  const DnaGrowth& g = dna_.header().growth;
  // One growth event at a time. A plateau lasts as long as it lasts, and
  // without this the next window would fire again on the same flat stretch and
  // walk every module to its cap inside a minute of simulated time.
  const uint64_t tick = network_.tick();
  if (last_growth_tick_ != 0 && tick - last_growth_tick_ < g.refractory_ticks) return;

  const Scalar err = critic_.slow_error();

  // Did the last event help? Judged here rather than at the moment of growth,
  // because a neuron inserted into a running network needs its refractory
  // period to wire in and start firing before its effect is readable.
  if (growth_.growth_events > 0) {
    if (err < growth_.error_at_growth - Scalar(g.epsilon)) growth_.ineffective = 0;
    else ++growth_.ineffective;
  }

  // Condition 2, in the form a regulated network can actually reach: the
  // creature has stopped improving (it would not be here otherwise) and is
  // still wrong about what happens next.
  if (err <= Scalar(g.error_floor)) return;
  if (g.patience > 0 && growth_.ineffective >= g.patience) return;

  for (uint32_t m = 0; m < network_.module_count(); ++m) {
    if (!network_.growable(m)) continue;
    // The literal §3.4 reading, kept because it is what every measurement
    // before DNA v5 was taken on — see the note on require_saturation.
    if (g.require_saturation && !network_.saturated(m)) continue;
    if (!network_.below_cap(m)) continue;   // condition 3 — the DNA budget cap
    if (network_.grow(m, g.insert_k) > 0) {
      last_growth_tick_ = tick;
      growth_.error_at_growth = err;
      ++growth_.growth_events;
    }
  }
}

// --- M4: replay (§3.6) -----------------------------------------------------

// Worth re-living: a moment that went better than the creature expected. The
// baseline subtraction is what makes that meaningful — raw reward would fill
// the buffer with whatever the caregiver does most often, which is the one
// thing there is no point rehearsing.
void Brain::record_episode() {
  if (episode_capacity_ == 0) return;
  const DnaConsolidate& c = dna_.header().consolidate;
  if (reward_.effective < Scalar(c.replay_threshold)) return;

  const uint32_t slot = episode_next_;
  const uint32_t mel_n = dna_.header().audio.mel_channels;
  const uint32_t vis_n = vision_features(dna_.header().vision);

  const Scalar* ears = auditory_.level();
  Scalar* into_mel = episode_mel_ + size_t(slot) * mel_n;
  for (uint32_t i = 0; i < mel_n; ++i) into_mel[i] = has_auditory_ ? ears[i] : kZero;

  const Scalar* eyes = vision_.level();
  Scalar* into_vis = episode_retina_ + size_t(slot) * vis_n;
  for (uint32_t i = 0; i < vis_n; ++i) into_vis[i] = has_vision_ ? eyes[i] : kZero;

  episode_reward_[slot] = reward_.effective;
  episode_next_ = (slot + 1) % episode_capacity_;
  if (episodes_stored_ < episode_capacity_) ++episodes_stored_;
}

void Brain::begin_replay() {
  if (episodes_stored_ == 0) return;
  const DnaConsolidate& c = dna_.header().consolidate;
  if (c.replay_ticks == 0) return;
  replay_queued_ = episodes_stored_;
  replay_index_ = 0;
  replay_left_ = c.replay_ticks;
}

// One tick of a replayed episode. The frame is re-presented at the top of the
// episode only, and then decays exactly as a real presentation would: an
// episode the creature holds artificially steady for half a second is not a
// memory of anything that happened to it.
void Brain::drive_replay() {
  const DnaConsolidate& c = dna_.header().consolidate;
  const uint32_t mel_n = dna_.header().audio.mel_channels;
  const uint32_t vis_n = vision_features(dna_.header().vision);

  if (replay_left_ == c.replay_ticks) {  // first tick of this episode
    if (has_auditory_) {
      auditory_.present(episode_mel_ + size_t(replay_index_) * mel_n, mel_n);
    }
    if (has_vision_) {
      vision_.present(episode_retina_ + size_t(replay_index_) * vis_n, vis_n);
    }
  }

  --replay_left_;
  if (replay_left_ > 0) return;

  // The episode is over: pay out the reward it earned. It arrives at the end,
  // against a two-second eligibility trace that now holds the replayed
  // activity — the same shape as the waking loop, which is what makes this
  // consolidation of the original episode rather than a new and different
  // lesson.
  pending_external_ += episode_reward_[replay_index_];
  network_.note_replay();

  ++replay_index_;
  if (replay_index_ < replay_queued_ && replay_index_ < episodes_stored_) {
    replay_left_ = c.replay_ticks;
  } else {
    replay_queued_ = 0;
  }
}

void Brain::update_drives() {
  const DnaDrives& d = dna_.header().drives;
  const Scalar dt = Scalar(dna_.header().sim.dt_ms);

  drives_.hunger = clamp01(drives_.hunger + Scalar(d.hunger_rate) * dt);

  // Comfort relaxes toward neutral rather than toward zero — an untouched
  // baby is neither happy nor distressed.
  const Scalar neutral = Scalar(0.5);
  drives_.comfort += (neutral - drives_.comfort) * Scalar(d.comfort_decay) * dt;

  // Fatigue tracks how hard the brain is actually working, so a quiet brain
  // stays awake and a busy one earns its sleep. Awake it only accumulates;
  // sleep is the only thing that discharges it, which is what makes sleep a
  // state the creature needs rather than a timer that happens to it.
  const Network& net = network_;
  if (sleep_ == SleepState::kAsleep) {
    drives_.fatigue = clamp01(drives_.fatigue - Scalar(d.fatigue_recovery) * dt);
    if (drives_.fatigue <= Scalar(d.wake_threshold)) sleep_ = SleepState::kAwake;
  } else {
    const Scalar activity = net.live_neurons()
                                ? Scalar(net.spike_count()) / Scalar(net.live_neurons())
                                : kZero;
    drives_.fatigue = clamp01(drives_.fatigue + Scalar(d.fatigue_rate) * dt * activity);
    if (drives_.fatigue >= Scalar(d.sleep_threshold)) sleep_ = SleepState::kAsleep;
  }
}

// Predictive coding (DNA v15). The critic is asked what it expected first, and
// what reaches B2 is what is left over.
//
// The ordering matters and is the reverse of v14's. observe() scores the
// prediction that the *previous* frame's activity implied and only then
// refreshes its features, so calling it before present() hands us a prediction
// of the frame now arriving that was made without having seen it. Presenting
// first and asking afterwards would give the same numbers — observe() reads the
// network, not the encoder — but it would leave the honest ordering resting on
// that coincidence rather than on the code saying it.
void Brain::hear(const Scalar* mel, uint32_t count) {
  // Asleep, the ears are shut: the frame is dropped rather than held, so the
  // encoder is not sitting on a stale sound when the baby wakes.
  if (sleep_ == SleepState::kAsleep) return;

  critic_.observe(network_, mel, count);
  pending_curiosity_ += critic_.progress();
  if (!has_auditory_) return;

  const Scalar g = Scalar(dna_.header().curiosity.predict_gain);
  if (g <= kZero) {
    auditory_.present(mel, count);
    return;
  }

  // Stack, not arena: this is one frame wide and dies with the call. The core
  // forbids allocation after birth, not automatic storage.
  const uint32_t n = count < kMaxMelChannels ? count : kMaxMelChannels;
  Scalar residual[kMaxMelChannels];
  const Scalar* pred = critic_.prediction();
  for (uint32_t c = 0; c < n; ++c) {
    // The prediction is clamped into the range a mel frame can actually
    // occupy before it is subtracted. An unclamped least-squares model is free
    // to predict negative energy, and subtracting that would *add* drive to a
    // channel the creature was confident would be silent — the loudest ear in
    // the brain would be the one with the most confident wrong prediction.
    const Scalar p = clamp01(pred[c]);
    residual[c] = clamp01(mel[c] - g * p);
  }
  auditory_.present(residual, n);
}

void Brain::see(const Scalar* features, uint32_t count) {
  // Asleep the eyes are shut, so the frame is dropped rather than queued —
  // the same reason the ears drop theirs.
  if (sleep_ == SleepState::kAsleep) return;
  if (has_vision_) vision_.present(features, count);
}

void Brain::praise(Scalar value) {
  if (value > kOne) value = kOne;
  if (value < -kOne) value = -kOne;
  pending_external_ += value;
}

void Brain::feed(Scalar amount) {
  drives_.hunger = clamp01(drives_.hunger - amount);
  drives_.comfort = clamp01(drives_.comfort + amount * Scalar(0.25));
  if (has_touch_) touch_.trigger(TouchAction::kFeed, amount);
}

void Brain::poke(Scalar intensity) {
  drives_.comfort = clamp01(drives_.comfort - intensity * Scalar(0.3));
  if (has_touch_) touch_.trigger(TouchAction::kPoke, intensity);
}

void Brain::tickle(Scalar intensity) {
  drives_.comfort = clamp01(drives_.comfort + intensity * Scalar(0.2));
  if (has_touch_) touch_.trigger(TouchAction::kTickle, intensity);
}

void Brain::stroke(Scalar intensity) {
  drives_.comfort = clamp01(drives_.comfort + intensity * Scalar(0.15));
  if (has_touch_) touch_.trigger(TouchAction::kStroke, intensity);
}

// --- Snapshot (§8) ---------------------------------------------------------

// The transducers travel as whole objects. Every one of them is a fixed-size
// bag of levels, countdowns and configuration with no pointer in it, so a
// field-by-field list would be a second copy of five class definitions and one
// more place to forget an edit — this way a new countdown is carried whether
// anyone remembers it or not. The static_assert is what keeps that true: the
// day one of them grows a pointer, this stops compiling instead of quietly
// writing an address into a file.
//
// Padding bytes ride along with them, so two saves of the same creature are
// not guaranteed to be byte-identical files. Nothing compares snapshots that
// way — what verification compares is the brain that comes back out.
template <typename T>
static void put_object(SnapshotWriter& w, const T& v) {
  static_assert(std::is_trivially_copyable<T>::value,
                "snapshot: transducer state must stay pointer-free");
  w.write(&v, sizeof(T));
}

template <typename T>
static void get_object(SnapshotReader& r, T& v) {
  static_assert(std::is_trivially_copyable<T>::value,
                "snapshot: transducer state must stay pointer-free");
  r.read(&v, sizeof(T));
}

void Brain::save_state(SnapshotWriter& w) const {
  network_.save_state(w);
  critic_.save_state(w);

  uint64_t rng[4];
  rng_.save(rng);
  for (int i = 0; i < 4; ++i) w.put(rng[i]);

  put_object(w, drives_);
  const uint32_t sleep = uint32_t(sleep_);
  w.put(sleep);
  put_object(w, reward_);

  put_object(w, auditory_);
  put_object(w, vision_);
  put_object(w, touch_);
  put_object(w, vocal_);
  put_object(w, expression_);

  w.put(reward_baseline_);
  w.put(reward_fast_);
  w.put(reward_slow_);
  w.put(pending_external_);
  w.put(pending_curiosity_);
  w.put(prev_hunger_);
  w.put(prev_comfort_);
  w.put(plasticity_events_);

  put_object(w, growth_);
  w.put(last_growth_tick_);
  w.put(last_consolidation_);

  // The episodes themselves are arena memory and travel with it; these are the
  // cursors into them, including a replay that was in progress when the
  // creature was put down.
  w.put(episodes_stored_);
  w.put(episode_next_);
  w.put(replay_index_);
  w.put(replay_left_);
  w.put(replay_queued_);
}

void Brain::load_state(SnapshotReader& r) {
  network_.load_state(r);
  critic_.load_state(r);

  uint64_t rng[4];
  for (int i = 0; i < 4; ++i) r.get(rng[i]);
  rng_.restore(rng);

  get_object(r, drives_);
  uint32_t sleep = 0;
  r.get(sleep);
  sleep_ = sleep == uint32_t(SleepState::kAsleep) ? SleepState::kAsleep : SleepState::kAwake;
  get_object(r, reward_);

  get_object(r, auditory_);
  get_object(r, vision_);
  get_object(r, touch_);
  get_object(r, vocal_);
  get_object(r, expression_);

  r.get(reward_baseline_);
  r.get(reward_fast_);
  r.get(reward_slow_);
  r.get(pending_external_);
  r.get(pending_curiosity_);
  r.get(prev_hunger_);
  r.get(prev_comfort_);
  r.get(plasticity_events_);

  get_object(r, growth_);
  r.get(last_growth_tick_);
  r.get(last_consolidation_);

  r.get(episodes_stored_);
  r.get(episode_next_);
  r.get(replay_index_);
  r.get(replay_left_);
  r.get(replay_queued_);

  // A file from a genome with a smaller replay buffer would otherwise index
  // past the end of one that is allocated for fewer episodes. The genome is
  // checked before this runs, so this cannot fire — which is exactly why it is
  // cheap to keep.
  if (episodes_stored_ > episode_capacity_) episodes_stored_ = episode_capacity_;
  // Exploration is derived from the two windows above rather than stored, but
  // it must be right *now*: the next plasticity interval is up to ten ticks
  // away, and a resumed creature that babbles at the wrong amplitude for ten
  // ticks is not the creature that was saved.
  apply_exploration();

  if (episode_capacity_ == 0) {
    episode_next_ = 0;
    replay_index_ = 0;
    replay_left_ = 0;
    replay_queued_ = 0;
  } else {
    if (episode_next_ >= episode_capacity_) episode_next_ = 0;
    if (replay_index_ >= episode_capacity_) replay_index_ = 0;
    if (replay_queued_ > episodes_stored_) replay_queued_ = episodes_stored_;
  }
}

}  // namespace aibaby
