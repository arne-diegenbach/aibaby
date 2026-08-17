#include "host/vision.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace aibaby_host {
namespace {

inline float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

}  // namespace

bool Retina::configure(const aibaby::DnaVision& cfg, std::string& error) {
  cfg_ = cfg;
  frame_size_ = cfg.frame_size;
  cells_.clear();
  features_.clear();
  contrast_ = 0.0f;
  frames_produced_ = 0;

  if (frame_size_ == 0 || cfg.fovea_grid == 0 || cfg.ring_grid == 0) {
    error = "vision frame and grids must be non-zero";
    return false;
  }
  if (cfg.surround_sigma <= cfg.center_sigma) {
    error = "vision surround_sigma must exceed center_sigma";
    return false;
  }

  // The layout itself lives in the core (aibaby::vision_cell), because since
  // DNA v7 the core wires receptive fields over it and the two descriptions of
  // where a ganglion cell sits must be one description. The sampler's job is
  // now only to turn each of those positions into a difference-of-Gaussians
  // kernel over actual pixels.
  const uint32_t expected = aibaby::vision_cells(cfg);
  const float frame = float(frame_size_);
  for (uint32_t c = 0; c < expected; ++c) {
    const aibaby::VisionCell g = aibaby::vision_cell(cfg, c);
    if (g.pitch <= 0.0f) {
      error = "retina cell " + std::to_string(c) + " has no geometry";
      return false;
    }
    build_cell(g.u * frame, g.v * frame, g.pitch * frame);
  }

  if (cells_.size() != expected) {
    // The genome validator sizes the vision module from vision_cells(); if the
    // sampler ever laid out a different number the brain would be reading cells
    // that do not exist. Better to refuse to hatch than to see wrong.
    error = "retina laid out " + std::to_string(cells_.size()) + " cells but the genome expects " +
            std::to_string(expected);
    return false;
  }

  features_.assign(cells_.size() * 2, 0.0f);
  magnitude_.assign(cells_.size(), 0.0f);

  gaze_x_ = 0.0f;
  gaze_y_ = 0.0f;
  gaze_moves_ = 0;
  cmd_seq_ = 0;
  eye_reports_ = 0;
  eye_stalls_ = 0;
  last_report_frame_ = 0;
  last_lag_seq_ = 0;
  eye_lag_frames_ = 0;
  for (uint32_t i = 0; i < kSeqRing; ++i) seq_frame_[i] = 0;
  servo_reset();
  // Re-aim timing is in *frames*, because the retina only exists on frame
  // boundaries: a rate in Hz has to be reconciled with frame_hz or the eye
  // would appear to move between two identical images.
  if (cfg.gaze_rate_hz > 0.0f && cfg.frame_hz > 0.0f) {
    const float per = cfg.frame_hz / cfg.gaze_rate_hz;
    aim_period_ = per < 1.0f ? 1u : uint32_t(per + 0.5f);
  } else {
    aim_period_ = 0;
  }
  frames_to_aim_ = aim_period_;
  return true;
}

// One difference-of-Gaussians cell. Centre and surround are each normalised to
// sum to one *after* clipping to the frame, so a uniform field responds with
// exactly zero everywhere — including in the cells that hang over the edge.
// Without that, the border cells would report a permanent phantom edge.
void Retina::build_cell(float cx, float cy, float pitch) {
  Cell cell;
  cell.cx = cx;
  cell.cy = cy;
  cell.pitch = pitch;

  const float sigma_c = cfg_.center_sigma * pitch;
  const float sigma_s = cfg_.surround_sigma * pitch;
  const int radius = int(std::ceil(3.0f * sigma_s));
  const int lo_x = std::max(0, int(std::floor(cx)) - radius);
  const int hi_x = std::min(int(frame_size_), int(std::floor(cx)) + radius + 1);
  const int lo_y = std::max(0, int(std::floor(cy)) - radius);
  const int hi_y = std::min(int(frame_size_), int(std::floor(cy)) + radius + 1);

  cell.x0 = uint32_t(lo_x);
  cell.y0 = uint32_t(lo_y);
  cell.w = uint32_t(hi_x - lo_x);
  cell.h = uint32_t(hi_y - lo_y);
  cell.weight.assign(size_t(cell.w) * cell.h, 0.0f);

  std::vector<float> gc(cell.weight.size(), 0.0f);
  std::vector<float> gs(cell.weight.size(), 0.0f);
  float sum_c = 0.0f, sum_s = 0.0f;
  const float inv_c = 1.0f / (2.0f * sigma_c * sigma_c);
  const float inv_s = 1.0f / (2.0f * sigma_s * sigma_s);

  for (uint32_t y = 0; y < cell.h; ++y) {
    for (uint32_t x = 0; x < cell.w; ++x) {
      const float dx = float(cell.x0 + x) + 0.5f - cx;
      const float dy = float(cell.y0 + y) + 0.5f - cy;
      const float d2 = dx * dx + dy * dy;
      const size_t k = size_t(y) * cell.w + x;
      gc[k] = std::exp(-d2 * inv_c);
      gs[k] = std::exp(-d2 * inv_s);
      sum_c += gc[k];
      sum_s += gs[k];
    }
  }
  const float norm_c = sum_c > 0.0f ? 1.0f / sum_c : 0.0f;
  const float norm_s = sum_s > 0.0f ? 1.0f / sum_s : 0.0f;
  for (size_t k = 0; k < cell.weight.size(); ++k) {
    cell.weight[k] = gc[k] * norm_c - gs[k] * norm_s;
  }

  cells_.push_back(std::move(cell));
}

void Retina::look_at(float x, float y) {
  if (mount_ == EyeMount::kExternal) {
    // Nothing to teleport: the eye is somebody else's hardware and the only
    // thing this side can do is ask. The position stays whatever the device
    // last reported, which is the truth rather than a wish.
    issue_command(x, y);
    return;
  }
  // Command first, then teleport onto it. The other order looks equivalent and
  // is not: `servo_reset` assigns the command from the position, so by the time
  // `issue_command` ran it had nothing left to change and the sequence number
  // never advanced. An instruction that does not advance the seq is one a
  // device polling for new commands cannot see.
  issue_command(x, y);
  gaze_x_ = cmd_x_;
  gaze_y_ = cmd_y_;
  servo_reset();
}

// --- The eye port ----------------------------------------------------------

void Retina::issue_command(float x, float y) {
  const float mid = 0.5f * float(frame_size_);
  const float cx = x < -mid ? -mid : (x > mid ? mid : x);
  const float cy = y < -mid ? -mid : (y > mid ? mid : y);
  // Only a command that commands something new gets a sequence number, so a
  // device echoing the seq it is acting on is echoing a distinct instruction
  // rather than a frame counter. A controller sitting still holds its seq and
  // the measured lag stays the lag of the last real move.
  if (cx == cmd_x_ && cy == cmd_y_ && cmd_seq_ != 0) return;
  cmd_x_ = cx;
  cmd_y_ = cy;
  ++cmd_seq_;
  seq_frame_[cmd_seq_ % kSeqRing] = frames_produced_;
}

void Retina::set_eye_mount(EyeMount mount) {
  if (mount == mount_) return;
  mount_ = mount;
  // Hand over at the position the eye is at rather than at zero. Switching a
  // live creature to a device should not make it look away from whatever it
  // was watching; the device's first report will correct any disagreement.
  cmd_x_ = gaze_x_;
  cmd_y_ = gaze_y_;
  servo_reset();
  eye_lag_frames_ = 0;
  last_lag_seq_ = cmd_seq_;
  last_report_frame_ = frames_produced_;
}

Retina::GazeCommand Retina::gaze_command() const {
  GazeCommand out;
  out.x_px = cmd_x_;
  out.y_px = cmd_y_;
  const float inv = frame_size_ ? 1.0f / float(frame_size_) : 0.0f;
  out.x_frac = cmd_x_ * inv;
  out.y_frac = cmd_y_ * inv;
  out.seq = cmd_seq_;
  return out;
}

void Retina::report_gaze(const GazeReport& report) {
  // Ignored on an internal eye, and deliberately not counted either: a client
  // that reports without switching the mount over sees `eye_reports()` stay at
  // zero, which says what is wrong. Accepting the report and quietly losing the
  // fight with the servo model would not.
  if (mount_ != EyeMount::kExternal) return;
  const float mid = 0.5f * float(frame_size_);
  gaze_x_ = report.x_px < -mid ? -mid : (report.x_px > mid ? mid : report.x_px);
  gaze_y_ = report.y_px < -mid ? -mid : (report.y_px > mid ? mid : report.y_px);
  ++eye_reports_;
  last_report_frame_ = frames_produced_;
  // Only a seq that has not been acknowledged yet measures anything. A device
  // that keeps echoing the command it is already sitting on is not being slow —
  // it is being still, and treating that as lag makes the number climb without
  // bound while the eye is parked on its target. The last real round trip
  // stands until there is a newer one, which is what a caller wants to read
  // when they ask how quick their own loop is.
  if (report.seq > last_lag_seq_ && report.seq <= cmd_seq_ &&
      cmd_seq_ - report.seq < kSeqRing) {
    const uint64_t issued = seq_frame_[report.seq % kSeqRing];
    if (frames_produced_ >= issued) {
      eye_lag_frames_ = uint32_t(frames_produced_ - issued);
    }
    last_lag_seq_ = report.seq;
  }
}

void Retina::set_eye_timeout_frames(uint32_t frames) {
  eye_timeout_frames_ = frames ? frames : 1;
}

bool Retina::eye_stale() const {
  if (mount_ != EyeMount::kExternal) return false;
  // A device gets the same grace period to say hello as it gets to answer, so
  // attaching one a few frames before it boots is not an immediate fault.
  return frames_produced_ - last_report_frame_ > uint64_t(eye_timeout_frames_);
}

void Retina::set_servo(const Servo& servo, uint64_t seed) {
  servo_ = servo;
  if (servo_.dead_frames > kMaxDead) servo_.dead_frames = kMaxDead;
  if (servo_.slew <= 0.0f || servo_.slew > 1.0f) servo_.slew = 1.0f;
  // Seeded rather than free-running: a noisy eye still has to give the same
  // creature twice, or G1 is gone and every arm using it is unrepeatable.
  servo_rng_ = seed ? seed : 1ull;
  servo_reset();
}

void Retina::servo_reset() {
  cmd_x_ = gaze_x_;
  cmd_y_ = gaze_y_;
  for (uint32_t i = 0; i <= kMaxDead; ++i) {
    cmd_q_x_[i] = gaze_x_;
    cmd_q_y_[i] = gaze_y_;
    pos_q_x_[i] = gaze_x_;
    pos_q_y_[i] = gaze_y_;
  }
}

// One frame of actuator. Shifts both queues by one, so the command issued
// dead_frames ago becomes the target now and the position from dead_frames ago
// becomes what the controller can see.
void Retina::servo_advance() {
  const uint32_t n = servo_.dead_frames;
  // Append the newest command, THEN read the oldest slot. Reading first would
  // put a frame of delay in even at dead_frames 0, which is not an ideal eye
  // and is not the pre-servo creature — the determinism hash caught exactly
  // that and it is the reason this order is spelled out.
  for (uint32_t i = 0; i < n; ++i) {
    cmd_q_x_[i] = cmd_q_x_[i + 1];
    cmd_q_y_[i] = cmd_q_y_[i + 1];
  }
  cmd_q_x_[n] = cmd_x_;
  cmd_q_y_[n] = cmd_y_;

  const float mid = 0.5f * float(frame_size_);
  gaze_x_ += servo_.slew * (cmd_q_x_[0] - gaze_x_);
  gaze_y_ += servo_.slew * (cmd_q_y_[0] - gaze_y_);
  gaze_x_ = std::max(-mid, std::min(mid, gaze_x_));
  gaze_y_ = std::max(-mid, std::min(mid, gaze_y_));

  // Same order for the readback: at dead_frames 0 the controller sees where it
  // actually is, which is what it saw before there was a servo at all.
  for (uint32_t i = 0; i < n; ++i) {
    pos_q_x_[i] = pos_q_x_[i + 1];
    pos_q_y_[i] = pos_q_y_[i + 1];
  }
  pos_q_x_[n] = gaze_x_;
  pos_q_y_[n] = gaze_y_;
}

// xorshift64*, so a noisy readback is still reproducible.
static float servo_jitter(uint64_t& state, float scale) {
  if (scale <= 0.0f) return 0.0f;
  state ^= state >> 12;
  state ^= state << 25;
  state ^= state >> 27;
  const uint64_t v = state * 2685821657736338717ull;
  // [-1, 1) from the top bits, then scaled.
  return scale * (float(double(v >> 40) / 8388608.0) - 1.0f);
}

float Retina::reported_x() {
  // An external eye brings its own staleness and its own encoder error, so
  // there is nothing to simulate: the last report IS the belief, and it is the
  // only thing the controller has.
  if (mount_ == EyeMount::kExternal) return gaze_x_;
  return pos_q_x_[0] + servo_jitter(servo_rng_, servo_.noise_px);
}

float Retina::reported_y() {
  if (mount_ == EyeMount::kExternal) return gaze_y_;
  return pos_q_y_[0] + servo_jitter(servo_rng_, servo_.noise_px);
}

void Retina::present(const uint8_t* pixels) {
  if (cells_.empty()) return;
  // The eye moves before the frame is taken, so a command issued at the end of
  // the previous frame is acting by the time this one is sampled — which is
  // what a motor does and what an assignment does not. An external eye has
  // already moved, in its own time, and said so through `report_gaze`.
  if (mount_ == EyeMount::kInternal) servo_advance();
  const float inv_gain = 1.0f / cfg_.contrast_gain;
  float total = 0.0f;

  // The whole retinal layout slides by the gaze offset, fovea and rings as one
  // piece — which is what an eye movement is. Rounded to whole pixels: a gaze
  // shift is a jump of many pixels and sub-pixel interpolation would cost a
  // resample of every stencil to buy nothing.
  //
  // On an external eye it does not slide at all. The head already turned, or an
  // upstream cropper already moved its window, so the frame in hand is the
  // aimed view and sliding over it would apply the aim a second time.
  const float sample_x = mount_ == EyeMount::kInternal ? gaze_x_ : 0.0f;
  const float sample_y = mount_ == EyeMount::kInternal ? gaze_y_ : 0.0f;
  const int gx = int(sample_x + (sample_x >= 0.0f ? 0.5f : -0.5f));
  const int gy = int(sample_y + (sample_y >= 0.0f ? 0.5f : -0.5f));

  // DNA v31: where the strongest response was, in the retina's own layout
  // coordinates. Tracked while the responses are being computed because it is
  // one comparison per cell and a second pass would double the only loop in
  // this file that costs anything.
  float peak = 0.0f, peak_x = 0.0f, peak_y = 0.0f;

  for (size_t c = 0; c < cells_.size(); ++c) {
    const Cell& cell = cells_[c];
    float response = 0.0f;
    for (uint32_t y = 0; y < cell.h; ++y) {
      // Out-of-frame samples are edge-EXTENDED, not skipped, and the difference
      // is a bug this cost a measurement to find. build_cell() normalises the
      // centre and surround Gaussians to sum to one over the pixels the cell
      // covers *at its layout position*, which is what makes a uniform field
      // read exactly zero. Dropping samples once the gaze shifts the window
      // breaks that sum, so a partly-off-image cell reports a large phantom
      // edge — the frame border becomes the brightest thing in the salience
      // map and the eye is driven straight into it. Measured: gaze ended 13.2
      // px from the toy when doing nothing at all would have left it at 4.9.
      // Clamping the coordinate keeps every weight applied to some pixel, so
      // the sum is preserved and a uniform field still cancels.
      const int sy = std::min(int(frame_size_) - 1, std::max(0, int(cell.y0 + y) + gy));
      const float* w = &cell.weight[size_t(y) * cell.w];
      for (uint32_t x = 0; x < cell.w; ++x) {
        const int sx = std::min(int(frame_size_) - 1, std::max(0, int(cell.x0 + x) + gx));
        response += w[x] * float(pixels[size_t(sy) * frame_size_ + size_t(sx)]);
      }
    }
    response *= (1.0f / 255.0f) * inv_gain;

    // ON and OFF are separate cells, as in a real retina: one reports light on
    // a dark surround, the other dark on light. Splitting a signed response
    // into two non-negative channels is also what lets a spike code carry it —
    // a neuron cannot fire a negative number of times.
    features_[c * 2] = clamp01(response);
    features_[c * 2 + 1] = clamp01(-response);
    const float mag = std::fabs(response);
    magnitude_[c] = mag;
    total += mag;
    if (mag > peak) {
      peak = mag;
      peak_x = cell.cx;
      peak_y = cell.cy;
    }
  }

  contrast_ = float(total / double(cells_.size()));
  ++frames_produced_;

  if (aim_period_ == 0) return;
  if (frames_to_aim_ > 0) --frames_to_aim_;
  if (frames_to_aim_ != 0) return;
  frames_to_aim_ = aim_period_;

  // The eye stopped answering. Freeze rather than keep correcting: this loop
  // has no model of its own motion and steers from where it *believes* it is,
  // so commanding into silence is the exact condition the servo sweep measured
  // as divergence — one frame of stale readback with a fast actuator put the
  // eye 26 px out on a 64 px frame. A device that has gone quiet is infinitely
  // stale readback, and the safe response to that is to stop asking.
  if (eye_stale()) {
    ++eye_stalls_;
    return;
  }

  // Nothing in view is not a reason to move. A creature that re-aims at the
  // peak of pure noise spends its life chasing grain, and `contrast_floor` is
  // the genome's own statement of what counts as seeing something.
  if (peak <= 0.0f || contrast_ <= cfg_.contrast_floor) return;

  // Centroid over the peak's neighbourhood: every cell responding at least
  // `gaze_peak_frac` of the peak. A DoG cell peaks on an *edge*, so the single
  // strongest cell sits one object-radius off centre and the eye parks there
  // forever; averaging the cells that surround the object puts the estimate
  // back in its middle. At frac 1.0 only the peak qualifies and this is the
  // pure-peak rule; at 0.0 every cell qualifies and it is v27's whole-field
  // centroid.
  float aim_x = peak_x, aim_y = peak_y;
  {
    const float bar = cfg_.gaze_peak_frac * peak;
    double wx = 0.0, wy = 0.0, ws = 0.0;
    for (size_t c = 0; c < cells_.size(); ++c) {
      const float mag = magnitude_[c];
      if (mag < bar) continue;
      wx += double(mag) * double(cells_[c].cx);
      wy += double(mag) * double(cells_[c].cy);
      ws += double(mag);
    }
    if (ws > 0.0) {
      aim_x = float(wx / ws);
      aim_y = float(wy / ws);
    }
  }

  // The aim point is an image position sampled through the current gaze, so
  // putting the fovea on it means moving the gaze by its offset from the layout
  // centre. The gaze already carries whatever offset produced this view, so the
  // correction is relative and iterating converges.
  // The correction is relative to where the controller *believes* it is, which
  // with an ideal eye is where it is. That belief is the whole reason a servo
  // can destabilise this loop: an eye that cannot yet see its own movement
  // keeps asking for more of it.
  const float mid = 0.5f * float(frame_size_);
  const float from_x = reported_x(), from_y = reported_y();
  const float nx = from_x + cfg_.gaze_gain * (aim_x - mid);
  const float ny = from_y + cfg_.gaze_gain * (aim_y - mid);
  // Only count a move that moved. Below a pixel the sampling offset rounds to
  // the same integer, so the eye has not been anywhere and saying it has would
  // make the guard below unable to fail.
  if (std::fabs(nx - cmd_x_) < 1.0f && std::fabs(ny - cmd_y_) < 1.0f) return;
  issue_command(nx, ny);
  ++gaze_moves_;
}

void Retina::reset_stream() {
  for (float& f : features_) f = 0.0f;
  contrast_ = 0.0f;
  // The gaze goes home with the stream. Resuming a camera with the eye still
  // pointed where the last frame of the previous stream happened to be is a
  // creature looking at nothing.
  //
  // On an external eye, home is a request. The command goes to centre and the
  // position stays wherever the device last said it was, because this side does
  // not get to decide where somebody else's motor is pointing — and reporting a
  // position the hardware never took would be the more comfortable lie.
  if (mount_ == EyeMount::kInternal) {
    gaze_x_ = 0.0f;
    gaze_y_ = 0.0f;
    servo_reset();
  }
  issue_command(0.0f, 0.0f);
  frames_to_aim_ = aim_period_;
}

Retina::CellGeometry Retina::geometry(uint32_t cell) const {
  if (cell >= cells_.size() || frame_size_ == 0) return CellGeometry{0.0f, 0.0f, 0.0f};
  const float inv = 1.0f / float(frame_size_);
  return CellGeometry{cells_[cell].cx * inv, cells_[cell].cy * inv,
                      cells_[cell].pitch * inv};
}

// --- Synthetic scenes ------------------------------------------------------

SceneSource::SceneSource(uint32_t frame_size, uint64_t seed) : frame_size_(frame_size) {
  rng_.seed(seed);
}

void SceneSource::render(Shape shape, float cx, float cy, float radius, float luminance,
                         float noise, uint8_t* out) {
  const float n = float(frame_size_);
  const float px = cx * n, py = cy * n, pr = radius * n;
  const float background = 0.45f;

  for (uint32_t y = 0; y < frame_size_; ++y) {
    for (uint32_t x = 0; x < frame_size_; ++x) {
      const float dx = float(x) + 0.5f - px;
      const float dy = float(y) + 0.5f - py;
      bool inside = false;
      if (shape == Shape::kDisc) inside = dx * dx + dy * dy <= pr * pr;
      else if (shape == Shape::kSquare) inside = std::fabs(dx) <= pr && std::fabs(dy) <= pr;

      float v = inside ? luminance : background;
      v += noise * float(rng_.signed_uniform());
      out[size_t(y) * frame_size_ + x] = uint8_t(clamp01(v) * 255.0f + 0.5f);
    }
  }
}

}  // namespace aibaby_host
