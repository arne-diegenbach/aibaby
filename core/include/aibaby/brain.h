// Top-level owner: genome, arena, network, transducers, drives, and the
// three-factor reward loop that ties them together.
//
// The host talks to this and nothing below it.

#ifndef AIBABY_BRAIN_H
#define AIBABY_BRAIN_H

#include "aibaby/arena.h"
#include "aibaby/critic.h"
#include "aibaby/dna.h"
#include "aibaby/network.h"
#include "aibaby/rng.h"
#include "aibaby/senses.h"

namespace aibaby {

// Homeostatic state, all normalised to [0, 1].
//
// These are the intrinsic half of the reward signal (§3.3). External reward is
// far too sparse to bootstrap anything on its own.
struct Drives {
  Scalar hunger = Scalar(0.2);
  Scalar comfort = Scalar(0.5);
  Scalar fatigue = Scalar(0);
};

// Sleep (§3.6). Two thresholds rather than one, because a single one would
// make the creature flutter between asleep and awake at the boundary.
//
// Sleep gates sensory input off and closes the larynx, so a sleeping baby
// neither hears nor babbles, and fatigue discharges. It is also the safe
// window for structural surgery: pruning, synaptic downscaling and replay of
// high-reward episodes all run here and nowhere else (M4).
enum class SleepState : uint32_t {
  kAwake = 0,
  kAsleep = 1,
};

// The plateau detector (§3.4's first growth condition), exposed because "the
// network grew at a moment when error was still improving" is exactly the
// failure G4 exists to catch, and it is invisible from neuron counts alone.
struct GrowthWatch {
  Scalar best_error = kZero;     // lowest prediction error seen this window
  Scalar last_best = kZero;      // and in the window before it
  Scalar improvement = kZero;    // last_best - best_error: positive means learning
  uint64_t window_start = 0;
  uint32_t windows = 0;          // completed windows
  bool plateaued = false;        // was the most recent window a plateau?
  bool improving = false;        // ...or was error still coming down?
  // Whether growth is earning its place: a run of events that changes nothing
  // stops growth rather than filling the budget cap with neurons that do not
  // help, and only an *improving* window clears the run. There used to be a
  // second quantity here, `error_at_growth`, holding the prediction error at
  // the last event so the next window could compare two point samples against
  // it — removed 2026-08-20 as a duplicate definition of "improving", not as a
  // bug fix: it and the window verdict agree at every judgement measured. See
  // Brain::try_grow for the three variants and their ledgers.
  uint32_t ineffective = 0;      // consecutive growth events that did not help
  uint32_t growth_events = 0;    // events this brain has had, for the panel
};

// The four terms of R(t), kept separated so the telemetry panel can show which
// one is actually driving learning. A failure to learn and a reward signal
// that is all curiosity look identical from the outside.
struct RewardBreakdown {
  Scalar external = kZero;
  Scalar hunger = kZero;
  Scalar comfort = kZero;
  Scalar curiosity = kZero;
  Scalar total = kZero;      // the four terms summed
  Scalar baseline = kZero;   // running expectation
  Scalar effective = kZero;  // total - baseline: what actually reaches the synapses
};

enum class BrainStatus {
  kOk = 0,
  kBadDna,
  kArenaTooSmall,
  kMissingModule,
};

class SnapshotReader;
class SnapshotWriter;

class Brain {
 public:
  // Arena size for a genome: network plus critic. The host allocates once.
  static size_t required_bytes(const Dna& dna);

  // `memory` must outlive the Brain and be 8-byte aligned. `dna_blob` too —
  // the genome is referenced in place, never copied.
  BrainStatus init(const void* dna_blob, size_t dna_size, void* memory, size_t memory_size);

  void step();

  // Caregiver actions from the UI. Each one is both a drive change and a
  // touch on the somatosensory module — the baby feels it, it does not just
  // read it off a gauge.
  void feed(Scalar amount);
  void poke(Scalar intensity);
  void tickle(Scalar intensity);
  void stroke(Scalar intensity);

  // Explicit good/bad from the caregiver, in [-1, 1]. Delivered as an impulse
  // and cashed in at the next plasticity interval, where the eligibility
  // traces are still holding what the baby did two seconds ago.
  void praise(Scalar value);

  // One mel frame from the host's cochlea, normalised to [0,1].
  void hear(const Scalar* mel, uint32_t count);

  // One retinal frame from the host's foveated sampler: ON/OFF centre-surround
  // responses, normalised to [0,1], `vision_features(dna)` of them.
  void see(const Scalar* features, uint32_t count);

  const Network& network() const { return network_; }
  Network& network() { return network_; }
  const Dna& dna() const { return dna_; }
  const Drives& drives() const { return drives_; }
  bool asleep() const { return sleep_ == SleepState::kAsleep; }

  // What the auditory encoder is currently driving B2 with, which is not the
  // same as the last frame that arrived: it fades when the sound stops, and it
  // is silent while the baby is asleep. This is what the panel should show, so
  // that "the ears" means what the brain is actually receiving.
  const Scalar* auditory_level() const { return auditory_.level(); }
  uint32_t auditory_channels() const { return auditory_.channels(); }

  // Likewise for the eyes: the retinal responses B3 is currently being driven
  // with, which fade when the camera stops and are silent while asleep.
  const Scalar* vision_level() const { return vision_.level(); }
  uint32_t vision_features_count() const { return vision_.features(); }
  bool has_vision() const { return has_vision_; }
  const VocalParams& voice() const { return vocal_.params(); }
  const Scalar* vocal_groups() const { return vocal_.groups(); }
  const Scalar* vocal_activities() const { return vocal_.activities(); }
  uint32_t vocal_frame() const { return vocal_.frame(); }
  // DNA v48. The motor dictionary, for telemetry and for experiments. A
  // dictionary stuck on one posture and a dictionary that is genuinely
  // selecting produce identical firing rates, identical duty cycles and an
  // identical `babble` verdict, so the usage has to be readable from outside.
  const VocalDecoder& vocal_decoder() const { return vocal_; }
  const Expression& expression() const { return expression_.value(); }
  const RewardBreakdown& reward() const { return reward_; }
  const Critic& critic() const { return critic_; }
  // Current motor exploration multiplier: 1.0 is the genome's plain noise_amp,
  // below is a creature settling into a habit, above is one casting about.
  // Exposed because babble that has stopped varying and babble that has stopped
  // are the same reading on every other number the panel shows.
  Scalar exploration() const { return explore_; }
  DnaStatus dna_status() const { return dna_status_; }
  size_t arena_used() const { return arena_.used(); }
  size_t arena_capacity() const { return arena_.capacity(); }
  const void* arena_base() const { return arena_.base(); }

  // Snapshot seam (§8) — see aibaby/snapshot.h, which is what calls these.
  // Everything the creature is that is not in the arena: the generator it draws
  // noise from, its drives, what the transducers are mid-way through, and the
  // reward loop's running state. Delegates to the network and the critic.
  void save_state(SnapshotWriter& w) const;
  void load_state(SnapshotReader& r);

  // Every plasticity interval the baby is asleep or awake; the counter is
  // useful for pacing the panel.
  uint64_t plasticity_events() const { return plasticity_events_; }

  // M4. The plateau detector's current reading, and whether the creature is
  // re-experiencing an episode rather than perceiving one.
  const GrowthWatch& growth_watch() const { return growth_; }
  bool replaying() const { return replay_left_ > 0; }
  uint32_t episodes_stored() const { return episodes_stored_; }

 private:
  void update_drives();
  // Recomputes `explore_` from the reward windows and pushes it into the
  // network, scaled by each module's explore_scale.
  void apply_exploration();
  void cash_in_reward();
  // §3.4's first condition, evaluated once per plateau window. Returns true on
  // the tick a window closes with no improvement left in it.
  bool check_plateau();
  void try_grow();
  void record_episode();
  void begin_replay();
  void drive_replay();

  Dna dna_;
  DnaStatus dna_status_ = DnaStatus::kOk;
  Arena arena_;
  Rng rng_;
  Network network_;
  Critic critic_;
  Drives drives_;
  SleepState sleep_ = SleepState::kAwake;
  RewardBreakdown reward_;

  AuditoryEncoder auditory_;
  VisionEncoder vision_;
  TouchEncoder touch_;
  VocalDecoder vocal_;
  ExpressionDecoder expression_;

  bool has_auditory_ = false;
  bool has_vision_ = false;
  bool has_touch_ = false;
  bool has_vocal_ = false;
  bool has_expression_ = false;

  uint32_t plasticity_interval_ = 1;
  uint32_t homeo_interval_ = 1;
  uint32_t vocal_interval_ = 1;

  // Exploration (DnaExploration). Two windows on total reward whose difference
  // says whether things are going better than they have been; `explore_` is the
  // multiplier that difference produces, before each module's explore_scale.
  Scalar reward_fast_ = kZero;
  Scalar reward_slow_ = kZero;
  Scalar fast_alpha_ = kZero;
  Scalar slow_alpha_ = kZero;
  Scalar explore_ = kOne;

  Scalar reward_baseline_ = kZero;
  // DNA v20: one running baseline per neuromodulator channel.
  Scalar nm_baseline_[4] = {};
  Scalar baseline_alpha_ = kZero;
  Scalar pending_external_ = kZero;
  Scalar pending_curiosity_ = kZero;
  Scalar prev_hunger_ = kZero;
  Scalar prev_comfort_ = kZero;
  uint64_t plasticity_events_ = 0;

  // --- M4 ---
  GrowthWatch growth_;
  uint64_t last_growth_tick_ = 0;
  uint64_t last_consolidation_ = 0;

  // Replay (§3.6). One episode is the sensory frame that was on the senses
  // when a better-than-expected reward arrived, plus that reward. Stored as
  // the *encoder levels* rather than the raw host frame, because that is what
  // the brain was actually being driven with — the thing worth re-living.
  Scalar* episode_mel_ = nullptr;      // capacity_episodes x mel channels
  Scalar* episode_retina_ = nullptr;   // capacity_episodes x vision features
  Scalar* episode_reward_ = nullptr;   // capacity_episodes
  uint32_t episode_capacity_ = 0;
  uint32_t episodes_stored_ = 0;
  uint32_t episode_next_ = 0;          // ring cursor: newest overwrites oldest
  uint32_t replay_index_ = 0;          // episode currently being re-experienced
  uint32_t replay_left_ = 0;           // ticks remaining in it
  uint32_t replay_queued_ = 0;         // episodes still to run this sleep pass
};

}  // namespace aibaby

#endif  // AIBABY_BRAIN_H
