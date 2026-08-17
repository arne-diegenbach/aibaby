// The creature's voice, and the ear that binds it to the world.
//
// The spectral analysis moved to host/mel.h on 2026-08-15 — Cochlea and Mfcc
// live there. What is here is the half that is about *this creature*: the
// synthesiser that stands in for its larynx, and the Ear that mixes the room
// with the creature's own voice and delivers the result to a Brain.

#ifndef AIBABY_HOST_AUDIO_H
#define AIBABY_HOST_AUDIO_H

#include <cstdint>
#include <string>
#include <vector>

#include "aibaby/brain.h"
#include "aibaby/dna.h"
#include "host/mel.h"

namespace aibaby_host {

// A tiny source-filter synthesiser, so the headless experiments can speak to
// the baby with the same kind of signal the browser will. Not used by the
// live path — there the microphone is the source.
class VowelSource {
 public:
  VowelSource() = default;
  VowelSource(uint32_t sample_rate) : sample_rate_(sample_rate) {}
  void configure(uint32_t sample_rate) { sample_rate_ = sample_rate; }

  // Generates `count` samples of a voiced vowel with the given pitch and
  // first two formants.
  void render(float f0, float f1, float f2, float amplitude, float* out, size_t count);

 private:
  uint32_t sample_rate_ = 16000;
  float phase_ = 0.0f;
  float y1_[2] = {0.0f, 0.0f};
  float y2_[2] = {0.0f, 0.0f};
};

// The ear, including the half that hears the creature itself.
//
// §5.3 says babble is "motor noise shaped by the curiosity drive", and
// curiosity is prediction error on the next mel frame. Until this existed the
// creature's synthesised voice went to the panel and never came back, so its
// own babbling produced no sensory consequence whatsoever and the mechanism the
// requirements name could not run. Nothing in the system carried a signal from
// which an articulatory map — which motor command makes which sound — could be
// learned.
//
// Two things this deliberately is:
//
//   - **Host code, like the rest of the DSP (§6.2).** The network still emits
//     eight parameters and never a sample; the body that turns them into air
//     and the ear that hears that air both live out here.
//   - **One class that every caller shares.** The live loop and the headless
//     experiments have to hear identically or an experiment stops predicting
//     the creature, and self-hearing wired separately into six places is six
//     chances to differ. Everything that drives the brain's ears goes through
//     `tick()`.
//
// The journal's contract survives unchanged: it records the mel frame the core
// was actually given, which now includes the creature's own voice. A replay
// that feeds those frames back must therefore *not* also run a self-voice, or
// it double-counts.
class Ear {
 public:
  bool configure(const aibaby::DnaAudio& cfg, std::string& error);

  // One tick of hearing. `room` is whatever the world is doing — microphone,
  // caregiver, test tone — and may be null for a silent room. The creature's
  // own voice is mixed in at the genome's `self_gain`, the result goes through
  // the cochlea, and every mel frame that completes is delivered to the brain.
  void tick(aibaby::Brain& brain, const float* room, size_t count);

  // The most recent completed frame, for the journal and the panel, and
  // whether one completed during the last tick.
  const std::vector<float>& latest_mel() const { return latest_; }
  bool had_frame() const { return had_frame_; }

  uint32_t channels() const { return cochlea_.channels(); }
  uint32_t sample_rate() const { return cochlea_.sample_rate(); }
  uint32_t hop() const { return cochlea_.hop(); }
  float take_peak() { return cochlea_.take_peak(); }
  void reset_stream() { cochlea_.reset_stream(); }

  // What the creature is hearing of itself, in the same units `render` emits.
  // Reported so an experiment can show the loop is closed rather than assume
  // it, and so a howl is visible as a number before it is audible.
  float self_level() const { return self_level_; }

 private:
  Cochlea cochlea_;
  VowelSource larynx_;
  std::vector<float> mix_;
  std::vector<float> latest_;
  std::vector<float> self_;
  float self_gain_ = 0.0f;
  float self_level_ = 0.0f;
  bool had_frame_ = false;
};

}  // namespace aibaby_host

#endif  // AIBABY_HOST_AUDIO_H
