
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
          res[k] += std::fabs(double(net.apical(i)));
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
  const double settled =
      learn.resid_early > 0.0 ? 100.0 * (1.0 - learn.resid_late / learn.resid_early) : 0.0;
  const double moved =
      learn.ffi_early > 0.0 ? learn.ffi_late / learn.ffi_early : 1.0;

  std::printf("\n  resid is the mean |apical| at the larynx. `ffi w` is the mean pooling\n"
              "  weight, and it is the control on the residual: a residual that fell\n"
              "  while the weight did not move is the creature going quiet, not the\n"
              "  circuit learning. obj|resid classifies cube against ball from the\n"
              "  per-neuron residual, against a 32-permutation null.\n");
  std::printf("\n  residual settled       %+.1f%%   (the learning arm must reduce it)\n"
              "  pooling weight moved   x%.3f   (and must have moved to do it)\n"
              "  obj: learn vs fixed    %.3f vs %.3f   (the payoff, not a check)\n"
              "  raw plateau reference  0.913 from burstprobe, three seeds\n",
              settled, moved, learn.obj_resid, fixed.obj_resid);

  const bool control_ok = std::fabs(soma.ffi_late - soma.ffi_early) < 1e-6 &&
                          std::fabs(fixed.ffi_late - fixed.ffi_early) < 1e-6;
  const bool ran = learn.plateau_pct > 0.5 && learn.trials >= 12;
  const bool learned = settled > 5.0 && moved > 1.05;
  if (!control_ok) {
    std::printf("\n  FAIL — a fixed-weight arm's pooling weight moved. ffi_learn 0 is not\n"
                "  off, so neither control is one.\n");
  } else if (!ran) {
    std::printf("\n  FAIL — the compartment never ran (%.1f%% plateau, %zu trials), so no\n"
                "  column above is a measurement of anything.\n",
                learn.plateau_pct, learn.trials);
  } else if (!learned) {
    std::printf("\n  The microcircuit did NOT cancel: residual %+.1f%%, weight x%.3f.\n"
                "  Sacramento's interneuron settles when its prediction can track the\n"
                "  top-down input; if the weight moved and the residual did not, one\n"
                "  scalar per neuron cannot predict this tract — which would be the\n"
                "  same shape as v24's shared-scalar failure one level up. Reported,\n"
                "  not fatal.\n", settled, moved);
  } else {
    std::printf("\n  PASS — the interneuron learned to cancel: the residual fell %.1f%%\n"
                "  and the weight moved x%.3f to do it. Whether the cleaner signal is a\n"
                "  BETTER one is obj|resid, and that is a result rather than a check.\n",
                settled, moved);
  }
  (void)verbose;
  return control_ok && ran;
}
