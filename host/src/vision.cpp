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

  gaze_x_ = 0.0f;
  gaze_y_ = 0.0f;
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
    total += std::fabs(response);
  }

  contrast_ = float(total / double(cells_.size()));
  ++frames_produced_;
}

void Retina::reset_stream() {
  for (float& f : features_) f = 0.0f;
  contrast_ = 0.0f;
  // The gaze goes home with the stream. Resuming a camera with the eye still
  // pointed where the last frame of the previous stream happened to be is a
  // creature looking at nothing.
  gaze_x_ = 0.0f;
  gaze_y_ = 0.0f;
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
