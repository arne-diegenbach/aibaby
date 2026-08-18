// Dispatch, and the two things dispatch is the right place for: how long an
// experiment has to run before its output is a measurement, and what its
// outcome is supposed to be. The experiments themselves are in
// experiments_milestones.cpp and experiments_probes.cpp; the scaffolding they
// share is in experiments_common.h.

#include "experiments_common.h"

namespace aibaby_host {
namespace {

// What running this experiment is supposed to produce.
enum class Expect {
  kPass,   // the criterion is met on the shipped genome
  kGuard,  // failing IS the right answer here: the mechanism it watches is
           // switched off in the shipped genome and the guard says so out loud
  kOpen,   // an unmet milestone. Distinct from kGuard: a guard is off on
           // purpose and would be switched on deliberately, whereas this is
           // simply the state of the world and the day it flips is the news
           // the project is waiting for.
};

enum class Tier {
  kFast,  // seconds to a minute
  kLong,  // minutes — the price of the minimum below
};

// The shortest run whose scored output is a measurement rather than a rumour.
//
// The global --ticks default of 120000 is a trap, and it has cost this project
// more than any single bug. Three separate ways:
//
//   - eligprobe reads its size-matched arcuate positive control at 0.570
//     against a chance of 0.510 there, and at 0.892 at 600k. A whole DNA v26
//     sweep was run at the default and discarded.
//   - g3probe gets 15 probes there, an accuracy step of 0.133, and its
//     condition table then blames vision->central for a loss that is actually
//     downstream. That is not noise, it is pointing at the wrong half of the
//     machine.
//   - sleep cannot complete a cycle and g4 cannot reach a plateau.
//
// In every case the table printed looked completely normal. A warning is not
// enough; the run is refused.
//
// Two kinds of `why` appear below and they are not equally strong. "derived"
// means the number falls out of the experiment's own trial arithmetic — a
// held-out table needs at least 100 trials for its accuracy step to be 0.02 or
// finer, which is the standard g3probe's own guard already applies. "measured"
// means a positive control was watched across lengths and this is where it came
// alive. "no requirement" means exactly that: 120000 is the length every
// recorded result in this project was taken at, and the minimum is there to
// stop a --ticks 5000 number being read as one of them, not because 120000 was
// shown to be enough.
struct Spec {
  const char* name;
  uint64_t min_ticks;
  Expect expect;
  Tier tier;
  const char* why;
};

constexpr uint64_t kDefaultMinimum = 120000;

const Spec kSpecs[] = {
    {"determinism", 120000, Expect::kPass, Tier::kFast,
     "the pinned hash below is recorded at exactly this length"},
    {"audio", kDefaultMinimum, Expect::kPass, Tier::kFast, "no requirement"},
    {"vision", kDefaultMinimum, Expect::kPass, Tier::kFast, "no requirement"},
    {"m2", kDefaultMinimum, Expect::kPass, Tier::kFast, "no requirement"},
    {"babble", kDefaultMinimum, Expect::kPass, Tier::kFast, "no requirement"},
    {"calibrate", kDefaultMinimum, Expect::kPass, Tier::kFast, "no requirement"},
    {"pcprobe", kDefaultMinimum, Expect::kPass, Tier::kFast, "no requirement"},
    {"dwprobe", kDefaultMinimum, Expect::kPass, Tier::kFast, "no requirement"},
    {"v1probe", kDefaultMinimum, Expect::kGuard, Tier::kFast,
     "no requirement; guard — the shipped genome has no visual cortex"},
    {"m3probe", 200000, Expect::kPass, Tier::kFast, "derived: 100 trials at 2000 ticks each"},
    {"projprobe", 200000, Expect::kPass, Tier::kFast, "derived: 100 trials at 2000 ticks each"},
    {"gazeprobe", 200000, Expect::kPass, Tier::kFast,
     "derived: 100 trials at 2000 ticks each, per scatter arm"},
    {"apicalprobe", 200000, Expect::kGuard, Tier::kFast,
     "derived: 100 trials; guard — the apical compartment ships off"},
    {"oscprobe", 200000, Expect::kGuard, Tier::kFast,
     "derived: 100 trials; guard — the oscillators ship off"},
    {"audprobe", 240000, Expect::kPass, Tier::kFast, "derived: 100 trials at 2400 ticks each"},
    {"invprobe", 200000, Expect::kPass, Tier::kLong,
     "derived: 100 trials per scatter arm, x3 creatures x3 upbringings"},
    {"cpprobe", 600000, Expect::kGuard, Tier::kLong,
     "derived: two 300 s phases. Expected to FAIL, and the failure is the current\n"
     "  finding: its own positive control says 300 s of praise does not measurably\n"
     "  shift the observable, so the protocol cannot yet ask its question. This\n"
     "  flipping to PASS means someone made it teach, which is news"},

    {"m3", kDefaultMinimum, Expect::kOpen, Tier::kLong,
     "no requirement; G3 is the open milestone, so FAIL is the current answer"},
    {"g2", kDefaultMinimum, Expect::kPass, Tier::kLong, "no requirement"},
    {"snapshot", kDefaultMinimum, Expect::kPass, Tier::kLong, "no requirement"},
    {"eligprobe", 600000, Expect::kPass, Tier::kLong,
     "measured: the arcuate control is 0.570 at 120k and 0.892 here"},
    {"g3probe", 900000, Expect::kPass, Tier::kLong,
     "derived: 141 probes, an accuracy step of 0.014 (15 and 0.133 at the default)"},
    {"condprobe", 900000, Expect::kPass, Tier::kLong, "derived: same instrument as g3probe"},
    {"pairprobe", 900000, Expect::kPass, Tier::kLong, "derived: same instrument as g3probe"},
    {"sleep", 1600000, Expect::kPass, Tier::kLong,
     "measured: the creature drops off at ~1.035M and is still asleep at 1.2M, so\n"
     "  the 1.2M in the old help text was never enough to close a cycle"},
    {"g4", 1500000, Expect::kPass, Tier::kLong,
     "measured: growth needs a plateau to form and then be acted on"},
};

// The determinism hash of dna/default.toml at 120000 ticks.
//
// Rule 1 of this project is that a new mechanism is bit-identical when it is
// switched off, and this number is how that is checked. Every version from DNA
// v14 to v28 has been added against it. It is pinned here rather than eyeballed
// because "the hash looked the same" is not a test — an accidental behaviour
// change is a red run, not a digit somebody has to notice.
//
// If you changed the genome or a mechanism's default ON purpose, this number
// moves and you update it here, in the same commit, having said in the
// changelog what moved it.
// Moved 2026-08-18 by two new tracts into the larynx, which are deliberate
// wiring changes and not mechanism defaults:
//
//   15b5dcb6d8616452 -> ca3234c61439b538   excitatory vision->vocal
//                    -> edd7d9e246927b2c   inhibitory auditory->vocal
//
// They ship together and the second is not optional: the first one alone makes
// the creature babble loudly enough to fail `audio` on five of the nine seeds.
// Four recalibrated target_rate_hz ride along. Every vocal number recorded
// before this was taken on a creature whose larynx had no visual input at all.
constexpr uint64_t kPinnedHash = 0xedd7d9e246927b2cull;

const Spec* find_spec(const std::string& name) {
  for (const Spec& s : kSpecs) {
    if (name == s.name) return &s;
  }
  return nullptr;
}

// Aliases and composites: real experiments that borrow another's protocol, so
// they borrow its minimum too.
const Spec* effective_spec(const std::string& name) {
  if (name == "g2probe") return find_spec("g2");
  if (name == "m3sweep") return find_spec("m3");
  return find_spec(name);
}

bool run_verify(const std::vector<uint8_t>& dna_blob, bool long_tier, bool verbose);

}  // namespace

bool run_experiment(const std::string& name, const std::vector<uint8_t>& dna_blob,
                    uint64_t ticks, bool verbose, const ExperimentOutput& output,
                    bool allow_short) {
  if (name == "verify") return run_verify(dna_blob, false, verbose);
  if (name == "verify-long") return run_verify(dna_blob, true, verbose);

  std::printf("experiment: %s\n", name.c_str());

  // Before anything runs, because a refusal after five minutes of simulation is
  // a refusal nobody will keep.
  if (const Spec* spec = effective_spec(name)) {
    if (ticks < spec->min_ticks) {
      if (!allow_short) {
        std::printf("  REFUSED — %s needs at least %llu ticks and was given %llu.\n"
                    "  %s\n"
                    "  Re-run with --ticks %llu. --allow-short runs it anyway, prints\n"
                    "  the numbers, and fails regardless of what they say.\n",
                    name.c_str(), (unsigned long long)spec->min_ticks,
                    (unsigned long long)ticks, spec->why,
                    (unsigned long long)spec->min_ticks);
        return false;
      }
      std::printf("  SHORT RUN — %llu ticks against a minimum of %llu. %s\n"
                  "  Everything below is printed for debugging and is NOT a\n"
                  "  measurement; this run cannot pass whatever it reads.\n\n",
                  (unsigned long long)ticks, (unsigned long long)spec->min_ticks,
                  spec->why);
    }
  }

  const Capture cap{output.wav, output.save};
  // Recording is wired into the two experiments where hearing the creature
  // answers something: what it sounds like at all, and what the milestone
  // actually scored. Saying so is the point — an ignored flag is worse than a
  // missing one, because the missing file looks like a silent baby.
  if (cap.wanted() && name != "babble" && name != "m3") {
    std::printf("  --wav/--save are not wired into %s; use babble or m3.\n", name.c_str());
    return false;
  }

  bool ok = false;
  bool known = true;
  if (name == "determinism") ok = run_determinism(dna_blob, ticks, verbose);
  else if (name == "audio") ok = run_audio(dna_blob, ticks, verbose);
  else if (name == "vision") ok = run_vision(dna_blob, ticks, verbose);
  else if (name == "m2") ok = run_m2(dna_blob, ticks, verbose);
  else if (name == "m3") ok = run_m3(dna_blob, ticks, Caregiver{}, kM3Replicates, verbose, cap);
  else if (name == "m3sweep") {
    for (float pv : {0.5f, 0.2f, 0.0f}) {
      Caregiver c;
      c.praise = pv;
      std::printf("\n########## praise = %.2f ##########\n", double(pv));
      run_m3(dna_blob, ticks, c, 2, verbose, Capture{});
    }
    ok = true;
  }
  else if (name == "m3probe") ok = run_m3probe(dna_blob, ticks, verbose);
  else if (name == "v1probe") ok = run_v1probe(dna_blob, ticks, verbose);
  else if (name == "g3probe") ok = run_g3probe(dna_blob, ticks, verbose);
  else if (name == "pcprobe") ok = run_pcprobe(dna_blob, ticks, verbose);
  else if (name == "audprobe") ok = run_audprobe(dna_blob, ticks, verbose);
  else if (name == "eligprobe") ok = run_eligprobe(dna_blob, ticks, verbose);
  else if (name == "dwprobe") ok = run_dwprobe(dna_blob, ticks, verbose);
  else if (name == "projprobe") ok = run_projprobe(dna_blob, ticks, verbose);
  else if (name == "condprobe") ok = run_condprobe(dna_blob, ticks, verbose);
  else if (name == "pairprobe") ok = run_pairprobe(dna_blob, ticks, verbose);
  else if (name == "apicalprobe") ok = run_apicalprobe(dna_blob, ticks, verbose);
  else if (name == "oscprobe") ok = run_oscprobe(dna_blob, ticks, verbose);
  else if (name == "gazeprobe") ok = run_gazeprobe(dna_blob, ticks, verbose);
  else if (name == "invprobe") ok = run_invprobe(dna_blob, ticks, verbose);
  else if (name == "cpprobe") ok = run_cpprobe(dna_blob, ticks, verbose);
  else if (name == "babble") ok = run_babble(dna_blob, ticks, verbose, cap);
  else if (name == "calibrate") ok = run_calibrate(dna_blob, ticks, verbose);
  else if (name == "sleep") ok = run_sleep(dna_blob, ticks, verbose);
  else if (name == "g4") ok = run_g4(dna_blob, ticks, verbose);
  else if (name == "g2") ok = run_g2(dna_blob, ticks, verbose, Regime{});
  else if (name == "snapshot") ok = run_snapshot(dna_blob, ticks, verbose);
  else if (name == "g2probe") {
    // Diagnostic ceiling: dense, immediate, full-strength feedback.
    Regime r;
    r.feedback_period = 10;
    r.delay = 0;
    r.praise = 1.0f;
    r.scold = -1.0f;
    ok = run_g2(dna_blob, ticks, verbose, r);
  }
  else known = false;

  if (known) {
    // A short run prints but cannot pass, whatever it read. Without this the
    // escape hatch is just the old behaviour with an extra banner, and a
    // banner scrolls off the top of the terminal.
    const Spec* spec = effective_spec(name);
    if (spec && ticks < spec->min_ticks) return false;
    return ok;
  }

  std::printf("  unknown experiment. Available:\n"
              "    verify        every fast experiment, against its expected outcome,\n"
              "                  at its own minimum length, with the hash pinned\n"
              "    verify-long   the same plus the long-horizon ones (~30 min)\n"
              "    determinism   G1: same genome + same inputs -> identical brain\n"
              "    audio         sound reaches the auditory module, silence does not\n"
              "    vision        an object reaches the vision module, an empty field does not\n"
              "    m2            M2: a held-out classifier reads object present vs absent\n"
              "                  off the association module\n"
              "    m3            M3/G3: cube and ball produce distinguishable\n"
              "                  vocalisations after the caregiver has named them\n"
              "    babble        the vocal tract has variety for reward to shape\n"
              "    calibrate     is this genome still at its measured operating point:\n"
              "                  free-running rates, amplitude floor, in-degree caps\n"
              "    sleep         fatigue discharges and the voice stops (needs 1.6M: the\n"
              "                  creature drops off at 1.035M and wakes at 1.22M)\n"
              "    g4            G4: structure grows only when needed — flat while error\n"
              "                  improves, only on a plateau, never past the DNA cap,\n"
              "                  with a forced arm proving the path works (~1.5M ticks)\n"
              "    g2            rewarded vocalisations increase within a session\n"
              "    g2probe       same, with dense immediate feedback: a diagnostic\n"
              "                  ceiling on how far reward can move this behaviour\n"
              "    g3probe       G3's ceiling: how well an idealised teacher can make\n"
              "                  the voice depend on the object it is shown\n"
              "    v1probe       is the visual cortex tuned the way its map says:\n"
              "                  measured orientation preference against predicted\n"
              "    m3probe       reads every module on the way with the same\n"
              "                  classifier: where the object and the word are, and\n"
              "                  where they stop being legible\n"
              "    pcprobe       is the critic a forward model — does it beat simply\n"
              "                  predicting the next frame looks like this one\n"
              "    audprobe      which word was said, with nothing in view, read at\n"
              "                  three integration windows\n"
              "    eligprobe     is the eligibility trace on central->vocal\n"
              "                  conditional on the object, or only large\n"
              "    dwprobe       what R-STDP actually writes: how much of the weight\n"
              "                  change depends on which object was shown\n"
              "    projprobe     can a sparse tract carry this code at all — the same\n"
              "                  activity pushed through a projection in software\n"
              "    condprobe     g3probe with the condition delivered by ear instead\n"
              "                  of by eye, so a null cannot be blamed on delivery\n"
              "    apicalprobe   does the apical compartment run, and does the\n"
              "                  plateau carry the object (DNA v25)\n"
              "    oscprobe      did the rhythm entrain, and did it produce a phase\n"
              "                  code — busier neurons firing earlier (DNA v26)\n"
              "    gazeprobe     what a displaced toy costs the visual code, whether\n"
              "                  an active fovea recovers it (DNA v31), and whether\n"
              "                  the same controller survives a motor and the eye port\n"
              "    invprobe      is displacement tolerance LEARNABLE here: three arms\n"
              "                  raised centred, jittered, or not at all, one curve\n"
              "    cpprobe       does closing the critical period protect early\n"
              "                  learning against later contradiction (DNA v28)\n"
              "    snapshot      a creature saved mid-life and resumed is bit-identical\n"
              "                  to the one that never stopped, with a perturbed control\n");
  return false;
}

namespace {

// --- verify ----------------------------------------------------------------
//
// "The suite is green" used to mean remembering that three of its eighteen
// entries legitimately exit 1 — v1probe because the shipped genome has no
// visual cortex, apicalprobe and oscprobe because the mechanisms they watch
// ship off. Three exceptions carried in a person's head is not a test; it is a
// test plus a chance to misremember, and the chance gets taken on the day the
// answer matters.
//
// So the table above records what each experiment is *supposed* to do, and this
// compares against that. A guard that starts passing is as red as a milestone
// that starts failing — it means the mechanism it watches quietly came on.
bool run_verify(const std::vector<uint8_t>& dna_blob, bool long_tier, bool verbose) {
  std::printf("verify: every %s experiment at its own minimum length, against the\n"
              "        outcome it is supposed to have.\n\n",
              long_tier ? "fast and long-horizon" : "fast");

  // The hash first and on its own, because it is the one check that fails for
  // a reason unrelated to any experiment: something in the kernel changed
  // behaviour. Everything after it would be a fact about a different creature.
  uint64_t hash = 0;
  std::printf("  --- determinism and the pinned hash ---\n");
  const bool det = run_determinism(dna_blob, 120000, verbose, &hash);
  const bool hash_ok = hash == kPinnedHash;
  if (!hash_ok) {
    std::printf("\n  HASH MOVED — %016llx, recorded %016llx.\n"
                "  Some behaviour changed. If that was on purpose, update kPinnedHash\n"
                "  in host/src/experiments.cpp and say in the changelog what moved it.\n"
                "  If it was not, nothing below is a measurement of the same creature.\n",
                (unsigned long long)hash, (unsigned long long)kPinnedHash);
  } else {
    std::printf("  hash              %016llx  as recorded\n", (unsigned long long)hash);
  }

  uint32_t passed = 0, failed = 0, open = 0;
  std::string bad;
  for (const Spec& s : kSpecs) {
    if (s.tier == Tier::kLong && !long_tier) continue;
    if (std::strcmp(s.name, "determinism") == 0) continue;
    std::printf("\n  --- %s, %llu ticks ---\n", s.name, (unsigned long long)s.min_ticks);
    const bool got = run_experiment(s.name, dna_blob, s.min_ticks, verbose);
    const bool want = s.expect == Expect::kPass;
    if (got == want) {
      ++passed;
      if (s.expect == Expect::kOpen) ++open;
    } else {
      ++failed;
      bad += std::string("    ") + s.name +
             (want ? "  expected PASS, got FAIL\n"
              : s.expect == Expect::kOpen
                  ? "  expected FAIL (open milestone), got PASS — READ THIS ONE.\n"
                    "               A milestone this project has never met just did.\n"
                  : "  expected FAIL (guard), got PASS — the mechanism it watches\n"
                    "               is no longer off\n");
    }
  }

  std::printf("\n\n========================================================\n");
  std::printf("  determinism       %s\n", det ? "PASS" : "FAIL");
  std::printf("  pinned hash       %s\n", hash_ok ? "PASS" : "FAIL");
  std::printf("  experiments       %u as expected, %u not\n", passed, failed);
  if (open) {
    std::printf("  open milestones   %u still failing, which is what \"as expected\"\n"
                "                    means for them\n", open);
  }
  if (!bad.empty()) std::printf("%s", bad.c_str());
  const bool ok = det && hash_ok && failed == 0;
  std::printf("  verify %s\n", ok ? "PASS" : "FAIL");
  if (ok && !long_tier) {
    std::printf("  (fast tier only — verify-long adds g3probe, condprobe, pairprobe,\n"
                "   eligprobe, m3, g2, snapshot, sleep and g4)\n");
  }
  std::printf("========================================================\n");
  return ok;
}

}  // namespace
}  // namespace aibaby_host
