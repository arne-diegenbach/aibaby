// Transducers: the places where the world becomes spikes and spikes become
// the world.
//
// These live in the core rather than the host because they are part of the
// creature, not part of the desktop. The host does DSP (FFT, mel) and audio
// output; everything between a normalised sensory vector and a neuron stays
// here, so the encoding travels with the creature rather than with the machine
// it happens to be running on.

#ifndef AIBABY_SENSES_H
#define AIBABY_SENSES_H

#include "aibaby/config.h"
#include "aibaby/dna.h"
#include "aibaby/network.h"
#include "aibaby/rng.h"

namespace aibaby {

enum class TouchAction : uint32_t {
  kFeed = 0,
  kPoke = 1,
  kTickle = 2,
  kStroke = 3,
  kTouchCount,
};

// §5.3 — eight continuous vocal-tract parameters plus a voicing gate. This is
// the entire motor output of the creature; the host turns it into sound.
struct VocalParams {
  Scalar f0 = Scalar(120);
  Scalar f1 = Scalar(500);
  Scalar f2 = Scalar(1500);
  Scalar f3 = Scalar(2500);
  Scalar bw1 = Scalar(80);
  Scalar bw2 = Scalar(90);
  Scalar bw3 = Scalar(120);
  Scalar amplitude = kZero;
  Scalar voicing = kZero;  // 0 or 1: the glottis is a valve, not a dial
};

// What the avatar shows (B6). Valence is *which* neurons fire, arousal is how
// hard the module is working — the same population-vector trick as the vocal
// tract, with two parameters instead of nine.
struct Expression {
  Scalar valence = Scalar(0.5);
  Scalar arousal = kZero;
};

// Rate-codes a mel frame into the auditory module.
//
// Within a channel's neuron group each neuron gets a different sensitivity,
// so a louder channel recruits more of the group. That is an intensity
// population code: it gives STDP something with structure to bite on, which a
// single scaled current per channel would not.
class AuditoryEncoder {
 public:
  void configure(const Dna& dna, uint32_t module_index);

  // `mel` holds normalised [0,1] band energies. Frames arrive at the hop rate
  // and are held until the next one, then faded — silence has to sound like
  // silence, not like the last thing the baby heard.
  void present(const Scalar* mel, uint32_t count);

  // `gate` open means the sound reaches B2. Shut — asleep, §3.6 — the levels
  // still decay: the ears close, they do not freeze on the last thing heard.
  // Anything reading level() for display therefore shows what the brain is
  // actually receiving rather than the last frame that happened to arrive.
  void drive(Network& net, bool gate);

  uint32_t module_index() const { return module_index_; }
  const Scalar* level() const { return level_; }
  uint32_t channels() const { return channels_; }

 private:
  Scalar level_[kMaxMelChannels] = {};
  uint32_t channels_ = 0;
  uint32_t module_index_ = 0;
  uint32_t hold_ticks_ = 0;
  uint32_t hold_left_ = 0;
  Scalar gain_ = kZero;
  Scalar fade_ = Scalar(0.9);
  bool active_ = false;
};

// Latency-codes a retinal frame into the vision module (§5.1).
//
// The auditory encoder spends population on intensity; this one spends *time*.
// Each ON/OFF cell owns a slice of B3 and fires once per camera frame, early
// if it responded strongly and late if it barely did, with the weakest cells
// not firing at all. So a frame arrives as a volley whose shape is the image,
// and two different scenes are two different spike patterns rather than two
// different amounts of activity.
//
// That distinction is the whole reason for the choice. A rate code would make
// "object present" mean "B3 is busier", which any bright wall would also mean.
class VisionEncoder {
 public:
  void configure(const Dna& dna, uint32_t module_index);

  // One retinal frame: `features` holds normalised [0,1] ON/OFF responses,
  // `vision_features(dna)` of them. Held for one frame period, then faded.
  void present(const Scalar* features, uint32_t count);

  // `gate` open means the frame reaches B3. Shut — asleep, §3.6 — the levels
  // still decay, for the same reason the ears do: eyes close, they do not
  // freeze on the last thing seen.
  void drive(Network& net, bool gate);

  uint32_t module_index() const { return module_index_; }
  const Scalar* level() const { return level_; }
  uint32_t features() const { return features_; }
  bool active() const { return active_; }

 private:
  // Scheduled tick, within the frame, at which each feature fires. A cell
  // below the contrast floor gets kSilent and never fires at all — which is
  // what makes an empty field quiet rather than uniformly late.
  static constexpr uint16_t kSilent = 0xFFFF;

  Scalar level_[kMaxVisionFeatures] = {};
  uint16_t fire_at_[kMaxVisionFeatures] = {};
  uint32_t features_ = 0;
  uint32_t module_index_ = 0;
  uint32_t hold_ticks_ = 0;
  uint32_t hold_left_ = 0;
  uint32_t phase_ = 0;         // ticks since this frame arrived
  uint32_t latency_ticks_ = 0;
  Scalar gain_ = kZero;
  Scalar floor_ = kZero;
  Scalar fade_ = Scalar(0.9);
  bool active_ = false;
};

// Caregiver touches (B4). Each action owns a slice of the somatosensory
// module, so "poke" and "tickle" are different places, not different sizes of
// the same thing.
class TouchEncoder {
 public:
  void configure(const Dna& dna, uint32_t module_index);
  void trigger(TouchAction action, Scalar intensity);
  void drive(Network& net);

  uint32_t module_index() const { return module_index_; }
  bool active() const { return active_; }

 private:
  Scalar level_[uint32_t(TouchAction::kTouchCount)] = {};
  uint32_t left_[uint32_t(TouchAction::kTouchCount)] = {};
  uint32_t module_index_ = 0;
  uint32_t duration_ticks_ = 0;
  Scalar gain_ = kZero;
  bool active_ = false;
};

// Reads the vocal motor module as nine population-coded groups.
class VocalDecoder {
 public:
  void configure(const Dna& dna, uint32_t module_index, Scalar update_ms);
  // `awake` gates the larynx: see §3.6.
  void update(const Network& net, bool awake);

  const VocalParams& params() const { return params_; }
  // Raw group readings before mapping, for telemetry: nine numbers in [0,1]
  // that say what the motor cortex is actually doing.
  const Scalar* groups() const { return group_value_; }
  // The other half of each group's reading, and until 2026-08-19 nothing could
  // see it. `groups()` is a population CENTROID over neuron index; this is the
  // group's mean rate against `rate_norm`. Only `activity[voicing]` and
  // `activity[8]` reach the larynx (as the gate and as loudness).
  //
  // This comment used to end "so if the object rides on rates rather than
  // positions, the vocal tract is discarding eight numbers that have it."
  // **Measured 2026-08-29 and the answer is no.** `m3probe` now prints a
  // bias-corrected d' for all eighteen readings against a 32-permutation null.
  // On four seed families the object is at the null in every one of them,
  // discarded and heard alike, while `vocal`'s population carries it at up to
  // 0.860 — so nothing is being thrown away that a different wiring of the
  // decoder would recover. The word, at the same population legibility, puts
  // d'^2 of +0.44 to +2.94 into the readings that do reach the air.
  // Read-only telemetry: exposing it changes no behaviour.
  const Scalar* activities() const { return group_activity_; }
  uint32_t frame() const { return frame_; }

  // --- DNA v48: the motor dictionary ---------------------------------------
  // Which posture is being held, its per-unit activities, and how many times
  // the winner has changed. All three are read-only telemetry and none of them
  // changes behaviour -- but a dictionary in which one unit always wins and a
  // dictionary that chatters every frame are both degenerate, and they look
  // identical in every formant the creature produces. `switches()` against
  // `frame()` is the number that tells them apart.
  uint32_t winner() const { return winner_; }
  const Scalar* unit_activities() const { return unit_activity_; }
  uint32_t units() const { return units_; }
  uint32_t switches() const { return switches_; }

  // DNA v49. The REINFORCE term for the posture just sampled, one per unit:
  // [u == chosen] - pi(u). Zero everywhere when the policy is deterministic.
  // `chose()` is true on exactly the frames a new posture was drawn, which is
  // when the term is meaningful and when the Brain hands it to the Network.
  const Scalar* policy_grad() const { return policy_grad_; }
  bool chose() const { return chose_; }
  // The larynx's module, so the Brain can tell the Network where to write.
  uint32_t module_index() const { return module_index_; }

 private:
  DnaVocal cfg_ = {};
  VocalParams params_;
  Scalar group_value_[kVocalGroups] = {};
  Scalar group_activity_[kVocalGroups] = {};
  uint32_t module_index_ = 0;
  uint32_t frame_ = 0;
  Scalar smooth_ = Scalar(1);
  Scalar gate_smooth_ = Scalar(1);
  bool configured_ = false;

  // DNA v48. The dictionary's own state: which slice holds the tract, how long
  // it is still entitled to, and the smoothed formant targets it names. The
  // targets are smoothed with `smooth_` exactly as the centroid path is, so
  // switching readouts does not also switch articulator inertia -- the whole
  // point is to change one thing.
  Scalar unit_activity_[kMaxDictionaryUnits] = {};
  uint32_t units_ = 0;
  uint32_t winner_ = 0;
  uint32_t dwell_ticks_ = 0;
  uint32_t dwell_left_ = 0;
  uint32_t switches_ = 0;
  Scalar dict_f1_ = kZero;
  Scalar dict_f2_ = kZero;
  Scalar held_f1_ = kZero;
  Scalar held_f2_ = kZero;
  Scalar policy_grad_[kMaxDictionaryUnits] = {};
  bool chose_ = false;
  // The decoder's own stream, so that turning the policy rate up or down does
  // not also re-roll the network's noise and make the two arms different
  // creatures. Seeded from the genome.
  //
  // KNOWN GAP: like `group_value_`, this is not carried in the snapshot, so a
  // resumed creature samples a different sequence of postures. The existing
  // decoder state has always had that property and `snapshot` does not cover
  // the decoder at all; it is recorded here rather than fixed because the
  // dictionary ships off, and it must be fixed before it ships on.
  Rng dict_rng_;
  bool dict_primed_ = false;
};

// Reads the expression module the same way, into two numbers.
class ExpressionDecoder {
 public:
  void configure(uint32_t module_index, Scalar rate_norm_hz);
  void update(const Network& net);
  const Expression& value() const { return value_; }

 private:
  Expression value_;
  uint32_t module_index_ = 0;
  Scalar rate_norm_ = Scalar(20);
};

}  // namespace aibaby

#endif  // AIBABY_SENSES_H
