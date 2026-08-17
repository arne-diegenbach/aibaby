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

void Retina::present(const uint8_t* pixels) {
  if (cells_.empty()) return;
  const float inv_gain = 1.0f / cfg_.contrast_gain;
  float total = 0.0f;

  // The whole retinal layout slides by the gaze offset, fovea and rings as one
  // piece — which is what an eye movement is. Rounded to whole pixels: a gaze
  // shift is a jump of many pixels and sub-pixel interpolation would cost a
  // resample of every stencil to buy nothing.
  const int gx = int(gaze_x_ + (gaze_x_ >= 0.0f ? 0.5f : -0.5f));
  const int gy = int(gaze_y_ + (gaze_y_ >= 0.0f ? 0.5f : -0.5f));

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
  const float mid = 0.5f * float(frame_size_);
  const float nx = gaze_x_ + cfg_.gaze_gain * (aim_x - mid);
  const float ny = gaze_y_ + cfg_.gaze_gain * (aim_y - mid);
  // Only count a move that moved. Below a pixel the sampling offset rounds to
  // the same integer, so the eye has not been anywhere and saying it has would
  // make the guard below unable to fail.
  if (std::fabs(nx - gaze_x_) < 1.0f && std::fabs(ny - gaze_y_) < 1.0f) return;
  gaze_x_ = std::max(-mid, std::min(mid, nx));
  gaze_y_ = std::max(-mid, std::min(mid, ny));
  ++gaze_moves_;
}

void Retina::reset_stream() {
  for (float& f : features_) f = 0.0f;
  contrast_ = 0.0f;
  // The gaze goes home with the stream. Resuming a camera with the eye still
  // pointed where the last frame of the previous stream happened to be is a
  // creature looking at nothing.
  gaze_x_ = 0.0f;
  gaze_y_ = 0.0f;
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
