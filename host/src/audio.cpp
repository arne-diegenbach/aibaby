#include "host/audio.h"

#include <cmath>

namespace aibaby_host {
namespace {

// Its own copy rather than a shared one: this file and mel.cpp are independent
// translation units and a one-line constant is not worth a header to hold it.
constexpr float kPi = 3.14159265358979323846f;

}  // namespace

// A two-formant source-filter voice: an impulse-ish glottal pulse train run
// through two resonators. Crude, but it is a *vowel* — which is all the
// experiments need it to be.
void VowelSource::render(float f0, float f1, float f2, float amplitude, float* out,
                         size_t count) {
  const float sr = float(sample_rate_);
  const float bw = 90.0f;
  const float r1 = std::exp(-kPi * bw / sr);
  const float r2 = std::exp(-kPi * bw / sr);
  const float a1 = 2.0f * r1 * std::cos(2.0f * kPi * f1 / sr);
  const float b1 = -r1 * r1;
  const float a2 = 2.0f * r2 * std::cos(2.0f * kPi * f2 / sr);
  const float b2 = -r2 * r2;

  for (size_t i = 0; i < count; ++i) {
    phase_ += f0 / sr;
    float excitation = 0.0f;
    if (phase_ >= 1.0f) {
      phase_ -= 1.0f;
      excitation = 1.0f;  // one pulse per glottal cycle
    }
    const float v1 = excitation + a1 * y1_[0] + b1 * y1_[1];
    y1_[1] = y1_[0];
    y1_[0] = v1;
    const float v2 = excitation + a2 * y2_[0] + b2 * y2_[1];
    y2_[1] = y2_[0];
    y2_[0] = v2;
    out[i] = amplitude * 0.25f * (v1 + v2);
  }
}

// --- Ear: the cochlea plus the half of §5.3 that hears the creature ---------

bool Ear::configure(const aibaby::DnaAudio& cfg, std::string& error) {
  if (!cochlea_.configure(cfg, error)) return false;
  larynx_.configure(cfg.sample_rate);
  self_gain_ = cfg.self_gain;
  latest_.assign(cochlea_.channels(), 0.0f);
  return true;
}

void Ear::tick(aibaby::Brain& brain, const float* room, size_t count) {
  had_frame_ = false;
  if (count == 0) return;

  mix_.assign(count, 0.0f);
  if (room) {
    for (size_t i = 0; i < count; ++i) mix_[i] = room[i];
  }

  // The creature's own voice, mixed into the room rather than replacing it —
  // a baby that babbles while its mother is talking hears both.
  if (self_gain_ > 0.0f) {
    const aibaby::VocalParams& v = brain.voice();
    self_.assign(count, 0.0f);
    // Voicing is a valve: shut, the glottis makes no pulses, and an amplitude
    // still ramping down must not leak through it.
    const float f0 = v.voicing > 0.5f ? float(v.f0) : 0.0f;
    larynx_.render(f0, float(v.f1), float(v.f2), float(v.amplitude) * self_gain_,
                   self_.data(), count);
    float peak = 0.0f;
    for (size_t i = 0; i < count; ++i) {
      mix_[i] += self_[i];
      const float a = self_[i] < 0.0f ? -self_[i] : self_[i];
      if (a > peak) peak = a;
    }
    self_level_ = peak;
  }

  cochlea_.push(mix_.data(), mix_.size());
  const std::vector<float>& frames = cochlea_.frames();
  const uint32_t channels = cochlea_.channels();
  for (size_t f = 0; f + channels <= frames.size(); f += channels) {
    brain.hear(&frames[f], channels);
    for (uint32_t c = 0; c < channels; ++c) latest_[c] = frames[f + c];
    had_frame_ = true;
  }
  cochlea_.clear_frames();
}

}  // namespace aibaby_host
