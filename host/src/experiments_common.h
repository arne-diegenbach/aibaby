// Shared scaffolding for the experiments, which live in three files:
//
//   experiments_milestones.cpp  the numbered goals and the path checks
//   experiments_probes.cpp      the diagnostics that ask why a goal failed
//   experiments.cpp             the dispatcher, and nothing else
//
// Everything here is used by more than one of them: the session wrapper, the
// held-out classifiers and their controls, the caregiver's two words, the two
// toys, and the constants that define a trial. It was one 5000-line anonymous
// namespace until 2026-08-15; the split is by *what a reader is looking for*,
// not by size.
//
// Marked inline throughout rather than moved into a .cpp, because these are
// small and the alternative is a fourth file whose only job is to hold them.

#ifndef AIBABY_HOST_EXPERIMENTS_COMMON_H
#define AIBABY_HOST_EXPERIMENTS_COMMON_H

#include "host/experiments.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <deque>
#include <fstream>

#include "aibaby/brain.h"
#include "aibaby/snapshot.h"
#include "host/audio.h"
#include "host/mel.h"
#include "host/snapshot_file.h"
#include "host/vision.h"
#include "host/wav.h"

namespace aibaby_host {

// A vocalisation is a voiced burst above an amplitude floor. The refractory
// period is what makes it an *event*: without it, one two-second wail would
// be counted two hundred times and every rate in the report would be noise.
constexpr float kAmplitudeFloor = 0.45f;

constexpr uint64_t kEventRefractoryTicks = 200;


// Human praise does not arrive instantly. Delaying it is the point: an
// eligibility trace with tau_e = 2 s is the mechanism that is supposed to
// survive this, and a test that rewards on the same tick would not test it.
constexpr uint64_t kRewardDelayTicks = 500;


// How often the caregiver comments while the baby is making a sound. A single
// impulse per vocalisation is a terrible training signal: the eligibility
// trace spans two seconds, and one instantaneous readout occupies about five
// percent of it, so nineteen twentieths of what gets credited is unrelated.
// A caregiver who keeps saying "goood baaaby" for as long as the cooing lasts
// is both more realistic and vastly better aligned with the trace.
constexpr uint64_t kFeedbackPeriodTicks = 150;

constexpr float kPraiseValue = 0.5f;

constexpr float kScoldValue = -0.5f;


// How the caregiver behaves. The realistic setting is what G2 is scored on;
// the dense/immediate setting is a diagnostic ceiling — it answers "can reward
// move this behaviour at all", separately from "can it do so through a delay".
struct Regime {
  uint64_t feedback_period = kFeedbackPeriodTicks;
  uint64_t delay = kRewardDelayTicks;
  float praise = kPraiseValue;
  float scold = kScoldValue;
};


// --- Listening to the creature ---------------------------------------------
//
// A held-out classifier is the only honest way to settle whether two
// vocalisations differ, and it is also completely mute about what the baby
// sounds like. This renders the vocal tract to PCM alongside the run so the
// question can be asked with ears as well.
//
// Two properties matter more than the audio quality:
//
//   - It changes nothing. The synthesiser has its own filter state and draws
//     no random numbers, so a recorded creature is bit-identical to the same
//     creature unrecorded. Recording is an observation, not a condition.
//   - It is not what the baby hears itself say. Nothing in this file feeds
//     back into the brain; the creature has no ear on its own larynx (§5.3).
//
// The voice is VowelSource, the same two-formant synthesiser the caregiver
// speaks through — so a recording is a fair rendering of f0, F1, F2 and
// loudness, but not of the third formant or the bandwidths the browser's
// three-resonator version also has. It sounds like the panel, not identical
// to it.
struct VoiceRecorder {
  VoiceRecorder(uint32_t sample_rate, bool with_caregiver)
      : rate(sample_rate), stereo(with_caregiver), larynx(sample_rate) {}

  uint32_t rate;
  // Stereo when there is a caregiver to hear: baby left and caregiver right,
  // so an echo can be heard as the two voices it is — what was said, and what
  // came back. Babbling alone in a quiet room is mono, because the other
  // channel would be nothing but silence.
  bool stereo;
  VowelSource larynx;
  std::vector<float> timeline;
  // The probe windows alone, split by which toy was in view. This is the
  // milestone's listening test — the classifier's 50% has to be audible as
  // "these two piles sound the same" or the number is not believable.
  std::vector<float> probe[2];
  // An Audacity-format label track for the timeline: start, end, text.
  std::string labels;

  void segment(double from, double to, const char* text) {
    char line[96];
    std::snprintf(line, sizeof(line), "%.3f\t%.3f\t%s\n", from, to, text);
    labels += line;
  }

  // One tick of the baby's larynx, and the same tick of the room.
  void tick(const aibaby::VocalParams& v, const float* caregiver, size_t count,
            std::vector<float>* probe_bin) {
    scratch.resize(count);
    // Voicing is a valve: shut, the glottis makes no pulses at all, and an
    // amplitude that has not finished ramping down should not leak through it.
    const float f0 = v.voicing > 0.5f ? float(v.f0) : 0.0f;
    larynx.render(f0, float(v.f1), float(v.f2), float(v.amplitude), scratch.data(), count);
    for (size_t i = 0; i < count; ++i) {
      timeline.push_back(scratch[i]);
      if (stereo) timeline.push_back(caregiver ? caregiver[i] : 0.0f);
    }
    if (probe_bin) probe_bin->insert(probe_bin->end(), scratch.begin(), scratch.end());
  }

  // This trial's probe audio, not yet committed to a pile.
  std::vector<float> pending;
  std::vector<float> scratch;
};


// Peak-normalises one interleaved channel in place. Recording levels here are
// arbitrary — the resonators have whatever gain they have — so a file written
// raw is usually inaudible. `scale` of 0 means "work it out from this buffer";
// passing a shared scale keeps a loudness difference between two files, which
// matters because loudness is one of the features the milestone classifies on.
inline float normalise(std::vector<float>& samples, uint32_t stride, uint32_t offset,
                float scale) {
  if (samples.empty()) return scale;
  if (scale <= 0.0f) {
    float peak = 0.0f;
    for (size_t i = offset; i < samples.size(); i += stride) {
      const float a = std::fabs(samples[i]);
      if (a > peak) peak = a;
    }
    if (peak <= 1e-6f) return 0.0f;
    scale = 0.9f / peak;
  }
  for (size_t i = offset; i < samples.size(); i += stride) samples[i] *= scale;
  return scale;
}


// What a session was asked to leave behind. Empty strings mean "nothing", so
// an experiment that is only being scored pays for none of this.
struct Capture {
  std::string wav;   // path prefix for the recordings
  std::string save;  // path for a snapshot of the creature at the end
  bool wanted() const { return !wav.empty() || !save.empty(); }
};


// Everything a captured session leaves on disk. Failures here are warnings,
// not errors: a run that measured the milestone correctly and then could not
// write a wav has still measured the milestone, and turning that into a red
// experiment would be a lie about what failed.
inline void write_capture(const Capture& cap, VoiceRecorder& rec, uint32_t sample_rate,
                   const aibaby::Brain& brain, const std::vector<uint8_t>& blob) {
  std::string error;

  if (!cap.wav.empty()) {
    const uint32_t channels = rec.stereo ? 2u : 1u;
    const double seconds = double(rec.timeline.size() / channels) / double(sample_rate);
    normalise(rec.timeline, channels, 0, 0.0f);                    // the baby
    if (rec.stereo) normalise(rec.timeline, channels, 1, 0.0f);    // the caregiver
    const std::string session = cap.wav + ".wav";
    if (write_wav(session, rec.timeline, channels, sample_rate, error)) {
      std::printf("  wrote %-28s %6.1f s  %s\n", session.c_str(), seconds,
                  rec.stereo ? "stereo: baby left, caregiver right" : "the baby, alone");
    } else {
      std::printf("  WARNING  %s\n", error.c_str());
    }

    if (!rec.labels.empty()) {
      const std::string labels = cap.wav + ".labels.txt";
      std::ofstream lf(labels, std::ios::trunc);
      if (lf) {
        lf << rec.labels;
        std::printf("  wrote %-28s %6zu segments  (Audacity label track)\n", labels.c_str(),
                    size_t(std::count(rec.labels.begin(), rec.labels.end(), '\n')));
      } else {
        std::printf("  WARNING  cannot write %s\n", labels.c_str());
      }
    }

    // The two piles of probes share one gain. Loudness is one of the features
    // the milestone classifies on, so normalising them separately would erase
    // a real difference — and manufacture the impression of one.
    float peak = 0.0f;
    for (int o = 0; o < 2; ++o) {
      for (float v : rec.probe[o]) {
        const float a = std::fabs(v);
        if (a > peak) peak = a;
      }
    }
    const float scale = peak > 1e-6f ? 0.9f / peak : 0.0f;
    const char* toy[2] = {"ball", "cube"};
    for (int o = 0; o < 2; ++o) {
      if (rec.probe[o].empty()) continue;
      if (scale > 0.0f) normalise(rec.probe[o], 1, 0, scale);
      const std::string path = cap.wav + "." + toy[o] + ".wav";
      if (write_wav(path, rec.probe[o], 1, sample_rate, error)) {
        std::printf("  wrote %-28s %6.1f s  every %s probe, back to back\n", path.c_str(),
                    double(rec.probe[o].size()) / double(sample_rate), toy[o]);
      } else {
        std::printf("  WARNING  %s\n", error.c_str());
      }
    }
  }

  if (!cap.save.empty()) {
    if (save_brain(cap.save, brain, blob, error)) {
      std::printf("  wrote %-28s tick %llu — resume it with --snapshot\n", cap.save.c_str(),
                  (unsigned long long)brain.network().tick());
    } else {
      std::printf("  WARNING  %s\n", error.c_str());
    }
  }
}


struct Session {
  std::vector<uint8_t> memory;
  aibaby::Brain brain;
  aibaby::Dna dna;

  bool init(const std::vector<uint8_t>& blob, std::string& error) {
    const aibaby::DnaStatus ds = dna.load(blob.data(), blob.size());
    if (ds != aibaby::DnaStatus::kOk) {
      error = std::string("genome rejected: ") + aibaby::dna_status_string(ds);
      return false;
    }
    memory.assign(aibaby::Brain::required_bytes(dna), 0);
    const aibaby::BrainStatus bs =
        brain.init(blob.data(), blob.size(), memory.data(), memory.size());
    if (bs != aibaby::BrainStatus::kOk) {
      error = "brain init failed (" + std::to_string(int(bs)) + ")";
      return false;
    }
    // The live path prints this in its startup banner; an experiment used to
    // hatch in silence. An over-subscribed genome is exactly the kind of thing
    // that shows up as a plausible failed result rather than as an error — and
    // a dropped *reverse* entry is the worst kind, because that synapse still
    // depresses and can never potentiate again.
    const aibaby::Network& net = brain.network();
    if (net.dropped_synapses() > 0 || net.dropped_reverse() > 0) {
      std::printf("  WARNING  %u synapses and %u reverse entries dropped — this is not\n"
                  "           the brain the genome describes. Raise max_out_degree on:\n",
                  net.dropped_synapses(), net.dropped_reverse());
      for (uint32_t m = 0; m < net.module_count(); ++m) {
        const uint32_t fwd = net.dropped_synapses(m);
        const uint32_t rev = net.dropped_reverse(m);
        if (fwd == 0 && rev == 0) continue;
        std::printf("           %-12s cap %u — %u outgoing, %u incoming lost\n",
                    net.module_dna(m).name, net.module_dna(m).max_out_degree, fwd, rev);
      }
    }
    return true;
  }

  // Hatch from a snapshot instead of from birth (§8). `snap` must outlive the
  // session: the genome the brain runs on is the one embedded in those bytes.
  bool resume(const std::vector<uint8_t>& snap, std::string& error) {
    const void* blob = nullptr;
    size_t blob_size = 0;
    aibaby::SnapshotStatus s =
        aibaby::snapshot_genome(snap.data(), snap.size(), &blob, &blob_size);
    if (s != aibaby::SnapshotStatus::kOk) {
      error = std::string("snapshot: ") + aibaby::snapshot_status_string(s);
      return false;
    }
    const aibaby::DnaStatus ds = dna.load(blob, blob_size);
    if (ds != aibaby::DnaStatus::kOk) {
      error = std::string("genome in snapshot rejected: ") + aibaby::dna_status_string(ds);
      return false;
    }
    memory.assign(aibaby::Brain::required_bytes(dna), 0);
    s = aibaby::load_snapshot(brain, snap.data(), snap.size(), memory.data(), memory.size());
    if (s != aibaby::SnapshotStatus::kOk) {
      error = std::string("snapshot: ") + aibaby::snapshot_status_string(s);
      return false;
    }
    return true;
  }
};


// --- M2: does the baby discriminate an object from an empty field? ---------

// Coarse population binning of a module's spikes over a trial. Binning rather
// than one feature per neuron is what keeps the classifier honest: 400 raw
// counts against 50 training trials would let a linear readout memorise the
// noise, and the accuracy it reported would be about the classifier rather
// than about the baby.
constexpr uint32_t kFeatureBins = 32;


// A held-out linear readout: standardise each dimension on the training set,
// then take the nearest class centroid. No hyperparameters, nothing to tune,
// and nothing that can be quietly fitted to the test half.
//
// It is deliberately the weakest classifier that could work. The claim being
// made is about the brain, so any capacity in the readout is capacity that
// belongs to the wrong side of the question.
inline double holdout_accuracy(const std::vector<std::vector<double>>& x,
                        const std::vector<int>& y, size_t train_count) {
  if (x.empty() || train_count == 0 || train_count >= x.size()) return 0.0;
  // This is a TWO-class classifier and it indexes a size-2 array with the
  // label. A stray 2 walks off the end of `centroid` and corrupts the heap —
  // which is not hypothetical: a four-word session handed this 0..3 and the
  // crash surfaced inside free(), three frames away from the cause. Refusing
  // is the only safe answer; silently clamping would score a four-way problem
  // as a two-way one and report a number.
  for (int label : y) {
    if (label != 0 && label != 1) return 0.0;
  }
  const size_t dims = x[0].size();

  std::vector<double> mean(dims, 0.0), sd(dims, 0.0);
  for (size_t i = 0; i < train_count; ++i) {
    for (size_t d = 0; d < dims; ++d) mean[d] += x[i][d];
  }
  for (size_t d = 0; d < dims; ++d) mean[d] /= double(train_count);
  for (size_t i = 0; i < train_count; ++i) {
    for (size_t d = 0; d < dims; ++d) {
      const double dev = x[i][d] - mean[d];
      sd[d] += dev * dev;
    }
  }
  for (size_t d = 0; d < dims; ++d) {
    sd[d] = std::sqrt(sd[d] / double(train_count));
    if (sd[d] < 1e-9) sd[d] = 1e-9;
  }

  std::vector<double> centroid[2] = {std::vector<double>(dims, 0.0),
                                     std::vector<double>(dims, 0.0)};
  size_t n[2] = {0, 0};
  for (size_t i = 0; i < train_count; ++i) {
    const int c = y[i];
    ++n[c];
    for (size_t d = 0; d < dims; ++d) centroid[c][d] += (x[i][d] - mean[d]) / sd[d];
  }
  if (n[0] == 0 || n[1] == 0) return 0.0;
  for (int c = 0; c < 2; ++c) {
    for (size_t d = 0; d < dims; ++d) centroid[c][d] /= double(n[c]);
  }

  size_t correct = 0, total = 0;
  for (size_t i = train_count; i < x.size(); ++i) {
    double dist[2] = {0.0, 0.0};
    for (size_t d = 0; d < dims; ++d) {
      const double z = (x[i][d] - mean[d]) / sd[d];
      for (int c = 0; c < 2; ++c) {
        const double dev = z - centroid[c][d];
        dist[c] += dev * dev;
      }
    }
    const int guess = dist[1] < dist[0] ? 1 : 0;
    if (guess == y[i]) ++correct;
    ++total;
  }
  return total ? double(correct) / double(total) : 0.0;
}


// Nearest centroid by *correlation*, for population codes the z-scored
// classifier above cannot read.
//
// `holdout_accuracy` trains on the first half of the trials and tests on the
// second. That is only clean if the creature is stationary across the session,
// and this one is not: per-module homeostasis retunes gains as it runs. If a
// module's mean drifts between the two halves, every test trial can land on
// the same side of both training centroids, the guess never changes, and
// against m3probe's alternating labels a constant guess scores exactly 0.500 —
// which is indistinguishable from "this module carries nothing". Four
// creatures in a density sweep once read exactly 0.500 on `vocal`, and the
// odds of that being real chance are about 4e-5.
//
// Splitting by alternating *pairs* keeps the two classes balanced within each
// fold while spreading both folds across the whole session, so a slow drift
// hits train and test equally instead of separating them.
// Correlation between the classifier's guess and the trial's position in the
// test fold, using the identical classifier `holdout_accuracy` runs.
//
// This is the check that catches drift, and the reason it has to exist is a
// small proof. m3probe alternates its labels 0,1,0,1,..., so consider any
// classifier whose decision is a monotone function of trial index — it guesses
// class 0 for the first m test trials and class 1 for the remaining n-m. Half
// of the first stretch is genuinely class 0, so it gets m/2; half of the second
// is genuinely class 1, so it gets (n-m)/2. The total is n/2 for *every* m.
//
// A classifier reading nothing but session time therefore scores exactly 0.500,
// and it does so robustly rather than by luck. It is not a constant guess, so
// checking whether the guess ever changes does not catch it. Eight creatures in
// a row printed exactly 0.500 on `vocal` before this was understood; intrinsic
// plasticity runs on vocal, so its excitability adapts monotonically across a
// session and the per-neuron classifier locks onto that instead of the label.
inline double holdout_guess_time_corr(const std::vector<std::vector<double>>& x,
                                      const std::vector<int>& y, size_t train_count) {
  if (x.empty() || train_count == 0 || train_count >= x.size()) return -1.0;
  const size_t dims = x[0].size();
  std::vector<double> mean(dims, 0.0), sd(dims, 0.0);
  for (size_t i = 0; i < train_count; ++i) {
    for (size_t d = 0; d < dims; ++d) mean[d] += x[i][d];
  }
  for (size_t d = 0; d < dims; ++d) mean[d] /= double(train_count);
  for (size_t i = 0; i < train_count; ++i) {
    for (size_t d = 0; d < dims; ++d) {
      const double dev = x[i][d] - mean[d];
      sd[d] += dev * dev;
    }
  }
  for (size_t d = 0; d < dims; ++d) {
    sd[d] = std::sqrt(sd[d] / double(train_count));
    if (sd[d] < 1e-9) sd[d] = 1e-9;
  }
  std::vector<double> centroid[2] = {std::vector<double>(dims, 0.0),
                                     std::vector<double>(dims, 0.0)};
  size_t n[2] = {0, 0};
  for (size_t i = 0; i < train_count; ++i) {
    const int c = y[i];
    ++n[c];
    for (size_t d = 0; d < dims; ++d) centroid[c][d] += (x[i][d] - mean[d]) / sd[d];
  }
  if (n[0] == 0 || n[1] == 0) return -1.0;
  for (int c = 0; c < 2; ++c) {
    for (size_t d = 0; d < dims; ++d) centroid[c][d] /= double(n[c]);
  }
  std::vector<double> guess, pos;
  for (size_t i = train_count; i < x.size(); ++i) {
    double dist[2] = {0.0, 0.0};
    for (size_t d = 0; d < dims; ++d) {
      const double z = (x[i][d] - mean[d]) / sd[d];
      for (int c = 0; c < 2; ++c) {
        const double dev = z - centroid[c][d];
        dist[c] += dev * dev;
      }
    }
    guess.push_back(dist[1] < dist[0] ? 1.0 : 0.0);
    pos.push_back(double(i - train_count));
  }
  if (guess.size() < 2) return -1.0;
  double mg = 0.0, mp = 0.0;
  for (size_t i = 0; i < guess.size(); ++i) {
    mg += guess[i];
    mp += pos[i];
  }
  mg /= double(guess.size());
  mp /= double(pos.size());
  double num = 0.0, dg = 0.0, dp = 0.0;
  for (size_t i = 0; i < guess.size(); ++i) {
    const double a = guess[i] - mg, b = pos[i] - mp;
    num += a * b;
    dg += a * a;
    dp += b * b;
  }
  // A constant guess has no variance to correlate; report it as fully
  // time-locked, because it is just as much "not a score" as drift is.
  if (dg < 1e-12) return 1.0;
  if (dp < 1e-12) return -1.0;
  return num / std::sqrt(dg * dp);
}

inline void interleave_pairs(const std::vector<std::vector<double>>& x,
                             const std::vector<int>& y,
                             std::vector<std::vector<double>>& xo,
                             std::vector<int>& yo, size_t& train_count) {
  xo.clear();
  yo.clear();
  train_count = 0;
  for (int pass = 0; pass < 2; ++pass) {
    for (size_t i = 0; i < x.size() && i < y.size(); ++i) {
      if (int((i / 2) % 2) != pass) continue;
      xo.push_back(x[i]);
      yo.push_back(y[i]);
    }
    if (pass == 0) train_count = xo.size();
  }
}

// `holdout_accuracy` divides every dimension by its own training standard
// deviation, floored at 1e-9. On a dense module that is the right thing. On a
// sparse one it is a trap: a neuron that fired in one training probe out of
// forty-six has a tiny but non-zero sd, so its noise is amplified by orders of
// magnitude, and with a thousand such dimensions the distance is decided
// entirely by rare accidents. That penalty falls hardest on exactly the codes a
// hippocampus is built to produce, and the vision positive control cannot catch
// it because vision is dense.
//
// This one compares shapes rather than scaled distances, so silent neurons
// contribute nothing instead of contributing noise.
inline double holdout_accuracy_corr(const std::vector<std::vector<double>>& x,
                             const std::vector<int>& y, size_t train_count) {
  if (x.empty() || train_count == 0 || train_count >= x.size()) return 0.0;
  // This is a TWO-class classifier and it indexes a size-2 array with the
  // label. A stray 2 walks off the end of `centroid` and corrupts the heap —
  // which is not hypothetical: a four-word session handed this 0..3 and the
  // crash surfaced inside free(), three frames away from the cause. Refusing
  // is the only safe answer; silently clamping would score a four-way problem
  // as a two-way one and report a number.
  for (int label : y) {
    if (label != 0 && label != 1) return 0.0;
  }
  const size_t dims = x[0].size();

  std::vector<double> centroid[2] = {std::vector<double>(dims, 0.0),
                                     std::vector<double>(dims, 0.0)};
  size_t n[2] = {0, 0};
  for (size_t i = 0; i < train_count; ++i) {
    const int c = y[i];
    ++n[c];
    for (size_t d = 0; d < dims; ++d) centroid[c][d] += x[i][d];
  }
  if (n[0] == 0 || n[1] == 0) return 0.0;
  for (int c = 0; c < 2; ++c) {
    for (size_t d = 0; d < dims; ++d) centroid[c][d] /= double(n[c]);
  }

  auto corr = [&](const std::vector<double>& a, const std::vector<double>& b) {
    double ma = 0, mb = 0;
    for (size_t d = 0; d < dims; ++d) { ma += a[d]; mb += b[d]; }
    ma /= double(dims); mb /= double(dims);
    double num = 0, va = 0, vb = 0;
    for (size_t d = 0; d < dims; ++d) {
      const double da = a[d] - ma, db = b[d] - mb;
      num += da * db; va += da * da; vb += db * db;
    }
    const double den = std::sqrt(va * vb);
    return den > 1e-12 ? num / den : 0.0;
  };

  size_t correct = 0, total = 0;
  for (size_t i = train_count; i < x.size(); ++i) {
    const double r0 = corr(x[i], centroid[0]);
    const double r1 = corr(x[i], centroid[1]);
    if ((r1 > r0 ? 1 : 0) == y[i]) ++correct;
    ++total;
  }
  return total ? double(correct) / double(total) : 0.0;
}


// What a population code looks like, past the single number a classifier gives.
//
// A held-out accuracy says how much a distinction survives; it does not say
// whether it survives as a strong signal in a few neurons or a weak one spread
// over many, and those two call for opposite fixes. `informative` counts the
// neurons that individually separate the classes, `mean_d` is how well the
// average neuron does, and `sparseness` is the Treves-Rolls measure of how
// distributed the code is at all (near 1 dense, near 1/n sparse).
struct CodeStats {
  double informative = 0;  // fraction of neurons with |d'| > 0.5
  double mean_d = 0;       // mean |d'| over neurons
  double sparseness = 1;   // population sparseness, averaged over probes
  // The three above are all biased against a sparse code, which matters
  // because sparseness is exactly what a hippocampus is for. A module whose
  // neurons are mostly silent has most of its dimensions contributing nothing
  // to `mean_d` while still counting in its denominator, so a sparse code is
  // penalised for being sparse. These four are the unbiased readings.
  double active = 1;       // fraction of neurons that vary at all
  double mean_d_active = 0;// mean |d'| over only those
  // Pattern separation as it is actually defined: overlap between
  // representations, not linear discriminability. `r_between` is the mean
  // correlation between probes of *different* classes, `r_within` between
  // probes of the same class. Separation is r_within - r_between, and a
  // hippocampus can improve it while a centroid classifier on raw rates sees
  // nothing.
  double r_within = 0, r_between = 0;
};


inline CodeStats code_stats(const std::vector<std::vector<double>>& x,
                     const std::vector<int>& y) {
  CodeStats cs;
  if (x.empty() || x[0].empty()) return cs;
  const size_t dims = x[0].size();

  double sum[2] = {0, 0}, sq[2] = {0, 0};
  size_t n[2] = {0, 0};
  double informative = 0, absd = 0;
  size_t active_dims = 0;
  for (size_t d = 0; d < dims; ++d) {
    sum[0] = sum[1] = sq[0] = sq[1] = 0;
    n[0] = n[1] = 0;
    for (size_t i = 0; i < x.size(); ++i) {
      const int c = y[i];
      sum[c] += x[i][d];
      sq[c] += x[i][d] * x[i][d];
      ++n[c];
    }
    if (!n[0] || !n[1]) continue;
    const double m0 = sum[0] / double(n[0]), m1 = sum[1] / double(n[1]);
    const double v0 = sq[0] / double(n[0]) - m0 * m0;
    const double v1 = sq[1] / double(n[1]) - m1 * m1;
    const double pooled = std::sqrt(0.5 * ((v0 > 0 ? v0 : 0) + (v1 > 0 ? v1 : 0)));
    if (pooled < 1e-9) continue;  // a silent neuron discriminates nothing
    ++active_dims;
    const double d_prime = std::fabs(m1 - m0) / pooled;
    absd += d_prime;
    if (d_prime > 0.5) informative += 1.0;
  }
  cs.informative = informative / double(dims);
  cs.mean_d = absd / double(dims);
  cs.active = active_dims ? double(active_dims) / double(dims) : 0.0;
  cs.mean_d_active = active_dims ? absd / double(active_dims) : 0.0;

  // Representation overlap. Correlation rather than distance, so it does not
  // care that a sparse module fires a tenth as much as a dense one.
  auto corr = [&](const std::vector<double>& a, const std::vector<double>& b) {
    double ma = 0, mb = 0;
    for (size_t d = 0; d < dims; ++d) { ma += a[d]; mb += b[d]; }
    ma /= double(dims); mb /= double(dims);
    double num = 0, va = 0, vb = 0;
    for (size_t d = 0; d < dims; ++d) {
      const double da = a[d] - ma, db = b[d] - mb;
      num += da * db; va += da * da; vb += db * db;
    }
    const double den = std::sqrt(va * vb);
    return den > 1e-12 ? num / den : 0.0;
  };
  double within = 0, between = 0;
  size_t nw = 0, nb = 0;
  for (size_t i = 0; i < x.size(); ++i) {
    for (size_t j = i + 1; j < x.size(); ++j) {
      const double r = corr(x[i], x[j]);
      if (y[i] == y[j]) { within += r; ++nw; } else { between += r; ++nb; }
    }
  }
  if (nw) cs.r_within = within / double(nw);
  if (nb) cs.r_between = between / double(nb);

  double sp = 0;
  size_t sp_n = 0;
  for (size_t i = 0; i < x.size(); ++i) {
    double s = 0, s2 = 0;
    for (size_t d = 0; d < dims; ++d) { s += x[i][d]; s2 += x[i][d] * x[i][d]; }
    const double mean = s / double(dims), meansq = s2 / double(dims);
    if (meansq > 1e-12) { sp += mean * mean / meansq; ++sp_n; }
  }
  if (sp_n) cs.sparseness = sp / double(sp_n);
  return cs;
}


// Population binning of per-neuron counts. The classifier never sees one
// feature per neuron: 400 counts against 50 training trials would let even a
// centroid rule fit the noise, and what it reported would be a fact about the
// readout rather than about the baby.
inline std::vector<double> rebin(const std::vector<double>& per_neuron, uint32_t bins) {
  std::vector<double> out(bins, 0.0);
  const size_t n = per_neuron.size();
  if (n == 0) return out;
  for (size_t i = 0; i < n; ++i) {
    const size_t b = n > 1 ? i * (bins - 1) / (n - 1) : 0;
    out[b] += per_neuron[i];
  }
  return out;
}


// How many slices of a retinal frame the time-resolved readout uses. Four is
// enough to separate "fired as soon as the frame landed" from "fired late",
// which is the distinction the retina's latency code is made of, without
// splitting the window so finely that each slice holds too few spikes to
// average.
constexpr uint32_t kPhaseBins = 4;


// The same population binning, but resolved by *when in the frame* the spike
// happened.
//
// This exists because the retina and every readout in this file disagree about
// what a representation is. The retina spends time: a cell fires once per
// frame, early if it responded strongly, so the picture is carried by the
// order of a volley rather than by how many spikes it contains (§5.1). Every
// feature above then accumulates a plain count over a window hundreds of ticks
// long, which is precisely the operation that discards spike order. A module
// could hold the shape perfectly in its timing and still score at chance.
//
// `counts` is indexed [neuron * kPhaseBins + phase]. The result concatenates
// one spatial histogram per phase, so a caller asking for `bins_per_phase` gets
// `bins_per_phase * kPhaseBins` features in total — which is how the comparison
// is kept fair. Against the same number of features, a spatial-only histogram
// and this one differ in what they describe and in nothing else.
inline std::vector<double> rebin_phases(const std::vector<double>& counts, uint32_t begin,
                                 uint32_t count, uint32_t bins_per_phase) {
  std::vector<double> out;
  out.reserve(size_t(bins_per_phase) * kPhaseBins);
  std::vector<double> slice(count, 0.0);
  for (uint32_t p = 0; p < kPhaseBins; ++p) {
    for (uint32_t k = 0; k < count; ++k) {
      slice[k] = counts[size_t(begin + k) * kPhaseBins + p];
    }
    const std::vector<double> binned = rebin(slice, bins_per_phase);
    out.insert(out.end(), binned.begin(), binned.end());
  }
  return out;
}


// Divides out the overall level, leaving only the shape of the activity.
inline std::vector<double> normalise(const std::vector<double>& v) {
  double total = 0.0;
  for (double x : v) total += x;
  std::vector<double> out(v.size(), 0.0);
  if (total <= 0.0) return out;
  for (size_t i = 0; i < v.size(); ++i) out[i] = v[i] / total;
  return out;
}


constexpr uint32_t kM2Replicates = 5;


// One delayed piece of caregiver feedback: when it lands, and whether it was
// "good baby" or "no".
struct Praise {
  uint64_t tick;
  float value;
};


// --- M3: cross-modal association (G3) --------------------------------------

// The caregiver's two words. Far apart in formant space, because what B2
// receives is a 24-band mel frame and two words that differ only in pitch
// would arrive as very nearly the same picture. An /a/ and an /i/ differ in
// the one dimension the filterbank resolves best.
struct Word {
  float f0, f1, f2;
};

// APPENDED to, never reordered. Every other experiment indexes kWords[0] and
// kWords[1] by name-of-object, so adding entries at the end is bit-identical
// for all of them and swapping any two would silently redefine what "cube"
// means in a dozen places.
//
// The first two are maximally far apart on both formants, which is what a
// two-alternative milestone wants and also what makes it a weak test of
// *repeating*: a creature that transmits nothing but "how bright is this
// sound" would pass it. 2 and 3 exist to break that tie.
constexpr Word kWords[8] = {
    {190.0f, 780.0f, 1180.0f},   // "ball" — an open /a/
    {250.0f, 320.0f, 2500.0f},   // "cube" — a close /i/
    // /u/: F1 within 30 Hz of /i/ and F2 1600 Hz away from it. The pair
    // (1, 2) is therefore a nearly pure F2 discrimination — if imitation is
    // one loud spectral axis, this is the pair that exposes it.
    {200.0f, 350.0f, 900.0f},    // "boot" — a close back /u/
    // /e/: sits between /a/ and /i/ on both formants, so its pairs are the
    // adjacent-vowel case rather than the corner-vowel one.
    {215.0f, 550.0f, 1850.0f},   // "bed"  — a mid front /e/
    // Appended for `vocab`, which asks how many words this creature can hold
    // apart at once. Chosen to CROWD the four above rather than to sit in the
    // gaps: /o/ is 50 Hz from /u/ on F1, /ae/ is 140 from /a/ on F1 and 20 on
    // F2, /^/ sits between /a/ and /e/, and /I/ is a close front vowel 550 Hz
    // below /i/ on F2. A vocabulary that only grows into empty space measures
    // the size of the space, not the creature.
    {205.0f, 450.0f, 850.0f},    // "boat" — /o/,  crowds /u/
    {195.0f, 690.0f, 1700.0f},   // "bat"  — /ae/, crowds /a/ and /e/
    {210.0f, 620.0f, 1200.0f},   // "but"  — /^/,  crowds /a/
    {235.0f, 400.0f, 1950.0f},   // "bit"  — /I/,  crowds /i/ and /e/
};
// The four the milestones are scored on. `imitate` and everything before it
// index this, so it must NOT grow: adding words to the table above is
// bit-identical for them only while this stays 4.
constexpr uint32_t kWordCount = 4;
// The whole table, for `vocab`.
constexpr uint32_t kVocabCount = 8;


// The two toys. A cube is a square and a ball is a disc, and the disc is drawn
// with the radius that gives it the *same area* as the square: 2/sqrt(pi).
//
// Without that they differ by 27% in area, and a milestone about telling a
// cube from a ball could be passed by a brightness meter. Matching the areas
// costs real accuracy — what is left is corners against curvature and nothing
// else — and it is the difference between a claim about form and a claim about
// how much of the field is lit.
constexpr float kAreaMatch = 1.1283791671f;


// Where the toy is held. The retina is foveated at a fixed gaze (§5.1), so it
// has no translation invariance to offer: an object moved by half a fovea is a
// different image to it, not the same image somewhere else. A caregiver holds a
// toy up in front of a baby, and the jitter here is what that is worth — a
// pixel or so, well under one foveal cell. Size varies more, because holding
// something nearer is the ordinary thing that happens to it.
struct Toy {
  SceneSource::Shape shape;
  float cx, cy, radius;
};


inline Toy m3_toy(aibaby::Rng& rng, int object) {
  Toy t;
  t.shape = object == 1 ? SceneSource::Shape::kSquare : SceneSource::Shape::kDisc;
  t.cx = 0.5f + 0.02f * float(rng.signed_uniform());
  t.cy = 0.5f + 0.02f * float(rng.signed_uniform());
  const float half_width = 0.095f + 0.020f * float(rng.uniform());
  t.radius = object == 1 ? half_width : half_width * kAreaMatch;
  return t;
}


// The same toy at a chosen place, with the size jitter removed. For probes that
// sweep POSITION and need everything else held: a toy that also changed size
// would confound "the eye cannot find it out there" with "it got smaller".
inline Toy m3_toy_at(double cx, double cy) {
  Toy t;
  t.shape = SceneSource::Shape::kDisc;
  t.cx = float(cx);
  t.cy = float(cy);
  t.radius = 0.105f * kAreaMatch;  // the mid-size disc m3_toy draws
  return t;
}


// One presentation: the object is in view for the whole trial and the
// caregiver says its name over the first part of it. Praise accompanies the
// naming — and it is deliberately *identical* for both words, so reward
// carries no information about which object this is. Everything that
// distinguishes the two classes has to travel picture -> sound -> voice, which
// is the claim M3 is making.
constexpr uint64_t kM3TrialTicks = 1400;

constexpr uint64_t kM3LabelTicks = 900;

constexpr uint64_t kM3SettleTicks = 500;

// The probe: same object, no word, no praise. This is where the baby's own
// vocalisations are recorded, and it is the only thing the classifier sees.
//
// The settle window is more than twice the trial's, and that is not caution:
// the articulators have 800 ms of inertia (§5.3), so half a second after a
// trial ends the vocal tract is still largely holding the posture the *last*
// one left it in. Measuring a probe too early measures the trial before it.
constexpr uint64_t kM3ProbeTicks = 2000;

constexpr uint64_t kM3ProbeSettleTicks = 1100;

// How many named presentations between probes. Frequent enough that the probes
// span the session — the classifier is split by time, so probes bunched at the
// end would give it nothing to generalise across.
constexpr uint32_t kM3TrainPerProbe = 3;


// What one trial leaves behind: the vocal tract averaged over the recorded
// window, and B1's population activity over the same window.
//
// `f0`/`f1`/`f2` are in Hz and are what the tract actually did, rather than the
// group values it was derived from. They are kept separately because the
// acoustic measure below has to render a sound, and re-deriving hertz from a
// group value would mean a second copy of the group->formant mapping that
// `senses.cpp` owns — which is exactly the sort of duplicate description that
// goes quietly out of date.
struct M3Record {
  double group[aibaby::kVocalGroups] = {};
  double act[aibaby::kVocalGroups] = {};   // the same groups read as RATES
  double amplitude = 0.0;
  double f0 = 0.0, f1 = 0.0, f2 = 0.0;
  uint64_t voiced = 0, frames = 0;
  std::vector<double> b1;
  bool slept = false;
};


// --- What does it SOUND like? ----------------------------------------------
//
// Every number this project reports about the creature's voice is a classifier
// on motor parameters, and a classifier will happily score 0.9 on a difference
// no ear can resolve. Nothing here has ever measured whether two utterances are
// *audible* as different, which for a milestone whose headline is "cube and
// ball produce distinguishable vocalisations" is the question itself.
//
// This renders a posture through the same two-formant tract the creature speaks
// with, pushes it through the same cochlea its ear uses, and takes the
// cepstrum. It is the measurement path only and never touches a brain — see the
// note on `Mfcc` for why a DCT must not feed a creature whose auditory encoder
// is tonotopic.
//
// Two honest limits. The renderer is `VowelSource`, so F3 and the bandwidths
// the browser's three-resonator voice also has are absent — the same limit the
// `--wav` files carry. And a posture is rendered as a steady vowel, which is
// what the trial actually is: 800 ms of articulator inertia over a 900-tick
// window means the tract is holding one shape, and that shape is the thing a
// listener would name.
class Timbre {
 public:
  bool configure(const aibaby::DnaAudio& audio, std::string& error) {
    if (!cochlea_.configure(audio, error)) return false;
    // Twelve coefficients is the usual speech-recognition choice and it is well
    // clear of the 24 bands, so the DCT is a rotation rather than a fit.
    if (!mfcc_.configure(cochlea_.channels(), 12, 22.0f, error)) return false;
    rate_ = audio.sample_rate;
    return true;
  }

  // Mean cepstrum over a steady vowel. Returns empty if the posture produced
  // too few frames to average.
  std::vector<double> of(double f0, double f1, double f2, double amplitude) {
    const size_t n = size_t(rate_) * 500 / 1000;  // 500 ms, ~31 mel frames
    pcm_.assign(n, 0.0f);
    VowelSource voice(rate_);
    voice.render(float(f0), float(f1), float(f2), float(amplitude), pcm_.data(), n);
    cochlea_.reset_stream();
    cochlea_.clear_frames();
    cochlea_.push(pcm_.data(), n);
    const std::vector<float>& mel = cochlea_.frames();
    const uint32_t ch = cochlea_.channels();
    const size_t frames = ch ? mel.size() / ch : 0;
    // The resonators start from silence, so the first frames are an onset
    // transient rather than the vowel. A listener naming a vowel is not
    // listening to its attack.
    const size_t skip = frames > 12 ? 6 : 0;
    if (frames <= skip) return {};
    std::vector<double> sum(mfcc_.coefficients(), 0.0);
    for (size_t f = skip; f < frames; ++f) {
      const std::vector<float>& c = mfcc_.transform(&mel[f * ch], ch);
      for (size_t k = 0; k < sum.size(); ++k) sum[k] += double(c[k]);
    }
    const double inv = 1.0 / double(frames - skip);
    for (double& v : sum) v *= inv;
    return sum;
  }

  uint32_t coefficients() const { return mfcc_.coefficients(); }

 private:
  Cochlea cochlea_;
  Mfcc mfcc_;
  std::vector<float> pcm_;
  uint32_t rate_ = 16000;
};


// How far apart two sets of sounds are, in units of how much the creature's
// own utterances of the *same* word already wander.
//
// Each coefficient is divided by its pooled within-class spread before the
// distance is taken, which is diagonal Mahalanobis: C0 carries overall loudness
// and is numerically enormous next to C9, and an unweighted distance would be a
// loudness measure wearing a cepstrum's clothes.
//
// The result is a d-prime, and it has a meaning outside this project: at
// d' = 1 a listener gets about 76% right in a two-alternative forced choice.
// That is the bar "audibly different" should be held to, rather than a
// classifier accuracy that says nothing about ears.
//
// `unbiased` removes the estimator's floor, and it is the difference between a
// ruler that can answer this question and one that cannot. A squared
// Mahalanobis distance built from two *sample* means is positively biased: even
// for two identical distributions each coefficient contributes
// E[z_k^2] = 1/n0 + 1/n1, because that is the variance of a difference of
// sample means in sigma units. Summed over d coefficients,
//
//     E[d'^2 | identical] = d * (1/n0 + 1/n1)
//
// which at m3's ~20 probes and 12 coefficients is 2.4, i.e. d' = 1.55 — and the
// shuffled-label null measures 1.57. The floor was never a property of the
// creature. Subtracting it analytically makes two identical sounds read ~0 at
// *any* sample size, turns the shuffled row from a floor into a check, and lets
// a real separation smaller than 1.5 be seen at all.
//
// Returned as a **signed squared** distance when `unbiased` is set, not as a
// d-prime, and that is deliberate. The correction is unbiased in d'^2 and not
// in d': clamping at zero and taking a root per creature before averaging over
// creatures puts the bias straight back — measured, it left the shuffled null
// reading 0.43 instead of ~0. Callers must average the squared values across
// creatures and take the root once, at the end. Negative is a real answer here;
// it means "no separation, and the sample says so".
//
// The raw d-prime is still what comes back when `unbiased` is false, so every
// number recorded before this existed stays reproducible.
inline double cepstral_dprime(const std::vector<std::vector<double>>& x,
                              const std::vector<int>& y,
                              std::vector<double>* sigma_out = nullptr,
                              bool unbiased = false) {
  if (x.empty() || x.size() != y.size()) return 0.0;
  const size_t d = x[0].size();
  std::vector<double> mean[2] = {std::vector<double>(d, 0.0), std::vector<double>(d, 0.0)};
  double n[2] = {0.0, 0.0};
  for (size_t i = 0; i < x.size(); ++i) {
    const int c = y[i] ? 1 : 0;
    for (size_t k = 0; k < d; ++k) mean[c][k] += x[i][k];
    n[c] += 1.0;
  }
  if (n[0] < 2.0 || n[1] < 2.0) return 0.0;
  for (int c = 0; c < 2; ++c) {
    for (size_t k = 0; k < d; ++k) mean[c][k] /= n[c];
  }
  std::vector<double> var(d, 0.0);
  for (size_t i = 0; i < x.size(); ++i) {
    const int c = y[i] ? 1 : 0;
    for (size_t k = 0; k < d; ++k) {
      const double e = x[i][k] - mean[c][k];
      var[k] += e * e;
    }
  }
  const double dof = n[0] + n[1] - 2.0;
  double acc = 0.0;
  std::vector<double> sigma(d, 0.0);
  for (size_t k = 0; k < d; ++k) {
    sigma[k] = std::sqrt(var[k] / dof);
    if (sigma[k] > 1e-9) {
      const double z = (mean[0][k] - mean[1][k]) / sigma[k];
      acc += z * z;
    }
  }
  if (sigma_out) *sigma_out = sigma;
  if (unbiased) {
    // Only the dimensions that actually contributed are counted: a coefficient
    // with no spread was skipped above and carries no bias either.
    double dims = 0.0;
    for (size_t k = 0; k < d; ++k) {
      if (sigma[k] > 1e-9) dims += 1.0;
    }
    // The second-order term, and it is not negligible where this is used most.
    // sigma_k is itself estimated, and E[1/sigma_hat^2] = (1/sigma^2) *
    // dof/(dof-2), so every z^2 is inflated by that factor before it is summed.
    // At the ~200 probes of a long m3 run it is a 1% correction and ignorable;
    // at the ~20 of a short one it is 12.5%, which left the corrected null
    // reading 0.58 instead of ~0 inside verify-long — a floor again, in the
    // one place the instrument is run most often.
    const double dof = n[0] + n[1] - 2.0;
    const double inflate = dof > 2.0 ? dof / (dof - 2.0) : 1.0;
    return acc - dims * (1.0 / n[0] + 1.0 / n[1]) * inflate;  // signed d'^2
  }
  return std::sqrt(acc);
}


// The same distance between two fixed postures, measured on a ruler somebody
// else's utterances provided. This is what turns a d-prime into a sentence:
// "as different as [i] from [a]" rather than "1.7".
inline double cepstral_dprime_between(const std::vector<double>& a,
                                      const std::vector<double>& b,
                                      const std::vector<double>& sigma) {
  if (a.size() != b.size() || a.size() != sigma.size()) return 0.0;
  double acc = 0.0;
  for (size_t k = 0; k < a.size(); ++k) {
    if (sigma[k] > 1e-9) {
      const double z = (a[k] - b[k]) / sigma[k];
      acc += z * z;
    }
  }
  return std::sqrt(acc);
}


// The nine groups as CENTROIDS and the nine as RATES, scored separately on the
// same trials. Added 2026-08-19 to ask which half of a group's reading carries
// the object, now that `vision->vocal` has put an object at the larynx to
// carry. Only the centroids reach the vocal tract's articulators; if the rates
// are what carry it, the decoder is discarding the signal by construction and
// that is a fixable readout rather than a missing one.
inline std::vector<double> m3_centroid_features(const M3Record& r) {
  std::vector<double> f;
  if (r.frames == 0) return f;
  for (uint32_t g = 0; g < aibaby::kVocalGroups; ++g) f.push_back(r.group[g] / double(r.frames));
  return f;
}

inline std::vector<double> m3_activity_features(const M3Record& r) {
  std::vector<double> f;
  if (r.frames == 0) return f;
  for (uint32_t g = 0; g < aibaby::kVocalGroups; ++g) f.push_back(r.act[g] / double(r.frames));
  return f;
}

// The milestone's feature: everything the vocal tract did. Nine motor groups,
// how loud it was, and how much of the time the glottis was open.
inline std::vector<double> m3_vocal_features(const M3Record& r) {
  std::vector<double> f;
  if (r.frames == 0) return f;
  const double n = double(r.frames);
  for (uint32_t g = 0; g < aibaby::kVocalGroups; ++g) f.push_back(r.group[g] / n);
  f.push_back(r.amplitude / n);
  f.push_back(double(r.voiced) / n);
  return f;
}


// The guard: the articulator groups only — F0, the three formants, the three
// bandwidths. Loudness and voicing are dropped entirely, so this cannot pass
// on "the cube makes it noisier". It is the difference between two *sounds*
// and two amounts of sound, and M2 is the reason to insist on it: most of what
// B1 carries about an object turned out to be how hard it was working.
inline std::vector<double> m3_timbre_features(const M3Record& r) {
  std::vector<double> f;
  if (r.frames == 0) return f;
  const double n = double(r.frames);
  const uint32_t articulators[7] = {0, 2, 3, 4, 5, 6, 7};
  for (uint32_t k = 0; k < 7; ++k) f.push_back(r.group[articulators[k]] / n);
  return f;
}


struct M3Run {
  bool ok = false;
  double vocal = 0;       // held-out accuracy on the baby's voice — the milestone
  double timbre = 0;      // the same with loudness dropped
  double shuffled = 0;    // the same again with labels shuffled: the control
  // The same trials, scored on what they SOUND like rather than on the motor
  // parameters that produced them. `acoustic` is directly comparable to
  // `vocal`; `dprime` is the one that answers the milestone's own sentence,
  // because a classifier accuracy says nothing about whether an ear could do
  // it. `anchor_*` are fixed vowel contrasts rendered through the same tract
  // and measured on this creature's own ruler, so the d-prime has a scale.
  double acoustic = 0;
  double acoustic_shuffled = 0;
  double motor_interleaved = 0;  // `vocal` on the same split, so it compares
  double dprime = 0;
  // d' estimated from a finite sample is biased upward by roughly
  // sqrt(D * (1/n_A + 1/n_B)) — with 12 coefficients and eight probes a side
  // that is about 1.7 for two IDENTICAL sounds. So the number above is
  // unreadable without this one beside it.
  double dprime_null = 0;
  // The same two quantities with that bias subtracted analytically rather than
  // merely displayed, and held as **signed d'^2** because that is the domain
  // the correction is unbiased in — averaged across creatures and rooted once,
  // by the caller. These are the ones to read: the first is what a listener
  // faces, and the second is a *check* that has to sit near zero rather than a
  // floor the signal has to clear.
  double dprime_unb_sq = 0;
  double dprime_null_unb_sq = 0;
  double anchor_ia = 0;   // [i] vs [a] — nobody would confuse these
  double anchor_near = 0; // [ɑ] vs [ʌ] — neighbours, still two vowels
  double anchor_sd = 0;   // the creature's own delivered F1 spread
  double echo = 0;        // voice *while* the word is playing: the ceiling
  double b1_shape = 0;    // B1 during a silent probe: is the shape even in there
  double alignment = 0;   // see below
  double reward = 0;      // mean |R - E[R]| actually reaching the synapses
  double reward_named = 0, reward_probe = 0;  // and its mean sign in each phase
  double curve[4] = {0, 0, 0, 0};    // alignment over each quarter of the session
  uint32_t taught[4] = {0, 0, 0, 0}; // presentations heard by the end of each
  uint32_t probes = 0, presentations = 0, skipped = 0;
};


// How far apart two sets of trials drove the voice, as a vector in feature
// space.
inline std::vector<double> mean_difference(const std::vector<std::vector<double>>& x,
                                    const std::vector<int>& y) {
  if (x.empty()) return {};
  std::vector<double> sum[2] = {std::vector<double>(x[0].size(), 0.0),
                                std::vector<double>(x[0].size(), 0.0)};
  size_t n[2] = {0, 0};
  for (size_t i = 0; i < x.size(); ++i) {
    const int c = y[i];
    ++n[c];
    for (size_t d = 0; d < x[i].size(); ++d) sum[c][d] += x[i][d];
  }
  if (n[0] == 0 || n[1] == 0) return {};
  std::vector<double> out(x[0].size(), 0.0);
  for (size_t d = 0; d < out.size(); ++d) {
    out[d] = sum[1][d] / double(n[1]) - sum[0][d] / double(n[0]);
  }
  return out;
}


// The mechanism, measured directly instead of through a classifier.
//
// Teaching is supposed to work like this: the word puts the vocal tract into a
// posture, and the picture that arrives with it learns to evoke the same one.
// So there are two directions in vocal-feature space — the one the two *words*
// drive the voice along, and the one the two *pictures* drive it along — and
// the claim is that after teaching they point the same way.
//
// Worth having because held-out accuracy over a few dozen probes is a blunt
// instrument: it is quantised to 1/n, and it throws away the size and the
// direction of the effect. A cosine over the same trials uses all of both, and
// it can say "moving the right way, not yet far enough", which an accuracy of
// 0.52 cannot.
inline double alignment(const std::vector<double>& a, const std::vector<double>& b) {
  if (a.size() != b.size() || a.empty()) return 0.0;
  double dot = 0.0, na = 0.0, nb = 0.0;
  for (size_t i = 0; i < a.size(); ++i) {
    dot += a[i] * b[i];
    na += a[i] * a[i];
    nb += b[i] * b[i];
  }
  if (na <= 0.0 || nb <= 0.0) return 0.0;
  return dot / (std::sqrt(na) * std::sqrt(nb));
}


// One creature, one upbringing.
//
// `paired` is the experiment: the word the caregiver says is the object's name.
// When it is false the caregiver says a word drawn independently of what is in
// front of the baby — the same objects, the same two words in the same
// proportions, the same praise, and no consistent relationship between them.
// That is the control that makes the number mean something: a creature whose
// two shapes already drive its voice apart before any teaching would score
// just as well in both conditions, and would have learned nothing.
// How the caregiver behaves while naming things. Reward is the gate on
// three-factor learning (§3.1), and where that gate opens relative to the
// teaching decides what gets written — so it is a parameter of the experiment,
// not a constant buried in it.
struct Caregiver {
  float praise = kPraiseValue;   // 0 leaves curiosity as the only reward
  uint64_t period = kFeedbackPeriodTicks;
};


constexpr uint32_t kM3Replicates = 5;


// --- Two things every scored table has to carry --------------------------
//
// Both of these exist because of a specific wrong answer that was believed for
// most of a day, and neither was caught by the experiment suite: in each case
// the number sat next to the control that refuted it and the control was not
// read. Making the instrument say it out loud is the only version of this that
// survives a tired reader.


// Who produced this table, and off which trial sequence.
//
// An arm scored 0.871 on apicalprobe against a 0.820 baseline from m3probe and
// was recorded as an improvement. Re-run with one instrument on both arms it
// was 0.813 against 0.820 — nothing. The probes derive their trial RNG from the
// genome seed by different constants, so they draw different toys in a
// different order, and at 300 trials that alone is worth about +/-0.05. Two
// numbers from two probes cannot share a column, and the only reliable way to
// enforce that is to make the instrument visible on every table it prints.
inline void instrument(const char* probe, uint64_t trial_seed, uint64_t count,
                       const char* unit) {
  std::printf("  instrument        %s   trial seed %016llx   %llu %s\n", probe,
              (unsigned long long)trial_seed, (unsigned long long)count, unit);
  std::printf("                    another probe is another trial sequence: its numbers\n"
              "                    are not comparable with these (~0.05 at 300 trials)\n");
}


// The row that is known to carry the signal, checked before anything is read
// off the rows that are not.
//
// A null is only evidence about the creature if the instrument could have seen
// a positive. eligprobe reads its size-matched arcuate — a tract that certainly
// carries a conditional trace — at 0.570 against a chance of 0.510 at the
// default tick count, and at 0.892 at 600k. A whole DNA v26 sweep was run and
// discarded at the first number, and the table it printed looked entirely
// normal. Returning false makes the experiment red, because a probe that cannot
// see is a broken instrument and not a result.
inline bool positive_control(const char* probe, const char* row, double value,
                             double floor, const char* remedy) {
  if (value >= floor) return true;
  std::printf("\n  UNDERPOWERED — %s's positive control (%s) reads %.3f,\n"
              "  under the %.3f this instrument needs. That row is KNOWN to carry\n"
              "  the signal, so every null above is a fact about the instrument and\n"
              "  not about the creature. %s\n",
              probe, row, value, floor, remedy);
  return false;
}


// Defined in experiments_milestones.cpp.
bool run_determinism(const std::vector<uint8_t>&, uint64_t, bool, uint64_t* hash_out = nullptr);
bool run_audio(const std::vector<uint8_t>&, uint64_t, bool);
bool run_vision(const std::vector<uint8_t>&, uint64_t, bool);
bool run_sleep(const std::vector<uint8_t>&, uint64_t, bool);
bool run_babble(const std::vector<uint8_t>&, uint64_t, bool, const Capture&);
bool run_m2(const std::vector<uint8_t>&, uint64_t, bool);
bool run_imitate(const std::vector<uint8_t>&, uint64_t, bool);
bool run_vocallearn(const std::vector<uint8_t>&, uint64_t, bool);
bool run_teachsound(const std::vector<uint8_t>&, uint64_t, bool, const Capture&);
bool run_retain(const std::vector<uint8_t>&, uint64_t, bool);
bool run_capacity(const std::vector<uint8_t>&, uint64_t, bool);
bool run_credit(const std::vector<uint8_t>&, uint64_t, bool);
bool run_driftprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_metaprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_trajprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_vocab(const std::vector<uint8_t>&, uint64_t, bool);
bool run_seqprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_m3(const std::vector<uint8_t>&, uint64_t, const Caregiver&, uint32_t, bool,
            const Capture&);
bool run_g2(const std::vector<uint8_t>&, uint64_t, bool, const Regime&);
bool run_g4(const std::vector<uint8_t>&, uint64_t, bool);
bool run_calibrate(const std::vector<uint8_t>&, uint64_t, bool);
bool run_snapshot(const std::vector<uint8_t>&, uint64_t, bool);

// What `m3probe` measured, keyed by module name, for a caller that needs the
// numbers rather than the table. It is an out-parameter on the existing runner
// rather than a second session function on purpose: two implementations of one
// measurement drift apart, which is the exact failure `restate` exists to
// catch, and it would be absurd for the drift detector to introduce one.
struct M3ProbeScores {
  struct Row {
    std::string module;
    double per_neuron = 0;
    double interleaved = 0;
  };
  std::vector<Row> visual;    // two shapes in silence
  std::vector<Row> auditory;  // two words to an empty field

  double get(bool vis, const char* name, bool interleaved = false) const {
    for (const Row& r : (vis ? visual : auditory)) {
      if (r.module == name) return interleaved ? r.interleaved : r.per_neuron;
    }
    return -1.0;  // "not measured" is distinguishable from any real score
  }
};

// Defined in experiments_probes.cpp.
bool run_v1probe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_m3probe(const std::vector<uint8_t>&, uint64_t, bool, M3ProbeScores* = nullptr);
bool run_restate(const std::vector<uint8_t>&, uint64_t, bool);
bool run_g3probe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_pcprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_audprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_eligprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_dwprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_projprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_condprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_pairprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_apicalprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_oscprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_gazeprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_invprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_cpprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_stpprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_burstprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_pruneprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_tauprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_ipprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_mechverify(const std::vector<uint8_t>&, uint64_t, bool);
bool run_errprobe(const std::vector<uint8_t>&, uint64_t, bool);
bool run_relayprobe(const std::vector<uint8_t>&, uint64_t, bool);

}  // namespace aibaby_host

#endif  // AIBABY_HOST_EXPERIMENTS_COMMON_H
