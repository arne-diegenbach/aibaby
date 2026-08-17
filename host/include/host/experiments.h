// Headless experiments.
//
// The measurable goals in §2 are the whole point of the project, and none of
// them can be checked by looking at a canvas. These run the same core the
// browser drives, with scripted input and no wall clock, and print a number
// with a verdict attached.

#ifndef AIBABY_HOST_EXPERIMENTS_H
#define AIBABY_HOST_EXPERIMENTS_H

#include <cstdint>
#include <string>
#include <vector>

namespace aibaby_host {

// What a run should leave behind besides its verdict. Both are empty by
// default, so scoring a milestone costs nothing extra — an experiment that is
// only being checked writes no files and renders no audio.
//
// Supported by `babble` and `m3`. Anything else says so rather than quietly
// producing nothing.
struct ExperimentOutput {
  std::string wav;   // path prefix: <prefix>.wav, and for m3 the probes too
  std::string save;  // §8 snapshot of the creature as the run left it
};

// Returns true if the experiment's success criterion was met. Unknown names
// print the list and fail; `verify` and `verify-long` run the whole suite
// against its expected outcomes.
//
// Every experiment declares the shortest run whose output is a measurement, and
// a shorter one is refused before anything simulates. `allow_short` overrides
// the refusal for debugging — the numbers print under a banner and the run
// fails whatever they say, because a short number that can be quoted is exactly
// what the minimum exists to prevent.
bool run_experiment(const std::string& name, const std::vector<uint8_t>& dna_blob,
                    uint64_t ticks, bool verbose, const ExperimentOutput& output = {},
                    bool allow_short = false);

}  // namespace aibaby_host

#endif  // AIBABY_HOST_EXPERIMENTS_H
