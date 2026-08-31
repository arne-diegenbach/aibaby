// aibaby core — compile-time configuration and the scalar portability seam.
//
// The core is headless and freestanding-ish: no exceptions, no RTTI, no STL,
// no I/O, no threads, no allocation outside the arena. Everything the host
// needs is passed in.
//
// This was once justified by an embedded target, which stopped being a project
// requirement on 2026-08-29 (and the creature never fitted one anyway — see
// `--experiment footprint`). The constraints are kept for what they buy here:
// a core that cannot allocate or reach a syscall is deterministic, is testable
// without a harness, and cannot hide a state change from the pinned hash.

#ifndef AIBABY_CONFIG_H
#define AIBABY_CONFIG_H

#include <stddef.h>
#include <stdint.h>

namespace aibaby {

// The scalar seam. Desktop builds use float; a fixed-point port swaps this
// typedef and the four math wrappers below, and nothing else in the core
// needs to know. Keep every arithmetic helper the kernel needs behind here.
#ifndef AIBABY_SCALAR_FIXED
using Scalar = float;
#else
#error "fixed-point scalar not implemented yet — see docs/portability.md"
#endif

constexpr Scalar kZero = Scalar(0);
constexpr Scalar kOne = Scalar(1);

// Hard structural limits. Static so the host can size the arena up front and
// so nothing in the kernel needs a growable container.
constexpr uint32_t kMaxModules = 16;
constexpr uint32_t kMaxProjections = 64;
constexpr uint32_t kMaxNameLen = 24;

// Cochlear channel ceiling. The genome picks the actual count (24 in §5.2);
// this only bounds the fixed-size buffers that carry a frame across the
// host/core seam.
constexpr uint32_t kMaxMelChannels = 64;

// The same, for the retina (§5.1): ON/OFF responses of the foveated ganglion
// field. The genome's own geometry decides the real count — 176 in the default
// body plan — and this only bounds the buffer that carries one frame across
// the seam.
constexpr uint32_t kMaxVisionFeatures = 512;

// §5.3: eight continuous vocal-tract parameters, plus a voicing gate. The
// count is fixed because it is the shape of the synthesiser on the other side
// of the wire, not a tunable.
constexpr uint32_t kVocalParams = 8;
constexpr uint32_t kVocalGroups = kVocalParams + 1;  // + voicing

// DNA v48. Most postures a motor dictionary may hold. Fixed because the decoder
// carries the per-unit activity inline rather than out of the arena, and
// because a vowel inventory this creature could actually select among is small:
// `vocab` reads one-of-eight at 0.210 against chance 0.125, so a dictionary
// larger than its vocabulary is a readout it cannot use.
constexpr uint32_t kMaxDictionaryUnits = 64;

// Population bins used to summarise a module's activity for the curiosity
// forward model. Binning rather than per-neuron features is what keeps the
// critic the same size before and after growth (M4).
constexpr uint32_t kCriticBins = 32;

// Sleep replay (§3.6). The ceiling on how many high-reward episodes a brain
// keeps to re-experience while it sleeps; the genome picks the actual number.
// Fixed because the episodes are arena-allocated at birth like everything
// else — a replay buffer that grew would be the one allocation after hatching.
constexpr uint32_t kMaxReplayEpisodes = 32;

}  // namespace aibaby

#endif  // AIBABY_CONFIG_H
