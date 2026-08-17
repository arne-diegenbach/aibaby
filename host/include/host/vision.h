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
  //
  // Applies to an internal eye only. On an external one the device *is* the
  // servo and brings its own lag, slew and encoder error, so there is nothing
  // here left to simulate.
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
  //
  // On an external eye there is nothing to teleport — you cannot assign a motor
  // a position — so this issues the command and leaves the position to whatever
  // the device reports. Use it for an attention system that wants to override
  // the reflex, not as a way to know where the eye is.
  void look_at(float x, float y);

  // --- The eye port ---------------------------------------------------------
  //
  // Everything above assumes the eye is a crop window this class slides over a
  // fixed camera. A real eye is a device somebody else owns: it accepts a
  // command, moves at its own pace, and reports back — and it may be a pan/tilt
  // head, a browser cropping a larger frame before it sends one, or a robot arm
  // holding a phone. The port is the seam between the two halves that were
  // never really one thing:
  //
  //   the controller  — where to look. Stays here: it is the creature's.
  //   the actuator    — how the eye gets there. Leaves: it is the body's.
  //
  // The port is data in and data out, with no callback and no ownership. Read
  // `gaze_command()` after each frame, drive whatever you have with it, and
  // hand back a `GazeReport` when the device says where it ended up. That is
  // the whole interface, and it is the same shape as `present()`/`features()`
  // for the same reason: an integrator can poll it from a loop they already
  // have, and nothing here can call into their code at a moment they did not
  // choose.
  enum class EyeMount {
    // This retina aims, by sliding its sampling window over the frame. The
    // camera is fixed and wide; the `Servo` model above applies. Default, and
    // what every experiment and the browser use today.
    kInternal,
    // Something outside aims. Frames arrive ALREADY AIMED — the head turned, or
    // an upstream cropper moved its window — so the sampling window must NOT
    // slide as well.
    //
    // Getting this wrong applies the aim twice, and `gazeprobe`'s `doubled`
    // arms say what that costs: nothing you can see. At gain 0.70 the doubled
    // loop is 1.4, inside the stability bound of 2, so it converges on the
    // target and scores like a working eye — 1.3 px from the toy while the host
    // reports 2.6 px. On a slow motor it is not punished either, because the
    // slew was already halving the loop. The only symptom is that the two ends
    // disagree about where the eye is, which is why this is an enum the caller
    // must pick rather than a note in a comment.
    kExternal,
  };

  // What the controller wants, published after every frame. Two units because
  // the device's are not the retina's: pixels are what this class thinks in,
  // and a fraction of the frame is what survives contact with a real lens —
  // multiply by the field of view to get degrees and the integrator never has
  // to know the retina's resolution.
  struct GazeCommand {
    float x_px = 0.0f, y_px = 0.0f;      // from frame centre, +x right, +y down
    float x_frac = 0.0f, y_frac = 0.0f;  // the same, as a fraction of the frame
    uint64_t seq = 0;                    // bumps on change; 0 = never commanded
  };

  // What the device says came back. `seq` is the command this position reflects
  // — echo the `seq` you were acting on and the retina will tell you your own
  // loop's dead time in frames, which is the one servo parameter measured to be
  // dangerous. Echo 0 if the device cannot say, and the lag reads unknown.
  struct GazeReport {
    float x_px = 0.0f, y_px = 0.0f;
    uint64_t seq = 0;
  };

  void set_eye_mount(EyeMount mount);
  EyeMount eye_mount() const { return mount_; }
  GazeCommand gaze_command() const;
  void report_gaze(const GazeReport& report);

  // How many frames of silence before the eye counts as gone. Counted in
  // FRAMES, not wall-clock: with the camera stopped the controller is not
  // acting, so there is nothing for the device to be late for.
  //
  // While it is stale the controller stops issuing commands. That was built
  // from the servo sweep's divergence and it is worth less than it looks:
  // measured, it saves 0.3 px of command drift, because this loop is
  // proportional with no integrator and so cannot wind up against a frozen
  // belief however long the device is gone. It is kept as an interface
  // guarantee — a predictable response to a dead device, and an explicit
  // `eye_stalls()` counter — rather than as a protection. The runaway it looks
  // like it prevents belongs to a device that answers LATE, which is `Servo`
  // above, at 26 px.
  void set_eye_timeout_frames(uint32_t frames);
  bool eye_stale() const;
  // Measured from the seq round-trip, 0 if the device does not echo one.
  uint32_t eye_lag_frames() const { return eye_lag_frames_; }
  uint64_t eye_reports() const { return eye_reports_; }
  // Re-aims suppressed because the eye had gone quiet. The did-it-run guard
  // for the freeze: a device that never reports and a controller that never
  // wanted to move look identical in `gaze_moves()`.
  uint64_t eye_stalls() const { return eye_stalls_; }

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
  // On an external eye it is the last report, because a real controller has no
  // other source of truth about where its own eye is.
  float reported_x();
  float reported_y();

  // Eye port state. `cmd_seq_` is bumped by every command that changes
  // anything, and `seq_frame_` remembers when each was issued so an echoed seq
  // becomes a lag in frames. The ring is a power of two and fixed-size: a
  // device that echoes something older than kSeqRing commands ago is not one
  // whose lag anybody needs to the frame.
  static constexpr uint32_t kSeqRing = 16;
  EyeMount mount_ = EyeMount::kInternal;
  uint64_t cmd_seq_ = 0;
  uint64_t seq_frame_[kSeqRing] = {};
  uint64_t eye_reports_ = 0;
  uint64_t eye_stalls_ = 0;
  uint64_t last_report_frame_ = 0;
  uint64_t last_lag_seq_ = 0;
  uint32_t eye_timeout_frames_ = 5;
  uint32_t eye_lag_frames_ = 0;

  void issue_command(float x, float y);
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
