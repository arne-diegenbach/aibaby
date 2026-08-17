#include "aibaby/critic.h"

#include <math.h>  // expf — build path only

#include "aibaby/snapshot_io.h"

namespace aibaby {
namespace {

inline Scalar clampf(Scalar v, Scalar lo, Scalar hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

}  // namespace

size_t Critic::required_bytes(const Dna& dna) {
  const uint32_t channels = dna.header().audio.mel_channels;
  return size_t(kCriticBins + 1) * channels * sizeof(Scalar) + 64;
}

bool Critic::build(const Dna& dna, Arena& arena, uint32_t source_module) {
  const DnaAudio& a = dna.header().audio;
  const DnaCuriosity& c = dna.header().curiosity;

  channels_ = a.mel_channels;
  source_module_ = source_module;
  learn_rate_ = Scalar(c.learn_rate);
  gain_ = Scalar(c.gain);
  frames_ = 0;
  fast_err_ = kZero;
  slow_err_ = kZero;
  have_features_ = false;
  have_prev_ = false;
  persistence_ = c.persistence_base != 0;
  for (uint32_t i = 0; i < kMaxMelChannels; ++i) {
    pred_[i] = kZero;
    prev_[i] = kZero;
  }

  // Error windows are stepped once per audio frame, not per tick, so their
  // smoothing constants are expressed in frames.
  const Scalar frame_ms = Scalar(1000) * Scalar(a.hop) / Scalar(a.sample_rate);
  fast_alpha_ = c.fast_tau_ms > 0.0f
                    ? kOne - Scalar(expf(-float(frame_ms) / c.fast_tau_ms))
                    : kOne;
  slow_alpha_ = c.slow_tau_ms > 0.0f
                    ? kOne - Scalar(expf(-float(frame_ms) / c.slow_tau_ms))
                    : kOne;

  // Features are normalised against the source module's own target rate, so a
  // module tuned to fire at 5 Hz and one tuned to 50 Hz both land in [0,1].
  const Scalar target = Scalar(dna.module(source_module).target_rate_hz);
  rate_norm_ = target > kZero ? target * Scalar(3) : Scalar(20);

  w_ = arena.alloc_zeroed<Scalar>(size_t(kCriticBins) * channels_);
  bias_ = arena.alloc_zeroed<Scalar>(channels_);
  return arena.ok();
}

// Coarse spatial binning of the source module. Binning rather than one feature
// per neuron is what keeps the critic the same size before and after growth
// (M4) — a grown brain must not silently reset its own curiosity.
void Critic::extract_features(const Network& net) {
  const ModuleState& ms = net.module(source_module_);
  for (uint32_t b = 0; b < kCriticBins; ++b) {
    const uint32_t begin = ms.begin + slice_begin(ms.count, kCriticBins, b);
    const uint32_t end = ms.begin + slice_begin(ms.count, kCriticBins, b + 1);
    if (end <= begin) {
      features_[b] = kZero;
      continue;
    }
    Scalar sum = kZero;
    for (uint32_t i = begin; i < end; ++i) sum += net.rate_fast(i);
    features_[b] = clampf((sum / Scalar(end - begin)) / rate_norm_, kZero, kOne);
  }
}

void Critic::observe(const Network& net, const Scalar* mel, uint32_t count) {
  if (w_ == nullptr || channels_ == 0) return;
  const uint32_t n = count < channels_ ? count : channels_;

  if (have_features_) {
    // Score the prediction the *previous* frame's activity implied, then
    // correct it. Predicting the frame we are already looking at would be
    // free, and would make progress meaningless.
    Scalar sq_error = kZero;
    for (uint32_t c = n; c < channels_; ++c) pred_[c] = kZero;
    for (uint32_t c = 0; c < n; ++c) {
      // The learned part is the same either way; what changes is what it is
      // added to. With a persistence base the weights carry the *departure*
      // from the last frame, so the model's floor is the trivial predictor
      // instead of its ceiling, and every point of R^2 above that floor is the
      // brain state actually contributing something.
      Scalar pred = (persistence_ && have_prev_) ? prev_[c] : kZero;
      pred += bias_[c];
      for (uint32_t b = 0; b < kCriticBins; ++b) {
        pred += w_[size_t(b) * channels_ + c] * features_[b];
      }
      // Kept raw here, before the correction below: this is the number the
      // error windows are scored on, and DNA v15 subtracts the very same one at
      // the ears. Two different predictions — one scored, one acted on — would
      // make the curiosity reward a statement about a signal the creature never
      // received.
      pred_[c] = pred;
      const Scalar err = mel[c] - pred;
      sq_error += err * err;

      const Scalar step = learn_rate_ * err;
      bias_[c] += step;
      for (uint32_t b = 0; b < kCriticBins; ++b) {
        w_[size_t(b) * channels_ + c] += step * features_[b];
      }
    }
    const Scalar mse = n ? sq_error / Scalar(n) : kZero;

    if (frames_ == 0) {
      fast_err_ = mse;
      slow_err_ = mse;
    } else {
      fast_err_ += fast_alpha_ * (mse - fast_err_);
      slow_err_ += slow_alpha_ * (mse - slow_err_);
    }
    ++frames_;
  }

  // Recorded after the scoring above, never before it: the frame the model was
  // asked to predict must not be in the evidence it predicted from.
  for (uint32_t c = 0; c < n; ++c) prev_[c] = mel[c];
  for (uint32_t c = n; c < channels_; ++c) prev_[c] = kZero;
  have_prev_ = true;

  extract_features(net);
  have_features_ = true;
}

Scalar Critic::progress() const {
  if (!ready()) return kZero;
  const Scalar delta = slow_err_ - fast_err_;
  return delta > kZero ? gain_ * delta : kZero;
}

// --- Snapshot (§8) ---------------------------------------------------------
//
// frames_ matters more than it looks: ready() is false for the first three
// frames, and a critic restored as unready would report no curiosity and no
// plateau until the creature had heard something again.

void Critic::save_state(SnapshotWriter& w) const {
  w.put(frames_);
  w.put(fast_err_);
  w.put(slow_err_);
  for (uint32_t i = 0; i < kCriticBins; ++i) w.put(features_[i]);
  const uint8_t have = have_features_ ? 1 : 0;
  w.put(have);
  // DNA v15. Written unconditionally rather than only when the persistence
  // base is switched on, because a snapshot's layout must depend on the build
  // and not on a genome field — otherwise two creatures write files of
  // different shapes and the layout fingerprint stops meaning anything.
  for (uint32_t i = 0; i < kMaxMelChannels; ++i) w.put(prev_[i]);
  const uint8_t had_prev = have_prev_ ? 1 : 0;
  w.put(had_prev);
}

void Critic::load_state(SnapshotReader& r) {
  r.get(frames_);
  r.get(fast_err_);
  r.get(slow_err_);
  for (uint32_t i = 0; i < kCriticBins; ++i) r.get(features_[i]);
  uint8_t have = 0;
  r.get(have);
  have_features_ = have != 0;
  for (uint32_t i = 0; i < kMaxMelChannels; ++i) r.get(prev_[i]);
  uint8_t had_prev = 0;
  r.get(had_prev);
  have_prev_ = had_prev != 0;
}

}  // namespace aibaby
