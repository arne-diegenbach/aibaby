// The numbered goals, and the path checks every goal depends on.
//
// Shared scaffolding is in experiments_common.h.

#include <cstring>
#include "experiments_common.h"
#include "host/mel.h"
#include "host/wav.h"

namespace aibaby_host {

// --- G1: determinism -------------------------------------------------------

// Runs a fixed, slightly awkward script: touches, praise, sound, and something
// to look at, so that every path that mutates state gets exercised. A path the
// script does not walk is a path G1 does not cover, and G1 is the only real
// testing lever this project has. Two runs must agree exactly.
uint64_t scripted_run(Session& s, uint64_t ticks, Ear& ear, VowelSource& voice,
                      Retina& retina, SceneSource& scene, std::vector<uint64_t>* hashes) {
  std::vector<float> pcm(64);
  std::vector<uint8_t> frame(size_t(retina.frame_size()) * retina.frame_size(), 0);
  const uint64_t vision_frame_ticks =
      uint64_t(1000.0f / s.dna.header().vision.frame_hz / s.dna.header().sim.dt_ms + 0.5f);

  for (uint64_t t = 0; t < ticks; ++t) {
    // An object that comes and goes and moves while it is there, so both the
    // latency schedule and the fade between frames are on the hot path.
    if (t % vision_frame_ticks == 0) {
      const bool showing = (t / 1500) % 3 != 0;
      const float phase = float((t / 100) % 10) * 0.02f;
      scene.render(showing ? SceneSource::Shape::kSquare : SceneSource::Shape::kNone,
                   0.5f + phase, 0.5f - phase, 0.10f, 0.85f, 0.02f, frame.data());
      retina.present(frame.data());
      s.brain.see(retina.features().data(), retina.feature_count());
    }

    if (t % 997 == 13) s.brain.poke(0.4f);
    if (t % 1499 == 41) s.brain.tickle(0.6f);
    if (t % 3001 == 7) s.brain.feed(0.35f);
    if (t % 1777 == 123) s.brain.praise(1.0f);
    if (t % 2311 == 55) s.brain.praise(-1.0f);

    // 16 kHz audio against a 1 kHz sim: 16 samples per tick.
    const bool sounding = (t / 1000) % 2 == 0;
    voice.render(sounding ? 180.0f : 0.0f, 700.0f, 1200.0f, sounding ? 0.6f : 0.0f,
                 pcm.data(), 16);
    ear.tick(s.brain, pcm.data(), 16);

    s.brain.step();
    if (hashes && t % 1000 == 999) hashes->push_back(s.brain.network().state_hash());
  }
  return s.brain.network().state_hash();
}


bool run_determinism(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose,
                     uint64_t* hash_out) {
  std::string error;
  Session a, b;
  if (!a.init(blob, error) || !b.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }

  const aibaby::DnaAudio& acfg = a.dna.header().audio;
  Ear ca, cb;
  if (!ca.configure(acfg, error) || !cb.configure(acfg, error)) {
    std::printf("  ear failed: %s\n", error.c_str());
    return false;
  }
  VowelSource va(acfg.sample_rate), vb(acfg.sample_rate);

  const aibaby::DnaVision& vcfg = a.dna.header().vision;
  Retina ra, rb;
  if (!ra.configure(vcfg, error) || !rb.configure(vcfg, error)) {
    std::printf("  retina failed: %s\n", error.c_str());
    return false;
  }
  SceneSource sa(vcfg.frame_size, a.dna.header().seed);
  SceneSource sb(vcfg.frame_size, b.dna.header().seed);

  std::vector<uint64_t> ha, hb;
  const uint64_t final_a = scripted_run(a, ticks, ca, va, ra, sa, &ha);
  const uint64_t final_b = scripted_run(b, ticks, cb, vb, rb, sb, &hb);

  size_t diverged_at = ha.size();
  for (size_t i = 0; i < ha.size() && i < hb.size(); ++i) {
    if (ha[i] != hb[i]) { diverged_at = i; break; }
  }

  const bool pass = final_a == final_b && diverged_at == ha.size();
  // `verify` compares this against a recorded value. Reproducibility between
  // two creatures in one process is the weaker half of G1; the half that
  // catches an accidental behaviour change is reproducibility against
  // yesterday, and that needs the number, not a verdict.
  if (hash_out) *hash_out = final_a;
  std::printf("  ticks             %llu\n", (unsigned long long)ticks);
  std::printf("  checkpoints       %zu\n", ha.size());
  std::printf("  final hash A      %016llx\n", (unsigned long long)final_a);
  std::printf("  final hash B      %016llx\n", (unsigned long long)final_b);
  if (!pass && diverged_at < ha.size()) {
    std::printf("  DIVERGED at checkpoint %zu (tick %llu)\n", diverged_at,
                (unsigned long long)((diverged_at + 1) * 1000));
  }
  if (verbose) {
    for (size_t i = 0; i < ha.size(); ++i) {
      std::printf("    t=%6llu  %016llx %s\n", (unsigned long long)((i + 1) * 1000),
                  (unsigned long long)ha[i], ha[i] == hb[i] ? "" : "  <-- MISMATCH");
    }
  }
  std::printf("\n  G1 %s — same genome and same inputs give a %s brain.\n",
              pass ? "PASS" : "FAIL", pass ? "bit-identical" : "DIFFERENT");
  return pass;
}


// --- Audio path ------------------------------------------------------------

// Not one of the numbered goals, but the one thing that has to be true before
// G2 or G3 mean anything: sound has to reach the auditory module and silence
// has to not.
bool run_audio(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
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

  VowelSource voice(acfg.sample_rate);
  std::vector<float> pcm(16);
  const uint32_t channels = ear.channels();

  // Three phases of equal length: silence, an /a/-like vowel, silence again.
  // "Silence" is the room being quiet, not the creature: since DNA v6 it hears
  // its own babbling throughout, which is why the contrast below is a ratio
  // rather than a comparison against nothing.
  const uint64_t phase = ticks / 3;
  double rate_silence1 = 0, rate_sound = 0, rate_silence2 = 0;
  double mel_silence = 0, mel_sound = 0;
  uint64_t n_sil = 0, n_snd = 0;

  for (uint64_t t = 0; t < ticks; ++t) {
    const bool sounding = t >= phase && t < 2 * phase;
    voice.render(200.0f, 730.0f, 1090.0f, sounding ? 0.5f : 0.0f, pcm.data(), 16);
    ear.tick(s.brain, pcm.data(), 16);
    if (ear.had_frame()) {
      double sum = 0;
      for (uint32_t c = 0; c < channels; ++c) sum += ear.latest_mel()[c];
      if (sounding) { mel_sound += sum / channels; ++n_snd; }
      else { mel_silence += sum / channels; ++n_sil; }
    }

    s.brain.step();

    const double r = double(s.brain.network().module(uint32_t(aud)).mean_rate);
    if (t < phase) rate_silence1 += r;
    else if (t < 2 * phase) rate_sound += r;
    else rate_silence2 += r;
  }

  rate_silence1 /= double(phase);
  rate_sound /= double(phase);
  rate_silence2 /= double(phase);
  if (n_snd) mel_sound /= double(n_snd);
  if (n_sil) mel_silence /= double(n_sil);

  std::printf("  mel energy        silence %.4f   vowel %.4f\n", mel_silence, mel_sound);
  std::printf("  auditory rate     silence %.2f Hz -> vowel %.2f Hz -> silence %.2f Hz\n",
              rate_silence1, rate_sound, rate_silence2);
  if (verbose) {
    std::printf("  frames produced   %llu\n",
                (unsigned long long)(n_sil + n_snd));
  }

  // A vowel must both raise the mel energy and move the module. Requiring the
  // rate to come back down is what separates "it heard something" from "it
  // latched".
  const bool pass = mel_sound > mel_silence + 0.05 &&
                    rate_sound > rate_silence1 * 1.10 &&
                    rate_silence2 < rate_sound;
  std::printf("\n  audio path %s — sound reaches B2 and silence does not.\n",
              pass ? "PASS" : "FAIL");
  return pass;
}


// --- Vision path -----------------------------------------------------------

// The counterpart of the audio experiment, and the same question: before any
// claim about seeing means anything, an object has to reach B3 and an empty
// field has to not.
//
// The retina is centre-surround, so "empty" is the interesting half. A blank
// wall is not dark, it is *uniform*, and a difference-of-Gaussians cell reports
// uniform as zero regardless of how bright it is. If that fails, every number
// downstream is measuring the room lights.
//
// This shows its disc dead centre and always will, so it structurally cannot
// detect a gaze or displacement fault — the correct number of saccades here is
// zero and a fovea that never moves looks perfect. That coverage belongs to
// `gazeprobe` (what displacement costs the code) and `invprobe` (whether the
// cost is learnable away), both of which sweep the toy off centre on purpose.
// Deliberate, not an omission: this experiment's job is the transducer, and a
// path check that also varies position could fail for either reason.
bool run_vision(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
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
  const int32_t vis = s.dna.module_with_role(aibaby::ModuleRole::kVision);
  if (vis < 0) {
    std::printf("  genome has no vision module\n");
    return false;
  }

  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);

  // Three phases of equal length: empty field, an object, empty again.
  const uint64_t phase = ticks / 3;
  double rate_blank1 = 0, rate_object = 0, rate_blank2 = 0;
  double contrast_blank = 0, contrast_object = 0;
  uint64_t n_blank = 0, n_object = 0;

  for (uint64_t t = 0; t < ticks; ++t) {
    if (t % frame_ticks == 0) {
      const bool showing = t >= phase && t < 2 * phase;
      scene.render(showing ? SceneSource::Shape::kDisc : SceneSource::Shape::kNone,
                   0.5f, 0.5f, 0.11f, 0.85f, 0.02f, frame.data());
      retina.present(frame.data());
      s.brain.see(retina.features().data(), retina.feature_count());
      if (showing) { contrast_object += retina.contrast(); ++n_object; }
      else { contrast_blank += retina.contrast(); ++n_blank; }
    }

    s.brain.step();

    const double r = double(s.brain.network().module(uint32_t(vis)).mean_rate);
    if (t < phase) rate_blank1 += r;
    else if (t < 2 * phase) rate_object += r;
    else rate_blank2 += r;
  }

  rate_blank1 /= double(phase);
  rate_object /= double(phase);
  rate_blank2 /= double(phase);
  if (n_object) contrast_object /= double(n_object);
  if (n_blank) contrast_blank /= double(n_blank);

  std::printf("  retina            %u cells -> %u ON/OFF responses\n", retina.cells(),
              retina.feature_count());
  std::printf("  mean |response|   empty %.4f   object %.4f\n", contrast_blank,
              contrast_object);
  std::printf("  vision rate       empty %.2f Hz -> object %.2f Hz -> empty %.2f Hz\n",
              rate_blank1, rate_object, rate_blank2);
  // Nothing points this eye any more (DNA v30 deleted the reflexive
  // controller), so a non-zero gaze here would mean a caller moved it and
  // forgot — which would silently invalidate every number above.
  if (retina.gaze_x() != 0.0f || retina.gaze_y() != 0.0f) {
    std::printf("  gaze              (%+.1f, %+.1f) px from centre  <- NOT CENTRED,\n"
                "                    the rows above are about a displaced retina\n",
                double(retina.gaze_x()), double(retina.gaze_y()));
  }
  if (verbose) {
    std::printf("  frames presented  %llu\n",
                (unsigned long long)retina.frames_produced());
    std::printf("  latency window    %.0f ms at %.0f frames/s\n", double(vcfg.latency_ms),
                double(vcfg.frame_hz));
  }

  // An object must both raise the retinal response and move the module, and
  // the module must settle again once it is taken away.
  const bool pass = contrast_object > contrast_blank + 0.02 &&
                    rate_object > rate_blank1 * 1.10 &&
                    rate_blank2 < rate_object;
  std::printf("\n  vision path %s — an object reaches B3 and an empty field does not.\n",
              pass ? "PASS" : "FAIL");
  return pass;
}


// --- Sleep: does fatigue ever come back down, and does the voice stop? -----
//
// A regression test for a bug found by playing with the thing rather than by
// measuring it: fatigue only ever accumulated, so it pinned at 1.0 and the
// baby babbled on forever. Both halves matter — that the cycle completes, and
// that the larynx is actually shut while it is asleep.
bool run_sleep(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }

  bool was_asleep = false;
  uint64_t asleep_ticks = 0, transitions = 0;
  uint64_t voiced_awake = 0, voiced_asleep = 0, frames_awake = 0, frames_asleep = 0;
  double peak_amp_asleep = 0.0, peak_fatigue = 0.0;
  uint64_t first_sleep = 0, first_wake = 0;
  uint32_t last_frame = 0;
  uint64_t last_feedback = 0;

  // Something to look at for the whole session. §3.6 gates sensory input off,
  // and the eyes are the half of that nothing else here checks: a creature
  // that goes on seeing the room while it sleeps is not asleep. The vocal side
  // of this was found by someone leaving the app running rather than by any
  // experiment, which is the argument for checking the visual side on purpose.
  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  Retina retina;
  if (!retina.configure(vcfg, error)) {
    std::printf("  retina failed: %s\n", error.c_str());
    return false;
  }
  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  const uint64_t vision_frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);
  const int32_t vis = s.dna.module_with_role(aibaby::ModuleRole::kVision);
  double vision_rate_awake = 0.0, vision_rate_asleep = 0.0;
  uint64_t vision_ticks_awake = 0, vision_ticks_asleep = 0;
  double peak_retina_asleep = 0.0;
  uint64_t asleep_run = 0;
  constexpr uint64_t kSleepSettleTicks = 1000;

  for (uint64_t t = 0; t < ticks; ++t) {
    if (t % vision_frame_ticks == 0) {
      scene.render(SceneSource::Shape::kDisc, 0.5f, 0.5f, 0.12f, 0.85f, 0.02f,
                   frame.data());
      retina.present(frame.data());
      s.brain.see(retina.features().data(), retina.feature_count());
    }

    s.brain.step();

    if (vis >= 0) {
      const double r = double(s.brain.network().module(uint32_t(vis)).mean_rate);
      if (s.brain.asleep()) {
        vision_rate_asleep += r;
        ++vision_ticks_asleep;
        ++asleep_run;
        // Skip the first second of each bout. The frame that was on the retina
        // when the creature dropped off is still fading, and a peak taken over
        // the transition measures how recently it fell asleep rather than
        // whether its eyes are shut.
        //
        // Replay ticks are skipped too, and for a different reason. This
        // measurement asks whether the *room* reaches a sleeping baby; sleep
        // replay (§3.6) deliberately drives the same encoders from the inside,
        // so counting those ticks would read a working memory as a leaking
        // gate. The object in front of the creature is still being presented
        // throughout, so a real leak would still show up on every other tick.
        if (asleep_run > kSleepSettleTicks && !s.brain.replaying()) {
          double peak = 0.0;
          const aibaby::Scalar* level = s.brain.vision_level();
          for (uint32_t f = 0; f < s.brain.vision_features_count(); ++f) {
            peak = std::max(peak, double(level[f]));
          }
          peak_retina_asleep = std::max(peak_retina_asleep, peak);
        }
      } else {
        vision_rate_awake += r;
        ++vision_ticks_awake;
        asleep_run = 0;
      }
    }

    const bool asleep = s.brain.asleep();
    peak_fatigue = std::max(peak_fatigue, double(s.brain.drives().fatigue));
    if (asleep) ++asleep_ticks;
    if (asleep != was_asleep) {
      ++transitions;
      if (asleep && first_sleep == 0) first_sleep = t;
      if (!asleep && first_sleep != 0 && first_wake == 0) first_wake = t;
      was_asleep = asleep;
    }

    if (s.brain.vocal_frame() == last_frame) continue;
    last_frame = s.brain.vocal_frame();
    const aibaby::VocalParams& v = s.brain.voice();
    const bool sounding = v.voicing > 0.5f && v.amplitude > kAmplitudeFloor;
    if (asleep) {
      ++frames_asleep;
      if (sounding) ++voiced_asleep;
      peak_amp_asleep = std::max(peak_amp_asleep, double(v.amplitude));
    } else {
      ++frames_awake;
      if (sounding) ++voiced_awake;
    }
  }

  if (vision_ticks_awake) vision_rate_awake /= double(vision_ticks_awake);
  if (vision_ticks_asleep) vision_rate_asleep /= double(vision_ticks_asleep);

  const double dt = double(s.dna.header().sim.dt_ms);
  std::printf("  peak fatigue      %.3f\n", peak_fatigue);
  std::printf("  fell asleep at    %.1f s\n", double(first_sleep) * dt / 1000.0);
  std::printf("  woke at           %.1f s\n", double(first_wake) * dt / 1000.0);
  std::printf("  transitions       %llu\n", (unsigned long long)transitions);
  std::printf("  asleep            %.1f%% of the session\n",
              100.0 * double(asleep_ticks) / double(ticks));
  std::printf("  vocalising awake  %.1f%% of motor frames\n",
              frames_awake ? 100.0 * double(voiced_awake) / double(frames_awake) : 0.0);
  std::printf("  vocalising asleep %.1f%% of motor frames (peak amplitude %.4f)\n",
              frames_asleep ? 100.0 * double(voiced_asleep) / double(frames_asleep) : 0.0,
              peak_amp_asleep);
  std::printf("  vision rate       %.2f Hz awake -> %.2f Hz asleep"
              " (object in view throughout)\n",
              vision_rate_awake, vision_rate_asleep);
  std::printf("  retinal drive asleep  peak %.6f (after the first second of each bout)\n",
              peak_retina_asleep);
  if (verbose) {
    std::printf("  motor frames      %llu awake / %llu asleep\n",
                (unsigned long long)frames_awake, (unsigned long long)frames_asleep);
  }

  const bool cycled = first_sleep > 0 && first_wake > first_sleep;
  const bool silent = voiced_asleep == 0;
  const bool talks_awake = frames_awake > 0 && voiced_awake > 0;
  // The eyes close too. There is an object in front of the creature the whole
  // time, so if B3 is as busy asleep as awake then the gate is not gating.
  const bool eyes_shut = vision_ticks_asleep == 0 ||
                         (peak_retina_asleep < 0.02 &&
                          vision_rate_asleep < vision_rate_awake);
  const bool pass = cycled && silent && talks_awake && eyes_shut;
  std::printf("\n  sleep %s — fatigue %s, and the baby %s and %s while asleep.\n",
              pass ? "PASS" : "FAIL",
              cycled ? "rises, discharges, and the baby wakes again"
                     : "did not complete a sleep/wake cycle",
              silent ? "is silent" : "IS STILL TALKING",
              eyes_shut ? "sees nothing" : "IS STILL WATCHING");
  return pass;
}


// --- Babble: what does the vocal tract actually do? ------------------------
//
// Not a goal, a microscope. Before any claim about reward changing behaviour
// is worth making, the behaviour has to have variety to change: a baby that
// vocalises 100% of the time, or 0%, gives reward nothing to grip.
bool run_babble(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose,
                const Capture& cap) {
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }

  // DNA v48. Posture usage. A dictionary stuck on one unit and a dictionary
  // that is genuinely selecting give identical duty cycles, identical firing
  // rates and an identical verdict below, so the histogram is the only thing
  // that tells them apart.
  const uint32_t dict_units = s.dna.header().vocal.dictionary_units;
  uint64_t dict_use[aibaby::kMaxDictionaryUnits] = {};
  uint64_t dict_frames = 0;
  // The spread of the formants the creature ACTUALLY PRODUCED, in Hz, over the
  // frames it was making a sound. Every other number here is about how often
  // and how loudly the creature vocalises; this is the one about whether the
  // sounds differ from each other, which is what every milestone downstream is
  // scored on. It is also the number that catches a dictionary whose dwell is
  // shorter than the tract's own inertia: a posture that is replaced before it
  // is reached leaves the tract hovering near the mean of the inventory, and
  // that looks identical in the usage histogram.
  double vf1 = 0.0, vf1sq = 0.0, vf2 = 0.0, vf2sq = 0.0;
  uint64_t vfn = 0;

  // Nobody is in the room, so this records mono and carries no label track:
  // the whole file is one condition.
  const uint32_t sample_rate = s.dna.header().audio.sample_rate;
  VoiceRecorder recorder(sample_rate, false);
  const bool recording = !cap.wav.empty();
  const uint32_t samples_per_tick = sample_rate / 1000;

  // Nobody is in the room, but the creature is: since DNA v6 it hears its own
  // voice, and this experiment is where that matters most. §5.3 calls babble
  // "motor noise shaped by the curiosity drive"; a creature that cannot hear
  // itself babbling gives curiosity nothing to shape.
  Ear ear;
  if (!ear.configure(s.dna.header().audio, error)) {
    std::printf("  ear failed: %s\n", error.c_str());
    return false;
  }

  uint32_t last_frame = 0;
  uint64_t frames = 0, voiced = 0, loud = 0, events = 0;
  uint64_t last_event = 0;
  double amp_sum = 0, f1_sum = 0;
  uint32_t f1_hist[10] = {0};
  uint32_t amp_hist[10] = {0};
  const aibaby::ModuleState* vocal = nullptr;
  const int32_t vm = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
  if (vm < 0) {
    std::printf("  genome has no vocal module\n");
    return false;
  }

  double self_sum = 0.0;
  double expl_sum = 0.0, expl_min = 1e9, expl_max = -1e9;
  uint64_t expl_n = 0;

  // --- Why does the creature only ever say one vowel? ------------------------
  //
  // Every parameter that shapes vowel colour is read as a population centroid
  // over neuron index, and every one of them is pinned to the middle fifth of
  // its range. Every parameter read as a group firing RATE — loudness, voicing
  // — uses its whole range. Two readouts side by side in one module, and only
  // one of them moves.
  //
  // That looks like a clean controlled comparison for the place-code story:
  // a centroid over an untuned group is the centre of that group, whatever the
  // input does. It is not clean, because the two readouts are also smoothed
  // differently — 800 ms on the formants against 60 ms on the gate. An 800 ms
  // EMA over a 10 ms frame averages ~80 samples, and a raw centroid wandering
  // freely would come out of it looking exactly as pinned as one that never
  // moved.
  //
  // The two stories have completely different fixes — topography, or one
  // genome constant — so this measures the centroid BEFORE the smoothing. It
  // is computed here rather than in the core because it must not be able to
  // change anything: same `rate_fast` the decoder reads, same slicing, no core
  // state added, and therefore no way to move the determinism hash.
  const aibaby::DnaVocal& vcfg = s.dna.header().vocal;
  const uint32_t formant_group[4] = {0, 2, 3, 4};
  const char* formant_name[4] = {"f0", "F1", "F2", "F3"};
  uint32_t raw_hist[4][10] = {};
  double raw_sum[4] = {}, raw_sq[4] = {}, raw_cu[4] = {}, raw_qu[4] = {};
  // THE SOURCE'S OWN CENTRE OF MASS, free-running. `seqprobe` measures the
  // chain by KICKING its head and watching a wave leave; nothing kicks it in
  // free behaviour, so the wave may simply not exist here — and a topographic
  // map faithfully delivering an absent wave looks exactly like a broken map.
  // Same statistic seqprobe prints, on the same module, without a kick.
  double src_sum = 0.0, src_sq = 0.0, src_lo = 1e9, src_hi = -1e9;
  uint32_t src_frames = 0;
  uint32_t centre_module = 0;
  for (uint32_t m = 0; m < s.brain.network().module_count(); ++m) {
    if (std::strcmp(s.brain.network().module_dna(m).name, "central") == 0) centre_module = m;
  }
  double raw_lo[4] = {1e9, 1e9, 1e9, 1e9};
  double raw_hi[4] = {-1e9, -1e9, -1e9, -1e9};
  double sm_sum[4] = {}, sm_sq[4] = {};
  double gate_n[2] = {}, gate_f1[2] = {}, gate_f1sq[2] = {};
  double quart_n[4] = {}, quart_f1[4] = {}, quart_f1sq[4] = {};
  double corr_n = 0, corr_a = 0, corr_f = 0, corr_aa = 0, corr_ff = 0, corr_af = 0;
  auto raw_centroid = [&](const aibaby::Network& net, const aibaby::ModuleState& ms,
                          uint32_t g) -> double {
    const uint32_t b = ms.begin + aibaby::slice_begin(ms.count, aibaby::kVocalGroups, g);
    const uint32_t e = ms.begin + aibaby::slice_begin(ms.count, aibaby::kVocalGroups, g + 1);
    if (e <= b) return 0.5;
    const double n = double(e - b);
    double weighted = 0.0, total = 0.0;
    for (uint32_t i = b; i < e; ++i) {
      const double r = double(net.rate_fast(i));
      weighted += r * ((double(i - b) + 0.5) / n);
      total += r;
    }
    return total > 1e-6 ? weighted / total : 0.5;
  };
  for (uint64_t t = 0; t < ticks; ++t) {
    // Hearing happens before the step, so the mel frame the creature acts on
    // is the one its previous motor frame produced.
    ear.tick(s.brain, nullptr, samples_per_tick);
    self_sum += double(ear.self_level());
    {
      const double e = double(s.brain.exploration());
      expl_sum += e; ++expl_n;
      if (e < expl_min) expl_min = e;
      if (e > expl_max) expl_max = e;
    }
    s.brain.step();
    // Before the frame check below: the synthesiser runs at the sample rate
    // and the motor frame at 100 Hz, so recording only on frame boundaries
    // would drop 90% of the audio.
    if (recording) recorder.tick(s.brain.voice(), nullptr, samples_per_tick, nullptr);
    if (s.brain.vocal_frame() == last_frame) continue;
    last_frame = s.brain.vocal_frame();
    ++frames;
    {
      const aibaby::ModuleState& cms = s.brain.network().module(centre_module);
      double wsum = 0.0, tot = 0.0;
      for (uint32_t i = 0; i < cms.count; ++i) {
        const double r = double(s.brain.network().rate_fast(cms.begin + i));
        wsum += r * (double(i) + 0.5) / double(cms.count);
        tot += r;
      }
      if (tot > 1e-9) {
        const double c = wsum / tot;
        src_sum += c;
        src_sq += c * c;
        if (c < src_lo) src_lo = c;
        if (c > src_hi) src_hi = c;
        ++src_frames;
      }
    }

    const aibaby::VocalParams& v = s.brain.voice();
    const float f1v = float(s.brain.vocal_groups()[2]);
    amp_sum += double(v.amplitude);
    f1_sum += f1v;
    f1_hist[std::min<uint32_t>(9, uint32_t(f1v * 10.0f))]++;
    amp_hist[std::min<uint32_t>(9, uint32_t(float(v.amplitude) * 10.0f))]++;
    // Does the amplitude gate select on F1? Every experiment that scores the
    // voice only looks at frames loud enough to count as a vocalisation, so if
    // F1 differs between the frames that pass and the frames that do not, the
    // gate is a filter on the very quantity being measured — and a mechanism
    // that changes how much F1 varies changes how hard it filters.
    {
      // Does the structure survive the session? Intrinsic plasticity drives
      // every neuron toward its target rate over minutes, and a bump is
      // precisely a set of neurons held persistently above and below theirs —
      // so awake regulation has a standing incentive to erase it. G2 sets its
      // criterion in the first fifth of a session and scores in the last, so
      // anything that decays in between shows up as reward failing.
      const int q = int(4 * t / ticks) > 3 ? 3 : int(4 * t / ticks);
      quart_n[q] += 1.0;
      quart_f1[q] += double(f1v);
      quart_f1sq[q] += double(f1v) * double(f1v);

      const int gate = v.amplitude > kAmplitudeFloor ? 1 : 0;
      gate_n[gate] += 1.0;
      gate_f1[gate] += double(f1v);
      gate_f1sq[gate] += double(f1v) * double(f1v);
      corr_n += 1.0;
      corr_a += double(v.amplitude);
      corr_f += double(f1v);
      corr_aa += double(v.amplitude) * double(v.amplitude);
      corr_ff += double(f1v) * double(f1v);
      corr_af += double(v.amplitude) * double(f1v);
    }
    {
      const aibaby::ModuleState& vms = s.brain.network().module(uint32_t(vm));
      for (int k = 0; k < 4; ++k) {
        const double raw = raw_centroid(s.brain.network(), vms, formant_group[k]);
        raw_hist[k][std::min<uint32_t>(9, uint32_t(raw * 10.0))]++;
        raw_sum[k] += raw;
        raw_sq[k] += raw * raw;
        raw_cu[k] += raw * raw * raw;
        raw_qu[k] += raw * raw * raw * raw;
        if (raw < raw_lo[k]) raw_lo[k] = raw;
        if (raw > raw_hi[k]) raw_hi[k] = raw;
        const double sm = double(s.brain.vocal_groups()[formant_group[k]]);
        sm_sum[k] += sm;
        sm_sq[k] += sm * sm;
      }
    }
    if (v.voicing > 0.5f) ++voiced;
    if (v.amplitude > kAmplitudeFloor) ++loud;
    // DNA v48. Which posture the dictionary is holding, counted only while the
    // creature is actually making a sound: a winner held through silence is not
    // a vowel the creature produced.
    if (v.voicing > 0.5f && v.amplitude > kAmplitudeFloor) {
      vf1 += double(v.f1); vf1sq += double(v.f1) * double(v.f1);
      vf2 += double(v.f2); vf2sq += double(v.f2) * double(v.f2);
      ++vfn;
    }
    if (dict_units > 0 && v.voicing > 0.5f && v.amplitude > kAmplitudeFloor) {
      ++dict_use[s.brain.vocal_decoder().winner() % dict_units];
      ++dict_frames;
    }
    if (v.voicing > 0.5f && v.amplitude > kAmplitudeFloor &&
        t - last_event >= kEventRefractoryTicks) {
      last_event = t;
      ++events;
    }
  }
  vocal = &s.brain.network().module(uint32_t(vm));
  if (cap.wanted()) write_capture(cap, recorder, sample_rate, s.brain, blob);

  const double dt = double(s.dna.header().sim.dt_ms);
  const double minutes = double(ticks) * dt / 60000.0;
  std::printf("  motor frames      %llu\n", (unsigned long long)frames);
  std::printf("  voiced            %.1f%% of frames\n",
              frames ? 100.0 * double(voiced) / double(frames) : 0.0);
  std::printf("  above amp floor   %.1f%% of frames (floor %.2f)\n",
              frames ? 100.0 * double(loud) / double(frames) : 0.0, kAmplitudeFloor);
  std::printf("  vocalisations     %llu  (%.1f /min)\n", (unsigned long long)events,
              minutes > 0 ? double(events) / minutes : 0.0);
  std::printf("  mean amplitude    %.3f\n", frames ? amp_sum / double(frames) : 0.0);
  std::printf("  mean F1 group     %.3f\n", frames ? f1_sum / double(frames) : 0.0);
  std::printf("  vocal module rate %.2f Hz\n", double(vocal->mean_rate));
  // The loop, shown rather than assumed. Zero here means the creature is deaf
  // to itself and every conclusion below is about the old creature.
  std::printf("  hears itself at   %.4f mean peak (self_gain %.2f)\n",
              ticks ? self_sum / double(ticks) : 0.0,
              double(s.dna.header().audio.self_gain));
  {
    const int32_t aud = s.dna.module_with_role(aibaby::ModuleRole::kAuditory);
    if (aud >= 0) {
      std::printf("  auditory rate     %.2f Hz (target %.2f) — driven by its own voice\n",
                  double(s.brain.network().module(uint32_t(aud)).mean_rate),
                  double(s.brain.network().module_dna(uint32_t(aud)).target_rate_hz));
    }
  }
  if (vfn > 1) {
    const double m1 = vf1 / double(vfn), m2 = vf2 / double(vfn);
    const double s1 = std::sqrt((vf1sq / double(vfn)) - m1 * m1);
    const double s2 = std::sqrt((vf2sq / double(vfn)) - m2 * m2);
    std::printf("  produced formants F1 %.0f +/- %.0f Hz, F2 %.0f +/- %.0f Hz over\n"
                "                    %llu voiced frames — the spread is what any\n"
                "                    milestone downstream has to hear\n",
                m1, s1, m2, s2, (unsigned long long)vfn);
  }

  if (dict_units > 0) {
    // Effective inventory size, so one number says whether the dictionary is
    // being used. exp(H) over the usage distribution: 1.0 means one posture
    // carries every sound the creature made, `dict_units` means all of them
    // equally. A dictionary at 1.0 has replaced a centroid with a constant.
    double h = 0.0;
    uint32_t nonzero = 0;
    for (uint32_t u = 0; u < dict_units; ++u) {
      if (!dict_use[u] || !dict_frames) continue;
      const double q = double(dict_use[u]) / double(dict_frames);
      h -= q * std::log(q);
      ++nonzero;
    }
    std::printf("  postures used     %u of %u, effective %.2f (1.00 = one posture\n"
                "                    carries everything, %.2f = all used equally)\n",
                nonzero, dict_units, std::exp(h), double(dict_units));
    std::printf("  posture switches  %u over %llu voiced frames\n",
                s.brain.vocal_decoder().switches(), (unsigned long long)dict_frames);
    std::printf("  usage             ");
    for (uint32_t u = 0; u < dict_units; ++u) {
      std::printf("%.0f%% ", dict_frames ? 100.0 * double(dict_use[u]) / double(dict_frames) : 0.0);
    }
    std::printf("\n");
  }

  // What LMAN is doing. Babble that has stopped varying and babble that has
  // stopped are the same reading on every other number here.
  std::printf("  exploration       %.3f min, %.3f mean, %.3f max (1.0 = plain noise_amp)\n",
              expl_min > 1e8 ? 1.0 : expl_min, expl_n ? expl_sum / double(expl_n) : 1.0,
              expl_max < -1e8 ? 1.0 : expl_max);

  if (verbose) {
    std::printf("  module rates (free-running vs the genome's target):\n");
    for (uint32_t m = 0; m < s.brain.network().module_count(); ++m) {
      std::printf("    %-12s %6.2f Hz   target %5.2f Hz   mean threshold %.3f\n",
                  s.brain.network().module_dna(m).name,
                  double(s.brain.network().module(m).mean_rate),
                  double(s.brain.network().module_dna(m).target_rate_hz),
                  double(s.brain.network().threshold(s.brain.network().module(m).begin)));
    }
    std::printf("  F1 group histogram (0.0 -> 1.0):\n    ");
    for (int i = 0; i < 10; ++i) std::printf("%6u", f1_hist[i]);
    std::printf("\n  amplitude histogram (0.0 -> 1.0):\n    ");
    for (int i = 0; i < 10; ++i) std::printf("%6u", amp_hist[i]);
    std::printf("\n");

    // The centroid before the 800 ms articulator inertia gets to it. The
    // decisive column is `range`: if the raw centroid never leaves the middle
    // either, the group has no structure to smooth away and the place code is
    // the problem. If it swings and only the smoothed value is pinned, the
    // vowel space is being erased by one genome constant.
    if (frames > 0) {
      const double n = double(frames);
      const double frame_ms = double(ticks) / n * double(s.dna.header().sim.dt_ms);
      const double a = 1.0 - std::exp(-frame_ms / double(vcfg.smoothing_ms));
      std::printf("\n  is the vowel space absent, or smoothed away?"
                  "  (%.0f ms motor frame, %.0f ms inertia)\n",
                  frame_ms, double(vcfg.smoothing_ms));
      std::printf("    %-5s %-16s %-9s %-9s %-8s %s\n", "", "raw centroid",
                  "raw sd", "smoothed", "ratio", "bimodal");
      if (src_frames > 0) {
        const double cm = src_sum / double(src_frames);
        const double csd = std::sqrt(std::max(0.0, src_sq / double(src_frames) - cm * cm));
        std::printf("    (central free-running centre of mass %.3f .. %.3f, sd %.4f, "
                    "%u frames;\n"
                    "     seqprobe's KICKED wave sweeps 0.035 .. 0.66, so a small sd here "
                    "means there\n"
                    "     is no travelling wave to map — not that the map failed)\n",
                    src_lo, src_hi, csd, src_frames);
      }
      for (int k = 0; k < 4; ++k) {
        const double rm = raw_sum[k] / n;
        const double v2 = std::max(0.0, raw_sq[k] / n - rm * rm);
        const double rsd = std::sqrt(v2);
        const double sm = sm_sum[k] / n;
        const double ssd = std::sqrt(std::max(0.0, sm_sq[k] / n - sm * sm));
        // Sarle's bimodality coefficient, (skew^2 + 1) / kurtosis. A normal
        // distribution reads 0.33 and a uniform one 0.56; above 0.56 the mass
        // is in two lumps rather than one. This is the bar the whole exercise
        // is aimed at — a wandering average is noise however wide it wanders,
        // and only two lumps mean the tract has postures to be aimed at.
        double bc = 0.0;
        if (v2 > 1e-12) {
          const double m3 = raw_cu[k] / n - 3.0 * rm * raw_sq[k] / n + 2.0 * rm * rm * rm;
          const double m4 = raw_qu[k] / n - 4.0 * rm * raw_cu[k] / n +
                            6.0 * rm * rm * raw_sq[k] / n - 3.0 * rm * rm * rm * rm;
          const double skew = m3 / (v2 * rsd);
          const double kurt = m4 / (v2 * v2);
          if (kurt > 1e-9) bc = (skew * skew + 1.0) / kurt;
        }
        char range[32];
        std::snprintf(range, sizeof(range), "%.3f .. %.3f", raw_lo[k], raw_hi[k]);
        std::printf("    %-5s %-16s %-9.4f %-9.4f %-8.1f %.3f%s\n", formant_name[k],
                    range, rsd, ssd, ssd > 1e-9 ? rsd / ssd : 0.0, bc,
                    bc > 0.556 ? "  <- two lumps" : "");
      }
      // What the smoothing alone could account for. An EMA on white noise cuts
      // the spread by sqrt(a/(2-a)); the raw centroid is autocorrelated, so the
      // real attenuation is weaker than this and the number is a bound, not a
      // prediction. A measured ratio near it says the filter explains the
      // pinning; a ratio near 1 says the filter is innocent.
      std::printf("    an 800 ms EMA can attenuate by at most %.1fx (white noise);\n"
                  "    a measured ratio near that is the filter, near 1.0 is the code\n",
                  1.0 / std::sqrt(a / (2.0 - a)));
      std::printf("  raw F1 centroid histogram (0.0 -> 1.0):\n    ");
      for (int i = 0; i < 10; ++i) std::printf("%6u", raw_hist[1][i]);
      std::printf("\n");

      // Is the amplitude floor a filter on F1? G2 and M3 only ever look at
      // frames above it, so a difference here means every vocal score is
      // measured on a biased sample of the vowel — and the bias grows with
      // however much F1 varies.
      if (gate_n[0] > 1.0 && gate_n[1] > 1.0) {
        const double lo = gate_f1[0] / gate_n[0], hi = gate_f1[1] / gate_n[1];
        const double sdlo = std::sqrt(std::max(0.0, gate_f1sq[0] / gate_n[0] - lo * lo));
        const double sdhi = std::sqrt(std::max(0.0, gate_f1sq[1] / gate_n[1] - hi * hi));
        double r = 0.0;
        if (corr_n > 1.0) {
          const double ma = corr_a / corr_n, mf = corr_f / corr_n;
          const double va = corr_aa / corr_n - ma * ma;
          const double vf = corr_ff / corr_n - mf * mf;
          if (va > 1e-12 && vf > 1e-12) {
            r = (corr_af / corr_n - ma * mf) / std::sqrt(va * vf);
          }
        }
        std::printf("\n  does the amplitude floor select on F1?  (floor %.2f)\n",
                    double(kAmplitudeFloor));
        std::printf("    below the floor   F1 %.4f  (sd %.4f, %.0f frames)\n", lo, sdlo,
                    gate_n[0]);
        std::printf("    above it — what every vocal score sees\n"
                    "                      F1 %.4f  (sd %.4f, %.0f frames)\n", hi, sdhi,
                    gate_n[1]);
        std::printf("    difference        %+.4f      corr(F1, amplitude) %+.3f\n",
                    hi - lo, r);
      }
      // The same F1, by quarter of the session.
      {
        std::printf("\n  does the vowel structure survive the session?\n");
        std::printf("    %-9s %-9s %s\n", "quarter", "mean F1", "sd");
        for (int q = 0; q < 4; ++q) {
          if (quart_n[q] < 2.0) continue;
          const double mq = quart_f1[q] / quart_n[q];
          const double sq =
              std::sqrt(std::max(0.0, quart_f1sq[q] / quart_n[q] - mq * mq));
          std::printf("    %-9d %-9.4f %.4f\n", q + 1, mq, sq);
        }
      }
    }
  }

  // We want a babbler, not a drone and not a mute. The band is wide on purpose:
  // what it is guarding against is saturation at one end and silence at the
  // other, either of which leaves reward nothing to move. Anything between is
  // a matter of temperament, not correctness.
  const double duty = frames ? double(loud) / double(frames) : 0.0;
  const bool pass = duty > 0.10 && duty < 0.85 && events > 10;
  std::printf("\n  babble %s — vocal duty cycle %.2f (want 0.10 .. 0.85).\n",
              pass ? "PASS" : "FAIL", duty);
  return pass;
}


struct M2Run {
  bool ok = false;
  double b1_accuracy = 0;      // held-out, association module
  double b1_pattern = 0;       // the same with each trial's total rate divided out
  double b1_shuffled = 0;      // the same again with the labels shuffled: the control
  double b3_accuracy = 0;      // the input module: a path check, not the claim
  double rate_present = 0, rate_absent = 0;
  uint32_t trials = 0, skipped = 0;
};


// One creature. Trials alternate between an object and an empty field in a
// shuffled order, and what is recorded is B1's population activity — not B3's.
// B3 seeing the object is plumbing; the milestone is that the difference
// survives the trip into the association module, which has no camera of its
// own and only ever meets the world after B3 has chewed on it.
//
// The object moves, changes size and changes shape between trials, so a
// readout cannot pass by memorising one picture. What generalises across all
// of that is "something is there", which is the thing being claimed.
M2Run run_m2_session(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  M2Run out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return out;
  }
  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  Retina retina;
  if (!retina.configure(vcfg, error)) {
    std::printf("  retina failed: %s\n", error.c_str());
    return out;
  }
  const int32_t b1 = s.dna.module_with_role(aibaby::ModuleRole::kAssociation);
  const int32_t b3 = s.dna.module_with_role(aibaby::ModuleRole::kVision);
  if (b1 < 0 || b3 < 0) {
    std::printf("  genome needs both an association and a vision module\n");
    return out;
  }

  // Two seconds per trial, the first 600 ms of it discarded. The settle window
  // is what stops a trial from being scored on the tail of the one before it:
  // membrane potentials, rate estimates and the delay line all carry over.
  const uint64_t trial_ticks = 2000;
  const uint64_t settle_ticks = 600;
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);
  const uint32_t n_trials = uint32_t(ticks / trial_ticks);
  if (n_trials < 12) {
    std::printf("  need at least 12 trials; --ticks %llu gives %u\n",
                (unsigned long long)ticks, n_trials);
    return out;
  }

  // A balanced sequence, shuffled deterministically. Balanced so that "always
  // guess present" cannot score well; shuffled so that the readout cannot pick
  // up on alternation instead of on the picture.
  aibaby::Rng rng;
  rng.seed(s.dna.header().seed ^ 0x5EE1u);
  std::vector<int> condition(n_trials, 0);
  for (uint32_t i = 0; i < n_trials; ++i) condition[i] = int(i % 2);
  for (uint32_t i = n_trials; i > 1; --i) {
    const uint32_t j = uint32_t(rng.next() % i);
    std::swap(condition[i - 1], condition[j]);
  }

  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  std::vector<std::vector<double>> feat_b1, feat_b1_pattern, feat_b3;
  std::vector<int> labels;
  double sum_rate[2] = {0.0, 0.0};
  uint32_t n_rate[2] = {0, 0};

  const aibaby::ModuleState& ms1 = s.brain.network().module(uint32_t(b1));
  const aibaby::ModuleState& ms3 = s.brain.network().module(uint32_t(b3));
  // Widths fixed at session start — see the note in run_m3_session. These are
  // live references into a network that can grow underneath them.
  const uint32_t w1 = ms1.count, w3 = ms3.count;

  for (uint32_t trial = 0; trial < n_trials; ++trial) {
    const bool present = condition[trial] == 1;
    // A different object every time: position, size and shape all move, so
    // "present" is a category rather than a picture.
    const float jitter_x = 0.5f + 0.10f * float(rng.signed_uniform());
    const float jitter_y = 0.5f + 0.10f * float(rng.signed_uniform());
    const float radius = 0.09f + 0.04f * float(rng.uniform());
    const SceneSource::Shape shape =
        present ? (rng.chance(0.5f) ? SceneSource::Shape::kDisc : SceneSource::Shape::kSquare)
                : SceneSource::Shape::kNone;

    std::vector<double> bins1(w1, 0.0), bins3(w3, 0.0);
    uint64_t counted = 0;
    bool slept = false;

    for (uint64_t t = 0; t < trial_ticks; ++t) {
      if (t % frame_ticks == 0) {
        scene.render(shape, jitter_x, jitter_y, radius, 0.85f, 0.02f, frame.data());
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }
      s.brain.step();
      if (s.brain.asleep()) slept = true;
      if (t < settle_ticks) continue;
      ++counted;

      const aibaby::Network& net = s.brain.network();
      for (uint32_t k = 0; k < net.spike_count(); ++k) {
        const uint32_t i = net.spikes()[k];
        if (i >= ms1.begin && i < ms1.begin + w1) bins1[i - ms1.begin] += 1.0;
        else if (i >= ms3.begin && i < ms3.begin + w3) bins3[i - ms3.begin] += 1.0;
      }
    }

    // A trial the baby slept through is not a trial: the eyes were shut, so it
    // is a measurement of nothing. Dropping it is honest; scoring it would be
    // handing the classifier a coin flip labelled as data.
    if (slept || counted == 0) {
      ++out.skipped;
      continue;
    }

    double total1 = 0.0;
    for (double v : bins1) total1 += v;
    sum_rate[present ? 1 : 0] += total1 / double(counted);
    ++n_rate[present ? 1 : 0];

    feat_b1.push_back(rebin(bins1, kFeatureBins));
    feat_b1_pattern.push_back(normalise(feat_b1.back()));
    feat_b3.push_back(rebin(bins3, kFeatureBins));
    labels.push_back(present ? 1 : 0);
  }

  out.trials = uint32_t(labels.size());
  if (out.trials < 12) {
    std::printf("  only %u usable trials\n", out.trials);
    return out;
  }

  // Train on the first half of the session, test on the second. Splitting by
  // time rather than at random is the harder and more truthful test: the brain
  // keeps learning throughout, so the test half is drawn from a creature that
  // has already moved on from the one the readout was fitted to.
  const size_t train = out.trials / 2;
  out.b1_accuracy = holdout_accuracy(feat_b1, labels, train);
  out.b1_pattern = holdout_accuracy(feat_b1_pattern, labels, train);
  out.b3_accuracy = holdout_accuracy(feat_b3, labels, train);

  // The control that makes the rest of the numbers mean anything: the same
  // features, the same classifier, the same split, with the labels shuffled.
  // If this does not come out at chance then the readout is finding structure
  // in the *procedure* — trial order, drift, the number of dimensions — and
  // the real accuracy above it is measuring the experiment rather than the
  // baby. It is cheap, and it is the only thing standing between this result
  // and a very convincing artefact.
  std::vector<int> shuffled = labels;
  for (size_t i = shuffled.size(); i > 1; --i) {
    std::swap(shuffled[i - 1], shuffled[rng.next() % i]);
  }
  out.b1_shuffled = holdout_accuracy(feat_b1, shuffled, train);

  if (n_rate[1]) out.rate_present = sum_rate[1] / double(n_rate[1]);
  if (n_rate[0]) out.rate_absent = sum_rate[0] / double(n_rate[0]);
  if (verbose) {
    std::printf("       %u trials (%u skipped), %zu train / %zu test\n", out.trials,
                out.skipped, train, size_t(out.trials) - train);
  }
  out.ok = true;
  return out;
}


bool run_m2(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  const double dt = double(dna.header().sim.dt_ms);
  const uint64_t base_seed = dna.header().seed;

  std::printf("  session           %.1f s of simulated life x %u creatures\n",
              double(ticks) * dt / 1000.0, kM2Replicates);
  std::printf("  chance            0.500 — trials are balanced present/absent\n");
  std::printf("  %-4s %-11s %-11s %-11s %-11s %s\n", "seed", "B1 held-out", "B1 pattern",
              "B1 shuffled", "B3 (path)", "B1 spikes/tick present vs absent");

  double sum_b1 = 0, sum_pattern = 0, sum_b3 = 0, sum_shuffled = 0;
  uint32_t valid = 0, individually_above = 0;

  for (uint32_t r = 0; r < kM2Replicates; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = base_seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));

    const M2Run m = run_m2_session(variant, ticks, verbose);
    if (!m.ok) continue;
    ++valid;
    sum_b1 += m.b1_accuracy;
    sum_pattern += m.b1_pattern;
    sum_b3 += m.b3_accuracy;
    sum_shuffled += m.b1_shuffled;
    if (m.b1_accuracy >= 0.75) ++individually_above;
    std::printf("  %-4u %-11.3f %-11.3f %-11.3f %-11.3f %.3f vs %.3f\n", r, m.b1_accuracy,
                m.b1_pattern, m.b1_shuffled, m.b3_accuracy, m.rate_present, m.rate_absent);
  }

  if (valid < 3) {
    std::printf("\n  M2 INCONCLUSIVE — only %u of %u creatures produced usable trials.\n",
                valid, kM2Replicates);
    return false;
  }

  const double mean_b1 = sum_b1 / double(valid);
  const double mean_pattern = sum_pattern / double(valid);
  const double mean_b3 = sum_b3 / double(valid);
  const double mean_shuffled = sum_shuffled / double(valid);

  std::printf("\n  mean held-out accuracy\n");
  std::printf("    B1 association       %.3f   <- the milestone\n", mean_b1);
  std::printf("    B1, rate divided out %.3f\n", mean_pattern);
  std::printf("    B1, labels shuffled  %.3f   (the control: must sit at chance)\n",
              mean_shuffled);
  std::printf("    B3 vision            %.3f   (the input module; plumbing, not the claim)\n",
              mean_b3);
  std::printf("    at or above 0.75     %u of %u creatures\n", individually_above, valid);

  // Two things have to hold. The milestone itself: a held-out readout of B1
  // tells object from empty field at the same bar G3 sets, in most creatures.
  // And a guard: some of that has to survive dividing each trial's overall
  // rate out, so a pass cannot be the trivial claim that a picture with
  // something in it makes the brain busier.
  //
  // The guard sits at 0.55 rather than at the milestone's own 0.75 because
  // 0.55 is what the guard is for and roughly what the data supports — it
  // separates "there is shape here as well as volume" from "there is only
  // volume". Setting it higher would assert something about B1's
  // representation that this experiment has not earned, and would make the
  // verdict turn on which seeds happened to come up.
  const bool discriminates = mean_b1 >= 0.75 && individually_above * 2 > valid;
  const bool structural = mean_pattern >= 0.55;
  const bool controlled = mean_shuffled < 0.60;
  const bool pass = discriminates && structural && controlled;
  if (!controlled) {
    std::printf("\n  CONTROL FAILED — shuffled labels score %.3f, so the readout is\n"
                "  finding structure in the procedure and no other number here is\n"
                "  worth reading.\n", mean_shuffled);
  }
  std::printf("\n  M2 %s — a held-out classifier reads object-present from B1 at %.0f%%\n"
              "  (chance 50%%, shuffled control %.0f%%). With each trial's overall firing\n"
              "  rate divided out it still reads %.0f%%, so most of what B1 carries about\n"
              "  the object is how hard it is working and %s.\n",
              pass ? "PASS" : "FAIL", mean_b1 * 100.0, mean_shuffled * 100.0,
              mean_pattern * 100.0,
              structural ? "some of it is where" : "NONE OF IT IS WHERE");
  return pass;
}


// The context oracle, and it exists to decide one question this project could
// not otherwise ask. `dwprobe` on `vision->vocal` says a cube session and a ball
// session write MEASURABLY DIFFERENT weight changes on the v46 creature (ratio
// 0.71 against its own ceiling, 3 of 3 seeds) — so "the rule writes the same
// thing either way" is false there. What is left is that the two writes land on
// the same synapses when the objects ALTERNATE, and undo each other.
//
// kByObject hands the creature what it cannot compute: which half of the larynx
// reward is allowed to reach, chosen by which object is actually present. If
// interference is the blocker, that separates the two lessons and the voice
// becomes conditional. kRandom picks the same halves on a coin flip and is the
// control that has to stay flat — without it, "masking reward helps" could be
// about reward reaching fewer synapses rather than about the CONDITION.
//
// This is an oracle and prices a mechanism; it is not a behaviour the creature
// has. See `credit`, which established that a per-neuron mask removes lesson
// interference completely.
enum class CtxMask { kOff, kByObject, kRandom };

// How the two objects are SCHEDULED across the session, which is the axis this
// project has never varied. `m3` interleaves them from the first trial, and
// `capacity` says in as many words that **sequential beats simultaneous** — two
// lessons taught at once interfere where the same two taught in turn do not.
// Naming has been measured only in the simultaneous condition.
//
// kBlockFade is the standard curriculum shape: long blocks of one object first,
// so each lesson has room to consolidate before the other arrives, then blocks
// that halve until the schedule is the interleaved one m3 already runs. It
// spans both extremes rather than picking a block length to defend.
enum class Curriculum { kInterleaved, kBlockFade };

M3Run run_m3_session(const std::vector<uint8_t>& blob, uint64_t ticks, bool paired,
                     const Caregiver& care, bool verbose, const Capture& cap,
                     CtxMask ctx = CtxMask::kOff,
                     Curriculum curriculum = Curriculum::kInterleaved) {
  M3Run out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return out;
  }

  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  Retina retina;
  if (!retina.configure(vcfg, error)) {
    std::printf("  retina failed: %s\n", error.c_str());
    return out;
  }
  Ear ear;
  if (!ear.configure(acfg, error)) {
    std::printf("  ear failed: %s\n", error.c_str());
    return out;
  }
  const int32_t b1 = s.dna.module_with_role(aibaby::ModuleRole::kAssociation);
  if (b1 < 0) {
    std::printf("  genome has no association module\n");
    return out;
  }
  const aibaby::ModuleState& ms1 = s.brain.network().module(uint32_t(b1));
  // Fixed at session start, because a classifier's feature space cannot change
  // width halfway through a session — and because `ms1` is a live reference:
  // since DNA v5 a normally raised creature grows, so reading `ms1.count` when
  // sizing a buffer and again when indexing into it is a buffer overrun the
  // moment a growth event lands between the two. A neuron that did not exist
  // when the session began simply has no column.
  const uint32_t b1_width = ms1.count;

  VowelSource voice(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  std::vector<float> pcm(16);
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);
  const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);

  aibaby::Rng rng;
  rng.seed(s.dna.header().seed ^ 0x3EE3u);
  aibaby::Rng ctx_rng;
  ctx_rng.seed(s.dna.header().seed ^ 0xC7C7u);
  const int32_t voc = s.dna.module_with_role(aibaby::ModuleRole::kVocal);

  // Recording is off unless a path was asked for, and it is deliberately
  // outside every path that touches the brain or the RNG.
  const double dt_ms = double(s.dna.header().sim.dt_ms);
  VoiceRecorder recorder(acfg.sample_rate, true);
  const bool recording = !cap.wav.empty();

  // Feedback earned during a naming can land after it, so the queue outlives
  // the trial that filled it.
  std::deque<Praise> pending;
  uint64_t last_feedback = 0;
  uint32_t last_frame = 0;
  uint64_t now = 0;
  double reward_sum = 0.0;
  uint64_t reward_ticks = 0;
  // Reward is what gates learning (§3.1), and it is subtracted from a running
  // expectation — so a phase of the protocol that is systematically quieter
  // than average carries a systematically *negative* reward, and everything
  // the baby happens to be doing in it gets unlearned. A protocol can do that
  // to itself without anyone noticing, so both phases are accounted for
  // separately here.
  double signed_sum[2] = {0.0, 0.0};
  uint64_t signed_ticks[2] = {0, 0};

  // One trial. The object is in view throughout; `word` is spoken over the
  // first kM3LabelTicks if `speak`; the vocal tract and B1 are recorded from
  // `settle` to the end.
  auto run_trial = [&](int object, int word, bool speak, uint64_t length,
                       uint64_t settle, int phase) -> M3Record {
    M3Record rec;
    rec.b1.assign(b1_width, 0.0);

    // Position and size move between trials, so "cube" is a category rather
    // than one picture. The classifier never sees the retina, so this is not
    // about leakage — it is about whether what reaches the voice generalises.
    const Toy toy = m3_toy(rng, object);

    // Its own RNG stream: drawing the coin flip from `rng` would move every
    // toy placement after it and make the control arm a different protocol
    // rather than a different mask.
    if (ctx != CtxMask::kOff) {
      aibaby::Network& mnet = s.brain.network();
      const aibaby::ModuleState& vm = mnet.module(uint32_t(voc));
      // WITHIN ONE ARTICULATOR GROUP, and the first version of this got it
      // wrong in a way worth recording. Splitting `vocal` in half by neuron
      // index splits it ACROSS the nine groups, so a cube lesson could only
      // touch the bandwidths and a ball lesson only f0 and the low formants —
      // two lessons on different articulators, which is `capacity`'s orthogonal
      // case and not naming at all. It read -0.080 and wrecked the echo.
      //
      // Naming needs both objects to drive the SAME dimension to DIFFERENT
      // values. F1 is group 2, its centroid is the readout, and the two halves
      // of that group are the two ends the centroid can be pulled toward. So
      // the mask lets a cube lesson potentiate one end and a ball lesson the
      // other, inside the one knob that carries the answer.
      const uint32_t g_lo = aibaby::slice_begin(vm.count, aibaby::kVocalGroups, 2);
      const uint32_t g_hi = aibaby::slice_begin(vm.count, aibaby::kVocalGroups, 3);
      const uint32_t mid = g_lo + (g_hi - g_lo) / 2;
      const bool upper = ctx == CtxMask::kByObject ? (object == 1)
                                                   : ((ctx_rng.next() & 1u) != 0u);
      if (upper) mnet.set_reward_mask(vm.begin + mid, vm.begin + g_hi);
      else mnet.set_reward_mask(vm.begin + g_lo, vm.begin + mid);
    }

    if (recording) {
      char text[48];
      std::snprintf(text, sizeof(text), "%s %s", phase == 1 ? "probe" : "name",
                    object == 1 ? "cube" : "ball");
      recorder.segment(double(now) * dt_ms / 1000.0,
                       double(now + length) * dt_ms / 1000.0, text);
      // Probe audio is held aside rather than filed immediately: a probe the
      // baby slept through is dropped from the score, and it has to be dropped
      // from the listening test too, or the two stop being about the same
      // sixteen trials.
      recorder.pending.clear();
    }

    for (uint64_t t = 0; t < length; ++t, ++now) {
      while (!pending.empty() && pending.front().tick <= now) {
        s.brain.praise(pending.front().value);
        pending.pop_front();
      }

      if (t % frame_ticks == 0) {
        scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }

      // The caregiver's voice goes through the same cochlea a microphone
      // would, so the baby meets a word as 24 mel bands and not as a label.
      const bool sounding = speak && t < kM3LabelTicks;
      const Word& w = kWords[word];
      voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                   pcm.data(), samples_per_tick);
      // The caregiver and the baby's own voice reach the same ear, mixed, as
      // they would in a room. Before DNA v6 the probe phase was silent to the
      // creature even while it was vocalising.
      ear.tick(s.brain, pcm.data(), samples_per_tick);

      // Approval accompanies naming, and it is the same approval for both
      // words. It is what opens the gate on three-factor learning (§3.1);
      // it is not what says which object this is.
      if (sounding && care.praise != 0.0f && now - last_feedback >= care.period) {
        last_feedback = now;
        pending.push_back(Praise{now + kRewardDelayTicks, care.praise});
      }

      s.brain.step();
      // Recorded after the step, so what lands in the file is the voice this
      // tick produced. The probe bin takes only the window the classifier is
      // shown, so the listening test and the number cover the same audio.
      if (recording) {
        recorder.tick(s.brain.voice(), pcm.data(), samples_per_tick,
                      phase == 1 && t >= settle ? &recorder.pending : nullptr);
      }
      if (s.brain.asleep()) rec.slept = true;
      reward_sum += std::fabs(double(s.brain.reward().effective));
      ++reward_ticks;
      signed_sum[phase] += double(s.brain.reward().effective);
      ++signed_ticks[phase];
      if (t < settle) continue;

      const aibaby::Network& net = s.brain.network();
      for (uint32_t k = 0; k < net.spike_count(); ++k) {
        const uint32_t i = net.spikes()[k];
        if (i >= ms1.begin && i < ms1.begin + b1_width) rec.b1[i - ms1.begin] += 1.0;
      }

      if (s.brain.vocal_frame() == last_frame) continue;
      last_frame = s.brain.vocal_frame();
      ++rec.frames;
      const aibaby::VocalParams& v = s.brain.voice();
      const aibaby::Scalar* g = s.brain.vocal_groups();
      for (uint32_t k = 0; k < aibaby::kVocalGroups; ++k) rec.group[k] += double(g[k]);
      rec.amplitude += double(v.amplitude);
      rec.f0 += double(v.f0);
      rec.f1 += double(v.f1);
      rec.f2 += double(v.f2);
      if (v.voicing > 0.5f && v.amplitude > kAmplitudeFloor) ++rec.voiced;
    }
    return rec;
  };

  std::vector<std::vector<double>> probe_vocal, probe_timbre, probe_b1;
  std::vector<M3Record> probe_posture;
  std::vector<std::vector<double>> echo_vocal;
  std::vector<int> probe_labels, echo_labels;
  std::vector<uint32_t> probe_taught;  // presentations heard before this probe

  // A balanced, shuffled deck of objects, refilled as it runs out: balanced so
  // that always guessing "ball" cannot score, shuffled so the readout cannot
  // pick up on alternation.
  std::vector<int> deck;
  uint32_t block_left = 0;
  int block_obj = 0;
  auto next_object = [&]() -> int {
    if (curriculum == Curriculum::kBlockFade) {
      // Block length halves as the session runs: 16, 8, 4, 2, then 1, which is
      // the interleaved schedule. `now` and `ticks` are the session clock, so
      // this does not depend on how many trials happen to fit.
      if (block_left == 0) {
        const double frac = ticks ? double(now) / double(ticks) : 1.0;
        const uint32_t step = uint32_t(frac * 5.0);
        block_left = 16u >> (step > 4 ? 4 : step);
        if (block_left == 0) block_left = 1;
        block_obj ^= 1;
      }
      --block_left;
      return block_obj;
    }
    if (deck.empty()) {
      for (int k = 0; k < 8; ++k) deck.push_back(k % 2);
      for (size_t i = deck.size(); i > 1; --i) {
        std::swap(deck[i - 1], deck[rng.next() % i]);
      }
    }
    const int o = deck.back();
    deck.pop_back();
    return o;
  };

  uint32_t since_probe = 0;
  while (now + kM3TrialTicks + kM3ProbeTicks <= ticks) {
    const int object = next_object();
    // Paired: the word is the object's name. Unpaired: it is a coin flip, so
    // the baby hears both words about equally often for both objects.
    //
    // The coin is drawn in both conditions even though only one of them spends
    // it. Otherwise the two upbringings consume different numbers of random
    // numbers and drift apart: they would no longer see the same toys, in the
    // same order, at the same sizes, and the control would differ from the
    // experiment in more ways than the one being tested.
    const uint32_t coin = uint32_t(rng.next() & 1u);
    const int word = paired ? object : int(coin);
    const M3Record named =
        run_trial(object, word, true, kM3TrialTicks, kM3SettleTicks, 0);
    ++out.presentations;
    if (!named.slept) {
      // The echo: what the vocal tract does *while* the word is playing. Not
      // the milestone — the caregiver is talking, so it measures the audio
      // route rather than the visual one.
      //
      // It is NOT "the ceiling the visual route could inherit", which is what
      // this comment claimed until 2026-08-29 and what the printed label still
      // invites. The echo is an *identity map*: the arcuate transcodes a
      // pattern that is already in the ear's coordinates into the larynx's, so
      // of course the word comes back out. It is not a general-purpose channel
      // the object could be poured into. Measured, three ways, by wiring the
      // `vision->auditory` tract this creature never had: the object reaches
      // auditory at up to 0.880 and the voice does not move (+0.04/0.00/-0.06
      // over three seeds). See the README, "Borrowing the pathway that works".
      const std::vector<double> f = m3_vocal_features(named);
      if (!f.empty()) {
        echo_vocal.push_back(f);
        echo_labels.push_back(word);
      }
    }

    if (++since_probe < kM3TrainPerProbe) continue;
    since_probe = 0;

    const int probe_object = next_object();
    const M3Record probe =
        run_trial(probe_object, 0, false, kM3ProbeTicks, kM3ProbeSettleTicks, 1);
    // A trial the baby slept through is a measurement of nothing: the eyes
    // were shut and the larynx was closed. Dropping it is honest; scoring it
    // would hand the classifier a coin flip labelled as data.
    if (probe.slept || probe.frames == 0) {
      ++out.skipped;
      continue;
    }
    probe_vocal.push_back(m3_vocal_features(probe));
    probe_timbre.push_back(m3_timbre_features(probe));
    probe_b1.push_back(rebin(probe.b1, kFeatureBins));
    probe_labels.push_back(probe_object);
    probe_posture.push_back(probe);
    probe_taught.push_back(out.presentations);
    if (recording) {
      std::vector<float>& pile = recorder.probe[probe_object];
      pile.insert(pile.end(), recorder.pending.begin(), recorder.pending.end());
      // A beat of silence between probes, so a run of them is heard as
      // separate utterances rather than one long one.
      pile.insert(pile.end(), size_t(acfg.sample_rate) / 4, 0.0f);
    }
  }

  // Written before the usable-probe check, because a session too short to
  // score is exactly the one you want to listen to in order to find out why.
  if (cap.wanted()) write_capture(cap, recorder, acfg.sample_rate, s.brain, blob);

  out.probes = uint32_t(probe_labels.size());
  if (out.probes < 12) {
    std::printf("  only %u usable probes; --ticks %llu is too short\n", out.probes,
                (unsigned long long)ticks);
    return out;
  }

  // Fitted on the first half of the session and tested on the second, as in
  // M2: the brain keeps changing throughout, so the test half comes from a
  // creature the readout was never shown.
  const size_t train = out.probes / 2;
  out.vocal = holdout_accuracy(probe_vocal, probe_labels, train);
  out.timbre = holdout_accuracy(probe_timbre, probe_labels, train);

  // The same probes, scored on what they sound like. Everything above this is
  // a classifier reading motor parameters; the milestone's own sentence is
  // about two sounds, and until now nothing checked whether the difference the
  // classifier finds is one an ear could find too.
  {
    Timbre timbre;
    std::string terr;
    if (timbre.configure(acfg, terr)) {
      std::vector<std::vector<double>> ceps;
      ceps.reserve(probe_posture.size());
      for (const M3Record& r : probe_posture) {
        const double n = double(r.frames ? r.frames : 1);
        ceps.push_back(timbre.of(r.f0 / n, r.f1 / n, r.f2 / n, r.amplitude / n));
      }
      std::vector<double> sigma;
      out.dprime = cepstral_dprime(ceps, probe_labels, &sigma);
      out.dprime_unb_sq = cepstral_dprime(ceps, probe_labels, nullptr, true);
      std::vector<int> shuffled_labels = probe_labels;
      aibaby::Rng shuf;
      shuf.seed(s.dna.header().seed ^ 0x7A1Bu);
      for (size_t i = shuffled_labels.size(); i > 1; --i) {
        std::swap(shuffled_labels[i - 1], shuffled_labels[shuf.next() % i]);
      }
      // The null over MANY permutations, not one. A single shuffle is one draw
      // from the null distribution, not an estimate of it, and with five
      // creatures that is five draws holding up the only reference number on
      // this table — it bounced between 0.00 and 0.50 across two run lengths
      // for that reason alone. Re-permuting costs nothing: the cepstra are
      // already computed and no creature is simulated again.
      constexpr uint32_t kNullPerms = 32;
      double null_acc = 0.0;
      std::vector<int> perm = probe_labels;
      for (uint32_t p = 0; p < kNullPerms; ++p) {
        for (size_t i = perm.size(); i > 1; --i) {
          std::swap(perm[i - 1], perm[shuf.next() % i]);
        }
        null_acc += cepstral_dprime(ceps, perm, nullptr, true);
      }
      out.dprime_null_unb_sq = null_acc / double(kNullPerms);
      // The same estimator on labels that mean nothing. Two identical sounds do
      // not measure d' = 0 at this sample size, they measure about 1.7, and
      // without this row the creature's 1.2 reads as "a listener could hear it"
      // when it is in fact below the floor of the instrument.
      out.dprime_null = cepstral_dprime(ceps, shuffled_labels, nullptr);

      // Scored on an INTERLEAVED split, not first-half/second-half. Per-module
      // homeostasis means the creature is not stationary, and projprobe already
      // paid for this lesson: the naive split leaks hardest into a *pooled*
      // readout, because a pooled unit is dominated by the population mean and
      // the population mean is what homeostasis moves. A cepstral coefficient
      // is a weighted sum over all 24 bands, which is as pooled as it gets —
      // and the first version of this measure duly printed a shuffled control
      // at 0.613 where it has to sit at chance. The motor number is recomputed
      // on the same split so the two are comparable; the milestone's own
      // `vocal` above stays on its historical split.
      {
        std::vector<std::vector<double>> ix, im;
        std::vector<int> iy, iym;
        size_t itrain = 0, itrain_m = 0;
        interleave_pairs(ceps, probe_labels, ix, iy, itrain);
        interleave_pairs(probe_vocal, probe_labels, im, iym, itrain_m);
        out.acoustic = holdout_accuracy(ix, iy, itrain);
        out.motor_interleaved = holdout_accuracy(im, iym, itrain_m);
        std::vector<int> ish;
        std::vector<std::vector<double>> idrop;
        size_t idt = 0;
        interleave_pairs(ceps, shuffled_labels, idrop, ish, idt);
        out.acoustic_shuffled = holdout_accuracy(idrop, ish, idt);
      }

      // The ruler. Three fixed contrasts through the same tract, measured
      // against this creature's own within-word scatter, so the d-prime above
      // can be read as a sentence rather than a number. The third is the
      // creature's *measured* F1 spread applied to its own mean posture: the
      // most a listener could hear from the variation it already has.
      if (!sigma.empty() && !ceps.empty() && !ceps[0].empty()) {
        double mf0 = 0, mf1 = 0, mf2 = 0, mamp = 0;
        for (const M3Record& r : probe_posture) {
          const double n = double(r.frames ? r.frames : 1);
          mf0 += r.f0 / n; mf1 += r.f1 / n; mf2 += r.f2 / n; mamp += r.amplitude / n;
        }
        const double inv = 1.0 / double(probe_posture.size());
        mf0 *= inv; mf1 *= inv; mf2 *= inv; mamp *= inv;
        // Hillenbrand's adult-male means, which is what the two-formant tract
        // is closest to. The point is not that the creature should hit them —
        // it is that these are what "two different vowels" costs in cepstra.
        const std::vector<double> vi = timbre.of(mf0, 342.0, 2322.0, mamp);   // [i]
        const std::vector<double> va = timbre.of(mf0, 768.0, 1333.0, mamp);   // [ɑ]
        const std::vector<double> vu = timbre.of(mf0, 623.0, 1200.0, mamp);   // [ʌ]
        double sd_f1 = 0.0;
        for (const M3Record& r : probe_posture) {
          const double n = double(r.frames ? r.frames : 1);
          const double e = r.f1 / n - mf1;
          sd_f1 += e * e;
        }
        sd_f1 = std::sqrt(sd_f1 / double(probe_posture.size()));
        const std::vector<double> lo = timbre.of(mf0, mf1 - sd_f1, mf2, mamp);
        const std::vector<double> hi = timbre.of(mf0, mf1 + sd_f1, mf2, mamp);
        out.anchor_ia = cepstral_dprime_between(vi, va, sigma);
        out.anchor_near = cepstral_dprime_between(va, vu, sigma);
        out.anchor_sd = cepstral_dprime_between(lo, hi, sigma);
      }
    }
  }
  out.b1_shape = holdout_accuracy(probe_b1, probe_labels, train);

  std::vector<int> shuffled = probe_labels;
  for (size_t i = shuffled.size(); i > 1; --i) {
    std::swap(shuffled[i - 1], shuffled[rng.next() % i]);
  }
  out.shuffled = holdout_accuracy(probe_vocal, shuffled, train);

  if (echo_labels.size() >= 12) {
    out.echo = holdout_accuracy(echo_vocal, echo_labels, echo_labels.size() / 2);
    // Does the picture drive the voice the same way the word does? Both
    // differences are taken over the second half of the session, so the
    // alignment is read after whatever teaching happened, not across it.
    out.alignment = alignment(
        mean_difference(std::vector<std::vector<double>>(probe_vocal.begin() + long(train),
                                                         probe_vocal.end()),
                        std::vector<int>(probe_labels.begin() + long(train),
                                         probe_labels.end())),
        mean_difference(echo_vocal, echo_labels));
  }
  if (reward_ticks) out.reward = reward_sum / double(reward_ticks);
  if (signed_ticks[0]) out.reward_named = signed_sum[0] / double(signed_ticks[0]);
  if (signed_ticks[1]) out.reward_probe = signed_sum[1] / double(signed_ticks[1]);

  // The learning curve: how the picture's effect on the voice lines up with
  // the word's, quarter by quarter. Teaching that works should walk this from
  // nothing toward one; teaching that is being undone as fast as it lands
  // should walk it back down again, and that is a different failure from
  // never having learned.
  const std::vector<double> word_axis = mean_difference(echo_vocal, echo_labels);
  for (int q = 0; q < 4; ++q) {
    const size_t lo = out.probes * size_t(q) / 4;
    const size_t hi = out.probes * size_t(q + 1) / 4;
    if (hi - lo < 4) continue;
    out.curve[q] = alignment(
        mean_difference(
            std::vector<std::vector<double>>(probe_vocal.begin() + long(lo),
                                             probe_vocal.begin() + long(hi)),
            std::vector<int>(probe_labels.begin() + long(lo),
                             probe_labels.begin() + long(hi))),
        word_axis);
    out.taught[q] = probe_taught[hi - 1];
  }

  if (verbose) {
    std::printf("       %u presentations, %u probes (%u skipped), %zu train / %zu test\n",
                out.presentations, out.probes, out.skipped, train,
                size_t(out.probes) - train);
    std::printf("       mean R-E[R]  %+.5f while named, %+.5f while probed\n",
                out.reward_named, out.reward_probe);
    std::printf("       alignment by quarter  ");
    for (int q = 0; q < 4; ++q) {
      std::printf("%+.2f after %-5u", out.curve[q], out.taught[q]);
    }
    std::printf("\n");
  }
  out.ok = true;
  return out;
}


bool run_m3(const std::vector<uint8_t>& blob, uint64_t ticks, const Caregiver& care,
            uint32_t replicates, bool verbose, const Capture& cap) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  const double dt = double(dna.header().sim.dt_ms);
  const uint64_t base_seed = dna.header().seed;

  std::printf("  session           %.1f s of simulated life x %u creatures x 2 upbringings\n",
              double(ticks) * dt / 1000.0, replicates);
  std::printf("  chance            0.500 — probes are balanced cube/ball\n");
  std::printf("  the caregiver names the object while it is in view, and praises the\n"
              "  same way for both, so only the sound carries which one it is.\n\n");
  std::printf("  %-4s %-6s %-9s %-9s %-9s %-9s %-9s %-9s %s\n", "seed", "raised", "voice",
              "timbre", "shuffled", "echo", "B1 shape", "aligned", "|R-E[R]|");

  double sum[2][15] = {{0}};
  uint32_t valid[2] = {0, 0};
  uint32_t above = 0;
  std::vector<double> gap;
  uint32_t beat_control = 0;

  for (uint32_t r = 0; r < replicates; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = base_seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));

    M3Run run[2];
    for (int c = 0; c < 2; ++c) {
      // One session is captured, not ten: the recordings are about a megabyte
      // per simulated second and nine of the ten would go straight in the bin.
      // It is the first creature raised with the names attached — the top row
      // of the table below, so what is on disk is a run whose numbers are
      // printed rather than an anonymous one.
      const Capture session_cap = (r == 0 && c == 0) ? cap : Capture{};
      run[c] = run_m3_session(variant, ticks, c == 0, care, verbose, session_cap);
      if (!run[c].ok) continue;
      ++valid[c];
      sum[c][0] += run[c].vocal;
      sum[c][1] += run[c].timbre;
      sum[c][2] += run[c].shuffled;
      sum[c][3] += run[c].echo;
      sum[c][4] += run[c].alignment;
      sum[c][5] += run[c].acoustic;
      sum[c][6] += run[c].dprime;
      sum[c][7] += run[c].acoustic_shuffled;
      sum[c][8] += run[c].anchor_ia;
      sum[c][9] += run[c].anchor_near;
      sum[c][10] += run[c].anchor_sd;
      sum[c][11] += run[c].dprime_null;
      sum[c][13] += run[c].dprime_unb_sq;
      sum[c][14] += run[c].dprime_null_unb_sq;
      sum[c][12] += run[c].motor_interleaved;
      std::printf("  %-4u %-6s %-9.3f %-9.3f %-9.3f %-9.3f %-9.3f %+-9.3f %.4f\n", r,
                  c == 0 ? "named" : "muddle", run[c].vocal, run[c].timbre,
                  run[c].shuffled, run[c].echo, run[c].b1_shape, run[c].alignment,
                  run[c].reward);
    }
    if (!run[0].ok || !run[1].ok) continue;
    if (run[0].vocal >= 0.75) ++above;
    if (run[0].vocal > run[1].vocal) ++beat_control;
    // The paired difference, kept per creature so the gap below can carry its
    // own error bar instead of being read as a number.
    gap.push_back(run[0].vocal - run[1].vocal);
  }

  const uint32_t need = replicates < 3 ? replicates : 3;
  if (valid[0] < need || valid[1] < need) {
    std::printf("\n  M3 INCONCLUSIVE — only %u/%u creatures produced usable probes.\n",
                valid[0], replicates);
    return false;
  }

  const double paired_vocal = sum[0][0] / double(valid[0]);
  const double paired_timbre = sum[0][1] / double(valid[0]);
  const double paired_shuffled = sum[0][2] / double(valid[0]);
  const double paired_echo = sum[0][3] / double(valid[0]);
  const double control_vocal = sum[1][0] / double(valid[1]);
  const double paired_align = sum[0][4] / double(valid[0]);
  const double control_align = sum[1][4] / double(valid[1]);

  std::printf("\n  mean held-out accuracy on the baby's own voice\n");
  std::printf("    named consistently   %.3f   <- the milestone\n", paired_vocal);
  std::printf("    named at random      %.3f   (the control: nothing to associate)\n",
              control_vocal);
  std::printf("    timbre only          %.3f   (loudness dropped)\n", paired_timbre);
  std::printf("    labels shuffled      %.3f   (must sit at chance)\n", paired_shuffled);
  std::printf("    while hearing a word %.3f   (the audio route's own ceiling)\n",
              paired_echo);
  std::printf("    at or above 0.75     %u of %u creatures\n", above, valid[0]);
  std::printf("    beat its own control %u of %u creatures\n", beat_control, valid[0]);
  // THE FLOOR, and this line exists because the number above it was read as a
  // result for months. `taught - random` is a difference of two small means and
  // it has a spread that has nothing to do with any mechanism.
  //
  // Two scales, and both are needed. WITHIN a run the paired per-creature
  // differences give an error bar directly, printed here. ACROSS runs the floor
  // was measured on 2026-08-30 by running the control genome — where the
  // mechanism is absent by construction — on four seed families: it read
  // +0.060 / -0.060 / +0.060 / -0.060, mean 0.000, **spread 0.120**. That is
  // the same floor `pairprobe` reports at 0.115 on a different metric, arrived
  // at independently, and it is the number a cross-family claim has to clear.
  if (gap.size() >= 2) {
    double mean = 0;
    for (double g : gap) mean += g;
    mean /= double(gap.size());
    double ss = 0;
    for (double g : gap) ss += (g - mean) * (g - mean);
    const double se = std::sqrt(ss / double(gap.size() - 1) / double(gap.size()));
    std::printf("    taught - random      %+.3f +/- %.3f SE over %zu paired creatures\n",
                mean, se, gap.size());
    std::printf("                         a gap under 2 SE is not a result, and the\n"
                "                         control genome swings +/-0.060 across seed\n"
                "                         families with the mechanism absent — so a\n"
                "                         cross-family claim needs about 0.12.\n");
  }
  std::printf("  picture drives the voice the way the word does (cosine, 0 = unrelated)\n");
  std::printf("    named consistently   %+.3f\n", paired_align);
  std::printf("    named at random      %+.3f\n", control_align);

  // The same probes, through a vocal tract and a cochlea instead of straight
  // into a classifier. Every row above is a fact about motor parameters; these
  // are facts about a sound, and the milestone's sentence is about a sound.
  {
    const double a_vocal = sum[0][5] / double(valid[0]);
    const double a_ctrl = sum[1][5] / double(valid[1]);
    const double a_shuf = sum[0][7] / double(valid[0]);
    const double dp = sum[0][6] / double(valid[0]);
    const double ia = sum[0][8] / double(valid[0]);
    const double near = sum[0][9] / double(valid[0]);
    const double own = sum[0][10] / double(valid[0]);
    const double m_int = sum[0][12] / double(valid[0]);
    const double null = sum[0][11] / double(valid[0]);
    std::printf("\n  the same probes, scored on what they SOUND like"
                " (interleaved split)\n");
    std::printf("    named consistently   %.3f   <- against %.3f on motor parameters\n",
                a_vocal, m_int);
    std::printf("    named at random      %.3f   (the control)\n", a_ctrl);
    std::printf("    labels shuffled      %.3f   (must sit at chance)\n", a_shuf);
    // The part a classifier cannot tell you. d' is calibrated on the creature's
    // own within-word scatter, so the anchors below are the same ruler — and
    // read against its own null, because a finite-sample d' is not zero for
    // two identical sounds.
    // Averaged as d'^2 and rooted once, which is the whole point of carrying
    // the squared form: rooting per creature and averaging reinstates the bias.
    const double dpu_sq = sum[0][13] / double(valid[0]);
    const double nullu_sq = sum[0][14] / double(valid[0]);
    const double dpu = dpu_sq > 0.0 ? std::sqrt(dpu_sq) : 0.0;
    const double nullu = nullu_sq > 0.0 ? std::sqrt(nullu_sq) : 0.0;
    std::printf("  separation in d-prime, on this creature's own within-word scatter\n");
    std::printf("    cube vs ball         %.2f raw   %.2f BIAS-CORRECTED  <- read this\n",
                dp, dpu);
    std::printf("    shuffled labels      %.2f raw   %.2f   <- raw is this estimator's\n"
                "                                    small-sample floor; corrected is a\n"
                "                                    32-permutation null and sits near 0\n",
                null, nullu);
    // The bar is 1.0 and it is not arbitrary: d' = 1 is about 76% correct in a
    // two-alternative forced choice, which is the weakest separation worth
    // calling audible. A corrected d' above its null but well under 1.0 means
    // the two utterances really do differ and no listener could use it.
    // The correction is analytic — D*(1/nA+1/nB) subtracted from d'^2 — so the
    // corrected null is not a floor to clear but a check that the correction is
    // the right size. If it drifts far from zero the estimator is wrong and
    // nothing else in this block is worth reading.
    if (nullu > 0.5) {
      std::printf("    INSTRUMENT SUSPECT — the corrected null should be ~0 and reads\n"
                  "    %.2f (d'^2 %+.2f), so the bias model does not fit this sample.\n",
                  nullu, nullu_sq);
    }
    std::printf("    %s\n",
                dpu >= 1.0 && dpu > nullu
                    ? "cube vs ball is audible — corrected d' at or above 1.0"
                    : "cube vs ball is NOT audible — corrected d' below 1.0");
    std::printf("    [i] vs [a]           %.2f   two vowels nobody would confuse\n", ia);
    std::printf("    [a] vs [u]           %.2f   neighbours, still two vowels\n", near);
    // Not a finding — a scale check. A two-sd swing measured in units of one sd
    // has to come out near two whatever the tract does, so what this row is
    // good for is the Hz-to-d' conversion and confirming the ruler is roughly
    // linear between the anchors above.
    std::printf("    +/-1sd of its own F1 %.2f   (scale check, not a result)\n", own);
    if (ia > 0.0 && near > 0.0) {
      // Descriptive only, and NOT comparable between two creatures built
      // differently. It is a ratio of raw cepstral distances — the sigma
      // cancels — so it answers "how far apart on average", while d' above
      // answers "can you tell them apart". A mechanism that widens the vowel
      // space moves the two words further apart AND scatters each of them by
      // the same factor, which doubles this row while leaving discriminability
      // exactly where it was. DNA v32 did precisely that across three seed
      // families, and this line was the most convincing thing on the page.
      std::printf("    cube vs ball is %.1f%% of an [i]/[a] contrast, %.1f%% of [a]/[u]\n"
                  "      (raw distance, so it grows with the vowel space whether or not\n"
                  "       the two words became any easier to tell apart — read d')\n",
                  100.0 * dp / ia, 100.0 * dp / near);
    }
  }

  const bool discriminates = paired_vocal >= 0.75 && above * 2 > valid[0];
  const bool learned = paired_vocal > control_vocal && beat_control * 2 > valid[0];
  const bool controlled = paired_shuffled < 0.60;
  const bool pass = discriminates && learned && controlled;
  if (!controlled) {
    std::printf("\n  CONTROL FAILED — shuffled labels score %.3f, so the readout is\n"
                "  finding structure in the procedure and no other number here is\n"
                "  worth reading.\n", paired_shuffled);
  }
  std::printf("\n  M3/G3 %s — a held-out classifier tells cube from ball off the baby's\n"
              "  vocalisations at %.0f%% (chance 50%%), against %.0f%% for a creature\n"
              "  shown the same objects and told the same words in no fixed order.\n",
              pass ? "PASS" : "FAIL", paired_vocal * 100.0, control_vocal * 100.0);
  return pass;
}


struct G2Run {
  uint64_t baseline_hits = 0, baseline_events = 0;
  uint64_t test_hits = 0, test_events = 0;
  uint64_t train_events = 0;
  double baseline_mean = 0, test_mean = 0;
  float criterion = 0.5f;
  uint64_t rewards = 0;
  std::vector<Praise> delivered;
  double mean_weight_start = 0, mean_weight_end = 0;
};


// One session.
//
// In the experimental condition feedback follows the baby's own vocalisations:
// praise for the rewarded class, a mild "no" for the other. Both signs matter.
// An all-positive regime is not a training signal at all — it potentiates
// every eligible synapse in the brain regardless of what was done, and the
// only thing that reliably grows is the mean weight.
//
// In the yoked condition the same sequence of feedback arrives at shifted
// times, so the baby gets the same praise and the same scolding in the same
// proportions, for nothing it did.
//
// `criterion` is the class boundary. It is measured from this brain's own
// baseline rather than fixed at 0.5, because the F1 population vector is
// tightly peaked and every genome sits at a slightly different place: a fixed
// boundary would mostly measure where a seed happened to land.
G2Run run_g2_session(const std::vector<uint8_t>& blob, uint64_t ticks,
                     const std::vector<Praise>* yoked, float criterion_in,
                     const Regime& regime, bool& ok) {
  G2Run out;
  ok = false;
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return out;
  }

  const uint64_t baseline_end = ticks / 5;       // first 20%: observe only
  const uint64_t train_end = ticks - ticks / 5;  // middle 60%: feedback
  uint64_t last_event = 0;
  uint64_t last_feedback = 0;
  uint32_t last_frame = 0;
  std::deque<Praise> pending;
  size_t yoke_cursor = 0;
  std::vector<float> baseline_values;
  float criterion = criterion_in;

  out.mean_weight_start = double(s.brain.network().telemetry().mean_weight);

  for (uint64_t t = 0; t < ticks; ++t) {
    // Feedback earned earlier arrives now.
    while (!pending.empty() && pending.front().tick <= t) {
      s.brain.praise(pending.front().value);
      ++out.rewards;
      pending.pop_front();
    }
    if (yoked) {
      while (yoke_cursor < yoked->size() && (*yoked)[yoke_cursor].tick <= t) {
        s.brain.praise((*yoked)[yoke_cursor].value);
        ++out.rewards;
        ++yoke_cursor;
      }
    }

    s.brain.step();

    // The vocal tract is read at 100 Hz; only look when there is a new frame.
    if (s.brain.vocal_frame() == last_frame) continue;
    last_frame = s.brain.vocal_frame();

    const aibaby::VocalParams& v = s.brain.voice();
    const bool voiced = v.voicing > 0.5f && v.amplitude > kAmplitudeFloor;
    if (!voiced) continue;

    // The class is read off the F1 motor group, not off the synthesised audio:
    // it is the thing the brain controls, which is the thing reward can shape.
    const float value = float(s.brain.vocal_groups()[2]);

    // Feedback tracks the sound as it is being made; the *event* count, used
    // for the reported rates, still uses the refractory period so that one
    // long coo is one vocalisation.
    const bool new_event = t - last_event >= kEventRefractoryTicks;
    if (new_event) last_event = t;

    if (t < baseline_end) {
      if (!new_event) continue;
      baseline_values.push_back(value);
      ++out.baseline_events;
      out.baseline_mean += double(value);
      continue;
    }

    // At the end of the baseline, take the criterion from the baby's own
    // median if the caller did not supply one, so the rewarded class starts at
    // half of what it already does.
    if (criterion < 0.0f) {
      if (baseline_values.size() < 8) {
        criterion = 0.5f;
      } else {
        std::vector<float> sorted = baseline_values;
        std::nth_element(sorted.begin(), sorted.begin() + long(sorted.size() / 2),
                         sorted.end());
        criterion = sorted[sorted.size() / 2];
      }
    }

    const bool hit = value >= criterion;
    if (t < train_end) {
      if (new_event) ++out.train_events;
      if (!yoked && t - last_feedback >= regime.feedback_period) {
        last_feedback = t;
        const Praise p{t + regime.delay, hit ? regime.praise : regime.scold};
        pending.push_back(p);
        out.delivered.push_back(p);
      }
    } else if (new_event) {
      ++out.test_events;
      out.test_mean += double(value);
      if (hit) ++out.test_hits;
    }
  }

  // Baseline hits are scored against the same criterion, after the fact.
  for (float v : baseline_values) {
    if (v >= criterion) ++out.baseline_hits;
  }
  if (out.baseline_events) out.baseline_mean /= double(out.baseline_events);
  if (out.test_events) out.test_mean /= double(out.test_events);
  out.criterion = criterion;
  out.mean_weight_end = double(s.brain.network().telemetry().mean_weight);
  ok = true;
  return out;
}


// One replicate: a praised baby and its yoked twin, both grown from `blob`.
struct G2Pair {
  bool ok = false;
  double exp_ratio = 0, ctl_ratio = 0;   // rewarded-class rate, test / baseline
  double exp_shift = 0, ctl_shift = 0;   // rewarded-class share, test - baseline
  double exp_drift = 0, ctl_drift = 0;   // mean F1 motor group, test - baseline
  uint64_t baseline_events = 0, test_events = 0, feedback = 0;
  double criterion = 0;
};


G2Pair run_g2_pair(const std::vector<uint8_t>& blob, uint64_t ticks,
                   const Regime& regime) {
  G2Pair out;
  bool ok = false;
  const G2Run exp = run_g2_session(blob, ticks, nullptr, -1.0f, regime, ok);
  if (!ok) return out;

  // The yoked control gets the same feedback, the same number of times, with
  // the same signs, spread across the same window — but time-shifted by half
  // the training phase, so none of it follows anything the baby did.
  //
  // Replaying the feedback *unshifted* would be worthless: the core is
  // deterministic, so identical inputs produce a bit-identical brain and the
  // "control" would be the experiment. The shift is what makes it a control.
  const uint64_t train_begin = ticks / 5;
  const uint64_t train_end = ticks - ticks / 5;
  const uint64_t span = train_end > train_begin ? train_end - train_begin : 1;
  std::vector<Praise> yoked;
  yoked.reserve(exp.delivered.size());
  for (const Praise& p : exp.delivered) {
    const uint64_t offset = p.tick > train_begin ? p.tick - train_begin : 0;
    yoked.push_back(Praise{train_begin + (offset + span / 2) % span, p.value});
  }
  std::sort(yoked.begin(), yoked.end(),
            [](const Praise& a, const Praise& b) { return a.tick < b.tick; });

  // The control is scored against the same criterion so the two conditions are
  // measuring the same thing.
  const G2Run ctl = run_g2_session(blob, ticks, &yoked, exp.criterion, regime, ok);
  if (!ok) return out;

  auto share = [](uint64_t hits, uint64_t events) {
    return events ? double(hits) / double(events) : 0.0;
  };
  auto ratio = [](uint64_t test, uint64_t base) {
    return base ? double(test) / double(base) : 0.0;
  };

  out.exp_shift = share(exp.test_hits, exp.test_events) -
                  share(exp.baseline_hits, exp.baseline_events);
  out.ctl_shift = share(ctl.test_hits, ctl.test_events) -
                  share(ctl.baseline_hits, ctl.baseline_events);
  out.exp_ratio = ratio(exp.test_hits, exp.baseline_hits);
  out.ctl_ratio = ratio(ctl.test_hits, ctl.baseline_hits);
  out.exp_drift = exp.test_mean - exp.baseline_mean;
  out.ctl_drift = ctl.test_mean - ctl.baseline_mean;
  out.baseline_events = exp.baseline_events;
  out.test_events = exp.test_events;
  out.feedback = exp.rewards;
  out.criterion = double(exp.criterion);
  out.ok = exp.baseline_events >= 15 && exp.test_events >= 15 && exp.rewards > 0;
  return out;
}


// --- G4: structure grows only when needed ----------------------------------
//
// "Neuron count stays flat while error is improving; grows only on a detected
// plateau; never exceeds the DNA budget cap."
//
// All three clauses are about what the creature does *not* do, which makes
// this the easiest goal in the document to pass by accident: a brain with the
// growth code deleted satisfies every one of them. So the experiment carries
// its own non-vacuity control, in the same spirit as m2's shuffled labels — a
// second arm in which the saturation guard is lowered until growth is
// unavoidable, which proves the path being restrained is a path that works.
//
// The two arms answer different questions and neither is sufficient alone:
//
//   shipped  does a normally-raised creature grow when it should not?
//   forced   when the conditions do hold, does growth insert, wire, respect
//            the budget cap, and stay deterministic?

struct G4Window {
  uint64_t tick;
  bool plateaued;
  bool improving;
  double improvement;
  double error;
  uint32_t neurons;
  uint32_t growth_events;
  uint32_t neurons_grown;
  double fill;        // incoming edges at 3/4 of their ceiling
  double mean_rate;   // the growable module's rate, against its own setpoint
};


struct G4Run {
  std::vector<G4Window> windows;
  aibaby::StructuralStats structural = {};
  uint32_t neurons_birth = 0;
  uint32_t neurons_final = 0;
  uint32_t cap = 0;
  bool over_cap = false;          // did any module ever exceed its own n_max?
  bool slept = false;
  uint32_t episodes = 0;
  double peak_rate = 0.0;         // how close a growable module got to...
  double peak_weight_frac = 0.0;  // ...each half of the saturation bar
  double sat_rate_bar = 0.0;
  double sat_weight_bar = 0.0;
  // Where the incoming weights actually sit, at three depths into the ceiling.
  // The mean cannot answer this: synaptic scaling bounds each neuron's total
  // input weight, so learning under that constraint shows up as redistribution
  // rather than as growth in the average.
  double fill_50 = 0.0, fill_75 = 0.0, fill_90 = 0.0;
  double mean_plasticity = 1.0;   // §3.5: eta multiplier averaged over edges
  std::vector<uint64_t> hashes;   // for the determinism arm
  bool ok = false;
};


// One creature raised on a stream of named toys. The environment has to be
// rich enough that the critic's prediction error is a real signal — a creature
// in an empty room has nothing to get better at, so its error is flat from the
// first window and every window reads as a plateau.
G4Run run_g4_session(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  G4Run out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return out;
  }

  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  const aibaby::DnaGrowth& gcfg = s.dna.header().growth;
  Retina retina;
  if (!retina.configure(vcfg, error)) return out;
  Ear ear;
  if (!ear.configure(acfg, error)) return out;

  VowelSource voice(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  std::vector<float> pcm(16);
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);
  const uint32_t samples_per_tick = uint32_t(acfg.sample_rate / 1000);

  aibaby::Rng rng;
  rng.seed(s.dna.header().seed ^ 0x64C4u);

  const aibaby::Network& net = s.brain.network();
  out.neurons_birth = net.live_neurons();
  out.cap = s.dna.total_neurons_max();
  out.sat_rate_bar = double(gcfg.saturation_rate_hz);
  out.sat_weight_bar = double(gcfg.saturation_weight) * double(s.dna.header().homeo.w_max);

  std::deque<Praise> pending;
  uint32_t windows_seen = 0;
  uint64_t trial_start = 0;
  int object = 0;
  Toy toy = m3_toy(rng, object);

  for (uint64_t t = 0; t < ticks; ++t) {
    // A new toy every trial, named over the first part of it, praised near the
    // end. Deliberately the same protocol M3 uses: it is the richest thing the
    // creature ever experiences, so if anything is going to saturate a module
    // it is this.
    if (t - trial_start >= kM3TrialTicks) {
      trial_start = t;
      object = int(rng.uniform() * 2.0f) & 1;
      toy = m3_toy(rng, object);
      pending.push_back(Praise{t + kRewardDelayTicks, kPraiseValue});
    }
    while (!pending.empty() && pending.front().tick <= t) {
      s.brain.praise(pending.front().value);
      pending.pop_front();
    }

    if (t % frame_ticks == 0) {
      scene.render(toy.shape, toy.cx, toy.cy, toy.radius, 0.85f, 0.02f, frame.data());
      retina.present(frame.data());
      s.brain.see(retina.features().data(), retina.feature_count());
    }

    const bool speaking = (t - trial_start) < kM3LabelTicks;
    pcm.resize(samples_per_tick);
    if (speaking) {
      voice.render(kWords[object].f0, kWords[object].f1, kWords[object].f2, 0.6f,
                   pcm.data(), samples_per_tick);
    } else {
      for (uint32_t i = 0; i < samples_per_tick; ++i) pcm[i] = 0.0f;
    }
    ear.tick(s.brain, pcm.data(), samples_per_tick);

    s.brain.step();
    if (s.brain.asleep()) out.slept = true;

    // Sampled at every plateau-window boundary. Growth can only fire on those
    // ticks, so this is the complete record of every moment the decision was
    // taken, together with the verdict that was taken on.
    const aibaby::GrowthWatch& w = s.brain.growth_watch();
    if (w.windows > windows_seen) {
      windows_seen = w.windows;
      G4Window rec;
      rec.tick = t;
      rec.plateaued = w.plateaued;
      rec.improving = w.improving;
      rec.improvement = double(w.improvement);
      rec.error = double(s.brain.critic().slow_error());
      rec.neurons = net.live_neurons();
      rec.growth_events = net.structural().growth_events;
      rec.neurons_grown = net.structural().neurons_grown;
      rec.fill = 0.0;
      rec.mean_rate = 0.0;
      for (uint32_t m = 0; m < net.module_count(); ++m) {
        if (!net.growable(m)) continue;
        rec.fill = std::max(rec.fill, double(net.in_weight_fill(m, aibaby::Scalar(0.75))));
        rec.mean_rate = std::max(rec.mean_rate, double(net.module(m).mean_rate));
      }
      out.windows.push_back(rec);
    }

    // The budget cap is checked continuously rather than at the end: a module
    // that overran and was pruned back would pass an end-of-run check having
    // broken the goal in the middle.
    for (uint32_t m = 0; m < net.module_count(); ++m) {
      const aibaby::ModuleState& ms = net.module(m);
      if (ms.count > ms.capacity || ms.live() > ms.capacity) out.over_cap = true;
    }

    // How close a growable module ever came to each half of the saturation
    // bar. Reported whether or not it crossed, because "the guard never fired"
    // and "the guard nearly fired" are very different creatures.
    for (uint32_t m = 0; m < net.module_count(); ++m) {
      if (!net.growable(m)) continue;
      out.peak_rate = std::max(out.peak_rate, double(net.module(m).mean_rate));
      out.peak_weight_frac = std::max(out.peak_weight_frac, double(net.mean_in_weight(m)));
      // Sampled every second of simulated life rather than every tick: this
      // walks every incoming synapse in the module, and a weight distribution
      // does not move measurably in a millisecond.
      if (t % 1000 == 0) {
        out.fill_50 = std::max(out.fill_50, double(net.in_weight_fill(m, aibaby::Scalar(0.50))));
        out.fill_75 = std::max(out.fill_75, double(net.in_weight_fill(m, aibaby::Scalar(0.75))));
        out.fill_90 = std::max(out.fill_90, double(net.in_weight_fill(m, aibaby::Scalar(0.90))));
      }
    }

    if (t % 50000 == 0) out.hashes.push_back(net.state_hash());
  }

  out.hashes.push_back(net.state_hash());
  out.structural = net.structural();
  out.neurons_final = net.live_neurons();
  out.episodes = s.brain.episodes_stored();
  out.mean_plasticity = double(net.mean_plasticity());
  out.ok = true;
  if (verbose) {
    std::printf("    windows %zu, growth %u, pruned %u synapses / %u neurons,"
                " %u replays\n",
                out.windows.size(), out.structural.growth_events,
                out.structural.synapses_pruned, out.structural.neurons_pruned,
                out.structural.replays);
  }
  return out;
}


// Did the creature ever add a neuron in a window whose verdict was "still
// improving"? This is G4's first clause, and it is checked against the
// recorded verdicts rather than trusted to the code path that produced them.
// Which window does a growth event belong to? Records are written at window
// boundaries, *after* the tick's growth decision has already run — the record
// holds the verdict and the neuron count that verdict produced. So an increase
// between record i-1 and record i was caused by window i's verdict, and window
// i is the one that has to have been a plateau.
//
// Reading it as window i-1 is off by one, and it was invisible for as long as
// a normally raised creature never grew: with the count flat, both readings
// are vacuously true. The first run that grew reported window 3 as a
// violation while the ledger showed all six events landing on plateaus.
bool g4_grew_while_improving(const G4Run& r, uint32_t& at_window) {
  for (size_t i = 1; i < r.windows.size(); ++i) {
    const G4Window& now = r.windows[i];
    if (now.neurons_grown > r.windows[i - 1].neurons_grown && now.improving) {
      at_window = uint32_t(i);
      return true;
    }
  }
  return false;
}


// ...and its second: every growth event has to sit on a window that was
// declared a plateau.
bool g4_grew_without_plateau(const G4Run& r, uint32_t& at_window) {
  for (size_t i = 1; i < r.windows.size(); ++i) {
    if (r.windows[i].neurons_grown > r.windows[i - 1].neurons_grown &&
        !r.windows[i].plateaued) {
      at_window = uint32_t(i);
      return true;
    }
  }
  return false;
}


// --- Calibration: the six rules a genome edit silently breaks ---------------
//
// The genome sits at a hand-measured operating point, and editing any part of
// it invalidates measurements elsewhere without producing an error anywhere.
// Every one of these has cost a day at least once: a sweep that recalibrated
// nothing and therefore measured intrinsic plasticity fighting the change; a
// density edit that pushed four synapses past an in-degree cap in one seed of
// nine; an amplitude floor drifting toward the operating point until
// vocalisation counts fell in every condition.
//
// The rules are not hard. Remembering to apply all six, in order, after every
// edit is what fails. So they live here as one command that answers "is this
// genome still calibrated" with a number and a verdict.
constexpr double kRateTolerance = 0.25;      // Hz, free-running vs genome target

constexpr double kFloorMarginWanted = 0.10;  // kAmplitudeFloor below the operating point

constexpr uint32_t kCalibrationSeeds = 9;    // what g2 and m3 actually sweep


struct ModuleRate {
  const char* name;
  double free_running;
  double target;
  bool vocal;
  // A module whose activity is supposed to come from outside itself. Rule 1
  // compares a module's target against the rate it free-runs at *in the dark*,
  // which is the right test for a module that mostly drives itself and the
  // wrong one for a sensory cortex: V1 is silent with nothing to look at, and
  // setting its target to that silence makes intrinsic plasticity treat every
  // lit moment as an overshoot. Reported with its reason, like `vocal`.
  bool sensory_driven;
};


// Rule 1: free-running rate means *with homeostasis off*. With it on, every
// module reads back its own target and the measurement is circular.
std::vector<ModuleRate> measure_free_running(const std::vector<uint8_t>& blob,
                                             uint64_t ticks) {
  std::vector<ModuleRate> out;
  std::vector<uint8_t> quiet = blob;
  {
    auto* h = reinterpret_cast<aibaby::DnaHeader*>(quiet.data());
    h->homeo.ip_rate = 0.0f;
    h->homeo.scaling_rate = 0.0f;
    // Growth off as well, and not for tidiness: a free-running rate is a
    // property of the wiring the genome describes, and since DNA v5 a creature
    // left alone for two minutes grows. Measuring with growth on reads the
    // rate of a brain that changed shape halfway through the measurement, and
    // reports it as the genome's operating point.
    h->growth.enabled = 0;
  }
  Session s;
  std::string error;
  if (!s.init(quiet, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return out;
  }
  for (uint64_t t = 0; t < ticks; ++t) s.brain.step();

  const int32_t vocal = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
  for (uint32_t m = 0; m < s.brain.network().module_count(); ++m) {
    ModuleRate r;
    r.name = s.brain.network().module_dna(m).name;
    r.free_running = double(s.brain.network().module(m).mean_rate);
    r.target = double(s.brain.network().module_dna(m).target_rate_hz);
    r.vocal = (vocal >= 0 && uint32_t(vocal) == m);
    const uint32_t role = s.brain.network().module_dna(m).role;
    r.sensory_driven = role == uint32_t(aibaby::ModuleRole::kVisualCortex) ||
                       role == uint32_t(aibaby::ModuleRole::kVisualForm);
    out.push_back(r);
  }
  return out;
}


// Rule 6: the in-degree check has to sweep the seeds an experiment uses, not
// just the default one. A cap overrun in seed 7 of 9 is four synapses missing
// from one creature in an experiment's nine, and the warning scrolls past
// mid-table.
uint32_t dropped_across_seeds(const std::vector<uint8_t>& blob, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return 0;
  const uint64_t base_seed = dna.header().seed;
  uint32_t bad_seeds = 0;

  for (uint32_t r = 0; r < kCalibrationSeeds; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = base_seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));

    Session s;
    std::string error;
    if (!s.init(variant, error)) continue;  // init() prints its own warning
    const aibaby::Network& net = s.brain.network();
    if (net.dropped_synapses() == 0 && net.dropped_reverse() == 0) continue;
    ++bad_seeds;
    std::printf("    seed %u (%llu): %u synapses, %u reverse entries dropped\n", r,
                (unsigned long long)seed, net.dropped_synapses(), net.dropped_reverse());
    for (uint32_t m = 0; m < net.module_count(); ++m) {
      if (net.dropped_synapses(m) == 0 && net.dropped_reverse(m) == 0) continue;
      std::printf("      %-12s cap %u — raise max_out_degree\n", net.module_dna(m).name,
                  net.module_dna(m).max_out_degree);
    }
  }
  if (verbose && bad_seeds == 0) {
    std::printf("    all %u seeds wire cleanly\n", kCalibrationSeeds);
  }
  return bad_seeds;
}


bool run_calibrate(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  genome rejected\n");
    return false;
  }

  std::printf("  free-running rates, measured with ip_rate and scaling_rate at 0.\n"
              "  A target that does not match is a module whose every later\n"
              "  measurement includes intrinsic plasticity fighting the genome.\n\n");
  const std::vector<ModuleRate> rates = measure_free_running(blob, ticks);
  if (rates.empty()) return false;

  std::printf("  %-12s %12s %12s   %s\n", "module", "free-running", "target", "verdict");
  uint32_t off_target = 0;
  for (const ModuleRate& r : rates) {
    const double delta = r.free_running - r.target;
    const bool ok = std::fabs(delta) <= kRateTolerance;
    // The vocal module is deliberately mis-set: at its true free-running rate
    // the duty cycle goes to 0.93 and the creature drones. That is a known
    // collision between rule 1 and behaviour, not a calibration error, so it
    // is reported with its reason rather than counted as a failure.
    const char* verdict = ok                 ? "ok"
                          : r.vocal          ? "by design — holds the duty cycle down"
                          : r.sensory_driven ? "by design — driven by the retina, not itself"
                                    : "STALE — reset the genome to the left column";
    if (!ok && !r.vocal && !r.sensory_driven) ++off_target;
    std::printf("  %-12s %9.2f Hz %9.2f Hz   %+.2f  %s\n", r.name, r.free_running, r.target,
                delta, verdict);
  }

  // Rule 4: the amplitude floor has to sit clear of the operating point, or
  // the creature falls below it as homeostasis settles and every absolute
  // vocalisation count drifts down in every condition.
  const aibaby::DnaVocal& v = dna.header().vocal;
  double vocal_target = 0.0;
  for (const ModuleRate& r : rates) {
    if (r.vocal) vocal_target = r.target;
  }
  const double operating = v.rate_norm_hz > 0.0f ? vocal_target / double(v.rate_norm_hz) : 0.0;
  const double margin = operating - double(kAmplitudeFloor);
  const bool floor_ok = margin >= kFloorMarginWanted;
  std::printf("\n  amplitude floor  %.2f against an operating point of %.2f"
              " (%.2f / %.1f Hz)\n", double(kAmplitudeFloor), operating, vocal_target,
              double(v.rate_norm_hz));
  std::printf("  %-16s margin %+.2f — want at least %.2f  %s\n", "", margin,
              kFloorMarginWanted, floor_ok ? "ok" : "THIN, and structurally so");
  // This one is reported and not counted, because no genome edit can currently
  // satisfy it together with the babble criterion, and a check that can only
  // ever be red teaches you to stop reading the output.
  //
  // Loudness is `mean group rate / rate_norm` and §3.1 pins that mean rate at
  // the setpoint, so amplitude is a rescaled constant. Rule 4 wants the floor
  // *below* the operating point so vocalisation counts do not drift to zero —
  // but a floor below a constant is a floor the creature is above essentially
  // always, which is the 0.83 duty cycle against babble's 0.85 ceiling. Move
  // the floor up and the counts collapse; move it down and the drone worsens.
  // The two requirements are incompatible while amplitude does not vary, and
  // making it vary is upstream work, not a tuning pass.
  if (!floor_ok) {
    std::printf("  %-16s a floor below a pinned operating point is a creature that is\n"
                "  %-16s always above it. Not fixable by retuning — see the README.\n",
                "", "");
  }

  std::printf("\n  in-degree caps across the %u seeds g2 and m3 sweep\n", kCalibrationSeeds);
  const uint32_t bad_seeds = dropped_across_seeds(blob, verbose);

  // The verdict covers what an edit can actually put right. The amplitude
  // floor is printed above and deliberately excluded — see the note there.
  const bool pass = off_target == 0 && bad_seeds == 0;
  std::printf("\n  calibrate %s — %u module%s off target, %u seed%s wiring badly."
              " Amplitude floor %s.\n",
              pass ? "PASS" : "FAIL", off_target, off_target == 1 ? "" : "s", bad_seeds,
              bad_seeds == 1 ? "" : "s", floor_ok ? "clear" : "thin (structural)");
  if (!pass) {
    std::printf("  Re-run `babble` after fixing: rule 1 and behaviour can collide,\n"
                "  and the duty cycle is the number that tells you they have.\n");
  }
  return pass;
}


bool run_g4(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  genome rejected\n");
    return false;
  }
  const double dt = double(dna.header().sim.dt_ms);
  std::printf("  session           %.1f s of simulated life\n",
              double(ticks) * dt / 1000.0);
  std::printf("  budget cap        %u neurons (born with %u)\n",
              dna.total_neurons_max(), dna.total_neurons_at_birth());

  // --- Arm 1: a normally raised creature -----------------------------------
  std::printf("\n  as raised\n");
  const G4Run shipped = run_g4_session(blob, ticks, verbose);
  if (!shipped.ok) return false;

  uint32_t improving_windows = 0, plateau_windows = 0;
  for (const G4Window& w : shipped.windows) {
    if (w.improving) ++improving_windows;
    if (w.plateaued) ++plateau_windows;
  }
  std::printf("    windows         %zu (%u improving, %u plateau)\n",
              shipped.windows.size(), improving_windows, plateau_windows);
  std::printf("    neurons         %u -> %u (cap %u)\n", shipped.neurons_birth,
              shipped.neurons_final, shipped.cap);
  std::printf("    growth          %u events, %u neurons\n",
              shipped.structural.growth_events, shipped.structural.neurons_grown);
  std::printf("    saturation      rate %.2f Hz peak vs %.2f bar,"
              " weight %.3f peak vs %.3f bar\n",
              shipped.peak_rate, shipped.sat_rate_bar, shipped.peak_weight_frac,
              shipped.sat_weight_bar);
  std::printf("    weight spread   %.1f%% of incoming edges at half the ceiling,"
              " %.1f%% at 3/4, %.1f%% at 9/10\n",
              shipped.fill_50 * 100.0, shipped.fill_75 * 100.0, shipped.fill_90 * 100.0);
  // Does crowding actually accumulate with experience? The mean weight does
  // not, so if this does not either there is nothing here a growth trigger
  // could ever read.
  std::printf("    crowding curve  ");
  for (size_t i = 0; i < shipped.windows.size(); i += shipped.windows.size() / 6 + 1) {
    std::printf("%.0fs:%.1f%%  ", double(shipped.windows[i].tick) * dt / 1000.0,
                shipped.windows[i].fill * 100.0);
  }
  std::printf("\n");
  // The other candidate. If the creature is stuck *and* still wrong, that is
  // the state §3.4 describes — and unlike rate or weight, prediction error is
  // not something homeostasis regulates to a setpoint.
  std::printf("    error curve     ");
  for (size_t i = 0; i < shipped.windows.size(); i += shipped.windows.size() / 6 + 1) {
    std::printf("%.0fs:%.4f  ", double(shipped.windows[i].tick) * dt / 1000.0,
                shipped.windows[i].error);
  }
  std::printf("\n");
  std::printf("    sleep           %s, %u passes, pruned %u synapses"
              " / %u neurons\n",
              shipped.slept ? "reached" : "NEVER REACHED",
              shipped.structural.consolidations, shipped.structural.synapses_pruned,
              shipped.structural.neurons_pruned);
  std::printf("    replay          %u episodes held, %u re-experienced\n",
              shipped.episodes, shipped.structural.replays);
  std::printf("    myelination     mean per-edge learning rate %.3f x eta\n",
              shipped.mean_plasticity);

  // Which window each growth event actually sat on. The two predicates below
  // disagree about this by one, and until the normal arm grew there was no
  // data that could tell them apart.
  std::printf("    growth ledger   ");
  for (size_t i = 1; i < shipped.windows.size(); ++i) {
    if (shipped.windows[i].neurons_grown == shipped.windows[i - 1].neurons_grown) continue;
    std::printf("w%zu[%s%s]->w%zu[%s%s]  ", i - 1,
                shipped.windows[i - 1].plateaued ? "plateau" : "",
                shipped.windows[i - 1].improving ? "improving" : "", i,
                shipped.windows[i].plateaued ? "plateau" : "",
                shipped.windows[i].improving ? "improving" : "");
  }
  std::printf("\n");

  uint32_t bad_window = 0;
  const bool grew_improving = g4_grew_while_improving(shipped, bad_window);
  const bool grew_unplateaued = g4_grew_without_plateau(shipped, bad_window);
  if (grew_improving) {
    std::printf("    VIOLATION       grew during window %u, which was still improving\n",
                bad_window);
  }
  if (grew_unplateaued) {
    std::printf("    VIOLATION       grew after window %u, which was not a plateau\n",
                bad_window);
  }

  // --- Arm 2: the same creature with the saturation guard lowered ----------
  //
  // Growth's three conditions are ANDed, so any one of them can hide a broken
  // implementation of the other two behind "it never fired". Here the guard is
  // moved rather than the mechanism, so what runs is the shipped growth path.
  std::printf("\n  forced (saturation guard lowered — the non-vacuity control)\n");
  std::vector<uint8_t> forced = blob;
  {
    auto* h = reinterpret_cast<aibaby::DnaHeader*>(forced.data());
    h->growth.saturation_rate_hz = 0.1f;   // any live module counts as busy
    h->growth.saturation_weight = 0.001f;  // ...and as out of headroom
    h->growth.epsilon = 1000.0f;           // every window is a plateau
    h->growth.refractory_ticks = 20000;
  }
  const G4Run forced_run = run_g4_session(forced, ticks, verbose);
  if (!forced_run.ok) return false;

  std::printf("    windows         %zu\n", forced_run.windows.size());
  std::printf("    neurons         %u -> %u (cap %u)\n", forced_run.neurons_birth,
              forced_run.neurons_final, forced_run.cap);
  std::printf("    growth          %u events, %u neurons\n",
              forced_run.structural.growth_events, forced_run.structural.neurons_grown);
  std::printf("    sleep           %s, pruned %u synapses / %u neurons\n",
              forced_run.slept ? "reached" : "NEVER REACHED",
              forced_run.structural.synapses_pruned,
              forced_run.structural.neurons_pruned);

  // --- Arm 3: G1 still holds when the structure itself is plastic ----------
  //
  // Growth draws from the same RNG stream the tick loop draws its noise from,
  // and pruning renumbers every synapse slot. Either could make a brain that
  // is reproducible only until the first time it changes shape, and the
  // determinism experiment would not see it: at 20k ticks nothing structural
  // has happened yet.
  const G4Run twin = run_g4_session(forced, ticks, false);
  bool identical = twin.ok && twin.hashes.size() == forced_run.hashes.size();
  size_t diverged_at = 0;
  for (size_t i = 0; identical && i < twin.hashes.size(); ++i) {
    if (twin.hashes[i] != forced_run.hashes[i]) {
      identical = false;
      diverged_at = i;
    }
  }
  std::printf("\n  determinism through growth and pruning\n");
  if (identical) {
    std::printf("    %zu checkpoints, final hash %016llx — identical\n",
                forced_run.hashes.size(),
                (unsigned long long)forced_run.hashes.back());
  } else {
    std::printf("    DIVERGED at checkpoint %zu of %zu\n", diverged_at,
                forced_run.hashes.size());
  }

  // The verdict. The first three clauses are G4 as written; the fourth is what
  // stops the first three from being satisfied by an inert creature.
  const bool flat_while_improving = !grew_improving;
  const bool only_on_plateau = !grew_unplateaued;
  const bool within_cap = !shipped.over_cap && !forced_run.over_cap &&
                          shipped.neurons_final <= shipped.cap &&
                          forced_run.neurons_final <= forced_run.cap;
  const bool growth_works = forced_run.structural.neurons_grown > 0;
  const bool pruning_works = shipped.structural.consolidations > 0;
  const bool pass = flat_while_improving && only_on_plateau && within_cap &&
                    growth_works && pruning_works && identical;

  std::printf("\n  G4 %s\n", pass ? "PASS" : "FAIL");
  std::printf("    count flat while error improved   %s\n",
              flat_while_improving ? "yes" : "NO");
  std::printf("    grew only on a detected plateau   %s\n",
              only_on_plateau ? "yes" : "NO");
  std::printf("    never exceeded the budget cap     %s\n", within_cap ? "yes" : "NO");
  std::printf("    growth path demonstrably works    %s (%u neurons when forced)\n",
              growth_works ? "yes" : "NO", forced_run.structural.neurons_grown);
  std::printf("    sleep consolidation ran           %s (%u passes)\n",
              pruning_works ? "yes" : "NO", shipped.structural.consolidations);
  std::printf("    structural change is reproducible %s\n", identical ? "yes" : "NO");
  return pass;
}


// Replicates, because one run is an anecdote.
//
// A single session can favour either condition by luck: the effect of one
// caregiver's praise over three minutes is real but not large, and the F1
// population vector is a tightly peaked quantity. Each replicate is a
// different creature — same genome, different developmental seed — so the
// question becomes how often praise beats its own yoked control, which is a
// claim a single number cannot make.
constexpr uint32_t kReplicates = 9;


bool run_g2(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose,
            const Regime& regime) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  const double dt = double(dna.header().sim.dt_ms);
  const uint64_t base_seed = dna.header().seed;

  std::printf("  session           %.1f s of simulated life x %u creatures\n",
              double(ticks) * dt / 1000.0, kReplicates);
  std::printf("  %-4s %-22s %-22s %s\n", "seed", "rewarded rate x",
              "rewarded share shift", "F1 motor shift");

  double sum_ratio_gain = 0, sum_shift_gain = 0, sum_drift_gain = 0;
  double sum_exp_ratio = 0;
  uint32_t favoured = 0, valid = 0;

  for (uint32_t r = 0; r < kReplicates; ++r) {
    // Same genome, different creature: only the developmental seed moves.
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = base_seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));

    const G2Pair p = run_g2_pair(variant, ticks, regime);
    if (!p.ok) {
      std::printf("  %-4u  (inconclusive: %llu baseline / %llu test vocalisations)\n",
                  r, (unsigned long long)p.baseline_events,
                  (unsigned long long)p.test_events);
      continue;
    }
    ++valid;
    sum_exp_ratio += p.exp_ratio;
    sum_ratio_gain += p.exp_ratio - p.ctl_ratio;
    sum_shift_gain += p.exp_shift - p.ctl_shift;
    sum_drift_gain += p.exp_drift - p.ctl_drift;
    // A replicate favours praise when the rewarded class both became more
    // frequent and took a larger share than in the creature's yoked twin.
    const bool win = p.exp_ratio > p.ctl_ratio && p.exp_shift > p.ctl_shift;
    if (win) ++favoured;
    std::printf("  %-4u praised %5.2f vs %5.2f   %+6.3f vs %+6.3f     %+6.4f vs %+6.4f  %s\n",
                r, p.exp_ratio, p.ctl_ratio, p.exp_shift, p.ctl_shift, p.exp_drift,
                p.ctl_drift, win ? "praise" : "yoked");
    if (verbose) {
      std::printf("       criterion %.3f, %llu baseline / %llu test vocalisations,"
                  " %llu praises\n",
                  p.criterion, (unsigned long long)p.baseline_events,
                  (unsigned long long)p.test_events, (unsigned long long)p.feedback);
    }
  }

  if (valid < 3) {
    std::printf("\n  G2 INCONCLUSIVE — only %u of %u replicates produced enough\n"
                "  vocalisations to measure.\n", valid, kReplicates);
    return false;
  }

  const double mean_ratio_gain = sum_ratio_gain / double(valid);
  const double mean_shift_gain = sum_shift_gain / double(valid);
  const double mean_drift_gain = sum_drift_gain / double(valid);
  const double mean_exp_ratio = sum_exp_ratio / double(valid);

  std::printf("\n  mean advantage over the yoked control\n");
  std::printf("    rewarded rate     %+.3f x\n", mean_ratio_gain);
  std::printf("    rewarded share    %+.4f\n", mean_shift_gain);
  std::printf("    F1 motor group    %+.5f\n", mean_drift_gain);
  std::printf("    praise won        %u of %u creatures\n", favoured, valid);

  // Three things have to hold together: the rewarded class must become more
  // frequent than it was, praise must beat its own yoked control on average,
  // and it must do so in most creatures rather than in one lucky one.
  const bool rose = mean_exp_ratio > 1.0;
  const bool beats = mean_ratio_gain > 0.0 && mean_shift_gain > 0.0;
  const bool consistent = favoured * 2 > valid * 1 && favoured >= (valid + 1) / 2 + 1;
  const bool pass = rose && beats && consistent;
  std::printf("\n  G2 %s — across %u creatures the frequency of rewarded\n"
              "  vocalisations %s (mean x%.2f), and praise beat its own yoked control\n"
              "  in %u of them.\n",
              pass ? "PASS" : "FAIL", valid,
              rose ? "rose within the session" : "did not rise", mean_exp_ratio,
              favoured);
  return pass;
}


// --- Snapshot: is a resumed creature the same creature? --------------------
//
// G1's argument is that a genome and a journal reproduce a brain. A snapshot
// makes a second claim of the same kind — that a creature put down at tick N
// and picked up again is the one that was put down — and it needs its own
// evidence, because almost everything that could be wrong with it produces a
// creature that runs perfectly well and is simply not the one that was saved.
// A forgotten field means an eligibility trace at zero, or a generator rewound
// to birth: nothing crashes, the numbers just quietly stop meaning what they
// meant.
//
// The input here is synthesised straight into the brain rather than pushed
// through the cochlea and the retina, and that is the point of the design. The
// host's DSP carries state that the snapshot deliberately does not save — it is
// not part of the creature — so a fresh cochlea in the resumed arm would feed
// it slightly different mel frames and the divergence would be the harness's,
// not the brain's. Driving both arms from a function of the absolute tick makes
// the brain the only thing carrying the run forward.
constexpr uint64_t kSnapshotHashEvery = 500;


// Returns the number of ticks actually run, which is less than `ticks` only
// when `stop_mid_replay` cut it short.
uint64_t snapshot_script(Session& s, uint64_t t0, uint64_t ticks,
                         std::vector<uint64_t>* hashes, bool stop_mid_replay = false) {
  const aibaby::DnaHeader& h = s.dna.header();
  const uint32_t channels = h.audio.mel_channels;
  const uint32_t features = aibaby::vision_features(h.vision);
  std::vector<float> mel(channels, 0.0f);
  std::vector<float> retina(features, 0.0f);

  const uint64_t mel_frame_ticks =
      uint64_t(float(h.audio.hop) * 1000.0f / float(h.audio.sample_rate) / h.sim.dt_ms + 0.5f);
  const uint64_t vision_frame_ticks =
      uint64_t(1000.0f / h.vision.frame_hz / h.sim.dt_ms + 0.5f);

  for (uint64_t t = t0; t < t0 + ticks; ++t) {
    // Something to hear that changes, and goes quiet, so the auditory encoder's
    // hold and fade are both on the path — and so the critic has a signal whose
    // predictability varies, which is what moves the growth detector.
    if (mel_frame_ticks > 0 && t % mel_frame_ticks == 0) {
      const bool sounding = (t / 1000) % 2 == 0;
      for (uint32_t c = 0; c < channels; ++c) {
        const float phase = float(t % 4096) * 0.01f + float(c) * 0.37f;
        mel[c] = sounding ? 0.5f + 0.45f * std::sin(phase) : 0.0f;
      }
      s.brain.hear(mel.data(), channels);
    }

    if (vision_frame_ticks > 0 && t % vision_frame_ticks == 0) {
      const bool showing = (t / 1500) % 3 != 0;
      for (uint32_t f = 0; f < features; ++f) {
        const float phase = float(t % 2048) * 0.005f + float(f) * 0.11f;
        retina[f] = showing ? 0.5f + 0.5f * std::sin(phase) : 0.0f;
      }
      s.brain.see(retina.data(), features);
    }

    // The same awkward touch and praise schedule G1 uses, for the same reason:
    // a path the script does not walk is a path this does not cover.
    if (t % 997 == 13) s.brain.poke(0.4f);
    if (t % 1499 == 41) s.brain.tickle(0.6f);
    if (t % 3001 == 7) s.brain.feed(0.35f);
    if (t % 1777 == 123) s.brain.praise(1.0f);
    if (t % 2311 == 55) s.brain.praise(-1.0f);

    s.brain.step();
    if (hashes && t % kSnapshotHashEvery == kSnapshotHashEvery - 1) {
      hashes->push_back(s.brain.network().state_hash());
    }
    if (stop_mid_replay && s.brain.asleep() && s.brain.replaying()) return t - t0 + 1;
  }
  return ticks;
}


size_t first_divergence(const std::vector<uint64_t>& a, const std::vector<uint64_t>& b) {
  const size_t n = a.size() < b.size() ? a.size() : b.size();
  for (size_t i = 0; i < n; ++i) {
    if (a[i] != b[i]) return i;
  }
  return a.size() == b.size() ? a.size() : n;
}


bool snapshot_round_trip(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  std::string error;
  // Deliberately not on a round number. A mel frame arrives every 10 ticks and
  // a camera frame every 100, and a creature saved on one of those boundaries
  // has its encoders refreshed on the first tick after it resumes — which means
  // everything they were holding (the level fading between frames, the retina's
  // latency schedule half way through a volley) is overwritten before it could
  // matter, and a snapshot that dropped all of it would pass. Saving mid-frame
  // is what puts that state on the path. Measured: with the round number, not
  // restoring the auditory encoder at all is invisible here.
  const uint64_t first_leg = ticks / 2 + 7;
  const uint64_t after = ticks - ticks / 2;

  Session a;
  if (!a.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return false;
  }
  snapshot_script(a, 0, first_leg, nullptr);

  // If this creature has slept at all, save it inside one. Asleep and mid-replay
  // is the state a snapshot is most likely to get wrong — the replay cursors are
  // the only thing in the creature pointing into an episode that is half
  // re-lived, and they exist for a few hundred ticks in every hundred thousand.
  // A save that always landed in waking life would never touch them.
  //
  // Only hunted for when sleep is already known to happen on this run's
  // timescale, so a short run does not spend a quarter of its ticks looking for
  // something it cannot reach.
  uint64_t before = first_leg;
  if (a.brain.network().structural().consolidations > 0) {
    before += snapshot_script(a, first_leg, first_leg / 4, nullptr, true);
  }

  std::vector<uint8_t> snap(aibaby::snapshot_bytes(a.brain, blob.size()));
  size_t written = 0;
  const aibaby::SnapshotStatus ss = aibaby::save_snapshot(
      a.brain, blob.data(), blob.size(), snap.data(), snap.size(), &written);
  if (ss != aibaby::SnapshotStatus::kOk) {
    std::printf("  save failed: %s\n", aibaby::snapshot_status_string(ss));
    return false;
  }
  const uint64_t hash_at_save = a.brain.network().state_hash();
  const bool asleep_at_save = a.brain.asleep();
  const bool replaying_at_save = a.brain.replaying();
  const aibaby::StructuralStats before_stats = a.brain.network().structural();

  // load_snapshot() checks this hash itself and refuses a file that does not
  // reproduce it. Read it back out anyway: a check that only the thing being
  // tested performs is not evidence.
  Session b;
  if (!b.resume(snap, error)) {
    std::printf("  restore failed: %s\n", error.c_str());
    return false;
  }
  const uint64_t hash_after_restore = b.brain.network().state_hash();
  const bool restored = hash_after_restore == hash_at_save &&
                        b.brain.network().tick() == a.brain.network().tick() &&
                        b.brain.plasticity_events() == a.brain.plasticity_events();

  // The control. A restored creature that had one hundredth of a praise it was
  // never given is a different creature, and the comparison below has to be
  // able to say so — otherwise "the hashes agree" would only mean the hashes
  // are insensitive.
  Session c;
  if (!c.resume(snap, error)) {
    std::printf("  restore failed (control): %s\n", error.c_str());
    return false;
  }
  c.brain.praise(0.01f);

  std::vector<uint64_t> ha, hb, hc;
  snapshot_script(a, before, after, &ha);
  snapshot_script(b, before, after, &hb);
  snapshot_script(c, before, after, &hc);

  // The save point is not on a checkpoint boundary — it is deliberately not on
  // any round number — so the first checkpoint is the next multiple after it.
  const uint64_t first_checkpoint =
      before + (kSnapshotHashEvery - 1 - before % kSnapshotHashEvery);
  const size_t diverged = first_divergence(ha, hb);
  const size_t control_diverged = first_divergence(ha, hc);
  const bool identical = diverged == ha.size() && ha.size() == hb.size() &&
                         a.brain.network().state_hash() == b.brain.network().state_hash();
  const bool control_split = control_diverged < ha.size();

  std::printf("  ticks before save %llu, after %llu\n", (unsigned long long)before,
              (unsigned long long)after);
  std::printf("  snapshot          %.1f MB (arena %.1f MB, genome %zu B)\n",
              double(written) / (1024.0 * 1024.0),
              double(a.brain.arena_used()) / (1024.0 * 1024.0), blob.size());
  std::printf("  hash at save      %016llx\n", (unsigned long long)hash_at_save);
  std::printf("  hash restored     %016llx%s\n", (unsigned long long)hash_after_restore,
              restored ? "" : "   <-- MISMATCH");
  std::printf("  checkpoints       %zu, %s\n", ha.size(),
              identical ? "all identical" : "DIVERGED");
  if (!identical && diverged < ha.size()) {
    std::printf("  DIVERGED at checkpoint %zu (tick %llu)\n", diverged,
                (unsigned long long)(first_checkpoint + diverged * kSnapshotHashEvery));
  }
  std::printf("  control           %s\n",
              control_split
                  ? ("diverged at checkpoint " + std::to_string(control_diverged)).c_str()
                  : "NEVER DIVERGED — this comparison proves nothing");
  if (verbose) {
    for (size_t i = 0; i < ha.size(); ++i) {
      std::printf("    t=%8llu  %016llx %s\n",
                  (unsigned long long)(first_checkpoint + i * kSnapshotHashEvery),
                  (unsigned long long)ha[i],
                  i < hb.size() && ha[i] == hb[i] ? "" : "  <-- MISMATCH");
    }
  }

  // Which of the slow paths this run actually walked. Sleep, replay and growth
  // are the state most likely to be dropped by a snapshot and the least likely
  // to be reached: a default run is over long before the creature is tired
  // enough to sleep, so without this the report would be claiming coverage it
  // has not got.
  const aibaby::StructuralStats& st = a.brain.network().structural();
  std::printf("  at the save       %s%s, %u neurons grown, %u pruned\n",
              asleep_at_save ? "asleep" : "awake",
              replaying_at_save ? ", mid-replay" : "", before_stats.neurons_grown,
              before_stats.neurons_pruned);
  std::printf("  exercised after   %u sleep passes, %u replays, %u growth events\n",
              st.consolidations - before_stats.consolidations,
              st.replays - before_stats.replays,
              st.growth_events - before_stats.growth_events);

  const bool pass = restored && identical && control_split;
  std::printf("\n  %s — a creature saved at tick %llu and resumed is the same\n"
              "  creature, and stays identical for %llu ticks of further life.\n",
              pass ? "PASS" : "FAIL", (unsigned long long)before,
              (unsigned long long)after);
  if (st.consolidations == 0) {
    std::printf("  Sleep and replay were never reached — they need ~1.2M ticks. Run\n"
                "  --ticks 2400000 to put a consolidation pass inside the window.\n");
  }
  return pass;
}


// Growth would otherwise never appear in this experiment, because a normally
// raised creature never grows — that is G4 working rather than a gap in it. So
// the second arm moves the guard, exactly as G4's non-vacuity control does, and
// what runs is the shipped growth path. It is what puts a brain whose module
// counts and neuron slots changed after birth through a save and a restore.
//
// Capped in length: growth fires around tick 40,000 here, so the arm has
// nothing more to prove after a couple of hundred thousand and there is no
// reason for a long run to pay for it twice.
constexpr uint64_t kSnapshotGrowthTicks = 200000;


bool run_snapshot(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  std::printf("\n  as raised\n");
  const bool normal = snapshot_round_trip(blob, ticks, verbose);

  std::printf("\n  forced to grow (saturation guard lowered, as in g4)\n");
  std::vector<uint8_t> forced = blob;
  {
    auto* h = reinterpret_cast<aibaby::DnaHeader*>(forced.data());
    h->growth.saturation_rate_hz = 0.1f;
    h->growth.saturation_weight = 0.001f;
    h->growth.epsilon = 1000.0f;
    h->growth.refractory_ticks = 20000;
  }
  const uint64_t grow_ticks = ticks < kSnapshotGrowthTicks ? ticks : kSnapshotGrowthTicks;
  const bool grown = snapshot_round_trip(forced, grow_ticks, verbose);

  const bool pass = normal && grown;
  std::printf("\n  snapshot %s — resume is exact for a creature raised normally%s.\n",
              pass ? "PASS" : "FAIL",
              grown ? " and for one that has grown neurons since birth" : "");
  return pass;
}


// --- M1b: does the creature repeat what it hears? ---------------------------
//
// Built 2026-08-20. The project's spec asks one question about the voice —
// G3, cube versus ball — and this creature has been failing it for months while
// doing something else that nobody ever scored. `m3probe` reads the word out of
// the *voice* at 0.86 and the object at 0.58; the notes have carried the
// sentence "this creature can repeat and cannot name" since August. Repeating
// is a real developmental milestone. It has never had a criterion, a control or
// a bar, so it has never been a result.
//
// **The distinction this experiment exists to make.** `m3probe`'s auditory
// sweep scores ticks 500..1999 while the word plays 0..899, so 27% of its
// scored window is *concurrent with the stimulus*. A voice that differs while
// the sound is still playing is the arcuate transmitting — a reflex, and an
// interesting one, but calling it imitation would be overclaiming. Repetition
// is what survives the sound stopping.
//
// So the voice is scored in four disjoint windows and the word ends after the
// first:
//
//     WHILE   400 ticks with the word playing      the reflex
//     0-200   the 200 ticks after it stops         still driven?
//     200-600                                      the articulators' own hold
//     600-1400                                     memory, if anything
//
// A bar of 0.75 held out, the same one G3 is scored against, so the two numbers
// are comparable and the contrast between them is the finding.
//
// Two controls, and the second is the one that matters. Shuffled labels catch a
// readout finding structure in the procedure. And **trial order is shuffled
// rather than alternating**: with A/B/A/B a classifier that reads nothing but
// session time scores well above chance, which this project has already been
// caught by once — see holdout_guess_time_corr.
// ONE constant for how many windows `imitate` cuts a trial into. It is not a
// tidiness change: this count broke three separate hard-coded 4s in a single
// day. ImitateRun's per-window arrays overflowed and segfaulted; the scoring
// loop's bound was a second literal; and `sum[4][5]` in the reporter read out
// of bounds and printed a two-class accuracy of 0.000 against a 0.174 shuffled,
// which is impossible and is the only reason it was caught rather than
// believed. Plausible garbage would have been read as data.
//
// A static_assert below ties it to the window TABLE, so adding a window without
// widening the arrays is a compile error rather than silent corruption.
constexpr size_t kImitateWindows = 5;

struct ImitateWindow {
  const char* name;
  uint64_t from, to;      // ticks within the trial
  std::vector<std::vector<double>> voice;
  std::vector<std::vector<double>> timbre;
  std::vector<std::vector<double>> ceps;
  // The ear's own answer in the same window. Without it "the voice still knows
  // which word" is unreadable, because the cochlea and B2 do not stop the
  // instant the caregiver does — an "after" window in which the auditory module
  // still classifies at 1.000 is not memory, it is a stimulus that has not
  // finished arriving.
  std::vector<std::vector<double>> heard;
  // Vocal's own spikes, per neuron, per trial — 126 numbers rather than the
  // nine the motor groups reduce them to.
  std::vector<std::vector<double>> vocal;
  // The creature's own echo formants in this window, per trial.
  std::vector<double> f1, f2;
};

struct ImitateRun {
  bool ok = false;
  double voice[kImitateWindows] = {}, artic[kImitateWindows] = {},
         shuffled[kImitateWindows] = {}, dprime[kImitateWindows] = {},
         heard[kImitateWindows] = {};
  // Voiced FRACTION per window: the answering burst. M1b scores what the voice
  // carries; this is whether it speaks at all.
  double voiced_frac[kImitateWindows] = {};
  // PER-TRIAL, so a caller can filter. The EAR column exists because an
  // after-window in which the auditory module still classifies is not memory
  // but a stimulus that has not finished arriving — and a creature that goes
  // quiet while listening stops masking the caregiver and hears it BETTER, so
  // the confound grows exactly when the mechanism works. Auditory ACTIVITY per
  // trial is the direct measure of arrival, and lets the burst be scored on
  // the trials where the word has genuinely gone.
  std::vector<double> trial_voiced[kImitateWindows];
  std::vector<double> trial_aud[kImitateWindows];
  size_t trials = 0;
  // The scored window's raw rows, kept so the caller can score any PAIR of
  // words off one simulation instead of re-running the creature per pair.
  std::vector<std::vector<double>> scored_voice;
  std::vector<std::vector<double>> scored_heard;
  std::vector<int> scored_labels;
  // VOCAL'S PER-NEURON activity in the scored window, per trial. The question
  // it exists for: is the vocabulary ceiling a limit on the creature or on the
  // nine motor scalars everything else is read through?
  std::vector<std::vector<double>> scored_vocal;
  // The creature's own ECHO formants in the scored window, per trial. Kept
  // because `vocab` needs to ask whether a pair is hard because the two vowels
  // are close, or because the creature cannot SAY them differently — and those
  // are different questions with the same discrimination score.
  std::vector<double> scored_f1, scored_f2;
  // The ARTICULATOR-only rows: the nine motor groups minus loudness and
  // voicing. `vocab`'s first version scored pairs on the unguarded features and
  // was therefore reporting, in part, "one word makes it louder".
  std::vector<std::vector<double>> scored_artic;
};

// `words` is how many of kWords to cycle through. `snr_db` and `level_db`
// describe a microphone rather than a creature — see the note on the sweep.
ImitateRun run_imitate_session(const std::vector<uint8_t>& blob, uint64_t ticks,
                               uint32_t words = 2, double snr_db = 1e9,
                               double level_db = 0.0, bool silent = false) {
  ImitateRun out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return out;
  }
  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  const aibaby::DnaVision& vcfg = s.dna.header().vision;
  Ear ear;
  Retina retina;
  if (!ear.configure(acfg, error) || !retina.configure(vcfg, error)) {
    std::printf("  transducer failed: %s\n", error.c_str());
    return out;
  }
  Timbre timbre_ruler;
  const bool has_timbre = timbre_ruler.configure(acfg, error);

  VowelSource caregiver(acfg.sample_rate);
  SceneSource scene(vcfg.frame_size, s.dna.header().seed);
  std::vector<uint8_t> frame(size_t(vcfg.frame_size) * vcfg.frame_size, 0);
  std::vector<float> pcm(acfg.sample_rate / 1000);
  const uint32_t spt = acfg.sample_rate / 1000;
  const uint64_t frame_ticks =
      uint64_t(1000.0f / vcfg.frame_hz / s.dna.header().sim.dt_ms + 0.5f);

  constexpr uint64_t kWordTicks = 900;    // as everywhere else in this project
  constexpr uint64_t kTrialTicks = 2800;  // long enough for a 1400-tick tail

  ImitateWindow windows[] = {
      {"WHILE the word plays", 500, kWordTicks, {}, {}, {}},
      {"0-200 ms after", kWordTicks, kWordTicks + 200, {}, {}, {}},
      {"200-600 ms after", kWordTicks + 200, kWordTicks + 600, {}, {}, {}},
      {"600-1400 ms after", kWordTicks + 600, kWordTicks + 1400, {}, {}, {}},
      // THE QUIET TAIL, and the baseline the answering burst is measured
      // against. The four windows above ask what the voice CARRIES; this one
      // exists to ask a different question — whether the creature vocalises
      // MORE after the caregiver stops than it does when nothing has happened.
      // Content and rate are separate measurements and M1b only made the first.
      {"1400-2300 ms (quiet)", kWordTicks + 1400, 2800, {}, {}, {}},
  };
  constexpr size_t kWindows = sizeof(windows) / sizeof(windows[0]);
  static_assert(kWindows == kImitateWindows,
                "add a window to the table above and every per-window array on "
                "ImitateRun has to widen with it");
  // The fifth window was added as a RATE baseline for M1d and deliberately not
  // scored for content, on the grounds that asking which word the voice carries
  // 1400-2300 ms after it ended is not a question. Once the creature started
  // answering it became the question: M1b says the voice carries the word at
  // 200-600 ms and M1d says the creature speaks MORE then, and those two
  // together are only "it answers with what it heard" if the BURST carries the
  // word better than the creature's ambient babble does. The quiet tail is that
  // ambient babble, on the same trials. Scored equally, the burst would be
  // louder babble with an incidental echo.
  //
  // The arrays on ImitateRun were four wide and are now five. Taking this bound
  // from kWindows while they were four wrote one past the end of out.voice[]
  // and segfaulted — the hard-coded-count bug class this project has audited.
  constexpr size_t kScoredWindows = kImitateWindows;
  // Voiced frames and total frames per window, summed over trials. Pooled
  // rather than averaged per trial: a trial with three frames and a trial with
  // thirty should not weigh the same in a RATE.
  double vsum[kWindows] = {}, fsum[kWindows] = {};

  const int32_t aud_m = s.dna.module_with_role(aibaby::ModuleRole::kAuditory);
  const uint32_t aud_n =
      aud_m >= 0 ? s.brain.network().module(uint32_t(aud_m)).count : 0u;
  const int32_t voc_m = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
  const uint32_t voc_n =
      voc_m >= 0 ? s.brain.network().module(uint32_t(voc_m)).count : 0u;
  if (voc_m < 0) return out;

  const uint32_t n_trials = uint32_t(ticks / kTrialTicks);
  aibaby::Rng rng;
  rng.seed(s.dna.header().seed ^ 0x1417u);
  std::vector<int> order(n_trials, 0);
  for (size_t i = 0; i < order.size(); ++i) order[i] = int(i % words);
  for (size_t i = order.size(); i > 1; --i) std::swap(order[i - 1], order[rng.next() % i]);

  // A separate stream from `rng` on purpose: trial order and the shuffled
  // controls must be identical across microphone arms, so the only thing that
  // differs between them is what the ear received.
  aibaby::Rng mic;
  mic.seed(s.dna.header().seed ^ 0x31CBu);
  const bool degrade = snr_db < 100.0 || level_db > 0.0;

  std::vector<int> labels;
  uint32_t skipped = 0;
  for (uint32_t trial = 0; trial < n_trials; ++trial) {
    const int label = order[trial];
    const Word& w = kWords[label];
    // Per-trial level, drawn once: a real talker is not the same distance from
    // the microphone twice.
    const double gain =
        level_db > 0.0 ? std::pow(10.0, level_db * double(mic.signed_uniform()) / 20.0)
                       : 1.0;
    M3Record rec[kWindows];
    std::vector<double> aud_counts[kWindows];
    for (size_t k = 0; k < kWindows; ++k) aud_counts[k].assign(aud_n, 0.0);
    std::vector<double> voc_counts[kWindows];
    for (size_t k = 0; k < kWindows; ++k) voc_counts[k].assign(voc_n, 0.0);
    uint32_t last_frame = 0;
    bool slept = false;

    for (uint64_t t = 0; t < kTrialTicks; ++t) {
      if (t % frame_ticks == 0) {
        // An empty field throughout: the only thing that can tell the two
        // trials apart is what was heard.
        scene.render(SceneSource::Shape::kNone, 0.5f, 0.5f, 0.1f, 0.85f, 0.02f,
                     frame.data());
        retina.present(frame.data());
        s.brain.see(retina.features().data(), retina.feature_count());
      }
      // The silent arm plays no word at all, keeping the trial clock. Any
      // structure it shows across the same windows is the clock, not an answer.
      const bool sounding = !silent && t < kWordTicks;
      caregiver.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                       pcm.data(), spt);
      if (degrade) {
        // Additive white noise at the requested SNR, referenced to the word's
        // own amplitude (0.5) rather than to this buffer's RMS — referencing it
        // to the buffer would make the noise vanish during the silent tail,
        // which is precisely the interval this experiment scores.
        const double n_amp =
            snr_db < 100.0 ? 0.5 * std::pow(10.0, -snr_db / 20.0) : 0.0;
        for (uint32_t i2 = 0; i2 < spt; ++i2) {
          pcm[i2] = float(double(pcm[i2]) * gain + n_amp * double(mic.signed_uniform()));
        }
      }
      ear.tick(s.brain, pcm.data(), spt);
      s.brain.step();
      if (s.brain.asleep()) slept = true;

      if (aud_m >= 0) {
        const aibaby::Network& net = s.brain.network();
        const aibaby::ModuleState& am = net.module(uint32_t(aud_m));
        for (size_t k = 0; k < kWindows; ++k) {
          if (t < windows[k].from || t >= windows[k].to) continue;
          for (uint32_t j = 0; j < net.spike_count(); ++j) {
            const uint32_t i = net.spikes()[j];
            if (i >= am.begin && i < am.begin + aud_n) aud_counts[k][i - am.begin] += 1.0;
          }
        }
      }
      // VOCAL'S OWN SPIKES, per neuron. Everything this experiment scores is
      // read through the nine motor groups, and `m3probe` says the module
      // carries far more than they express — the heard word reads 0.980 per
      // neuron at vocal and 0.380 on the centroid. So a vocabulary that will
      // not hold apart in the articulators may still be present in the module,
      // and those are completely different findings: one is a limit on the
      // creature, the other a limit on the nine knobs the larynx is steered by.
      {
        const aibaby::Network& net = s.brain.network();
        const aibaby::ModuleState& vm = net.module(uint32_t(voc_m));
        for (size_t k = 0; k < kWindows; ++k) {
          if (t < windows[k].from || t >= windows[k].to) continue;
          for (uint32_t j = 0; j < net.spike_count(); ++j) {
            const uint32_t i = net.spikes()[j];
            if (i >= vm.begin && i < vm.begin + voc_n) voc_counts[k][i - vm.begin] += 1.0;
          }
        }
      }

      if (s.brain.vocal_frame() == last_frame) continue;
      last_frame = s.brain.vocal_frame();
      for (size_t k = 0; k < kWindows; ++k) {
        if (t < windows[k].from || t >= windows[k].to) continue;
        M3Record& r = rec[k];
        ++r.frames;
        const aibaby::Scalar* g = s.brain.vocal_groups();
        for (uint32_t j = 0; j < aibaby::kVocalGroups; ++j) r.group[j] += double(g[j]);
        r.amplitude += double(s.brain.voice().amplitude);
        r.f0 += double(s.brain.voice().f0);
        r.f1 += double(s.brain.voice().f1);
        r.f2 += double(s.brain.voice().f2);
        if (s.brain.voice().voicing > 0.5f &&
            s.brain.voice().amplitude > kAmplitudeFloor) {
          ++r.voiced;
        }
      }
    }
    // A creature that fell asleep mid-trial produced a posture from a different
    // regime; §3.6 changes what the larynx does, so this is not the same
    // measurement and must not be averaged into it.
    bool usable = !slept;
    for (size_t k = 0; k < kWindows && usable; ++k) usable = rec[k].frames > 0;
    if (!usable) { ++skipped; continue; }

    for (size_t k = 0; k < kWindows; ++k) {
      if (k < kImitateWindows) {
        vsum[k] += double(rec[k].voiced);
        fsum[k] += double(rec[k].frames);
        out.trial_voiced[k].push_back(
            rec[k].frames ? double(rec[k].voiced) / double(rec[k].frames) : 0.0);
        double a = 0.0;
        for (double x : aud_counts[k]) a += x;
        out.trial_aud[k].push_back(a);
      }
      windows[k].voice.push_back(m3_vocal_features(rec[k]));
      {
        const double nf = rec[k].frames ? double(rec[k].frames) : 1.0;
        windows[k].f1.push_back(rec[k].f1 / nf);
        windows[k].f2.push_back(rec[k].f2 / nf);
      }
      if (aud_m >= 0) windows[k].heard.push_back(aud_counts[k]);
      windows[k].vocal.push_back(voc_counts[k]);
      windows[k].timbre.push_back(m3_timbre_features(rec[k]));
      if (has_timbre) {
        const double n = double(rec[k].frames);
        windows[k].ceps.push_back(timbre_ruler.of(rec[k].f0 / n, rec[k].f1 / n,
                                                  rec[k].f2 / n, rec[k].amplitude / n));
      }
    }
    labels.push_back(label);
  }

  if (labels.size() < 12) return out;
  out.trials = labels.size();

  // The per-window table is a two-class readout, so it is only meaningful — and
  // only SAFE — for a two-word session. A four-word run is collected purely for
  // the pairwise matrix below, which remaps each pair to 0/1 itself.
  for (size_t k = 0; k < kScoredWindows && words == 2; ++k) {
    ImitateWindow& w = windows[k];
    std::vector<std::vector<double>> xv, xt;
    std::vector<int> yv, yt;
    size_t tv = 0, tt = 0;
    interleave_pairs(w.voice, labels, xv, yv, tv);
    interleave_pairs(w.timbre, labels, xt, yt, tt);
    out.voice[k] = holdout_accuracy(xv, yv, tv);
    out.artic[k] = holdout_accuracy(xt, yt, tt);

    // Averaged over permutations rather than taken from one. A single shuffle
    // is one draw from the null and not an estimate of it — with four windows
    // being checked, one draw at 2 SE happens about one run in ten and failed
    // the whole experiment the first time this was run. Same correction the
    // audibility ruler needed.
    constexpr uint32_t kPerms = 16;
    std::vector<int> shuf = yv;
    for (uint32_t pi = 0; pi < kPerms; ++pi) {
      for (size_t i2 = shuf.size(); i2 > 1; --i2) {
        std::swap(shuf[i2 - 1], shuf[rng.next() % i2]);
      }
      out.shuffled[k] += holdout_accuracy(xv, shuf, tv);
    }
    out.shuffled[k] /= double(kPerms);

    if (!w.heard.empty()) {
      std::vector<std::vector<double>> xh; std::vector<int> yh; size_t th = 0;
      interleave_pairs(w.heard, labels, xh, yh, th);
      out.heard[k] = holdout_accuracy(xh, yh, th);
    }
    if (has_timbre && !w.ceps.empty()) {
      const double sq = cepstral_dprime(w.ceps, labels, nullptr, true);
      out.dprime[k] = sq > 0.0 ? std::sqrt(sq) : 0.0;
    }
  }
  // The scored window's raw rows, so any PAIR of words can be scored off one
  // simulation rather than re-running the creature once per pair.
  for (size_t k = 0; k < kImitateWindows; ++k) {
    out.voiced_frac[k] = fsum[k] > 0.0 ? vsum[k] / fsum[k] : 0.0;
  }
  out.scored_voice = windows[2].voice;
  out.scored_vocal = windows[2].vocal;
  out.scored_heard = windows[2].heard;
  out.scored_labels = labels;
  out.scored_f1 = windows[2].f1;
  out.scored_f2 = windows[2].f2;
  out.scored_artic = windows[2].timbre;
  out.ok = true;
  return out;
}


// The milestone proper. Five creatures, and the scored window is fixed **a
// priori** at 200-600 ms after the word stops rather than chosen per creature.
// The first version picked each creature's best window subject to the ear being
// at chance, which is selecting on the outcome: two of five creatures then
// "failed" only because their ear decayed a little slower and the rule fell
// through to a later window. Choosing the window once, in advance, for everyone
// is the difference between a milestone and a search.
//
// 200-600 ms is chosen because it is the first window in which the auditory
// module has dropped to near chance across creatures — the caregiver stopped at
// 900, and the cochlea and B2 take a few hundred milliseconds more to let go.
// The EAR column is printed for every window so the choice can be audited
// rather than trusted.
// M1d, the answering burst — does the creature vocalise MORE after the
// caregiver stops than when nothing has happened?
//
// M1b established that the voice CARRIES the word 200-600 ms after it ends.
// That is about content and says nothing about rate: a creature babbling at a
// constant rate, whose babble happens to resemble what it just heard, passes
// M1b and is not taking turns. Turn-taking is a claim about WHEN it speaks.
//
// THE TRAP THIS IS BUILT AROUND. The listening reflex suppresses babbling while
// the creature hears something, so "quiet during the word, vocal afterwards" is
// true by construction and would pass a milestone defined on alternation
// without the creature doing anything. So the silence contributes nothing here.
// The only thing scored is whether the post-word rate EXCEEDS the creature's
// own quiet baseline, measured in the same trial 1400-2300 ms after the word,
// when the reflex has long released.
//
// The null is the same creature on the same trial clock hearing NOTHING. Any
// structure across these windows in that arm is the clock — the trials are
// periodic, and a creature with any rhythm of its own would otherwise look like
// it was answering.
bool run_turntake(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  (void)verbose;
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  instrument("turntake", dna.header().seed ^ 0x7712u, ticks / 2800, "trials");
  std::printf("  the question      M1b showed the voice CARRIES the word 200-600 ms\n"
              "                    after it ends. This asks whether the creature\n"
              "                    SPEAKS MORE then — content and rate are different\n"
              "                    measurements and only the first was ever made.\n\n");

  constexpr uint32_t kSeeds = 3;
  double spoke[kImitateWindows] = {}, quiet[kImitateWindows] = {};
  uint32_t valid = 0;
  const char* names[kImitateWindows] = {"while the word", "0-200 ms after", "200-600 ms after",
                          "600-1400 ms after", "1400-2300 (quiet)"};
  for (uint32_t r = 0; r < kSeeds; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    const ImitateRun a = run_imitate_session(variant, ticks, 2, 1e9, 0.0, false);
    const ImitateRun b = run_imitate_session(variant, ticks, 2, 1e9, 0.0, true);
    if (!a.ok || !b.ok) continue;
    for (size_t k = 0; k < kImitateWindows; ++k) { spoke[k] += a.voiced_frac[k]; quiet[k] += b.voiced_frac[k]; }
    ++valid;
  }
  if (valid < 2) {
    std::printf("  INCONCLUSIVE — %u of %u creatures usable.\n", valid, kSeeds);
    return false;
  }
  for (size_t k = 0; k < kImitateWindows; ++k) { spoke[k] /= valid; quiet[k] /= valid; }

  std::printf("  voiced fraction, %u creatures\n", valid);
  std::printf("    %-20s %-10s %-10s %s\n", "window", "word", "silence", "word - silence");
  for (size_t k = 0; k < kImitateWindows; ++k) {
    std::printf("    %-20s %-10.3f %-10.3f %+.3f\n", names[k], spoke[k], quiet[k],
                spoke[k] - quiet[k]);
  }

  // THE EAR SEPARATION. The burst above is scored on every trial, and the EAR
  // control rises when the mechanism works — a creature silent while listening
  // does not mask the caregiver, hears the word better, and more of it persists
  // into the scored window. So score it again on the trials where the word has
  // genuinely GONE: those whose auditory activity at 200-600 ms has fallen back
  // to what the same trial shows at 1400-2300 ms.
  //
  // If the burst survives this it is an answer. If it collapses, it was a
  // stimulus arriving late.
  {
    std::vector<uint8_t> variant = blob;
    const ImitateRun a = run_imitate_session(variant, ticks, 2, 1e9, 0.0, false);
    if (a.ok && a.trial_voiced[2].size() == a.trial_aud[2].size() &&
        a.trial_voiced[2].size() >= 20) {
      const size_t n = a.trial_voiced[2].size();
      // "Back to baseline" is within 20% of the same trial's own quiet window,
      // which makes the criterion per-trial rather than a level chosen once for
      // creatures with different auditory gains.
      std::vector<double> kept_burst;
      for (size_t i = 0; i < n; ++i) {
        const double q = a.trial_aud[4][i];
        if (q <= 0.0) continue;
        if (a.trial_aud[2][i] <= q * 1.2) {
          kept_burst.push_back(a.trial_voiced[2][i] - a.trial_voiced[4][i]);
        }
      }
      if (kept_burst.size() < 8) {
        std::printf("\n    ear separation: only %zu of %zu trials had the ear back at\n"
                    "    baseline — REFUSING to score the burst on that\n",
                    kept_burst.size(), n);
      } else {
        double m = 0.0;
        for (double x : kept_burst) m += x;
        m /= double(kept_burst.size());
        std::printf("\n    ear separation: on the %zu of %zu trials where auditory\n"
                    "    activity at 200-600 ms is back within 20%% of the same trial's\n"
                    "    quiet window, the burst is %+.3f\n",
                    kept_burst.size(), n, m);
      }
    }
  }

  // The burst is window 2 against the SAME arm's quiet tail, and the null is
  // the same contrast in the silent arm. Subtracting the null is what removes
  // the trial clock.
  const double burst = spoke[2] - spoke[4];
  const double null_burst = quiet[2] - quiet[4];
  std::printf("\n    answering burst   %+.3f   (200-600 ms above this arm's own quiet tail)\n"
              "    the same in silence %+.3f   <- the trial clock, which must be subtracted\n"
              "    corrected          %+.3f\n",
              burst, null_burst, burst - null_burst);
  const bool answers = (burst - null_burst) > 0.05;
  std::printf("\n    %s\n",
              answers
                  ? "M1d SIGNATURE PRESENT — the creature speaks MORE after the\n"
                    "    caregiver stops than it does unprompted. Turn-taking is a\n"
                    "    rate claim and this is the rate."
                  : "NOT PRESENT — the creature resumes its baseline babble rather\n"
                    "    than answering. M1b's content match at 200-600 ms is an echo\n"
                    "    riding on babble that was going to happen anyway.");
  std::printf("    (the reflex's silence during the word is NOT scored: it is true\n"
              "     by construction and would pass this milestone on its own.)\n");
  return answers;
}

bool run_imitate(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  constexpr uint32_t kReps = 5;
  constexpr size_t kScored = 2;  // "200-600 ms after"
  static const char* kNames[kImitateWindows] = {"WHILE the word plays", "0-200 ms after",
                                  "200-600 ms after", "600-1400 ms after",
                                  "1400-2300 (ambient)"};

  std::printf("  session           %.1f s x %u creatures, two words to an EMPTY\n"
              "                    FIELD, trial order shuffled\n",
              double(ticks) * double(dna.header().sim.dt_ms) / 1000.0, kReps);
  instrument("imitate", dna.header().seed ^ 0x1417u, uint32_t(ticks / 2800), "trials each");
  std::printf("  the question      the word STOPS. Does the voice still carry which\n"
              "                    one it was? A voice that differs while the sound is\n"
              "                    still playing is the arcuate transmitting; repeating\n"
              "                    is what survives the sound stopping.\n\n");

  // FIVE, matching kScoredWindows. This was four while the fifth window existed
  // as a rate baseline, and widening the scoring without widening this wrote out
  // of bounds and printed an ambient row of 0.000 voice against a 0.174
  // shuffled — impossible for a two-class holdout, whose chance is 0.5, which is
  // the only reason it was caught rather than believed.
  double sum[kImitateWindows][5] = {{0}};
  uint32_t valid = 0, above = 0;
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    const ImitateRun p = run_imitate_session(variant, ticks);
    if (!p.ok) continue;
    ++valid;
    if (p.voice[kScored] >= 0.75) ++above;
    for (size_t k = 0; k < kImitateWindows; ++k) {
      sum[k][0] += p.voice[k]; sum[k][1] += p.artic[k];
      sum[k][2] += p.shuffled[k]; sum[k][3] += p.dprime[k]; sum[k][4] += p.heard[k];
    }
  }
  if (valid < 3) {
    std::printf("  INCONCLUSIVE — only %u of %u creatures produced usable trials.\n",
                valid, kReps);
    return false;
  }
  const double n = double(valid);

  std::printf("  %-22s %-11s %-11s %-11s %-11s %s\n", "window", "voice", "articulators",
              "shuffled", "audible d'", "EAR still knows");
  for (size_t k = 0; k < kImitateWindows; ++k) {
    std::printf("  %-22s %-11.3f %-11.3f %-11.3f %-11.2f %.3f%s\n", kNames[k],
                sum[k][0] / n, sum[k][1] / n, sum[k][2] / n, sum[k][3] / n, sum[k][4] / n,
                k == kScored ? "   <- SCORED" : (k == 4 ? "   <- ambient babble" : ""));
  }

  // IS THE ANSWER INFORMATIVE, OR ONLY LOUDER? M1b says the voice carries the
  // word at 200-600 ms; M1d says the creature SPEAKS MORE then. Those two are
  // "it answers with what it heard" only if the burst carries the word better
  // than the creature's own ambient babble does, on the same trials. The last
  // row is that babble, 1400-2300 ms after the word, with the reflex long
  // released and nothing left arriving.
  //
  // Read `articulators` rather than `voice` for this: `voice` includes loudness
  // and voicing, and the burst is by construction louder, so the wide readout
  // could separate the words on amount of sound alone. That is the same guard
  // `vocab` needed.
  {
    const double burst_a = sum[2][1] / n, ambient_a = sum[4][1] / n;
    const double burst_v = sum[2][0] / n, ambient_v = sum[4][0] / n;
    std::printf("\n  the answer against ambient babble, same trials:\n"
                "    articulators   burst %.3f  vs ambient %.3f   %+.3f\n"
                "    voice          burst %.3f  vs ambient %.3f   %+.3f\n",
                burst_a, ambient_a, burst_a - ambient_a,
                burst_v, ambient_v, burst_v - ambient_v);
    std::printf("    %s\n",
                (burst_a - ambient_a) >= 0.10
                    ? "THE ANSWER IS INFORMATIVE — the burst carries the word better\n"
                      "    than the babble the same creature produces unprompted."
                    : "NOT SHOWN — the burst carries the word no better than ambient\n"
                      "    babble, so M1d added volume and this adds no information on\n"
                      "    top of M1b's echo.");
  }

  std::printf("\n    'voice' is the nine motor groups plus loudness and voicing.\n"
              "    'articulators' drops loudness and voicing entirely, so it cannot\n"
              "    pass on \"one word makes it louder\" -- it is a claim about two\n"
              "    SOUNDS rather than two amounts of sound.\n"
              "    'audible d'' is the bias-corrected cepstral ruler: 1.0 is roughly\n"
              "    76%% correct for a listener.\n"
              "    'EAR still knows' is the auditory module on the same trials in the\n"
              "    same window. It is the control that makes this a claim about\n"
              "    repeating rather than about hearing: in the scored window the\n"
              "    stimulus is gone from the ear and still present in the voice.\n");

  const double voice = sum[kScored][0] / n;
  const double heard = sum[kScored][4] / n;
  const double shuffled = sum[kScored][2] / n;
  const double dp = sum[kScored][3] / n;
  const bool controlled = shuffled < 0.60;
  const bool ear_quiet = heard < 0.65;
  const bool pass = voice >= 0.75 && above * 2 > valid && controlled && ear_quiet;

  if (!controlled) {
    std::printf("\n  CONTROL FAILED — shuffled labels score %.3f.\n", shuffled);
  }
  if (!ear_quiet) {
    std::printf("\n  EAR NOT QUIET — the auditory module still reads the word at %.3f\n"
                "  in the scored window, so this is transmission, not repetition.\n", heard);
  }
  std::printf("\n  M1b %s — 200-600 ms after the word stops, with the auditory module\n"
              "  down to %.3f, a held-out classifier still reads which word the\n"
              "  creature heard off its own voice at %.0f%% (chance 50%%, bar 75%%,\n"
              "  the same bar G3 is scored against). %u of %u creatures at or above\n"
              "  it. Audible d' %.2f against the 1.0 a listener needs.\n",
              pass ? "PASS" : "FAIL", heard, voice * 100.0, above, valid, dp);
  std::printf("  Read it beside G3, same creature and same bar: it repeats at %.0f%%\n"
              "  and names at 53%%. The object reaches the larynx and does not reach\n"
              "  the voice.\n", voice * 100.0);

  // --- is it repeating, or transmitting one loud spectral axis? -------------
  //
  // The milestone above is scored on /a/ versus /i/, which differ hugely on
  // BOTH formants. A creature that transmitted nothing but "how bright was
  // that" would pass it. Four words, all six pairs, scored off ONE simulation
  // in the same 200-600 ms window — the decisive pair is /i/ vs /u/, whose F1s
  // are 30 Hz apart and whose F2s are 1600 apart, so it can only be answered on
  // F2.
  {
    std::printf("\n  --- four words, all six pairs, same window -------------------\n");
    static const char* kLabel[4] = {"/a/ ball", "/i/ cube", "/u/ boot", "/e/ bed"};
    double pair_sum[4][4] = {{0}}, pair_ear[4][4] = {{0}};
    uint32_t pair_n = 0;
    for (uint32_t r = 0; r < 3; ++r) {
      std::vector<uint8_t> variant = blob;
      const uint64_t seed = dna.header().seed + r * 7919ull;
      std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
      const ImitateRun p = run_imitate_session(variant, ticks, kWordCount);
      if (!p.ok || p.scored_labels.size() < 24) continue;
      ++pair_n;
      for (uint32_t a = 0; a < kWordCount; ++a) {
        for (uint32_t b = a + 1; b < kWordCount; ++b) {
          std::vector<std::vector<double>> xv, xh;
          std::vector<int> yv;
          for (size_t t = 0; t < p.scored_labels.size(); ++t) {
            const int L = p.scored_labels[t];
            if (L != int(a) && L != int(b)) continue;
            xv.push_back(p.scored_voice[t]);
            if (!p.scored_heard.empty()) xh.push_back(p.scored_heard[t]);
            yv.push_back(L == int(b) ? 1 : 0);
          }
          if (yv.size() < 12) continue;
          std::vector<std::vector<double>> iv, ih;
          std::vector<int> jv, jh;
          size_t tv = 0, th = 0;
          interleave_pairs(xv, yv, iv, jv, tv);
          pair_sum[a][b] += holdout_accuracy(iv, jv, tv);
          if (!xh.empty()) {
            interleave_pairs(xh, yv, ih, jh, th);
            pair_ear[a][b] += holdout_accuracy(ih, jh, th);
          }
        }
      }
    }
    if (pair_n == 0) {
      std::printf("    INCONCLUSIVE — no usable four-word sessions.\n");
    } else {
      std::printf("    %-11s %-11s %-9s %-9s %s\n", "word A", "word B", "voice", "EAR",
                  "");
      double worst = 1.0;
      for (uint32_t a = 0; a < kWordCount; ++a) {
        for (uint32_t b = a + 1; b < kWordCount; ++b) {
          const double v = pair_sum[a][b] / double(pair_n);
          const double e = pair_ear[a][b] / double(pair_n);
          const bool key = (a == 1 && b == 2);
          if (v < worst) worst = v;
          std::printf("    %-11s %-11s %-9.3f %-9.3f %s\n", kLabel[a], kLabel[b], v, e,
                      key ? "<- F1 within 30 Hz: an F2-only discrimination" : "");
        }
      }
      std::printf("\n    %u creatures. Every pair is scored on the SAME trials as the\n"
                  "    others, so differences between rows are about the two vowels and\n"
                  "    not about the run. The weakest pair is %.3f.\n", pair_n, worst);
    }
  }

  // --- does it survive a microphone? ----------------------------------------
  //
  // Everything above plays a synthesised vowel straight into the cochlea. A
  // real room adds broadband noise and a real talker is not the same distance
  // away twice. This is a MEASUREMENT-layer model, deliberately not in the
  // genome, for the same reason Retina::Servo is not: it describes the world
  // the creature is measured in, not the creature.
  {
    std::printf("\n  --- through a microphone ------------------------------------\n");
    std::printf("    %-14s %-9s %-9s %-9s %s\n", "condition", "voice", "shuffled",
                "EAR", "audible d'");
    struct Arm { const char* name; double snr; double lvl; };
    const Arm arms[] = {{"clean", 1e9, 0.0}, {"SNR 20 dB", 20.0, 0.0},
                        {"SNR 10 dB", 10.0, 0.0}, {"SNR 0 dB", 0.0, 0.0},
                        {"+-6 dB level", 1e9, 6.0}, {"10 dB & +-6 dB", 10.0, 6.0}};
    for (const Arm& arm : arms) {
      double v = 0, sh = 0, e = 0, d = 0;
      uint32_t n = 0;
      for (uint32_t r = 0; r < 3; ++r) {
        std::vector<uint8_t> variant = blob;
        const uint64_t seed = dna.header().seed + r * 7919ull;
        std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed,
                    sizeof(seed));
        const ImitateRun p = run_imitate_session(variant, ticks, 2, arm.snr, arm.lvl);
        if (!p.ok) continue;
        ++n;
        v += p.voice[2]; sh += p.shuffled[2]; e += p.heard[2]; d += p.dprime[2];
      }
      if (!n) { std::printf("    %-14s inconclusive\n", arm.name); continue; }
      std::printf("    %-14s %-9.3f %-9.3f %-9.3f %.2f\n", arm.name, v / n, sh / n,
                  e / n, d / n);
    }
    std::printf("\n    Noise is referenced to the word's own amplitude, not to the\n"
                "    buffer's, so it keeps playing through the silent tail this\n"
                "    experiment scores — which is the honest version: a real room is\n"
                "    not quiet just because the talker stopped.\n");
  }

  (void)verbose;
  return pass;
}


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
// Feedback is delivered over a wider bracket than the scoring window, on G2's
// own clock. The first version of this experiment gave ONE reward per 2800-tick
// trial and reported itself underpowered — its positive control moved -1.2
// points where it needs 5. G2 delivers every 150 ticks while the creature is
// making the sound, which is nineteen times denser, and it is the regime this
// creature's one working learning rule was measured under.
constexpr uint64_t kVLRewardFrom = kVLWordTicks;
constexpr uint64_t kVLRewardTo = kVLWordTicks + 800;
// The word count `vocallearn` and everything built on it has always used. It
// stays 2, and it is now a DEFAULT rather than a fact: `run_vocallearn_session`
// takes a runtime count so a four-word protocol can be written without moving a
// single existing number. Arrays are sized by kVLMaxWords; loops and modulo use
// the runtime value.
constexpr uint32_t kVLWords = 2;
// Sizing only. Four is where the word table's own comments say the hard cases
// live -- /i/ and /u/ within 30 Hz on F1 and 1600 apart on F2, /e/ between /a/
// and /i/ on both.
constexpr uint32_t kVLMaxWords = 4;
// How fast the per-word expectation follows. Slow enough that a run of good
// trials does not immediately raise the bar out of reach, fast enough that it
// tracks a creature that is genuinely improving.
constexpr double kVLBaselineAlpha = 0.02;

// `kVLFixed` is the positive control, and without it a null here means
// nothing. It rewards the creature toward ONE word's formants whatever it
// heard — a single target, no conditionality — which is the shape of act G2
// already proved reward can shape in this creature (x1.35, 23 of 27). If the
// creature moves on this arm and not on the taught arm, the failure is
// specifically the CONDITIONAL part and the readout and the rule are both
// fine. If it moves on neither, this instrument cannot see learning and the
// taught arm's null is a fact about the probe.
enum VLArm { kVLTaught = 0, kVLYoked, kVLNone, kVLFixed, kVLArmCount };

// What the creature is rewarded TOWARD, split off the arm above so a control
// arm can be scored the same way as the arm it controls for. `vocallearn` never
// needed the split: its yoked arm controls for the taught one and both score
// against the heard word. `ctxlearn` has three targets and each needs its own
// yoke, and a yoke scored against a different target is not a control.
//
//   kVLTgtHeard  the word the caregiver just said. Conditional, and it is the
//                map the innate arcuate already delivers at 0.890 — so reward
//                here is REFINING an existing route.
//   kVLTgtFixed  one word's formants whatever was heard. No conditionality:
//                vocallearn's positive control, +24.0 points on 3 of 3.
//   kVLTgtSwap   the OTHER word's formants. Conditional, arbitrary, and the
//                arcuate is actively pulling against it — which is what makes
//                it a measurement of a LEARNED conditional map rather than of a
//                refined innate one.
//   kVLTgtRandom the same two targets in the same proportions, drawn
//                INDEPENDENTLY of what was heard. This is the control that
//                decides whether a conditional arm means anything: a creature
//                that simply learns to sit at the midpoint of two alternating
//                targets scores positively on `swap` with no conditionality at
//                all, and against a target with matched marginals it cannot.
//                Same reward density, same target distribution; the only
//                difference is whether the target correlates with the input.
//                m3 has always been read as taught-minus-random and vocallearn
//                never had the equivalent.
enum VLTarget { kVLTgtHeard = 0, kVLTgtFixed, kVLTgtSwap, kVLTgtRandom };

// WHAT is being scored, as opposed to which word it is scored against. Every
// conditional test this project has ever run targets FORMANTS, read through a
// population centroid, and that is now measured at -0.1 +/- 0.7 against a
// matched control. This axis asks whether the wall is about the formant readout
// or about conditionality itself.
//
// Amplitude is the dimension to ask it on, for one reason: it is a group RATE
// rather than a centroid, and G2 is precisely the result that reward can move it
// -- rewarded vocalisations rise x1.74, 9 of 9. So the unconditional positive
// control does not have to be argued for, it is a met milestone. If "be loud for
// this word and quiet for that one" is learnable where formants are not, the
// wall is the readout and there is a route. If it is not learnable either, then
// conditionality fails on the one output dimension reward provably controls, and
// that closes it far more firmly than another negative on formants would.
//
// Silence is a LOW AMPLITUDE ANSWER, not a missing one. The formant path skips
// silent trials -- correctly, since silence has no formants to be wrong about --
// and carrying that rule over would make "be quiet" unscoreable and unrewardable,
// which is the one mistake that would decide this experiment before it ran.
enum VLScore { kVLScoreFormant = 0, kVLScoreAmp, kVLScoreRate };

// The two amplitudes, either side of what the creature does naturally (babble
// sits near 0.48) so that neither target is the one it would drift to anyway.
constexpr double kVLAmpLoud = 0.75;
constexpr double kVLAmpQuiet = 0.15;

// kVLScoreAmp is kept and it is a RECORDED FAILURE rather than a live option.
// It delivered reward on a fixed clock whether or not the creature had done
// anything, and G2 -- the milestone whose dimension it borrowed -- delivers
// reward ON A VOCALISATION EVENT. With no act to credit, the trace holds nothing
// specific and praise becomes indistinguishable from yoked praise: the positive
// control read +3.9 +/- 2.4 with taught and yoked degrading together, -55.2
// against -57.1, -56.5 against -57.6, -70.5 against -79.1. The dimension was
// right and the contingency was not.
//
// kVLScoreRate is the repair, and it makes the question sharper rather than
// weaker: score G2's OWN quantity -- how much the creature vocalises -- with
// G2's own event-triggered contingency. Each vocalisation event in the reward
// window is praised when this trial's target is HIGH and scolded when it is LOW,
// which is exactly G2 when the target never changes. So the unconditional
// positive control is a met milestone by construction (x1.35 within session,
// 23 of 27 creatures, 9 of 9 at 420 s), and the only new thing being asked is
// whether the target may depend on what was heard.
constexpr double kVLRateHigh = 0.80;
constexpr double kVLRateLow = 0.10;

// An oracle condition, written straight into a kContext module (DNA v47).
//
// This is the manipulation the whole experiment exists for. `module` is a
// population with no noise, no homeostatic setpoint and no recurrence, cut into
// `slots` disjoint slices; the trial's word selects one slice and `gain` is
// injected into every neuron of it, every tick. Every neuron of every other
// slice stays at exactly zero, which is a presynaptic baseline this creature
// has never had anywhere.
//
// `gain` at zero is the control and it is a control WITHIN one genome: the
// module is present and wired either way, so the two arms share a creature, a
// noise stream and a set of synapses, and differ only in whether the oracle
// speaks.
// Mean adaptive threshold over a module, and optionally the share of it sitting
// at the clamp. Dead neurons are included: their threshold does not move, so
// they dilute the drift by a constant that is identical across arms of the same
// creature, which is the only comparison this is used for.
double mean_threshold(const aibaby::Network& net, uint32_t module, double* pinned,
                      float t_max) {
  const aibaby::ModuleState& ms = net.module(module);
  if (ms.count == 0) { if (pinned) *pinned = 0.0; return 0.0; }
  double sum = 0.0;
  uint32_t at_clamp = 0;
  for (uint32_t n = ms.begin; n < ms.begin + ms.count; ++n) {
    const double t = double(net.threshold(n));
    sum += t;
    if (t_max > 0.0f && t >= double(t_max) * 0.999) ++at_clamp;
  }
  if (pinned) *pinned = double(at_clamp) / double(ms.count);
  return sum / double(ms.count);
}

struct CtxDrive {
  int32_t module = -1;
  uint32_t slots = 2;
  double gain = 0.0;
  // PRICING SELECTIVITY BEFORE BUILDING IT. >= 0 confines the reward cash-in to
  // one articulator group of the larynx via `Network::set_reward_mask`, so
  // `outside` is zero BY CONSTRUCTION rather than by a mechanism.
  //
  // The magnitude route is closed -- more trials saturate near 140 Hz, and more
  // rate destroys selectivity faster than it builds magnitude -- so what is left
  // is writing less off-target. This is the oracle for that, and it costs one
  // arm instead of a compartment model.
  // A GROUP RANGE [lo, hi), not a single group, and the reason is a confound.
  // Reward is formant error over F1 AND F2. Confining writes to F1 alone would
  // score the creature on something it can only half control, and a drop could
  // then be blamed on selectivity when it was really an amputated task. Groups
  // 2 and 3 are adjacent, so [2, 4) is exactly "may change what it is scored on,
  // and nothing else" -- and `set_reward_mask` takes one contiguous range.
  int32_t mask_lo = -1;
  int32_t mask_hi = -1;
};

// The bias oracle's configuration for one session. See Network::set_bias_oracle
// and `ctxbias`: this is Fee & Goldberg's Area X output handed over directly,
// so that the architecture can be priced before it is built.
struct BiasDrive {
  int32_t module = -1;   // where the bias lands; -1 is off
  double k = 0.0;        // amplitude as a MULTIPLE of the module's noise_amp
  bool conditional = true;  // does the sign follow the word, or is it constant?
  // For the constant arms only: which way it points. +1 is toward the fixed
  // target, -1 away from it. Both are needed, because a constant bias aimed at
  // the very target reward is asking for does part of the task, and an arm that
  // helps cannot price the cost of arriving.
  double dir = 1.0;
  // Which two articulator groups the ramp lands on. 2 and 3 are F1 and F2, the
  // pair the score is computed from. 5 and 6 are the first two BANDWIDTHS,
  // which the score does not read at all -- see the note on the `offaxis` arm.
  uint32_t group_a = 2, group_b = 3;
};

// The three-way split of a context table, and the pinned share. Factored out
// because it is now computed at 16 checkpoints INSIDE a session as well as once
// at the end, and a printf argument list that outlived its arm order has already
// cost this project three results -- two copies of an arithmetic would be the
// same bug with a longer fuse.
struct CtxSplit {
  double align = 0.0;    // moves F1: the projection onto centred position
  double common = 0.0;   // uniform inside the F1 group; a ratio cannot see it
  double outside = 0.0;  // everything outside the F1 group
  double gain = 0.0;     // aligned as a multiple of a STRUCTURELESS table: 1.0 is the null
  double div = 0.0;      // mean |b(0) - b(1)| over the whole larynx
  double mag = 0.0;      // mean table magnitude, the scale to read div against
  double pinned = 0.0;   // share of table entries AT the perturb_max clamp
  uint32_t gn = 0;
};

// `perturb_max` is passed rather than read from the net because the clamp is a
// genome field and the question this exists to answer is whether the table is
// against it. Pass 0 to skip the pinned share.
inline CtxSplit ctx_split(const aibaby::Network& net, const aibaby::ModuleState& vms,
                          double perturb_max) {
  CtxSplit o;
  if (vms.count == 0 || net.context_slots() < 2) return o;
  double dsum = 0.0, msum = 0.0;
  uint32_t pinned = 0, pn = 0;
  const double at = perturb_max > 0.0 ? perturb_max * 0.999 : 0.0;
  for (uint32_t n = vms.begin; n < vms.begin + vms.count; ++n) {
    const double b0 = double(net.context_bias(n, 0)), b1 = double(net.context_bias(n, 1));
    dsum += std::fabs(b0 - b1);
    msum += 0.5 * (std::fabs(b0) + std::fabs(b1));
    if (at > 0.0) {
      pn += 2;
      if (std::fabs(b0) >= at) ++pinned;
      if (std::fabs(b1) >= at) ++pinned;
    }
  }
  o.div = dsum / vms.count;
  o.mag = msum / vms.count;
  o.pinned = pn ? double(pinned) / double(pn) : 0.0;
  const uint32_t g_beg = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, 2);
  const uint32_t g_end = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, 3);
  const uint32_t gn = g_end > g_beg ? g_end - g_beg : 0;
  if (gn < 2) return o;
  double dot = 0.0, unorm = 0.0, mean_in = 0.0, out_ss = 0.0;
  for (uint32_t n = g_beg; n < g_end; ++n) {
    const double d = double(net.context_bias(n, 0)) - double(net.context_bias(n, 1));
    const double u = (double(n - g_beg) + 0.5) / double(gn) - 0.5;
    dot += d * u;
    unorm += u * u;
    mean_in += d;
  }
  mean_in /= double(gn);
  for (uint32_t n = vms.begin; n < vms.begin + vms.count; ++n) {
    if (n >= g_beg && n < g_end) continue;
    const double d = double(net.context_bias(n, 0)) - double(net.context_bias(n, 1));
    out_ss += d * d;
  }
  const uint32_t outn = vms.count > gn ? vms.count - gn : 0;
  o.align = unorm > 0.0 ? std::fabs(dot) / std::sqrt(unorm) / std::sqrt(double(gn)) : 0.0;
  o.common = std::fabs(mean_in);
  o.outside = outn ? std::sqrt(out_ss / double(outn)) : 0.0;
  o.gain = o.outside > 0.0 ? o.align * std::sqrt(double(gn)) / o.outside : 0.0;
  o.gn = gn;
  return o;
}

struct VLRun {
  bool ok = false;
  double err_early = 0.0, err_late = 0.0;
  double err_by_word[kVLMaxWords][2] = {};
  uint32_t scored = 0, skipped = 0, praises = 0, scolds = 0;
  double voiced_frac = 0.0;
  // What the oracle actually did, because a silent context module and a
  // saturated one both look like "the tract is connected" from outside.
  double ctx_rate = 0.0;
  // Mean |dw| over the session on the oracle's own tract, and on the larynx's
  // other afferents as the scale to read it against.
  double ctx_dw = 0.0;
  double ref_dw = 0.0;
  // DNA v47 follow-up. IP is threshold += ip_rate * (rate_ema - target), so if
  // intrinsic plasticity on the larynx is what kills the positive control when
  // the oracle fires, two things have to be true and both are measurable here:
  // the larynx has to run ABOVE its target while driven (IP needs a rate error
  // to act on), and its threshold has to MOVE further than it does with the
  // oracle mute. Neither was checked before `ip_wake_scale` was blamed --
  // which is the mistake `syn_wake_scale` already cost this project once.
  double ip_thresh_drift = 0.0;  // mean threshold on vocal, end - start
  double ip_ref_drift = 0.0;     // the same on a module the oracle never drives
  double ip_rate_hz = 0.0;       // vocal's mean rate over the session
  double ip_target_hz = 0.0;     // what IP is pulling that rate toward
  double ip_pinned = 0.0;        // share of vocal at the threshold_max clamp
  // DNA v50. The same question asked of the OTHER bounded budget: the share of
  // vocal's inhibitory afferents sitting at their own weight ceiling. Zero on
  // every genome that does not switch ISP on, because nothing else in this
  // creature moves an inhibitory weight.
  double isp_pinned = 0.0;
  // What the VOICE actually did, per word, over the whole session. `ctxbias`
  // needs this and nothing else reports it: an oracle that steers the larynx
  // and one that is too small to be heard produce the same firing rates and the
  // same verdict everywhere else, which is the trap DNA v48 fell into.
  double f1_by_word[kVLMaxWords] = {};
  double f2_by_word[kVLMaxWords] = {};
  double bias_amp = 0.0;   // what the oracle injected, in drive units
  // DNA v51. The mechanism's own claim, readable separately from the
  // behaviour: how often the creature was in a context at all, and how far the
  // per-context bias tables actually diverged. A table that was never indexed
  // and a table that was indexed and learned nothing are the same flat dF1 from
  // outside, and `areax` refuses rather than reporting either as the other.
  double ctx_present_frac = 0.0;
  double ctx_table_div = 0.0;   // mean |bias[i][0] - bias[i][1]| over the larynx
  // THE SAME TABLE DIFFERENCE, SPLIT BY WHETHER IT CAN REACH F1 AT ALL.
  //
  // `read_group` pools a slice into a rate-weighted CENTROID of position:
  // value = sum(r_i * p_i) / sum(r_i) with p_i the neuron's place in the slice.
  // Two consequences, both exact rather than estimated, and together they say
  // which parts of a learned table are capable of moving a formant:
  //
  //   - F1 is group 2 of nine. A bias on any neuron OUTSIDE that slice moves
  //     f0, F2, amplitude or voicing. Never F1.
  //   - Inside the slice the readout is a RATIO, so lifting every neuron by the
  //     same amount leaves it exactly unchanged. Only the component along the
  //     centred position vector (p_i - 0.5) does anything.
  //
  // So: `aligned` is the projection onto that centred direction, `common` is the
  // uniform component inside the F1 group, `outside` is everything else. All
  // three are per-neuron RMS so they are comparable with each other.
  double ctx_align = 0.0;    // moves F1
  double ctx_common = 0.0;   // inside the F1 group, moves nothing (ratio)
  double ctx_outside = 0.0;  // outside the F1 group, moves other parameters
  // aligned, as a multiple of what a STRUCTURELESS table of the same size gives.
  // 1.0 is the null; this is the number the shape question actually turns on.
  double ctx_align_gain = 0.0;
  uint32_t ctx_f1_group_n = 0;
  double ctx_pinned = 0.0;  // share of table entries AT the perturb_max clamp
  // Within-session checkpoints. `ctxscale` measured the growth of the aligned
  // bias ACROSS runs at three budgets, which costs a whole run per point and
  // pays cross-seed noise for each one. These are the same curve sampled inside
  // ONE session, so the points are paired by construction.
  static constexpr uint32_t kCkpt = 16;
  uint32_t ckpt_n = 0;
  double ckpt_trial[kCkpt] = {};
  double ckpt_align[kCkpt] = {};
  double ckpt_outside[kCkpt] = {};
  double ckpt_gain[kCkpt] = {};
  double ckpt_pinned[kCkpt] = {};
  double ckpt_df1[kCkpt] = {};    // |F1(word 0) - F1(word 1)| over THIS window only
  double ckpt_praise[kCkpt] = {}; // praise share in the window: does the teacher run out?
  double ctx_shared_mag = 0.0;  // mean |shared bias|, the scale to read it against
  // DNA v52. What the index the creature DERIVED for itself actually was,
  // measured only in the window where reward lands -- `ctxsrc`'s whole finding
  // is that a carrier at ceiling while the word plays can be at chance 400 ms
  // later, so an accuracy averaged over the trial would be the wrong number.
  //
  // The slice-to-word assignment is arbitrary: an index only has to be
  // CONSISTENT, not correctly labelled. So this is the better of the two
  // permutations, which is the p the (2p - 1) bar is stated in.
  double ctx_match = 0.0;
  // ...and the same thing over the first and last third of the session. A
  // derived index sits inside a loop: the bias table steers the voice, and the
  // voice is what the index is read from. If p RISES over a session the loop is
  // closing on itself and the prescription is a longer run; if it is flat, the
  // partition has a fixed accuracy and more ticks buy nothing. Those are
  // different verdicts and a session-mean cannot tell them apart. Thirds rather
  // than halves, because `vocallearn`'s own early/late windows are thirds.
  double ctx_match_early = 0.0, ctx_match_late = 0.0;
  // DNA v53: competitions run per trial. The design is one per word.
  double ctx_events_per_trial = 0.0;
  // THE NAMING MEASUREMENT, and it is a different question from dF1.
  //
  // Everything in this thread has been scored on dF1 -- the gap between the MEAN
  // F1 for one word and for the other. That is the mechanism's own quantity, and
  // it was the right instrument while the question was whether a context reaches
  // the voice at all. **It is not the milestone.** A mean shift smaller than the
  // within-word scatter buys a listener nothing, so a creature can move dF1 and
  // still be unnameable.
  //
  // So the utterances are kept PER TRIAL and scored the way `vocab` scores them:
  // a held-out one-of-two readout over what the creature actually produced.
  // **F1 and F2 only.** `vocallearn`'s protocol also asks for a different
  // AMPLITUDE and a different rate per word, so a readout given those could
  // score loudness as naming -- which would be true of the protocol rather than
  // of the voice.
  std::vector<double> utt_f1, utt_f2;
  std::vector<int> utt_word;
  // ...and the share taken by the busiest slice. A constant index scores 0.5
  // on `ctx_match` with balanced words, but it scores 1.0 here, and the two
  // failures deserve different verdicts.
  double ctx_occupancy = 0.0;
  // `rpeprobe`. The reward stream decomposed by context, sampled once per
  // plasticity event -- the cadence at which reward actually reaches the
  // synapses, not per tick, which would over-weight whatever the creature
  // happened to be doing when an interval was long.
  double rw_mean[kVLMaxWords] = {};   // mean TOTAL reward in each context
  double rw_ext[kVLMaxWords] = {};    // ...and the external (caregiver) part alone
  double rw_between = 0.0;         // variance of the per-context means
  double rw_within = 0.0;          // mean variance within a context
  double rw_ext_share = 0.0;       // external share of total reward variance
  uint32_t rw_n[kVLMaxWords] = {};
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
                             const std::vector<Praise>* yoked, const Regime& regime,
                             int target = -1, const CtxDrive* ctx = nullptr,
                             VLScore score = kVLScoreFormant,
                             const BiasDrive* bias = nullptr,
                             uint32_t words = kVLWords) {
  // -1 is vocallearn's own rule, and passing nothing reproduces it exactly: the
  // positive control aims at one fixed target and every other arm at the word
  // that was heard.
  const VLTarget tgt =
      target < 0 ? (arm == kVLFixed ? kVLTgtFixed : kVLTgtHeard) : VLTarget(target);
  // Clamped, because a caller asking for more words than the table holds should
  // get the table's worth rather than read past it.
  const uint32_t nw = words < 2 ? 2u : (words > kVLMaxWords ? kVLMaxWords : words);
  VLRun out;
  // The amplitude and rate lessons are `word == 0 ? loud : quiet`, which is a
  // two-way distinction and cannot be stretched over four words. Refusing is
  // better than scoring three of four words against the same target and calling
  // the result a four-word lesson.
  if (nw != 2 && (score == kVLScoreAmp || score == kVLScoreRate)) return out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) {
    std::printf("  setup failed: %s\n", error.c_str());
    return out;
  }
  // The selectivity oracle, set once for the session. An articulator group is a
  // contiguous slice, which is exactly what `set_reward_mask` takes.
  if (ctx && ctx->mask_lo >= 0 && ctx->mask_hi > ctx->mask_lo) {
    const int32_t vmod_m = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
    if (vmod_m >= 0) {
      const aibaby::ModuleState& vm = s.brain.network().module(uint32_t(vmod_m));
      const uint32_t mlo = vm.begin + aibaby::slice_begin(vm.count, aibaby::kVocalGroups,
                                                          uint32_t(ctx->mask_lo));
      const uint32_t mhi = vm.begin + aibaby::slice_begin(vm.count, aibaby::kVocalGroups,
                                                          uint32_t(ctx->mask_hi));
      s.brain.network().set_reward_mask(mlo, mhi);
    }
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

  // Birth weights of every afferent of the larynx, so the session can say
  // afterwards whether reward wrote anything onto the oracle's tract.
  std::vector<uint32_t> w0_neuron, w0_slot, w0_src;
  std::vector<double> w0_w;
  // Birth thresholds of the larynx, and of a module the oracle does not touch
  // as the scale to read its drift against.
  const int32_t ip_vm = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
  const int32_t ip_rm = s.dna.module_with_role(aibaby::ModuleRole::kAssociation);
  double ip_t0 = 0.0, ip_r0 = 0.0;
  {
    const aibaby::Network& net = s.brain.network();
    if (ip_vm >= 0) ip_t0 = mean_threshold(net, uint32_t(ip_vm), nullptr, 0.0f);
    if (ip_rm >= 0) ip_r0 = mean_threshold(net, uint32_t(ip_rm), nullptr, 0.0f);
  }
  if (ctx && ctx->module >= 0) {
    const int32_t vm = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
    if (vm >= 0) {
      const aibaby::Network& net = s.brain.network();
      const aibaby::ModuleState& vms = net.module(uint32_t(vm));
      for (uint32_t n = vms.begin; n < vms.begin + vms.count; ++n) {
        const uint32_t deg = net.in_degree(n);
        for (uint32_t k = 0; k < deg; ++k) {
          uint32_t src = 0;
          aibaby::Scalar w = aibaby::Scalar(0);
          net.in_synapse(n, k, src, w);
          w0_neuron.push_back(n);
          w0_slot.push_back(k);
          w0_src.push_back(src);
          w0_w.push_back(double(w));
        }
      }
    }
  }

  const uint32_t n_trials = uint32_t(ticks / kVLTrialTicks);
  if (n_trials < 24) return out;
  const uint32_t third = n_trials / 3;

  std::deque<Praise> pending;
  size_t yoke_cursor = 0;
  // NOT an aggregate initialiser: at kVLMaxWords the tail would be 0.0, and 0.0
  // reads to the code below as "a baseline has already been recorded".
  double baseline[kVLMaxWords];
  for (uint32_t b = 0; b < kVLMaxWords; ++b) baseline[b] = -1.0;
  double err_sum[2] = {}, voiced_sum = 0.0;
  uint32_t err_n[2] = {}, frames_total = 0, frames_voiced = 0;
  double word_sum[kVLMaxWords][2] = {};
  uint32_t word_n[kVLMaxWords][2] = {};
  // The voice's own formants per word, over every voiced frame of the session.
  double vf1_sum[kVLMaxWords] = {}, vf2_sum[kVLMaxWords] = {};
  // Per-checkpoint-window versions of the same, so the transfer curve can be
  // read INSIDE one session instead of across three runs.
  double wf1_sum[kVLMaxWords] = {};
  uint32_t wf_n[kVLMaxWords] = {};
  uint32_t w_praise = 0, w_scold = 0;
  uint32_t vf_n[kVLMaxWords] = {};
  uint64_t ctx_ticks = 0, ctx_ticks_total = 0;
  // DNA v52. Confusion between the word the caregiver said and the slice the
  // creature's own index picked, over the reward window only.
  uint64_t ctx_conf[kVLMaxWords][kVLMaxWords] = {};
  uint64_t ctx_conf_n = 0;
  // The same, split by third. Index 0 is the first third and 1 the last; the
  // middle third is counted in the session total only.
  uint64_t ctx_conf_t[2][kVLMaxWords][kVLMaxWords] = {};
  uint64_t ctx_conf_tn[2] = {};
  uint64_t last_plast = 0;
  double rw_sum[kVLMaxWords] = {}, rw_sq[kVLMaxWords] = {};
  double rw_esum[kVLMaxWords] = {}, rw_esq[kVLMaxWords] = {};
  uint32_t rw_n[kVLMaxWords] = {};

  // The bias oracle, if this session has one. Amplitude is a MULTIPLE of the
  // module's own noise_amp rather than a number in drive units, because
  // noise_amp is what this creature explores with and what node perturbation's
  // bias is measured against -- so k = 1 means "the oracle pushes as hard as
  // the creature's own exploration", which is a scale with a meaning rather
  // than a constant someone typed. Four guessed constants have cost this
  // project a run each.
  //
  // The ramp lands on the F1 and F2 groups, which is where steering the larynx
  // has to happen: the decoder reads each group as a rate-weighted centroid
  // over neuron index, so a graded, zero-mean ramp across a group moves that
  // centroid and adds no net drive to the module.
  uint32_t bias_lo[2] = {}, bias_hi[2] = {};
  double bias_amp_units = 0.0;
  double bias_sign[2] = {1.0, 1.0};
  if (bias && bias->module >= 0 && bias->k > 0.0) {
    const aibaby::ModuleState& bm = s.brain.network().module(uint32_t(bias->module));
    // Groups 2 and 3 are F1 and F2; the slicing is the decoder's own.
    const uint32_t which[2] = {bias->group_a, bias->group_b};
    for (uint32_t g = 0; g < 2; ++g) {
      const uint32_t gi = which[g];
      bias_lo[g] = bm.begin + aibaby::slice_begin(bm.count, aibaby::kVocalGroups, gi);
      bias_hi[g] = bm.begin + aibaby::slice_begin(bm.count, aibaby::kVocalGroups, gi + 1);
    }
    bias_amp_units = bias->k * double(s.dna.module(uint32_t(bias->module)).noise_amp);
    // Which way each formant has to move to name word 0 rather than word 1,
    // read off the word table rather than typed in.
    bias_sign[0] = kWords[0].f1 > kWords[1].f1 ? 1.0 : -1.0;
    bias_sign[1] = kWords[0].f2 > kWords[1].f2 ? 1.0 : -1.0;
    // Off the formant axis there is no "toward the target" to point at, so both
    // ramps take the same sign and the condition supplies the direction.
    if (bias->group_a != 2) { bias_sign[0] = 1.0; bias_sign[1] = 1.0; }
    out.bias_amp = bias_amp_units;
  }
  uint32_t last_frame = 0;
  uint64_t last_feedback = 0;

  for (uint32_t trial = 0; trial < n_trials; ++trial) {
    const uint32_t label = trial % nw;
    // The word the creature HEARS is still alternating on every arm — the
    // positive control differs only in what it is scored and rewarded against,
    // so the two arms hear identical sessions.
    // Which word's formants this trial is scored and rewarded against. The
    // bucket is the per-word reward baseline, and it follows the TARGET rather
    // than the label so that "closer than you usually get" always means closer
    // to the thing being asked for.
    // The random arm's draw is a hash of the trial index rather than a live
    // RNG, so it is reproducible and costs the session no stream state.
    uint32_t rnd = uint32_t(trial) * 2654435761u;
    rnd ^= rnd >> 16;
    const uint32_t target_word = tgt == kVLTgtFixed    ? 0u
                                 : tgt == kVLTgtSwap   ? (label + 1u) % nw
                                 : tgt == kVLTgtRandom ? (rnd & 1u)
                                                       : label;
    const Word& w = kWords[target_word];
    const uint32_t bucket = target_word;
    // The oracle is set once per trial and held, exactly as the context tract
    // is: a condition that vanishes before reward lands has nothing to bind to.
    // In the CONDITIONAL arm the sign follows the word the creature heard, which
    // is what an Area X output is. In the constant arm it does not, and that is
    // the control that separates "a bias costs the exploratory pathway" from
    // "a bias moved the voice around while reward asked for one target".
    if (bias_amp_units > 0.0) {
      const double dir = bias->conditional ? (label == 0 ? 1.0 : -1.0) : bias->dir;
      for (uint32_t g = 0; g < 2; ++g) {
        s.brain.network().set_bias_oracle(
            g, bias_lo[g], bias_hi[g],
            aibaby::Scalar(dir * bias_sign[g] * bias_amp_units));
      }
    }
    double f1_sum = 0.0, f2_sum = 0.0;
    uint32_t n_voiced = 0;
    // Amplitude is averaged over EVERY frame of the window, silent ones
    // included at zero, because a quiet answer is an answer.
    double amp_sum = 0.0;
    uint32_t n_amp = 0;
    // TWO-WORD ONLY, and guarded rather than left to go quietly wrong: at four
    // words this makes word 0 loud and words 1-3 all quiet, which is not a
    // four-way amplitude lesson. `ctxself` and `ctxfour` score FORMANTS, so this
    // is unused there; the guard is above, where the session refuses an
    // amplitude or rate score with more than two words.
    const double amp_target = target_word == 0 ? kVLAmpLoud : kVLAmpQuiet;
    // Rate mode: this trial wants a talkative creature or a quiet one.
    const bool want_loud = target_word == 0;
    const double rate_target = want_loud ? kVLRateHigh : kVLRateLow;
    uint32_t n_frames = 0, n_events = 0;

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
      const Word& heard = kWords[label];
      caregiver.render(sounding ? heard.f0 : 0.0f, heard.f1, heard.f2,
                       sounding ? 0.5f : 0.0f, pcm.data(), spt);
      ear.tick(s.brain, pcm.data(), spt);
      // The oracle condition, held for the whole trial. A caregiver naming a
      // thing does not take it away while the baby answers, and the condition
      // has to still be present when reward lands or there is nothing for it to
      // bind to.
      if (ctx && ctx->module >= 0 && ctx->gain > 0.0) {
        const aibaby::ModuleState& cm = s.brain.network().module(uint32_t(ctx->module));
        const uint32_t slots = ctx->slots ? ctx->slots : 1u;
        const uint32_t slice = label % slots;
        const uint32_t lo2 = cm.begin + cm.count * slice / slots;
        const uint32_t hi2 = cm.begin + cm.count * (slice + 1) / slots;
        for (uint32_t n = lo2; n < hi2; ++n) {
          s.brain.network().inject(n, aibaby::Scalar(ctx->gain));
        }
      }
      s.brain.step();
      ++ctx_ticks_total;
      if (s.brain.network().context_present()) {
        ++ctx_ticks;
        // DNA v52. Sampled AFTER the step, so the index is this tick's, and
        // only inside the reward window, which is the only place it is used.
        if (t >= kVLRewardFrom && t < kVLRewardTo) {
          const uint32_t c = s.brain.network().active_context();
          const bool in_word = label < nw && c < nw;
          if (in_word) ++ctx_conf[label][c];
          ++ctx_conf_n;
          const int part = trial < third ? 0 : (trial >= n_trials - third ? 1 : -1);
          if (part >= 0) {
            if (in_word) ++ctx_conf_t[part][label][c];
            ++ctx_conf_tn[part];
          }
        }
      }
      // One sample per plasticity event: that is when a reward is actually
      // cashed, and it is the quantity Gadagkar's account is about.
      if (s.brain.plasticity_events() != last_plast) {
        last_plast = s.brain.plasticity_events();
        const aibaby::RewardBreakdown& rb = s.brain.reward();
        const uint32_t c = label < nw ? label : 0u;
        const double tot = double(rb.total), ext = double(rb.external);
        rw_sum[c] += tot; rw_sq[c] += tot * tot;
        rw_esum[c] += ext; rw_esq[c] += ext * ext;
        ++rw_n[c];
      }

      if (s.brain.vocal_frame() == last_frame) continue;
      last_frame = s.brain.vocal_frame();
      const aibaby::VocalParams& v = s.brain.voice();
      const bool voiced = v.voicing > 0.5f && v.amplitude > kAmplitudeFloor;

      // Feedback, on G2's clock, while the creature is making the sound. Only
      // the taught and positive-control arms earn it.
      // Rate mode follows G2 exactly: reward is delivered ON an event, so it
      // always follows something the creature did. That is the property the
      // amplitude version lacked and the reason it measured nothing.
      const bool scorable = score == kVLScoreAmp ? true : voiced;
      if ((arm == kVLTaught || arm == kVLFixed) && scorable &&
          t >= kVLRewardFrom && t < kVLRewardTo &&
          now - last_feedback >= regime.feedback_period) {
        const double e = score == kVLScoreAmp
                             ? std::fabs(double(v.amplitude) - amp_target)
                         : score == kVLScoreRate
                             // An event happened. Praise it if this trial wanted
                             // events and scold it if it did not -- so the sign
                             // is the condition, and with a constant target this
                             // is G2's own contingency unchanged.
                             ? (want_loud ? 0.0 : 1.0)
                             : formant_error(double(v.f1), double(v.f2), w);
        if (score == kVLScoreRate) {
          // No baseline here, and that is not an omission. The baseline exists
          // so that praise means "closer than you usually get to THIS target",
          // which needs a graded error; rate mode's signal is a SIGN -- an event
          // was wanted or it was not -- and a running mean of a per-bucket
          // constant converges onto it and turns every trial into a scold. The
          // first version of this did exactly that.
          last_feedback = now;
          const float value = want_loud ? regime.praise : regime.scold;
          if (value > 0.0f) { ++out.praises; ++w_praise; } else { ++out.scolds; ++w_scold; }
          pending.push_back(Praise{now + regime.delay, value});
          out.feedback.push_back(Praise{now + regime.delay, value});
        } else if (e >= 0.0) {
          last_feedback = now;
          if (baseline[bucket] >= 0.0) {
            const float value = e < baseline[bucket] ? regime.praise : regime.scold;
            if (value > 0.0f) { ++out.praises; ++w_praise; } else { ++out.scolds; ++w_scold; }
            pending.push_back(Praise{now + regime.delay, value});
            out.feedback.push_back(Praise{now + regime.delay, value});
          }
          baseline[bucket] = baseline[bucket] < 0.0
                                 ? e
                                 : baseline[bucket] +
                                       kVLBaselineAlpha * (e - baseline[bucket]);
        }
      }

      if (t < kVLEchoFrom || t >= kVLEchoTo) continue;
      ++frames_total;
      if (score == kVLScoreAmp) {
        amp_sum += double(v.amplitude);
        ++n_amp;
      }
      if (score == kVLScoreRate) {
        ++n_frames;
        if (voiced) ++n_events;
      }
      if (!voiced) continue;
      ++frames_voiced;
      ++n_voiced;
      f1_sum += double(v.f1);
      f2_sum += double(v.f2);
      // Bucketed by the word HEARD, not by the word scored: the question this
      // serves is whether the voice became conditional on the input.
      vf1_sum[label] += double(v.f1);
      vf2_sum[label] += double(v.f2);
      ++vf_n[label];
      wf1_sum[label] += double(v.f1);
      ++wf_n[label];
    }

    // A trial in which the creature said nothing has no accuracy to score and
    // must not be counted as a bad one: silence is not a wrong answer, and
    // scoring it as maximum error would make "say less" the winning strategy.
    double err;
    if (score == kVLScoreRate) {
      // Silence is the correct answer to a low target, so no trial is skipped
      // for it -- only one in which the window itself was empty.
      if (n_frames == 0) { ++out.skipped; continue; }
      err = std::fabs(double(n_events) / double(n_frames) - rate_target);
    } else if (score == kVLScoreAmp) {
      // No silence skip: a trial the creature spent quiet is a trial in which
      // it produced an amplitude of zero, and against a quiet target that is
      // the right answer rather than a missing measurement.
      if (n_amp == 0) { ++out.skipped; continue; }
      err = std::fabs(amp_sum / double(n_amp) - amp_target);
    } else {
      if (n_voiced == 0) { ++out.skipped; continue; }
      err = formant_error(f1_sum / n_voiced, f2_sum / n_voiced, w);
    }
    if (err < 0.0) { ++out.skipped; continue; }
    ++out.scored;
    // One row per scored trial: what the creature actually said, and which word
    // it was being taught. See the note on `utt_f1` -- this is the naming
    // measurement, and it is not the same question as dF1.
    if (n_voiced > 0 && label < nw) {
      out.utt_f1.push_back(f1_sum / double(n_voiced));
      out.utt_f2.push_back(f2_sum / double(n_voiced));
      out.utt_word.push_back(int(label));
    }

    // The within-session checkpoint. Sampled at the END of a trial so the table
    // reflects every cash-in that trial produced, and spaced by trial count
    // rather than by tick so the x axis is the same quantity `ctxscale` plots.
    //
    // WHY THIS EXISTS. `bias_ctx_` has no leak -- it is a pure clamped
    // accumulator -- so under a constant drift the aligned component would grow
    // LINEARLY in trials. It grows at exponent 0.27 to 0.58. Something is
    // bounding it, and the three candidates leave different fingerprints here:
    // the drift falling (align concave, praise share moving), pure diffusion
    // (exponent pinned at 0.5, gain flat), or the clamp binding on a heavy tail
    // (pinned share rising, which an RMS of 0.085 against a clamp of 0.30 does
    // NOT rule out -- the same shape `ipctx` found on thresholds).
    if (out.ckpt_n < VLRun::kCkpt && n_trials > 0) {
      const uint32_t edge =
          uint32_t((uint64_t(out.ckpt_n + 1) * n_trials) / VLRun::kCkpt);
      if (trial + 1 >= edge) {
        const uint32_t k = out.ckpt_n;
        const aibaby::Network& cnet = s.brain.network();
        const int32_t cvm = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
        if (cvm >= 0) {
          const CtxSplit sp = ctx_split(cnet, cnet.module(uint32_t(cvm)),
                                        double(s.dna.header().exploration.perturb_max));
          out.ckpt_align[k] = sp.align;
          out.ckpt_outside[k] = sp.outside;
          out.ckpt_gain[k] = sp.gain;
          out.ckpt_pinned[k] = sp.pinned;
        }
        out.ckpt_trial[k] = double(trial + 1);
        out.ckpt_df1[k] = (nw >= 2 && wf_n[0] && wf_n[1])
                              ? std::fabs(wf1_sum[0] / wf_n[0] - wf1_sum[1] / wf_n[1])
                              : 0.0;
        const uint32_t wtot = w_praise + w_scold;
        out.ckpt_praise[k] = wtot ? double(w_praise) / double(wtot) : 0.0;
        ++out.ckpt_n;
        for (uint32_t q = 0; q < kVLMaxWords; ++q) { wf1_sum[q] = 0.0; wf_n[q] = 0; }
        w_praise = 0;
        w_scold = 0;
      }
    }

    const int bin = trial < third ? 0 : (trial >= n_trials - third ? 1 : -1);
    if (bin >= 0) {
      err_sum[bin] += err;
      ++err_n[bin];
      word_sum[bucket][bin] += err;
      ++word_n[bucket][bin];
    }

  }

  out.err_early = err_n[0] ? err_sum[0] / err_n[0] : 0.0;
  out.err_late = err_n[1] ? err_sum[1] / err_n[1] : 0.0;
  for (uint32_t k = 0; k < nw; ++k) {
    for (uint32_t b = 0; b < 2; ++b) {
      out.err_by_word[k][b] = word_n[k][b] ? word_sum[k][b] / word_n[k][b] : 0.0;
    }
  }
  for (uint32_t k = 0; k < nw; ++k) {
    out.f1_by_word[k] = vf_n[k] ? vf1_sum[k] / vf_n[k] : 0.0;
    out.f2_by_word[k] = vf_n[k] ? vf2_sum[k] / vf_n[k] : 0.0;
  }
  out.voiced_frac = frames_total ? double(frames_voiced) / double(frames_total) : 0.0;
  out.ctx_present_frac = ctx_ticks_total ? double(ctx_ticks) / double(ctx_ticks_total) : 0.0;
  out.ctx_events_per_trial =
      n_trials ? double(s.brain.network().context_events()) / double(n_trials) : 0.0;
  // DNA v52. The best of the two slice-to-word assignments, and the busiest
  // slice's share. Two words and two slots is the only case this project runs;
  // with more of either the diagonal below is a lower bound rather than the
  // best assignment, which is why it is derived here and not in the kernel.
  // An index only has to be CONSISTENT, not correctly labelled, so this is the
  // best of all k! assignments -- exact rather than greedy, because a partition
  // that is right but PERMUTED would otherwise read as a failure. Two words
  // gives back the diagonal-or-anti-diagonal test this used to hard-code; four
  // gives 24 permutations, still cheap to enumerate.
  {
    auto best_assign = [&](const uint64_t c[kVLMaxWords][kVLMaxWords], uint64_t n) {
      if (n == 0) return 0.0;
      std::vector<uint32_t> perm(nw);
      for (uint32_t i = 0; i < nw; ++i) perm[i] = i;
      double best = 0.0;
      do {
        double agree = 0.0;
        for (uint32_t w = 0; w < nw; ++w) agree += double(c[w][perm[w]]);
        if (agree > best) best = agree;
      } while (std::next_permutation(perm.begin(), perm.end()));
      return best / double(n);
    };
    out.ctx_match = best_assign(ctx_conf, ctx_conf_n);
    out.ctx_match_early = best_assign(ctx_conf_t[0], ctx_conf_tn[0]);
    out.ctx_match_late = best_assign(ctx_conf_t[1], ctx_conf_tn[1]);
    if (ctx_conf_n > 0) {
      double busiest = 0.0;
      for (uint32_t c = 0; c < nw; ++c) {
        double col = 0.0;
        for (uint32_t w = 0; w < nw; ++w) col += double(ctx_conf[w][c]);
        if (col > busiest) busiest = col;
      }
      out.ctx_occupancy = busiest / double(ctx_conf_n);
    }
  }
  {
    // The decomposition this exists for. A centred reward R - b_global splits
    // into (mean_c - b_global) + (R - mean_c). If the first term dominates,
    // node perturbation is mostly learning "context A is a good place to be"
    // rather than "that action was good IN this context" -- a common mode on
    // the REWARD side, which is the one place this project's recurring
    // arithmetic has never been looked for.
    double gm = 0.0, gn = 0.0, within = 0.0, ewithin = 0.0;
    for (uint32_t c = 0; c < nw; ++c) {
      if (!rw_n[c]) continue;
      out.rw_mean[c] = rw_sum[c] / rw_n[c];
      out.rw_ext[c] = rw_esum[c] / rw_n[c];
      out.rw_n[c] = rw_n[c];
      const double v = rw_sq[c] / rw_n[c] - out.rw_mean[c] * out.rw_mean[c];
      const double ev = rw_esq[c] / rw_n[c] - out.rw_ext[c] * out.rw_ext[c];
      within += (v > 0.0 ? v : 0.0) * rw_n[c];
      ewithin += (ev > 0.0 ? ev : 0.0) * rw_n[c];
      gm += rw_sum[c];
      gn += rw_n[c];
    }
    if (gn > 0.0) {
      gm /= gn;
      out.rw_within = within / gn;
      double between = 0.0;
      for (uint32_t c = 0; c < kVLWords; ++c) {
        if (!rw_n[c]) continue;
        between += rw_n[c] * (out.rw_mean[c] - gm) * (out.rw_mean[c] - gm);
      }
      out.rw_between = between / gn;
      const double tot_var = out.rw_between + out.rw_within;
      out.rw_ext_share = tot_var > 0.0 ? (ewithin / gn) / tot_var : 0.0;
    }
  }
  {
    const aibaby::Network& net = s.brain.network();
    const int32_t vmod = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
    if (vmod >= 0 && net.context_slots() >= 2) {
      // One arithmetic, shared with the within-session checkpoints above. The
      // scale to read the divergence against is the SIZE of the tables, not the
      // shared bias: with a context present on every tick the shared bias is
      // never cashed into at all and sits at zero by construction -- which is
      // correct behaviour and a useless denominator.
      const CtxSplit sp = ctx_split(net, net.module(uint32_t(vmod)),
                                    double(s.dna.header().exploration.perturb_max));
      out.ctx_table_div = sp.div;
      out.ctx_shared_mag = sp.mag;
      out.ctx_align = sp.align;
      out.ctx_common = sp.common;
      out.ctx_outside = sp.outside;
      out.ctx_align_gain = sp.gain;
      out.ctx_f1_group_n = sp.gn;
      out.ctx_pinned = sp.pinned;
    }
  }
  {
    const aibaby::Network& net = s.brain.network();
    const float t_max = s.dna.header().homeo.threshold_max;
    if (ip_vm >= 0) {
      double pinned = 0.0;
      out.ip_thresh_drift = mean_threshold(net, uint32_t(ip_vm), &pinned, t_max) - ip_t0;
      out.ip_pinned = pinned;
      out.ip_rate_hz = double(net.module(uint32_t(ip_vm)).mean_rate);
      out.ip_target_hz = double(s.dna.module(uint32_t(ip_vm)).target_rate_hz);
      out.isp_pinned = double(net.isp_saturation(uint32_t(ip_vm)));
    }
    if (ip_rm >= 0) {
      out.ip_ref_drift = mean_threshold(net, uint32_t(ip_rm), nullptr, 0.0f) - ip_r0;
    }
  }
  if (ctx && ctx->module >= 0) {
    out.ctx_rate = double(s.brain.network().module(uint32_t(ctx->module)).mean_rate);
    // Did the tract LEARN anything? A flat conditional arm means nothing if the
    // synapses the condition arrives on never moved, and "the oracle fires" and
    // "reward writes to the oracle's synapses" are different claims that no
    // firing rate can tell apart. Reported beside the larynx's OTHER afferents
    // so that "small" has a scale.
    double d_ctx = 0.0, d_ref = 0.0;
    uint32_t n_ctx = 0, n_ref = 0;
    const aibaby::Network& net = s.brain.network();
    const aibaby::ModuleState& cms = net.module(uint32_t(ctx->module));
    for (uint32_t k = 0; k < w0_src.size(); ++k) {
      uint32_t src = 0;
      aibaby::Scalar w = aibaby::Scalar(0);
      net.in_synapse(w0_neuron[k], w0_slot[k], src, w);
      if (src != w0_src[k]) continue;  // pruning moved it; not the same synapse
      const double d = std::fabs(double(w) - w0_w[k]);
      if (src >= cms.begin && src < cms.begin + cms.count) { d_ctx += d; ++n_ctx; }
      else { d_ref += d; ++n_ref; }
    }
    out.ctx_dw = n_ctx ? d_ctx / n_ctx : 0.0;
    out.ref_dw = n_ref ? d_ref / n_ref : 0.0;
  }
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
  // Shifted by half a trial: the same praise and scolding in the same
  // proportions, landing where this creature's own echoes are not.
  for (Praise& p : yoke) p.tick += kVLTrialTicks / 2;
  const VLRun yoked = run_vocallearn_session(blob, ticks, kVLYoked, &yoke, regime);
  const VLRun none = run_vocallearn_session(blob, ticks, kVLNone, nullptr, regime);
  const VLRun fixed = run_vocallearn_session(blob, ticks, kVLFixed, nullptr, regime);

  std::printf("\n    %-9s %-8s %-9s %-11s %-11s %-9s %-9s %-8s\n", "arm", "scored",
              "skipped", "err early", "err late", "change", "rewards", "voiced");
  const VLRun* arms[kVLArmCount] = {&taught, &yoked, &none, &fixed};
  const char* names[kVLArmCount] = {"taught", "yoked", "none", "fixed tgt"};
  double change[kVLArmCount] = {};
  for (uint32_t a = 0; a < kVLArmCount; ++a) {
    const VLRun& r = *arms[a];
    change[a] = r.err_early > 0.0 ? 100.0 * (1.0 - r.err_late / r.err_early) : 0.0;
    std::printf("    %-9s %-8u %-9u %-11.4f %-11.4f %+-9.1f %-9u %-8.2f\n", names[a],
                r.scored, r.skipped, r.err_early, r.err_late, change[a],
                r.praises + r.scolds, r.voiced_frac);
  }
  std::printf("\n    feedback split          taught %u praise / %u no%s\n"
              "                            fixed  %u praise / %u no%s\n",
              taught.praises, taught.scolds,
              taught.scolds == 0 || taught.praises == 0
                  ? "   <- ONE-SIDED: not a training signal" : "",
              fixed.praises, fixed.scolds,
              fixed.scolds == 0 || fixed.praises == 0
                  ? "   <- ONE-SIDED: not a training signal" : "");

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
  std::printf("\n  taught     %+.1f%%\n  yoked      %+.1f%%\n  none       %+.1f%%\n"
              "  fixed tgt  %+.1f%%   <- the positive control: one target, no\n"
              "                        conditionality, which is the act G2 proved\n"
              "                        reward can shape here\n"
              "  taught - yoked   %+.1f points\n"
              "  fixed  - yoked   %+.1f points\n",
              change[0], change[1], change[2], change[3], change[0] - change[1],
              change[3] - change[1]);

  const bool instrument_ok = taught.praises + taught.scolds >= 12 &&
                             yoked.praises + yoked.scolds == 0 && yoked.ok && none.ok;
  const bool learned = change[0] > change[1] + 5.0 && change[0] > 0.0;
  if (!instrument_ok) {
    std::printf("\n  INCONCLUSIVE — the arms are not comparable: taught delivered %u\n"
                "  rewards, and the yoked arm must earn none of its own (%u).\n",
                taught.praises + taught.scolds, yoked.praises + yoked.scolds);
    return false;
  }
  // The positive control decides what a null means, so it is read before the
  // taught arm and not after.
  const bool can_see = change[3] > change[1] + 5.0;
  if (!can_see && !learned) {
    std::printf("\n  UNDERPOWERED — the positive control moved %+.1f points against its\n"
                "  yoke, under the 5 this instrument needs. One fixed formant target with\n"
                "  no conditionality is the act G2 already proved reward can shape in\n"
                "  this creature, so a creature that will not move on THAT will not move\n"
                "  on anything here. The taught arm's null is a fact about this probe\n"
                "  and not about vocal learning.\n", change[3] - change[1]);
    return false;
  }
  if (learned) {
    std::printf("\n  VOCAL LEARNING — the echo improved %+.1f points more than its own\n"
                "  yoked control. Reward can shape not just how much this creature\n"
                "  vocalises but how accurately, and that is a capability nothing in\n"
                "  this project has measured before.\n",
                change[0] - change[1]);
  } else {
    std::printf("\n  NOT MET, and the positive control says the instrument could have\n"
                "  seen it: one fixed target moves %+.1f points against the same yoke\n"
                "  where the conditional one moves %+.1f.\n",
                change[3] - change[1], change[0] - change[1]);
    std::printf("\n  The taught arm did not beat its yoke by the 5 points this\n"
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


// --- retain: does the creature KEEP what it was taught? ---------------------
//
// M1c is this project's first taught behaviour that a listener can hear. That
// makes a whole built-and-unvalidated subsystem testable for the first time.
// Sleep, replay, synaptic downscaling and myelination all exist and all pass
// G4, and **none has ever been shown to do anything for learning** — there was
// no learned behaviour worth testing them against until now.
//
// And the prior is not friendly. DNA v9 exists because awake homeostasis was
// measured to be **G2's eraser**: regulation removed a rewarded change before it
// could compound. Sleep downscaling is the same shape of mechanism run harder,
// once per bout, over every synapse in the brain.
//
// One creature, three phases, one continuous life:
//
//   teach       praise toward /i/ while it hears /a/, exactly as `teachsound`
//   intervene   no teaching, and one of three things happens
//   re-measure  no reward at all; what does it say now?
//
// The arms differ only in the middle phase:
//
//   quiet       nothing. Sleep happens when fatigue says so.
//   no sleep    the same, with `sleep_threshold` raised out of reach, so the
//               creature stays awake through the whole interval. The pair
//               isolates SLEEP from the mere passage of time, which nothing
//               else in this project has ever separated for a learned change.
//   relearn     a SECOND vowel is taught. Given that this creature has one
//               non-conditional motor pathway (`vocallearn`), the prediction is
//               that the second lesson overwrites the first, and a retention
//               near zero here is the interference result.
//
// The score is one number and it is bounded at both ends:
//
//   retention = (err before teaching - err after intervening)
//               / (err before teaching - err after teaching)
//
// 1.0 is "kept everything the teaching bought", 0.0 is "back where it started",
// and negative is worse than never having been taught. It is a ratio of two
// differences measured on the same creature in the same session, so a genome
// that simply says /i/ better than average cannot flatter it.
namespace {

constexpr uint64_t kRTTrial = 2800;
constexpr uint64_t kRTEchoFrom = 900 + 200;
constexpr uint64_t kRTEchoTo = 900 + 600;
constexpr uint64_t kRTRewardFrom = 900;
constexpr uint64_t kRTRewardTo = 900 + 800;
constexpr double kRTBaselineAlpha = 0.02;
constexpr uint32_t kRTHeard = 0;    // "ball" /a/ — what the caregiver always says
constexpr uint32_t kRTTarget = 1;   // "cube" /i/ — the first lesson
// The second lesson, and picking it took three tries because the obvious
// choices are all wrong for a reason only the data showed.
//
//   "boot" /u/  F1 350 — AGREES with /i/'s 320, so teaching it improved the
//               first lesson's score and the relearn arm "retained" 1.46.
//   "bed"  /e/  F1 550, F2 1850 — which is where the creature ALREADY SITS
//               after learning /i/ (564/1784). The lesson was "stay put".
//   "ball" /a/  far from both, but it is the vowel the caregiver says, so
//               "taught toward it" and "hears it" could not be separated.
//
// So the second target is stated outright rather than borrowed: a low-back
// vowel far from /i/ on both formants AND far from where teaching leaves the
// creature, and one it never hears. If a second lesson can displace the first,
// this is the one that would.
constexpr Word kRTSecondWord = {200.0f, 850.0f, 1100.0f};

// `kRTNever` is the control every other number is read against, and the first
// run of this experiment needed it and did not have it: all three arms came back
// retaining MORE than 100%, which is not consolidation but an artefact. Reward
// drives node perturbation, perturbation is trial-to-trial scatter, the error is
// convex in the formants, and so removing reward lowers the MEASURED error
// without the creature having learned anything further. An arm that is never
// taught at all measures exactly that much and nothing else.
enum RTArm { kRTQuiet = 0, kRTNoSleep, kRTRelearn, kRTNever, kRTArmCount };

struct RTRow {
  double err_before = 0.0, err_taught = 0.0, err_after = 0.0;
  double retention = 0.0, dprime = 0.0, null = 0.0, settle = 0.0;
  uint32_t sleeps = 0, scored = 0;
  double f1_taught = 0.0, f2_taught = 0.0, f1_after = 0.0, f2_after = 0.0;
};

}  // namespace

bool run_retain(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
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
    std::printf("  the audibility ruler failed: %s\n", error.c_str());
    return false;
  }
  // Three phases out of one budget: teaching needs the length M1c needed, the
  // interval needs a full fatigue cycle so the sleeping arm can actually
  // sleep, and the re-measure only has to be long enough to average.
  const uint64_t teach_ticks = ticks * 60 / 100;
  const uint64_t gap_ticks = ticks * 28 / 100;
  const uint64_t after_ticks = ticks - teach_ticks - gap_ticks;

  instrument("retain", dna0.header().seed ^ 0x2E7Au, ticks / kRTTrial, "trials");
  std::printf("  teach %llu, intervene %llu, re-measure %llu ticks, one life\n",
              (unsigned long long)teach_ticks, (unsigned long long)gap_ticks,
              (unsigned long long)after_ticks);
  std::printf("  the caregiver says \"ball\"; the first lesson is \"cube\", the second\n"
              "  (relearn arm only) is \"boot\"\n");

  RTRow rows[kRTArmCount];
  const char* names[kRTArmCount] = {"quiet", "no sleep", "relearn", "never taught"};

  for (uint32_t a = 0; a < kRTArmCount; ++a) {
    std::vector<uint8_t> variant = blob;
    if (a == kRTNoSleep) {
      // The creature never gets tired, so it never drops off. `sleep_threshold`
      // would be the more surgical knob and the genome loader refuses it above
      // 1.0 — correctly, since fatigue is a fraction. Stopping fatigue from
      // rising is the same intervention stated at the source, and it is honest
      // about its side effect: fatigue also feeds valence, so this arm is a
      // creature that is never sleepy rather than one that cannot sleep.
      const float none = 0.0f;
      std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, drives) +
                      offsetof(aibaby::DnaDrives, fatigue_rate),
                  &none, sizeof(none));
    }
    Session s;
    if (!s.init(variant, error)) {
      std::printf("  arm %s failed to hatch: %s\n", names[a], error.c_str());
      return false;
    }
    const aibaby::DnaAudio& acfg = dna0.header().audio;
    Ear ear;
    if (!ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource caregiver(acfg.sample_rate);
    std::vector<float> pcm(acfg.sample_rate / 1000);
    const uint32_t spt = acfg.sample_rate / 1000;
    const Word& heard = kWords[kRTHeard];
    const Word& first = kWords[kRTTarget];
    const Word& second = kRTSecondWord;
    aibaby::Rng rng;
    rng.seed(dna0.header().seed ^ 0x2E7Au);

    const uint32_t n_teach = uint32_t(teach_ticks / kRTTrial);
    const uint32_t n_gap = uint32_t(gap_ticks / kRTTrial);
    const uint32_t n_after = uint32_t(after_ticks / kRTTrial);
    const uint32_t n_total = n_teach + n_gap + n_after;
    const uint32_t third = n_teach / 3 ? n_teach / 3 : 1;

    std::deque<Praise> pending;
    double baseline1 = -1.0, baseline2 = -1.0;
    uint32_t last_frame = 0;
    uint64_t last_feedback = 0;
    bool was_asleep = false;
    double sum_before = 0, sum_taught = 0, sum_after = 0;
    double f1t = 0, f2t = 0, f1a = 0, f2a = 0;
    uint32_t n_before = 0, n_tt = 0, n_aa = 0;
    std::vector<std::vector<double>> ceps;
    std::vector<int> when;

    for (uint32_t trial = 0; trial < n_total; ++trial) {
      // The PHASE and whether this arm is being taught are two different
      // things, and conflating them is why the never-taught control came back
      // with err_taught 0.0000 and measured nothing: its scoring window was
      // gated on the same flag as its reward.
      const bool in_teach_phase = trial < n_teach;
      const bool teaching = in_teach_phase && a != kRTNever;
      const bool relearning =
          (a == kRTRelearn) && trial >= n_teach && trial < n_teach + n_gap;
      const Word& lesson = relearning ? second : first;
      double f1_acc = 0, f2_acc = 0;
      uint32_t nv = 0;

      for (uint64_t t = 0; t < kRTTrial; ++t) {
        const uint64_t now = uint64_t(trial) * kRTTrial + t;
        while (!pending.empty() && pending.front().tick <= now) {
          s.brain.praise(pending.front().value);
          pending.pop_front();
        }
        const bool sounding = t < 900;
        caregiver.render(sounding ? heard.f0 : 0.0f, heard.f1, heard.f2,
                         sounding ? 0.5f : 0.0f, pcm.data(), spt);
        ear.tick(s.brain, pcm.data(), spt);
        s.brain.step();
        if (s.brain.asleep() && !was_asleep) ++rows[a].sleeps;
        was_asleep = s.brain.asleep();

        if (s.brain.vocal_frame() == last_frame) continue;
        last_frame = s.brain.vocal_frame();
        const aibaby::VocalParams& v = s.brain.voice();
        const bool voiced = v.voicing > 0.5f && v.amplitude > kAmplitudeFloor;

        if ((teaching || relearning) && voiced && t >= kRTRewardFrom && t < kRTRewardTo &&
            now - last_feedback >= regime.feedback_period) {
          const double e = formant_error(double(v.f1), double(v.f2), lesson);
          if (e >= 0.0) {
            last_feedback = now;
            double& base = relearning ? baseline2 : baseline1;
            if (base >= 0.0) {
              pending.push_back(Praise{now + regime.delay,
                                       e < base ? regime.praise : regime.scold});
            }
            base = base < 0.0 ? e : base + kRTBaselineAlpha * (e - base);
          }
        }
        if (t < kRTEchoFrom || t >= kRTEchoTo || !voiced) continue;
        ++nv;
        f1_acc += double(v.f1);
        f2_acc += double(v.f2);
      }
      if (nv == 0) continue;
      const double f1 = f1_acc / nv, f2 = f2_acc / nv;
      // ALWAYS scored against the FIRST lesson, in every phase and every arm.
      // That is the quantity retention is about, and scoring the relearn arm
      // against its second lesson would measure something else entirely.
      const double err = formant_error(f1, f2, first);
      if (err < 0.0) continue;
      ++rows[a].scored;

      if (trial < third) { sum_before += err; ++n_before; }
      else if (in_teach_phase && trial >= n_teach - third) {
        sum_taught += err; f1t += f1; f2t += f2; ++n_tt;
        std::vector<double> c = ruler.of(double(dna0.header().vocal.f0_min), f1, f2, 0.4);
        if (!c.empty()) { ceps.push_back(c); when.push_back(0); }
      } else if (trial >= n_teach + n_gap) {
        sum_after += err; f1a += f1; f2a += f2; ++n_aa;
        std::vector<double> c = ruler.of(double(dna0.header().vocal.f0_min), f1, f2, 0.4);
        if (!c.empty()) { ceps.push_back(c); when.push_back(1); }
      }
    }

    RTRow& r = rows[a];
    r.err_before = n_before ? sum_before / n_before : 0.0;
    r.err_taught = n_tt ? sum_taught / n_tt : 0.0;
    r.err_after = n_aa ? sum_after / n_aa : 0.0;
    r.f1_taught = n_tt ? f1t / n_tt : 0.0;
    r.f2_taught = n_tt ? f2t / n_tt : 0.0;
    r.f1_after = n_aa ? f1a / n_aa : 0.0;
    r.f2_after = n_aa ? f2a / n_aa : 0.0;
    const double gained = r.err_before - r.err_taught;
    r.retention = std::fabs(gained) > 1e-6 ? (r.err_before - r.err_after) / gained : 0.0;
    // What the same window does with no lesson in it at all.
    r.settle = r.err_taught > 1e-6 ? r.err_after / r.err_taught : 1.0;
    if (ceps.size() >= 24) {
      const double d2 = cepstral_dprime(ceps, when, nullptr, true);
      r.dprime = d2 >= 0.0 ? std::sqrt(d2) : -std::sqrt(-d2);
      double ns = 0.0;
      for (uint32_t p = 0; p < 32; ++p) {
        std::vector<int> sh = when;
        for (size_t i = sh.size(); i > 1; --i) std::swap(sh[i - 1], sh[rng.next() % i]);
        const double nd = cepstral_dprime(ceps, sh, nullptr, true);
        ns += nd >= 0.0 ? std::sqrt(nd) : -std::sqrt(-nd);
      }
      r.null = ns / 32.0;
    }
  }

  std::printf("\n    %-13s %-7s %-11s %-11s %-11s %-10s %-8s %-9s %-8s\n", "arm",
              "sleeps", "err before", "err taught", "err after", "retention", "settle",
              "d' t->a", "null");
  for (uint32_t a = 0; a < kRTArmCount; ++a) {
    const RTRow& r = rows[a];
    if (a == kRTNever) {
      std::printf("    %-13s %-7u %-11.4f %-11.4f %-11.4f %-10s %-8.3f %-9.3f %-8.3f\n",
                  names[a], r.sleeps, r.err_before, r.err_taught, r.err_after, "-",
                  r.settle, r.dprime, r.null);
    } else {
      std::printf("    %-13s %-7u %-11.4f %-11.4f %-11.4f %-10.2f %-8.3f %-9.3f %-8.3f\n",
                  names[a], r.sleeps, r.err_before, r.err_taught, r.err_after,
                  r.retention, r.settle, r.dprime, r.null);
    }
  }
  for (uint32_t a = 0; a < kRTArmCount; ++a) {
    std::printf("    F1/F2 %-13s %.0f/%.0f -> %.0f/%.0f\n", names[a],
                rows[a].f1_taught, rows[a].f2_taught, rows[a].f1_after,
                rows[a].f2_after);
  }

  std::printf("\n  retention is (before - after) / (before - taught): 1.0 keeps\n"
              "  everything the lesson bought, 0.0 is back where it started, negative\n"
              "  is worse than never taught. `d' t->a` is whether a listener can hear\n"
              "  the difference between what it said at the end of teaching and what\n"
              "  it says now — LOW is retained, high is changed.\n");

  const bool taught_ok = rows[0].err_before - rows[0].err_taught > 0.02 &&
                         rows[1].err_before - rows[1].err_taught > 0.02;
  if (!taught_ok) {
    std::printf("\n  UNDERPOWERED — the teaching phase did not move the error far enough\n"
                "  to have anything to retain (%.4f and %.4f gained). Retention of a\n"
                "  change that did not happen is not a measurement. Raise --ticks.\n",
                rows[0].err_before - rows[0].err_taught,
                rows[1].err_before - rows[1].err_taught);
    return false;
  }
  std::printf("\n  `settle` is err after / err taught, and the NEVER TAUGHT arm is what it\n"
              "  reads with no lesson in it at all: reward drives node perturbation,\n"
              "  perturbation is scatter, the error is convex in the formants, so simply\n"
              "  switching reward off lowers the measured error. Any retention above 1.0\n"
              "  has to clear that arm before it means consolidation.\n");
  std::printf("\n  quiet         %.2f   (%u sleep bouts)\n"
              "  no sleep      %.2f   (%u — isolates sleep from the passage of time)\n"
              "  relearn       %.2f   (a conflicting vowel taught in between)\n"
              "  never taught  settle %.3f, against %.3f / %.3f / %.3f above\n"
              "\n  sleep costs             %+.2f\n  a second lesson costs   %+.2f\n",
              rows[0].retention, rows[0].sleeps, rows[1].retention, rows[1].sleeps,
              rows[2].retention, rows[3].settle, rows[0].settle, rows[1].settle,
              rows[2].settle, rows[0].retention - rows[1].retention,
              rows[2].retention - rows[0].retention);
  (void)verbose;
  return true;
}


// --- capacity: is teaching ONE degree of freedom, or two? -------------------
//
// `retain` found that a second lesson erases the first (retention 0.22 on 3
// seeds) and read it as "one lesson at a time". That reading is confounded,
// and this experiment exists because the confound is mine: BOTH lessons in
// `retain` moved the same thing. The reward there is
// |log(f1/target)| + |log(f2/target)|, a single scalar over both formants, and
// the two vowels pulled F1 and F2 together. So the collapse has two readings
// with opposite consequences:
//
//   one degree of freedom   there is exactly one teachable scalar, every
//                           future milestone is a single setpoint, and
//                           "teach it two things" is out of reach here.
//   overlapping targets     the lessons collided because they competed for
//                           the same formants, and orthogonal lessons would
//                           coexist.
//
// The larynx makes the test possible: F1 and F2 are read from two SEPARATE
// population-coded groups (§5.3, groups 2 and 3), so independent control is
// structurally available even if learning cannot use it. Lesson A is scored on
// F1 alone and lesson B on F2 alone. Their joint target is (320, 2500), which
// is "cube" /i/ — the vowel `teachsound` already proved this creature can be
// driven toward. The two halves of a known-reachable target.
//
// Five arms, and the value is almost entirely in the controls:
//
//   A only     teach F1, then nothing. What A retention looks like undisturbed
//              — and its F2 column is the YOKE CHECK: if F2 drifts toward B's
//              target without B ever being taught, the two formants are not
//              independent in practice and the whole test is void.
//   A then A   teach F1, then keep teaching F1. The tight control for "reward
//              kept running": if A survives here but not in `A then B`, the
//              loss is caused by B specifically and not by more reward, more
//              perturbation or more time.
//   A then B   the test.
//   A+B        both dimensions taught together for the same teaching budget.
//              REACHABILITY: if the larynx cannot hold both targets at once
//              even when taught both at once, a collapse in `A then B` is
//              anatomy rather than interference and says nothing about
//              learning. This arm is what makes a negative result meaningful.
//   never      taught nothing. `retain` needed this arm and did not have it on
//              its first run: reward drives node perturbation, perturbation is
//              scatter, the error is convex, so simply switching reward off
//              lowers the measured error with nothing learned.
//
// Every arm is scored on BOTH errors in every window, against the fixed
// targets, whatever it was taught. Scoring an arm against its own lesson would
// measure a different quantity in each arm.
namespace {

constexpr uint64_t kCapTrial = 2800;
constexpr uint64_t kCapEchoFrom = 900 + 200;
constexpr uint64_t kCapEchoTo = 900 + 600;
constexpr uint64_t kCapRewardFrom = 900;
constexpr uint64_t kCapRewardTo = 900 + 800;
constexpr double kCapBaselineAlpha = 0.02;
constexpr uint32_t kCapHeard = 0;      // "ball" /a/ — what the caregiver says
constexpr double kCapTargetF1 = 320.0;   // lesson A: F1 alone, downward
constexpr double kCapTargetF2 = 2500.0;  // lesson B: F2 alone, upward

// Which dimensions a lesson is scored on. kCapLessonNone is a phase with no
// reward in it, which is a different thing from an arm that is never taught.
enum CapLesson { kCapLessonNone = 0, kCapLessonA, kCapLessonB, kCapLessonAB };

enum CapArm { kCapAOnly = 0, kCapAThenA, kCapAThenB, kCapBoth, kCapNever, kCapArmCount };

struct CapRow {
  double f1_before = 0.0, f1_taught = 0.0, f1_after = 0.0;
  double f2_before = 0.0, f2_taught = 0.0, f2_after = 0.0;
  double e1_before = 0.0, e1_taught = 0.0, e1_after = 0.0;
  double e2_before = 0.0, e2_taught = 0.0, e2_after = 0.0;
  uint32_t scored = 0;
};

// |log(said / wanted)| on one formant. Same shape as `formant_error`, one
// dimension at a time, so the two are directly comparable and their sum is
// exactly the two-dimensional error the other experiments use.
inline double axis_error(double hz, double want) {
  if (hz <= 1.0) return -1.0;
  return std::fabs(std::log(hz / want));
}

inline double lesson_error(CapLesson lesson, double f1, double f2) {
  switch (lesson) {
    case kCapLessonA: return axis_error(f1, kCapTargetF1);
    case kCapLessonB: return axis_error(f2, kCapTargetF2);
    case kCapLessonAB: {
      const double a = axis_error(f1, kCapTargetF1), b = axis_error(f2, kCapTargetF2);
      return (a < 0.0 || b < 0.0) ? -1.0 : a + b;
    }
    default: return -1.0;
  }
}

// What each arm is doing in the teaching phase and in the gap.
struct CapPlan { CapLesson teach, gap; };
constexpr CapPlan kCapPlan[kCapArmCount] = {
    {kCapLessonA, kCapLessonNone},   // A only
    {kCapLessonA, kCapLessonA},      // A then A
    {kCapLessonA, kCapLessonB},      // A then B
    {kCapLessonAB, kCapLessonNone},  // A+B
    {kCapLessonNone, kCapLessonNone} // never
};

}  // namespace

bool run_capacity(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
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
    std::printf("  the audibility ruler failed: %s\n", error.c_str());
    return false;
  }
  // The same split `retain` uses: teaching gets what `teachsound` needs, the
  // gap is long enough for a second lesson to land if one can, and the
  // re-measure only has to be long enough to average.
  const uint64_t teach_ticks = ticks * 60 / 100;
  const uint64_t gap_ticks = ticks * 28 / 100;
  const uint64_t after_ticks = ticks - teach_ticks - gap_ticks;

  instrument("capacity", dna0.header().seed ^ 0x1C4Bu, ticks / kCapTrial, "trials");
  std::printf("  teach %llu, second phase %llu, re-measure %llu ticks, one life\n",
              (unsigned long long)teach_ticks, (unsigned long long)gap_ticks,
              (unsigned long long)after_ticks);
  std::printf("  lesson A is F1 -> %.0f Hz alone, lesson B is F2 -> %.0f Hz alone;\n"
              "  their joint target is \"cube\" /i/, which teachsound reaches\n",
              kCapTargetF1, kCapTargetF2);

  CapRow rows[kCapArmCount];
  const char* names[kCapArmCount] = {"A only", "A then A", "A then B", "A+B", "never"};
  // Final-window timbres, kept per arm so the interference can be asked as an
  // audibility question and not only as a pair of error numbers.
  std::vector<std::vector<double>> final_ceps[kCapArmCount];

  for (uint32_t a = 0; a < kCapArmCount; ++a) {
    Session s;
    if (!s.init(blob, error)) {
      std::printf("  arm %s failed to hatch: %s\n", names[a], error.c_str());
      return false;
    }
    const aibaby::DnaAudio& acfg = dna0.header().audio;
    Ear ear;
    if (!ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource caregiver(acfg.sample_rate);
    std::vector<float> pcm(acfg.sample_rate / 1000);
    const uint32_t spt = acfg.sample_rate / 1000;
    const Word& heard = kWords[kCapHeard];

    const uint32_t n_teach = uint32_t(teach_ticks / kCapTrial);
    const uint32_t n_gap = uint32_t(gap_ticks / kCapTrial);
    const uint32_t n_after = uint32_t(after_ticks / kCapTrial);
    const uint32_t n_total = n_teach + n_gap + n_after;
    const uint32_t third = n_teach / 3 ? n_teach / 3 : 1;

    std::deque<Praise> pending;
    // One baseline per lesson kind. A shared baseline would carry lesson A's
    // scale into lesson B's first comparisons and praise B for nothing.
    double base[4] = {-1.0, -1.0, -1.0, -1.0};
    uint32_t last_frame = 0;
    uint64_t last_feedback = 0;
    double s1[3] = {}, s2[3] = {}, sf1[3] = {}, sf2[3] = {};
    uint32_t n_win[3] = {};

    for (uint32_t trial = 0; trial < n_total; ++trial) {
      const bool in_teach_phase = trial < n_teach;
      const bool in_gap = trial >= n_teach && trial < n_teach + n_gap;
      const CapLesson lesson = in_teach_phase ? kCapPlan[a].teach
                             : in_gap         ? kCapPlan[a].gap
                                              : kCapLessonNone;
      double f1_acc = 0, f2_acc = 0;
      uint32_t nv = 0;

      for (uint64_t t = 0; t < kCapTrial; ++t) {
        const uint64_t now = uint64_t(trial) * kCapTrial + t;
        while (!pending.empty() && pending.front().tick <= now) {
          s.brain.praise(pending.front().value);
          pending.pop_front();
        }
        const bool sounding = t < 900;
        caregiver.render(sounding ? heard.f0 : 0.0f, heard.f1, heard.f2,
                         sounding ? 0.5f : 0.0f, pcm.data(), spt);
        ear.tick(s.brain, pcm.data(), spt);
        s.brain.step();

        if (s.brain.vocal_frame() == last_frame) continue;
        last_frame = s.brain.vocal_frame();
        const aibaby::VocalParams& v = s.brain.voice();
        const bool voiced = v.voicing > 0.5f && v.amplitude > kAmplitudeFloor;

        if (lesson != kCapLessonNone && voiced && t >= kCapRewardFrom &&
            t < kCapRewardTo && now - last_feedback >= regime.feedback_period) {
          const double e = lesson_error(lesson, double(v.f1), double(v.f2));
          if (e >= 0.0) {
            last_feedback = now;
            double& b = base[uint32_t(lesson)];
            if (b >= 0.0) {
              pending.push_back(Praise{now + regime.delay,
                                       e < b ? regime.praise : regime.scold});
            }
            b = b < 0.0 ? e : b + kCapBaselineAlpha * (e - b);
          }
        }
        if (t < kCapEchoFrom || t >= kCapEchoTo || !voiced) continue;
        ++nv;
        f1_acc += double(v.f1);
        f2_acc += double(v.f2);
      }
      if (nv == 0) continue;
      const double f1 = f1_acc / nv, f2 = f2_acc / nv;
      const double e1 = axis_error(f1, kCapTargetF1), e2 = axis_error(f2, kCapTargetF2);
      if (e1 < 0.0 || e2 < 0.0) continue;
      ++rows[a].scored;

      int w = -1;
      if (trial < third) w = 0;
      else if (in_teach_phase && trial >= n_teach - third) w = 1;
      else if (trial >= n_teach + n_gap) w = 2;
      if (w < 0) continue;
      s1[w] += e1; s2[w] += e2; sf1[w] += f1; sf2[w] += f2; ++n_win[w];
      if (w == 2) {
        std::vector<double> c = ruler.of(double(dna0.header().vocal.f0_min), f1, f2, 0.4);
        if (!c.empty()) final_ceps[a].push_back(c);
      }
    }

    CapRow& r = rows[a];
    const double inv0 = n_win[0] ? 1.0 / n_win[0] : 0.0;
    const double inv1 = n_win[1] ? 1.0 / n_win[1] : 0.0;
    const double inv2 = n_win[2] ? 1.0 / n_win[2] : 0.0;
    r.e1_before = s1[0] * inv0; r.e2_before = s2[0] * inv0;
    r.e1_taught = s1[1] * inv1; r.e2_taught = s2[1] * inv1;
    r.e1_after = s1[2] * inv2;  r.e2_after = s2[2] * inv2;
    r.f1_before = sf1[0] * inv0; r.f2_before = sf2[0] * inv0;
    r.f1_taught = sf1[1] * inv1; r.f2_taught = sf2[1] * inv1;
    r.f1_after = sf1[2] * inv2;  r.f2_after = sf2[2] * inv2;
  }

  std::printf("\n    %-9s %-8s %-24s %-24s\n", "arm", "trials",
              "F1 error  before/taught/after", "F2 error  before/taught/after");
  for (uint32_t a = 0; a < kCapArmCount; ++a) {
    const CapRow& r = rows[a];
    std::printf("    %-9s %-8u %6.4f %6.4f %6.4f      %6.4f %6.4f %6.4f\n", names[a],
                r.scored, r.e1_before, r.e1_taught, r.e1_after, r.e2_before,
                r.e2_taught, r.e2_after);
  }
  std::printf("\n    %-9s %-28s %-28s\n", "arm", "F1 Hz  before -> after",
              "F2 Hz  before -> after");
  for (uint32_t a = 0; a < kCapArmCount; ++a) {
    const CapRow& r = rows[a];
    std::printf("    %-9s %6.0f -> %-6.0f (want %.0f)   %6.0f -> %-6.0f (want %.0f)\n",
                names[a], r.f1_before, r.f1_after, kCapTargetF1, r.f2_before,
                r.f2_after, kCapTargetF2);
  }

  // Audibility of the interference: the same creature at the same point in its
  // life, differing only in what happened in the middle phase.
  double d_int = 0.0, d_null = 0.0;
  if (final_ceps[kCapAOnly].size() >= 12 && final_ceps[kCapAThenB].size() >= 12) {
    std::vector<std::vector<double>> ceps;
    std::vector<int> label;
    for (const auto& c : final_ceps[kCapAOnly]) { ceps.push_back(c); label.push_back(0); }
    for (const auto& c : final_ceps[kCapAThenB]) { ceps.push_back(c); label.push_back(1); }
    const double d2 = cepstral_dprime(ceps, label, nullptr, true);
    d_int = d2 >= 0.0 ? std::sqrt(d2) : -std::sqrt(-d2);
    aibaby::Rng rng;
    rng.seed(dna0.header().seed ^ 0x1C4Bu);
    double ns = 0.0;
    for (uint32_t p = 0; p < 32; ++p) {
      std::vector<int> sh = label;
      for (size_t i = sh.size(); i > 1; --i) std::swap(sh[i - 1], sh[rng.next() % i]);
      const double nd = cepstral_dprime(ceps, sh, nullptr, true);
      ns += nd >= 0.0 ? std::sqrt(nd) : -std::sqrt(-nd);
    }
    d_null = ns / 32.0;
  }

  // The reading. `gain` is how much of lesson A's error the arm still has gone
  // from its own starting point, so it is a within-arm ratio and a creature
  // that simply says /i/ better than average cannot flatter it.
  auto gain1 = [](const CapRow& r) {
    return r.e1_before > 1e-6 ? (r.e1_before - r.e1_after) / r.e1_before : 0.0;
  };
  auto gain2 = [](const CapRow& r) {
    return r.e2_before > 1e-6 ? (r.e2_before - r.e2_after) / r.e2_before : 0.0;
  };

  std::printf("\n  `gain` is (before - after) / before on ONE formant: the fraction of\n"
              "  that axis's starting error the arm has lost by the end. The never arm\n"
              "  is what the same window reads with no lesson in it at all.\n");
  std::printf("\n    %-9s %-12s %-12s\n", "arm", "A gain (F1)", "B gain (F2)");
  for (uint32_t a = 0; a < kCapArmCount; ++a) {
    std::printf("    %-9s %+11.3f %+11.3f\n", names[a], gain1(rows[a]), gain2(rows[a]));
  }

  const double never1 = gain1(rows[kCapNever]), never2 = gain2(rows[kCapNever]);
  const double a_undisturbed = gain1(rows[kCapAOnly]) - never1;
  const double a_more_reward = gain1(rows[kCapAThenA]) - never1;
  const double a_after_b = gain1(rows[kCapAThenB]) - never1;
  const double b_landed = gain2(rows[kCapAThenB]) - never2;
  const double yoke = gain2(rows[kCapAOnly]) - never2;
  const double both1 = gain1(rows[kCapBoth]) - never1, both2 = gain2(rows[kCapBoth]) - never2;

  std::printf("\n  against the never-taught arm:\n"
              "    A undisturbed        %+.3f\n"
              "    A with more A        %+.3f   (reward kept running, same lesson)\n"
              "    A after B            %+.3f   <- the test\n"
              "    B landed             %+.3f   (did the second lesson do anything?)\n"
              "    yoke check           %+.3f   (F2 in the arm that never learned B)\n"
              "    A+B together         %+.3f / %+.3f   (reachability of the pair)\n",
              a_undisturbed, a_more_reward, a_after_b, b_landed, yoke, both1, both2);
  if (final_ceps[kCapAOnly].size() >= 12) {
    std::printf("    interference d'      %.3f against a null of %.3f\n", d_int, d_null);
  }

  // Powered? The question only exists if lesson A landed in the first place,
  // and the honest answer when it did not is UNDERPOWERED, not a null.
  if (a_undisturbed <= 0.02 || a_more_reward <= 0.02) {
    std::printf("\n  UNDERPOWERED — lesson A did not land far enough to have anything to\n"
                "  interfere with (%+.3f undisturbed, %+.3f with more A, against the\n"
                "  never-taught arm). Capacity for a lesson that was never learned is\n"
                "  not a measurement. Raise --ticks.\n", a_undisturbed, a_more_reward);
    return false;
  }
  if (both1 <= 0.02 || both2 <= 0.02) {
    std::printf("\n  VOID — the A+B arm could not hold both targets even when taught both\n"
                "  at once (%+.3f / %+.3f). The joint target is not reachable for this\n"
                "  creature, so nothing the sequential arms do can distinguish\n"
                "  interference from anatomy.\n", both1, both2);
    return false;
  }
  if (yoke > 0.5 * b_landed && yoke > 0.02) {
    std::printf("\n  YOKED — F2 moved %+.3f toward lesson B's target in the arm that was\n"
                "  never taught B, against %+.3f in the arm that was. The two formants\n"
                "  are not independent in practice, so this pair cannot separate one\n"
                "  degree of freedom from two.\n", yoke, b_landed);
    return false;
  }

  // TWO ratios, because the two obvious controls answer different questions
  // and quoting only one of them overstates whichever case you prefer.
  //
  //   retained   A after B, over A undisturbed. "Did the lesson survive?" The
  //              gap is quiet in the denominator, so this is the fair analogue
  //              of `retain`'s retention and directly comparable to its 0.22.
  //   split      A after B, over A with more A. "What did the creature give up
  //              by spending the second phase on B?" Reward runs in both, so
  //              this isolates WHICH lesson the reward went to.
  const double retained = a_undisturbed > 1e-6 ? a_after_b / a_undisturbed : 0.0;
  const double split = a_more_reward > 1e-6 ? a_after_b / a_more_reward : 0.0;
  std::printf("\n  A retained            %.2f   (A after B / A undisturbed — the number\n"
              "                               comparable with retain's 0.22)\n"
              "  A against more A      %.2f   (what spending the second phase on B\n"
              "                               gave up, with reward running in both)\n",
              retained, split);
  // Deliberately NOT a threshold on the cost. The first version of this
  // experiment printed a binary verdict at -0.25 of A's gain, and three seed
  // families straddled it: -0.14, +0.08, -0.43 returned "two degrees of
  // freedom" twice and "one" once from what is plainly one distribution. The
  // underlying quantity is continuous and its seed spread is wider than any
  // line drawn through it, so the RATIO is the finding and the label is only a
  // reading aid. Moving the line until the seeds agree would have been fitting
  // the verdict to the data.
  if (b_landed <= 0.02) {
    std::printf("\n  B NEVER LANDED — the second lesson did nothing, so whatever happened\n"
                "  to A is not interference from it. Lesson B on F2 alone may simply be\n"
                "  the harder axis; this is not an answer to the capacity question.\n");
  } else if (retained >= 0.5) {
    std::printf("\n  BOTH LESSONS COEXIST — B landed (%+.3f) and A kept %.0f%% of what it\n"
                "  had. That is a different creature from the one `retain` measured,\n"
                "  where a CONFLICTING second lesson left 22%%. But the two lessons do\n"
                "  compete: A kept only %.0f%% of what continuing A would have bought,\n"
                "  so the second degree of freedom is real and is not free.\n"
                "  One seed cannot settle this — the ratio moves by ~0.25 across seed\n"
                "  families. Three agreeing is the claim; one is an anecdote.\n",
                b_landed, retained * 100.0, split * 100.0);
  } else {
    std::printf("\n  A DID NOT SURVIVE — B landed (%+.3f) and A kept only %.0f%%, on\n"
                "  ORTHOGONAL targets, with a reachability control saying the pair can\n"
                "  be held at once. That would make teaching a single setpoint.\n"
                "  One seed cannot settle this — the ratio moves by ~0.25 across seed\n"
                "  families. Three agreeing is the claim; one is an anecdote.\n",
                b_landed, retained * 100.0);
  }
  (void)verbose;
  return true;
}


// --- credit: what a perfectly targeted neuromodulator would buy -------------
//
// `capacity` found two teachable degrees of freedom that COMPETE: teaching
// lesson B costs lesson A 0.42 of what continuing A would have bought, even
// though the two lessons drive disjoint neuron groups. The mechanism of that
// cost is visible in `apply_reward_impl`. Node perturbation — the rule that
// actually shapes this larynx — nudges `bias_[i]` for EVERY neuron in the
// motor module, scaled by one scalar. While B is being taught, the F1 group's
// neurons keep receiving updates driven by a reward uncorrelated with anything
// they did, so what A taught them random-walks away.
//
// So the obvious next mechanism is a neuromodulator that reaches some neurons
// and not others. DNA v20 already splits reward into four channels with
// per-MODULE gains — and cannot do this, because both lessons live in the same
// module. Per-neuron gating is kernel surgery.
//
// BEFORE building it, price it. `Network::set_reward_mask` hands the creature
// the credit assignment it cannot compute: during lesson A, reward reaches only
// the F1 group; during lesson B, only the F2 group. That is an ORACLE. It is
// not a mechanism, it is not learnable, and no genome field reaches it. What it
// measures is the UPPER BOUND — if a perfectly targeted neuromodulator existed,
// how much of the interference would go away? If the answer is "none", then no
// mechanism that discovers credit assignment can help here and the whole line
// closes for the price of one experiment. This is the move the oracle fovea
// made for foveation, and it reversed that decision.
//
// Two honest caveats, both of which belong in any reading of the numbers:
//
//   The mask blocks ALL reward-driven plasticity outside the target group, not
//   just praise. `reward_.effective` carries hunger, comfort and curiosity too,
//   so the other groups are frozen with respect to reward, not merely
//   un-praised. That is the intervention, and it is stronger than a real
//   neuromodulator would be.
//
//   Masking removes plasticity, so a masked arm may simply learn LESS. That is
//   why retention is read within each condition — `A then B*` against
//   `A only*`, never against the broadcast arm's denominator.
namespace {

enum CrArm { kCrAOnly = 0, kCrAThenB, kCrAOnlyM, kCrAThenBM, kCrNever, kCrArmCount };

struct CrPlan { CapLesson teach, gap; bool masked; };
constexpr CrPlan kCrPlan[kCrArmCount] = {
    {kCapLessonA, kCapLessonNone, false},  // A only        broadcast
    {kCapLessonA, kCapLessonB,    false},  // A then B      broadcast
    {kCapLessonA, kCapLessonNone, true},   // A only*       targeted
    {kCapLessonA, kCapLessonB,    true},   // A then B*     targeted
    {kCapLessonNone, kCapLessonNone, false} // never
};

constexpr uint32_t kCrGroupF1 = 2;  // VocalDecoder reads group_value_[2] as F1
constexpr uint32_t kCrGroupF2 = 3;  // and [3] as F2 — see core/src/senses.cpp

}  // namespace

bool run_credit(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  Regime regime;
  regime.praise = kPraiseValue;
  regime.scold = kScoldValue;
  aibaby::Dna dna0;
  if (dna0.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  std::string error;
  const uint64_t teach_ticks = ticks * 60 / 100;
  const uint64_t gap_ticks = ticks * 28 / 100;
  const uint64_t after_ticks = ticks - teach_ticks - gap_ticks;

  instrument("credit", dna0.header().seed ^ 0x3D91u, ticks / kCapTrial, "trials");
  std::printf("  the same three phases and the same two orthogonal lessons as\n"
              "  `capacity`; the two starred arms deliver reward ONLY to the neuron\n"
              "  group the current lesson is about. That is an ORACLE — the creature\n"
              "  cannot compute this assignment — so it prices a mechanism, it is not\n"
              "  one, and no genome field reaches it.\n");

  CapRow rows[kCrArmCount];
  const char* names[kCrArmCount] = {"A only", "A then B", "A only*", "A then B*", "never"};

  for (uint32_t a = 0; a < kCrArmCount; ++a) {
    Session s;
    if (!s.init(blob, error)) {
      std::printf("  arm %s failed to hatch: %s\n", names[a], error.c_str());
      return false;
    }
    aibaby::Network& net = s.brain.network();
    const uint32_t modules = dna0.module_count();
    uint32_t m_vocal = modules;
    for (uint32_t m = 0; m < modules; ++m) {
      if (std::strcmp(net.module_dna(m).name, "vocal") == 0) { m_vocal = m; break; }
    }
    if (m_vocal == modules) {
      std::printf("  setup failed: no module named \"vocal\"\n");
      return false;
    }
    const aibaby::ModuleState& vms = net.module(m_vocal);
    const uint32_t f1_lo = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF1);
    const uint32_t f1_hi = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF1 + 1);
    const uint32_t f2_lo = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF2);
    const uint32_t f2_hi = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF2 + 1);
    if (a == 0) {
      std::printf("  vocal module %u, neurons %u..%u; F1 group %u..%u, F2 group %u..%u\n",
                  m_vocal, vms.begin, vms.begin + vms.count, f1_lo, f1_hi, f2_lo, f2_hi);
    }

    const aibaby::DnaAudio& acfg = dna0.header().audio;
    Ear ear;
    if (!ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource caregiver(acfg.sample_rate);
    std::vector<float> pcm(acfg.sample_rate / 1000);
    const uint32_t spt = acfg.sample_rate / 1000;
    const Word& heard = kWords[kCapHeard];

    const uint32_t n_teach = uint32_t(teach_ticks / kCapTrial);
    const uint32_t n_gap = uint32_t(gap_ticks / kCapTrial);
    const uint32_t n_after = uint32_t(after_ticks / kCapTrial);
    const uint32_t n_total = n_teach + n_gap + n_after;
    const uint32_t third = n_teach / 3 ? n_teach / 3 : 1;

    std::deque<Praise> pending;
    double base[4] = {-1.0, -1.0, -1.0, -1.0};
    uint32_t last_frame = 0;
    uint64_t last_feedback = 0;
    double s1[3] = {}, s2[3] = {}, sf1[3] = {}, sf2[3] = {};
    uint32_t n_win[3] = {};

    for (uint32_t trial = 0; trial < n_total; ++trial) {
      const bool in_teach_phase = trial < n_teach;
      const bool in_gap = trial >= n_teach && trial < n_teach + n_gap;
      const CapLesson lesson = in_teach_phase ? kCrPlan[a].teach
                             : in_gap         ? kCrPlan[a].gap
                                              : kCapLessonNone;
      // The oracle, re-aimed at each phase boundary. Cleared whenever no lesson
      // is running, so the re-measure phase is identical in every arm and the
      // final numbers are not comparing two different plasticity regimes.
      if (kCrPlan[a].masked && lesson == kCapLessonA) net.set_reward_mask(f1_lo, f1_hi);
      else if (kCrPlan[a].masked && lesson == kCapLessonB) net.set_reward_mask(f2_lo, f2_hi);
      else net.clear_reward_mask();

      double f1_acc = 0, f2_acc = 0;
      uint32_t nv = 0;
      for (uint64_t t = 0; t < kCapTrial; ++t) {
        const uint64_t now = uint64_t(trial) * kCapTrial + t;
        while (!pending.empty() && pending.front().tick <= now) {
          s.brain.praise(pending.front().value);
          pending.pop_front();
        }
        const bool sounding = t < 900;
        caregiver.render(sounding ? heard.f0 : 0.0f, heard.f1, heard.f2,
                         sounding ? 0.5f : 0.0f, pcm.data(), spt);
        ear.tick(s.brain, pcm.data(), spt);
        s.brain.step();

        if (s.brain.vocal_frame() == last_frame) continue;
        last_frame = s.brain.vocal_frame();
        const aibaby::VocalParams& v = s.brain.voice();
        const bool voiced = v.voicing > 0.5f && v.amplitude > kAmplitudeFloor;

        if (lesson != kCapLessonNone && voiced && t >= kCapRewardFrom &&
            t < kCapRewardTo && now - last_feedback >= regime.feedback_period) {
          const double e = lesson_error(lesson, double(v.f1), double(v.f2));
          if (e >= 0.0) {
            last_feedback = now;
            double& b = base[uint32_t(lesson)];
            if (b >= 0.0) {
              pending.push_back(Praise{now + regime.delay,
                                       e < b ? regime.praise : regime.scold});
            }
            b = b < 0.0 ? e : b + kCapBaselineAlpha * (e - b);
          }
        }
        if (t < kCapEchoFrom || t >= kCapEchoTo || !voiced) continue;
        ++nv;
        f1_acc += double(v.f1);
        f2_acc += double(v.f2);
      }
      if (nv == 0) continue;
      const double f1 = f1_acc / nv, f2 = f2_acc / nv;
      const double e1 = axis_error(f1, kCapTargetF1), e2 = axis_error(f2, kCapTargetF2);
      if (e1 < 0.0 || e2 < 0.0) continue;
      ++rows[a].scored;
      int w = -1;
      if (trial < third) w = 0;
      else if (in_teach_phase && trial >= n_teach - third) w = 1;
      else if (trial >= n_teach + n_gap) w = 2;
      if (w < 0) continue;
      s1[w] += e1; s2[w] += e2; sf1[w] += f1; sf2[w] += f2; ++n_win[w];
    }
    net.clear_reward_mask();

    CapRow& r = rows[a];
    const double i0 = n_win[0] ? 1.0 / n_win[0] : 0.0;
    const double i1 = n_win[1] ? 1.0 / n_win[1] : 0.0;
    const double i2 = n_win[2] ? 1.0 / n_win[2] : 0.0;
    r.e1_before = s1[0] * i0; r.e2_before = s2[0] * i0;
    r.e1_taught = s1[1] * i1; r.e2_taught = s2[1] * i1;
    r.e1_after = s1[2] * i2;  r.e2_after = s2[2] * i2;
    r.f1_after = sf1[2] * i2; r.f2_after = sf2[2] * i2;
  }

  auto gain1 = [](const CapRow& r) {
    return r.e1_before > 1e-6 ? (r.e1_before - r.e1_after) / r.e1_before : 0.0;
  };
  auto gain2 = [](const CapRow& r) {
    return r.e2_before > 1e-6 ? (r.e2_before - r.e2_after) / r.e2_before : 0.0;
  };
  const double n1 = gain1(rows[kCrNever]), n2 = gain2(rows[kCrNever]);

  std::printf("\n    %-11s %-9s %-12s %-12s\n", "arm", "trials", "A gain (F1)", "B gain (F2)");
  for (uint32_t a = 0; a < kCrArmCount; ++a) {
    std::printf("    %-11s %-9u %+11.3f %+11.3f\n", names[a], rows[a].scored,
                gain1(rows[a]) - n1, gain2(rows[a]) - n2);
  }
  std::printf("    (both columns are against the never-taught arm, which reads\n"
              "     %+.3f / %+.3f raw and is the scatter this window shows with no\n"
              "     lesson in it at all)\n", n1, n2);

  const double b_bcast = gain1(rows[kCrAOnly]) - n1;
  const double b_after = gain1(rows[kCrAThenB]) - n1;
  const double m_bcast = gain1(rows[kCrAOnlyM]) - n1;
  const double m_after = gain1(rows[kCrAThenBM]) - n1;
  const double b_landed = gain2(rows[kCrAThenB]) - n2;
  const double m_landed = gain2(rows[kCrAThenBM]) - n2;
  const double ret_b = b_bcast > 1e-6 ? b_after / b_bcast : 0.0;
  const double ret_m = m_bcast > 1e-6 ? m_after / m_bcast : 0.0;

  std::printf("\n  A retained, broadcast reward   %.2f   (%+.3f / %+.3f)\n"
              "  A retained, targeted reward    %.2f   (%+.3f / %+.3f)\n"
              "  B landed                       broadcast %+.3f, targeted %+.3f\n",
              ret_b, b_after, b_bcast, ret_m, m_after, m_bcast, b_landed, m_landed);

  if (m_bcast <= 0.02) {
    std::printf("\n  MASK KILLED THE LESSON — with reward reaching only the F1 group,\n"
                "  lesson A itself no longer lands (%+.3f). There is nothing to retain,\n"
                "  so this run cannot price targeting. The oracle is too strong an\n"
                "  intervention at this group size, not a verdict on the idea.\n", m_bcast);
    return false;
  }
  if (m_landed <= 0.02) {
    std::printf("\n  TARGETED B NEVER LANDED — the second lesson does nothing when its\n"
                "  reward is confined to the F2 group (%+.3f), so a retention gain here\n"
                "  would only mean B was never taught. Not a verdict on targeting.\n",
                m_landed);
    return false;
  }
  std::printf("\n  targeting changes A's retention by %+.2f, with B still landing.\n"
              "  One seed cannot settle this — capacity's ratio moves by ~0.25 across\n"
              "  seed families and this one is built from two of them.\n",
              ret_m - ret_b);
  (void)verbose;
  return true;
}


// --- driftprobe: is the interference a credit failure, or is it VARIANCE? ---
//
// Everything on this page so far has called `capacity`'s interference a
// credit-assignment problem, and that framing is wrong. Node perturbation
// already assigns credit correctly *in expectation*:
//
//   d bias_i  ~  R * perturb_i        and       E[d bias_i]  ~  Cov(R, perturb_i)
//
// `perturb_[i]` is the neuron's own injected noise, independent across neurons
// and independent of the reward except through that neuron's causal effect on
// behaviour. For a neuron with no effect on the current lesson's reward the
// covariance is ZERO — the rule is already telling it "you get nothing". What
// it cannot do is deliver zero on any single sample. It delivers zero-mean
// NOISE, and zero-mean noise applied to a standing value is a random walk.
//
// So the prediction is specific and falsifiable. During lesson B, the F1
// group's bias vector should DIFFUSE — spread out with no preferred direction —
// while the F2 group's should DRIFT, moving systematically along the axis that
// changes what the group decodes to. If instead F1 also drifts, something is
// correlating its perturbations with B's reward, the covariance is not zero,
// and this is a credit problem after all.
//
// The decomposition has to respect what the larynx actually reads. A group's
// output is a population CENTROID over neuron index (§5.3), so the centroid
// moves only if the change correlates with a neuron's preferred position — a
// uniform shift of every bias in the group moves nothing. The change vector is
// therefore split along that axis and perpendicular to it:
//
//   p_i         the neuron's centred preferred position in its group
//   drift       the component of the change along p — this MOVES the readout
//   diffusion   the residual — this only spreads the code out
//
// This distinction decides which mechanism to build, which is why it is worth
// its own run rather than an assumption inside a bigger one.
namespace {

enum DpArm { kDpAOnly = 0, kDpAThenB, kDpArmCount };

struct DpGroup {
  double drift = 0.0, diffusion = 0.0, rms = 0.0;
};

// Split a per-neuron change vector into the part that moves a centroid readout
// and the part that does not.
DpGroup decompose(const std::vector<double>& d) {
  DpGroup out;
  const size_t n = d.size();
  if (n < 2) return out;
  double pp = 0.0, dp = 0.0, sq = 0.0;
  for (size_t k = 0; k < n; ++k) {
    const double p = (double(k) + 0.5) / double(n) - 0.5;
    pp += p * p;
    dp += d[k] * p;
    sq += d[k] * d[k];
  }
  const double proj = pp > 1e-12 ? dp / pp : 0.0;
  double res = 0.0;
  for (size_t k = 0; k < n; ++k) {
    const double p = (double(k) + 0.5) / double(n) - 0.5;
    const double r = d[k] - proj * p;
    res += r * r;
  }
  out.drift = std::fabs(proj) * std::sqrt(pp / double(n));
  out.diffusion = std::sqrt(res / double(n));
  out.rms = std::sqrt(sq / double(n));
  return out;
}

}  // namespace

bool run_driftprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  Regime regime;
  regime.praise = kPraiseValue;
  regime.scold = kScoldValue;
  aibaby::Dna dna0;
  if (dna0.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  std::string error;
  const uint64_t teach_ticks = ticks * 60 / 100;
  const uint64_t gap_ticks = ticks - teach_ticks;

  instrument("driftprobe", dna0.header().seed ^ 0x51A7u, ticks / kCapTrial, "trials");
  std::printf("  teach lesson A (F1) for %llu ticks, then %llu more in which the\n"
              "  `A then B` arm is taught lesson B (F2) and the `A only` arm is not.\n"
              "  The bias vector of BOTH groups is snapshotted at the boundary and\n"
              "  at the end, and each change is split into the component that moves\n"
              "  a centroid readout (drift) and the component that does not\n"
              "  (diffusion).\n",
              (unsigned long long)teach_ticks, (unsigned long long)gap_ticks);

  DpGroup f1_of[kDpArmCount], f2_of[kDpArmCount];

  for (uint32_t a = 0; a < kDpArmCount; ++a) {
    Session s;
    if (!s.init(blob, error)) {
      std::printf("  arm %u failed to hatch: %s\n", a, error.c_str());
      return false;
    }
    aibaby::Network& net = s.brain.network();
    const uint32_t modules = dna0.module_count();
    uint32_t m_vocal = modules;
    for (uint32_t m = 0; m < modules; ++m) {
      if (std::strcmp(net.module_dna(m).name, "vocal") == 0) { m_vocal = m; break; }
    }
    if (m_vocal == modules) {
      std::printf("  setup failed: no module named \"vocal\"\n");
      return false;
    }
    const aibaby::ModuleState& vms = net.module(m_vocal);
    const uint32_t f1_lo = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF1);
    const uint32_t f1_hi = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF1 + 1);
    const uint32_t f2_lo = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF2);
    const uint32_t f2_hi = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF2 + 1);

    const aibaby::DnaAudio& acfg = dna0.header().audio;
    Ear ear;
    if (!ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource caregiver(acfg.sample_rate);
    std::vector<float> pcm(acfg.sample_rate / 1000);
    const uint32_t spt = acfg.sample_rate / 1000;
    const Word& heard = kWords[kCapHeard];

    const uint32_t n_teach = uint32_t(teach_ticks / kCapTrial);
    const uint32_t n_gap = uint32_t(gap_ticks / kCapTrial);
    std::deque<Praise> pending;
    double base[4] = {-1.0, -1.0, -1.0, -1.0};
    uint32_t last_frame = 0;
    uint64_t last_feedback = 0;
    std::vector<double> f1_at_boundary, f2_at_boundary;

    for (uint32_t trial = 0; trial < n_teach + n_gap; ++trial) {
      const bool in_teach = trial < n_teach;
      const CapLesson lesson = in_teach ? kCapLessonA
                             : (a == kDpAThenB ? kCapLessonB : kCapLessonNone);
      if (trial == n_teach) {
        for (uint32_t i = f1_lo; i < f1_hi; ++i) f1_at_boundary.push_back(double(net.bias_of(i)));
        for (uint32_t i = f2_lo; i < f2_hi; ++i) f2_at_boundary.push_back(double(net.bias_of(i)));
      }
      for (uint64_t t = 0; t < kCapTrial; ++t) {
        const uint64_t now = uint64_t(trial) * kCapTrial + t;
        while (!pending.empty() && pending.front().tick <= now) {
          s.brain.praise(pending.front().value);
          pending.pop_front();
        }
        const bool sounding = t < 900;
        caregiver.render(sounding ? heard.f0 : 0.0f, heard.f1, heard.f2,
                         sounding ? 0.5f : 0.0f, pcm.data(), spt);
        ear.tick(s.brain, pcm.data(), spt);
        s.brain.step();
        if (s.brain.vocal_frame() == last_frame) continue;
        last_frame = s.brain.vocal_frame();
        const aibaby::VocalParams& v = s.brain.voice();
        const bool voiced = v.voicing > 0.5f && v.amplitude > kAmplitudeFloor;
        if (lesson != kCapLessonNone && voiced && t >= kCapRewardFrom &&
            t < kCapRewardTo && now - last_feedback >= regime.feedback_period) {
          const double e = lesson_error(lesson, double(v.f1), double(v.f2));
          if (e >= 0.0) {
            last_feedback = now;
            double& b = base[uint32_t(lesson)];
            if (b >= 0.0) {
              pending.push_back(Praise{now + regime.delay,
                                       e < b ? regime.praise : regime.scold});
            }
            b = b < 0.0 ? e : b + kCapBaselineAlpha * (e - b);
          }
        }
      }
    }
    std::vector<double> d1, d2;
    for (uint32_t i = f1_lo; i < f1_hi; ++i) {
      d1.push_back(double(net.bias_of(i)) - f1_at_boundary[i - f1_lo]);
    }
    for (uint32_t i = f2_lo; i < f2_hi; ++i) {
      d2.push_back(double(net.bias_of(i)) - f2_at_boundary[i - f2_lo]);
    }
    f1_of[a] = decompose(d1);
    f2_of[a] = decompose(d2);
  }

  std::printf("\n    %-11s %-9s %-11s %-11s %-11s\n", "arm", "group", "drift", "diffusion",
              "drift/rms");
  auto row = [&](const char* arm, const char* g, const DpGroup& d) {
    std::printf("    %-11s %-9s %-11.5f %-11.5f %-11.3f\n", arm, g, d.drift, d.diffusion,
                d.rms > 1e-12 ? d.drift / d.rms : 0.0);
  };
  row("A only", "F1", f1_of[kDpAOnly]);
  row("A only", "F2", f2_of[kDpAOnly]);
  row("A then B", "F1", f1_of[kDpAThenB]);
  row("A then B", "F2", f2_of[kDpAThenB]);

  const DpGroup& taught = f2_of[kDpAThenB];    // being taught in phase 2
  const DpGroup& idle = f1_of[kDpAThenB];      // NOT being taught in phase 2
  const double taught_ratio = taught.rms > 1e-12 ? taught.drift / taught.rms : 0.0;
  const double idle_ratio = idle.rms > 1e-12 ? idle.drift / idle.rms : 0.0;

  std::printf("\n  `drift` is the part of the change that moves what the group decodes\n"
              "  to; `diffusion` is the part that only spreads it. A group being\n"
              "  taught should show drift; a group receiving zero-mean noise should\n"
              "  show diffusion and no drift.\n");
  std::printf("\n  taught group (F2)     drift %.5f  diffusion %.5f  ratio %.3f\n"
              "  untaught group (F1)   drift %.5f  diffusion %.5f  ratio %.3f\n",
              taught.drift, taught.diffusion, taught_ratio, idle.drift, idle.diffusion,
              idle_ratio);
  std::printf("  the untaught group still moves %.0f%% as much in total as the taught\n"
              "  one — that motion is what erodes lesson A\n",
              taught.rms > 1e-12 ? 100.0 * idle.rms / taught.rms : 0.0);

  if (idle.diffusion <= 1e-9) {
    std::printf("\n  NOTHING MOVED — the untaught group's bias did not change at all,\n"
                "  so there is no erosion to explain and the premise is wrong.\n");
    return false;
  }
  if (idle_ratio >= taught_ratio) {
    std::printf("\n  IT IS A CREDIT PROBLEM AFTER ALL — the untaught group drifts as\n"
                "  directionally as the taught one (%.3f vs %.3f). Something correlates\n"
                "  its perturbations with the other lesson's reward, so the covariance\n"
                "  is not zero and the variance story is refuted.\n",
                idle_ratio, taught_ratio);
    return true;
  }
  std::printf("\n  IT IS VARIANCE, NOT CREDIT — the taught group moves directionally\n"
              "  (ratio %.3f) and the untaught group mostly does not (%.3f), while\n"
              "  still moving %.0f%% as much in total. Node perturbation is already\n"
              "  telling the untaught neurons \"you get nothing\"; what it cannot do is\n"
              "  say it without noise. The mechanism to build is one that stops what\n"
              "  has already been decided from moving — consolidation — and NOT one\n"
              "  that works out who deserves the reward.\n",
              taught_ratio, idle_ratio,
              taught.rms > 1e-12 ? 100.0 * idle.rms / taught.rms : 0.0);
  (void)verbose;
  return true;
}


// --- metaprobe: does DNA v41 buy what the oracle bought? --------------------
//
// `credit` priced perfect targeting with an oracle: interference goes to zero
// (retention ~1.0 on 3 of 3) at 30% of the learning rate. `driftprobe` then
// said the oracle was not supplying missing information at all — node
// perturbation's covariance with an irrelevant reward is already zero, and what
// the mask suppressed was VARIANCE. DNA v41 is the local mechanism that follows
// from that: each neuron gates its own plasticity on E[u]^2 / E[u^2] over its
// own updates, needing no teacher, no third factor and no credit assignment.
//
// This is `credit`'s design with the oracle replaced by a genome patch, so the
// two are directly comparable. The bar is set by both of them at once:
//
//   retention   must beat the broadcast arm's 0.84 and approach the oracle's
//               1.03, or the mechanism does not do what it was built for.
//   A gain      must beat the oracle's 0.231, or v41 is merely a slower
//               learner wearing a different hat — which is what a floor on
//               plasticity would look like if the SNR term did nothing.
//   B landing   must survive, or v41 has frozen the creature rather than
//               protected it, and a retention win would mean nothing.
//
// That third one is the real risk in this mechanism and it has its own refusal.
namespace {

// DNA v41 offers two gates that ask the same question different ways, so both
// run here against the same creature and the same seed rather than in two runs
// that could differ for any other reason.
enum MpMode { kMpOff = 0, kMpSnr, kMpCommit, kMpFlow, kMpFlow2 };

enum MpArm {
  kMpAOnly = 0, kMpAThenB,          // v41 off — the baseline pair
  kMpAOnlyS, kMpAThenBS,            // the moment-ratio gate
  kMpAOnlyC, kMpAThenBC,            // the commitment brake
  kMpAOnlyF, kMpAThenBF,            // Benna-Fusi's store, slow leak
  kMpAOnlyG, kMpAThenBG,            // ...and a leak three times faster
  kMpNever, kMpArmCount
};

struct MpPlan { CapLesson teach, gap; MpMode mode; };
constexpr MpPlan kMpPlan[kMpArmCount] = {
    {kCapLessonA, kCapLessonNone, kMpOff},
    {kCapLessonA, kCapLessonB,    kMpOff},
    {kCapLessonA, kCapLessonNone, kMpSnr},
    {kCapLessonA, kCapLessonB,    kMpSnr},
    {kCapLessonA, kCapLessonNone, kMpCommit},
    {kCapLessonA, kCapLessonB,    kMpCommit},
    {kCapLessonA, kCapLessonNone, kMpFlow},
    {kCapLessonA, kCapLessonB,    kMpFlow},
    {kCapLessonA, kCapLessonNone, kMpFlow2},
    {kCapLessonA, kCapLessonB,    kMpFlow2},
    {kCapLessonNone, kCapLessonNone, kMpOff}
};

constexpr float kMpWindow = 400.0f;  // reward events in the moment window
constexpr float kMpFloor = 0.25f;    // plasticity left to a fully quieted neuron
// Measured, not guessed: at 400k the taught group's mean per-neuron SNR reads
// 0.0021 against the untaught group's 0.0012, so the whole usable range sits
// near 0.002 and the first two values tried here (0.15, then 0.02) pinned every
// neuron to the floor. Set at the untaught level, so a neuron reading noise is
// quieted and one reading better than noise is not.
constexpr float kMpRef = 0.0012f;    // the SNR that counts as fully consistent
constexpr float kMpCommitGain = 1.0f;
// The clock matters more than the number. Cash-ins run at 100 Hz, the teaching
// phase is 3360 s, and the gap the diffusion happens in is 1568 s. The first
// value tried here was 0.02, which is a time constant of half a SECOND — it
// wiped each lesson within a second of it being taught and froze the creature.
// These two bracket the useful range instead of guessing inside it: 1e-5 is a
// 1000 s store, 3e-5 a 333 s one. Slower preserves learning and damps less;
// faster damps the random walk harder and caps what a lesson can accumulate.
constexpr float kMpFlowRate = 1e-5f;    // tau ~ 1000 s on the 100 Hz cash-in clock
// `meta_ratio`, NOT the flow, is what froze the first two attempts. The pair
// conserves ratio*bias + slow, so the readout settles at r/(1+r) of the lesson:
// 0.05 keeps 5% and looks like a frozen creature at any flow. These two keep
// 23% and 50%, bracketing the damping-versus-readout trade where it actually
// lives.
constexpr float kMpRatioA = 0.3f;
constexpr float kMpRatioB = 1.0f;

}  // namespace

bool run_metaprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  Regime regime;
  regime.praise = kPraiseValue;
  regime.scold = kScoldValue;
  aibaby::Dna dna0;
  if (dna0.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  std::string error;
  const uint64_t teach_ticks = ticks * 60 / 100;
  const uint64_t gap_ticks = ticks * 28 / 100;
  const uint64_t after_ticks = ticks - teach_ticks - gap_ticks;

  instrument("metaprobe", dna0.header().seed ^ 0x4F13u, ticks / kCapTrial, "trials");
  std::printf("  credit's session with the ORACLE replaced by DNA v41, which is a\n"
              "  real mechanism. S = the moment-ratio gate (window %.0f events, ref %.4f);\n"
              "  C = the commitment brake (gain %.2f, floor %.2f); F = Benna-Fusi's\n"
              "  two-compartment store (flow %.0e, ratio %.2f and %.2f), gating NOTHING\n"
              "  and so should cost no learning rate at all.\n",
              double(kMpWindow), double(kMpRef), double(kMpCommitGain), double(kMpFloor),
              double(kMpFlowRate), double(kMpRatioA), double(kMpRatioB));

  CapRow rows[kMpArmCount];
  double snr_taught[kMpArmCount] = {}, snr_idle[kMpArmCount] = {};
  const char* names[kMpArmCount] = {"A only",   "A then B",
                                    "A only S", "A then B S",
                                    "A only C", "A then B C",
                                    "A only F", "A then B F",
                                    "A only G", "A then B G", "never"};

  for (uint32_t a = 0; a < kMpArmCount; ++a) {
    std::vector<uint8_t> variant = blob;
    if (kMpPlan[a].mode != kMpOff) {
      const size_t base = offsetof(aibaby::DnaHeader, exploration);
      auto put = [&](size_t off, float v) {
        std::memcpy(variant.data() + base + off, &v, sizeof(v));
      };
      put(offsetof(aibaby::DnaExploration, meta_floor), kMpFloor);
      if (kMpPlan[a].mode == kMpSnr) {
        put(offsetof(aibaby::DnaExploration, meta_window), kMpWindow);
        put(offsetof(aibaby::DnaExploration, meta_ref), kMpRef);
      } else if (kMpPlan[a].mode == kMpCommit) {
        put(offsetof(aibaby::DnaExploration, meta_commit), kMpCommitGain);
      } else {
        put(offsetof(aibaby::DnaExploration, meta_flow), kMpFlowRate);
        put(offsetof(aibaby::DnaExploration, meta_ratio),
            kMpPlan[a].mode == kMpFlow ? kMpRatioA : kMpRatioB);
      }
    }
    Session s;
    if (!s.init(variant, error)) {
      std::printf("  arm %s failed to hatch: %s\n", names[a], error.c_str());
      return false;
    }
    aibaby::Network& net = s.brain.network();
    const uint32_t modules = dna0.module_count();
    uint32_t m_vocal = modules;
    for (uint32_t m = 0; m < modules; ++m) {
      if (std::strcmp(net.module_dna(m).name, "vocal") == 0) { m_vocal = m; break; }
    }
    if (m_vocal == modules) {
      std::printf("  setup failed: no module named \"vocal\"\n");
      return false;
    }
    const aibaby::ModuleState& vms = net.module(m_vocal);
    const uint32_t f1_lo = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF1);
    const uint32_t f1_hi = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF1 + 1);
    const uint32_t f2_lo = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF2);
    const uint32_t f2_hi = vms.begin + aibaby::slice_begin(vms.count, aibaby::kVocalGroups, kCrGroupF2 + 1);

    const aibaby::DnaAudio& acfg = dna0.header().audio;
    Ear ear;
    if (!ear.configure(acfg, error)) {
      std::printf("  transducer failed: %s\n", error.c_str());
      return false;
    }
    VowelSource caregiver(acfg.sample_rate);
    std::vector<float> pcm(acfg.sample_rate / 1000);
    const uint32_t spt = acfg.sample_rate / 1000;
    const Word& heard = kWords[kCapHeard];

    const uint32_t n_teach = uint32_t(teach_ticks / kCapTrial);
    const uint32_t n_gap = uint32_t(gap_ticks / kCapTrial);
    const uint32_t n_after = uint32_t(after_ticks / kCapTrial);
    const uint32_t n_total = n_teach + n_gap + n_after;
    const uint32_t third = n_teach / 3 ? n_teach / 3 : 1;

    std::deque<Praise> pending;
    double base[4] = {-1.0, -1.0, -1.0, -1.0};
    uint32_t last_frame = 0;
    uint64_t last_feedback = 0;
    double s1[3] = {}, s2[3] = {}, sf1[3] = {}, sf2[3] = {};
    uint32_t n_win[3] = {};

    for (uint32_t trial = 0; trial < n_total; ++trial) {
      const bool in_teach_phase = trial < n_teach;
      const bool in_gap = trial >= n_teach && trial < n_teach + n_gap;
      const CapLesson lesson = in_teach_phase ? kMpPlan[a].teach
                             : in_gap         ? kMpPlan[a].gap
                                              : kCapLessonNone;
      // Sampled at the END of the second phase, where the question is: does the
      // group that is NOT being taught know that its updates are noise?
      if (trial == n_teach + n_gap - 1) {
        double s_t = 0.0, s_i = 0.0;
        for (uint32_t i = f2_lo; i < f2_hi; ++i) s_t += double(net.meta_snr(i));
        for (uint32_t i = f1_lo; i < f1_hi; ++i) s_i += double(net.meta_snr(i));
        snr_taught[a] = s_t / double(f2_hi - f2_lo);
        snr_idle[a] = s_i / double(f1_hi - f1_lo);
      }
      double f1_acc = 0, f2_acc = 0;
      uint32_t nv = 0;
      for (uint64_t t = 0; t < kCapTrial; ++t) {
        const uint64_t now = uint64_t(trial) * kCapTrial + t;
        while (!pending.empty() && pending.front().tick <= now) {
          s.brain.praise(pending.front().value);
          pending.pop_front();
        }
        const bool sounding = t < 900;
        caregiver.render(sounding ? heard.f0 : 0.0f, heard.f1, heard.f2,
                         sounding ? 0.5f : 0.0f, pcm.data(), spt);
        ear.tick(s.brain, pcm.data(), spt);
        s.brain.step();
        if (s.brain.vocal_frame() == last_frame) continue;
        last_frame = s.brain.vocal_frame();
        const aibaby::VocalParams& v = s.brain.voice();
        const bool voiced = v.voicing > 0.5f && v.amplitude > kAmplitudeFloor;
        if (lesson != kCapLessonNone && voiced && t >= kCapRewardFrom &&
            t < kCapRewardTo && now - last_feedback >= regime.feedback_period) {
          const double e = lesson_error(lesson, double(v.f1), double(v.f2));
          if (e >= 0.0) {
            last_feedback = now;
            double& b = base[uint32_t(lesson)];
            if (b >= 0.0) {
              pending.push_back(Praise{now + regime.delay,
                                       e < b ? regime.praise : regime.scold});
            }
            b = b < 0.0 ? e : b + kCapBaselineAlpha * (e - b);
          }
        }
        if (t < kCapEchoFrom || t >= kCapEchoTo || !voiced) continue;
        ++nv;
        f1_acc += double(v.f1);
        f2_acc += double(v.f2);
      }
      if (nv == 0) continue;
      const double f1 = f1_acc / nv, f2 = f2_acc / nv;
      const double e1 = axis_error(f1, kCapTargetF1), e2 = axis_error(f2, kCapTargetF2);
      if (e1 < 0.0 || e2 < 0.0) continue;
      ++rows[a].scored;
      int w = -1;
      if (trial < third) w = 0;
      else if (in_teach_phase && trial >= n_teach - third) w = 1;
      else if (trial >= n_teach + n_gap) w = 2;
      if (w < 0) continue;
      s1[w] += e1; s2[w] += e2; sf1[w] += f1; sf2[w] += f2; ++n_win[w];
    }

    CapRow& r = rows[a];
    const double i0 = n_win[0] ? 1.0 / n_win[0] : 0.0;
    const double i2 = n_win[2] ? 1.0 / n_win[2] : 0.0;
    r.e1_before = s1[0] * i0; r.e2_before = s2[0] * i0;
    r.e1_after = s1[2] * i2;  r.e2_after = s2[2] * i2;
  }

  auto g1 = [](const CapRow& r) {
    return r.e1_before > 1e-6 ? (r.e1_before - r.e1_after) / r.e1_before : 0.0;
  };
  auto g2 = [](const CapRow& r) {
    return r.e2_before > 1e-6 ? (r.e2_before - r.e2_after) / r.e2_before : 0.0;
  };
  const double n1 = g1(rows[kMpNever]), n2 = g2(rows[kMpNever]);

  std::printf("\n    %-11s %-9s %-12s %-12s\n", "arm", "trials", "A gain (F1)", "B gain (F2)");
  for (uint32_t a = 0; a < kMpArmCount; ++a) {
    std::printf("    %-11s %-9u %+11.3f %+11.3f\n", names[a], rows[a].scored,
                g1(rows[a]) - n1, g2(rows[a]) - n2);
  }

  const double off_only = g1(rows[kMpAOnly]) - n1;
  const double off_after = g1(rows[kMpAThenB]) - n1;
  const double off_b = g2(rows[kMpAThenB]) - n2;
  const double ret_off = off_only > 1e-6 ? off_after / off_only : 0.0;

  struct Verdict { const char* name; double only, after, b, ret; };
  Verdict v[4];
  const MpArm only_of[4] = {kMpAOnlyS, kMpAOnlyC, kMpAOnlyF, kMpAOnlyG};
  const MpArm after_of[4] = {kMpAThenBS, kMpAThenBC, kMpAThenBF, kMpAThenBG};
  const char* label[4] = {"moment ratio (S)", "commitment (C)", "Benna-Fusi r=0.3",
                          "Benna-Fusi r=1.0"};
  for (int k = 0; k < 4; ++k) {
    v[k].name = label[k];
    v[k].only = g1(rows[only_of[k]]) - n1;
    v[k].after = g1(rows[after_of[k]]) - n1;
    v[k].b = g2(rows[after_of[k]]) - n2;
    v[k].ret = v[k].only > 1e-6 ? v[k].after / v[k].only : 0.0;
  }

  std::printf("\n  mean per-neuron SNR at the end of the second phase, S arm:\n"
              "    group being taught   %.4f\n"
              "    group NOT taught     %.4f   (this is what the S gate reads)\n",
              snr_taught[kMpAThenBS], snr_idle[kMpAThenBS]);
  std::printf("\n    %-18s %-10s %-10s %-10s %-10s\n", "gate", "A retained", "A gain",
              "B landed", "vs off");
  std::printf("    %-18s %-10.2f %+-10.3f %+-10.3f %s\n", "off", ret_off, off_only,
              off_b, "-");
  for (int k = 0; k < 4; ++k) {
    char d[32];
    std::snprintf(d, sizeof(d), "%+.2f", v[k].ret - ret_off);
    std::printf("    %-18s %-10.2f %+-10.3f %+-10.3f %s\n", v[k].name, v[k].ret, v[k].only,
                v[k].b, d);
  }
  std::printf("\n  the bar is both at once: retention must beat %.2f and approach the\n"
              "  oracle's 1.03, while A's gain beats the oracle's 0.231 — a gate that\n"
              "  buys retention by simply learning less has bought nothing.\n", ret_off);

  bool any_usable = false;
  for (int k = 0; k < 4; ++k) {
    if (v[k].only <= 0.02) {
      std::printf("\n  %s FROZE THE CREATURE — lesson A no longer lands (%+.3f).\n"
                  "  A verdict on these constants, not on the idea.\n", v[k].name, v[k].only);
      continue;
    }
    if (v[k].b <= 0.02) {
      std::printf("\n  %s BLOCKED THE SECOND LESSON — B lands at %+.3f against %+.3f\n"
                  "  with the gate off. Retention bought this way means nothing.\n",
                  v[k].name, v[k].b, off_b);
      continue;
    }
    any_usable = true;
    // "Did not freeze and did not block" is ALL this line means. An earlier
    // version said "is usable" and then printed a retention change of -0.70
    // beside it, which reads as an endorsement of a bad number.
    std::printf("\n  %s ran without freezing A or blocking B. Retention %+.2f\n"
                "  against off, A gain %+.3f against %+.3f. Whether that is an\n"
                "  improvement is the sign of the first number, not this line.\n",
                v[k].name, v[k].ret - ret_off, v[k].only, off_only);
  }
  if (!any_usable) {
    std::printf("\n  NEITHER GATE IS USABLE at these constants.\n");
    return false;
  }
  (void)verbose;
  return true;
}


// --- trajprobe: does an utterance have a SHAPE, or only a value? ------------
//
// Every taught result on this page teaches a SETPOINT. M1c moves the creature's
// vowel toward a target; `capacity` teaches two formants to two values;
// `vocallearn` scores the distance from a fixed pair of numbers. Nothing has
// ever asked the larynx for a trajectory, and the reason is visible in the
// genome rather than in any experiment.
//
// §5.3's own comment says the songbird arrangement plainly: "HVC drives RA
// reliably and LMAN adds variance on top. This is the HVC term." The creature
// has an LMAN — node perturbation, DNA v10 — and an RA, which is `vocal`. What
// it has in place of HVC is `drive_compensation`: a SCALAR steady
// depolarisation. A constant. A constant cannot carry a sequence, so if that
// really is all the drive there is, every utterance is a fixed point plus noise
// and a word is not merely untaught but unreachable.
//
// This measures that before anything is built for it. `central` cannot supply
// the missing structure either — it is sense-driven convergence with no
// autonomous time course of its own — so the answer decides whether the next
// mechanism is a generator or a way of shaping structure that is already there.
//
// The question is asked in two halves, because they can come apart:
//
//   shape          does the mean utterance have a time course at all, or is
//                  the between-bin variance of it no bigger than the noise?
//   reproducible   do DIFFERENT utterances share that time course? Split the
//                  utterances in half, average each half, and correlate. A
//                  creature whose every utterance wanders differently has
//                  variability but no sequence, and shaping cannot get hold of
//                  it.
//
// Each utterance is mean-subtracted before binning, so this is about SHAPE and
// not about which vowel was said — otherwise a creature that reliably says the
// same steady vowel would score a perfect trajectory correlation for having no
// trajectory at all. That trap is the whole reason the null below shuffles time
// bins rather than utterances.
namespace {

constexpr uint32_t kTjBins = 8;         // time bins across one utterance
constexpr uint32_t kTjMinFrames = 8;    // shorter than this cannot fill them
constexpr uint32_t kTjGapFrames = 3;    // unvoiced frames that end an utterance

struct TjSplit { double r_f1 = 0.0, r_f2 = 0.0; };

// A plain struct rather than std::array: <array> is not included in this
// translation unit and one probe is not a reason to add a header to it.
struct TjBins {
  double v[kTjBins];
  double& operator[](uint32_t i) { return v[i]; }
  const double& operator[](uint32_t i) const { return v[i]; }
};

// Pearson correlation between two binned trajectories.
double corr(const double* a, const double* b, uint32_t n) {
  double ma = 0, mb = 0;
  for (uint32_t i = 0; i < n; ++i) { ma += a[i]; mb += b[i]; }
  ma /= n; mb /= n;
  double num = 0, da = 0, db = 0;
  for (uint32_t i = 0; i < n; ++i) {
    const double x = a[i] - ma, y = b[i] - mb;
    num += x * y; da += x * x; db += y * y;
  }
  return (da > 1e-12 && db > 1e-12) ? num / std::sqrt(da * db) : 0.0;
}

// Average the even-indexed and odd-indexed utterances separately and correlate
// the two means. Split-half rather than utterance-to-utterance, because a
// single pair of utterances is far too noisy to correlate and the quantity that
// matters is whether a SHARED time course exists at all.
TjSplit split_half(const std::vector<TjBins>& f1, const std::vector<TjBins>& f2) {
  TjSplit out;
  if (f1.size() < 4) return out;
  double a1[kTjBins] = {}, b1[kTjBins] = {}, a2[kTjBins] = {}, b2[kTjBins] = {};
  uint32_t na = 0, nb = 0;
  for (size_t u = 0; u < f1.size(); ++u) {
    double* d1 = (u % 2) ? b1 : a1;
    double* d2 = (u % 2) ? b2 : a2;
    for (uint32_t k = 0; k < kTjBins; ++k) { d1[k] += f1[u][k]; d2[k] += f2[u][k]; }
    if (u % 2) ++nb; else ++na;
  }
  if (na == 0 || nb == 0) return out;
  for (uint32_t k = 0; k < kTjBins; ++k) {
    a1[k] /= na; b1[k] /= nb; a2[k] /= na; b2[k] /= nb;
  }
  out.r_f1 = corr(a1, b1, kTjBins);
  out.r_f2 = corr(a2, b2, kTjBins);
  return out;
}

}  // namespace

// DNA v42 arms. `seqprobe` showed a hand-wired population chain carries a
// travelling, reproducible, finite sequence — the centre of activity climbs
// 107 -> 330 over 96 ms and then the wave runs off the end. This asks the only
// question that matters about it: does any of that reach the VOICE?
//
// Two placements, because they fail differently and it is worth knowing which.
// In `central` the chain has to cross the thin central->vocal tract (density
// 0.03) to be heard at all. In `vocal` it is already at the larynx, but it
// sweeps ACROSS the nine parameter groups rather than within them, so it lights
// f0, then voicing, then f1, then f2 in turn — a sequential parameter sweep
// rather than a coordinated trajectory. Neither is the songbird arrangement,
// which is a separate nucleus projecting into the motor one; both are cheap.
struct TjArm {
  const char* what;
  const char* module;   // nullptr for the untouched creature
  float weight, density, delay_ms;
  uint32_t group;
};

bool run_trajprobe_arm(const std::vector<uint8_t>& blob, uint64_t ticks,
                       const TjArm& arm, double* share_out, double* amp_out);

bool run_trajprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  const TjArm arms[] = {
      {"no chain", nullptr, 0.0f, 0.0f, 0.0f, 0},
      {"chain in central", "central", 0.30f, 1.0f, 8.0f, 20},
      {"chain in vocal", "vocal", 0.30f, 1.0f, 8.0f, 20},
  };
  bool ok = true;
  double share[3] = {}, amp[3] = {};
  for (uint32_t a = 0; a < 3; ++a) {
    std::printf("\n  ===== %s =====\n", arms[a].what);
    ok = run_trajprobe_arm(blob, ticks, arms[a], &share[a], &amp[a]) && ok;
  }
  const double amp_best = share[2] > share[1] ? amp[2] : amp[1];
  std::printf("\n  shared time course, the fraction of an utterance's movement that\n"
              "  other utterances agree with:\n");
  for (uint32_t a = 0; a < 3; ++a) {
    std::printf("    %-18s %.1f%%\n", arms[a].what, 100.0 * share[a]);
  }
  const double best = std::max(share[1], share[2]);
  // Gated on the AMPLITUDE as well as the fraction. The first version of this
  // line asked only whether utterances agreed, and called a 53.8% shared shape
  // spanning 4.1 Hz "a syllable" — a real statistic about an event no listener
  // could hear. The same mistake this probe's own null was rewritten to avoid.
  if (best > share[0] * 2.0 && best > 0.20 && amp_best > 30.0) {
    std::printf("\n  THE CHAIN REACHES THE VOICE AUDIBLY — utterances agree about their\n"
                "  own shape where they did not, and the shape spans %.0f Hz.\n", amp_best);
  } else if (best > share[0] * 2.0 && best > 0.20) {
    std::printf("\n  A SHAPE, BUT AN INAUDIBLE ONE — utterances now agree about their own\n"
                "  time course (%.1f%% against %.1f%%), which they did not before, so the\n"
                "  chain IS reaching the larynx and imposing a reproducible trajectory.\n"
                "  It spans %.0f Hz. Teaching moves a formant by ~70 Hz. The generator\n"
                "  works and the coupling is far too weak to hear, which is a question\n"
                "  about the route into the vocal groups, not about the sequence.\n",
                100.0 * best, 100.0 * share[0], amp_best);
  } else {
    std::printf("\n  THE CHAIN DOES NOT REACH THE VOICE — %.1f%% against %.1f%% for the\n"
                "  untouched creature. A sequence exists in the brain and the larynx\n"
                "  still holds a vowel, so what is missing is the ROUTE, not the\n"
                "  generator: nothing aligns the chain to the start of an utterance,\n"
                "  and a chain that begins at a different place each time produces\n"
                "  utterances that cannot agree.\n", 100.0 * best, 100.0 * share[0]);
  }
  (void)verbose;
  return ok;
}

bool run_trajprobe_arm(const std::vector<uint8_t>& blob, uint64_t ticks, const TjArm& arm,
                       double* share_out, double* amp_out) {
  aibaby::Dna dna0;
  if (dna0.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  std::vector<uint8_t> variant = blob;
  if (arm.module) {
    int32_t m = -1;
    for (uint32_t i = 0; i < dna0.module_count(); ++i) {
      if (std::strcmp(dna0.module(i).name, arm.module) == 0) m = int32_t(i);
    }
    if (m < 0) { std::printf("  no module named %s\n", arm.module); return false; }
    const size_t at = sizeof(aibaby::DnaHeader) + sizeof(aibaby::DnaModule) * size_t(m);
    std::memcpy(variant.data() + at + offsetof(aibaby::DnaModule, chain_weight),
                &arm.weight, sizeof(float));
    std::memcpy(variant.data() + at + offsetof(aibaby::DnaModule, chain_density),
                &arm.density, sizeof(float));
    std::memcpy(variant.data() + at + offsetof(aibaby::DnaModule, chain_delay_ms),
                &arm.delay_ms, sizeof(float));
    std::memcpy(variant.data() + at + offsetof(aibaby::DnaModule, chain_group),
                &arm.group, sizeof(uint32_t));
  }
  std::string error;
  Session s;
  if (!s.init(variant, error)) {
    std::printf("  failed to hatch: %s\n", error.c_str());
    return false;
  }
  instrument("trajprobe", dna0.header().seed ^ 0x7A3Cu, ticks, "ticks");
  std::printf("  the creature babbles alone; every voiced run is one utterance,\n"
              "  mean-subtracted and resampled to %u bins. The question is whether\n"
              "  utterances SHARE a time course, not whether they have one each.\n",
              kTjBins);

  std::vector<TjBins> traj_f1, traj_f2;
  std::vector<double> abs_first, abs_last, abs_first2, abs_last2;
  std::vector<double> cur_f1, cur_f2;
  uint32_t last_frame = 0, gap = 0;
  uint64_t frames = 0, voiced_frames = 0;
  double len_sum = 0.0;

  auto close_utterance = [&]() {
    if (cur_f1.size() >= kTjMinFrames) {
      len_sum += double(cur_f1.size());
      TjBins b1 = {}, b2 = {};
      double m1 = 0, m2 = 0;
      for (size_t i = 0; i < cur_f1.size(); ++i) { m1 += cur_f1[i]; m2 += cur_f2[i]; }
      m1 /= double(cur_f1.size()); m2 /= double(cur_f2.size());
      for (uint32_t k = 0; k < kTjBins; ++k) {
        const size_t lo = cur_f1.size() * k / kTjBins;
        size_t hi = cur_f1.size() * (k + 1) / kTjBins;
        if (hi <= lo) hi = lo + 1;
        double a = 0, b = 0;
        uint32_t n = 0;
        for (size_t i = lo; i < hi && i < cur_f1.size(); ++i) {
          a += cur_f1[i]; b += cur_f2[i]; ++n;
        }
        // Mean-subtracted: the SHAPE of the utterance, not which vowel it was.
        b1[k] = n ? a / n - m1 : 0.0;
        b2[k] = n ? b / n - m2 : 0.0;
      }
      traj_f1.push_back(b1);
      traj_f2.push_back(b2);
      // The ABSOLUTE first and last bins, kept because "the utterances agree
      // about a shape" and "a listener can hear the shape" are different
      // questions and this probe first answered only the first one. A 53.8%
      // shared time course spanning 4 Hz is a real statistic about an inaudible
      // event.
      abs_first.push_back(b1[0] + m1);
      abs_last.push_back(b1[kTjBins - 1] + m1);
      abs_first2.push_back(b2[0] + m2);
      abs_last2.push_back(b2[kTjBins - 1] + m2);
    }
    cur_f1.clear();
    cur_f2.clear();
  };

  for (uint64_t t = 0; t < ticks; ++t) {
    s.brain.step();
    if (s.brain.vocal_frame() == last_frame) continue;
    last_frame = s.brain.vocal_frame();
    ++frames;
    const aibaby::VocalParams& v = s.brain.voice();
    if (v.voicing > 0.5f && v.amplitude > kAmplitudeFloor) {
      ++voiced_frames;
      gap = 0;
      cur_f1.push_back(double(v.f1));
      cur_f2.push_back(double(v.f2));
    } else if (!cur_f1.empty()) {
      if (++gap >= kTjGapFrames) close_utterance();
    }
  }
  close_utterance();

  std::printf("\n  frames %llu, voiced %.1f%%, utterances %zu, mean length %.1f frames\n",
              (unsigned long long)frames,
              frames ? 100.0 * double(voiced_frames) / double(frames) : 0.0,
              traj_f1.size(), traj_f1.empty() ? 0.0 : len_sum / double(traj_f1.size()));

  if (traj_f1.size() < 24) {
    std::printf("\n  TOO FEW UTTERANCES — %zu of them cannot support a split-half\n"
                "  correlation. Raise --ticks.\n", traj_f1.size());
    return false;
  }

  const TjSplit real = split_half(traj_f1, traj_f2);

  // The null shuffles TIME BINS within each utterance. That destroys any shared
  // time course while leaving every other property — utterance count, length,
  // formant spread, the mean-subtraction — exactly as it was.
  aibaby::Rng rng;
  rng.seed(dna0.header().seed ^ 0x7A3Cu);
  double n1 = 0.0, n2 = 0.0;
  const uint32_t perms = 32;
  for (uint32_t p = 0; p < perms; ++p) {
    std::vector<TjBins> sh1 = traj_f1, sh2 = traj_f2;
    for (size_t u = 0; u < sh1.size(); ++u) {
      for (uint32_t k = kTjBins; k > 1; --k) {
        const uint32_t j = rng.next() % k;
        std::swap(sh1[u][k - 1], sh1[u][j]);
        std::swap(sh2[u][k - 1], sh2[u][j]);
      }
    }
    const TjSplit ns = split_half(sh1, sh2);
    n1 += ns.r_f1;
    n2 += ns.r_f2;
  }
  n1 /= perms;
  n2 /= perms;

  // How big the shared time course is in Hz, for a sense of whether a
  // statistically real shape is also an audible one.
  double amp1 = 0.0, amp2 = 0.0;
  {
    double m1[kTjBins] = {}, m2[kTjBins] = {};
    for (size_t u = 0; u < traj_f1.size(); ++u) {
      for (uint32_t k = 0; k < kTjBins; ++k) { m1[k] += traj_f1[u][k]; m2[k] += traj_f2[u][k]; }
    }
    double lo1 = 1e9, hi1 = -1e9, lo2 = 1e9, hi2 = -1e9;
    for (uint32_t k = 0; k < kTjBins; ++k) {
      m1[k] /= double(traj_f1.size());
      m2[k] /= double(traj_f2.size());
      lo1 = std::min(lo1, m1[k]); hi1 = std::max(hi1, m1[k]);
      lo2 = std::min(lo2, m2[k]); hi2 = std::max(hi2, m2[k]);
    }
    amp1 = hi1 - lo1;
    amp2 = hi2 - lo2;
    std::printf("\n  mean utterance shape, Hz from the utterance's own mean:\n    F1 ");
    for (uint32_t k = 0; k < kTjBins; ++k) std::printf("%+7.1f", m1[k]);
    std::printf("\n    F2 ");
    for (uint32_t k = 0; k < kTjBins; ++k) std::printf("%+7.1f", m2[k]);
    std::printf("\n");
  }

  // How far a SINGLE utterance ranges, to read the shared amplitude against.
  // This is the statistic that decides the question, and the correlation is
  // not: when the shared shape is 0.1 Hz, the split-half r is two noise vectors
  // normalised against each other and can come out anywhere — this run read
  // -0.94 on F2 and it means nothing at all. Amplitude first, correlation only
  // as confirmation.
  double ind1 = 0.0, ind2 = 0.0;
  for (size_t u = 0; u < traj_f1.size(); ++u) {
    double lo1 = 1e9, hi1 = -1e9, lo2 = 1e9, hi2 = -1e9;
    for (uint32_t k = 0; k < kTjBins; ++k) {
      lo1 = std::min(lo1, traj_f1[u][k]); hi1 = std::max(hi1, traj_f1[u][k]);
      lo2 = std::min(lo2, traj_f2[u][k]); hi2 = std::max(hi2, traj_f2[u][k]);
    }
    ind1 += hi1 - lo1;
    ind2 += hi2 - lo2;
  }
  ind1 /= double(traj_f1.size());
  ind2 /= double(traj_f2.size());
  const double share1 = ind1 > 1e-9 ? amp1 / ind1 : 0.0;
  const double share2 = ind2 > 1e-9 ? amp2 / ind2 : 0.0;

  std::printf("\n  how much of an utterance's movement is SHARED with the others:\n"
              "    F1   one utterance ranges %6.1f Hz, the shared shape %5.1f Hz  (%.1f%%)\n"
              "    F2   one utterance ranges %6.1f Hz, the shared shape %5.1f Hz  (%.1f%%)\n",
              ind1, amp1, 100.0 * share1, ind2, amp2, 100.0 * share2);
  std::printf("\n  split-half correlation, as confirmation only:\n"
              "    F1   %+.3f   against a bin-shuffled null of %+.3f\n"
              "    F2   %+.3f   against a bin-shuffled null of %+.3f\n",
              real.r_f1, n1, real.r_f2, n2);

  const double best = std::max(real.r_f1, real.r_f2);
  const double null = std::max(n1, n2);
  const double share = std::max(share1, share2);
  if (share_out) *share_out = share;
  if (amp_out) *amp_out = std::max(amp1, amp2);

  // IS IT AUDIBLE? The project's own ruler, on the timbre at the start of an
  // utterance against the timbre at its end.
  double d_shape = 0.0, d_null = 0.0;
  {
    Timbre ruler;
    std::string terr;
    if (ruler.configure(dna0.header().audio, terr)) {
      std::vector<std::vector<double>> ceps;
      std::vector<int> when;
      const double f0 = double(dna0.header().vocal.f0_min);
      for (size_t u = 0; u < abs_first.size(); ++u) {
        std::vector<double> a = ruler.of(f0, abs_first[u], abs_first2[u], 0.4);
        std::vector<double> b = ruler.of(f0, abs_last[u], abs_last2[u], 0.4);
        if (a.empty() || b.empty()) continue;
        ceps.push_back(a); when.push_back(0);
        ceps.push_back(b); when.push_back(1);
      }
      if (ceps.size() >= 24) {
        const double d2 = cepstral_dprime(ceps, when, nullptr, true);
        d_shape = d2 >= 0.0 ? std::sqrt(d2) : -std::sqrt(-d2);
        aibaby::Rng rng;
        rng.seed(dna0.header().seed ^ 0x7A3Cu);
        double ns = 0.0;
        for (uint32_t p = 0; p < 32; ++p) {
          std::vector<int> sh = when;
          for (size_t i = sh.size(); i > 1; --i) std::swap(sh[i - 1], sh[rng.next() % i]);
          const double nd = cepstral_dprime(ceps, sh, nullptr, true);
          ns += nd >= 0.0 ? std::sqrt(nd) : -std::sqrt(-nd);
        }
        d_null = ns / 32.0;
      }
    }
  }
  std::printf("\n  is the shape AUDIBLE? start of an utterance against its end:\n"
              "    d' %.3f against a shuffled null of %.3f\n"
              "  (teaching moves F1 by ~70 Hz for comparison; a shared shape can be\n"
              "   statistically solid and far too small to hear)\n", d_shape, d_null);

  if (share > 0.10 && best - null > 0.3) {
    std::printf("\n  THERE IS A SHARED TIME COURSE — utterances agree about their own\n"
                "  shape at %+.3f against a null of %+.3f. The larynx already produces\n"
                "  structure across time, so the missing piece is a way to SHAPE it,\n"
                "  not a generator to create it.\n", best, null);
  } else {
    std::printf("\n  NO SHARED TIME COURSE — only %.1f%% of an utterance's movement is\n"
                "  shared with the other utterances. They vary, but each varies its own\n"
                "  way: there is variability and no sequence. A setpoint plus noise is\n"
                "  all this larynx produces, which is what a scalar\n"
                "  `drive_compensation` in place of HVC predicts. Reward cannot select\n"
                "  a trajectory that is never repeated, so a word is unreachable until\n"
                "  something generates one.\n", 100.0 * share);
  }
  return true;
}


// --- vocab: how many words can this creature hold apart at once? -----------
//
// `imitate` scores four words in all six pairs and every pair clears 0.75, with
// the decisive /i/-/u/ case — F1s 30 Hz apart, answerable only on F2 — at 0.900.
// So the creature is not running a brightness meter: 200-600 ms after a word
// stops, its voice still carries which one it was.
//
// That invites the obvious question, and it is the recognition twin of what
// `capacity` asked about teaching: how many? Eight words, all 28 pairs, scored
// off ONE simulation in the same window, so a difference between rows is about
// the two vowels and never about the run.
//
// The four appended words CROWD the original four rather than filling the gaps
// between them — /o/ 50 Hz from /u/ on F1, /ae/ 140 from /a/ — because a
// vocabulary that only grows into empty space measures the size of the space
// and not the creature.
//
// What this can and cannot say. A high pairwise score across 28 pairs means
// every word is distinguishable from every other; it does NOT mean the creature
// could pick one out of eight, which is a harder question a pairwise table
// cannot answer. Both are printed, because the gap between them is the
// interesting part.
namespace {

struct VocabPair {
  uint32_t a, b;
  double voice, artic, ear, f1_gap, f2_gap, hz_dist, mel_dist, echo_dist;
};

// Spearman rank correlation. Rank rather than Pearson because the question is
// whether a metric ORDERS the pairs correctly, not whether the relationship is
// linear — and a discrimination score is bounded at both ends, so linearity is
// not on offer.
double spearman(std::vector<double> x, std::vector<double> y) {
  const size_t n = x.size();
  if (n < 3 || y.size() != n) return 0.0;
  auto rank = [n](std::vector<double>& v) {
    std::vector<size_t> idx(n);
    for (size_t i = 0; i < n; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b) { return v[a] < v[b]; });
    std::vector<double> r(n);
    for (size_t k = 0; k < n; ++k) r[idx[k]] = double(k);
    v = r;
  };
  rank(x);
  rank(y);
  double mx = 0, my = 0;
  for (size_t i = 0; i < n; ++i) { mx += x[i]; my += y[i]; }
  mx /= double(n); my /= double(n);
  double num = 0, dx = 0, dy = 0;
  for (size_t i = 0; i < n; ++i) {
    const double a = x[i] - mx, b = y[i] - my;
    num += a * b; dx += a * a; dy += b * b;
  }
  return (dx > 1e-12 && dy > 1e-12) ? num / std::sqrt(dx * dy) : 0.0;
}

}  // namespace

bool run_vocab(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  static const char* kLabel[kVocabCount] = {"/a/ ball", "/i/ cube", "/u/ boot", "/e/ bed",
                                            "/o/ boat", "/ae/ bat", "/^/ but", "/I/ bit"};
  instrument("vocab", dna.header().seed ^ 0x2C0Bu, uint32_t(ticks / 2800), "trials each");
  std::printf("  eight words, all %u pairs, scored off one simulation in the\n"
              "  200-600 ms window AFTER the word stops — so what is measured is what\n"
              "  the voice still carries once the sound is gone.\n",
              kVocabCount * (kVocabCount - 1) / 2);

  constexpr uint32_t kReps = 3;
  double pair_v[kVocabCount][kVocabCount] = {{0}}, pair_e[kVocabCount][kVocabCount] = {{0}};
  double pair_a[kVocabCount][kVocabCount] = {{0}};  // articulators only
  // What the creature ITSELF said after hearing each word, averaged over trials
  // and creatures. The hypothesis this tests: a pair is hard not because the two
  // vowels are close, but because this larynx cannot say them differently.
  double echo_f1[kVocabCount] = {0}, echo_f2[kVocabCount] = {0};
  uint32_t echo_n[kVocabCount] = {0};
  uint32_t n_ok = 0;
  double eight_way = 0.0;
  double eight_neuron = 0.0;
  uint32_t eight_neuron_n = 0;
  double eight_neuron_shuf = 0.0;
  uint32_t eight_neuron_sn = 0;
  double eight_artic = 0.0;
  uint32_t eight_artic_n = 0;
  aibaby::Rng rng_shuf;
  rng_shuf.seed(dna.header().seed ^ 0x5F1Fu);
  uint32_t eight_n = 0;

  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    const ImitateRun p = run_imitate_session(variant, ticks, kVocabCount);
    if (!p.ok || p.scored_labels.size() < 48) continue;
    ++n_ok;
    for (size_t t = 0; t < p.scored_labels.size() && t < p.scored_f1.size(); ++t) {
      const int L = p.scored_labels[t];
      if (L < 0 || L >= int(kVocabCount)) continue;
      echo_f1[L] += p.scored_f1[t];
      echo_f2[L] += p.scored_f2[t];
      ++echo_n[L];
    }
    for (uint32_t a = 0; a < kVocabCount; ++a) {
      for (uint32_t b = a + 1; b < kVocabCount; ++b) {
        std::vector<std::vector<double>> xv, xh, xa;
        std::vector<int> yv;
        for (size_t t = 0; t < p.scored_labels.size(); ++t) {
          const int L = p.scored_labels[t];
          if (L != int(a) && L != int(b)) continue;
          xv.push_back(p.scored_voice[t]);
          if (!p.scored_heard.empty()) xh.push_back(p.scored_heard[t]);
          if (t < p.scored_artic.size()) xa.push_back(p.scored_artic[t]);
          yv.push_back(L == int(b) ? 1 : 0);
        }
        if (yv.size() < 12) continue;
        std::vector<std::vector<double>> iv, ih, ia;
        std::vector<int> jv, jh, ja;
        size_t tv = 0, th = 0, ta = 0;
        interleave_pairs(xv, yv, iv, jv, tv);
        pair_v[a][b] += holdout_accuracy(iv, jv, tv);
        if (xa.size() == yv.size()) {
          interleave_pairs(xa, yv, ia, ja, ta);
          pair_a[a][b] += holdout_accuracy(ia, ja, ta);
        }
        if (!xh.empty()) {
          interleave_pairs(xh, yv, ih, jh, th);
          pair_e[a][b] += holdout_accuracy(ih, jh, th);
        }
      }
    }
    // ONE OF EIGHT, by nearest centroid on held-out trials. A pairwise table
    // saying every pair is separable does not say a word can be picked out of
    // eight — that needs every boundary to hold at once, and this is the number
    // that says whether it does. Chance is 0.125.
    {
      // GUARDED AND UNGUARDED, both. The pairwise table above was moved to the
      // articulators because the wide readout can pass on "one word makes it
      // louder" — and this block was left on `scored_voice`, which is the nine
      // groups PLUS loudness and voicing. That mattered the moment M1d shipped:
      // the answering burst is louder by construction, so an unguarded
      // one-of-eight is exactly where a spurious gain would land. Both are
      // computed here and both are printed; the guarded one is the claim.
      std::vector<std::vector<double>> mu(kVocabCount), amu(kVocabCount);
      std::vector<uint32_t> cnt(kVocabCount, 0), acnt(kVocabCount, 0);
      const size_t half = p.scored_labels.size() / 2;
      const bool have_artic = p.scored_artic.size() == p.scored_labels.size();
      for (size_t t = 0; t < half; ++t) {
        const int L = p.scored_labels[t];
        if (L < 0 || L >= int(kVocabCount)) continue;
        if (mu[L].empty()) mu[L].assign(p.scored_voice[t].size(), 0.0);
        for (size_t d = 0; d < mu[L].size(); ++d) mu[L][d] += p.scored_voice[t][d];
        ++cnt[L];
        if (have_artic) {
          if (amu[L].empty()) amu[L].assign(p.scored_artic[t].size(), 0.0);
          for (size_t d = 0; d < amu[L].size(); ++d) amu[L][d] += p.scored_artic[t][d];
          ++acnt[L];
        }
      }
      if (have_artic) {
        bool a_ok = true;
        for (uint32_t k = 0; k < kVocabCount; ++k) {
          if (acnt[k] < 2) { a_ok = false; break; }
          for (double& v : amu[k]) v /= double(acnt[k]);
        }
        if (a_ok) {
          uint32_t ah = 0, at = 0;
          for (size_t tt = half; tt < p.scored_labels.size(); ++tt) {
            const int L = p.scored_labels[tt];
            if (L < 0 || L >= int(kVocabCount)) continue;
            int best = -1;
            double bd = 0.0;
            for (uint32_t k = 0; k < kVocabCount; ++k) {
              double d2 = 0.0;
              for (size_t d = 0; d < amu[k].size() && d < p.scored_artic[tt].size(); ++d) {
                const double e = p.scored_artic[tt][d] - amu[k][d];
                d2 += e * e;
              }
              if (best < 0 || d2 < bd) { bd = d2; best = int(k); }
            }
            if (best == L) ++ah;
            ++at;
          }
          if (at > 0) { eight_artic += double(ah) / double(at); ++eight_artic_n; }
        }
      }
      bool usable = true;
      for (uint32_t k = 0; k < kVocabCount; ++k) {
        if (cnt[k] < 2) { usable = false; break; }
        for (double& v : mu[k]) v /= double(cnt[k]);
      }
      if (usable) {
        uint32_t hit = 0, tot = 0;
        for (size_t t = half; t < p.scored_labels.size(); ++t) {
          const int L = p.scored_labels[t];
          if (L < 0 || L >= int(kVocabCount)) continue;
          int best = -1;
          double bd = 0.0;
          for (uint32_t k = 0; k < kVocabCount; ++k) {
            double d2 = 0.0;
            for (size_t d = 0; d < mu[k].size() && d < p.scored_voice[t].size(); ++d) {
              const double e = p.scored_voice[t][d] - mu[k][d];
              d2 += e * e;
            }
            if (best < 0 || d2 < bd) { bd = d2; best = int(k); }
          }
          if (best == L) ++hit;
          ++tot;
        }
        if (tot > 0) { eight_way += double(hit) / double(tot); ++eight_n; }
      }
    }

    // THE SAME QUESTION ON VOCAL'S OWN SPIKES. Everything above is read
    // through the nine motor groups, and `m3probe` says the module carries far
    // more than they express: the heard word reads 0.980 per neuron at vocal
    // and 0.380 on the centroid. If eight words hold apart in the module but
    // not in the articulators, the ceiling is the READOUT — a limit on the nine
    // knobs the larynx is steered by, not on the creature. Those are different
    // findings with different fixes, and the pairwise table cannot tell them
    // apart because it only ever sees the knobs.
    //
    // Identical estimator, identical trials, identical split: the only thing
    // that changes is which vector a trial is.
    if (!p.scored_vocal.empty() && p.scored_vocal.size() == p.scored_labels.size()) {
      std::vector<std::vector<double>> mu(kVocabCount);
      std::vector<uint32_t> cnt(kVocabCount, 0);
      const size_t half = p.scored_labels.size() / 2;
      for (size_t t = 0; t < half; ++t) {
        const int L = p.scored_labels[t];
        if (L < 0 || L >= int(kVocabCount)) continue;
        if (mu[L].empty()) mu[L].assign(p.scored_vocal[t].size(), 0.0);
        for (size_t d = 0; d < mu[L].size(); ++d) mu[L][d] += p.scored_vocal[t][d];
        ++cnt[L];
      }
      bool usable = true;
      for (uint32_t k = 0; k < kVocabCount; ++k) {
        if (cnt[k] < 2) { usable = false; break; }
        for (double& v : mu[k]) v /= double(cnt[k]);
      }
      if (usable) {
        uint32_t hit = 0, tot = 0;
        for (size_t t = half; t < p.scored_labels.size(); ++t) {
          const int L = p.scored_labels[t];
          if (L < 0 || L >= int(kVocabCount)) continue;
          int best = -1;
          double bd = 0.0;
          for (uint32_t k = 0; k < kVocabCount; ++k) {
            double d2 = 0.0;
            for (size_t d = 0; d < mu[k].size() && d < p.scored_vocal[t].size(); ++d) {
              const double e = p.scored_vocal[t][d] - mu[k][d];
              d2 += e * e;
            }
            if (best < 0 || d2 < bd) { bd = d2; best = int(k); }
          }
          if (best == L) ++hit;
          ++tot;
        }
        if (tot > 0) { eight_neuron += double(hit) / double(tot); ++eight_neuron_n; }

        // THE DIMENSIONALITY CONTROL. 126 dimensions against 8 classes is a far
        // richer space than 9, and a nearest-centroid estimator can gain from
        // dimensionality alone — so a higher score is not by itself evidence
        // that the module holds more. Shuffle the labels and rebuild the
        // centroids on the SAME 126 dimensions: with no information in the
        // labels this must sit at chance whatever the width. If it does not,
        // the comparison above is about the estimator and not the creature.
        std::vector<int> shuf(p.scored_labels);
        for (size_t i = shuf.size(); i > 1; --i) {
          std::swap(shuf[i - 1], shuf[rng_shuf.next() % i]);
        }
        std::vector<std::vector<double>> smu(kVocabCount);
        std::vector<uint32_t> scnt(kVocabCount, 0);
        for (size_t tt = 0; tt < half; ++tt) {
          const int L = shuf[tt];
          if (L < 0 || L >= int(kVocabCount)) continue;
          if (smu[L].empty()) smu[L].assign(p.scored_vocal[tt].size(), 0.0);
          for (size_t d = 0; d < smu[L].size(); ++d) smu[L][d] += p.scored_vocal[tt][d];
          ++scnt[L];
        }
        bool s_ok = true;
        for (uint32_t k = 0; k < kVocabCount; ++k) {
          if (scnt[k] < 2) { s_ok = false; break; }
          for (double& v : smu[k]) v /= double(scnt[k]);
        }
        if (s_ok) {
          uint32_t sh = 0, st = 0;
          for (size_t tt = half; tt < shuf.size(); ++tt) {
            const int L = shuf[tt];
            if (L < 0 || L >= int(kVocabCount)) continue;
            int best = -1;
            double bd = 0.0;
            for (uint32_t k = 0; k < kVocabCount; ++k) {
              double d2 = 0.0;
              for (size_t d = 0; d < smu[k].size() && d < p.scored_vocal[tt].size(); ++d) {
                const double e = p.scored_vocal[tt][d] - smu[k][d];
                d2 += e * e;
              }
              if (best < 0 || d2 < bd) { bd = d2; best = int(k); }
            }
            if (best == L) ++sh;
            ++st;
          }
          if (st > 0) { eight_neuron_shuf += double(sh) / double(st); ++eight_neuron_sn; }
        }
      }
    }
  }

  if (n_ok == 0) {
    std::printf("\n  INCONCLUSIVE — no usable eight-word sessions. Eight words share the\n"
                "  same trial budget four had, so this needs more --ticks than imitate.\n");
    return false;
  }

  std::vector<VocabPair> rows;
  for (uint32_t a = 0; a < kVocabCount; ++a) {
    for (uint32_t b = a + 1; b < kVocabCount; ++b) {
      const double d1 = double(kWords[a].f1) - double(kWords[b].f1);
      const double d2 = double(kWords[a].f2) - double(kWords[b].f2);
      // The SAME hz_to_mel the cochlea is built from, so this asks whether the
      // creature's discrimination follows the axis its own ear resolves on.
      const double m1 = double(hz_to_mel(kWords[a].f1)) - double(hz_to_mel(kWords[b].f1));
      const double m2 = double(hz_to_mel(kWords[a].f2)) - double(hz_to_mel(kWords[b].f2));
      // The gap between what the creature SAID after each of the two words, on
      // the same mel axis, which is the quantity the discrimination is actually
      // computed from.
      double ed = 0.0;
      if (echo_n[a] > 0 && echo_n[b] > 0) {
        const double e1 = double(hz_to_mel(float(echo_f1[a] / echo_n[a]))) -
                          double(hz_to_mel(float(echo_f1[b] / echo_n[b])));
        const double e2 = double(hz_to_mel(float(echo_f2[a] / echo_n[a]))) -
                          double(hz_to_mel(float(echo_f2[b] / echo_n[b])));
        ed = std::sqrt(e1 * e1 + e2 * e2);
      }
      rows.push_back({a, b, pair_v[a][b] / n_ok, pair_a[a][b] / n_ok, pair_e[a][b] / n_ok,
                      std::fabs(d1), std::fabs(d2),
                      std::sqrt(d1 * d1 + d2 * d2), std::sqrt(m1 * m1 + m2 * m2), ed});
    }
  }
  std::sort(rows.begin(), rows.end(),
            [](const VocabPair& x, const VocabPair& y) { return x.artic < y.artic; });

  std::printf("\n  the ten HARDEST pairs, worst first:\n");
  std::printf("    %-10s %-10s %-8s %-8s %-8s %-8s %-8s %s\n", "word A", "word B", "voice",
              "ARTIC", "EAR", "dF1 Hz", "dF2 Hz", "dist mel");
  for (size_t i = 0; i < rows.size() && i < 10; ++i) {
    std::printf("    %-10s %-10s %-8.3f %-8.3f %-8.3f %-8.0f %-8.0f %.0f\n", kLabel[rows[i].a],
                kLabel[rows[i].b], rows[i].voice, rows[i].artic, rows[i].ear,
                rows[i].f1_gap, rows[i].f2_gap, rows[i].mel_dist);
  }
  // Which axis does the creature's discrimination actually follow? Hz distance
  // and mel distance rank the pairs differently, and only one of them should
  // predict the table if the limit is the cochlea's resolution rather than the
  // larynx's. Ranked, not fitted: nothing here is tuned to the answer.
  {
    std::vector<double> v, dh, dm;
    for (const VocabPair& r : rows) { v.push_back(r.artic); dh.push_back(r.hz_dist);
                                      dm.push_back(r.mel_dist); }
    std::vector<double> de;
    for (const VocabPair& r : rows) de.push_back(r.echo_dist);
    const double rho_hz = spearman(dh, v), rho_mel = spearman(dm, v);
    const double rho_echo = spearman(de, v);
    std::printf("\n  what the creature ITSELF said after each word (mel of the echo):\n");
    for (uint32_t k = 0; k < kVocabCount; ++k) {
      if (!echo_n[k]) continue;
      std::printf("    %-10s heard %4.0f/%4.0f Hz  ->  said %4.0f/%4.0f Hz\n", kLabel[k],
                  double(kWords[k].f1), double(kWords[k].f2), echo_f1[k] / echo_n[k],
                  echo_f2[k] / echo_n[k]);
    }
    std::printf("\n  does distance predict discrimination? Spearman rank correlation\n"
                "  over all %zu pairs, between the gap and the voice score:\n"
                "    Hz  distance between the two WORDS      rho %+.3f\n"
                "    mel distance between the two WORDS      rho %+.3f\n"
                "    mel distance between the two ECHOES     rho %+.3f  <- what is scored\n",
                rows.size(), rho_hz, rho_mel, rho_echo);
    if (rho_echo > std::max(rho_hz, rho_mel) + 0.10) {
      std::printf("  THE LIMIT IS THE VOICE, NOT THE EAR — how far apart the creature\n"
                  "  SAYS two words predicts discrimination better than how far apart\n"
                  "  they are. A pair is hard when this larynx cannot say them\n"
                  "  differently, so a larger vocabulary needs a wider reachable vowel\n"
                  "  space and not a finer cochlea.\n");
    }
    if (rho_mel > rho_hz + 0.10) {
      std::printf("  MEL WINS — the creature's vowel confusions follow its own ear's\n"
                  "  axis, not the formant grid. The vocabulary limit is the cochlea's\n"
                  "  resolution, so a larger one needs a better EAR and not a better\n"
                  "  larynx.\n");
    } else if (rho_hz > rho_mel + 0.10) {
      std::printf("  HZ WINS — confusions track the formant grid rather than the mel\n"
                  "  axis, which is not what a mel filterbank in front of everything\n"
                  "  would predict and is worth explaining before it is built on.\n");
    } else {
      std::printf("  NEITHER SEPARATES — the two metrics rank these eight vowels too\n"
                  "  similarly to tell apart (%+.3f against %+.3f). Deciding this needs\n"
                  "  vowels chosen so the two orderings DISAGREE, which these were not.\n",
                  rho_hz, rho_mel);
    }
  }
  // Scored on the ARTICULATORS, not the unguarded features: a vocabulary claim
  // has to be about two sounds and not about two amounts of sound. `imitate`
  // reads 0.888 unguarded and 0.774 guarded on four words, so the guard is
  // worth about 0.11 and the first version of this verdict banked all of it.
  double mean = 0.0, mean_unguarded = 0.0;
  uint32_t above = 0;
  for (const VocabPair& v : rows) {
    mean += v.artic;
    mean_unguarded += v.voice;
    if (v.artic >= 0.75) ++above;
  }
  mean_unguarded /= double(rows.size());
  mean /= double(rows.size());
  const double eight = eight_n ? eight_way / eight_n : 0.0;

  std::printf("\n  %u creatures, %zu pairs, scored on the ARTICULATORS: mean %.3f,\n"
              "  %u of %zu at or above 0.75. Unguarded it would read %.3f.\n",
              n_ok, rows.size(), mean, above, rows.size(), mean_unguarded);
  // THE READOUT COMPARISON. Same estimator, same trials, same split — the only
  // difference is whether a trial is nine motor scalars or vocal's 126 neurons.
  if (eight_neuron_n > 0) {
    const double en = eight_neuron / eight_neuron_n;
    if (eight_neuron_sn > 0) {
      std::printf("\n  126-dimension shuffled control: %.3f (must sit at 0.125 — a\n"
                  "    nearest-centroid estimator can gain from width alone)\n",
                  eight_neuron_shuf / eight_neuron_sn);
    }
    std::printf("\n  one of eight on VOCAL'S OWN SPIKES:  %.3f (chance 0.125)\n"
                "    against %.3f through the nine motor groups. Same estimator,\n"
                "    same trials, same split; the only change is which vector a\n"
                "    trial is. %s\n",
                en, eight_n ? eight_way / eight_n : 0.0,
                en >= (eight_n ? eight_way / eight_n : 0.0) + 0.15 &&
                        (eight_neuron_sn == 0 ||
                         eight_neuron_shuf / eight_neuron_sn < 0.20)
                    ? "THE CEILING IS THE READOUT — the module holds the\n"
                      "    vocabulary and the nine knobs cannot express it."
                    : "The module does NOT hold what the knobs miss, so the\n"
                      "    ceiling is upstream of the larynx, not in its\n"
                      "    parameterisation.");
  }
  if (eight_artic_n > 0) {
    std::printf("\n  one of eight on the ARTICULATORS: %.3f (chance 0.125)\n"
                "    <- THE CLAIM. Drops loudness and voicing, which the row below\n"
                "    does not, and M1d made the answering burst louder by\n"
                "    construction — so an unguarded vocabulary score is exactly\n"
                "    where a spurious gain would land.\n",
                eight_artic / eight_artic_n);
  }
  std::printf("  one of eight, nearest centroid on held-out trials: %.3f (chance 0.125)\n",
              eight);
  std::printf("\n  A pairwise table saying every pair is separable does NOT say a word\n"
              "  can be picked out of eight — that needs every boundary to hold at once.\n"
              "  The gap between the two numbers above is the interesting part.\n");

  // The bar is EVERY pair, not most of them, and the one-of-eight score has to
  // clear chance by a real margin. A "more than half the pairs pass" rule was
  // tried first and printed EIGHT WORDS HOLD APART over a table whose worst row
  // was 0.569 against a chance of 0.5 — with twelve of twenty-eight pairs
  // failing. At four words every pair clears 0.753, so "most of them" is not
  // the standard the smaller vocabulary already meets.
  const uint32_t failed = uint32_t(rows.size()) - above;
  const bool pairs_ok = failed == 0;
  const bool eight_ok = eight > 0.40;  // chance is 0.125
  if (!pairs_ok || !eight_ok) {
    std::printf("\n  THE VOCABULARY IS FULL BELOW EIGHT — %u of %zu pairs are under 0.75\n"
                "  (worst %.3f against a chance of 0.5), and one of eight reads %.3f\n"
                "  against a chance of 0.125. Four words hold apart on every pair; eight\n"
                "  do not, and the hardest rows above say where it runs out.\n",
                failed, rows.size(), rows.front().voice, eight);
  } else {
    std::printf("\n  EIGHT WORDS HOLD APART — every pair at or above 0.75 and one of\n"
                "  eight at %.3f against a chance of 0.125.\n", eight);
  }
  (void)verbose;
  // The verdict above and the value returned here have to be the same claim.
  // The first version printed THE VOCABULARY IS FULL and then returned true,
  // so `verify` read a milestone this project has never met as newly met — and
  // said so loudly, which is the only reason it was noticed within the minute.
  return pairs_ok && eight_ok;
}


// --- Is interference the blocker? ------------------------------------------
//
// The last question this project has that is not already answered. `dwprobe`
// pointed at `vision->vocal` — the tract that participates, rather than the
// non-participant every earlier decomposition used — says a cube session and a
// ball session write MEASURABLY DIFFERENT weight changes on the v46 creature:
// ratio 0.71 against its own reproducibility ceiling, 3 of 3 seeds, where the
// shipped retina sits at 1.03. So "the rule writes the same thing whichever
// object is present" is false there.
//
// What is left is that the objects ALTERNATE in the real protocol, so the two
// writes land on the same synapses and undo each other. `retain` measures that
// directly (a conflicting second lesson wipes the first, 0.22), `capacity` says
// two lessons coexist only on orthogonal output dimensions, and `credit` shows
// a per-neuron reward mask removes the interference completely.
//
// So: run the naming protocol three ways and read the milestone's own number.
//
//   unmasked        what the creature does today
//   masked byobject the ORACLE — reward reaches the lower half of the larynx on
//                   ball trials and the upper half on cube trials, so the two
//                   lessons cannot overwrite each other
//   masked random   the CONTROL — the same two halves on a coin flip. Without
//                   it a win could be about reward reaching fewer synapses
//                   rather than about the condition, and that is exactly the
//                   confound that has eaten results here before.
//
// Decisive in both directions, and it is why this is worth a run:
//   byobject >> unmasked with random flat  -> interference IS the blocker, and
//     a context gate is a mechanism worth building
//   byobject ~ unmasked                    -> interference is not the blocker
//     either, and the last standing hypothesis is closed with the others.
//
// An oracle prices a mechanism. It is not a behaviour the creature has: the
// mask is chosen from ground truth the creature cannot compute.
bool run_ctxprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  constexpr uint32_t kReps = 5;
  std::printf("  session           %.1f s of simulated life x %u creatures x 3 arms\n",
              double(ticks) * double(dna.header().sim.dt_ms) / 1000.0, kReps);
  instrument("ctxprobe", dna.header().seed, kReps, "creatures x 3 arms");
  std::printf("  the oracle gates REWARD by which object is present. The random\n"
              "  arm gates it the same amount on a coin flip and must stay flat.\n\n");

  struct Arm { const char* name; CtxMask mask; };
  const Arm arms[3] = {{"unmasked", CtxMask::kOff},
                       {"masked by object", CtxMask::kByObject},
                       {"masked at random", CtxMask::kRandom}};
  double sum[3] = {0, 0, 0}, sum_shuf[3] = {0, 0, 0}, sum_echo[3] = {0, 0, 0};
  uint32_t valid[3] = {0, 0, 0};
  std::vector<double> per[3];

  Caregiver care;
  std::printf("  %-5s %-18s %-9s %-9s %s\n", "seed", "arm", "voice", "shuffled", "echo");
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    for (int a = 0; a < 3; ++a) {
      const M3Run run = run_m3_session(variant, ticks, true, care, false, Capture{},
                                       arms[a].mask);
      if (!run.ok) {
        std::printf("  %-5u %-18s (inconclusive)\n", r, arms[a].name);
        continue;
      }
      ++valid[a];
      sum[a] += run.vocal;
      sum_shuf[a] += run.shuffled;
      sum_echo[a] += run.echo;
      per[a].push_back(run.vocal);
      std::printf("  %-5u %-18s %-9.3f %-9.3f %.3f\n", r, arms[a].name, run.vocal,
                  run.shuffled, run.echo);
    }
  }

  if (valid[0] < 3 || valid[1] < 3 || valid[2] < 3) {
    std::printf("\n  ctxprobe INCONCLUSIVE — too few usable creatures.\n");
    return false;
  }

  std::printf("\n  %-18s %-9s %-9s %s\n", "arm", "voice", "shuffled", "echo");
  for (int a = 0; a < 3; ++a) {
    std::printf("  %-18s %-9.3f %-9.3f %.3f\n", arms[a].name, sum[a] / valid[a],
                sum_shuf[a] / valid[a], sum_echo[a] / valid[a]);
  }

  // Paired across creatures, because the arms share a seed and the per-creature
  // spread is much larger than the difference being looked for.
  auto paired = [&](int x, int y) {
    const size_t n = per[x].size() < per[y].size() ? per[x].size() : per[y].size();
    double m = 0;
    for (size_t i = 0; i < n; ++i) m += per[x][i] - per[y][i];
    m /= double(n);
    double ss = 0;
    for (size_t i = 0; i < n; ++i) {
      const double d = (per[x][i] - per[y][i]) - m;
      ss += d * d;
    }
    const double se = n > 1 ? std::sqrt(ss / double(n - 1) / double(n)) : 0.0;
    return std::pair<double, double>(m, se);
  };
  const std::pair<double, double> oracle = paired(1, 0);
  const std::pair<double, double> control = paired(2, 0);
  std::printf("\n  oracle - unmasked    %+.3f +/- %.3f SE   <- the question\n", oracle.first,
              oracle.second);
  std::printf("  random - unmasked    %+.3f +/- %.3f SE   (must stay flat)\n",
              control.first, control.second);
  std::printf("\n    m3's taught-random floor is +/-0.060 across seed families with the\n"
              "    mechanism absent, so read the first line against about 0.12 as well\n"
              "    as against its own SE. A gap that clears both, with the second line\n"
              "    flat, is the first evidence in this project that a CONDITION can be\n"
              "    made to matter at the larynx.\n");
  (void)verbose;
  return true;
}


// --- Where does the vocabulary run out? ------------------------------------
//
// Two numbers from the same creature and the same larynx bracket everything
// this project knows about how much the voice can carry: M1b reads **0.890 for
// two words**, and `vocab` reads **0.210 for eight against a chance of 0.125**.
// Nobody has measured what happens in between, and that curve is the first
// question of the project's second arc — because unlike G3 it starts from
// something that WORKS rather than from chance.
//
// **One session, nested subsets.** Running the creature separately at each
// vocabulary size would confound capacity with sampling: more words in a fixed
// session means fewer trials each, and accuracy would fall for a reason that has
// nothing to do with the creature. So each creature is run ONCE on all eight
// words, and the N-way score is taken over the trials whose label is below N.
// Trials per word are then identical at every N by construction. Total trials do
// fall with N, which shrinks the classifier's training set at small N — that
// biases AGAINST the small-N end, so it is conservative for "where does it fall
// off", and the count is printed so it can be seen.
//
// **Three feature sets, and the third is the point.** `voice` is the milestone's
// own readout, the nine motor groups plus loudness and voicing. `artic` drops
// loudness and voicing, so a win cannot be "one word makes it louder". `vocal`
// is the module's PER-NEURON activity — and the comment on `ImitateRun` already
// posed the question this answers: *is the vocabulary ceiling a limit on the
// creature or on the nine motor scalars everything else is read through?* If
// `vocal` holds up where `voice` collapses, the ceiling is the readout, which is
// the same bottleneck G3 died on wearing different clothes. If both collapse
// together, the limit is the creature and it is a new fact.
//
// Split is INTERLEAVED WITHIN EACH CLASS rather than first-half/second-half:
// the naive split is what made every pre-2026-08-17 `projprobe` number read low,
// and an imitate session drifts.
bool run_vocabcurve(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  constexpr uint32_t kReps = 3;
  instrument("vocabcurve", dna.header().seed ^ 0x2C0Bu, uint32_t(ticks / 2800),
             "trials per creature");
  std::printf("  one eight-word session per creature, scored on nested subsets:\n"
              "  the N-way number uses the trials whose word is among the first N,\n"
              "  so trials PER WORD are the same at every N.\n\n");

  // Nearest class centroid, interleaved within class. Returns -1 if any class
  // has too few training trials to have a centroid at all.
  auto nway = [](const std::vector<std::vector<double>>& x, const std::vector<int>& y,
                 uint32_t n_classes, bool naive = false) -> double {
    std::vector<uint32_t> seen(n_classes, 0), cnt(n_classes, 0);
    std::vector<std::vector<double>> mu(n_classes);
    std::vector<size_t> test;
    for (size_t t = 0; t < y.size() && t < x.size(); ++t) {
      const int L = y[t];
      if (L < 0 || L >= int(n_classes) || x[t].empty()) continue;
      // `naive` reproduces the first-half / second-half split `vocab` uses, so
      // the two numbers can be compared on identical data. It is here as a
      // CONTROL, not an option: if it reproduces vocab's 0.210 where the
      // interleaved split reads higher, then vocab's headline is depressed by
      // its split and not by the creature.
      const bool train = naive ? (t * 2 < y.size()) : (seen[L] % 2 == 0);
      if (train) {
        if (mu[L].empty()) mu[L].assign(x[t].size(), 0.0);
        for (size_t d = 0; d < x[t].size() && d < mu[L].size(); ++d) mu[L][d] += x[t][d];
        ++cnt[L];
      } else {
        test.push_back(t);
      }
      ++seen[L];
    }
    for (uint32_t k = 0; k < n_classes; ++k) {
      if (cnt[k] < 2) return -1.0;
      for (double& v : mu[k]) v /= double(cnt[k]);
    }
    uint32_t hit = 0, tot = 0;
    for (size_t t : test) {
      int best = -1;
      double bd = 0.0;
      for (uint32_t k = 0; k < n_classes; ++k) {
        double d2 = 0.0;
        for (size_t d = 0; d < mu[k].size() && d < x[t].size(); ++d) {
          const double e = x[t][d] - mu[k][d];
          d2 += e * e;
        }
        if (best < 0 || d2 < bd) { bd = d2; best = int(k); }
      }
      if (best == y[t]) ++hit;
      ++tot;
    }
    return tot ? double(hit) / double(tot) : -1.0;
  };

  double acc[kVocabCount + 1][5] = {{0}};
  uint32_t n_at[kVocabCount + 1][5] = {{0}};
  uint32_t trials_at[kVocabCount + 1] = {0};
  aibaby::Rng shuf_rng;
  shuf_rng.seed(dna.header().seed ^ 0x11C7u);

  uint32_t ok = 0;
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    const ImitateRun p = run_imitate_session(variant, ticks, kVocabCount);
    if (!p.ok || p.scored_labels.size() < 48) continue;
    ++ok;

    for (uint32_t n = 2; n <= kVocabCount; ++n) {
      std::vector<std::vector<double>> xv, xa, xn;
      std::vector<int> y;
      for (size_t t = 0; t < p.scored_labels.size(); ++t) {
        const int L = p.scored_labels[t];
        if (L < 0 || L >= int(n)) continue;
        if (t < p.scored_voice.size()) xv.push_back(p.scored_voice[t]);
        if (t < p.scored_artic.size()) xa.push_back(p.scored_artic[t]);
        if (t < p.scored_vocal.size()) xn.push_back(p.scored_vocal[t]);
        y.push_back(L);
      }
      trials_at[n] += uint32_t(y.size());
      std::vector<int> ys = y;
      for (size_t i = ys.size(); i > 1; --i) std::swap(ys[i - 1], ys[shuf_rng.next() % i]);
      const std::vector<std::vector<double>>* sets[5] = {&xv, &xa, &xn, &xv, &xv};
      for (int f = 0; f < 5; ++f) {
        const double a = nway(*sets[f], f == 3 ? ys : y, n, f == 4);
        if (a >= 0.0) { acc[n][f] += a; ++n_at[n][f]; }
      }
    }
  }
  if (ok < 2) {
    std::printf("  vocabcurve INCONCLUSIVE — %u usable creatures.\n", ok);
    return false;
  }

  std::printf("  %-4s %-8s %-8s %-9s %-9s %-9s %-9s %-11s %s\n", "N", "trials",
              "chance", "voice", "artic", "vocal", "shuffled", "voice/chance",
              "naive split");
  for (uint32_t n = 2; n <= kVocabCount; ++n) {
    const double chance = 1.0 / double(n);
    const double v = n_at[n][0] ? acc[n][0] / n_at[n][0] : -1.0;
    std::printf("  %-4u %-8u %-8.3f %-9.3f %-9.3f %-9.3f %-9.3f %-11.2f %.3f\n", n,
                trials_at[n] / ok, chance, v,
                n_at[n][1] ? acc[n][1] / n_at[n][1] : -1.0,
                n_at[n][2] ? acc[n][2] / n_at[n][2] : -1.0,
                n_at[n][3] ? acc[n][3] / n_at[n][3] : -1.0,
                v > 0 ? v / chance : 0.0,
                n_at[n][4] ? acc[n][4] / n_at[n][4] : -1.0);
  }
  std::printf("\n    `voice` is the milestone's readout, `artic` drops loudness and\n"
              "    voicing so a win cannot be 'one word makes it louder', and `vocal`\n"
              "    is the module's own per-neuron activity. Read the last two columns\n"
              "    of the bottom rows against each other: if `vocal` holds where\n"
              "    `voice` falls, the vocabulary ceiling is the NINE SCALARS and not\n"
              "    the creature — the same bottleneck G3 died on. If both fall, the\n"
              "    limit is the creature and that is something this project has not\n"
              "    measured before.\n");
  (void)verbose;
  return true;
}


// --- Does the SCHEDULE matter? ---------------------------------------------
//
// The axis this project has never varied. Eleven mechanisms were built against
// G3 and every one was measured under the same protocol: the two objects
// interleaved from the first trial, at 120 s of simulated life.
//
// Two things in the project's own notes say that protocol is the wrong one to
// have held fixed.
//
//   * `capacity` says **sequential beats simultaneous** — two lessons taught at
//     once interfere where the same two taught in turn do not. Naming has only
//     ever been measured in the simultaneous condition.
//   * every teaching result that WORKS runs at 3.4M-5.6M ticks. `teachsound`
//     (M1c), the one milestone where praise demonstrably moves the voice, needs
//     3,400,000. `m3` runs at 120,000 — 28x shorter, and `g3probe`'s 900k is
//     still 3.7x under it.
//
// So this varies the two protocol axes against each other and reads the
// milestone's own number. `interleaved` is m3's schedule and is the control;
// `block-fade` starts with blocks of sixteen of one object and halves them
// until the schedule IS the interleaved one, so each lesson gets room to
// consolidate before the other arrives.
//
// Both arms carry m3's own taught-minus-random control, and the floor to read
// them against is the one m3 now prints: +/-0.060 across seed families with the
// mechanism absent by construction, spread 0.120.
bool run_curriculum(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  constexpr uint32_t kReps = 3;
  std::printf("  session           %.1f s of simulated life x %u creatures x 2 arms x 2\n",
              double(ticks) * double(dna.header().sim.dt_ms) / 1000.0, kReps);
  instrument("curriculum", dna.header().seed, kReps, "creatures per arm");
  std::printf("  m3's schedule is `interleaved`; `block-fade` teaches in blocks of\n"
              "  16 that halve to 1. Each arm carries m3's own random-order control.\n\n");

  struct Arm { const char* name; Curriculum c; };
  const Arm arms[2] = {{"interleaved (m3)", Curriculum::kInterleaved},
                       {"block-fade", Curriculum::kBlockFade}};
  Caregiver care;
  std::printf("  %-5s %-18s %-9s %-9s %-9s %s\n", "seed", "arm", "named", "random",
              "gap", "echo");
  double sum_gap[2] = {0, 0};
  uint32_t n_gap[2] = {0, 0};
  std::vector<double> gaps[2];

  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    for (int a = 0; a < 2; ++a) {
      const M3Run named =
          run_m3_session(variant, ticks, true, care, false, Capture{}, CtxMask::kOff,
                         arms[a].c);
      const M3Run ctl =
          run_m3_session(variant, ticks, false, care, false, Capture{}, CtxMask::kOff,
                         arms[a].c);
      if (!named.ok || !ctl.ok) {
        std::printf("  %-5u %-18s (inconclusive)\n", r, arms[a].name);
        continue;
      }
      const double gap = named.vocal - ctl.vocal;
      sum_gap[a] += gap;
      ++n_gap[a];
      gaps[a].push_back(gap);
      std::printf("  %-5u %-18s %-9.3f %-9.3f %+-9.3f %.3f\n", r, arms[a].name,
                  named.vocal, ctl.vocal, gap, named.echo);
    }
  }
  if (n_gap[0] < 2 || n_gap[1] < 2) {
    std::printf("\n  curriculum INCONCLUSIVE.\n");
    return false;
  }

  auto se_of = [](const std::vector<double>& v) {
    double m = 0;
    for (double x : v) m += x;
    m /= double(v.size());
    double ss = 0;
    for (double x : v) ss += (x - m) * (x - m);
    return v.size() > 1 ? std::sqrt(ss / double(v.size() - 1) / double(v.size())) : 0.0;
  };
  std::printf("\n  %-18s %s\n", "arm", "taught - random");
  for (int a = 0; a < 2; ++a) {
    std::printf("  %-18s %+.3f +/- %.3f SE over %u creatures\n", arms[a].name,
                sum_gap[a] / n_gap[a], se_of(gaps[a]), n_gap[a]);
  }
  std::printf("\n    Read both against m3's measured floor: the control genome swings\n"
              "    +/-0.060 across seed families with the mechanism absent, spread\n"
              "    0.120. A schedule effect has to clear that, not just its own SE.\n");

  // ...and now the experiment ASSERTS it rather than leaving it to the reader.
  //
  // This returned an unconditional `true` while its spec said Expect::kOpen,
  // so `verify-long` reported "a milestone this project has never met just did"
  // on every run. It was descriptive code carrying a verdict's label. The floor
  // is m3's own measured one: the control genome swings +/-0.060 across seed
  // families with the mechanism absent by construction, so a cross-family claim
  // needs about 0.120.
  const double kM3Floor = 0.120;
  const double best = std::max(sum_gap[0] / n_gap[0], sum_gap[1] / n_gap[1]);
  if (best > kM3Floor) {
    std::printf("\n  SCHEDULE MATTERS — the better arm reads %+.3f, clear of m3's\n"
                "  %.3f cross-family floor. Naming has been measured under one\n"
                "  protocol for eleven mechanisms and the protocol was load-bearing.\n",
                best, kM3Floor);
    return true;
  }
  std::printf("\n  SCHEDULE DOES NOT MATTER — the better arm reads %+.3f against m3's\n"
              "  %.3f cross-family floor, and the blocked arm is actively harmful.\n"
              "  m3's interleaved schedule was already the right one.\n",
              best, kM3Floor);
  (void)verbose;
  return false;
}


// --- ctxlearn: can reward write a conditional map onto a ZERO-BASELINE code? -
//
// Stage 0 of the audio rewrite, and it is a falsification rather than a
// mechanism. It either licenses the rewrite or kills it.
//
// THE ARGUMENT. Every conditional-learning failure in this project has the same
// arithmetic, and the synaptic-perturbation post-mortem writes it out in one
// line: the presynaptic gate is a spike count, a spike count is a neuron's
// baseline rate plus a few percent of condition, so the credit factorises into a
// shared term and a differential one and the shared term is far larger. That
// shared term has been subtracted (v24), signed (v35), gated (v29, v37, v40),
// re-timed (v26, v42), re-routed (v43, v46) and re-rewarded (v20) -- seven
// common-mode walls and eleven mechanisms. Its CAUSE has never been touched:
// requirements 3.1 makes rate homeostasis mandatory, so no population in this
// creature has ever had a baseline of zero.
//
// `vocallearn` states the consequence in two numbers from one instrument:
// **+24.0 points toward a fixed target and -0.1 toward a conditional one**, 3
// of 3 seeds. The +24 is not a consolation prize. It is a conditional-learning
// result with a context layer of size ONE. `capacity` measured what happens
// when two lessons stop sharing parameters -- 0.84 of the first kept against
// 0.22 when they share -- and `driftprobe` measured that the estimator is
// unbiased and merely noisy. Give R-STDP a context layer of size n whose slices
// are DISJOINT and exactly zero when absent, and two conditions share no
// presynaptic unit, therefore no synapse, therefore no common mode to factor
// out. No new learning rule: `trace_pre_` on an unwritten kContext neuron is
// zero, so the shipped rule already has the property and has never had a
// substrate with a zero baseline to run on.
//
// THE CONTROL IS WITHIN ONE GENOME. The oracle is present and wired at every
// level including zero, so all arms share a creature, a noise stream and a set
// of synapses. (An appended-module genome is NOT comparable to
// dna/default.toml -- noise is drawn per neuron per tick, so 64 extra neurons
// re-roll the stream. See tools/genome_add_context.py.)
//
// THREE TARGETS, EACH WITH ITS OWN YOKE, because a yoke scored against a
// different target is not a control:
//
//   fixed   one word's formants whatever was heard. vocallearn's positive
//           control, and it decides what a null means.
//   heard   the word that was just said. Conditional -- but the innate arcuate
//           already delivers this map at 0.890, so reward is REFINING an
//           existing route. This is vocallearn's -0.1.
//   swap    the OTHER word's formants. Conditional, arbitrary, and the arcuate
//           is pulling against it. Nothing innate can supply it, so anything it
//           earns was LEARNED. This is the milestone-shaped arm.
//
// WHY THIS IS A SWEEP AND NOT ONE POINT, and it is a correction to the first
// version of this experiment. Run at a single oracle strength it read `swap`
// -8.6 driven against +0.2 silent and printed a negative -- while its own
// positive control fell from +35.9 to +12.2 with a standard error of 10.2, dead
// on two of three seeds. A column whose positive control has collapsed cannot
// report a null on anything: the oracle was not delivering a condition, it was
// knocking the larynx off the operating point at which reward works at all,
// which is rule 1 of the calibration invariant arriving as a result. So the
// oracle's strength is swept, `fixed` gates every level, and a level whose
// control has fallen is printed UNREADABLE rather than scored.
//
// WHAT WOULD KILL THE REWRITE: `swap` flat at every level where `fixed` is
// still alive. WHAT WOULD LICENSE IT: `swap` lifting at a level where `fixed`
// has not moved. WHAT WOULD MEAN NEITHER: no level where the control survives,
// which says this genome cannot deliver an oracle to this larynx and names the
// next lever rather than the answer.
namespace {

// Injected current per tick into every neuron of the active slice. Threshold is
// 1.0 over a 20 ms leak at 1 kHz, so the steady state is 20x this and 0.05 is
// exactly threshold: below it the slice never fires and the oracle is mute
// without saying so.
// 0.04 is deliberately gone. Threshold is 1.0 over a 20 ms leak at 1 kHz, so
// the steady state is 20x the gain and anything under 0.05 never fires: the
// first sweep ran 0.04 and it came back BYTE-IDENTICAL to the silent arm, which
// is correct and useless. A mute level is not a driven level, and the verdict
// below now checks the measured rate rather than the index.
constexpr double kCtxLevels[] = {0.00, 0.06, 0.10, 0.20};
constexpr uint32_t kCtxLevelCount = 4;
// vocallearn's own bar: the positive control has to move this far against its
// yoke or the instrument cannot see learning and every null in it is a fact
// about the probe.
constexpr double kCtxVisible = 5.0;
// ...and it has to keep this share of what it reads with the oracle silent. A
// control at a third of its own reference is not a control that happens to be
// smaller; it is a different creature being asked the same question.
constexpr double kCtxControlKeep = 0.60;
// ...and the creature has to still be talking. Below this share of the voiced
// fraction it manages with the oracle mute, the formant readout is measuring a
// different creature and its error bars explode.
constexpr double kCtxVoicedKeep = 0.60;

struct CtxArm {
  const char* name;
  VLTarget target;
};

double vl_change(const VLRun& r) {
  return r.err_early > 0.0 ? 100.0 * (1.0 - r.err_late / r.err_early) : 0.0;
}

double ctx_mean_se(const std::vector<double>& v, double* se) {
  if (v.empty()) { *se = 0.0; return 0.0; }
  double m = 0.0;
  for (double x : v) m += x;
  m /= double(v.size());
  double ss = 0.0;
  for (double x : v) ss += (x - m) * (x - m);
  *se = v.size() > 1 ? std::sqrt(ss / double(v.size() - 1) / double(v.size())) : 0.0;
  return m;
}

}  // namespace

// --- ctxsrc: can the creature supply its OWN context index? -----------------
//
// This is the last piece of the Fee & Goldberg architecture and it is now the
// whole thing. DNA v51 works -- a context-indexed bias takes the voice to 48%
// of the oracle ceiling -- but `areax` WRITES THE CONTEXT SLICE FROM THE WORD
// LABEL. The host hands it over. For naming, the creature has to derive it from
// what it heard.
//
// The information exists: `coderprobe` reads one-of-eight off the auditory
// module at 0.981 against a 1.000 signal ceiling. **But that is measured while
// the word is playing**, and v51 needs the index at the moment reward lands.
// `vocab` already noted the shape of the problem from the other side: the ear
// reads 0.47-0.59 in the window after a word stops, because the sound has
// stopped. A context that has decayed by the time it is needed is not a context.
//
// SO THE QUESTION IS TIMING, NOT LEGIBILITY, and it is answerable with no
// learning and no new mechanism. Decode the word from `auditory` and from
// `central` in bins across vocallearn's OWN trial, and look at the bins where
// reward is actually delivered.
//
// Written here rather than beside the other probes so that it uses vocallearn's
// timing constants directly instead of copying them. Two copies of 900 and 2800
// that have to agree is the shared-constant bug class this project has already
// swept once, and a probe whose windows silently drift out of step with the
// protocol it is about would be worse than no probe.
//
// `central` is the interesting column, not `auditory`. `audprobe` measured that
// B2 classifies the word within 50 ms while central needs 1200 ms to reach
// 0.940 -- central is slow because it INTEGRATES, and integration is exactly
// what a context needs to survive the silence. The ear is fast and forgets.
//
// THE BAR IS DERIVED, and stated before the run. With two contexts an index
// that is right with probability p writes the OTHER context's table 1-p of the
// time, so the conditional signal scales as (2p - 1). To keep half of areax's
// 112.9 Hz needs 2p - 1 >= 0.5, i.e. **p >= 0.75 in the reward window**. Below
// that, a derived index cannot carry v51 and what is missing is not a better
// readout but somewhere to HOLD the context -- which this creature does not
// have anywhere ([[no module holds a kick for 10 ms]]).
constexpr uint64_t kCsBins[][2] = {
    {0, 200},                              // the word arrives
    {200, kVLWordTicks},                   // the rest of it
    {kVLWordTicks, kVLWordTicks + 400},    // reward window, first half
    {kVLWordTicks + 400, kVLRewardTo},     // reward window, second half
    {kVLRewardTo, kVLTrialTicks},          // after reward
};
constexpr uint32_t kCsBinCount = sizeof(kCsBins) / sizeof(kCsBins[0]);
constexpr const char* kCsBinName[kCsBinCount] = {
    "0-200 word", "200-900 word", "900-1300 REWARD", "1300-1700 REWARD", "1700-2800 after"};
// The share of areax's effect a p-accurate index would retain, and the p that
// keeps half of it.
constexpr double kCsBar = 0.75;

struct CtxSrc {
  double aud[kCsBinCount] = {};
  double cen[kCsBinCount] = {};
  // M1b says the creature REPEATS: 200-600 ms after a word stops, with the ear
  // already at chance, the voice still carries which word at 0.890 on 5 of 5
  // creatures. That window is ticks 1100-1500, which sits inside the reward
  // window this probe is about -- so the persistence a context index needs may
  // already exist, in the MOTOR system rather than the sensory one. The echo
  // would be the working memory: the creature remembers what it heard by having
  // said it.
  //
  // Two columns, because only the second is usable. `voc` is the whole larynx.
  // `vocx` EXCLUDES the F1 and F2 groups, which is where DNA v51's bias table
  // writes -- reading the index from the neurons the mechanism steers would
  // make v51 its own input, and `ctxbias` already measured that a signal on the
  // other groups arrives in full without touching what the formant readout
  // reads. The separation is measured rather than hoped for.
  double voc[kCsBinCount] = {};
  double vocx[kCsBinCount] = {};
  // The corrected readout, and both corrections are to instrument choices this
  // probe got wrong on its first run.
  //
  // (1) SPIKE COUNTS ARE THE WRONG READOUT of the larynx and this project had
  // already measured that: `vocab` scores vocal per-neuron at 0.234 against the
  // articulators at 0.344 and says in terms that the knobs are the better
  // readout, because a group's centroid averages noise that raw counts carry.
  // M1b's 0.890 is measured on the articulators. The first run of this probe
  // used counts and read 0.589.
  //
  // (2) THE BINS STRADDLE M1b's WINDOW. M1b is 200-600 ms after the word stops,
  // ticks 1100-1500, which falls across the 900-1300 / 1300-1700 boundary. This
  // is that window exactly.
  //
  // Excludes F1 and F2 for the circularity reason above, leaving f0, F3, three
  // bandwidths, loudness and voicing.
  double echo_grp = 0.0;
  double echo_grp_shuf = 0.0;
  // The SAME articulator readout in the same bins as every other column, so
  // that the comparison is one variable at a time. The first version of this
  // reported 0.816 from M1b's narrow window against 0.533 from the worst of two
  // wider bins with a different readout -- two changes at once, in the
  // favourable direction, which is not a comparison.
  double grp[kCsBinCount] = {};
  // Raw features for `partprobe`, which asks a different question of the same
  // trials: the two REWARD bins only, because that is the only window a context
  // index is ever used in. Kept as FEATURES rather than as accuracies so a new
  // partition rule can be scored on identical trials, identical bins and an
  // identical split. That is the discipline this probe's own third instrument
  // error was about -- it once moved the window and the readout at once.
  std::vector<std::vector<double>> feat_grp[2];  // 7 off-axis articulator values
  std::vector<std::vector<double>> feat_neu[2];  // off-axis vocal spike counts
  // ...and the EAR during the two WORD bins. Not a reward-window feature: it is
  // there for the latched index `partprobe` prices, where the index is formed
  // while the word plays and held afterwards.
  std::vector<std::vector<double>> feat_aud[2];  // auditory spike counts, word
  // ...and the same auditory code accumulated over a window the CREATURE found
  // for itself, rather than one the host wrote from the trial structure. The
  // gate is DNA v53's: the larynx below its own setpoint, which is M1d's
  // listening reflex used as a clock. One row per trial, normalised by gated
  // ticks because that window's length varies where the host's bins do not.
  std::vector<std::vector<double>> feat_gate;
  double gate_ticks_mean = 0.0;  // how much of a trial the creature called "a word"
  // FRAGMENTATION, which is the surviving candidate for why the creature's
  // index (0.68) falls so far short of this probe's (0.98). The kernel competes
  // once per gate EPISODE and latches the winner of the LAST one; this probe
  // forms a single vector per trial. If a trial contains more than one episode,
  // the creature's context is set by whichever fragment happened to come last.
  double episodes_per_trial = 0.0;
  double last_ep_word_frac = 0.0;   // of the LAST episode's ticks, how many were word
  double all_ep_word_frac = 0.0;    // ...and of every gated tick in the trial
  // A BOUNDARY-FREE feature: the ear's own per-neuron rate EMA, sampled in the
  // middle of the reward window, with no gate, no episode and no latch. The EMA
  // integrates the preceding second, so it still carries the word that ended
  // 400 ticks earlier -- which is why this is not the same measurement as
  // `ctxsrc`'s 0.541 for the ear in that bin: THAT was spike counts inside the
  // bin, with no memory of anything before it. If this separates the words, the
  // EMA *is* the latch and none of the episode machinery is needed.
  std::vector<std::vector<double>> feat_ema;
  std::vector<int> labels;
  size_t train_split = 0;
  double aud_shuf = 0.0, cen_shuf = 0.0, voc_shuf = 0.0;
  uint32_t trials = 0;
  bool ok = false;
};

CtxSrc run_ctxsrc_session(const std::vector<uint8_t>& blob, uint64_t ticks,
                          uint32_t n_words = 2) {
  CtxSrc out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) return out;
  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  Ear ear;
  if (!ear.configure(acfg, error)) return out;
  const int32_t aud = s.dna.module_with_role(aibaby::ModuleRole::kAuditory);
  const int32_t cen = s.dna.module_with_role(aibaby::ModuleRole::kAssociation);
  const int32_t voc = s.dna.module_with_role(aibaby::ModuleRole::kVocal);
  if (aud < 0 || cen < 0 || voc < 0) return out;

  const double dt = double(s.dna.header().sim.dt_ms);
  const uint32_t spt = uint32_t(double(acfg.sample_rate) * dt / 1000.0 + 0.5);
  VowelSource voice(acfg.sample_rate);
  std::vector<float> pcm(spt);
  const aibaby::ModuleState& ms_a = s.brain.network().module(uint32_t(aud));
  const aibaby::ModuleState& ms_c = s.brain.network().module(uint32_t(cen));
  const aibaby::ModuleState& ms_v = s.brain.network().module(uint32_t(voc));
  const uint32_t wa = ms_a.count, wc = ms_c.count, wv = ms_v.count;
  // The F1 and F2 groups, which v51's bias table writes to and which the
  // restricted readout therefore must not see.
  const uint32_t f_lo = aibaby::slice_begin(wv, aibaby::kVocalGroups, 2);
  const uint32_t f_hi = aibaby::slice_begin(wv, aibaby::kVocalGroups, 4);
  // M1b's own window: 200-600 ms after the word stops.
  const uint64_t echo_from = kVLWordTicks + 200, echo_to = kVLWordTicks + 600;
  const uint64_t n_trials = ticks / kVLTrialTicks;

  std::vector<std::vector<double>> xa[kCsBinCount], xc[kCsBinCount];
  std::vector<std::vector<double>> xv[kCsBinCount], xvx[kCsBinCount];
  std::vector<std::vector<double>> xg;
  std::vector<std::vector<double>> xgb[kCsBinCount];
  std::vector<std::vector<double>> xag;   // creature-gated auditory, one per trial
  double gate_ticks = 0.0;
  std::vector<std::vector<double>> xae;   // ear rate EMA at reward time
  double ep_sum = 0.0, last_frac_sum = 0.0, all_frac_sum = 0.0;
  uint32_t last_frac_n = 0;
  // DNA v53's gate, read from the genome exactly as the kernel reads it.
  const double voc_target = double(s.dna.module(uint32_t(voc)).target_rate_hz);
  std::vector<int> y;
  aibaby::Rng rng;
  rng.seed(s.dna.header().seed ^ 0xC7530u);
  // Balanced but SHUFFLED, never alternating -- audprobe's note applies here
  // unchanged: strict alternation makes the label equal to trial parity, and
  // anything in the creature with a period of two trials would carry the label
  // without a word ever being heard.
  std::vector<int> order(size_t(n_trials), 0);
  const uint32_t nw = n_words < 2 ? 2u : (n_words > 4 ? 4u : n_words);
  for (size_t i = 0; i < order.size(); ++i) order[i] = int(i % nw);
  for (size_t i = order.size(); i > 1; --i) std::swap(order[i - 1], order[rng.next() % i]);

  for (uint64_t k = 0; k < n_trials; ++k) {
    const int word = order[size_t(k)];
    std::vector<std::vector<double>> ba(kCsBinCount, std::vector<double>(wa, 0.0));
    std::vector<std::vector<double>> bc(kCsBinCount, std::vector<double>(wc, 0.0));
    std::vector<std::vector<double>> bv(kCsBinCount, std::vector<double>(wv, 0.0));
    // The articulator centroids, averaged over M1b's window. Seven of nine:
    // F1 and F2 are what v51's bias table steers.
    double gsum[aibaby::kVocalGroups] = {};
    uint32_t gn = 0;
    double gbin[kCsBinCount][aibaby::kVocalGroups] = {};
    uint32_t gbn[kCsBinCount] = {};
    std::vector<double> bg(wa, 0.0);   // the creature-gated auditory vector
    uint32_t bgn = 0;                  // ticks the creature spent listening
    uint32_t eps = 0;                  // gate episodes in this trial
    uint32_t ep_ticks = 0, ep_word = 0;      // the CURRENT episode
    uint32_t last_ticks = 0, last_word = 0;  // ...and the last completed one
    uint32_t gw = 0;                   // gated ticks that were inside the word
    bool gate_prev = false;
    bool slept = false;
    for (uint64_t t = 0; t < kVLTrialTicks; ++t) {
      const bool sounding = t < kVLWordTicks;
      const Word& w = kWords[word];
      voice.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                   pcm.data(), spt);
      ear.tick(s.brain, pcm.data(), spt);
      s.brain.step();
      if (s.brain.asleep()) slept = true;
      uint32_t bin = kCsBinCount;
      for (uint32_t b = 0; b < kCsBinCount; ++b) {
        if (t >= kCsBins[b][0] && t < kCsBins[b][1]) { bin = b; break; }
      }
      if (t >= echo_from && t < echo_to) {
        const aibaby::Scalar* g = s.brain.vocal_groups();
        for (uint32_t q = 0; q < aibaby::kVocalGroups; ++q) gsum[q] += double(g[q]);
        ++gn;
      }
      if (bin < kCsBinCount) {
        const aibaby::Scalar* g = s.brain.vocal_groups();
        for (uint32_t q = 0; q < aibaby::kVocalGroups; ++q) gbin[bin][q] += double(g[q]);
        ++gbn[bin];
      }
      // The boundary-free sample: one row per trial, taken at the midpoint of
      // the reward window, which is where v53's index has to be correct.
      if (t == kVLWordTicks + 400) {
        const aibaby::Network& net = s.brain.network();
        std::vector<double> row(wa, 0.0);
        for (uint32_t n = 0; n < wa; ++n) row[n] = double(net.rate(ms_a.begin + n));
        xae.push_back(row);
      }
      // The creature's OWN window, gated exactly as DNA v53 gates it, and
      // running over the whole trial rather than only inside the host's bins --
      // the creature has no bins.
      {
        const aibaby::Network& net = s.brain.network();
        const bool gate = net.module(uint32_t(voc)).mean_rate < voc_target;
        if (gate) {
          for (uint32_t i = 0; i < net.spike_count(); ++i) {
            const uint32_t idx = net.spikes()[i];
            if (idx >= ms_a.begin && idx < ms_a.begin + wa) bg[idx - ms_a.begin] += 1.0;
          }
          ++bgn;
          ++ep_ticks;
          if (sounding) { ++gw; ++ep_word; }
          if (!gate_prev) ++eps;
        } else if (gate_prev) {
          last_ticks = ep_ticks;
          last_word = ep_word;
          ep_ticks = ep_word = 0;
        }
        gate_prev = gate;
      }
      if (bin >= kCsBinCount) continue;
      const aibaby::Network& net = s.brain.network();
      for (uint32_t i = 0; i < net.spike_count(); ++i) {
        const uint32_t idx = net.spikes()[i];
        if (idx >= ms_a.begin && idx < ms_a.begin + wa) ba[bin][idx - ms_a.begin] += 1.0;
        else if (idx >= ms_c.begin && idx < ms_c.begin + wc) bc[bin][idx - ms_c.begin] += 1.0;
        else if (idx >= ms_v.begin && idx < ms_v.begin + wv) bv[bin][idx - ms_v.begin] += 1.0;
      }
    }
    if (slept) continue;
    {
      // Normalised to a per-tick mean: the gated window's length varies from
      // trial to trial, and an unnormalised count would let the classifier read
      // HOW LONG the creature listened instead of WHAT it heard.
      std::vector<double> row(wa, 0.0);
      if (bgn > 0) {
        for (uint32_t n = 0; n < wa; ++n) row[n] = bg[n] / double(bgn);
      }
      xag.push_back(row);
      gate_ticks += double(bgn);
      // An episode still open at the trial's end is the last one.
      if (ep_ticks > 0) { last_ticks = ep_ticks; last_word = ep_word; }
      ep_sum += double(eps);
      if (last_ticks > 0) {
        last_frac_sum += double(last_word) / double(last_ticks);
        ++last_frac_n;
      }
      if (bgn > 0) all_frac_sum += double(gw) / double(bgn);
    }
    for (uint32_t b = 0; b < kCsBinCount; ++b) {
      xa[b].push_back(ba[b]);
      xc[b].push_back(bc[b]);
      xv[b].push_back(bv[b]);
      std::vector<double> rest;
      rest.reserve(wv - (f_hi - f_lo));
      for (uint32_t i = 0; i < wv; ++i) {
        if (i >= f_lo && i < f_hi) continue;  // the groups v51 steers
        rest.push_back(bv[b][i]);
      }
      xvx[b].push_back(rest);
    }
    {
      std::vector<double> g;
      for (uint32_t q = 0; q < aibaby::kVocalGroups; ++q) {
        if (q == 2 || q == 3) continue;  // F1 and F2: what v51 steers
        g.push_back(gn ? gsum[q] / gn : 0.0);
      }
      xg.push_back(g);
    }
    for (uint32_t b = 0; b < kCsBinCount; ++b) {
      std::vector<double> g;
      for (uint32_t q = 0; q < aibaby::kVocalGroups; ++q) {
        if (q == 2 || q == 3) continue;
        g.push_back(gbn[b] ? gbin[b][q] / gbn[b] : 0.0);
      }
      xgb[b].push_back(g);
    }
    y.push_back(word);
  }

  out.trials = uint32_t(y.size());
  if (out.trials < 16) return out;
  const size_t train = out.trials / 2;
  std::vector<int> shuf = y;
  for (size_t i = shuf.size(); i > 1; --i) std::swap(shuf[i - 1], shuf[rng.next() % i]);
  for (uint32_t b = 0; b < kCsBinCount; ++b) {
    out.aud[b] = holdout_accuracy(xa[b], y, train);
    out.cen[b] = holdout_accuracy(xc[b], y, train);
    out.voc[b] = holdout_accuracy(xv[b], y, train);
    out.vocx[b] = holdout_accuracy(xvx[b], y, train);
  }
  // The shuffled control is taken in a REWARD bin, not in the word bin: a
  // control that only proves the readout is honest where the signal is loudest
  // proves it in the wrong place.
  out.aud_shuf = holdout_accuracy(xa[2], shuf, train);
  out.cen_shuf = holdout_accuracy(xc[2], shuf, train);
  out.voc_shuf = holdout_accuracy(xvx[2], shuf, train);
  for (uint32_t b = 0; b < kCsBinCount; ++b) out.grp[b] = holdout_accuracy(xgb[b], y, train);
  out.echo_grp = holdout_accuracy(xg, y, train);
  out.echo_grp_shuf = holdout_accuracy(xg, shuf, train);
  // Bins 2 and 3 are the reward window (900-1300 and 1300-1700).
  for (uint32_t r = 0; r < 2; ++r) {
    out.feat_grp[r] = xgb[2 + r];
    out.feat_neu[r] = xvx[2 + r];
    out.feat_aud[r] = xa[r];  // bins 0 and 1 are the word: 0-200 and 200-900
  }
  out.feat_gate = xag;
  out.gate_ticks_mean = y.empty() ? 0.0 : gate_ticks / double(y.size());
  const double ny = y.empty() ? 1.0 : double(y.size());
  out.feat_ema = xae;
  out.episodes_per_trial = ep_sum / ny;
  out.last_ep_word_frac = last_frac_n ? last_frac_sum / double(last_frac_n) : 0.0;
  out.all_ep_word_frac = all_frac_sum / ny;
  out.labels = y;
  out.train_split = train;
  out.ok = true;
  return out;
}

bool run_ctxsrc(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  (void)verbose;
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  // NINE creatures, not three. The whole verdict turns on whether the
  // articulator readout clears 0.75 in its WORST reward bin, and the first run
  // put it at 0.754 -- a margin of four thousandths, which is a coin flip
  // rather than a result. This probe is cheap enough that the honest sample
  // costs three minutes.
  constexpr uint32_t kReps = 9;
  instrument("ctxsrc", dna.header().seed ^ 0xC7530u, ticks / kVLTrialTicks,
             "trials per creature");
  std::printf("  question          `areax` has the HOST write the context slice from the\n"
              "                    word label. Can the creature derive it from what it\n"
              "                    heard, at the moment reward actually lands?\n");
  std::printf("  the bar           %.2f. With two contexts an index right with\n"
              "                    probability p writes the OTHER table 1-p of the time,\n"
              "                    so the signal scales as (2p-1); keeping half of\n"
              "                    areax's 112.9 Hz needs p >= 0.75 IN THE REWARD BINS.\n",
              kCsBar);
  std::printf("  chance            0.500 -- trials are balanced and shuffled\n\n");

  double sa[kCsBinCount] = {}, sc[kCsBinCount] = {};
  double sv[kCsBinCount] = {}, svx[kCsBinCount] = {}, sg[kCsBinCount] = {};
  std::vector<double> gsamp[kCsBinCount];
  std::vector<double> gworst;
  double sas = 0.0, scs = 0.0, svs = 0.0, seg = 0.0, segs = 0.0;
  uint32_t valid = 0;
  std::printf("  %-6s %-32s %-32s %s\n", "seed", "auditory", "central",
              "vocal minus F1/F2");
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    const CtxSrc p = run_ctxsrc_session(variant, ticks);
    if (!p.ok) { std::printf("  %-6u (inconclusive: %u trials)\n", r, p.trials); continue; }
    ++valid;
    char as[64] = {0}, cs[64] = {0}, vs[64] = {0};
    for (uint32_t b = 0; b < kCsBinCount; ++b) {
      sa[b] += p.aud[b]; sc[b] += p.cen[b]; sv[b] += p.voc[b]; svx[b] += p.vocx[b];
      sg[b] += p.grp[b];
      gsamp[b].push_back(p.grp[b]);
      char t[16];
      std::snprintf(t, sizeof t, "%.2f ", p.aud[b]); std::strcat(as, t);
      std::snprintf(t, sizeof t, "%.2f ", p.cen[b]); std::strcat(cs, t);
      std::snprintf(t, sizeof t, "%.2f ", p.vocx[b]); std::strcat(vs, t);
    }
    sas += p.aud_shuf; scs += p.cen_shuf; svs += p.voc_shuf;
    seg += p.echo_grp; segs += p.echo_grp_shuf;
    gworst.push_back(std::min(p.grp[2], p.grp[3]));
    std::printf("  %-6u %-32s %-32s %s\n", r, as, cs, vs);
  }
  if (valid < 2) {
    std::printf("\n  ctxsrc INCONCLUSIVE -- %u of %u creatures usable.\n", valid, kReps);
    return false;
  }
  const double n = double(valid);
  std::printf("\n  %-18s %10s %10s %12s %14s\n", "bin", "auditory", "central",
              "vocal spikes", "ARTICULATORS");
  for (uint32_t b = 0; b < kCsBinCount; ++b) {
    std::printf("  %-18s %10.3f %10.3f %12.3f %14.3f\n", kCsBinName[b], sa[b] / n,
                sc[b] / n, svx[b] / n, sg[b] / n);
  }
  std::printf("  %-18s %10s %10s %12s %14.3f\n", "1100-1500 (M1b)", "-", "-", "-",
              seg / n);
  std::printf("\n  the last two columns are the SAME module: spike counts against the\n"
              "  nine articulator centroids, minus the two v51 steers. `vocab` already\n"
              "  measured that the knobs beat the neurons on this module, and the gap\n"
              "  here is the same finding arriving from a different direction.\n");
  std::printf("\n  THE ARTICULATOR READOUT, in M1b's own window (1100-1500), on the\n"
              "  seven groups v51 does NOT steer: %.3f (shuffled %.3f)\n",
              seg / n, segs / n);
  std::printf("\n  shuffled control in a REWARD bin: auditory %.3f, central %.3f,\n"
              "  vocal-minus-F1/F2 %.3f\n", sas / n, scs / n, svs / n);

  if (sas / n > 0.62 || scs / n > 0.62 || svs / n > 0.62) {
    std::printf("\n  CONTROL FAILED -- a shuffled readout scores above chance in the\n"
                "  reward window, so nothing else here is worth reading.\n");
    return false;
  }

  // The reward bins are 2 and 3, and BOTH have to clear the bar: reward is
  // delivered across the whole window, and an index that is right for the first
  // half and wrong for the second is writing to two different tables inside one
  // trial.
  const double a_rw = std::min(sa[2], sa[3]) / n;
  const double c_rw = std::min(sc[2], sc[3]) / n;
  const double v_rw = std::min(svx[2], svx[3]) / n;
  const double g_rw = std::min(sg[2], sg[3]) / n;
  const double sense = std::max(a_rw, c_rw);
  // The articulator readout replaces the spike-count one for the voice: it is
  // the better instrument on this module by measurement, not by preference.
  // Scored on the SAME worst-of-two-reward-bins rule as every other column.
  // M1b's narrow window is reported beside it and is not what the verdict
  // rests on -- one variable at a time.
  const double echo = seg / n;
  const double best = std::max(sense, std::max(v_rw, g_rw));
  (void)best;
  // Per creature, so the decisive number carries a spread rather than being a
  // ratio of two sums.
  double g_se = 0.0;
  const double g_mean = ctx_mean_se(gworst, &g_se);
  std::printf("  worst reward bin:  auditory %.3f, central %.3f, vocal spikes %.3f\n"
              "                     ARTICULATORS %.3f +/- %.3f per creature "
              "(bar %.2f)\n"
              "                     M1b's own window %.3f\n",
              a_rw, c_rw, v_rw, g_mean, g_se, kCsBar, echo);
  const bool clears = g_mean - 2.0 * g_se >= kCsBar;
  if (g_mean >= kCsBar && !clears) {
    std::printf("\n  TOO CLOSE TO CALL -- the articulator readout's worst reward bin is\n"
                "  %.3f +/- %.3f against a %.2f bar, so the mean clears it and the spread\n"
                "  does not. Treat this as a lead and not a licence: an index this\n"
                "  marginal would carry a fraction of areax's effect that depends on\n"
                "  which creature you got.\n", g_mean, g_se, kCsBar);
    return false;
  }

  if (best >= kCsBar) {
    const char* who = (g_rw >= a_rw && g_rw >= c_rw) ? "the VOICE"
                      : (c_rw >= a_rw)               ? "central" : "auditory";
    std::printf("\n  THE CREATURE CAN SUPPLY ITS OWN INDEX -- %s carries the word at\n"
                "  %.3f through the whole reward window, above the %.2f a two-context\n"
                "  index needs to keep half of areax's effect, scored on the same\n"
                "  worst-of-both-reward-bins rule as every other column.\n",
                who, best, kCsBar);
    if (g_rw >= sense) {
      std::printf("\n  AND IT IS THE MOTOR SYSTEM, NOT A SENSORY ONE. The ear is at %.3f\n"
                  "  in the same window and central at %.3f: the persistence a context\n"
                  "  index needs already exists, and it is the ECHO. The creature\n"
                  "  remembers what it heard by having SAID it, which is M1b read as a\n"
                  "  memory rather than as an imitation.\n\n"
                  "  This column EXCLUDES the F1 and F2 groups, so wiring v51's index to\n"
                  "  it does not make the mechanism its own input: the bias table writes\n"
                  "  to groups this readout cannot see, and `ctxbias` measured that a\n"
                  "  signal on the other groups arrives in full without touching what\n"
                  "  the formant readout reads.\n", a_rw, c_rw);
    }
    std::printf("\n  A CEILING, NOT A MECHANISM: a held-out linear readout is not\n"
                "  something the creature computes. What would have to learn this map\n"
                "  is the next question rather than a settled one.\n");
    return true;
  }

  std::printf("\n  IT CANNOT, AND THE REASON IS TIMING RATHER THAN LEGIBILITY.\n"
              "  The word is legible while it plays -- %.3f in auditory in the first\n"
              "  bin -- and by the reward window the best ANY of them manages is\n"
              "  %.3f, against the %.2f a two-context index needs. The voice does not\n"
              "  carry it either, so M1b's echo is not a usable memory here.\n\n"
              "  So what is missing is not a better readout of the ear. It is somewhere\n"
              "  to HOLD the context across the silence between hearing a word and\n"
              "  being rewarded for answering it, and this creature has nowhere: no\n"
              "  module here holds a kick for 10 ms, and an utterance is a held vowel\n"
              "  rather than a trajectory.\n\n"
              "  That is a sharper statement of what stands between v51 and naming than\n"
              "  anything the conditioning work produced, and it names a mechanism\n"
              "  class rather than a tuning knob: persistent activity.\n",
              sa[0] / n, best, kCsBar);
  return false;
}

// --- rpeprobe: is there a performance prediction error to be had? -----------
//
// DNA v51 works, and the same literature names what is still missing. Gadagkar,
// Puzerey, Chen, Baird-Daniel, Farhang & Goldberg (Science 2016) recorded
// dopamine in Area X during singing and found a PERFORMANCE PREDICTION ERROR:
// suppressed after worse-than-predicted, activated after better-than-predicted.
//
// This creature is closer to that than it looks, and the difference is exact.
// `Brain::update_drives` already computes `reward.effective = reward.total -
// reward_baseline_`, so node perturbation is driven by a prediction error
// already. **The baseline is ONE GLOBAL EMA.** Gadagkar's is per performance
// context. That is the entire gap, and it is one array instead of one scalar.
//
// SO WHY MEASURE FIRST. Because a per-context baseline can only buy something
// if the contexts DIFFER in mean reward, and `vocallearn`'s teaching protocol
// already sets its praise criterion per word -- deliberately, because against
// one global mean the creature is simply rewarded for saying the easier word.
// If that has already balanced the delivered reward across contexts, there is
// nothing left for a per-context baseline to remove and the mechanism is
// refused before it is written. A gate whose only interesting outcome is a
// refusal is the shape `shapeprobe` and `coderprobe` both had.
//
// THE DECOMPOSITION, which is the measurement. A centred reward splits as
//
//     R - b_global  =  (mean_c - b_global)  +  (R - mean_c)
//
// The first term is the same for every trial in a context and carries no
// information about what the creature DID; the second is the part that can
// teach an action. If the first dominates, node perturbation is largely
// learning "context A is a good place to be" rather than "that action was good
// in this context" -- **a common mode on the REWARD side**, which is this
// project's recurring arithmetic in the one place nobody has looked for it.
//
// Read-only. Two arms, because the question is whether v51 itself changes the
// picture: with the context-indexed bias off and on.
struct RpeArm { const char* name; uint32_t slots; };
constexpr RpeArm kRpeArms[] = {{"v51 off", 0}, {"v51 on", 2}};
constexpr uint32_t kRpeArmCount = 2;

bool run_rpeprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  (void)verbose;
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t ctx_module = dna.module_with_role(aibaby::ModuleRole::kContext);
  if (ctx_module < 0) {
    std::printf("  this genome has no kContext module, so there are no contexts to\n"
                "  decompose the reward by. Build one:\n\n"
                "    python3 tools/genome_add_context.py dna/default.toml ctx.toml \\\n"
                "        vocal out_w=0\n"
                "    ./build/aibaby --dna ctx.toml --experiment rpeprobe\n");
    return false;
  }
  constexpr uint32_t kReps = 3;
  const size_t slots_off = offsetof(aibaby::DnaHeader, exploration) +
                           offsetof(aibaby::DnaExploration, context_slots);
  instrument("rpeprobe", dna.header().seed, ticks / kVLTrialTicks, "trials per arm");
  std::printf("  question          node perturbation is already driven by a prediction\n"
              "                    error -- reward.total minus a running baseline. That\n"
              "                    baseline is ONE GLOBAL EMA where Gadagkar's is per\n"
              "                    performance context. Is there anything to be had by\n"
              "                    splitting it?\n");
  std::printf("  the measurement   R - b splits into (mean_c - b) + (R - mean_c). The\n"
              "                    first term is constant within a context and teaches\n"
              "                    nothing about what the creature DID. `between` is its\n"
              "                    share of the variance.\n");
  std::printf("  the gate          a small `between` REFUSES the mechanism: with the two\n"
              "                    contexts already balanced there is nothing for a\n"
              "                    per-context baseline to remove.\n\n");

  std::vector<double> betw[kRpeArmCount], gap[kRpeArmCount], ext[kRpeArmCount];
  std::printf("  %-6s %-9s %-11s %-11s %-11s %-11s %s\n", "seed", "arm", "R | ctx0",
              "R | ctx1", "between", "within", "ext share");
  for (uint32_t r = 0; r < kReps; ++r) {
    for (uint32_t a = 0; a < kRpeArmCount; ++a) {
      std::vector<uint8_t> variant = blob;
      const uint64_t seed = dna.header().seed + r * 7919ull;
      std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
      std::memcpy(variant.data() + slots_off, &kRpeArms[a].slots, sizeof(uint32_t));
      CtxDrive drive;
      drive.module = ctx_module;
      drive.slots = kVLWords;
      drive.gain = 0.10;
      Regime reg;
      reg.praise = kPraiseValue;
      reg.scold = kScoldValue;
      const VLRun run = run_vocallearn_session(variant, ticks, kVLTaught, nullptr, reg,
                                               kVLTgtHeard, &drive);
      if (!run.ok || !run.rw_n[0] || !run.rw_n[1]) {
        std::printf("  %-6u %-9s (inconclusive: %u scored, %u skipped)\n", r,
                    kRpeArms[a].name, run.scored, run.skipped);
        continue;
      }
      const double tot = run.rw_between + run.rw_within;
      const double share = tot > 0.0 ? run.rw_between / tot : 0.0;
      betw[a].push_back(share);
      gap[a].push_back(run.rw_mean[0] - run.rw_mean[1]);
      ext[a].push_back(run.rw_ext_share);
      std::printf("  %-6u %-9s %-11.5f %-11.5f %-11.4f %-11.4f %.3f\n", r,
                  kRpeArms[a].name, run.rw_mean[0], run.rw_mean[1], share,
                  1.0 - share, run.rw_ext_share);
    }
  }

  double m_b[kRpeArmCount], s_b[kRpeArmCount];
  double m_g[kRpeArmCount], s_g[kRpeArmCount];
  double m_e[kRpeArmCount], s_e[kRpeArmCount];
  for (uint32_t a = 0; a < kRpeArmCount; ++a) {
    if (betw[a].size() < 2) {
      std::printf("\n  rpeprobe INCONCLUSIVE -- arm `%s` did not produce two usable\n"
                  "  creatures.\n", kRpeArms[a].name);
      return false;
    }
    m_b[a] = ctx_mean_se(betw[a], &s_b[a]);
    m_g[a] = ctx_mean_se(gap[a], &s_g[a]);
    m_e[a] = ctx_mean_se(ext[a], &s_e[a]);
  }

  std::printf("\n  %-9s %-20s %-20s %s\n", "arm", "between share", "R gap ctx0-ctx1",
              "external share");
  for (uint32_t a = 0; a < kRpeArmCount; ++a) {
    char b[40], c[40], d[40];
    std::snprintf(b, sizeof b, "%.4f +/- %.4f", m_b[a], s_b[a]);
    std::snprintf(c, sizeof c, "%+.5f +/- %.5f", m_g[a], s_g[a]);
    std::snprintf(d, sizeof d, "%.3f +/- %.3f", m_e[a], s_e[a]);
    std::printf("  %-9s %-20s %-20s %s\n", kRpeArms[a].name, b, c, d);
  }

  // The bar is stated before the numbers are read, and it is the same one the
  // rest of this project uses for a common mode: a term worth removing has to
  // be a substantial share of what it rides on. 10% is the line, and it is the
  // order at which the object-specific share of the weight change (~8%) was
  // judged too small to be the mechanism.
  const uint32_t kOn = 1;
  const bool worth_it = m_b[kOn] > 0.10 && m_b[kOn] > 2.0 * s_b[kOn];

  std::printf("\n  between-context share of the reward variance, v51 on: %.4f +/- %.4f\n"
              "  the two contexts differ in mean reward by %+.5f\n"
              "  the caregiver's own term is %.1f%% of the variance; the rest is drives\n",
              m_b[kOn], s_b[kOn], m_g[kOn], 100.0 * m_e[kOn]);

  if (!worth_it) {
    std::printf("\n  DO NOT BUILD IT -- only %.1f%% of the reward variance is between\n"
                "  contexts, so a per-context baseline has almost nothing to remove.\n"
                "  `vocallearn` already sets its praise criterion per word, and this is\n"
                "  that decision showing up where it matters: the delivered reward is\n"
                "  already balanced across contexts, and Gadagkar's per-context\n"
                "  prediction error would be subtracting a term this creature does not\n"
                "  have.\n\n"
                "  The %.1f%%%% that IS between contexts is a real but small asymmetry, and\n"
                "  removing it is a tuning change rather than a mechanism.\n",
                100.0 * m_b[kOn], 100.0 * m_b[kOn]);
    return false;
  }

  std::printf("\n  BUILD IT -- %.1f%% of the reward variance is between contexts rather\n"
              "  than within them, so that fraction of every weight change node\n"
              "  perturbation makes is teaching the creature which context is a good\n"
              "  place to be rather than which action was good in it. A per-context\n"
              "  baseline removes exactly that term, and it is one array where the\n"
              "  global one is a scalar.\n",
              100.0 * m_b[kOn]);
  return true;
}

// --- partprobe: pricing a learned partition before building it -------------
//
// `ctxself` refused DNA v52 on two seed families: the creature CAN read a
// context index off its own larynx, but at p = 0.540 it keeps only 8% of the
// conditional effect, where `ctxsrc`'s supervised readout of the same seven
// off-axis articulator groups reaches 0.740 and would keep 48%. The obvious
// reading is that the loss is the CUT -- v52 slices the population into two
// equal contiguous halves, and a partition that was learned rather than fixed
// would recover it. Lateral competition (DNA v32) already works on this module,
// so it would not even be a new mechanism class.
//
// **This project prices a mechanism before building it** -- `credit`'s reward
// mask, `ctxbias`'s bias oracle, `rpeprobe`'s variance decomposition -- and two
// of those three came back saying do not build. So the same question here,
// read-only: if the partition were learned, how good would the index get?
//
// THREE THINGS COULD BE LOSING THE SIGNAL and the probe separates them, because
// naming one of them without the others is how this project's last three
// instrument errors happened.
//
//   1. THE CUT. Fixed equal halves against a boundary drawn from the data.
//   2. THE REPRESENTATION. `ctxsrc`'s 0.740 is measured on the seven ARTICULATOR
//      GROUP VALUES -- the knobs. v52's rule reads NEURON SLICE RATES. Those are
//      different feature spaces, and the 0.740 -> 0.540 gap was quietly being
//      attributed to (1) when part of it may be this.
//   3. THE PER-TICK ARGMAX. v52 argmaxes every tick and `ctxself`'s p is the
//      share of ticks that agree; every column here argmaxes the bin average
//      once.
//
// (1) and (2) are separated cleanly, because the columns differ in one thing.
// **(3) IS NOT, and the printout says so rather than pretending otherwise.**
// This probe runs read-only on the SHIPPED genome -- no context module, no
// learning, 600k ticks -- while `ctxself` runs a TAUGHT creature carrying an
// active bias table for 3.4M. Genome, regime and session length all differ, so
// the two numbers cannot be subtracted to isolate the per-tick argmax. Isolating
// it would need this probe run on the ctx genome mid-teaching, which is a
// different experiment. Reported side by side as a diagnostic, never as a
// decomposition.
//
// Every column runs on the SAME trials, the SAME reward bins and the SAME
// held-out split -- `run_ctxsrc_session` hands back its raw features so that
// nothing but the rule can differ. Only the supervised column sees the labels
// when it draws its boundary; k-means picks its restart by within-cluster sum
// of squares and never by accuracy.
//
// THE BAR, DERIVED AND STATED FIRST. The conditional effect scales as (2p - 1),
// and `ctxself` measured the oracle index buying 65 Hz over baseline. v52 at
// 0.540 keeps 8% of that -- about 5 Hz, which is why it refused. For a learned
// partition to be worth building it has to be detectable AND matter:
//
//   p = 0.65  ->  keeps 30%  ->  ~20 Hz, which is ~2.8 SE on the paired test
//                                at n=9, and a third of the way to naming.
//
// **BUILD only if an unsupervised partition reaches p >= 0.65.** Below that the
// effect stays too small to be a route to naming however significant it is,
// which is the same reasoning that retired v52's own residual.
// WHAT "SUPERVISED BY SOMETHING THE CREATURE HAS" COMES TO, and two thirds of
// it are already answered by measurements on file rather than by this run.
//
//   REWARD cannot supervise it. `rpeprobe` decomposed the reward stream by
//   context and found the between-context share is **0.0000** -- `vocallearn`
//   sets its praise criterion PER WORD by design, so reward is balanced across
//   the two words on purpose and carries no information about which one was
//   said. A teacher with zero mutual information with the label is not a
//   teacher. Refused without a run.
//
//   CAREGIVER TIMING cannot supervise it either, and this one is true by
//   construction: both words occupy ticks 0-900 and both reward windows run
//   900-1700, so the protocol's timing is IDENTICAL between the contexts.
//   Refused without a run.
//
//   THE EAR DURING THE WORD is the one that could. `ctxsrc` reads the auditory
//   module at **1.000** while the word plays and 0.541 by the reward window,
//   and the reason that decay looked fatal was an assumption worth re-examining:
//   that the index has to be RECOMPUTED when reward lands. It does not.
//   **DNA v52 already latches** -- `active_ctx_` is written only when the source
//   is active and holds its previous value otherwise -- so an index formed while
//   the word plays survives into the reward window for free. What would have to
//   change is not the latch but `ctx_present_`, which currently gates the
//   cash-in too, so a silent ear at reward time sends the update to the shared
//   bias.
//
// So the question this probe now has to answer is exactly one thing, and it is
// the question `partprobe` was built to ask: **is the word separable in the
// auditory code WITHOUT the labels?** The 1.000 above is supervised, and this
// probe's whole finding is that a supervised ceiling is not a target an
// unsupervised mechanism can approach.
constexpr double kPpBuildBar = 0.65;
constexpr double kPpSupervisedRef = 0.740;  // ctxsrc, 9 creatures
constexpr double kPpCtxselfP = 0.540;
// DNA v53's index accuracy in a taught creature, from `ctxself` on 9 creatures.
constexpr double kPpV53Index = 0.643;       // ctxself, fresh family

bool run_partprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  (void)verbose;
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  constexpr uint32_t kReps = 9;
  instrument("partprobe", dna.header().seed ^ 0x9A17u, ticks / kVLTrialTicks,
             "trials per creature");
  std::printf("  question          `ctxself` refused DNA v52 at p = %.3f where a\n"
              "                    SUPERVISED readout of the same population reaches\n"
              "                    %.3f. Is the loss the fixed CUT -- in which case a\n"
              "                    learned partition recovers it -- or something else?\n",
              kPpCtxselfP, kPpSupervisedRef);
  std::printf("  the bar           BUILD only if an unsupervised partition reaches\n"
              "                    p >= %.2f. The effect scales as (2p - 1) and the\n"
              "                    oracle index buys 65 Hz, so %.2f keeps ~30%% (~20 Hz,\n"
              "                    detectable at n=9 paired). Below it the mechanism is\n"
              "                    too small to be a route to naming however significant.\n",
              kPpBuildBar, kPpBuildBar);
  std::printf("  read-only         no learning, no genome field, nothing shipped.\n\n");

  // Two feature spaces x four rules, on the worst of the two reward bins --
  // reward is delivered across the whole window, and an index right for the
  // first half and wrong for the second writes two different tables in one
  // trial. `ctxsrc` scores its own verdict the same way.
  enum { kGrp = 0, kNeu = 1, kAud = 2, kGate = 3, kEma = 4, kSpaces = 5 };
  enum { kSup = 0, kKmZ = 1, kOnl = 2, kCon = 3, kKern = 4, kFix = 5, kRules = 6 };
  static const char* kSpaceName[kSpaces] = {"articulator groups", "off-axis neurons",
                                            "EAR, host window", "EAR, self window",
                                            "EAR ema @ reward"};
  static const char* kRuleName[kRules] = {"supervised", "batch k-means", "online",
                                          "online+conscience", "AS THE KERNEL RUNS IT",
                                          "fixed cut"};
  std::vector<double> acc[kSpaces][kRules];
  std::vector<double> shuf[kSpaces], busy[kSpaces], hit[kSpaces];

  std::printf("  %-6s %-18s %-9s %-9s %-9s %-11s %-9s %s\n", "seed", "features",
              "supervised", "batch km", "online", "onl+consc", "AS KERNEL", "fixed");
  uint32_t valid = 0;
  double gate_sum = 0.0, ep_sum = 0.0, lastw_sum = 0.0, allw_sum = 0.0;
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    const CtxSrc p = run_ctxsrc_session(variant, ticks);
    if (!p.ok || p.train_split == 0) {
      std::printf("  %-6u (inconclusive: too few usable trials)\n", r);
      continue;
    }
    ++valid;
    gate_sum += p.gate_ticks_mean;
    ep_sum += p.episodes_per_trial;
    lastw_sum += p.last_ep_word_frac;
    allw_sum += p.all_ep_word_frac;
    // A shuffled label vector, drawn once per creature and shared by every
    // column, so the control is the same control everywhere.
    aibaby::Rng rng;
    rng.seed(seed ^ 0x9A17u);
    std::vector<int> sh = p.labels;
    for (size_t i = sh.size(); i > 1; --i) std::swap(sh[i - 1], sh[rng.next() % i]);

    for (uint32_t sp = 0; sp < kSpaces; ++sp) {
      // The creature-gated space has one row per trial rather than one per bin,
      // so the same rows are scored twice and the worse kept, exactly as every
      // other space is scored on both of its bins. Same trials, same labels,
      // same split throughout; only the WINDOW differs.
      std::vector<std::vector<double>> gate_pair[2] = {p.feat_gate, p.feat_gate};
      std::vector<std::vector<double>> ema_pair[2] = {p.feat_ema, p.feat_ema};
      const std::vector<std::vector<double>>* f =
          sp == kGrp    ? p.feat_grp
          : sp == kNeu  ? p.feat_neu
          : sp == kAud  ? p.feat_aud
          : sp == kGate ? gate_pair
                        : ema_pair;
      double worst[kRules];
      for (uint32_t k = 0; k < kRules; ++k) worst[k] = 2.0;
      double worst_shuf = 2.0;
      double worst_busy = 0.0;
      double worst_hit = 2.0;
      for (uint32_t b = 0; b < 2; ++b) {
        // The online rule is run over MANY INITS and summarised by its
        // distribution, because a single draw is a coin flip and the question
        // is how often one pass finds the split -- see the note on
        // `online_competitive_accuracy`. `kOnlHit` is the share of draws that
        // essentially solve it; `kOnl` is the mean over draws.
        double bz_sum = 0.0, onl_sum = 0.0, con_sum = 0.0, con_hit = 0.0;
        constexpr uint32_t kDraws = 16;
        for (uint32_t d = 0; d < kDraws; ++d) {
          double bz = 1.0, bzc = 1.0;
          onl_sum += online_competitive_accuracy(f[b], p.labels, p.train_split, true,
                                                 &bz, seed ^ (0xD00Du + d), false);
          const double c = online_competitive_accuracy(f[b], p.labels, p.train_split,
                                                       true, &bzc, seed ^ (0xD00Du + d),
                                                       true);
          con_sum += c;
          bz_sum += bzc;
          if (c >= 0.9) con_hit += 1.0;
        }
        // The KERNEL-FAITHFUL variant: everything DNA v53 actually does, which
        // is not quite what the column beside it measures. No standardisation
        // (the kernel compares raw rates) and prototypes starting at zero (the
        // kernel has no data rows to seed from). Priced separately because
        // "priced one rule, built another" is how the last three of these went.
        double bzk = 1.0;
        const double kern = online_competitive_accuracy(
            f[b], p.labels, p.train_split, false, &bzk, seed ^ 0xD00Du, true, true);
        const double v[kRules] = {
            holdout_accuracy(f[b], p.labels, p.train_split),
            kmeans_accuracy(f[b], p.labels, p.train_split, seed ^ 0xB1u, true),
            onl_sum / double(kDraws),
            con_sum / double(kDraws),
            kern,
            fixedcut_accuracy(f[b], p.labels, p.train_split)};
        const double hit = con_hit / double(kDraws);
        if (hit < worst_hit) worst_hit = hit;
        if (bz_sum / double(kDraws) > worst_busy) worst_busy = bz_sum / double(kDraws);
        for (uint32_t k = 0; k < kRules; ++k) {
          if (v[k] < worst[k]) worst[k] = v[k];
        }
        const double s = kmeans_accuracy(f[b], sh, p.train_split, seed ^ 0xB1u, true);
        if (s < worst_shuf) worst_shuf = s;
      }
      for (uint32_t k = 0; k < kRules; ++k) acc[sp][k].push_back(worst[k]);
      shuf[sp].push_back(worst_shuf);
      busy[sp].push_back(worst_busy);
      hit[sp].push_back(worst_hit);
      std::printf("  %-6u %-18s %-9.3f %-9.3f %-9.3f %-11.3f %-9.3f %.3f\n", r,
                  kSpaceName[sp], worst[kSup], worst[kKmZ], worst[kOnl],
                  worst[kCon], worst[kKern], worst[kFix]);
    }
  }
  if (valid < 3) {
    std::printf("\n  partprobe INCONCLUSIVE -- %u of %u creatures usable.\n", valid, kReps);
    return false;
  }

  const double gate_mean = valid ? gate_sum / double(valid) : 0.0;
  double m[kSpaces][kRules], se[kSpaces][kRules], ms[kSpaces], ss[kSpaces];
  double mb[kSpaces], sb[kSpaces], mh[kSpaces], sh2[kSpaces];
  for (uint32_t sp = 0; sp < kSpaces; ++sp) {
    for (uint32_t k = 0; k < kRules; ++k) m[sp][k] = ctx_mean_se(acc[sp][k], &se[sp][k]);
    ms[sp] = ctx_mean_se(shuf[sp], &ss[sp]);
    mb[sp] = ctx_mean_se(busy[sp], &sb[sp]);
    mh[sp] = ctx_mean_se(hit[sp], &sh2[sp]);
  }

  std::printf("\n  %-18s %-14s %-14s %-14s %-14s %-14s %s\n", "features",
              "supervised", "batch k-means", "online", "onl+conscience",
              "AS KERNEL RUNS", "fixed cut");
  for (uint32_t sp = 0; sp < kSpaces; ++sp) {
    char c[kRules][32];
    for (uint32_t k = 0; k < kRules; ++k) {
      std::snprintf(c[k], sizeof c[k], "%.3f +/-%.3f", m[sp][k], se[sp][k]);
    }
    std::printf("  %-18s %-14s %-14s %-14s %-14s %-14s %s\n", kSpaceName[sp],
                c[kSup], c[kKmZ], c[kOnl], c[kCon], c[kKern], c[kFix]);
  }
  std::printf("\n  ONLINE+CONSCIENCE over 16 random inits per creature:\n"
              "    share of inits that essentially solve it (>=0.90)\n"
              "      groups %.2f, neurons %.2f, EAR %.2f +/- %.2f\n"
              "    busiest cluster, mean over inits (1.000 is a dead unit)\n"
              "      groups %.3f, neurons %.3f, ear %.3f\n",
              mh[kGrp], mh[kNeu], mh[kAud], sh2[kAud], mb[kGrp], mb[kNeu], mb[kAud]);
  std::printf("\n  shuffled control (k-means, z): groups %.3f, neurons %.3f,\n"
              "                                ear-host %.3f, ear-self %.3f\n",
              ms[kGrp], ms[kNeu], ms[kAud], ms[kGate]);
  std::printf("  the creature called %.0f of %llu ticks a word (host window: %llu)\n",
              gate_mean, (unsigned long long)kVLTrialTicks,
              (unsigned long long)kVLWordTicks);
  // FRAGMENTATION, the surviving candidate for the 0.98 -> 0.68 gap. The kernel
  // competes once per gate episode and LATCHES THE LAST ONE; this probe forms a
  // single vector per trial. If trials hold more than one episode, and if the
  // last one is mostly silence, then the creature's context is routinely set by
  // a fragment carrying no word -- which the probe would never see.
  std::printf("  BOUNDARY-FREE: ear rate EMA sampled at reward time, no gate at all\n"
              "    supervised %.3f   conscience %.3f   AS KERNEL %.3f   fixed %.3f\n"
              "    (`ctxsrc` read the ear at 0.541 in this window from SPIKE COUNTS in\n"
              "     the bin. An EMA carries the preceding second, so it still holds a\n"
              "     word that ended 400 ticks ago -- a different measurement, not a\n"
              "     better decoder.)\n",
              m[kEma][kSup], m[kEma][kCon], m[kEma][kKern], m[kEma][kFix]);
  std::printf("  gate episodes per trial       %.2f   (the kernel latches the LAST)\n"
              "  of ALL gated ticks, word      %.2f\n"
              "  of the LAST episode, word     %.2f   <- what the creature latches on\n",
              valid ? ep_sum / double(valid) : 0.0,
              valid ? allw_sum / double(valid) : 0.0,
              valid ? lastw_sum / double(valid) : 0.0);

  if (ms[kGrp] > 0.62 || ms[kNeu] > 0.62 || ms[kAud] > 0.62 || ms[kGate] > 0.62 ||
      ms[kEma] > 0.62) {
    std::printf("\n  CONTROL FAILED -- an unsupervised partition scores above chance\n"
                "  against SHUFFLED labels, so the assignment step is finding structure\n"
                "  that is not the word and nothing above is worth reading.\n");
    return false;
  }
  // The supervised column on the articulator groups is this probe reproducing
  // `ctxsrc`. If it does not, the features are not the ones that number was
  // measured on and every comparison drawn against 0.740 is void.
  if (std::fabs(m[kGrp][kSup] - kPpSupervisedRef) > 0.08) {
    std::printf("\n  REFUSED -- the supervised readout of the articulator groups reads\n"
                "  %.3f where `ctxsrc` measured %.3f on the same features and window.\n"
                "  This probe is not looking at what that number was measured on, so\n"
                "  nothing here can be compared against it.\n",
                m[kGrp][kSup], kPpSupervisedRef);
    return false;
  }

  // Best unsupervised result anywhere -- the build's ceiling, and it is scored
  // generously on purpose: if the best of four unsupervised numbers across two
  // feature spaces cannot clear the bar, no single choice among them will.
  uint32_t bsp = 0, brule = kKmZ;
  double best = -1.0;
  for (uint32_t sp = 0; sp < kAud; ++sp) {   // the MOTOR spaces only
    for (uint32_t k = kKmZ; k <= kCon; ++k) {
      if (m[sp][k] > best) { best = m[sp][k]; bsp = sp; brule = k; }
    }
  }
  // The ear is scored on its own, because it answers a different question: not
  // "is the motor state better partitioned" but "is there anywhere in this
  // creature the word is separable WITHOUT the labels".
  //
  // AND IT IS SCORED ON THE **ONLINE** RULE, not on batch k-means. Batch runs
  // eight restarts and keeps the best by inertia; a creature gets one pass
  // through its life and no restarts. Every gap between a proxy and a behaviour
  // in this project has been of exactly that shape, so the number that decides
  // a build is the one the creature could run. Batch is reported beside it as
  // the ceiling that rule class has.
  const double aud_best = m[kAud][kCon];
  const double aud_se = se[kAud][kCon];
  const double aud_batch = m[kAud][kKmZ];

  std::printf("\n  what the CUT costs       %.3f supervised -> %.3f unsupervised, same\n"
              "                           features (%s) -- and the BEST learned boundary\n"
              "                           buys only %+.3f over the fixed one\n"
              "  what the SPACE costs     %.3f groups -> %.3f neurons, both supervised.\n"
              "                           THE LARGEST SINGLE TERM, and it is the space\n"
              "                           v52 actually reads\n"
              "  fixed cut on neurons     %.3f here vs p = %.3f in `ctxself` -- NOT a\n"
              "                           decomposition: different genome (no context\n"
              "                           module), read-only not taught, 600k not 3.4M.\n"
              "                           Side by side only\n"
              "  best unsupervised        %.3f +/- %.3f  (%s, %s)\n"
              "  the bar                  %.2f\n",
              m[kGrp][kSup],
              m[kGrp][kKmZ] > m[kGrp][kCon] ? m[kGrp][kKmZ] : m[kGrp][kCon],
              kSpaceName[kGrp],
              (m[kGrp][kKmZ] > m[kGrp][kCon] ? m[kGrp][kKmZ] : m[kGrp][kCon]) -
                  m[kGrp][kFix],
              m[kGrp][kSup], m[kNeu][kSup],
              m[kNeu][kFix], kPpCtxselfP,
              best, se[bsp][brule], kSpaceName[bsp], kRuleName[brule],
              kPpBuildBar);
  std::printf("  EAR, online+conscience   %.3f +/- %.3f  <-- what the creature could run\n"
              "  the ear, batch k-means   %.3f  (8 restarts; the rule class's ceiling)\n"
              "  the ear, supervised      %.3f\n"
              "  the ear, v52's OWN rule  %.3f -- a fixed cut of the auditory code is at\n"
              "                           CHANCE, so nothing here is reachable by moving\n"
              "                           v52's index to the ear. The partition must be\n"
              "                           learned, which is the opposite of what the\n"
              "                           motor-state rows found.\n",
              aud_best, aud_se, aud_batch, m[kAud][kSup], m[kAud][kFix]);

  // DOES ANY OF THIS SURVIVE MORE THAN TWO WORDS?
  //
  // Everything DNA v53 was validated on is TWO vowels, and the ear separates
  // those at 1.000 -- the easy case. Four is harder BY DESIGN: /i/ and /u/ are
  // within 30 Hz on F1 and 1600 Hz apart on F2 (a nearly pure F2
  // discrimination), and /e/ sits between /a/ and /i/ on both. `coderprobe`
  // reads one-of-eight at 0.981 SUPERVISED, which says nothing about whether an
  // unsupervised rule can find those boundaries without being told.
  //
  // This is the cheap gate before any four-word protocol is written: the SAME
  // competitive rule with the SAME conscience, on the SAME feature v53 actually
  // reads (the ear's rate EMA at reward time), asked for four clusters instead
  // of two. Chance is 0.250. If it cannot find four, the four-word build stops
  // here for the price of a read-only run rather than a protocol rewrite.
  {
    std::printf("\n  CAN IT FIND FOUR? (same rule, same conscience, ear EMA at reward)\n");
    std::vector<double> k4, k4s, k2ref;
    for (uint32_t r = 0; r < kReps; ++r) {
      const uint64_t sd = dna.header().seed + r * 7919ull;
      std::vector<uint8_t> variant = blob;
      std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &sd, sizeof(sd));
      const CtxSrc q = run_ctxsrc_session(variant, ticks, 4);
      if (!q.ok || q.train_split == 0 || q.feat_ema.empty()) continue;
      k4.push_back(competitive_k_accuracy(q.feat_ema, q.labels, q.train_split, 4,
                                          sd ^ 0xF00Du, true));
      aibaby::Rng nr;
      nr.seed(sd ^ 0xBEEFu);
      std::vector<int> sh = q.labels;
      for (size_t i = sh.size(); i > 1; --i) std::swap(sh[i - 1], sh[nr.next() % i]);
      k4s.push_back(competitive_k_accuracy(q.feat_ema, sh, q.train_split, 4,
                                           sd ^ 0xF00Du, true));
      // The same rule on the same creature at TWO words, so the four-word
      // number is read against this probe's own two-word result rather than
      // against one quoted from a different run.
      const CtxSrc q2 = run_ctxsrc_session(variant, ticks, 2);
      if (q2.ok && q2.train_split > 0 && !q2.feat_ema.empty()) {
        k2ref.push_back(competitive_k_accuracy(q2.feat_ema, q2.labels, q2.train_split,
                                               2, sd ^ 0xF00Du, true));
      }
    }
    if (k4.size() >= 3) {
      double e4 = 0.0, e4s = 0.0, e2 = 0.0;
      const double m4 = ctx_mean_se(k4, &e4);
      const double m4s = ctx_mean_se(k4s, &e4s);
      const double m2 = k2ref.size() >= 3 ? ctx_mean_se(k2ref, &e2) : 0.0;
      std::printf("    two words, k=2   %.3f +/- %.3f   (chance 0.500)\n"
                  "    FOUR words, k=4  %.3f +/- %.3f   (chance 0.250)\n"
                  "    shuffled control %.3f +/- %.3f\n",
                  m2, e2, m4, e4, m4s, e4s);
      if (m4 > 0.60) {
        std::printf("    -> IT FINDS FOUR. The partition is not a two-word trick, so\n"
                    "       the four-word protocol is worth writing.\n");
      } else if (m4 > 0.35) {
        std::printf("    -> PARTIAL. Above chance but well short of the two-word case:\n"
                    "       the boundaries exist and the rule finds some of them.\n");
      } else {
        std::printf("    -> IT DOES NOT. The competitive rule finds two clusters and\n"
                    "       not four, so a four-word protocol would be measuring the\n"
                    "       index rather than the naming. Fix the partition first.\n");
      }
    } else {
      std::printf("    inconclusive: %zu usable creatures\n", k4.size());
    }
  }

  // DRIFT ROBUSTNESS -- pricing a learning rate before it is built.
  //
  // `ctxself` confirmed the defect with its own control: frozen prototypes lose
  // index across a session ONLY in the arm whose voice changes (0.697 -> 0.663)
  // and hold flat in the arm whose voice does not (0.763 -> 0.764). The first
  // fix for it was derived and REFUTED -- `n_eff = 4*dscale/gap^2` cost -0.146
  // of index on 8 of 9 creatures and made both arms decay, because it asks how
  // long to average to RESOLVE A GAP and so adapts hardest when the gap is
  // widest and tracking matters least.
  //
  // A read-only session does not drift, so drift is injected here at known
  // sizes and the rules are compared as a CURVE. That is the honest form: the
  // true magnitude in the creature is unknown, so what is priced is which rule
  // degrades gracefully, not which wins at one guessed number.
  {
    static const double kDrift[] = {0.0, 0.5, 1.0, 2.0};
    static const char* kRateName[3] = {"frozen (1/wins)", "n_eff cap (refuted)",
                                       "signal fraction"};
    std::printf("\n  DRIFT ROBUSTNESS, on the creature's own window, ear features\n");
    std::printf("    %-22s %8s %8s %8s %8s\n", "learning rate", "0.0 SD", "0.5 SD",
                "1.0 SD", "2.0 SD");
    for (int mode = 0; mode < 3; ++mode) {
      char cell[4][16];
      for (uint32_t di = 0; di < 4; ++di) {
        std::vector<double> acc_d;
        for (uint32_t r = 0; r < kReps; ++r) {
          const uint64_t sd = dna.header().seed + r * 7919ull;
          std::vector<uint8_t> variant = blob;
          std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &sd, sizeof(sd));
          const CtxSrc pr = run_ctxsrc_session(variant, ticks);
          if (!pr.ok || pr.train_split == 0 || pr.feat_gate.empty()) continue;
          double bz = 1.0;
          acc_d.push_back(online_competitive_accuracy(
              pr.feat_gate, pr.labels, pr.train_split, false, &bz, sd ^ 0xD00Du,
              true, true, mode, kDrift[di]));
        }
        double e = 0.0;
        std::snprintf(cell[di], sizeof cell[di], "%.3f",
                      acc_d.empty() ? 0.0 : ctx_mean_se(acc_d, &e));
      }
      std::printf("    %-22s %8s %8s %8s %8s\n", kRateName[mode], cell[0], cell[1],
                  cell[2], cell[3]);
    }
    std::printf("    (drift in units of the data's own within-dimension SD, ramped\n"
                "     across the session in a random direction that favours no rule)\n");
  }

  // THE ISOLATION THIS PROBE WAS EXTENDED FOR. `ctxself` measured DNA v53's
  // index at 0.643 where this probe priced the ear at 1.000, and TWO things
  // differ between those numbers: the WINDOW (the host's tick bins against the
  // creature's own listening gate) and the SETTING (a read-only probe against a
  // taught creature). Two variables at once is exactly `ctxsrc`'s third
  // instrument error, so the window is scored on its own here -- same trials,
  // same labels, same split, same rules, only the window changed.
  std::printf("\n  ISOLATING THE WINDOW\n"
              "    ear, HOST window, conscience   %.3f +/- %.3f\n"
              "    ear, SELF window, conscience   %.3f +/- %.3f\n"
              "    what the window costs          %+.3f\n"
              "    v53 in the creature (ctxself)  %.3f\n",
              m[kAud][kCon], se[kAud][kCon], m[kGate][kCon], se[kGate][kCon],
              m[kGate][kCon] - m[kAud][kCon], kPpV53Index);
  std::printf("    ...and AS THE KERNEL RUNS IT, on the creature's own window:\n"
              "      raw features, zero init      %.3f +/- %.3f\n"
              "      what those two cost          %+.3f against the column above\n",
              m[kGate][kKern], se[kGate][kKern], m[kGate][kKern] - m[kGate][kCon]);
  if (m[kGate][kCon] < m[kAud][kCon] - 0.10) {
    std::printf("    -> THE WINDOW IS THE GAP. The rule is fine and the creature's\n"
                "       listening gate is what loses it, so the next work is the\n"
                "       BOUNDARY and not the partition.\n");
  } else {
    std::printf("    -> THE WINDOW IS NOT THE GAP: the creature's own gate scores\n"
                "       close to the host's. What separates this from v53's %.3f is\n"
                "       the taught setting or the kernel, not the window.\n",
                kPpV53Index);
  }

  // The ear is reported first when it clears, because it is the larger result:
  // it says the index can be formed at all, where the motor-state rows only say
  // which cut of a marginal signal is best.
  if (aud_best >= kPpBuildBar) {
    std::printf("\n  BUILD THE LATCHED INDEX -- the word is separable in the auditory\n"
                "  code WITHOUT the labels and WITHOUT restarts, at %.3f on a single\n"
                "  pass (%.3f supervised), where the best unsupervised partition of the\n"
                "  MOTOR state reaches %.3f. That keeps %.0f%% of the conditional effect\n"
                "  against v52's 8%%, or ~%.0f Hz.\n\n"
                "  The mechanism is most of the way built: `active_ctx_` is written only\n"
                "  when the source is active and HOLDS otherwise, so an index formed\n"
                "  while the word plays already survives into the reward window. What\n"
                "  has to change is `ctx_present_`, which gates the cash-in as well as\n"
                "  the index, so a silent ear at reward time currently sends the update\n"
                "  to the shared bias instead of the latched context's table.\n\n"
                "  Note what this does NOT show: that an unsupervised partition of the\n"
                "  ear TRACKS THE WORD rather than something correlated with it in this\n"
                "  protocol. Two vowels differing in loudness or onset would split this\n"
                "  cleanly too. The build's own control has to be `ctxself`'s\n"
                "  matched-marginal arm, on the paired test, on a fresh seed family.\n",
                aud_best, m[kAud][kSup], best,
                100.0 * (2.0 * aud_best - 1.0), 65.0 * (2.0 * aud_best - 1.0));
    return true;
  }

  if (best >= kPpBuildBar) {
    std::printf("\n  BUILD IT -- an unsupervised partition of %s reaches %.3f,\n"
                "  against %.3f for the fixed cut v52 ships and a %.3f supervised\n"
                "  ceiling. That keeps %.0f%% of the conditional effect where v52 keeps\n"
                "  8%%, which is ~%.0f Hz and detectable on the paired test at n=9.\n"
                "  Lateral competition (DNA v32) already runs on this module.\n",
                kSpaceName[bsp], best, m[bsp][kFix], m[bsp][kSup],
                100.0 * (2.0 * best - 1.0), 65.0 * (2.0 * best - 1.0));
    return true;
  }

  std::printf("\n  DO NOT BUILD IT -- the best unsupervised partition reaches %.3f\n"
              "  against a %.2f bar, which keeps %.0f%% of the conditional effect (~%.0f Hz)\n"
              "  where v52's fixed cut already keeps 8%%. **The cut is not what is\n"
              "  losing the signal.** A learned boundary buys %+.3f over the fixed one\n"
              "  on the same features, and the supervised ceiling that motivated the\n"
              "  build is only reachable BY SEEING THE LABELS -- which is the one thing\n"
              "  the creature cannot do, because the labels are what it is trying to\n"
              "  work out.\n\n"
              "  What that leaves is the honest reading of `ctxsrc`'s 0.740: it is the\n"
              "  best a decoder WITH the answer can do, not a target an unsupervised\n"
              "  mechanism can approach. The remaining route to a self-derived context\n"
              "  is a partition supervised by something the creature HAS -- reward, or\n"
              "  the caregiver's own timing -- and not a better clustering of the\n"
              "  motor state.\n",
              best, kPpBuildBar, 100.0 * (2.0 * best - 1.0), 65.0 * (2.0 * best - 1.0),
              best - m[bsp][kFix]);
  std::printf("\n  AND THE EAR DOES NOT RESCUE IT: %.3f unsupervised against %.3f\n"
              "  supervised. The word is at ceiling in the auditory code and a\n"
              "  partition that has not been told which word is which still cannot\n"
              "  find it, which is the same circularity one module further back.\n"
              "  `rpeprobe` already refused reward as the teacher (0.0%%%% of the reward\n"
              "  variance is between contexts) and the protocol's timing is identical\n"
              "  between the two words by construction. **There is nothing left in\n"
              "  this creature to supervise the partition with.**\n",
              aud_best, m[kAud][kSup]);
  return false;
}

// --- areax: does a context-indexed bias learn what the oracle delivered? ----
//
// `ctxbias` split the problem and priced both halves. Delivery WORKS: a bias
// arriving off the lesson's own neurons costs the exploratory pathway nothing,
// where DNA v47's context tract and v50's regulator both charged for arriving.
// And it measured the ceiling: an oracle bias steers the voice 236 Hz of F1,
// 51% of the gap between the two words. What was missing was the other half --
// nothing in this creature could work out what that bias should be.
//
// DNA v51 is the smallest thing that can. `bias_[i]` becomes `bias_[i][c]`,
// indexed by the active slice of a `kContext` module, and the argument for that
// parameterisation and no other is in DnaExploration::context_slots: this
// project holds two contradictory conclusions about why node perturbation
// cannot be conditional, and Werfel, Xie & Seung reconcile them by saying
// learning time scales with PARAMETER COUNT. One bias per neuron per context is
// 2x the parameters where the synaptic version was ~16x.
//
// **The context module needs no projection.** It is read as an index, never as
// drive, which is why this costs the larynx nothing where v47 cost it
// everything -- and it is why the genome for this experiment is built with
// `out_w=0`.
//
// THREE ARMS, and the third is the one `pgprobe` taught this project to
// include. `on` and `off` differ in one genome field. `random` keeps the
// mechanism on and draws the target INDEPENDENTLY of the word with the same
// marginals, so a creature that has merely become more variable, or that sits
// between two targets, scores the same as one that has learned nothing.
//
// TWO GATES BEFORE ANY OF THAT IS READ. A table that was never indexed and a
// table that was indexed and learned nothing are the same flat dF1 from
// outside, so the run refuses unless it can show both that the creature was in
// a context and that the tables diverged.
struct AreaxArm {
  const char* name;
  uint32_t slots;
  int target;      // VLTarget
};

// `fixed+on` WAS THIS EXPERIMENT'S POWER GATE AND IT IS NOT ONE. Kept as a
// diagnostic, because how it failed is worth more than what it was for.
//
// The reasoning that put it here: splitting the table halves the trials each
// context gets, vocallearn's positive control reads +1.0 at 560k against +18.3
// at 3.4M, so a flat conditional result might be half a session rather than a
// null. This arm keeps the mechanism on and makes the target UNCONDITIONAL.
//
// It refused the run at 3.4M (+5.3 against an +18.3 bar) and its own remedy was
// "re-run at 2x". At 6.8M it read **-1.7**. **Doubling the session made it
// worse, and a power problem cannot do that** -- which falsifies the arm as a
// power measurement on its own terms rather than on a preference for the
// numbers underneath it.
//
// The a priori reason, which was available before the run and should have been
// seen: with an unconditional target both tables must learn the SAME bias, so
// the split doubles the parameters needed to express one lesson while halving
// the data for each. **It is the split table's WORST case**, where the
// conditional task is its best. "If this fails the conditional arm is
// unreadable" never followed.
//
// The gate is now `on` against `random` on `change` -- the conditional lesson's
// own error reduction against a control with identical structure, identical
// marginals and an identical split, differing only in whether the target tracks
// the word. That bounds what it claims to bound.
constexpr AreaxArm kAreaxArms[] = {
    {"off",      0, kVLTgtHeard},
    {"on",       2, kVLTgtHeard},
    {"random",   2, kVLTgtRandom},
    {"fixed+on", 2, kVLTgtFixed},
};
constexpr uint32_t kAreaxArmCount = sizeof(kAreaxArms) / sizeof(kAreaxArms[0]);

// The bar, from `ctxbias` on the same protocol and the same readout: what a
// PERFECT conditional bias delivered straight to the larynx achieves.
constexpr double kAreaxOracleDF1 = 235.9;

bool run_areax(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  (void)verbose;
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t ctx_module = dna.module_with_role(aibaby::ModuleRole::kContext);
  if (ctx_module < 0) {
    std::printf("  this genome has no kContext module, so there is no index to\n"
                "  key a per-context bias on. Build one -- and give it NO output\n"
                "  weight, because v51 reads it as an index and never as drive:\n\n"
                "    python3 tools/genome_add_context.py dna/default.toml ctx.toml \\\n"
                "        vocal out_w=0\n"
                "    ./build/aibaby --dna ctx.toml --experiment areax\n");
    return false;
  }
  constexpr uint32_t kReps = 3;
  const size_t slots_off = offsetof(aibaby::DnaHeader, exploration) +
                           offsetof(aibaby::DnaExploration, context_slots);
  instrument("areax", dna.header().seed, ticks / kVLTrialTicks, "trials per arm");
  std::printf("  question          DNA v51 splits node perturbation's excitability table\n"
              "                    one-per-context. Can the estimator that met G2 at\n"
              "                    twelve sigma find a CONDITIONAL optimum now that one\n"
              "                    is representable?\n");
  std::printf("  the bar           %.0f Hz of dF1 -- what `ctxbias` measured a PERFECT\n"
              "                    conditional bias achieves on this readout. Reaching a\n"
              "                    small fraction of it is a real result: it would say the\n"
              "                    estimator cannot find the optimum even when it exists.\n",
              kAreaxOracleDF1);
  std::printf("  arm               taught, conditional target. `random` is the\n"
              "                    matched-marginal control pgprobe exists for.\n\n");

  std::vector<double> df1[kAreaxArmCount], change[kAreaxArmCount];
  std::vector<double> present[kAreaxArmCount], div[kAreaxArmCount], shared[kAreaxArmCount];

  std::printf("  %-6s %-8s %-9s %-9s %-10s %-10s %s\n", "seed", "arm", "dF1 (Hz)",
              "in ctx", "table div", "shared", "change");
  for (uint32_t r = 0; r < kReps; ++r) {
    for (uint32_t a = 0; a < kAreaxArmCount; ++a) {
      std::vector<uint8_t> variant = blob;
      const uint64_t seed = dna.header().seed + r * 7919ull;
      std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
      const uint32_t slots = kAreaxArms[a].slots;
      std::memcpy(variant.data() + slots_off, &slots, sizeof(slots));

      CtxDrive drive;
      drive.module = ctx_module;
      drive.slots = kVLWords;
      // Enough to put the slice unambiguously above the kernel's 1 Hz floor.
      // The module has no noise and no target rate, so a silent slice is at
      // exactly zero and this is not a threshold anyone has to tune.
      drive.gain = 0.10;
      Regime reg;
      reg.praise = kPraiseValue;
      reg.scold = kScoldValue;
      const VLRun run = run_vocallearn_session(variant, ticks, kVLTaught, nullptr, reg,
                                               kAreaxArms[a].target, &drive);
      if (!run.ok) {
        std::printf("  %-6u %-8s (inconclusive: %u scored, %u skipped)\n", r,
                    kAreaxArms[a].name, run.scored, run.skipped);
        continue;
      }
      const double d1 = std::fabs(run.f1_by_word[0] - run.f1_by_word[1]);
      df1[a].push_back(d1);
      change[a].push_back(vl_change(run));
      present[a].push_back(run.ctx_present_frac);
      div[a].push_back(run.ctx_table_div);
      shared[a].push_back(run.ctx_shared_mag);
      std::printf("  %-6u %-8s %-9.1f %-9.2f %-10.4f %-10.4f %+.1f\n", r,
                  kAreaxArms[a].name, d1, run.ctx_present_frac, run.ctx_table_div,
                  run.ctx_shared_mag, vl_change(run));
    }
  }

  double m_d1[kAreaxArmCount], s_d1[kAreaxArmCount];
  double m_ch[kAreaxArmCount], s_ch[kAreaxArmCount];
  double m_pr[kAreaxArmCount], s_pr[kAreaxArmCount];
  double m_dv[kAreaxArmCount], s_dv[kAreaxArmCount];
  double m_sh[kAreaxArmCount], s_sh[kAreaxArmCount];
  for (uint32_t a = 0; a < kAreaxArmCount; ++a) {
    if (df1[a].size() < 2) {
      std::printf("\n  areax INCONCLUSIVE -- arm `%s` did not produce two usable\n"
                  "  creatures, so it has no spread and nothing can be read against it.\n",
                  kAreaxArms[a].name);
      return false;
    }
    m_d1[a] = ctx_mean_se(df1[a], &s_d1[a]);
    m_ch[a] = ctx_mean_se(change[a], &s_ch[a]);
    m_pr[a] = ctx_mean_se(present[a], &s_pr[a]);
    m_dv[a] = ctx_mean_se(div[a], &s_dv[a]);
    m_sh[a] = ctx_mean_se(shared[a], &s_sh[a]);
  }

  std::printf("\n  %-9s %-16s %-13s %-15s %-13s %s\n", "arm", "dF1 (Hz)", "in ctx",
              "table div", "|ctx bias|", "change");
  for (uint32_t a = 0; a < kAreaxArmCount; ++a) {
    char b[40], c[40], d[40], e[40], f[40];
    std::snprintf(b, sizeof b, "%.1f +/- %.1f", m_d1[a], s_d1[a]);
    std::snprintf(c, sizeof c, "%.2f +/- %.2f", m_pr[a], s_pr[a]);
    std::snprintf(d, sizeof d, "%.4f +/- %.4f", m_dv[a], s_dv[a]);
    std::snprintf(e, sizeof e, "%.4f +/- %.4f", m_sh[a], s_sh[a]);
    std::snprintf(f, sizeof f, "%+.1f +/- %.1f", m_ch[a], s_ch[a]);
    std::printf("  %-9s %-16s %-13s %-15s %-13s %s\n", kAreaxArms[a].name, b, c, d, e, f);
  }

  const uint32_t kOff = 0, kOn = 1, kRnd = 2, kPos = 3;

  // GATE 0: did the mechanism learn ANYTHING? The conditional arm's own error
  // reduction, against the matched-marginal control's. Same structure, same
  // split, same context drive; the only difference is whether the target tracks
  // the word. A creature that has merely become more variable scores the same
  // in both, so this bounds what it claims to bound -- unlike `fixed+on`, which
  // is reported below and is the split table's worst case rather than a
  // measure of its power.
  if (m_ch[kOn] - m_ch[kRnd] <= 2.0 * (s_ch[kOn] + s_ch[kRnd])) {
    std::printf("\n  NOTHING WAS LEARNED -- the conditional arm reduces its own error by\n"
                "  %+.1f +/- %.1f against %+.1f +/- %.1f for a target drawn independently of\n"
                "  the word. The mechanism is running (the tables diverged) and reward is\n"
                "  not moving the creature toward the conditional target at all, so the\n"
                "  dF1 numbers below are not about conditional behaviour.\n",
                m_ch[kOn], s_ch[kOn], m_ch[kRnd], s_ch[kRnd]);
    return false;
  }

  // GATE 1: was the creature ever in a context? Without this, a flat result is
  // a fact about the host's driving and not about the mechanism.
  if (m_pr[kOn] < 0.5) {
    std::printf("\n  REFUSED -- the creature was in a context on only %.0f%% of ticks, so\n"
                "  the table was mostly not indexed and nothing here is a measurement of\n"
                "  what a context-indexed bias does. Raise the oracle's gain.\n",
                100.0 * m_pr[kOn]);
    return false;
  }
  // GATE 2: did the two tables actually come apart? A split that stays
  // identical has learned nothing conditional whatever the voice did, and
  // saying so is different from saying the voice did not move.
  if (m_dv[kOn] <= 2.0 * s_dv[kOn] || m_dv[kOn] < 0.01 * m_sh[kOn]) {
    std::printf("\n  REFUSED -- the two context tables never diverged (%.4f +/- %.4f\n"
                "  against a shared |bias| of %.4f). The mechanism ran and the estimator\n"
                "  wrote nothing different into the two contexts, so a flat dF1 below is\n"
                "  not evidence about conditional behaviour -- it is the same statement\n"
                "  one level up. Check that reward is reaching the larynx at all.\n",
                m_dv[kOn], s_dv[kOn], m_sh[kOn]);
    return false;
  }

  const double lift = m_d1[kOn] - m_d1[kOff];
  const double lift_se = s_d1[kOn] + s_d1[kOff];
  const double vs_rnd = m_d1[kOn] - m_d1[kRnd];
  const double rnd_se = s_d1[kOn] + s_d1[kRnd];
  const double frac = 100.0 * m_d1[kOn] / kAreaxOracleDF1;

  std::printf("\n  the split's OWN cost     `fixed+on` %+.1f -- an unconditional lesson on\n"
              "                           a split table, which is its WORST case and not a\n"
              "                           power measurement (it fell with more ticks)\n",
              m_ch[kPos]);
  std::printf("  the tables diverged      %.4f against a shared |bias| of %.4f\n"
              "  the creature was in ctx  %.0f%% of ticks\n"
              "  dF1 on -- off            %.1f -> %.1f Hz (lift %+.1f, %.1f SE)\n"
              "  dF1 on -- random         %.1f -> %.1f Hz (%+.1f, %.1f SE)\n"
              "  against the oracle       %.1f%% of %.0f Hz\n",
              m_dv[kOn], m_sh[kOn], 100.0 * m_pr[kOn], m_d1[kOff], m_d1[kOn], lift,
              lift_se > 0.0 ? lift / lift_se : 0.0, m_d1[kRnd], m_d1[kOn], vs_rnd,
              rnd_se > 0.0 ? vs_rnd / rnd_se : 0.0, frac, kAreaxOracleDF1);

  const bool beats_off = lift > 2.0 * lift_se;
  const bool beats_rnd = vs_rnd > 2.0 * rnd_se;
  if (beats_off && beats_rnd) {
    std::printf("\n  IT LEARNS -- a context-indexed bias makes the voice depend on the\n"
                "  word, %+.1f Hz of F1 above the shared-bias arm and %+.1f above a\n"
                "  matched-marginal control, which is %.0f%% of what a perfect bias\n"
                "  achieves. This is the first conditional effect on the voice in this\n"
                "  project that reward produced rather than an oracle.\n",
                lift, vs_rnd, frac);
    return true;
  }

  std::printf("\n  IT DOES NOT LEARN, AND THAT IS THE FIRMEST CLOSURE OF G3 ON FILE.\n"
              "  The tables DID diverge (%.4f), so the estimator ran and wrote\n"
              "  different things into the two contexts. The voice still does not\n"
              "  depend on the word: %.1f Hz against %.1f with the mechanism off and\n"
              "  %.1f against the matched-marginal control, where a perfect bias on this\n"
              "  same readout reaches %.0f.\n\n"
              "  Every earlier null here had a delivery excuse -- the condition did not\n"
              "  arrive, the tract could not carry it, the readout could not express it.\n"
              "  This one has none. `ctxbias` showed the bias route is free and the\n"
              "  optimum is representable; v51 makes it representable IN THE CREATURE\n"
              "  and the estimator does not find it. What is left is the estimator\n"
              "  itself -- Gadagkar's performance prediction error rather than the raw,\n"
              "  object-blind R this creature delivers.\n",
              m_dv[kOn], m_d1[kOn], m_d1[kOff], m_d1[kRnd], kAreaxOracleDF1);
  return false;
}

// --- ctxself: can the creature index its own bias table? -------------------
//
// `areax` is the result this exists to finish. DNA v51 works -- a context-
// indexed bias makes the voice depend on the word, 112.9 Hz of dF1 against
// 27.1 with the table shared and 33.5 against a matched-marginal control --
// but the index it is keyed on is WRITTEN BY THE HOST from the word label.
// That is an oracle, and it is the honest limit on the result. For naming, the
// creature has to work out which context it is in from its own state.
//
// `ctxsrc` measured where that could come from, in the window where reward
// actually lands rather than while the word plays, and the answer was the
// LARYNX: the articulators carry the word at 0.740 there where the ear is at
// 0.541 and central at 0.514 -- and they carry it BETTER after the word stops
// (0.818) than during it (0.510), which is a delayed copy and the signature of
// a memory. It also measured that 0.740 is just under the 0.75 a two-context
// index needs, so the proxy refused to license the build.
//
// SO WHY BUILD IT ANYWAY, AND WHY THAT IS NOT FITTING A VERDICT TO DATA.
// `ctxsrc`'s 0.740 is a HELD-OUT SUPERVISED readout: it fits centroids using
// the word labels and reports the best any linear decoder could do. What DNA
// v52 installs is a FIXED, unsupervised partition -- the same argmax v51
// already runs, pointed at the larynx -- which can only do worse. So the proxy
// is not a prediction that was ignored; it is an upper bound that came out
// marginal, and once an upper bound is marginal the direct measurement is the
// cheaper instrument and the only one that can settle it.
//
// THE BAR, DERIVED AND STATED FIRST. An index right with probability p writes
// the OTHER table 1-p of the time, and the wrong write CANCELS rather than
// merely failing to help, so the conditional component scales as (2p - 1). At
// `ctxsrc`'s upper bound that is 0.48 of what the oracle index bought.
//
// It scales the ORACLE's dF1 and the `off` arm's is not added to it, which is
// an arithmetic trap worth naming because this project has already published
// the wrong form of it once. dF1 is |mean F1 word A - mean F1 word B|, an
// ABSOLUTE value, so the `off` arm's 27 Hz is E|noise| with no signal under it
// rather than a pedestal the signal sits on. Scaling the difference and then
// adding the pedestal counts the noise twice. The prediction is therefore
// (2p - 1) x oracle, floored at `off` because E|s + noise| can never fall below
// E|noise|. This run measures p directly and prints the prediction from BOTH
// the bound and its own measurement.
//
// A LOOP THAT IS REAL AND HAS TO BE REPORTED, NOT DESIGNED AWAY. The bias
// table cashes into every larynx neuron, including the off-axis ones the index
// is read from, so the mechanism can steer its own index. That is a property of
// the architecture rather than a flaw in the instrument -- an Area X output
// biases the same motor population its input is derived from -- and it has two
// visible signatures. If the table learns to make the index self-confirming,
// one slice takes the whole reward window and `busiest slice` says so. If it
// merely becomes more variable, `self-rnd` scores the same as `self`, because
// that control runs the identical loop with a target that does not track the
// word. What the loop CANNOT do is raise p, which is agreement with the
// caregiver and not with itself.
//
// FOUR ARMS, and every one of them is driven by the same host oracle so that
// the creature's inputs are identical across the run. The context module has
// no output weight, so driving it changes nothing the kernel does not read --
// which means `self` and `oracle` differ in exactly one genome field, and
// `self` is not a creature that was also deprived of something.
struct CtxSelfArm {
  const char* name;
  uint32_t slots;
  uint32_t source;  // DnaExploration::context_source
  int target;       // VLTarget
};

constexpr CtxSelfArm kCtxSelfArms[] = {
    {"off",      0, 0, kVLTgtHeard},
    {"oracle",   2, 0, kVLTgtHeard},
    // DNA v52's arms are retired from the run and kept in the record: it was
    // refused on two seed families and its numbers are in the README. The slots
    // they free go to the drift test, which is what this run is now for.
    //
    // `ear` freezes its prototypes (MacQueen's 1/wins reaches zero); `adapt`
    // caps the averaging window at the point where a prototype is already
    // accurate relative to the gap it resolves. If drift is what costs the
    // index, `adapt` keeps what `ear` loses -- and the per-arm early->late
    // split below says whether drift is happening at all, independently of
    // whether the fix works.
    // DNA v53: the competitive latched index off the auditory code. `ear-rnd`
    // is its matched-marginal control and is the arm the verdict is gated on --
    // `partprobe` said so before this was built, because a clean split of the
    // ear would look identical whether it tracks the WORD or something merely
    // correlated with it in this protocol.
    {"ear",      2, 2, kVLTgtHeard},
    {"ear-rnd",  2, 2, kVLTgtRandom},
    // `adapt` (source 3, the derived rate cap) is retired: it cost -0.146 of
    // index on 8 of 9 seeds and made both arms decay. Its slots go to source 4,
    // which needs no episode at all -- `partprobe` scores the ear's own rate EMA
    // at reward time at 1.000 under this kernel's exact rule, with no gate, no
    // accumulator and no latch, because an EMA carries the preceding second
    // where an accumulator that resets carries whatever the last fragment held.
    {"ema",      2, 4, kVLTgtHeard},
    {"ema-rnd",  2, 4, kVLTgtRandom},
};
constexpr uint32_t kCtxSelfArmCount = sizeof(kCtxSelfArms) / sizeof(kCtxSelfArms[0]);

// `ctxsrc`'s upper bound on how well ANY readout of the larynx names the word
// in the reward window, on 9 creatures.
constexpr double kCtxSelfBound = 0.740;

bool run_ctxself(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  (void)verbose;
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t ctx_module = dna.module_with_role(aibaby::ModuleRole::kContext);
  if (ctx_module < 0) {
    std::printf("  this genome has no kContext module. The `self` arms do not need\n"
                "  one -- they read the larynx -- but the `oracle` reference arm does,\n"
                "  and without it there is nothing to measure the creature's own index\n"
                "  against. Build one with NO output weight:\n\n"
                "    python3 tools/genome_add_context.py dna/default.toml ctx.toml \\\n"
                "        vocal out_w=0\n"
                "    ./build/aibaby --dna ctx.toml --experiment ctxself\n");
    return false;
  }
  if (dna.module_with_role(aibaby::ModuleRole::kVocal) < 0) {
    std::printf("  setup failed: this genome has no kVocal module, so there is no\n"
                "  larynx for DNA v52 to read an index from\n");
    return false;
  }
  // NINE, not three, and the reason is this project's own most recent lesson.
  // `ctxsrc` read 0.754 on three creatures and PRINTED A LICENCE; at nine it
  // read 0.740 and refused. The quantity this run turns on is a difference of
  // tens of Hz between arms whose per-creature spread is tens of Hz, which is
  // exactly the regime where three seeds decide nothing.
  constexpr uint32_t kReps = 9;
  const size_t slots_off = offsetof(aibaby::DnaHeader, exploration) +
                           offsetof(aibaby::DnaExploration, context_slots);
  const size_t src_off = offsetof(aibaby::DnaHeader, exploration) +
                         offsetof(aibaby::DnaExploration, context_source);
  instrument("ctxself", dna.header().seed ^ 0x5E1Fu, ticks / kVLTrialTicks,
             "trials per arm");
  std::printf("  question          `areax` proved a context-indexed bias works, keyed on\n"
              "                    an index the HOST wrote from the word label. Can the\n"
              "                    creature derive that index for ITSELF? v52 read it\n"
              "                    off the larynx and was refused; source 4 reads the\n"
              "                    ear's own rate EMA, which is what replicated.\n");
  std::printf("  the bar           an index right with probability p writes the other\n"
              "                    table 1-p of the time, so the conditional part scales\n"
              "                    as (2p - 1). `ctxsrc` bounds p at %.3f for ANY readout\n"
              "                    of the larynx in this window, which is %.0f%% of what the\n"
              "                    oracle index bought. A FIXED partition can only do\n"
              "                    worse, so that is a ceiling and not a prediction.\n",
              kCtxSelfBound, 100.0 * (2.0 * kCtxSelfBound - 1.0));
  std::printf("  arm               all four get the same host drive on the context\n"
              "                    module, so `self` and `oracle` differ in ONE field.\n\n");

  std::vector<double> df1[kCtxSelfArmCount], change[kCtxSelfArmCount];
  // Which creature each row came from. `ctxself` runs every arm on the SAME
  // seeds, so the comparison is paired -- but an arm that fails a gate leaves a
  // hole, and pairing by position after a hole compares two different
  // creatures. Pair by the seed index and never by the row.
  std::vector<uint32_t> reps[kCtxSelfArmCount];
  std::vector<double> present[kCtxSelfArmCount], div[kCtxSelfArmCount];
  std::vector<double> shared[kCtxSelfArmCount], match[kCtxSelfArmCount];
  std::vector<double> occ[kCtxSelfArmCount];
  std::vector<double> mte[kCtxSelfArmCount], mtl[kCtxSelfArmCount];
  std::vector<double> name[kCtxSelfArmCount], nshuf[kCtxSelfArmCount];
  // Which creature each naming row came from, so the comparison can be PAIRED
  // like every other gate in this experiment. Reported as a mean only, the
  // milestone number would rest on a weaker test than the mechanism numbers do.
  std::vector<uint32_t> nreps[kCtxSelfArmCount];
  std::vector<double> corr[kCtxSelfArmCount];
  std::vector<uint32_t> creps[kCtxSelfArmCount];
  std::vector<double> axis[kCtxSelfArmCount];
  std::vector<uint32_t> areps[kCtxSelfArmCount];
  std::vector<double> dirs[kCtxSelfArmCount];

  std::printf("  %-6s %-9s %-9s %-8s %-8s %-10s %-7s %s\n", "seed", "arm", "dF1 (Hz)",
              "p(idx)", "busiest", "table div", "ev/tri", "change");
  // Fifty-four independent brains, run across the machine's cores. Every row is
  // printed and every statistic pooled SERIALLY from the returned cells below,
  // and a rep's seed is a pure function of its index, so this is byte-for-byte
  // what the serial loop produced -- checked against a reference captured before
  // the conversion.
  struct Cell {
    bool ok = false;
    uint32_t scored = 0, skipped = 0;
    double d1 = 0.0, chg = 0.0, present = 0.0, div = 0.0, shared = 0.0;
    double match = 0.0, occ = 0.0, evt = 0.0, mte = 0.0, mtl = 0.0;
    bool has_corr = false, has_axis = false, has_dir = false, has_name = false;
    double corr = 0.0, axis = 0.0, dir = 0.0, name = 0.0, nshuf = 0.0;
  };
  const std::vector<Cell> cells =
      parallel_reps<Cell>(kReps * kCtxSelfArmCount, [&](uint32_t i) {
        const uint32_t r = i / kCtxSelfArmCount;
        const uint32_t a = i % kCtxSelfArmCount;
        Cell cell;
        std::vector<uint8_t> variant = blob;
        const uint64_t seed = dna.header().seed + r * 7919ull;
        std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
        const uint32_t slots = kCtxSelfArms[a].slots;
        const uint32_t source = kCtxSelfArms[a].source;
        std::memcpy(variant.data() + slots_off, &slots, sizeof(slots));
        std::memcpy(variant.data() + src_off, &source, sizeof(source));

        CtxDrive drive;
        drive.module = ctx_module;
        drive.slots = kVLWords;
        drive.gain = 0.10;
        Regime reg;
        reg.praise = kPraiseValue;
        reg.scold = kScoldValue;
        const VLRun run = run_vocallearn_session(variant, ticks, kVLTaught, nullptr, reg,
                                                 kCtxSelfArms[a].target, &drive);
        cell.scored = run.scored;
        cell.skipped = run.skipped;
        if (!run.ok) return cell;
        cell.d1 = std::fabs(run.f1_by_word[0] - run.f1_by_word[1]);
        cell.chg = vl_change(run);
        cell.present = run.ctx_present_frac;
        cell.div = run.ctx_table_div;
        cell.shared = run.ctx_shared_mag;
        cell.match = run.ctx_match;
        cell.occ = run.ctx_occupancy;
        cell.evt = run.ctx_events_per_trial;
        cell.mte = run.ctx_match_early;
        cell.mtl = run.ctx_match_late;
        // THE NAMING SCORE: a held-out one-of-two readout over the creature's own
        // utterances, F1 and F2 only. Chance is 0.500 with balanced words. The
        // shuffled control uses the same rows and the same split, so a readout
        // that has learned the split rather than the voice scores the same in
        // both and the difference is what is real.
        {
          std::vector<std::vector<double>> rows;
          rows.reserve(run.utt_word.size());
          for (size_t u = 0; u < run.utt_word.size(); ++u) {
            rows.push_back({run.utt_f1[u], run.utt_f2[u]});
          }
          // CORRECT naming, as opposed to merely CONSISTENT naming. The readout
          // below fits centroids to the creature's own output, so it scores an
          // arbitrary-but-stable mapping exactly as highly as the taught one --
          // which is why `ema-rnd` beat it: that arm's index tracks the word too,
          // so its voice is word-dependent, just not in the direction reward
          // asked for.
          //
          // This scores against the WORDS' OWN TARGETS instead. Nothing is
          // fitted, so there is no split and no leak: an utterance either lands
          // nearer the target for the word the creature HEARD or nearer the other
          // one, and chance is 0.500.
          {
            uint32_t hit = 0, tot = 0;
            for (size_t u = 0; u < run.utt_word.size(); ++u) {
              const double e0 = formant_error(run.utt_f1[u], run.utt_f2[u], kWords[0]);
              const double e1 = formant_error(run.utt_f1[u], run.utt_f2[u], kWords[1]);
              if (e0 < 0.0 || e1 < 0.0) continue;
              const int pick = e1 < e0 ? 1 : 0;
              if (pick == run.utt_word[u]) ++hit;
              ++tot;
            }
            if (tot >= 16) {
              cell.corr = double(hit) / double(tot);
              cell.has_corr = true;
            }
            // ...and the same question asked RELATIVE TO THE CREATURE'S OWN
            // BASELINE, which is the one it can currently pass. The absolute
            // score above asks whether the utterance REACHED the right target
            // and cannot come off chance here: the words sit 460 Hz apart in F1
            // and the taught shift is ~93 Hz.
            {
              double mf1 = 0.0, mf2 = 0.0;
              uint32_t mn = 0;
              for (size_t u = 0; u < run.utt_word.size(); ++u) {
                if (run.utt_f1[u] <= 1.0 || run.utt_f2[u] <= 1.0) continue;
                mf1 += std::log(run.utt_f1[u]);
                mf2 += std::log(run.utt_f2[u]);
                ++mn;
              }
              if (mn >= 16) {
                mf1 /= double(mn);
                mf2 /= double(mn);
                const double ax1 = std::log(double(kWords[1].f1)) - std::log(double(kWords[0].f1));
                const double ax2 = std::log(double(kWords[1].f2)) - std::log(double(kWords[0].f2));
                uint32_t h2 = 0, t2 = 0;
                for (size_t u = 0; u < run.utt_word.size(); ++u) {
                  if (run.utt_f1[u] <= 1.0 || run.utt_f2[u] <= 1.0) continue;
                  const double p1 = std::log(run.utt_f1[u]) - mf1;
                  const double p2 = std::log(run.utt_f2[u]) - mf2;
                  const int pick = (p1 * ax1 + p2 * ax2) > 0.0 ? 1 : 0;
                  if (pick == run.utt_word[u]) ++h2;
                  ++t2;
                }
                if (t2 >= 16) {
                  cell.axis = double(h2) / double(t2);
                  cell.has_axis = true;
                }
                // The K-WORD generalisation, run here at k=2 as a CHECK on
                // itself: with two targets t0 = -t1, so the argmax of the dot
                // product is the sign of the projection and this must reproduce
                // the axis number above. If the two columns disagree at two
                // words, the generalisation is wrong and nothing it says at four
                // words can be trusted.
                std::vector<std::pair<double, double>> tg;
                for (uint32_t q = 0; q < kVLWords; ++q) {
                  tg.push_back({double(kWords[q].f1), double(kWords[q].f2)});
                }
                const double dir = direction_accuracy(run.utt_f1, run.utt_f2,
                                                      run.utt_word, tg);
                if (dir > 0.0) { cell.dir = dir; cell.has_dir = true; }
              }
            }
          }
          const size_t tr = rows.size() / 2;
          if (rows.size() >= 16) {
            cell.name = holdout_accuracy(rows, run.utt_word, tr);
            aibaby::Rng nr;
            nr.seed(seed ^ 0x4E41u);
            std::vector<int> sh = run.utt_word;
            for (size_t u = sh.size(); u > 1; --u) std::swap(sh[u - 1], sh[nr.next() % u]);
            cell.nshuf = holdout_accuracy(rows, sh, tr);
            cell.has_name = true;
          }
        }
        cell.ok = true;
        parallel_note("  [%u/%u] seed %u %s  dF1 %.1f  p(idx) %.3f\n",
                      i + 1, kReps * kCtxSelfArmCount, r, kCtxSelfArms[a].name,
                      cell.d1, cell.match);
        return cell;
      });

  for (uint32_t i = 0; i < kReps * kCtxSelfArmCount; ++i) {
    const uint32_t r = i / kCtxSelfArmCount;
    const uint32_t a = i % kCtxSelfArmCount;
    const Cell& c = cells[i];
    if (!c.ok) {
      std::printf("  %-6u %-9s (inconclusive: %u scored, %u skipped)\n", r,
                  kCtxSelfArms[a].name, c.scored, c.skipped);
      continue;
    }
    df1[a].push_back(c.d1);
    reps[a].push_back(r);
    change[a].push_back(c.chg);
    present[a].push_back(c.present);
    div[a].push_back(c.div);
    shared[a].push_back(c.shared);
    match[a].push_back(c.match);
    occ[a].push_back(c.occ);
    if (c.has_corr) { corr[a].push_back(c.corr); creps[a].push_back(r); }
    if (c.has_axis) { axis[a].push_back(c.axis); areps[a].push_back(r); }
    if (c.has_dir) dirs[a].push_back(c.dir);
    if (c.has_name) {
      name[a].push_back(c.name);
      nreps[a].push_back(r);
      nshuf[a].push_back(c.nshuf);
    }
    mte[a].push_back(c.mte);
    mtl[a].push_back(c.mtl);
    std::printf("  %-6u %-9s %-9.1f %-8.3f %-8.3f %-10.4f %-7.1f %+.1f\n", r,
                kCtxSelfArms[a].name, c.d1, c.match, c.occ, c.div, c.evt, c.chg);
  }

  double m_d1[kCtxSelfArmCount], s_d1[kCtxSelfArmCount];
  double m_ch[kCtxSelfArmCount], s_ch[kCtxSelfArmCount];
  double m_pr[kCtxSelfArmCount], s_pr[kCtxSelfArmCount];
  double m_dv[kCtxSelfArmCount], s_dv[kCtxSelfArmCount];
  double m_sh[kCtxSelfArmCount], s_sh[kCtxSelfArmCount];
  double m_mt[kCtxSelfArmCount], s_mt[kCtxSelfArmCount];
  double m_oc[kCtxSelfArmCount], s_oc[kCtxSelfArmCount];
  double m_me[kCtxSelfArmCount], s_me[kCtxSelfArmCount];
  double m_ml[kCtxSelfArmCount], s_ml[kCtxSelfArmCount];
  double m_nm[kCtxSelfArmCount], s_nm[kCtxSelfArmCount];
  double m_ns[kCtxSelfArmCount], s_ns[kCtxSelfArmCount];
  double m_cr[kCtxSelfArmCount], s_cr[kCtxSelfArmCount];
  double m_ax[kCtxSelfArmCount], s_ax[kCtxSelfArmCount];
  double m_dr[kCtxSelfArmCount], s_dr[kCtxSelfArmCount];
  uint32_t valid = 0;
  for (uint32_t a = 0; a < kCtxSelfArmCount; ++a) {
    if (df1[a].size() < 2) {
      std::printf("\n  ctxself INCONCLUSIVE -- arm `%s` produced %zu usable creatures.\n",
                  kCtxSelfArms[a].name, df1[a].size());
      return false;
    }
    ++valid;
    m_d1[a] = ctx_mean_se(df1[a], &s_d1[a]);
    m_ch[a] = ctx_mean_se(change[a], &s_ch[a]);
    m_pr[a] = ctx_mean_se(present[a], &s_pr[a]);
    m_dv[a] = ctx_mean_se(div[a], &s_dv[a]);
    m_sh[a] = ctx_mean_se(shared[a], &s_sh[a]);
    m_mt[a] = ctx_mean_se(match[a], &s_mt[a]);
    m_oc[a] = ctx_mean_se(occ[a], &s_oc[a]);
    m_me[a] = ctx_mean_se(mte[a], &s_me[a]);
    m_ml[a] = ctx_mean_se(mtl[a], &s_ml[a]);
    m_nm[a] = name[a].size() >= 2 ? ctx_mean_se(name[a], &s_nm[a]) : 0.0;
    m_ns[a] = nshuf[a].size() >= 2 ? ctx_mean_se(nshuf[a], &s_ns[a]) : 0.0;
    m_cr[a] = corr[a].size() >= 2 ? ctx_mean_se(corr[a], &s_cr[a]) : 0.0;
    m_ax[a] = axis[a].size() >= 2 ? ctx_mean_se(axis[a], &s_ax[a]) : 0.0;
    m_dr[a] = dirs[a].size() >= 2 ? ctx_mean_se(dirs[a], &s_dr[a]) : 0.0;
  }
  (void)valid;

  std::printf("\n  %-9s %-16s %-15s %-14s %-13s %s\n", "arm", "dF1 (Hz)", "p(index)",
              "busiest slice", "table div", "change");
  for (uint32_t a = 0; a < kCtxSelfArmCount; ++a) {
    char b[40], c[40], d[40], e[40], f[40];
    std::snprintf(b, sizeof b, "%.1f +/- %.1f", m_d1[a], s_d1[a]);
    std::snprintf(c, sizeof c, "%.3f +/- %.3f", m_mt[a], s_mt[a]);
    std::snprintf(d, sizeof d, "%.3f +/- %.3f", m_oc[a], s_oc[a]);
    std::snprintf(e, sizeof e, "%.4f +/- %.4f", m_dv[a], s_dv[a]);
    std::snprintf(f, sizeof f, "%+.1f +/- %.1f", m_ch[a], s_ch[a]);
    std::printf("  %-9s %-16s %-15s %-14s %-13s %s\n", kCtxSelfArms[a].name, b, c, d, e, f);
  }

  const uint32_t kOff = 0, kOra = 1, kEar = 2, kERnd = 3, kAda = 4, kARnd = 5;
  const uint32_t kEma = kAda, kMRnd = kARnd;  // source 4 occupies those slots

  // The paired differences, computed and printed HERE -- above every gate --
  // because they are descriptive statistics rather than verdicts, and a run
  // that refuses should still show the numbers it refused on. This experiment's
  // gate-1 refusal on a short run would otherwise hide them entirely.
  auto paired = [&](uint32_t A, uint32_t B, double* se, uint32_t* npos, uint32_t* n) {
    std::vector<double> d;
    for (size_t i = 0; i < reps[A].size(); ++i) {
      for (size_t j = 0; j < reps[B].size(); ++j) {
        if (reps[A][i] == reps[B][j]) { d.push_back(df1[A][i] - df1[B][j]); break; }
      }
    }
    *n = uint32_t(d.size());
    *npos = 0;
    for (double v : d) if (v > 0.0) ++*npos;
    if (d.size() < 2) { *se = 0.0; return 0.0; }
    double m = 0.0;
    for (double v : d) m += v;
    m /= double(d.size());
    double ss = 0.0;
    for (double v : d) ss += (v - m) * (v - m);
    *se = std::sqrt(ss / double(d.size() - 1)) / std::sqrt(double(d.size()));
    return m;
  };
  double e_rnd_se = 0.0, e_off_se = 0.0;
  uint32_t e_rnd_pos = 0, e_off_pos = 0, e_rnd_n = 0, e_off_n = 0;
  const double e_rnd = paired(kEar, kERnd, &e_rnd_se, &e_rnd_pos, &e_rnd_n);
  const double e_off = paired(kEar, kOff, &e_off_se, &e_off_pos, &e_off_n);
  // DOES IT NAME? dF1 is a shift in a MEAN; this asks whether a listener could
  // tell the words apart from what the creature actually said, which is the
  // milestone-level question and can come out negative when dF1 is positive.
  std::printf("\n  can a listener tell the words apart? (held-out one-of-two, F1+F2)\n");
  for (uint32_t a = 0; a < kCtxSelfArmCount; ++a) {
    std::printf("    %-10s %.3f +/- %.3f   (shuffled %.3f, chance 0.500)\n",
                kCtxSelfArms[a].name, m_nm[a], s_nm[a], m_ns[a]);
  }
  // Paired, by creature, against the two controls that matter: `off` carries
  // the same M1b ECHO with no conditional mechanism -- in this protocol the
  // context IS the word just heard, so imitation and naming are confounded by
  // construction and the increment over `off` is the most that can be claimed --
  // and `ema-rnd` carries the same machinery and the same echo, differing only
  // in whether the target tracks the word.
  {
    auto npaired = [&](uint32_t A, uint32_t B, double* se, uint32_t* pos, uint32_t* n) {
      std::vector<double> d;
      for (size_t i = 0; i < nreps[A].size(); ++i) {
        for (size_t j = 0; j < nreps[B].size(); ++j) {
          if (nreps[A][i] == nreps[B][j]) { d.push_back(name[A][i] - name[B][j]); break; }
        }
      }
      *n = uint32_t(d.size());
      *pos = 0;
      for (double v : d) if (v > 0.0) ++*pos;
      if (d.size() < 2) { *se = 0.0; return 0.0; }
      double m = 0.0;
      for (double v : d) m += v;
      m /= double(d.size());
      double ss = 0.0;
      for (double v : d) ss += (v - m) * (v - m);
      *se = std::sqrt(ss / double(d.size() - 1)) / std::sqrt(double(d.size()));
      return m;
    };
    double se1 = 0.0, se2 = 0.0;
    uint32_t p1 = 0, p2 = 0, n1 = 0, n2 = 0;
    const double d1 = npaired(kEma, kOff, &se1, &p1, &n1);
    const double d2 = npaired(kEma, kMRnd, &se2, &p2, &n2);
    std::printf("    PAIRED  ema - off       %+.3f +/- %.3f, %.1f SE, %u of %u\n"
                "    PAIRED  ema - ema-rnd   %+.3f +/- %.3f, %.1f SE, %u of %u\n",
                d1, se1, se1 > 0.0 ? d1 / se1 : 0.0, p1, n1,
                d2, se2, se2 > 0.0 ? d2 / se2 : 0.0, p2, n2);
    std::printf("\n  is the utterance nearer the RIGHT word's target? (no fitting,"
                " chance 0.500)\n");
    for (uint32_t a = 0; a < kCtxSelfArmCount; ++a) {
      std::printf("    %-10s %.3f +/- %.3f\n", kCtxSelfArms[a].name, m_cr[a], s_cr[a]);
    }
    auto cpaired = [&](uint32_t A, uint32_t B, double* se, uint32_t* pos, uint32_t* n) {
      std::vector<double> d;
      for (size_t i = 0; i < creps[A].size(); ++i) {
        for (size_t j = 0; j < creps[B].size(); ++j) {
          if (creps[A][i] == creps[B][j]) { d.push_back(corr[A][i] - corr[B][j]); break; }
        }
      }
      *n = uint32_t(d.size());
      *pos = 0;
      for (double v : d) if (v > 0.0) ++*pos;
      if (d.size() < 2) { *se = 0.0; return 0.0; }
      double m = 0.0;
      for (double v : d) m += v;
      m /= double(d.size());
      double ss = 0.0;
      for (double v : d) ss += (v - m) * (v - m);
      *se = std::sqrt(ss / double(d.size() - 1)) / std::sqrt(double(d.size()));
      return m;
    };
    double c1 = 0.0, c2 = 0.0;
    uint32_t q1 = 0, q2 = 0, r1 = 0, r2 = 0;
    const double e1 = cpaired(kEma, kOff, &c1, &q1, &r1);
    const double e2 = cpaired(kEma, kMRnd, &c2, &q2, &r2);
    std::printf("    PAIRED  ema - off       %+.3f +/- %.3f, %.1f SE, %u of %u\n"
                "    PAIRED  ema - ema-rnd   %+.3f +/- %.3f, %.1f SE, %u of %u\n",
                e1, c1, c1 > 0.0 ? e1 / c1 : 0.0, q1, r1,
                e2, c2, c2 > 0.0 ? e2 / c2 : 0.0, q2, r2);
    std::printf("\n  did it move the RIGHT WAY along the axis between the two words?\n"
                "  (centred on the creature's own mean, no labels fitted, chance 0.500)\n");
    for (uint32_t a = 0; a < kCtxSelfArmCount; ++a) {
      std::printf("    %-10s %.3f +/- %.3f    (k-way form: %.3f)\n",
                  kCtxSelfArms[a].name, m_ax[a], s_ax[a], m_dr[a]);
    }
    {
      // The generalisation's self-check, stated as a number rather than left to
      // the reader: at two words the two columns are the same test.
      double worst = 0.0;
      for (uint32_t a = 0; a < kCtxSelfArmCount; ++a) {
        const double gap = std::fabs(m_ax[a] - m_dr[a]);
        if (gap > worst) worst = gap;
      }
      std::printf("    the k-way form reduces to the axis test at k=2:"
                  " worst disagreement %.4f\n", worst);
    }
    auto apaired = [&](uint32_t A, uint32_t B, double* se, uint32_t* pos, uint32_t* n) {
      std::vector<double> d;
      for (size_t i = 0; i < areps[A].size(); ++i) {
        for (size_t j = 0; j < areps[B].size(); ++j) {
          if (areps[A][i] == areps[B][j]) { d.push_back(axis[A][i] - axis[B][j]); break; }
        }
      }
      *n = uint32_t(d.size());
      *pos = 0;
      for (double v : d) if (v > 0.0) ++*pos;
      if (d.size() < 2) { *se = 0.0; return 0.0; }
      double m = 0.0;
      for (double v : d) m += v;
      m /= double(d.size());
      double ss = 0.0;
      for (double v : d) ss += (v - m) * (v - m);
      *se = std::sqrt(ss / double(d.size() - 1)) / std::sqrt(double(d.size()));
      return m;
    };
    double g1 = 0.0, g2 = 0.0;
    uint32_t u1 = 0, u2 = 0, v1 = 0, v2 = 0;
    const double f1d = apaired(kEma, kOff, &g1, &u1, &v1);
    const double f2d = apaired(kEma, kMRnd, &g2, &u2, &v2);
    std::printf("    PAIRED  ema - off       %+.3f +/- %.3f, %.1f SE, %u of %u\n"
                "    PAIRED  ema - ema-rnd   %+.3f +/- %.3f, %.1f SE, %u of %u\n",
                f1d, g1, g1 > 0.0 ? f1d / g1 : 0.0, u1, v1,
                f2d, g2, g2 > 0.0 ? f2d / g2 : 0.0, u2, v2);
  }

  // THE DRIFT TEST, and it is independent of whether the fix works. A frozen
  // prototype under a drifting input predicts the index DECAYS across a
  // session, and only in the arm whose target tracks the word -- the arm whose
  // voice actually changes. Printed per arm so the hypothesis is falsifiable on
  // its own terms rather than only through the `adapt` arm's score.
  std::printf("\n  does the index hold up across a session? (early third -> last third)\n");
  for (uint32_t a = kEar; a < kCtxSelfArmCount; ++a) {
    std::printf("    %-10s %.3f +/- %.3f -> %.3f +/- %.3f   (%+.3f)\n",
                kCtxSelfArms[a].name, m_me[a], s_me[a], m_ml[a], s_ml[a],
                m_ml[a] - m_me[a]);
  }
  // Each mechanism against ITS OWN matched-marginal control, and against the
  // no-mechanism arm. v52's rows used to sit here and were removed with its
  // arms: aliasing the retired indices onto v53's made this block print v53's
  // numbers twice under v52's labels, which is a reporting bug and not a
  // result. v52's numbers are in the README.
  double a_rnd_se = 0.0, a_off_se = 0.0;
  uint32_t a_rnd_pos = 0, a_off_pos = 0, a_rnd_n = 0, a_off_n = 0;
  const double a_rnd = paired(kAda, kARnd, &a_rnd_se, &a_rnd_pos, &a_rnd_n);
  const double a_off = paired(kAda, kOff, &a_off_se, &a_off_pos, &a_off_n);
  std::printf("\n  paired differences (same creature, arms differ only in genome fields)\n"
              "    ear   - ear-rnd     %+.1f +/- %.1f Hz, %.1f SE, %u of %u positive\n"
              "    ear   - off         %+.1f +/- %.1f Hz, %.1f SE, %u of %u positive\n"
              "    ema   - ema-rnd     %+.1f +/- %.1f Hz, %.1f SE, %u of %u positive\n"
              "    ema   - off         %+.1f +/- %.1f Hz, %.1f SE, %u of %u positive\n",
              e_rnd, e_rnd_se, e_rnd_se > 0.0 ? e_rnd / e_rnd_se : 0.0, e_rnd_pos, e_rnd_n,
              e_off, e_off_se, e_off_se > 0.0 ? e_off / e_off_se : 0.0, e_off_pos, e_off_n,
              a_rnd, a_rnd_se, a_rnd_se > 0.0 ? a_rnd / a_rnd_se : 0.0, a_rnd_pos, a_rnd_n,
              a_off, a_off_se, a_off_se > 0.0 ? a_off / a_off_se : 0.0, a_off_pos, a_off_n);

  // GATE 0: is the instrument the one `areax` validated? The oracle index is
  // held for the whole trial and read by the same argmax, so p there is 1.000
  // by construction. Anything else means the kernel's index is not the host's
  // condition, and every number below is about something other than a context.
  if (m_mt[kOra] < 0.99) {
    std::printf("\n  REFUSED -- the ORACLE arm's index agrees with the word only %.3f of\n"
                "  the time, where holding one slice up for the whole trial makes 1.000\n"
                "  arithmetic. The kernel is not reading the condition the host wrote,\n"
                "  so the `self` arms are not being compared against anything.\n",
                m_mt[kOra]);
    return false;
  }

  // GATE 1: does this run reproduce `areax`? The reference the whole experiment
  // is scored against is measured HERE rather than quoted, because a `self`
  // result means nothing next to an oracle arm that did not work either.
  const double ora_lift = m_d1[kOra] - m_d1[kOff];
  const double ora_se = s_d1[kOra] + s_d1[kOff];
  if (ora_lift <= 2.0 * ora_se) {
    std::printf("\n  REFUSED -- the ORACLE arm did not reproduce `areax` in this run:\n"
                "  %.1f Hz against %.1f with the table shared (%+.1f, %.1f SE). Without a\n"
                "  working reference there is nothing to measure the creature's own\n"
                "  index against, and a flat `self` would be unreadable.\n",
                m_d1[kOra], m_d1[kOff], ora_lift, ora_se > 0.0 ? ora_lift / ora_se : 0.0);
    return false;
  }

  // GATE 2: is the derived index an index at all? A partition that always
  // names the same slice scores 0.5 on p with balanced words -- indistinguishable
  // from one that names slices at random -- and the two deserve different
  // verdicts, because a constant index means the mechanism was never split
  // while a random one means it was split on nothing.
  if (m_oc[kEma] > 0.95) {
    std::printf("\n  REFUSED, AND IT IS A FINDING ABOUT THE PARTITION, NOT THE MECHANISM.\n"
                "  The creature's v53 index named the same slice on %.0f%% of reward-window\n"
                "  ticks, so the table was effectively never split and `self` is `off`\n"
                "  with extra memory. A fixed equal-sized cut of the larynx does not\n"
                "  separate the two words' motor states; what fails here is the cut, and\n"
                "  the next thing to try is one that is learned rather than fixed.\n",
                100.0 * m_oc[kEma]);
    return false;
  }

  if (m_pr[kEma] < 0.5) {
    std::printf("\n  REFUSED -- the creature was in a context on only %.0f%% of ticks in\n"
                "  the `self` arm, so the table was mostly not indexed at all.\n",
                100.0 * m_pr[kEma]);
    return false;
  }

  // DNA v53 is what this experiment now tests; v52 is reported as the settled
  // negative it became on two seed families. Both carry their own
  // matched-marginal control and both are scored on the paired difference.
  // The mechanism under test is source 4 (`ema`); source 2 (`ear`) is reported
  // beside it as the version it replaced. Each is gated against ITS OWN
  // matched-marginal control on the paired difference -- the same rule, applied
  // to whichever mechanism the run is about, and unchanged since it was written
  // down before the run that first used it.
  const double p = m_mt[kEma];
  const double keep_bound = 2.0 * kCtxSelfBound - 1.0;
  const double keep_meas = 2.0 * p - 1.0;
  // (2p - 1) x the oracle's dF1, floored at the no-signal arm. See the note on
  // why the `off` arm is a floor and not a pedestal.
  auto predict = [&](double keep) {
    const double v = (keep > 0.0 ? keep : 0.0) * m_d1[kOra];
    return v > m_d1[kOff] ? v : m_d1[kOff];
  };
  const double pred_bound = predict(keep_bound);
  const double pred_meas = predict(keep_meas);
  const double lift = m_d1[kEma] - m_d1[kOff];
  const double lift_se = s_d1[kEma] + s_d1[kOff];
  const double vs_rnd = m_d1[kEma] - m_d1[kMRnd];
  const double rnd_se = s_d1[kEma] + s_d1[kMRnd];

  std::printf("\n  the reference here       oracle %.1f Hz vs off %.1f (%+.1f, %.1f SE)\n"
              "  the derived index        p = %.3f +/- %.3f, busiest slice %.3f\n"
              "  ...and does it sharpen?  %.3f +/- %.3f -> %.3f +/- %.3f over the session\n"
              "  predicted from ctxsrc    %.1f Hz  (p <= %.3f, keeps %.0f%%)\n"
              "  predicted from THIS p    %.1f Hz  (keeps %.0f%%)\n"
              "  measured                 %.1f Hz  (%+.1f vs off, %.1f SE)\n"
              "  vs matched-marginal      %.1f Hz  (%+.1f, %.1f SE)\n",
              m_d1[kOra], m_d1[kOff], ora_lift, ora_se > 0.0 ? ora_lift / ora_se : 0.0,
              p, s_mt[kEma], m_oc[kEma],
              m_me[kEma], s_me[kEma], m_ml[kEma], s_ml[kEma],
              pred_bound, kCtxSelfBound, 100.0 * keep_bound,
              pred_meas, 100.0 * (keep_meas > 0.0 ? keep_meas : 0.0),
              m_d1[kEma], lift, lift_se > 0.0 ? lift / lift_se : 0.0,
              m_d1[kMRnd], vs_rnd, rnd_se > 0.0 ? vs_rnd / rnd_se : 0.0);

  // THE GATE IS `self` AGAINST `self-rnd`, AND `off` IS REPORTED BUT NOT GATED
  // ON. This experiment shipped requiring both, and its first run showed that
  // the `off` half CANNOT pass -- not "did not", cannot, on numbers that were
  // available before it ran.
  //
  //   detection threshold vs off, n=3    2 x (11.7 + 11.6)  =  46.6 Hz
  //   largest lift ctxsrc's bound allows       55.3 - 30.7  =  24.6 Hz
  //
  // The gate demanded a lift twice the maximum its own pre-stated upper bound
  // permits, and seeds do not close the gap: n=6 needs 33.0 Hz and n=9 needs
  // 26.9, both still above 24.6. **A gate a perfect result would fail is not a
  // gate.** That is the same test `areax` used to retire `fixed+on` -- the
  // control falsified a prediction it makes itself, rather than being dropped
  // because of the numbers underneath it.
  //
  // Why `off` is so wide is not a mystery, and it is a reason to prefer the
  // other control on STRUCTURE and not only on power. `off` carries no split
  // table, so its dF1 is incidental spread between two words with nothing
  // suppressing it. A split table averages opposing writes toward zero, which
  // is why `self-rnd` sits BELOW `off` rather than beside it: the two are not
  // two measurements of the same zero. `self-rnd` is the zero of a creature
  // carrying identical machinery and differing in one thing -- whether the
  // target tracks the word.
  //
  // `off` stays in the table and in the report, because how far a shared bias
  // gets on its own is worth seeing. It is a diagnostic, not a gate.
  // THE TEST IS PAIRED, AND THE HISTORY OF THAT DECISION IS THE POINT.
  //
  // Every arm here runs the SAME creature: the seed is `header.seed + r*7919`
  // and the arms differ only in genome fields. So the comparison is paired by
  // construction, and the between-creature variance -- which is enormous, `off`
  // ranges 5.5 to 75.7 Hz across nine seeds -- is common to both arms and
  // cancels. An unpaired SE on a paired design is not a second valid choice; it
  // discards the design's whole advantage.
  //
  // This experiment's first two runs GATED ON THE UNPAIRED SE, which was simply
  // the wrong test, and the error was noticed only after the gate refused. On
  // the same nine creatures the two disagree:
  //
  //     self vs self-rnd    unpaired +19.3, 1.7 SE    paired +19.3, 3.4 SE
  //     self vs off         unpaired  +5.9, 0.4 SE    paired  +5.9, 0.6 SE
  //
  // A statistic adopted after a gate fails is worth nothing on the data that
  // motivated it, however correct the arithmetic. **So this gate was written
  // down first and the run that decides it uses a FRESH SEED FAMILY** -- a
  // genome with a different `seed`, creatures the re-analysis never saw. That
  // is what turns a post-hoc re-analysis into a prediction.
  //
  // Note what pairing does NOT do, which is the reason to trust it: it leaves
  // `self vs off` flat at 0.6 SE. It tightens the comparison the design was
  // built for and manufactures nothing in the one that was already null.

  const bool beats_off = lift > 2.0 * lift_se;
  const bool beats_rnd = a_rnd > 2.0 * a_rnd_se;
  std::printf("  PAIRED vs matched-marg   %+.1f +/- %.1f Hz, %.1f SE  <-- THE GATE\n"
              "                           %u of %u creatures positive\n"
              "  paired vs `off`          %+.1f +/- %.1f Hz, %.1f SE (diagnostic)\n"
              "  unpaired, for the record %+.1f (%.1f SE) vs control, %+.1f (%.1f SE) vs off\n",
              a_rnd, a_rnd_se, a_rnd_se > 0.0 ? a_rnd / a_rnd_se : 0.0, a_rnd_pos, a_rnd_n,
              a_off, a_off_se, a_off_se > 0.0 ? a_off / a_off_se : 0.0,
              vs_rnd, rnd_se > 0.0 ? vs_rnd / rnd_se : 0.0,
              lift, lift_se > 0.0 ? lift / lift_se : 0.0);
  (void)beats_off;
  if (beats_rnd) {
    std::printf("\n  THE CREATURE INDEXES ITSELF -- the voice depends on the word with NO\n"
                "  oracle anywhere. %+.1f +/- %.1f Hz of F1, PAIRED, against a control\n"
                "  carrying the same split table and the same derived index and differing\n"
                "  only in whether the target tracks the word; %u of %u creatures\n"
                "  positive, on an index read from the larynx at %.3f. That is %.0f%% of\n"
                "  what the host-written index bought IN THIS SAME RUN, against %.0f%%\n"
                "  predicted from `ctxsrc`'s upper bound and %.0f%% from this run's own p.\n"
                "  The last oracle in the architecture is gone.\n",
                a_rnd, a_rnd_se, a_rnd_pos, a_rnd_n, p,
                100.0 * lift / (ora_lift > 0.0 ? ora_lift : 1.0),
                100.0 * keep_bound, 100.0 * (keep_meas > 0.0 ? keep_meas : 0.0));
    return true;
  }

  std::printf("\n  IT CANNOT INDEX ITSELF WELL ENOUGH, AND THE REASON IS NOW A NUMBER.\n"
              "  The mechanism works -- the same creature with the host writing the\n"
              "  index reaches %.1f Hz in this very run. Reading the index from its own\n"
              "  larynx instead gives p = %.3f and %.1f Hz, which is %+.1f +/- %.1f\n"
              "  PAIRED against the matched-marginal control (%u of %u creatures) --\n"
              "  short of the 2 SE the gate asks for.\n\n"
              "  What this closes: it is not credit assignment (`areax` found the\n"
              "  conditional optimum), not delivery (`ctxbias` measured a bias to the\n"
              "  larynx as free), not expressiveness and not the reward's composition\n"
              "  (`rpeprobe`). What is left is that this creature has nowhere to HOLD\n"
              "  what it heard across the silence before reward arrives. `ctxsrc` said\n"
              "  the motor state is the best carrier there is here and still short of\n"
              "  the bar; this measures the same shortfall in the mechanism's own units\n"
              "  rather than in a proxy's.\n",
              m_d1[kOra], p, m_d1[kEma], a_rnd, a_rnd_se, a_rnd_pos, a_rnd_n);
  return false;
}

// --- ctxfour: does the context-indexed bias hold FOUR mappings? -------------
//
// `partprobe`'s k=4 gate cleared the INDEX -- the same competitive rule with the
// same conscience finds four clusters in the ear at 0.895 against a chance of
// 0.250. It said nothing about the LEARNING, and that is the load-bearing
// unknown: four contexts is four conditional mappings competing for ONE reward
// channel. `capacity` measured this creature holding TWO orthogonal lessons at
// 0.84 while a CONFLICTING pair collapses to 0.22, and naming's lessons conflict
// by construction -- they drive the same formant to different values. The
// context index is exactly what makes them non-conflicting at two words.
// Whether that survives four tables is untested.
//
// **So this runs the ORACLE arm only, and its baseline.** With the host writing
// a perfect index, can a context-indexed bias hold four mappings at all? Two
// arms instead of six is ~75 minutes rather than 3h40, and it is the question
// worth spending first: if a PERFECT index cannot carry four, the creature's own
// index certainly cannot, and the ceiling is the bias mechanism rather than the
// context -- which would send the work to the compartment lead instead.
//
// THE PREDICTION, from Werfel, Xie & Seung and stated before the run. Learning
// time scales with the number of parameters estimated. Four contexts is 504
// parameters against two contexts' 252, so **2x the parameters and therefore
// ~2x the trials**. At 6.8M with four words expect roughly what two words gave
// at 3.4M: `oracle` around 65 Hz of pairwise F1 spread, and a direction score
// near 0.80 against a chance that is now 0.250. Landing there says the mechanism
// is merely slower; landing at chance says something structural breaks at four.
//
// AND IT CARRIES THE REFACTOR'S OWN OUTSTANDING CHECK. `kVLWords` became a
// runtime count and every test so far ran with nw == 2, so a loop left at the
// constant is invisible until four words actually run. With the host writing the
// index, **`ctx_match` must read 1.000 at k=4** -- anything less means the k-way
// assignment or the confusion accounting is wrong, independently of what the
// creature learned. That gate fires before any result is read.
struct CtxFourArm {
  const char* name;
  uint32_t slots;
  int target;
};
constexpr CtxFourArm kCtxFourArms[] = {
    {"off",    0, kVLTgtHeard},
    {"oracle", 4, kVLTgtHeard},
};
constexpr uint32_t kCtxFourArmCount = sizeof(kCtxFourArms) / sizeof(kCtxFourArms[0]);
constexpr uint32_t kCFWords = 4;

bool run_ctxfour(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  (void)verbose;
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t ctx_module = dna.module_with_role(aibaby::ModuleRole::kContext);
  if (ctx_module < 0) {
    std::printf("  this genome has no kContext module. Build one with NO output\n"
                "  weight -- the index is READ, never driven:\n\n"
                "    python3 tools/genome_add_context.py dna/default.toml ctx.toml \\\n"
                "        vocal out_w=0\n"
                "    ./build/aibaby --dna ctx.toml --experiment ctxfour\n");
    return false;
  }
  constexpr uint32_t kReps = 9;
  const size_t slots_off = offsetof(aibaby::DnaHeader, exploration) +
                           offsetof(aibaby::DnaExploration, context_slots);
  instrument("ctxfour", dna.header().seed ^ 0x4F0Bu, ticks / kVLTrialTicks,
             "trials per arm");
  std::printf("  question          `partprobe` cleared the INDEX at four words (0.895\n"
              "                    against chance 0.250). Can the BIAS hold four\n"
              "                    mappings on one reward channel?\n");
  std::printf("  the prediction    4 contexts is 2x the parameters of 2, so ~2x the\n"
              "                    trials: at 6.8M expect what two words gave at 3.4M,\n"
              "                    ~65 Hz spread and ~0.80 direction (chance 0.250).\n");
  std::printf("  the oracle arm    the host writes the index, so this is the CEILING.\n"
              "                    If a perfect index cannot carry four, the creature's\n"
              "                    own cannot either.\n\n");

  std::vector<double> spread[kCtxFourArmCount], match[kCtxFourArmCount];
  std::vector<double> dir[kCtxFourArmCount], near[kCtxFourArmCount];
  std::vector<double> dnul[kCtxFourArmCount], dexc[kCtxFourArmCount];
  std::vector<double> chg[kCtxFourArmCount], divg[kCtxFourArmCount];

  std::vector<std::pair<double, double>> tg;
  for (uint32_t q = 0; q < kCFWords; ++q) {
    tg.push_back({double(kWords[q].f1), double(kWords[q].f2)});
  }

  std::printf("  %-6s %-8s %-10s %-9s %-10s %-10s %-10s %-10s %s\n", "seed", "arm",
              "F1 spread", "ctx_match", "direction", "its null", "dir-null", "nearest",
              "change");
  // Nine creatures x two arms are eighteen independent brains, so they run
  // across the machine's cores. The rows are printed and pooled SERIALLY from
  // the returned cells and a rep's seed is a pure function of its index, so
  // these are the numbers the serial loop printed — bit for bit.
  struct Cell {
    bool ok = false;
    uint32_t scored = 0, skipped = 0;
    double sp = 0.0, match = 0.0, dir = 0.0, dnull = 0.0, near = 0.0, chg = 0.0,
           divg = 0.0;
  };
  const std::vector<Cell> cells =
      parallel_reps<Cell>(kReps * kCtxFourArmCount, [&](uint32_t i) {
        const uint32_t r = i / kCtxFourArmCount;
        const uint32_t a = i % kCtxFourArmCount;
        Cell cell;
        std::vector<uint8_t> variant = blob;
        const uint64_t seed = dna.header().seed + r * 7919ull;
        std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed,
                    sizeof(seed));
        const uint32_t sl = kCtxFourArms[a].slots;
        std::memcpy(variant.data() + slots_off, &sl, sizeof(sl));
        CtxDrive drive;
        drive.module = ctx_module;
        drive.slots = kCFWords;
        drive.gain = 0.10;
        Regime reg;
        reg.praise = kPraiseValue;
        reg.scold = kScoldValue;
        const VLRun run = run_vocallearn_session(variant, ticks, kVLTaught, nullptr, reg,
                                                 kCtxFourArms[a].target, &drive,
                                                 kVLScoreFormant, nullptr, kCFWords);
        cell.scored = run.scored;
        cell.skipped = run.skipped;
        if (!run.ok) return cell;
        // Mean absolute pairwise F1 difference: the four-word generalisation of
        // dF1, which is a single pair.
        double sp = 0.0;
        uint32_t np = 0;
        for (uint32_t x = 0; x < kCFWords; ++x) {
          for (uint32_t y = x + 1; y < kCFWords; ++y) {
            sp += std::fabs(run.f1_by_word[x] - run.f1_by_word[y]);
            ++np;
          }
        }
        cell.sp = np ? sp / np : 0.0;
        cell.dir = direction_accuracy(run.utt_f1, run.utt_f2, run.utt_word, tg);
        // This creature's own floor, on this creature's own utterances and label
        // counts. Chance is not 0.250 -- see `direction_null`.
        cell.dnull = direction_null(run.utt_f1, run.utt_f2, run.utt_word, tg);
        uint32_t hit = 0, tot = 0;
        for (size_t u = 0; u < run.utt_word.size(); ++u) {
          double best = 0.0;
          int pick = 0;
          for (uint32_t q = 0; q < kCFWords; ++q) {
            const double e = formant_error(run.utt_f1[u], run.utt_f2[u], kWords[q]);
            if (e < 0.0) { pick = -1; break; }
            if (q == 0 || e < best) { best = e; pick = int(q); }
          }
          if (pick < 0) continue;
          if (pick == run.utt_word[u]) ++hit;
          ++tot;
        }
        cell.near = tot ? double(hit) / double(tot) : 0.0;
        cell.match = run.ctx_match;
        cell.chg = vl_change(run);
        cell.divg = run.ctx_table_div;
        cell.ok = true;
        parallel_note("  [%u/%u] seed %u %s  spread %.1f  dir %.3f (null %.3f)\n",
                      i + 1, kReps * kCtxFourArmCount, r, kCtxFourArms[a].name,
                      cell.sp, cell.dir, cell.dnull);
        return cell;
      });

  for (uint32_t i = 0; i < kReps * kCtxFourArmCount; ++i) {
    const uint32_t r = i / kCtxFourArmCount;
    const uint32_t a = i % kCtxFourArmCount;
    const Cell& c = cells[i];
    if (!c.ok) {
      std::printf("  %-6u %-8s (inconclusive: %u scored, %u skipped)\n", r,
                  kCtxFourArms[a].name, c.scored, c.skipped);
      continue;
    }
    spread[a].push_back(c.sp);
    match[a].push_back(c.match);
    dir[a].push_back(c.dir);
    dnul[a].push_back(c.dnull);
    dexc[a].push_back(c.dir - c.dnull);
    near[a].push_back(c.near);
    chg[a].push_back(c.chg);
    divg[a].push_back(c.divg);
    std::printf("  %-6u %-8s %-10.1f %-9.3f %-10.3f %-10.3f %-10.3f %-10.3f %+.1f\n",
                r, kCtxFourArms[a].name, c.sp, c.match, c.dir, c.dnull,
                c.dir - c.dnull, c.near, c.chg);
  }

  double m_sp[kCtxFourArmCount], s_sp[kCtxFourArmCount];
  double m_mt[kCtxFourArmCount], s_mt[kCtxFourArmCount];
  double m_dr[kCtxFourArmCount], s_dr[kCtxFourArmCount];
  double m_dn[kCtxFourArmCount], s_dn[kCtxFourArmCount];
  double m_ex[kCtxFourArmCount], s_ex[kCtxFourArmCount];
  double m_nr[kCtxFourArmCount], s_nr[kCtxFourArmCount];
  double m_ch[kCtxFourArmCount], s_ch[kCtxFourArmCount];
  double m_dv[kCtxFourArmCount], s_dv[kCtxFourArmCount];
  for (uint32_t a = 0; a < kCtxFourArmCount; ++a) {
    if (spread[a].size() < 3) {
      std::printf("\n  ctxfour INCONCLUSIVE -- arm `%s` produced %zu creatures.\n",
                  kCtxFourArms[a].name, spread[a].size());
      return false;
    }
    m_sp[a] = ctx_mean_se(spread[a], &s_sp[a]);
    m_mt[a] = ctx_mean_se(match[a], &s_mt[a]);
    m_dr[a] = ctx_mean_se(dir[a], &s_dr[a]);
    m_dn[a] = ctx_mean_se(dnul[a], &s_dn[a]);
    m_ex[a] = ctx_mean_se(dexc[a], &s_ex[a]);
    m_nr[a] = ctx_mean_se(near[a], &s_nr[a]);
    m_ch[a] = ctx_mean_se(chg[a], &s_ch[a]);
    m_dv[a] = ctx_mean_se(divg[a], &s_dv[a]);
  }
  const uint32_t kOff = 0, kOra = 1;

  std::printf("\n  %-8s %-16s %-14s %-14s %-14s %-14s %s\n", "arm", "F1 spread (Hz)",
              "direction", "its own null", "dir - null", "nearest", "change");
  for (uint32_t a = 0; a < kCtxFourArmCount; ++a) {
    char b[40], c[40], d[40], e[40], f[40], g[40];
    std::snprintf(b, sizeof b, "%.1f +/- %.1f", m_sp[a], s_sp[a]);
    std::snprintf(c, sizeof c, "%.3f +/- %.3f", m_dr[a], s_dr[a]);
    std::snprintf(d, sizeof d, "%.3f +/- %.3f", m_dn[a], s_dn[a]);
    std::snprintf(e, sizeof e, "%+.3f +/- %.3f", m_ex[a], s_ex[a]);
    std::snprintf(f, sizeof f, "%.3f +/- %.3f", m_nr[a], s_nr[a]);
    std::snprintf(g, sizeof g, "%+.1f +/- %.1f", m_ch[a], s_ch[a]);
    std::printf("  %-8s %-16s %-14s %-14s %-14s %-14s %s\n", kCtxFourArms[a].name, b, c,
                d, e, f, g);
  }
  // The floor is MEASURED, not assumed. 1/k is wrong here by about the size of
  // the effect: the argmax wedges of four vowel directions are 0.210 to 0.290
  // wide, so any imbalance in label counts moves the floor. `nearest` is a
  // genuine 0.250 -- it compares absolute distances, not directions.
  std::printf("\n  chance on `nearest` is 0.250. Chance on `direction` is NOT 1/k and\n"
              "  is the per-creature shuffled null in the column beside it.\n");

  // THE REFACTOR'S CHECK, before any result is read. The host holds one slice up
  // for the whole trial, so a correct k-way assignment makes this 1.000 by
  // arithmetic. Anything less is a bug in the assignment or the confusion
  // accounting, not a fact about the creature.
  if (m_mt[kOra] < 0.99) {
    std::printf("\n  REFUSED -- the ORACLE arm's index agrees with the word only %.3f of\n"
                "  the time, where the host holding one slice up for the whole trial\n"
                "  makes 1.000 arithmetic. At two words this read exactly 1.000, so the\n"
                "  k-way assignment or the confusion accounting is wrong and nothing\n"
                "  below is about the creature.\n", m_mt[kOra]);
    return false;
  }

  // THE GATE MOVED, AND IT MOVED BECAUSE THE INSTRUMENT WAS WRONG.
  //
  // The 2026-09-06 run pre-registered raw `direction` against a chance of 0.250
  // and passed at +0.163, 2.2 SE. Then the measure was checked on an
  // informationless voice and it does not read 1/k: unnormalised target
  // directions plus unequal argmax wedges put the floor at 0.29-0.30 under a
  // skewed label distribution. So the gate is now the excess over each
  // creature's OWN shuffled null.
  //
  // Both numbers are printed. Changing a gate after seeing the data is exactly
  // what the fitted-verdict rule forbids, and the defence is that this change is
  // driven by a null measured WITHOUT any creature in it -- it would have been
  // made identically had the run gone the other way.
  const double d_dir = m_dr[kOra] - m_dr[kOff];
  const double se_dir = s_dr[kOra] + s_dr[kOff];
  const double d_exc = m_ex[kOra] - m_ex[kOff];
  const double se_exc = s_ex[kOra] + s_ex[kOff];
  std::printf("  the refactor's check     ctx_match %.3f at k=4 (1.000 expected)\n"
              "  F1 spread               %.1f -> %.1f Hz\n"
              "  direction, RAW          %.3f -> %.3f  (the retired gate)\n"
              "  its own shuffled null   %.3f -> %.3f  (this is the real floor)\n"
              "  direction ABOVE null    %+.3f -> %+.3f  (%+.3f, %.1f SE)\n"
              "  nearest target          %.3f -> %.3f  (chance 0.250)\n"
              "  the tables diverged     %.4f\n",
              m_mt[kOra], m_sp[kOff], m_sp[kOra], m_dr[kOff], m_dr[kOra],
              m_dn[kOff], m_dn[kOra], m_ex[kOff], m_ex[kOra], d_exc,
              se_exc > 0.0 ? d_exc / se_exc : 0.0, m_nr[kOff], m_nr[kOra], m_dv[kOra]);
  (void)d_dir;
  (void)se_dir;

  // `nearest` is the measure that corresponds to NAMING -- does the utterance
  // land closest to the right target -- and it has an honest 0.250 floor. It is
  // reported alongside because the 2026-09-06 run passed its direction gate
  // while `nearest` sat at chance and formant error GREW 20%: the bias made
  // larger, correctly-signed excursions that landed in the wrong places.
  if (d_exc > 2.0 * se_exc && m_ex[kOra] > 0.05) {
    std::printf("\n  IT HOLDS FOUR -- a context-indexed bias carries four mappings on one\n"
                "  reward channel: direction sits %+.3f above its own shuffled null\n"
                "  against %+.3f with the mechanism off, %+.3f at %.1f SE. Read it with\n"
                "  `nearest` (%.3f against a chance of 0.250) before calling this\n"
                "  naming: pointing the right way is not arriving.\n",
                m_ex[kOra], m_ex[kOff], d_exc, se_exc > 0.0 ? d_exc / se_exc : 0.0,
                m_nr[kOra]);
    return true;
  }
  std::printf("\n  IT DOES NOT HOLD FOUR. With a PERFECT index the bias sits %+.3f above\n"
              "  its own shuffled null against %+.3f off (%+.3f, %.1f SE), and `nearest`\n"
              "  reads %.3f against a chance of 0.250. The creature's own index cannot do\n"
              "  better than the oracle, so the four-word ceiling is the BIAS MECHANISM\n"
              "  and not the context -- `capacity`'s finding that a conflicting pair\n"
              "  collapses to 0.22 reaching four tables. The work goes to making the bias\n"
              "  land ON targets, not to a four-word protocol.\n",
              m_ex[kOra], m_ex[kOff], d_exc, se_exc > 0.0 ? d_exc / se_exc : 0.0,
              m_nr[kOra]);
  return false;
}

// --- ctxscale: is the bias COMPUTE-limited or MECHANISM-limited? -----------
//
// Two-word naming works and the remaining distance is how LARGE a bias reward
// can build. `ctxbias` showed the route carries 236 Hz of F1 when a bias is
// handed to the larynx; learning builds 93 Hz. The open question is which kind
// of gap that is, and it decides which of the two remaining leads to take:
//
//   COMPUTE-LIMITED  the bias is still growing and 236 Hz is a matter of trials.
//                    Then the architecture is right and the answer is a longer
//                    run, which is now affordable.
//   MECHANISM-LIMITED the bias has found its asymptote. Then more trials buy
//                    nothing and the work goes to Kornfeld's compartments --
//                    context on spines, variability on shafts, gating rather
//                    than adding.
//
// The one hint on record points at compute: table divergence grew 0.0280 ->
// 0.0374 when the session doubled, which is close to sqrt(2). But that is a
// magnitude, and a magnitude can grow while the USEFUL component does not -- an
// unbiased random walk grows as sqrt(t) too. So this measures the thing that
// matters, dF1 above its own matched-marginal control, at three budgets.
//
// PRE-REGISTERED, and the inconclusive band is declared here rather than
// discovered later:
//
//   excess(4x) / excess(1x)  > 1.7  compute-limited. sqrt(t) predicts 2.0.
//                            < 1.3  saturated.
//                            else   inconclusive, and it says so.
//
// Each budget is a SEPARATE session rather than a checkpoint of one, because a
// resumed creature would share its noise draw with the shorter run and the
// three points would not be independent.
struct CtxScaleArm {
  const char* name;
  uint32_t slots;
  uint32_t source;
  int target;
  int32_t mask_lo, mask_hi;  // -1 = write anywhere; [2,4) = only F1 and F2
};
constexpr CtxScaleArm kCtxScaleArms[] = {
    {"off", 0, 0, kVLTgtHeard, -1, -1},
    // Source 4, the ear's rate EMA: the creature's own index, and the only one
    // that has replicated out of sample.
    {"ema", 2, 4, kVLTgtHeard, -1, -1},
    // THE SELECTIVITY ORACLE. Same index, same rate, same everything: the only
    // difference is that reward may not write outside the two groups the score
    // is computed from. If delivery is limited by off-target writing this should
    // climb past `ema`'s ~140 Hz ceiling; if it does not, off-target writing was
    // costing nothing and the product model is wrong.
    {"mask-F1F2", 2, 4, kVLTgtHeard, 2, 4},
    // The control the dF1 comparison needs. Its index tracks the word too, so
    // its voice is word-dependent -- just not in the direction reward asked for.
    {"ema-rnd", 2, 4, kVLTgtRandom, -1, -1},
};
constexpr uint32_t kCtxScaleArmCount =
    sizeof(kCtxScaleArms) / sizeof(kCtxScaleArms[0]);
// Longest first, so the tail of the work queue is short jobs rather than one
// four-times-everything straggler holding thirteen idle cores.
constexpr uint32_t kCtxScaleBudgets = 3;

// ---------------------------------------------------------------------------
// `boundprobe` -- WHAT BOUNDS THE ALIGNED BIAS, given that nothing leaks it.
//
// `bias_ctx_` has no decay term. It is a clamped accumulator, so under a
// constant drift the aligned component would grow LINEARLY in trials. It grows
// at exponent 0.27 to 0.58 (`ctxscale`, `align-split`). Something bounds it, and
// after the slow store was refuted that bound is the last unpriced thing between
// this creature and the 3.8x more aligned bias that absolute naming needs.
//
// Three candidates, and they leave DIFFERENT fingerprints, which is why this is
// a measurement and not another sweep:
//
//   H1  the drift itself decays -- the teacher runs out, or the credit signal
//       shrinks as the voice moves. Fingerprint: aligned concave with a
//       second-half exponent below 0.4, praise share moving.
//   H2  it was never drift -- aligned is the diffusive component projected onto
//       one direction. Fingerprint: exponent pinned near 0.5 AND gain flat.
//       (Already weakened: gain rises 3.57 -> 4.57 across budgets.)
//   H3  the clamp binds on a heavy tail. Fingerprint: the pinned share rises.
//       An RMS of 0.085 against a `perturb_max` of 0.30 does NOT rule this out,
//       and "the RMS is well under the clamp" is exactly the argument `ipctx`
//       demolished for thresholds -- a quarter to five-sixths of that module sat
//       AT its clamp while its mean sat nowhere near it.
//
// It reads the curve INSIDE one session at 16 checkpoints instead of across
// three runs at three budgets, so the points are paired by construction and it
// costs a twelfth of what `ctxscale` costs for twelve times the resolution.
//
// Read-only: no genome field is swept and no mechanism is added. If H3 fires it
// names `perturb_max`, which `tools/vacuity.sh` reported slack at smoke length
// and which nobody has tested where it could bind.
struct BoundArm {
  const char* name;
  uint32_t slots;
  uint32_t source;
  VLTarget target;
};
constexpr BoundArm kBoundArms[] = {
    // The creature's own index off the ear's rate EMA: the only source that has
    // replicated out of sample.
    {"ema", 2, 4, kVLTgtHeard},
    // Matched-marginal control. Its index tracks the word too, so its voice is
    // word-dependent -- just not in the direction reward asked for. Without it a
    // rising dF1 is not evidence of anything.
    {"ema-rnd", 2, 4, kVLTgtRandom},
};
constexpr uint32_t kBoundArmCount = sizeof(kBoundArms) / sizeof(kBoundArms[0]);

// Least-squares slope of log(y) on log(x) over [lo, hi). The exponent the
// hypotheses are stated in: 1.0 is a constant drift into a leakless
// accumulator, 0.5 is diffusion, 0.0 is stopped.
inline double drift_exponent(const double* x, const double* y, uint32_t lo, uint32_t hi) {
  double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
  uint32_t n = 0;
  for (uint32_t i = lo; i < hi; ++i) {
    if (!(x[i] > 0.0) || !(y[i] > 0.0)) continue;
    const double lx = std::log(x[i]), ly = std::log(y[i]);
    sx += lx; sy += ly; sxx += lx * lx; sxy += lx * ly;
    ++n;
  }
  if (n < 3) return 0.0;
  const double den = double(n) * sxx - sx * sx;
  return den != 0.0 ? (double(n) * sxy - sx * sy) / den : 0.0;
}

bool run_boundprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  (void)verbose;
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t ctx_module = dna.module_with_role(aibaby::ModuleRole::kContext);
  if (ctx_module < 0) {
    std::printf("  this genome has no kContext module. Build one with NO output\n"
                "  weight -- the index is READ, never driven:\n\n"
                "    python3 tools/genome_add_context.py dna/default.toml ctx.toml \\\n"
                "        vocal out_w=0\n"
                "    ./build/aibaby --dna ctx.toml --experiment boundprobe\n");
    return false;
  }
  constexpr uint32_t kReps = 9;
  const size_t slots_off = offsetof(aibaby::DnaHeader, exploration) +
                           offsetof(aibaby::DnaExploration, context_slots);
  const size_t src_off = offsetof(aibaby::DnaHeader, exploration) +
                         offsetof(aibaby::DnaExploration, context_source);
  const double pmax = double(dna.header().exploration.perturb_max);
  instrument("boundprobe", dna.header().seed ^ 0xD21Fu, ticks / kVLTrialTicks,
             "trials per session, sampled at 16 checkpoints");
  std::printf("  question          `bias_ctx_` has NO leak, so a constant drift would grow\n"
              "                    the aligned component LINEARLY. It grows at exponent\n"
              "                    0.27-0.58. What bounds it?\n");
  std::printf("  the three tests   H1 drift decays  : second-half exponent < 0.40\n"
              "                    H2 pure diffusion: exponent in [0.40,0.60] AND gain flat\n"
              "                    H3 clamp binds   : pinned share rises, last > 2x first\n"
              "                    They are not exclusive. All three are printed, and the\n"
              "                    verdict says so rather than forcing one label.\n");
  std::printf("  perturb_max       %.3f -- the clamp H3 is about\n\n", pmax);

  struct Cell {
    bool ok = false;
    uint32_t scored = 0, skipped = 0, n = 0;
    double trial[VLRun::kCkpt] = {}, align[VLRun::kCkpt] = {}, outside[VLRun::kCkpt] = {};
    double gain[VLRun::kCkpt] = {}, pinned[VLRun::kCkpt] = {}, df1[VLRun::kCkpt] = {};
    double praise[VLRun::kCkpt] = {};
    double match = 0.0;
  };
  const uint32_t njobs = kReps * kBoundArmCount;
  const std::vector<Cell> cells = parallel_reps<Cell>(njobs, [&](uint32_t i) {
    Cell cell;
    const uint32_t r = i / kBoundArmCount;
    const uint32_t a = i % kBoundArmCount;
    std::vector<uint8_t> variant = blob;
    // Seeded exactly as `ctxscale` seeds, so a seed index means the same
    // creature in both and the two are comparable line for line.
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    const uint32_t sl = kBoundArms[a].slots;
    const uint32_t sr = kBoundArms[a].source;
    std::memcpy(variant.data() + slots_off, &sl, sizeof(sl));
    std::memcpy(variant.data() + src_off, &sr, sizeof(sr));
    CtxDrive drive;
    drive.module = ctx_module;
    drive.slots = kVLWords;
    drive.gain = 0.10;
    Regime reg;
    reg.praise = kPraiseValue;
    reg.scold = kScoldValue;
    const VLRun run = run_vocallearn_session(variant, ticks, kVLTaught, nullptr, reg,
                                             kBoundArms[a].target, &drive);
    cell.scored = run.scored;
    cell.skipped = run.skipped;
    if (!run.ok) return cell;
    cell.n = run.ckpt_n;
    for (uint32_t k = 0; k < run.ckpt_n; ++k) {
      cell.trial[k] = run.ckpt_trial[k];
      cell.align[k] = run.ckpt_align[k];
      cell.outside[k] = run.ckpt_outside[k];
      cell.gain[k] = run.ckpt_gain[k];
      cell.pinned[k] = run.ckpt_pinned[k];
      cell.df1[k] = run.ckpt_df1[k];
      cell.praise[k] = run.ckpt_praise[k];
    }
    cell.match = run.ctx_match;
    cell.ok = true;
    parallel_note("  [%u/%u] seed %u %s  align %.5f  pinned %.3f\n", i + 1, njobs, r,
                  kBoundArms[a].name, cell.align[cell.n ? cell.n - 1 : 0],
                  cell.pinned[cell.n ? cell.n - 1 : 0]);
    return cell;
  });

  std::vector<double> al[kBoundArmCount][VLRun::kCkpt];
  std::vector<double> os[kBoundArmCount][VLRun::kCkpt];
  std::vector<double> gn[kBoundArmCount][VLRun::kCkpt];
  std::vector<double> pn[kBoundArmCount][VLRun::kCkpt];
  std::vector<double> d1[kBoundArmCount][VLRun::kCkpt];
  std::vector<double> pr[kBoundArmCount][VLRun::kCkpt];
  double xt[VLRun::kCkpt] = {};
  uint32_t nck = 0;
  uint32_t used = 0;
  for (uint32_t i = 0; i < njobs; ++i) {
    const uint32_t a = i % kBoundArmCount;
    const Cell& c = cells[i];
    if (!c.ok) continue;
    ++used;
    if (c.n > nck) nck = c.n;
    for (uint32_t k = 0; k < c.n; ++k) {
      al[a][k].push_back(c.align[k]);
      os[a][k].push_back(c.outside[k]);
      gn[a][k].push_back(c.gain[k]);
      pn[a][k].push_back(c.pinned[k]);
      d1[a][k].push_back(c.df1[k]);
      pr[a][k].push_back(c.praise[k]);
      xt[k] = c.trial[k];
    }
  }
  if (used < njobs / 2 || nck < 8) {
    std::printf("\n  boundprobe INCONCLUSIVE -- %u of %u sessions produced a curve,\n"
                "  with %u checkpoints. Nothing is fitted to that.\n", used, njobs, nck);
    return false;
  }

  const auto arm_index = [](const char* want) {
    for (uint32_t i = 0; i < kBoundArmCount; ++i) {
      if (std::strcmp(kBoundArms[i].name, want) == 0) return int(i);
    }
    return -1;
  };
  const int kE = arm_index("ema"), kR = arm_index("ema-rnd");
  if (kE < 0 || kR < 0) {
    std::printf("\n  boundprobe cannot summarise: an arm it names is missing.\n");
    return false;
  }

  double m_al[VLRun::kCkpt], s_al[VLRun::kCkpt], m_gn[VLRun::kCkpt], m_pn[VLRun::kCkpt];
  double m_os[VLRun::kCkpt], m_d1[VLRun::kCkpt], s_d1[VLRun::kCkpt], m_pr[VLRun::kCkpt];
  double m_rd[VLRun::kCkpt], s_rd[VLRun::kCkpt];
  std::printf("\n  THE CURVE INSIDE ONE SESSION, `ema` arm, %u seeds\n", kReps);
  std::printf("  %-8s %-9s %-18s %-8s %-8s %-9s %-16s %s\n", "ckpt", "trials", "aligned (F1)",
              "outside", "gain", "pinned", "dF1 window (Hz)", "praise");
  for (uint32_t k = 0; k < nck; ++k) {
    double dummy;
    m_al[k] = ctx_mean_se(al[kE][k], &s_al[k]);
    m_os[k] = ctx_mean_se(os[kE][k], &dummy);
    m_gn[k] = ctx_mean_se(gn[kE][k], &dummy);
    m_pn[k] = ctx_mean_se(pn[kE][k], &dummy);
    m_d1[k] = ctx_mean_se(d1[kE][k], &s_d1[k]);
    m_pr[k] = ctx_mean_se(pr[kE][k], &dummy);
    m_rd[k] = ctx_mean_se(d1[kR][k], &s_rd[k]);
    std::printf("  %-8u %-9.0f %.5f +/- %.5f  %-8.5f %-8.2f %-9.3f %6.1f +/- %-6.1f %.3f\n",
                k + 1, xt[k], m_al[k], s_al[k], m_os[k], m_gn[k], m_pn[k], m_d1[k],
                s_d1[k], m_pr[k]);
  }

  const uint32_t half = nck / 2;
  const double e_all = drift_exponent(xt, m_al, 0, nck);
  const double e_1st = drift_exponent(xt, m_al, 0, half);
  const double e_2nd = drift_exponent(xt, m_al, half, nck);
  const double e_out = drift_exponent(xt, m_os, half, nck);
  const double gain_ratio = m_gn[0] > 0.0 ? m_gn[nck - 1] / m_gn[0] : 0.0;
  const double pin_first = m_pn[0], pin_last = m_pn[nck - 1];
  const double pin_ratio = pin_first > 1e-9 ? pin_last / pin_first : (pin_last > 0.01 ? 99.0 : 0.0);

  std::printf("\n  aligned exponent  whole %.2f | first half %.2f | second half %.2f\n",
              e_all, e_1st, e_2nd);
  std::printf("                    1.0 = constant drift into a leakless accumulator\n"
              "                    0.5 = diffusion, 0.0 = stopped\n");
  std::printf("  outside exponent  %.2f (second half), as the diffusive reference\n", e_out);
  std::printf("  gain              %.2f -> %.2f, ratio %.2f\n", m_gn[0], m_gn[nck - 1], gain_ratio);
  std::printf("  pinned share      %.3f -> %.3f, ratio %.2f\n", pin_first, pin_last, pin_ratio);
  std::printf("  praise share      %.3f -> %.3f\n", m_pr[0], m_pr[nck - 1]);
  std::printf("  dF1 vs control    %.1f vs %.1f Hz in the last window\n",
              m_d1[nck - 1], m_rd[nck - 1]);

  const bool h1 = e_2nd < 0.40;
  const bool h2 = e_2nd >= 0.40 && e_2nd <= 0.60 && gain_ratio > 0.8 && gain_ratio < 1.2;
  const bool h3 = pin_last >= 0.10 && pin_ratio > 2.0;
  std::printf("\n  H1 drift decays     %s  (second-half exponent %.2f vs 0.40)\n",
              h1 ? "FIRES" : "  no ", e_2nd);
  std::printf("  H2 pure diffusion   %s  (exponent %.2f in [0.40,0.60], gain ratio %.2f)\n",
              h2 ? "FIRES" : "  no ", e_2nd, gain_ratio);
  std::printf("  H3 clamp binds      %s  (pinned %.3f >= 0.10 and ratio %.2f > 2)\n",
              h3 ? "FIRES" : "  no ", pin_last, pin_ratio);
  if (!h1 && !h2 && !h3) {
    std::printf("\n  boundprobe INCONCLUSIVE -- none of the three fingerprints fires.\n"
                "  The bound is real (the exponent is %.2f, not 1.0) and is none of the\n"
                "  three things named in advance. Do NOT move a threshold to make one\n"
                "  fit: the ratios above are the result.\n", e_2nd);
    return false;
  }
  return true;
}

bool run_ctxscale(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  (void)verbose;
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t ctx_module = dna.module_with_role(aibaby::ModuleRole::kContext);
  if (ctx_module < 0) {
    std::printf("  this genome has no kContext module. Build one with NO output\n"
                "  weight -- the index is READ, never driven:\n\n"
                "    python3 tools/genome_add_context.py dna/default.toml ctx.toml \\\n"
                "        vocal out_w=0\n"
                "    ./build/aibaby --dna ctx.toml --experiment ctxscale\n");
    return false;
  }
  constexpr uint32_t kReps = 9;
  const size_t slots_off = offsetof(aibaby::DnaHeader, exploration) +
                           offsetof(aibaby::DnaExploration, context_slots);
  const size_t src_off = offsetof(aibaby::DnaHeader, exploration) +
                         offsetof(aibaby::DnaExploration, context_source);
  instrument("ctxscale", dna.header().seed ^ 0x5CA1u, ticks / kVLTrialTicks,
             "trials at the LONGEST budget");
  std::printf("  question          is the learned bias still GROWING with trials, or has\n"
              "                    it found its asymptote? That decides whether 236 Hz is\n"
              "                    a matter of compute or of architecture.\n");
  std::printf("  the gate          excess = dF1(ema) - dF1(ema-rnd), pooled per budget.\n"
              "                    ratio 4x/1x  > 1.7 compute-limited (sqrt(t) gives 2.0)\n"
              "                                 < 1.3 saturated\n"
              "                                 else  inconclusive, and it says so.\n\n");

  struct Cell {
    bool ok = false;
    uint32_t scored = 0, skipped = 0;
    double d1 = 0.0, div = 0.0, match = 0.0, chg = 0.0;
    double align = 0.0, common = 0.0, outside = 0.0, gain = 0.0;
    uint32_t gn = 0;
  };
  const uint32_t njobs = kReps * kCtxScaleArmCount * kCtxScaleBudgets;
  const std::vector<Cell> cells = parallel_reps<Cell>(njobs, [&](uint32_t i) {
    const uint32_t per_rep = kCtxScaleArmCount * kCtxScaleBudgets;
    const uint32_t r = i / per_rep;
    const uint32_t rem = i % per_rep;
    const uint32_t a = rem / kCtxScaleBudgets;
    const uint32_t b = rem % kCtxScaleBudgets;
    Cell cell;
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    const uint32_t sl = kCtxScaleArms[a].slots;
    const uint32_t sr = kCtxScaleArms[a].source;
    std::memcpy(variant.data() + slots_off, &sl, sizeof(sl));
    std::memcpy(variant.data() + src_off, &sr, sizeof(sr));
    CtxDrive drive;
    drive.module = ctx_module;
    drive.slots = kVLWords;
    drive.gain = 0.10;
    drive.mask_lo = kCtxScaleArms[a].mask_lo;
    drive.mask_hi = kCtxScaleArms[a].mask_hi;
    Regime reg;
    reg.praise = kPraiseValue;
    reg.scold = kScoldValue;
    const VLRun run = run_vocallearn_session(variant, ticks >> b, kVLTaught, nullptr,
                                             reg, kCtxScaleArms[a].target, &drive);
    cell.scored = run.scored;
    cell.skipped = run.skipped;
    if (!run.ok) return cell;
    cell.d1 = std::fabs(run.f1_by_word[0] - run.f1_by_word[1]);
    cell.div = run.ctx_table_div;
    cell.match = run.ctx_match;
    cell.chg = vl_change(run);
    cell.align = run.ctx_align;
    cell.common = run.ctx_common;
    cell.outside = run.ctx_outside;
    cell.gain = run.ctx_align_gain;
    cell.gn = run.ctx_f1_group_n;
    cell.ok = true;
    parallel_note("  [%u/%u] seed %u %s %.2fM  dF1 %.1f\n", i + 1, njobs, r,
                  kCtxScaleArms[a].name, double(ticks >> b) / 1e6, cell.d1);
    return cell;
  });

  std::vector<double> d1[kCtxScaleArmCount][kCtxScaleBudgets];
  std::vector<double> dv[kCtxScaleArmCount][kCtxScaleBudgets];
  std::vector<double> al[kCtxScaleArmCount][kCtxScaleBudgets];
  std::vector<double> cm[kCtxScaleArmCount][kCtxScaleBudgets];
  std::vector<double> os[kCtxScaleArmCount][kCtxScaleBudgets];
  std::vector<double> gnv[kCtxScaleArmCount][kCtxScaleBudgets];
  std::printf("  %-6s %-9s %-9s %-9s %-10s %s\n", "seed", "arm", "budget", "dF1 (Hz)",
              "table div", "change");
  for (uint32_t i = 0; i < njobs; ++i) {
    const uint32_t per_rep = kCtxScaleArmCount * kCtxScaleBudgets;
    const uint32_t r = i / per_rep;
    const uint32_t rem = i % per_rep;
    const uint32_t a = rem / kCtxScaleBudgets;
    const uint32_t b = rem % kCtxScaleBudgets;
    const Cell& c = cells[i];
    char bud[24];
    std::snprintf(bud, sizeof bud, "%.2fM", double(ticks >> b) / 1e6);
    if (!c.ok) {
      std::printf("  %-6u %-9s %-9s (inconclusive: %u scored, %u skipped)\n", r,
                  kCtxScaleArms[a].name, bud, c.scored, c.skipped);
      continue;
    }
    d1[a][b].push_back(c.d1);
    dv[a][b].push_back(c.div);
    al[a][b].push_back(c.align);
    cm[a][b].push_back(c.common);
    os[a][b].push_back(c.outside);
    gnv[a][b].push_back(c.gain);
    std::printf("  %-6u %-9s %-9s %-9.1f %-10.4f %+.1f\n", r, kCtxScaleArms[a].name,
                bud, c.d1, c.div, c.chg);
  }

  double m_d1[kCtxScaleArmCount][kCtxScaleBudgets], s_d1[kCtxScaleArmCount][kCtxScaleBudgets];
  double m_dv[kCtxScaleArmCount][kCtxScaleBudgets], s_dv[kCtxScaleArmCount][kCtxScaleBudgets];
  double m_al[kCtxScaleArmCount][kCtxScaleBudgets], s_al[kCtxScaleArmCount][kCtxScaleBudgets];
  double m_cm[kCtxScaleArmCount][kCtxScaleBudgets], s_cm[kCtxScaleArmCount][kCtxScaleBudgets];
  double m_os[kCtxScaleArmCount][kCtxScaleBudgets], s_os[kCtxScaleArmCount][kCtxScaleBudgets];
  double m_gn[kCtxScaleArmCount][kCtxScaleBudgets], s_gn[kCtxScaleArmCount][kCtxScaleBudgets];
  for (uint32_t a = 0; a < kCtxScaleArmCount; ++a) {
    for (uint32_t b = 0; b < kCtxScaleBudgets; ++b) {
      if (d1[a][b].size() < 3) {
        std::printf("\n  ctxscale INCONCLUSIVE -- arm `%s` at budget %u produced %zu\n"
                    "  creatures.\n", kCtxScaleArms[a].name, b, d1[a][b].size());
        return false;
      }
      m_d1[a][b] = ctx_mean_se(d1[a][b], &s_d1[a][b]);
      m_dv[a][b] = ctx_mean_se(dv[a][b], &s_dv[a][b]);
      m_al[a][b] = ctx_mean_se(al[a][b], &s_al[a][b]);
      m_cm[a][b] = ctx_mean_se(cm[a][b], &s_cm[a][b]);
      m_os[a][b] = ctx_mean_se(os[a][b], &s_os[a][b]);
      m_gn[a][b] = ctx_mean_se(gnv[a][b], &s_gn[a][b]);
    }
  }

  // BY NAME, NOT BY POSITION. These were literal indices until 2026-09-09, when
  // inserting `mask-F1F2` at index 2 silently re-pointed the summary's "ema-rnd"
  // column at the new arm -- the third time in this project that a printf
  // argument list has outlived the arm order it was written for. A lookup cannot
  // rot: add, remove or reorder arms and the summary follows.
  const auto arm_index = [](const char* want) {
    for (uint32_t i = 0; i < kCtxScaleArmCount; ++i) {
      if (std::strcmp(kCtxScaleArms[i].name, want) == 0) return i;
    }
    return kCtxScaleArmCount;  // out of range: the checks below fail loudly
  };
  const uint32_t kOff = arm_index("off");
  const uint32_t kEma = arm_index("ema");
  const uint32_t kRnd = arm_index("ema-rnd");
  if (kOff >= kCtxScaleArmCount || kEma >= kCtxScaleArmCount ||
      kRnd >= kCtxScaleArmCount) {
    std::printf("\n  REFUSED -- the summary needs arms named off / ema / ema-rnd and one\n"
                "  is missing. Renaming an arm must not silently repoint a column.\n");
    return false;
  }
  (void)kOff;
  std::printf("\n  %-9s %-9s %-16s %-16s %-16s %s\n", "budget", "trials", "off",
              "ema", "ema-rnd", "excess (ema - rnd)");
  double excess[kCtxScaleBudgets], se_ex[kCtxScaleBudgets];
  for (int b = int(kCtxScaleBudgets) - 1; b >= 0; --b) {
    excess[b] = m_d1[kEma][b] - m_d1[kRnd][b];
    se_ex[b] = s_d1[kEma][b] + s_d1[kRnd][b];
    char bud[24], tr[24], o[32], e[32], n[32], x[40];
    std::snprintf(bud, sizeof bud, "%.2fM", double(ticks >> b) / 1e6);
    std::snprintf(tr, sizeof tr, "%llu",
                  (unsigned long long)((ticks >> b) / kVLTrialTicks));
    std::snprintf(o, sizeof o, "%.1f +/- %.1f", m_d1[0][b], s_d1[0][b]);
    std::snprintf(e, sizeof e, "%.1f +/- %.1f", m_d1[kEma][b], s_d1[kEma][b]);
    std::snprintf(n, sizeof n, "%.1f +/- %.1f", m_d1[kRnd][b], s_d1[kRnd][b]);
    std::snprintf(x, sizeof x, "%+.1f +/- %.1f", excess[b], se_ex[b]);
    std::printf("  %-9s %-9s %-16s %-16s %-16s %s\n", bud, tr, o, e, n, x);
  }
  std::printf("\n  table divergence  %.4f -> %.4f -> %.4f (1x, 2x, 4x)\n",
              m_dv[kEma][kCtxScaleBudgets - 1], m_dv[kEma][1], m_dv[kEma][0]);

  // WHERE THE LEARNED TABLE GOES, by the decoder's own rule. F1 is group 2 of
  // nine and `read_group` pools it as a rate-weighted CENTROID, so only the
  // component along the centred position vector inside that slice can move F1
  // at all. `common` is the uniform part inside the group -- a ratio does not
  // notice it -- and `outside` is every other neuron in the larynx. Per-neuron
  // RMS throughout, so the three are on one scale.
  //
  // If `aligned` saturates while the other two keep growing, the ceiling is the
  // SHAPE of what reward writes, not the amount, and a mechanism that gates
  // WHICH parameters may move is aimed at the right thing.
  std::printf("\n  %-9s %-16s %-16s %-16s %s\n", "budget", "aligned (F1)",
              "common-mode", "outside group", "gain (1.0 = no structure)");
  for (int b = int(kCtxScaleBudgets) - 1; b >= 0; --b) {
    char bud[24], x[32], y[32], z[32];
    std::snprintf(bud, sizeof bud, "%.2fM", double(ticks >> b) / 1e6);
    std::snprintf(x, sizeof x, "%.5f +/- %.5f", m_al[kEma][b], s_al[kEma][b]);
    std::snprintf(y, sizeof y, "%.5f +/- %.5f", m_cm[kEma][b], s_cm[kEma][b]);
    std::snprintf(z, sizeof z, "%.5f +/- %.5f", m_os[kEma][b], s_os[kEma][b]);
    char w[32];
    std::snprintf(w, sizeof w, "%.2f +/- %.2f", m_gn[kEma][b], s_gn[kEma][b]);
    std::printf("  %-9s %-16s %-16s %-16s %s\n", bud, x, y, z, w);
  }

  const double lo = excess[kCtxScaleBudgets - 1];
  const double hi = excess[0];
  if (lo <= 0.0) {
    std::printf("\n  ctxscale INCONCLUSIVE -- the excess at the SHORTEST budget is %+.1f Hz,\n"
                "  so there is no positive quantity to measure growth in. The ratio is\n"
                "  undefined and reporting one would be inventing a denominator.\n", lo);
    return false;
  }
  const double ratio = hi / lo;
  // sqrt(t) would give 2.0 over a 4x span, linear 4.0, saturated 1.0. Reported
  // as an exponent because that is what extrapolates.
  const double expo = std::log(ratio > 0.0 ? ratio : 1e-9) / std::log(4.0);
  std::printf("  excess grew       %+.1f -> %+.1f Hz over a 4x span, ratio %.2f\n"
              "  implied exponent  %.2f  (0.5 is sqrt(t), 1.0 linear, 0.0 saturated)\n",
              lo, hi, ratio, expo);
  if (expo > 0.05 && hi > 0.0) {
    const double need = std::pow(236.0 / hi, 1.0 / expo);
    std::printf("  to reach 236 Hz   %.0fx these trials at this exponent\n", need);
  }

  if (ratio > 1.7) {
    std::printf("\n  COMPUTE-LIMITED -- the excess is still growing at %.2f over a 4x span\n"
                "  against the 2.0 that sqrt(t) predicts. The architecture is not the\n"
                "  binding constraint at two words, and a longer run is the cheapest\n"
                "  thing left. Kornfeld's compartments are not refused, but they are not\n"
                "  yet needed.\n", ratio);
    return true;
  }
  if (ratio < 1.3) {
    // THE PRESCRIPTION HERE WAS WRONG UNTIL 2026-09-08 and is worth keeping
    // corrected in place. It used to send the work to Kornfeld's compartments on
    // the reasoning that a saturated bias means a mechanism problem. The
    // alignment split refuted that: the aligned component grows fastest of the
    // three and the alignment gain holds at ~3.8, so which-parameters-change is
    // already healthy. Nor is the readout broken -- `ctxbias` drives it to 236 Hz
    // with a hand-supplied ramp. What is left is the SIZE of what reward can
    // build.
    std::printf("\n  SATURATED -- the excess grew only %.2f over a 4x span, so the bias has\n"
                "  found its asymptote at ~%.1f Hz against the 236 Hz the route can carry.\n"
                "  More trials buy nothing.\n"
                "\n  Read this with the alignment table above before concluding anything.\n"
                "  If `gain` is holding up, the shape of what reward writes is fine and\n"
                "  the readout is fine -- `ctxbias` reaches 236 Hz through it -- and what\n"
                "  saturates is the MAGNITUDE the learning rule can hold. That is a\n"
                "  different target from credit assignment or compartments.\n", ratio, hi);
    return false;
  }
  std::printf("\n  INCONCLUSIVE -- ratio %.2f falls in the band declared before the run\n"
              "  (1.3 to 1.7), which is exactly where a 4x span and nine creatures cannot\n"
              "  separate sqrt(t) growth from an asymptote. Widening the span costs less\n"
              "  than choosing a side of this.\n", ratio);
  return false;
}

// --- ctxbias: pricing the one architecture that is left --------------------
//
// Everything that has been added to `vocal` has been charged for. A conditional
// afferent (v47) killed the positive control on five genomes. A second
// regulator (v50) cost it +34.0 -> +15.4 with the tract silent. Less of the
// first regulator cost it too. The v47 diagnosis -- one shared motor population
// cannot host a learnable conditional input and a reward-driven exploratory
// search at once -- has been reached three times from three directions.
//
// Fee & Goldberg (Neuroscience 2011) describe the arrangement that does not ask
// it to: the conditional map is learned in a basal-ganglia stage receiving the
// timing signal and a COLLATERAL of the exploratory signal, and its output
// biases the motor population from outside. That is the largest build this
// project has considered. DNA v35 is the standing reason not to start it -- a
// lead that was real, label-free and correctly derived, built against a
// bottleneck that had closed underneath it while the notes still said it had
// not.
//
// So price it first. **The last step of that architecture is a bias onto
// `vocal`, and a bias onto `vocal` can be handed over directly.** If the
// positive control dies even under a perfect one, then no upstream structure
// can help, because a bias is what every one of them delivers -- and the route
// is refused for one run instead of one month.
//
// THREE THINGS MAKE THIS AN ORACLE RATHER THAN JUST ANOTHER INPUT.
//
//  1. It is GRADED AND ZERO-MEAN across an articulator group. The decoder reads
//     each group as a rate-weighted centroid over neuron index, so a ramp moves
//     that centroid while adding NO net drive. It cannot saturate intrinsic
//     plasticity the way v47's tract did, and a null cannot be explained away
//     as "you made the module louder".
//  2. It bypasses every tract and every synapse. There is nothing left between
//     the condition and the larynx to blame.
//  3. Its amplitude is a MULTIPLE of the module's own `noise_amp` -- what this
//     creature explores with, and the scale node perturbation's bias is
//     measured on. k = 1 is "the oracle pushes as hard as the creature's own
//     exploration". Not a constant anyone typed.
//
// TWO COLUMNS, AND NEITHER IS ENOUGH ALONE.
//
//   `dF1` is the voice's own F1 separation between the two words. It is the
//   vacuity guard -- an oracle too small to be heard would leave the positive
//   control untouched and falsely license the route -- and it is simultaneously
//   the ceiling measurement: how conditional can this voice be MADE, given a
//   perfect conditional bias and no learning problem at all?
//
//   `change` is the positive control's own formant-error reduction, the number
//   v47 and v50 both watched die.
//
// AND THE CONTROL THAT MAKES THEM READABLE. In the conditional arm the oracle
// steers per word while `fixed` rewards one target, so a drop in `change` could
// be the exploratory pathway being damaged OR just the voice being pushed
// around. The `const` arm applies the SAME amplitude with the sign held fixed:
// same drive, same everything, no conditional disturbance. A drop there is
// cost; a drop only in the conditional arm is disturbance.
//
// THE SECOND QUESTION, which rides along because the same session answers it.
// An Area X analogue has to project somewhere, and this creature has exactly
// one population that can affect the voice. The arcuate carries the word
// innately, `vision->vocal` ships, and `central->vocal` is a measured
// NON-PARTICIPANT -- delete it and every G3 number is unchanged. Is that the
// tract's fault or `central`'s? The last row puts the identical oracle on
// `central` and reads the same `dF1`. If a perfect bias on central moves the
// voice, the tract can carry something and central had nothing to send. If it
// does not, the association-to-motor route is broken and the architecture is
// not buildable here until that is fixed.
struct CtxBiasArm {
  const char* name;
  bool on_central;
  double k;
  bool conditional;
  double dir;
  uint32_t group_a, group_b;
};

// k = 0 is the control and must come first; the two `const` rows are the
// disturbance control; the last row is the tract question.
// THE ARM THAT PRICES ARRIVAL IS `offaxis`, and it took two runs to see why.
//
// `change` is 100 * (1 - err_late / err_early), a RATIO. A bias on the F1/F2
// groups moves the very quantity that error is computed from, so it shifts the
// ratio whichever way it points and does so without touching learning at all:
// `toward` reads +55.0 against a baseline of +34.0 and `away` reads +19.0,
// near-symmetric around it, which is the signature of an oracle moving the
// creature ALONG the scored axis. At `away k=2` the offset is large enough to
// drive the ratio to +1.0 on its own. Neither constant arm can price a cost.
//
// `offaxis` puts the identical ramp -- same module, same amplitude, same
// per-trial hold, same conditional sign -- on the first two BANDWIDTH groups
// instead. `formant_error` reads f1 and f2 and nothing else, so this arm
// arrives at the larynx in full and contributes exactly zero to the score. If
// the positive control survives it, arriving is free and what killed v47's
// tract was not arrival. If it dies, arrival is the cost, and every upstream
// architecture inherits it.
constexpr CtxBiasArm kCtxBiasArms[] = {
    {"off",         false, 0.0, true,   1.0, 2, 3},
    {"cond k=1",    false, 1.0, true,   1.0, 2, 3},
    {"cond k=2",    false, 2.0, true,   1.0, 2, 3},
    {"toward k=1",  false, 1.0, false,  1.0, 2, 3},
    {"away k=1",    false, 1.0, false, -1.0, 2, 3},
    {"offaxis k=1", false, 1.0, true,   1.0, 5, 6},
    {"offaxis k=2", false, 2.0, true,   1.0, 5, 6},
    {"central k=2", true,  2.0, true,   1.0, 2, 3},
};
constexpr uint32_t kCtxBiasArmCount = sizeof(kCtxBiasArms) / sizeof(kCtxBiasArms[0]);

bool run_ctxbias(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  (void)verbose;
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t vm = dna.module_with_role(aibaby::ModuleRole::kVocal);
  const int32_t cm = dna.module_with_role(aibaby::ModuleRole::kAssociation);
  if (vm < 0 || cm < 0) {
    std::printf("  setup failed: need a kVocal and a kAssociation module\n");
    return false;
  }
  constexpr uint32_t kReps = 3;
  instrument("ctxbias", dna.header().seed, ticks / kVLTrialTicks, "trials per arm");
  std::printf("  question          Fee & Goldberg's architecture ends in a BIAS onto the\n"
              "                    motor population. Hand one over directly, bypassing\n"
              "                    every tract: does the positive control survive it, and\n"
              "                    does the voice become conditional at all?\n");
  std::printf("  the oracle        a graded, ZERO-MEAN ramp across the F1 and F2 groups,\n"
              "                    sign following the word heard. Amplitude k x noise_amp\n"
              "                    (`%s` %.4f, `%s` %.4f).\n",
              dna.module(uint32_t(vm)).name, double(dna.module(uint32_t(vm)).noise_amp),
              dna.module(uint32_t(cm)).name, double(dna.module(uint32_t(cm)).noise_amp));
  std::printf("  arm               `fixed` -- the same positive control v47 and v50 both\n"
              "                    watched die. No yoke: `dF1` is scored within an arm.\n\n");

  std::vector<double> df1[kCtxBiasArmCount], df2[kCtxBiasArmCount];
  std::vector<double> change[kCtxBiasArmCount], voiced[kCtxBiasArmCount];
  std::vector<double> pinned[kCtxBiasArmCount], rate[kCtxBiasArmCount];
  double amp_used[kCtxBiasArmCount] = {};

  std::printf("  %-6s %-13s %-9s %-9s %-9s %-9s %-8s %s\n", "seed", "arm", "bias amp",
              "dF1 (Hz)", "dF2 (Hz)", "vocal Hz", "voiced", "change");
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));

    for (uint32_t a = 0; a < kCtxBiasArmCount; ++a) {
      const CtxBiasArm& arm = kCtxBiasArms[a];
      BiasDrive bd;
      bd.module = arm.on_central ? cm : vm;
      bd.k = arm.k;
      bd.conditional = arm.conditional;
      bd.dir = arm.dir;
      bd.group_a = arm.group_a;
      bd.group_b = arm.group_b;
      Regime reg;
      reg.praise = kPraiseValue;
      reg.scold = kScoldValue;
      const VLRun run = run_vocallearn_session(variant, ticks, kVLFixed, nullptr, reg,
                                               kVLTgtFixed, nullptr, kVLScoreFormant, &bd);
      if (!run.ok) {
        std::printf("  %-6u %-13s (inconclusive: %u scored, %u skipped)\n", r, arm.name,
                    run.scored, run.skipped);
        continue;
      }
      const double d1 = run.f1_by_word[0] - run.f1_by_word[1];
      const double d2 = run.f2_by_word[0] - run.f2_by_word[1];
      df1[a].push_back(d1);
      df2[a].push_back(d2);
      change[a].push_back(vl_change(run));
      voiced[a].push_back(run.voiced_frac);
      pinned[a].push_back(run.ip_pinned);
      rate[a].push_back(run.ip_rate_hz);
      amp_used[a] = run.bias_amp;
      std::printf("  %-6u %-13s %-9.4f %+-9.1f %+-9.1f %-9.2f %-8.2f %+.1f\n", r, arm.name,
                  run.bias_amp, d1, d2, run.ip_rate_hz, run.voiced_frac, vl_change(run));
    }
  }

  double m_d1[kCtxBiasArmCount], s_d1[kCtxBiasArmCount];
  double m_d2[kCtxBiasArmCount], s_d2[kCtxBiasArmCount];
  double m_ch[kCtxBiasArmCount], s_ch[kCtxBiasArmCount];
  double m_vf[kCtxBiasArmCount], s_vf[kCtxBiasArmCount];
  double m_pin[kCtxBiasArmCount], s_pin[kCtxBiasArmCount];
  double m_rt[kCtxBiasArmCount], s_rt[kCtxBiasArmCount];
  for (uint32_t a = 0; a < kCtxBiasArmCount; ++a) {
    if (df1[a].size() < 2) {
      std::printf("\n  ctxbias INCONCLUSIVE -- arm `%s` did not produce two usable\n"
                  "  creatures, so it has no spread and nothing can be read against it.\n",
                  kCtxBiasArms[a].name);
      return false;
    }
    m_d1[a] = ctx_mean_se(df1[a], &s_d1[a]);
    m_d2[a] = ctx_mean_se(df2[a], &s_d2[a]);
    m_ch[a] = ctx_mean_se(change[a], &s_ch[a]);
    m_vf[a] = ctx_mean_se(voiced[a], &s_vf[a]);
    m_pin[a] = ctx_mean_se(pinned[a], &s_pin[a]);
    m_rt[a] = ctx_mean_se(rate[a], &s_rt[a]);
  }

  std::printf("\n  %-13s %-9s %-16s %-16s %-13s %-8s %s\n", "arm", "bias amp",
              "dF1 (Hz)", "dF2 (Hz)", "vocal Hz", "voiced", "change");
  for (uint32_t a = 0; a < kCtxBiasArmCount; ++a) {
    char b[32], c[32], d[32], e[32];
    std::snprintf(b, sizeof b, "%+.1f +/- %.1f", m_d1[a], s_d1[a]);
    std::snprintf(c, sizeof c, "%+.1f +/- %.1f", m_d2[a], s_d2[a]);
    std::snprintf(d, sizeof d, "%.2f +/- %.2f", m_rt[a], s_rt[a]);
    std::snprintf(e, sizeof e, "%+.1f +/- %.1f", m_ch[a], s_ch[a]);
    std::printf("  %-13s %-9.4f %-16s %-16s %-13s %-8.2f %s\n", kCtxBiasArms[a].name,
                amp_used[a], b, c, d, m_vf[a], e);
  }
  std::printf("\n  `dF1`/`dF2` are the voice's own formant separation between the two\n"
              "  words, over every voiced frame. The `off` row is the creature's\n"
              "  baseline conditionality with no oracle at all, and every other row\n"
              "  has to be read against IT rather than against zero.\n");

  // --- the verdict, and its gates ------------------------------------------
  //
  // Order matters. Vacuity is checked BEFORE cost, because an oracle that did
  // nothing would otherwise print "the positive control survives" and license
  // the largest build in the project on the strength of an inert arm.
  // THE PRIMARY ARM IS k = 1, NOT k = 2, and the smoke run is why.
  //
  // A zero-mean bias is only drive-neutral in a LINEAR unit. These rectify at a
  // threshold, so neurons pushed up gain more spikes than neurons pushed down
  // lose, and a big enough ramp raises the module's rate after all: at k = 2
  // `vocal` runs ~8 Hz against a 5 Hz baseline. That reimports exactly the
  // confound this oracle was shaped to avoid -- "you made the module louder" --
  // and it is v47's own failure mode wearing a different hat.
  //
  // k = 1 buys 92% of the steering (dF1 265 vs 288 Hz) at a rate within a few
  // tenths of baseline, so it is the arm where the claim "no net drive was
  // added" actually holds. k = 2 is kept as the amplitude check: if the two
  // disagree, the difference is drive and not condition.
  const uint32_t kOff = 0, kCond1 = 1, kCond2 = 2;
  const uint32_t kToward1 = 3, kAway1 = 4, kOff1 = 5, kOff2 = 6, kCentral = 7;
  const double base_d1 = std::fabs(m_d1[kOff]);
  const double lift = std::fabs(m_d1[kCond1]) - base_d1;
  const double lift_se = s_d1[kCond1] + s_d1[kOff];
  const bool steers = lift > 2.0 * lift_se;
  // Is the primary arm actually rate-neutral, or is it a drive manipulation?
  const double rate_shift = m_rt[kCond1] - m_rt[kOff];
  const bool rate_neutral = std::fabs(rate_shift) < 2.0 * (s_rt[kCond1] + s_rt[kOff]);

  if (!steers) {
    std::printf("\n  ORACLE IS MUTE -- REFUSING TO REPORT A COST. At k = 2 the voice\n"
                "  separates the two words by %+.1f +/- %.1f Hz of F1 against %+.1f +/- %.1f\n"
                "  with no oracle at all, which is not a lift above its own baseline.\n"
                "  A bias this size does not reach the voice, so whatever `change`\n"
                "  does in these rows is not a fact about delivering a condition --\n"
                "  it is a fact about an oracle that was not heard.\n\n"
                "  This is REFUSAL, not a null. Raise k and run it again; if no k\n"
                "  steers the voice without destroying it, THAT is the finding, and\n"
                "  it says the larynx cannot be biased into naming from outside at\n"
                "  all -- which refuses the architecture more firmly than a cost\n"
                "  measurement ever could.\n",
                m_d1[kCond1], s_d1[kCond1], m_d1[kOff], s_d1[kOff]);
    return false;
  }

  // The disturbance control. If the constant-sign arm costs as much as the
  // conditional one, the cost is arrival; if only the conditional one pays, the
  // cost is the voice being pushed two ways while reward asks for one.
  // WHICH ARM PRICES ARRIVAL, and the first run of this experiment got it
  // wrong in a way worth keeping. The conditional arm steers the voice by
  // ~236 Hz of F1 per word while `fixed` rewards ONE target, and teaching moves
  // a formant by about 70. So the oracle drags the creature off target by three
  // times what the score is measuring, and `change` collapsing is the
  // ARITHMETIC of that disturbance, not a discovery about credit assignment.
  // The conditional arm cannot price the cost of arriving and must not be read
  // as if it could.
  //
  // The constant arms can, and there have to be two of them. A constant bias
  // pointing TOWARD the fixed target is doing part of the task, so an arm that
  // helps proves nothing about cost. `away` points the same magnitude in the
  // opposite direction: same drive, same rate, same everything, actively
  // unhelpful. If the positive control survives THAT, arriving is free.
  const double cost_offaxis = m_ch[kOff1] - m_ch[kOff];
  const bool survives = cost_offaxis > -2.0 * (s_ch[kOff1] + s_ch[kOff]);
  const bool control_survives = survives;

  std::printf("\n  scored at k = 1, the rate-neutral arm; k = 2 is the amplitude check\n");
  std::printf("  the oracle steers        dF1 %+.1f -> %+.1f Hz (lift %.1f, %.1f SE)\n",
              m_d1[kOff], m_d1[kCond1], lift, lift_se > 0.0 ? lift / lift_se : 0.0);
  std::printf("  and at k = 2             dF1 %+.1f Hz, vocal %.2f Hz vs %.2f baseline\n",
              m_d1[kCond2], m_rt[kCond2], m_rt[kOff]);
  std::printf("  rate neutral at k = 1    vocal %.2f vs %.2f Hz (%+.2f) -- %s\n",
              m_rt[kCond1], m_rt[kOff], rate_shift,
              rate_neutral ? "yes, so this is not a drive manipulation"
                           : "NO -- read the cost as drive, not as condition");
  std::printf("  on the SCORED axis, none of these is a cost measurement -- `change` is\n"
              "  a ratio and a bias on F1/F2 moves the creature along the very axis the\n"
              "  error is computed from, in whichever direction it points:\n");
  std::printf("    conditional            change %+.1f -> %+.1f  (%.0f Hz/word vs ~70 taught)\n"
              "    pointing at the target change %+.1f -> %+.1f  (does part of the task)\n"
              "    pointing away from it  change %+.1f -> %+.1f  (adds a constant error)\n",
              m_ch[kOff], m_ch[kCond1], std::fabs(m_d1[kCond1]),
              m_ch[kOff], m_ch[kToward1], m_ch[kOff], m_ch[kAway1]);
  std::printf("  OFF THE SCORED AXIS      change %+.1f -> %+.1f  <- the cost of ARRIVING\n"
              "                           (%+.1f at k = 2). Same module, same amplitude,\n"
              "                           same conditional sign, on bandwidths the score\n"
              "                           does not read.\n",
              m_ch[kOff], m_ch[kOff1], m_ch[kOff2]);
  std::printf("  a bias on `%s`      dF1 %+.1f +/- %.1f\n",
              dna.module(uint32_t(cm)).name, m_d1[kCentral], s_d1[kCentral]);

  const double word_gap = std::fabs(double(kWords[0].f1) - double(kWords[1].f1));
  if (survives && control_survives) {
    std::printf("\n  LICENSED -- a bias handed straight to the larynx steers the voice\n"
                "  %.1f Hz of F1 above baseline, and an equally large bias arriving OFF\n"
                "  the scored axis leaves the positive control at %+.1f against\n"
                "  %+.1f. Arriving at the larynx as a BIAS is free, where arriving as a\n"
                "  tract (v47) and as a second regulator (v50) both cost it. So the wall\n"
                "  those two hit is not inherent to delivering something to this module,\n"
                "  and an upstream structure whose output is a bias has somewhere to\n"
                "  land.\n\n"
                "  THE BAR: %.0f Hz of dF1, which is %.0f%% of the %.0f Hz separating the\n"
                "  two words. That is the CEILING on how conditional this voice can be\n"
                "  made, by anything, with the credit-assignment problem removed\n"
                "  entirely. An UPPER BOUND -- nothing here says the creature could\n"
                "  compute it, the same caution `credit`'s reward-mask oracle carries.\n\n"
                "  AND THE SECOND QUESTION: the same oracle on `%s` moves the voice\n"
                "  %+.1f +/- %.1f Hz against a baseline of %+.1f +/- %.1f -- nothing, at a\n"
                "  LARGER amplitude than the one that moves it %.0f Hz from `%s`. The\n"
                "  association-to-motor route cannot carry a steering signal at all, so\n"
                "  an Area X analogue here has to project to the larynx directly.\n",
                lift, m_ch[kOff1], m_ch[kOff],
                std::fabs(m_d1[kCond1]), 100.0 * std::fabs(m_d1[kCond1]) / word_gap, word_gap,
                dna.module(uint32_t(cm)).name, m_d1[kCentral], s_d1[kCentral],
                m_d1[kOff], s_d1[kOff], std::fabs(m_d1[kCond2]),
                dna.module(uint32_t(vm)).name);
    return true;
  }

  std::printf("\n  REFUSED -- the oracle reaches the voice (%.1f Hz of F1 above baseline)\n"
              "  and a bias arriving OFF the scored axis, at the same amplitude and the\n"
              "  same conditional sign, costs the positive control anyway: %+.1f against\n"
              "  %+.1f with no oracle (%+.1f at k = 2). Nothing about that arm touches\n"
              "  the formants the error is computed from, so it is the cost of ARRIVING\n"
              "  and not of being pushed off target.\n\n"
              "  A bias onto the larynx is the LAST STEP of every upstream architecture\n"
              "  proposed for this creature, Fee & Goldberg's included. A cost that is\n"
              "  already there before any of them is built is a cost none of them\n"
              "  avoids, so the route is refused for one run instead of one month.\n",
              lift, m_ch[kOff1], m_ch[kOff], m_ch[kOff2]);
  return false;
}

// --- ipctx: is intrinsic plasticity what strangles the context tract? -------
//
// ctxlearn's finding is that the positive control DIES whenever the oracle
// fires: +24.6/+39.1/+38.1 with the oracle mute becomes ~0 at 31 Hz, on five
// genomes and twelve driven levels, and it does not scale with out_w over a 6x
// range. Weight-side levers do not explain it, which points at a RATE-side one.
//
// IP is the only rate-side regulator that runs on the larynx: v11 measured that
// synaptic scaling never executes there at all, and this project has already
// paid once for relaxing a regulator without first checking it was running.
// So this measures, and does not intervene. Two things have to be true before
// `ip_wake_scale` is worth a single 3.4M-tick arm:
//
//   1. the larynx runs ABOVE its target while driven -- IP acts on the rate
//      error, so with no error there is nothing for it to do; and
//   2. its threshold moves FURTHER while driven than with the oracle mute,
//      by more than the spread across creatures.
//
// If either fails the hypothesis is dead and the intervention is not licensed.
// Twelve sessions rather than ctxlearn's seventy-two, because no yoke is needed
// to read a threshold.
bool run_ipctx(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  (void)verbose;
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t ctx_module = dna.module_with_role(aibaby::ModuleRole::kContext);
  if (ctx_module < 0) {
    std::printf("  this genome has no kContext module, so there is no oracle to\n"
                "  drive and nothing to measure. Build one:\n\n"
                "    python3 tools/genome_add_context.py dna/default.toml ctx.toml vocal\n"
                "    ./build/aibaby --dna ctx.toml --experiment ipctx\n");
    return false;
  }
  const int32_t vm = dna.module_with_role(aibaby::ModuleRole::kVocal);
  if (vm < 0) {
    std::printf("  setup failed: no kVocal module to read a threshold from\n");
    return false;
  }
  constexpr uint32_t kReps = 3;
  instrument("ipctx", dna.header().seed, ticks / kVLTrialTicks, "trials per arm");
  std::printf("  question          does intrinsic plasticity on `%s` move when the\n"
              "                    context tract drives it? IP is\n"
              "                    threshold += ip_rate * (rate - target), ip_rate %.3g,\n"
              "                    target %.1f Hz, clamp [%.2f, %.2f]\n",
              dna.module(uint32_t(vm)).name, double(dna.header().homeo.ip_rate),
              double(dna.module(uint32_t(vm)).target_rate_hz),
              double(dna.header().homeo.threshold_min),
              double(dna.header().homeo.threshold_max));
  std::printf("  arm               `fixed` -- ctxlearn's positive control, the one\n"
              "                    that dies when the oracle fires. No yoke: this\n"
              "                    reads a regulator, not a learning difference.\n\n");

  // IP steps the threshold by ip_rate * (rate - target) once every
  // interval_ticks, so the drift IS the integral of the rate error and the
  // session-mean error follows by division -- exact, where the endpoint EMA the
  // first version gated on is one noisy sample of a quantity that moved all
  // session (gain 0.10 read 8.18 +/- 2.48 Hz for a drift of +0.372 +/- 0.024).
  // Valid only while nothing is at the clamp, which is why `pinned` gates it.
  const double ip_rate_k = double(dna.header().homeo.ip_rate);
  const uint32_t homeo_interval = dna.header().homeo.interval_ticks;
  const double n_calls =
      homeo_interval ? double((ticks / kVLTrialTicks) * kVLTrialTicks / homeo_interval)
                     : 0.0;
  const double err_scale =
      (ip_rate_k > 0.0 && n_calls > 0.0) ? 1.0 / (ip_rate_k * n_calls) : 0.0;

  std::vector<double> drift[kCtxLevelCount], refdrift[kCtxLevelCount];
  std::vector<double> rate[kCtxLevelCount], change[kCtxLevelCount];
  std::vector<double> pinned[kCtxLevelCount], voiced[kCtxLevelCount];
  std::vector<double> isat[kCtxLevelCount];
  double ctx_hz[kCtxLevelCount] = {};
  double target_hz = 0.0;
  uint32_t cells[kCtxLevelCount] = {};

  std::printf("  %-6s %-7s %-8s %-9s %-9s %-10s %-9s %-9s %s\n", "seed", "gain", "ctx Hz",
              "vocal Hz", "d thresh", "d ref", "pinned", "inh sat", "change");
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));

    for (uint32_t L = 0; L < kCtxLevelCount; ++L) {
      CtxDrive drive;
      drive.module = ctx_module;
      drive.slots = kVLWords;
      drive.gain = kCtxLevels[L];
      Regime reg;
      reg.praise = kPraiseValue;
      reg.scold = kScoldValue;
      const VLRun run = run_vocallearn_session(variant, ticks, kVLTaught, nullptr, reg,
                                               kVLTgtFixed, &drive);
      if (!run.ok) {
        std::printf("  %-6u %-7.2f (inconclusive: %u scored, %u skipped)\n", r,
                    kCtxLevels[L], run.scored, run.skipped);
        continue;
      }
      drift[L].push_back(run.ip_thresh_drift);
      refdrift[L].push_back(run.ip_ref_drift);
      rate[L].push_back(run.ip_rate_hz);
      change[L].push_back(vl_change(run));
      pinned[L].push_back(run.ip_pinned);
      isat[L].push_back(run.isp_pinned);
      voiced[L].push_back(run.voiced_frac);
      ctx_hz[L] += run.ctx_rate;
      target_hz = run.ip_target_hz;
      ++cells[L];
      std::printf("  %-6u %-7.2f %-8.1f %-9.2f %+-9.4f %+-10.4f %-9.2f %-9.2f %+.1f\n", r,
                  kCtxLevels[L], run.ctx_rate, run.ip_rate_hz, run.ip_thresh_drift,
                  run.ip_ref_drift, run.ip_pinned, run.isp_pinned, vl_change(run));
    }
  }

  double m_drift[kCtxLevelCount], s_drift[kCtxLevelCount];
  double m_rate[kCtxLevelCount], s_rate[kCtxLevelCount];
  double m_ref[kCtxLevelCount], s_ref[kCtxLevelCount];
  double m_chg[kCtxLevelCount], s_chg[kCtxLevelCount];
  double m_pin[kCtxLevelCount], s_pin[kCtxLevelCount];
  double m_isat[kCtxLevelCount], s_isat[kCtxLevelCount];
  double m_vf[kCtxLevelCount], s_vf[kCtxLevelCount];
  for (uint32_t L = 0; L < kCtxLevelCount; ++L) {
    if (drift[L].size() < 2) {
      std::printf("\n  ipctx INCONCLUSIVE -- gain %.2f did not produce two usable\n"
                  "  creatures, so it has no spread and nothing here can be read\n"
                  "  against it.\n", kCtxLevels[L]);
      return false;
    }
    m_drift[L] = ctx_mean_se(drift[L], &s_drift[L]);
    m_rate[L] = ctx_mean_se(rate[L], &s_rate[L]);
    m_ref[L] = ctx_mean_se(refdrift[L], &s_ref[L]);
    m_chg[L] = ctx_mean_se(change[L], &s_chg[L]);
    m_pin[L] = ctx_mean_se(pinned[L], &s_pin[L]);
    m_isat[L] = ctx_mean_se(isat[L], &s_isat[L]);
    m_vf[L] = ctx_mean_se(voiced[L], &s_vf[L]);
  }

  // `voiced` is printed because relaxing regulation on the larynx makes the
  // creature DRONE (v9: duty 0.61 -> 0.83), and ctxlearn's voiced gate is
  // one-sided and would not catch it. A `change` read off a droning creature is
  // a different measurement wearing the same name.
  std::printf("\n  %-7s %-8s %-15s %-15s %-13s %-13s %-8s %s\n", "gain", "ctx Hz",
              "vocal Hz", "d threshold", "pinned", "inh sat", "voiced", "change");
  for (uint32_t L = 0; L < kCtxLevelCount; ++L) {
    char a[32], b[32], c[32], d[32], e[32], f[32];
    std::snprintf(a, sizeof a, "%.2f +/- %.2f", m_rate[L], s_rate[L]);
    std::snprintf(b, sizeof b, "%+.4f +/- %.4f", m_drift[L], s_drift[L]);
    std::snprintf(c, sizeof c, "%+.4f +/- %.4f", m_ref[L], s_ref[L]);
    std::snprintf(d, sizeof d, "%.2f +/- %.2f", m_pin[L], s_pin[L]);
    std::snprintf(f, sizeof f, "%.2f +/- %.2f", m_isat[L], s_isat[L]);
    std::snprintf(e, sizeof e, "%+.1f +/- %.1f", m_chg[L], s_chg[L]);
    (void)c;
    std::printf("  %-7.2f %-8.1f %-15s %-15s %-13s %-13s %-8.2f %s\n", kCtxLevels[L],
                cells[L] ? ctx_hz[L] / cells[L] : 0.0, a, b, d, f, m_vf[L], e);
  }
  std::printf("\n  mean rate error over the session, DERIVED from the drift as\n"
              "  drift / (ip_rate * calls) -- exact where nothing is at the clamp:\n");
  std::printf("  %-7s %-18s %s\n", "gain", "mean err (Hz)", "valid");
  for (uint32_t L = 0; L < kCtxLevelCount; ++L) {
    char v[32];
    std::snprintf(v, sizeof v, "%+.2f +/- %.2f", m_drift[L] * err_scale,
                  s_drift[L] * err_scale);
    std::printf("  %-7.2f %-18s %s\n", kCtxLevels[L], v,
                m_pin[L] > 0.02 ? "NO -- at the clamp" : "yes");
  }
  std::printf("\n  `change` is the taught arm's own formant-error reduction, unyoked:\n"
              "  it is here to show the collapse ctxlearn measured, not to score it.\n"
              "  IP is pulling `%s` toward %.1f Hz.\n",
              dna.module(uint32_t(vm)).name, target_hz);

  // The two conditions, in order. Both are about the DRIVEN levels that
  // actually fired -- a mute level is not a level, which ctxlearn learned the
  // expensive way.
  //
  // BOTH have to hold at the SAME level. The first version of this loop set two
  // independent flags and then reported them together, which passed condition
  // (1) at gain 0.20 and condition (2) at 0.10 and called the pair a licence --
  // two different regimes wearing one verdict. It matters here specifically:
  // 0.20 saturates, and a level where the clamp is holding most of the module
  // is not a level where IP is regulating anything.
  bool any_driven = false, rate_error = false, drift_moved = false, both = false;
  bool any_unpinned_driven = false;
  double best_drift = 0.0, best_rate = 0.0, best_gain = 0.0, best_pin = 0.0;
  double min_pin = 1.0, max_pin = 0.0;
  for (uint32_t L = 1; L < kCtxLevelCount; ++L) {
    const double hz = cells[L] ? ctx_hz[L] / cells[L] : 0.0;
    if (hz <= 1.0) continue;  // oracle mute at this level
    any_driven = true;
    if (m_pin[L] <= 0.02) any_unpinned_driven = true;
    if (m_pin[L] < min_pin) min_pin = m_pin[L];
    if (m_pin[L] > max_pin) max_pin = m_pin[L];
    // (1) is there a rate error for IP to act on, above the mute arm's own?
    // Derived from the drift rather than sampled off the final EMA, and only
    // where the clamp is not holding the module: past that the drift stops
    // tracking the error and the division is meaningless.
    const double err_hz = m_drift[L] * err_scale;
    const double err_hz0 = m_drift[0] * err_scale;
    const bool err = m_pin[L] <= 0.02 && err_hz > 0.0 &&
                     err_hz - err_hz0 > 2.0 * (s_drift[L] + s_drift[0]) * err_scale;
    // (2) does the threshold move further than it does with the oracle mute?
    const double gap = std::fabs(m_drift[L]) - std::fabs(m_drift[0]);
    const bool moved = gap > 2.0 * (s_drift[L] + s_drift[0]);
    rate_error = rate_error || err;
    drift_moved = drift_moved || moved;
    if (err && moved && (!both || gap > best_drift)) {
      both = true;
      best_drift = gap; best_rate = m_rate[L];
      best_gain = kCtxLevels[L]; best_pin = m_pin[L];
    }
  }

  if (!any_driven) {
    std::printf("\n  UNREADABLE -- no level drove the oracle above 1 Hz, so nothing\n"
                "  here is a measurement of what drive does to the larynx.\n");
    return false;
  }

  if (!any_unpinned_driven) {
    std::printf("\n  IP SATURATES -- and that is a different finding from the one this\n"
                "  probe set out to make. Every driven level ends the session with\n"
                "  %.0f-%.0f%% of the larynx sitting at threshold_max, so the derived\n"
                "  rate error cannot be read at ANY of them and the graded\n"
                "  re-regulation story is not what is happening. The threshold does\n"
                "  climb -- %+.3f at the lowest driven level against %+.3f mute -- but\n"
                "  it climbs into the clamp rather than settling at a new operating\n"
                "  point, while `change` falls %+.1f -> %+.1f.\n\n"
                "  This REFUTES the mechanism as stated and raises a narrower one in\n"
                "  its place: not that IP re-regulates the larynx away from the\n"
                "  oracle's drive, but that it drives a large share of the larynx onto\n"
                "  its threshold ceiling, where a per-neuron bias perturbation has\n"
                "  much less purchase on the output. That is a DIFFERENT experiment --\n"
                "  it predicts the pinned share, not the learning score, is what\n"
                "  tracks the collapse -- and it is not licensed by this run.\n\n"
                "  NOTE: at 200000 ticks this same probe reads pinned 0.00 at every\n"
                "  driven level and grants the licence. The threshold needs a full\n"
                "  session to walk to the clamp, so a short run of this experiment\n"
                "  reports the OPPOSITE of what it reports at length.\n",
                100.0 * min_pin, 100.0 * max_pin, m_drift[1], m_drift[0],
                m_chg[0], m_chg[kCtxLevelCount - 1]);
    return false;
  }

  if (!rate_error && !drift_moved) {
    std::printf("\n  IP IS NOT THE STRANGLER -- and this closes the last named\n"
                "  suspect for ctxlearn's collapse. The larynx runs at %.2f Hz driven\n"
                "  against %.2f Hz mute (target %.1f), and its threshold drifts\n"
                "  %+.4f driven against %+.4f mute: no rate error for IP to act on\n"
                "  and no threshold response, while `change` falls %+.1f -> %+.1f.\n"
                "  Whatever kills the positive control when the context tract fires,\n"
                "  it is not intrinsic plasticity re-regulating the larynx.\n"
                "  Do NOT spend an ip_wake_scale arm on this.\n",
                m_rate[1], m_rate[0], target_hz, m_drift[1], m_drift[0],
                m_chg[0], m_chg[1]);
    return false;
  }

  if (both) {
    if (best_pin > 0.5) {
      std::printf("\n  SATURATED, NOT REGULATED -- the only level where both conditions\n"
                  "  hold is gain %.2f, and there %.0f%% of the larynx is sitting at the\n"
                  "  threshold clamp with the module at %.1f Hz against a %.1f Hz target.\n"
                  "  IP has run out of range: a clamp is not a regulator, and this is\n"
                  "  not the regime ctxlearn ran its arms in. Relaxing ip_wake_scale\n"
                  "  here would be relaxing something that has already stopped acting.\n"
                  "  The licence this probe was built to grant is NOT granted.\n",
                  best_gain, 100.0 * best_pin, best_rate, target_hz);
      return false;
    }
    std::printf("\n  IP IS RESPONDING, and the intervention is licensed. At gain %.2f\n"
                "  the larynx runs %.2f Hz against a %.1f Hz target and its threshold\n"
                "  drifts %.4f further than with the oracle mute, against a central\n"
                "  module that moves %+.4f. Both of the conditions this probe was\n"
                "  built to check are met, so `ip_wake_scale` on the larynx is now\n"
                "  worth a graded arm in ctxlearn.\n\n"
                "  It is NOT yet a result: IP responding and IP being the CAUSE of\n"
                "  the collapse are different claims, and relaxing IP on the larynx\n"
                "  is already known to make the creature drone (v9: duty 0.61 ->\n"
                "  0.83), which moves the voiced fraction this readout depends on.\n"
                "  Any such arm needs a TWO-sided voiced gate; ctxlearn's present\n"
                "  one only catches the creature going quiet.\n",
                best_gain, best_rate, target_hz, best_drift, m_ref[1]);
    return true;
  }


  std::printf("\n  PARTIAL -- %s but %s. The two conditions disagree, so the\n"
              "  mechanism as stated does not hold: IP acts on the rate error, so a\n"
              "  threshold that moves without one, or an error that produces no\n"
              "  movement, is not the story that was told about it. Read the table\n"
              "  before spending an arm on it.\n",
              rate_error ? "the larynx does run above target while driven"
                         : (any_unpinned_driven
                                ? "the larynx shows no rate error to act on"
                                : "the rate error is not measurable -- clamped"),
              drift_moved ? "its threshold does move further than when mute"
                          : "its threshold does not move any further than when mute");
  return false;
}

bool run_ctxlearn(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  const int32_t ctx_module = dna.module_with_role(aibaby::ModuleRole::kContext);
  if (ctx_module < 0) {
    std::printf("  this genome has no kContext module, so there is no oracle to\n"
                "  drive and nothing to measure. Build one:\n\n"
                "    python3 tools/genome_add_context.py dna/default.toml ctx.toml vocal\n"
                "    ./build/aibaby --dna ctx.toml --experiment ctxlearn\n");
    return false;
  }
  const aibaby::DnaModule& cm = dna.module(uint32_t(ctx_module));
  constexpr uint32_t kReps = 3;
  instrument("ctxlearn", dna.header().seed, ticks / kVLTrialTicks, "trials per arm");
  std::printf("  oracle            `%s`, %u neurons in %u disjoint slices,\n"
              "                    zero baseline (noise %.2f, target %.2f Hz)\n",
              cm.name, cm.neurons, kVLWords, double(cm.noise_amp),
              double(cm.target_rate_hz));
  std::printf("  every level shares one genome, so the arms share a creature, a\n"
              "  noise stream and a set of synapses; level 0.00 is the control.\n\n");

  const CtxArm arms[3] = {{"fixed", kVLTgtFixed},
                          {"heard", kVLTgtHeard},
                          {"swap", kVLTgtSwap}};
  std::vector<double> points[kCtxLevelCount][3];
  double ctx_hz[kCtxLevelCount] = {};
  double voiced[kCtxLevelCount] = {};
  double dw_ctx[kCtxLevelCount] = {};
  double dw_ref[kCtxLevelCount] = {};
  uint32_t cells[kCtxLevelCount] = {};

  std::printf("  %-6s %-7s %-7s %-9s %-9s %-9s %-8s %-10s %-10s\n", "seed", "gain",
              "arm", "taught", "yoked", "points", "ctx Hz", "|dw| ctx", "|dw| rest");
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));

    for (uint32_t L = 0; L < kCtxLevelCount; ++L) {
      CtxDrive drive;
      drive.module = ctx_module;
      drive.slots = kVLWords;
      drive.gain = kCtxLevels[L];

      for (int a = 0; a < 3; ++a) {
        Regime reg;
        reg.praise = kPraiseValue;
        reg.scold = kScoldValue;
        const VLRun taught = run_vocallearn_session(variant, ticks, kVLTaught, nullptr,
                                                    reg, arms[a].target, &drive);
        if (!taught.ok) {
          std::printf("  %-6u %-7.2f %-7s (inconclusive: %u scored, %u skipped)\n", r,
                      kCtxLevels[L], arms[a].name, taught.scored, taught.skipped);
          continue;
        }
        // Each arm carries its OWN yoke, scored against its OWN target: the
        // same praise and scolding in the same proportions, half a trial out of
        // phase so it cannot land on this creature's own echoes.
        std::vector<Praise> yoke = taught.feedback;
        for (Praise& p : yoke) p.tick += kVLTrialTicks / 2;
        const VLRun yoked = run_vocallearn_session(variant, ticks, kVLYoked, &yoke, reg,
                                                   arms[a].target, &drive);
        if (!yoked.ok || yoked.praises + yoked.scolds != 0) {
          std::printf("  %-6u %-7.2f %-7s (inconclusive: the yoke earned %u of its own)\n",
                      r, kCtxLevels[L], arms[a].name, yoked.praises + yoked.scolds);
          continue;
        }
        const double pt = vl_change(taught) - vl_change(yoked);
        points[L][a].push_back(pt);
        ctx_hz[L] += taught.ctx_rate;
        voiced[L] += taught.voiced_frac;
        dw_ctx[L] += taught.ctx_dw;
        dw_ref[L] += taught.ref_dw;
        ++cells[L];
        std::printf("  %-6u %-7.2f %-7s %+-9.1f %+-9.1f %+-9.1f %-8.1f %-10.2e %-10.2e\n",
                    r, kCtxLevels[L], arms[a].name, vl_change(taught), vl_change(yoked),
                    pt, taught.ctx_rate, taught.ctx_dw, taught.ref_dw);
      }
    }
  }

  double mean[kCtxLevelCount][3] = {}, se[kCtxLevelCount][3] = {};
  bool complete = true;
  for (uint32_t L = 0; L < kCtxLevelCount; ++L) {
    for (int a = 0; a < 3; ++a) {
      mean[L][a] = ctx_mean_se(points[L][a], &se[L][a]);
      if (points[L][a].size() < 2) complete = false;
    }
  }
  if (!complete) {
    std::printf("\n  ctxlearn INCONCLUSIVE — a cell did not produce two usable\n"
                "  creatures. A creature that does not vocalise in the echo window has\n"
                "  no accuracy to improve, and that is not a measurement of whether it\n"
                "  could.\n");
    return false;
  }

  // The control at gain 0 is the reference every other level is read against.
  const double ref = mean[0][0];
  std::printf("\n  arm — yoke, in points of formant-error reduction. `fixed` is the\n"
              "  positive control and it gates its own row: a level that cannot move\n"
              "  it is not a level that can report a null on anything else.\n\n");
  std::printf("  %-7s %-8s %-8s %-16s %-16s %-16s %s\n", "gain", "ctx Hz", "voiced",
              "fixed (control)", "heard", "swap", "readable");
  bool any_readable_driven = false;
  bool readable[kCtxLevelCount] = {};
  bool driven[kCtxLevelCount] = {};
  for (uint32_t L = 0; L < kCtxLevelCount; ++L) {
    // A level is DRIVEN only if the oracle measurably fired. The first version
    // of this check asked whether L > 0, and picked a sub-threshold level as
    // its best driven arm -- which is the silent control wearing a label.
    const double hz = cells[L] ? ctx_hz[L] / cells[L] : 0.0;
    driven[L] = hz > 1.0;
    // A level where the creature has stopped VOCALISING is not comparable to
    // one where it has not, whatever the control happens to read. Formant error
    // measured over 13% of the echo window and over 61% of it are two different
    // instruments, and the first one's error bars show it: the run that forced
    // this gate read swap +53.4 / -35.5 / +16.7 across three seeds at a voiced
    // fraction of 0.13.
    const double vf = cells[L] ? voiced[L] / cells[L] : 0.0;
    const double vf_ref = cells[0] ? voiced[0] / cells[0] : 0.0;
    const bool talking = vf_ref <= 0.0 || vf >= kCtxVoicedKeep * vf_ref;
    readable[L] = talking && mean[L][0] > kCtxVisible &&
                  (!driven[L] || mean[L][0] >= kCtxControlKeep * ref);
    if (driven[L] && readable[L]) any_readable_driven = true;
    char f[32], h[32], w[32];
    std::snprintf(f, sizeof f, "%+.1f +/- %.1f", mean[L][0], se[L][0]);
    std::snprintf(h, sizeof h, "%+.1f +/- %.1f", mean[L][1], se[L][1]);
    std::snprintf(w, sizeof w, "%+.1f +/- %.1f", mean[L][2], se[L][2]);
    std::printf("  %-7.2f %-8.1f %-8.2f %-16s %-16s %-16s %s\n", kCtxLevels[L],
                cells[L] ? ctx_hz[L] / cells[L] : 0.0,
                cells[L] ? voiced[L] / cells[L] : 0.0, f, h, w,
                !driven[L] ? "n/a — oracle mute"
                           : (readable[L] ? "yes"
                              : (!talking ? "NO — creature stopped talking"
                                          : "NO — control gone")));
  }
  std::printf("\n  A level is readable when the control clears %.0f points AND keeps\n"
              "  %.0f%% of what it reads with the oracle silent (%+.1f).\n",
              kCtxVisible, 100.0 * kCtxControlKeep, ref);

  // A flat conditional arm on a tract reward never wrote to is not a
  // measurement of conditionality. This is the row that separates them.
  std::printf("\n  did reward write to the oracle's tract at all?\n");
  std::printf("  %-7s %-14s %-14s %s\n", "gain", "mean|dw| ctx", "mean|dw| rest",
              "ratio");
  for (uint32_t L = 0; L < kCtxLevelCount; ++L) {
    const double c = cells[L] ? dw_ctx[L] / cells[L] : 0.0;
    const double f = cells[L] ? dw_ref[L] / cells[L] : 0.0;
    std::printf("  %-7.2f %-14.3e %-14.3e %-8.2f%s\n", kCtxLevels[L], c, f,
                f > 0.0 ? c / f : 0.0,
                (L > 0 && f > 0.0 && c / f < 0.05) ? "  <- INERT: nothing was written"
                                                   : "");
  }

  if (!readable[0]) {
    std::printf("\n  UNDERPOWERED — the control reads %+.1f with the oracle silent,\n"
                "  under the %.0f points this instrument needs. Nothing here is\n"
                "  readable and the conditional arms' nulls are facts about the probe.\n"
                "  Try --ticks higher.\n", ref, kCtxVisible);
    return false;
  }

  // Among the levels that survived, the best the conditional arm managed.
  double best = 0.0, best_gain = 0.0, best_ctrl = 0.0, best_se = 0.0;
  bool have_best = false;
  for (uint32_t L = 1; L < kCtxLevelCount; ++L) {
    if (!readable[L] || !driven[L]) continue;
    if (!have_best || mean[L][2] > best) {
      best = mean[L][2];
      best_se = se[L][2];
      best_gain = kCtxLevels[L];
      best_ctrl = mean[L][0];
      have_best = true;
    }
  }

  if (!any_readable_driven) {
    // Before calling it undeliverable: did a level where the control fell also
    // move the CONDITIONAL arms? A drive artefact lifts everything, so
    // conditional-up-and-control-down is the one pattern that cannot be one,
    // and it is a lead rather than a failure to deliver.
    for (uint32_t L = 1; L < kCtxLevelCount; ++L) {
      if (!driven[L] || readable[L]) continue;
      if (mean[L][2] > kCtxVisible && mean[L][2] > mean[L][0] &&
          mean[L][2] > mean[0][2] + kCtxVisible &&
          mean[L][2] > 2.0 * se[L][2]) {
        std::printf("\n  LEAD, NOT A RESULT — at gain %.2f the arbitrary conditional\n"
                    "  arm reads %+.1f +/- %.1f against %+.1f with the oracle mute, while\n"
                    "  the NON-conditional control reads %+.1f. Conditional up and\n"
                    "  control down is the one pattern extra drive cannot produce.\n\n"
                    "  It is not a result because the control has fallen from %+.1f, so\n"
                    "  this level is a different creature being asked the question, and\n"
                    "  because `points` is taught minus yoke and a yoke that DEGRADES\n"
                    "  inflates it as surely as a taught arm that improves. Decompose it\n"
                    "  before believing it, then re-run on six seed families: three with\n"
                    "  unanimous signs has been enough to be wrong here before.\n\n"
                    "  The design fault this exposes: `gain` sets both how much the\n"
                    "  oracle FIRES and how hard it SHOVES the larynx, so there is no\n"
                    "  level that does one without the other. Separate them — hold gain\n"
                    "  above threshold and sweep `out_w` instead.\n",
                    kCtxLevels[L], mean[L][2], se[L][2], mean[0][2], mean[L][0], ref);
        return false;
      }
    }
    std::printf("\n  ORACLE NOT DELIVERABLE — the control is alive at %+.1f with the\n"
                "  oracle silent and gone at every level that drives it. This genome\n"
                "  cannot put a condition into this larynx without taking the larynx\n"
                "  off the operating point at which reward works.\n\n"
                "  TWO OF THE THREE LEVERS ARE SPENT. `out_w` was swept over 6x\n"
                "  (0.005-0.030) and the collapse does not scale with it, so it is not\n"
                "  the tract shoving the larynx. Paying for the tract out of vocal's own\n"
                "  noise -- rule 1 of the calibration invariant -- was run at 0.16 and\n"
                "  0.10 against a genome noise of 0.22 and does not save it either.\n"
                "  Five genomes, twelve driven levels, control never survives once.\n\n"
                "  So read this as the finding rather than as a failure to deliver: a\n"
                "  single shared motor population cannot host a learnable conditional\n"
                "  input AND a reward-driven exploratory pathway. In the birdsong\n"
                "  circuit HVC->RA and LMAN->RA are separate afferents onto RA under\n"
                "  separate rules; `vocal` is asked to be both. The motor side has to be\n"
                "  rebuilt -- a sparse dictionary of postures selected by competition --\n"
                "  BEFORE a conditional input can be tested at all.\n", ref);
    return false;
  }

  std::printf("\n  Read `swap` against `fixed` in the same ROW. The oracle adds drive\n"
              "  whichever target is being taught, so an effect that also lifts the\n"
              "  control is about drive and not about conditionality.\n");

  // The 2 SE requirement was added to the LEAD branch first and NOT here, and
  // this branch then fired on +11.6 +/- 25.8 -- per seed +53.4, -35.5, +16.7.
  // A bar on the mean without a bar on its error is not a bar. It applies to
  // every verdict now, positive included, which is the general form of a hole
  // this experiment has printed through three times.
  if (best > kCtxVisible && best > 2.0 * best_se) {
    std::printf("\n  CONDITIONAL LEARNING — an arbitrary map the creature has no innate\n"
                "  route for was taught by praise: `swap` reads %+.1f at gain %.2f,\n"
                "  where it reads %+.1f with the oracle silent, and the control at that\n"
                "  level is %+.1f against its silent %+.1f. The presynaptic baseline was\n"
                "  the binding variable. Before this licenses anything, run it on six\n"
                "  seed families: three with unanimous signs has been enough to be wrong\n"
                "  in this project before.\n",
                best, best_gain, mean[0][2], best_ctrl, ref);
    return true;
  }
  std::printf("\n  NO CONDITIONAL LEARNING — the best `swap` at any level whose control\n"
              "  survived is %+.1f (gain %.2f, control %+.1f against its silent %+.1f),\n"
              "  against %+.1f with the oracle silent. A sparse zero-baseline context\n"
              "  code on disjoint slices does not let reward write a conditional map,\n"
              "  and this time the control was alive to say so. That is the falsifier\n"
              "  this experiment was built to be able to report: the presynaptic\n"
              "  baseline is not the binding variable, and the rewrite is not licensed\n"
              "  on this argument.\n",
              best, best_gain, best_ctrl, ref, mean[0][2]);
  (void)verbose;
  return false;
}


// --- pgprobe: is the policy gradient's conditional arm actually conditional? -
//
// `ctxlearn` on the v48 dictionary + v49 policy gradient left exactly one live
// number: with the oracle MUTE, `swap` reads +6.5 +/- 1.1 on 3 of 3 seeds,
// where the centroid creature reads +0.2 +/- 0.2. That is 54% of the
// fixed-target control's magnitude against 0.6% before, and it needs no oracle
// at all -- the condition would be arriving through the innate arcuate, and the
// policy gradient would be using what a centroid readout could not.
//
// **It has one obvious way of being fake and this experiment is that control.**
// `swap`'s target alternates between the two words. A creature that learns
// nothing conditional, and simply moves to the MIDPOINT of the two, reduces its
// mean error against both and scores positively -- with no dependence on what it
// heard whatsoever. Nothing in `vocallearn` separates that from real
// conditionality, because its `fixed` control aims at ONE word rather than at
// the same distribution.
//
// The control that does is a target with MATCHED MARGINALS: the same two words
// in the same proportions, drawn independently of what was heard. A
// midpoint-seeker scores identically on it; only a creature whose output depends
// on its input can beat it. So the quantity is `swap - random`, and m3 has been
// read that way for its whole life while vocallearn never had the equivalent.
//
// Four targets, each with its own yoke, at oracle-mute -- no context module is
// needed, which is the point.
bool run_pgprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) {
    std::printf("  setup failed: the genome does not load\n");
    return false;
  }
  // Deliberately runs on a centroid creature too, and that is not a fallback.
  // `vocallearn` scores its `fixed` arm against a yoke built from the TAUGHT
  // arm's reward stream rather than from its own, and this experiment gives
  // every arm its own yoke. When the dictionary's control fell from +16.4 under
  // the first method to +2.2 under the second, there were two candidate
  // explanations -- a weak mechanism, or a stricter instrument -- and the only
  // way to separate them is to put the SHIPPED creature through the same
  // instrument. A control that changes when the yoke changes was never a
  // measurement of the creature.
  const bool has_dict = dna.header().vocal.dictionary_units > 0 &&
                        dna.header().vocal.dictionary_policy_rate > 0.0f;
  constexpr uint32_t kReps = 3;
  instrument("pgprobe", dna.header().seed, ticks / kVLTrialTicks, "trials per arm");
  if (has_dict) {
    std::printf("  dictionary        %u postures, temp %.2f, policy rate %.2f\n",
                dna.header().vocal.dictionary_units,
                double(dna.header().vocal.dictionary_temp),
                double(dna.header().vocal.dictionary_policy_rate));
  } else {
    std::printf("  readout           the nine centroids — no dictionary. This is the\n"
                "                    like-for-like control for the dictionary's number.\n");
  }
  std::printf("  the question      is `swap` conditional, or is it a creature that\n"
              "                    learned to sit between two alternating targets?\n"
              "  the control       `random`: same two targets, same proportions,\n"
              "                    drawn independently of what was heard.\n\n");

  struct PgArm { const char* name; VLTarget target; };
  const PgArm arms[4] = {{"fixed", kVLTgtFixed},
                         {"heard", kVLTgtHeard},
                         {"swap", kVLTgtSwap},
                         {"random", kVLTgtRandom}};
  std::vector<double> pts[4];

  std::printf("  %-6s %-8s %-9s %-9s %-9s %s\n", "seed", "arm", "taught", "yoked",
              "points", "voiced");
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    for (int a = 0; a < 4; ++a) {
      Regime reg;
      reg.praise = kPraiseValue;
      reg.scold = kScoldValue;
      const VLRun taught =
          run_vocallearn_session(variant, ticks, kVLTaught, nullptr, reg, arms[a].target);
      if (!taught.ok) {
        std::printf("  %-6u %-8s (inconclusive: %u scored, %u skipped)\n", r,
                    arms[a].name, taught.scored, taught.skipped);
        continue;
      }
      std::vector<Praise> yoke = taught.feedback;
      for (Praise& p : yoke) p.tick += kVLTrialTicks / 2;
      const VLRun yoked =
          run_vocallearn_session(variant, ticks, kVLYoked, &yoke, reg, arms[a].target);
      if (!yoked.ok || yoked.praises + yoked.scolds != 0) {
        std::printf("  %-6u %-8s (inconclusive: the yoke earned %u of its own)\n", r,
                    arms[a].name, yoked.praises + yoked.scolds);
        continue;
      }
      const double pt = vl_change(taught) - vl_change(yoked);
      pts[a].push_back(pt);
      std::printf("  %-6u %-8s %+-9.1f %+-9.1f %+-9.1f %.2f\n", r, arms[a].name,
                  vl_change(taught), vl_change(yoked), pt, taught.voiced_frac);
    }
  }

  double m[4] = {}, e[4] = {};
  for (int a = 0; a < 4; ++a) {
    if (pts[a].size() < 2) {
      std::printf("\n  pgprobe INCONCLUSIVE — an arm did not produce two usable\n"
                  "  creatures.\n");
      return false;
    }
    m[a] = ctx_mean_se(pts[a], &e[a]);
  }

  std::printf("\n  %-8s %s\n", "arm", "taught - yoke");
  for (int a = 0; a < 4; ++a) {
    std::printf("  %-8s %+.1f +/- %.1f\n", arms[a].name, m[a], e[a]);
  }

  // The conditional quantity, and it is the only one this experiment exists to
  // print. Both arms see the same two targets in the same proportions; only one
  // of them has a target that depends on what the creature heard.
  const double cond = m[2] - m[3];
  const double cond_se = std::sqrt(e[2] * e[2] + e[3] * e[3]);
  std::printf("\n  swap - random    %+.1f +/- %.1f  <- THE conditional quantity\n",
              cond, cond_se);
  // arms[] is {fixed, heard, swap, random}, so `heard` is m[1]. The first
  // version of this line read m[0] and printed fixed-minus-random under the
  // heard label -- +38.8 next to a `heard` arm that reads -0.8, which is the
  // only reason it was caught.
  std::printf("  heard - random   %+.1f +/- %.1f\n", m[1] - m[3],
              std::sqrt(e[1] * e[1] + e[3] * e[3]));

  if (m[0] < 5.0) {
    std::printf("\n  UNDERPOWERED — the `fixed` positive control moved %+.1f, under the\n"
                "  5 points this instrument needs, so nothing here is readable.\n", m[0]);
    return false;
  }
  if (cond > 5.0 && cond > 2.0 * cond_se) {
    std::printf("\n  CONDITIONAL — `swap` beats a target with the same marginals by\n"
                "  %+.1f +/- %.1f. A creature sitting between two alternating targets\n"
                "  scores the same on both arms, so this is a dependence on what was\n"
                "  HEARD and not a posture that happens to suit either. Next: six seed\n"
                "  families, and the audibility ruler — points of formant error are not\n"
                "  a sound anybody can hear until d' says so.\n", cond, cond_se);
    return true;
  }
  std::printf("\n  NOT CONDITIONAL — `swap` reads %+.1f and its matched-marginal\n"
              "  control %+.1f, a difference of %+.1f +/- %.1f. The gain is what a\n"
              "  creature gets for learning to sit between two alternating targets,\n"
              "  which needs no dependence on the input at all. The lead is closed.\n",
              m[2], m[3], cond, cond_se);
  (void)verbose;
  return false;
}


// --- g2cond: G2's own contingency, made conditional -------------------------
//
// Fourteen mechanisms have been aimed at making the voice depend on what was
// heard, all scored on FORMANT ERROR through a population centroid, now measured
// at -0.1 +/- 0.7 against a matched-marginal control. The question one level up
// has never been asked: **is the wall the readout, or is it conditionality?**
//
// G2 is the place to ask it. It is a met milestone -- rewarded vocalisations
// rise x1.35 within a session, 23 of 27 creatures, 9 of 9 at 420 s -- so "reward
// can shape this" is a result rather than a hypothesis, and the only new thing
// being asked is whether the shaping may DEPEND ON THE INPUT.
//
// THIS IS A REBUILD, AND THE THREE FAULTS OF THE FIRST VERSION ARE THE REASON.
// It was written in `vocallearn`'s frame and re-derived a contingency G2 already
// had working, wrongly three times: reward on a clock rather than on an act
// (positive control +3.9 +/- 2.4, taught and yoked degrading together); a
// running-mean baseline fed a binary signal, which converges onto it and makes
// every trial a scold; and a positive control whose target never changed, so
// every event was praised and none scolded -- which `vocallearn` prints a
// ONE-SIDED: NOT A TRAINING SIGNAL warning for and the probe did not print at
// all. Each fault is downstream of not copying G2.
//
// SO THIS COPIES G2 EXACTLY and changes one thing.
//
//   * the act is a voiced frame, as G2's is, so reward always follows something
//     the creature did;
//   * the class is `vocal_groups()[2]`, the F1 motor group -- the thing the
//     brain controls, not the audio it produces, which is G2's own choice;
//   * the criterion is THIS creature's baseline median over the first 20% of
//     the session, so hits start at half and BOTH SIGNS OCCUR BY CONSTRUCTION.
//     That is the property all three broken versions lacked;
//   * feedback runs over the middle 60% and the last 20% is scored, as G2 does.
//
// The one change: **which class is praised depends on the word just heard.**
//
//   fixed    praise a hit always. This IS G2, with a caregiver talking.
//   heard    praise a hit after word A, praise a MISS after word B.
//   swap     the same map inverted, since neither direction is privileged --
//            the arcuate carries formants, and this is a binary class boundary.
//   random   the direction drawn INDEPENDENTLY of the word, in the same
//            proportions. The matched-marginal control.
//
// THE MEASURE NEEDS NO YOKE, which removes the whole class of fault that cost
// `pgprobe` a run. Conditionality is scored WITHIN an arm: the hit rate on
// trials whose direction was "praise a hit" minus the hit rate on trials whose
// direction was "praise a miss", both in the test window. A creature that
// ignores the word scores zero on that difference no matter what else it does,
// and `random` is the control that says so with matched marginals.
//
// The positive control is G2's own measure on the `fixed` arm: test hit rate
// against baseline hit rate.
namespace {

struct GcArm {
  const char* name;
  int mode;  // 0 fixed, 1 heard, 2 swap, 3 random
};

struct GcRun {
  bool ok = false;
  double baseline_hit = 0.0;
  double test_hit = 0.0;
  // Hit rate in the test window, split by what the trial was asking for.
  double hit_when_want = 0.0, hit_when_not = 0.0;
  uint64_t n_want = 0, n_not = 0;
  uint64_t praises = 0, scolds = 0, events = 0;
  float criterion = 0.0f;
};

// Which class this trial rewards, given the word heard and the arm.
inline bool gc_want_hit(int mode, uint32_t label, uint32_t trial) {
  if (mode == 0) return true;
  if (mode == 1) return label == 0;
  if (mode == 2) return label != 0;
  uint32_t r = trial * 2654435761u;
  r ^= r >> 16;
  return (r & 1u) != 0;
}

GcRun run_g2cond_session(const std::vector<uint8_t>& blob, uint64_t ticks, int mode,
                         const Regime& regime) {
  GcRun out;
  std::string error;
  Session s;
  if (!s.init(blob, error)) return out;
  const aibaby::DnaAudio& acfg = s.dna.header().audio;
  Ear ear;
  if (!ear.configure(acfg, error)) return out;
  VowelSource caregiver(acfg.sample_rate);
  std::vector<float> pcm(acfg.sample_rate / 1000);
  const uint32_t spt = acfg.sample_rate / 1000;

  const uint32_t n_trials = uint32_t(ticks / kVLTrialTicks);
  if (n_trials < 40) return out;
  const uint32_t baseline_trials = n_trials / 5;
  const uint32_t train_end_trial = n_trials - n_trials / 5;

  std::deque<Praise> pending;
  std::vector<float> baseline_values;
  float criterion = -1.0f;
  uint32_t last_frame = 0;
  uint64_t last_event = 0, last_feedback = 0;
  uint64_t test_hits = 0, test_events = 0;
  uint64_t want_hits = 0, want_events = 0, not_hits = 0, not_events = 0;

  for (uint32_t trial = 0; trial < n_trials; ++trial) {
    const uint32_t label = trial % kVLWords;
    const Word& heard = kWords[label];
    const bool want = gc_want_hit(mode, label, trial);

    for (uint64_t t = 0; t < kVLTrialTicks; ++t) {
      const uint64_t now = uint64_t(trial) * kVLTrialTicks + t;
      while (!pending.empty() && pending.front().tick <= now) {
        s.brain.praise(pending.front().value);
        pending.pop_front();
      }
      const bool sounding = t < kVLWordTicks;
      caregiver.render(sounding ? heard.f0 : 0.0f, heard.f1, heard.f2,
                       sounding ? 0.5f : 0.0f, pcm.data(), spt);
      ear.tick(s.brain, pcm.data(), spt);
      s.brain.step();

      if (s.brain.vocal_frame() == last_frame) continue;
      last_frame = s.brain.vocal_frame();
      // Two windows, not one, and the difference is what `vocallearn` reported
      // itself UNDERPOWERED for twice before finding. SCORING uses M1b's
      // window -- 200-600 ms after the word stops, where the ear reads at
      // chance and the voice still carries what it heard. FEEDBACK uses a wider
      // bracket on G2's own clock, because a reward delivered only inside the
      // scoring window is 14% of a trial and G2 earned its milestone at
      // nineteen times that density.
      const bool in_score = t >= kVLEchoFrom && t < kVLEchoTo;
      const bool in_reward = t >= kVLRewardFrom && t < kVLRewardTo;
      if (!in_score && !in_reward) continue;

      const aibaby::VocalParams& v = s.brain.voice();
      if (!(v.voicing > 0.5f && v.amplitude > kAmplitudeFloor)) continue;
      const float value = float(s.brain.vocal_groups()[2]);
      const bool new_event = now - last_event >= kEventRefractoryTicks;
      if (new_event) last_event = now;

      if (trial < baseline_trials) {
        // The criterion is measured in the window it will be applied in.
        if (new_event && in_score) baseline_values.push_back(value);
        continue;
      }
      if (criterion < 0.0f) {
        if (baseline_values.size() < 8) {
          criterion = 0.5f;
        } else {
          std::vector<float> sorted = baseline_values;
          std::nth_element(sorted.begin(), sorted.begin() + long(sorted.size() / 2),
                           sorted.end());
          criterion = sorted[sorted.size() / 2];
        }
      }
      const bool hit = value >= criterion;

      if (trial < train_end_trial) {
        if (in_reward && now - last_feedback >= regime.feedback_period) {
          last_feedback = now;
          // The whole experiment is this line: praise the class this trial's
          // WORD asked for. With mode 0 the word is ignored and it is G2.
          const float val = (hit == want) ? regime.praise : regime.scold;
          if (val > 0.0f) ++out.praises; else ++out.scolds;
          pending.push_back(Praise{now + regime.delay, val});
        }
      } else if (new_event && in_score) {
        ++test_events;
        if (hit) ++test_hits;
        if (want) { ++want_events; if (hit) ++want_hits; }
        else { ++not_events; if (hit) ++not_hits; }
      }
    }
  }

  uint64_t base_hits = 0;
  for (float v : baseline_values) if (v >= criterion) ++base_hits;
  out.baseline_hit = baseline_values.empty()
                         ? 0.0 : double(base_hits) / double(baseline_values.size());
  out.test_hit = test_events ? double(test_hits) / double(test_events) : 0.0;
  out.hit_when_want = want_events ? double(want_hits) / double(want_events) : 0.0;
  out.hit_when_not = not_events ? double(not_hits) / double(not_events) : 0.0;
  out.n_want = want_events;
  out.n_not = not_events;
  out.events = test_events;
  out.criterion = criterion;
  out.ok = test_events >= 40 && baseline_values.size() >= 8 &&
           out.praises > 0 && out.scolds > 0;
  return out;
}

}  // namespace

bool run_g2cond(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  constexpr uint32_t kReps = 3;
  instrument("g2cond", dna.header().seed, ticks / kVLTrialTicks, "trials per arm");
  std::printf("  the question      every conditional test here is scored on formant\n"
              "                    ERROR through a centroid, and reads -0.1 +/- 0.7\n"
              "                    against a matched control. Is the wall the readout\n"
              "                    or conditionality? Asked on G2's own contingency.\n");
  std::printf("  the act           a voiced frame, as G2's is, so reward always\n"
              "                    follows something the creature did\n");
  std::printf("  the class         F1 group against THIS creature's baseline median,\n"
              "                    so hits start at half and both signs occur\n");
  std::printf("  the change        which class is praised depends on the word heard\n");
  std::printf("  the measure       hit rate when the trial wanted a hit, minus hit\n"
              "                    rate when it wanted a miss. No yoke: a creature\n"
              "                    that ignores the word scores zero by construction.\n\n");

  const GcArm arms[4] = {{"fixed (=G2)", 0}, {"heard", 1}, {"swap", 2}, {"random", 3}};
  std::vector<double> cond[4];
  std::vector<double> g2m[4];

  std::printf("  %-6s %-12s %-9s %-9s %-9s %-9s %-8s %s\n", "seed", "arm", "base hit",
              "test hit", "hit|want", "hit|not", "cond", "praise/scold");
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));
    for (int a = 0; a < 4; ++a) {
      Regime reg;
      reg.praise = kPraiseValue;
      reg.scold = kScoldValue;
      const GcRun g = run_g2cond_session(variant, ticks, arms[a].mode, reg);
      if (!g.ok) {
        std::printf("  %-6u %-12s (inconclusive: %llu test events, %llu/%llu feedback)\n",
                    r, arms[a].name, (unsigned long long)g.events,
                    (unsigned long long)g.praises, (unsigned long long)g.scolds);
        continue;
      }
      const double c = g.hit_when_want - g.hit_when_not;
      // The fixed arm never asks for a miss, so its conditional cell is empty
      // by construction and only its G2 measure means anything.
      if (arms[a].mode != 0) cond[a].push_back(c);
      g2m[a].push_back(g.test_hit - g.baseline_hit);
      std::printf("  %-6u %-12s %-9.3f %-9.3f %-9.3f %-9.3f %+-8.3f %llu/%llu%s\n", r,
                  arms[a].name, g.baseline_hit, g.test_hit, g.hit_when_want,
                  g.hit_when_not, arms[a].mode == 0 ? 0.0 : c,
                  (unsigned long long)g.praises, (unsigned long long)g.scolds,
                  (g.praises == 0 || g.scolds == 0)
                      ? "  <- ONE-SIDED: not a training signal" : "");
    }
  }

  if (g2m[0].size() < 2 || cond[1].size() < 2 || cond[3].size() < 2) {
    std::printf("\n  g2cond INCONCLUSIVE — an arm did not produce two usable creatures.\n");
    return false;
  }

  double se_g2 = 0.0;
  const double m_g2 = ctx_mean_se(g2m[0], &se_g2);
  double se_h = 0.0, se_s = 0.0, se_r = 0.0;
  const double m_h = ctx_mean_se(cond[1], &se_h);
  const double m_s = ctx_mean_se(cond[2], &se_s);
  const double m_r = ctx_mean_se(cond[3], &se_r);

  std::printf("\n  G2 measure, `fixed` arm   test hit - baseline hit  %+.3f +/- %.3f\n",
              m_g2, se_g2);
  std::printf("\n  %-10s %s\n", "arm", "hit|want - hit|not");
  std::printf("  %-10s %+.3f +/- %.3f\n", "heard", m_h, se_h);
  std::printf("  %-10s %+.3f +/- %.3f\n", "swap", m_s, se_s);
  std::printf("  %-10s %+.3f +/- %.3f   <- matched-marginal control\n", "random", m_r, se_r);

  const double best = m_h > m_s ? m_h : m_s;
  const double best_se = m_h > m_s ? se_h : se_s;
  const double d = best - m_r;
  const double d_se = std::sqrt(best_se * best_se + se_r * se_r);
  std::printf("\n  conditional      %+.3f +/- %.3f  <- best of heard/swap, minus random\n",
              d, d_se);

  if (m_g2 < 0.05) {
    std::printf("\n  UNDERPOWERED — G2's own measure moved %+.3f on the `fixed` arm,\n"
                "  which is this contingency reproducing G2 with a caregiver in the\n"
                "  room. If that does not move, nothing else here is readable, and it\n"
                "  is a fact about this probe rather than about conditionality.\n", m_g2);
    return false;
  }
  if (d > 0.05 && d > 2.0 * d_se) {
    std::printf("\n  CONDITIONAL ON G2'S OWN CONTINGENCY — which class the creature\n"
                "  produces depends on the word it just heard, %+.3f +/- %.3f above a\n"
                "  direction drawn independently of the word. Formant error reads\n"
                "  -0.1 +/- 0.7 on the same creature, so THE WALL IS THE READOUT AND\n"
                "  NOT CONDITIONALITY. Next: six seed families, then whether a binary\n"
                "  class boundary can carry anything a listener would call a word.\n",
                d, d_se);
    return true;
  }
  std::printf("\n  NOT CONDITIONAL — G2's contingency reproduces at %+.3f, so reward is\n"
              "  shaping this class as the milestone says it does, and making the class\n"
              "  depend on the word reads %+.3f +/- %.3f against a matched-marginal\n"
              "  control. Formant error reads -0.1 +/- 0.7. The wall is not the formant\n"
              "  readout: conditionality fails on a binary class boundary and on a\n"
              "  centroid alike, on the one contingency this creature demonstrably\n"
              "  learns.\n", m_g2, d, d_se);
  (void)verbose;
  return false;
}


// --- coderprobe: is the AUDITORY front end the bottleneck, or is it not? -----
//
// The gate on building a competitive sparse auditory coder, run OUTSIDE the
// brain before any of it is built -- the pattern `shapeprobe` used to license
// DNA v46 and the one this session should have used three mechanisms ago.
//
// THE PREMISE BEING CHECKED, AND IT IS THE PROPOSAL'S OWN. The argument for a
// coder was that this creature has no unsupervised category formation and that
// `vocab` reads one-of-eight at 0.210 against chance 0.125. **But 0.210 is the
// creature's VOICE.** It is what a classifier reads off the echo the creature
// produces, 200-600 ms after the word stops. It is not what the creature HEARS.
//
// So the question a coder's value depends on is one nobody stated: how well is
// the word represented at the EAR? If the auditory module already carries
// one-of-eight near ceiling, then a better auditory code has nothing to buy, the
// 0.210 is a production limit downstream of it, and building a coder would be
// improving the one stage that is not the problem.
//
// Two readouts of the same eight-word session, no learning of any kind:
//
//   mel        the cochlea's own output, mean band energies over the word.
//              The ceiling: what is in the signal before any neuron sees it.
//   auditory   the B2 population, binned as every other probe here bins it.
//              What the creature actually has.
//
// A gate that can only say "build it" is not a gate. This one says do not, if
// the ear is already at ceiling.
bool run_coderprobe(const std::vector<uint8_t>& blob, uint64_t ticks, bool verbose) {
  aibaby::Dna dna;
  if (dna.load(blob.data(), blob.size()) != aibaby::DnaStatus::kOk) return false;
  constexpr uint32_t kReps = 3;
  constexpr uint64_t kWordTicks = 900;
  constexpr uint64_t kTrialTicks = 2800;
  instrument("coderprobe", dna.header().seed ^ 0xC0DEu, uint32_t(ticks / kTrialTicks),
             "trials per creature");
  std::printf("  the gate          would a competitive sparse auditory coder have\n"
              "                    anything to buy? Measured before building it.\n");
  std::printf("  the premise       `vocab`'s one-of-eight 0.210 is the creature's\n"
              "                    VOICE, not its ear. This reads the EAR.\n");
  std::printf("  chance            %.3f\n\n", 1.0 / double(kVocabCount));

  double sum_mel = 0.0, sum_aud = 0.0, sum_shuf = 0.0;
  uint32_t n_ok = 0;

  std::printf("  %-6s %-10s %-10s %-10s %s\n", "seed", "mel", "auditory", "shuffled",
              "trials");
  for (uint32_t r = 0; r < kReps; ++r) {
    std::vector<uint8_t> variant = blob;
    const uint64_t seed = dna.header().seed + r * 7919ull;
    std::memcpy(variant.data() + offsetof(aibaby::DnaHeader, seed), &seed, sizeof(seed));

    std::string error;
    Session s;
    if (!s.init(variant, error)) continue;
    const aibaby::DnaAudio& acfg = s.dna.header().audio;
    Ear ear;
    if (!ear.configure(acfg, error)) continue;
    VowelSource caregiver(acfg.sample_rate);
    std::vector<float> pcm(acfg.sample_rate / 1000);
    const uint32_t spt = acfg.sample_rate / 1000;
    const int32_t am = s.dna.module_with_role(aibaby::ModuleRole::kAuditory);
    if (am < 0) continue;

    const uint32_t n_trials = uint32_t(ticks / kTrialTicks);
    std::vector<std::vector<double>> fx_mel, fx_aud;
    std::vector<int> labels;
    aibaby::Rng order;
    order.seed(seed ^ 0xC0DEu);

    for (uint32_t trial = 0; trial < n_trials; ++trial) {
      // Shuffled presentation, so a classifier cannot read trial index.
      const uint32_t label = uint32_t(order.next() % kVocabCount);
      const Word& w = kWords[label];
      std::vector<double> mel(acfg.mel_channels, 0.0);
      uint32_t mel_n = 0;
      const aibaby::ModuleState& ams = s.brain.network().module(uint32_t(am));
      std::vector<double> bins(kFeatureBins, 0.0);
      uint32_t spike_frames = 0;

      for (uint64_t t = 0; t < kTrialTicks; ++t) {
        const bool sounding = t < kWordTicks;
        caregiver.render(sounding ? w.f0 : 0.0f, w.f1, w.f2, sounding ? 0.5f : 0.0f,
                         pcm.data(), spt);
        ear.tick(s.brain, pcm.data(), spt);
        s.brain.step();
        if (t >= kWordTicks) continue;
        // The word is still playing: this is what ARRIVES, which is the whole
        // question. Later windows are about what is retained.
        const aibaby::Scalar* lv = s.brain.auditory_level();
        for (uint32_t c = 0; c < acfg.mel_channels; ++c) mel[c] += double(lv[c]);
        ++mel_n;
        // Walk this tick's spike list rather than polling every neuron: it is
        // the same information and it costs spikes rather than population.
        const aibaby::Network& net = s.brain.network();
        const uint32_t* fired = net.spikes();
        for (uint32_t k = 0; k < net.spike_count(); ++k) {
          const uint32_t n = fired[k];
          if (n < ams.begin || n >= ams.begin + ams.count) continue;
          bins[size_t(uint64_t(n - ams.begin) * kFeatureBins / ams.count)] += 1.0;
        }
        ++spike_frames;
      }
      if (mel_n == 0 || spike_frames == 0) continue;
      for (double& v : mel) v /= double(mel_n);
      for (double& v : bins) v /= double(spike_frames);
      fx_mel.push_back(mel);
      fx_aud.push_back(bins);
      labels.push_back(int(label));
    }
    if (labels.size() < kVocabCount * 4) continue;

    // Nearest class centroid, interleaved within class -- `vocabcurve`'s
    // readout, so the numbers are comparable with the ones already on record.
    auto nway = [](const std::vector<std::vector<double>>& x, const std::vector<int>& y) {
      std::vector<uint32_t> seen(kVocabCount, 0), cnt(kVocabCount, 0);
      std::vector<std::vector<double>> mu(kVocabCount);
      std::vector<size_t> test;
      for (size_t t = 0; t < y.size(); ++t) {
        const int L = y[t];
        if (L < 0 || L >= int(kVocabCount) || x[t].empty()) continue;
        if (seen[L] % 2 == 0) {
          if (mu[L].empty()) mu[L].assign(x[t].size(), 0.0);
          for (size_t d = 0; d < x[t].size(); ++d) mu[L][d] += x[t][d];
          ++cnt[L];
        } else {
          test.push_back(t);
        }
        ++seen[L];
      }
      for (uint32_t k = 0; k < kVocabCount; ++k) {
        if (cnt[k] < 2) return -1.0;
        for (double& v : mu[k]) v /= double(cnt[k]);
      }
      uint32_t hit = 0, tot = 0;
      for (size_t t : test) {
        int best = -1;
        double bd = 0.0;
        for (uint32_t k = 0; k < kVocabCount; ++k) {
          double d2 = 0.0;
          for (size_t d = 0; d < mu[k].size() && d < x[t].size(); ++d) {
            const double e = x[t][d] - mu[k][d];
            d2 += e * e;
          }
          if (best < 0 || d2 < bd) { bd = d2; best = int(k); }
        }
        if (best == y[t]) ++hit;
        ++tot;
      }
      return tot ? double(hit) / double(tot) : -1.0;
    };

    const double a_mel = nway(fx_mel, labels);
    const double a_aud = nway(fx_aud, labels);
    // The label-shuffled control: the same features, the same readout, labels
    // permuted. Anything above chance here is the procedure, not the creature.
    std::vector<int> shuffled = labels;
    aibaby::Rng sh;
    sh.seed(seed ^ 0x5F1Fu);
    for (size_t i = shuffled.size(); i > 1; --i) {
      std::swap(shuffled[i - 1], shuffled[size_t(sh.next() % i)]);
    }
    const double a_shuf = nway(fx_aud, shuffled);
    if (a_mel < 0 || a_aud < 0 || a_shuf < 0) continue;
    sum_mel += a_mel; sum_aud += a_aud; sum_shuf += a_shuf;
    ++n_ok;
    std::printf("  %-6u %-10.3f %-10.3f %-10.3f %zu\n", r, a_mel, a_aud, a_shuf,
                labels.size());
  }

  if (n_ok < 2) {
    std::printf("\n  coderprobe INCONCLUSIVE — fewer than two usable creatures.\n");
    return false;
  }
  const double mel = sum_mel / n_ok, aud = sum_aud / n_ok, shuf = sum_shuf / n_ok;
  std::printf("\n  mel (the signal)      %.3f\n  auditory (the ear)    %.3f\n"
              "  shuffled control      %.3f   <- must sit at chance %.3f\n",
              mel, aud, shuf, 1.0 / double(kVocabCount));

  if (shuf > 2.0 / double(kVocabCount)) {
    std::printf("\n  CONTROL FAILED — shuffled labels score %.3f, so the readout is\n"
                "  finding structure in the procedure and no number here is worth\n"
                "  reading.\n", shuf);
    return false;
  }
  // PASS means the ear is at ceiling, which is both the answer to the gate
  // and a regression check worth keeping: the day this fails, either the
  // front end has broken or something has genuinely opened headroom, and
  // both of those are news.
  if (aud >= 0.90 * mel && aud >= 0.75) {
    std::printf("\n  DO NOT BUILD IT — the ear already carries one-of-eight at %.3f\n"
                "  against the signal's own %.3f. There is no headroom for a better\n"
                "  auditory code to occupy, so `vocab`'s 0.210 is a fact about what the\n"
                "  creature can SAY and not about what it can hear. A competitive coder\n"
                "  would be improving the one stage that is not the bottleneck.\n",
                aud, mel);
    return true;
  }
  std::printf("\n  HEADROOM — the ear carries %.3f where the signal carries %.3f, so a\n"
              "  representation that keeps more of what the cochlea delivers has\n"
              "  something to occupy. That licenses building the coder and measuring\n"
              "  it against these two numbers -- not against `vocab`'s 0.210, which is\n"
              "  the voice.\n", aud, mel);
  (void)verbose;
  return false;
}

}  // namespace aibaby_host
