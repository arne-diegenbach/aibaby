
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
};

struct MechPin {
  const char* name;
  const char* dna;     // which DNA version introduced it
  Edit edits[5];
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
     {{PatchScope::kHeader, nullptr, kCuriosityPredict, 0.5f, false}}, 1, kShort, 0},
    {"eligibility baseline", "v16",
     {{PatchScope::kHeader, nullptr, kStdpElig, 20000.0f, false}}, 1, kShort, 0},
    {"presynaptic centring", "v17",
     {{PatchScope::kHeader, nullptr, kStdpPre, 1.0f, false}}, 1, kShort, 0},
    {"per-pathway Hebbian", "v23",
     {{PatchScope::kProjection, "central->vocal", P_(hebb), 1e-4f, false}}, 1, kShort, 0},
    {"pooling interneurons", "v24",
     {{PatchScope::kModule, "central", M_(ffi_gain), 0.5f, false}}, 1, kShort, 0},
    {"apical compartment", "v25",
     {{PatchScope::kModule, "vocal", M_(apical_threshold), 0.35f, false},
      {PatchScope::kModule, "vocal", M_(apical_gain), 1.0f, false},
      {PatchScope::kProjection, "vision->vocal", P_(apical), 1.0f, true}},
     3, kShort, 0},
    {"oscillations", "v26",
     {{PatchScope::kModule, "central", M_(theta_amp), 0.05f, false},
      {PatchScope::kModule, "central", M_(gamma_amp), 0.02f, false}},
     2, kShort, 0},
    {"critical period", "v28",
     {{PatchScope::kModule, "central", M_(critical_tau_ms), 300000.0f, false}}, 1,
     kShort, 0},
    {"plateau-gated plasticity", "v29",
     {{PatchScope::kModule, "vocal", M_(apical_threshold), 0.35f, false},
      {PatchScope::kModule, "vocal", M_(apical_gain), 1.0f, false},
      {PatchScope::kProjection, "vision->vocal", P_(apical), 1.0f, true},
      {PatchScope::kModule, "vocal", M_(plateau_gate), 0.5f, false}},
     4, kShort, 0},
    {"lateral competition", "v32",
     {{PatchScope::kModule, "central", M_(lateral_gain), 0.3f, false},
      {PatchScope::kModule, "central", M_(lateral_sigma), 0.2f, false}},
     2, kShort, 0},
    {"dynamic synapses", "v36",
     {{PatchScope::kProjection, "auditory->central", P_(stp_use), 0.5f, false},
      {PatchScope::kProjection, "auditory->central", P_(stp_recover_ms), 300.0f, false}},
     2, kShort, 0},
    {"burst plasticity", "v37",
     {{PatchScope::kModule, "vocal", M_(burst_ms), 20.0f, false},
      {PatchScope::kHeader, nullptr, kStdpBurstTau, 2000.0f, false},
      {PatchScope::kProjection, "central->vocal", P_(burst_learn), 1e-3f, false}},
     3, kShort, 0},
    {"competitive pruning", "v38",
     {{PatchScope::kHeader, nullptr, kPruneCompete, 0.5f, false}}, 1, kThroughSleep, 0},
    {"per-module elig tau", "v39",
     {{PatchScope::kModule, "central", M_(elig_tau_scale), 4.0f, false}}, 1, kShort, 0},
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
      if (ed.as_uint) {
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
