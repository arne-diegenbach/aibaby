// Diagnostics: not goals, but the measurements that say why a goal failed.
//
// Shared scaffolding is in experiments_common.h.

#include <cstring>

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


bool run_m3probe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose,
                 M3ProbeScores* scores) {
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
  auto sweep = [&](bool visual, const char* title, const char* control,
                   std::vector<M3ProbeScores::Row>* rows) -> double {
    const uint32_t n_trials = uint32_t(ticks / kM3ProbeTicks);
    std::vector<std::vector<std::vector<double>>> per_module(modules);
    std::vector<std::vector<std::vector<double>>> per_module_phase(modules);
    std::vector<std::vector<std::vector<double>>> per_module_ema(modules);
    std::vector<std::vector<double>> vocal;
    std::vector<std::vector<double>> centroids, activities;
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
        const aibaby::Scalar* a = s.brain.vocal_activities();
        for (uint32_t k = 0; k < aibaby::kVocalGroups; ++k) {
          rec.group[k] += double(g[k]);
          rec.act[k] += double(a[k]);
        }
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
      centroids.push_back(m3_centroid_features(rec));
      activities.push_back(m3_activity_features(rec));
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
    // `centroid` is the prerequisite question for topography, and it is not a
    // variant of the columns beside it — it is the readout the larynx actually
    // uses. The vocal decoder reads sum(rate_i * i) / sum(rate_i) over a group,
    // so a topographic projection from some module into vocal can only deliver
    // whatever that module's OWN centroid carries about the object. If a module
    // scores 0.85 per neuron and 0.50 on its centroid, then every neuron knows
    // which toy it is and their centre of mass does not, and no wiring into a
    // centroid readout can rescue that. Cheaper to ask than to build.
    std::printf("    %-12s %-11s %-11s %-11s %-11s %-11s %-11s %-11s %-11s %s\n", "module",
                "per-neuron", "interleaved", "rate EMA", "32 space", "8x4 space+t",
                "rate only", "shuffled", "centroid", "spikes/trial");
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
      std::vector<std::vector<double>> centroid;
      for (const std::vector<double>& v : per_module[m]) {
        binned.push_back(rebin(v, kFeatureBins));
        double total = 0.0;
        for (double x : v) total += x;
        spikes += total;
        rate.push_back(std::vector<double>{total});
        // Exactly the vocal decoder's readout: the centre of mass of activity
        // over neuron index, normalised to [0,1] so it is the same quantity a
        // motor group hands the larynx.
        double weighted = 0.0;
        for (size_t k = 0; k < v.size(); ++k) {
          weighted += v[k] * ((double(k) + 0.5) / double(v.size()));
        }
        centroid.push_back(std::vector<double>{total > 1e-9 ? weighted / total : 0.5});
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
      const double interleaved = holdout_accuracy(xi, yi, itrain);
      if (std::strcmp(net.module_dna(m).name, control) == 0) control_score = per_neuron;
      // Recorded from the same locals the table prints, so the two can never
      // disagree about what was measured.
      if (rows) rows->push_back({net.module_dna(m).name, per_neuron, interleaved});

      std::printf("    %-12s %-11.3f %-11.3f %-11.3f %-11.3f %-11.3f %-11.3f %-11.3f %-11.3f %.1f%s\n",
                  net.module_dna(m).name,
                  per_neuron,
                  interleaved,
                  holdout_accuracy(per_module_ema[m], labels, train),
                  holdout_accuracy(binned, labels, train),
                  holdout_accuracy(per_module_phase[m], labels, train),
                  holdout_accuracy(rate, labels, train),
                  holdout_accuracy(per_module_phase[m], shuffled, train),
                  holdout_accuracy(centroid, labels, train),
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
    // The same nine groups split into the two numbers a group reading has.
    // Only `centroid` reaches the articulators; `activity` reaches the larynx
    // for exactly one group (voicing, as loudness) and is discarded for the
    // other eight. If activity beats centroid here, the vocal tract is
    // throwing away the object on purpose and the decoder is the fix.
    std::printf("    %-12s %-11.3f %s\n", "  centroids",
                holdout_accuracy(centroids, labels, train),
                "<- the 9 group CENTROIDS alone: what the articulators get");
    std::printf("    %-12s %-11.3f %s\n", "  activities",
                holdout_accuracy(activities, labels, train),
                "<- the 9 group RATES alone: what they do NOT get");
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
  const double vis_ctl = sweep(true, "a cube or a ball, in silence", "vision",
                               scores ? &scores->visual : nullptr);
  const double aud_ctl = sweep(false, "one of two words, to an empty field", "auditory",
                               scores ? &scores->auditory : nullptr);
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
// --- Where acquisition actually breaks --------------------------------------
//
// The reflex recovers 72% of the oracle gain inside the fovea and 0% outside
// it, and DNA v33 established the aiming RULE is not why: a spatial radius
// changes nothing out there at any setting. So the question left is whether the
// controller is given anything to aim at, and that is answerable directly.
//
// Put the eye at the frame centre, show one toy at a known offset, run one
// frame. The controller's update is `gaze = gain * (aim - centre)`, so
// `gaze / gain` recovers exactly where it believed the toy was — no new
// accessor required. Sweep the offset and compare the belief with the truth:
//
//   belief tracks truth        -> the retina sees it; the loop is the problem
//   belief collapses to zero   -> nothing to aim at, and this says at what
//                                 radius the retina stops localising
//
// `contrast` is printed beside it because the controller refuses to move below
// `contrast_floor`, and a refusal and a wrong estimate look identical from the
// outside.
void run_acquisition_section(const aibaby::Dna& dna, const aibaby::DnaVision& vcfg) {
  Retina retina;
  std::string error;
  if (!retina.configure(vcfg, error)) return;
  SceneSource scene(vcfg.frame_size, dna.header().seed);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  const float mid = 0.5f;
  const float fs = float(vcfg.frame_size);
  const float gain = vcfg.gaze_gain > 0.0f ? vcfg.gaze_gain : 1.0f;

  std::printf("\n  ACQUISITION: where does the eye end up, per starting offset\n");
  std::printf("    the fovea spans +/-%.0f px; ring 1 +/-%.0f px; ring 2 +/-%.0f px\n",
              double(vcfg.fovea_size) / 2.0, double(vcfg.fovea_size),
              double(vcfg.fovea_size) * 2.0);
  std::printf("    one toy, fixed size, eye released from the centre and given 40\n"
              "    frames to converge. The first version of this read the gaze after a\n"
              "    SINGLE frame and reported the eye blind at 2 px, which cannot be\n"
              "    true because the reflex works inside the fovea — the command goes\n"
              "    through the eye's queue and is not visible that soon.\n");
  std::printf("\n    %-10s %-10s %-10s %-10s %s\n", "toy at", "eye ends", "error",
              "contrast", "verdict");
  const double offsets[] = {0.0, 2.0, 4.0, 6.0, 8.0, 12.0, 16.0, 24.0};
  for (double off : offsets) {
    Toy toy = m3_toy_at(0.5 + off / double(fs), 0.5);
    scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
    retina.look_at(0.0f, 0.0f);
    for (int f = 0; f < 40; ++f) retina.present(frame.data());
    const double ended = double(retina.gaze_x());
    const double err = std::fabs(off - ended);
    const double contrast = double(retina.contrast());
    const char* verdict =
        contrast <= double(vcfg.contrast_floor)
            ? "REFUSED — under contrast_floor"
            : (err < 2.0 ? "acquired"
                         : (std::fabs(ended) < 2.0 ? "never left the centre" : "partial"));
    std::printf("    %-10.1f %-10.1f %-10.1f %-10.4f %s\n", off, ended, err, contrast,
                verdict);
  }
  // The control this whole idea lives or dies on. `gaze_contrast_floor` exists
  // to stop the eye chasing grain, so any setting that improves acquisition has
  // to be checked against an EMPTY field: if the eye wanders with nothing in
  // view, the floor is too low and the acquisition numbers above were bought by
  // making the creature twitchy rather than perceptive.
  scene.render(SceneSource::Shape::kNone, 0.5f, 0.5f, 0.1f, 0.85f, 0.02f, frame.data());
  retina.look_at(0.0f, 0.0f);
  for (int f = 0; f < 40; ++f) retina.present(frame.data());
  const double drift = std::sqrt(double(retina.gaze_x()) * double(retina.gaze_x()) +
                                 double(retina.gaze_y()) * double(retina.gaze_y()));
  std::printf("\n    empty field: eye drifts %.1f px, contrast %.4f, floor %.4f  %s\n",
              drift, double(retina.contrast()), double(vcfg.gaze_contrast_floor),
              drift < 1.0 ? "<- still, as it must be" : "<- CHASING NOISE");
  std::printf("\n    The radius where this stops being 'acquired' is what a peripheral\n"
              "    channel would have to cover, and whether it stops by REFUSING or by\n"
              "    aiming short says whether the fix is the floor or the rings.\n");
  (void)mid;
  (void)gain;
}

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
  enum Fovea {
    kFixed = 0, kReflex = 1, kOracle = 2, kServo = 3,
    // The eye-port arms. Everything from here on drives an eye this class does
    // not own: the frame arrives already aimed and the position comes back as a
    // report, which is what a pan/tilt head or an upstream cropper looks like
    // from in here. They run in their own section rather than in the table
    // above, because they answer "does the seam work" and not "what is
    // foveation worth".
    kDevice = 4,        // an ideal external eye, wired up correctly
    kDoubled = 5,       // the same eye with the mount left internal
    kDoubledServo = 6,  // ...on a motor, where a doubled gain has somewhere to go
    kDropout = 7,       // the eye goes silent half way through every trial
    kDropNoFreeze = 8,  // ...with the freeze disabled, to price it
  };
  const char* fovea_name[9] = {"fixed", "reflex", "oracle", "servo", "device",
                               "doubled", "dbl+servo", "dropout", "no freeze"};
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
  // Written by the eye-port arms and read by the section that prints them.
  // Out-params rather than a struct only because `arm` already has six and the
  // two ways of adding a seventh are both worse than this.
  double arm_drift = 0.0, arm_believed = 0.0;
  uint64_t arm_stalls = 0, arm_reports = 0, drop_after_override = 0;
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
      const bool driven = fovea != kFixed && fovea != kOracle;
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
    if (fovea == kServo || fovea == kDoubledServo) {
      retina.set_servo(servo_model, s.dna.header().seed ^ 0x5E70u);
    }
    // The eye port. The `doubled` pair are the integration mistake this enum
    // exists to price, so they are the arms that have a device aiming while the
    // mount still tells the retina to aim as well.
    const bool device = fovea >= kDevice;
    const bool doubled = fovea == kDoubled || fovea == kDoubledServo;
    if (device && !doubled) retina.set_eye_mount(Retina::EyeMount::kExternal);
    if (fovea == kDropNoFreeze) retina.set_eye_timeout_frames(1u << 30);
    float dev_x = 0.0f, dev_y = 0.0f;
    uint64_t dev_frames = 0;
    // Half of EVERY trial, not the first ten frames of the run. Killing the eye
    // once leaves ninety-nine trials measuring one frozen position, which reads
    // as a dead controller and says nothing about what losing a device costs.
    // A link that drops half way through each fixation is also the failure a
    // real one has.
    const uint64_t drop_after = drop_after_override ? drop_after_override : 10;
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
    double gaze_err = 0.0, want_err = 0.0, believed_err = 0.0;
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
          // An external eye has already turned by the time the frame is taken,
          // so the world arrives displaced by the opposite of where the head is
          // pointing. That is the whole difference between the two mounts, and
          // rendering it here rather than cropping is what makes this a test of
          // the port instead of a reimplementation of the sampler.
          float tx = toy.cx, ty = toy.cy;
          if (device) {
            tx -= dev_x / float(vcfg.frame_size);
            ty -= dev_y / float(vcfg.frame_size);
          }
          if (t == 0) dev_frames = 0;
          scene.render(toy.shape, tx, ty, toy.radius, 0.85f, 0.02f, frame.data());
          retina.present(frame.data());
          s.brain.see(retina.features().data(), retina.feature_count());
          if (device) {
            // The device: ideal, so what is being measured is the seam and not
            // a motor — `servo` above already prices the motor. It reads the
            // published command, is there by the next frame, and says so.
            const Retina::GazeCommand g = retina.gaze_command();
            ++dev_frames;
            if (fovea < kDropout || dev_frames <= drop_after) {
              dev_x = g.x_px;
              dev_y = g.y_px;
              Retina::GazeReport rep;
              rep.x_px = dev_x;
              rep.y_px = dev_y;
              rep.seq = g.seq;
              retina.report_gaze(rep);
            }
          }
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
        const double want_x = (double(toy.cx) - 0.5) * double(vcfg.frame_size);
        const double want_y = (double(toy.cy) - 0.5) * double(vcfg.frame_size);
        // Where the eye effectively ended up. On the doubled arm that is the
        // sum of two aims — the device turned the head and the retina slid its
        // window over the result — and reading either one alone would report a
        // controller doing fine while the creature looks at nothing.
        const double eye_x = double(retina.gaze_x()) + (doubled ? double(dev_x) : 0.0);
        const double eye_y = double(retina.gaze_y()) + (doubled ? double(dev_y) : 0.0);
        gaze_err += std::sqrt((eye_x - want_x) * (eye_x - want_x) +
                              (eye_y - want_y) * (eye_y - want_y));
        // What the host would have told you the eye was doing. On every other
        // arm this is the same number; on the doubled ones it is the lie the
        // mistake produces, and the size of the gap is the whole finding.
        believed_err += std::sqrt((double(retina.gaze_x()) - want_x) *
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
    // How far the command ended up from the eye it is supposed to be steering.
    // This is the quantity the freeze protects: while a device is silent the
    // eye cannot move, so the damage a runaway controller does is entirely to
    // the instruction waiting for the device when it comes back.
    {
      const Retina::GazeCommand g = retina.gaze_command();
      arm_drift = std::sqrt((double(g.x_px) - double(dev_x)) *
                                (double(g.x_px) - double(dev_x)) +
                            (double(g.y_px) - double(dev_y)) *
                                (double(g.y_px) - double(dev_y)));
      arm_stalls = retina.eye_stalls();
      arm_reports = retina.eye_reports();
      arm_believed = gaze_n ? believed_err / double(gaze_n) : 0.0;
    }
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
  // The eye port. Everything above drives an eye this class owns — a crop
  // window over a fixed camera, with or without a simulated motor. A real
  // moving eye belongs to somebody else: it takes a command, turns in its own
  // time, and hands back a position, and the frames it produces are already
  // aimed. These four arms are that seam, measured rather than asserted.
  //
  // `device` is the one that has to come out equal to `reflex`. If a correctly
  // wired external eye scores differently from the internal one, the port is
  // lying about something and nothing built on it can be trusted.
  bool port_ok = true;
  {
    std::printf("\n  the eye port, at scatter 0.10 (toy 4.9 px out)\n");
    std::printf("    %-11s %-9s %-10s %-10s %-9s %s\n", "eye", "vision", "gaze err",
                "host says", "reports", "held");
    double reflex_v = 0.0, reflex_err = 0.0, device_v = 0.0, device_err = 0.0;
    double dbl_err = 0.0, dbl_says = 0.0, dbl_servo_err = 0.0, servo_err = 0.0;
    double freeze_drift = 0.0, nofreeze_drift = 0.0;
    for (int f : {kReflex, kServo, kDevice, kDoubled, kDoubledServo, kDropout,
                  kDropNoFreeze}) {
      double v = 0, c = 0, sh = 0, err = 0, base = 0;
      uint64_t moves = 0;
      if (!arm(0.10, f, &v, &c, &sh, &err, &base, &moves)) {
        std::printf("    %-11s arm failed\n", fovea_name[f]);
        port_ok = false;
        continue;
      }
      const char* note = f == kReflex ? "   <- the internal eye, for comparison"
                       : f == kServo  ? "   <- ...and on the pessimistic motor"
                                      : "";
      std::printf("    %-11s %-9.3f %-10.1f %-10.1f %-9llu %llu%s\n", fovea_name[f],
                  v, err, arm_believed, (unsigned long long)arm_reports,
                  (unsigned long long)arm_stalls, note);
      if (f == kReflex) { reflex_v = v; reflex_err = err; }
      if (f == kServo) servo_err = err;
      if (f == kDevice) { device_v = v; device_err = err; }
      if (f == kDoubled) { dbl_err = err; dbl_says = arm_believed; }
      if (f == kDoubledServo) dbl_servo_err = err;
      if (f == kDropout) freeze_drift = arm_drift;
      if (f == kDropNoFreeze) nofreeze_drift = arm_drift;
    }
    // Not exact equality: the external arm's device turns a head and sees new
    // world where the internal arm's crop runs off the frame and edge-extends,
    // so the two disagree about the background at the border by construction.
    // The toy is what either eye is aiming at, and if the seam is right they
    // land on it equally well.
    const double dv = std::fabs(device_v - reflex_v);
    const double de = std::fabs(device_err - reflex_err);
    if (dv > 0.10 || de > 1.0) {
      std::printf("    THE PORT DISAGREES WITH ITSELF — external %.3f/%.1f px against\n"
                  "    internal %.3f/%.1f px. A correctly wired device is the same\n"
                  "    creature; this is a bug in the seam, not a finding about eyes.\n",
                  device_v, device_err, reflex_v, reflex_err);
      port_ok = false;
    }
    // The doubled arms. The expectation going in was that applying the aim
    // twice would tear the loop apart, and it does not: at gain 0.70 the
    // doubled loop is 1.4, inside the stability bound of 2 for a proportional
    // controller, so it converges on the toy and scores like a working eye.
    // That is the finding, and it is worse news than divergence would have
    // been — the mistake is SILENT, and the only thing that gives it away is
    // that the two ends disagree about where the eye is.
    std::printf("\n    applying the aim twice CONVERGES: the loop lands %.1f px from the\n"
                "    toy while the host reports %.1f px, because each end is doing half\n"
                "    the aiming and only one of them is being asked. Nothing above\n"
                "    fails; the telemetry, the journal and every gaze number are wrong.\n",
                dbl_err, dbl_says);
    if (dbl_servo_err > 0.0) {
      // And it is not even punished on a slow motor, which is where a doubled
      // gain was supposed to have somewhere to go. It is not: slew 0.5 was
      // already halving the loop, so doubling it lands back near the gain the
      // controller was tuned at. Two mistakes cancelling is not a defence —
      // it means there is no arm here in which performance reveals the bug.
      std::printf("    on the pessimistic motor it costs %.1f px against %.1f px wired\n"
                  "    correctly, i.e. nothing: slew 0.5 was already halving the loop, so\n"
                  "    the doubling cancels it. No arm in this probe detects the mistake\n"
                  "    from performance. The position disagreement is the only symptom,\n"
                  "    which is why `mount` is an enum and not a comment.\n",
                  dbl_servo_err, servo_err);
    }
    // The freeze, priced honestly. It was built from the servo sweep's
    // divergence, and the sweep's divergence turns out not to be this failure.
    std::printf("\n    a silent eye costs %.1f px of command drift with the freeze on and\n"
                "    %.1f px with it off, because by the time the link drops the eye is\n"
                "    already on the toy and there is no error left to act on.\n",
                freeze_drift, nofreeze_drift);
    {
      // The case where there IS error left to act on: a device that dies on the
      // first frame, before the eye ever reaches the toy. Dropping at frame 10
      // finds the eye already there and dropping at 12 px finds a residual the
      // controller cannot perceive anyway — peripheral acquisition is exactly
      // what it cannot do — so this is the one arrangement that leaves the loop
      // wanting something when the link goes.
      double v = 0, c = 0, sh = 0, err = 0, base = 0;
      uint64_t moves = 0;
      double early_freeze = 0.0, early_open = 0.0;
      drop_after_override = 1;
      if (arm(0.10, kDropout, &v, &c, &sh, &err, &base, &moves)) early_freeze = arm_drift;
      if (arm(0.10, kDropNoFreeze, &v, &c, &sh, &err, &base, &moves)) early_open = arm_drift;
      drop_after_override = 0;
      std::printf("    with the eye lost on the FIRST frame, before it ever gets there:\n"
                  "    %.1f px held against %.1f px open. That is the whole of it, and it\n"
                  "    is bounded by construction rather than by the freeze — the loop is\n"
                  "    proportional with no integrator, so a command is one gain-step off\n"
                  "    a frozen belief and cannot wind up however long the device is gone.\n"
                  "    The freeze buys that step and an explicit `held` counter. The\n"
                  "    runaway it was built against needs a device that answers LATE, not\n"
                  "    one that stops: that is the servo row, at 26 px.\n",
                  early_freeze, early_open);
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
  // Diagnosis for the OUTSIDE bucket above, which the arms can only report
  // as a null: is the controller aiming badly, or aiming at nothing?
  run_acquisition_section(probe.dna, vcfg);
  (void)verbose;
  if (!ok || !port_ok) return false;
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

// --- restate: does the documented creature still exist? ----------------------
//
// Built 2026-08-20, after two documented numbers sent work in the wrong
// direction inside three days.
//
//   - "the object never reaches the larynx; vocal is at chance for the seen
//     object, 0.500" was true when written and false since `vision->vocal`
//     shipped. An entire mechanism (the interneuron relay) was designed,
//     built, calibrated and measured against a bottleneck that had already
//     closed. It reads 0.660.
//   - `eligprobe`'s central->vocal conditionality was on record at 0.654 and
//     reads 0.742 on the current creature, which changed the premise of the
//     experiment it was quoted to justify.
//
// Both were correct when recorded. Nothing noticed when they stopped being
// correct, because a README number has no test attached to it. `verify` pins
// exactly one quantity this way — the determinism hash — and that pin has paid
// for itself repeatedly. This is the same idea for the numbers that actually
// drive what gets built next.
//
// **The expectations are pinned to what THIS experiment reads**, the way
// `kPinnedHash` is pinned to what the build produces — not to a figure quoted
// elsewhere under a different protocol. That distinction is load-bearing: the
// first version pinned the prose's three-seed-family means against this
// experiment's three within-family replicates and carried a systematic offset
// of up to 0.08 before anything had drifted at all, spending the tolerance
// budget on a units mismatch. The `recorded` column says where the prose claim
// lives so a human can reconcile the two whenever either moves.
//
// It is also **deterministic** — same genome, same seeds, same trial RNG — so a
// row that moves means the creature moved, not that the dice did.
//
// **The tolerances are how much change is worth hearing about, not the
// instrument's noise.**
// A per-neuron accuracy over 100 trials has SE ~0.05, so ±0.15 is roughly three
// of those: wide enough that ordinary variation is quiet, tight enough that a
// number moving from chance to two-thirds is loud. The point is to catch a
// premise that has died, not to detect drift in the third decimal.
//
// Adding an entry is one row plus one measurement. The bar for being here is
// narrow: a number is a candidate only if some *decision* would change if it
// moved. Everything else belongs in the probe that prints it.
struct Restatement {
  const char* quantity;   // what it is
  const char* recorded;   // where the project says it, and what it says
  double expected;
  double tolerance;
  double measured;
  bool measured_ok;       // false when the probe could not produce it at all
};

bool run_restate(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;

  std::printf("  Re-derives the numbers this project makes decisions on, against\n"
              "  what it has written down. A red row does not mean the creature is\n"
              "  broken -- it means a document is describing a creature that no\n"
              "  longer exists, and anything built on that row is built on sand.\n\n");

  // --- the delivery numbers, from m3probe's own table ------------------------
  // 200k rather than the 900k the milestone uses: this asks where the object
  // arrives in the resting dynamics, which needs no learning and no teacher.
  // Three creatures, not one, and that is not a luxury. A per-neuron accuracy
  // over 100 trials has SE ~0.05, and the figures this checks against were
  // themselves recorded as three-seed means -- pinning a one-seed draw against
  // a three-seed mean builds a systematic offset into the comparison and spends
  // the tolerance budget on it.
  std::printf("  --- m3probe, 200000 ticks x 3 creatures ---\n");
  M3ProbeScores sc;
  {
    std::vector<M3ProbeScores> each;
    for (uint32_t r = 0; r < 3; ++r) {
      std::vector<uint8_t> variant = blob;
      const uint64_t seed = dna.header().seed + r * 7919ull;
      std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
      M3ProbeScores one;
      run_m3probe(variant, 200000, verbose, &one);
      each.push_back(one);
    }
    // Averaged by module name rather than by index: growth can leave two
    // creatures with different module widths, and nothing here should quietly
    // average a row onto a different module.
    auto fold = [&](bool vis) {
      std::vector<M3ProbeScores::Row> out;
      for (const M3ProbeScores::Row& proto : (vis ? each[0].visual : each[0].auditory)) {
        double pn = 0, il = 0; uint32_t n = 0;
        for (const M3ProbeScores& e : each) {
          const double v = e.get(vis, proto.module.c_str());
          if (v < 0.0) continue;
          pn += v; il += e.get(vis, proto.module.c_str(), true); ++n;
        }
        if (n) out.push_back({proto.module, pn / n, il / n});
      }
      return out;
    };
    sc.visual = fold(true);
    sc.auditory = fold(false);
  }

  // --- the trace, from eligprobe's own session -------------------------------
  // 600k because below it eligprobe's positive control is blind, and a blind
  // control would make this whole experiment agree with anything.
  std::printf("\n  --- eligprobe, 600000 ticks x 3 creatures ---\n");
  double cen = 0, arc = 0;
  uint32_t valid = 0;
  for (uint32_t r = 0; r < 3; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    const EligProbe p = run_eligprobe_session(variant, 600000);
    if (!p.ok) continue;
    cen += p.central; arc += p.arcuate_matched; ++valid;
  }
  if (valid) { cen /= valid; arc /= valid; }
  std::printf("    central->vocal conditionality  %.3f   (arcuate control %.3f, %u creatures)\n",
              cen, arc, valid);

  Restatement rows[] = {
      {"object at vocal, per neuron",
       "README 'three caps': 0.660 over three seed FAMILIES -- NOT the old 0.500",
       0.733, 0.15, sc.get(true, "vocal"), sc.get(true, "vocal") >= 0.0},
      {"object at central, per neuron",
       "README G3 cascade: the condition upstream of the larynx",
       0.653, 0.15, sc.get(true, "central"), sc.get(true, "central") >= 0.0},
      {"object at vision, per neuron",
       "the positive control: if this falls, nothing else here is readable",
       0.947, 0.15, sc.get(true, "vision"), sc.get(true, "vision") >= 0.0},
      {"word at vocal, per neuron",
       "the echo route -- what makes the creature able to repeat but not name",
       0.847, 0.15, sc.get(false, "vocal"), sc.get(false, "vocal") >= 0.0},
      {"word at auditory, per neuron",
       "the auditory positive control",
       1.000, 0.10, sc.get(false, "auditory"), sc.get(false, "auditory") >= 0.0},
      {"central->vocal trace conditionality",
       "eligprobe: 0.742 at 5 creatures, NOT the long-recorded 0.654",
       0.813, 0.12, cen, valid >= 2},
      {"arcuate trace, size-matched",
       "eligprobe's positive control -- a tract known to carry a condition",
       0.973, 0.12, arc, valid >= 2},
  };

  std::printf("\n  %-38s %-9s %-9s %-8s %s\n", "quantity", "expected", "measured",
              "drift", "verdict");
  bool ok = true;
  for (const Restatement& r : rows) {
    if (!r.measured_ok) {
      std::printf("  %-38s %-9.3f %-9s %-8s %s\n", r.quantity, r.expected, "--", "--",
                  "NOT MEASURED — the probe could not produce this");
      ok = false;
      continue;
    }
    const double drift = r.measured - r.expected;
    const bool held = std::fabs(drift) <= r.tolerance;
    if (!held) ok = false;
    std::printf("  %-38s %-9.3f %-9.3f %+-8.3f %s\n", r.quantity, r.expected, r.measured,
                drift, held ? "ok" : "DRIFTED — see below");
  }

  std::printf("\n  where each expectation is written down\n");
  for (const Restatement& r : rows) std::printf("    %-38s %s\n", r.quantity, r.recorded);

  if (!ok) {
    std::printf("\n  RESTATE FAIL — at least one documented number no longer describes\n"
                "  this creature. Before treating that as a regression, check whether\n"
                "  the creature improved: both drifts that motivated this experiment\n"
                "  were numbers getting BETTER while the prose still said otherwise.\n"
                "  Fix the document, or the expectation here, in the same change that\n"
                "  moved the creature.\n");
  } else {
    std::printf("\n  restate PASS — every pinned number still describes this creature.\n");
  }
  (void)ticks;
  return ok;
}


// --- stpprobe: is a dynamic synapse a filter, and what does it filter? ------
//
// DNA v36 puts Webb's cricket mechanism into this creature: a synapse whose
// resources deplete with use and recover on their own time constant, so that
// what it transmits depends on the *interval* between the spikes arriving at
// it. Webb's model gets song recognition out of exactly this and nothing else —
// BN1 fires efficiently only when the gap between sound bursts is long enough
// for it to have recovered, BN2 only when onsets arrive close enough together
// for facilitation to still be standing, and no part of the resulting bandpass
// on syllable rate is learned.
//
// This asks the two questions that have to be separated before any of that can
// be believed, and it asks them on `auditory -> central`, the tract a heard
// word crosses:
//
//   1. **Does the mechanism run?** The `gain` column is what the tract actually
//      delivered as a fraction of its genome weight. The off arm must read
//      1.000 — it is the control that says the patch is real and not a
//      relabelled copy of the same creature — and the depressing arm must fall
//      as the rate rises. That pair is the PASS criterion, because it is about
//      the kernel's arithmetic rather than about the creature, and a failure
//      there is a bug.
//
//   2. **Does the filter reach the target?** The `transfer` column is central's
//      spikes per auditory spike. It is a ratio and not a count for the reason
//      the whole probe would otherwise be worthless: the ear responds
//      differently to a 2 Hz and a 12 Hz envelope all by itself, and a rate
//      effect measured at central alone would mostly be that. This column is
//      the finding, and it is NOT gated on — whether a per-edge filter survives
//      a sparse random tract is a result, not a correctness check.
//
// Three things about the design that are not free choices.
//
// **The duty cycle is fixed at 50%, not the burst duration.** Every rate then
// carries identical total sound and identical mean amplitude, and only the
// timing differs. With a fixed burst length the fast rates would simply be
// louder, and a depressing synapse would be measured as an energy meter.
//
// **The rates stop at 12 Hz because the ear stops there.** The cochlea's window
// is 32 ms with a 16 ms hop, so an envelope whose half-period is under ~32 ms
// is inside one analysis frame and reaches the brain as steady energy. Webb's
// crickets work at 20-30 Hz syllables through an ear with microsecond
// resolution; the band this creature's ear can actually resolve is 1-12 Hz,
// which is where the syllable rate of human speech sits anyway.
//
// **There is a shuffled row and a silent row.** The shuffled row holds the mean
// rate and the total sound of the 4 Hz row and destroys only the regularity, so
// it separates "selective for an interval" from "selective for a modulation
// depth". The silent row is central's baseline: it receives vision and touch
// and its own noise as well as the ear, and a transfer ratio that ignored that
// would credit the tract with spikes nothing sent it.
namespace {

struct StpArm {
  const char* name;
  float use;         // U: released per spike
  float recover_ms;  // tau_rec
  float facil_ms;    // tau_facil
};

// Webb's two synapses, plus the constant-weight one this creature has always
// had. The numbers are the standard depressing and facilitating corners of the
// Tsodyks-Markram parameter space rather than a fit to anything: this probe
// exists to show what the mechanism does, and the genome is where a tract that
// ships would state its own.
constexpr StpArm kStpArms[] = {
    {"off",          0.00f,   0.0f,   0.0f},
    {"depressing",   0.50f, 300.0f,   0.0f},
    {"facilitating", 0.10f,  50.0f, 300.0f},
};
constexpr uint32_t kStpArmCount = sizeof(kStpArms) / sizeof(kStpArms[0]);

// Hz. The last one is at the ear's resolution limit and is meant to be.
constexpr float kStpRates[] = {2.0f, 4.0f, 8.0f, 12.0f};
constexpr uint32_t kStpRateCount = sizeof(kStpRates) / sizeof(kStpRates[0]);

// silence, the four rates, and the shuffled null
constexpr uint32_t kStpConditions = kStpRateCount + 2;

// Long enough for §3.1 to have settled, which 1500 was not. The first version
// of this probe used 1500, ran its silent condition first on a just-hatched
// creature, and read silence at 1.30 spikes/tick where a settled creature reads
// 0.72 — so the "silence versus speech" gap it reported was mostly the
// difference between an unsettled brain and a settled one. `ipprobe` measures
// the same quantity with a settled half on each side and disagrees by an order
// of magnitude, which is what exposed it.
constexpr uint64_t kStpSettleTicks = 20000;

struct StpRow {
  double aud_per_tick = 0.0;  // the quantity a dynamic synapse actually reads
  double cen_per_tick = 0.0;
  double transfer = 0.0;      // central's spikes per auditory spike
  double gain = 1.0;          // delivered, as a fraction of the genome weight
  uint32_t bursts = 0;
};

}  // namespace

bool run_stpprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  aibaby::Dna dna0;
  if (dna0.load(dna_blob.data(), dna_blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t aud_m = dna0.module_with_role(aibaby::ModuleRole::kAuditory);
  const int32_t cen_m = dna0.module_with_role(aibaby::ModuleRole::kAssociation);
  if (aud_m < 0 || cen_m < 0) {
    std::printf("  this genome has no auditory or no association module\n");
    return false;
  }
  // The tract to make dynamic. Chosen because it is the one a heard word
  // crosses, and because `pcprobe` measures it as the place the word is lost.
  int32_t tract = -1;
  for (uint32_t i = 0; i < dna0.header().projection_count; ++i) {
    const aibaby::DnaProjection& p = dna0.projection(i);
    if (int32_t(p.src) == aud_m && int32_t(p.dst) == cen_m) { tract = int32_t(i); break; }
  }
  if (tract < 0) {
    std::printf("  this genome has no auditory->central tract to make dynamic\n");
    return false;
  }

  const aibaby::DnaAudio& acfg = dna0.header().audio;
  const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);
  const uint64_t budget = ticks / (kStpArmCount * kStpConditions);

  instrument("stpprobe", dna0.header().seed ^ 0x57Bu, budget, "ticks per condition");
  std::printf("  tract             %s -> %s   density %.3f   weight %.3f\n",
              dna0.module(uint32_t(aud_m)).name, dna0.module(uint32_t(cen_m)).name,
              double(dna0.projection(uint32_t(tract)).density),
              double(dna0.projection(uint32_t(tract)).weight));
  const double window_ms = 1000.0 * double(acfg.window) / double(acfg.sample_rate);
  std::printf("  ear               %.0f ms window, %.0f ms hop — an envelope whose half\n"
              "                    period is under the window (~%.0f Hz) is inside one\n"
              "                    frame and reaches the brain as steady energy\n",
              window_ms, 1000.0 * double(acfg.hop) / double(acfg.sample_rate),
              500.0 / window_ms);

  // The condition index is the same across arms so the rows line up: 0 is
  // silence, 1..kStpRateCount are the regular rates, and the last is shuffled.
  StpRow rows[kStpArmCount][kStpConditions];

  for (uint32_t a = 0; a < kStpArmCount; ++a) {
    // Every arm patches all three fields EXPLICITLY, including the off arm.
    // Reading them from the genome instead is the bug gazeprobe shipped: the
    // day someone switches the mechanism on in dna/default.toml, the control
    // arm would silently stop being a control and all three rows would agree.
    std::vector<uint8_t> variant = dna_blob;
    {
      const size_t base = sizeof(aibaby::DnaHeader) +
                          sizeof(aibaby::DnaModule) * dna0.module_count() +
                          sizeof(aibaby::DnaProjection) * size_t(tract);
      const float u = kStpArms[a].use;
      const float rec = kStpArms[a].recover_ms;
      const float fac = kStpArms[a].facil_ms;
      std::memcpy(variant.data() + base + offsetof(aibaby::DnaProjection, stp_use),
                  &u, sizeof(u));
      std::memcpy(variant.data() + base + offsetof(aibaby::DnaProjection, stp_recover_ms),
                  &rec, sizeof(rec));
      std::memcpy(variant.data() + base + offsetof(aibaby::DnaProjection, stp_facil_ms),
                  &fac, sizeof(fac));
    }

    Session s;
    if (!s.init(variant, error)) {
      std::printf("  arm %s failed to hatch: %s\n", kStpArms[a].name, error.c_str());
      return false;
    }
    const aibaby::Network& net = s.brain.network();
    Ear ear;
    if (!ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource voice(acfg.sample_rate);
    std::vector<float> pcm(samples_per_tick);
    const Word& w = kWords[0];

    // One RNG per arm, seeded identically, so the shuffled schedule is the
    // same irregular pattern in all three. A different draw per arm would put
    // the arms on different stimuli, which is the mistake `instrument` exists
    // to shout about.
    aibaby::Rng rng;
    rng.seed(dna0.header().seed ^ 0x57Bu);

    // Silence first, then the rates in order, then the shuffled null. The
    // creature carries state across conditions — that is what a dynamic
    // synapse IS — so each block opens with silence long enough for the
    // longest recovery constant to have run several times over.
    for (uint32_t c = 0; c < kStpConditions; ++c) {
      const bool silent = (c == 0);
      const bool shuffled = (c == kStpConditions - 1);
      const double rate = shuffled ? 4.0 : (silent ? 0.0 : double(kStpRates[c - 1]));
      const double half_ms = silent ? 0.0 : 500.0 / rate;

      // The shuffled schedule: the same mean half-period and so the same total
      // sound, with each half drawn uniformly over +/-75% of it. Precomputed
      // rather than drawn inline so that both halves of a burst are known
      // before the block starts and the burst count is exact.
      std::vector<uint64_t> schedule;
      if (shuffled) {
        uint64_t total = 0;
        while (total < budget + 4000) {
          const double f = 0.25 + 1.5 * double(rng.uniform());
          const uint64_t len = uint64_t(half_ms * f) + 1;
          schedule.push_back(len);
          total += len;
        }
      }

      uint64_t aud_spikes = 0, cen_spikes = 0, bursts = 0, scored = 0;
      double gain_sum = 0.0;
      size_t slot = 0;
      uint64_t slot_left = schedule.empty() ? 0 : schedule[0];
      bool sounding = false;
      bool prev_sounding = false;

      // Only the first condition pays the long settle. After that the creature
      // is already regulated and each block needs no more than the time for
      // the previous stimulus to have drained.
      const uint64_t settle = (c == 0) ? kStpSettleTicks : 1500;
      for (uint64_t t = 0; t < settle + budget; ++t) {
        const bool measuring = t >= settle;
        const uint64_t bt = measuring ? t - settle : t;
        if (silent) {
          sounding = false;
        } else if (shuffled) {
          if (slot_left == 0) {
            slot = (slot + 1) % schedule.size();
            slot_left = schedule[slot];
            sounding = !sounding;
          }
          --slot_left;
        } else {
          sounding = std::fmod(double(bt), 2.0 * half_ms) < half_ms;
        }
        // A burst is counted on its rising edge, which is the event the
        // mechanism is about: Webb's BN1 responds to onsets and to nothing
        // else, so "per burst" has to mean "per onset" and not "per second".
        if (measuring && sounding && !prev_sounding) ++bursts;
        prev_sounding = sounding;

        voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                     pcm.data(), samples_per_tick);
        ear.tick(s.brain, pcm.data(), samples_per_tick);
        s.brain.step();
        if (!measuring || s.brain.asleep()) continue;
        aud_spikes += net.module(uint32_t(aud_m)).spikes;
        cen_spikes += net.module(uint32_t(cen_m)).spikes;
        gain_sum += double(net.stp_gain(uint32_t(aud_m), uint32_t(cen_m)));
        ++scored;
      }

      StpRow& row = rows[a][c];
      row.bursts = uint32_t(bursts);
      const double per = scored ? double(scored) : 1.0;
      row.aud_per_tick = double(aud_spikes) / per;
      row.cen_per_tick = double(cen_spikes) / per;
      row.transfer = aud_spikes ? double(cen_spikes) / double(aud_spikes) : 0.0;
      row.gain = scored ? gain_sum / double(scored) : 1.0;
    }
  }

  std::printf("\n    %-13s %-9s %-7s %-10s %-10s %-10s %-8s %-8s\n", "arm", "envelope",
              "bursts", "aud/tick", "cen/tick", "transfer", "vs off", "gain");
  for (uint32_t a = 0; a < kStpArmCount; ++a) {
    for (uint32_t c = 0; c < kStpConditions; ++c) {
      char envelope[16];
      if (c == 0) std::snprintf(envelope, sizeof(envelope), "silence");
      else if (c == kStpConditions - 1) std::snprintf(envelope, sizeof(envelope), "shuffled");
      else std::snprintf(envelope, sizeof(envelope), "%.0f Hz", double(kStpRates[c - 1]));
      const StpRow& r = rows[a][c];
      const double base = rows[0][c].transfer;
      std::printf("    %-13s %-9s %-7u %-10.2f %-10.2f %-10.4f %-8.3f %-8.3f\n",
                  c == 0 ? kStpArms[a].name : "", envelope, r.bursts, r.aud_per_tick,
                  r.cen_per_tick, r.transfer, base > 0.0 ? r.transfer / base : 0.0,
                  r.gain);
    }
    if (a + 1 < kStpArmCount) std::printf("\n");
  }

  // How strongly the delivered gain tracks the traffic the synapse saw. This is
  // the mechanism's defining property stated as a number: a dynamic synapse
  // delivers less the more it is used, so over the six conditions the gain and
  // the presynaptic rate have to move in opposite directions. A constant-weight
  // synapse would give 0 here, and so would a kernel that had computed the
  // depression from anything other than this synapse's own history.
  auto rate_gain_corr = [&](uint32_t arm) {
    double mx = 0.0, my = 0.0;
    for (uint32_t c = 0; c < kStpConditions; ++c) {
      mx += rows[arm][c].aud_per_tick;
      my += rows[arm][c].gain;
    }
    mx /= double(kStpConditions);
    my /= double(kStpConditions);
    double num = 0.0, dx = 0.0, dy = 0.0;
    for (uint32_t c = 0; c < kStpConditions; ++c) {
      const double p = rows[arm][c].aud_per_tick - mx, q = rows[arm][c].gain - my;
      num += p * q; dx += p * p; dy += q * q;
    }
    return (dx > 0.0 && dy > 0.0) ? num / std::sqrt(dx * dy) : 0.0;
  };

  double off_worst = 1.0, dep_worst = 0.0, fac_best = 2.0;
  for (uint32_t c = 0; c < kStpConditions; ++c) {
    if (std::fabs(rows[0][c].gain - 1.0) > std::fabs(off_worst - 1.0)) off_worst = rows[0][c].gain;
    if (rows[1][c].gain > dep_worst) dep_worst = rows[1][c].gain;
    if (rows[2][c].gain < fac_best) fac_best = rows[2][c].gain;
  }
  const double dep_corr = rate_gain_corr(1);

  std::printf("\n  gain is what the tract DELIVERED as a fraction of its genome weight.\n"
              "  transfer is central's spikes per auditory spike — a ratio, because the\n"
              "  ear's own response to an envelope is not flat and a count at central\n"
              "  would mostly be measuring that. `vs off` is the same ratio against the\n"
              "  off arm's row for the SAME envelope, and it is where an envelope filter\n"
              "  would show up: a gain change is flat down that column and a filter is\n"
              "  not.\n");
  std::printf("\n  off arm gain           %.4f   (must be 1.0000 — the control)\n"
              "  depressing, worst      %.3f   (must be under 0.950)\n"
              "  facilitating, best     %.3f   (must be over 1.050)\n"
              "  rate/gain corr, dep.   %+.3f   (must be under -0.500)\n",
              off_worst, dep_worst, fac_best, dep_corr);

  const bool control_ok = std::fabs(off_worst - 1.0) < 1e-6;
  const bool corners_ok = dep_worst < 0.95 && fac_best > 1.05;
  const bool reads_rate = dep_corr < -0.5;
  if (!control_ok) {
    std::printf("\n  FAIL — the off arm did not deliver its full weight. The patched blob\n"
                "  is not the creature it claims to be, so no row above means anything.\n");
  } else if (!corners_ok) {
    std::printf("\n  FAIL — the two corners did not do opposite things. A Tsodyks-Markram\n"
                "  synapse cannot depress and facilitate the same way, so the kernel is\n"
                "  wrong rather than the creature being surprising.\n");
  } else if (!reads_rate) {
    std::printf("\n  FAIL — the delivered gain does not track the traffic the synapse saw.\n"
                "  Depression that is blind to presynaptic rate is not depression.\n");
  } else {
    std::printf("\n  PASS — the mechanism runs, the two corners do opposite things, and\n"
                "  what each delivers tracks the traffic it carried.\n");
  }

  // What the table says about this creature, which is not the same question.
  std::printf("\n  WHAT IT BUYS HERE, read off the `vs off` column:\n"
              "  nothing that a volume knob would not. `vs off` is FLAT across the\n"
              "  envelopes — a filter would peak somewhere and it does not.\n"
              "\n  The reason is NOT that the ear is regulated flat. It is not: the\n"
              "  module goes %.2f spikes per tick in silence to %.2f under speech,\n"
              "  %+.0f%%, and `ipprobe` measures the same thing independently. An\n"
              "  earlier version of this probe settled for only 1500 ticks and read\n"
              "  silence on a just-hatched creature, which made that gap look like\n"
              "  3%% and supported an explanation that was wrong.\n"
              "\n  The real reason is that ONE dynamic synapse cannot be a bandpass.\n"
              "  Depression scales everything this synapse transmits by a single\n"
              "  number that follows its own mean rate, so it is a high-pass with no\n"
              "  upper corner: `gain` tracks the traffic faithfully and every spike\n"
              "  gets the same multiplier. Webb's bandpass is TWO stages — BN1\n"
              "  depressing, feeding BN2 facilitating — and the tuning lives in the\n"
              "  mismatch between their time constants, not in either one. This\n"
              "  genome can express that today: a relay module between auditory and\n"
              "  central, depressing on the way in and facilitating on the way out.\n"
              "  It has not been built.\n",
              rows[0][0].aud_per_tick, rows[0][1].aud_per_tick,
              rows[0][0].aud_per_tick > 0.0
                  ? 100.0 * (rows[0][1].aud_per_tick / rows[0][0].aud_per_tick - 1.0)
                  : 0.0);
  (void)verbose;
  return control_ok && corners_ok && reads_rate;
}


// --- burstprobe: does a burst code exist, does the tuft steer it, and does it
// --- carry the object? -----------------------------------------------------
//
// DNA v37 is the only structurally untried class left in the conditioning cap:
// every mechanism fenced there keeps R-STDP's *global scalar* third factor, and
// burst-dependent plasticity replaces it with a per-neuron one that the apical
// dendrite controls (Payeur, Guerguiev, Zenke, Richards & Naud 2021; the same
// per-neuron learning signal e-prop argues a spiking network needs).
//
// The mechanism is a chain of three links and any one of them can be dead while
// the other two look healthy, so this measures them separately. All three arms
// patch every field explicitly, including the off arm, for the reason gazeprobe
// learned the hard way.
//
//   1. **Is there a burst code?** `burst%` is burst spikes as a fraction of all
//      spikes at the larynx. At 0 the window is narrower than anything the
//      module does and the learning signal is identically zero; near 100 every
//      spike is a burst spike and the "burst rate" is the firing rate under
//      another name. Either rail means no row below is a measurement.
//
//   2. **Does the tuft steer it?** `burst|plat` against `burst|no plat`. This is
//      the link DNA v29 needed and never had: its plateau gate could only
//      *attenuate* learning, so a plateau that discriminated changed how much
//      was written and never what. A burst signal is signed, so a tuft that
//      raises burst probability flips the sign of the update rather than
//      scaling it — but only if it raises it, which is what this column asks.
//
//   3. **Does the signal carry the object?** `obj|burst` is a held-out
//      classification of cube against ball from the per-neuron burst
//      *deviation* — the exact quantity the weight update multiplies — against
//      `obj|spikes` from the same neurons' spike counts and a shuffled-label
//      null. This is the payoff. `eligprobe` measured the last third factor to
//      be object-blind at 0.93 correlation between conditions; if this one is
//      too, v37 dies the same death as the other five and the probe says so in
//      one number.
//
// The architecture under test is Payeur's, mapped onto this body plan:
// `vision->vocal` is moved onto the tuft and `central->vocal` is what learns.
// The seen object then arrives at the larynx's dendrites and signs the
// plasticity of the tract carrying the heard word, which is G3's wiring stated
// as a learning rule rather than as a delivery problem.
namespace {

struct BurstArm {
  const char* name;
  float burst_ms;      // 0 = no burst code
  float refrac_scale;  // what a plateau does to the refractory period
  bool apical;         // move vision->vocal onto the tuft
};

// 20 ms, and the interval table below is why. A pyramidal burst in the
// literature is 100-200 Hz, i.e. a 5-10 ms window — and this larynx fires at a
// few Hz, so only 0.1% of its spikes follow another within 5 ms and 1.8% within
// 10 ms. A code that scores 1 spike in 500 is a learning signal that is zero
// almost everywhere. 20 ms is the shortest window at which the code is live in
// THIS creature (8.2% without a tuft, 13.0% with one), and the honest way to
// report it is to print the whole curve and let the window be read off it,
// which is what the second table does.
constexpr BurstArm kBurstArms[] = {
    {"off",          0.0f, 1.0f, false},
    {"burst",       20.0f, 1.0f, false},
    {"burst+tuft",  20.0f, 0.3f, true},
};
constexpr uint32_t kBurstArmCount = sizeof(kBurstArms) / sizeof(kBurstArms[0]);

// A plateau has to be reachable before it can steer anything, and the shipped
// genome has no compartment at all — every module ships at threshold 0. These
// are apicalprobe's working values.
// Candidate burst windows, reported for every arm from the intervals the probe
// already tracks. A pyramidal burst is two to four spikes at 100-200 Hz, so 5
// and 10 ms are what "burst" means in the literature; the wider ones are here
// because a module firing at a few Hz may simply not do that, and the table
// should say so rather than leave the reader to infer it from one zero.
constexpr float kBurstWindows[] = {5.0f, 10.0f, 20.0f, 40.0f, 80.0f};
constexpr uint32_t kBurstWindowCount = sizeof(kBurstWindows) / sizeof(kBurstWindows[0]);

constexpr float kBurstApicalThreshold = 0.35f;
constexpr float kBurstApicalGain = 1.0f;

struct BurstRow {
  double burst_pct = 0.0;
  double burst_in_plat = 0.0;
  double burst_out_plat = 0.0;
  double plateau_pct = 0.0;
  double obj_burst = 0.0;
  double obj_spikes = 0.0;
  double obj_plateau = 0.0;
  double shuffled = 0.0;
  double isi_pct[kBurstWindowCount] = {};
  size_t trials = 0;
};

}  // namespace

bool run_burstprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  aibaby::Dna dna0;
  if (dna0.load(dna_blob.data(), dna_blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t voc_m = dna0.module_with_role(aibaby::ModuleRole::kVocal);
  const int32_t cen_m = dna0.module_with_role(aibaby::ModuleRole::kAssociation);
  const int32_t vis_m = dna0.module_with_role(aibaby::ModuleRole::kVision);
  if (voc_m < 0 || cen_m < 0 || vis_m < 0) {
    std::printf("  this genome is missing a vocal, association or vision module\n");
    return false;
  }
  int32_t teach = -1, tuft = -1;
  for (uint32_t i = 0; i < dna0.header().projection_count; ++i) {
    const aibaby::DnaProjection& p = dna0.projection(i);
    if (int32_t(p.dst) != voc_m) continue;
    if (int32_t(p.src) == cen_m) teach = int32_t(i);
    if (int32_t(p.src) == vis_m) tuft = int32_t(i);
  }
  if (teach < 0 || tuft < 0) {
    std::printf("  this genome has no central->vocal or no vision->vocal tract\n");
    return false;
  }

  const aibaby::DnaVision& vcfg = dna0.header().vision;
  const aibaby::DnaAudio& acfg = dna0.header().audio;
  const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / dna0.header().sim.dt_ms + 0.5f);
  const uint32_t n_trials = uint32_t(ticks / (kBurstArmCount * kM3ProbeTicks));

  instrument("burstprobe", dna0.header().seed ^ 0xB025u, n_trials, "trials per arm");
  std::printf("  architecture      vision->vocal on the TUFT, central->vocal LEARNS\n"
              "  burst window      %.0f ms; a plateau scales the refractory period\n"
              "                    by %.2f in the third arm\n",
              double(kBurstArms[1].burst_ms), double(kBurstArms[2].refrac_scale));

  const size_t mod_base = sizeof(aibaby::DnaHeader);
  const size_t proj_base = mod_base + sizeof(aibaby::DnaModule) * dna0.module_count();
  auto put_module = [&](std::vector<uint8_t>& v, int32_t m, size_t off, float value) {
    std::memcpy(v.data() + mod_base + sizeof(aibaby::DnaModule) * size_t(m) + off,
                &value, sizeof(value));
  };

  BurstRow rows[kBurstArmCount];

  for (uint32_t a = 0; a < kBurstArmCount; ++a) {
    const BurstArm& arm = kBurstArms[a];
    std::vector<uint8_t> variant = dna_blob;
    {
      put_module(variant, voc_m, offsetof(aibaby::DnaModule, burst_ms), arm.burst_ms);
      put_module(variant, voc_m, offsetof(aibaby::DnaModule, burst_refrac_scale),
                 arm.refrac_scale);
      const float thr = arm.apical ? kBurstApicalThreshold : 0.0f;
      const float gain = arm.apical ? kBurstApicalGain : 0.0f;
      put_module(variant, voc_m, offsetof(aibaby::DnaModule, apical_threshold), thr);
      put_module(variant, voc_m, offsetof(aibaby::DnaModule, apical_gain), gain);
      const uint32_t ap = arm.apical ? 1u : 0u;
      std::memcpy(variant.data() + proj_base +
                      sizeof(aibaby::DnaProjection) * size_t(tuft) +
                      offsetof(aibaby::DnaProjection, apical),
                  &ap, sizeof(ap));
      // The baseline time constant, and the learning rate on the taught tract.
      // Set on every arm including `off`, where the module's burst_ms of 0
      // makes it inert — the genome loader refuses `burst_learn` without a
      // burst code, so the off arm has to zero the rate as well.
      const float tau = 2000.0f;
      std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, stdp) +
                      offsetof(aibaby::DnaStdp, burst_baseline_tau_ms),
                  &tau, sizeof(tau));
      const float learn = arm.burst_ms > 0.0f ? 1e-3f : 0.0f;
      std::memcpy(variant.data() + proj_base +
                      sizeof(aibaby::DnaProjection) * size_t(teach) +
                      offsetof(aibaby::DnaProjection, burst_learn),
                  &learn, sizeof(learn));
    }

    Session s;
    if (!s.init(variant, error)) {
      std::printf("  arm %s failed to hatch: %s\n", arm.name, error.c_str());
      return false;
    }
    const aibaby::Network& net = s.brain.network();
    Retina retina;
    Ear ear;
    if (!retina.configure(vcfg, error) || !ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource voice(acfg.sample_rate);
    SceneSource scene(vcfg.frame_size, dna0.header().seed);
    std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
    std::vector<float> pcm(samples_per_tick);

    aibaby::Rng rng;
    rng.seed(dna0.header().seed ^ 0xB025u);

    const aibaby::ModuleState& vm = net.module(uint32_t(voc_m));
    const uint32_t width = vm.count;
    // The probe decides for itself what a burst is, from the spike train it can
    // see, rather than asking the kernel. Two reasons and both are about
    // honesty: a probe that reads the kernel's own burst trace cannot tell a
    // correct implementation from a consistent one, and that trace is a 50 ms
    // one-pole — "has bursted recently", not "this spike was part of a burst".
    // Reading it as the latter is what made the first run of this probe report
    // 80.9%.
    const uint64_t burst_win =
        uint64_t(double(kBurstArms[1].burst_ms) / double(dna0.header().sim.dt_ms) + 0.5);
    std::vector<uint64_t> last_seen(width, 0);
    std::vector<bool> ever(width, false);
    // The interval distribution at the larynx, so the table can say which
    // window WOULD give a live burst code rather than only whether the
    // configured one did. Free: the probe is already tracking every interval.
    double isi_at[kBurstWindowCount] = {};
    std::vector<std::vector<double>> feat_burst, feat_spike, feat_plat;
    std::vector<int> labels;
    double spikes_all = 0.0, spikes_burst = 0.0;
    double plat_spikes = 0.0, plat_burst = 0.0, plat_ticks = 0.0, all_ticks = 0.0;

    for (uint32_t trial = 0; trial < n_trials; ++trial) {
      const int label = int(trial % 2);
      const Toy toy = m3_toy(rng, label);
      std::vector<double> dev(width, 0.0), spk(width, 0.0), plat(width, 0.0);
      bool slept = false;
      uint64_t scored = 0;
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
        ++scored;
        for (uint32_t k = 0; k < width; ++k) {
          const uint32_t i = vm.begin + k;
          // The quantity the weight update multiplies, sampled where it is
          // read: burst rate minus this neuron's own running baseline.
          dev[k] += double(net.burst_rate(i)) - double(net.burst_base(i));
          if (net.in_plateau(i)) { plat_ticks += 1.0; plat[k] += 1.0; }
          ++all_ticks;
        }
        for (uint32_t k = 0; k < net.spike_count(); ++k) {
          const uint32_t i = net.spikes()[k];
          if (i < vm.begin || i >= vm.begin + width) continue;
          const uint32_t idx = i - vm.begin;
          spk[idx] += 1.0;
          spikes_all += 1.0;
          const uint64_t gap = t - last_seen[idx];
          if (ever[idx]) {
            for (uint32_t wi = 0; wi < kBurstWindowCount; ++wi) {
              const uint64_t w = uint64_t(double(kBurstWindows[wi]) /
                                              double(dna0.header().sim.dt_ms) + 0.5);
              if (gap <= w) isi_at[wi] += 1.0;
            }
          }
          const bool bursty = ever[idx] && gap <= burst_win && arm.burst_ms > 0.0f;
          last_seen[idx] = t;
          ever[idx] = true;
          if (bursty) spikes_burst += 1.0;
          if (net.in_plateau(i)) {
            plat_spikes += 1.0;
            if (bursty) plat_burst += 1.0;
          }
        }
      }
      if (slept || scored == 0) continue;
      for (uint32_t k = 0; k < width; ++k) dev[k] /= double(scored);
      feat_burst.push_back(dev);
      feat_spike.push_back(spk);
      feat_plat.push_back(plat);
      labels.push_back(label);
    }

    BurstRow& row = rows[a];
    row.trials = labels.size();
    row.burst_pct = spikes_all > 0.0 ? 100.0 * spikes_burst / spikes_all : 0.0;
    row.burst_in_plat = plat_spikes > 0.0 ? 100.0 * plat_burst / plat_spikes : 0.0;
    row.burst_out_plat = (spikes_all - plat_spikes) > 0.0
                             ? 100.0 * (spikes_burst - plat_burst) / (spikes_all - plat_spikes)
                             : 0.0;
    row.plateau_pct = all_ticks > 0.0 ? 100.0 * plat_ticks / all_ticks : 0.0;
    for (uint32_t wi = 0; wi < kBurstWindowCount; ++wi) {
      row.isi_pct[wi] = spikes_all > 0.0 ? 100.0 * isi_at[wi] / spikes_all : 0.0;
    }
    if (labels.size() >= 12) {
      std::vector<std::vector<double>> xb, xs;
      std::vector<int> yb, ys;
      size_t tb = 0, ts = 0;
      interleave_pairs(feat_burst, labels, xb, yb, tb);
      interleave_pairs(feat_spike, labels, xs, ys, ts);
      row.obj_burst = holdout_accuracy(xb, yb, tb);
      row.obj_spikes = holdout_accuracy(xs, ys, ts);
      // THE control this probe needs. DNA v29 already had a plateau that
      // discriminated the object — apicalprobe measures it — and it still
      // could not teach, because a gate can only attenuate. So "the burst
      // signal carries the object" is only news if the burst carries MORE than
      // the plateau it is derived from. If these two columns agree, v37 has
      // added a sign to something v29 already had and nothing else.
      std::vector<std::vector<double>> xpl;
      std::vector<int> ypl;
      size_t tpl = 0;
      interleave_pairs(feat_plat, labels, xpl, ypl, tpl);
      row.obj_plateau = holdout_accuracy(xpl, ypl, tpl);
      // 32 permutations, not one. A single shuffle is one draw from the null
      // distribution rather than an estimate of it, and at ~50 test trials one
      // draw wanders far enough to be read as a finding in either direction —
      // the first run of this probe reported a null of 0.380 against a chance
      // of 0.500 that way. This is the same correction the audibility ruler
      // needed.
      double null_sum = 0.0;
      for (uint32_t perm = 0; perm < 32; ++perm) {
        std::vector<int> shuffled = yb;
        for (size_t i = shuffled.size(); i > 1; --i) {
          std::swap(shuffled[i - 1], shuffled[rng.next() % i]);
        }
        null_sum += holdout_accuracy(xb, shuffled, tb);
      }
      row.shuffled = null_sum / 32.0;
    }
  }

  std::printf("\n    %-12s %-7s %-7s %-8s %-9s %-8s %-10s %-10s %-10s %-8s\n", "arm",
              "trials", "burst%", "plat%", "burst|plat", "burst|no", "obj|burst",
              "obj|plat", "obj|spikes", "shuffled");
  for (uint32_t a = 0; a < kBurstArmCount; ++a) {
    const BurstRow& r = rows[a];
    std::printf("    %-12s %-7zu %-7.1f %-8.1f %-9.1f %-8.1f %-10.3f %-10.3f %-10.3f %-8.3f\n",
                kBurstArms[a].name, r.trials, r.burst_pct, r.plateau_pct, r.burst_in_plat,
                r.burst_out_plat, r.obj_burst, r.obj_plateau, r.obj_spikes, r.shuffled);
  }

  std::printf("\n  what fraction of spikes at the larynx follow another within:\n    %-12s",
              "arm");
  for (uint32_t wi = 0; wi < kBurstWindowCount; ++wi) {
    std::printf("%-8.0f", double(kBurstWindows[wi]));
  }
  std::printf("ms\n");
  for (uint32_t a = 0; a < kBurstArmCount; ++a) {
    std::printf("    %-12s", kBurstArms[a].name);
    for (uint32_t wi = 0; wi < kBurstWindowCount; ++wi) {
      std::printf("%-8.1f", rows[a].isi_pct[wi]);
    }
    std::printf("\n");
  }

  const BurstRow& off = rows[0];
  const BurstRow& plain = rows[1];
  const BurstRow& tuft_arm = rows[2];

  std::printf("\n  burst%% is burst spikes over all spikes at the larynx. burst|plat and\n"
              "  burst|no split that by whether the neuron's tuft was in a plateau, and\n"
              "  their difference is whether feedback can steer the sign of learning.\n"
              "  obj|burst classifies cube against ball from the per-neuron burst\n"
              "  DEVIATION — the exact quantity the weight update multiplies.\n");

  const bool off_silent = off.burst_pct < 0.001;
  const bool code_live = plain.burst_pct > 1.0 && plain.burst_pct < 95.0;
  const bool tuft_steers = tuft_arm.burst_in_plat > tuft_arm.burst_out_plat * 1.05 &&
                           tuft_arm.plateau_pct > 0.5 && tuft_arm.plateau_pct < 95.0;

  std::printf("\n  off arm burst%%         %.3f   (must be 0 — the control)\n"
              "  burst code             %.1f%%    (must be inside 1-95%%)\n"
              "  plateau occupancy      %.1f%%    (must be inside 0.5-95%%)\n"
              "  burst|plat vs |no      %.1f%% vs %.1f%%   (the tuft must raise it)\n",
              off.burst_pct, plain.burst_pct, tuft_arm.plateau_pct,
              tuft_arm.burst_in_plat, tuft_arm.burst_out_plat);

  std::printf("  burst vs plateau       %.3f vs %.3f%s\n", tuft_arm.obj_burst,
              tuft_arm.obj_plateau,
              tuft_arm.obj_burst >= tuft_arm.obj_plateau
                  ? "   (the burst adds specificity)"
                  : "   <- the burst LOSES specificity vs the plateau it comes from");

  if (!off_silent) {
    std::printf("\n  FAIL — the off arm has a burst code. The patched blob is not the\n"
                "  creature it claims to be, so no row above means anything.\n");
  } else if (!code_live) {
    std::printf("\n  FAIL — the burst code is at a rail. At 0 the learning signal is\n"
                "  identically zero; near 100 it is the firing rate under another name.\n"
                "  Either way v37 is present and not running, and the burst window is\n"
                "  the thing to move.\n");
  } else if (!tuft_steers) {
    std::printf("\n  The burst code runs, and the TUFT DOES NOT STEER IT. That is the\n"
                "  same shape as DNA v29's failure one level down: without a plateau\n"
                "  that changes burst probability there is no per-neuron learning\n"
                "  signal, only a second global one. Reported, not fatal — the link\n"
                "  is measured and the answer is no.\n");
  } else {
    std::printf("\n  PASS — the burst code runs and the tuft steers it.\n");
    std::printf("\n  WHAT IT BUYS, and the plateau column is the honest reading of it:\n"
                "  the per-neuron burst deviation at the larynx carries cube-versus-ball\n"
                "  at %.3f against a 32-permutation null of %.3f. That is a third factor\n"
                "  that DISCRIMINATES, which no previous one here did — `eligprobe` reads\n"
                "  the trace R-STDP multiplies as object-blind, +0.93 correlated between\n"
                "  the two conditions.\n"
                "\n  But the plateau it is derived from reads %.3f, and the burst reads\n"
                "  %.3f. Turning a plateau into a burst rate is a nonlinearity applied to\n"
                "  a signal that was already there, and it %s. DNA v29 had the %.3f\n"
                "  signal and could not teach with it, because a gate can only attenuate;\n"
                "  v37's claim is that a SIGN is worth more than the specificity it\n"
                "  costs. That claim is not settled by this probe — it needs `m3`.\n",
                tuft_arm.obj_burst, tuft_arm.shuffled, tuft_arm.obj_plateau,
                tuft_arm.obj_burst,
                tuft_arm.obj_burst >= tuft_arm.obj_plateau ? "gains" : "loses",
                tuft_arm.obj_plateau);
  }
  (void)verbose;
  return off_silent && code_live;
}


// --- pruneprobe: does competition remove the losers, or just remove? --------
//
// DNA v38 lets a synapse be pruned for being weak *relative to its own target's
// other afferents*, with no idle test. The exuberance post-mortem asked for
// exactly that and declined to build it, so the question this has to answer is
// not "did anything get removed" — a rule that removed a random half would also
// answer yes — but **was the removal selective**.
//
// The measurement is one number and it has a null that is not a guess.
// Pruning k of a target neuron's afferents at random leaves the mean |w| over
// the survivors unchanged in expectation, so **0% is the exact null** for
// "surviving mean |w| after the pass, against before". Competition that works
// raises it; competition that is really a decimation does not.
//
// Three things this does not wait for. It drives the creature awake and then
// calls `consolidate()` directly rather than waiting for a sleep bout at ~1.04M
// ticks — that function IS what sleep calls, and testing the rule is a
// different job from testing that fatigue reaches it, which `sleep` and `g4`
// already do. It runs on the shipped genome rather than an exuberant one,
// because the rule has to be safe on the creature that exists before it is
// interesting on one that does not. And it checks the floor as well as the
// ceiling: a mechanism that strips a neuron to nothing is worse than one that
// does nothing.
namespace {

struct PruneArm {
  const char* name;
  float compete;
};

constexpr PruneArm kPruneArms[] = {
    {"off",      0.00f},
    {"compete",  0.50f},
};
constexpr uint32_t kPruneArmCount = sizeof(kPruneArms) / sizeof(kPruneArms[0]);
constexpr uint32_t kPrunePasses = 5;

}  // namespace

bool run_pruneprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  aibaby::Dna dna0;
  if (dna0.load(dna_blob.data(), dna_blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const uint64_t drive = ticks / kPruneArmCount;
  instrument("pruneprobe", dna0.header().seed ^ 0x9700u, drive, "ticks driven per arm");
  std::printf("  %u consolidation passes per arm, called directly rather than waited\n"
              "  for; competition removes afferents under %.2f of their target's mean\n",
              kPrunePasses, double(kPruneArms[1].compete));

  double mean_before[kPruneArmCount] = {}, mean_after[kPruneArmCount] = {};
  uint32_t pruned[kPruneArmCount] = {}, competed[kPruneArmCount] = {};
  uint32_t min_in[kPruneArmCount] = {}, orphans[kPruneArmCount] = {};
  uint32_t live_after[kPruneArmCount] = {};

  for (uint32_t a = 0; a < kPruneArmCount; ++a) {
    std::vector<uint8_t> variant = dna_blob;
    {
      const float c = kPruneArms[a].compete;
      std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, consolidate) +
                      offsetof(aibaby::DnaConsolidate, prune_compete),
                  &c, sizeof(c));
      // Every arm sets it explicitly, the off arm included, so that the day
      // this ships on in dna/default.toml the control does not silently stop
      // being one.
    }
    Session s;
    if (!s.init(variant, error)) {
      std::printf("  arm %s failed to hatch: %s\n", kPruneArms[a].name, error.c_str());
      return false;
    }
    aibaby::Network& net = s.brain.network();
    const aibaby::DnaVision& vcfg = dna0.header().vision;
    const aibaby::DnaAudio& acfg = dna0.header().audio;
    Retina retina;
    Ear ear;
    if (!retina.configure(vcfg, error) || !ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource voice(acfg.sample_rate);
    SceneSource scene(vcfg.frame_size, dna0.header().seed);
    std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
    std::vector<float> pcm(acfg.sample_rate / 1000);
    const uint64_t frame_ticks =
        uint64_t(1000.0f / vcfg.frame_hz / dna0.header().sim.dt_ms + 0.5f);
    aibaby::Rng rng;
    rng.seed(dna0.header().seed ^ 0x9700u);
    const Toy toy = m3_toy(rng, 0);

    // Ordinary waking life, so the weights spread out the way experience
    // spreads them. A brain pruned straight from birth would be scoring the
    // genome's own weight jitter, which is a distribution nothing selected.
    for (uint64_t t = 0; t < drive; ++t) {
      if (t % frame_ticks == 0) {
        scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }
      voice.render(0.0f, 0.0f, 0.0f, 0.0f, pcm.data(), pcm.size());
      ear.tick(s.brain, pcm.data(), pcm.size());
      s.brain.step();
    }

    auto survey = [&](double* mean, uint32_t* min_in_out, uint32_t* orphan_out) {
      double sum = 0.0;
      uint32_t counted = 0, lowest = 0xFFFFFFFFu, none = 0;
      for (uint32_t m = 0; m < net.module_count(); ++m) {
        const aibaby::ModuleState& ms = net.module(m);
        for (uint32_t k = 0; k < ms.count; ++k) {
          const uint32_t i = ms.begin + k;
          const uint32_t in_n = net.in_degree(i);
          if (in_n == 0) { ++none; continue; }
          if (in_n < lowest) lowest = in_n;
          sum += double(net.mean_in_weight_of(i)) * double(in_n);
          counted += in_n;
        }
      }
      *mean = counted ? sum / double(counted) : 0.0;
      *min_in_out = lowest == 0xFFFFFFFFu ? 0 : lowest;
      *orphan_out = none;
    };

    uint32_t dummy_min = 0, dummy_orph = 0;
    survey(&mean_before[a], &dummy_min, &dummy_orph);
    const uint32_t pruned0 = net.structural().synapses_pruned;
    for (uint32_t pass = 0; pass < kPrunePasses; ++pass) {
      net.consolidate();
      competed[a] += net.competed_out();
    }
    pruned[a] = net.structural().synapses_pruned - pruned0;
    live_after[a] = net.telemetry().live_synapses;
    survey(&mean_after[a], &min_in[a], &orphans[a]);
  }

  const double change_off =
      mean_before[0] > 0.0 ? 100.0 * (mean_after[0] / mean_before[0] - 1.0) : 0.0;
  const double change_on =
      mean_before[1] > 0.0 ? 100.0 * (mean_after[1] / mean_before[1] - 1.0) : 0.0;

  std::printf("\n    %-9s %-8s %-9s %-8s %-11s %-11s %-9s %-7s %-8s\n", "arm", "pruned",
              "competed", "% of", "mean|w| in", "mean|w| out", "change", "min in",
              "orphans");
  for (uint32_t a = 0; a < kPruneArmCount; ++a) {
    const double change =
        mean_before[a] > 0.0 ? 100.0 * (mean_after[a] / mean_before[a] - 1.0) : 0.0;
    const double frac = (live_after[a] + pruned[a]) > 0
                            ? 100.0 * double(pruned[a]) / double(live_after[a] + pruned[a])
                            : 0.0;
    std::printf("    %-9s %-8u %-9u %-8.1f %-11.5f %-11.5f %+-9.2f %-7u %-8u\n",
                kPruneArms[a].name, pruned[a], competed[a], frac, mean_before[a],
                mean_after[a], change, min_in[a], orphans[a]);
  }


  std::printf("\n  `change` is the mean |w| over surviving afferents after the passes\n"
              "  against before, and the OFF ARM is its null — not 0%%. Removing\n"
              "  synapses at random would leave the mean where it was, but a\n"
              "  consolidation pass also downscales every weight (§3.6), so the off\n"
              "  arm's %+.2f%% is what a pass costs with no competition in it. The\n"
              "  difference between the two columns is selection and the rest is sleep.\n",
              change_off);
  std::printf("\n  off arm competed       %u   (must be 0 — the control)\n"
              "  competitive removals   %u   (must be > 0, or nothing was tested)\n"
              "  selectivity            %+.2f%% against the off arm's %+.2f%%\n"
              "  orphaned neurons       %u against the off arm's %u   (must not rise:\n"
              "                             a stripped cell deletes the tract it was\n"
              "                             meant to sharpen. Six are already there at\n"
              "                             birth and are not this rule's doing)\n",
              competed[0], competed[1], change_on, change_off, orphans[1], orphans[0]);

  const bool control_ok = competed[0] == 0;
  const bool ran = competed[1] > 0;
  const bool selective = change_on > change_off + 1.0;
  // Against the off arm and not against zero. The shipped genome hatches with
  // six neurons that nothing projects onto, and a test that blamed those on
  // competition would fail for a reason that has nothing to do with it — the
  // same mistake as reading `change` against 0%.
  const bool safe = orphans[1] <= orphans[0] && min_in[1] >= min_in[0];
  if (!control_ok) {
    std::printf("\n  FAIL — the off arm competed. prune_compete 0 is not off.\n");
  } else if (!ran) {
    std::printf("\n  FAIL — competition removed nothing, so no column above is a\n"
                "  measurement of it. Either every afferent is above half its\n"
                "  target's mean — check the weight spread — or the bar never fired.\n");
  } else if (!selective) {
    std::printf("\n  FAIL — removal is not selective: the surviving mean did not rise\n"
                "  against a null of 0%%. That is decimation with a comparison in\n"
                "  front of it, which is worse than the absolute floor it replaces.\n");
  } else if (!safe) {
    std::printf("\n  FAIL — competition cost connectivity the off arm kept: %u orphans\n"
                "  against %u, minimum in-degree %u against %u. A rule that strips a\n"
                "  cell deletes the tract it was meant to sharpen.\n",
                orphans[1], orphans[0], min_in[1], min_in[0]);
  } else {
    std::printf("\n  PASS — competition removes synapses the absolute floor did not,\n"
                "  the survivors are stronger than the population it selected from,\n"
                "  and no neuron was stripped.\n");
  }
  (void)verbose;
  return control_ok && ran && selective && safe;
}


// --- tauprobe: does a per-module eligibility time constant do anything? -----
//
// DNA v39 gives each module its own tau_elig, after e-prop's prediction that the
// trace's timescale tracks the postsynaptic neuron's own history-dependence.
// The mechanism is four lines and the only way it can be wrong is silently: a
// scale that is read on the wrong side of the synapse, or folded into a decay
// that was already computed, would leave every number in the brain unchanged
// and the field would look enabled forever.
//
// So this measures the one thing that must be true if it runs. With no weight
// change the trace is a leaky accumulator driven by spike timing:
//
//     e_ss = input / (1 - exp(-interval / tau))  ->  approximately input x tau
//
// for a tau well above the plasticity interval. Scaling tau must scale the
// steady-state |e| onto that module in proportion, and must leave every OTHER
// module alone — which is the half that catches a scale applied globally by
// mistake.
namespace {
constexpr float kTauScales[] = {1.0f, 2.0f, 4.0f, 8.0f};
constexpr uint32_t kTauScaleCount = sizeof(kTauScales) / sizeof(kTauScales[0]);
}  // namespace

bool run_tauprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  aibaby::Dna dna0;
  if (dna0.load(dna_blob.data(), dna_blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t cen_m = dna0.module_with_role(aibaby::ModuleRole::kAssociation);
  const int32_t voc_m = dna0.module_with_role(aibaby::ModuleRole::kVocal);
  if (cen_m < 0 || voc_m < 0) {
    std::printf("  this genome has no association or vocal module\n");
    return false;
  }
  const uint64_t per_arm = ticks / kTauScaleCount;
  instrument("tauprobe", dna0.header().seed ^ 0x7A0u, per_arm, "ticks per arm");
  std::printf("  scaling %s's tau_elig; %s is the untouched control module\n"
              "  global tau_elig    %.0f ms, cashed in every %u ticks\n",
              dna0.module(uint32_t(cen_m)).name, dna0.module(uint32_t(voc_m)).name,
              double(dna0.header().stdp.tau_elig_ms),
              dna0.header().sim.plasticity_interval_ticks);

  double e_scaled[kTauScaleCount] = {}, e_control[kTauScaleCount] = {};

  for (uint32_t a = 0; a < kTauScaleCount; ++a) {
    std::vector<uint8_t> variant = dna_blob;
    const float sc = kTauScales[a];
    std::memcpy(variant.data() + sizeof(aibaby::DnaHeader) +
                    sizeof(aibaby::DnaModule) * size_t(cen_m) +
                    offsetof(aibaby::DnaModule, elig_tau_scale),
                &sc, sizeof(sc));
    Session s;
    if (!s.init(variant, error)) {
      std::printf("  arm %.0fx failed to hatch: %s\n", double(sc), error.c_str());
      return false;
    }
    const aibaby::Network& net = s.brain.network();
    const aibaby::DnaVision& vcfg = dna0.header().vision;
    const aibaby::DnaAudio& acfg = dna0.header().audio;
    Retina retina;
    Ear ear;
    if (!retina.configure(vcfg, error) || !ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource voice(acfg.sample_rate);
    SceneSource scene(vcfg.frame_size, dna0.header().seed);
    std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
    std::vector<float> pcm(acfg.sample_rate / 1000);
    const uint64_t frame_ticks =
        uint64_t(1000.0f / vcfg.frame_hz / dna0.header().sim.dt_ms + 0.5f);
    aibaby::Rng rng;
    rng.seed(dna0.header().seed ^ 0x7A0u);
    const Toy toy = m3_toy(rng, 0);

    double sum_s = 0.0, sum_c = 0.0;
    uint64_t samples = 0;
    for (uint64_t t = 0; t < per_arm; ++t) {
      if (t % frame_ticks == 0) {
        scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }
      voice.render(0.0f, 0.0f, 0.0f, 0.0f, pcm.data(), pcm.size());
      ear.tick(s.brain, pcm.data(), pcm.size());
      s.brain.step();
      // Sampled after the trace has filled: a leaky accumulator with a 8x tau
      // takes 8x as long to reach its steady state, and reading all four arms
      // at the same early tick would measure the fill and call it the level.
      if (t < per_arm / 2 || t % 500 != 0) continue;
      sum_s += double(net.mean_eligibility(uint32_t(cen_m)));
      sum_c += double(net.mean_eligibility(uint32_t(voc_m)));
      ++samples;
    }
    e_scaled[a] = samples ? sum_s / double(samples) : 0.0;
    e_control[a] = samples ? sum_c / double(samples) : 0.0;
  }

  std::printf("\n    %-10s %-14s %-10s %-14s %-10s\n", "scale", "mean|e| central",
              "vs 1x", "mean|e| vocal", "vs 1x");
  for (uint32_t a = 0; a < kTauScaleCount; ++a) {
    std::printf("    %-10.0f %-14.6f %-10.2f %-14.6f %-10.2f\n", double(kTauScales[a]),
                e_scaled[a], e_scaled[0] > 0.0 ? e_scaled[a] / e_scaled[0] : 0.0,
                e_control[a], e_control[0] > 0.0 ? e_control[a] / e_control[0] : 0.0);
  }

  const double ratio = e_scaled[0] > 0.0 ? e_scaled[kTauScaleCount - 1] / e_scaled[0] : 0.0;
  const double leak =
      e_control[0] > 0.0 ? e_control[kTauScaleCount - 1] / e_control[0] : 0.0;
  bool monotone = true;
  for (uint32_t a = 1; a < kTauScaleCount; ++a) {
    if (e_scaled[a] <= e_scaled[a - 1]) monotone = false;
  }

  std::printf("\n  a leaky accumulator's steady state is proportional to its time\n"
              "  constant, so scaling tau_elig 8x must raise the trace on that module\n"
              "  and must leave the others where they were.\n");
  std::printf("\n  central 8x / 1x        %.2f   (must be over 2.0 and rise at every step)\n"
              "  vocal   8x / 1x        %.2f   (must stay inside 0.8-1.25: the scale is\n"
              "                         per module, and a global one would move this too)\n",
              ratio, leak);

  const bool scaled = ratio > 2.0 && monotone;
  const bool contained = leak > 0.8 && leak < 1.25;
  if (!scaled) {
    std::printf("\n  FAIL — scaling tau_elig did not raise the trace%s. The field is\n"
                "  present and inert, which is the failure mode it was built to avoid.\n",
                monotone ? "" : " monotonically");
  } else if (!contained) {
    std::printf("\n  FAIL — the untouched module moved too. The scale is being applied\n"
                "  more widely than one module, most likely on the wrong side of the\n"
                "  synapse: v39 is read through syn_target_, not from the loop's own\n"
                "  module.\n");
  } else {
    std::printf("\n  PASS — the trace scales with the module's own time constant and\n"
                "  the other modules are untouched. Whether a longer window BUYS\n"
                "  anything is a separate question, and the answer on record is no:\n"
                "  eligibility distribution is one of the five refuted conditioning\n"
                "  classes.\n");
  }
  (void)verbose;
  return scaled && contained;
}

// --- ipprobe: is a rate code available at the ear, and at what price? -------
//
// `stpprobe` measured that `auditory` emits 1.30 spikes per tick in silence and
// 1.33 during speech — 2.3% — because §3.1's intrinsic plasticity holds the
// module at a rate setpoint. That blinds every mechanism in this creature that
// reads a population rate: v12's normalisation, v21-v24's pooling, `ffi`, the
// critic's bins, and v36's dynamic synapses. It is worth a standing test rather
// than a note, because it is a property of one genome field and the day someone
// changes that field the whole class of mechanisms changes with it.
//
// Two columns, and the second is the price. `gap` is what a rate-reading
// mechanism would have to work with. `audio ratio` is the experiment's own
// measure of whether sound reaches B2 at all, and it is here because relaxing a
// regulator is exactly the kind of change that buys a number and breaks a
// milestone — the `[normalisation]` note records separability collapsing to
// chance when central's copy of this knob reaches zero.
bool run_ipprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  aibaby::Dna dna0;
  if (dna0.load(dna_blob.data(), dna_blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t aud_m = dna0.module_with_role(aibaby::ModuleRole::kAuditory);
  if (aud_m < 0) {
    std::printf("  this genome has no auditory module\n");
    return false;
  }
  static const float kIpScales[] = {1.0f, 0.25f, 0.10f, 0.0f};
  const uint32_t n_arms = sizeof(kIpScales) / sizeof(kIpScales[0]);
  const uint64_t per_arm = ticks / n_arms;
  const uint64_t half = per_arm / 2;

  instrument("ipprobe", dna0.header().seed ^ 0x19Fu, per_arm, "ticks per arm");
  std::printf("  sweeping %s's ip_wake_scale; shipped value %.2f\n",
              dna0.module(uint32_t(aud_m)).name,
              double(dna0.module(uint32_t(aud_m)).ip_wake_scale));

  std::printf("\n    %-8s %-11s %-11s %-9s\n", "ip", "silence", "speech", "gap");
  double gap[8] = {};
  for (uint32_t a = 0; a < n_arms; ++a) {
    std::vector<uint8_t> variant = dna_blob;
    const float sc = kIpScales[a];
    std::memcpy(variant.data() + sizeof(aibaby::DnaHeader) +
                    sizeof(aibaby::DnaModule) * size_t(aud_m) +
                    offsetof(aibaby::DnaModule, ip_wake_scale),
                &sc, sizeof(sc));
    Session s;
    if (!s.init(variant, error)) {
      std::printf("  arm %.2f failed to hatch: %s\n", double(sc), error.c_str());
      return false;
    }
    const aibaby::Network& net = s.brain.network();
    const aibaby::DnaAudio& acfg = dna0.header().audio;
    Ear ear;
    if (!ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource voice(acfg.sample_rate);
    std::vector<float> pcm(acfg.sample_rate / 1000);
    const Word& w = kWords[0];
    double quiet = 0.0, loud = 0.0;
    uint64_t nq = 0, nl = 0;
    for (uint64_t t = 0; t < per_arm; ++t) {
      // Silence first and speech second, each long enough for the regulator to
      // have settled into it — the point of the probe is the STEADY difference,
      // not the transient when the sound arrives.
      const bool sounding = t >= half;
      voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                   pcm.data(), pcm.size());
      ear.tick(s.brain, pcm.data(), pcm.size());
      s.brain.step();
      const uint64_t into = sounding ? t - half : t;
      if (into < half / 2) continue;  // let the regulator settle in each half
      if (sounding) { loud += net.module(uint32_t(aud_m)).spikes; ++nl; }
      else { quiet += net.module(uint32_t(aud_m)).spikes; ++nq; }
    }
    const double q = nq ? quiet / double(nq) : 0.0;
    const double l = nl ? loud / double(nl) : 0.0;
    gap[a] = q > 0.0 ? 100.0 * (l / q - 1.0) : 0.0;
    std::printf("    %-8.2f %-11.2f %-11.2f %+-9.1f\n", double(sc), q, l, gap[a]);
  }

  std::printf("\n  gap is what a mechanism reading presynaptic RATE would have to work\n"
              "  with: the module's spikes per tick during speech against silence.\n");
  std::printf("\n  shipped (1.00)         %+.1f%%\n"
              "  relaxed (0.10)         %+.1f%%   (must be at least 1.2x the shipped gap)\n"
              "  ablated (0.00)         %+.1f%%   (reported, not required. The\n"
              "                         [normalisation] note's collapse-at-zero is about\n"
              "                         central's SEPARABILITY, which is a different\n"
              "                         observable — a wide rate gap here is no evidence\n"
              "                         the ablated creature is a good one)\n",
              gap[0], gap[2], gap[3]);

  const bool relaxing_helps = gap[0] > 0.0 && gap[2] > 1.2 * gap[0];
  if (!relaxing_helps) {
    std::printf("\n  FAIL — relaxing the regulator no longer opens the gap. Either the\n"
                "  ear or the module's operating point has moved, and every claim\n"
                "  about rate-reading mechanisms being blind here has to be re-taken.\n");
  } else {
    std::printf("\n  PASS — the regulator costs real dynamic range and relaxing it\n"
                "  returns some. Note what this does NOT say: the shipped gap is\n"
                "  %+.1f%%, not nothing, so a rate-reading mechanism here is working\n"
                "  against a narrowed signal rather than a flat one. An earlier\n"
                "  reading of 2.8%% came from `stpprobe` measuring its silent block\n"
                "  first, on a creature whose regulator had not settled.\n", gap[0]);
  }
  (void)verbose;
  return relaxing_helps;
}


// --- mechverify: a pinned hash for every mechanism that ships OFF -----------
//
// `verify` pins exactly one number, the determinism hash of the shipped genome,
// and that pin has paid for itself repeatedly. It also has a blind spot large
// enough to hide a bug for four DNA versions: **it is taken on one genome, and
// a dozen mechanisms are switched off in it.** Nothing those mechanisms do is
// hashed, so nothing about them can go red.
//
// That is not hypothetical. `syn_elig_mean_` was not carried through the
// pruning compaction from DNA v16 until 2026-08-23, so after every sleep prune
// a surviving synapse inherited another edge's eligibility baseline. The pin
// stayed green the whole time, because reaching the bug needs BOTH a sleep
// prune and `elig_baseline_tau_ms` above zero, and the shipped genome has
// neither. Proving the fix was real required a genome nobody runs.
//
// So: one pinned hash per mechanism, each on a genome where that mechanism is
// switched on and nothing else is, at a length where it is demonstrably doing
// something.
//
// **The second check is what makes this worth having.** A pin that matches is
// only evidence if the variant differs from the shipped creature at all. A
// mechanism that is enabled and inert hashes identically to the off genome, and
// pinning that number would lock in a mechanism that does nothing while looking
// tested — which is the exact failure mode this project has paid for under
// half a dozen names (v18 measured flat, v28 inert by arithmetic, v35's cap
// already closed). Every variant therefore has to move the hash as well as
// match its pin, and a variant that does not is reported as VACUOUS and fails.
namespace {

enum class PatchScope { kHeader, kModule, kProjection };

struct Edit {
  PatchScope scope;
  const char* target;  // module name, "src->dst", or nullptr for the header
  size_t offset;       // byte offset within DnaHeader / DnaModule / DnaProjection
  float value;
  bool as_uint;        // a few genome switches are uint32_t, not float
  // When set, the value written is the INDEX of this module rather than
  // `value`. DnaModule::ffi_source is a module index and the first version of
  // this table forgot it, which made the v24 variant hash identically to the
  // shipped creature — caught by the vacuity check below on its first run.
  const char* index_of;
};

struct MechPin {
  const char* name;
  const char* dna;     // which DNA version introduced it
  Edit edits[8];
  uint32_t n_edits;
  uint64_t ticks;
  uint64_t hash;       // 0 means "not pinned yet"; the run prints what to paste
};

#define M_(f) offsetof(aibaby::DnaModule, f)
#define P_(f) offsetof(aibaby::DnaProjection, f)

constexpr size_t kStdpElig =
    offsetof(aibaby::DnaHeader, stdp) + offsetof(aibaby::DnaStdp, elig_baseline_tau_ms);
constexpr size_t kStdpPre =
    offsetof(aibaby::DnaHeader, stdp) + offsetof(aibaby::DnaStdp, elig_pre_centre);
constexpr size_t kStdpBurstTau =
    offsetof(aibaby::DnaHeader, stdp) + offsetof(aibaby::DnaStdp, burst_baseline_tau_ms);
constexpr size_t kCuriosityPredict =
    offsetof(aibaby::DnaHeader, curiosity) + offsetof(aibaby::DnaCuriosity, predict_gain);
constexpr size_t kPruneCompete = offsetof(aibaby::DnaHeader, consolidate) +
                                 offsetof(aibaby::DnaConsolidate, prune_compete);

constexpr uint64_t kShort = 120000;
// Pruning only executes inside a consolidation pass, and the creature does not
// fall asleep until ~1.04M ticks. A 120000-tick pin on it would be vacuous by
// construction, which is exactly what the vacuity check exists to catch — so
// the variant is given a length at which it can actually run instead.
constexpr uint64_t kThroughSleep = 1300000;

const MechPin kMechPins[] = {
    {"predictive coding", "v15",
     {{PatchScope::kHeader, nullptr, kCuriosityPredict, 0.5f, false, nullptr}}, 1, kShort, 0x5246e218f7c2b8d6ull},
    {"eligibility baseline", "v16",
     {{PatchScope::kHeader, nullptr, kStdpElig, 20000.0f, false, nullptr}}, 1, kShort, 0x2ef07ecfd3c1756dull},
    {"presynaptic centring", "v17",
     {{PatchScope::kHeader, nullptr, kStdpPre, 1.0f, false, nullptr}}, 1, kShort, 0x12c4c9061cc62019ull},
    {"per-pathway Hebbian", "v23",
     {{PatchScope::kProjection, "central->vocal", P_(hebb), 1e-4f, false, nullptr}}, 1, kShort, 0x1fc86fcdc2b06e2dull},
    {"pooling interneurons", "v24",
     {{PatchScope::kModule, "central", M_(ffi_source), 0.0f, true, "vision"},
      {PatchScope::kModule, "central", M_(ffi_gain), 0.5f, false, nullptr}},
     2, kShort, 0x633dcd0314e81925ull},
    {"apical compartment", "v25",
     {{PatchScope::kModule, "vocal", M_(apical_threshold), 0.35f, false, nullptr},
      {PatchScope::kModule, "vocal", M_(apical_gain), 1.0f, false, nullptr},
      {PatchScope::kProjection, "vision->vocal", P_(apical), 1.0f, true, nullptr}},
     3, kShort, 0x73653dbad8466837ull},
    {"oscillations", "v26",
     {{PatchScope::kModule, "central", M_(theta_amp), 0.05f, false, nullptr},
      {PatchScope::kModule, "central", M_(gamma_amp), 0.02f, false, nullptr}},
     2, kShort, 0xe77ded57ffba8e6cull},
    {"critical period", "v28",
     {{PatchScope::kModule, "central", M_(critical_tau_ms), 300000.0f, false, nullptr}}, 1,
     kShort, 0x7dcf64323b9e6ae3ull},
    {"plateau-gated plasticity", "v29",
     {{PatchScope::kModule, "vocal", M_(apical_threshold), 0.35f, false, nullptr},
      {PatchScope::kModule, "vocal", M_(apical_gain), 1.0f, false, nullptr},
      {PatchScope::kProjection, "vision->vocal", P_(apical), 1.0f, true, nullptr},
      {PatchScope::kModule, "vocal", M_(plateau_gate), 0.5f, false, nullptr}},
     4, kShort, 0xd0a3b887d3dfd855ull},
    {"lateral competition", "v32",
     {{PatchScope::kModule, "central", M_(lateral_gain), 0.3f, false, nullptr},
      {PatchScope::kModule, "central", M_(lateral_sigma), 0.2f, false, nullptr}},
     2, kShort, 0xfeb08e20bf42c877ull},
    {"dynamic synapses", "v36",
     {{PatchScope::kProjection, "auditory->central", P_(stp_use), 0.5f, false, nullptr},
      {PatchScope::kProjection, "auditory->central", P_(stp_recover_ms), 300.0f, false, nullptr}},
     2, kShort, 0xed756042e4167e0cull},
    {"burst plasticity", "v37",
     {{PatchScope::kModule, "vocal", M_(burst_ms), 20.0f, false, nullptr},
      {PatchScope::kHeader, nullptr, kStdpBurstTau, 2000.0f, false, nullptr},
      {PatchScope::kProjection, "central->vocal", P_(burst_learn), 1e-3f, false, nullptr}},
     3, kShort, 0xd4159bcbf1dddfe2ull},
    {"competitive pruning", "v38",
     {{PatchScope::kHeader, nullptr, kPruneCompete, 0.5f, false, nullptr}}, 1, kThroughSleep, 0x8bc54c9948268aedull},
    {"per-module elig tau", "v39",
     {{PatchScope::kModule, "central", M_(elig_tau_scale), 4.0f, false, nullptr}}, 1, kShort, 0x6037b59ae289c878ull},
    {"dendritic error", "v40",
     {{PatchScope::kModule, "vocal", M_(apical_threshold), 0.35f, false, nullptr},
      {PatchScope::kModule, "vocal", M_(apical_gain), 1.0f, false, nullptr},
      {PatchScope::kProjection, "vision->vocal", P_(apical), 1.0f, true, nullptr},
      {PatchScope::kModule, "vocal", M_(ffi_source), 0.0f, true, "vision"},
      {PatchScope::kModule, "vocal", M_(ffi_gain), 0.5f, false, nullptr},
      {PatchScope::kModule, "vocal", M_(ffi_apical), 1.0f, true, nullptr},
      {PatchScope::kModule, "vocal", M_(ffi_learn), 2e-4f, false, nullptr}},
     7, kShort, 0x457a17d1af252433ull},
};
constexpr uint32_t kMechPinCount = sizeof(kMechPins) / sizeof(kMechPins[0]);

}  // namespace

bool run_mechverify(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  aibaby::Dna dna0;
  if (dna0.load(dna_blob.data(), dna_blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const size_t mod_base = sizeof(aibaby::DnaHeader);
  const size_t proj_base = mod_base + sizeof(aibaby::DnaModule) * dna0.module_count();

  auto module_index = [&](const char* name) -> int32_t {
    for (uint32_t m = 0; m < dna0.module_count(); ++m) {
      if (std::strcmp(dna0.module(m).name, name) == 0) return int32_t(m);
    }
    return -1;
  };
  auto projection_index = [&](const char* route) -> int32_t {
    const char* arrow = std::strstr(route, "->");
    if (!arrow) return -1;
    const std::string src(route, size_t(arrow - route));
    const std::string dst(arrow + 2);
    const int32_t si = module_index(src.c_str());
    const int32_t di = module_index(dst.c_str());
    if (si < 0 || di < 0) return -1;
    for (uint32_t i = 0; i < dna0.header().projection_count; ++i) {
      const aibaby::DnaProjection& p = dna0.projection(i);
      if (int32_t(p.src) == si && int32_t(p.dst) == di) return int32_t(i);
    }
    return -1;
  };

  // The shipped creature's hash at each length used below, so a variant can be
  // compared against the right baseline rather than against a constant.
  auto run_blob = [&](const std::vector<uint8_t>& blob, uint64_t n,
                      uint64_t* out) -> bool {
    Session s;
    if (!s.init(blob, error)) return false;
    Retina retina;
    Ear ear;
    const aibaby::DnaVision& vcfg = dna0.header().vision;
    const aibaby::DnaAudio& acfg = dna0.header().audio;
    if (!retina.configure(vcfg, error) || !ear.configure(acfg, error)) return false;
    VowelSource voice(acfg.sample_rate);
    SceneSource scene(vcfg.frame_size, dna0.header().seed);
    std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
    std::vector<float> pcm(acfg.sample_rate / 1000);
    const uint64_t frame_ticks =
        uint64_t(1000.0f / vcfg.frame_hz / dna0.header().sim.dt_ms + 0.5f);
    aibaby::Rng rng;
    rng.seed(dna0.header().seed ^ 0x9EC4u);
    const Toy toy = m3_toy(rng, 0);
    const Word& w = kWords[0];
    for (uint64_t t = 0; t < n; ++t) {
      if (t % frame_ticks == 0) {
        scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }
      // A word every other second, so tracts that only move when something is
      // heard are exercised rather than left at rest. Silence would make a
      // pin on an auditory mechanism nearly vacuous by construction.
      const bool sounding = (t % 2000) < 600;
      voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                   pcm.data(), pcm.size());
      ear.tick(s.brain, pcm.data(), pcm.size());
      s.brain.step();
    }
    *out = s.brain.network().state_hash();
    return true;
  };

  instrument("mechverify", dna0.header().seed ^ 0x9EC4u, kMechPinCount, "mechanisms");
  std::printf("  one pinned hash per mechanism that ships OFF, each on a genome where\n"
              "  that mechanism alone is switched on. A variant must MATCH its pin and\n"
              "  DIFFER from the shipped creature: an enabled mechanism that hashes the\n"
              "  same as the off one is inert, and pinning it would lock in a test that\n"
              "  cannot fail.\n");
  (void)ticks;

  uint64_t base_short = 0, base_long = 0;
  if (!run_blob(dna_blob, kShort, &base_short)) {
    std::printf("  baseline failed: %s\n", error.c_str());
    return false;
  }
  bool need_long = false;
  for (uint32_t i = 0; i < kMechPinCount; ++i) {
    if (kMechPins[i].ticks == kThroughSleep) need_long = true;
  }
  if (need_long && !run_blob(dna_blob, kThroughSleep, &base_long)) {
    std::printf("  long baseline failed: %s\n", error.c_str());
    return false;
  }
  std::printf("\n  shipped baseline   %016llx at %llu ticks\n",
              (unsigned long long)base_short, (unsigned long long)kShort);
  if (need_long) {
    std::printf("                     %016llx at %llu ticks\n",
                (unsigned long long)base_long, (unsigned long long)kThroughSleep);
  }

  std::printf("\n    %-26s %-5s %-9s %-18s %-18s %s\n", "mechanism", "dna", "ticks",
              "expected", "measured", "verdict");
  uint32_t drifted = 0, vacuous = 0, unpinned = 0, broken = 0;
  for (uint32_t i = 0; i < kMechPinCount; ++i) {
    const MechPin& pin = kMechPins[i];
    std::vector<uint8_t> variant = dna_blob;
    bool ok = true;
    for (uint32_t e = 0; e < pin.n_edits && ok; ++e) {
      const Edit& ed = pin.edits[e];
      size_t at = 0;
      if (ed.scope == PatchScope::kHeader) {
        at = ed.offset;
      } else if (ed.scope == PatchScope::kModule) {
        const int32_t m = module_index(ed.target);
        if (m < 0) { ok = false; break; }
        at = mod_base + sizeof(aibaby::DnaModule) * size_t(m) + ed.offset;
      } else {
        const int32_t p = projection_index(ed.target);
        if (p < 0) { ok = false; break; }
        at = proj_base + sizeof(aibaby::DnaProjection) * size_t(p) + ed.offset;
      }
      if (ed.index_of) {
        const int32_t idx = module_index(ed.index_of);
        if (idx < 0) { ok = false; break; }
        std::memcpy(variant.data() + at, &idx, sizeof(idx));
      } else if (ed.as_uint) {
        const uint32_t v = uint32_t(ed.value);
        std::memcpy(variant.data() + at, &v, sizeof(v));
      } else {
        std::memcpy(variant.data() + at, &ed.value, sizeof(ed.value));
      }
    }
    if (!ok) {
      std::printf("    %-26s %-5s %-9s %-18s %-18s %s\n", pin.name, pin.dna, "-", "-",
                  "-", "SKIP: this body plan has no such module or tract");
      ++broken;
      continue;
    }
    uint64_t hash = 0;
    if (!run_blob(variant, pin.ticks, &hash)) {
      std::printf("    %-26s %-5s %-9llu %-18s %-18s %s\n", pin.name, pin.dna,
                  (unsigned long long)pin.ticks, "-", "-", "FAIL: genome refused");
      std::printf("      %s\n", error.c_str());
      ++broken;
      continue;
    }
    const uint64_t base = pin.ticks == kThroughSleep ? base_long : base_short;
    const char* verdict = "ok";
    if (hash == base) { verdict = "VACUOUS: inert at this length"; ++vacuous; }
    else if (pin.hash == 0) { verdict = "unpinned — paste the measured value"; ++unpinned; }
    else if (hash != pin.hash) { verdict = "DRIFTED"; ++drifted; }
    char expected[24];
    if (pin.hash == 0) std::snprintf(expected, sizeof(expected), "%s", "-");
    else std::snprintf(expected, sizeof(expected), "%016llx",
                       (unsigned long long)pin.hash);
    char measured[24];
    std::snprintf(measured, sizeof(measured), "%016llx", (unsigned long long)hash);
    std::printf("    %-26s %-5s %-9llu %-18s %-18s %s\n", pin.name, pin.dna,
                (unsigned long long)pin.ticks, expected, measured, verdict);
  }

  std::printf("\n  %u mechanisms, %u drifted, %u vacuous, %u unpinned, %u broken\n",
              kMechPinCount, drifted, vacuous, unpinned, broken);
  if (drifted) {
    std::printf("\n  FAIL — a mechanism that ships OFF changed behaviour. `verify` cannot\n"
                "  see this and did not: its pin is on one genome and these are not it.\n"
                "  If the change was deliberate, move the pin in the same commit and say\n"
                "  in the changelog what moved it.\n");
  } else if (vacuous) {
    std::printf("\n  FAIL — a mechanism is enabled and hashes identically to the shipped\n"
                "  creature, so it did nothing at all over its run. Pinning that number\n"
                "  would lock in a test that cannot fail. Either the settings above are\n"
                "  too weak to bite, or the mechanism does not run.\n");
  } else if (broken) {
    std::printf("\n  FAIL — a variant could not be built or was refused by the genome\n"
                "  loader. A mechanism nobody can switch on is not covered by anything.\n");
  } else if (unpinned) {
    std::printf("\n  UNPINNED — every variant ran and moved the hash, and none has a\n"
                "  recorded value yet. Paste the measured column into kMechPins and\n"
                "  this becomes a test.\n");
  } else {
    std::printf("\n  PASS — every off-by-default mechanism still behaves exactly as it\n"
                "  did when its hash was recorded, and every one of them does something.\n");
  }
  (void)verbose;
  return drifted == 0 && vacuous == 0 && broken == 0 && unpinned == 0;
}


// --- errprobe: does the tuft learn to carry an ERROR? ----------------------
//
// DNA v40 is the dendritic error microcircuit (Sacramento, Ponte Costa, Bengio
// & Senn 2018): the pooling interneuron moves onto the apical compartment and
// its weight learns until what the tuft integrates is top-down input MINUS what
// the lateral pool predicted of it. Right prediction, zero residual, nothing
// written; wrong prediction, the residual is what drives learning.
//
// It is worth measuring against a specific number rather than against zero.
// `burstprobe` reads the *raw* apical signal at the larynx — the plateau — as
// carrying cube-versus-ball at **0.913**, and the burst derived from it at
// 0.673. So the tuft already carries the object very well, and v40's claim is
// not that it makes the tuft informative. The claim is that subtracting a
// learned prediction removes the part that is the same for both objects, which
// is this project's standing diagnosis stated as a circuit: *a small
// differential riding on a large common mode*.
//
// Three arms, each patched explicitly including the control:
//
//   soma           v24 as it stands — interneuron on the soma, weight fixed at
//                  its in-degree-weighted birth value. The mechanism that
//                  measurably worked (+0.077, 3/3 families).
//   tuft, fixed    the same interneuron moved onto the compartment, still
//                  fixed. This isolates *where* it lands from *whether it
//                  learns*, and without it a result could be either.
//   tuft, learning the microcircuit.
//
// Four columns, and the second exists to stop the first being believed on its
// own. `resid early` and `resid late` are the mean |apical| over the first and
// last thirds of the run: the microcircuit should drive it down. But a residual
// falling because the input went quiet looks identical, so `ffi w` reports
// whether the weight actually moved — a residual that fell with a weight that
// did not is the creature going silent, not the circuit learning.
namespace {

struct ErrArm {
  const char* name;
  uint32_t apical;   // land the interneuron on the tuft
  float learn;       // and let its weight move
};

constexpr ErrArm kErrArms[] = {
    {"soma",           0u, 0.0f},
    {"tuft, fixed",    1u, 0.0f},
    {"tuft, learning", 1u, 2e-4f},
};
constexpr uint32_t kErrArmCount = sizeof(kErrArms) / sizeof(kErrArms[0]);

constexpr float kErrApicalThreshold = 0.35f;
constexpr float kErrApicalGain = 1.0f;
constexpr float kErrFfiGain = 0.5f;

struct ErrRow {
  double resid_early = 0.0, resid_late = 0.0;
  double ffi_early = 0.0, ffi_late = 0.0;
  double plateau_pct = 0.0;
  double obj_resid = 0.0, shuffled = 0.0;
  size_t trials = 0;
};

}  // namespace

bool run_errprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  aibaby::Dna dna0;
  if (dna0.load(dna_blob.data(), dna_blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t voc_m = dna0.module_with_role(aibaby::ModuleRole::kVocal);
  const int32_t vis_m = dna0.module_with_role(aibaby::ModuleRole::kVision);
  if (voc_m < 0 || vis_m < 0) {
    std::printf("  this genome has no vocal or vision module\n");
    return false;
  }
  int32_t tuft = -1;
  for (uint32_t i = 0; i < dna0.header().projection_count; ++i) {
    const aibaby::DnaProjection& p = dna0.projection(i);
    if (int32_t(p.dst) == voc_m && int32_t(p.src) == vis_m) tuft = int32_t(i);
  }
  if (tuft < 0) {
    std::printf("  this genome has no vision->vocal tract to put on the tuft\n");
    return false;
  }

  const aibaby::DnaVision& vcfg = dna0.header().vision;
  const aibaby::DnaAudio& acfg = dna0.header().audio;
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / dna0.header().sim.dt_ms + 0.5f);
  const uint32_t n_trials = uint32_t(ticks / (kErrArmCount * kM3ProbeTicks));
  const size_t mod_base = sizeof(aibaby::DnaHeader);
  const size_t proj_base = mod_base + sizeof(aibaby::DnaModule) * dna0.module_count();

  instrument("errprobe", dna0.header().seed ^ 0xE770u, n_trials, "trials per arm");
  std::printf("  architecture      vision->vocal on the TUFT; the interneuron pools\n"
              "                    %s, which is the input it has to predict\n",
              dna0.module(uint32_t(vis_m)).name);
  std::printf("  reference         burstprobe reads the RAW plateau at 0.913 across\n"
              "                    three seeds. v40 is not trying to beat that with a\n"
              "                    bigger signal, but with a cleaner one\n");

  ErrRow rows[kErrArmCount];

  for (uint32_t a = 0; a < kErrArmCount; ++a) {
    const ErrArm& arm = kErrArms[a];
    std::vector<uint8_t> variant = dna_blob;
    {
      auto put_m = [&](size_t off, const void* v, size_t n) {
        std::memcpy(variant.data() + mod_base +
                        sizeof(aibaby::DnaModule) * size_t(voc_m) + off,
                    v, n);
      };
      const float thr = kErrApicalThreshold, gain = kErrApicalGain;
      put_m(offsetof(aibaby::DnaModule, apical_threshold), &thr, sizeof(thr));
      put_m(offsetof(aibaby::DnaModule, apical_gain), &gain, sizeof(gain));
      const int32_t src = vis_m;
      put_m(offsetof(aibaby::DnaModule, ffi_source), &src, sizeof(src));
      const float fg = kErrFfiGain;
      put_m(offsetof(aibaby::DnaModule, ffi_gain), &fg, sizeof(fg));
      put_m(offsetof(aibaby::DnaModule, ffi_apical), &arm.apical, sizeof(arm.apical));
      put_m(offsetof(aibaby::DnaModule, ffi_learn), &arm.learn, sizeof(arm.learn));
      const uint32_t ap = 1u;
      std::memcpy(variant.data() + proj_base +
                      sizeof(aibaby::DnaProjection) * size_t(tuft) +
                      offsetof(aibaby::DnaProjection, apical),
                  &ap, sizeof(ap));
    }
    Session s;
    if (!s.init(variant, error)) {
      std::printf("  arm %s failed to hatch: %s\n", arm.name, error.c_str());
      return false;
    }
    const aibaby::Network& net = s.brain.network();
    Retina retina;
    Ear ear;
    if (!retina.configure(vcfg, error) || !ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource voice(acfg.sample_rate);
    SceneSource scene(vcfg.frame_size, dna0.header().seed);
    std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
    std::vector<float> pcm(acfg.sample_rate / 1000);
    aibaby::Rng rng;
    rng.seed(dna0.header().seed ^ 0xE770u);

    const aibaby::ModuleState& vm = net.module(uint32_t(voc_m));
    const uint32_t width = vm.count;
    std::vector<std::vector<double>> feat;
    std::vector<int> labels;
    double resid[2] = {}, ffiw[2] = {};
    uint64_t nres[2] = {};
    double plat_ticks = 0.0, all_ticks = 0.0;
    const uint32_t third = n_trials / 3 ? n_trials / 3 : 1;

    for (uint32_t trial = 0; trial < n_trials; ++trial) {
      const int label = int(trial % 2);
      const Toy toy = m3_toy(rng, label);
      std::vector<double> res(width, 0.0);
      bool slept = false;
      uint64_t scored = 0;
      for (uint64_t t = 0; t < kM3ProbeTicks; ++t) {
        if (t % frame_ticks == 0) {
          scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
          retina.present(frame.data());
          s.brain.see(retina.features().data(), retina.feature_count());
        }
        voice.render(0.0f, 0.0f, 0.0f, 0.0f, pcm.data(), pcm.size());
        ear.tick(s.brain, pcm.data(), pcm.size());
        s.brain.step();
        if (s.brain.asleep()) slept = true;
        if (t < kM3SettleTicks) continue;
        ++scored;
        for (uint32_t k = 0; k < width; ++k) {
          const uint32_t i = vm.begin + k;
          res[k] += std::fabs(double(net.apical_membrane(i)));
          if (net.in_plateau(i)) plat_ticks += 1.0;
          ++all_ticks;
        }
      }
      // The first and last thirds, so "did it settle" is a comparison inside
      // one creature's life rather than between two of them.
      const int bin = trial < third ? 0 : (trial >= n_trials - third ? 1 : -1);
      if (bin >= 0) {
        resid[bin] += double(net.mean_apical(uint32_t(voc_m)));
        ffiw[bin] += double(net.mean_ffi_weight(uint32_t(voc_m)));
        ++nres[bin];
      }
      if (slept || scored == 0) continue;
      for (uint32_t k = 0; k < width; ++k) res[k] /= double(scored);
      feat.push_back(res);
      labels.push_back(label);
    }

    ErrRow& row = rows[a];
    row.trials = labels.size();
    row.resid_early = nres[0] ? resid[0] / double(nres[0]) : 0.0;
    row.resid_late = nres[1] ? resid[1] / double(nres[1]) : 0.0;
    row.ffi_early = nres[0] ? ffiw[0] / double(nres[0]) : 0.0;
    row.ffi_late = nres[1] ? ffiw[1] / double(nres[1]) : 0.0;
    row.plateau_pct = all_ticks > 0.0 ? 100.0 * plat_ticks / all_ticks : 0.0;
    if (labels.size() >= 12) {
      std::vector<std::vector<double>> x;
      std::vector<int> y;
      size_t tr = 0;
      interleave_pairs(feat, labels, x, y, tr);
      row.obj_resid = holdout_accuracy(x, y, tr);
      double null_sum = 0.0;
      for (uint32_t perm = 0; perm < 32; ++perm) {
        std::vector<int> sh = y;
        for (size_t i = sh.size(); i > 1; --i) std::swap(sh[i - 1], sh[rng.next() % i]);
        null_sum += holdout_accuracy(x, sh, tr);
      }
      row.shuffled = null_sum / 32.0;
    }
  }

  std::printf("\n    %-16s %-7s %-11s %-11s %-9s %-9s %-8s %-11s %-9s\n", "arm", "trials",
              "resid early", "resid late", "ffi w in", "ffi w out", "plat%",
              "obj|resid", "shuffled");
  for (uint32_t a = 0; a < kErrArmCount; ++a) {
    const ErrRow& r = rows[a];
    std::printf("    %-16s %-7zu %-11.5f %-11.5f %-9.4f %-9.4f %-8.1f %-11.3f %-9.3f\n",
                kErrArms[a].name, r.trials, r.resid_early, r.resid_late, r.ffi_early,
                r.ffi_late, r.plateau_pct, r.obj_resid, r.shuffled);
  }

  const ErrRow& soma = rows[0];
  const ErrRow& fixed = rows[1];
  const ErrRow& learn = rows[2];
  // Between arms, not within one. The weight converges inside the first trial,
  // so first-third against last-third cannot see the transient — the first run
  // of this probe reported "x0.971, did not move" for a weight that had gone
  // from 1.0 to 0.0018 before the first bin closed. The fixed arm is the same
  // creature with the same interneuron on the same compartment, differing only
  // in whether the weight may move, and that is the comparison.
  const double cancelled =
      fixed.resid_late > 0.0 ? 100.0 * (1.0 - learn.resid_late / fixed.resid_late) : 0.0;

  std::printf("\n  resid is the mean |apical| at the larynx. `ffi w` is the mean pooling\n"
              "  weight, and it is the control on the residual: a residual that fell\n"
              "  while the weight did not move is the creature going quiet, not the\n"
              "  circuit learning. obj|resid classifies cube against ball from the\n"
              "  per-neuron residual, against a 32-permutation null.\n");
  std::printf("\n  residual vs fixed arm  %+.1f%%   (learning must cancel what a fixed\n"
              "                         weight on the same compartment does not)\n"
              "  weight, birth -> end   %.4f -> %.4f\n"
              "  plateau, learn/fixed   %.1f%% / %.1f%%   (a compartment driven to a rail\n"
              "                         by an uncalibrated prediction never fires)\n"
              "  obj|resid, learn/soma  %.3f / %.3f   (the payoff, not a check)\n"
              "  raw plateau reference  0.913 from burstprobe, three seeds\n",
              cancelled, learn.ffi_early, learn.ffi_late, learn.plateau_pct,
              fixed.plateau_pct, learn.obj_resid, soma.obj_resid);

  const bool control_ok = std::fabs(soma.ffi_late - soma.ffi_early) < 1e-6 &&
                          std::fabs(fixed.ffi_late - fixed.ffi_early) < 1e-6;
  const bool ran = learn.plateau_pct > 0.5 && learn.trials >= 12;
  const bool learned = cancelled > 50.0 && learn.plateau_pct > fixed.plateau_pct;
  if (!control_ok) {
    std::printf("\n  FAIL — a fixed-weight arm's pooling weight moved. ffi_learn 0 is not\n"
                "  off, so neither control is one.\n");
  } else if (!ran) {
    std::printf("\n  FAIL — the compartment never ran (%.1f%% plateau, %zu trials), so no\n"
                "  column above is a measurement of anything.\n",
                learn.plateau_pct, learn.trials);
  } else if (!learned) {
    std::printf("\n  The microcircuit did NOT cancel: residual %+.1f%% against the fixed\n"
                "  arm, plateau %.1f%% against %.1f%%. Sacramento's interneuron settles\n"
                "  when its prediction can track the top-down input; one scalar per\n"
                "  neuron predicting a whole tract may not be able to, which would be\n"
                "  v24's shared-scalar failure one level up. Reported, not fatal.\n",
                cancelled, learn.plateau_pct, fixed.plateau_pct);
  } else {
    std::printf("\n  PASS — the circuit runs and settles. Read that narrowly: the fixed\n"
                "  arm is a broken configuration, not a rival, so beating it by %.1f%%\n"
                "  only says a learned weight is usable on a tuft where a fixed one is\n"
                "  not (%.1f%% plateau against %.1f%%).\n",
                cancelled, learn.plateau_pct, fixed.plateau_pct);
    std::printf("\n  WHAT IT BUYS, against the arm that is a rival: obj|resid reads %.3f\n"
                "  learning against %.3f with the interneuron left at the soma. The\n"
                "  converged weight is %.4f — the circuit finds almost nothing to\n"
                "  cancel, and the residual is the raw apical signal with a rounding\n"
                "  error taken off it.\n"
                "\n  The scales are the reason to suspect rather than the mechanism. `ffi`\n"
                "  is a pooled rate in Hz, order 1, while the apical input is a sparse\n"
                "  tract's per-tick arrival, order 0.1 — so one scalar per neuron\n"
                "  multiplying a smooth rate cannot track a bursty sparse input, and the\n"
                "  cancelling weight it converges on is small because that is the best\n"
                "  such a predictor can do. A predictor that could is one that sees the\n"
                "  same spikes: an interneuron POPULATION sampled per target, which is\n"
                "  what Sacramento's circuit actually has and what DnaModule::ffi_source\n"
                "  is not. That is a body-plan change, not a genome field.\n",
                learn.obj_resid, soma.obj_resid, learn.ffi_late);
  }
  (void)verbose;
  return control_ok && ran;
}


// --- relayprobe: is Webb's TWO-stage circuit a bandpass? --------------------
//
// `stpprobe` measured that one dynamic synapse is a gain knob and not a filter,
// and named why: depression scales everything a synapse transmits by a single
// number tracking its own mean rate — a high-pass with no upper corner. Webb's
// bandpass is two stages, BN1 depressing feeding BN2 facilitating, and **the
// tuning lives in the mismatch between their time constants rather than in
// either synapse**. This builds that and asks whether the mismatch produces a
// peak.
//
// Four arms, because "two stages" and "the RIGHT two stages" are different
// claims and only the second is Webb's:
//
//   off        a constant-weight relay. The control, and the baseline every
//              `vs off` column is read against.
//   dep -> dep both stages depressing. If a peak appears here, the finding is
//              about having a relay at all and nothing to do with Webb.
//   dep -> fac Webb's circuit: recover-gated onset detection feeding a
//              facilitating stage that needs those onsets close together.
//   fac -> dep the same two synapses in the wrong order. A bandpass built from
//              a mismatch should not survive swapping which side it is on.
//
// The stimulus is `stpprobe`'s: a 50% duty cycle at every rate, so all of them
// carry identical total sound and only the timing differs, plus a shuffled row
// holding the mean rate of the 4 Hz row and destroying only its regularity.
//
// The band is chosen by the ear, not by preference. The cochlea's window is
// 32 ms, so an envelope whose half-period is shorter arrives as steady energy
// and 12 Hz is the ceiling. Webb's crickets work at 20-30 Hz through an ear with
// microsecond resolution; the band this creature resolves, 1-12 Hz, is where the
// syllable rate of human speech sits, which is why the test is worth running
// here at all rather than waiting for a better cochlea.
namespace {

struct RelayArm {
  const char* name;
  float in_use, in_rec, in_fac;    // auditory -> relay
  float out_use, out_rec, out_fac; // relay -> central
};

constexpr RelayArm kRelayArms[] = {
    {"off",        0.00f,   0.0f,   0.0f,  0.00f,   0.0f,   0.0f},
    {"dep -> dep", 0.50f, 300.0f,   0.0f,  0.50f, 300.0f,   0.0f},
    {"dep -> fac", 0.50f, 300.0f,   0.0f,  0.10f,  50.0f, 300.0f},
    {"fac -> dep", 0.10f,  50.0f, 300.0f,  0.50f, 300.0f,   0.0f},
};
constexpr uint32_t kRelayArmCount = sizeof(kRelayArms) / sizeof(kRelayArms[0]);

constexpr float kRelayRates[] = {2.0f, 4.0f, 8.0f, 12.0f};
constexpr uint32_t kRelayRateCount = sizeof(kRelayRates) / sizeof(kRelayRates[0]);
constexpr uint32_t kRelayConditions = kRelayRateCount + 2;  // silence + rates + shuffled
constexpr uint64_t kRelayWarmTicks = 20000;

struct RelayRow {
  double aud = 0.0, relay = 0.0, cen = 0.0;
  double transfer = 0.0;
  double gain_in = 1.0, gain_out = 1.0;
  uint32_t bursts = 0;
};

}  // namespace

bool run_relayprobe(const std::vector<uint8_t>& dna_blob, uint64_t ticks, bool verbose) {
  std::string error;
  aibaby::Dna dna0;
  if (dna0.load(dna_blob.data(), dna_blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t aud_m = dna0.module_with_role(aibaby::ModuleRole::kAuditory);
  const int32_t cen_m = dna0.module_with_role(aibaby::ModuleRole::kAssociation);
  if (aud_m < 0 || cen_m < 0) {
    std::printf("  this genome has no auditory or association module\n");
    return false;
  }
  // The relay: an interneuron population with a tract in from the ear and a
  // tract out to the association module. Found rather than named, so a genome
  // built by tools/genome_add_relay.py works whatever it called the module.
  int32_t relay_m = -1, in_p = -1, out_p = -1;
  for (uint32_t m = 0; m < dna0.module_count(); ++m) {
    if (dna0.module(m).role != uint32_t(aibaby::ModuleRole::kInterneuron)) continue;
    int32_t a = -1, b = -1;
    for (uint32_t i = 0; i < dna0.header().projection_count; ++i) {
      const aibaby::DnaProjection& p = dna0.projection(i);
      if (int32_t(p.src) == aud_m && p.dst == m) a = int32_t(i);
      if (p.src == m && int32_t(p.dst) == cen_m) b = int32_t(i);
    }
    if (a >= 0 && b >= 0) { relay_m = int32_t(m); in_p = a; out_p = b; break; }
  }
  if (relay_m < 0) {
    std::printf("  this genome has no auditory -> relay -> central path.\n"
                "  Build one:  python3 tools/genome_add_relay.py dna/default.toml \\\n"
                "                out.toml auditory central inhib=0.0 name=webbrelay\n");
    return false;
  }

  const aibaby::DnaAudio& acfg = dna0.header().audio;
  const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);
  const uint64_t budget = ticks / (kRelayArmCount * kRelayConditions);
  const size_t proj_base = sizeof(aibaby::DnaHeader) +
                           sizeof(aibaby::DnaModule) * dna0.module_count();

  instrument("relayprobe", dna0.header().seed ^ 0x2E1Au, budget, "ticks per condition");
  std::printf("  path              %s -> %s (%u cells, %s) -> %s\n",
              dna0.module(uint32_t(aud_m)).name, dna0.module(uint32_t(relay_m)).name,
              dna0.module(uint32_t(relay_m)).neurons,
              dna0.module(uint32_t(relay_m)).inhib_fraction >= 0.5f ? "inhibitory"
                                                                   : "excitatory",
              dna0.module(uint32_t(cen_m)).name);
  const double window_ms = 1000.0 * double(acfg.window) / double(acfg.sample_rate);
  std::printf("  ear ceiling       ~%.0f Hz (%.0f ms window), so the sweep stops at %.0f\n",
              500.0 / window_ms, window_ms, double(kRelayRates[kRelayRateCount - 1]));

  RelayRow rows[kRelayArmCount][kRelayConditions];

  for (uint32_t a = 0; a < kRelayArmCount; ++a) {
    const RelayArm& arm = kRelayArms[a];
    std::vector<uint8_t> variant = dna_blob;
    {
      auto put = [&](int32_t p, size_t off, float v) {
        std::memcpy(variant.data() + proj_base +
                        sizeof(aibaby::DnaProjection) * size_t(p) + off,
                    &v, sizeof(v));
      };
      // Explicitly on every arm including `off`, so the control stays a control
      // the day a genome ships with the relay's synapses already dynamic.
      put(in_p, offsetof(aibaby::DnaProjection, stp_use), arm.in_use);
      put(in_p, offsetof(aibaby::DnaProjection, stp_recover_ms), arm.in_rec);
      put(in_p, offsetof(aibaby::DnaProjection, stp_facil_ms), arm.in_fac);
      put(out_p, offsetof(aibaby::DnaProjection, stp_use), arm.out_use);
      put(out_p, offsetof(aibaby::DnaProjection, stp_recover_ms), arm.out_rec);
      put(out_p, offsetof(aibaby::DnaProjection, stp_facil_ms), arm.out_fac);
    }
    Session s;
    if (!s.init(variant, error)) {
      std::printf("  arm %s failed to hatch: %s\n", arm.name, error.c_str());
      return false;
    }
    const aibaby::Network& net = s.brain.network();
    Ear ear;
    if (!ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource voice(acfg.sample_rate);
    std::vector<float> pcm(samples_per_tick);
    const Word& w = kWords[0];
    aibaby::Rng rng;
    rng.seed(dna0.header().seed ^ 0x2E1Au);

    for (uint32_t c = 0; c < kRelayConditions; ++c) {
      const bool silent = (c == 0);
      const bool shuffled = (c == kRelayConditions - 1);
      const double rate = shuffled ? 4.0 : (silent ? 0.0 : double(kRelayRates[c - 1]));
      const double half_ms = silent ? 0.0 : 500.0 / rate;

      std::vector<uint64_t> schedule;
      if (shuffled) {
        uint64_t total = 0;
        while (total < budget + 4000) {
          const double f = 0.25 + 1.5 * double(rng.uniform());
          schedule.push_back(uint64_t(half_ms * f) + 1);
          total += schedule.back();
        }
      }
      // The first condition pays a long warm-up. `stpprobe` learned this the
      // expensive way: measuring a silent control on a just-hatched creature
      // reads the hatch transient and reports it as the creature.
      const uint64_t settle = (c == 0) ? kRelayWarmTicks : 1500;
      uint64_t aud = 0, rel = 0, cen = 0, bursts = 0, scored = 0;
      double gin = 0.0, gout = 0.0;
      size_t slot = 0;
      uint64_t slot_left = schedule.empty() ? 0 : schedule[0];
      bool sounding = false, prev = false;

      for (uint64_t t = 0; t < settle + budget; ++t) {
        const bool measuring = t >= settle;
        const uint64_t bt = measuring ? t - settle : t;
        if (silent) sounding = false;
        else if (shuffled) {
          if (slot_left == 0) {
            slot = (slot + 1) % schedule.size();
            slot_left = schedule[slot];
            sounding = !sounding;
          }
          --slot_left;
        } else {
          sounding = std::fmod(double(bt), 2.0 * half_ms) < half_ms;
        }
        if (measuring && sounding && !prev) ++bursts;
        prev = sounding;
        voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                     pcm.data(), samples_per_tick);
        ear.tick(s.brain, pcm.data(), samples_per_tick);
        s.brain.step();
        if (!measuring || s.brain.asleep()) continue;
        aud += net.module(uint32_t(aud_m)).spikes;
        rel += net.module(uint32_t(relay_m)).spikes;
        cen += net.module(uint32_t(cen_m)).spikes;
        gin += double(net.stp_gain(uint32_t(aud_m), uint32_t(relay_m)));
        gout += double(net.stp_gain(uint32_t(relay_m), uint32_t(cen_m)));
        ++scored;
      }
      RelayRow& row = rows[a][c];
      const double n = scored ? double(scored) : 1.0;
      row.aud = double(aud) / n;
      row.relay = double(rel) / n;
      row.cen = double(cen) / n;
      row.transfer = aud ? double(cen) / double(aud) : 0.0;
      row.gain_in = gin / n;
      row.gain_out = gout / n;
      row.bursts = uint32_t(bursts);
    }
  }

  std::printf("\n    %-12s %-9s %-8s %-8s %-8s %-9s %-8s %-8s %-8s\n", "arm", "envelope",
              "aud", "relay", "cen", "transfer", "vs off", "gain in", "gain out");
  for (uint32_t a = 0; a < kRelayArmCount; ++a) {
    for (uint32_t c = 0; c < kRelayConditions; ++c) {
      char env[16];
      if (c == 0) std::snprintf(env, sizeof(env), "silence");
      else if (c == kRelayConditions - 1) std::snprintf(env, sizeof(env), "shuffled");
      else std::snprintf(env, sizeof(env), "%.0f Hz", double(kRelayRates[c - 1]));
      const RelayRow& r = rows[a][c];
      const double base = rows[0][c].transfer;
      std::printf("    %-12s %-9s %-8.2f %-8.2f %-8.2f %-9.4f %-8.3f %-8.3f %-8.3f\n",
                  c == 0 ? kRelayArms[a].name : "", env, r.aud, r.relay, r.cen,
                  r.transfer, base > 0.0 ? r.transfer / base : 0.0, r.gain_in,
                  r.gain_out);
    }
    if (a + 1 < kRelayArmCount) std::printf("\n");
  }

  // A bandpass is a PEAK in `vs off` over the rate rows, and the honest way to
  // ask is how far the best rate stands above the worst. A gain change is flat
  // down that column whatever its absolute level, so the spread is the quantity
  // and the level is not.
  auto spread = [&](uint32_t arm, uint32_t* at) {
    double lo = 1e9, hi = -1e9;
    for (uint32_t c = 1; c <= kRelayRateCount; ++c) {
      const double base = rows[0][c].transfer;
      const double v = base > 0.0 ? rows[arm][c].transfer / base : 0.0;
      if (v < lo) lo = v;
      if (v > hi) { hi = v; *at = c; }
    }
    return lo > 0.0 ? hi / lo : 0.0;
  };
  uint32_t peak_dd = 0, peak_df = 0, peak_fd = 0, peak_off = 0;
  const double s_off = spread(0, &peak_off);
  const double s_dd = spread(1, &peak_dd);
  const double s_df = spread(2, &peak_df);
  const double s_fd = spread(3, &peak_fd);

  // The off arm's spread is 1.000 BY CONSTRUCTION — that column is the arm
  // divided by itself — so it is an identity and not a noise floor, and the
  // first version of this probe wrongly offered it as one. The two honest
  // controls are already in the table:
  //
  //   dep -> dep   two stages with no mismatch. If Webb's order is not clearly
  //                above this, the tuning is not coming from the mismatch.
  //   shuffled     the 4 Hz row's mean rate with its regularity destroyed. An
  //                INTERVAL filter must prefer the regular train; anything that
  //                prefers the shuffled one is reading rate, not timing.
  const double shuf_df = rows[0][kRelayConditions - 1].transfer > 0.0
                             ? rows[2][kRelayConditions - 1].transfer /
                                   rows[0][kRelayConditions - 1].transfer
                             : 0.0;
  double peak_df_val = 0.0;
  for (uint32_t c = 1; c <= kRelayRateCount; ++c) {
    const double base = rows[0][c].transfer;
    const double v = base > 0.0 ? rows[2][c].transfer / base : 0.0;
    if (v > peak_df_val) peak_df_val = v;
  }

  std::printf("\n  `vs off` is central's spikes-per-auditory-spike against the constant-\n"
              "  weight relay at the SAME envelope. A gain change is flat down that\n"
              "  column; a bandpass peaks. `spread` is the best rate over the worst.\n"
              "  The off arm's spread is 1.000 by construction — it is that column\n"
              "  divided by itself — so it is an identity and NOT a noise floor.\n");
  std::printf("\n  dep -> dep spread %.3f   peak at %.0f Hz   (two stages, no mismatch)\n"
              "  dep -> fac spread %.3f   peak at %.0f Hz   <- Webb's order\n"
              "  fac -> dep spread %.3f   peak at %.0f Hz   (the same pair, swapped)\n"
              "\n  Webb's peak %.3f against its own SHUFFLED row %.3f — an interval\n"
              "  filter must prefer the regular train at the same mean rate.\n",
              s_dd, double(kRelayRates[peak_dd - 1]), s_df,
              double(kRelayRates[peak_df - 1]), s_fd, double(kRelayRates[peak_fd - 1]),
              peak_df_val, shuf_df);
  (void)s_off;

  bool relay_alive = true;
  for (uint32_t a = 0; a < kRelayArmCount; ++a) {
    if (rows[a][1].relay < 0.01) relay_alive = false;
  }
  const bool control_ok = rows[0][1].gain_in > 0.999 && rows[0][1].gain_in < 1.001 &&
                          rows[0][1].gain_out > 0.999 && rows[0][1].gain_out < 1.001;
  const bool arms_differ = rows[2][1].gain_in < 0.95 || rows[2][1].gain_out > 1.05;

  if (!relay_alive) {
    std::printf("\n  FAIL — the relay is silent in at least one arm, so nothing crossed\n"
                "  it and no column above is a measurement of a two-stage anything.\n");
  } else if (!control_ok) {
    std::printf("\n  FAIL — the off arm's synapses are not delivering their genome\n"
                "  weight, so it is not a control.\n");
  } else if (!arms_differ) {
    std::printf("\n  FAIL — the Webb arm's synapses did not move off 1.000. The\n"
                "  mechanism is present and not running.\n");
  } else if (s_df > s_dd * 1.3 && peak_df_val > shuf_df) {
    std::printf("\n  A BANDPASS. `dep -> fac` peaks at %.0f Hz, spreads %.3f against\n"
                "  `dep -> dep`'s %.3f — so the tuning comes from the MISMATCH between\n"
                "  the two stages and not from having a relay — and it prefers the\n"
                "  regular train (%.3f) to the shuffled one at the same mean rate\n"
                "  (%.3f), so it is reading the interval and not the rate.\n",
                double(kRelayRates[peak_df - 1]), s_df, s_dd, peak_df_val, shuf_df);
  } else {
    std::printf("\n  NO BANDPASS, and both controls say so.\n"
                "\n  Webb's order spreads %.3f where two DEPRESSING stages spread %.3f —\n"
                "  indistinguishable, so nothing here comes from the mismatch the whole\n"
                "  circuit is built on. And its peak regular row reads %.3f against a\n"
                "  SHUFFLED row of %.3f: the circuit responds MORE to irregular timing\n"
                "  at the same mean rate, which is the opposite of an interval filter.\n"
                "\n  The stages themselves are working — `gain in` sits near %.2f and\n"
                "  `gain out` near %.2f, moving in opposite directions exactly as two\n"
                "  mismatched time constants should. What does not happen is the\n"
                "  mismatch turning into selectivity at the target.\n",
                s_df, s_dd, peak_df_val, shuf_df, rows[2][2].gain_in,
                rows[2][2].gain_out);
  }
  (void)verbose;
  return relay_alive && control_ok && arms_differ;
}


// --- seqprobe: can a module in this kernel HOLD a sequence? ----------------
//
// `trajprobe` found that an utterance is a held vowel — the formants move ~10 Hz
// inside it and almost none of that movement is shared between utterances. So
// reward has no trajectory to select, and a word is unreachable until something
// generates one. §5.3's own comment names the missing piece: "HVC drives RA
// reliably and LMAN adds variance on top." The creature has an LMAN (DNA v10)
// and an RA (`vocal`), and in place of HVC it has `drive_compensation` — a
// scalar. A constant cannot carry a sequence.
//
// Before designing a generator, ask whether this kernel could run one at all.
// The substrate is already there and nobody has looked at it: `wire_intra_module`
// gives every module dense local recurrence (density 0.5 inside radius 0.4,
// weight 0.12, 20% inhibitory). A sequence needs that loop to do three things
// at once, and they pull against each other:
//
//   persist      activity outlasts the input that started it
//   differ       the pattern CHANGES over time rather than freezing, or it is
//                a memory and not a sequence
//   repeat       the same kick produces the same succession, or reward cannot
//                select it — which is exactly what `trajprobe` found missing
//
// The measurement kicks a fixed random subset of one module, removes the kick,
// and watches. Reliability is the correlation between REPEATS of the same kick
// at the same delay, so the creature's own spontaneous activity is the null
// rather than a confound: noise is uncorrelated across repeats by construction.
//
// Swept over the recurrent weight, because the interesting answer is not what
// the shipped genome does but whether ANY setting of it has a usable regime.
// DNA v14 already found that excitatory feedback between modules diverges and
// only inhibitory feedback settles; this asks the same question inside one.
namespace {

constexpr uint32_t kSqRepeats = 24;    // repeats of the identical kick
// 8 ms bins so one bin is one chain link and a wave advances exactly one group
// per column. At 10 ms against a 3 ms link the whole chain crossed inside two
// bins and the centre of activity read flat — the wave was real and the
// instrument could not see it.
constexpr uint32_t kSqBins = 24;
constexpr uint32_t kSqBinTicks = 8;    // 192 ms of aftermath
constexpr uint32_t kSqKickTicks = 10;
constexpr uint32_t kSqSettle = 20000;  // the rule from `stpprobe`: settle first

// Correlation between two population vectors.
double pop_corr(const std::vector<float>& a, const std::vector<float>& b) {
  const size_t n = a.size();
  if (n < 2 || b.size() != n) return 0.0;
  double ma = 0, mb = 0;
  for (size_t i = 0; i < n; ++i) { ma += a[i]; mb += b[i]; }
  ma /= double(n); mb /= double(n);
  double num = 0, da = 0, db = 0;
  for (size_t i = 0; i < n; ++i) {
    const double x = a[i] - ma, y = b[i] - mb;
    num += x * y; da += x * x; db += y * y;
  }
  return (da > 1e-12 && db > 1e-12) ? num / std::sqrt(da * db) : 0.0;
}

}  // namespace

bool run_seqprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna0;
  if (dna0.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const size_t mod_base = sizeof(aibaby::DnaHeader);
  // Prefers an `hvc` if the genome has one — a variant built by
  // tools/genome_add_hvc.py — so the same instrument can ask whether the chain
  // runs in the nucleus that was added for it. Without that, a null at the
  // larynx cannot be told from a chain that never fired.
  int32_t m_target = -1;
  for (uint32_t m = 0; m < dna0.module_count(); ++m) {
    if (std::strcmp(dna0.module(m).name, "central") == 0 && m_target < 0) m_target = int32_t(m);
    if (std::strcmp(dna0.module(m).name, "hvc") == 0) m_target = int32_t(m);
  }
  if (m_target < 0) {
    std::printf("  setup failed: no module named \"central\"\n");
    return false;
  }

  instrument("seqprobe", dna0.header().seed ^ 0x6B21u, ticks, "ticks");
  std::printf("  kick the HEAD of `central` — a contiguous block the size of one\n"
              "  chain link — remove the kick, and watch %u bins of\n"
              "  %u ms. Reliability is the correlation between REPEATS of the same\n"
              "  kick, so the creature's own spontaneous activity is uncorrelated by\n"
              "  construction and acts as the null.\n", kSqBins, kSqBinTicks);
  std::printf("  the first four arms sweep the SYMMETRIC weight — is the existing\n"
              "  wiring merely too weak? The last two switch on DNA v42's asymmetric\n"
              "  chain instead — does it need structure? Same creature, one run.\n");

  const float shipped = dna0.module(uint32_t(m_target)).weight_init;
  // The first four arms sweep the SYMMETRIC weight, which asks "is the existing
  // wiring merely too weak". The last two switch on DNA v42's asymmetric chain
  // instead, which asks "does it need structure". Both in one run, so the
  // comparison is against the same creature and the same kick.
  // The chain propagates two links and stops dead — 0.88, 0.17, 0.01 — and it
  // stops SOONER at higher chain weight, which points at the module's own
  // regulation rather than at the chain. Two things in `central` are fast
  // enough to do that on a 20 ms timescale, and both are patchable here without
  // touching the kernel:
  //
  //   inhib_gain 2.5   the chain excites the next group's inhibitory cells too,
  //                    which then suppress it locally. Feedforward inhibition
  //                    riding the wave.
  //   norm_gain 1.0    divisive normalisation, DNA v12, shipped ON for central.
  //                    A loud wave divides itself down.
  //
  // Each gets its own arm and then both together, so a partial effect is
  // attributable rather than a shrug.
  // `central` hatches at 400 neurons — `n_max` 4096 is the arena ceiling M4
  // growth may one day reach, not the live count, and reading it as the live
  // count is what made the first version of this sweep meaningless. 400 neurons
  // has to pay for BOTH properties a chain needs, and they pull opposite ways:
  //
  //   convergence   each cell of link k+1 needs tens of coincident inputs from
  //                 link k, which wants LARGE groups
  //   length        a syllable needs many links, which wants MANY groups
  //
  // At 64 per group there are six links and, at 3 ms each, a chain 18 ms long
  // end to end — which is why the shipped arm "persisted 20 ms". That was the
  // chain RUNNING TO ITS END, not dying at link two. So the length is bought
  // with the link delay instead, where there is room: `max_delay_ticks` is 32.
  struct Arm {
    const char* what;
    float chain, inhib_gain, norm_gain;
    uint32_t group;
    float density, delay_ms;
  };
  const Arm arms[] = {
      {"no chain",        0.00f, -1.0f, -1.0f, 64, 0.4f,  3.0f},
      {"6 links, 3 ms",   0.30f, -1.0f, -1.0f, 64, 0.4f,  3.0f},
      {"6 links, 16 ms",  0.30f, -1.0f, -1.0f, 64, 0.4f, 16.0f},
      {"12 links, 8 ms",  0.30f, -1.0f, -1.0f, 32, 0.8f,  8.0f},
      {"20 links, 8 ms",  0.30f, -1.0f, -1.0f, 20, 1.0f,  8.0f},
      {"12 links, no norm", 0.30f, -1.0f, 0.0f, 32, 0.8f, 8.0f},
  };
  const uint32_t n_scales = 6;

  std::printf("\n    %-8s %-8s %-9s %-9s %-11s %-11s %-10s\n", "w_rec", "w_chain",
              "rate Hz", "runaway", "persist ms", "reliab r", "changes r");
  bool any_usable = false;
  double best_persist = 0.0;

  for (uint32_t sc = 0; sc < n_scales; ++sc) {
    std::vector<uint8_t> variant = blob;
    const float w = shipped;
    const size_t mbase = mod_base + sizeof(aibaby::DnaModule) * size_t(m_target);
    std::memcpy(variant.data() + mbase + offsetof(aibaby::DnaModule, weight_init), &w,
                sizeof(w));
    std::memcpy(variant.data() + mbase + offsetof(aibaby::DnaModule, chain_weight),
                &arms[sc].chain, sizeof(float));
    if (arms[sc].inhib_gain >= 0.0f) {
      std::memcpy(variant.data() + mbase + offsetof(aibaby::DnaModule, inhib_gain),
                  &arms[sc].inhib_gain, sizeof(float));
    }
    if (arms[sc].norm_gain >= 0.0f) {
      std::memcpy(variant.data() + mbase + offsetof(aibaby::DnaModule, norm_gain),
                  &arms[sc].norm_gain, sizeof(float));
    }
    std::memcpy(variant.data() + mbase + offsetof(aibaby::DnaModule, chain_group),
                &arms[sc].group, sizeof(uint32_t));
    std::memcpy(variant.data() + mbase + offsetof(aibaby::DnaModule, chain_density),
                &arms[sc].density, sizeof(float));
    std::memcpy(variant.data() + mbase + offsetof(aibaby::DnaModule, chain_delay_ms),
                &arms[sc].delay_ms, sizeof(float));
    std::string error;
    Session s;
    if (!s.init(variant, error)) {
      std::printf("    %-17s failed to hatch: %s\n", arms[sc].what, error.c_str());
      continue;
    }
    aibaby::Network& net = s.brain.network();
    const aibaby::ModuleState& ms = net.module(uint32_t(m_target));
    if (sc == 0) {
      std::printf("  the chain module is %u neurons LIVE — `n_max` %u is the arena ceiling M4\n"
                  "  growth may reach, not the live count. Every chain length below is\n"
                  "  derived from 400, and each arm's kick is one link wide.\n",
                  ms.count, dna0.module(uint32_t(m_target)).n_max);
    }
    for (uint32_t t = 0; t < kSqSettle; ++t) s.brain.step();

    // One fixed kick pattern for every repeat — the same question asked of the
    // module 24 times.
    // THE HEAD OF THE CHAIN, not a random scatter. The first version kicked a
    // random 5% spread across all 64 groups, which fires a chain from everywhere
    // at once and leaves no wave to follow — it measured reliability rising to
    // 0.65 while persistence stayed pinned at one bin, and that pinning was the
    // stimulus, not the module. A contiguous block from the start of the module
    // is the same size of kick for every arm, chain or no chain, so the
    // comparison stays fair.
    aibaby::Rng rng;
    rng.seed(dna0.header().seed ^ 0x6B21u);
    std::vector<uint32_t> kick;
    const uint32_t head = arms[sc].group;
    for (uint32_t k = 0; k < head && k < ms.count; ++k) kick.push_back(ms.begin + k);

    // counts[repeat][bin][neuron]
    std::vector<std::vector<std::vector<float>>> counts(
        kSqRepeats, std::vector<std::vector<float>>(kSqBins, std::vector<float>(ms.count, 0.0f)));
    std::vector<std::vector<float>> during(kSqRepeats, std::vector<float>(ms.count, 0.0f));
    double base_spikes = 0.0;
    for (uint32_t t = 0; t < 2000; ++t) {
      s.brain.step();
      const uint32_t* fired = net.spikes();
      for (uint32_t f = 0; f < net.spike_count(); ++f) {
        const uint32_t id = fired[f];
        if (id >= ms.begin && id < ms.begin + ms.count) base_spikes += 1.0;
      }
    }
    const double base_rate = base_spikes / 2000.0 / double(ms.count) / 0.001;
    double rate_sum = 0.0;
    uint64_t rate_n = 0;

    for (uint32_t r = 0; r < kSqRepeats; ++r) {
      // Let the module return to its own baseline between repeats, or repeat
      // r+1 measures the tail of repeat r and every reliability is inflated.
      for (uint32_t t = 0; t < 400; ++t) s.brain.step();
      // THE POSITIVE CONTROL, and without it this probe cannot tell "the trace
      // dies" from "the kick never landed". The pattern during the kick itself
      // must be both loud and reproducible; if it is not, every zero below is
      // about the stimulus and not about the module.
      for (uint32_t t = 0; t < kSqKickTicks; ++t) {
        for (uint32_t idx : kick) net.inject(idx, aibaby::Scalar(2.0));
        s.brain.step();
        const uint32_t* fired = net.spikes();
        for (uint32_t f = 0; f < net.spike_count(); ++f) {
          const uint32_t id = fired[f];
          if (id >= ms.begin && id < ms.begin + ms.count) during[r][id - ms.begin] += 1.0f;
        }
      }
      for (uint32_t b = 0; b < kSqBins; ++b) {
        for (uint32_t t = 0; t < kSqBinTicks; ++t) {
          s.brain.step();
          // Walk the tick's spike list rather than testing every neuron: the
          // list is short and the module is 4096 wide.
          const uint32_t* fired = net.spikes();
          for (uint32_t f = 0; f < net.spike_count(); ++f) {
            const uint32_t id = fired[f];
            if (id >= ms.begin && id < ms.begin + ms.count) counts[r][b][id - ms.begin] += 1.0f;
          }
        }
        double bin_spikes = 0.0;
        for (uint32_t k = 0; k < ms.count; ++k) bin_spikes += counts[r][b][k];
        rate_sum += bin_spikes;
        ++rate_n;
      }
    }

    const double mean_rate = rate_n ? rate_sum / double(rate_n) / double(ms.count) /
                                          (double(kSqBinTicks) * 0.001) : 0.0;
    const bool runaway = mean_rate > 200.0;

    // Reliability per bin: mean pairwise correlation across repeats.
    double rel[kSqBins] = {};
    for (uint32_t b = 0; b < kSqBins; ++b) {
      double acc = 0.0;
      uint32_t n = 0;
      for (uint32_t i = 0; i < kSqRepeats; ++i) {
        for (uint32_t j = i + 1; j < kSqRepeats; ++j) {
          acc += pop_corr(counts[i][b], counts[j][b]);
          ++n;
        }
      }
      rel[b] = n ? acc / double(n) : 0.0;
    }
    // How long reliability stays above 0.1.
    uint32_t persist = 0;
    for (uint32_t b = 0; b < kSqBins; ++b) {
      if (rel[b] > 0.1) persist = b + 1; else break;
    }
    // Does the pattern CHANGE? Correlate the first bin's mean pattern with the
    // last reliable one: a frozen attractor reads ~1, a sequence reads low.
    std::vector<float> first(ms.count, 0.0f), last(ms.count, 0.0f);
    const uint32_t lb = persist ? persist - 1 : 0;
    for (uint32_t r = 0; r < kSqRepeats; ++r) {
      for (uint32_t k = 0; k < ms.count; ++k) {
        first[k] += counts[r][0][k];
        last[k] += counts[r][lb][k];
      }
    }
    // Meaningless when nothing persisted: `first` and `last` are then the same
    // bin and the correlation is 1.000 by construction, which reads as "a
    // frozen attractor" when it is really "no data".
    // Needs at least TWO bins: with one, `first` and `last` are the same bin
    // and the correlation is 1.000 by construction — which reads as "a frozen
    // attractor" when it is really "one bin of data". The persist==0 case had
    // this guard already; the persist==1 case slipped through and printed
    // exactly that spurious 1.000.
    const double changes = persist >= 2 ? pop_corr(first, last) : -2.0;

    double kick_r = 0.0;
    {
      double acc = 0.0;
      uint32_t n = 0;
      for (uint32_t i = 0; i < kSqRepeats; ++i) {
        for (uint32_t j = i + 1; j < kSqRepeats; ++j) { acc += pop_corr(during[i], during[j]); ++n; }
      }
      kick_r = n ? acc / double(n) : 0.0;
      double ks = 0.0;
      for (uint32_t i = 0; i < kSqRepeats; ++i)
        for (uint32_t k = 0; k < ms.count; ++k) ks += during[i][k];
      const double kick_rate = ks / double(kSqRepeats) / double(kSqKickTicks) /
                               double(ms.count) / 0.001;
      std::printf("    %-17s %-9.1f %-9s %-11u %-11.3f %-11s | kick %.0f->%.0f Hz r=%.2f\n",
                  arms[sc].what, mean_rate, runaway ? "SEIZED" : "no",
                  persist * kSqBinTicks,
                  rel[0], changes < -1.0 ? "-" : (std::snprintf(nullptr, 0, "%.3f", changes),
                  [&]{ static char buf[16]; std::snprintf(buf, sizeof(buf), "%.3f", changes);
                       return buf; }()),
                  base_rate, kick_rate, kick_r);
      std::printf("      reliability by 10 ms bin:");
      for (uint32_t b = 0; b < kSqBins; ++b) std::printf(" %.2f", rel[b]);
      std::printf("\n");
      // WHERE the activity is, in neuron index, bin by bin. This is the direct
      // test for a travelling wave and the correlation is not: a chain of
      // 64-neuron groups stepping every 3 ms should advance about 200 indices
      // per 10 ms bin, while a persistent blob sits still. `changes r` cannot
      // tell those apart — it reads 0.91 for a blob and would read low for a
      // wave, but it also reads low for noise.
      std::printf("      centre of activity, neuron index:");
      for (uint32_t b = 0; b < kSqBins; ++b) {
        double num = 0.0, den = 0.0;
        for (uint32_t r = 0; r < kSqRepeats; ++r) {
          for (uint32_t k = 0; k < ms.count; ++k) {
            num += double(k) * double(counts[r][b][k]);
            den += double(counts[r][b][k]);
          }
        }
        std::printf(" %5.0f", den > 0.0 ? num / den : -1.0);
      }
      std::printf("\n");
      if (kick_r < 0.3 || kick_rate < base_rate * 1.5) {
        std::printf("      ^ POSITIVE CONTROL FAILED at this weight: the kick did not\n"
                    "        make a loud reproducible pattern, so the zero above is\n"
                    "        about the stimulus, not the module.\n");
      }
    }
    if (!runaway && persist * kSqBinTicks >= 30 && changes < 0.9) {
      any_usable = true;
      best_persist = std::max(best_persist, double(persist * kSqBinTicks));
    }
  }

  std::printf("\n  `persist ms` is how long a repeat of the same kick still produces the\n"
              "  same population pattern; `reliab r` is that correlation in the first\n"
              "  bin; `changes r` compares the first bin's pattern with the last\n"
              "  reliable one — near 1 is a frozen attractor holding still, which is a\n"
              "  memory rather than a sequence.\n");
  if (!any_usable) {
    std::printf("\n  NO USABLE REGIME — at every recurrent weight tried, the module\n"
                "  either fails to carry its own activity forward or seizes. Local\n"
                "  recurrence as this kernel wires it cannot hold a sequence, so a\n"
                "  generator needs new structure and not a new constant.\n");
    return true;
  }
  std::printf("\n  THERE IS A REGIME — the kick survives %.0f ms with the pattern still\n"
              "  reproducible and still changing. This kernel can carry a sequence;\n"
              "  what it lacks is something that USES one.\n", best_persist);
  (void)verbose;
  return true;
}

}  // namespace aibaby_host
