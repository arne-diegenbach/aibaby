
// --- teachsound: can you TEACH the baby a sound, and can you HEAR that you
// --- did? -------------------------------------------------------------------
//
// `vocallearn` produced the largest effect anything in this project has had on
// the voice, and it produced it in the arm that was only ever meant to be a
// control. Rewarding the creature toward ONE formant target — no conditionality,
// just "closer than you usually get" — cuts its formant error by **18-36%
// against its own yoked control, on 3 of 3 seeds**. G2's ×1.35 on vocalisation
// RATE was this project's headline motor result; this is several times larger
// and it is about what the creature says rather than how often.
//
// Nobody has asked the only question that matters about it. **This project's own
// standard is that a classifier number means nothing until a listener can hear
// it** — the audibility ruler exists because "cube and ball produce
// distinguishable vocalisations" was true at 0.75 in a readout and inaudible to
// anyone. M1b is the one result that cleared that bar (d' 1.37). The teaching
// effect has never been tested against it at all.
//
// So this teaches the creature a vowel by praise alone and asks whether the
// change is AUDIBLE: d-prime between what it said in the first third of the
// session and what it said in the last, through the same two-formant tract and
// the same cochlea a listener would hear it with.
//
// Three things make it a test rather than a demonstration.
//
// **The yoked arm.** It receives the same praise and the same scolding in the
// same proportions at shifted times. Reward moves weights merely by arriving,
// and a creature that drifts on its own drifts in both arms.
//
// **The unbiased ruler, rooted once.** A squared Mahalanobis distance between
// two sample means is positively biased — for two identical piles it still
// reads D·(1/n0 + 1/n1). The correction is subtracted analytically and the
// quantity is carried as a signed d'^2, rooted at the end, because clamping and
// rooting per arm puts the bias straight back.
//
// **A 32-permutation null.** One shuffle is a draw from the null distribution,
// not an estimate of it.
//
// With `--wav` it also writes what the creature said early, what it said late,
// and the target it was being taught, so the number can be checked by ear.
namespace {

constexpr uint64_t kTSWordTicks = 900;
constexpr uint64_t kTSTrialTicks = 2800;
constexpr uint64_t kTSEchoFrom = kTSWordTicks + 200;
constexpr uint64_t kTSEchoTo = kTSWordTicks + 600;
constexpr uint64_t kTSRewardFrom = kTSWordTicks;
constexpr uint64_t kTSRewardTo = kTSWordTicks + 800;
constexpr double kTSBaselineAlpha = 0.02;
// The vowel being taught, and the vowel the caregiver keeps saying. They are
// DIFFERENT on purpose: the creature hears /a/ and is praised toward /i/, so a
// change toward the target cannot be the echo pathway doing its innate job.
constexpr uint32_t kTSHeard = 0;   // "ball", an open /a/
constexpr uint32_t kTSTarget = 1;  // "cube", a close /i/

struct TSRun {
  bool ok = false;
  double err_early = 0.0, err_late = 0.0;
  double dprime = 0.0, null = 0.0;
  uint32_t scored = 0, skipped = 0, praises = 0, scolds = 0;
  double f1_early = 0.0, f2_early = 0.0, f1_late = 0.0, f2_late = 0.0, amp = 0.0;
  std::vector<Praise> feedback;
};

TSRun run_teachsound_session(const std::vector<uint8_t>& blob, uint64_t ticks,
                             bool taught, const std::vector<Praise>* yoked,
                             const Regime& regime, Timbre* ruler, aibaby::Rng& rng) {
  TSRun out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return out;
  }
  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  Ear ear;
  if (!ear.configure(acfg, error)) {
    std::printf("  transducer failed: %s\n", error.c_str());
    return out;
  }
  VowelSource caregiver(acfg.sample_rate);
  std::vector<float> pcm(acfg.sample_rate / 1000);
  const uint32_t spt = acfg.sample_rate / 1000;
  const Word& heard = kWords[kTSHeard];
  const Word& target = kWords[kTSTarget];

  const uint32_t n_trials = uint32_t(ticks / kTSTrialTicks);
  if (n_trials < 60) return out;
  const uint32_t third = n_trials / 3;

  std::deque<Praise> pending;
  size_t yoke_cursor = 0;
  double baseline = -1.0;
  uint32_t last_frame = 0;
  uint64_t last_feedback = 0;
  double err_sum[2] = {}, f1_sum[2] = {}, f2_sum[2] = {}, amp_sum = 0.0;
  uint32_t err_n[2] = {}, amp_n = 0;
  std::vector<std::vector<double>> ceps;
  std::vector<int> when;

  for (uint32_t trial = 0; trial < n_trials; ++trial) {
    double f1_acc = 0.0, f2_acc = 0.0, a_acc = 0.0;
    uint32_t n_voiced = 0;
    for (uint64_t t = 0; t < kTSTrialTicks; ++t) {
      const uint64_t now = uint64_t(trial) * kTSTrialTicks + t;
      while (!pending.empty() && pending.front().tick <= now) {
        s.brain.praise(pending.front().value);
        pending.pop_front();
      }
      if (yoked) {
        while (yoke_cursor < yoked->size() && (*yoked)[yoke_cursor].tick <= now) {
          s.brain.praise((*yoked)[yoke_cursor].value);
          ++yoke_cursor;
        }
      }
      const bool sounding = t < kTSWordTicks;
      caregiver.render(sounding ? heard.f0 : 0.0f, heard.f1, heard.f2,
                       sounding ? 0.5f : 0.0f, pcm.data(), spt);
      ear.tick(s.brain, pcm.data(), spt);
      s.brain.step();

      if (s.brain.vocal_frame() == last_frame) continue;
      last_frame = s.brain.vocal_frame();
      const aibaby::VocalParams& v = s.brain.voice();
      const bool voiced = v.voicing > 0.5f && v.amplitude > kAmplitudeFloor;

      if (taught && voiced && t >= kTSRewardFrom && t < kTSRewardTo &&
          now - last_feedback >= regime.feedback_period) {
        const double e = formant_error(double(v.f1), double(v.f2), target);
        if (e >= 0.0) {
          last_feedback = now;
          if (baseline >= 0.0) {
            const float value = e < baseline ? regime.praise : regime.scold;
            if (value > 0.0f) ++out.praises; else ++out.scolds;
            pending.push_back(Praise{now + regime.delay, value});
            out.feedback.push_back(Praise{now + regime.delay, value});
          }
          baseline = baseline < 0.0 ? e : baseline + kTSBaselineAlpha * (e - baseline);
        }
      }

      if (t < kTSEchoFrom || t >= kTSEchoTo || !voiced) continue;
      ++n_voiced;
      f1_acc += double(v.f1);
      f2_acc += double(v.f2);
      a_acc += double(v.amplitude);
    }
    if (n_voiced == 0) { ++out.skipped; continue; }
    const double f1 = f1_acc / n_voiced, f2 = f2_acc / n_voiced, amp = a_acc / n_voiced;
    const double err = formant_error(f1, f2, target);
    if (err < 0.0) { ++out.skipped; continue; }
    ++out.scored;
    amp_sum += amp;
    ++amp_n;

    const int bin = trial < third ? 0 : (trial >= n_trials - third ? 1 : -1);
    if (bin < 0) continue;
    err_sum[bin] += err;
    f1_sum[bin] += f1;
    f2_sum[bin] += f2;
    ++err_n[bin];
    // What a listener would hear: the posture rendered through the same tract
    // and the same cochlea, as a cepstrum. Not the motor parameters — a
    // classifier on those is exactly the kind of number the audibility ruler
    // exists to distrust.
    if (ruler) {
      std::vector<double> c = ruler->of(double(s.dna.header().vocal.f0_min), f1, f2, amp);
      if (!c.empty()) { ceps.push_back(c); when.push_back(bin); }
    }
  }

  for (uint32_t b = 0; b < 2; ++b) {
    const double n = err_n[b] ? double(err_n[b]) : 1.0;
    (b ? out.err_late : out.err_early) = err_sum[b] / n;
    (b ? out.f1_late : out.f1_early) = f1_sum[b] / n;
    (b ? out.f2_late : out.f2_early) = f2_sum[b] / n;
  }
  out.amp = amp_n ? amp_sum / amp_n : 0.0;

  if (ceps.size() >= 24) {
    const double d2 = cepstral_dprime(ceps, when, nullptr, true);
    out.dprime = d2 >= 0.0 ? std::sqrt(d2) : -std::sqrt(-d2);
    double null_sum = 0.0;
    for (uint32_t p = 0; p < 32; ++p) {
      std::vector<int> sh = when;
      for (size_t i = sh.size(); i > 1; --i) std::swap(sh[i - 1], sh[rng.next() % i]);
      const double nd = cepstral_dprime(ceps, sh, nullptr, true);
      null_sum += nd >= 0.0 ? std::sqrt(nd) : -std::sqrt(-nd);
    }
    out.null = null_sum / 32.0;
  }
  out.ok = out.scored >= 48 && err_n[0] > 0 && err_n[1] > 0;
  return out;
}

}  // namespace

bool run_teachsound(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose,
                    const Capture& cap) {
  Regime regime;
  regime.praise = kPraiseValue;
  regime.scold = kScoldValue;
  aibaby::Dna dna0;
  if (dna0.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  std::string error;
  Timbre ruler;
  if (!ruler.configure(dna0.header().audio, error)) {
    std::printf("  the audibility ruler failed to configure: %s\n", error.c_str());
    return false;
  }
  aibaby::Rng rng;
  rng.seed(dna0.header().seed ^ 0x7EAC0u);

  instrument("teachsound", dna0.header().seed, ticks / kTSTrialTicks, "trials per arm");
  std::printf("  the caregiver says \"%s\" and praises the creature toward \"%s\" —\n"
              "  a DIFFERENT vowel, so a shift toward the target cannot be the echo\n"
              "  pathway doing its innate job.\n",
              kTSHeard == 0 ? "ball" : "cube", kTSTarget == 0 ? "ball" : "cube");
  std::printf("  target formants   F1 %.0f Hz, F2 %.0f Hz\n",
              double(kWords[kTSTarget].f1), double(kWords[kTSTarget].f2));

  const TSRun taught = run_teachsound_session(blob, ticks, true, nullptr, regime,
                                              &ruler, rng);
  if (!taught.ok) {
    std::printf("\n  inconclusive: %u trials scored, %u skipped. A creature that does\n"
                "  not vocalise has nothing to be taught.\n", taught.scored,
                taught.skipped);
    return false;
  }
  std::vector<Praise> yoke = taught.feedback;
  for (Praise& p : yoke) p.tick += kTSTrialTicks / 2;
  const TSRun yoked = run_teachsound_session(blob, ticks, false, &yoke, regime,
                                             &ruler, rng);

  std::printf("\n    %-9s %-8s %-11s %-11s %-9s %-11s %-11s %-9s %-8s\n", "arm",
              "scored", "err early", "err late", "change", "F1 early", "F1 late",
              "d' early/late", "null");
  const TSRun* arms[2] = {&taught, &yoked};
  const char* names[2] = {"taught", "yoked"};
  double change[2] = {};
  for (uint32_t a = 0; a < 2; ++a) {
    const TSRun& r = *arms[a];
    change[a] = r.err_early > 0.0 ? 100.0 * (1.0 - r.err_late / r.err_early) : 0.0;
    std::printf("    %-9s %-8u %-11.4f %-11.4f %+-9.1f %-11.0f %-11.0f %-9.3f %-8.3f\n",
                names[a], r.scored, r.err_early, r.err_late, change[a], r.f1_early,
                r.f1_late, r.dprime, r.null);
  }
  std::printf("\n    F2   taught  %.0f -> %.0f Hz    target %.0f\n"
              "         yoked   %.0f -> %.0f Hz\n",
              taught.f2_early, taught.f2_late, double(kWords[kTSTarget].f2),
              yoked.f2_early, yoked.f2_late);

  std::printf("\n  `change` is how much the formant error toward the TAUGHT vowel fell\n"
              "  from the first third of the session to the last. d' is between what the\n"
              "  creature said early and what it said late, through its own tract and\n"
              "  cochlea, bias-corrected and rooted once, against a 32-permutation null.\n"
              "  At d' = 1 a listener gets about 76%% right in a two-alternative forced\n"
              "  choice, which is the bar M1b cleared at 1.37.\n");
  std::printf("\n  error moved      taught %+.1f%%   yoked %+.1f%%   (%+.1f points)\n"
              "  audible change   taught d' %.3f against a null of %.3f\n"
              "                   yoked  d' %.3f against a null of %.3f\n",
              change[0], change[1], change[0] - change[1], taught.dprime, taught.null,
              yoked.dprime, yoked.null);

  if (!cap.wav.empty()) {
    // Three steady vowels a second apart: what it said early, what it said late,
    // and what it was being taught. The number is checkable by ear or it is not
    // a claim about sound.
    const uint32_t sr = dna0.header().audio.sample_rate;
    VowelSource v(sr);
    std::vector<float> out;
    auto say = [&](double f1, double f2, double amp) {
      std::vector<float> buf(sr / 2, 0.0f);
      v.render(float(dna0.header().vocal.f0_min), float(f1), float(f2), float(amp),
               buf.data(), buf.size());
      out.insert(out.end(), buf.begin(), buf.end());
      out.insert(out.end(), sr / 4, 0.0f);
    };
    say(taught.f1_early, taught.f2_early, taught.amp);
    say(taught.f1_late, taught.f2_late, taught.amp);
    say(double(kWords[kTSTarget].f1), double(kWords[kTSTarget].f2), 0.5);
    const std::string path = cap.wav + ".taught.wav";
    if (write_wav(path, out, 1, sr, error)) {
      std::printf("\n  wrote %s — early, late, then the target, half a second each.\n",
                  path.c_str());
    } else {
      std::printf("\n  could not write %s: %s\n", path.c_str(), error.c_str());
    }
  }

  const bool moved = change[0] > change[1] + 5.0;
  const bool audible = taught.dprime > taught.null + 1.0 &&
                       taught.dprime > yoked.dprime + 0.5;
  if (!moved) {
    std::printf("\n  NOT TAUGHT — the formant error did not fall further than in the\n"
                "  yoked arm. Nothing was learned, so there is nothing to hear.\n");
  } else if (!audible) {
    std::printf("\n  TAUGHT BUT NOT AUDIBLE — the error fell %+.1f points against the\n"
                "  yoke and a listener could not tell the early utterances from the\n"
                "  late ones (d' %.3f against a null of %.3f). This is the exact gap\n"
                "  the audibility ruler exists to catch: a readout moved and the sound\n"
                "  did not.\n", change[0] - change[1], taught.dprime, taught.null);
  } else {
    std::printf("\n  TAUGHT, AND YOU CAN HEAR IT — the error fell %+.1f points against\n"
                "  its own yoked control and the creature's late utterances are audibly\n"
                "  different from its early ones: d' %.3f against a 32-permutation null\n"
                "  of %.3f, where the yoked arm reads %.3f. Praise alone moved what this\n"
                "  creature says, and a listener can tell.\n",
                change[0] - change[1], taught.dprime, taught.null, yoked.dprime);
  }
  (void)verbose;
  return moved && audible;
}
