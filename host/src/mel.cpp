#include "host/mel.h"

#include <cmath>

namespace aibaby_host {
namespace {

constexpr float kPi = 3.14159265358979323846f;

// The mel scale is what the cochlea actually does: roughly linear below 1 kHz
// and logarithmic above it. Using it instead of raw FFT bins is what makes 24
// numbers a sufficient description of a sound (§5.2).
float hz_to_mel(float hz) { return 2595.0f * std::log10(1.0f + hz / 700.0f); }
float mel_to_hz(float mel) { return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f); }

// In-place iterative radix-2 Cooley-Tukey. The window size is validated as a
// power of two when the genome loads, so there is no mixed-radix path.
void fft(std::vector<float>& re, std::vector<float>& im) {
  const size_t n = re.size();
  for (size_t i = 1, j = 0; i < n; ++i) {
    size_t bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) {
      std::swap(re[i], re[j]);
      std::swap(im[i], im[j]);
    }
  }

  for (size_t len = 2; len <= n; len <<= 1) {
    const float angle = -2.0f * kPi / float(len);
    const float wr = std::cos(angle);
    const float wi = std::sin(angle);
    for (size_t i = 0; i < n; i += len) {
      float cr = 1.0f, ci = 0.0f;
      for (size_t k = 0; k < len / 2; ++k) {
        const size_t a = i + k;
        const size_t b = a + len / 2;
        const float tr = re[b] * cr - im[b] * ci;
        const float ti = re[b] * ci + im[b] * cr;
        re[b] = re[a] - tr;
        im[b] = im[a] - ti;
        re[a] += tr;
        im[a] += ti;
        const float ncr = cr * wr - ci * wi;
        ci = cr * wi + ci * wr;
        cr = ncr;
      }
    }
  }
}

}  // namespace

bool Cochlea::configure(const aibaby::DnaAudio& cfg, std::string& error) {
  sample_rate_ = cfg.sample_rate;
  window_ = cfg.window;
  hop_ = cfg.hop;
  channels_ = cfg.mel_channels;
  floor_db_ = cfg.floor_db;

  if (window_ == 0 || (window_ & (window_ - 1)) != 0) {
    error = "audio window must be a power of two";
    return false;
  }
  if (hop_ == 0 || hop_ > window_) {
    error = "audio hop must be in (0, window]";
    return false;
  }
  if (floor_db_ >= 0.0f) {
    error = "audio floor_db must be negative";
    return false;
  }

  window_fn_.resize(window_);
  for (uint32_t i = 0; i < window_; ++i) {
    window_fn_[i] = 0.5f - 0.5f * std::cos(2.0f * kPi * float(i) / float(window_ - 1));
  }

  re_.resize(window_);
  im_.resize(window_);
  power_.resize(window_ / 2 + 1);
  pending_.clear();
  frames_.clear();
  frames_produced_ = 0;

  // channels_ triangular filters, each spanning from the previous centre to
  // the next one, evenly spaced on the mel scale.
  const float mel_low = hz_to_mel(cfg.mel_low_hz);
  const float mel_high = hz_to_mel(cfg.mel_high_hz);
  const uint32_t bins = window_ / 2 + 1;
  const float bin_hz = float(sample_rate_) / float(window_);

  std::vector<float> centre_bin(channels_ + 2);
  for (uint32_t i = 0; i < channels_ + 2; ++i) {
    const float mel = mel_low + (mel_high - mel_low) * float(i) / float(channels_ + 1);
    centre_bin[i] = mel_to_hz(mel) / bin_hz;
  }

  filters_.assign(channels_, Filter{});
  for (uint32_t c = 0; c < channels_; ++c) {
    const float lo = centre_bin[c];
    const float mid = centre_bin[c + 1];
    const float hi = centre_bin[c + 2];
    const uint32_t begin = uint32_t(std::max(0.0f, std::floor(lo)));
    const uint32_t end = std::min(bins, uint32_t(std::ceil(hi)) + 1);
    if (end <= begin) {
      error = "mel filter " + std::to_string(c) + " is empty; widen the frequency range";
      return false;
    }
    Filter f;
    f.begin = begin;
    f.weight.resize(end - begin, 0.0f);
    float sum = 0.0f;
    for (uint32_t b = begin; b < end; ++b) {
      const float x = float(b);
      float w = 0.0f;
      if (x >= lo && x <= mid && mid > lo) w = (x - lo) / (mid - lo);
      else if (x > mid && x <= hi && hi > mid) w = (hi - x) / (hi - mid);
      f.weight[b - begin] = w;
      sum += w;
    }
    // Normalising by the filter's own weight keeps a wide high-frequency band
    // and a narrow low-frequency one on the same scale — otherwise the top of
    // the spectrum simply shouts down the bottom.
    f.norm = sum > 0.0f ? 1.0f / sum : 1.0f;
    filters_[c] = std::move(f);
  }
  return true;
}

void Cochlea::push(const float* samples, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    const float s = samples[i];
    const float a = s < 0.0f ? -s : s;
    if (a > peak_) peak_ = a;
    pending_.push_back(s);
  }

  while (pending_.size() >= window_) {
    transform();
    pending_.erase(pending_.begin(), pending_.begin() + long(hop_));
  }

  // A stalled or spammy client must not turn into unbounded memory. One
  // window of slack is all the algorithm ever needs.
  const size_t cap = size_t(window_) * 4;
  if (pending_.size() > cap) {
    pending_.erase(pending_.begin(), pending_.end() - long(cap));
  }
}

void Cochlea::transform() {
  for (uint32_t i = 0; i < window_; ++i) {
    re_[i] = pending_[i] * window_fn_[i];
    im_[i] = 0.0f;
  }
  fft(re_, im_);

  const float scale = 1.0f / float(window_);
  for (size_t b = 0; b < power_.size(); ++b) {
    const float r = re_[b] * scale;
    const float i = im_[b] * scale;
    power_[b] = r * r + i * i;
  }

  const size_t base = frames_.size();
  frames_.resize(base + channels_);
  for (uint32_t c = 0; c < channels_; ++c) {
    const Filter& f = filters_[c];
    float energy = 0.0f;
    for (size_t k = 0; k < f.weight.size(); ++k) {
      energy += power_[f.begin + k] * f.weight[k];
    }
    energy *= f.norm;

    // Log compression, then a linear map from the silence floor to full
    // scale. Everything the core sees is in [0,1].
    const float db = 10.0f * std::log10(energy + 1e-12f);
    float norm = (db - floor_db_) / (0.0f - floor_db_);
    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;
    frames_[base + c] = norm;
  }
  ++frames_produced_;
}

float Cochlea::take_peak() {
  const float p = peak_;
  peak_ = 0.0f;
  return p;
}

// --- MFCC ------------------------------------------------------------------

bool Mfcc::configure(uint32_t channels, uint32_t coefficients, float lifter,
                     std::string& error) {
  if (channels == 0) {
    error = "mfcc: needs at least one mel channel";
    return false;
  }
  if (coefficients == 0 || coefficients > channels) {
    error = "mfcc: coefficients must be between 1 and the number of mel channels";
    return false;
  }
  channels_ = channels;
  coefficients_ = coefficients;
  out_.assign(coefficients_, 0.0f);
  basis_.assign(size_t(coefficients_) * channels_, 0.0f);

  // Orthonormal DCT-II. The orthonormal scaling matters here rather than being
  // cosmetic: it makes the transform a rotation, so the total energy in the
  // coefficient vector equals the total in the band vector and a coefficient
  // can be compared against a band without a hidden factor of N.
  const float n = float(channels_);
  for (uint32_t k = 0; k < coefficients_; ++k) {
    const float scale = (k == 0) ? std::sqrt(1.0f / n) : std::sqrt(2.0f / n);
    // Sinusoidal liftering, the usual HTK weighting. Without it the high
    // quefrency coefficients are numerically tiny next to the low ones, and
    // anything that treats the vector as a whole — a distance, a regression —
    // effectively ignores them.
    const float w = (lifter > 0.0f)
                        ? 1.0f + 0.5f * lifter * std::sin(kPi * float(k) / lifter)
                        : 1.0f;
    for (uint32_t c = 0; c < channels_; ++c) {
      const float angle = kPi * float(k) * (float(c) + 0.5f) / n;
      basis_[size_t(k) * channels_ + c] = scale * w * std::cos(angle);
    }
  }
  return true;
}

// Cochlea hands over dB mapped linearly onto [0,1] rather than raw log energy.
// That is an affine function of the log, and an affine offset is constant
// across the band index, so it lands entirely in c0 and leaves every other
// coefficient exactly what it would have been. c0 is therefore "overall
// loudness, in this genome's units" and is not comparable to a textbook c0 —
// which is fine, because nothing here compares one.
const std::vector<float>& Mfcc::transform(const float* log_mel, uint32_t count) {
  const uint32_t n = count < channels_ ? count : channels_;
  for (uint32_t k = 0; k < coefficients_; ++k) {
    const float* row = &basis_[size_t(k) * channels_];
    float sum = 0.0f;
    for (uint32_t c = 0; c < n; ++c) sum += row[c] * log_mel[c];
    out_[k] = sum;
  }
  return out_;
}

}  // namespace aibaby_host
