// Diagnostics: not goals, but the measurements that say why a goal failed.
//
// Shared scaffolding is in experiments_common.h.

#include "experiments_common.h"

namespace aibaby_host {

// --- M3's microscope: where does the difference stop being legible? --------
//
// When the milestone reads chance, the useful question is not "how much
// learning" but "how far does the stimulus get". This walks the two routes the
// association has to join — a shape into the eyes, a word into the ears — and
// reads every module on the way with the same held-out classifier the
// milestone uses, at full per-neuron resolution as well as binned. No learning
// is involved and none is wanted: it measures what the wiring can carry.
// Is the visual cortex actually a visual cortex?
//
// Every other experiment here scores a *downstream* quantity — what the
// association module knows, what the voice says — and that makes them unable
// to tell "V1 is wired wrong" apart from "V1 is fine and the next stage lost
// it". This one asks the only question that has a right answer independent of
// the rest of the creature: a simple cell's receptive field is derived from
// where it sits, so its measured orientation preference must agree with the one
// its own coordinates predict.
//
// That agreement is the whole test, and it carries its own control. Pair each
// neuron's measured preference with a *different* neuron's predicted one and
// the error must collapse to chance (45 degrees, for orientations that wrap at
// 180). A map that scores well against its own prediction and badly against
// the shuffle is a real map; one that scores 45 both ways is a module full of
// noise, however healthy its firing rate looks.
bool run_v1probe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }
  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  Retina retina;
  if (!retina.configure(vcfg, error)) {
    std::printf("  retina failed: %s\n", error.c_str());
    return false;
  }
  const int32_t v1 = s.dna.module_with_role(aibaby::ModuleRole::kVisualCortex);
  if (v1 < 0) {
    std::printf("  genome has no visual cortex — nothing to probe\n");
    return false;
  }
  const aibaby::Network& net = s.brain.network();
  const aibaby::ModuleState& ms = net.module(uint32_t(v1));
  const aibaby::DnaModule& dm = s.dna.module(uint32_t(v1));

  // --- afferent structure ---------------------------------------------------
  // Before any tuning is measured: did the map wire at all? A simple cell with
  // no afferents is a noise generator, and it would sit at chance in every
  // column of every downstream table without ever looking broken.
  uint32_t zero_in = 0, min_in = 0xFFFFFFFFu, max_in = 0;
  double mean_in = 0;
  for (uint32_t k = 0; k < ms.count; ++k) {
    const uint32_t d = net.in_degree(ms.begin + k);
    if (d == 0) ++zero_in;
    if (d < min_in) min_in = d;
    if (d > max_in) max_in = d;
    mean_in += double(d);
  }
  if (ms.count) mean_in /= double(ms.count);

  // --- structural tuning ----------------------------------------------------
  // What the afferents alone say, before a single spike. A simple cell's field
  // is elongated *along* its preferred orientation and its ON and OFF lobes
  // alternate across it, so the axis of the field's second moment predicts the
  // preference directly. If this disagrees with the map the wiring is wrong; if
  // it agrees and the spikes do not, the wiring is right and something
  // downstream of it is losing the signal. No other measurement here can tell
  // those two apart.
  const int32_t vis_m = s.dna.module_with_role(aibaby::ModuleRole::kVision);
  const uint32_t nfeat = aibaby::vision_features(vcfg);
  double struct_err = 0, struct_shuf = 0;
  uint32_t struct_n = 0;
  std::vector<double> struct_theta(ms.count, -1.0);
  if (vis_m >= 0) {
    const aibaby::ModuleState& vms = net.module(uint32_t(vis_m));
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t j = ms.begin + k;
      // What orientation is this field actually built for? Not the shape of the
      // afferent cloud — that is mostly the envelope, which is nearly round —
      // but the axis along which its ON and OFF lobes alternate. Recovered by
      // asking which direction the signed field oscillates in most strongly,
      // which is the same question the grating asks, put to the wiring.
      const uint32_t deg = net.in_degree(j);
      double wsum = 0, cu = 0, cv = 0;
      for (uint32_t a = 0; a < deg; ++a) {
        uint32_t srcn; aibaby::Scalar w;
        net.in_synapse(j, a, srcn, w);
        if (srcn < vms.begin || srcn >= vms.begin + vms.count) continue;
        const uint32_t f = aibaby::slice_of(vms.count, nfeat, srcn - vms.begin);
        const aibaby::VisionCell g = aibaby::vision_cell(vcfg, f / 2);
        const double ww = std::fabs(double(w));
        cu += ww * g.u; cv += ww * g.v; wsum += ww;
      }
      if (wsum <= 0) continue;
      cu /= wsum; cv /= wsum;

      // The field's own wavelength, from the genome and the local pitch.
      double pitch = 0.0, bestd = 1e30;
      for (uint32_t c = 0; c < aibaby::vision_cells(vcfg); ++c) {
        const aibaby::VisionCell g = aibaby::vision_cell(vcfg, c);
        const double d = (g.u - cu) * (g.u - cu) + (g.v - cv) * (g.v - cv);
        if (d < bestd) { bestd = d; pitch = g.pitch; }
      }
      double lambda = 0.0;
      for (uint32_t pi = 0; pi < s.dna.projection_count(); ++pi) {
        const aibaby::DnaProjection& pr = s.dna.projection(pi);
        if (pr.kind == uint32_t(aibaby::ProjectionKind::kGabor)) lambda = pr.rf_lambda * pitch;
      }
      if (lambda <= 0) continue;

      double best_r = -1.0, axis = 0.0;
      for (uint32_t step = 0; step < 180; ++step) {
        const double phi = M_PI * double(step) / 180.0;
        const double cp = std::cos(phi), sp = std::sin(phi);
        double re = 0, im = 0;
        for (uint32_t a = 0; a < deg; ++a) {
          uint32_t srcn; aibaby::Scalar w;
          net.in_synapse(j, a, srcn, w);
          if (srcn < vms.begin || srcn >= vms.begin + vms.count) continue;
          const uint32_t f = aibaby::slice_of(vms.count, nfeat, srcn - vms.begin);
          const aibaby::VisionCell g = aibaby::vision_cell(vcfg, f / 2);
          // ON and OFF are the two signs of one field.
          const double sign = (f % 2) == 0 ? 1.0 : -1.0;
          const double proj = (double(g.u) - cu) * cp + (double(g.v) - cv) * sp;
          const double ang = 2.0 * M_PI * proj / lambda;
          re += sign * double(w) * std::cos(ang);
          im += sign * double(w) * std::sin(ang);
        }
        const double r = std::sqrt(re * re + im * im);
        if (r > best_r) { best_r = r; axis = phi; }
      }
      struct_theta[k] = axis;

      aibaby::Scalar px, py, pz;
      net.position(j, px, py, pz);
      const double predicted = M_PI * double(pz) / double(dm.extent[2]);
      auto wrap = [](double d) {
        while (d < -M_PI / 2) d += M_PI;
        while (d > M_PI / 2) d -= M_PI;
        return std::fabs(d);
      };
      struct_err += wrap(axis - predicted) * 180.0 / M_PI;
      aibaby::Scalar qx, qy, qz;
      net.position(ms.begin + (k + ms.count / 3) % ms.count, qx, qy, qz);
      struct_shuf += wrap(axis - M_PI * double(qz) / double(dm.extent[2])) * 180.0 / M_PI;
      ++struct_n;
    }
  }

  // --- orientation tuning ---------------------------------------------------
  // A drifting sinusoidal grating at the receptive fields' own spatial
  // frequency. Drifting rather than static because a simple cell is
  // phase-sensitive: a static grating that happens to land its dark bar on the
  // ON lobe measures the cell's phase, not its orientation.
  constexpr uint32_t kOrients = 8;
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);
  const uint64_t per_orient = ticks / kOrients;
  const uint64_t settle = per_orient / 4;  // discard the onset transient

  // One full ON-OFF-ON cycle of a foveal receptive field, in pixels.
  const aibaby::VisionCell fov = aibaby::vision_cell(vcfg, 0);
  const float lambda_px = 3.0f * fov.pitch * float(vcfg.frame_size);

  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  std::vector<std::vector<double>> response(kOrients,
                                            std::vector<double>(ms.count, 0.0));

  for (uint32_t o = 0; o < kOrients; ++o) {
    const float theta = float(M_PI) * float(o) / float(kOrients);
    const float ct = std::cos(theta), st = std::sin(theta);
    uint32_t phase_step = 0;

    for (uint64_t t = 0; t < per_orient; ++t) {
      if (t % frame_ticks == 0) {
        const float phi = 2.0f * float(M_PI) * float(phase_step) / 4.0f;
        ++phase_step;
        for (uint32_t y = 0; y < vcfg.frame_size; ++y) {
          for (uint32_t x = 0; x < vcfg.frame_size; ++x) {
            const float px = float(x) + 0.5f - float(vcfg.frame_size) * 0.5f;
            const float py = float(y) + 0.5f - float(vcfg.frame_size) * 0.5f;
            // Projection onto the grating's normal: the axis the bars cross.
            const float proj = px * ct + py * st;
            const float v =
                0.5f + 0.4f * std::cos(2.0f * float(M_PI) * proj / lambda_px + phi);
            frame[size_t(y) * vcfg.frame_size + x] =
                uint8_t((v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v)) * 255.0f + 0.5f);
          }
        }
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }

      s.brain.step();
      if (t < settle) continue;
      const uint32_t* spikes = net.spikes();
      for (uint32_t i = 0; i < net.spike_count(); ++i) {
        const uint32_t n = spikes[i];
        if (n >= ms.begin && n < ms.begin + ms.count) response[o][n - ms.begin] += 1.0;
      }
    }
  }

  // --- measured preference against predicted --------------------------------
  // Orientation wraps at 180 degrees, so the preference is summarised as a
  // vector average at *twice* the angle — the standard circular statistic, and
  // the reason a cell that answers equally to 0 and 179 reads as sharply tuned
  // rather than as untuned.
  double err_sum = 0, err_shuf = 0, osi_sum = 0;
  uint32_t tuned = 0, counted = 0;
  for (uint32_t k = 0; k < ms.count; ++k) {
    double total = 0, sx = 0, sy = 0, best = 0, worst = 1e30;
    for (uint32_t o = 0; o < kOrients; ++o) {
      const double r = response[o][k];
      const double a = 2.0 * M_PI * double(o) / double(kOrients);
      sx += r * std::cos(a);
      sy += r * std::sin(a);
      total += r;
      if (r > best) best = r;
      if (r < worst) worst = r;
    }
    if (total <= 0.0) continue;
    ++counted;

    // Orientation selectivity index: 1 - circular variance.
    const double osi = std::sqrt(sx * sx + sy * sy) / total;
    osi_sum += osi;
    if (osi > 0.2) ++tuned;

    const double measured = 0.5 * std::atan2(sy, sx);  // back to [0, pi)
    aibaby::Scalar px, py, pz;
    net.position(ms.begin + k, px, py, pz);
    const double predicted = M_PI * double(pz) / double(dm.extent[2]);

    auto wrap = [](double d) {
      while (d < -M_PI / 2) d += M_PI;
      while (d > M_PI / 2) d -= M_PI;
      return std::fabs(d);
    };
    err_sum += wrap(measured - predicted) * 180.0 / M_PI;

    // The control: the same measured preference against a different neuron's
    // prediction. Offset by a third of the module so it is a fixed, reportable
    // pairing rather than another draw of noise.
    aibaby::Scalar qx, qy, qz;
    net.position(ms.begin + (k + ms.count / 3) % ms.count, qx, qy, qz);
    err_shuf += wrap(measured - M_PI * double(qz) / double(dm.extent[2])) * 180.0 / M_PI;
  }

  const double mean_err = counted ? err_sum / double(counted) : 0.0;
  const double mean_shuf = counted ? err_shuf / double(counted) : 0.0;
  const double mean_osi = counted ? osi_sum / double(counted) : 0.0;

  std::printf("  module            %s — %u neurons\n", dm.name, ms.count);
  instrument("v1probe", s.dna.header().seed, kOrients, "orientations");
  std::printf("  afferents         in-degree min %u  mean %.1f  max %u\n", min_in,
              mean_in, max_in);
  std::printf("                    %u neurons with no input at all\n", zero_in);
  std::printf("  field axis        %.1f deg from its own map, %.1f shuffled  (%u cells)\n",
              struct_n ? struct_err / double(struct_n) : 0.0,
              struct_n ? struct_shuf / double(struct_n) : 0.0, struct_n);
  std::printf("  responded         %u of %u to a drifting grating\n", counted, ms.count);
  std::printf("  mean OSI          %.3f   (%u of %u above 0.2)\n", mean_osi, tuned,
              counted);
  // How spread the measured preferences are. A cortex where every cell prefers
  // the same orientation scores nonzero OSI per cell and exactly chance against
  // the map, which is the one failure the numbers above cannot distinguish.
  {
    uint32_t hist[8] = {};
    for (uint32_t k = 0; k < ms.count; ++k) {
      if (struct_theta[k] < -0.5) continue;
      double total = 0, sx = 0, sy = 0;
      for (uint32_t o = 0; o < kOrients; ++o) {
        const double r = response[o][k];
        sx += r * std::cos(2.0 * M_PI * double(o) / double(kOrients));
        sy += r * std::sin(2.0 * M_PI * double(o) / double(kOrients));
        total += r;
      }
      if (total <= 0) continue;
      double pref = 0.5 * std::atan2(sy, sx);
      if (pref < 0) pref += M_PI;
      uint32_t bin = uint32_t(pref / M_PI * 8.0);
      hist[bin > 7 ? 7 : bin]++;
    }
    std::printf("  measured pref     ");
    for (uint32_t b = 0; b < 8; ++b) std::printf("%4u", hist[b]);
    std::printf("   (spread over 8 bins; all in one = no map)\n");
  }
  std::printf("  orientation error %.1f deg against its own map\n", mean_err);
  std::printf("                    %.1f deg against a shuffled map  (chance 45.0)\n",
              mean_shuf);
  if (verbose) {
    std::printf("  per-orientation population response\n");
    for (uint32_t o = 0; o < kOrients; ++o) {
      double total = 0;
      for (uint32_t k = 0; k < ms.count; ++k) total += response[o][k];
      std::printf("    %5.1f deg  %8.0f spikes\n", 180.0 * double(o) / double(kOrients),
                  total);
    }
  }

  // The criterion is the *gap*, not the absolute error, and the reason is the
  // line above it: the wiring's own structural precision is around 24 degrees
  // on this retina, because a receptive field built from ~35 afferents on a
  // discrete lattice cannot specify an orientation more finely than that. A
  // spiking readout can approach that ceiling and cannot beat it, so scoring
  // against a fixed number would be scoring against the sampling density of
  // the eye. What has to be true is that the cortex is tuned the way its own
  // map says rather than some other way, and the shuffle is what measures it.
  const bool ok = zero_in == 0 && mean_shuf - mean_err > 8.0 && mean_err < 38.0;
  std::printf("\n  v1probe %s — the cortex is %s tuned the way its map says.\n",
              ok ? "PASS" : "FAIL", ok ? "" : "NOT ");
  return ok;
}


bool run_m3probe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }
  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  Retina retina;
  Ear ear;
  if (!retina.configure(vcfg, error) || !ear.configure(acfg, error)) {
    std::printf("  transducer failed: %s\n", error.c_str());
    return false;
  }
  VowelSource voice(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  std::vector<float> pcm(16);
  const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);

  aibaby::Rng rng;
  rng.seed(s.dna.header().seed ^ 0x11B3u);
  const aibaby::Network& net = s.brain.network();
  const uint32_t modules = net.module_count();
  // Fixed at the start for the same reason as in run_m3_session: a module that
  // grows mid-session would otherwise hand the classifier feature vectors of
  // two different widths and call them the same measurement.
  std::vector<uint32_t> width(modules);
  for (uint32_t m = 0; m < modules; ++m) width[m] = net.module(m).count;

  // One condition: `visual` shows two shapes in silence, otherwise two words
  // are spoken to an empty field. Returns the per-neuron score of the module
  // named in `control` — the row that has to work before any other row on the
  // table can be read as anything.
  auto sweep = [&](bool visual, const char* title, const char* control) -> double {
    const uint32_t n_trials = uint32_t(ticks / kM3ProbeTicks);
    std::vector<std::vector<std::vector<double>>> per_module(modules);
    std::vector<std::vector<std::vector<double>>> per_module_phase(modules);
    std::vector<std::vector<std::vector<double>>> per_module_ema(modules);
    std::vector<std::vector<double>> vocal;
    std::vector<int> labels;
    uint32_t last_frame = 0;

    for (uint32_t trial = 0; trial < n_trials; ++trial) {
      const int label = int(trial % 2);
      Toy toy = m3_toy(rng, label);
      if (!visual) toy.shape = SceneSource::Shape::kNone;

      std::vector<double> counts(net.total_capacity(), 0.0);
      // The same spikes, kept by their phase within the retinal frame, so the
      // readout can ask *when* as well as *where*.
      std::vector<double> phase_counts(size_t(net.total_capacity()) * kPhaseBins, 0.0);
      M3Record rec;
      bool slept = false;

      for (uint64_t t = 0; t < kM3ProbeTicks; ++t) {
        if (t % frame_ticks == 0) {
          scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f,
                       frame.data());
          retina.present(frame.data());
          s.brain.see(retina.features().data(), retina.feature_count());
        }
        const bool sounding = !visual && t < kM3LabelTicks;
        const Word& w = kWords[label];
        voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                     pcm.data(), samples_per_tick);
        ear.tick(s.brain, pcm.data(), samples_per_tick);

        s.brain.step();
        if (s.brain.asleep()) slept = true;
        if (t < kM3SettleTicks) continue;
        // Phase is measured from the frame that is currently on the retina,
        // because that is the event the volley is timed against.
        const uint32_t phase =
            uint32_t((t % frame_ticks) * kPhaseBins / frame_ticks);
        for (uint32_t k = 0; k < net.spike_count(); ++k) {
          const uint32_t i = net.spikes()[k];
          counts[i] += 1.0;
          phase_counts[size_t(i) * kPhaseBins + phase] += 1.0;
        }
        if (s.brain.vocal_frame() == last_frame) continue;
        last_frame = s.brain.vocal_frame();
        ++rec.frames;
        const aibaby::Scalar* g = s.brain.vocal_groups();
        for (uint32_t k = 0; k < aibaby::kVocalGroups; ++k) rec.group[k] += double(g[k]);
        rec.amplitude += double(s.brain.voice().amplitude);
        if (s.brain.voice().voicing > 0.5f &&
            s.brain.voice().amplitude > kAmplitudeFloor) {
          ++rec.voiced;
        }
      }
      if (slept || rec.frames == 0) continue;

      for (uint32_t m = 0; m < modules; ++m) {
        const aibaby::ModuleState& ms = net.module(m);
        per_module[m].emplace_back(counts.begin() + ms.begin,
                                   counts.begin() + ms.begin + width[m]);
        // The same module read the way g3probe reads it: the one-second rate
        // EMA sampled at the end of the trial, rather than the spike counts
        // over it. The two experiments disagreed about whether the object
        // reaches `vocal`, and this column is here to say whether that
        // disagreement is the feature or the protocol.
        std::vector<double> ema;
        ema.reserve(width[m]);
        for (uint32_t k = 0; k < width[m]; ++k) {
          ema.push_back(double(net.rate(ms.begin + k)));
        }
        per_module_ema[m].push_back(ema);
        // Same total number of features as the `kFeatureBins` spatial
        // histogram it is compared against.
        per_module_phase[m].push_back(
            rebin_phases(phase_counts, ms.begin, width[m], kFeatureBins / kPhaseBins));
      }
      vocal.push_back(m3_vocal_features(rec));
      labels.push_back(label);
    }

    std::printf("\n  %s — %zu trials\n", title, labels.size());
    if (labels.size() < 12) {
      std::printf("    too few trials to score\n");
      return 0.0;
    }
    double control_score = 0.0;
    const size_t train = labels.size() / 2;
    // The two middle columns are the comparison this experiment exists for.
    // Both hand the classifier exactly kFeatureBins numbers per trial; one
    // describes where the spikes were and the other where *and when*. A gap
    // between them is spike timing carrying something counts cannot see.
    std::printf("    %-12s %-11s %-11s %-11s %-11s %-11s %-11s %-11s %s\n", "module",
                "per-neuron", "interleaved", "rate EMA", "32 space", "8x4 space+t",
                "rate only", "shuffled", "spikes/trial");
    for (uint32_t m = 0; m < modules; ++m) {
      std::vector<std::vector<double>> binned, rate;
      // Without this column a 0.500 is ambiguous. `holdout_accuracy` floors
      // every standard deviation at 1e-9, so a module that never fires gives
      // both centroids the same zero vector, the tie resolves to class 0, and
      // the alternating labels make that exactly half right. A silent module
      // and an active-but-uninformative one therefore print the same number.
      // A four-point density sweep once read 0.500 four times and was taken
      // for "the tract is wide enough already"; it meant the module was mute.
      double spikes = 0.0;
      for (const std::vector<double>& v : per_module[m]) {
        binned.push_back(rebin(v, kFeatureBins));
        double total = 0.0;
        for (double x : v) total += x;
        spikes += total;
        rate.push_back(std::vector<double>{total});
      }
      spikes /= double(per_module[m].size());
      std::vector<int> shuffled = labels;
      for (size_t i = shuffled.size(); i > 1; --i) {
        std::swap(shuffled[i - 1], shuffled[rng.next() % i]);
      }
      // Same features and same classifier as `per-neuron`; only the fold
      // boundary moves, so a gap between the two columns is drift across the
      // session rather than anything about the code the module carries.
      std::vector<std::vector<double>> xi;
      std::vector<int> yi;
      size_t itrain = 0;
      interleave_pairs(per_module[m], labels, xi, yi, itrain);
      // Eight creatures running printed exactly 0.500 for `vocal` here. That
      // is what a classifier reading session time scores against alternating
      // labels — see the proof on holdout_guess_time_corr.
      const double tcorr = holdout_guess_time_corr(per_module[m], labels, train);
      const double per_neuron = holdout_accuracy(per_module[m], labels, train);
      if (std::strcmp(net.module_dna(m).name, control) == 0) control_score = per_neuron;

      std::printf("    %-12s %-11.3f %-11.3f %-11.3f %-11.3f %-11.3f %-11.3f %-11.3f %.1f%s\n",
                  net.module_dna(m).name,
                  per_neuron,
                  holdout_accuracy(xi, yi, itrain),
                  holdout_accuracy(per_module_ema[m], labels, train),
                  holdout_accuracy(binned, labels, train),
                  holdout_accuracy(per_module_phase[m], labels, train),
                  holdout_accuracy(rate, labels, train),
                  holdout_accuracy(per_module_phase[m], shuffled, train),
                  spikes,
                  spikes < 0.5 ? "  <- MUTE, the row above is not a score"
                  : std::fabs(tcorr) > 0.5
                      ? "  <- per-neuron is reading session TIME, use interleaved"
                      : "");
      if (verbose) {
        std::printf("      %-10s guess-vs-time corr %+.3f\n", net.module_dna(m).name,
                    tcorr);
      }
    }
    std::printf("    %-12s %-11.3f %s\n", "voice", holdout_accuracy(vocal, labels, train),
                "  <- the nine motor groups, loudness and voicing");
    return control_score;
  };

  std::printf("  each trial %llu ms, first %llu discarded; no learning, no reward\n",
              (unsigned long long)kM3ProbeTicks, (unsigned long long)kM3SettleTicks);
  instrument("m3probe", s.dna.header().seed ^ 0x11B3u, ticks / kM3ProbeTicks,
             "trials per sweep");
  std::printf("  '32 space' and '8x4 space+t' are %u features each — the same budget\n"
              "  spent on where the spikes were, or on where and when. 'shuffled'\n"
              "  controls the space+t column.\n",
              kFeatureBins);
  // Each sweep's control is the module the signal is delivered *into*. If the
  // picture is not legible in `vision`, or the word is not legible in
  // `auditory`, then nothing this table says about the modules downstream is
  // about the creature — the signal never entered it.
  const double vis_ctl = sweep(true, "a cube or a ball, in silence", "vision");
  const double aud_ctl = sweep(false, "one of two words, to an empty field", "auditory");
  // m3probe is a microscope and reports rather than judges — but a microscope
  // with no light in it is a failure, not a null, and this used to return true
  // unconditionally.
  bool ok = true;
  ok = positive_control("m3probe", "the object in `vision`, per neuron", vis_ctl, 0.85,
                        "Both sweeps read every module downstream of this one.") && ok;
  ok = positive_control("m3probe", "the word in `auditory`, per neuron", aud_ctl, 0.85,
                        "The word sweep below it is not a measurement.") && ok;
  if (ok) {
    std::printf("\n  m3probe is a microscope, not a criterion — both delivery controls\n"
                "  are lit (%.3f, %.3f), so the rows above are about the creature.\n",
                vis_ctl, aud_ctl);
  }
  (void)verbose;
  return ok;
}


// --- G2: rewarded vocalisations increase ----------------------------------

// How much of G3 is learnable at all?
//
// `m3` teaches the way a caregiver does: the praise that accompanies naming is
// identical for both objects, so reward opens the gate on three-factor learning
// and says nothing about *which* toy this is. Everything that distinguishes the
// two classes has to travel picture -> sound -> voice on its own. That is the
// honest protocol and the milestone rightly uses it — but when it fails, it
// cannot say whether the creature lacked the teaching signal or lacked the
// capacity to comply with one.
//
// This is the other half, and it is built like `g2probe`: an idealised teacher.
// Reward becomes dense, immediate, and *class-informative* — it tells the baby,
// continuously, how much its voice right now resembles the sound that belongs
// with the toy in front of it. Nothing about the creature changes; only the
// generosity of the teaching.
//
// Two properties make the number a ceiling rather than a wish:
//
//   - **The targets are the creature's own.** Each session begins by playing
//     both words to an empty field and recording what that particular larynx
//     does in response. The teacher then asks for postures this body has
//     already been observed to produce. Scoring against arbitrary targets would
//     measure the vocal tract's range, not the association.
//   - **Homeostasis is left alone.** Switching it off to stop it erasing things
//     silences the creature instead — 11 usable probes instead of 32 and the
//     echo falling 0.83 to 0.57 — so a ceiling measured that way is a ceiling
//     for a different animal.
//
// The control arm gets reward just as dense and just as immediate, but the
// target it is shaped toward is drawn at random each trial rather than fixed by
// the object. That is what separates "a teacher can shape this voice per
// object" from "dense reward makes any voice more classifiable", and without it
// the ceiling would be unreadable in the same way the milestone is.
struct G3Probe {
  bool ok = false;
  double vocal = 0;      // held-out accuracy on the probe voice — the ceiling
  double shuffled = 0;   // the same with labels shuffled: must sit at chance
  double echo = 0;       // the calibration echo, for reference
  double reward = 0;     // mean |R - E[R]| reaching the synapses
  uint32_t probes = 0, taught = 0;
  uint32_t probes_seen = 0;  // counts attempts, so alternation survives a drop
  // Is synaptic scaling on the larynx actually doing anything? `press` is the
  // mean of sum|w_in| / setpoint over the vocal module, `outside` the fraction
  // of samples that left the dead band — which is the only part scaling ever
  // acts on. A relaxation sweep that reads flat means one of two opposite
  // things, and these two columns are what separate them.
  double press = 0, outside = 0;
  uint32_t press_n = 0;
  // Cube vs ball read off the association module itself, and the same with the
  // labels shuffled. This is the condition side of the conditional mapping:
  // the teacher supplies the answer, but the creature has to supply the
  // question, and it can only do that from its own activity.
  double central = 0, central_shuffled = 0;
  double vision = 0;        // the same readout at the retina's own cortex
  double vision_shuffled = 0;
  // And at the larynx, which is the module G3 is ultimately about. Missing
  // from this table until 2026-08-15: `m3probe` can say whether the object
  // reaches vocal with learning *off*, and nothing said whether it survives a
  // taught session with homeostasis and R-STDP both running.
  double vocal_code_acc = 0, vocal_code_shuf = 0;
  // How much of each scored window the two conditions were actually present
  // for. Three separate protocol bugs in one session were a stimulus that was
  // present but *mistimed*, and calibrate, the shuffled control and the duty
  // cycle all looked healthy through every one of them. Nothing else here
  // measures this.
  double teach_seen = 0, teach_heard = 0, teach_ticks = 0;
  double probe_seen = 0, probe_heard = 0, probe_ticks = 0;
  // The fast learner, when a genome has one (DNA v13). Pattern separation is
  // the claim: if this does not read *above* central, the hippocampus is not
  // earning its neurons.
  double hippo = 0, hippo_shuffled = 0;
  double hippo_corr = 0, central_corr = 0, vision_corr = 0;
  bool has_hippo = false;
  CodeStats hip_code;
  double central_full = 0;  // and central again, unpooled, in case 16 is coarse
  CodeStats vis_code, cen_code;
};


// `word_cond` swaps which sense carries the condition. With it false the
// caregiver is silent and the object is shown, which is g3probe's protocol and
// hands the rule a condition that reaches vocal at chance. With it true the
// object is hidden and the *word* is spoken instead — a condition that reaches
// vocal at 0.960 — so a null with it true cannot be blamed on delivery.
// `mode` picks which sense carries the condition during teaching:
//   0  object shown, caregiver silent          (g3probe)
//   1  object hidden, word spoken throughout   (condprobe)
//   2  object shown AND word spoken            (pairprobe — CS and US together)
// The probe is always the object alone, except in mode 1 where it is the word.
// Mode 2 is the classical-conditioning test: the US drives the correct output
// during teaching, the CS is present at the same time, and the probe asks
// whether the CS alone has come to evoke it.
G3Probe run_g3probe_session(const std::vector<uint8_t>& blob, uint64_t ticks,
                            bool consistent, uint64_t seed_shift, int mode) {
  G3Probe out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) return out;

  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  Retina retina;
  Ear ear;
  if (!retina.configure(vcfg, error) || !ear.configure(acfg, error)) return out;

  VowelSource voice(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, s.dna.header().seed ^ seed_shift);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  std::vector<float> pcm(16);
  const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);
  const uint32_t plasticity = s.dna.header().sim.plasticity_interval_ticks;

  aibaby::Rng rng;
  rng.seed((s.dna.header().seed ^ seed_shift) + 0x6D3Bu);

  uint64_t now = 0;
  uint32_t last_frame = 0;

  // How hard synaptic scaling is leaning on the larynx. Sampled rather than
  // counted: the host cannot see the homeostasis pass, but it can see the
  // quantity that pass looks at, and a neuron sitting inside the dead band at
  // every sample is a neuron scaling never touched.
  // Can the association module tell the two toys apart at all?
  //
  // The idealised teacher fixes the *reward* — it says which object this is.
  // It does not give the creature that fact: the condition side of a
  // conditional mapping still has to be read off central's own activity. If
  // cube and ball look the same there, no teacher of any generosity can build
  // a mapping keyed on the difference, and the ceiling would be a statement
  // about the input representation rather than about the learning rule. That
  // distinction is invisible in the voice column, so it is measured here, with
  // the same classifier and the same held-out split that scores the voice.
  //
  // Pooled to 16 bins because 400 neurons against ~46 training probes is a
  // classifier that fits noise; the shuffled column beside it is what says
  // whether even 16 is too many.
  constexpr uint32_t kCentralBins = 16;
  const int32_t cen_m = s.dna.module_with_role(aibaby::ModuleRole::kAssociation);
  const int32_t vis_m = s.dna.module_with_role(aibaby::ModuleRole::kVision);
  const int32_t hip_m = s.dna.module_with_role(aibaby::ModuleRole::kHippocampus);
  out.has_hippo = hip_m >= 0;
  std::vector<double> last_central, last_vision, last_central_full, last_hippo;
  std::vector<double> last_vocal;

  // Pool a module's rates into `bins` averages, or into one per neuron when
  // `bins` is 0. Applied to vision as well as central, because a chance
  // reading at central means nothing on its own: it could be a module that
  // does not carry the distinction, or a readout too coarse to see one. Vision
  // is the positive control — the same pooling, the same classifier, on a
  // module that demonstrably does carry it (see `m2`).
  auto pool = [&](int32_t mod, uint32_t bins, std::vector<double>& dst) {
    dst.clear();
    if (mod < 0) return;
    const aibaby::Network& net = s.brain.network();
    const aibaby::ModuleState& pms = net.module(uint32_t(mod));
    if (bins == 0) {
      for (uint32_t k = 0; k < pms.count; ++k) dst.push_back(double(net.rate(pms.begin + k)));
      return;
    }
    dst.assign(bins, 0.0);
    std::vector<uint32_t> n(bins, 0);
    for (uint32_t k = 0; k < pms.count; ++k) {
      const uint32_t b = pms.count > 1 ? k * bins / pms.count : 0;
      dst[b] += double(net.rate(pms.begin + k));
      ++n[b];
    }
    for (uint32_t b = 0; b < bins; ++b) {
      if (n[b]) dst[b] /= double(n[b]);
    }
  };

  const int32_t voc_m = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
  const double band = s.dna.header().homeo.scaling_band > 1.0f
                          ? double(s.dna.header().homeo.scaling_band)
                          : 1.0;
  constexpr uint64_t kPressEvery = 5000;  // 5 s of simulated life

  // Reads the vocal tract over a stretch of ticks, optionally with a word
  // playing and optionally with a toy in view, and returns the mean of the nine
  // motor groups over the vocal frames it saw.
  auto observe = [&](int object, int word, bool speak, bool show, uint64_t length,
                     uint64_t settle, const double* shape_target,
                     const double* shape_mid, double* reward_acc,
                     uint32_t* reward_n,
                     bool want_probe_presence = false) -> std::vector<double> {
    std::vector<double> sum(aibaby::kVocalGroups, 0.0);
    double amp = 0, voiced = 0;
    uint32_t frames = 0;
    const Toy toy = m3_toy(rng, object);

    for (uint64_t t = 0; t < length; ++t, ++now) {
      if (show && t % frame_ticks == 0) {
        scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }
      // A visual condition is re-rendered every frame and so is present for the
      // whole trial; a spoken one would stop at kM3LabelTicks = 900, which is
      // 200 ticks *before* the probe's scored window even opens at 1100. Under
      // `word_cond` the word is held for the whole length, so the two senses
      // deliver the condition over the same interval and the comparison is
      // about the sense rather than about the timing.
      // Modes 1 and 2 hold the word for the whole trial. For mode 2 this is
      // not cosmetic: with the word stopping at kM3LabelTicks = 900 of 2000
      // ticks while the object is shown throughout, 55% of every pairing trial
      // was CS-without-US — an extinction protocol running alongside the
      // acquisition one, which is a fair way to teach a creature that the
      // object predicts nothing.
      const bool sounding = speak && (mode != 0 || t < kM3LabelTicks);
      if (t >= settle && shape_target != nullptr) {
        if (show) out.teach_seen += 1.0;
        if (sounding) out.teach_heard += 1.0;
        out.teach_ticks += 1.0;
      } else if (t >= settle && want_probe_presence) {
        // Tagged explicitly rather than inferred from `shape_target`, because
        // the calibration-echo trials also pass a null target and would
        // otherwise be averaged in with the real probes — which is exactly what
        // they did on the first run of this guard, reading object 0.73 / word
        // 0.12 for a probe that is object-only in silence.
        if (show) out.probe_seen += 1.0;
        if (sounding) out.probe_heard += 1.0;
        out.probe_ticks += 1.0;
      }
      const Word& w = kWords[word];
      voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                   pcm.data(), samples_per_tick);
      ear.tick(s.brain, pcm.data(), samples_per_tick);

      // The idealised teacher. Dense — every plasticity interval — immediate,
      // and signed by how far the voice currently sits along the axis between
      // the two target postures, toward the one this object calls for.
      if (shape_target && t >= settle && now % plasticity == 0) {
        const aibaby::Scalar* g = s.brain.vocal_groups();
        // Centred on the midpoint between the two postures. Uncentred, the
        // projection of an all-positive group vector onto the axis is dominated
        // by how *much* the larynx is doing rather than which way, and the
        // teacher spends its authority rewarding loudness. Same trap as the
        // exploration signal: a difference needs an origin.
        double dot = 0, norm = 0;
        for (uint32_t k = 0; k < aibaby::kVocalGroups; ++k) {
          dot += (double(g[k]) - shape_mid[k]) * shape_target[k];
          norm += shape_target[k] * shape_target[k];
        }
        if (norm > 0) {
          const double r = dot / std::sqrt(norm);
          const float clipped = float(r > 1.0 ? 1.0 : (r < -1.0 ? -1.0 : r));
          s.brain.praise(clipped);
          if (reward_acc) { *reward_acc += std::fabs(double(clipped)); ++*reward_n; }
        }
      }

      s.brain.step();

      if (voc_m >= 0 && now % kPressEvery == 0) {
        const aibaby::Network& net = s.brain.network();
        const aibaby::ModuleState& vms = net.module(uint32_t(voc_m));
        for (uint32_t k = 0; k < vms.count; ++k) {
          const uint32_t i = vms.begin + k;
          const double target = double(net.scaling_setpoint(i));
          const uint32_t deg = net.in_degree(i);
          if (target <= 0.0 || deg == 0) continue;
          double sum = 0;
          for (uint32_t a = 0; a < deg; ++a) {
            uint32_t src;
            aibaby::Scalar w;
            net.in_synapse(i, a, src, w);
            sum += std::fabs(double(w));
          }
          const double ratio = sum / target;
          out.press += ratio;
          if (ratio > band || ratio < 1.0 / band) out.outside += 1.0;
          ++out.press_n;
        }
      }

      if (t < settle) continue;
      if (s.brain.vocal_frame() == last_frame) continue;
      last_frame = s.brain.vocal_frame();
      const aibaby::Scalar* g = s.brain.vocal_groups();
      for (uint32_t k = 0; k < aibaby::kVocalGroups; ++k) sum[k] += double(g[k]);
      amp += double(s.brain.voice().amplitude);
      voiced += s.brain.voice().voicing > aibaby::Scalar(0.5) ? 1.0 : 0.0;
      ++frames;
    }

    // What the two modules looked like at the end of this stretch. The rate
    // EMA is a one-second window and the scored part of a probe is about that
    // long, so this is their state while the toy was in view.
    pool(cen_m, kCentralBins, last_central);
    pool(vis_m, 0, last_vision);
    pool(cen_m, 0, last_central_full);
    pool(hip_m, 0, last_hippo);
    // The larynx itself. This table read `vision` and `central` for a long
    // time and never the module the whole G3 question is about, so "is the
    // object present in vocal *while the creature is being taught*" had no
    // answer here — m3probe could only say what happens with learning off.
    pool(voc_m, 0, last_vocal);

    std::vector<double> f;
    if (frames == 0) return f;
    for (uint32_t k = 0; k < aibaby::kVocalGroups; ++k) f.push_back(sum[k] / double(frames));
    f.push_back(amp / double(frames));
    f.push_back(voiced / double(frames));
    return f;
  };

  // --- Phase A: what does *this* larynx do when it hears each word? ---------
  // Repeated a few times per word and averaged, because one trial of a noisy
  // motor module is not a posture.
  std::vector<std::vector<double>> word_mean(2, std::vector<double>(aibaby::kVocalGroups, 0.0));
  std::vector<std::vector<std::vector<double>>> echo_feat(2);
  constexpr uint32_t kCalibTrials = 8;
  for (uint32_t rep = 0; rep < kCalibTrials; ++rep) {
    for (int w = 0; w < 2; ++w) {
      const std::vector<double> f =
          observe(w, w, true, false, kM3TrialTicks, kM3SettleTicks, nullptr, nullptr,
                  nullptr, nullptr);
      if (f.empty()) continue;
      for (uint32_t k = 0; k < aibaby::kVocalGroups; ++k) word_mean[w][k] += f[k];
      echo_feat[w].push_back(f);
    }
  }
  for (int w = 0; w < 2; ++w) {
    for (uint32_t k = 0; k < aibaby::kVocalGroups; ++k) {
      word_mean[w][k] /= double(kCalibTrials);
    }
  }

  // The discriminative axis between the two postures. Rewarding the projection
  // onto *this* rather than onto either posture alone is what makes the teacher
  // ask for a difference rather than for loudness: a voice that drifts toward
  // both targets at once earns nothing.
  std::vector<double> axis(aibaby::kVocalGroups, 0.0);
  std::vector<double> mid(aibaby::kVocalGroups, 0.0);
  for (uint32_t k = 0; k < aibaby::kVocalGroups; ++k) {
    axis[k] = word_mean[1][k] - word_mean[0][k];
    mid[k] = 0.5 * (word_mean[1][k] + word_mean[0][k]);
  }

  // The echo, scored the same way the milestone is, for reference.
  {
    // Interleaved, not one class after the other: holdout_accuracy splits on
    // position, so class-ordered data trains on nothing but word 0 and tests on
    // nothing but word 1. That reads as 0.000 and looks like a dead larynx.
    std::vector<std::vector<double>> x;
    std::vector<int> y;
    const size_t pairs = echo_feat[0].size() < echo_feat[1].size() ? echo_feat[0].size()
                                                                   : echo_feat[1].size();
    for (size_t i = 0; i < pairs; ++i) {
      for (int w = 0; w < 2; ++w) { x.push_back(echo_feat[w][i]); y.push_back(w); }
    }
    if (x.size() >= 8) out.echo = holdout_accuracy(x, y, x.size() / 2);
  }

  // --- Phases B and C: teach densely, probe silently ------------------------
  std::vector<std::vector<double>> probe_x;
  std::vector<std::vector<double>> central_x, vision_x, cfull_x, hippo_x, vocal_x;
  std::vector<int> probe_y;
  double reward_acc = 0;
  uint32_t reward_n = 0;
  uint32_t since_probe = 0;

  while (now + kM3TrialTicks + kM3ProbeTicks <= ticks) {
    const int object = int(rng.next() & 1u);
    // The control draws its own coin so the two arms consume the same random
    // numbers and see the same toys in the same order.
    const int coin = int(rng.next() & 1u);
    const int target_for = consistent ? object : coin;

    // Toward object 1's posture for object 1, away from it for object 0.
    std::vector<double> shaped(aibaby::kVocalGroups, 0.0);
    for (uint32_t k = 0; k < aibaby::kVocalGroups; ++k) {
      shaped[k] = target_for == 1 ? axis[k] : -axis[k];
    }

    observe(mode == 1 ? 0 : object, mode == 0 ? 0 : object, mode != 0, mode != 1,
            kM3TrialTicks, kM3SettleTicks, shaped.data(), mid.data(), &reward_acc,
            &reward_n);
    ++out.taught;

    if (++since_probe < kM3TrainPerProbe) continue;
    since_probe = 0;

    // Alternating rather than drawn, so the train and test halves are balanced
    // by construction. The classifier's split is positional.
    const int probe_object = int(out.probes_seen++ & 1u);
    // The probe is the CS alone in mode 2: object shown, caregiver silent. That
    // is the whole question — has the sight come to evoke the sound by itself.
    const std::vector<double> f =
        observe(mode == 1 ? 0 : probe_object, mode == 1 ? probe_object : 0, mode == 1,
                mode != 1, kM3ProbeTicks, kM3ProbeSettleTicks, nullptr, nullptr,
                nullptr, nullptr, /*want_probe_presence=*/true);
    if (f.empty()) continue;
    probe_x.push_back(f);
    probe_y.push_back(probe_object);
    central_x.push_back(last_central);
    vision_x.push_back(last_vision);
    cfull_x.push_back(last_central_full);
    vocal_x.push_back(last_vocal);
    hippo_x.push_back(last_hippo);
  }

  out.probes = uint32_t(probe_x.size());
  out.reward = reward_n ? reward_acc / double(reward_n) : 0.0;
  if (out.press_n) {
    out.press /= double(out.press_n);
    out.outside /= double(out.press_n);
  }
  if (probe_x.size() < 12) return out;

  const size_t train = probe_x.size() / 2;
  out.vocal = holdout_accuracy(probe_x, probe_y, train);
  std::vector<int> shuffled = probe_y;
  for (size_t i = shuffled.size(); i > 1; --i) {
    const size_t j = size_t(rng.next() % uint64_t(i));
    std::swap(shuffled[i - 1], shuffled[j]);
  }
  out.shuffled = holdout_accuracy(probe_x, shuffled, train);
  out.central = holdout_accuracy(central_x, probe_y, train);
  out.vision = holdout_accuracy(vision_x, probe_y, train);
  out.central_full = holdout_accuracy(cfull_x, probe_y, train);
  // Each shuffled control is computed on the *same* feature set as the row it
  // licenses. A shuffled column borrowed from a different, coarser feature set
  // is not a control for this one — high-dimensional nearest-centroid has its
  // own chance level and it is not necessarily 0.5.
  out.central_shuffled = holdout_accuracy(cfull_x, shuffled, train);
  out.vision_shuffled = holdout_accuracy(vision_x, shuffled, train);
  if (out.has_hippo) {
    out.hippo = holdout_accuracy(hippo_x, probe_y, train);
    out.hippo_shuffled = holdout_accuracy(hippo_x, shuffled, train);
    out.hippo_corr = holdout_accuracy_corr(hippo_x, probe_y, train);
    out.hip_code = code_stats(hippo_x, probe_y);
  }
  out.central_corr = holdout_accuracy_corr(cfull_x, probe_y, train);
  out.vision_corr = holdout_accuracy_corr(vision_x, probe_y, train);
  out.vocal_code_acc = holdout_accuracy(vocal_x, probe_y, train);
  out.vocal_code_shuf = holdout_accuracy(vocal_x, shuffled, train);
  out.vis_code = code_stats(vision_x, probe_y);
  out.cen_code = code_stats(cfull_x, probe_y);
  out.ok = true;
  return out;
}


bool run_g3probe_impl(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose,
                      int mode);

bool run_g3probe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  return run_g3probe_impl(blob, ticks, verbose, 0);
}

// Classical conditioning. Teaching pairs the object with the spoken word; the
// probe shows the object alone. Read the ABSOLUTE ceiling here, not the
// taught-minus-random margin: Hebbian consolidation is reward-independent, so
// if it works it lifts both arms and the margin stays flat by construction.
bool run_pairprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  return run_g3probe_impl(blob, ticks, verbose, 2);
}

// Identical to g3probe except for which sense carries the condition. g3probe
// hands the rule a condition that reaches vocal at chance, so its null cannot
// separate "R-STDP cannot learn a conditional mapping" from "there was nothing
// at the larynx to condition on". Here the object is hidden and the caregiver
// speaks instead — a condition that reaches vocal at 0.960.
//
// The taught-vs-random-target control is what makes this readable. The creature
// already reacts to a word natively (the calibration echo is 0.825 with no
// learning at all), and that reactivity is present in *both* arms, so only
// "taught beats its own random control" counts as learning.
bool run_condprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  return run_g3probe_impl(blob, ticks, verbose, 1);
}

bool run_g3probe_impl(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose,
                      int mode) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  const double dt = double(dna.header().sim.dt_ms);
  // Three experiments share this protocol and differ only in how the condition
  // reaches the creature, so they also differ in which row is their positive
  // control. Named here rather than at the bottom so the table says which of
  // the three produced it.
  const bool by_ear = mode == 1;
  const char* probe = mode == 1 ? "condprobe" : mode == 2 ? "pairprobe" : "g3probe";

  std::printf("  session           %.1f s of simulated life x %u creatures x 2 arms\n",
              double(ticks) * dt / 1000.0, kM3Replicates);
  instrument(probe, dna.header().seed, kM3Replicates, "creatures x 2 arms");
  std::printf("  the teacher is idealised: dense, immediate, and it says which\n"
              "  object this is. The targets are postures this creature was\n"
              "  observed to produce when it heard each word.\n\n");
  std::printf("  %-4s %-8s %-9s %-9s %-7s %s\n", "seed", "arm", "voice", "shuffled",
              "echo", "probes");

  double sum_taught = 0, sum_ctl = 0, sum_shuf = 0, sum_echo = 0;
  double sum_press = 0, sum_outside = 0, sum_central = 0, sum_cshuf = 0;
  double sum_vision = 0, sum_vshuf = 0, sum_cfull = 0;
  double vi = 0, vd = 0, vs = 0, ci = 0, cd = 0, cs = 0;
  double hi = 0, hd = 0, hs = 0, sum_hip = 0, sum_hshuf = 0;
  double hcorr = 0, ccorr = 0, vcorr = 0;
  double va_ = 0, vda = 0, vw = 0, vb = 0;
  double ca_ = 0, cda = 0, cw = 0, cb = 0;
  double ha_ = 0, hda = 0, hw = 0, hb = 0;
  bool any_hippo = false;
  uint32_t valid = 0, above = 0, beat = 0, max_probes = 0;
  double sum_vocacc = 0, sum_vocshuf = 0;
  double tseen = 0, theard = 0, pseen = 0, pheard = 0;

  for (uint32_t r = 0; r < kM3Replicates; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));

    const G3Probe taught = run_g3probe_session(variant, ticks, true, 0x51E7u, mode);
    const G3Probe ctl = run_g3probe_session(variant, ticks, false, 0x51E7u, mode);
    if (!taught.ok || !ctl.ok) {
      std::printf("  %-4u  (inconclusive: %u / %u probes)\n", r, taught.probes, ctl.probes);
      continue;
    }
    ++valid;
    sum_taught += taught.vocal;
    sum_ctl += ctl.vocal;
    sum_shuf += taught.shuffled;
    sum_echo += taught.echo;
    sum_press += taught.press;
    sum_outside += taught.outside;
    sum_central += taught.central;
    sum_cshuf += taught.central_shuffled;
    sum_hip += taught.hippo; sum_hshuf += taught.hippo_shuffled;
    hi += taught.hip_code.informative; hd += taught.hip_code.mean_d;
    hs += taught.hip_code.sparseness;
    any_hippo = any_hippo || taught.has_hippo;
    sum_vision += taught.vision;
    sum_vshuf += taught.vision_shuffled;
    sum_cfull += taught.central_full;
    vi += taught.vis_code.informative; vd += taught.vis_code.mean_d;
    vs += taught.vis_code.sparseness;
    va_ += taught.vis_code.active; vda += taught.vis_code.mean_d_active;
    vw += taught.vis_code.r_within; vb += taught.vis_code.r_between;
    ca_ += taught.cen_code.active; cda += taught.cen_code.mean_d_active;
    cw += taught.cen_code.r_within; cb += taught.cen_code.r_between;
    ha_ += taught.hip_code.active; hda += taught.hip_code.mean_d_active;
    hw += taught.hip_code.r_within; hb += taught.hip_code.r_between;
    hcorr += taught.hippo_corr; ccorr += taught.central_corr;
    vcorr += taught.vision_corr;
    ci += taught.cen_code.informative; cd += taught.cen_code.mean_d;
    cs += taught.cen_code.sparseness;
    if (taught.vocal >= 0.75) ++above;
    if (taught.vocal > ctl.vocal) ++beat;
    if (taught.probes > max_probes) max_probes = taught.probes;
    tseen += taught.teach_ticks > 0 ? taught.teach_seen / taught.teach_ticks : 0.0;
    theard += taught.teach_ticks > 0 ? taught.teach_heard / taught.teach_ticks : 0.0;
    pseen += taught.probe_ticks > 0 ? taught.probe_seen / taught.probe_ticks : 0.0;
    pheard += taught.probe_ticks > 0 ? taught.probe_heard / taught.probe_ticks : 0.0;
    sum_vocacc += taught.vocal_code_acc;
    sum_vocshuf += taught.vocal_code_shuf;
    std::printf("  %-4u %-8s %-9.3f %-9.3f %-7.3f %u\n", r, "taught", taught.vocal,
                taught.shuffled, taught.echo, taught.probes);
    std::printf("  %-4u %-8s %-9.3f %-9.3f %-7.3f %u\n", r, "random", ctl.vocal,
                ctl.shuffled, ctl.echo, ctl.probes);
    if (verbose) {
      std::printf("       mean |R-E[R]| taught %.4f, random %.4f\n", taught.reward,
                  ctl.reward);
    }
  }

  if (!valid) {
    std::printf("\n  g3probe INCONCLUSIVE — no creature produced enough probes.\n");
    return false;
  }

  // Half the probes train and half test, so the finest accuracy step this run
  // can express is 2/probes. At the default tick count that is 15 probes and a
  // step of 0.125, which is coarser than every effect this experiment is
  // looking for — and worse, the condition table below then reads `central` at
  // 0.475 and the closing paragraph blames vision->central for the whole loss.
  // At 141 probes the same creature reads 0.724 and the loss is downstream. An
  // underpowered run here does not merely add noise, it points at the wrong
  // half of the machine, so it has to say so out loud.
  if (max_probes > 0 && 2.0 / double(max_probes) > 0.02) {
    std::printf(
        "\n  UNDERPOWERED: %u probes gives an accuracy step of %.3f. Every number\n"
        "  below is quantised to that, and the condition table has pointed at the\n"
        "  wrong bottleneck at this resolution. Re-run with --ticks 900000.\n",
        max_probes, 2.0 / double(max_probes));
  }

  const double v = sum_taught / valid;
  const double c = sum_ctl / valid;
  // Presence, not just identity. A condition that is on the wrong channel is
  // obvious; one that is on the right channel at the wrong time is not, and it
  // has produced three wrong readings in this file's history — a word that
  // stopped 200 ticks before the probe window opened, and a pairing trial that
  // spent 55% of itself as CS-without-US. Both looked healthy everywhere else.
  std::printf("\n  condition present, as a fraction of each scored window\n"
              "    teaching   object %.2f   word %.2f\n"
              "    probe      object %.2f   word %.2f\n",
              tseen / valid, theard / valid, pseen / valid, pheard / valid);
  std::printf("\n  mean held-out accuracy on the probe voice\n");
  std::printf("    idealised teacher    %.3f   <- the ceiling\n", v);
  std::printf("    random target        %.3f   (dense reward, no object mapping)\n", c);
  std::printf("    labels shuffled      %.3f   (must sit at chance)\n", sum_shuf / valid);
  std::printf("    calibration echo     %.3f\n", sum_echo / valid);
  std::printf("    at or above 0.75     %u of %u creatures\n", above, valid);
  std::printf("    beat its own control %u of %u creatures\n", beat, valid);
  // Read these before concluding anything from a relaxation sweep: if the
  // larynx sits inside the dead band, synaptic scaling never ran there and
  // relaxing it was never going to change the row above.
  std::printf("    vocal sum|w| / setpoint  %.3f, outside the band %.1f%% of samples\n",
              sum_press / valid, 100.0 * sum_outside / valid);
  // The condition side. The voice column above can only be a verdict on the
  // learning rule if this one says the question was answerable.
  // Which of these rows is the positive control depends on the arm, and getting
  // that wrong is not cosmetic: condprobe hides the object, so its `vision` row
  // is *supposed* to sit at chance, and labelling that row "positive control"
  // on every arm invites reading condprobe's healthy 0.507 as a broken probe —
  // or, worse, not noticing when the row that does matter has fallen over.
  std::printf("\n  cube vs ball, read off the creature's own activity\n");
  std::printf("    vision, per neuron   %.3f   (shuffled %.3f)  %s\n",
              sum_vision / valid, sum_vshuf / valid,
              by_ear ? "<- nothing is shown: chance is CORRECT"
                     : "<- positive control");
  std::printf("    central, per neuron  %.3f   (shuffled %.3f)  <- the condition\n",
              sum_cfull / valid, sum_cshuf / valid);
  std::printf("    vocal,   per neuron  %.3f   (shuffled %.3f)  %s\n",
              sum_vocacc / valid, sum_vocshuf / valid,
              by_ear ? "<- positive control: does it ARRIVE"
                     : "<- does it ARRIVE");
  std::printf("    central, 16 bins     %.3f   (pooled; kept because it reads lower —\n"
              "                                 pooling loses the code, see vision)\n",
              sum_central / valid);
  if (any_hippo) {
    std::printf("    hippocampus          %.3f   (shuffled %.3f)  <- must beat central\n",
                sum_hip / valid, sum_hshuf / valid);
  }
  // How the distinction is carried, not just how much survives. A few strong
  // neurons and many weak ones read the same in a classifier and call for
  // opposite fixes.
  std::printf("\n  how that distinction is carried\n");
  std::printf("    %-10s %-14s %-12s %s\n", "", "|d'| > 0.5", "mean |d'|", "sparseness");
  std::printf("    %-10s %-14.1f%% %-12.3f %.3f\n", "vision", 100.0 * vi / valid,
              vd / valid, vs / valid);
  std::printf("    %-10s %-14.1f%% %-12.3f %.3f\n", "central", 100.0 * ci / valid,
              cd / valid, cs / valid);
  if (any_hippo) {
    std::printf("    %-10s %-14.1f%% %-12.3f %.3f\n", "hippocamp", 100.0 * hi / valid,
                hd / valid, hs / valid);
  }
  std::printf("    (sparseness: 1.0 = every neuron equally active, ->0 = few active)\n");

  // The same three modules read without the sparse-code penalty. `mean |d'|`
  // above divides by every neuron including the silent ones, and the z-scored
  // classifier amplifies a near-silent neuron's noise; both punish a module for
  // being sparse, which is what a hippocampus is for. These do not, and they
  // are the columns to use whenever a sparse module is in the comparison.
  std::printf("\n  the same, without the penalty a sparse code pays\n");
  std::printf("    %-10s %-8s %-12s %-10s %-11s %s\n", "", "active", "|d'| active",
              "r within", "r between", "corr-clf");
  std::printf("    %-10s %-8.2f %-12.3f %-10.3f %-11.3f %.3f\n", "vision",
              va_ / valid, vda / valid, vw / valid, vb / valid, vcorr / valid);
  std::printf("    %-10s %-8.2f %-12.3f %-10.3f %-11.3f %.3f\n", "central",
              ca_ / valid, cda / valid, cw / valid, cb / valid, ccorr / valid);
  if (any_hippo) {
    std::printf("    %-10s %-8.2f %-12.3f %-10.3f %-11.3f %.3f\n", "hippocamp",
                ha_ / valid, hda / valid, hw / valid, hb / valid, hcorr / valid);
  }
  std::printf("    (pattern separation is r_within - r_between going UP, which a\n"
              "     centroid classifier on raw rates can miss entirely)\n");

  // This experiment cannot fail in the way a milestone fails: whatever it reads
  // is the answer. What it can do is be unreadable, and that is what the two
  // controls are for.
  std::printf(
      "\n  g3probe is a bound, not a criterion. It reads %.2f against a 0.75 bar:\n"
      "  %s\n",
      v,
      v >= 0.75
          ? "the creature CAN be taught object-specific vocalisations, so what G3\n"
            "  lacks is the teaching signal — the picture->sound->voice route — and\n"
            "  not the capacity to comply with one."
          : "a perfect teacher cannot make this voice depend on what the creature\n"
            "  sees. Read that against the calibration echo above: the same features\n"
            "  and the same classifier take the word-driven voice at ~0.83, so the\n"
            "  readout is not the limit and the null is not a broken pipeline. What\n"
            "  is missing is a mechanism that can learn a *conditional* mapping from\n"
            "  what is in view to what the larynx does. Node perturbation cannot: a\n"
            "  per-neuron bias is a constant, not a function of the input. That\n"
            "  leaves reward-modulated STDP on central->vocal as the only candidate\n"
            "  in the creature, and this says it does not manage it even when the\n"
            "  reward is ideal.\n"
            "\n"
            "  But weigh that against the condition table above before charging the\n"
            "  learning rule for all of it. The teacher is ideal; what the mapping\n"
            "  has to key on is only partly so — read the two losses off the\n"
            "  condition table rather than assuming which one is larger. At 141\n"
            "  probes they are vision 0.927 -> central 0.724 (-0.203) and then\n"
            "  central -> voice (-0.237), so the larger loss is the downstream one,\n"
            "  which is where m3probe and projprobe both localise it. At the default\n"
            "  tick count this same table read central at 0.475 and pointed upstream\n"
            "  instead; that estimate was resolution-limited, not a finding.");

  // Two controls, and neither is optional.
  //
  // The echo is the *readout* control: the same eleven vocal features and the
  // same held-out classifier, applied to the voice while the word is playing.
  // It reads ~0.83 with no learning at all, so if it collapses the null on the
  // teacher row is a fact about the classifier and not about the creature.
  //
  // The other is the *delivery* control, and which row it is depends on how
  // this arm delivers the condition. g3probe shows the object, so the object
  // has to be legible in `vision`; condprobe hides it and speaks instead, so
  // `vision` at chance there is correct and what has to be legible is the word
  // arriving at `vocal`. Reading the wrong one of those two would have made
  // condprobe look broken for the same reason it makes g3probe look fine.
  bool ok = true;
  ok = positive_control(probe, "the calibration echo — the voice readout itself",
                        sum_echo / valid, 0.700,
                        "The teacher row above is a fact about the classifier.") && ok;
  ok = positive_control(probe, by_ear ? "the word arriving at `vocal`, per neuron"
                                      : "the object in `vision`, per neuron",
                        by_ear ? sum_vocacc / valid : sum_vision / valid, 0.700,
                        "The condition never got into the creature; there was\n"
                        "  nothing at the larynx for the rule to condition on.") && ok;
  return ok;
}



// --- Predictive coding (DNA v15) -------------------------------------------
//
// Before sweeping a gain on a subtraction, find out whether there is anything
// worth subtracting. The critic has been running since the first milestone and
// nothing has ever looked at its *prediction* — only at the two error windows
// derived from it — so "the forward model works well enough to pay curiosity
// reward" has never been the same claim as "the forward model is good".
//
// The number that settles it is R^2 against the per-channel mean. Predicting
// each mel channel's long-run average is free and requires no brain, and it is
// what a linear model with a bias term falls back to when its features carry
// nothing. So R^2 <= 0 means the prediction is the mean spectrum wearing a hat,
// and subtracting it is mean subtraction — which might still be worth
// something, but it is not predictive coding and must not be reported as it.
//
// Split by whether the room is loud, because the specific hypothesis behind
// v15 is reafference: the creature's own voice reaches its own ears (DNA v6),
// and the association module that drives the larynx is exactly the signal the
// forward model reads. If predictive coding does anything for this creature it
// should show up as a better prediction when the only sound is its own.
// The mean is the weaker of the two baselines and passing it is not enough.
// A mel spectrum changes slowly — a vowel lasts a second and a frame is
// sixteen milliseconds — so "the next frame looks like this one" is very nearly
// free and scores well against the mean without containing one bit of model.
// If the critic cannot beat persistence then what v15 subtracts is the previous
// frame, which is a high-pass filter: possibly useful, definitely not
// prediction, and it must not be written up as prediction.
struct PcChannel {
  double n = 0;
  double mel = 0, mel2 = 0;      // for the mean-predictor baseline
  double pred = 0, pred2 = 0;    // for "is the prediction dynamic at all"
  double sq_err = 0;             // for R^2
  double sq_persist = 0;         // ...and for the baseline that matters
  double prev_dot = 0;           // corr(pred, previous frame): is it an echo?
  double prev = 0, prev2 = 0;
};


// R^2 of the forward model against predicting this channel's own mean.
// Returns 0 for a channel that never varies: a constant channel is perfectly
// predicted by its mean, and calling that a win for the model would be scoring
// silence.
double pc_r2(const PcChannel& c, bool persistence = false) {
  if (c.n < 2) return 0.0;
  const double mean = c.mel / c.n;
  const double var = c.mel2 / c.n - mean * mean;
  if (var <= 1e-12) return 0.0;
  const double sq = persistence ? c.sq_persist : c.sq_err;
  return 1.0 - (sq / c.n) / var;
}


// How much of the prediction is simply the frame before it.
double pc_prev_corr(const PcChannel& c) {
  if (c.n < 2) return 0.0;
  const double mp = c.pred / c.n, mv = c.prev / c.n;
  const double cov = c.prev_dot / c.n - mp * mv;
  const double sp = std::sqrt(std::max(0.0, c.pred2 / c.n - mp * mp));
  const double sv = std::sqrt(std::max(0.0, c.prev2 / c.n - mv * mv));
  return (sp > 1e-9 && sv > 1e-9) ? cov / (sp * sv) : 0.0;
}


double pc_sd(double sum, double sum2, double n) {
  if (n < 2) return 0.0;
  const double m = sum / n;
  const double v = sum2 / n - m * m;
  return v > 0.0 ? std::sqrt(v) : 0.0;
}


// --- Can R-STDP build a conditional mapping on central->vocal at all? -------
//
// The suspect that outlived every other one. Wiring has been ruled out from
// both sides — widening `central->vocal` and thinning the arcuate each leave
// the voice at chance — and five separate improvements to what the association
// module represents have moved nothing downstream. What has never been looked
// at is the quantity R-STDP actually spends: the eligibility trace on that
// tract.
//
// The question is sharper than "is there a trace". Reward does not create
// eligibility, it only cashes it — the trace is pure pre/post spike timing and
// would look the same with the caregiver absent. So a conditional mapping
// (this object -> this posture, that object -> that one) is possible **only if
// the eligibility pattern itself differs between the two conditions** at the
// moment reward lands. If it does not, then no reward schedule whatsoever can
// produce a conditional voice: R-STDP can scale the tract up or down and
// nothing else. That would explain every observation on record, including why
// the echo tracks whichever tract into vocal is denser.
//
// So: hold the creature in each condition, snapshot the tract's eligibility
// vector at the same phase of every trial, and ask a held-out classifier which
// object it was. `auditory->vocal` is the positive control — that tract
// demonstrably carries a conditional signal, because the echo reads 0.825 —
// and it is also **subsampled to central->vocal's synapse count**, because it
// is five times denser and a classifier with more features wins for free.
struct EligProbe {
  double central = 0, central_shuf = 0;
  double arcuate = 0, arcuate_matched = 0, arcuate_shuf = 0;
  double central_absE = 0, arcuate_absE = 0;
  double central_live = 0;       // fraction of the tract with |e| above the floor
  double credit = 0;             // the same classification on what reward multiplies
  // The algebra says e_ij ~ r_i * r_j * K, so every synapse sharing a
  // postsynaptic neuron carries the same r_j. `post_share` is how much of the
  // trace's trial-to-trial variance that per-target factor alone explains, and
  // `divided` is the classification after dividing it out.
  double post_share = 0;
  double divided = 0;
  double central_r = 0;          // corr(mean e | A, mean e | B): 1.0 = no distinction
  double arcuate_r = 0;          // the same on the size-matched arcuate
  // Does a synapse's trace inherit the conditionality of the cell behind it?
  // Neurons are ranked by |d'| on the TRAINING trials only and the split
  // applied to the test half, so selecting them cannot leak the label.
  double from_informative = 0;   // trace classification, top-decile sources
  double from_silent = 0;        // ...and bottom-decile, same synapse count
  double informative_frac = 0;   // share of central neurons with |d'| > 0.5
  double mean_d = 0;
  uint32_t central_n = 0, arcuate_n = 0, trials = 0;
  bool ok = false;
};


double vector_corr(const std::vector<double>& a, const std::vector<double>& b) {
  if (a.size() != b.size() || a.empty()) return 0.0;
  const double n = double(a.size());
  double sa = 0, sb = 0;
  for (size_t i = 0; i < a.size(); ++i) { sa += a[i]; sb += b[i]; }
  const double ma = sa / n, mb = sb / n;
  double cov = 0, va = 0, vb = 0;
  for (size_t i = 0; i < a.size(); ++i) {
    cov += (a[i] - ma) * (b[i] - mb);
    va += (a[i] - ma) * (a[i] - ma);
    vb += (b[i] - mb) * (b[i] - mb);
  }
  return (va > 1e-18 && vb > 1e-18) ? cov / std::sqrt(va * vb) : 0.0;
}


EligProbe run_eligprobe_session(const std::vector<uint8_t>& blob, uint64_t ticks) {
  EligProbe out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) return out;

  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  Ear ear;
  if (!ear.configure(acfg, error)) return out;
  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  Retina retina;
  if (!retina.configure(vcfg, error)) return out;

  const int32_t aud = s.dna.module_with_role(aibaby::ModuleRole::kAuditory);
  const int32_t cen = s.dna.module_with_role(aibaby::ModuleRole::kAssociation);
  const int32_t voc = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
  if (aud < 0 || cen < 0 || voc < 0) return out;

  const aibaby::Network& net0 = s.brain.network();
  std::vector<uint32_t> cv(net0.tract_synapses(uint32_t(cen), uint32_t(voc), nullptr, 0));
  std::vector<uint32_t> av(net0.tract_synapses(uint32_t(aud), uint32_t(voc), nullptr, 0));
  if (cv.empty() || av.empty()) return out;
  net0.tract_synapses(uint32_t(cen), uint32_t(voc), cv.data(), uint32_t(cv.size()));
  net0.tract_synapses(uint32_t(aud), uint32_t(voc), av.data(), uint32_t(av.size()));
  out.central_n = uint32_t(cv.size());
  out.arcuate_n = uint32_t(av.size());
  const aibaby::ModuleState& ms_c = net0.module(uint32_t(cen));
  const uint32_t ms_c_count = ms_c.count;

  const double dt = double(s.dna.header().sim.dt_ms);
  const uint32_t spt = uint32_t(double(acfg.sample_rate) * dt / 1000.0 + 0.5);
  VowelSource voice(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<float> pcm(spt);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  const uint64_t frame_ticks = uint64_t(1000.0 / double(vcfg.frame_hz) / dt + 0.5);

  constexpr uint64_t kWord = 1200;
  constexpr uint64_t kTrial = 3000;
  constexpr uint64_t kSample = 1700;   // after the word, inside the 2 s trace
  const uint64_t n_trials = ticks / kTrial;

  aibaby::Rng rng;
  rng.seed(s.dna.header().seed ^ 0xE117Bu);
  std::vector<int> order(size_t(n_trials), 0);
  for (size_t i = 0; i < order.size(); ++i) order[i] = int(i % 2);
  for (size_t i = order.size(); i > 1; --i) std::swap(order[i - 1], order[rng.next() % i]);

  std::vector<std::vector<double>> xc, xa, xam, xcen, xcred, xdiv;
  std::vector<int> y;
  std::vector<double> mean_e[2] = {std::vector<double>(cv.size(), 0.0),
                                   std::vector<double>(cv.size(), 0.0)};
  std::vector<double> mean_a[2];
  double n_cond[2] = {0, 0};
  double abs_c = 0, abs_a = 0, live_c = 0, cells_c = 0;
  double share_sum = 0, share_n = 0;
  const uint32_t vocal_n = net0.module(uint32_t(voc)).begin + net0.module(uint32_t(voc)).count;
  std::vector<uint32_t> cv_target(cv.size(), 0);
  for (size_t i = 0; i < cv.size(); ++i) cv_target[i] = net0.synapse_target(cv[i]);

  // Matched control: an evenly spaced subsample of the arcuate, the same size
  // as central->vocal. Evenly spaced rather than random so it spans the tract.
  std::vector<uint32_t> av_matched;
  if (av.size() >= cv.size() && !cv.empty()) {
    for (size_t i = 0; i < cv.size(); ++i) {
      av_matched.push_back(av[i * av.size() / cv.size()]);
    }
  }
  mean_a[0].assign(av_matched.size(), 0.0);
  mean_a[1].assign(av_matched.size(), 0.0);

  for (uint64_t k = 0; k < n_trials; ++k) {
    const int object = order[size_t(k)];
    const Toy toy = m3_toy(rng, object);
    bool slept = false;
    std::vector<double> cen_counts(ms_c_count, 0.0);

    for (uint64_t t = 0; t < kTrial; ++t) {
      if (t % frame_ticks == 0) {
        scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }
      const bool sounding = t < kWord;
      const Word& w = kWords[object];
      voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                   pcm.data(), spt);
      ear.tick(s.brain, pcm.data(), spt);
      // Praise as the caregiver would give it. It does not shape the trace —
      // eligibility is spike timing and reward only cashes it — but it keeps
      // the creature in the state a real session puts it in.
      if (sounding && t % kFeedbackPeriodTicks == 0) s.brain.praise(kPraiseValue);
      s.brain.step();
      if (s.brain.asleep()) slept = true;
      {
        const aibaby::Network& n2 = s.brain.network();
        for (uint32_t i = 0; i < n2.spike_count(); ++i) {
          const uint32_t idx = n2.spikes()[i];
          if (idx >= ms_c.begin && idx < ms_c.begin + ms_c_count) {
            cen_counts[idx - ms_c.begin] += 1.0;
          }
        }
      }

      if (t != kSample) continue;
      const aibaby::Network& net = s.brain.network();
      std::vector<double> ec(cv.size()), ea(av.size()), eam(av_matched.size());
      std::vector<double> cr(cv.size());
      for (size_t i = 0; i < cv.size(); ++i) {
        ec[i] = double(net.synapse_eligibility(cv[i]));
        cr[i] = double(net.synapse_credit(cv[i]));
        abs_c += std::fabs(ec[i]);
        cells_c += 1;
        if (std::fabs(ec[i]) > 1e-6) live_c += 1;
      }
      for (size_t i = 0; i < av.size(); ++i) {
        ea[i] = double(net.synapse_eligibility(av[i]));
        abs_a += std::fabs(ea[i]);
      }
      for (size_t i = 0; i < av_matched.size(); ++i) {
        eam[i] = double(net.synapse_eligibility(av_matched[i]));
      }
      if (!slept) {
        for (size_t i = 0; i < cv.size(); ++i) mean_e[object][i] += ec[i];
        for (size_t i = 0; i < eam.size(); ++i) mean_a[object][i] += eam[i];
        n_cond[object] += 1;
        xc.push_back(ec);
        xa.push_back(ea);
        xam.push_back(eam);
        xcen.push_back(cen_counts);
        xcred.push_back(cr);
        // Divide each synapse by the mean over the synapses that share its
        // target. If the common mode is multiplicative and per-target, this
        // leaves the presynaptic pattern and nothing else.
        std::vector<double> dv(cv.size(), 0.0);
        {
          std::vector<double> tsum(vocal_n, 0.0), tcnt(vocal_n, 0.0);
          for (size_t i = 0; i < cv.size(); ++i) {
            tsum[cv_target[i]] += ec[i];
            tcnt[cv_target[i]] += 1.0;
          }
          for (size_t i = 0; i < cv.size(); ++i) {
            const uint32_t t = cv_target[i];
            const double m = tcnt[t] > 0 ? tsum[t] / tcnt[t] : 0.0;
            dv[i] = std::fabs(m) > 1e-12 ? ec[i] / m : 0.0;
          }
          // How much of this trial's variance the per-target means account for.
          double tot = 0, exp_ = 0, mean = 0;
          for (size_t i = 0; i < cv.size(); ++i) mean += ec[i];
          mean /= double(cv.size());
          for (size_t i = 0; i < cv.size(); ++i) {
            const uint32_t t = cv_target[i];
            const double m = tcnt[t] > 0 ? tsum[t] / tcnt[t] : 0.0;
            tot += (ec[i] - mean) * (ec[i] - mean);
            exp_ += (m - mean) * (m - mean);
          }
          if (tot > 1e-18) { share_sum += exp_ / tot; share_n += 1; }
        }
        xdiv.push_back(dv);
        y.push_back(object);
      }
    }
  }

  out.trials = uint32_t(y.size());
  if (out.trials < 16) return out;
  const size_t train = out.trials / 2;
  std::vector<int> shuf = y;
  for (size_t i = shuf.size(); i > 1; --i) std::swap(shuf[i - 1], shuf[rng.next() % i]);

  out.central = holdout_accuracy(xc, y, train);
  out.credit = holdout_accuracy(xcred, y, train);
  out.divided = holdout_accuracy(xdiv, y, train);
  out.post_share = share_n > 0 ? share_sum / share_n : 0.0;
  out.central_shuf = holdout_accuracy(xc, shuf, train);
  out.arcuate = holdout_accuracy(xa, y, train);
  out.arcuate_shuf = holdout_accuracy(xa, shuf, train);
  out.arcuate_matched = xam.empty() ? 0.0 : holdout_accuracy(xam, y, train);
  out.central_absE = cells_c > 0 ? abs_c / cells_c : 0.0;
  out.arcuate_absE = (av.size() && out.trials) ? abs_a / (double(av.size()) * double(out.trials)) : 0.0;
  out.central_live = cells_c > 0 ? live_c / cells_c : 0.0;

  for (int c = 0; c < 2; ++c) {
    if (n_cond[c] <= 0) continue;
    for (double& v : mean_e[c]) v /= n_cond[c];
    for (double& v : mean_a[c]) v /= n_cond[c];
  }
  out.central_r = vector_corr(mean_e[0], mean_e[1]);
  out.arcuate_r = vector_corr(mean_a[0], mean_a[1]);

  // --- Does the trace inherit the cell's conditionality? -------------------
  //
  // The two-tract comparison above is suggestive but correlational: central and
  // B2 differ in more than how conditional they are. This asks the same
  // question *inside one creature*, where the post side, the tract, the trial
  // structure and the readout are all held fixed and the only thing that varies
  // is which presynaptic cell a synapse hangs off.
  //
  // |d'| per central neuron is computed on the **training trials only** and the
  // resulting split applied to the held-out half. Ranking on all trials would
  // pick the neurons that happen to separate the test set too, and the number
  // would then be a measure of how many neurons there are.
  {
    std::vector<double> sum[2], sq[2];
    double cnt[2] = {0, 0};
    for (int c = 0; c < 2; ++c) { sum[c].assign(ms_c_count, 0.0); sq[c].assign(ms_c_count, 0.0); }
    for (size_t i = 0; i < train; ++i) {
      const int c = y[i];
      cnt[c] += 1;
      for (uint32_t j = 0; j < ms_c_count; ++j) {
        sum[c][j] += xcen[i][j];
        sq[c][j] += xcen[i][j] * xcen[i][j];
      }
    }
    std::vector<double> dprime(ms_c_count, 0.0);
    uint32_t informative = 0;
    double d_total = 0;
    for (uint32_t j = 0; j < ms_c_count; ++j) {
      if (cnt[0] < 2 || cnt[1] < 2) break;
      const double m0 = sum[0][j] / cnt[0], m1 = sum[1][j] / cnt[1];
      const double v0 = sq[0][j] / cnt[0] - m0 * m0, v1 = sq[1][j] / cnt[1] - m1 * m1;
      const double pooled = std::sqrt(std::max(1e-9, 0.5 * (std::max(0.0, v0) + std::max(0.0, v1))));
      dprime[j] = std::fabs(m1 - m0) / pooled;
      d_total += dprime[j];
      if (dprime[j] > 0.5) ++informative;
    }
    out.informative_frac = ms_c_count ? double(informative) / double(ms_c_count) : 0.0;
    out.mean_d = ms_c_count ? d_total / double(ms_c_count) : 0.0;

    std::vector<uint32_t> rank(ms_c_count);
    for (uint32_t j = 0; j < ms_c_count; ++j) rank[j] = j;
    std::sort(rank.begin(), rank.end(),
              [&](uint32_t a2, uint32_t b2) { return dprime[a2] > dprime[b2]; });
    const uint32_t decile = ms_c_count / 10;
    if (decile >= 4) {
      std::vector<char> top(ms_c_count, 0), bot(ms_c_count, 0);
      for (uint32_t j = 0; j < decile; ++j) {
        top[rank[j]] = 1;
        bot[rank[ms_c_count - 1 - j]] = 1;
      }
      std::vector<size_t> take_top, take_bot;
      for (size_t i = 0; i < cv.size(); ++i) {
        const uint32_t src = net0.synapse_source(cv[i]) - ms_c.begin;
        if (src < ms_c_count && top[src]) take_top.push_back(i);
        else if (src < ms_c_count && bot[src]) take_bot.push_back(i);
      }
      // Size-matched, so the two rows differ in which cells feed them and in
      // nothing else. Without this the top decile could simply own more
      // synapses — a busy neuron is exactly the kind that wires widely.
      const size_t take = std::min(take_top.size(), take_bot.size());
      if (take >= 8) {
        std::vector<std::vector<double>> xt(xc.size()), xb(xc.size());
        for (size_t i = 0; i < xc.size(); ++i) {
          xt[i].resize(take); xb[i].resize(take);
          for (size_t k2 = 0; k2 < take; ++k2) {
            xt[i][k2] = xc[i][take_top[k2]];
            xb[i][k2] = xc[i][take_bot[k2]];
          }
        }
        out.from_informative = holdout_accuracy(xt, y, train);
        out.from_silent = holdout_accuracy(xb, y, train);
      }
    }
  }

  out.ok = true;
  return out;
}


bool run_eligprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  constexpr uint32_t kReps = 5;
  std::printf("  session           %.1f s x %u creatures, object + its word, shuffled\n",
              double(ticks) * double(dna.header().sim.dt_ms) / 1000.0, kReps);
  instrument("eligprobe", dna.header().seed ^ 0xE117Bu, ticks / 3000, "trials per creature");
  std::printf("  the question      is the eligibility trace on central->vocal\n"
              "                    DIFFERENT for the two objects? If it is not, no\n"
              "                    reward schedule can make the voice conditional.\n\n");
  std::printf("  %-5s %-9s %-9s %-9s %-9s %-9s %s\n", "seed", "cen->voc", "shuffled",
              "arc full", "arc match", "mean|e|", "corr(A,B)");

  double sc = 0, scs = 0, sa = 0, sam = 0, se = 0, sr = 0, sae = 0, sar = 0;
  double sti = 0, sts = 0, sif = 0, sd = 0, scr = 0, sps = 0, sdv = 0;
  uint32_t valid = 0, cn = 0, an = 0;
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    const EligProbe p = run_eligprobe_session(variant, ticks);
    if (!p.ok) { std::printf("  %-5u (inconclusive: %u trials)\n", r, p.trials); continue; }
    ++valid; cn = p.central_n; an = p.arcuate_n;
    sc += p.central; scs += p.central_shuf; sa += p.arcuate;
    sam += p.arcuate_matched; se += p.central_absE; sr += p.central_r;
    sae += p.arcuate_absE; sar += p.arcuate_r;
    scr += p.credit; sps += p.post_share; sdv += p.divided;
    sti += p.from_informative; sts += p.from_silent;
    sif += p.informative_frac; sd += p.mean_d;
    std::printf("  %-5u %-9.3f %-9.3f %-9.3f %-9.3f %-9.2e %+.3f\n", r, p.central,
                p.central_shuf, p.arcuate, p.arcuate_matched, p.central_absE, p.central_r);
  }
  if (valid < 3) { std::printf("\n  INCONCLUSIVE — %u of %u usable.\n", valid, kReps); return false; }

  const double n = double(valid);
  std::printf("\n  tract sizes       central->vocal %u synapses, arcuate %u\n", cn, an);
  std::printf("  mean |e|          central->vocal %.3e, arcuate %.3e\n", se / n, sae / n);
  std::printf("  live fraction     see per-seed column; a trace at 0 is no trace\n");
  std::printf("\n  which object, read off the eligibility trace:\n");
  std::printf("    central->vocal        %.3f   (shuffled %.3f)\n", sc / n, scs / n);
  std::printf("    arcuate, full         %.3f\n", sa / n);
  std::printf("    arcuate, size-matched %.3f   <- the control that matters\n", sam / n);
  // The size-matched arcuate is a tract known to carry a conditional trace, so
  // it is what says this probe can see anything at all. At the default 120k it
  // reads about 0.570 against a chance near 0.510 — almost no dynamic range —
  // and every other row is then a null produced by a blind instrument rather
  // than by the creature. A whole DNA v26 sweep was run and discarded for this,
  // and this check is what invalidated it. At 600k the same control reads 0.892,
  // which is why 600k is now this experiment's declared minimum.
  const bool control_ok =
      positive_control("eligprobe", "the size-matched arcuate trace", sam / n, 0.700,
                       "This probe cannot presently detect a conditional trace\n"
                       "  anywhere, so it has not found the absence of one either.");
  std::printf("    what reward multiplies %.3f   (the DNA v16 credit: equals the\n"
              "                                 row above when the baseline is off)\n", scr / n);
  std::printf("\n  the postsynaptic common mode (e_ij ~ r_i * r_j * K):\n");
  std::printf("    variance explained by the per-target factor alone  %.3f\n", sps / n);
  std::printf("    which object, after dividing that factor out       %.3f\n", sdv / n);
  std::printf("\n  how much of the trace is the same either way:\n");
  std::printf("    corr(mean e | A, mean e | B)  central->vocal   %+.3f\n", sr / n);
  std::printf("    corr(mean e | A, mean e | B)  arcuate, matched %+.3f\n", sar / n);
  std::printf("\n  inside one creature: which central cells feed the conditional part?\n");
  std::printf("    central neurons with |d'| > 0.5   %.1f%%  (mean |d'| %.3f)\n",
              100.0 * sif / n, sd / n);
  std::printf("    trace from the top-decile cells   %.3f\n", sti / n);
  std::printf("    trace from the bottom decile      %.3f   (same synapse count)\n",
              sts / n);
  std::printf("\n    A correlation near 1 means the trace is the same whichever object\n"
              "    the creature is looking at, and R-STDP can then only scale the tract.\n"
              "    The size-matched arcuate is the honest comparison: it has the same\n"
              "    number of features, so a difference between the two rows is about\n"
              "    the signal and not about the width of the classifier.\n");
  (void)verbose;
  return control_ok;
}


// --- What does reward actually write onto central->vocal? ------------------
//
// Every measurement so far has been of the eligibility *trace*, one trial at a
// time, and the trace is shot-noise limited: about 2.4 coincidences per synapse
// per trial, 64% Poisson noise, with the signal riding on a potentiation term
// that nearly cancels its depression term. Per-trial discriminability of 0.58 is
// therefore the wrong bar — R-STDP never needed to identify the object from one
// trial, it accumulates over hundreds.
//
// So this asks the question that actually decides G3, and that nothing here has
// asked: **after a session of teaching, does the accumulated weight change
// differ depending on which object was shown?**
//
// A single weight vector cannot itself be "conditional" — what makes behaviour
// conditional is that one set of weights maps two inputs to two outputs. But if
// showing a cube and showing a ball drive the *same* weight change, then no
// amount of training can separate them, because the rule is writing the same
// thing either way. That is decisive in both directions:
//
//   - dw differs by object -> the rule does differentiate, and G3's failure is
//     downstream, in the population-vector readout the genome already suspects.
//   - dw does not differ    -> R-STDP cannot learn this mapping at all.
//
// Blocked rather than interleaved, so the change is attributable to one object.
// The control that makes the number readable is **the same object run twice**
// with a different placement draw: that is the reproducibility ceiling, and
// corr(A,B) has to be read against it rather than against 1.0.
struct DwRun {
  std::vector<double> dw;
  double moved = 0;   // mean |dw|, so "nothing happened" is visible
  bool ok = false;
};

// How the caregiver behaves, because the reproducible object-independent
// component of dw has to come from somewhere and reward is the first suspect.
// kPraiseOnly leaves the effective reward systematically positive whenever the
// baseline lags, which would potentiate everything eligible in common.
// kNone is the control that decides whether reward is writing it at all: in a
// 300k-tick session nothing else touches a weight — sleep does not arrive until
// ~1.2M ticks, and vocal's synaptic scaling never leaves its dead band — so dw
// under kNone should be nil, and if it is not, the attribution is wrong.
enum class DwReward { kPraiseOnly = 0, kSymmetric = 1, kNone = 2 };

DwRun run_dw_session(const std::vector<uint8_t>& blob, uint64_t ticks, int object,
                     uint64_t placement_seed, DwReward mode) {
  DwRun out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) return out;

  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  Ear ear;
  if (!ear.configure(acfg, error)) return out;
  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  Retina retina;
  if (!retina.configure(vcfg, error)) return out;

  const int32_t cen = s.dna.module_with_role(aibaby::ModuleRole::kAssociation);
  const int32_t voc = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
  if (cen < 0 || voc < 0) return out;

  const aibaby::Network& net0 = s.brain.network();
  std::vector<uint32_t> cv(net0.tract_synapses(uint32_t(cen), uint32_t(voc), nullptr, 0));
  if (cv.empty()) return out;
  net0.tract_synapses(uint32_t(cen), uint32_t(voc), cv.data(), uint32_t(cv.size()));

  std::vector<double> before(cv.size());
  for (size_t i = 0; i < cv.size(); ++i) before[i] = double(net0.synapse_weight(cv[i]));

  const double dt = double(s.dna.header().sim.dt_ms);
  const uint32_t spt = uint32_t(double(acfg.sample_rate) * dt / 1000.0 + 0.5);
  VowelSource voice(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<float> pcm(spt);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  const uint64_t frame_ticks = uint64_t(1000.0 / double(vcfg.frame_hz) / dt + 0.5);

  aibaby::Rng rng;
  rng.seed(placement_seed);
  constexpr uint64_t kWord = 1200;
  constexpr uint64_t kTrial = 3000;

  for (uint64_t k = 0; k * kTrial < ticks; ++k) {
    const Toy toy = m3_toy(rng, object);
    for (uint64_t t = 0; t < kTrial; ++t) {
      if (t % frame_ticks == 0) {
        scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }
      const bool sounding = t < kWord;
      const Word& w = kWords[object];
      voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                   pcm.data(), spt);
      ear.tick(s.brain, pcm.data(), spt);
      if (sounding && t % kFeedbackPeriodTicks == 0) {
        if (mode == DwReward::kPraiseOnly) {
          s.brain.praise(kPraiseValue);
        } else if (mode == DwReward::kSymmetric) {
          // Balanced within the session so the mean lands at zero without the
          // baseline having to chase it. The object is fixed for the whole
          // session, so this cannot leak the label.
          s.brain.praise(((k + t / kFeedbackPeriodTicks) % 2 == 0) ? kPraiseValue
                                                                  : kScoldValue);
        }
      }
      s.brain.step();
    }
  }

  const aibaby::Network& net = s.brain.network();
  out.dw.resize(cv.size());
  double moved = 0;
  for (size_t i = 0; i < cv.size(); ++i) {
    out.dw[i] = double(net.synapse_weight(cv[i])) - before[i];
    moved += std::fabs(out.dw[i]);
  }
  out.moved = moved / double(cv.size());
  out.ok = true;
  return out;
}

bool run_dwprobe_arm(const std::vector<uint8_t>& blob, uint64_t ticks, DwReward mode) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  constexpr uint32_t kReps = 3;
  instrument("dwprobe", 0xA11CEull, kReps, "creatures per arm");
  std::printf("  session           %.1f s per arm x %u creatures, one object throughout\n",
              double(ticks) * double(dna.header().sim.dt_ms) / 1000.0, kReps);
  std::printf("  the question      does a session of ball-teaching write a DIFFERENT\n"
              "                    weight change onto central->vocal than a session of\n"
              "                    cube-teaching? If not, no training can separate them.\n\n");
  std::printf("  %-5s %-11s %-11s %-11s %s\n", "seed", "mean|dw|", "corr(A,A')",
              "corr(A,B)", "verdict");

  double sum_same = 0, sum_diff = 0, sum_moved = 0;
  uint32_t valid = 0;
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));

    const DwRun a = run_dw_session(variant, ticks, 0, 0xA11CE, mode);
    const DwRun a2 = run_dw_session(variant, ticks, 0, 0xB0B, mode);
    const DwRun b = run_dw_session(variant, ticks, 1, 0xA11CE, mode);
    if (!a.ok || !a2.ok || !b.ok) { std::printf("  %-5u (inconclusive)\n", r); continue; }
    ++valid;
    const double same = vector_corr(a.dw, a2.dw);
    const double diff = vector_corr(a.dw, b.dw);
    sum_same += same; sum_diff += diff; sum_moved += a.moved;
    std::printf("  %-5u %-11.3e %-11.3f %-11.3f %s\n", r, a.moved, same, diff,
                diff < same - 0.05 ? "differentiates" : "same either way");
  }
  if (valid < 2) { std::printf("\n  INCONCLUSIVE\n"); return false; }

  const double n = double(valid);
  std::printf("\n  mean |dw| per synapse        %.3e\n", sum_moved / n);
  std::printf("  corr(same object, redrawn)   %.3f   <- the reproducibility ceiling\n",
              sum_same / n);
  std::printf("  corr(cube vs ball)           %.3f\n", sum_diff / n);
  std::printf("\n    Read the second against the first, never against 1.0. If they are\n"
              "    equal, showing a cube and showing a ball write the same weight change\n"
              "    and R-STDP cannot build a conditional mapping on this tract. If the\n"
              "    object correlation is clearly lower, the rule does differentiate and\n"
              "    G3's failure is downstream of it.\n");
  return true;
}

bool run_dwprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  bool ok = true;
  const char* label[3] = {"praise only (what a caregiver actually does)",
                          "praise and scold balanced (mean reward zero)",
                          "no reward at all (the control: dw should be nil)"};
  for (int m = 0; m < 3; ++m) {
    std::printf("\n########## %s ##########\n", label[m]);
    ok = run_dwprobe_arm(blob, ticks, DwReward(m)) && ok;
  }
  (void)verbose;
  return ok;
}

// --- Does the word reach the association module? ---------------------------
//
// G3 is cross-modal: hear a word, pair it with a seen object. Every measurement
// this project has of the association module scores *cube versus ball*, which
// the creature can answer from vision alone — so none of them has ever tested
// whether the word arrives at all. This one presents the two words with **an
// empty field in view**, so nothing visual distinguishes the trials and the only
// route to an answer is the auditory one.
//
// B2 is the positive control and it is not optional: if the auditory module
// cannot be read for which word was just said, the trials are broken and
// central's number means nothing (rule 6). Both readouts are reported, the
// z-scored centroid and the scale-free correlation one, because the first
// penalises sparse codes and central is the module where that has bitten before.
// Three integration windows, because that is the axis on which this probe and
// `pcprobe` disagree. pcprobe reads an instantaneous fast rate and finds central
// carries nothing about the spectrum; this reads spikes accumulated over more
// than a second and finds the word at ceiling. Both can be true, and if the
// short window here lands near chance while the long one does not, that is the
// reconciliation rather than a contradiction.
constexpr uint64_t kAudWindows[3] = {50, 200, 1200};


struct AudProbe {
  double b2[3] = {0, 0, 0}, b2_shuf = 0, b2_corr = 0;
  double central[3] = {0, 0, 0}, central_shuf = 0, central_corr = 0;
  uint32_t trials = 0, skipped = 0;
  bool ok = false;
};


AudProbe run_audprobe_session(const std::vector<uint8_t>& blob, uint64_t ticks) {
  AudProbe out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) return out;

  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  Ear ear;
  if (!ear.configure(acfg, error)) return out;
  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  Retina retina;
  if (!retina.configure(vcfg, error)) return out;

  const int32_t aud = s.dna.module_with_role(aibaby::ModuleRole::kAuditory);
  const int32_t cen = s.dna.module_with_role(aibaby::ModuleRole::kAssociation);
  if (aud < 0 || cen < 0) return out;

  const double dt = double(s.dna.header().sim.dt_ms);
  const uint32_t spt = uint32_t(double(acfg.sample_rate) * dt / 1000.0 + 0.5);
  VowelSource voice(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<float> pcm(spt);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  const uint64_t frame_ticks = uint64_t(1000.0 / double(vcfg.frame_hz) / dt + 0.5);

  const aibaby::ModuleState& ms_a = s.brain.network().module(uint32_t(aud));
  const aibaby::ModuleState& ms_c = s.brain.network().module(uint32_t(cen));
  const uint32_t wa = ms_a.count, wc = ms_c.count;

  constexpr uint64_t kSpeak = 1200;   // 1.2 s of word
  constexpr uint64_t kGap = 1200;     // then quiet, so trials do not run together
  const uint64_t trial = kSpeak + kGap;
  const uint64_t n_trials = ticks / trial;

  std::vector<std::vector<double>> xa[3], xc[3];
  std::vector<int> y;
  aibaby::Rng rng;
  rng.seed(s.dna.header().seed ^ 0xA0D17u);

  // Balanced but shuffled, never alternating. Strict alternation makes the
  // label equal to the trial's parity, and anything in the creature with a
  // period of two trials — a drive cycle, a slow oscillation, the gap structure
  // itself — would then carry the label without a word ever being heard.
  std::vector<int> order(size_t(n_trials), 0);
  for (size_t i = 0; i < order.size(); ++i) order[i] = int(i % 2);
  for (size_t i = order.size(); i > 1; --i) std::swap(order[i - 1], order[rng.next() % i]);

  for (uint64_t k = 0; k < n_trials; ++k) {
    const int word = order[size_t(k)];
    std::vector<double> ba[3] = {std::vector<double>(wa, 0.0), std::vector<double>(wa, 0.0),
                                 std::vector<double>(wa, 0.0)};
    std::vector<double> bc[3] = {std::vector<double>(wc, 0.0), std::vector<double>(wc, 0.0),
                                 std::vector<double>(wc, 0.0)};
    bool slept = false;

    for (uint64_t t = 0; t < trial; ++t) {
      // An empty field, every frame. A blank scene is not darkness to a
      // centre-surround retina, it is *uniform*, which reads as zero — so the
      // eyes are open and have nothing to say, which is the condition wanted.
      if (t % frame_ticks == 0) {
        scene.render(SceneSource::Shape::kNone, 0.5f, 0.5f, 0.10f, 0.85f, 0.02f,
                     frame.data());
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }
      const bool sounding = t < kSpeak;
      const Word& w = kWords[word];
      voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                   pcm.data(), spt);
      ear.tick(s.brain, pcm.data(), spt);
      s.brain.step();
      if (s.brain.asleep()) slept = true;
      if (!sounding) continue;
      const aibaby::Network& net = s.brain.network();
      for (uint32_t i = 0; i < net.spike_count(); ++i) {
        const uint32_t idx = net.spikes()[i];
        for (int w = 0; w < 3; ++w) {
          if (t >= kAudWindows[w]) continue;
          if (idx >= ms_a.begin && idx < ms_a.begin + wa) ba[w][idx - ms_a.begin] += 1.0;
          else if (idx >= ms_c.begin && idx < ms_c.begin + wc) bc[w][idx - ms_c.begin] += 1.0;
        }
      }
    }
    if (slept) { ++out.skipped; continue; }
    for (int w = 0; w < 3; ++w) { xa[w].push_back(ba[w]); xc[w].push_back(bc[w]); }
    y.push_back(word);
  }

  out.trials = uint32_t(y.size());
  if (out.trials < 16) return out;
  const size_t train = out.trials / 2;

  std::vector<int> shuf = y;
  for (size_t i = shuf.size(); i > 1; --i) std::swap(shuf[i - 1], shuf[rng.next() % i]);

  for (int w = 0; w < 3; ++w) {
    out.b2[w] = holdout_accuracy(xa[w], y, train);
    out.central[w] = holdout_accuracy(xc[w], y, train);
  }
  out.b2_shuf = holdout_accuracy(xa[2], shuf, train);
  out.b2_corr = holdout_accuracy_corr(xa[2], y, train);
  out.central_shuf = holdout_accuracy(xc[2], shuf, train);
  out.central_corr = holdout_accuracy_corr(xc[2], y, train);
  out.ok = true;
  return out;
}


bool run_audprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  const double dt = double(dna.header().sim.dt_ms);
  constexpr uint32_t kReps = 5;

  std::printf("  session           %.1f s x %u creatures, %s\n",
              double(ticks) * dt / 1000.0, kReps,
              "two words, alternating, nothing in view");
  instrument("audprobe", dna.header().seed ^ 0xA0D17u, ticks / 2400,
             "trials per creature");
  std::printf("  the question      which word was just said, read off each module\n");
  std::printf("  chance            0.500 — trials are balanced\n\n");
  std::printf("  %-5s %-24s %-24s %-9s %s\n", "seed",
              "B2 at 50 / 200 / 1200 ms", "central, same windows", "cen corr", "shuffled");

  double sa[3] = {0, 0, 0}, sc[3] = {0, 0, 0};
  double sac = 0, sas = 0, scc = 0, scs = 0;
  uint32_t valid = 0;
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    const AudProbe p = run_audprobe_session(variant, ticks);
    if (!p.ok) { std::printf("  %-5u (inconclusive: %u trials)\n", r, p.trials); continue; }
    ++valid;
    for (int w = 0; w < 3; ++w) { sa[w] += p.b2[w]; sc[w] += p.central[w]; }
    sac += p.b2_corr; sas += p.b2_shuf; scc += p.central_corr; scs += p.central_shuf;
    char b2s[32], cns[32];
    std::snprintf(b2s, sizeof(b2s), "%.2f %.2f %.2f", p.b2[0], p.b2[1], p.b2[2]);
    std::snprintf(cns, sizeof(cns), "%.2f %.2f %.2f", p.central[0], p.central[1],
                  p.central[2]);
    std::printf("  %-5u %-24s %-24s %-9.3f %.3f\n", r, b2s, cns, p.central_corr,
                p.central_shuf);
  }
  if (valid < 3) {
    std::printf("\n  INCONCLUSIVE — %u of %u creatures usable.\n", valid, kReps);
    return false;
  }
  const double n = double(valid);
  std::printf("\n  %-10s %8s %8s %8s\n", "window", "50 ms", "200 ms", "1200 ms");
  std::printf("  %-10s %8.3f %8.3f %8.3f\n", "B2", sa[0] / n, sa[1] / n, sa[2] / n);
  std::printf("  %-10s %8.3f %8.3f %8.3f\n", "central", sc[0] / n, sc[1] / n, sc[2] / n);
  std::printf("\n  at 1200 ms:  B2 corr %.3f, shuffled %.3f | central corr %.3f, "
              "shuffled %.3f\n", sac / n, sas / n, scc / n, scs / n);

  const bool controlled = sas / n < 0.62 && scs / n < 0.62;
  const bool heard = sa[2] / n >= 0.70 || sac / n >= 0.70;
  std::printf("\n  %s\n", controlled ? "  controls at chance." :
              "  CONTROL FAILED — a shuffled readout scores above chance, so nothing\n"
              "  else here is worth reading.");
  if (controlled && !heard) {
    std::printf("  POSITIVE CONTROL FAILED — B2 itself cannot be read for which word\n"
                "  was said, so central's number is a statement about the trials.\n");
  }
  std::printf("\n  audprobe: with nothing to look at, the word is legible in B2 at %.0f%%\n"
              "  and in the association module at %.0f%%. This is the auditory half of\n"
              "  the cross-modal task, which the cube-versus-ball cascade cannot see.\n",
              std::max(sa[2], sac) / n * 100.0, std::max(sc[2], scc) / n * 100.0);
  (void)verbose;
  return controlled && heard;
}


// --- The offline ceiling ---------------------------------------------------
//
// "The critic's prediction is poor" has three possible causes and they call for
// three different repairs: the online least-mean-squares fit is bad at a job
// the features could support, the 32-bin pooling throws the information away
// before the fit sees it, or the association module simply does not know what
// sound is coming next. A ridge regression fitted offline on the same data,
// with a held-out split, tells them apart — it is the best any linear readout
// of those features could do, so whatever it fails to reach is not there.
//
// The pooling is the suspect worth naming in advance. kCriticBins is 32 over an
// association module of 400 neurons, and pooling of exactly this shape has
// already been caught destroying a signal in this project once: a 16-bin
// version of the G3 readout scored central at chance and scored *vision* at
// chance too, on a module a per-neuron readout reads at 0.98.
bool chol_decompose(std::vector<double>& a, int n) {
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j <= i; ++j) {
      double s = a[size_t(i) * n + j];
      for (int k = 0; k < j; ++k) s -= a[size_t(i) * n + k] * a[size_t(j) * n + k];
      if (i == j) {
        if (s <= 1e-12) return false;
        a[size_t(i) * n + i] = std::sqrt(s);
      } else {
        a[size_t(i) * n + j] = s / a[size_t(j) * n + j];
      }
    }
    for (int j = i + 1; j < n; ++j) a[size_t(i) * n + j] = 0.0;
  }
  return true;
}


void chol_solve(const std::vector<double>& l, int n, double* x) {
  for (int i = 0; i < n; ++i) {
    double s = x[i];
    for (int k = 0; k < i; ++k) s -= l[size_t(i) * n + k] * x[k];
    x[i] = s / l[size_t(i) * n + i];
  }
  for (int i = n - 1; i >= 0; --i) {
    double s = x[i];
    for (int k = i + 1; k < n; ++k) s -= l[size_t(k) * n + i] * x[k];
    x[i] = s / l[size_t(i) * n + i];
  }
}


// Held-out R^2 of the best ridge fit from `cols` features to `targets` outputs,
// trained on the first half of the rows and scored on the second. `order` maps
// a feature row to the target row it is paired with; passing a shuffled order
// gives the control, which must land at or below zero — a fit of 400 features
// to 7500 rows has plenty of room to memorise noise, and a shuffled fit that
// scores well means the number above it is measuring that room.
double ridge_holdout(const std::vector<float>& x, size_t rows, size_t cols,
                     const std::vector<float>& y, size_t targets, double lambda,
                     const std::vector<uint32_t>& order) {
  if (rows < 8 * cols / 7 + 8) return 0.0;  // not enough data to fit this wide
  const int n = int(cols) + 1;              // + intercept
  const size_t split = rows / 2;

  std::vector<double> a(size_t(n) * n, 0.0);
  std::vector<double> b(size_t(n) * targets, 0.0);
  std::vector<double> row(size_t(n), 0.0);
  for (size_t r = 0; r < split; ++r) {
    const float* xr = &x[r * cols];
    for (size_t c = 0; c < cols; ++c) row[c] = double(xr[c]);
    row[cols] = 1.0;
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j <= i; ++j) a[size_t(i) * n + j] += row[i] * row[j];
    }
    const float* yr = &y[size_t(order[r]) * targets];
    for (size_t t = 0; t < targets; ++t) {
      for (int i = 0; i < n; ++i) b[size_t(i) * targets + t] += row[i] * double(yr[t]);
    }
  }
  for (int i = 0; i < n; ++i) {
    for (int j = i + 1; j < n; ++j) a[size_t(i) * n + j] = a[size_t(j) * n + i];
  }
  // The intercept is deliberately left unregularised: shrinking it toward zero
  // would bias every prediction toward silence and understate the fit.
  //
  // The penalty is scaled to the data rather than taken as an absolute, because
  // the feature sets compared here differ in scale by more than an order of
  // magnitude — 24 mel channels averaging 0.09 against 400 firing rates
  // averaging 0.3 — and a fixed lambda is a different amount of shrinkage on
  // each. That is not a fair comparison, it is a comparison of how loud the
  // features happen to be. Caught by a sanity check that should have been run
  // first: with a fixed lambda of 1, fitting the mel frame from a copy of
  // itself scored 0.73 where the only defensible answer is 1.
  double trace = 0.0;
  for (size_t c = 0; c < cols; ++c) trace += a[c * n + c];
  const double lam = lambda * trace / double(cols);
  for (size_t c = 0; c < cols; ++c) a[c * n + c] += lam;

  if (!chol_decompose(a, n)) return 0.0;
  std::vector<double> w(size_t(n) * targets, 0.0);
  std::vector<double> col(size_t(n), 0.0);
  for (size_t t = 0; t < targets; ++t) {
    for (int i = 0; i < n; ++i) col[i] = b[size_t(i) * targets + t];
    chol_solve(a, n, col.data());
    for (int i = 0; i < n; ++i) w[size_t(i) * targets + t] = col[i];
  }

  std::vector<double> sse(targets, 0.0), sy(targets, 0.0), syy(targets, 0.0);
  for (size_t r = split; r < rows; ++r) {
    const float* xr = &x[r * cols];
    const float* yr = &y[size_t(order[r]) * targets];
    for (size_t t = 0; t < targets; ++t) {
      double p = w[size_t(cols) * targets + t];
      for (size_t c = 0; c < cols; ++c) p += w[c * targets + t] * double(xr[c]);
      const double d = double(yr[t]) - p;
      sse[t] += d * d;
      sy[t] += double(yr[t]);
      syy[t] += double(yr[t]) * double(yr[t]);
    }
  }
  // Averaged over the channels that carry something, not over all of them. A
  // mel band that never moves has no variance to explain and R^2 is undefined
  // there; scoring it zero and averaging it in silently multiplies every row of
  // the table by the same fraction. That is what made the self-test read 0.769
  // — about six of twenty-four bands sit at the noise floor for this whole
  // soundscape — and it understated every comparison in the same breath.
  const double m = double(rows - split);
  double sum = 0;
  uint32_t scored = 0;
  for (size_t t = 0; t < targets; ++t) {
    const double mean = sy[t] / m;
    const double var = syy[t] / m - mean * mean;
    if (var <= 1e-9) continue;
    sum += 1.0 - (sse[t] / m) / var;
    ++scored;
  }
  return scored ? sum / double(scored) : 0.0;
}


bool run_pcprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }
  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  Ear ear;
  if (!ear.configure(acfg, error)) {
    std::printf("  ear failed: %s\n", error.c_str());
    return false;
  }
  const int32_t aud = s.dna.module_with_role(aibaby::ModuleRole::kAuditory);
  if (aud < 0) {
    std::printf("  genome has no auditory module\n");
    return false;
  }

  const double dt = double(s.dna.header().sim.dt_ms);
  const float g = s.dna.header().curiosity.predict_gain;
  const uint32_t channels = ear.channels();
  const uint32_t samples_per_tick = uint32_t(double(acfg.sample_rate) * dt / 1000.0 + 0.5);

  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  Retina retina;
  if (!retina.configure(vcfg, error)) {
    std::printf("  retina failed: %s\n", error.c_str());
    return false;
  }

  VowelSource voice(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<float> pcm(samples_per_tick);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  const uint64_t frame_ticks = uint64_t(1000.0 / double(vcfg.frame_hz) / dt + 0.5);

  // Two conditions in a repeating cycle: the caregiver names the toy in view,
  // then the room goes quiet and the only sound is whatever the baby is doing.
  // Both halves are the creature's real acoustic life, and they are the two
  // halves the reafference question is about.
  constexpr uint64_t kCycle = 4000;   // 4 s at dt = 1 ms
  constexpr uint64_t kSpeakFor = 1200;

  // Split in half so "the model is bad" can be told apart from "the model has
  // not finished learning". Curiosity is paid for *progress*, so a model with a
  // large but shrinking error is doing its job even while it loses to a
  // baseline; a model that is flat and losing is not.
  std::vector<PcChannel> all(channels), loud(channels), quiet(channels);
  std::vector<PcChannel> early(channels), late(channels);
  std::vector<double> prev_mel(channels, 0.0);
  bool have_prev = false;
  double delivered_sum = 0, mel_sum = 0, frames = 0;

  // Held for the offline ceiling below. The features are recorded *before* the
  // frame they are asked to predict arrives, which is the whole point of the
  // exercise and the easiest thing in it to get wrong by one frame.
  const int32_t central = s.dna.module_with_role(aibaby::ModuleRole::kAssociation);
  const uint32_t central_n =
      central >= 0 ? s.brain.network().module(uint32_t(central)).count : 0;
  // B2 as well as B1, because "central is deaf" and "the creature is deaf" are
  // different diagnoses with different repairs, and only walking the pathway
  // separates them. If the auditory module cannot be read either then the tract
  // between them is innocent and the loss is at the ear.
  const uint32_t aud_n = s.brain.network().module(uint32_t(aud)).count;
  const double aud_norm =
      std::max(1e-6, 3.0 * double(s.dna.module(uint32_t(aud)).target_rate_hz));
  std::vector<float> fit_aud, pending_aud;
  const double rate_norm =
      central >= 0 ? std::max(1e-6, 3.0 * double(s.dna.module(uint32_t(central)).target_rate_hz))
                   : 1.0;
  constexpr size_t kMaxFitRows = 12000;
  std::vector<float> fit_bins, fit_cells, fit_prev, fit_target;
  // The same targets in the cepstral domain. Mel bands are heavily collinear —
  // that is what makes the ridge self-test read 0.972 instead of 1 — and the
  // DCT rotates them into something a linear fit can actually invert.
  std::vector<float> fit_mfcc, fit_prev_mfcc;
  Mfcc mfcc;
  const uint32_t n_cep = channels >= 13 ? 13 : channels;
  if (!mfcc.configure(channels, n_cep, 22.0f, error)) {
    std::printf("  mfcc failed: %s\n", error.c_str());
    return false;
  }
  std::vector<float> pending_bins, pending_cells, pending_frame;
  bool have_pending = false;
  double clip_now = 0, clip_full = 0, cells = 0;
  double shape_corr = 0, shape_frames = 0;
  double rate_sum = 0;
  uint64_t slept = 0;

  for (uint64_t t = 0; t < ticks; ++t) {
    const uint64_t phase = t % kCycle;
    const int object = int((t / kCycle) % 2);
    const bool sounding = phase < kSpeakFor;

    if (t % frame_ticks == 0) {
      scene.render(object == 1 ? SceneSource::Shape::kSquare : SceneSource::Shape::kDisc,
                   0.5f, 0.5f, 0.10f, 0.85f, 0.02f, frame.data());
      retina.present(frame.data());
      s.brain.see(retina.features().data(), retina.feature_count());
    }

    const Word& w = kWords[object];
    voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                 pcm.data(), samples_per_tick);
    ear.tick(s.brain, pcm.data(), samples_per_tick);

    // Read between hear() and step(): prediction() is written inside hear() and
    // the encoder level is the residual it has just been handed, before any
    // fade. One tick later both are stale.
    if (ear.had_frame() && !s.brain.asleep()) {
      const float* mel = ear.latest_mel().data();
      const aibaby::Scalar* pred = s.brain.critic().prediction();
      const aibaby::Scalar* got = s.brain.auditory_level();
      double sum_m = 0, sum_p = 0, sum_mp = 0, sum_mm = 0, sum_pp = 0;
      for (uint32_t c = 0; c < channels; ++c) {
        const double m = double(mel[c]);
        const double p = std::min(1.0, std::max(0.0, double(pred[c])));
        const double e = m - p;
        const double q = prev_mel[c];
        if (have_prev) {
          PcChannel* rows[3] = {&all[c], sounding ? &loud[c] : &quiet[c],
                                t * 2 < ticks ? &early[c] : &late[c]};
          for (PcChannel* row : rows) {
            row->n += 1;
            row->mel += m;
            row->mel2 += m * m;
            row->pred += p;
            row->pred2 += p * p;
            row->sq_err += e * e;
            row->sq_persist += (m - q) * (m - q);
            row->prev_dot += p * q;
            row->prev += q;
            row->prev2 += q * q;
          }
        }
        prev_mel[c] = m;
        delivered_sum += double(got[c]);
        mel_sum += m;
        cells += 1;
        if (m - double(g) * p < 0.0) clip_now += 1;
        if (m - p < 0.0) clip_full += 1;
        sum_m += m; sum_p += p;
        sum_mp += m * p; sum_mm += m * m; sum_pp += p * p;
      }
      // Does the prediction have the right *shape* across the spectrum, frame
      // by frame? R^2 above is per channel over time; this is the transpose,
      // and a model that only tracked overall loudness would score well here
      // and badly there.
      const double n = double(channels);
      const double cov = sum_mp / n - (sum_m / n) * (sum_p / n);
      const double sm = std::sqrt(std::max(0.0, sum_mm / n - (sum_m / n) * (sum_m / n)));
      const double sp = std::sqrt(std::max(0.0, sum_pp / n - (sum_p / n) * (sum_p / n)));
      if (sm > 1e-9 && sp > 1e-9) {
        shape_corr += cov / (sm * sp);
        ++shape_frames;
      }
      // One row of the offline problem: the features standing when the
      // previous frame had just been heard, paired with the frame that then
      // arrived. `pending_*` is what makes the pairing one-ahead rather than
      // simultaneous — predicting the frame you are already holding is free.
      if (have_pending && fit_target.size() / channels < kMaxFitRows) {
        fit_bins.insert(fit_bins.end(), pending_bins.begin(), pending_bins.end());
        fit_cells.insert(fit_cells.end(), pending_cells.begin(), pending_cells.end());
        fit_aud.insert(fit_aud.end(), pending_aud.begin(), pending_aud.end());
        // The frame those features were captured on, taken from pending_frame
        // and NOT from prev_mel: prev_mel is advanced in the channel loop above
        // and by here already holds the current frame. Reading it here made the
        // positive control a copy of the row it was supposed to control, and
        // the two printed the same number to three decimals for four runs
        // before the coincidence was noticed.
        fit_prev.insert(fit_prev.end(), pending_frame.begin(), pending_frame.end());
        for (uint32_t c = 0; c < channels; ++c) fit_target.push_back(mel[c]);
        const std::vector<float>& cep_prev = mfcc.transform(pending_frame.data(), channels);
        fit_prev_mfcc.insert(fit_prev_mfcc.end(), cep_prev.begin(), cep_prev.end());
        const std::vector<float>& cep_now = mfcc.transform(mel, channels);
        fit_mfcc.insert(fit_mfcc.end(), cep_now.begin(), cep_now.end());
      }
      if (central >= 0) {
        const aibaby::Network& net = s.brain.network();
        const aibaby::ModuleState& ms = net.module(uint32_t(central));
        pending_frame.assign(channels, 0.0f);
        for (uint32_t c = 0; c < channels; ++c) pending_frame[c] = mel[c];
        const aibaby::ModuleState& as = net.module(uint32_t(aud));
        pending_aud.assign(aud_n, 0.0f);
        for (uint32_t i = 0; i < aud_n; ++i) {
          pending_aud[i] = float(std::min(1.0, double(net.rate_fast(as.begin + i)) / aud_norm));
        }
        pending_cells.assign(central_n, 0.0f);
        for (uint32_t i = 0; i < central_n; ++i) {
          pending_cells[i] = float(std::min(1.0, double(net.rate_fast(ms.begin + i)) / rate_norm));
        }
        // The critic's own view of the same activity, so the two rows below
        // differ in the pooling and in nothing else.
        pending_bins.assign(aibaby::kCriticBins, 0.0f);
        for (uint32_t b = 0; b < aibaby::kCriticBins; ++b) {
          const uint32_t lo = aibaby::slice_begin(ms.count, aibaby::kCriticBins, b);
          const uint32_t hi = aibaby::slice_begin(ms.count, aibaby::kCriticBins, b + 1);
          if (hi <= lo) continue;
          double sum = 0;
          for (uint32_t i = lo; i < hi; ++i) sum += double(net.rate_fast(ms.begin + i));
          pending_bins[b] = float(std::min(1.0, (sum / double(hi - lo)) / rate_norm));
        }
        have_pending = true;
      }

      if (have_prev) ++frames;
      have_prev = true;
    }

    s.brain.step();
    if (s.brain.asleep()) ++slept;
    rate_sum += double(s.brain.network().module(uint32_t(aud)).mean_rate);
  }

  if (frames < 10) {
    std::printf("  only %.0f usable frames — nothing to say\n", frames);
    return false;
  }

  // Same rule as the offline table: dead bands are skipped, not scored zero.
  auto mean_r2 = [&](const std::vector<PcChannel>& rows, bool persistence = false) {
    double sum = 0;
    uint32_t scored = 0;
    for (const PcChannel& c : rows) {
      if (c.n < 2) continue;
      const double mean = c.mel / c.n;
      if (c.mel2 / c.n - mean * mean <= 1e-9) continue;
      sum += pc_r2(c, persistence);
      ++scored;
    }
    return scored ? sum / double(scored) : 0.0;
  };
  auto mean_prev_corr = [&](const std::vector<PcChannel>& rows) {
    double sum = 0;
    for (const PcChannel& c : rows) sum += pc_prev_corr(c);
    return sum / double(rows.size());
  };
  auto mean_sd_ratio = [&](const std::vector<PcChannel>& rows) {
    double sp = 0, sm = 0;
    for (const PcChannel& c : rows) {
      sp += pc_sd(c.pred, c.pred2, c.n);
      sm += pc_sd(c.mel, c.mel2, c.n);
    }
    return sm > 1e-9 ? sp / sm : 0.0;
  };

  std::printf("  session           %.1f s, caregiver speaks %.0f%% of it, asleep %.0f%%\n",
              double(ticks) * dt / 1000.0,
              100.0 * double(kSpeakFor) / double(kCycle),
              100.0 * double(slept) / double(ticks));
  std::printf("  predict_gain      %.2f%s\n", double(g),
              g <= 0.0f ? "   (off — this run measures the model, not the effect)" : "");
  std::printf("  frames scored     %.0f\n\n", frames);

  std::printf("  Is the prediction worth subtracting?\n");
  std::printf("    %-22s %8s %8s %8s\n", "", "all", "loud", "quiet");
  std::printf("    %-22s %8.3f %8.3f %8.3f\n", "R^2 vs channel mean",
              mean_r2(all), mean_r2(loud), mean_r2(quiet));
  std::printf("    %-22s %8.3f %8.3f %8.3f   <- the baseline that matters\n",
              "R^2 of persistence", mean_r2(all, true), mean_r2(loud, true),
              mean_r2(quiet, true));
  std::printf("    %-22s %8.3f %8.3f %8.3f\n", "corr(pred, prev frame)",
              mean_prev_corr(all), mean_prev_corr(loud), mean_prev_corr(quiet));
  std::printf("    %-22s %8.3f %8.3f %8.3f\n", "sd(pred)/sd(mel)",
              mean_sd_ratio(all), mean_sd_ratio(loud), mean_sd_ratio(quiet));
  std::printf("    %-22s %8.3f\n", "spectral shape corr",
              shape_frames > 0 ? shape_corr / shape_frames : 0.0);
  std::printf("    %-22s %8.3f -> %.3f   (persistence %.3f -> %.3f)\n",
              "R^2 first half / second", mean_r2(early), mean_r2(late),
              mean_r2(early, true), mean_r2(late, true));
  std::printf("\n    R^2 at or below zero means the forward model does no better than\n"
              "    predicting each channel's own average. Beating that is necessary and\n"
              "    nowhere near sufficient: persistence — \"the next frame looks like this\n"
              "    one\" — costs nothing and scores well on a spectrum that changes over\n"
              "    a syllable. The model has to beat *that* row to be a model, and if\n"
              "    corr(pred, prev frame) is near 1 it has simply learned to echo.\n"
              "    \"quiet\" is the reafference test: the only sound then is the\n"
              "    creature's own voice, driven by the very module the model reads.\n\n");

  std::printf("  What the subtraction costs\n");
  std::printf("    %-22s %8.4f -> %.4f  (%.0f%% kept)\n", "mean channel energy",
              mel_sum / cells, delivered_sum / cells,
              mel_sum > 1e-12 ? 100.0 * delivered_sum / mel_sum : 0.0);
  std::printf("    %-22s %7.1f%%\n", "cells rectified away", 100.0 * clip_now / cells);
  std::printf("    %-22s %7.1f%%   (what full subtraction would clip)\n",
              "...at gain 1.0", 100.0 * clip_full / cells);
  std::printf("    %-22s %8.2f Hz\n", "B2 mean rate", rate_sum / double(ticks));

  // --- The ceiling ---------------------------------------------------------
  const size_t rows = channels ? fit_target.size() / channels : 0;
  if (rows > 200 && central_n > 0) {
    std::vector<uint32_t> order(rows), shuffled(rows);
    for (size_t i = 0; i < rows; ++i) order[i] = uint32_t(i);
    shuffled = order;
    aibaby::Rng rng;
    rng.seed(0x9E3779B97F4A7C15ull);
    for (size_t i = rows; i > 1; --i) std::swap(shuffled[i - 1], shuffled[rng.next() % i]);

    // Persistence as a feature set of its own, so the "+ previous frame" rows
    // below are read as what the brain adds to it rather than as a total.
    std::vector<float> both;
    both.reserve(rows * (central_n + channels));
    for (size_t r = 0; r < rows; ++r) {
      both.insert(both.end(), fit_cells.begin() + r * central_n,
                  fit_cells.begin() + (r + 1) * central_n);
      both.insert(both.end(), fit_prev.begin() + r * channels,
                  fit_prev.begin() + (r + 1) * channels);
    }

    // Fraction of the mean diagonal, not an absolute — see ridge_holdout().
    const double lam = 1e-4;
    // Does the solver work? Fitting the target from a copy of itself is the one
    // question with a known answer, and a fit that cannot reach 1.0 there is
    // not evidence about anything else it reports. This is cheap and it stays.
    const double r_self = ridge_holdout(fit_target, rows, channels, fit_target, channels,
                                        lam, order);
    // The same three questions asked of a decorrelated target. If these come
    // out higher than their mel counterparts, the mel numbers were limited by
    // how invertible the target was and not by what the module knows.
    const double m_self = ridge_holdout(fit_mfcc, rows, n_cep, fit_mfcc, n_cep, lam, order);
    const double m_now = ridge_holdout(fit_cells, rows, central_n, fit_prev_mfcc, n_cep,
                                       lam, order);
    const double m_next = ridge_holdout(fit_cells, rows, central_n, fit_mfcc, n_cep,
                                        lam, order);
    const double m_prev = ridge_holdout(fit_prev_mfcc, rows, n_cep, fit_mfcc, n_cep,
                                        lam, order);
    const double r_bins = ridge_holdout(fit_bins, rows, aibaby::kCriticBins, fit_target,
                                        channels, lam, order);
    const double r_cells = ridge_holdout(fit_cells, rows, central_n, fit_target, channels,
                                         lam, order);
    const double r_prev = ridge_holdout(fit_prev, rows, channels, fit_target, channels,
                                        lam, order);
    const double r_both = ridge_holdout(both, rows, central_n + channels, fit_target,
                                        channels, lam, order);
    const double r_ctl = ridge_holdout(fit_cells, rows, central_n, fit_target, channels,
                                       lam, shuffled);
    // The positive control, and the row without which none of the others can be
    // believed. `fit_prev` is the frame these very features were responding to
    // when they were sampled, so this asks central what it is hearing *now*
    // rather than next. A readout that cannot answer that is not measuring a
    // module that fails to predict — it is measuring a bug in the features.
    const double r_now = ridge_holdout(fit_cells, rows, central_n, fit_prev, channels,
                                       lam, order);
    const double r_aud_now = ridge_holdout(fit_aud, rows, aud_n, fit_prev, channels,
                                           lam, order);
    const double r_aud_next = ridge_holdout(fit_aud, rows, aud_n, fit_target, channels,
                                            lam, order);

    std::printf("\n  Ceiling: the best a linear readout could do (%zu rows, held out)\n", rows);
    std::printf("    %-34s %8.3f\n", "critic's 32 bins, fitted offline", r_bins);
    std::printf("    %-34s %8.3f\n", "every central neuron", r_cells);
    std::printf("    %-34s %8.3f\n", "previous frame alone", r_prev);
    std::printf("    %-34s %8.3f\n", "central + previous frame", r_both);
    std::printf("    %-34s %8.3f   (control: must be <= 0)\n",
                "central, rows shuffled", r_ctl);
    std::printf("    %-34s %8.3f   (control: the frame it is hearing NOW)\n",
                "central -> current frame", r_now);
    std::printf("    %-34s %8.3f\n", "B2 -> current frame", r_aud_now);
    std::printf("    %-34s %8.3f\n", "B2 -> next frame", r_aud_next);
    std::printf("    %-34s %8.3f   (self-test: must be ~1)\n",
                "target from a copy of itself", r_self);
    std::printf("\n  the same, with the target decorrelated (%u MFCCs, DCT of the log-mel)\n",
                n_cep);
    std::printf("    %-34s %8.3f   (self-test: must be ~1)\n",
                "MFCC from a copy of itself", m_self);
    std::printf("    %-34s %8.3f\n", "central -> current frame", m_now);
    std::printf("    %-34s %8.3f\n", "central -> next frame", m_next);
    std::printf("    %-34s %8.3f\n", "previous frame alone", m_prev);
    std::printf("\n    Read down the first two rows for the pooling: if per-neuron beats\n"
                "    32 bins the critic is throwing the signal away before it fits, and\n"
                "    kCriticBins is the repair. Read the last two against \"previous frame\n"
                "    alone\" for the harder question — whether the association module knows\n"
                "    anything about the next sound that the last sound does not already say.\n");
  }

  if (verbose) {
    std::printf("\n  per channel: mel mean / pred mean / R^2\n");
    for (uint32_t c = 0; c < channels; ++c) {
      std::printf("    %2u  %.4f  %.4f  %+.3f\n", c, all[c].mel / all[c].n,
                  all[c].pred / all[c].n, pc_r2(all[c]));
    }
  }

  // Not a milestone: this is a measurement, and it reports rather than judges.
  // The one thing it can fail on is the creature having slept through the
  // session, which would make every number above a statement about silence.
  const bool usable = double(slept) < 0.5 * double(ticks);
  if (!usable) {
    std::printf("\n  UNUSABLE — the baby was asleep for most of it.\n");
  }
  return usable;
}

// --- projprobe: can a sparse tract carry this code at all? -----------------
//
// m3probe localised G3: with no learning and no reward, cube-vs-ball reads
// 0.850 in `central` and 0.510 in `vocal`. Widening central->vocal from 0.03 to
// 0.40 left vocal at chance, so the fault is not tract width. Two explanations
// survive that sweep, and they point at opposite halves of the machine:
//
//   A. The tract cannot carry this *kind* of code. Read across m3probe's own
//      columns: the word scores 1.000 per-neuron and 1.000 on 32 spatial bins,
//      while the object scores 0.850 per-neuron and 0.640 binned in central,
//      and 1.000 / 0.690 in vision. The word is coarse; the object is only
//      legible at per-neuron resolution wherever it exists. A sparse tract is a
//      random pooling operation, and pooling keeps coarse structure while
//      destroying fine distributed structure.
//   B. The tract carries it fine and `vocal` then loses it, because vocal fires
//      ~945 spikes a trial with nothing being said to it. An object signal
//      arriving into that would be a small perturbation on a large intrinsic
//      drive.
//
// This experiment separates them without touching the creature. It records the
// same per-neuron activity m3probe does, then applies the projection *in
// software* — a random sparse binary matrix, the linear part of what the tract
// does — and re-scores. Linear pooling with no threshold and no competing input
// is the most generous version of the tract there is:
//
//   object survives the projection  -> A is refuted; the fault is inside vocal
//   object dies in the projection   -> A is confirmed; no tract of this kind
//                                      can deliver this code, at any width
//
// The word from `auditory` runs through the identical code path as the control.
// It has to survive, because in the real creature it does: auditory->vocal is a
// direct tract at density 0.15 and vocal reads the word at 0.960. If the word
// dies here too, the projection model is wrong and neither arm means anything.
bool run_projprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  Session s;
  if (!s.init(dna_blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }
  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  Retina retina;
  Ear ear;
  if (!retina.configure(vcfg, error) || !ear.configure(acfg, error)) {
    std::printf("  transducer failed: %s\n", error.c_str());
    return false;
  }
  VowelSource voice(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  std::vector<float> pcm(16);
  const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);

  aibaby::Rng rng;
  rng.seed(s.dna.header().seed ^ 0x5C0Bu);
  const aibaby::Network& net = s.brain.network();
  const uint32_t modules = net.module_count();
  std::vector<uint32_t> width(modules);
  for (uint32_t m = 0; m < modules; ++m) width[m] = net.module(m).count;

  auto module_by_name = [&](const char* want) -> uint32_t {
    for (uint32_t m = 0; m < modules; ++m) {
      if (std::strcmp(net.module_dna(m).name, want) == 0) return m;
    }
    return modules;
  };
  const uint32_t m_central = module_by_name("central");
  const uint32_t m_auditory = module_by_name("auditory");
  const uint32_t m_vision = module_by_name("vision");
  const uint32_t m_vocal = module_by_name("vocal");
  if (m_central == modules || m_auditory == modules || m_vision == modules ||
      m_vocal == modules) {
    std::printf("  this genome does not have the four modules this probe reads\n");
    return false;
  }

  // Collect per-neuron counts for one condition, exactly as m3probe does.
  auto record = [&](bool visual, std::vector<std::vector<std::vector<double>>>& out,
                    std::vector<int>& labels) {
    const uint32_t n_trials = uint32_t(ticks / kM3ProbeTicks);
    out.assign(modules, std::vector<std::vector<double>>());
    labels.clear();
    for (uint32_t trial = 0; trial < n_trials; ++trial) {
      const int label = int(trial % 2);
      Toy toy = m3_toy(rng, label);
      if (!visual) toy.shape = SceneSource::Shape::kNone;
      std::vector<double> counts(net.total_capacity(), 0.0);
      bool slept = false;
      for (uint64_t t = 0; t < kM3ProbeTicks; ++t) {
        if (t % frame_ticks == 0) {
          scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f,
                       frame.data());
          retina.present(frame.data());
          s.brain.see(retina.features().data(), retina.feature_count());
        }
        const bool sounding = !visual && t < kM3LabelTicks;
        const Word& w = kWords[label];
        voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                     pcm.data(), samples_per_tick);
        ear.tick(s.brain, pcm.data(), samples_per_tick);
        s.brain.step();
        if (s.brain.asleep()) slept = true;
        if (t < kM3SettleTicks) continue;
        for (uint32_t k = 0; k < net.spike_count(); ++k) counts[net.spikes()[k]] += 1.0;
      }
      if (slept) continue;
      for (uint32_t m = 0; m < modules; ++m) {
        const aibaby::ModuleState& ms = net.module(m);
        out[m].emplace_back(counts.begin() + ms.begin,
                            counts.begin() + ms.begin + width[m]);
      }
      labels.push_back(label);
    }
  };

  // y = P x, with P a random binary matrix at the given density. This is the
  // linear part of a tract and nothing else: no threshold, no leak, no
  // competing input, no weight variation. Anything lost here is lost to the
  // pooling geometry alone.
  auto project = [&](const std::vector<std::vector<double>>& x, uint32_t n_dst,
                     double density, aibaby::Rng& prng) {
    const size_t n_src = x.empty() ? 0 : x[0].size();
    std::vector<std::vector<uint32_t>> pre(n_dst);
    for (uint32_t d = 0; d < n_dst; ++d) {
      for (size_t sidx = 0; sidx < n_src; ++sidx) {
        if (double(prng.next() % 1000000u) / 1000000.0 < density) {
          pre[d].push_back(uint32_t(sidx));
        }
      }
    }
    std::vector<std::vector<double>> y;
    y.reserve(x.size());
    for (const std::vector<double>& row : x) {
      std::vector<double> o(size_t(n_dst), 0.0);
      for (uint32_t d = 0; d < n_dst; ++d) {
        double sum = 0.0;
        for (uint32_t sidx : pre[d]) sum += row[sidx];
        o[d] = sum;
      }
      y.push_back(o);
    }
    return y;
  };

  std::printf("  each trial %llu ms, first %llu discarded; no learning, no reward\n",
              (unsigned long long)kM3ProbeTicks, (unsigned long long)kM3SettleTicks);
  std::printf("  a random sparse binary projection onto %u units — the linear part\n"
              "  of a tract, with no threshold and nothing else driving the target.\n",
              width[m_vocal]);
  instrument("projprobe", s.dna.header().seed ^ 0x5C0Bu, ticks / kM3ProbeTicks,
             "trials per arm");

  std::vector<std::vector<std::vector<double>>> obj, wrd;
  std::vector<int> obj_labels, wrd_labels;
  record(true, obj, obj_labels);
  record(false, wrd, wrd_labels);
  if (obj_labels.size() < 12 || wrd_labels.size() < 12) {
    std::printf("  too few trials to score\n");
    return false;
  }

  struct Arm {
    const char* name;
    uint32_t module;
    const std::vector<std::vector<std::vector<double>>>* data;
    const std::vector<int>* labels;
  };
  const Arm arms[] = {
      {"object from central", m_central, &obj, &obj_labels},
      {"object from vision", m_vision, &obj, &obj_labels},
      {"word from auditory", m_auditory, &wrd, &wrd_labels},
  };

  // Every score below is on the pair-interleaved split rather than first-half /
  // second-half, and that is a repair, not a preference.
  //
  // The naive split is only clean if the creature is stationary across the
  // session, and per-module homeostasis guarantees it is not. That bites a
  // *pooled* readout much harder than a per-neuron one, because a pooled unit
  // is dominated by the population mean and the population mean is exactly what
  // homeostasis moves — which is this experiment's whole subject. It showed up
  // as the control decaying with run length: the word at d=0.15 read 1.000 at
  // 60 trials and 0.860 at 100, while the same word at source held 1.000 at
  // both. A control that gets worse the longer you look is not measuring the
  // tract.
  auto scored = [](const std::vector<std::vector<double>>& x, const std::vector<int>& y) {
    std::vector<std::vector<double>> xi;
    std::vector<int> yi;
    size_t train = 0;
    interleave_pairs(x, y, xi, yi, train);
    return holdout_accuracy(xi, yi, train);
  };

  double word_control = 0.0;
  std::printf("\n    %-22s %-11s %-11s %-11s %-11s %s\n", "arm", "at source",
              "d=0.03", "d=0.15", "d=0.40", "shuffled");
  for (const Arm& a : arms) {
    const std::vector<std::vector<double>>& x = (*a.data)[a.module];
    const std::vector<int>& y = *a.labels;
    double scores[3];
    const double densities[3] = {0.03, 0.15, 0.40};
    std::vector<std::vector<double>> last;
    for (int i = 0; i < 3; ++i) {
      // Every arm and every density draws its projection from a stream seeded
      // the same way, so the columns differ by density and by the code being
      // pushed through, not by which random matrix happened to come up.
      aibaby::Rng prng;
      prng.seed(s.dna.header().seed ^ 0x9E37u ^ uint32_t(i * 7919));
      last = project(x, width[m_vocal], densities[i], prng);
      scores[i] = scored(last, y);
    }
    std::vector<int> shuffled = y;
    for (size_t i = shuffled.size(); i > 1; --i) {
      std::swap(shuffled[i - 1], shuffled[rng.next() % i]);
    }
    // The word at d=0.15 is the control this whole table rests on: that is a
    // tract the creature actually has, at the density it actually has, and
    // `vocal` reads the word through it at 0.960. If the word dies in the
    // model, the model is wrong about tracts and every object row is noise.
    if (std::strcmp(a.name, "word from auditory") == 0) word_control = scores[1];
    std::printf("    %-22s %-11.3f %-11.3f %-11.3f %-11.3f %.3f\n", a.name,
                scored(x, y), scores[0], scores[1], scores[2],
                scored(last, shuffled));
  }

  // Why a code survives pooling or does not, in one number, plus the arm that
  // makes the explanation falsifiable.
  //
  // Let delta_i be neuron i's mean firing difference between the two objects.
  // A pooled unit sums a random subset S, so its signal is sum_{i in S} delta_i
  // while its noise grows as sqrt(|S|). If the delta vector is same-signed the
  // signal grows as |S| and pooled SNR *improves* as sqrt(|S|); if the delta
  // vector is balanced around zero the sum is a random walk, the signal grows
  // only as sqrt(|S|), and pooled SNR does not improve at all. Coherence is
  // |mean(delta)| / mean(|delta|): 1.0 is perfectly same-signed, 0.0 balanced.
  //
  // The `flipped` arm multiplies each neuron by the sign of its own delta,
  // making the code coherent by construction, and then pools it exactly as
  // before. If cancellation is the mechanism this must rescue the object; if it
  // does not, the story is wrong. **The flips are derived from the training
  // trials only** — computing them on all trials would leak the labels and the
  // arm could not fail. `flip-shuf` derives the flips from *shuffled* training
  // labels and is the control: it must not rescue anything. This is an oracle
  // transform, not something the creature can do; it tests the explanation, not
  // a fix.
  std::printf("\n    %-22s %-11s %-11s %-11s %-11s %-11s %s\n", "arm", "coherence",
              "d=0.15", "centred", "E-I", "flipped", "flip-shuf");
  for (const Arm& a : arms) {
    // Interleaved once, up front, so the sign-flip arm derives its flips from
    // exactly the trials the classifier then trains on. Deriving them from the
    // naive first half while scoring on interleaved folds would put test trials
    // into the flips, and an arm that cannot fail is not a test.
    std::vector<std::vector<double>> x;
    std::vector<int> y;
    size_t train = 0;
    interleave_pairs((*a.data)[a.module], *a.labels, x, y, train);
    const size_t n_src = x.empty() ? 0 : x[0].size();

    auto deltas = [&](const std::vector<int>& labels) {
      std::vector<double> mu[2] = {std::vector<double>(n_src, 0.0),
                                   std::vector<double>(n_src, 0.0)};
      size_t n[2] = {0, 0};
      for (size_t t = 0; t < train; ++t) {
        const int c = labels[t];
        ++n[c];
        for (size_t i = 0; i < n_src; ++i) mu[c][i] += x[t][i];
      }
      std::vector<double> d(n_src, 0.0);
      if (n[0] == 0 || n[1] == 0) return d;
      for (size_t i = 0; i < n_src; ++i) {
        d[i] = mu[1][i] / double(n[1]) - mu[0][i] / double(n[0]);
      }
      return d;
    };

    const std::vector<double> d = deltas(y);
    double sum = 0.0, abs_sum = 0.0;
    for (double v : d) {
      sum += v;
      abs_sum += std::fabs(v);
    }
    const double coherence = abs_sum > 0.0 ? std::fabs(sum) / abs_sum : 0.0;

    std::vector<int> shuf_labels = y;
    for (size_t i = train; i > 1; --i) {
      std::swap(shuf_labels[i - 1], shuf_labels[rng.next() % i]);
    }
    const std::vector<double> d_shuf = deltas(shuf_labels);

    auto flip_and_score = [&](const std::vector<double>& signs) {
      std::vector<std::vector<double>> xf = x;
      for (std::vector<double>& row : xf) {
        for (size_t i = 0; i < n_src; ++i) {
          if (signs[i] < 0.0) row[i] = -row[i];
        }
      }
      aibaby::Rng prng;
      prng.seed(s.dna.header().seed ^ 0x9E37u ^ uint32_t(1 * 7919));
      return holdout_accuracy(project(xf, width[m_vocal], 0.15, prng), y, train);
    };

    aibaby::Rng prng;
    prng.seed(s.dna.header().seed ^ 0x9E37u ^ uint32_t(1 * 7919));
    const double plain = holdout_accuracy(project(x, width[m_vocal], 0.15, prng), y, train);

    // Subtract each trial's own population mean before pooling. If the pooled
    // sum is dominated by an object-independent common mode and its trial
    // noise, removing that mode should rescue the code. **This transform uses
    // no labels at all**, which is exactly why it is trustworthy where the
    // sign-flip arm is not: there is nothing here to leak.
    std::vector<std::vector<double>> xc = x;
    for (std::vector<double>& row : xc) {
      double m = 0.0;
      for (double v : row) m += v;
      m /= double(n_src ? n_src : 1);
      for (double& v : row) v -= m;
    }
    aibaby::Rng prngc;
    prngc.seed(s.dna.header().seed ^ 0x9E37u ^ uint32_t(1 * 7919));
    const double centred =
        holdout_accuracy(project(xc, width[m_vocal], 0.15, prngc), y, train);

    // Balanced feedforward excitation and inhibition: two independent random
    // subsets per target, one added and one subtracted. Both carry the same
    // object-independent population mean, so it cancels in the difference,
    // while the differential structure only partly cancels because the two
    // subsets are different neurons. This is what cortex actually does, it is
    // *subtractive* where `norm_gain` is divisive, and unlike `centred` it is
    // something a tract can physically be — so if it works it is buildable.
    // No labels involved.
    aibaby::Rng prnge, prngi;
    prnge.seed(s.dna.header().seed ^ 0x9E37u ^ uint32_t(1 * 7919));
    prngi.seed(s.dna.header().seed ^ 0x4B1Du);
    const std::vector<std::vector<double>> pe = project(x, width[m_vocal], 0.15, prnge);
    const std::vector<std::vector<double>> pi = project(x, width[m_vocal], 0.15, prngi);
    std::vector<std::vector<double>> ei = pe;
    for (size_t t = 0; t < ei.size(); ++t) {
      for (size_t j = 0; j < ei[t].size(); ++j) ei[t][j] -= pi[t][j];
    }
    const double ei_score = holdout_accuracy(ei, y, train);

    std::printf("    %-22s %-11.3f %-11.3f %-11.3f %-11.3f %-11.3f %.3f\n", a.name,
                coherence, plain, centred, ei_score, flip_and_score(d),
                flip_and_score(d_shuf));
  }

  std::printf(
      "\n    'at source' is the same number m3probe prints. Read each row across:\n"
      "    the drop from source to projected is what a tract of that density costs\n"
      "    this code, before the target neuron has done anything at all.\n"
      "\n    The word row is the control and has to hold up — auditory->vocal is a\n"
      "    real tract at density 0.15 and vocal reads the word at 0.960. If the word\n"
      "    falls apart here too, the projection is a bad model of a tract and no\n"
      "    other row on this table means anything.\n");
  (void)verbose;
  return positive_control("projprobe", "the word through d=0.15, the density of a real tract",
                          word_control, 0.85,
                          "The projection is not modelling a tract this creature has.");
}

// --- apicalprobe: does the second compartment run, and does it discriminate? -
//
// DNA v25 gives a neuron an apical tuft. Two questions have to be answered in
// that order, and this project has learned the hard way what happens when the
// first is skipped: DNA v11's synaptic scaling was tuned, swept and theorised
// about for a week before a direct measurement showed it never executed on the
// larynx at all. A mechanism that does not run produces exactly the same null
// as a mechanism that runs and does not help.
//
//   1. Does the compartment execute? Plateau occupancy per module. A tuft that
//      never crosses threshold and a tuft that is saturated at 100% are both
//      inert — the first delivers no gain, the second delivers a constant one,
//      which intrinsic plasticity absorbs within seconds. The mechanism only
//      exists in between.
//   2. Does the plateau carry the object? A plateau is not a spike. It appears
//      in no rate, no weight and no hash, so a tract can be pushing a perfectly
//      good code into the tuft while every existing probe reads chance. This
//      scores cube-vs-ball off the plateau pattern directly, on the same
//      held-out classifier and against the same shuffled control the rest of
//      the project uses.
//
// Question 2 is the one that decides what gets built next. Eligibility on
// central->vocal is large but object-blind, and that is why R-STDP cannot make
// the voice conditional — the third factor is a global scalar, so it can only
// scale a tract, never steer it. A plateau is a *per-neuron, input-specific*
// event. If plateaus discriminate the object, then gating plasticity on them is
// a conditional learning rule by construction, and that is the first thing all
// session that would attack the blocker rather than route around it. If they do
// not discriminate, the gate is not worth building and this probe has saved it.
bool run_apicalprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  Session s;
  if (!s.init(dna_blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }
  const aibaby::Dna& dna = s.dna;
  const aibaby::Network& net = s.brain.network();
  const uint32_t modules = net.module_count();

  // The guard that gives this probe its point. An apical tract needs *both* a
  // projection that says `apical = 1` and a target module whose
  // `apical_threshold` is above zero; a genome with one and not the other has
  // no compartment, and every number below would be a well-formed measurement
  // of nothing.
  instrument("apicalprobe", dna.header().seed ^ 0xA71Cu, ticks / kM3ProbeTicks, "trials");
  std::printf("  apical tracts in this genome:\n");
  uint32_t apical_tracts = 0;
  for (uint32_t pi = 0; pi < dna.projection_count(); ++pi) {
    const aibaby::DnaProjection& pr = dna.projection(pi);
    if (pr.apical == 0) continue;
    const bool live = dna.module(pr.dst).apical_threshold > 0.0f;
    std::printf("    %-10s -> %-10s  d=%.3f w=%.3f  threshold %.3f  %s\n",
                dna.module(pr.src).name, dna.module(pr.dst).name, pr.density,
                pr.weight, dna.module(pr.dst).apical_threshold,
                live ? "" : "<- DEAD: target has no compartment");
    if (live) ++apical_tracts;
  }
  if (apical_tracts == 0) {
    std::printf("    none — this genome is a point-neuron creature, nothing to probe\n");
    return false;
  }

  const aibaby::DnaVision& vcfg = dna.header().vision;
  const aibaby::DnaAudio& acfg = dna.header().audio;
  Retina retina;
  Ear ear;
  if (!retina.configure(vcfg, error) || !ear.configure(acfg, error)) {
    std::printf("  transducer failed: %s\n", error.c_str());
    return false;
  }
  VowelSource voice(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, dna.header().seed);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  std::vector<float> pcm(16);
  const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / dna.header().sim.dt_ms + 0.5f);

  aibaby::Rng rng;
  rng.seed(dna.header().seed ^ 0xA71Cu);

  std::vector<uint32_t> width(modules);
  for (uint32_t m = 0; m < modules; ++m) width[m] = net.module(m).count;

  // Per trial, per module: how many ticks each neuron spent in plateau, and how
  // many spikes it fired. Scored the same way, so the two columns differ by
  // what is being read and by nothing else.
  std::vector<std::vector<std::vector<double>>> plat(modules), spk(modules);
  std::vector<int> labels;
  std::vector<double> occupancy(modules, 0.0);
  uint64_t scored_ticks = 0;

  const uint32_t n_trials = uint32_t(ticks / kM3ProbeTicks);
  for (uint32_t trial = 0; trial < n_trials; ++trial) {
    const int label = int(trial % 2);
    const Toy toy = m3_toy(rng, label);
    std::vector<double> p_counts(net.total_capacity(), 0.0);
    std::vector<double> s_counts(net.total_capacity(), 0.0);
    bool slept = false;
    for (uint64_t t = 0; t < kM3ProbeTicks; ++t) {
      if (t % frame_ticks == 0) {
        scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }
      voice.render(0.0f, 0.0f, 0.0f, 0.0f, pcm.data(), samples_per_tick);
      ear.tick(s.brain, pcm.data(), samples_per_tick);
      s.brain.step();
      if (s.brain.asleep()) slept = true;
      if (t < kM3SettleTicks) continue;
      for (uint32_t k = 0; k < net.spike_count(); ++k) s_counts[net.spikes()[k]] += 1.0;
      for (uint32_t m = 0; m < modules; ++m) {
        const aibaby::ModuleState& ms = net.module(m);
        for (uint32_t k = 0; k < ms.count; ++k) {
          if (net.in_plateau(ms.begin + k)) p_counts[ms.begin + k] += 1.0;
        }
      }
      ++scored_ticks;
    }
    if (slept) continue;
    for (uint32_t m = 0; m < modules; ++m) {
      const aibaby::ModuleState& ms = net.module(m);
      plat[m].emplace_back(p_counts.begin() + ms.begin,
                           p_counts.begin() + ms.begin + width[m]);
      spk[m].emplace_back(s_counts.begin() + ms.begin,
                          s_counts.begin() + ms.begin + width[m]);
      for (uint32_t k = 0; k < width[m]; ++k) occupancy[m] += p_counts[ms.begin + k];
    }
    labels.push_back(label);
  }

  if (labels.size() < 12) {
    std::printf("  too few trials to score (%zu)\n", labels.size());
    return false;
  }

  std::printf("\n  %u trials of %llu ms, object in view, nothing said; no learning\n",
              uint32_t(labels.size()), (unsigned long long)kM3ProbeTicks);
  std::printf("\n    %-10s %-10s %-11s %-11s %-11s\n", "module", "plateau%",
              "obj|plateau", "obj|spikes", "shuffled");

  bool any_live_compartment = false;
  for (uint32_t m = 0; m < modules; ++m) {
    if (dna.module(m).apical_threshold <= 0.0f) continue;
    const double denom =
        double(scored_ticks ? scored_ticks : 1) * double(width[m] ? width[m] : 1);
    const double occ = 100.0 * occupancy[m] / denom;

    std::vector<std::vector<double>> xp, xs;
    std::vector<int> yp, ys;
    size_t train_p = 0, train_s = 0;
    interleave_pairs(plat[m], labels, xp, yp, train_p);
    interleave_pairs(spk[m], labels, xs, ys, train_s);
    const double a_plat = holdout_accuracy(xp, yp, train_p);
    const double a_spk = holdout_accuracy(xs, ys, train_s);

    std::vector<int> shuffled = yp;
    for (size_t i = shuffled.size(); i > 1; --i) {
      std::swap(shuffled[i - 1], shuffled[rng.next() % i]);
    }
    const double a_shuf = holdout_accuracy(xp, shuffled, train_p);

    // A tuft pinned at either rail carries nothing regardless of what the
    // classifier says, so the flag is on the occupancy and not on the score.
    const char* tag = "";
    if (occ < 0.5) tag = "  <- never fires";
    else if (occ > 95.0) tag = "  <- saturated";
    std::printf("    %-10s %-10.1f %-11.3f %-11.3f %-11.3f%s\n", dna.module(m).name,
                occ, a_plat, a_spk, a_shuf, tag);
    if (occ >= 0.5 && occ <= 95.0) any_live_compartment = true;
  }

  if (!any_live_compartment) {
    std::printf("\n  Every compartment is pinned at a rail. The mechanism is present\n"
                "  and not running; no score above is a measurement of it.\n");
    return false;
  }
  std::printf("\n  A compartment is live. `obj|plateau` above `shuffled` means the\n"
              "  tuft discriminates, and a plateau-gated learning rule would be\n"
              "  conditional by construction.\n");

  // DNA v29's did-it-run guard, and it is a different number from the
  // occupancy above. Occupancy is over all ticks and all neurons; this is over
  // the moments a neuron *fired* on a gated module, which is the only moment
  // the gate is ever consulted. A busy neuron and a quiet one contribute
  // equally to occupancy and very unequally to this, so a compartment can look
  // healthy at 30% and gate almost nothing.
  if (s.brain.network().plateau_gated()) {
    const double rate = s.brain.network().plateau_pass_rate();
    std::printf("\n  plateau gate      %.1f%% of %llu potentiation events passed%s\n",
                100.0 * rate,
                (unsigned long long)s.brain.network().plateau_gate_events(),
                rate < 0.005  ? "   <- SHUT: learning is off, not conditional"
                : rate > 0.995 ? "   <- WIDE OPEN: gating nothing"
                               : "");
    if (rate < 0.005 || rate > 0.995) {
      std::printf("  The gate is at a rail. It is not making the rule conditional,\n"
                  "  it is scaling it — which is the thing the rule could already do.\n");
      return false;
    }
  }
  (void)verbose;
  return true;
}

// --- oscprobe: did the rhythm entrain, and did it produce a phase code? -----
//
// DNA v26's whole argument is one claim: on a shared rising ramp, a neuron with
// more synaptic drive crosses threshold *earlier in the cycle* than one with
// less, so a rate difference becomes a timing difference and STDP — a timing
// rule — can finally see it. `eligprobe` says the trace on central->vocal stays
// object-blind at every amplitude the creature tolerates. That null has two
// readings and they call for opposite next moves:
//
//   A. The rhythm entrained, a phase code appeared, and STDP still could not
//      use it. The idea is wrong for this creature.
//   B. The rhythm never entrained at all, so no phase code existed to be used,
//      and the null is about amplitude rather than about the idea.
//
// Two numbers separate them, per module:
//
//   vector strength  — |mean(e^{i phi})| over spikes, 0 for spikes scattered
//                      uniformly across the cycle and 1 for a module firing at
//                      one phase. This is entrainment, and B predicts ~0.
//   rate/phase corr  — across neurons, the correlation between a neuron's
//                      firing rate and its mean firing phase. **This is the
//                      claim itself**, not a proxy for it. Phase-of-firing
//                      coding means busier neurons fire systematically earlier,
//                      so the claim predicts a large negative correlation, and
//                      entrainment without it is a module firing in lockstep —
//                      which carries no information at all.
//
// A module can entrain hard and still have no phase code. That is the case the
// second column exists to catch, and it is the one that would explain a null
// without exonerating the mechanism.
bool run_oscprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  Session s;
  if (!s.init(dna_blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }
  const aibaby::Dna& dna = s.dna;
  const aibaby::Network& net = s.brain.network();
  const uint32_t modules = net.module_count();

  instrument("oscprobe", dna.header().seed ^ 0x05C1u, ticks / kM3ProbeTicks, "trials");
  std::printf("  oscillating modules:\n");
  uint32_t live = 0;
  for (uint32_t m = 0; m < modules; ++m) {
    const aibaby::DnaModule& dm = dna.module(m);
    if (dm.theta_amp == 0.0f && dm.gamma_amp == 0.0f) continue;
    std::printf("    %-10s theta %.1f Hz x %.3f   gamma %.1f Hz x %.3f   coupling %.2f\n",
                dm.name, dm.theta_hz, dm.theta_amp, dm.gamma_hz, dm.gamma_amp,
                dm.gamma_theta_coupling);
    ++live;
  }
  if (live == 0) {
    std::printf("    none — this genome has no rhythm, nothing to probe\n");
    return false;
  }

  const aibaby::DnaVision& vcfg = dna.header().vision;
  const aibaby::DnaAudio& acfg = dna.header().audio;
  Retina retina;
  Ear ear;
  if (!retina.configure(vcfg, error) || !ear.configure(acfg, error)) {
    std::printf("  transducer failed: %s\n", error.c_str());
    return false;
  }
  VowelSource voice(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, dna.header().seed);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  std::vector<float> pcm(16);
  const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / dna.header().sim.dt_ms + 0.5f);

  aibaby::Rng rng;
  rng.seed(dna.header().seed ^ 0x05C1u);
  const Toy toy = m3_toy(rng, 0);

  const size_t cap = net.total_capacity();
  std::vector<double> sx(cap, 0.0), sy(cap, 0.0), n(cap, 0.0);
  // Which module each neuron belongs to. The kernel keeps this privately and a
  // probe has no business widening that interface for a diagnostic, so it is
  // rebuilt here from the module ranges the kernel already publishes.
  std::vector<uint32_t> mod_of(cap, modules);
  for (uint32_t m = 0; m < modules; ++m) {
    const aibaby::ModuleState& ms = net.module(m);
    for (uint32_t k = 0; k < ms.count; ++k) mod_of[ms.begin + k] = m;
  }

  const uint64_t settle = kM3SettleTicks;
  for (uint64_t t = 0; t < ticks; ++t) {
    if (t % frame_ticks == 0) {
      scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
      retina.present(frame.data());
      s.brain.see(retina.features().data(), retina.feature_count());
    }
    voice.render(0.0f, 0.0f, 0.0f, 0.0f, pcm.data(), samples_per_tick);
    ear.tick(s.brain, pcm.data(), samples_per_tick);
    s.brain.step();
    if (t < settle || s.brain.asleep()) continue;
    for (uint32_t k = 0; k < net.spike_count(); ++k) {
      const uint32_t i = net.spikes()[k];
      if (mod_of[i] >= modules) continue;
      const double phi = 6.283185307179586 * double(net.gamma_phase(mod_of[i]));
      sx[i] += std::cos(phi);
      sy[i] += std::sin(phi);
      n[i] += 1.0;
    }
  }

  std::printf("\n    %-10s %-10s %-16s %-10s\n", "module", "vec str", "rate/phase corr",
              "spiking");
  for (uint32_t m = 0; m < modules; ++m) {
    const aibaby::DnaModule& dm = dna.module(m);
    if (dm.theta_amp == 0.0f && dm.gamma_amp == 0.0f) continue;
    const aibaby::ModuleState& ms = net.module(m);

    // Vector strength pooled over the module's spikes, and per-neuron rate and
    // mean phase for the correlation. Neurons with too few spikes are dropped
    // from the correlation: a mean phase from three spikes is noise, and with a
    // thousand such neurons the correlation is decided entirely by them.
    double px = 0.0, py = 0.0, total = 0.0;
    std::vector<double> rates, phases;
    for (uint32_t k = 0; k < ms.count; ++k) {
      const uint32_t i = ms.begin + k;
      px += sx[i];
      py += sy[i];
      total += n[i];
      if (n[i] < 20.0) continue;
      rates.push_back(n[i]);
      phases.push_back(std::atan2(sy[i], sx[i]));
    }
    const double vs = total > 0.0 ? std::sqrt(px * px + py * py) / total : 0.0;

    // Phase is circular and a Pearson correlation is not. If the population
    // straddles the +/-pi branch cut, neurons a hair apart in phase land a full
    // turn apart in the arithmetic and the correlation is decided by where the
    // cut happens to fall. Rotating every neuron so the population's own mean
    // phase sits at zero puts the cut on the far side of a unimodal
    // distribution, which is what this one is: vector strength rises to 0.69,
    // so the module fires in a single clump rather than in two.
    const double centre = std::atan2(py, px);
    for (size_t i = 0; i < phases.size(); ++i) {
      double d = phases[i] - centre;
      while (d > 3.141592653589793) d -= 6.283185307179586;
      while (d < -3.141592653589793) d += 6.283185307179586;
      phases[i] = d;
    }

    double corr = 0.0;
    if (rates.size() >= 8) {
      double mr = 0.0, mp = 0.0;
      for (size_t i = 0; i < rates.size(); ++i) { mr += rates[i]; mp += phases[i]; }
      mr /= double(rates.size());
      mp /= double(rates.size());
      double num = 0.0, dr = 0.0, dp = 0.0;
      for (size_t i = 0; i < rates.size(); ++i) {
        const double a = rates[i] - mr, b = phases[i] - mp;
        num += a * b; dr += a * a; dp += b * b;
      }
      corr = (dr > 0.0 && dp > 0.0) ? num / std::sqrt(dr * dp) : 0.0;
    }

    const char* tag = "";
    if (vs < 0.05) tag = "  <- not entrained";
    else if (std::fabs(corr) < 0.15) tag = "  <- entrained, but no phase code";
    std::printf("    %-10s %-10.3f %-16.3f %-10zu%s\n", dm.name, vs, corr, rates.size(),
                tag);
  }
  std::printf("\n  vector strength is entrainment; rate/phase correlation is the\n"
              "  claim. A large NEGATIVE correlation means busier neurons fire\n"
              "  earlier, which is the phase code DNA v26 exists to create.\n");
  (void)verbose;
  return true;
}

// --- gazeprobe: what does the creature owe to being handed a centred object? -
//
// DNA v27 was built on a premise that turned out to be false, and this probe
// exists because the false premise pointed at a real question.
//
// The premise: this retina is foveated with the fovea nailed to the middle of
// the frame, the toy appears wherever it likes, and so a large part of the
// trial-to-trial variance in the visual code is positional rather than about
// the object — which would feed the common-mode noise `projprobe` measured as
// what buries the object in a sparse tract. The fix would be to point the
// fovea.
//
// The fact: `m3_toy` places every toy at 0.5 +/- 0.02 of the frame. On 64
// pixels that is +/-1.3 px — dead centre. What varies trial to trial is the
// *radius*, not the position. There is no positional variance to remove, which
// is why a correctly built saccade mechanism fires zero times: the error is
// smaller than one pixel of eye movement.
//
// The question that survives is better than the one it replaces. **Every visual
// number this project has ever reported was measured with the object already
// centred on the fovea.** M2's 0.98, m3probe's vision 1.000, the whole
// attenuation cascade. Nobody has asked what any of it is worth when the
// creature has to find the object first. So:
//
//   scatter    how far off centre the toy may land, as a fraction of the frame
//   saccades   off: the pre-v27 creature, staring straight ahead
//              on:  the fovea orients to the salience peak
//
// Read the first row against the last. If performance is flat in scatter, the
// foveation in this retina is decorative and active vision has nothing to buy.
// If it falls and saccades restore it, then the existing numbers are resting on
// a courtesy of the protocol and this mechanism is what pays for it honestly.
bool run_gazeprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  Session probe;
  if (!probe.init(dna_blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }
  const aibaby::DnaVision& vcfg = probe.dna.header().vision;
  const aibaby::DnaAudio& acfg = probe.dna.header().audio;

  std::printf("  toy scatter is added to m3_toy's own +/-0.02, so the 0.00 row is\n"
              "  the protocol every other experiment in this project uses.\n");
  // Three fovea modes per scatter, and the point of the middle one is that it
  // is bracketed. `fixed` is what the creature does with no controller at all
  // and `oracle` is the ceiling on what any controller could achieve, so a
  // reflex that lands between them can be scored as a *fraction of what was
  // available* rather than against an absolute anyone has to argue about.
  enum Fovea { kFixed = 0, kReflex = 1, kOracle = 2, kServo = 3 };
  const char* fovea_name[4] = {"fixed", "reflex", "oracle", "servo"};
  // The imperfect eye the reflex would have to drive on real hardware. Not a
  // guess at any particular motor: the point is to find out whether the
  // controller degrades gracefully or falls over, before anyone buys one.
  // One frame is 100 ms at this frame rate, so a single dead frame is already
  // a pessimistic servo.
  Retina::Servo servo_model;
  servo_model.dead_frames = 1;
  servo_model.slew = 0.5f;
  servo_model.noise_px = 0.5f;

  std::printf("  each scatter is run three ways: fovea fixed at the frame centre,\n"
              "  the DNA v31 reflex controller, and an ORACLE fovea placed exactly\n"
              "  on the toy. The reflex re-aims every frame at gain %.2f.\n",
              double(vcfg.gaze_gain));

  // Swept by --verbose, because the threshold trades locality against centring
  // and both ends are a previously refuted design.
  float peak_frac_override = 0.0f;
  auto arm = [&](double scatter, int fovea, double* vis_out, double* cen_out,
                 double* shuf_out, double* err_out, double* base_out,
                 uint64_t* moves_out) -> bool {
    // The reflex arm is the same creature with the controller switched on, so
    // the rate is patched into a copy of the blob rather than asking the user
    // for two genomes. Re-aiming every frame is the most iterations this trial
    // structure allows, which is the right setting for asking whether the
    // design can converge at all.
    std::vector<uint8_t> variant = dna_blob;
    {
      // Every arm sets the rate EXPLICITLY, including the ones that want it
      // off. Reading it from the genome was a latent bug and v1.0.1 sprang it:
      // switching the fovea on shipped a default of 10 Hz, so `fixed` stopped
      // being fixed and the oracle was dragged around by the very controller
      // it exists to bracket. All three rows printed the same numbers, which
      // is the only reason it was noticed.
      const bool driven = fovea == kReflex || fovea == kServo;
      const float rate = driven ? vcfg.frame_hz : 0.0f;
      std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, vision) +
                      offsetof(aibaby::DnaVision, gaze_rate_hz),
                  &rate, sizeof(rate));
      if (driven && peak_frac_override > 0.0f) {
        std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, vision) +
                        offsetof(aibaby::DnaVision, gaze_peak_frac),
                    &peak_frac_override, sizeof(peak_frac_override));
      }
    }
    Session s;
    if (!s.init(variant, error)) return false;
    Retina retina;
    Ear ear;
    if (!retina.configure(s.dna.header().vision, error) || !ear.configure(acfg, error)) {
      return false;
    }
    if (fovea == kServo) retina.set_servo(servo_model, s.dna.header().seed ^ 0x5E70u);
    VowelSource voice(acfg.sample_rate);
    SceneSource scene(vcfg.frame_size, s.dna.header().seed);
    std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
    std::vector<float> pcm(16);
    const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);
    const uint64_t frame_ticks =
        uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);

    // Every arm draws its toys from a stream seeded the same way, so the arms
    // differ by scatter and by nothing else — the same toy sequence, displaced.
    aibaby::Rng rng;
    rng.seed(s.dna.header().seed ^ 0x6A2Eu);

    const aibaby::Network& net = s.brain.network();
    const uint32_t modules = net.module_count();
    uint32_t m_vis = modules, m_cen = modules;
    for (uint32_t m = 0; m < modules; ++m) {
      if (std::strcmp(net.module_dna(m).name, "vision") == 0) m_vis = m;
      if (std::strcmp(net.module_dna(m).name, "central") == 0) m_cen = m;
    }
    if (m_vis == modules || m_cen == modules) return false;

    std::vector<std::vector<double>> xv, xc;
    std::vector<int> labels;
    double gaze_err = 0.0, want_err = 0.0;
    uint32_t gaze_n = 0;
    const uint32_t n_trials = uint32_t(ticks / kM3ProbeTicks);
    for (uint32_t trial = 0; trial < n_trials; ++trial) {
      const int label = int(trial % 2);
      Toy toy = m3_toy(rng, label);
      // Displace it, and keep it wholly inside the frame: a toy clipped by the
      // edge is a different *shape*, which would hand the classifier a cue that
      // has nothing to do with where the creature is looking.
      const double lo = double(toy.radius), hi = 1.0 - double(toy.radius);
      toy.cx = float(std::min(hi, std::max(lo, double(toy.cx) +
                                                   scatter * rng.signed_uniform())));
      toy.cy = float(std::min(hi, std::max(lo, double(toy.cy) +
                                                   scatter * rng.signed_uniform())));

      // The oracle arm: the eye is put exactly on the toy, which is the upper
      // bound on anything a saccade controller could ever achieve. DNA v27's
      // reflexive controller landed at ~4.4 px and was deleted in v30; the
      // question it could never answer, because a bad controller and a useless
      // fovea produce the same null, is whether *perfect* fixation buys
      // anything. This answers that and costs one extra arm.
      if (fovea == kOracle) {
        retina.look_at((float(toy.cx) - 0.5f) * float(vcfg.frame_size),
                       (float(toy.cy) - 0.5f) * float(vcfg.frame_size));
      }
      std::vector<double> cv(net.total_capacity(), 0.0);
      bool slept = false;
      for (uint64_t t = 0; t < kM3ProbeTicks; ++t) {
        if (t % frame_ticks == 0) {
          scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
          retina.present(frame.data());
          s.brain.see(retina.features().data(), retina.feature_count());
        }
        voice.render(0.0f, 0.0f, 0.0f, 0.0f, pcm.data(), samples_per_tick);
        ear.tick(s.brain, pcm.data(), samples_per_tick);
        s.brain.step();
        if (s.brain.asleep()) slept = true;
        if (t < kM3SettleTicks) continue;
        for (uint32_t k = 0; k < net.spike_count(); ++k) cv[net.spikes()[k]] += 1.0;
      }
      // Where the eye ended up against where the toy was. In the oracle arm
      // this is 0 by construction and is printed as the proof of that.
      {
        const double mid = 0.5 * double(vcfg.frame_size);
        const double want_x = (double(toy.cx) - 0.5) * double(vcfg.frame_size);
        const double want_y = (double(toy.cy) - 0.5) * double(vcfg.frame_size);
        gaze_err += std::sqrt((double(retina.gaze_x()) - want_x) *
                                  (double(retina.gaze_x()) - want_x) +
                              (double(retina.gaze_y()) - want_y) *
                                  (double(retina.gaze_y()) - want_y));
        want_err += std::sqrt(want_x * want_x + want_y * want_y);
        ++gaze_n;
      }
      if (slept) continue;
      const aibaby::ModuleState& vs = net.module(m_vis);
      const aibaby::ModuleState& cs = net.module(m_cen);
      xv.emplace_back(cv.begin() + vs.begin, cv.begin() + vs.begin + vs.count);
      xc.emplace_back(cv.begin() + cs.begin, cv.begin() + cs.begin + cs.count);
      labels.push_back(label);
    }
    if (labels.size() < 12) return false;

    std::vector<std::vector<double>> iv, ic;
    std::vector<int> yv, yc;
    size_t tv = 0, tc = 0;
    interleave_pairs(xv, labels, iv, yv, tv);
    interleave_pairs(xc, labels, ic, yc, tc);
    *vis_out = holdout_accuracy(iv, yv, tv);
    *cen_out = holdout_accuracy(ic, yc, tc);

    std::vector<int> sh = yv;
    for (size_t i = sh.size(); i > 1; --i) std::swap(sh[i - 1], sh[rng.next() % i]);
    *shuf_out = holdout_accuracy(iv, sh, tv);
    *err_out = gaze_n ? gaze_err / double(gaze_n) : 0.0;
    *base_out = gaze_n ? want_err / double(gaze_n) : 0.0;
    *moves_out = retina.gaze_moves();
    return true;
  };

  instrument("gazeprobe", probe.dna.header().seed ^ 0x6A2Eu, ticks / kM3ProbeTicks,
             "trials per scatter arm");
  std::printf("\n    %-9s %-8s %-10s %-10s %-10s %-16s %s\n", "scatter", "fovea",
              "vision", "central", "shuffled", "gaze err (px)", "re-aims");
  // Fine near zero on purpose. The fovea covers the central 16 px of a 64 px
  // frame, so scatter 0.10 displaces the toy by about 5 px and never takes it
  // out of the fovea at all — and vision collapses anyway. Whatever the
  // tolerance is, it is far tighter than the foveal field, so the interesting
  // part of this curve is between 0 and 0.10 rather than beyond it.
  const double scatters[] = {0.00, 0.02, 0.05, 0.10, 0.25};
  bool ok = false;
  // The scatter-0.00 arm is this probe's positive control, and it is a strong
  // one: it is *the protocol every other experiment in this project uses*, so
  // the visual code is known to be legible there — m3probe reads the same
  // module on the same trials at 0.967. A falling curve to its right is only a
  // finding about displacement if this row is up.
  double centred_vision = 0.0;
  double sum[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
  double sum_err[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
  uint64_t total_moves = 0;
  uint32_t hard_n[2] = {0, 0};
  for (double sc : scatters) {
    for (int f = 0; f < 4; ++f) {
      double v = 0, c = 0, sh = 0, err = 0, base = 0;
      uint64_t moves = 0;
      if (!arm(sc, f, &v, &c, &sh, &err, &base, &moves)) {
        std::printf("    %-9.2f  arm failed\n", sc);
        continue;
      }
      char errbuf[48];
      std::snprintf(errbuf, sizeof(errbuf), "%.1f (toy at %.1f)", err, base);
      std::printf("    %-9.2f %-8s %-10.3f %-10.3f %-10.3f %-16s %llu\n", sc,
                  fovea_name[f], v, c, sh, errbuf, (unsigned long long)moves);
      if (sc == 0.00 && f == kFixed) centred_vision = v;
      if (f == kReflex) total_moves += moves;
      // Averaged over the displacements that actually cost the fixed eye
      // something. Including scatter 0.00, where there is nothing to recover,
      // would dilute the one comparison this table exists to make.
      // Split by whether the toy starts inside the fovea, because averaging
      // across that boundary describes neither regime. Inside it the
      // controller has fine cells to home in with; outside it has only coarse
      // rings, and acquiring a target the fovea cannot yet see is a different
      // problem from centring one it can.
      if (sc >= 0.05) {
        const bool reachable = base <= 0.5 * double(vcfg.fovea_size);
        const int bucket = reachable ? 0 : 1;
        sum[bucket][f] += v;
        sum_err[bucket][f] += err;
        if (f == kFixed) ++hard_n[bucket];
      }
      ok = true;
    }
  }
  const char* bucket_name[2] = {"toy starts INSIDE the fovea",
                                "toy starts OUTSIDE the fovea"};
  for (int b = 0; b < 2; ++b) {
    if (!hard_n[b]) continue;
    const double n = double(hard_n[b]);
    const double f = sum[b][0] / n, r = sum[b][1] / n, o = sum[b][2] / n;
    std::printf("\n  %s (scatter >= 0.05, %u rows)\n", bucket_name[b], hard_n[b]);
    const double sv = sum[b][3] / n;
    std::printf("    fixed   vision %.3f, gaze %.1f px\n"
                "    reflex  vision %.3f, gaze %.1f px\n"
                "    servo   vision %.3f, gaze %.1f px   <- the same controller on a motor\n"
                "    oracle  vision %.3f, gaze %.1f px\n",
                f, sum_err[b][0] / n, r, sum_err[b][1] / n, sv, sum_err[b][3] / n,
                o, sum_err[b][2] / n);
    const double avail = o - f;
    if (avail > 0.05) {
      std::printf("    the reflex recovers %+.3f of the %+.3f available — %.0f%%\n"
                  "    on a motor it recovers %+.3f — %.0f%%\n",
                  r - f, avail, 100.0 * (r - f) / avail, sv - f,
                  100.0 * (sv - f) / avail);
    }
  }
  // How bad does the eye have to be before the controller stops working? The
  // loop has no model of its own motion — it re-aims every frame at gain 0.70
  // and corrects from where it *believes* it is — so stale readback is the
  // parameter to fear: an eye that cannot yet see its own movement asks for
  // more of it, which is how a proportional controller overshoots.
  if (verbose) {
    std::printf("\n  servo tolerance, at scatter 0.10 (toy 4.9 px out)\n");
    std::printf("    %-6s %-6s %-8s %-10s %s\n", "dead", "slew", "noise", "vision",
                "gaze err");
    const Retina::Servo probes[] = {
        {0, 1.0f, 0.0f}, {0, 0.5f, 0.0f}, {1, 1.0f, 0.0f}, {1, 0.5f, 0.0f},
        {2, 0.5f, 0.0f}, {1, 0.5f, 0.5f}, {1, 0.5f, 2.0f},
    };
    for (const Retina::Servo& sv : probes) {
      servo_model = sv;
      double v = 0, c = 0, sh = 0, err = 0, base = 0;
      uint64_t moves = 0;
      if (!arm(0.10, kServo, &v, &c, &sh, &err, &base, &moves)) continue;
      std::printf("    %-6u %-6.2f %-8.1f %-10.3f %.1f px\n", sv.dead_frames,
                  double(sv.slew), double(sv.noise_px), v, err);
    }
    servo_model.dead_frames = 1;
    servo_model.slew = 0.5f;
    servo_model.noise_px = 0.5f;
  }

  // The threshold sweep. One number spans both previously refuted designs —
  // 1.0 is the pure peak cell that parks on an object edge, and a low value is
  // v27's whole-field centroid that the periphery dilutes — so the useful
  // setting, if there is one, is somewhere in between and this is where it
  // shows up.
  if (verbose) {
    std::printf("\n  gaze_peak_frac sweep, at scatter 0.10 (toy 4.9 px out)\n");
    std::printf("    %-8s %-10s %-10s %s\n", "frac", "vision", "central", "gaze err");
    for (float frac : {0.20f, 0.35f, 0.50f, 0.70f, 0.90f, 1.00f}) {
      peak_frac_override = frac;
      double v = 0, c = 0, sh = 0, err = 0, base = 0;
      uint64_t moves = 0;
      if (!arm(0.10, kReflex, &v, &c, &sh, &err, &base, &moves)) continue;
      std::printf("    %-8.2f %-10.3f %-10.3f %.1f px  (%llu re-aims)\n", frac, v, c,
                  err, (unsigned long long)moves);
    }
    peak_frac_override = 0.0f;
  }

  // The did-it-run guard. A controller that never fires and a controller that
  // fires and lands nowhere read identically in every column above.
  if (total_moves == 0) {
    std::printf("\n  THE EYE NEVER MOVED — the reflex rows are the fixed rows under\n"
                "  another name. Nothing above is a measurement of a controller.\n");
    ok = false;
  }
  (void)verbose;
  if (!ok) return false;
  return positive_control("gazeprobe", "scatter 0.00 — the standard protocol — in `vision`",
                          centred_vision, 0.85,
                          "The curve below it is not measuring displacement.");
}


// --- invprobe: is translation invariance learnable in this creature? --------
//
// `gazeprobe` established the debt: the visual code dies at 2.6 px of
// displacement, which on a 64 px frame with 2x2 px foveal cells is one cell.
// Every visual number this project has reported — M2's 0.98, m3probe's vision
// 1.000, the whole attenuation cascade — was measured inside a 1.4 px window.
//
// The tempting fix is a complex-cell layer that pools over position. Three
// hand-designed visual hierarchies have already failed here, and every one of
// them was tuned for shape selectivity and never scored on a displacement
// curve, so the fourth deserves a measurement before it gets written. This is
// that measurement, and it is deliberately cheap relative to the thing it
// decides.
//
// **The question is not whether the retina is invariant.** It is not, it cannot
// be trained, and that is settled: it is a fixed transducer with a fovea nailed
// to the frame. The question is whether a *downstream* module can learn to be.
// `central` reads `vision` through a plastic tract, and a held-out accuracy of
// 0.500 on `vision` at scatter 0.05 does not mean the spikes carry nothing — it
// means a nearest-centroid rule on spike counts cannot read them. A code that a
// linear readout cannot use and a learned one can is exactly what a pooling
// layer would be for.
//
// Three arms, one shared test:
//
//   naive      no learning on vision->central at all: the reference curve
//   centred    exposed to the usual pre-centred toys, then tested displaced
//   jittered   exposed to displaced toys, then tested on the same curve
//
// Read `jittered` against `centred`. Flatter means displacement tolerance is
// learnable in this architecture and a pooling layer is worth building. The
// same cliff in both means it is not, and no hierarchy will help — which is
// worth knowing for the price of one experiment rather than one architecture.
//
// Two controls, because a null here has three boring explanations before it has
// an interesting one:
//
//   - **Did training change anything?** Mean |dw| over vision->central across
//     the exposure. If the tract did not move, the three arms are the same
//     creature and the flat result is about the genome's `hebb` being zero.
//   - **Is the object there to begin with?** `vision` at scatter 0.00. Below
//     that, nothing downstream can be about displacement.
//
// The last table is the sharpest version of the question and costs nothing
// extra: train and test one classifier across *all* scatter levels pooled. A
// per-level curve asks "can a readout tuned to this position read this
// position"; the pooled row asks "can one readout read every position", which
// is what invariance actually means.
bool run_invprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  Session probe;
  if (!probe.init(dna_blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }
  const aibaby::DnaVision& vcfg = probe.dna.header().vision;
  const aibaby::DnaAudio& acfg = probe.dna.header().audio;

  const double test_scatter[] = {0.00, 0.02, 0.05, 0.10};
  constexpr uint32_t kScatters = 4;
  constexpr double kTrainScatter = 0.10;

  instrument("invprobe", probe.dna.header().seed ^ 0x1E7Bu, ticks / kM3ProbeTicks,
             "trials per scatter arm");
  std::printf("  exposure          %llu ticks per arm, then the same displacement\n"
              "                    curve for all three\n",
              (unsigned long long)ticks);

  // One arm: expose, then test at every scatter with the identical toy stream.
  struct ArmResult {
    double vis[kScatters] = {};
    double cen[kScatters] = {};
    double shuf[kScatters] = {};
    double pooled_vis = 0, pooled_cen = 0, pooled_shuf = 0;
    double dw = 0;
    bool ok = false;
  };

  auto run_arm = [&](const std::vector<uint8_t>& variant, bool train,
                     double train_scatter) -> ArmResult {
    ArmResult out;
    Session s;
    if (!s.init(variant, error)) return out;
    Retina retina;
    Ear ear;
    if (!retina.configure(vcfg, error) || !ear.configure(acfg, error)) return out;
    VowelSource voice(acfg.sample_rate);
    SceneSource scene(vcfg.frame_size, s.dna.header().seed);
    std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
    std::vector<float> pcm(16);
    const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);
    const uint64_t frame_ticks =
        uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);

    const aibaby::Network& net = s.brain.network();
    const uint32_t modules = net.module_count();
    uint32_t m_vis = modules, m_cen = modules;
    for (uint32_t m = 0; m < modules; ++m) {
      if (std::strcmp(net.module_dna(m).name, "vision") == 0) m_vis = m;
      if (std::strcmp(net.module_dna(m).name, "central") == 0) m_cen = m;
    }
    if (m_vis == modules || m_cen == modules) return out;

    std::vector<uint32_t> tract(net.tract_synapses(m_vis, m_cen, nullptr, 0));
    if (!tract.empty()) {
      net.tract_synapses(m_vis, m_cen, tract.data(), uint32_t(tract.size()));
    }
    std::vector<double> w0(tract.size(), 0.0);
    for (size_t i = 0; i < tract.size(); ++i) w0[i] = double(net.synapse_weight(tract[i]));

    // Toys come off a stream seeded identically in every arm, so two arms see
    // the same shapes at the same sizes in the same order and differ only in
    // where they were put. Exposure and test draw from separate streams for the
    // same reason: an arm that trains longer must not thereby get a different
    // test set.
    aibaby::Rng train_rng, test_rng;
    train_rng.seed(s.dna.header().seed ^ 0x1E45u);

    auto place = [&](Toy& toy, double scatter, aibaby::Rng& rng) {
      const double lo = double(toy.radius), hi = 1.0 - double(toy.radius);
      toy.cx = float(std::min(hi, std::max(lo, double(toy.cx) +
                                                   scatter * rng.signed_uniform())));
      toy.cy = float(std::min(hi, std::max(lo, double(toy.cy) +
                                                   scatter * rng.signed_uniform())));
    };

    // --- exposure -----------------------------------------------------------
    //
    // Named, because naming is what makes the exposure a teaching signal rather
    // than a slideshow: the only learning write in this kernel is reward-gated
    // (`w += eta * credit * r`) unless a projection carries its own `hebb`
    // rate, so an unrewarded exposure would leave the weights where it found
    // them and all three arms would be the same creature. Praise is identical
    // for both objects on purpose — it opens the gate and says nothing about
    // which toy this is, so nothing here can teach the distinction by fiat.
    if (train) {
      const uint32_t n_train = uint32_t(ticks / kM3TrialTicks);
      for (uint32_t trial = 0; trial < n_train; ++trial) {
        const int label = int(trial % 2);
        Toy toy = m3_toy(train_rng, label);
        place(toy, train_scatter, train_rng);
        const Word& w = kWords[label];
        for (uint64_t t = 0; t < kM3TrialTicks; ++t) {
          if (t % frame_ticks == 0) {
            scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
            retina.present(frame.data());
            s.brain.see(retina.features().data(), retina.feature_count());
          }
          const bool sounding = t < kM3LabelTicks;
          voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                       pcm.data(), samples_per_tick);
          ear.tick(s.brain, pcm.data(), samples_per_tick);
          if (sounding && t % kFeedbackPeriodTicks == 0) s.brain.praise(kPraiseValue);
          s.brain.step();
        }
      }
      double moved = 0.0;
      for (size_t i = 0; i < tract.size(); ++i) {
        moved += std::fabs(double(net.synapse_weight(tract[i])) - w0[i]);
      }
      out.dw = tract.empty() ? 0.0 : moved / double(tract.size());
    }

    // --- test ---------------------------------------------------------------
    test_rng.seed(s.dna.header().seed ^ 0x7E57u);
    std::vector<std::vector<double>> pool_v, pool_c;
    std::vector<int> pool_y;
    const uint32_t n_trials = uint32_t(ticks / kM3ProbeTicks);
    for (uint32_t si = 0; si < kScatters; ++si) {
      std::vector<std::vector<double>> xv, xc;
      std::vector<int> labels;
      for (uint32_t trial = 0; trial < n_trials; ++trial) {
        const int label = int(trial % 2);
        Toy toy = m3_toy(test_rng, label);
        place(toy, test_scatter[si], test_rng);
        std::vector<double> cv(net.total_capacity(), 0.0);
        bool slept = false;
        for (uint64_t t = 0; t < kM3ProbeTicks; ++t) {
          if (t % frame_ticks == 0) {
            scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
            retina.present(frame.data());
            s.brain.see(retina.features().data(), retina.feature_count());
          }
          voice.render(0.0f, 0.0f, 0.0f, 0.0f, pcm.data(), samples_per_tick);
          ear.tick(s.brain, pcm.data(), samples_per_tick);
          s.brain.step();
          if (s.brain.asleep()) slept = true;
          if (t < kM3SettleTicks) continue;
          for (uint32_t k = 0; k < net.spike_count(); ++k) cv[net.spikes()[k]] += 1.0;
        }
        if (slept) continue;
        const aibaby::ModuleState& vs = net.module(m_vis);
        const aibaby::ModuleState& cs = net.module(m_cen);
        xv.emplace_back(cv.begin() + vs.begin, cv.begin() + vs.begin + vs.count);
        xc.emplace_back(cv.begin() + cs.begin, cv.begin() + cs.begin + cs.count);
        labels.push_back(label);
        pool_v.push_back(xv.back());
        pool_c.push_back(xc.back());
        pool_y.push_back(label);
      }
      if (labels.size() < 12) return out;
      std::vector<std::vector<double>> iv, ic;
      std::vector<int> yv, yc;
      size_t tv = 0, tc = 0;
      interleave_pairs(xv, labels, iv, yv, tv);
      interleave_pairs(xc, labels, ic, yc, tc);
      out.vis[si] = holdout_accuracy(iv, yv, tv);
      out.cen[si] = holdout_accuracy(ic, yc, tc);
      std::vector<int> sh = yv;
      for (size_t i = sh.size(); i > 1; --i) std::swap(sh[i - 1], sh[test_rng.next() % i]);
      out.shuf[si] = holdout_accuracy(iv, sh, tv);
    }

    // The pooled row. Interleaved so that the four scatter blocks — which the
    // test visits in order — cannot be split by the fold boundary; without it
    // the classifier would train on the two easy blocks and test on the two
    // hard ones and the number would be about the block order.
    if (pool_y.size() >= 24) {
      std::vector<std::vector<double>> iv, ic;
      std::vector<int> yv, yc;
      size_t tv = 0, tc = 0;
      interleave_pairs(pool_v, pool_y, iv, yv, tv);
      interleave_pairs(pool_c, pool_y, ic, yc, tc);
      out.pooled_vis = holdout_accuracy(iv, yv, tv);
      out.pooled_cen = holdout_accuracy(ic, yc, tc);
      std::vector<int> sh = yc;
      for (size_t i = sh.size(); i > 1; --i) std::swap(sh[i - 1], sh[test_rng.next() % i]);
      out.pooled_shuf = holdout_accuracy(ic, sh, tc);
    }
    out.ok = true;
    return out;
  };

  // Replicated over creatures, because the effect this is looking for is the
  // size of one standard error at a single seed. 100 test trials per scatter
  // level puts the per-cell standard deviation near 0.07 and the pooled one
  // near 0.035, and the difference the experiment is built to detect — one
  // upbringing against another — has no reason to be larger than that. A
  // single-seed +0.060 is a coin landing the same way twice, and this project
  // has already published one of those.
  constexpr uint32_t kReps = 3;
  const char* names[3] = {"naive", "centred", "jittered"};
  ArmResult arms[3];
  double per_seed[3][kReps] = {};
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = dna_blob;
    const uint64_t seed = probe.dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    ArmResult one[3];
    one[0] = run_arm(variant, false, 0.0);
    one[1] = run_arm(variant, true, 0.0);
    one[2] = run_arm(variant, true, kTrainScatter);
    for (int a = 0; a < 3; ++a) {
      if (!one[a].ok) {
        std::printf("  arm %s failed on seed %u\n", names[a], r);
        return false;
      }
      for (uint32_t si = 0; si < kScatters; ++si) {
        arms[a].vis[si] += one[a].vis[si] / double(kReps);
        arms[a].cen[si] += one[a].cen[si] / double(kReps);
        arms[a].shuf[si] += one[a].shuf[si] / double(kReps);
      }
      arms[a].pooled_vis += one[a].pooled_vis / double(kReps);
      arms[a].pooled_cen += one[a].pooled_cen / double(kReps);
      arms[a].pooled_shuf += one[a].pooled_shuf / double(kReps);
      arms[a].dw += one[a].dw / double(kReps);
      arms[a].ok = true;
      per_seed[a][r] = one[a].pooled_cen;
    }
  }
  std::printf("\n  pooled `central` per creature, so the spread is visible\n");
  for (int a = 0; a < 3; ++a) {
    std::printf("    %-10s", names[a]);
    for (uint32_t r = 0; r < kReps; ++r) std::printf(" %-9.3f", per_seed[a][r]);
    std::printf("\n");
  }

  std::printf("\n  the displacement curve in `central`, by what the arm was raised on\n");
  std::printf("    %-10s", "scatter");
  for (uint32_t si = 0; si < kScatters; ++si) std::printf(" %-9.2f", test_scatter[si]);
  std::printf("  pooled\n");
  for (int a = 0; a < 3; ++a) {
    std::printf("    %-10s", names[a]);
    for (uint32_t si = 0; si < kScatters; ++si) std::printf(" %-9.3f", arms[a].cen[si]);
    std::printf("  %.3f\n", arms[a].pooled_cen);
  }

  std::printf("\n  the same in `vision`, which cannot learn — this is the ceiling\n");
  for (int a = 0; a < 3; ++a) {
    std::printf("    %-10s", names[a]);
    for (uint32_t si = 0; si < kScatters; ++si) std::printf(" %-9.3f", arms[a].vis[si]);
    std::printf("  %.3f\n", arms[a].pooled_vis);
  }
  std::printf("    %-10s", "shuffled");
  for (uint32_t si = 0; si < kScatters; ++si) std::printf(" %-9.3f", arms[0].shuf[si]);
  std::printf("  %.3f\n", arms[0].pooled_shuf);

  std::printf("\n  did the exposure write anything on vision->central?\n"
              "    centred    mean |dw| %.3e\n"
              "    jittered   mean |dw| %.3e\n",
              arms[1].dw, arms[2].dw);

  bool ok = true;
  // Three controls, in the order in which a failure of each makes the table
  // above meaningless.
  ok = positive_control("invprobe", "the object in `vision` at scatter 0.00",
                        arms[0].vis[0], 0.85,
                        "Nothing below is about displacement.") && ok;
  // The negative control has to be checked too, and for once that is not
  // pedantry: a single-seed run of this experiment read 0.700 on shuffled
  // labels at scatter 0.10, which is 2.9 sd from chance and would have made
  // that whole column unreadable had it not been printed next to the others.
  double worst_shuf = 0.5;
  for (uint32_t si = 0; si < kScatters; ++si) {
    const double d = std::fabs(arms[0].shuf[si] - 0.5);
    if (d > std::fabs(worst_shuf - 0.5)) worst_shuf = arms[0].shuf[si];
  }
  if (std::fabs(worst_shuf - 0.5) > 0.12) {
    std::printf("\n  NEGATIVE CONTROL FAILED — shuffled labels read %.3f somewhere on\n"
                "  the curve above. A readout that scores off labels it was given at\n"
                "  random is reading something other than the object, and the row it\n"
                "  sits on is not a measurement.\n",
                worst_shuf);
    ok = false;
  }
  const double moved = arms[1].dw > arms[2].dw ? arms[1].dw : arms[2].dw;
  if (moved < 1e-6) {
    std::printf("\n  INERT — the exposure moved vision->central by %.3e, which is\n"
                "  nothing. All three arms are the same creature and the rows above\n"
                "  are three measurements of it. The tract's `hebb` is probably 0 and\n"
                "  reward-gated learning alone did not reach it; this experiment has\n"
                "  not answered its question.\n",
                moved);
    ok = false;
  }
  if (ok) {
    const double gain = arms[2].pooled_cen - arms[1].pooled_cen;
    std::printf("\n  Pooled `central`, jittered minus centred: %+.3f. Positive means one\n"
                "  readout got better at spanning positions because the creature was\n"
                "  raised across them — displacement tolerance is learnable here and a\n"
                "  pooling layer is worth building. Flat or negative means it is not,\n"
                "  and no hierarchy bolted on top will change that.\n",
                gain);
  }
  (void)verbose;
  return ok;
}


// --- cpprobe: does closing the critical period protect what was learned? ----
//
// DNA v28 added a per-module critical period — the reward-gated learning rate
// onto a module decays from 1 to `critical_floor` with the module's own age —
// and shipped it off with its **value unmeasured**. `dwprobe` could show the
// envelope reaching the weights at an unrealistically short tau and nothing at
// a realistic one, but that was an instrument limit: dwprobe's window sits
// early in the creature's life, so a 30 s constant has barely begun to close
// inside it. Whether closing the period helps or hurts was never measured, and
// **no experiment in this suite could detect an age-dependent effect of any
// kind**. That gap is worth closing on its own merits.
//
// What a critical period is *for* is not "less learning". It is protection: a
// rate that stays high forever is a brain that keeps overwriting what it knows.
// So the protocol has to teach one thing early, contradict it late, and ask
// which one survives — the imprinting experiment, in this creature's terms.
//
//   phase 1   praise every vocalisation      (learn: be loud)
//   phase 2   scold every vocalisation       (contradict: be quiet)
//   score     the vocalisation rate over the last quarter of phase 2
//
// Two arms, and the difference between them is the whole result:
//
//   open      critical_tau_ms 0 — plasticity never closes. Phase 2 should win.
//   closing   tau chosen to be wide open through phase 1 and near the floor
//             through phase 2. Phase 1 should survive.
//
// The positive control is not optional and it is the arm most likely to fail:
// **the open arm must show phase 2 working.** A no-phase-1 creature that gets
// only the scolding is run as the reference, and if the open arm does not drop
// toward it then nothing is being taught at all, both arms are the same
// untaught animal, and any difference between them is noise with a story
// attached. This experiment has no result without that row.
bool run_cpprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  aibaby::Dna probe;
  if (probe.load(dna_blob.data(), dna_blob.size()) != aibaby::DnaStatus::kOk) return false;
  const double dt = double(probe.header().sim.dt_ms);

  constexpr uint32_t kReps = 3;
  instrument("cpprobe", probe.header().seed, kReps, "creatures per arm");
  std::printf("  phase 1 %.0f s praise every vocalisation, phase 2 %.0f s scold it,\n"
              "  scored over the last quarter of phase 2\n",
              double(ticks) * dt / 2000.0, double(ticks) * dt / 2000.0);

  // One creature. `teach_first` false skips phase 1 and idles instead, which is
  // the reference for "what does scolding alone produce".
  // The observable is the *share of vocalisations above a criterion*, not the
  // raw rate, and the first version of this experiment got that wrong. Raw rate
  // read 131/min against 137/min for an untaught creature — the larynx is near
  // saturated and praise cannot make it vocalise more often. G2 met its
  // milestone by shaping *which* vocalisations happen rather than how many, and
  // this borrows that: the criterion is set from an early baseline window, only
  // events above it are praised (or scolded), and the score is the share above.
  // `criterion` in/out so the untaught reference is judged by the same bar as
  // the creature it is being compared against.
  auto session = [&](const std::vector<uint8_t>& blob, bool teach_first,
                     double* rate_out, double* eta_end, float* criterion) -> bool {
    std::string error;
    Session s;
    if (!s.init(blob, error)) return false;
    const aibaby::DnaAudio& acfg = s.dna.header().audio;
    Ear ear;
    if (!ear.configure(acfg, error)) return false;
    VowelSource voice(acfg.sample_rate);
    std::vector<float> pcm(16);
    const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);

    const uint64_t half = ticks / 2;
    const uint64_t baseline_end = ticks / 10;   // criterion window, inside phase 1
    const uint64_t score_from = ticks - ticks / 4;
    uint64_t events = 0, hits = 0, last_event = 0;
    uint32_t last_frame = 0;
    std::vector<float> baseline_amps;

    for (uint64_t t = 0; t < ticks; ++t) {
      voice.render(0.0f, 0.0f, 0.0f, 0.0f, pcm.data(), samples_per_tick);
      ear.tick(s.brain, pcm.data(), samples_per_tick);
      s.brain.step();

      const float amp = float(s.brain.voice().amplitude);
      const bool loud = s.brain.voice().voicing > 0.5f && amp > kAmplitudeFloor;
      const bool new_event =
          loud && s.brain.vocal_frame() != last_frame && t - last_event >= kEventRefractoryTicks;
      if (s.brain.vocal_frame() != last_frame) last_frame = s.brain.vocal_frame();
      if (new_event) last_event = t;

      // The criterion comes out of an early window of the creature's own
      // behaviour, so the bar is one this body has been observed to clear.
      if (t < baseline_end) {
        if (new_event) baseline_amps.push_back(amp);
      } else if (t == baseline_end && criterion && *criterion <= 0.0f) {
        if (baseline_amps.size() < 8) return false;
        std::sort(baseline_amps.begin(), baseline_amps.end());
        // The 60th percentile: a bar that ~40% of untaught vocalisations clear,
        // so there is room to move in both directions. A median would floor the
        // scolding arm and a high percentile would floor the praising one.
        *criterion = baseline_amps[baseline_amps.size() * 3 / 5];
      }
      const float bar = criterion ? *criterion : kAmplitudeFloor;

      // Phase 1 rewards clearing the bar; phase 2 punishes it. Same schedule,
      // same magnitude — the arms differ in the *sign* of the teaching only.
      const bool phase_one = t < half;
      const bool over = loud && amp >= bar;
      if (phase_one && !teach_first) {
        // Idle: no teaching, so the reference creature meets the scolding with
        // the same amount of life behind it and a critical period at the same
        // point in its own decay.
      } else if (over && t % kFeedbackPeriodTicks == 0) {
        s.brain.praise(phase_one ? kPraiseValue : kScoldValue);
      }

      if (t >= score_from && new_event) {
        ++events;
        if (amp >= bar) ++hits;
      }
    }
    if (events < 15) return false;
    *rate_out = 100.0 * double(hits) / double(events);
    // What the envelope had actually reached by the end, so a null cannot be
    // "the period never closed inside this run" without saying so.
    const aibaby::DnaModule& vm = s.dna.module(uint32_t(s.dna.module_with_role(
        aibaby::ModuleRole::kVocal)));
    if (vm.critical_tau_ms > 0.0f) {
      const double age = double(ticks) * dt;
      const double f = double(vm.critical_floor);
      *eta_end = f + (1.0 - f) * std::exp(-age / double(vm.critical_tau_ms));
    } else {
      *eta_end = 1.0;
    }
    return true;
  };

  struct Arm { const char* name; float tau_ms; float floor; };
  // The closing arm's tau is a quarter of phase 1, so the envelope is wide open
  // while phase 1 teaches and has decayed to within a few percent of the floor
  // before phase 2 starts. A tau comparable to the whole run would close during
  // phase 2 as well and confound "protected" with "stopped learning late".
  const double phase_ms = double(ticks) * dt / 2.0;
  const Arm arms[2] = {{"open", 0.0f, 0.2f},
                       {"closing", float(phase_ms / 4.0), 0.05f}};

  std::printf("\n  scored as the %% of vocalisations clearing the creature's own\n"
              "  60th-percentile amplitude bar, over the last quarter of phase 2\n");
  std::printf("\n    %-9s %-11s %-11s %-11s %s\n", "arm", "taught", "untaught",
              "protected", "eta at end");
  bool ok = true;
  double open_taught = 0, open_untaught = 0, closing_taught = 0, closing_untaught = 0;
  for (int a = 0; a < 2; ++a) {
    double taught = 0, untaught = 0, eta = 1.0, one = 0, two = 0;
    uint32_t n = 0;
    for (uint32_t r = 0; r < kReps; ++r) {
      std::vector<uint8_t> variant = dna_blob;
      const uint64_t seed = probe.header().seed + r * 7919ull;
      std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
      // The envelope is set on `vocal`, which is the module the teaching writes
      // onto. Editing the blob directly rather than the TOML keeps the two arms
      // the same creature in every other respect.
      aibaby::Dna tmp;
      if (tmp.load(variant.data(), variant.size()) != aibaby::DnaStatus::kOk) return false;
      const int32_t voc = tmp.module_with_role(aibaby::ModuleRole::kVocal);
      if (voc < 0) return false;
      const size_t off = sizeof(aibaby::DnaHeader) +
                         size_t(voc) * sizeof(aibaby::DnaModule);
      aibaby::DnaModule dm;
      std::memcpy(&dm, variant.data() + off, sizeof(dm));
      dm.critical_tau_ms = arms[a].tau_ms;
      dm.critical_floor = arms[a].floor;
      std::memcpy(variant.data() + off, &dm, sizeof(dm));

      // One criterion per creature, set by the taught run and reused by its
      // own reference, so the two are judged against the same bar.
      float crit = 0.0f;
      if (!session(variant, true, &one, &eta, &crit)) continue;
      if (!session(variant, false, &two, &eta, &crit)) continue;
      taught += one;
      untaught += two;
      ++n;
    }
    if (!n) return false;
    taught /= double(n);
    untaught /= double(n);
    if (a == 0) { open_taught = taught; open_untaught = untaught; }
    else { closing_taught = taught; closing_untaught = untaught; }
    std::printf("    %-9s %-10.1f%% %-10.1f%% %-+10.1f%% %.3f\n", arms[a].name, taught,
                untaught, taught - untaught, eta);
  }

  std::printf("\n  'protected' is taught minus untaught: how much of phase 1 was still\n"
              "  showing after phase 2 spent an equal time contradicting it.\n");

  // The control, and the row without which neither arm means anything: with the
  // period wide open, phase 2 has to be able to overwrite phase 1. If the open
  // arm's two creatures are indistinguishable, the teaching is not reaching the
  // larynx and both arms are the same untaught animal.
  // A 3-point gap on a percentage over ~40 scored events is about one event;
  // below that the open arm has not demonstrably been taught anything.
  if (std::fabs(open_taught - open_untaught) < 3.0) {
    std::printf("\n  POSITIVE CONTROL FAILED — with plasticity never closing, being\n"
                "  praised for half a life and then scolded for the other half leaves\n"
                "  the creature at %.1f%% against %.1f%% for one that was only ever\n"
                "  scolded. Nothing measurable was taught, so nothing could be\n"
                "  protected, and the closing arm is not evidence about critical\n"
                "  periods — it is two untaught creatures differing by noise.\n",
                open_taught, open_untaught);
    ok = false;
  } else {
    const double gain = (closing_taught - closing_untaught) - (open_taught - open_untaught);
    std::printf("\n  Closing minus open, on the protected column: %+.1f points. Positive\n"
                "  means shutting plasticity down with age preserved early learning\n"
                "  against later contradiction, which is the only thing a critical\n"
                "  period is for. Zero or negative means it bought nothing here and\n"
                "  DNA v28's envelope should ship off on evidence rather than on\n"
                "  never having been asked.\n",
                gain);
  }
  (void)verbose;
  return ok;
}

}  // namespace aibaby_host
