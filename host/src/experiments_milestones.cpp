// The numbered goals, and the path checks every goal depends on.
//
// Shared scaffolding is in experiments_common.h.

#include "experiments_common.h"

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

    const aibaby::VocalParams& v = s.brain.voice();
    const float f1v = float(s.brain.vocal_groups()[2]);
    amp_sum += double(v.amplitude);
    f1_sum += f1v;
    f1_hist[std::min<uint32_t>(9, uint32_t(f1v * 10.0f))]++;
    amp_hist[std::min<uint32_t>(9, uint32_t(float(v.amplitude) * 10.0f))]++;
    if (v.voicing > 0.5f) ++voiced;
    if (v.amplitude > kAmplitudeFloor) ++loud;
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


M3Run run_m3_session(const std::vector<uint8_t>& blob, uint64_t ticks, bool paired,
                     const Caregiver& care, bool verbose, const Capture& cap) {
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
      if (v.voicing > 0.5f && v.amplitude > kAmplitudeFloor) ++rec.voiced;
    }
    return rec;
  };

  std::vector<std::vector<double>> probe_vocal, probe_timbre, probe_b1;
  std::vector<std::vector<double>> echo_vocal;
  std::vector<int> probe_labels, echo_labels;
  std::vector<uint32_t> probe_taught;  // presentations heard before this probe

  // A balanced, shuffled deck of objects, refilled as it runs out: balanced so
  // that always guessing "ball" cannot score, shuffled so the readout cannot
  // pick up on alternation.
  std::vector<int> deck;
  auto next_object = [&]() -> int {
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
      // route rather than the visual one. It is the ceiling the visual route
      // could inherit, which is the first thing to look at when the milestone
      // fails.
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

  double sum[2][5] = {{0}};
  uint32_t valid[2] = {0, 0};
  uint32_t above = 0;
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
      std::printf("  %-4u %-6s %-9.3f %-9.3f %-9.3f %-9.3f %-9.3f %+-9.3f %.4f\n", r,
                  c == 0 ? "named" : "muddle", run[c].vocal, run[c].timbre,
                  run[c].shuffled, run[c].echo, run[c].b1_shape, run[c].alignment,
                  run[c].reward);
    }
    if (!run[0].ok || !run[1].ok) continue;
    if (run[0].vocal >= 0.75) ++above;
    if (run[0].vocal > run[1].vocal) ++beat_control;
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
  std::printf("  picture drives the voice the way the word does (cosine, 0 = unrelated)\n");
  std::printf("    named consistently   %+.3f\n", paired_align);
  std::printf("    named at random      %+.3f\n", control_align);

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


}  // namespace aibaby_host
