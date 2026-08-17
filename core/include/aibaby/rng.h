// xoshiro256++ — seeded from the DNA, the only randomness in the core.
//
// Determinism is requirement G1: same DNA + same journal must yield a
// bit-identical brain. That means one PRNG, seeded once, advanced in a fixed
// order by a single-threaded kernel. Do not add a second stream, and do not
// draw from this in a parallel loop.

#ifndef AIBABY_RNG_H
#define AIBABY_RNG_H

#include "aibaby/config.h"

namespace aibaby {

class Rng {
 public:
  void seed(uint64_t value) {
    // splitmix64 to spread a small seed across the full state.
    for (int i = 0; i < 4; ++i) {
      value += 0x9E3779B97F4A7C15ULL;
      uint64_t z = value;
      z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
      z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
      s_[i] = z ^ (z >> 31);
    }
  }

  uint64_t next() {
    const uint64_t result = rotl(s_[0] + s_[3], 23) + s_[0];
    const uint64_t t = s_[1] << 17;
    s_[2] ^= s_[0];
    s_[3] ^= s_[1];
    s_[1] ^= s_[2];
    s_[0] ^= s_[3];
    s_[2] ^= t;
    s_[3] = rotl(s_[3], 45);
    return result;
  }

  // Uniform in [0, 1).
  Scalar uniform() {
    return Scalar(next() >> 40) * Scalar(1.0 / 16777216.0);
  }

  // Uniform in [-1, 1).
  Scalar signed_uniform() { return uniform() * Scalar(2) - Scalar(1); }

  // Approximately normal, mean 0, unit variance, via Irwin-Hall.
  //
  // Deliberately not Box-Muller: that needs log/sqrt/cos, and keeping
  // transcendentals out of the core is what makes the fixed-point port a
  // typedef change. This is only used at build time for weight
  // initialisation, so twelve draws costs nothing.
  Scalar normal() {
    Scalar sum = kZero;
    for (int i = 0; i < 12; ++i) sum += uniform();
    return sum - Scalar(6);
  }

  bool chance(Scalar p) { return uniform() < p; }

  // The snapshot seam (§8). A resumed creature has to draw the numbers it
  // would have drawn had it never stopped — re-seeding from the DNA would
  // rewind the stream to birth, and the membrane noise every neuron takes on
  // every tick comes out of it.
  void save(uint64_t out[4]) const {
    for (int i = 0; i < 4; ++i) out[i] = s_[i];
  }
  void restore(const uint64_t in[4]) {
    for (int i = 0; i < 4; ++i) s_[i] = in[i];
  }

 private:
  static uint64_t rotl(uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }
  uint64_t s_[4] = {0, 0, 0, 0};
};

}  // namespace aibaby

#endif  // AIBABY_RNG_H
