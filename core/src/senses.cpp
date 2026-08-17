#include "aibaby/senses.h"

namespace aibaby {
namespace {

inline Scalar clampf(Scalar v, Scalar lo, Scalar hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

inline Scalar lerp(Scalar lo, Scalar hi, Scalar t) { return lo + (hi - lo) * t; }

// slice_begin() moved to aibaby/dna.h in DNA v7: a structured projection has
// to be able to ask which retinal cell a source neuron carries, which is this
// same channel map read backwards, and the two directions must not be able to
// drift apart.

// Population-vector readout of one slice: the mean preferred value weighted by
// firing rate. This is what makes *which* neuron fires meaningful, rather than
// only how many — and it is the property M3 needs, because two objects have to
// be able to drive the same group to different values.
struct GroupReading {
  Scalar value;     // [0,1], the population vector
  Scalar activity;  // [0,1], mean rate against the normalising rate
};

GroupReading read_group(const Network& net, uint32_t begin, uint32_t end,
                        Scalar rate_norm, Scalar fallback) {
  GroupReading out{fallback, kZero};
  if (end <= begin) return out;
  const Scalar n = Scalar(end - begin);
  Scalar weighted = kZero;
  Scalar total = kZero;
  for (uint32_t i = begin; i < end; ++i) {
    const Scalar r = net.rate_fast(i);
    const Scalar preferred = (Scalar(i - begin) + Scalar(0.5)) / n;
    weighted += r * preferred;
    total += r;
  }
  // A silent group has no opinion; holding the previous value is better than
  // snapping the vocal tract to zero every time the module goes quiet.
  if (total > Scalar(1e-6)) out.value = clampf(weighted / total, kZero, kOne);
  out.activity = clampf((total / n) / rate_norm, kZero, kOne);
  return out;
}

// Reading this against each neuron's homeostatic setpoint instead of its
// absolute rate was tried on 2026-08-11 and reverted: it changed nothing
// measurable (echo 0.808 vs 0.812, M2 96%/80% vs 95%/82%, F1 histogram
// identical). Worth knowing why, because the reasoning for it was sound and
// the diagnosis it came from still stands.
//
// The diagnosis: over 12000 motor frames the F1 centroid never leaves the
// middle fifth of its range — histogram `0 0 0 0 4298 7702 0 0 0 0` — and
// loudness sits in a hump at 0.5 because it reads mean rate over rate_norm
// while §3.1 pins that mean rate at the genome's setpoint.
//
// Why the fix does not work: a centroid over `preferred` values weighted by
// *anything* returns the centre of the group when the group is undifferentiated,
// and nothing differentiates it. Each neuron's preferred value is its index — a
// label with no wiring behind it — so an input pattern excites a random subset
// and the centroid averages back to 0.5 whatever the weights are read against.
// Deviation coding cannot recover structure the group never had.
//
// What that argues for is upstream: either topography, so a neuron's index
// means something to the projection that drives it, or — cheaper and far more
// likely the real gap — closing the loop from the larynx to the ears, so
// babbling has a sensory consequence the creature can learn its own
// articulatory map from. See the README.

}  // namespace

// --- Auditory --------------------------------------------------------------

void AuditoryEncoder::configure(const Dna& dna, uint32_t module_index) {
  const DnaAudio& a = dna.header().audio;
  module_index_ = module_index;
  channels_ = a.mel_channels;
  gain_ = Scalar(a.gain);

  const Scalar dt = Scalar(dna.header().sim.dt_ms);
  const Scalar frame_ms = Scalar(1000) * Scalar(a.hop) / Scalar(a.sample_rate);
  hold_ticks_ = uint32_t(frame_ms / dt + Scalar(0.5));
  if (hold_ticks_ == 0) hold_ticks_ = 1;
  hold_left_ = 0;
  // Fade to roughly nothing over three frames once the stream stops.
  fade_ = Scalar(0.9);
  active_ = false;
  for (uint32_t c = 0; c < kMaxMelChannels; ++c) level_[c] = kZero;
}

void AuditoryEncoder::present(const Scalar* mel, uint32_t count) {
  const uint32_t n = count < channels_ ? count : channels_;
  for (uint32_t c = 0; c < n; ++c) level_[c] = clampf(mel[c], kZero, kOne);
  for (uint32_t c = n; c < channels_; ++c) level_[c] = kZero;
  hold_left_ = hold_ticks_;
  active_ = true;
}

void AuditoryEncoder::drive(Network& net, bool gate) {
  if (!active_ || channels_ == 0) return;
  if (hold_left_ > 0 && gate) {
    --hold_left_;
  } else {
    for (uint32_t c = 0; c < channels_; ++c) level_[c] *= fade_;
  }
  if (!gate) return;

  const ModuleState& ms = net.module(module_index_);
  if (ms.count == 0) return;

  for (uint32_t c = 0; c < channels_; ++c) {
    const Scalar energy = level_[c];
    if (energy <= kZero) continue;
    const uint32_t begin = ms.begin + slice_begin(ms.count, channels_, c);
    const uint32_t end = ms.begin + slice_begin(ms.count, channels_, c + 1);
    const Scalar n = Scalar(end - begin);
    if (end <= begin) continue;
    for (uint32_t i = begin; i < end; ++i) {
      // Each neuron in the channel needs a different amount of energy before
      // it responds, so loudness recruits population rather than only rate.
      const Scalar sensitivity = (Scalar(i - begin) + Scalar(0.5)) / n;
      const Scalar drive = energy - sensitivity;
      if (drive > kZero) net.inject(i, gain_ * drive);
    }
  }
}

// --- Vision ----------------------------------------------------------------

void VisionEncoder::configure(const Dna& dna, uint32_t module_index) {
  const DnaVision& v = dna.header().vision;
  module_index_ = module_index;
  features_ = vision_features(v);
  if (features_ > kMaxVisionFeatures) features_ = kMaxVisionFeatures;
  gain_ = Scalar(v.gain);
  floor_ = Scalar(v.contrast_floor);

  const Scalar dt = Scalar(dna.header().sim.dt_ms);
  const Scalar frame_ms = Scalar(1000) / Scalar(v.frame_hz);
  hold_ticks_ = uint32_t(frame_ms / dt + Scalar(0.5));
  if (hold_ticks_ == 0) hold_ticks_ = 1;
  latency_ticks_ = uint32_t(Scalar(v.latency_ms) / dt + Scalar(0.5));
  if (latency_ticks_ == 0) latency_ticks_ = 1;
  // The genome already forbids a latency window longer than a frame; clamping
  // here as well means a rounding step cannot put a spike into the next frame.
  if (latency_ticks_ >= hold_ticks_) latency_ticks_ = hold_ticks_ - 1;

  hold_left_ = 0;
  phase_ = 0;
  // Scaled to the frame period rather than fixed, so "fades over about three
  // missed frames" means the same thing at any frame rate. The cochlea's 0.9
  // per tick is calibrated for frames arriving every sixteen ticks; at a
  // hundred ticks apart it would collapse to nothing in the gap between one
  // frame and the next, and the eyes would read as shut every time the browser
  // was a few milliseconds late.
  fade_ = hold_ticks_ > 1 ? kOne - kOne / Scalar(hold_ticks_) : kZero;
  active_ = false;
  for (uint32_t f = 0; f < kMaxVisionFeatures; ++f) {
    level_[f] = kZero;
    fire_at_[f] = kSilent;
  }
}

void VisionEncoder::present(const Scalar* features, uint32_t count) {
  const uint32_t n = count < features_ ? count : features_;
  for (uint32_t f = 0; f < n; ++f) level_[f] = clampf(features[f], kZero, kOne);
  for (uint32_t f = n; f < features_; ++f) level_[f] = kZero;

  // Schedule the volley: full contrast fires on the tick the frame lands,
  // threshold contrast fires a whole latency window later, and anything below
  // the floor does not fire at all.
  for (uint32_t f = 0; f < features_; ++f) {
    if (level_[f] <= floor_) {
      fire_at_[f] = kSilent;
      continue;
    }
    const Scalar t = (kOne - level_[f]) * Scalar(latency_ticks_);
    fire_at_[f] = uint16_t(t + Scalar(0.5));
  }
  hold_left_ = hold_ticks_;
  phase_ = 0;
  active_ = true;
}

void VisionEncoder::drive(Network& net, bool gate) {
  if (!active_ || features_ == 0) return;

  // Eyes shut (§3.6). The volley in flight is dropped rather than held, so a
  // frame that arrived just before the baby fell asleep is not delivered on
  // waking; the responses keep fading, so anything reading level() shows what
  // B3 is receiving now instead of a souvenir of the last thing seen.
  if (!gate) {
    for (uint32_t f = 0; f < features_; ++f) level_[f] *= fade_;
    hold_left_ = 0;
    return;
  }
  // Between frames — the camera is off, or simply slower than the simulation.
  if (hold_left_ == 0) {
    for (uint32_t f = 0; f < features_; ++f) level_[f] *= fade_;
    return;
  }
  --hold_left_;

  const ModuleState& ms = net.module(module_index_);
  if (ms.count == 0) return;

  if (phase_ <= latency_ticks_) {
    for (uint32_t f = 0; f < features_; ++f) {
      if (fire_at_[f] != phase_) continue;
      const uint32_t begin = ms.begin + slice_begin(ms.count, features_, f);
      const uint32_t end = ms.begin + slice_begin(ms.count, features_, f + 1);
      for (uint32_t i = begin; i < end; ++i) net.inject(i, gain_);
    }
  }
  ++phase_;
}

// --- Touch -----------------------------------------------------------------

void TouchEncoder::configure(const Dna& dna, uint32_t module_index) {
  const DnaTouch& t = dna.header().touch;
  module_index_ = module_index;
  gain_ = Scalar(t.gain);
  const Scalar dt = Scalar(dna.header().sim.dt_ms);
  duration_ticks_ = uint32_t(Scalar(t.duration_ms) / dt + Scalar(0.5));
  if (duration_ticks_ == 0) duration_ticks_ = 1;
  active_ = false;
  for (uint32_t a = 0; a < uint32_t(TouchAction::kTouchCount); ++a) {
    level_[a] = kZero;
    left_[a] = 0;
  }
}

void TouchEncoder::trigger(TouchAction action, Scalar intensity) {
  const uint32_t a = uint32_t(action);
  if (a >= uint32_t(TouchAction::kTouchCount)) return;
  level_[a] = clampf(intensity, kZero, kOne);
  left_[a] = duration_ticks_;
  active_ = true;
}

void TouchEncoder::drive(Network& net) {
  if (!active_) return;
  const ModuleState& ms = net.module(module_index_);
  if (ms.count == 0) return;

  const uint32_t groups = uint32_t(TouchAction::kTouchCount);
  bool any = false;
  for (uint32_t a = 0; a < groups; ++a) {
    if (left_[a] == 0) continue;
    --left_[a];
    any = true;
    const uint32_t begin = ms.begin + slice_begin(ms.count, groups, a);
    const uint32_t end = ms.begin + slice_begin(ms.count, groups, a + 1);
    for (uint32_t i = begin; i < end; ++i) net.inject(i, gain_ * level_[a]);
  }
  active_ = any;
}

// --- Vocal motor -----------------------------------------------------------

void VocalDecoder::configure(const Dna& dna, uint32_t module_index, Scalar update_ms) {
  cfg_ = dna.header().vocal;
  module_index_ = module_index;
  frame_ = 0;
  // One-pole articulator inertia. A vocal tract has mass; without this the
  // formants jitter at the readout rate and the output is a buzz, not a voice.
  smooth_ = cfg_.smoothing_ms > kZero
                ? clampf(update_ms / Scalar(cfg_.smoothing_ms), kZero, kOne)
                : kOne;
  gate_smooth_ = cfg_.gate_smoothing_ms > kZero
                     ? clampf(update_ms / Scalar(cfg_.gate_smoothing_ms), kZero, kOne)
                     : kOne;
  params_ = VocalParams{};
  params_.f0 = lerp(Scalar(cfg_.f0_min), Scalar(cfg_.f0_max), Scalar(0.5));
  params_.f1 = lerp(Scalar(cfg_.f1_min), Scalar(cfg_.f1_max), Scalar(0.5));
  params_.f2 = lerp(Scalar(cfg_.f2_min), Scalar(cfg_.f2_max), Scalar(0.5));
  params_.f3 = lerp(Scalar(cfg_.f3_min), Scalar(cfg_.f3_max), Scalar(0.5));
  params_.bw1 = params_.bw2 = params_.bw3 =
      lerp(Scalar(cfg_.bw_min), Scalar(cfg_.bw_max), Scalar(0.5));
  params_.amplitude = kZero;
  params_.voicing = kZero;
  for (uint32_t g = 0; g < kVocalGroups; ++g) {
    group_value_[g] = Scalar(0.5);
    group_activity_[g] = kZero;
  }
  configured_ = true;
}

void VocalDecoder::update(const Network& net, bool awake) {
  if (!configured_) return;
  const ModuleState& ms = net.module(module_index_);
  if (ms.count < kVocalGroups) return;

  // Asleep the glottis is shut. The motor module keeps running — a sleeping
  // brain is not a silent one, and M4 wants that activity for replay — but
  // nothing reaches the air. Amplitude is ramped rather than cut so the
  // synthesiser does not click.
  if (!awake) {
    params_.voicing = kZero;
    params_.amplitude += gate_smooth_ * (kZero - params_.amplitude);
    ++frame_;
    return;
  }

  // Inertia is applied to the motor *command*, not only to the acoustic
  // parameters it produces. A vocal tract is a physical object: an articulatory
  // posture is held for a syllable, not redrawn every ten milliseconds.
  //
  // This is also what makes delayed reward learnable at all. An eligibility
  // trace can only bridge a delay if the behaviour still resembles itself when
  // the praise arrives; a motor state that decorrelates in fifty milliseconds
  // credits whatever the creature happened to be doing half a second later,
  // which is nothing in particular. Posture duration has to exceed caregiver
  // latency, and in a real infant it comfortably does.
  const Scalar rate_norm = Scalar(cfg_.rate_norm_hz);
  for (uint32_t g = 0; g < kVocalGroups; ++g) {
    const uint32_t begin = ms.begin + slice_begin(ms.count, kVocalGroups, g);
    const uint32_t end = ms.begin + slice_begin(ms.count, kVocalGroups, g + 1);
    const GroupReading r = read_group(net, begin, end, rate_norm, group_value_[g]);
    group_value_[g] += smooth_ * (r.value - group_value_[g]);
    group_activity_[g] += gate_smooth_ * (r.activity - group_activity_[g]);
  }

  // Group order is fixed: it is the wiring between motor cortex and larynx.
  const Scalar target_f0 = lerp(Scalar(cfg_.f0_min), Scalar(cfg_.f0_max), group_value_[0]);
  const Scalar target_f1 = lerp(Scalar(cfg_.f1_min), Scalar(cfg_.f1_max), group_value_[2]);
  const Scalar target_f2 = lerp(Scalar(cfg_.f2_min), Scalar(cfg_.f2_max), group_value_[3]);
  const Scalar target_f3 = lerp(Scalar(cfg_.f3_min), Scalar(cfg_.f3_max), group_value_[4]);
  const Scalar target_bw1 = lerp(Scalar(cfg_.bw_min), Scalar(cfg_.bw_max), group_value_[5]);
  const Scalar target_bw2 = lerp(Scalar(cfg_.bw_min), Scalar(cfg_.bw_max), group_value_[6]);
  const Scalar target_bw3 = lerp(Scalar(cfg_.bw_min), Scalar(cfg_.bw_max), group_value_[7]);
  const Scalar target_amp = group_activity_[8];

  params_.f0 = target_f0;
  params_.f1 = target_f1;
  params_.f2 = target_f2;
  params_.f3 = target_f3;
  params_.bw1 = target_bw1;
  params_.bw2 = target_bw2;
  params_.bw3 = target_bw3;
  params_.amplitude = target_amp;

  // Voicing is a gate, not a dial, so it is not smoothed.
  params_.voicing = group_activity_[1] > Scalar(cfg_.voicing_threshold) ? kOne : kZero;
  ++frame_;
}

// --- Expression ------------------------------------------------------------

void ExpressionDecoder::configure(uint32_t module_index, Scalar rate_norm_hz) {
  module_index_ = module_index;
  rate_norm_ = rate_norm_hz > kZero ? rate_norm_hz : Scalar(20);
  value_ = Expression{};
}

void ExpressionDecoder::update(const Network& net) {
  const ModuleState& ms = net.module(module_index_);
  if (ms.count < 2) return;
  const uint32_t mid = ms.begin + slice_begin(ms.count, 2, 1);
  const GroupReading valence =
      read_group(net, ms.begin, mid, rate_norm_, value_.valence);
  const GroupReading arousal =
      read_group(net, mid, ms.begin + ms.count, rate_norm_, Scalar(0.5));
  value_.valence = valence.value;
  value_.arousal = arousal.activity;
}

}  // namespace aibaby
