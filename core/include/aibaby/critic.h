// Curiosity: a forward model of the next sensory frame, and the two error
// windows that turn its failures into a drive.
//
// §3.3 is explicit that reward comes from the *reduction* in prediction error,
// not from the error itself. Rewarding raw error points the baby at the
// noisiest thing in the room and leaves it there — static is maximally
// unpredictable and teaches nothing. Rewarding improvement points it at
// whatever it is currently getting better at.
//
// The model is a linear map from a coarse binning of association-module
// activity to the next mel frame, trained online by least-mean-squares. It is
// a critic, not part of the brain: it never sends a spike anywhere. Its only
// output is one number, and that number is learned from scratch like
// everything else here.

#ifndef AIBABY_CRITIC_H
#define AIBABY_CRITIC_H

#include "aibaby/arena.h"
#include "aibaby/config.h"
#include "aibaby/dna.h"
#include "aibaby/network.h"

namespace aibaby {

class SnapshotReader;
class SnapshotWriter;

class Critic {
 public:
  static size_t required_bytes(const Dna& dna);

  bool build(const Dna& dna, Arena& arena, uint32_t source_module);

  // One sensory frame. `mel` is the frame that has just been heard; the
  // prediction being scored was made from the activity that preceded it.
  void observe(const Network& net, const Scalar* mel, uint32_t count);

  // Learning progress: how much the short-window error has dropped below the
  // long-window one. Clamped at zero — getting worse is not a punishment, it
  // is just the absence of a reward.
  Scalar progress() const;
  Scalar fast_error() const { return fast_err_; }
  Scalar slow_error() const { return slow_err_; }
  bool ready() const { return frames_ > 2; }

  // The prediction the last observe() scored: what the creature expected the
  // frame it has just heard to be, computed from the activity that preceded it
  // and read *before* that frame's correction was applied. Zero until there is
  // a previous frame to have predicted from.
  //
  // This is transient — written and consumed inside one hear() — so it is
  // deliberately not in the snapshot. Nothing carries it across a save.
  const Scalar* prediction() const { return pred_; }

  // Snapshot seam (§8): what observing has changed. The weight matrix and the
  // bias live in the arena and travel with it; everything else here is either
  // learned online — the two error windows are what the growth detector reads —
  // or comes back from the genome at build.
  void save_state(SnapshotWriter& w) const;
  void load_state(SnapshotReader& r);

 private:
  void extract_features(const Network& net);

  Scalar* w_ = nullptr;      // kCriticBins x channels_, row-major by bin
  Scalar* bias_ = nullptr;   // channels_
  Scalar features_[kCriticBins] = {};
  Scalar pred_[kMaxMelChannels] = {};
  // The frame before this one, when the model predicts changes rather than
  // levels. Unlike pred_ this is genuine carried state and is in the snapshot:
  // a creature resumed without it mispredicts its first frame back.
  Scalar prev_[kMaxMelChannels] = {};

  uint32_t channels_ = 0;
  uint32_t source_module_ = 0;
  uint32_t frames_ = 0;
  Scalar learn_rate_ = kZero;
  Scalar fast_alpha_ = kZero;
  Scalar slow_alpha_ = kZero;
  Scalar gain_ = kZero;
  Scalar rate_norm_ = kOne;
  Scalar fast_err_ = kZero;
  Scalar slow_err_ = kZero;
  bool have_features_ = false;
  bool have_prev_ = false;
  bool persistence_ = false;
};

}  // namespace aibaby

#endif  // AIBABY_CRITIC_H
