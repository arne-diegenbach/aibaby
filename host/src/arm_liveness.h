// ---------------------------------------------------------------------------
// ARM LIVENESS -- the check for the failure that cost this project four results
// in one session on 2026-09-10/11.
//
// Every one of them was the same shape: SOMETHING LOOKED LIKE IT RAN AND DID NOT.
//
//   - `meta_floor` was swept for four hours while gated off by `meta_window = 0`,
//     and read as a clean null.
//   - `replay_credit` could not move the pinned hash because `verify` is 120k
//     ticks and the creature never sleeps in that time, so replay never fires --
//     a mechanism the determinism check structurally cannot reach.
//   - `ctxretain`'s context arms carried a live table whose index filed BOTH
//     lessons under the same slot (slot0 teach/gap 0.23/0.23), so an arm that
//     cost learning tested nothing, and "contexts do not rescue retention" was
//     published and then retracted.
//   - At smoke length six of `interleave`'s eight arms were byte-identical
//     because the creature never slept, and five printed the same retention to
//     two decimals.
//
// `tools/vacuity.sh` catches the first of those and only the first: it pokes a
// genome field to two extremes and fails if the output is unchanged. It cannot
// see an arm whose treatment is a host-side flag, a mechanism that needs a long
// run to fire, or an index that is live but uninformative.
//
// THE GENERAL RULE IS ONE LINE: an arm that is supposed to differ from its
// control must differ SOMEWHERE, and the experiment should refuse when it does
// not -- rather than pooling means over identical creatures and printing a
// beautifully clean null that nobody can distinguish from a real one.
//
// Usage, at the point an experiment has its per-arm results and before it reads
// any of them:
//
//     ArmLiveness live("interleave");
//     for (uint32_t r = 0; r < reps; ++r) {
//       live.observe(arm_name, seed_index, some_scalar_the_treatment_should_move);
//     }
//     if (!live.report(control_name)) return false;
//
// It compares each arm against the named control SEED BY SEED, which is what
// makes it sharp: two arms can have different pooled means through noise alone,
// but an arm that is byte-identical to its control on every seed is not a
// treatment, it is a copy.
#ifndef AIBABY_HOST_ARM_LIVENESS_H
#define AIBABY_HOST_ARM_LIVENESS_H

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace aibaby_host {

class ArmLiveness {
 public:
  explicit ArmLiveness(const char* experiment) : experiment_(experiment) {}

  // AN ARM THAT IS SUPPOSED TO BE INERT, and this exists because the guard was
  // MIS-SPECIFIED for exactly one arm and refused three runs for it.
  //
  // `leverprobe`'s `burst` arm enables the burst code with `f1_burst_weight = 0`, so
  // the burst centroid is computed and never read. Its whole purpose is to show that
  // merely enabling the code costs NOTHING -- which is a precondition for
  // interpreting `burst-read`, because if switching the machinery on moved the voice
  // by itself then `burst-read`'s effect could not be attributed to the readout.
  //
  // Identity is therefore the RESULT for that arm, not a failure. The guard read it
  // as "the treatment did not run" and returned false on `leverprobe_burst2`,
  // `leverprobe_v63` and `leverprobe_v66` -- and the v63 burst numbers were written
  // up off runs that had declared themselves void. A failing guard is a lead, not a
  // footnote, and this one was right about every other arm and wrong about this one.
  //
  // So the test is INVERTED for a declared-inert arm: identical on every shared seed
  // is a PASS, and any difference REFUSES -- because a difference would mean the
  // burst code leaks into the rate path.
  void expect_inert(const std::string& arm) { inert_.push_back(arm); }

  // One observation: which arm, which seed, and a quantity the arm's treatment
  // is expected to move. Anything the treatment touches will do -- the point is
  // not what it measures but that it is NOT the same number as the control's.
  void observe(const std::string& arm, uint32_t seed, double value) {
    by_arm_[arm][seed] = value;
    if (order_.empty() || std::find(order_.begin(), order_.end(), arm) == order_.end()) {
      order_.push_back(arm);
    }
  }

  // Returns false when an arm is identical to the control on every seed they
  // share, which means that arm did not run its treatment. Prints what it found
  // either way, because a liveness check that is silent on success is one more
  // thing nobody reads.
  bool report(const std::string& control) {
    auto ctl = by_arm_.find(control);
    if (ctl == by_arm_.end()) {
      std::printf("\n  %s LIVENESS: no control arm `%s` -- cannot check.\n",
                  experiment_.c_str(), control.c_str());
      return false;
    }
    bool all_live = true;
    std::printf("\n  ARM LIVENESS vs `%s` (seeds where the arm differs at all)\n",
                control.c_str());
    for (const std::string& arm : order_) {
      if (arm == control) continue;
      uint32_t shared = 0, differ = 0;
      for (const auto& kv : by_arm_[arm]) {
        auto c = ctl->second.find(kv.first);
        if (c == ctl->second.end()) continue;
        ++shared;
        if (kv.second != c->second) ++differ;
      }
      const bool declared_inert =
          std::find(inert_.begin(), inert_.end(), arm) != inert_.end();
      if (declared_inert) {
        const bool ok = shared > 0 && differ == 0;
        std::printf("    %-18s %u/%u   <- declared INERT: %s\n", arm.c_str(), differ,
                    shared,
                    ok ? "identical as required"
                       : "MOVED, and it must not -- the mechanism leaks");
        if (!ok) all_live = false;
        continue;
      }
      const bool live = shared > 0 && differ > 0;
      std::printf("    %-18s %u/%u%s\n", arm.c_str(), differ, shared,
                  live ? "" : "   <- DEAD: identical to the control on every seed");
      if (!live) all_live = false;
    }
    if (!all_live) {
      std::printf("\n  %s REFUSED -- an arm did not behave as declared. A live arm that is\n"
                  "  byte-identical to its control did not run its treatment, and pooled means\n"
                  "  over identical creatures look exactly like a clean null. An arm declared\n"
                  "  INERT that MOVED is the opposite failure and just as disqualifying.\n",
                  experiment_.c_str());
    }
    return all_live;
  }

 private:
  std::string experiment_;
  std::vector<std::string> order_;
  std::vector<std::string> inert_;   // arms whose identity to the control is required
  std::map<std::string, std::map<uint32_t, double>> by_arm_;
};

}  // namespace aibaby_host

#endif  // AIBABY_HOST_ARM_LIVENESS_H
