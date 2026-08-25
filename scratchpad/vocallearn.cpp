
// --- vocallearn: does the echo get BETTER with practice? -------------------
//
// M1b is the result in this project that works: the creature repeats a heard
// word at 0.890 with an audible d-prime of 1.37, the only number here that
// clears the audibility bar. Nothing has ever been built on it. Eleven
// mechanisms have been aimed at G3, which is settled negative, and none at the
// capability that already exists.
//
// The question M1b raises and nobody has asked: **the creature imitates — does
// it get better at it?** That is vocal learning, and it is what the songbird
// literature this project already borrows from (LMAN, DNA v10) is actually
// about: motor variability selected by how close a rendition lands to a
// template.
//
// **Why this is a fair question and G3 is not.** Improving the echo does not
// require the creature to LEARN a conditional mapping — M1b measured that it
// already has one, delivered by the ear-to-larynx route. It requires the
// existing mapping to be refined toward the heard formants. And the rule that
// would do it is the one thing in this project that has ever worked: node
// perturbation met G2, so reward can shape a motor act here. It has only ever
// been scored on how MUCH the creature vocalises, never on how well.
//
// The design is G2's, with accuracy in place of rate:
//
//   taught  reward follows the creature's own echo — praise when this trial
//           landed closer than it usually does on THIS word, a mild no when
//           further.
//   yoked   the same praise and the same scolding in the same proportions, at
//           shifted times, for nothing it did. G2's control, and the only one
//           that holds mean weight and reward count fixed.
//   none    no feedback at all, because the yoked arm still receives reward and
//           reward moves weights. This is the arm that says whether the error
//           drifts on its own.
//
// **The baseline is per word, and that is not a detail.** Against one global
// running mean, a word whose natural posture happens to sit closer to its
// formants would earn praise every time, and the creature would be rewarded for
// word identity rather than for accuracy — it would learn to say the easy word.
// Against a per-word mean, reward can only ever mean "closer than you usually
// get to THIS one".
namespace {

// Where the echo lives. M1b measured it: 200-600 ms after the word stops, the
// ear reads at chance and the voice still carries the word at 0.890. Scoring
// while the word plays would score the caregiver.
constexpr uint64_t kVLWordTicks = 900;
constexpr uint64_t kVLTrialTicks = 2800;
constexpr uint64_t kVLEchoFrom = kVLWordTicks + 200;
constexpr uint64_t kVLEchoTo = kVLWordTicks + 600;
constexpr uint32_t kVLWords = 2;
// How fast the per-word expectation follows. Slow enough that a run of good
// trials does not immediately raise the bar out of reach, fast enough that it
// tracks a creature that is genuinely improving.
constexpr double kVLBaselineAlpha = 0.15;

enum VLArm { kVLTaught = 0, kVLYoked, kVLNone, kVLArmCount };

struct VLRun {
  bool ok = false;
  double err_early = 0.0, err_late = 0.0;
  double err_by_word[kVLWords][2] = {};
  uint32_t scored = 0, skipped = 0, praises = 0, scolds = 0;
  double voiced_frac = 0.0;
  std::vector<Praise> feedback;  // what the taught arm earned, for the yoke
};

// Distance between what the creature said and what it heard, in log formant
// space. Log because a formant difference of 100 Hz means something very
// different at 300 Hz and at 2500 Hz, and the two words this is scored on are
// 780/1180 against 320/2500.
inline double formant_error(double f1, double f2, const Word& w) {
  if (f1 <= 1.0 || f2 <= 1.0) return -1.0;
  return std::fabs(std::log(f1 / double(w.f1))) + std::fabs(std::log(f2 / double(w.f2)));
}

VLRun run_vocallearn_session(const std::vector<uint8_t>& blob, uint64_t ticks, VLArm arm,
                             const std::vector<Praise>* yoked, const Regime& regime) {
  VLRun out;
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

  const uint32_t n_trials = uint32_t(ticks / kVLTrialTicks);
  if (n_trials < 24) return out;
  const uint32_t third = n_trials / 3;

  std::deque<Praise> pending;
  size_t yoke_cursor = 0;
  double baseline[kVLWords] = {-1.0, -1.0};
  double err_sum[2] = {}, voiced_sum = 0.0;
  uint32_t err_n[2] = {}, frames_total = 0, frames_voiced = 0;
  double word_sum[kVLWords][2] = {};
  uint32_t word_n[kVLWords][2] = {};
  uint32_t last_frame = 0;

  for (uint32_t trial = 0; trial < n_trials; ++trial) {
    const uint32_t label = trial % kVLWords;
    const Word& w = kWords[label];
    double f1_sum = 0.0, f2_sum = 0.0;
    uint32_t n_voiced = 0;

    for (uint64_t t = 0; t < kVLTrialTicks; ++t) {
      const uint64_t now = uint64_t(trial) * kVLTrialTicks + t;
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
      const bool sounding = t < kVLWordTicks;
      caregiver.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                       pcm.data(), spt);
      ear.tick(s.brain, pcm.data(), spt);
      s.brain.step();

      if (s.brain.vocal_frame() == last_frame) continue;
      last_frame = s.brain.vocal_frame();
      if (t < kVLEchoFrom || t >= kVLEchoTo) continue;
      ++frames_total;
      const aibaby::VocalParams& v = s.brain.voice();
      if (v.voicing <= 0.5f || v.amplitude <= kAmplitudeFloor) continue;
      ++frames_voiced;
      ++n_voiced;
      f1_sum += double(v.f1);
      f2_sum += double(v.f2);
    }

    // A trial in which the creature said nothing has no accuracy to score and
    // must not be counted as a bad one: silence is not a wrong answer, and
    // scoring it as maximum error would make "say less" the winning strategy.
    if (n_voiced == 0) { ++out.skipped; continue; }
    const double err = formant_error(f1_sum / n_voiced, f2_sum / n_voiced, w);
    if (err < 0.0) { ++out.skipped; continue; }
    ++out.scored;

    const int bin = trial < third ? 0 : (trial >= n_trials - third ? 1 : -1);
    if (bin >= 0) {
      err_sum[bin] += err;
      ++err_n[bin];
      word_sum[label][bin] += err;
      ++word_n[label][bin];
    }

    // Feedback, and only in the taught arm. Against this word's own running
    // expectation, so praise means "closer than you usually get to this one".
    if (arm == kVLTaught) {
      if (baseline[label] >= 0.0) {
        const float value = err < baseline[label] ? regime.praise : regime.scold;
        if (value > 0.0f) ++out.praises; else ++out.scolds;
        const uint64_t at = uint64_t(trial) * kVLTrialTicks + kVLEchoTo + regime.delay;
        pending.push_back(Praise{at, value});
        out.feedback.push_back(Praise{at, value});
      }
      baseline[label] = baseline[label] < 0.0
                            ? err
                            : baseline[label] + kVLBaselineAlpha * (err - baseline[label]);
    }
  }

  out.err_early = err_n[0] ? err_sum[0] / err_n[0] : 0.0;
  out.err_late = err_n[1] ? err_sum[1] / err_n[1] : 0.0;
  for (uint32_t k = 0; k < kVLWords; ++k) {
    for (uint32_t b = 0; b < 2; ++b) {
      out.err_by_word[k][b] = word_n[k][b] ? word_sum[k][b] / word_n[k][b] : 0.0;
    }
  }
  out.voiced_frac = frames_total ? double(frames_voiced) / double(frames_total) : 0.0;
  (void)voiced_sum;
  out.ok = out.scored >= 18 && err_n[0] > 0 && err_n[1] > 0;
  return out;
}

}  // namespace

bool run_vocallearn(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  Regime regime;
  regime.praise = kPraiseValue;
  regime.scold = kScoldValue;

  aibaby::Dna dna0;
  if (dna0.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  instrument("vocallearn", dna0.header().seed, ticks / kVLTrialTicks, "trials per arm");
  std::printf("  the caregiver says one of two words; the creature's echo is scored\n"
              "  %llu-%llu ms after the word stops — M1b's window, where the ear reads\n"
              "  at chance and the voice still carries the word at 0.890.\n",
              (unsigned long long)(kVLEchoFrom - kVLWordTicks),
              (unsigned long long)(kVLEchoTo - kVLWordTicks));
  std::printf("  error is |log(f1/heard f1)| + |log(f2/heard f2)| over the voiced\n"
              "  frames of that window. Praise when a trial lands closer than this\n"
              "  creature usually gets to THAT word, a mild no when further.\n");

  const VLRun taught = run_vocallearn_session(blob, ticks, kVLTaught, nullptr, regime);
  if (!taught.ok) {
    std::printf("\n  inconclusive: the taught arm scored %u trials and skipped %u.\n"
                "  A creature that does not vocalise in the echo window has no\n"
                "  accuracy to improve, and this is not a measurement of whether it\n"
                "  could. Try --ticks higher, or a genome that babbles more.\n",
                taught.scored, taught.skipped);
    return false;
  }
  // The yoke replays exactly what the taught arm earned, shifted by half a
  // trial so it cannot line up with this creature's own echoes.
  std::vector<Praise> yoke = taught.feedback;
  for (Praise& p : yoke) p.tick += kVLTrialTicks / 2;
  const VLRun yoked = run_vocallearn_session(blob, ticks, kVLYoked, &yoke, regime);
  const VLRun none = run_vocallearn_session(blob, ticks, kVLNone, nullptr, regime);

  std::printf("\n    %-9s %-8s %-9s %-11s %-11s %-9s %-9s %-8s\n", "arm", "scored",
              "skipped", "err early", "err late", "change", "rewards", "voiced");
  const VLRun* arms[kVLArmCount] = {&taught, &yoked, &none};
  const char* names[kVLArmCount] = {"taught", "yoked", "none"};
  double change[kVLArmCount] = {};
  for (uint32_t a = 0; a < kVLArmCount; ++a) {
    const VLRun& r = *arms[a];
    change[a] = r.err_early > 0.0 ? 100.0 * (1.0 - r.err_late / r.err_early) : 0.0;
    std::printf("    %-9s %-8u %-9u %-11.4f %-11.4f %+-9.1f %-9u %-8.2f\n", names[a],
                r.scored, r.skipped, r.err_early, r.err_late, change[a],
                r.praises + r.scolds, r.voiced_frac);
  }

  std::printf("\n    per word, taught arm    early      late       change\n");
  for (uint32_t k = 0; k < kVLWords; ++k) {
    const double e = taught.err_by_word[k][0], l = taught.err_by_word[k][1];
    std::printf("    %-23s %-10.4f %-10.4f %+.1f%%\n",
                k == 0 ? "\"ball\" /a/" : "\"cube\" /i/", e, l,
                e > 0.0 ? 100.0 * (1.0 - l / e) : 0.0);
  }

  std::printf("\n  `change` is how much the echo's formant error fell from the first\n"
              "  third of the session to the last. The YOKED arm is the criterion, not\n"
              "  zero: it receives the same praise and the same scolding in the same\n"
              "  proportions for nothing it did, so anything reward does to a brain\n"
              "  merely by arriving happens in both.\n");
  std::printf("\n  taught  %+.1f%%\n  yoked   %+.1f%%\n  none    %+.1f%%\n"
              "  taught - yoked   %+.1f points\n",
              change[0], change[1], change[2], change[0] - change[1]);

  const bool instrument_ok = taught.praises + taught.scolds >= 12 &&
                             yoked.praises + yoked.scolds == 0 && yoked.ok && none.ok;
  const bool learned = change[0] > change[1] + 5.0 && change[0] > 0.0;
  if (!instrument_ok) {
    std::printf("\n  INCONCLUSIVE — the arms are not comparable: taught delivered %u\n"
                "  rewards, and the yoked arm must earn none of its own (%u).\n",
                taught.praises + taught.scolds, yoked.praises + yoked.scolds);
    return false;
  }
  if (learned) {
    std::printf("\n  VOCAL LEARNING — the echo improved %+.1f points more than its own\n"
                "  yoked control. Reward can shape not just how much this creature\n"
                "  vocalises but how accurately, and that is a capability nothing in\n"
                "  this project has measured before.\n",
                change[0] - change[1]);
  } else {
    std::printf("\n  NOT MET — the taught arm did not beat its yoke by the 5 points this\n"
                "  asks for. Read it against what is already known: node perturbation\n"
                "  moves a per-neuron BIAS, which is a constant, and G2 was met by\n"
                "  shifting an entire posture in one direction. Steering the SAME\n"
                "  larynx to two different targets depending on what was heard is a\n"
                "  conditional act, and this creature's conditional route is the\n"
                "  ear-to-larynx one it was born with rather than one reward can\n"
                "  reshape.\n");
  }
  (void)verbose;
  return learned;
}
