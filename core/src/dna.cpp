#include "aibaby/dna.h"

namespace aibaby {

const char* dna_status_string(DnaStatus status) {
  switch (status) {
    case DnaStatus::kOk: return "ok";
    case DnaStatus::kTooSmall: return "blob smaller than header";
    case DnaStatus::kBadMagic: return "bad magic";
    case DnaStatus::kBadVersion: return "unsupported version";
    case DnaStatus::kBadCounts: return "module/projection count out of range";
    case DnaStatus::kSizeMismatch: return "blob size does not match counts";
    case DnaStatus::kBadModule: return "invalid module parameters";
    case DnaStatus::kBadProjection: return "projection references unknown module";
    case DnaStatus::kBadRole: return "unknown or duplicated module role";
    case DnaStatus::kBadAudio: return "invalid cochlea parameters";
    case DnaStatus::kBadVision: return "invalid retina parameters";
    case DnaStatus::kBadPlasticity: return "invalid plasticity parameters";
    case DnaStatus::kBadGrowth: return "invalid growth or consolidation parameters";
  }
  return "unknown";
}

uint32_t vision_rings(const DnaVision& v) {
  if (v.fovea_size == 0 || v.frame_size < v.fovea_size) return 0;
  uint32_t rings = 0;
  for (uint32_t window = v.fovea_size; window < v.frame_size; window *= 2) ++rings;
  return rings;
}

uint32_t vision_cells(const DnaVision& v) {
  const uint32_t hole = v.ring_grid / 2;  // covered by the ring inside
  const uint32_t per_ring = v.ring_grid * v.ring_grid - hole * hole;
  return v.fovea_grid * v.fovea_grid + vision_rings(v) * per_ring;
}

// The same walk the host sampler makes, in the same order, normalised to the
// frame. Ring 0 is the fovea; each further ring doubles its window and keeps
// the same small grid, dropping the middle because the ring inside already
// covers that square at twice the acuity.
//
// Index order is load-bearing twice over: it is the order the host packs a
// frame in, and it is therefore the order the vision module's channel map is
// sliced in. This function and Retina::configure() must walk it identically,
// which is why the host now calls this one.
VisionCell vision_cell(const DnaVision& v, uint32_t cell) {
  const VisionCell none{0.0f, 0.0f, 0.0f};
  if (v.frame_size == 0 || v.fovea_grid == 0 || v.ring_grid == 0) return none;

  const float frame = float(v.frame_size);
  const float centre = 0.5f;  // normalised
  const uint32_t fovea_cells = v.fovea_grid * v.fovea_grid;

  if (cell < fovea_cells) {
    const float window = float(v.fovea_size) / frame;
    const float pitch = window / float(v.fovea_grid);
    const float origin = centre - window * 0.5f;
    const uint32_t gx = cell % v.fovea_grid;
    const uint32_t gy = cell / v.fovea_grid;
    return VisionCell{origin + (float(gx) + 0.5f) * pitch,
                      origin + (float(gy) + 0.5f) * pitch, pitch};
  }

  const uint32_t hole = v.ring_grid / 2;
  const uint32_t hole_lo = (v.ring_grid - hole) / 2;
  const uint32_t hole_hi = hole_lo + hole;
  const uint32_t per_ring = v.ring_grid * v.ring_grid - hole * hole;
  if (per_ring == 0) return none;

  uint32_t rest = cell - fovea_cells;
  const uint32_t ring = rest / per_ring + 1;
  if (ring > vision_rings(v)) return none;
  rest %= per_ring;

  const float window = float(v.fovea_size << ring) / frame;
  const float pitch = window / float(v.ring_grid);
  const float origin = centre - window * 0.5f;

  // Walk the ring's grid skipping the hole, so the k-th surviving square is
  // found by counting rather than by arithmetic that would have to re-derive
  // the skip pattern.
  uint32_t seen = 0;
  for (uint32_t gy = 0; gy < v.ring_grid; ++gy) {
    for (uint32_t gx = 0; gx < v.ring_grid; ++gx) {
      if (gx >= hole_lo && gx < hole_hi && gy >= hole_lo && gy < hole_hi) continue;
      if (seen == rest) {
        return VisionCell{origin + (float(gx) + 0.5f) * pitch,
                          origin + (float(gy) + 0.5f) * pitch, pitch};
      }
      ++seen;
    }
  }
  return none;
}

DnaStatus Dna::load(const void* blob, size_t size) {
  header_ = nullptr;
  if (size < sizeof(DnaHeader)) return DnaStatus::kTooSmall;

  const DnaHeader* h = static_cast<const DnaHeader*>(blob);
  if (h->magic != kDnaMagic) return DnaStatus::kBadMagic;
  if (h->version != kDnaVersion) return DnaStatus::kBadVersion;
  if (h->module_count == 0 || h->module_count > kMaxModules) return DnaStatus::kBadCounts;
  if (h->projection_count > kMaxProjections) return DnaStatus::kBadCounts;

  const size_t expected = sizeof(DnaHeader) +
                          sizeof(DnaModule) * h->module_count +
                          sizeof(DnaProjection) * h->projection_count;
  if (size != expected) return DnaStatus::kSizeMismatch;

  const uint8_t* cursor = static_cast<const uint8_t*>(blob) + sizeof(DnaHeader);
  const DnaModule* modules = reinterpret_cast<const DnaModule*>(cursor);
  cursor += sizeof(DnaModule) * h->module_count;
  const DnaProjection* projections = reinterpret_cast<const DnaProjection*>(cursor);

  // Semantic validation. A malformed genome should fail here, at load, rather
  // than as a division by zero somewhere in the tick loop an hour later.
  if (h->sim.dt_ms <= 0.0f) return DnaStatus::kBadModule;
  if (h->sim.max_delay_ticks < 2) return DnaStatus::kBadModule;
  if (h->sim.conduction_velocity <= 0.0f) return DnaStatus::kBadModule;

  if (h->sim.plasticity_interval_ticks == 0) return DnaStatus::kBadPlasticity;
  if (h->homeo.interval_ticks == 0) return DnaStatus::kBadPlasticity;
  if (h->stdp.tau_plus_ms <= 0.0f || h->stdp.tau_minus_ms <= 0.0f) {
    return DnaStatus::kBadPlasticity;
  }
  if (h->stdp.tau_elig_ms <= 0.0f) return DnaStatus::kBadPlasticity;
  if (h->stdp.reward_clip <= 0.0f) return DnaStatus::kBadPlasticity;
  if (h->homeo.w_max <= 0.0f) return DnaStatus::kBadPlasticity;
  // A band below 1 would mean the low bound sits above the high one.
  if (h->homeo.scaling_band < 1.0f) return DnaStatus::kBadPlasticity;
  if (h->homeo.threshold_min <= 0.0f ||
      h->homeo.threshold_max < h->homeo.threshold_min) {
    return DnaStatus::kBadPlasticity;
  }
  // Cashing in eligibility more slowly than it decays would throw most of the
  // trace away unread, which looks exactly like "learning does not work".
  if (float(h->sim.plasticity_interval_ticks) * h->sim.dt_ms > h->stdp.tau_elig_ms) {
    return DnaStatus::kBadPlasticity;
  }

  // Growth (§3.4) and consolidation (§3.5, §3.6). These are structural: a bad
  // value here does not misbehave, it deletes or duplicates neurons, so the
  // genome is rejected rather than clamped.
  {
    const DnaGrowth& g = h->growth;
    if (g.enabled) {
      if (g.plateau_window_ticks == 0) return DnaStatus::kBadGrowth;
      if (g.epsilon < 0.0f) return DnaStatus::kBadGrowth;
      if (g.insert_k == 0) return DnaStatus::kBadGrowth;
      if (g.saturation_rate_hz <= 0.0f) return DnaStatus::kBadGrowth;
      if (g.saturation_weight <= 0.0f || g.saturation_weight > 1.0f) {
        return DnaStatus::kBadGrowth;
      }
      // A grown synapse born above the ceiling would be clamped on its first
      // reward and the genome would be describing a brain nobody builds.
      if (g.new_weight <= 0.0f || g.new_weight > h->homeo.w_max) {
        return DnaStatus::kBadGrowth;
      }
    }
    const DnaConsolidate& c = h->consolidate;
    if (c.traffic_tau_ms <= 0.0f) return DnaStatus::kBadGrowth;
    if (c.traffic_half <= 0.0f) return DnaStatus::kBadGrowth;
    // Myelination only ever shortens a delay and only ever slows learning, so
    // both fractions live in (0, 1]. At 1 the mechanism is off, which is a
    // legitimate genome and the control condition every measurement needs.
    if (c.delay_floor_frac <= 0.0f || c.delay_floor_frac > 1.0f) {
      return DnaStatus::kBadGrowth;
    }
    if (c.eta_floor_frac <= 0.0f || c.eta_floor_frac > 1.0f) return DnaStatus::kBadGrowth;
    if (c.enabled) {
      // Downscaling above 1 would be upscaling — a positive feedback loop with
      // no restoring force, run once per sleep bout.
      if (c.downscale <= 0.0f || c.downscale > 1.0f) return DnaStatus::kBadGrowth;
      if (c.downscale_floor <= 0.0f || c.downscale_floor > 1.0f) {
        return DnaStatus::kBadGrowth;
      }
      if (c.prune_weight < 0.0f) return DnaStatus::kBadGrowth;
      if (c.prune_traffic < 0.0f) return DnaStatus::kBadGrowth;
      if (c.interval_ticks == 0) return DnaStatus::kBadGrowth;
      if (c.replay_episodes > kMaxReplayEpisodes) return DnaStatus::kBadGrowth;
      if (c.replay_episodes > 0 && c.replay_ticks == 0) return DnaStatus::kBadGrowth;
    }
  }

  if (h->audio.mel_channels == 0 || h->audio.mel_channels > kMaxMelChannels) {
    return DnaStatus::kBadAudio;
  }
  if (h->audio.sample_rate == 0 || h->audio.window == 0 || h->audio.hop == 0) {
    return DnaStatus::kBadAudio;
  }
  if (h->audio.hop > h->audio.window) return DnaStatus::kBadAudio;
  // A non-power-of-two window would need a mixed-radix transform on the host.
  if ((h->audio.window & (h->audio.window - 1)) != 0) return DnaStatus::kBadAudio;
  if (h->audio.mel_low_hz < 0.0f ||
      h->audio.mel_high_hz <= h->audio.mel_low_hz ||
      h->audio.mel_high_hz > float(h->audio.sample_rate) * 0.5f) {
    return DnaStatus::kBadAudio;
  }
  // The retina (§5.1). Each ring doubles the window it covers, so the frame
  // has to be the fovea scaled by a power of two or the outermost ring would
  // either overrun the frame or leave a border no cell ever sees.
  {
    const DnaVision& vis = h->vision;
    if (vis.fovea_size == 0 || vis.frame_size < vis.fovea_size) return DnaStatus::kBadVision;
    if (vis.fovea_grid < 2 || vis.ring_grid < 4) return DnaStatus::kBadVision;
    // A ring drops its middle ring_grid/2 cells, and that hole has to sit
    // centred on a cell boundary — which it only does when the grid is a
    // multiple of four.
    if (vis.ring_grid % 4 != 0) return DnaStatus::kBadVision;
    uint32_t window = vis.fovea_size;
    while (window < vis.frame_size) window *= 2;
    if (window != vis.frame_size) return DnaStatus::kBadVision;
    // A grid that does not divide its window would put cells at fractional
    // pixel pitches, and the two ends of the row would sample different areas.
    if (vis.fovea_size % vis.fovea_grid != 0) return DnaStatus::kBadVision;
    for (uint32_t w = vis.fovea_size * 2; w <= vis.frame_size; w *= 2) {
      if (w % vis.ring_grid != 0) return DnaStatus::kBadVision;
    }
    if (vision_features(vis) > kMaxVisionFeatures) return DnaStatus::kBadVision;
    if (vis.center_sigma <= 0.0f || vis.surround_sigma <= vis.center_sigma) {
      return DnaStatus::kBadVision;
    }
    if (vis.contrast_gain <= 0.0f) return DnaStatus::kBadVision;
    if (vis.contrast_floor < 0.0f || vis.contrast_floor >= 1.0f) return DnaStatus::kBadVision;
    if (vis.gain <= 0.0f) return DnaStatus::kBadVision;
    if (vis.frame_hz <= 0.0f) return DnaStatus::kBadVision;
    if (vis.latency_ms <= 0.0f) return DnaStatus::kBadVision;
    // The latency window is the code. If it outran the frame the slowest cells
    // would still be waiting to fire when the next frame overwrote them.
    if (vis.latency_ms > 1000.0f / vis.frame_hz) return DnaStatus::kBadVision;
  }

  if (h->vocal.f0_min <= 0.0f || h->vocal.f0_max <= h->vocal.f0_min) {
    return DnaStatus::kBadAudio;
  }
  if (h->vocal.rate_norm_hz <= 0.0f) return DnaStatus::kBadAudio;
  // The wake threshold must sit below the sleep threshold, or the creature
  // would wake on the tick it fell asleep and chatter between the two.
  if (h->drives.sleep_threshold <= h->drives.wake_threshold ||
      h->drives.sleep_threshold > 1.0f || h->drives.wake_threshold < 0.0f) {
    return DnaStatus::kBadPlasticity;
  }
  if (h->drives.fatigue_recovery <= 0.0f) return DnaStatus::kBadPlasticity;
  if (h->vocal.smoothing_ms < 0.0f || h->vocal.gate_smoothing_ms < 0.0f) {
    return DnaStatus::kBadAudio;
  }

  if (h->exploration.enabled) {
    if (h->exploration.fast_tau_ms <= 0.0f || h->exploration.slow_tau_ms <= 0.0f) {
      return DnaStatus::kBadPlasticity;
    }
    // Fast must be faster, or the difference is the wrong sign and success
    // would make the creature *more* random.
    if (h->exploration.fast_tau_ms >= h->exploration.slow_tau_ms) {
      return DnaStatus::kBadPlasticity;
    }
    if (h->exploration.floor <= 0.0f || h->exploration.ceiling < h->exploration.floor) {
      return DnaStatus::kBadPlasticity;
    }
    if (h->exploration.perturb_tau_ms <= 0.0f) return DnaStatus::kBadPlasticity;
    if (h->exploration.perturb_rate < 0.0f) return DnaStatus::kBadPlasticity;
    // An unbounded bias is a neuron that one lucky second can silence or pin on
    // for the rest of the creature's life.
    if (h->exploration.perturb_rate > 0.0f && h->exploration.perturb_max <= 0.0f) {
      return DnaStatus::kBadPlasticity;
    }
  }

  if (h->normalisation.enabled) {
    // A floor at or below zero lets the divisor invert the sign of every drive
    // in the module, which is not attenuation but a different network.
    if (h->normalisation.floor <= 0.0f) return DnaStatus::kBadPlasticity;
    if (h->normalisation.ceiling < h->normalisation.floor) return DnaStatus::kBadPlasticity;
  }

  bool role_seen[uint32_t(ModuleRole::kRoleCount)] = {false};

  for (uint32_t i = 0; i < h->module_count; ++i) {
    const DnaModule& m = modules[i];
    if (m.neurons == 0 || m.n_max < m.neurons) return DnaStatus::kBadModule;
    if (m.max_out_degree == 0) return DnaStatus::kBadModule;
    if (m.leak_tau_ms <= 0.0f) return DnaStatus::kBadModule;
    if (m.conn_radius <= 0.0f) return DnaStatus::kBadModule;
    if (m.threshold <= m.v_rest) return DnaStatus::kBadModule;
    if (m.inhib_fraction < 0.0f || m.inhib_fraction > 1.0f) return DnaStatus::kBadModule;
    if (m.explore_scale < 0.0f) return DnaStatus::kBadModule;
    if (m.ip_wake_scale < 0.0f || m.ip_sleep_scale < 0.0f) return DnaStatus::kBadModule;
    if (m.syn_wake_scale < 0.0f || m.syn_sleep_scale < 0.0f) return DnaStatus::kBadModule;
    if (m.norm_gain < 0.0f) return DnaStatus::kBadModule;
    if (m.eta_scale < 0.0f) return DnaStatus::kBadModule;
    if (m.extent[0] <= 0.0f || m.extent[1] <= 0.0f || m.extent[2] <= 0.0f) {
      return DnaStatus::kBadModule;
    }
    // The name must be NUL-terminated inside the field or later string use
    // walks off the end of the struct.
    bool terminated = false;
    for (uint32_t c = 0; c < kMaxNameLen; ++c) {
      if (m.name[c] == '\0') { terminated = true; break; }
    }
    if (!terminated) return DnaStatus::kBadModule;

    if (m.role >= uint32_t(ModuleRole::kRoleCount)) return DnaStatus::kBadRole;
    // Association and interneuron are the roles a genome may hand out more than
    // once; the transducer roles each own a hardware channel and cannot be
    // shared. A relay owns no channel and nothing looks it up, so there is no
    // reason a creature may not have one between each of several module pairs —
    // and a rule that allowed only one would be a limit nothing asked for.
    if (m.role != uint32_t(ModuleRole::kAssociation) &&
        m.role != uint32_t(ModuleRole::kInterneuron)) {
      if (role_seen[m.role]) return DnaStatus::kBadRole;
      role_seen[m.role] = true;
    }
  }

  // The vocal readout slices B5 into nine groups; fewer neurons than groups
  // would leave parameters with no population to code them.
  for (uint32_t i = 0; i < h->module_count; ++i) {
    if (modules[i].role == uint32_t(ModuleRole::kVocal) &&
        modules[i].neurons < kVocalGroups) {
      return DnaStatus::kBadModule;
    }
    if (modules[i].role == uint32_t(ModuleRole::kAuditory) &&
        modules[i].neurons < h->audio.mel_channels) {
      return DnaStatus::kBadModule;
    }
    // Latency coding gives each ON/OFF feature its own moment rather than its
    // own population, so one neuron per feature is enough — but fewer than one
    // would silently merge two cells into a single place in the module.
    if (modules[i].role == uint32_t(ModuleRole::kVision) &&
        modules[i].neurons < vision_features(h->vision)) {
      return DnaStatus::kBadModule;
    }
  }

  for (uint32_t i = 0; i < h->projection_count; ++i) {
    const DnaProjection& p = projections[i];
    if (p.src >= h->module_count || p.dst >= h->module_count) {
      return DnaStatus::kBadProjection;
    }
    if (p.density < 0.0f || p.density > 1.0f) return DnaStatus::kBadProjection;
    if (p.delay_ms < 0.0f) return DnaStatus::kBadProjection;
    if (p.kind >= uint32_t(ProjectionKind::kKindCount)) return DnaStatus::kBadProjection;
    if (p.source >= uint32_t(ProjectionSource::kSourceCount)) {
      return DnaStatus::kBadProjection;
    }
    // DNA v36. Dynamic synapses. Rejected rather than clamped for the reason
    // the plateau gate below is: each of these is a genome that reads as an
    // enabled mechanism and behaves as a disabled one, which is the failure
    // this project has paid for most often.
    //
    //   U outside (0, 1]  — above 1 a spike releases resources the synapse does
    //                       not have and R goes negative; below 0 it inverts
    //                       the sign of the tract at delivery.
    //   no recovery time  — resources return instantly, R is 1 at every spike,
    //                       and the synapse is a constant-weight one wearing
    //                       three genome fields.
    if (p.stp_use < 0.0f || p.stp_use > 1.0f) return DnaStatus::kBadProjection;
    if (p.stp_use > 0.0f && p.stp_recover_ms <= 0.0f) return DnaStatus::kBadProjection;
    if (p.stp_recover_ms < 0.0f || p.stp_facil_ms < 0.0f) return DnaStatus::kBadProjection;
    if (p.kind == uint32_t(ProjectionKind::kGabor)) {
      // A receptive field only means anything over a retina, and the geometry
      // it reads is the vision module's channel map. Sourcing one from
      // anywhere else would silently reinterpret whatever that module's slices
      // happen to be as positions on a retina that is not there.
      if (modules[p.src].role != uint32_t(ModuleRole::kVision)) {
        return DnaStatus::kBadProjection;
      }
      if (p.rf_sigma <= 0.0f || p.rf_lambda <= 0.0f) return DnaStatus::kBadProjection;
      if (p.rf_aspect <= 0.0f) return DnaStatus::kBadProjection;
      // Below one the warp would push the cortex out to the periphery, which is
      // a body plan nothing in this project wants and a very confusing one to
      // debug from a firing rate.
      if (p.rf_magnification < 1.0f) return DnaStatus::kBadProjection;
      // At or below zero every cell in the envelope is wired and the field stops
      // being a field; at or above one nothing is.
      if (p.rf_floor <= 0.0f || p.rf_floor >= 1.0f) return DnaStatus::kBadProjection;
    }
    if (p.kind == uint32_t(ProjectionKind::kCurvature)) {
      // A form cell pools *oriented* cells, so its source has to be a module
      // that some kGabor projection actually built. Sourcing it anywhere else
      // would read that module's z axis as an orientation it never had.
      if (modules[p.src].role != uint32_t(ModuleRole::kVisualCortex)) {
        return DnaStatus::kBadProjection;
      }
      bool has_map = false;
      for (uint32_t q = 0; q < h->projection_count; ++q) {
        if (projections[q].kind == uint32_t(ProjectionKind::kGabor) &&
            projections[q].dst == p.src) {
          has_map = true;
        }
      }
      if (!has_map) return DnaStatus::kBadProjection;
      if (p.rf_sigma <= 0.0f || p.rf_tangent_sigma <= 0.0f) return DnaStatus::kBadProjection;
      if (p.rf_radius_min <= 0.0f || p.rf_radius_max < p.rf_radius_min) {
        return DnaStatus::kBadProjection;
      }
      if (p.rf_floor <= 0.0f || p.rf_floor >= 1.0f) return DnaStatus::kBadProjection;
      if (p.rf_magnification < 1.0f) return DnaStatus::kBadProjection;
    }
  }

  // DNA v29. A plateau gate needs a plateau to gate on, and that takes two
  // things the genome states in two different places: a compartment on the
  // module, and a tract that lands on it. With either missing the tuft never
  // crosses threshold, the gate is shut for the whole of the creature's life,
  // and every eligibility is multiplied by (1 - gate) — plasticity turned down
  // or off by what reads as an enabled mechanism. Rejected here rather than
  // clamped, because the wrong genome should not run at all.
  for (uint32_t i = 0; i < h->module_count; ++i) {
    if (modules[i].plateau_gate <= 0.0f) continue;
    if (modules[i].plateau_gate > 1.0f) return DnaStatus::kBadModule;
    if (modules[i].apical_threshold <= 0.0f) return DnaStatus::kBadModule;
    bool fed = false;
    for (uint32_t q = 0; q < h->projection_count; ++q) {
      if (projections[q].dst == i && projections[q].apical != 0) fed = true;
    }
    if (!fed) return DnaStatus::kBadModule;
  }

  header_ = h;
  modules_ = modules;
  projections_ = projections;
  return DnaStatus::kOk;
}

int32_t Dna::module_with_role(ModuleRole role) const {
  for (uint32_t i = 0; i < header_->module_count; ++i) {
    if (modules_[i].role == uint32_t(role)) return int32_t(i);
  }
  return -1;
}

uint32_t Dna::total_neurons_at_birth() const {
  uint32_t total = 0;
  for (uint32_t i = 0; i < header_->module_count; ++i) total += modules_[i].neurons;
  return total;
}

uint32_t Dna::total_neurons_max() const {
  uint32_t total = 0;
  for (uint32_t i = 0; i < header_->module_count; ++i) total += modules_[i].n_max;
  return total;
}

}  // namespace aibaby
