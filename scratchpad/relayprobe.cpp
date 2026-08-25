
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

  std::printf("\n  `vs off` is central's spikes-per-auditory-spike against the constant-\n"
              "  weight relay at the SAME envelope. A gain change is flat down that\n"
              "  column; a bandpass peaks. The spread below is the best rate over the\n"
              "  worst, so 1.00 is flat and the off arm's own spread is the noise floor.\n");
  std::printf("\n  off        spread %.3f   (the floor: this column is 1.000 by\n"
              "                            construction, so this is measurement noise)\n"
              "  dep -> dep spread %.3f   peak at %.0f Hz\n"
              "  dep -> fac spread %.3f   peak at %.0f Hz   <- Webb's order\n"
              "  fac -> dep spread %.3f   peak at %.0f Hz   (the same pair, swapped)\n",
              s_off, s_dd, double(kRelayRates[peak_dd - 1]), s_df,
              double(kRelayRates[peak_df - 1]), s_fd, double(kRelayRates[peak_fd - 1]));

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
  } else if (s_df > s_off * 1.5 && s_df > s_dd) {
    std::printf("\n  A BANDPASS. `dep -> fac` peaks at %.0f Hz with a spread of %.3f,\n"
                "  above the off arm's %.3f floor and above `dep -> dep`'s %.3f — so it\n"
                "  is the MISMATCH between the two stages and not the relay. That is\n"
                "  Webb's circuit doing what Webb's circuit does, in a creature that\n"
                "  reached it by measurement rather than by copying.\n",
                double(kRelayRates[peak_df - 1]), s_df, s_off, s_dd);
  } else {
    std::printf("\n  NO BANDPASS. `dep -> fac` spreads %.3f against an off-arm floor of\n"
                "  %.3f, so the two-stage circuit is still a gain change. The mismatch\n"
                "  between the time constants is real — `gain in` and `gain out` move in\n"
                "  opposite directions — but what reaches central does not depend on the\n"
                "  envelope. Reported, not fatal: this is the finding.\n",
                s_df, s_off);
  }
  (void)verbose;
  return relay_alive && control_ok && arms_differ;
}
