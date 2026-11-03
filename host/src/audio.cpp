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
  // The pre-v83 call: no third formant to give and no commanded bandwidths. The
  // zeroes are never read, because both gates below are gains, not values.
  render(f0, f1, f2, 0.0f, 0.0f, 0.0f, 0.0f, amplitude, out, count);
}

// DNA v83. See DnaAudio::render_f3_gain. The two gains are the whole of the
// change, and AT ZERO THIS IS THE OLD FUNCTION LINE FOR LINE -- `bw` is the same
// literal 90.0f and the third resonator is never evaluated, so the output is
// bit-identical by construction rather than because two float expressions ought
// to agree. That distinction is the point: `x + 0.0f * y` is not reliably `x`.
void VowelSource::render(float f0, float f1, float f2, float f3, float bw1,
                         float bw2, float bw3, float amplitude, float* out,
                         size_t count) {
  const float sr = float(sample_rate_);
  const bool bw_on = bw_gain_ > 0.0f;
  const bool f3_on = f3_gain_ > 0.0f;
  // Interpolated from the old fixed 90 Hz toward what the creature commanded, so
  // the gain is a dose and 1.0 means "the bandwidths mean what they say".
  const float bwa = bw_on ? 90.0f + bw_gain_ * (bw1 - 90.0f) : 90.0f;
  const float bwb = bw_on ? 90.0f + bw_gain_ * (bw2 - 90.0f) : 90.0f;
  const float bwc = bw_on ? 90.0f + bw_gain_ * (bw3 - 90.0f) : 90.0f;
  const float r1 = std::exp(-kPi * bwa / sr);
  const float r2 = std::exp(-kPi * bwb / sr);
  const float r3 = std::exp(-kPi * bwc / sr);
  const float a1 = 2.0f * r1 * std::cos(2.0f * kPi * f1 / sr);
  const float b1 = -r1 * r1;
  const float a2 = 2.0f * r2 * std::cos(2.0f * kPi * f2 / sr);
  const float b2 = -r2 * r2;
  const float a3 = 2.0f * r3 * std::cos(2.0f * kPi * f3 / sr);
  const float b3 = -r3 * r3;

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
    if (!f3_on) {
      out[i] = amplitude * 0.25f * (v1 + v2);
      continue;
    }
    const float v3 = excitation + a3 * y3_[0] + b3 * y3_[1];
    y3_[1] = y3_[0];
    y3_[0] = v3;
    out[i] = amplitude * 0.25f * (v1 + v2 + f3_gain_ * v3);
  }
}

// --- Ear: the cochlea plus the half of §5.3 that hears the creature ---------

bool Ear::configure(const aibaby::DnaAudio& cfg, std::string& error) {
  if (!cochlea_.configure(cfg, error)) return false;
  // DNA v83. At the shipped 0.0 / 0.0 this is the one-argument configure exactly.
  larynx_.configure(cfg.sample_rate, cfg.render_f3_gain, cfg.render_bw_gain);
  self_gain_ = cfg.self_gain;
  // DNA v69. Sized from the genome, in samples. Zero length means the delay is
  // off and tick() keeps its original branch.
  const uint32_t n = cfg.self_delay_ms > 0.0f
                         ? uint32_t(cfg.self_delay_ms * float(cfg.sample_rate) / 1000.0f + 0.5f)
                         : 0u;
  self_delay_.assign(n, 0.0f);
  self_delay_w_ = 0;
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
    // `selfloop`'s altered auditory feedback. Clamped to the tract's own range
    // so a large shift cannot synthesise a formant the creature could never
    // produce, which would confound "did not respond" with "heard nonsense".
    float heard_f1 = (self_f1_fixed_ > 0.0f ? self_f1_fixed_ : float(v.f1)) +
                     self_f1_shift_;
    if (heard_f1 < 250.0f) heard_f1 = 250.0f;
    if (heard_f1 > 1000.0f) heard_f1 = 1000.0f;
    // DNA v83. The four parameters the creature commands and never used to hear.
    // At the shipped gains they are read and discarded inside `render`, which is
    // the same thing that happened before, one function deeper.
    larynx_.render(f0, heard_f1, float(v.f2), float(v.f3), float(v.bw1),
                   float(v.bw2), float(v.bw3), float(v.amplitude) * self_gain_,
                   self_.data(), count);
    float peak = 0.0f;
    if (self_delay_.empty()) {
      for (size_t i = 0; i < count; ++i) {
        mix_[i] += self_[i];
        const float a = self_[i] < 0.0f ? -self_[i] : self_[i];
        if (a > peak) peak = a;
      }
    } else {
      // DNA v69. The ear receives what the larynx emitted `self_delay_ms` ago.
      // `self_level_` is measured on the DELAYED signal, i.e. on what the
      // cochlea actually gets, so the did-it-take check reads the real thing
      // rather than the undelayed render.
      const size_t n = self_delay_.size();
      for (size_t i = 0; i < count; ++i) {
        const float d = self_delay_[self_delay_w_];
        self_delay_[self_delay_w_] = self_[i];
        self_delay_w_ = self_delay_w_ + 1 == n ? 0 : self_delay_w_ + 1;
        mix_[i] += d;
        const float a = d < 0.0f ? -d : d;
        if (a > peak) peak = a;
      }
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
