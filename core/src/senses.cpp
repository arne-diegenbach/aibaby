#include "aibaby/senses.h"

// expf, for DNA v48's softmax over the motor dictionary. The kernel keeps
// transcendentals out of the TICK loop on purpose (see network.cpp); this runs
// once per motor frame per posture -- 100 Hz times nine, not 1 kHz times nine
// thousand -- so it is the same category as critic.cpp's use rather than an
// exception to the rule.
#include <math.h>  // expf, sqrtf

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

  // DNA v48. The dictionary is off at 0 and the block below never executes,
  // which is what makes this version bit-identical to v47 on the shipped
  // genome.
  units_ = cfg_.dictionary_units > kMaxDictionaryUnits ? kMaxDictionaryUnits
                                                       : cfg_.dictionary_units;
  dwell_ticks_ = cfg_.dictionary_dwell_ms > kZero
                     ? uint32_t(Scalar(cfg_.dictionary_dwell_ms) / update_ms + Scalar(0.5))
                     : 0;
  winner_ = 0;
  dwell_left_ = 0;
  switches_ = 0;
  dict_primed_ = false;
  for (uint32_t u = 0; u < kMaxDictionaryUnits; ++u) unit_activity_[u] = kZero;
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
  Scalar target_f1 = lerp(Scalar(cfg_.f1_min), Scalar(cfg_.f1_max), group_value_[2]);
  Scalar target_f2 = lerp(Scalar(cfg_.f2_min), Scalar(cfg_.f2_max), group_value_[3]);

  // DNA v48. The dictionary, if the genome asked for one. The same neurons,
  // cut a second way: `units_` slices compete on mean rate, the most active
  // wins, and the winner names a cell of a grid over (F1, F2). The nine groups
  // above still set everything else, so this replaces the centroid readout of
  // two parameters and nothing else in the creature.
  if (units_ > 0) {
    uint32_t best = winner_;
    Scalar best_act = Scalar(-1);
    for (uint32_t u = 0; u < units_; ++u) {
      const uint32_t b = ms.begin + slice_begin(ms.count, units_, u);
      const uint32_t e = ms.begin + slice_begin(ms.count, units_, u + 1);
      unit_activity_[u] = read_group(net, b, e, rate_norm, kZero).activity;
      if (unit_activity_[u] > best_act) { best_act = unit_activity_[u]; best = u; }
    }
    // Hysteresis. An argmax over near-equal activities changes its mind at the
    // readout rate, and a vocal tract that redraws its posture every ten
    // milliseconds produces a buzz rather than a vowel -- the same argument
    // `smoothing_ms` makes about how fast the tract moves, applied to how often
    // it is allowed to be aimed somewhere new.
    const bool reselect = dwell_left_ == 0;
    if (dwell_left_ > 0) --dwell_left_;
    if (reselect) {
      if (best != winner_) { winner_ = best; ++switches_; }
      dwell_left_ = dwell_ticks_;
    }

    // The grid. Square, and the last row is short when `units_` is not a
    // perfect square -- an inventory with a ragged edge is better than one
    // whose size the genome may not choose freely.
    uint32_t side = 1;
    while (side * side < units_) ++side;
    const Scalar n = Scalar(side);
    auto cell_f1 = [&](uint32_t u) {
      return lerp(Scalar(cfg_.f1_min), Scalar(cfg_.f1_max),
                  (Scalar(u / side) + Scalar(0.5)) / n);
    };
    auto cell_f2 = [&](uint32_t u) {
      return lerp(Scalar(cfg_.f2_min), Scalar(cfg_.f2_max),
                  (Scalar(u % side) + Scalar(0.5)) / n);
    };

    Scalar f1_cell, f2_cell;
    const Scalar temp = Scalar(cfg_.dictionary_temp);
    if (temp <= kZero) {
      // Hard argmax: the winner alone holds the tract.
      f1_cell = cell_f1(winner_);
      f2_cell = cell_f2(winner_);
    } else {
      // Softmax over the unit activities, weighting the postures. Shifted by
      // the maximum before exponentiating -- the activities are bounded in
      // [0,1] but the temperature is not, and exp of a large quotient
      // overflows a float long before it stops being a useful weight.
      // Z-scored before the softmax, and this is the correction that makes the
      // temperature mean anything. Measured on the raw activities the blend
      // collapses to the grid's mean at every usable temperature -- F1 spread
      // 147 Hz at hard argmax, 30 Hz at temp 0.02, 24 Hz at 0.05, against the
      // centroid readout's own 25 Hz. The reason is that the units' activities
      // differ by one or two percent of their mean, because `vocal` is a
      // homeostatically regulated population: it is the same small differential
      // on a big common mode that this creature has everywhere else, arriving
      // at the readout. An absolute temperature is therefore either far below
      // the differences (argmax, no gradient) or far above them (uniform blend,
      // no spread), with no regime in between.
      //
      // Scaling by the units' own spread makes the knob scale-free: temp 1.0
      // blends everything within one standard deviation of the field, 0.3
      // prefers the leader sharply. The intermediate regime exists by
      // construction rather than by luck.
      Scalar mean = kZero;
      for (uint32_t u = 0; u < units_; ++u) mean += unit_activity_[u];
      mean /= Scalar(units_);
      Scalar var = kZero;
      for (uint32_t u = 0; u < units_; ++u) {
        const Scalar d = unit_activity_[u] - mean;
        var += d * d;
      }
      const Scalar sd = sqrtf(float(var / Scalar(units_)));
      // A field with no spread at all has no opinion; fall back to the winner
      // rather than dividing by zero.
      const Scalar scale = sd > Scalar(1e-6) ? sd : kOne;

      Scalar total = kZero, w1 = kZero, w2 = kZero;
      for (uint32_t u = 0; u < units_; ++u) {
        const Scalar w = expf((unit_activity_[u] - best_act) / (temp * scale));
        total += w;
        w1 += w * cell_f1(u);
        w2 += w * cell_f2(u);
      }
      if (total > Scalar(1e-9)) { f1_cell = w1 / total; f2_cell = w2 / total; }
      else { f1_cell = cell_f1(winner_); f2_cell = cell_f2(winner_); }
    }
    // Smoothed with the SAME constant the centroid path uses, so that swapping
    // readouts does not quietly also swap articulator inertia. Primed on the
    // first frame, or the tract slides up from zero and the first syllable of
    // every life is a chirp.
    // The target is refreshed only when the dwell expires, and HELD in between.
    // The argmax path was getting this for free -- `winner_` is dwell-held --
    // and the softmax path was not: it followed the instantaneous leader, which
    // flips every frame, and `smooth_` then averaged a flickering target down
    // to the mean of the whole inventory. That is why the temperature sweep read
    // 27 Hz at temp 0.2, which is very nearly an argmax, against 147 Hz for the
    // argmax itself. A difference that survives at temperatures where the two
    // rules agree was never about the temperature.
    if (!dict_primed_) {
      held_f1_ = f1_cell; held_f2_ = f2_cell;
      dict_f1_ = f1_cell; dict_f2_ = f2_cell;
      dict_primed_ = true;
    }
    if (reselect) { held_f1_ = f1_cell; held_f2_ = f2_cell; }
    dict_f1_ += smooth_ * (held_f1_ - dict_f1_);
    dict_f2_ += smooth_ * (held_f2_ - dict_f2_);
    target_f1 = dict_f1_;
    target_f2 = dict_f2_;
  }
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
