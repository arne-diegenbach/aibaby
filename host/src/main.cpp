// aibaby host — M2
//
// Compiles the genome, builds the brain, ticks it in real time, and closes the
// loop: microphone -> mel -> B2 -> B1 -> B5 -> formant parameters -> browser
// synthesiser -> caregiver praise -> weights, with camera -> foveated retina
// -> B3 -> B1 alongside it.

#include <signal.h>
#include <time.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "aibaby/brain.h"
#include "aibaby/snapshot.h"
#include "host/audio.h"
#include "host/dna_toml.h"
#include "host/experiments.h"
#include "host/journal.h"
#include "host/snapshot_file.h"
#include "host/vision.h"
#include "host/ws_server.h"

namespace {

volatile sig_atomic_t g_running = 1;
void handle_signal(int) { g_running = 0; }

double now_seconds() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return double(ts.tv_sec) + double(ts.tv_nsec) * 1e-9;
}

// --- Tiny JSON readers for the handful of commands the panel sends ---------

std::string json_string(const std::string& msg, const std::string& key) {
  const std::string needle = "\"" + key + "\"";
  size_t pos = msg.find(needle);
  if (pos == std::string::npos) return "";
  pos = msg.find(':', pos + needle.size());
  if (pos == std::string::npos) return "";
  size_t open = msg.find('"', pos);
  if (open == std::string::npos) return "";
  size_t close = msg.find('"', open + 1);
  if (close == std::string::npos) return "";
  return msg.substr(open + 1, close - open - 1);
}

float json_number(const std::string& msg, const std::string& key, float fallback) {
  const std::string needle = "\"" + key + "\"";
  size_t pos = msg.find(needle);
  if (pos == std::string::npos) return fallback;
  pos = msg.find(':', pos + needle.size());
  if (pos == std::string::npos) return fallback;
  return float(std::strtod(msg.c_str() + pos + 1, nullptr));
}

bool json_has(const std::string& msg, const std::string& key) {
  return msg.find("\"" + key + "\"") != std::string::npos;
}

// Sequence numbers go through this rather than through json_number: a float
// stops counting exactly at 2^24, and a gaze sequence that silently starts
// repeating itself would make the eye's measured lag quietly wrong instead of
// obviously wrong.
uint64_t json_uint(const std::string& msg, const std::string& key) {
  const std::string needle = "\"" + key + "\"";
  size_t pos = msg.find(needle);
  if (pos == std::string::npos) return 0;
  pos = msg.find(':', pos + needle.size());
  if (pos == std::string::npos) return 0;
  return uint64_t(std::strtoull(msg.c_str() + pos + 1, nullptr, 10));
}

struct Options {
  std::string dna = "dna/default.toml";
  std::string journal = "journal.aibj";
  std::string web_root = "web";
  std::string experiment;
  std::string snapshot;    // §8: resume from it if it exists, save back to it
  std::string wav;         // experiment: render the creature's voice to disk
  std::string save;        // experiment: snapshot the creature it raised
  uint16_t port = 8080;
  double speed = 1.0;      // simulation speed multiplier
  uint64_t ticks = 120000; // experiment length
  // Wall-clock seconds between snapshots. Sleep is the other trigger and the
  // more natural one, but a creature can go twenty minutes of simulated time
  // without sleeping, and a crash in the nineteenth minute should not cost the
  // whole session.
  double snapshot_every = 300.0;
  bool fresh = false;      // start from birth even though the file is there
  bool verbose = false;
  // Run an experiment below its declared minimum. It prints and then fails,
  // whatever it read — see run_experiment.
  bool allow_short = false;
};

bool parse_args(int argc, char** argv, Options& opt, std::string& error) {
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto next = [&](const char* name) -> const char* {
      if (i + 1 >= argc) {
        error = std::string("missing value for ") + name;
        return nullptr;
      }
      return argv[++i];
    };
    if (arg == "--dna") { const char* v = next("--dna"); if (!v) return false; opt.dna = v; }
    else if (arg == "--journal") { const char* v = next("--journal"); if (!v) return false; opt.journal = v; }
    else if (arg == "--web") { const char* v = next("--web"); if (!v) return false; opt.web_root = v; }
    else if (arg == "--port") { const char* v = next("--port"); if (!v) return false; opt.port = uint16_t(atoi(v)); }
    else if (arg == "--speed") { const char* v = next("--speed"); if (!v) return false; opt.speed = atof(v); }
    else if (arg == "--experiment") { const char* v = next("--experiment"); if (!v) return false; opt.experiment = v; }
    else if (arg == "--ticks") { const char* v = next("--ticks"); if (!v) return false; opt.ticks = strtoull(v, nullptr, 10); }
    else if (arg == "--wav") { const char* v = next("--wav"); if (!v) return false; opt.wav = v; }
    else if (arg == "--save") { const char* v = next("--save"); if (!v) return false; opt.save = v; }
    else if (arg == "--snapshot") { const char* v = next("--snapshot"); if (!v) return false; opt.snapshot = v; }
    else if (arg == "--snapshot-every") { const char* v = next("--snapshot-every"); if (!v) return false; opt.snapshot_every = atof(v); }
    else if (arg == "--fresh") { opt.fresh = true; }
    else if (arg == "--allow-short") { opt.allow_short = true; }
    else if (arg == "--verbose") { opt.verbose = true; }
    else if (arg == "--help" || arg == "-h") {
      std::printf(
          "aibaby host\n"
          "  --dna <file.toml>     genome to compile and hatch (default dna/default.toml)\n"
          "  --journal <file>      interaction log (default journal.aibj)\n"
          "  --web <dir>           static panel root (default web)\n"
          "  --port <n>            http+ws port (default 8080)\n"
          "  --speed <x>           simulation speed multiplier (default 1.0)\n"
          "  --snapshot <file>     the creature's saved state: resumed from if the file\n"
          "                        exists, written back to it on sleep, on the interval\n"
          "                        below, and on shutdown\n"
          "  --snapshot-every <s>  wall-clock seconds between snapshots (default 300,\n"
          "                        0 to save only on sleep and shutdown)\n"
          "  --fresh               hatch from birth even if the snapshot file exists\n"
          "                        (it is overwritten at the next save)\n"
          "  --experiment <name>   run headless and exit. `verify` runs the whole\n"
          "                        fast suite against its expected outcomes and is\n"
          "                        the one to reach for; `verify-long` adds the\n"
          "                        long-horizon ones. An unknown name lists them all\n"
          "  --ticks <n>           experiment length in ticks (default 120000). Each\n"
          "                        experiment declares a minimum and refuses below it\n"
          "  --allow-short         run below that minimum anyway: the numbers print\n"
          "                        under a banner and the run fails whatever they say\n"
          "  --wav <prefix>        babble, m3: render the baby's voice to\n"
          "                        <prefix>.wav — for m3 also the probes it was\n"
          "                        scored on, split by which toy was in view\n"
          "  --save <file>         babble, m3: snapshot the creature the run\n"
          "                        raised, to resume with --snapshot and listen\n"
          "                        to it in the browser\n"
          "  --verbose             more detail from experiments\n");
      return false;
    } else {
      error = "unknown argument: " + arg;
      return false;
    }
  }
  return true;
}

// Spatial spike histogram per module, accumulated between telemetry frames.
constexpr int kBins = 32;
constexpr int kTraceLength = 240;

// How far above threshold a spike is drawn in the membrane trace. The crossing
// itself never survives to be sampled: step() resets v to rest on the same tick
// it crosses, so by the time the host reads membrane() the peak is gone and all
// that is left is the reset. Without drawing the spike, the trace shows a
// neuron that wanders and drops to rest but never once reaches its own
// threshold line — which reads as a neuron that never fires.
constexpr float kSpikeOvershoot = 1.15f;

// Browser -> host binary frames. One byte of type, then payload.
constexpr uint8_t kFrameAudio = 1;   // int16 LE PCM, mono, at the genome's rate
constexpr uint8_t kFrameVideo = 2;   // one grayscale byte per pixel, frame_size square
// Host -> browser binary frames.
constexpr uint8_t kFrameVocal = 1;   // count byte, then count x 9 float32

constexpr uint32_t kVocalFloats = 9;

void put_float(std::vector<uint8_t>& out, float v) {
  uint8_t bytes[4];
  std::memcpy(bytes, &v, 4);
  out.insert(out.end(), bytes, bytes + 4);
}

void json_number_out(std::ostringstream& json, float v) {
  json << (std::isfinite(v) ? v : 0.0f);
}

}  // namespace

int main(int argc, char** argv) {
  Options opt;
  std::string error;
  if (!parse_args(argc, argv, opt, error)) {
    if (!error.empty()) {
      std::fprintf(stderr, "error: %s\n", error.c_str());
      return 2;
    }
    return 0;
  }

  std::vector<uint8_t> dna_blob;
  if (!aibaby_host::compile_dna_toml(opt.dna, dna_blob, error)) {
    std::fprintf(stderr, "genome error: %s\n", error.c_str());
    return 1;
  }

  aibaby::Dna dna;
  const aibaby::DnaStatus status = dna.load(dna_blob.data(), dna_blob.size());
  if (status != aibaby::DnaStatus::kOk) {
    std::fprintf(stderr, "compiled genome rejected by core: %s\n",
                 aibaby::dna_status_string(status));
    return 1;
  }

  if (!opt.experiment.empty()) {
    // An experiment raises its own creatures to a protocol, and a resumed one
    // would break the arm-versus-control symmetry it is built on. Say so rather
    // than ignoring the flag: silently not saving is exactly the failure the
    // flag exists to prevent.
    if (!opt.snapshot.empty()) {
      std::fprintf(stderr,
                   "note: --snapshot does not apply to experiments; they hatch their own\n"
                   "      creatures. Use --save to keep one, or --experiment snapshot to\n"
                   "      test the format itself.\n");
    }
    aibaby_host::ExperimentOutput out;
    out.wav = opt.wav;
    out.save = opt.save;
    return aibaby_host::run_experiment(opt.experiment, dna_blob, opt.ticks, opt.verbose, out,
                                       opt.allow_short)
               ? 0
               : 1;
  }

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  // Resume (§8). The buffer is declared here because the restored brain points
  // its genome into it, exactly as it would point into the compiled blob: the
  // snapshot has to outlive the creature that came out of it.
  std::vector<uint8_t> snapshot_data;
  bool resuming = false;
  if (!opt.snapshot.empty() && !opt.fresh && aibaby_host::file_exists(opt.snapshot)) {
    if (!aibaby_host::read_snapshot(opt.snapshot, snapshot_data, error)) {
      std::fprintf(stderr, "%s\n", error.c_str());
      return 1;
    }
    const void* saved_blob = nullptr;
    size_t saved_size = 0;
    const aibaby::SnapshotStatus gs = aibaby::snapshot_genome(
        snapshot_data.data(), snapshot_data.size(), &saved_blob, &saved_size);
    if (gs != aibaby::SnapshotStatus::kOk) {
      std::fprintf(stderr, "cannot resume %s: %s\n", opt.snapshot.c_str(),
                   aibaby::snapshot_status_string(gs));
      return 1;
    }
    // The saved arena was laid out and wired by the genome inside the file, and
    // nothing else can rebuild it. An edited TOML is therefore not a tweak to
    // this creature, it is a different creature — and resuming across the
    // difference would silently invalidate every measurement taken afterwards.
    if (saved_size != dna_blob.size() ||
        std::memcmp(saved_blob, dna_blob.data(), saved_size) != 0) {
      std::fprintf(stderr,
                   "cannot resume %s: it was grown from a different genome than %s\n"
                   "  compiles to today. Hatch a new creature (--fresh, or a new\n"
                   "  --snapshot path), or restore the genome this one was raised on.\n",
                   opt.snapshot.c_str(), opt.dna.c_str());
      return 1;
    }
    resuming = true;
  }

  const size_t needed = aibaby::Brain::required_bytes(dna);
  std::vector<uint8_t> memory(needed);

  aibaby::Brain brain;
  if (resuming) {
    const aibaby::SnapshotStatus ls = aibaby::load_snapshot(
        brain, snapshot_data.data(), snapshot_data.size(), memory.data(), memory.size());
    if (ls != aibaby::SnapshotStatus::kOk) {
      std::fprintf(stderr, "cannot resume %s: %s\n", opt.snapshot.c_str(),
                   aibaby::snapshot_status_string(ls));
      return 1;
    }
  } else {
    const aibaby::BrainStatus bs =
        brain.init(dna_blob.data(), dna_blob.size(), memory.data(), memory.size());
    if (bs != aibaby::BrainStatus::kOk) {
      std::fprintf(stderr, "brain init failed (%d)\n", int(bs));
      return 1;
    }
  }

  // Saving is best-effort by design: a full disk should cost the caregiver a
  // warning, not the session that is still running fine in memory.
  auto save_snapshot_now = [&](const char* why) {
    if (opt.snapshot.empty()) return;
    std::string err;
    if (!aibaby_host::save_brain(opt.snapshot, brain, dna_blob, err)) {
      std::fprintf(stderr, "snapshot failed (%s): %s\n", why, err.c_str());
      return;
    }
    std::printf("snapshot %s at tick %llu (%s)\n", opt.snapshot.c_str(),
                (unsigned long long)brain.network().tick(), why);
    std::fflush(stdout);
  };

  // One ear for the whole host: the cochlea plus the path from the creature's
  // own larynx back into it (§5.3). The experiments use the same class, so a
  // headless result and the live creature hear the same world.
  aibaby_host::Ear ear;
  if (!ear.configure(dna.header().audio, error)) {
    std::fprintf(stderr, "cochlea error: %s\n", error.c_str());
    return 1;
  }

  aibaby_host::Retina retina;
  if (!retina.configure(dna.header().vision, error)) {
    std::fprintf(stderr, "retina error: %s\n", error.c_str());
    return 1;
  }

  const aibaby::Network& net = brain.network();
  std::printf(resuming ? "resumed '%s'\n" : "hatched '%s'\n", opt.dna.c_str());
  std::printf("  seed          %llu\n", (unsigned long long)dna.header().seed);
  std::printf("  modules       %u\n", net.module_count());
  std::printf("  neurons       %u live / %u capacity\n", net.live_neurons(),
              net.total_capacity());
  std::printf("  synapses      %u\n", net.live_synapses());
  std::printf("  arena         %.1f MB\n", double(brain.arena_used()) / (1024.0 * 1024.0));
  std::printf("  cochlea       %u mel channels @ %.1f frames/s\n", ear.channels(),
              double(ear.sample_rate()) / double(ear.hop()));
  std::printf("  retina        %u cells -> %u ON/OFF responses @ %.1f frames/s\n",
              retina.cells(), retina.feature_count(),
              double(dna.header().vision.frame_hz));
  if (net.dropped_synapses() > 0 || net.dropped_reverse() > 0) {
    std::printf(
        "  WARNING       %u synapses dropped, %u reverse entries dropped: the built\n"
        "                brain is not the brain the genome describes. Raise\n"
        "                max_out_degree on:\n",
        net.dropped_synapses(), net.dropped_reverse());
    for (uint32_t m = 0; m < net.module_count(); ++m) {
      const uint32_t fwd = net.dropped_synapses(m);
      const uint32_t rev = net.dropped_reverse(m);
      if (fwd == 0 && rev == 0) continue;
      std::printf("                  %-12s cap %u — %u outgoing, %u incoming lost\n",
                  net.module_dna(m).name, net.module_dna(m).max_out_degree, fwd, rev);
    }
  }
  for (uint32_t m = 0; m < net.module_count(); ++m) {
    std::printf("    %-14s %5u neurons (cap %u)\n", net.module_dna(m).name,
                net.module(m).count, net.module(m).capacity);
  }
  if (!opt.snapshot.empty()) {
    std::printf("  snapshot      %s — on sleep, on shutdown", opt.snapshot.c_str());
    if (opt.snapshot_every > 0.0) std::printf(", every %.0f s", opt.snapshot_every);
    std::printf("\n");
  }
  if (resuming) {
    std::printf("  resumed at    tick %llu, %llu plasticity events\n",
                (unsigned long long)net.tick(),
                (unsigned long long)brain.plasticity_events());
  }

  aibaby_host::Journal journal;
  if (!journal.open(opt.journal, dna_blob, error)) {
    std::fprintf(stderr, "journal error: %s\n", error.c_str());
    return 1;
  }
  // The journal is a life from birth (G1: genome + journal reproduce the
  // brain), and this session did not start at birth. Truncating it is the
  // honest thing to do — a log that silently began in the middle would look
  // replayable and would not be — but a replay of what follows has to start
  // from the snapshot, not from a new-born.
  if (resuming) {
    std::printf("  journal       restarts here: replay it against %s, not against birth\n",
                opt.snapshot.c_str());
  }

  aibaby_host::WsServer server;
  if (!server.start(opt.port, opt.web_root, error)) {
    std::fprintf(stderr, "server error: %s\n", error.c_str());
    return 1;
  }

  // The neuron whose membrane we stream: the first in the association module,
  // which is the one place activity from everywhere converges.
  const int32_t central = dna.module_with_role(aibaby::ModuleRole::kAssociation);
  const uint32_t sampled_neuron = net.module(uint32_t(central < 0 ? 0 : central)).begin;

  // Declared before the handlers because both the command and the binary
  // callback touch them.
  std::vector<float> mic_pending;
  std::vector<uint8_t> cam_frame(size_t(retina.frame_size()) * retina.frame_size(), 0);
  bool cam_pending = false;

  server.on_message = [&](const std::string& msg) {
    const std::string cmd = json_string(msg, "cmd");
    const float amount = json_number(msg, "amount", 0.5f);
    using aibaby_host::CaregiverAction;
    if (cmd == "feed") {
      brain.feed(amount);
      journal.record_caregiver(net.tick(), CaregiverAction::kFeed, amount);
    } else if (cmd == "poke") {
      brain.poke(amount);
      journal.record_caregiver(net.tick(), CaregiverAction::kPoke, amount);
    } else if (cmd == "tickle") {
      brain.tickle(amount);
      journal.record_caregiver(net.tick(), CaregiverAction::kTickle, amount);
    } else if (cmd == "stroke") {
      brain.stroke(amount);
      journal.record_caregiver(net.tick(), CaregiverAction::kStroke, amount);
    } else if (cmd == "mic") {
      // The browser says the stream has stopped. Drop the partial window and
      // anything still queued, so a resumed microphone starts clean.
      if (amount < 0.5f) {
        mic_pending.clear();
        ear.reset_stream();
      }
    } else if (cmd == "camera") {
      // The browser says the camera has stopped. Drop the frame in flight and
      // clear the retina, so the panel's contrast meter falls to nothing
      // instead of holding the last thing the baby saw. What B3 is being
      // driven with fades on its own — that is the encoder's business, not
      // ours — but the host's own view of the world has to end here.
      if (amount < 0.5f) {
        cam_pending = false;
        retina.reset_stream();
      }
    } else if (cmd == "praise") {
      brain.praise(amount);
      journal.record_reward(net.tick(), amount, amount);
    } else if (cmd == "eye" || cmd == "gaze" || cmd == "look") {
      // The eye port over the wire. Three messages, and between them they are
      // the whole interface a moving eye needs — whether that eye is a pan/tilt
      // head on a serial link, a browser cropping a larger frame before it
      // sends one, or an attention system that wants to override the reflex.
      //
      // Positions travel in TWO units and either may be omitted. `fx`/`fy` are
      // fractions of the frame and are what a device should speak: multiply by
      // the field of view for degrees and never learn how many pixels this
      // retina has. `x`/`y` are those pixels, for the panel, which is drawing
      // on a picture of them.
      using aibaby_host::Retina;
      const float frame = float(retina.frame_size());
      const bool has_frac = json_has(msg, "fx") || json_has(msg, "fy");
      const float px = has_frac ? json_number(msg, "fx", 0.0f) * frame
                                : json_number(msg, "x", 0.0f);
      const float py = has_frac ? json_number(msg, "fy", 0.0f) * frame
                                : json_number(msg, "y", 0.0f);
      if (cmd == "eye") {
        // Who owns the actuator. Switching to external stops the sampling
        // window sliding, because from here on the frames arrive already aimed.
        const std::string mount = json_string(msg, "mount");
        if (mount == "external") retina.set_eye_mount(Retina::EyeMount::kExternal);
        else if (mount == "internal") retina.set_eye_mount(Retina::EyeMount::kInternal);
        if (json_has(msg, "timeout")) {
          retina.set_eye_timeout_frames(uint32_t(json_number(msg, "timeout", 5.0f)));
        }
      } else if (cmd == "gaze") {
        // Feedback: where the device says the eye actually is. Echo the `seq`
        // you were acting on and the host will report your loop's dead time
        // back to you in the telemetry.
        Retina::GazeReport report;
        report.x_px = px;
        report.y_px = py;
        report.seq = json_uint(msg, "seq");
        retina.report_gaze(report);
      } else {
        // Steering from outside. On an internal eye this teleports; on an
        // external one it is a request the device is free to be slow about.
        retina.look_at(px, py);
      }
    }
  };

  // Microphone PCM. Decoded straight into the pending buffer and turned into
  // mel frames inside the simulation loop, so the brain only ever hears sound
  // on a tick boundary and a replay can land it on the same one.
  server.on_binary = [&](const uint8_t* data, size_t size) {
    if (size < 1) return;
    if (data[0] == kFrameVideo) {
      // Only ever one frame in flight. A camera is a live sensor: if the
      // browser outruns the simulation the right thing is to show the baby
      // what is in front of it now, not to work through a backlog of what was
      // there a second ago.
      if (size - 1 != cam_frame.size()) return;
      std::memcpy(cam_frame.data(), data + 1, cam_frame.size());
      cam_pending = true;
      return;
    }
    if (data[0] != kFrameAudio) return;
    const size_t samples = (size - 1) / 2;
    mic_pending.reserve(mic_pending.size() + samples);
    for (size_t i = 0; i < samples; ++i) {
      int16_t s;
      std::memcpy(&s, data + 1 + i * 2, 2);
      mic_pending.push_back(float(s) * (1.0f / 32768.0f));
    }
    // If the browser ever outruns us, drop the oldest audio rather than grow.
    // Latency matters more than completeness for a live sensor.
    const size_t cap = size_t(ear.sample_rate());  // one second
    if (mic_pending.size() > cap) {
      mic_pending.erase(mic_pending.begin(), mic_pending.end() - long(cap));
    }
  };

  std::printf("\npanel: http://localhost:%u  (ctrl-c to stop)\n", opt.port);
  std::fflush(stdout);

  const double dt_seconds = double(dna.header().sim.dt_ms) / 1000.0;
  const double telemetry_interval = 1.0 / 30.0;
  const double samples_per_tick = double(ear.sample_rate()) * dt_seconds;
  const uint32_t channels = ear.channels();

  std::vector<std::vector<uint32_t>> bins(net.module_count(),
                                          std::vector<uint32_t>(kBins, 0));
  std::vector<float> trace;
  trace.reserve(kTraceLength);
  std::vector<float> latest_mel(channels, 0.0f);
  std::vector<uint8_t> vocal_out;
  std::vector<float> pcm_slice;
  uint32_t vocal_frames_queued = 0;
  uint32_t last_vocal_frame = 0;
  uint64_t vocalizations = 0;
  uint64_t last_vocal_event_tick = 0;
  float mic_peak = 0.0f;

  // Reward is an impulse cashed in at 100 Hz; the panel refreshes at 30.
  // Sampling the latest value would miss almost every praise the caregiver
  // ever gives, and a reward display that usually reads zero is worse than
  // none. Accumulate over the frame instead.
  aibaby::RewardBreakdown reward_sum;
  uint64_t last_plasticity_events = brain.plasticity_events();

  double last_time = now_seconds();
  double sim_debt = 0.0;
  double last_telemetry = last_time;
  double sample_debt = 0.0;
  uint64_t last_journal_tick = 0;
  double last_snapshot = last_time;
  bool was_asleep = brain.asleep();

  while (g_running) {
    const double now = now_seconds();
    const double elapsed = now - last_time;
    last_time = now;

    sim_debt += elapsed * opt.speed;
    // Cap catch-up so a stall (or a laptop lid) cannot spiral into a
    // multi-minute burst of simulation.
    if (sim_debt > 0.25) sim_debt = 0.25;

    while (sim_debt >= dt_seconds) {
      // Hand the cochlea this tick's worth of microphone samples. Doing it
      // here rather than in the socket callback is what keeps audio aligned
      // to ticks, and therefore replayable.
      sample_debt += samples_per_tick;
      const size_t want = size_t(sample_debt);
      if (want > 0) {
        // Whatever the room is doing, padded with silence. The ear runs every
        // tick now rather than only when the microphone has something to say:
        // since DNA v6 the creature hears its own voice, and a baby alone in a
        // quiet room is exactly the case that has to keep working.
        size_t take = 0;
        if (!mic_pending.empty()) {
          take = want < mic_pending.size() ? want : mic_pending.size();
          pcm_slice.assign(mic_pending.begin(), mic_pending.begin() + long(take));
          mic_pending.erase(mic_pending.begin(), mic_pending.begin() + long(take));
        } else {
          pcm_slice.clear();
        }
        // The level meter is about the microphone, so it is read before the
        // creature's own voice is mixed in — otherwise a babbling baby lights
        // up the input meter and the caregiver thinks the mic is live.
        for (size_t i = 0; i < take; ++i) {
          const float a = std::fabs(pcm_slice[i]);
          if (a > mic_peak) mic_peak = a;
        }
        pcm_slice.resize(want, 0.0f);
        sample_debt -= double(want);

        ear.tick(brain, pcm_slice.data(), want);
        if (ear.had_frame()) {
          latest_mel = ear.latest_mel();
          journal.record_audio(net.tick(), latest_mel.data(), channels);
        }
        ear.take_peak();
      }

      // A camera frame, if one arrived since the last tick. Presented here
      // rather than in the socket callback for the same reason the microphone
      // is: the brain should only ever meet the world on a tick boundary, so a
      // replay can land it on the same one.
      if (cam_pending) {
        cam_pending = false;
        retina.present(cam_frame.data());
        brain.see(retina.features().data(), retina.feature_count());
        journal.record_vision(net.tick(), retina.features().data(),
                              retina.feature_count());
      }

      brain.step();
      sim_debt -= dt_seconds;

      // §8 asks for a snapshot on sleep, and sleep is the right moment for one:
      // it is the window where the structure is being rewritten (§3.6), so a
      // file written just after the creature drops off is a creature whose
      // consolidation is about to be reflected in it either way, and never one
      // caught mid-surgery.
      if (brain.asleep() != was_asleep) {
        was_asleep = brain.asleep();
        if (was_asleep) {
          save_snapshot_now("asleep");
          last_snapshot = now_seconds();
        }
      }

      if (brain.plasticity_events() != last_plasticity_events) {
        last_plasticity_events = brain.plasticity_events();
        const aibaby::RewardBreakdown& r = brain.reward();
        reward_sum.external += r.external;
        reward_sum.hunger += r.hunger;
        reward_sum.comfort += r.comfort;
        reward_sum.curiosity += r.curiosity;
        reward_sum.total += r.total;
        reward_sum.effective += r.effective;
        reward_sum.baseline = r.baseline;  // a level, not an impulse
      }

      // Accumulate the raster between frames: at 1 kHz we cannot ship every
      // spike to a 30 Hz panel, but a spatial histogram keeps the shape.
      bool sampled_spiked = false;
      for (uint32_t s = 0; s < net.spike_count(); ++s) {
        const uint32_t i = net.spikes()[s];
        if (i == sampled_neuron) sampled_spiked = true;
        for (uint32_t m = 0; m < net.module_count(); ++m) {
          const aibaby::ModuleState& ms = net.module(m);
          if (i >= ms.begin && i < ms.begin + ms.count) {
            const uint32_t offset = i - ms.begin;
            const uint32_t bin = ms.count > 1 ? offset * (kBins - 1) / (ms.count - 1) : 0;
            ++bins[m][bin];
            break;
          }
        }
      }

      if (trace.size() >= kTraceLength) trace.erase(trace.begin());
      trace.push_back(sampled_spiked
                          ? float(net.threshold(sampled_neuron)) * kSpikeOvershoot
                          : float(net.membrane(sampled_neuron)));

      // A new motor frame: queue it for the synthesiser and, when it is a
      // fresh voiced burst, count it as a vocalisation.
      if (brain.vocal_frame() != last_vocal_frame) {
        last_vocal_frame = brain.vocal_frame();
        const aibaby::VocalParams& v = brain.voice();
        put_float(vocal_out, float(v.f0));
        put_float(vocal_out, float(v.f1));
        put_float(vocal_out, float(v.f2));
        put_float(vocal_out, float(v.f3));
        put_float(vocal_out, float(v.bw1));
        put_float(vocal_out, float(v.bw2));
        put_float(vocal_out, float(v.bw3));
        put_float(vocal_out, float(v.amplitude));
        put_float(vocal_out, float(v.voicing));
        ++vocal_frames_queued;

        if (last_vocal_frame % 5 == 0) {
          const float params[kVocalFloats] = {
              float(v.f0), float(v.f1), float(v.f2), float(v.f3), float(v.bw1),
              float(v.bw2), float(v.bw3), float(v.amplitude), float(v.voicing)};
          journal.record_vocal(net.tick(), params, kVocalFloats);
        }
        if (v.voicing > 0.5f && v.amplitude > 0.30f &&
            net.tick() - last_vocal_event_tick > 200) {
          last_vocal_event_tick = net.tick();
          ++vocalizations;
        }
      }

      if (net.tick() - last_journal_tick >= 100) {
        last_journal_tick = net.tick();
        const aibaby::Telemetry t = net.telemetry();
        journal.record_telemetry(t.tick, t.total_spikes, float(t.mean_rate_hz),
                                 float(brain.drives().hunger),
                                 float(brain.drives().comfort),
                                 float(brain.drives().fatigue));
      }
    }

    server.poll();

    if (opt.snapshot_every > 0.0 && now - last_snapshot >= opt.snapshot_every) {
      last_snapshot = now;
      save_snapshot_now("interval");
    }

    if (now - last_telemetry >= telemetry_interval) {
      last_telemetry = now;
      if (server.has_clients()) {
        // Motor parameters first: the synthesiser is the one consumer that
        // notices latency.
        if (vocal_frames_queued > 0) {
          std::vector<uint8_t> packet;
          packet.reserve(vocal_out.size() + 2);
          packet.push_back(kFrameVocal);
          packet.push_back(uint8_t(vocal_frames_queued > 255 ? 255 : vocal_frames_queued));
          const size_t keep = size_t(packet[1]) * kVocalFloats * 4;
          const size_t skip = vocal_out.size() - keep;
          packet.insert(packet.end(), vocal_out.begin() + long(skip), vocal_out.end());
          server.broadcast_binary(packet.data(), packet.size());
        }

        const aibaby::Telemetry t = net.telemetry();
        const aibaby::VocalParams& v = brain.voice();
        const aibaby::RewardBreakdown& r = reward_sum;
        const aibaby::Expression& e = brain.expression();

        std::ostringstream json;
        json << "{\"tick\":" << t.tick
             << ",\"neurons\":" << t.live_neurons
             << ",\"synapses\":" << t.live_synapses
             << ",\"rate\":" << t.mean_rate_hz
             << ",\"weight\":" << t.mean_weight
             << ",\"elig\":" << t.mean_eligibility
             << ",\"vocalizations\":" << vocalizations
             << ",\"micPeak\":" << mic_peak
             << ",\"asleep\":" << (brain.asleep() ? "true" : "false")
             << ",\"drives\":{\"hunger\":" << brain.drives().hunger
             << ",\"comfort\":" << brain.drives().comfort
             << ",\"fatigue\":" << brain.drives().fatigue << "}"
             << ",\"reward\":{\"external\":"; json_number_out(json, float(r.external));
        json << ",\"hunger\":"; json_number_out(json, float(r.hunger));
        json << ",\"comfort\":"; json_number_out(json, float(r.comfort));
        json << ",\"curiosity\":"; json_number_out(json, float(r.curiosity));
        json << ",\"total\":"; json_number_out(json, float(r.total));
        json << ",\"baseline\":"; json_number_out(json, float(r.baseline));
        json << ",\"effective\":"; json_number_out(json, float(r.effective));
        json << "},\"critic\":{\"fast\":"; json_number_out(json, float(brain.critic().fast_error()));
        json << ",\"slow\":"; json_number_out(json, float(brain.critic().slow_error()));
        json << "},\"expression\":{\"valence\":" << e.valence
             << ",\"arousal\":" << e.arousal << "}"
             << ",\"vocal\":{\"f0\":" << v.f0 << ",\"f1\":" << v.f1
             << ",\"f2\":" << v.f2 << ",\"f3\":" << v.f3
             << ",\"amp\":" << v.amplitude << ",\"voicing\":" << v.voicing
             << ",\"groups\":[";
        for (uint32_t g = 0; g < aibaby::kVocalGroups; ++g) {
          if (g) json << ',';
          json_number_out(json, float(brain.vocal_groups()[g]));
        }
        json << "]},\"mel\":[";
        // The encoder's current drive, not the last frame that arrived: it
        // fades when the room goes quiet and is silent while the baby is
        // asleep, so the panel shows what B2 is receiving rather than a
        // souvenir of the last thing anyone said.
        const aibaby::Scalar* ears = brain.auditory_level();
        for (uint32_t c = 0; c < channels; ++c) {
          if (c) json << ',';
          json_number_out(json, float(ears[c]));
        }
        // The eyes, on the same principle as the ears: what B3 is being driven
        // with, not the last frame that happened to arrive. One signed number
        // per cell — ON minus OFF — because that is what a retina view draws,
        // and it halves the payload.
        json << "],\"retina\":[";
        const aibaby::Scalar* eyes = brain.vision_level();
        const uint32_t cells = brain.vision_features_count() / 2;
        for (uint32_t c = 0; c < cells; ++c) {
          if (c) json << ',';
          json_number_out(json, float(eyes[c * 2] - eyes[c * 2 + 1]));
        }
        // The geometry the panel needs to lay those cells out. Sent as the
        // shape rather than as coordinates: five integers instead of a few
        // hundred floats, thirty times a second.
        const aibaby::DnaVision& vcfg = dna.header().vision;
        json << "],\"retinaShape\":{\"frame\":" << vcfg.frame_size
             << ",\"fovea\":" << vcfg.fovea_size
             << ",\"foveaGrid\":" << vcfg.fovea_grid
             << ",\"ringGrid\":" << vcfg.ring_grid
             << ",\"rings\":" << aibaby::vision_rings(vcfg)
             << ",\"hz\":" << vcfg.frame_hz << "}"
             << ",\"visionContrast\":";
        json_number_out(json, retina.contrast());
        // Where the eye is, where it was told to go, and whether whoever owns
        // the motor is still answering. This is the outbound half of the eye
        // port: a device reads `fx`/`fy` and drives with it, and the panel
        // reads `x`/`y` and draws a crosshair. The v1.0.1 fovea shipped without
        // any of this, so the eye moved in the browser with no way to see it —
        // which is the invisible-mechanism failure this project keeps paying
        // for, landing on the very path the change was justified by.
        {
          const aibaby_host::Retina::GazeCommand g = retina.gaze_command();
          json << ",\"gaze\":{\"x\":"; json_number_out(json, retina.gaze_x());
          json << ",\"y\":"; json_number_out(json, retina.gaze_y());
          json << ",\"cx\":"; json_number_out(json, g.x_px);
          json << ",\"cy\":"; json_number_out(json, g.y_px);
          json << ",\"fx\":"; json_number_out(json, g.x_frac);
          json << ",\"fy\":"; json_number_out(json, g.y_frac);
          json << ",\"seq\":" << g.seq
               << ",\"moves\":" << retina.gaze_moves()
               << ",\"mount\":\""
               << (retina.eye_mount() == aibaby_host::Retina::EyeMount::kExternal
                       ? "external" : "internal")
               << "\",\"stale\":" << (retina.eye_stale() ? "true" : "false")
               << ",\"lag\":" << retina.eye_lag_frames()
               << ",\"reports\":" << retina.eye_reports()
               << ",\"stalls\":" << retina.eye_stalls() << "}";
        }
        json << ",\"modules\":[";
        for (uint32_t m = 0; m < net.module_count(); ++m) {
          if (m) json << ',';
          json << "{\"name\":\"" << net.module_dna(m).name << "\""
               << ",\"n\":" << net.module(m).count
               << ",\"rate\":" << net.module(m).mean_rate << ",\"bins\":[";
          for (int b = 0; b < kBins; ++b) {
            if (b) json << ',';
            json << bins[m][size_t(b)];
          }
          json << "]}";
        }
        // The sampled neuron's own threshold, so the panel can draw the line
        // where it actually is. Intrinsic plasticity moves it over minutes
        // (§3.1), and a threshold painted at a constant would quietly stop
        // describing the trace underneath it.
        // Growth and prune events (§9). Cumulative counters rather than a
        // stream: structural change is rare and the panel is a live view, so a
        // client that connects an hour in should still see what has happened
        // to this brain rather than only what happens next.
        const aibaby::StructuralStats& st = net.structural();
        const aibaby::GrowthWatch& gw = brain.growth_watch();
        json << "],\"structure\":{\"grown\":" << st.neurons_grown
             << ",\"growthEvents\":" << st.growth_events
             << ",\"prunedSynapses\":" << st.synapses_pruned
             << ",\"prunedNeurons\":" << st.neurons_pruned
             << ",\"consolidations\":" << st.consolidations
             << ",\"replays\":" << st.replays
             << ",\"lastGrowthTick\":" << st.last_growth_tick
             << ",\"lastPruneTick\":" << st.last_prune_tick
             << ",\"capacity\":" << net.total_capacity()
             << ",\"replaying\":" << (brain.replaying() ? "true" : "false")
             << ",\"plateau\":" << (gw.plateaued ? "true" : "false")
             << ",\"windows\":" << gw.windows
             << ",\"improvement\":"; json_number_out(json, float(gw.improvement));
        json << ",\"plasticity\":"; json_number_out(json, float(net.mean_plasticity()));
        json << "},\"threshold\":";
        json_number_out(json, float(net.threshold(sampled_neuron)));
        json << ",\"trace\":[";
        for (size_t i = 0; i < trace.size(); ++i) {
          if (i) json << ',';
          json_number_out(json, trace[i]);
        }
        json << "]}";
        server.broadcast_text(json.str());
      }
      vocal_out.clear();
      vocal_frames_queued = 0;
      mic_peak = 0.0f;
      reward_sum = aibaby::RewardBreakdown{};
      for (auto& row : bins) std::fill(row.begin(), row.end(), 0u);
    }

    // Yield rather than spin when we are ahead of the simulation clock.
    if (sim_debt < dt_seconds) {
      timespec nap{0, 500000};  // 0.5 ms
      nanosleep(&nap, nullptr);
    }
  }

  journal.flush();
  journal.close();
  server.stop();
  save_snapshot_now("shutdown");

  std::printf("\nstopped at tick %llu — %llu vocalisations — journal: %s"
              " (%llu records, %.1f KB)\n",
              (unsigned long long)net.tick(), (unsigned long long)vocalizations,
              opt.journal.c_str(), (unsigned long long)journal.records(),
              double(journal.bytes()) / 1024.0);
  return 0;
}
