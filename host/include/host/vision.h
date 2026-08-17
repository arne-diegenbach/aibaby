// The retina: grayscale pixels in, foveated ON/OFF responses out.
//
// Host code for the same reason the cochlea is (§6.2). Sampling a camera frame
// is signal processing that a laptop, a phone and an ESP32 each do
// differently; what the creature is born with is the *shape* of the result —
// a foveated field of centre-surround cells, ON and OFF — and that shape lives
// in the genome.
//
// Everything downstream of a normalised response vector is core code, so the
// spikes a neuron sees are identical on every platform.

#ifndef AIBABY_HOST_VISION_H
#define AIBABY_HOST_VISION_H

#include <cstdint>
#include <string>
#include <vector>

#include "aibaby/dna.h"
#include "aibaby/rng.h"

namespace aibaby_host {

class Retina {
 public:
  bool configure(const aibaby::DnaVision& cfg, std::string& error);

  // One grayscale frame, `frame_size()` square, row-major, 0..255. Produces
  // one response vector; there is no windowing and no history, so a frame in
  // is a frame out.
  void present(const uint8_t* pixels);

  // Normalised [0,1] responses, two per cell: ON then OFF, cell-major.
  // Centre-surround means a uniform field — a blank wall, a dark room —
  // responds with zeros, which is what makes "nothing there" legible.
  const std::vector<float>& features() const { return features_; }

  // Discards the last response. Call when the camera stops, so the panel and
  // the encoder are not left holding the final frame of a stream that ended.
  void reset_stream();

  uint32_t frame_size() const { return frame_size_; }
  uint32_t cells() const { return uint32_t(cells_.size()); }
  uint32_t feature_count() const { return uint32_t(features_.size()); }
  uint64_t frames_produced() const { return frames_produced_; }

  // Mean |response| over the last frame, for the panel's "is anything there"
  // meter. The equivalent of the microphone's peak level.
  float contrast() const { return contrast_; }

  // Where the fovea is pointing, in pixels from the frame centre. The gaze
  // lives in the retina rather than in the caller because pointing an eye is
  // something a body does, and because there are seventeen places that present
  // a frame — a gaze parameter on present() would have been seventeen chances
  // to forget it.
  //
  // DNA v27 also shipped a reflexive controller that aimed this at the salience
  // centroid; DNA v30 deleted it. It landed the eye ~4.4 px from the toy where
  // the visual code needs ~1.4, and a centroid over cells cannot be sharper
  // than the cells, so that was never a tuning distance. What is kept is the
  // *mechanism* — the layout genuinely slides, and `look_at` lets an experiment
  // drive it directly. That is strictly more informative than the controller
  // was: an oracle fovea separates "is foveation worth anything here" from "was
  // my controller any good", which the reflexive version confounded.
  float gaze_x() const { return gaze_x_; }
  float gaze_y() const { return gaze_y_; }
  // DNA v31's did-it-run guard. A controller that never fires and one that
  // fires and lands nowhere produce the same null everywhere else.
  uint64_t gaze_moves() const { return gaze_moves_; }

  // A model of an imperfect eye, so the controller can be asked whether it
  // survives a real motor before there is one to buy.
  //
  // Not in the genome, deliberately. This describes a body the creature does
  // not have yet, and a required TOML key per parameter is the tax the DNA v30
  // deletion policy exists to avoid. It is a host-side testing facility and the
  // default is the ideal eye, which is bit-identical to not having it.
  //
  // The three parameters are the three ways a motor differs from an assignment:
  //
  //   dead_frames  a command takes this long to start acting, AND the position
  //                the controller reads back is this stale. The second half is
  //                the one that bites: an eye that cannot yet see its own
  //                movement keeps commanding more of it.
  //   slew         fraction of the remaining distance covered per frame. 1 is
  //                a teleport; a real actuator is well under that.
  //   noise_px     readback error, because an encoder is not a promise.
  struct Servo {
    uint32_t dead_frames = 0;
    float slew = 1.0f;
    float noise_px = 0.0f;
  };
  void set_servo(const Servo& servo, uint64_t seed);
  // Teleport the eye, servo state and all. Setting only the position was a bug
  // the moment the servo existed: the actuator pulls toward its *command*, so a
  // direct placement was undone on the very next frame and the oracle arm
  // silently became the fixed arm. Anything that positions the eye from outside
  // has to move the command with it.
  void look_at(float x, float y);

  // Where a cell sits in the image, normalised to [0,1], plus its footprint.
  // Only the panel needs this; the core is told a count and nothing more.
  struct CellGeometry {
    float cx, cy, extent;
  };
  CellGeometry geometry(uint32_t cell) const;

 private:
  // One ganglion cell: a difference-of-Gaussians stencil over a clipped box of
  // the image. Precomputed at configure() so a frame costs only multiply-adds.
  struct Cell {
    uint32_t x0 = 0, y0 = 0, w = 0, h = 0;
    float cx = 0.0f, cy = 0.0f, pitch = 0.0f;
    std::vector<float> weight;
  };

  void build_cell(float cx, float cy, float pitch);

  std::vector<Cell> cells_;
  std::vector<float> features_;
  // |response| per cell for the frame just presented. Kept so the gaze
  // controller can take a second pass without re-convolving anything — the
  // convolution is the only loop in this file that costs real time.
  std::vector<float> magnitude_;
  aibaby::DnaVision cfg_{};
  uint32_t frame_size_ = 0;
  float contrast_ = 0.0f;
  uint64_t frames_produced_ = 0;

  // Gaze state: offsets in pixels applied to every cell's sampling window, so
  // the whole retinal layout — fovea and rings together — slides across the
  // image as one. Zero is the frame centre and is the pre-v27 retina exactly,
  // which is where every creature that is not being probed sits.
  float gaze_x_ = 0.0f;
  float gaze_y_ = 0.0f;

  // DNA v31 controller state. `frames_to_aim_` counts down to the next re-aim;
  // 0 period means the controller is off and nothing below is read.
  uint64_t gaze_moves_ = 0;
  uint32_t aim_period_ = 0;
  uint32_t frames_to_aim_ = 0;

  // The imperfect-eye model. `cmd_*` is where the controller asked the eye to
  // go; `gaze_*` above is where it actually is, which is what the sampling
  // uses. Both queues are (dead_frames + 1) long: index 0 is the oldest and
  // therefore the one in force now.
  static constexpr uint32_t kMaxDead = 8;
  Servo servo_{};
  float cmd_x_ = 0.0f, cmd_y_ = 0.0f;
  float cmd_q_x_[kMaxDead + 1] = {}, cmd_q_y_[kMaxDead + 1] = {};
  float pos_q_x_[kMaxDead + 1] = {}, pos_q_y_[kMaxDead + 1] = {};
  uint64_t servo_rng_ = 0;

  void servo_reset();
  void servo_advance();
  // What the controller believes about its own position: stale by dead_frames
  // and wrong by noise_px. With the default servo this is exactly gaze_x_.
  float reported_x();
  float reported_y();
};

// Synthetic scenes, so the headless experiments can show the baby something
// with the same kind of frame the browser will send. Not used by the live
// path — there the camera is the source.
//
// The noise is what makes the test honest: a blank field that is *exactly*
// uniform is a stimulus no camera ever produces, and a retina that only has to
// separate a shape from mathematical nothing has not been asked a real
// question.
class SceneSource {
 public:
  SceneSource(uint32_t frame_size, uint64_t seed);

  enum class Shape { kNone, kDisc, kSquare };

  // Renders into `out`, which must hold frame_size x frame_size bytes.
  void render(Shape shape, float cx, float cy, float radius, float luminance,
              float noise, uint8_t* out);

  uint32_t frame_size() const { return frame_size_; }

 private:
  uint32_t frame_size_;
  aibaby::Rng rng_;
};

}  // namespace aibaby_host

#endif  // AIBABY_HOST_VISION_H
