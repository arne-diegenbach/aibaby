// The spectral front end: PCM in, mel bands out, and the cepstrum of those.
//
// Split out of host/audio.h on 2026-08-15. What is left in that file is the
// creature's *voice* and the ear that binds the two together; this file is pure
// analysis and knows nothing about a brain. The dependency runs one way — Ear
// includes this, nothing here includes Ear — which is what makes it possible to
// use the cochlea from a tool, a test, or an offline analysis without dragging
// a Brain along.
//
// This is host code on purpose (§6.2). The FFT and the filterbank are signal
// processing that a desktop, a phone, or an ESP32 each do differently; what the
// creature is born with is the *shape* of the result — 24 log-spaced bands at
// 62.5 frames per second — and that shape lives in the genome.
//
// Everything downstream of a normalised mel frame is core code, so the encoding
// a neuron sees is identical on every platform.

#ifndef AIBABY_HOST_MEL_H
#define AIBABY_HOST_MEL_H

#include <cstdint>
#include <string>
#include <vector>

#include "aibaby/dna.h"

namespace aibaby_host {

class Cochlea {
 public:
  bool configure(const aibaby::DnaAudio& cfg, std::string& error);

  // Feeds mono PCM in [-1, 1] at the genome's sample rate. Frames are emitted
  // as they complete; `frames()` holds them until the caller drains it.
  void push(const float* samples, size_t count);

  // Normalised [0,1] band energies, `channels()` per frame, concatenated.
  const std::vector<float>& frames() const { return frames_; }
  void clear_frames() { frames_.clear(); }

  // Discards any partial window. Call when the microphone stops, or the
  // leftover samples are silently spliced onto whatever is said next.
  void reset_stream() { pending_.clear(); peak_ = 0.0f; }

  uint32_t channels() const { return channels_; }
  uint32_t window() const { return window_; }
  uint32_t hop() const { return hop_; }
  uint32_t sample_rate() const { return sample_rate_; }
  uint64_t frames_produced() const { return frames_produced_; }

  // Peak |sample| seen since the last call, for the mic level meter. Reading
  // it resets it.
  float take_peak();

 private:
  void transform();

  std::vector<float> pending_;      // samples not yet consumed by a frame
  std::vector<float> window_fn_;    // Hann
  std::vector<float> re_, im_;
  std::vector<float> power_;
  std::vector<float> frames_;

  // Triangular mel filters, stored as a bin range plus weights so the inner
  // loop touches only the bins a filter actually covers.
  struct Filter {
    uint32_t begin = 0;
    std::vector<float> weight;
    float norm = 1.0f;
  };
  std::vector<Filter> filters_;

  uint32_t sample_rate_ = 16000;
  uint32_t window_ = 512;
  uint32_t hop_ = 256;
  uint32_t channels_ = 24;
  float floor_db_ = -60.0f;
  float peak_ = 0.0f;
  uint64_t frames_produced_ = 0;
};

// Mel-frequency cepstral coefficients: the DCT of what Cochlea produces.
//
// Cochlea owns the mel half of the name — FFT, triangular filterbank, log
// compression, normalisation. This owns the *cepstral* half, which the project
// did not have: a discrete cosine transform of the log-mel vector, which
// rotates 24 heavily correlated band energies into a handful of nearly
// uncorrelated coefficients ordered from coarse spectral tilt to fine detail.
//
// **This deliberately does not feed the brain, and that is a design decision
// rather than an omission.** Two reasons, in order of weight:
//
//   - A cochlea does not do a DCT. It is a bank of tuned resonators, and what
//     leaves it is band energies on a tonotopic map. MFCCs are an engineering
//     convenience invented to decorrelate features for Gaussian mixture models,
//     and feeding them to a creature whose whole premise is biological
//     plausibility would be a realism regression, not an upgrade.
//   - The auditory encoder maps mel channel c onto a contiguous slice of B2, so
//     neighbouring neurons hear neighbouring frequencies. That is tonotopy, and
//     the structured wiring rules downstream assume it. Cepstral coefficient 3
//     is not a frequency and has no neighbour, so the map would become
//     meaningless while still looking like it worked.
//
// Where it does belong is the *measurement* layer, and there it earns its keep
// immediately. `pcprobe` fits linear readouts to the mel frame, and mel bands
// are so collinear that the fit's own self-test — recover the target from a
// copy of itself — reads 0.972 instead of 1. Against an MFCC target the same
// self-test reads exactly 1.000, and every conclusion drawn from that probe
// survives the change of representation.
class Mfcc {
 public:
  // `channels` mel bands in, `coefficients` cepstral values out. `lifter` is
  // the usual sinusoidal weighting that stops the high quefrency terms being
  // numerically tiny; 0 disables it.
  bool configure(uint32_t channels, uint32_t coefficients, float lifter,
                 std::string& error);

  // One frame of Cochlea's normalised log-mel energies in. Returns the
  // coefficient vector, which is also kept in `latest()`.
  const std::vector<float>& transform(const float* log_mel, uint32_t count);

  const std::vector<float>& latest() const { return out_; }
  uint32_t coefficients() const { return coefficients_; }
  uint32_t channels() const { return channels_; }

 private:
  // The DCT-II basis, coefficients_ x channels_, row-major, with the
  // orthonormal scaling and any liftering already folded in. Precomputed for
  // the same reason Cochlea precomputes its filters: the sizes are fixed at
  // configure time and a per-frame cosine call would be pure waste.
  std::vector<float> basis_;
  std::vector<float> out_;
  uint32_t channels_ = 0;
  uint32_t coefficients_ = 0;
};

}  // namespace aibaby_host

#endif  // AIBABY_HOST_MEL_H
