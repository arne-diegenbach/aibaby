#include "aibaby/network.h"

#include <math.h>  // sqrtf/expf — build path only; the tick loop stays free of them

#include "aibaby/snapshot_io.h"

namespace aibaby {
namespace {

inline Scalar clampf(Scalar v, Scalar lo, Scalar hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

inline Scalar absf(Scalar v) { return v < kZero ? -v : v; }

// DNA v28. The density a tract is *born* at, before experience prunes it back.
// Clamped at 1: a projection cannot be denser than all-to-all, and a genome
// asking for that has made an arithmetic error rather than a request. Build
// path only — this is read while wiring and never again.
inline Scalar birth_density(const DnaProjection& p) {
  const Scalar e = p.exuberance > 0.0f ? Scalar(p.exuberance) : kOne;
  const Scalar d = Scalar(p.density) * e;
  return d > kOne ? kOne : d;
}

// DNA v30. The weight a tract is *born* at, before experience strengthens or
// prunes it. The companion to birth_density and the reason exuberance can
// select at all: overproducing contacts already at adult strength is a
// permanent density increase, not a developmental surplus. Build path only.
inline Scalar birth_weight(const DnaProjection& p) {
  return Scalar(p.weight) * (p.birth_weight > 0.0f ? Scalar(p.birth_weight) : kOne);
}

// Per-tick (or per-interval) multiplier for an exponential decay. Evaluated
// only at build time so the kernel itself never calls expf.
inline Scalar decay_per(Scalar step_ms, Scalar tau_ms) {
  if (tau_ms <= kZero) return kZero;
  return Scalar(expf(-float(step_ms) / float(tau_ms)));
}

// Cortical magnification (DnaProjection::rf_magnification). Maps a uniform
// coordinate in [0,1] onto the retina, pulled toward the centre: k = 1 is the
// identity, larger k spends more of the cortex on the fovea. Signed about the
// midpoint so the warp is symmetric and the centre stays the centre.
inline Scalar magnify(Scalar t, Scalar k) {
  const Scalar s = Scalar(2) * t - kOne;
  const Scalar m = Scalar(powf(float(s < kZero ? -s : s), float(k)));
  return Scalar(0.5) * (kOne + (s < kZero ? -m : m));
}

inline void hash_bytes(uint64_t& h, const void* data, size_t len) {
  const uint8_t* p = static_cast<const uint8_t*>(data);
  for (size_t i = 0; i < len; ++i) {
    h ^= p[i];
    h *= 0x100000001B3ULL;
  }
}

inline void hash_scalar(uint64_t& h, Scalar v) {
  // Collapse -0.0 onto +0.0 first: the two compare equal but hash apart, and
  // a determinism check that trips on the sign of a zero teaches us nothing.
  if (v == kZero) v = kZero;
  hash_bytes(h, &v, sizeof(v));
}

}  // namespace

size_t Network::required_bytes(const Dna& dna) {
  size_t capacity = 0;
  size_t synapse_pool = 0;
  bool lateral = false;
  for (uint32_t m = 0; m < dna.module_count(); ++m) {
    const DnaModule& dm = dna.module(m);
    capacity += dm.n_max;
    synapse_pool += size_t(dm.n_max) * dm.max_out_degree;
    if (dm.lateral_gain > 0.0f) lateral = true;
  }

  // Mirrors the allocation block in build(), in the same order.
  const size_t per_neuron = 19 * sizeof(Scalar)     // v, v_rest, threshold, target_rate,
                                                    // leak, noise, rate, rate_fast, x, y,
                                                    // z, perturb, bias, trace_pre,
                                                    // trace_post, w_in_target, w_in_struct,
                                                    // ffi_w (DNA v24), v_apical (v25)
                            + 4 * sizeof(uint32_t)  // refrac_until, last_spike, syn_base,
                                                    // plateau_until (DNA v25)
                            + 4 * sizeof(uint16_t)  // syn_count, syn_cap, in_count, refrac
                            + 3 * sizeof(uint8_t);  // module_of, is_inhib, dead
  const size_t per_synapse = 3 * sizeof(uint32_t)   // target, source, reverse entry
                             + 4 * sizeof(Scalar)   // weight, elig, elig mean, traffic
                             + 2 * sizeof(uint16_t);  // delay, birth delay

  size_t total = capacity * per_neuron + synapse_pool * per_synapse;
  // Two delay rings since DNA v25: one per compartment.
  total += 2 * capacity * size_t(dna.header().sim.max_delay_ticks) * sizeof(Scalar);
  total += capacity * sizeof(uint32_t);  // spike_list
  // DNA v32's lateral scratch, and only when a module asks for it — so a
  // genome with the mechanism off gets an arena byte-for-byte the size it was
  // before the mechanism existed.
  if (lateral) total += capacity * sizeof(Scalar);

  // Slack for per-allocation alignment padding.
  total += 1024;
  return total;
}

bool Network::build(const Dna& dna, Arena& arena, Rng& rng) {
  dna_ = dna;
  rng_ = &rng;
  tick_ = 0;
  spike_count_ = 0;
  dropped_synapses_ = 0;
  dropped_reverse_ = 0;
  for (uint32_t m = 0; m < kMaxModules; ++m) {
    dropped_by_module_[m] = 0;
    dropped_reverse_by_module_[m] = 0;
  }
  last_reward_ = kZero;

  const DnaHeader& h = dna.header();
  module_count_ = h.module_count;
  dt_ms_ = Scalar(h.sim.dt_ms);
  delay_slots_ = h.sim.max_delay_ticks;

  // A 1-second EMA for homeostasis, a 50 ms one for the motor readout, and
  // the Hz value a single spike represents.
  rate_alpha_ = dt_ms_ / Scalar(1000);
  rate_fast_alpha_ = clampf(dt_ms_ / Scalar(50), kZero, kOne);
  spike_rate_unit_ = Scalar(1000) / dt_ms_;

  pre_decay_ = decay_per(dt_ms_, Scalar(h.stdp.tau_plus_ms));
  post_decay_ = decay_per(dt_ms_, Scalar(h.stdp.tau_minus_ms));
  const Scalar interval_ms = dt_ms_ * Scalar(h.sim.plasticity_interval_ticks);
  elig_decay_ = decay_per(interval_ms, Scalar(h.stdp.tau_elig_ms));
  // Per cash-in, not per tick: the baseline is only ever read and written in
  // apply_reward(), so its time constant is expressed on that clock.
  elig_pre_centre_ = Scalar(h.stdp.elig_pre_centre);
  {
    const Scalar tau = Scalar(h.interneuron.tau_ms);
    const Scalar dt = Scalar(h.sim.dt_ms);
    pool_alpha_ = tau > kZero ? clampf(dt / tau, kZero, kOne) : kOne;
  }
  // DNA v23: fold the per-pathway Hebbian rates into a (src, dst) matrix. Two
  // projections sharing a pair would overwrite rather than sum, which is worth
  // knowing but does not arise in the shipped genome.
  for (uint32_t pi = 0; pi < dna.header().projection_count; ++pi) {
    const DnaProjection& pr = dna.projection(pi);
    if (pr.src >= kMaxModules || pr.dst >= kMaxModules) continue;
    hebb_pair_[pr.src][pr.dst] = Scalar(pr.hebb);
    if (pr.hebb != 0.0f) any_hebb_pair_ = true;
    // DNA v25, same fold and the same caveat. A tract is apical only if the
    // module it lands on actually has a compartment for it to land in — a
    // genome that marks a projection apical but leaves the target's threshold
    // at zero would otherwise post spikes into a ring nothing ever reads, and
    // that is silent weight loss rather than a configuration.
    if (pr.apical != 0 && dna.module(pr.dst).apical_threshold > 0.0f) {
      apical_pair_[pr.src][pr.dst] = true;
      any_apical_ = true;
    }
  }
  // DNA v25 per-module compartment parameters. Only read when any_apical_.
  for (uint32_t m = 0; m < module_count_ && m < kMaxModules; ++m) {
    const DnaModule& dm = dna.module(m);
    apical_leak_[m] = dm.apical_tau_ms > 0.0f
                          ? clampf(dt_ms_ / Scalar(dm.apical_tau_ms), kZero, kOne)
                          : kOne;
    apical_thresh_[m] = Scalar(dm.apical_threshold);
    apical_gain_[m] = Scalar(dm.apical_gain);
    apical_ticks_[m] = uint32_t(Scalar(dm.apical_plateau_ms) / dt_ms_ + Scalar(0.5));
    if (apical_ticks_[m] == 0) apical_ticks_[m] = 1;
    // DNA v29. The gate is armed only where a plateau can actually happen: the
    // module needs a compartment *and* a tract landing on it. A compartment
    // with no afferent never crosses threshold, so the gate would shut
    // permanently and multiply all learning by (1 - gate) — a mechanism that
    // switches learning off while looking like it ran. `dna.cpp` rejects that
    // genome outright; this is the same defence a second time.
    bool fed = false;
    for (uint32_t src = 0; src < module_count_ && src < kMaxModules; ++src) {
      if (apical_pair_[src][m]) { fed = true; break; }
    }
    plateau_gate_[m] = fed ? clampf(Scalar(dm.plateau_gate), kZero, kOne) : kZero;
    if (plateau_gate_[m] > kZero) any_plateau_gate_ = true;
  }
  // DNA v26 oscillations. The phase increment is cycles-per-tick scaled to a
  // full uint32 turn: at 1 kHz and 6 Hz theta that is 0.006 of a turn per tick,
  // and the truncation to an integer costs less than a millionth of a cycle.
  {
    const double two_pi = 6.283185307179586;
    for (uint32_t i = 0; i < kSinSize; ++i) {
      sin_table_[i] = Scalar(sinf(float(two_pi * double(i) / double(kSinSize))));
    }
    const double turn = 4294967296.0;  // 2^32
    for (uint32_t m = 0; m < module_count_ && m < kMaxModules; ++m) {
      const DnaModule& dm = dna.module(m);
      const double per_tick = double(dt_ms_) / 1000.0;
      osc_theta_inc_[m] = uint32_t(uint64_t(double(dm.theta_hz) * per_tick * turn));
      osc_gamma_inc_[m] = uint32_t(uint64_t(double(dm.gamma_hz) * per_tick * turn));
      osc_theta_amp_[m] = Scalar(dm.theta_amp);
      osc_gamma_amp_[m] = Scalar(dm.gamma_amp);
      osc_coupling_[m] = clampf(Scalar(dm.gamma_theta_coupling), kZero, kOne);
      if (dm.theta_amp != 0.0f || dm.gamma_amp != 0.0f) any_osc_ = true;
      if (dm.critical_tau_ms > 0.0f) any_critical_ = true;
    }
  }
  elig_mean_alpha_ = h.stdp.elig_baseline_tau_ms > 0.0f
                         ? (interval_ms / Scalar(h.stdp.elig_baseline_tau_ms) > kOne
                                ? kOne
                                : interval_ms / Scalar(h.stdp.elig_baseline_tau_ms))
                         : kZero;
  // Traffic is the myelination counter (§3.5), read by myelinate() and by the
  // pruning half of consolidate().
  traffic_decay_ = decay_per(interval_ms, Scalar(h.consolidate.traffic_tau_ms));
  inhib_plastic_ = h.homeo.inhib_plastic != 0;
  structural_ = StructuralStats{};

  // Lay modules out contiguously at full n_max so growth appends in place.
  capacity_ = 0;
  synapse_pool_ = 0;
  for (uint32_t m = 0; m < module_count_; ++m) {
    const DnaModule& dm = dna.module(m);
    modules_[m].begin = capacity_;
    modules_[m].count = dm.neurons;
    modules_[m].capacity = dm.n_max;
    modules_[m].spikes = 0;
    modules_[m].dead = 0;
    modules_[m].mean_rate = kZero;
    modules_[m].mean_rate_fast = kZero;
    modules_[m].norm = kOne;  // no division until a tick has measured something
    capacity_ += dm.n_max;
    synapse_pool_ += dm.n_max * dm.max_out_degree;
  }

  // DNA v32. Lateral competition. The width is kept as the fraction the genome
  // states rather than as a filter coefficient: a module's neuron count can
  // grow (§3.4), and a coefficient baked at birth would quietly describe a
  // field narrower than the one it runs over. It is turned into a one-pole per
  // field in step(), which costs one divide per field per tick.
  for (uint32_t m = 0; m < module_count_ && m < kMaxModules; ++m) {
    const DnaModule& dm = dna.module(m);
    lateral_gain_[m] = kZero;
    lateral_sigma_[m] = kZero;
    lateral_fields_[m] = 1;
    if (!(dm.lateral_gain > 0.0f)) continue;
    lateral_gain_[m] = Scalar(dm.lateral_gain);
    lateral_sigma_[m] = Scalar(dm.lateral_sigma);
    lateral_fields_[m] = dm.lateral_fields > 1 ? dm.lateral_fields : 1;
    any_lateral_ = true;
  }

  v_ = arena.alloc_zeroed<Scalar>(capacity_);
  v_rest_ = arena.alloc_zeroed<Scalar>(capacity_);
  threshold_ = arena.alloc_zeroed<Scalar>(capacity_);
  target_rate_ = arena.alloc_zeroed<Scalar>(capacity_);
  leak_alpha_ = arena.alloc_zeroed<Scalar>(capacity_);
  noise_amp_ = arena.alloc_zeroed<Scalar>(capacity_);
  rate_ema_ = arena.alloc_zeroed<Scalar>(capacity_);
  rate_fast_ = arena.alloc_zeroed<Scalar>(capacity_);
  pos_x_ = arena.alloc_zeroed<Scalar>(capacity_);
  pos_y_ = arena.alloc_zeroed<Scalar>(capacity_);
  pos_z_ = arena.alloc_zeroed<Scalar>(capacity_);
  perturb_ = arena.alloc_zeroed<Scalar>(capacity_);
  bias_ = arena.alloc_zeroed<Scalar>(capacity_);
  trace_pre_ = arena.alloc_zeroed<Scalar>(capacity_);
  trace_post_ = arena.alloc_zeroed<Scalar>(capacity_);
  w_in_target_ = arena.alloc_zeroed<Scalar>(capacity_);
  w_in_struct_ = arena.alloc_zeroed<Scalar>(capacity_);
  ffi_w_ = arena.alloc_zeroed<Scalar>(capacity_);
  v_apical_ = arena.alloc_zeroed<Scalar>(capacity_);
  refrac_until_ = arena.alloc_zeroed<uint32_t>(capacity_);
  last_spike_ = arena.alloc_zeroed<uint32_t>(capacity_);
  plateau_until_ = arena.alloc_zeroed<uint32_t>(capacity_);
  syn_base_ = arena.alloc_zeroed<uint32_t>(capacity_);
  syn_count_ = arena.alloc_zeroed<uint16_t>(capacity_);
  syn_cap_ = arena.alloc_zeroed<uint16_t>(capacity_);
  in_count_ = arena.alloc_zeroed<uint16_t>(capacity_);
  refrac_ticks_ = arena.alloc_zeroed<uint16_t>(capacity_);
  module_of_ = arena.alloc_zeroed<uint8_t>(capacity_);
  is_inhib_ = arena.alloc_zeroed<uint8_t>(capacity_);
  dead_ = arena.alloc_zeroed<uint8_t>(capacity_);

  syn_target_ = arena.alloc_zeroed<uint32_t>(synapse_pool_);
  syn_source_ = arena.alloc_zeroed<uint32_t>(synapse_pool_);
  syn_in_ = arena.alloc_zeroed<uint32_t>(synapse_pool_);
  syn_weight_ = arena.alloc_zeroed<Scalar>(synapse_pool_);
  syn_elig_ = arena.alloc_zeroed<Scalar>(synapse_pool_);
  syn_elig_mean_ = arena.alloc_zeroed<Scalar>(synapse_pool_);
  syn_traffic_ = arena.alloc_zeroed<Scalar>(synapse_pool_);
  syn_delay_ = arena.alloc_zeroed<uint16_t>(synapse_pool_);
  syn_delay0_ = arena.alloc_zeroed<uint16_t>(synapse_pool_);
  if (any_lateral_) lateral_ = arena.alloc_zeroed<Scalar>(capacity_);

  inbox_ = arena.alloc_zeroed<Scalar>(size_t(capacity_) * delay_slots_);
  inbox_apical_ = arena.alloc_zeroed<Scalar>(size_t(capacity_) * delay_slots_);
  spike_list_ = arena.alloc_zeroed<uint32_t>(capacity_);

  if (!arena.ok()) return false;

  // Per-neuron parameters and positions. Positions are drawn for the full
  // capacity, not just the live count, so that a neuron grown in M4 already
  // has a home and the wiring rules stay reproducible.
  uint32_t syn_cursor = 0;
  for (uint32_t m = 0; m < module_count_; ++m) {
    const DnaModule& dm = dna.module(m);
    for (uint32_t k = 0; k < dm.n_max; ++k) {
      const uint32_t i = modules_[m].begin + k;
      module_of_[i] = uint8_t(m);
      v_rest_[i] = Scalar(dm.v_rest);
      v_[i] = Scalar(dm.v_rest);
      threshold_[i] = Scalar(dm.threshold);
      target_rate_[i] = Scalar(dm.target_rate_hz);
      noise_amp_[i] = Scalar(dm.noise_amp);
      leak_alpha_[i] = clampf(dt_ms_ / Scalar(dm.leak_tau_ms), kZero, kOne);
      refrac_ticks_[i] = uint16_t(Scalar(dm.refractory_ms) / dt_ms_ + Scalar(0.5));
      pos_x_[i] = rng.uniform() * Scalar(dm.extent[0]);
      pos_y_[i] = rng.uniform() * Scalar(dm.extent[1]);
      pos_z_[i] = rng.uniform() * Scalar(dm.extent[2]);
      is_inhib_[i] = rng.chance(Scalar(dm.inhib_fraction)) ? 1 : 0;
      syn_base_[i] = syn_cursor;
      syn_cap_[i] = uint16_t(dm.max_out_degree);
      syn_count_[i] = 0;
      in_count_[i] = 0;
      syn_cursor += dm.max_out_degree;
    }
  }

  for (uint32_t m = 0; m < kMaxModules; ++m) explore_mult_[m] = kOne;
  drive_comp_ = Scalar(dna.header().exploration.drive_compensation);
  {
    const DnaExploration& ex = dna.header().exploration;
    perturb_decay_ = decay_per(dt_ms_, Scalar(ex.perturb_tau_ms));
    perturb_rate_ = ex.enabled ? Scalar(ex.perturb_rate) : kZero;
    perturb_max_ = Scalar(ex.perturb_max);
    for (uint32_t m = 0; m < kMaxModules; ++m) {
      perturb_scale_[m] = m < dna.module_count() ? Scalar(dna.module(m).explore_scale) : kZero;
    }
  }

  for (uint32_t m = 0; m < kMaxModules; ++m) {
    eta_scale_[m] = m < dna.module_count() ? Scalar(dna.module(m).eta_scale) : kOne;
  }

  {
    const DnaNormalisation& nm = dna.header().normalisation;
    norm_floor_ = Scalar(nm.floor);
    norm_ceiling_ = Scalar(nm.ceiling);
    for (uint32_t m = 0; m < kMaxModules; ++m) {
      const bool have = nm.enabled && m < dna.module_count();
      norm_gain_[m] = have ? Scalar(dna.module(m).norm_gain) : kZero;
      norm_target_[m] = have ? Scalar(dna.module(m).target_rate_hz) : kZero;
    }
  }

  for (uint32_t m = 0; m < module_count_; ++m) wire_intra_module(m, rng);
  for (uint32_t p = 0; p < dna.projection_count(); ++p) {
    wire_projection(dna.projection(p), rng);
  }
  build_reverse_index();
  return true;
}

// The largest magnitude a synapse leaving `src` is allowed to reach. Dale's
// law again: excitatory synapses live in [0, w_max], inhibitory ones in
// [-w_max*gain, 0], and nothing ever crosses zero.
// Whether a projection may recruit this presynaptic neuron (DNA v14).
//
// A source filter rather than a forced weight sign: apply_reward takes a
// synapse's clamp bounds from is_inhib_ of its presynaptic neuron, so a
// negative weight hanging off an excitatory cell would be clamped to [0, ceil]
// and driven back across zero by the first reward that arrived.
bool Network::source_allowed(const DnaProjection& p, uint32_t src_neuron) const {
  switch (ProjectionSource(p.source)) {
    case ProjectionSource::kExcitatory: return !is_inhib_[src_neuron];
    case ProjectionSource::kInhibitory: return is_inhib_[src_neuron];
    default: return true;
  }
}

Scalar Network::weight_ceiling(uint32_t src) const {
  const Scalar w_max = Scalar(dna_.header().homeo.w_max);
  if (!is_inhib_[src]) return w_max;
  return w_max * Scalar(dna_.module(module_of_[src]).inhib_gain);
}

void Network::rebuild_reverse_index() {
  // Every forward synapse gets exactly one reverse entry, and the reverse
  // pool is the same size as the forward one, so the only way this overflows
  // is a neuron with more incoming than max_out_degree allows.
  //
  // Recomputed from scratch rather than patched, because pruning moves
  // surviving synapses into the slots the dead ones vacated and every reverse
  // entry is a slot index. The drop counters are recomputed with it: they
  // describe the structure that exists now, and after a prune that is a
  // different structure.
  for (uint32_t i = 0; i < capacity_; ++i) in_count_[i] = 0;
  dropped_reverse_ = 0;
  for (uint32_t m = 0; m < kMaxModules; ++m) dropped_reverse_by_module_[m] = 0;

  for (uint32_t m = 0; m < module_count_; ++m) {
    const ModuleState& ms = modules_[m];
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t i = ms.begin + k;
      if (dead_[i]) continue;
      const uint32_t base = syn_base_[i];
      for (uint32_t s = 0; s < syn_count_[i]; ++s) {
        const uint32_t syn = base + s;
        const uint32_t j = syn_target_[syn];
        if (in_count_[j] >= syn_cap_[j]) {
          ++dropped_reverse_;
          ++dropped_reverse_by_module_[module_of_[j]];
          continue;
        }
        syn_in_[syn_base_[j] + in_count_[j]] = syn;
        ++in_count_[j];
      }
    }
  }
}

// DNA v24. Per-neuron weight for the pooling interneurons, proportional to how
// much of the source module that neuron actually receives, normalised so the
// mean is 1 and `ffi_gain` keeps the scale it had in v21/v22.
//
// v22 subtracted the same amount from every target and did not work. The
// algebra says why: projprobe's `centred` arm computes
// sum_{i in S_j}(x_i - m) = sum(x_i) - |S_j| * m, and a uniform term computes
// sum(x_i) - g * m. Those agree only if every target has the same fan-in, and
// at density 0.03 over ~1024 sources |S_j| is Poisson with mean 31 and spread
// +/-5.5 — an 18% variation whose residual scales with the very common mode the
// term is meant to remove. A real basket cell innervates in proportion to its
// target's excitatory input rather than equally across the population.
void Network::capture_ffi_weights() {
  for (uint32_t m = 0; m < module_count_; ++m) {
    const ModuleState& ms = modules_[m];
    const int32_t src = dna_.module(m).ffi_source;
    if (src < 0 || uint32_t(src) >= module_count_) {
      for (uint32_t k = 0; k < ms.count; ++k) ffi_w_[ms.begin + k] = kZero;
      continue;
    }
    Scalar total = kZero;
    uint32_t counted = 0;
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t j = ms.begin + k;
      Scalar n = kZero;
      for (uint32_t t = 0; t < in_count_[j]; ++t) {
        const uint32_t syn = syn_in_[syn_base_[j] + t];
        if (module_of_[syn_source_[syn]] == uint32_t(src)) n += kOne;
      }
      ffi_w_[j] = n;
      total += n;
      ++counted;
    }
    // Normalise to mean 1. A module with no afferents from the source gets
    // zero everywhere, which disables the term rather than dividing by zero.
    const Scalar mean = counted > 0 ? total / Scalar(counted) : kZero;
    for (uint32_t k = 0; k < ms.count; ++k) {
      ffi_w_[ms.begin + k] = mean > kZero ? ffi_w_[ms.begin + k] / mean : kZero;
    }
  }
}

// Synaptic scaling holds each neuron near the total drive it was born with,
// rather than near a number someone typed into the genome. Self-calibrating,
// and the setpoint survives any rewiring the DNA describes.
//
// Called once, at birth. Growth and pruning adjust the setpoints by the weight
// they add or remove instead of recapturing them: recapturing would re-centre
// the band on whatever the creature had just learned, which is precisely the
// deviation from birth that scaling exists to bound.
void Network::capture_scaling_setpoints() {
  for (uint32_t m = 0; m < module_count_; ++m) {
    const ModuleState& ms = modules_[m];
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t j = ms.begin + k;
      Scalar sum = kZero;
      for (uint32_t s = 0; s < in_count_[j]; ++s) {
        sum += absf(syn_weight_[syn_in_[syn_base_[j] + s]]);
      }
      w_in_target_[j] = sum;
      w_in_struct_[j] = sum;
    }
  }
}

void Network::build_reverse_index() {
  rebuild_reverse_index();
  capture_scaling_setpoints();
  capture_ffi_weights();
}

uint16_t Network::delay_ticks(Scalar ms) const {
  Scalar t = ms / dt_ms_;
  // Delay must be at least one tick: a zero-tick delay would deliver into the
  // slot the tick loop is currently draining, and the spike would vanish.
  uint32_t ticks = uint32_t(t + Scalar(0.5));
  if (ticks < 1) ticks = 1;
  if (ticks > delay_slots_ - 1) ticks = delay_slots_ - 1;
  return uint16_t(ticks);
}

bool Network::add_synapse(uint32_t src, uint32_t dst, Scalar weight, uint16_t delay) {
  if (syn_count_[src] >= syn_cap_[src]) {
    ++dropped_synapses_;
    ++dropped_by_module_[module_of_[src]];
    return false;
  }
  const uint32_t slot = syn_base_[src] + syn_count_[src];
  syn_target_[slot] = dst;
  syn_source_[slot] = src;
  syn_weight_[slot] = weight;
  syn_delay_[slot] = delay;
  syn_delay0_[slot] = delay;
  syn_elig_[slot] = kZero;
  syn_traffic_[slot] = kZero;
  ++syn_count_[src];
  return true;
}

void Network::wire_intra_module(uint32_t m, Rng& rng) {
  const DnaModule& dm = dna_.module(m);
  const ModuleState& ms = modules_[m];
  const Scalar radius = Scalar(dm.conn_radius);
  const Scalar inv_radius = kOne / radius;
  const Scalar velocity = Scalar(dna_.header().sim.conduction_velocity);

  for (uint32_t a = 0; a < ms.count; ++a) {
    const uint32_t i = ms.begin + a;
    for (uint32_t b = 0; b < ms.count; ++b) {
      if (a == b) continue;
      const uint32_t j = ms.begin + b;
      const Scalar dx = pos_x_[i] - pos_x_[j];
      const Scalar dy = pos_y_[i] - pos_y_[j];
      const Scalar dz = pos_z_[i] - pos_z_[j];
      const Scalar d2 = dx * dx + dy * dy + dz * dz;
      if (d2 >= radius * radius) continue;  // hard cutoff, so wiring stays sparse

      const Scalar d = sqrtf(d2);
      // Linear falloff with finite support rather than an exponential: it
      // guarantees no long-range stragglers, and it is one multiply in a
      // fixed-point port.
      const Scalar p = Scalar(dm.conn_density) * (kOne - d * inv_radius);
      if (!rng.chance(p)) continue;

      Scalar w = Scalar(dm.weight_init) * (kOne + Scalar(0.3) * rng.normal());
      if (w < kZero) w = kZero;
      if (is_inhib_[i]) w = -w * Scalar(dm.inhib_gain);

      add_synapse(i, j, w, delay_ticks(d / velocity));
    }
  }
}

// A retinotopic, orientation-selective map from the retina onto a cortical
// module: the Hubel–Wiesel simple cell, built the way one is actually built.
//
// Every projection before DNA v7 connected a random `density` of pairs, and
// that is why the association module could never tell a cube from a ball. A
// random projection is a mixer — it can carry a pattern through, attenuated,
// but the categories it hands downstream are the categories its input already
// had. Measurement said so precisely: shape was legible in the retina's spike
// timing at 0.70 and in B1's at chance, and neither more density nor a faster
// membrane moved it. What was missing was not bandwidth but a stage that makes
// *orientation* an explicit thing a neuron can be about.
//
// The target's own position in its module volume is read as the whole of its
// receptive field, which is §3.2's spatial premise taken seriously:
//
//   x, y  -> where on the retina this cell looks
//   z     -> its preferred orientation, over [0, pi)
//
// Cortex does something very close to this — retinotopy across the surface,
// orientation turning through pinwheels — and it buys two things for free.
// Intra-module wiring is distance-based, so V1 neurons that are near in *all
// three* axes are wired together, which is exactly the iso-orientation
// horizontal connectivity of real V1. And a neuron grown at a coordinate would
// inherit a sensible field rather than a random one, if V1 were ever allowed to
// grow (it is not — see growable()).
//
// The sign structure of the field lives in ON/OFF channel identity rather than
// in the sign of any synapse: the positive lobe recruits ON cells, the negative
// lobe OFF cells, and both *excite* the simple cell. That is what a real simple
// cell does, and it is the only arrangement that survives Dale's law.
void Network::wire_projection_gabor(const DnaProjection& p, Rng& rng) {
  const ModuleState& src = modules_[p.src];
  const ModuleState& dst = modules_[p.dst];
  const DnaModule& dst_dna = dna_.module(p.dst);
  const DnaVision& vis = dna_.header().vision;

  const uint32_t features = vision_features(vis);
  const uint32_t cells = vision_cells(vis);
  if (features == 0 || cells == 0 || src.count == 0 || dst.count == 0) return;

  const Scalar aspect2 = Scalar(p.rf_aspect) * Scalar(p.rf_aspect);
  const Scalar floor_amp = Scalar(p.rf_floor);
  const Scalar mag = Scalar(p.rf_magnification);

  for (uint32_t b = 0; b < dst.count; ++b) {
    const uint32_t j = dst.begin + b;

    // Position and preferred orientation, read straight off the volume, with
    // the field's centre pulled toward the fovea by the magnification factor.
    const Scalar uj = magnify(pos_x_[j] / Scalar(dst_dna.extent[0]), mag);
    const Scalar vj = magnify(pos_y_[j] / Scalar(dst_dna.extent[1]), mag);
    const Scalar theta = Scalar(3.14159265) * pos_z_[j] / Scalar(dst_dna.extent[2]);
    const Scalar ct = Scalar(cosf(float(theta)));
    const Scalar st = Scalar(sinf(float(theta)));

    // The sampling pitch where this cell is looking, taken from the nearest
    // ganglion cell. This is what makes the field eccentricity-invariant: the
    // same simple cell in the fovea and in the outer ring covers the same
    // number of inputs, over eight times the visual angle.
    Scalar pitch = kZero;
    Scalar best = Scalar(1e9);
    for (uint32_t c = 0; c < cells; ++c) {
      const VisionCell g = vision_cell(vis, c);
      const Scalar dx = Scalar(g.u) - uj;
      const Scalar dy = Scalar(g.v) - vj;
      const Scalar d2 = dx * dx + dy * dy;
      if (d2 < best) { best = d2; pitch = Scalar(g.pitch); }
    }
    if (pitch <= kZero) continue;

    const Scalar sigma = Scalar(p.rf_sigma) * pitch;
    const Scalar inv_2sigma2 = kOne / (Scalar(2) * sigma * sigma);
    const Scalar k_lambda = Scalar(6.2831853) / (Scalar(p.rf_lambda) * pitch);
    // Past three sigma the envelope contributes less than a part in ten
    // thousand; testing the whole retina against every target would build the
    // same map at several times the cost.
    const Scalar reach2 = Scalar(9) * sigma * sigma;

    for (uint32_t a = 0; a < src.count; ++a) {
      const uint32_t i = src.begin + a;
      // Long-range projections are excitatory. The inhibitory fraction of a
      // sensory module is local circuitry, and letting it into the field would
      // sign-flip a fifth of the receptive field and scale it by inhib_gain —
      // which is not a noisier simple cell, it is a different one.
      if (is_inhib_[i]) continue;

      const uint32_t f = slice_of(src.count, features, a);
      const VisionCell cell = vision_cell(vis, f / 2);
      if (cell.pitch <= kZero) continue;

      const Scalar dx = Scalar(cell.u) - uj;
      const Scalar dy = Scalar(cell.v) - vj;
      if (dx * dx + dy * dy > reach2) continue;

      // Into the cell's frame: x' across the preferred orientation (the axis
      // the grating runs along), y' parallel to it.
      const Scalar xr = dx * ct + dy * st;
      const Scalar yr = -dx * st + dy * ct;
      const Scalar env = Scalar(expf(-float((xr * xr + aspect2 * yr * yr) * inv_2sigma2)));
      const Scalar lobe = env * Scalar(cosf(float(k_lambda * xr)));

      // Feature parity is the ON/OFF channel, exactly as the host packs it.
      const bool on_cell = (f % 2) == 0;
      const Scalar amp = on_cell ? lobe : -lobe;
      if (amp < floor_amp) continue;
      if (!rng.chance(birth_density(p))) continue;

      Scalar w = birth_weight(p) * amp * (kOne + Scalar(0.3) * rng.normal());
      if (w < kZero) w = kZero;

      const Scalar jitter = Scalar(p.delay_jitter_ms) * rng.signed_uniform();
      Scalar ms = Scalar(p.delay_ms) + jitter;
      if (ms < kZero) ms = kZero;
      add_synapse(i, j, w, delay_ticks(ms));
    }
  }
}

// The conjunction stage: cells selective for the *radius of a curve*, built
// over the oriented cells V1 supplies.
//
// DNA v7 made orientation explicit and measurement said plainly that this was
// not enough — V1 carried 0.567 of the cube-versus-ball distinction where the
// retina carried 0.993. The reason is not a defect in V1. Area-matched, a
// square and a disc present the same orientations in the same amounts; a
// histogram of local edge orientation cannot tell them apart even in principle.
// What differs is arrangement, and the cheapest arrangement that separates
// these two toys is curvature.
//
// So a form cell asks one question of its neighbourhood: are these edges
// tangent to a common circle of radius r? Every point on a disc's boundary
// answers yes at once, which is what makes the response enormous and
// unambiguous. A square's boundary is tangent to that circle at four points and
// runs away from it everywhere else. This is a radial-frequency cell, which is
// a V4 property rather than a V2 one — the hierarchy position is V2's and the
// computation is V4's, and the role is named for the computation.
//
// The target's coordinates carry the whole parameterisation again:
//
//   x, y  -> where on the retina the circle is centred
//   z     -> its radius, over [rf_radius_min, rf_radius_max]
//
// No reference orientation is needed and that is the point of choosing this
// template over an angle detector: tangency is rotation-invariant, so three
// coordinates are enough and a corner is detected however it is turned.
void Network::wire_projection_curvature(const DnaProjection& p, Rng& rng) {
  const ModuleState& src = modules_[p.src];
  const ModuleState& dst = modules_[p.dst];
  const DnaModule& src_dna = dna_.module(p.src);
  const DnaModule& dst_dna = dna_.module(p.dst);
  const DnaVision& vis = dna_.header().vision;
  if (src.count == 0 || dst.count == 0) return;

  // V1's map is defined by the gabor projection that built it, so this one has
  // to read the same parameters rather than assume them. A form cortex wired
  // over a differently-magnified V1 than the one that exists is a cortex
  // looking at the wrong part of the visual field, and nothing downstream would
  // report it.
  const DnaProjection* fromv1 = nullptr;
  for (uint32_t i = 0; i < dna_.projection_count(); ++i) {
    const DnaProjection& q = dna_.projection(i);
    if (q.kind == uint32_t(ProjectionKind::kGabor) && q.dst == p.src) fromv1 = &q;
  }
  if (!fromv1) return;  // the genome validator refuses this case

  const uint32_t cells = vision_cells(vis);
  const Scalar fovea_pitch = cells ? Scalar(vision_cell(vis, 0).pitch) : Scalar(0.03);
  const Scalar sigma_r = Scalar(p.rf_sigma) * fovea_pitch;
  const Scalar inv_2sr2 = kOne / (Scalar(2) * sigma_r * sigma_r);
  const Scalar sigma_t = Scalar(p.rf_tangent_sigma);
  const Scalar inv_2st2 = kOne / (Scalar(2) * sigma_t * sigma_t);
  const Scalar floor_amp = Scalar(p.rf_floor);
  const Scalar half_pi = Scalar(1.57079633);
  const Scalar pi = Scalar(3.14159265);

  for (uint32_t b = 0; b < dst.count; ++b) {
    const uint32_t j = dst.begin + b;
    const Scalar uj = magnify(pos_x_[j] / Scalar(dst_dna.extent[0]),
                              Scalar(p.rf_magnification));
    const Scalar vj = magnify(pos_y_[j] / Scalar(dst_dna.extent[1]),
                              Scalar(p.rf_magnification));
    const Scalar zt = pos_z_[j] / Scalar(dst_dna.extent[2]);
    const Scalar radius =
        Scalar(p.rf_radius_min) + zt * (Scalar(p.rf_radius_max) - Scalar(p.rf_radius_min));
    if (radius <= kZero) continue;

    for (uint32_t a = 0; a < src.count; ++a) {
      const uint32_t i = src.begin + a;
      if (is_inhib_[i]) continue;  // long-range projections are excitatory

      // Where this V1 cell looks and what it is tuned to — the same map its own
      // afferents were built from.
      const Scalar ui = magnify(pos_x_[i] / Scalar(src_dna.extent[0]),
                                Scalar(fromv1->rf_magnification));
      const Scalar vi = magnify(pos_y_[i] / Scalar(src_dna.extent[1]),
                                Scalar(fromv1->rf_magnification));
      const Scalar theta_i = pi * pos_z_[i] / Scalar(src_dna.extent[2]);

      const Scalar dx = ui - uj;
      const Scalar dy = vi - vj;
      const Scalar d = Scalar(sqrtf(float(dx * dx + dy * dy)));
      // Does it sit on the circle?
      const Scalar dr = d - radius;
      if (dr * dr * inv_2sr2 > Scalar(9)) continue;  // past three sigma
      const Scalar radial = Scalar(expf(-float(dr * dr * inv_2sr2)));

      // Is it pointing along it? The tangent to a circle at angular position
      // psi is psi + 90 degrees, and orientations wrap at 180.
      if (d <= kZero) continue;
      const Scalar psi = Scalar(atan2f(float(dy), float(dx)));
      Scalar diff = theta_i - (psi + half_pi);
      while (diff < -half_pi) diff += pi;
      while (diff > half_pi) diff -= pi;
      const Scalar tangential = Scalar(expf(-float(diff * diff * inv_2st2)));

      const Scalar amp = radial * tangential;
      if (amp < floor_amp) continue;
      if (!rng.chance(birth_density(p))) continue;

      Scalar w = birth_weight(p) * amp * (kOne + Scalar(0.3) * rng.normal());
      if (w < kZero) w = kZero;
      const Scalar jitter = Scalar(p.delay_jitter_ms) * rng.signed_uniform();
      Scalar ms = Scalar(p.delay_ms) + jitter;
      if (ms < kZero) ms = kZero;
      add_synapse(i, j, w, delay_ticks(ms));
    }
  }
}

void Network::wire_projection(const DnaProjection& p, Rng& rng) {
  if (p.kind == uint32_t(ProjectionKind::kGabor)) {
    wire_projection_gabor(p, rng);
    return;
  }
  if (p.kind == uint32_t(ProjectionKind::kCurvature)) {
    wire_projection_curvature(p, rng);
    return;
  }

  const ModuleState& src = modules_[p.src];
  const ModuleState& dst = modules_[p.dst];
  const DnaModule& src_dna = dna_.module(p.src);

  for (uint32_t a = 0; a < src.count; ++a) {
    const uint32_t i = src.begin + a;
    for (uint32_t b = 0; b < dst.count; ++b) {
      // The coin is drawn first and the filter applied second, deliberately.
      // Skipping a source before its draws would consume fewer random numbers
      // and re-roll every projection wired after this one, so changing a
      // projection's `source` would silently change the whole rest of the
      // brain. This way the two arms of such a comparison differ only in which
      // synapses exist.
      if (!rng.chance(birth_density(p))) continue;
      if (!source_allowed(p, i)) continue;
      const uint32_t j = dst.begin + b;

      Scalar w = birth_weight(p) * (kOne + Scalar(0.3) * rng.normal());
      if (w < kZero) w = kZero;
      if (is_inhib_[i]) w = -w * Scalar(src_dna.inhib_gain);

      const Scalar jitter = Scalar(p.delay_jitter_ms) * rng.signed_uniform();
      Scalar ms = Scalar(p.delay_ms) + jitter;
      if (ms < kZero) ms = kZero;
      add_synapse(i, j, w, delay_ticks(ms));
    }
  }
}

void Network::inject(uint32_t neuron, Scalar current) {
  // Lands in the slot the next step() will drain.
  const uint32_t slot = uint32_t(tick_ % delay_slots_);
  inbox_[size_t(slot) * capacity_ + neuron] += current;
}

void Network::step() {
  const uint32_t slot = uint32_t(tick_ % delay_slots_);
  Scalar* in = inbox_ + size_t(slot) * capacity_;
  Scalar* in_ap = inbox_apical_ + size_t(slot) * capacity_;
  spike_count_ = 0;

  for (uint32_t m = 0; m < module_count_; ++m) {
    ModuleState& ms = modules_[m];
    ms.spikes = 0;
    Scalar rate_sum = kZero;
    Scalar fast_sum = kZero;
    Scalar pre_sum = kZero;

    // Divisive normalisation (§3.1, DNA v12). One factor for the whole module,
    // from how active it was on the previous tick relative to its own target.
    // The one-tick lag is deliberate: reading this tick's own activity would
    // need a second pass over every neuron, and the quantity is a one-second
    // EMA, so a tick of delay is far inside its own smoothing.
    //
    // At norm_gain == 0 this is exactly 1.0 and the multiply below is the
    // identity on every finite value, which is what keeps a v11 genome
    // bit-identical under a v12 core.
    Scalar norm = kOne;
    if (norm_gain_[m] > kZero && norm_target_[m] > kZero) {
      const Scalar excess = ms.mean_rate_fast / norm_target_[m];
      const Scalar denom = kOne + norm_gain_[m] * (excess - kOne);
      norm = denom > kZero ? kOne / denom : norm_ceiling_;
      norm = clampf(norm, norm_floor_, norm_ceiling_);
    }
    ms.norm = norm;

    // DNA v21. Pooling interneurons: a subtractive common-mode term, taken
    // from another module's population mean rate. Divisive normalisation above
    // rescales the drive; this shifts it, which is the difference between
    // preserving the ratio of signal to common mode and changing it. At
    // ffi_source < 0 this is exactly zero and costs one branch.
    // DNA v26. One oscillator evaluation for the whole module: the rhythm is a
    // network property, and every neuron here rides the same one. What differs
    // between neurons is their own synaptic drive, which is exactly what makes
    // the crossing *time* on a shared ramp a readout of drive.
    Scalar osc = kZero;
    if (any_osc_) {
      const uint32_t th_ph = uint32_t(uint64_t(tick_) * osc_theta_inc_[m]);
      const Scalar th = sin_at(th_ph);
      // Nesting: gamma rides the theta cycle, at full amplitude on the theta
      // peak and none in the trough when coupling is 1.
      const Scalar env =
          kOne - osc_coupling_[m] + osc_coupling_[m] * Scalar(0.5) * (kOne + th);
      const uint32_t g_ph = uint32_t(uint64_t(tick_) * osc_gamma_inc_[m]);
      osc = osc_theta_amp_[m] * th + osc_gamma_amp_[m] * env * sin_at(g_ph);
    }

    // DNA v32. Lateral competition, one pass per competitive field.
    //
    // Local excitation against the field's own mean: a neuron sitting in a
    // locally-active stretch of the field gets a push and everything else gets
    // a pull, which is what turns a wandering population average into a bump
    // that has somewhere to sit. The local average is a one-pole run forward
    // and then backward over the field's neuron indices — O(width), symmetric,
    // and DC gain one, so subtracting the field mean leaves a term that sums
    // to about zero and cannot act as a hidden excitatory bias.
    //
    // Read off `rate_fast_`, which is the same signal the vocal decoder reads,
    // for the same reason `ffi` reads `pool_fast_`: an interneuron here is a
    // rate approximation, and this is the rate the readout cares about.
    if (any_lateral_ && lateral_gain_[m] > kZero) {
      const uint32_t fields = lateral_fields_[m];
      for (uint32_t f = 0; f < fields; ++f) {
        const uint32_t fb = ms.begin + slice_begin(ms.count, fields, f);
        const uint32_t fe = ms.begin + slice_begin(ms.count, fields, f + 1);
        if (fe <= fb) continue;
        const uint32_t width = fe - fb;

        // sigma 0: competition WITHOUT local excitation.
        //
        // The version below it amplifies whatever neighbourhood is already
        // busiest, which is positive feedback and is where the fragility comes
        // from: a bump forms from noise as readily as from input, holds against
        // being moved, and lands wherever incidental asymmetries put it. Its G2
        // effect duly reverses sign twice over 20% of one unrelated constant.
        //
        // Subtracting the field's own mean has the opposite sign of feedback —
        // busier field, more subtraction — so it cannot run away, and it still
        // produces a place code, because a uniform subtraction against a spike
        // threshold leaves only the best-driven neurons firing. That is the
        // same trick `ffi` already uses; what is new is doing it per readout
        // group rather than per module, so each group competes with itself.
        //
        // The point of the difference is where the winner comes from. Here it
        // is whichever neurons the AFFERENTS drive hardest, so reward — which
        // changes afferent weights — has something to push.
        if (!(lateral_sigma_[m] > kZero)) {
          Scalar sum = kZero;
          for (uint32_t i = fb; i < fe; ++i) sum += rate_fast_[i];
          const Scalar suppress = lateral_gain_[m] * (sum / Scalar(width));
          for (uint32_t i = fb; i < fe; ++i) lateral_[i] = -suppress;
          continue;
        }

        // Read off the smoothed synaptic DRIVE, not off firing rate. The first
        // formulation used `rate_fast_` and was fragile in a specific, measured
        // way: its G2 effect reversed sign twice across 20% of one unrelated
        // scaling constant, because a term computed from a neuron's own output
        // is positive feedback — the winner is whoever got busy first, a bump
        // forms from noise as readily as from input, and once formed it resists
        // being moved. Reward changes afferent WEIGHTS, so a competition that
        // reads afferent drive is one reward can steer, and one with no loop in
        // it to destabilise.
        const Scalar span = lateral_sigma_[m] * Scalar(width);
        const Scalar a = span > kOne ? clampf(kOne / span, Scalar(1e-3), kOne) : kOne;
        Scalar acc = rate_fast_[fb];
        for (uint32_t i = fb; i < fe; ++i) {
          acc += a * (rate_fast_[i] - acc);
          lateral_[i] = acc;
        }
        acc = rate_fast_[fe - 1];
        Scalar sum = kZero;
        for (uint32_t i = fe; i-- > fb;) {
          acc += a * (lateral_[i] - acc);
          lateral_[i] = acc;
          sum += acc;
        }
        // Relative to the field's own mean, not an absolute rate difference.
        // The first version subtracted hertz from hertz and was unstable at
        // every gain tried, including the smallest: the term scales with the
        // rate it is derived from, so a field that starts firing harder
        // produces a bigger push to fire harder still. Vocal free-ran at 66 Hz
        // against 4.4 and the duty cycle pinned at 1.00.
        //
        // As a fraction of the mean it is a contrast — bounded by construction,
        // scale-free, and it says the thing the mechanism is actually about:
        // "this neighbourhood is busier than its field", not "by this many
        // hertz". Divisive normalisation upstairs already works in exactly
        // these units for the same reason.
        // Contrast rather than a difference of absolutes, for the reason the
        // very first version failed: a term that scales with the quantity it is
        // derived from amplifies itself. The scale here is the field's own mean
        // drive, and the guard keeps a silent field from dividing by nothing.
        const Scalar field_mean = sum / Scalar(width);
        const Scalar scale = field_mean > Scalar(1e-4) ? field_mean : Scalar(1e-4);
        const Scalar inv = kOne / scale;
        for (uint32_t i = fb; i < fe; ++i) {
          const Scalar contrast = clampf((lateral_[i] - field_mean) * inv,
                                         Scalar(-1), kOne);
          lateral_[i] = lateral_gain_[m] * contrast;
        }
      }
    }

    Scalar ffi = kZero;
    if (dna_.module(m).ffi_source >= 0) {
      const uint32_t src = uint32_t(dna_.module(m).ffi_source);
      if (src < module_count_) {
        ffi = Scalar(dna_.module(m).ffi_gain) * pool_fast_[src];
      }
    }

    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t i = ms.begin + k;
      // A tombstoned slot has no synapses in either direction, so it could
      // only spike into nothing — but it would still burn noise, accumulate a
      // rate, and drag the module mean down. Skipped outright.
      if (dead_[i]) {
        in[i] = kZero;
        if (any_apical_) in_ap[i] = kZero;
        continue;
      }

      // DNA v25. The apical tuft, integrated before the soma is. Its own leak,
      // its own threshold, and — the point of the whole thing — no path from
      // here to a spike. Crossing the threshold starts a plateau and resets the
      // compartment, which is what makes the plateau an event rather than a
      // level: a tuft held above threshold produces one plateau, not a plateau
      // every tick for as long as the input lasts.
      //
      // Two things v25 deliberately does NOT change, both worth knowing before
      // reading any apical result. Synaptic scaling sums a neuron's incoming
      // weights without regard to compartment, so an apical tract counts
      // against the same setpoint as a basal one — real cortex homeostats the
      // two pools separately, and this creature does not. And STDP still
      // accumulates eligibility on an apical synapse exactly as on any other:
      // v25 changes where input is *integrated*, and nothing about how it is
      // learned. Both are conservative choices, and both mean a genome that
      // moves a tract to the tuft changes one thing rather than three.
      Scalar apical_mult = kOne;
      if (any_apical_) {
        const Scalar th = apical_thresh_[m];
        if (th > kZero) {
          v_apical_[i] += apical_leak_[m] * (kZero - v_apical_[i]) + in_ap[i];
          if (tick_ >= plateau_until_[i] && v_apical_[i] >= th) {
            plateau_until_[i] = uint32_t(tick_) + apical_ticks_[m];
            v_apical_[i] = kZero;
          }
          if (tick_ < plateau_until_[i]) apical_mult = kOne + apical_gain_[m];
        }
        in_ap[i] = kZero;  // drained like the somatic slot, and for the same reason
      }

      // Decay the STDP traces before this tick's spikes are added to them, so
      // no spike is ever counted as having preceded itself.
      trace_pre_[i] *= pre_decay_;
      trace_post_[i] *= post_decay_;
      // DNA v17's centring term. Accumulated here rather than in a pass of its
      // own because `accumulate_eligibility` runs immediately after this loop
      // and reads the traces before bumping them, so this is exactly the value
      // the rule will see — not a lagged approximation of it.
      pre_sum += trace_pre_[i];

      // Exploration scales the *variance* and hands back what it removes as
      // steady drive (DnaExploration::drive_compensation), so closing it makes
      // the module predictable rather than mute. At explore_mult_ == 1 the
      // second term is exactly zero and this is the original expression.
      const Scalar xi = rng_->signed_uniform();
      // Normalisation divides the synaptic drive only. Noise is this creature's
      // motor source as much as its jitter (see DnaExploration), and bias is
      // node perturbation's learned excitability — dividing either by how busy
      // the module is would make it quieter rather than more selective.
      // `ffi` is subtracted from the synaptic drive only, alongside `norm`, and
      // for the same reason: it models inhibition arriving on the same
      // dendrites as the excitation it is cancelling, not a change to the
      // neuron's noise source or its learned excitability.
      //
      // DNA v25's plateau multiplies the synaptic term and nothing else, for
      // the same reason `ffi` is subtracted from it and nothing else: it is a
      // dendritic event, and what a dendrite can change is how strongly the
      // synapses on it are heard. Amplifying the noise would make an apical
      // plateau a source of spikes in a silent module, which is the one thing a
      // segregated compartment must never be.
      //
      // v26's oscillation is added outside the apical multiply and outside the
      // normalisation, alongside noise and bias. It is not synaptic drive — it
      // is the module's own rhythm arriving at every neuron equally, so
      // dividing it by how busy the module is, or amplifying it with a plateau,
      // would both be describing it as something it is not.
      // v32's lateral term joins noise, bias and the oscillation rather than
      // the synaptic sum, and for the same reason each of those does. Dividing
      // it by `norm` would double-count: divisive normalisation scales drive by
      // how busy the module is, and this term is *derived* from how busy the
      // neighbourhood is. Multiplying it by `apical_mult` would be worse — a
      // plateau changes how loudly the synapses on that tuft are heard, and a
      // lateral interneuron is not on the tuft.
      const Scalar lateral = any_lateral_ ? lateral_[i] : kZero;
      const Scalar drive = (in[i] * norm - ffi * ffi_w_[i]) * apical_mult +
                           noise_amp_[i] * (explore_mult_[m] * xi +
                                            drive_comp_ * (kOne - explore_mult_[m])) +
                           bias_[i] + osc + lateral;

      // Node perturbation: remember what this neuron was actually given, so
      // that a reward arriving a second from now can credit it. Decays on the
      // reward's own timescale, not the membrane's.
      perturb_[i] = perturb_[i] * perturb_decay_ + xi;
      in[i] = kZero;  // drain as we go; this slot is reused delay_slots_ ticks from now

      bool spiked = false;
      if (tick_ < refrac_until_[i]) {
        v_[i] = v_rest_[i];
      } else {
        v_[i] += leak_alpha_[i] * (v_rest_[i] - v_[i]) + drive;
        if (v_[i] >= threshold_[i]) {
          v_[i] = v_rest_[i];
          refrac_until_[i] = uint32_t(tick_) + refrac_ticks_[i];
          last_spike_[i] = uint32_t(tick_);
          spike_list_[spike_count_++] = i;
          ++ms.spikes;
          spiked = true;
        }
      }

      const Scalar inst = spiked ? spike_rate_unit_ : kZero;
      rate_ema_[i] += rate_alpha_ * (inst - rate_ema_[i]);
      rate_fast_[i] += rate_fast_alpha_ * (inst - rate_fast_[i]);
      rate_sum += rate_ema_[i];
      fast_sum += rate_fast_[i];
    }

    ms.mean_rate = ms.live() ? rate_sum / Scalar(ms.live()) : kZero;
    // The normalisation pool, on the motor readout's timescale rather than
    // homeostasis's. Cortical normalisation acts within tens of milliseconds;
    // pooled over a second it would be a second rate regulator, and intrinsic
    // plasticity is already that — it holds the slow mean *at* the target, so a
    // slow pool would find nothing to divide by and the mechanism would be
    // inert exactly where it is supposed to bite.
    ms.mean_rate_fast = ms.live() ? fast_sum / Scalar(ms.live()) : kZero;
    pre_trace_mean_[m] = ms.live() ? pre_sum / Scalar(ms.live()) : kZero;
    // This tick's spiking, in Hz, smoothed on the interneuron's own short
    // constant. `ms.spikes` is final by here, so the pool sees the volley that
    // is about to be pooled rather than a second-old average of it.
    {
      const Scalar per_neuron = ms.live() ? Scalar(ms.spikes) / Scalar(ms.live()) : kZero;
      const Scalar hz = per_neuron * (Scalar(1000.0) / Scalar(dna_.header().sim.dt_ms));
      pool_fast_[m] += pool_alpha_ * (hz - pool_fast_[m]);
    }
  }

  accumulate_eligibility();

  // Deliver into future slots. Every delay is >= 1 tick, so nothing lands in
  // the slot we just drained.
  for (uint32_t s = 0; s < spike_count_; ++s) {
    const uint32_t i = spike_list_[s];
    const uint32_t base = syn_base_[i];
    const uint32_t n = syn_count_[i];
    for (uint32_t k = 0; k < n; ++k) {
      const uint32_t syn = base + k;
      const uint32_t target_slot = uint32_t((tick_ + syn_delay_[syn]) % delay_slots_);
      // DNA v25. Which compartment this synapse terminates on. The `any_apical_`
      // test short-circuits the pair lookup on a genome with no apical tract,
      // which is what keeps the hottest loop in the kernel at its old cost.
      Scalar* ring = inbox_;
      if (any_apical_ &&
          apical_pair_[module_of_[i]][module_of_[syn_target_[syn]]]) {
        ring = inbox_apical_;
      }
      ring[size_t(target_slot) * capacity_ + syn_target_[syn]] += syn_weight_[syn];
      syn_traffic_[syn] += kOne;  // read by myelination in M4
    }
  }

  ++tick_;
}

// Three-factor learning, first two factors (§3.1). STDP writes to the
// eligibility trace and never to the weight:
//
//   pre fires, post fired recently  ->  e -= A- * trace_post   (depression)
//   post fires, pre fired recently  ->  e += A+ * trace_pre    (potentiation)
//
// Both halves are driven by this tick's spikes, so the cost is proportional
// to spikes rather than to synapses. Timing is read at the somas rather than
// at the synapse: folding the axonal delay into the STDP window too would be
// more faithful, but it needs a second delay line for traces and it is not
// what stands between us and G2.
void Network::accumulate_eligibility() {
  const DnaStdp& s = dna_.header().stdp;
  const Scalar a_plus = Scalar(s.a_plus);
  const Scalar a_minus = Scalar(s.a_minus);

  // DNA v29. The gate, as a multiplier on both halves of the pair. Read on the
  // *postsynaptic* neuron in both cases: the depression half is driven by a
  // presynaptic spike but the synapse it touches belongs to the target, and the
  // claim this mechanism makes is about which target neurons are plastic right
  // now. Gating only potentiation would leave depression running whenever the
  // tuft is quiet, which is a downward drift wearing a gate's clothes.
  auto gate = [&](uint32_t post) -> Scalar {
    if (!any_plateau_gate_) return kOne;
    const Scalar g = plateau_gate_[module_of_[post]];
    if (g <= kZero) return kOne;
    return tick_ < plateau_until_[post] ? kOne : kOne - g;
  };

  for (uint32_t k = 0; k < spike_count_; ++k) {
    const uint32_t i = spike_list_[k];
    const uint32_t base = syn_base_[i];  // forward and reverse share the slicing

    // As presynaptic: whatever we fire into that fired just *before* us was
    // not caused by us. Depress.
    const uint32_t out_n = syn_count_[i];
    for (uint32_t s2 = 0; s2 < out_n; ++s2) {
      const uint32_t syn = base + s2;
      const uint32_t post = syn_target_[syn];
      syn_elig_[syn] -= gate(post) * a_minus * trace_post_[post];
    }

    // As postsynaptic: whatever fired into us just before we fired may have
    // caused us. Potentiate.
    const Scalar post_gate = gate(i);
    // Counted here and only here, once per spiking neuron on a gated module,
    // so the rate reads as "of the moments a neuron fired and could have been
    // taught, how many was its tuft endorsing" — one number with one meaning.
    // Counting inside the depression loop as well would weight it by
    // out-degree and it would no longer be a fraction of anything.
    if (any_plateau_gate_ && plateau_gate_[module_of_[i]] > kZero) {
      ++plateau_events_;
      if (tick_ < plateau_until_[i]) ++plateau_hits_;
    }
    const uint32_t in_n = in_count_[i];
    for (uint32_t s2 = 0; s2 < in_n; ++s2) {
      const uint32_t syn = syn_in_[base + s2];
      // DNA v17: read the presynaptic trace as a deviation from its own
      // module's population mean rather than as an absolute. At
      // elig_pre_centre_ == 0 this subtracts exactly zero and the expression is
      // the v16 one.
      const uint32_t src = syn_source_[syn];
      const Scalar mean = pre_trace_mean_[module_of_[src]];
      const Scalar pre = trace_pre_[src] - elig_pre_centre_ * mean;
      syn_elig_[syn] += post_gate * a_plus * pre;
    }
  }

  // Traces are bumped only after both halves have read them.
  for (uint32_t k = 0; k < spike_count_; ++k) {
    const uint32_t i = spike_list_[k];
    trace_pre_[i] += kOne;
    trace_post_[i] += kOne;
  }
}

// DNA v20. Same pass as apply_reward, but the scalar is computed per module
// from the four channels and that module's own gains, so the caregiver can
// reach the larynx without hunger, comfort and curiosity riding along.
void Network::apply_reward_channels(const Scalar* channels) {
  const Scalar clip = Scalar(dna_.header().stdp.reward_clip);
  Scalar per_module[kMaxModules];
  bool any = false;
  for (uint32_t m = 0; m < module_count_; ++m) {
    const DnaModule& md = dna_.module(m);
    const Scalar v = md.nm_external * channels[0] + md.nm_hunger * channels[1] +
                     md.nm_comfort * channels[2] + md.nm_curiosity * channels[3];
    per_module[m] = clampf(v, -clip, clip);
    if (per_module[m] != kZero) any = true;
  }
  apply_reward_impl(per_module, any);
}

void Network::apply_reward(Scalar reward) {
  const Scalar clip = Scalar(dna_.header().stdp.reward_clip);
  const Scalar r = clampf(reward, -clip, clip);
  Scalar per_module[kMaxModules];
  for (uint32_t m = 0; m < module_count_; ++m) per_module[m] = r;
  apply_reward_impl(per_module, r != kZero);
}

// `any` is "is any channel non-zero anywhere", kept as a parameter so the
// common no-reward tick still short-circuits exactly as it did before v20.
void Network::apply_reward_impl(const Scalar* per_module, bool any) {
  const DnaStdp& s = dna_.header().stdp;
  const Scalar r = per_module[0];
  const Scalar eta = Scalar(s.eta);
  // §3.5's second effect: a well-trafficked edge learns more slowly, because a
  // pathway that is carrying the creature's behaviour is one we do not want
  // the next two seconds of reward to overwrite. Applied here rather than in
  // myelinate() because it is a property of the weight update, and folding it
  // into a stored per-edge rate would be a fourth array holding a number that
  // is a pure function of one we already have.
  const Scalar eta_floor = Scalar(dna_.header().consolidate.eta_floor_frac);
  const Scalar eta_span = kOne - eta_floor;
  last_reward_ = r;
  (void)any;

  // DNA v28. The critical period, folded into the per-module learning rate for
  // this cash-in. Computed from tick_ rather than carried as decaying state,
  // for the reason v26's oscillator phase is: a value derived from the tick is
  // exact across a snapshot and a resume, and costs no saved field. This runs
  // at the plasticity interval (100 Hz), not in the tick loop, which is why an
  // expf here does not break that file's standing invariant.
  Scalar critical[kMaxModules];
  for (uint32_t m = 0; m < module_count_; ++m) critical[m] = kOne;
  if (any_critical_) {
    const Scalar age_ms = Scalar(double(tick_) * double(dt_ms_));
    for (uint32_t m = 0; m < module_count_; ++m) {
      const DnaModule& md = dna_.module(m);
      if (md.critical_tau_ms <= 0.0f) continue;
      const Scalar f = clampf(Scalar(md.critical_floor), kZero, kOne);
      critical[m] = f + (kOne - f) * Scalar(expf(-float(age_ms / Scalar(md.critical_tau_ms))));
    }
  }

  // Node perturbation, cashed in on the same clock and against the same
  // centred reward as the synaptic traces. Each neuron's excitability moves
  // along its own recent perturbation: a random push that preceded a
  // better-than-expected outcome is kept, one that preceded a worse outcome is
  // undone. This is a *second* learning rule beside STDP, and it is the one
  // that can shape a motor act, because it needs no path back through the
  // vocal tract to know which way to move.
  if (perturb_rate_ > kZero) {
    for (uint32_t m = 0; m < module_count_; ++m) {
      const Scalar gate = perturb_scale_[m];
      if (gate <= kZero) continue;  // not a motor module
      const Scalar step = perturb_rate_ * gate * per_module[m];
      const ModuleState& ms = modules_[m];
      for (uint32_t k = 0; k < ms.count; ++k) {
        const uint32_t i = ms.begin + k;
        if (dead_[i]) continue;
        bias_[i] = clampf(bias_[i] + step * perturb_[i], -perturb_max_, perturb_max_);
      }
    }
  }

  for (uint32_t m = 0; m < module_count_; ++m) {
    const ModuleState& ms = modules_[m];
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t i = ms.begin + k;
      if (dead_[i]) continue;
      const uint32_t base = syn_base_[i];
      const uint32_t n = syn_count_[i];
      // Eligibility is sign-free spike timing, so potentiating an inhibitory
      // synapse has to make it more negative, not walk it toward excitation.
      if (is_inhib_[i] && !inhib_plastic_) {
        // Inhibition still decays its traces; it just does not learn.
        for (uint32_t k2 = 0; k2 < n; ++k2) {
          const Scalar e = syn_elig_[base + k2] * elig_decay_;
          syn_elig_[base + k2] = e;
          syn_traffic_[base + k2] *= traffic_decay_;
          if (elig_mean_alpha_ > kZero) {
            syn_elig_mean_[base + k2] += elig_mean_alpha_ * (e - syn_elig_mean_[base + k2]);
          }
        }
        continue;
      }
      const Scalar ceiling = weight_ceiling(i);
      const Scalar sign = is_inhib_[i] ? -kOne : kOne;
      const Scalar lo = is_inhib_[i] ? -ceiling : kZero;
      const Scalar hi = is_inhib_[i] ? kZero : ceiling;

      for (uint32_t k2 = 0; k2 < n; ++k2) {
        const uint32_t syn = base + k2;
        const Scalar e = syn_elig_[syn] * elig_decay_;
        syn_elig_[syn] = e;
        syn_traffic_[syn] *= traffic_decay_;
        // DNA v16: reward multiplies the *deviation* from this synapse's own
        // running mean, not the raw trace. The mean is updated whether or not
        // reward arrived, so it tracks what this synapse ordinarily does rather
        // than only what it does on rewarded intervals.
        Scalar credit = e;
        if (elig_mean_alpha_ > kZero) {
          credit = e - syn_elig_mean_[syn];
          syn_elig_mean_[syn] += elig_mean_alpha_ * (e - syn_elig_mean_[syn]);
        }
        // DNA v23: the reward-independent half, per pathway. Written every
        // interval, so coincidence alone can consolidate and a CS paired with a
        // US can bind without reward ever naming the object. Per pathway rather
        // than global because v19's global version potentiated every tract at
        // once, including the dense arcuate, and the creature droned at every
        // rate that did anything at all.
        const Scalar hebb_here =
            any_hebb_pair_ ? hebb_pair_[module_of_[syn_source_[syn]]]
                                       [module_of_[syn_target_[syn]]]
                           : kZero;
        if (hebb_here != kZero) {
          const Scalar eta_h = hebb_here * eta_scale_[module_of_[syn_target_[syn]]] *
                               critical[module_of_[syn_target_[syn]]] *
                               (eta_floor + eta_span * (kOne - myelination(syn)));
          syn_weight_[syn] = clampf(syn_weight_[syn] + eta_h * credit * sign, lo, hi);
        }
        const Scalar r_syn = per_module[module_of_[syn_target_[syn]]];
        if (r_syn != kZero) {
          // eta_scale is the *postsynaptic* module's, which is why it is read
          // through syn_target_ rather than taken from the loop's module: this
          // pass walks presynaptic neurons, but plasticity is gated on the
          // receiving side. See DnaModule::eta_scale.
          const Scalar eta_syn = eta * eta_scale_[module_of_[syn_target_[syn]]] *
                                 critical[module_of_[syn_target_[syn]]] *
                                 (eta_floor + eta_span * (kOne - myelination(syn)));
          syn_weight_[syn] =
              clampf(syn_weight_[syn] + eta_syn * credit * r_syn * sign, lo, hi);
        }
      }
    }
  }
}

// The caller knows whether the creature is asleep; each module knows how
// strongly it wants each of the two regulators to act in that state
// (DnaModule::ip_*_scale for intrinsic plasticity, syn_*_scale for synaptic
// scaling). With every module at 1.0 the arithmetic is unchanged and the brain
// is bit-identical to one built before the parameters existed, which is the
// property that makes any sweep over them readable.
//
// The two are looked up separately rather than from one number because they
// are asked for in opposite directions on the larynx: see DnaModule.
void Network::homeostasis(bool asleep) {
  const DnaHomeo& hm = dna_.header().homeo;
  const Scalar t_min = Scalar(hm.threshold_min);
  const Scalar t_max = Scalar(hm.threshold_max);
  const Scalar band = Scalar(hm.scaling_band) > kOne ? Scalar(hm.scaling_band) : kOne;

  for (uint32_t m = 0; m < module_count_; ++m) {
    const ModuleState& ms = modules_[m];
    const DnaModule& dm = dna_.module(m);
    const Scalar ip_scale = asleep ? Scalar(dm.ip_sleep_scale) : Scalar(dm.ip_wake_scale);
    const Scalar syn_scale = asleep ? Scalar(dm.syn_sleep_scale) : Scalar(dm.syn_wake_scale);
    // Unregulated by both in this state: nothing below would change anything.
    if (ip_scale <= kZero && syn_scale <= kZero) continue;
    const Scalar ip_rate = Scalar(hm.ip_rate) * ip_scale;
    const Scalar scaling = Scalar(hm.scaling_rate) * syn_scale;
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t i = ms.begin + k;
      if (dead_[i]) continue;

      // Intrinsic plasticity: firing above target raises the bar, below it
      // lowers the bar. This is what keeps a module alive when reward learning
      // quietly weakens all of its inputs.
      if (ip_scale > kZero) {
        threshold_[i] = clampf(threshold_[i] + ip_rate * (rate_ema_[i] - target_rate_[i]),
                               t_min, t_max);
      }
      if (syn_scale <= kZero) continue;  // scaling off: skip the afferent sweep

      // Synaptic scaling: keep total incoming |w| inside a band around the
      // birth setpoint, multiplicatively, so relative strengths — which is
      // where the learning lives — survive the correction.
      //
      // The band is the whole point. Pulling toward an exact setpoint
      // regulates precisely the quantity that reward-modulated learning moves,
      // and it regulates it faster than reward can change it, so the creature
      // ends up unable to learn anything that shows up as a change in drive.
      // Bounding it instead stops runaway without having an opinion about
      // anything inside the bounds.
      const Scalar target = w_in_target_[i];
      if (target <= kZero || in_count_[i] == 0) continue;
      const uint32_t base = syn_base_[i];
      Scalar sum = kZero;
      for (uint32_t s = 0; s < in_count_[i]; ++s) {
        sum += absf(syn_weight_[syn_in_[base + s]]);
      }
      if (sum <= kZero) continue;

      const Scalar hi_bound = target * band;
      const Scalar lo_bound = target / band;
      Scalar bound;
      if (sum > hi_bound) bound = hi_bound;
      else if (sum < lo_bound) bound = lo_bound;
      else continue;  // inside the band: leave learning alone

      const Scalar factor = clampf(kOne + scaling * (bound / sum - kOne),
                                   Scalar(0.5), Scalar(2));
      for (uint32_t s = 0; s < in_count_[i]; ++s) {
        const uint32_t syn = syn_in_[base + s];
        // Scaling multiplies by a positive factor, so the sign is safe; only
        // the magnitude bound has to be re-imposed.
        const Scalar ceiling = weight_ceiling(syn_source_[syn]);
        syn_weight_[syn] = clampf(syn_weight_[syn] * factor, -ceiling, ceiling);
      }
    }
  }
}

// --- M4: myelination (§3.5) ------------------------------------------------
//
// How consolidated an edge is, in [0, 1). A saturating hyperbola of traffic
// rather than an exponential: monotone, cheap, no transcendental in a pass over
// the whole synapse pool, and — the property §3.5 actually asks for — it runs
// backwards on its own as traffic decays. Nothing has to remember that an edge
// was once busy.
//
// Traffic settles near (presynaptic rate in Hz) x (traffic_tau in seconds), so
// traffic_half is quoted in those units and says which axons count as busy.
Scalar Network::myelination(uint32_t syn) const {
  const Scalar t = syn_traffic_[syn];
  const Scalar half = Scalar(dna_.header().consolidate.traffic_half);
  return t / (t + half);
}

void Network::myelinate() {
  const DnaConsolidate& c = dna_.header().consolidate;
  const Scalar floor_frac = Scalar(c.delay_floor_frac);
  const Scalar span = kOne - floor_frac;
  const bool move_delays = floor_frac < kOne;  // at 1.0 the genome disables it
  const Scalar eta_floor = Scalar(c.eta_floor_frac);
  const Scalar eta_span = kOne - eta_floor;

  // The mean per-edge learning rate is accumulated here rather than computed
  // on demand. It is a whole-pool statistic and the panel asks for it thirty
  // times a second, which would cost more than the plasticity pass that
  // actually uses the number; this pass is already walking every synapse.
  Scalar plastic_sum = kZero;
  uint32_t counted = 0;

  for (uint32_t m = 0; m < module_count_; ++m) {
    const ModuleState& ms = modules_[m];
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t i = ms.begin + k;
      if (dead_[i]) continue;
      const uint32_t base = syn_base_[i];
      for (uint32_t s = 0; s < syn_count_[i]; ++s) {
        const uint32_t syn = base + s;
        const Scalar m_frac = myelination(syn);
        plastic_sum += eta_floor + eta_span * (kOne - m_frac);
        ++counted;
        if (!move_delays) continue;
        // Recomputed from the birth delay every pass rather than nudged, so
        // the delay is a pure function of current traffic and reverting costs
        // nothing extra.
        Scalar ticks = Scalar(syn_delay0_[syn]) * (kOne - span * m_frac) + Scalar(0.5);
        uint32_t d = uint32_t(ticks);
        // The one-tick floor is not a tuning choice: a zero-tick delay delivers
        // into the slot step() is draining and the spike disappears.
        if (d < 1) d = 1;
        if (d > delay_slots_ - 1) d = delay_slots_ - 1;
        syn_delay_[syn] = uint16_t(d);
      }
    }
  }
  mean_plasticity_ = counted ? plastic_sum / Scalar(counted) : kOne;
}

Scalar Network::mean_plasticity() const { return mean_plasticity_; }

// --- M4: growth (§3.4) -----------------------------------------------------

// Growth is confined to association modules, and this is a structural fact
// rather than a policy: every transducer reads its module by slicing the live
// range [begin, begin+count) into equal contiguous pieces, one per mel channel,
// retinal feature, caregiver action or motor group. Changing `count` moves
// every slice boundary at once, so growing the cochlea by one neuron
// renumbers all twenty-four channels and every weight downstream of them is
// suddenly about the wrong frequency. That is a catastrophic-forgetting event
// caused by the mechanism meant to add capacity.
//
// The body plan agrees: a cochlea has as many channels as it has, and the
// number of vocal-tract parameters is the shape of the synthesiser. What a
// creature can grow is association.
//
// Visual cortex is excluded for a second, different reason, and it is worth
// keeping separate from the first. Its afferents are a *map* — every neuron's
// receptive field is derived from where it sits (see wire_projection_gabor) —
// and growth wires a new neuron to its local neighbours only. A neuron
// inserted into V1 would therefore have a position in the map, iso-orientation
// neighbours, and no retina: it would sit in the visual field seeing nothing.
// Growing a structured map means growing the structure, which is a different
// mechanism from the one §3.4 describes.
bool Network::growable(uint32_t m) const {
  if (m >= module_count_) return false;
  return dna_.module(m).role == uint32_t(ModuleRole::kAssociation);
}

bool Network::below_cap(uint32_t m) const {
  if (m >= module_count_) return false;
  const ModuleState& ms = modules_[m];
  // A tombstoned slot is room: growth reuses it before extending the module.
  return ms.live() < ms.capacity;
}

// §3.4's second condition. Firing rate alone would call a merely loud module
// saturated; a module with weight headroom can still learn what it needs
// without more neurons, and growing it would hide that fact rather than fix it.
bool Network::saturated(uint32_t m) const {
  if (m >= module_count_) return false;
  const ModuleState& ms = modules_[m];
  if (ms.live() == 0) return false;
  const DnaGrowth& g = dna_.header().growth;
  if (ms.mean_rate < Scalar(g.saturation_rate_hz)) return false;

  const Scalar w_max = Scalar(dna_.header().homeo.w_max);
  const Scalar mean_w = mean_in_weight(m);
  if (mean_w <= kZero) return false;
  return mean_w >= Scalar(g.saturation_weight) * w_max;
}

Scalar Network::mean_in_weight(uint32_t m) const {
  if (m >= module_count_) return kZero;
  const ModuleState& ms = modules_[m];
  Scalar sum = kZero;
  uint32_t counted = 0;
  for (uint32_t k = 0; k < ms.count; ++k) {
    const uint32_t j = ms.begin + k;
    if (dead_[j]) continue;
    const uint32_t base = syn_base_[j];
    for (uint32_t s = 0; s < in_count_[j]; ++s) {
      sum += absf(syn_weight_[syn_in_[base + s]]);
      ++counted;
    }
  }
  return counted ? sum / Scalar(counted) : kZero;
}

Scalar Network::in_weight_fill(uint32_t m, Scalar frac) const {
  if (m >= module_count_) return kZero;
  const ModuleState& ms = modules_[m];
  uint32_t at_ceiling = 0, counted = 0;
  for (uint32_t k = 0; k < ms.count; ++k) {
    const uint32_t j = ms.begin + k;
    if (dead_[j]) continue;
    const uint32_t base = syn_base_[j];
    for (uint32_t s = 0; s < in_count_[j]; ++s) {
      const uint32_t syn = syn_in_[base + s];
      // Each edge against its own ceiling, exactly as synaptic scaling clamps
      // it. An inhibitory synapse's bound is w_max * inhib_gain — measuring it
      // against the excitatory ceiling would call every inhibitory edge in the
      // module far from a bound it has already reached.
      if (absf(syn_weight_[syn]) >= frac * weight_ceiling(syn_source_[syn])) ++at_ceiling;
      ++counted;
    }
  }
  return counted ? Scalar(at_ceiling) / Scalar(counted) : kZero;
}

// How badly this neuron is failing to keep up: how far its rate sits above its
// homeostatic setpoint, plus how full its incoming weights are. This is the
// "error" whose highest region §3.4 places new neurons in, and it is the only
// error the network can see locally — the critic's prediction error is one
// number for the whole brain and has no address.
Scalar Network::pressure(uint32_t i) const {
  if (dead_[i]) return kZero;
  const Scalar target = target_rate_[i] > kZero ? target_rate_[i] : kOne;
  Scalar excess = (rate_ema_[i] - target_rate_[i]) / target;
  if (excess < kZero) excess = kZero;

  const Scalar w_max = Scalar(dna_.header().homeo.w_max);
  Scalar fill = kZero;
  if (in_count_[i] > 0 && w_max > kZero) {
    Scalar sum = kZero;
    const uint32_t base = syn_base_[i];
    for (uint32_t s = 0; s < in_count_[i]; ++s) {
      sum += absf(syn_weight_[syn_in_[base + s]]);
    }
    fill = sum / (Scalar(in_count_[i]) * w_max);
  }
  return excess + fill;
}

// A free slot in the module: a tombstone first, then the unused tail. Returns
// capacity_ when the module is full, which below_cap() has normally ruled out
// already.
uint32_t Network::claim_slot(uint32_t m) {
  ModuleState& ms = modules_[m];
  for (uint32_t k = 0; k < ms.count; ++k) {
    const uint32_t i = ms.begin + k;
    if (dead_[i]) {
      dead_[i] = 0;
      --ms.dead;
      return i;
    }
  }
  if (ms.count < ms.capacity) {
    const uint32_t i = ms.begin + ms.count;
    ++ms.count;
    return i;
  }
  return capacity_;
}

void Network::init_neuron(uint32_t i, uint32_t m, Scalar x, Scalar y, Scalar z) {
  const DnaModule& dm = dna_.module(m);
  module_of_[i] = uint8_t(m);
  v_rest_[i] = Scalar(dm.v_rest);
  v_[i] = Scalar(dm.v_rest);
  // Born at the module's genome threshold rather than at its neighbours'
  // current one. Intrinsic plasticity will move it within seconds, and
  // inheriting an adapted threshold would import the saturation this neuron
  // was grown to relieve.
  threshold_[i] = Scalar(dm.threshold);
  target_rate_[i] = Scalar(dm.target_rate_hz);
  noise_amp_[i] = Scalar(dm.noise_amp);
  leak_alpha_[i] = clampf(dt_ms_ / Scalar(dm.leak_tau_ms), kZero, kOne);
  refrac_ticks_[i] = uint16_t(Scalar(dm.refractory_ms) / dt_ms_ + Scalar(0.5));
  pos_x_[i] = clampf(x, kZero, Scalar(dm.extent[0]));
  pos_y_[i] = clampf(y, kZero, Scalar(dm.extent[1]));
  pos_z_[i] = clampf(z, kZero, Scalar(dm.extent[2]));
  // Drawn from the module's own inhibitory fraction, so growth preserves the
  // E/I balance the body plan specifies instead of quietly making the module
  // more excitatory every time it grows.
  is_inhib_[i] = rng_->chance(Scalar(dm.inhib_fraction)) ? 1 : 0;
  // Born believing it is already at its setpoint. Starting the rate estimate at
  // zero would have the first homeostasis pass read a silent neuron and drive
  // its threshold to the floor, and the new neuron would spend its first
  // minute as a noise generator rather than as capacity.
  rate_ema_[i] = Scalar(dm.target_rate_hz);
  rate_fast_[i] = kZero;
  trace_pre_[i] = kZero;
  trace_post_[i] = kZero;
  w_in_target_[i] = kZero;
  w_in_struct_[i] = kZero;
  refrac_until_[i] = 0;
  last_spike_[i] = 0;
  // DNA v25: born with a quiet tuft and no plateau in progress. Unlike the
  // rate estimate above there is nothing to flatter here — a plateau is an
  // event, and inheriting one would amplify the new neuron's first inputs for
  // no reason at all.
  v_apical_[i] = kZero;
  plateau_until_[i] = 0;
  syn_count_[i] = 0;
  in_count_[i] = 0;
  dead_[i] = 0;
}

// One edge of a grown neuron. Checks *both* caps: the source's outgoing slice
// and the target's reverse slice. A synapse whose reverse entry does not fit
// is worse than no synapse at all — it would depress under STDP and could
// never potentiate, so growth would be adding a one-way ratchet toward
// silence.
bool Network::connect_grown(uint32_t src, uint32_t dst, Scalar weight, Scalar distance) {
  if (syn_count_[src] >= syn_cap_[src]) return false;
  if (in_count_[dst] >= syn_cap_[dst]) return false;

  const Scalar velocity = Scalar(dna_.header().sim.conduction_velocity);
  const uint16_t delay = delay_ticks(distance / velocity);
  if (!add_synapse(src, dst, weight, delay)) return false;

  const uint32_t syn = syn_base_[src] + syn_count_[src] - 1;
  syn_in_[syn_base_[dst] + in_count_[dst]] = syn;
  ++in_count_[dst];
  // Scaling's setpoint is the total incoming weight this neuron is entitled
  // to. Without this the new synapse reads as excess drive and the very next
  // homeostasis pass shrinks the whole afferent set to make room for it —
  // growth would arrive already being undone.
  w_in_target_[dst] += absf(weight);
  w_in_struct_[dst] += absf(weight);
  return true;
}

uint32_t Network::grow(uint32_t m, uint32_t k) {
  if (!growable(m) || k == 0) return 0;
  ModuleState& ms = modules_[m];
  const DnaModule& dm = dna_.module(m);
  const DnaGrowth& g = dna_.header().growth;
  const Scalar radius = Scalar(dm.conn_radius);
  const Scalar inv_radius = kOne / radius;

  // Where the error is worst: the peak-pressure neuron, then the
  // pressure-weighted centroid of its neighbourhood. The peak alone would be
  // one neuron's noise; a centroid over the whole module would always be the
  // middle of the module and would carry no information at all.
  uint32_t peak = capacity_;
  Scalar peak_pressure = kZero;
  for (uint32_t a = 0; a < ms.count; ++a) {
    const uint32_t i = ms.begin + a;
    if (dead_[i]) continue;
    const Scalar p = pressure(i);
    if (peak == capacity_ || p > peak_pressure) {
      peak = i;
      peak_pressure = p;
    }
  }
  if (peak == capacity_) return 0;

  Scalar cx = pos_x_[peak], cy = pos_y_[peak], cz = pos_z_[peak];
  {
    Scalar wsum = kZero, sx = kZero, sy = kZero, sz = kZero;
    for (uint32_t a = 0; a < ms.count; ++a) {
      const uint32_t i = ms.begin + a;
      if (dead_[i]) continue;
      const Scalar dx = pos_x_[i] - pos_x_[peak];
      const Scalar dy = pos_y_[i] - pos_y_[peak];
      const Scalar dz = pos_z_[i] - pos_z_[peak];
      if (dx * dx + dy * dy + dz * dz >= radius * radius) continue;
      const Scalar p = pressure(i);
      if (p <= kZero) continue;
      wsum += p;
      sx += p * pos_x_[i];
      sy += p * pos_y_[i];
      sz += p * pos_z_[i];
    }
    if (wsum > kZero) {
      cx = sx / wsum;
      cy = sy / wsum;
      cz = sz / wsum;
    }
  }

  uint32_t grown = 0;
  for (uint32_t n = 0; n < k; ++n) {
    if (!below_cap(m)) break;
    const uint32_t i = claim_slot(m);
    if (i >= capacity_) break;

    // Scattered within a quarter-radius of the centroid rather than stacked on
    // it: k neurons at one point would all see the same neighbours at the same
    // distance and be exact duplicates, which is k times one neuron's capacity.
    const Scalar spread = radius * Scalar(0.25);
    init_neuron(i, m, cx + spread * rng_->signed_uniform(),
                cy + spread * rng_->signed_uniform(),
                cz + spread * rng_->signed_uniform());

    // Wired to local neighbours with small random weights, both directions:
    // a neuron with only outputs has nothing to say and one with only inputs
    // says nothing.
    for (uint32_t a = 0; a < ms.count; ++a) {
      const uint32_t j = ms.begin + a;
      if (j == i || dead_[j]) continue;
      const Scalar dx = pos_x_[i] - pos_x_[j];
      const Scalar dy = pos_y_[i] - pos_y_[j];
      const Scalar dz = pos_z_[i] - pos_z_[j];
      const Scalar d2 = dx * dx + dy * dy + dz * dz;
      if (d2 >= radius * radius) continue;
      const Scalar d = sqrtf(d2);
      const Scalar p = Scalar(dm.conn_density) * (kOne - d * inv_radius);

      if (rng_->chance(p)) {
        Scalar w = Scalar(g.new_weight) * (kOne + Scalar(0.3) * rng_->normal());
        if (w < kZero) w = kZero;
        if (is_inhib_[i]) w = -w * Scalar(dm.inhib_gain);
        connect_grown(i, j, w, d);
      }
      if (rng_->chance(p)) {
        Scalar w = Scalar(g.new_weight) * (kOne + Scalar(0.3) * rng_->normal());
        if (w < kZero) w = kZero;
        if (is_inhib_[j]) w = -w * Scalar(dm.inhib_gain);
        connect_grown(j, i, w, d);
      }
    }
    ++grown;
  }

  if (grown > 0) {
    ++structural_.growth_events;
    structural_.neurons_grown += grown;
    structural_.last_growth_tick = tick_;
  }
  return grown;
}

// --- M4: sleep consolidation (§3.6) ----------------------------------------

// Weak *and* idle. Both halves matter: a strong synapse that has been quiet is
// a memory waiting for its cue, and a busy synapse that happens to be weak is
// carrying traffic. Only an edge that is both is genuinely unused.
uint32_t Network::prune_synapses() {
  const DnaConsolidate& c = dna_.header().consolidate;
  const Scalar w_floor = Scalar(c.prune_weight);
  const Scalar t_floor = Scalar(c.prune_traffic);
  uint32_t pruned = 0;

  for (uint32_t m = 0; m < module_count_; ++m) {
    const ModuleState& ms = modules_[m];
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t i = ms.begin + k;
      const uint32_t base = syn_base_[i];
      uint32_t keep = 0;
      for (uint32_t s = 0; s < syn_count_[i]; ++s) {
        const uint32_t syn = base + s;
        const uint32_t dst = syn_target_[syn];
        const bool doomed = !dead_[i] && absf(syn_weight_[syn]) < w_floor &&
                            syn_traffic_[syn] < t_floor;
        if (doomed || dead_[dst]) {
          // The setpoint follows the structure. Leaving it alone would make
          // scaling treat the removed weight as a deficit and boost the
          // survivors to replace it, which is redistribution, not pruning.
          w_in_target_[dst] -= absf(syn_weight_[syn]);
          if (w_in_target_[dst] < kZero) w_in_target_[dst] = kZero;
          w_in_struct_[dst] -= absf(syn_weight_[syn]);
          if (w_in_struct_[dst] < kZero) w_in_struct_[dst] = kZero;
          ++pruned;
          continue;
        }
        // Compact in place: survivors slide down into the vacated slots, which
        // is why every reverse entry has to be rebuilt afterwards.
        const uint32_t into = base + keep;
        if (into != syn) {
          syn_target_[into] = syn_target_[syn];
          syn_source_[into] = syn_source_[syn];
          syn_weight_[into] = syn_weight_[syn];
          syn_elig_[into] = syn_elig_[syn];
          syn_traffic_[into] = syn_traffic_[syn];
          syn_delay_[into] = syn_delay_[syn];
          syn_delay0_[into] = syn_delay0_[syn];
        }
        ++keep;
      }
      syn_count_[i] = uint16_t(keep);
    }
  }
  return pruned;
}

// §3.4: "Neurons left with no surviving connections are removed." Read
// strictly — a neuron with an input but no output, or an output but no input,
// still has a connection and is left alone. Only one that is completely
// disconnected is a neuron the brain is paying for and not using.
uint32_t Network::prune_neurons() {
  uint32_t pruned = 0;
  for (uint32_t m = 0; m < module_count_; ++m) {
    // The same reason growth is confined to association modules: a transducer's
    // neuron indices are its channel map, and tombstoning one deletes part of a
    // mel channel or a motor group rather than an unused neuron.
    if (!growable(m)) continue;
    ModuleState& ms = modules_[m];
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t i = ms.begin + k;
      if (dead_[i]) continue;
      if (syn_count_[i] != 0 || in_count_[i] != 0) continue;
      dead_[i] = 1;
      ++ms.dead;
      w_in_target_[i] = kZero;
      w_in_struct_[i] = kZero;
      rate_ema_[i] = kZero;
      rate_fast_[i] = kZero;
      v_[i] = v_rest_[i];
      ++pruned;
    }
  }
  return pruned;
}

void Network::consolidate() {
  const DnaConsolidate& c = dna_.header().consolidate;
  if (!c.enabled) return;

  // Synaptic downscaling first, and this order is the mechanism, not a
  // preference. Downscaling is what makes the prune threshold selective:
  // multiply everything by 0.98 and the edges that survive are the ones
  // reward and traffic have been holding up, while everything coasting on its
  // birth weight drifts under the floor. Pruning without it deletes whatever
  // happened to start small.
  if (c.downscale < kOne) {
    // The setpoint comes down with the weights, or synaptic scaling simply
    // undoes the whole pass over the following few seconds — but only as far
    // as the structural floor, or the multiplier compounds over every sleep of
    // a long life and nothing awake ever puts the weight back.
    for (uint32_t m = 0; m < module_count_; ++m) {
      const ModuleState& ms = modules_[m];
      for (uint32_t k = 0; k < ms.count; ++k) {
        const uint32_t j = ms.begin + k;
        if (dead_[j]) continue;
        const Scalar floor = Scalar(c.downscale_floor) * w_in_struct_[j];
        const Scalar want = w_in_target_[j] * Scalar(c.downscale);
        w_in_target_[j] = want < floor ? floor : want;
      }
    }
    // Weights are scaled by the factor their *target* actually received, so a
    // neuron sitting on the floor stops being downscaled instead of drifting
    // away from a setpoint scaling would then have to drag it back to.
    for (uint32_t m = 0; m < module_count_; ++m) {
      const ModuleState& ms = modules_[m];
      for (uint32_t k = 0; k < ms.count; ++k) {
        const uint32_t j = ms.begin + k;
        if (dead_[j]) continue;
        const Scalar floor = Scalar(c.downscale_floor) * w_in_struct_[j];
        if (w_in_target_[j] <= floor) continue;  // already at the floor
        const uint32_t base = syn_base_[j];
        for (uint32_t s = 0; s < in_count_[j]; ++s) {
          syn_weight_[syn_in_[base + s]] *= Scalar(c.downscale);
        }
      }
    }
  }

  const uint32_t syn_pruned = prune_synapses();
  rebuild_reverse_index();
  // Only after the reverse index is current: "no surviving connections" is a
  // statement about in_count_, and in_count_ is meaningless until it has been
  // rebuilt from the compacted pool. A neuron is only ever tombstoned once it
  // is completely disconnected, so it has no outgoing synapses left to clean up
  // and no second pass is needed.
  const uint32_t cells_pruned = prune_neurons();

  structural_.synapses_pruned += syn_pruned;
  structural_.neurons_pruned += cells_pruned;
  ++structural_.consolidations;
  if (syn_pruned > 0 || cells_pruned > 0) structural_.last_prune_tick = tick_;
}

uint32_t Network::live_neurons() const {
  uint32_t total = 0;
  for (uint32_t m = 0; m < module_count_; ++m) total += modules_[m].live();
  return total;
}

uint32_t Network::live_synapses() const {
  uint32_t total = 0;
  for (uint32_t m = 0; m < module_count_; ++m) {
    const ModuleState& ms = modules_[m];
    for (uint32_t k = 0; k < ms.count; ++k) {
      if (dead_[ms.begin + k]) continue;
      total += syn_count_[ms.begin + k];
    }
  }
  return total;
}

// Walks presynaptic neurons in the source module in index order and their
// synapse slots in slot order, so the n-th index returned is the same synapse
// on every call. Dead neurons are skipped, which is safe for the comparison
// this exists for: a neuron pruned between two calls would shift every index
// after it, but pruning only happens during consolidation and nothing samples
// a tract across a sleep.
uint32_t Network::tract_synapses(uint32_t src_module, uint32_t dst_module, uint32_t* out,
                                 uint32_t max) const {
  if (src_module >= module_count_ || dst_module >= module_count_) return 0;
  const ModuleState& src = modules_[src_module];
  uint32_t found = 0;
  for (uint32_t k = 0; k < src.count; ++k) {
    const uint32_t i = src.begin + k;
    if (dead_[i]) continue;
    const uint32_t base = syn_base_[i];
    const uint32_t n = syn_count_[i];
    for (uint32_t s = 0; s < n; ++s) {
      const uint32_t syn = base + s;
      if (module_of_[syn_target_[syn]] != dst_module) continue;
      if (out != nullptr && found < max) out[found] = syn;
      ++found;
    }
  }
  return found;
}

Telemetry Network::telemetry() const {
  Telemetry t;
  t.tick = tick_;
  t.total_spikes = spike_count_;
  t.live_neurons = live_neurons();
  t.live_synapses = live_synapses();
  t.last_reward = last_reward_;

  Scalar rate_sum = kZero;
  uint32_t counted = 0;
  Scalar w_sum = kZero;
  Scalar e_sum = kZero;
  uint32_t syn_counted = 0;
  for (uint32_t m = 0; m < module_count_; ++m) {
    const ModuleState& ms = modules_[m];
    rate_sum += ms.mean_rate * Scalar(ms.live());
    counted += ms.live();
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t i = ms.begin + k;
      if (dead_[i]) continue;
      const uint32_t base = syn_base_[i];
      for (uint32_t s = 0; s < syn_count_[i]; ++s) {
        w_sum += absf(syn_weight_[base + s]);
        e_sum += absf(syn_elig_[base + s]);
        ++syn_counted;
      }
    }
  }
  t.mean_rate_hz = counted ? rate_sum / Scalar(counted) : kZero;
  t.mean_weight = syn_counted ? w_sum / Scalar(syn_counted) : kZero;
  t.mean_eligibility = syn_counted ? e_sum / Scalar(syn_counted) : kZero;
  return t;
}

uint64_t Network::state_hash() const {
  uint64_t h = 0xCBF29CE484222325ULL;
  hash_bytes(h, &tick_, sizeof(tick_));
  for (uint32_t m = 0; m < module_count_; ++m) {
    const ModuleState& ms = modules_[m];
    // Structure is part of the state G1 checks. Two brains that agree on every
    // weight but disagree about how many neurons they have are not the same
    // brain, and with M4 that is now a difference the same genome and the same
    // journal are supposed to make impossible.
    hash_bytes(h, &ms.count, sizeof(ms.count));
    hash_bytes(h, &ms.dead, sizeof(ms.dead));
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t i = ms.begin + k;
      hash_bytes(h, &dead_[i], sizeof(dead_[i]));
      if (dead_[i]) continue;
      hash_scalar(h, v_[i]);
      hash_scalar(h, threshold_[i]);
      hash_scalar(h, rate_ema_[i]);
      const uint32_t base = syn_base_[i];
      for (uint32_t s = 0; s < syn_count_[i]; ++s) {
        hash_scalar(h, syn_weight_[base + s]);
        hash_scalar(h, syn_elig_[base + s]);
        // Delay is state now: myelination moves it, so a brain whose pathways
        // have consolidated differently must hash differently.
        hash_bytes(h, &syn_delay_[base + s], sizeof(uint16_t));
      }
    }
  }
  return h;
}

// --- Snapshot (§8) ---------------------------------------------------------
//
// Written and read in the same order by the same list. What is here and what
// is not is the whole content of the pair: modules_ carries the structure M4
// grew, structural_ the counters the panel shows, and the rest is the tick
// loop's own bookkeeping. The pointers are absent on purpose — they are arena
// addresses, and the arena is restored by copying its bytes into a brain that
// has just been rebuilt at the same layout.

void Network::save_state(SnapshotWriter& w) const {
  w.put(tick_);
  w.put(module_count_);
  w.put(spike_count_);
  for (uint32_t m = 0; m < module_count_; ++m) w.put(modules_[m]);
  w.put(last_reward_);
  w.put(mean_plasticity_);
  w.put(structural_);
  w.put(dropped_synapses_);
  w.put(dropped_reverse_);
  for (uint32_t m = 0; m < kMaxModules; ++m) {
    w.put(dropped_by_module_[m]);
    w.put(dropped_reverse_by_module_[m]);
  }
}

void Network::load_state(SnapshotReader& r) {
  uint32_t saved_modules = 0;
  r.get(tick_);
  r.get(saved_modules);
  r.get(spike_count_);
  // The genome decides the module count and the genome is checked before this
  // runs, so a disagreement here is a corrupt file rather than a different
  // creature. Read what the file claims and let the caller's fingerprint check
  // fail, rather than walking off the end of modules_.
  if (saved_modules > kMaxModules) saved_modules = kMaxModules;
  for (uint32_t m = 0; m < saved_modules; ++m) r.get(modules_[m]);
  r.get(last_reward_);
  r.get(mean_plasticity_);
  r.get(structural_);
  r.get(dropped_synapses_);
  r.get(dropped_reverse_);
  for (uint32_t m = 0; m < kMaxModules; ++m) {
    r.get(dropped_by_module_[m]);
    r.get(dropped_reverse_by_module_[m]);
  }
}

}  // namespace aibaby
