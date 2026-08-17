// Brain snapshot (§8): a whole creature as bytes, so a life that took hours to
// raise can be put down and picked up again.
//
// The file is not a description of the brain. It is the arena verbatim, plus
// the handful of fields that live outside it, plus the genome those bytes were
// laid out from. Restoring hatches the creature from that genome first — which
// reproduces the arena layout exactly, because the allocator is a bump
// allocator and every size in it comes from the DNA — and then overwrites the
// contents. So nothing in the file is a pointer and nothing is fixed up on
// load: the pointers are re-derived by the rebuild, not restored.
//
// That is what makes the format's one hard rule cheap to enforce: a snapshot
// loads only against the genome it was written from, and only into a binary
// whose structures are laid out the same way. Both are checked, and a mismatch
// is refused rather than reinterpreted — the same choice the DNA loader makes
// for an old blob, for the same reason. A creature restored from a subtly
// wrong file would keep running, and every number measured after that would be
// quietly wrong.
//
// The core does no I/O: these functions work on caller-owned buffers, and the
// host turns a buffer into a file (host/snapshot_file.h).

#ifndef AIBABY_SNAPSHOT_H
#define AIBABY_SNAPSHOT_H

#include "aibaby/brain.h"
#include "aibaby/config.h"
#include "aibaby/snapshot_io.h"

namespace aibaby {

constexpr uint32_t kSnapshotMagic = 0x53424941;  // "AIBS"
// 2: DNA v15 put the previous mel frame in the critic, so the state block is a
//    different shape. Files written by a v1 build are rejected rather than
//    misread — state_bytes would catch the size change anyway, but kBadVersion
//    says what happened and kLayoutMismatch does not.
constexpr uint32_t kSnapshotVersion = 2;

// Fixed-layout file header. 64 bytes, every field explicitly sized, so the
// first thing a reader does can be a bounds check rather than a guess.
struct SnapshotHeader {
  uint32_t magic;
  uint32_t version;
  uint32_t scalar_bytes;      // sizeof(Scalar): the fixed-point port is not this creature
  uint32_t dna_size;
  uint64_t state_bytes;       // the non-arena block, which doubles as a layout fingerprint
  uint64_t arena_bytes;       // == Brain::arena_used() when this was written
  uint64_t tick;              // provenance, readable without loading
  uint64_t state_hash;        // Network::state_hash() at save; re-checked after restore
  uint32_t neuron_capacity;
  uint32_t module_count;
  uint64_t plasticity_events; // how much life is in here, for the same reason as tick
};

enum class SnapshotStatus : uint32_t {
  kOk = 0,
  kTruncated,       // shorter than its own header says it is
  kBadMagic,
  kBadVersion,
  kBadScalar,       // written by a build with a different Scalar type
  kBadDna,          // the embedded genome does not load
  kSmallBuffer,     // the output buffer is smaller than snapshot_bytes()
  kArenaTooSmall,   // caller's memory is not what this genome needs
  kLayoutMismatch,  // same genome, differently laid out binary
  kStateMismatch,   // restored, but the brain does not hash to what was saved
};

const char* snapshot_status_string(SnapshotStatus s);

// --- Saving -----------------------------------------------------------------

// Exact size of the snapshot this brain would write, so the host can allocate
// once. `dna_size` is the blob the brain was hatched from — it is embedded, so
// that a snapshot is a complete creature and not half of one.
size_t snapshot_bytes(const Brain& brain, size_t dna_size);

SnapshotStatus save_snapshot(const Brain& brain, const void* dna_blob, size_t dna_size,
                             void* out, size_t out_size, size_t* written);

// --- Loading ----------------------------------------------------------------

// Header of a snapshot, without loading it: what tick it stopped at, how big a
// creature it holds. Cheap, and it lets a caller report a bad file before
// allocating anything.
SnapshotStatus snapshot_header(const void* data, size_t size, SnapshotHeader& out);

// The genome embedded in a snapshot, pointing into `data`. The host needs this
// before it can size the arena, and it is what makes a snapshot alone enough to
// hatch the right creature.
SnapshotStatus snapshot_genome(const void* data, size_t size, const void** blob,
                               size_t* blob_size);

// Rebuilds `brain` from the snapshot's own genome and restores its state.
//
// `memory` must be Brain::required_bytes() for that genome — take it from
// snapshot_genome(), not from whatever the TOML on disk says today.
//
// `data` must outlive the Brain and be 8-byte aligned, exactly as the DNA blob
// must: the genome is referenced in place, and here it lives inside the
// snapshot.
SnapshotStatus load_snapshot(Brain& brain, const void* data, size_t size, void* memory,
                             size_t memory_size);

}  // namespace aibaby

#endif  // AIBABY_SNAPSHOT_H
