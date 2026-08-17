// Writing audio out of a headless run.
//
// The experiments measure the baby's voice with a classifier, and a number is
// the only honest way to settle whether two vocalisations differ. But a
// classifier cannot tell you what the creature *sounds* like, and that is a
// real question when the milestone is about speech — so the recorder renders
// the vocal tract to PCM and this writes it somewhere you can play it.
//
// 16-bit PCM only. The point is a file that every player and every editor
// opens without being asked twice.

#ifndef AIBABY_HOST_WAV_H
#define AIBABY_HOST_WAV_H

#include <cstdint>
#include <string>
#include <vector>

namespace aibaby_host {

// Interleaved float samples in [-1, 1], `channels` per frame. Anything outside
// the range is clipped rather than wrapped: a recording that overflowed should
// sound loud, not shredded.
bool write_wav(const std::string& path, const std::vector<float>& samples,
               uint32_t channels, uint32_t sample_rate, std::string& error);

}  // namespace aibaby_host

#endif  // AIBABY_HOST_WAV_H
