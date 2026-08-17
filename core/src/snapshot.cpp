#include "aibaby/snapshot.h"

namespace aibaby {
namespace {

// Sections are 8-byte aligned so that the embedded genome is aligned when the
// buffer is, which is what lets the core point Dna straight into the snapshot
// instead of copying the blob out of it.
constexpr size_t kAlign = 8;

inline size_t align_up(size_t n) { return (n + (kAlign - 1)) & ~(kAlign - 1); }

void copy_bytes(void* dst, const void* src, size_t n) {
  uint8_t* d = static_cast<uint8_t*>(dst);
  const uint8_t* s = static_cast<const uint8_t*>(src);
  for (size_t i = 0; i < n; ++i) d[i] = s[i];
}

// How many bytes this brain's out-of-arena state occupies, measured by writing
// it to nowhere. Both sides compute it the same way, and the saved value is the
// format's layout fingerprint: add a field to any save_state() and every older
// file stops loading, loudly, instead of being read one field out of step.
size_t state_bytes(const Brain& brain) {
  SnapshotWriter measure(nullptr, 0);
  brain.save_state(measure);
  return measure.used();
}

}  // namespace

const char* snapshot_status_string(SnapshotStatus s) {
  switch (s) {
    case SnapshotStatus::kOk: return "ok";
    case SnapshotStatus::kTruncated: return "snapshot is truncated";
    case SnapshotStatus::kBadMagic: return "not a snapshot file";
    case SnapshotStatus::kBadVersion: return "snapshot from an incompatible version";
    case SnapshotStatus::kBadScalar: return "snapshot written with a different scalar type";
    case SnapshotStatus::kBadDna: return "the genome inside the snapshot is not loadable";
    case SnapshotStatus::kSmallBuffer: return "output buffer too small";
    case SnapshotStatus::kArenaTooSmall: return "arena is not the size this genome needs";
    case SnapshotStatus::kLayoutMismatch: return "snapshot was written by a differently built binary";
    case SnapshotStatus::kStateMismatch: return "snapshot restored to a different brain than it recorded";
  }
  return "unknown";
}

size_t snapshot_bytes(const Brain& brain, size_t dna_size) {
  return sizeof(SnapshotHeader) + align_up(dna_size) + align_up(state_bytes(brain)) +
         brain.arena_used();
}

SnapshotStatus save_snapshot(const Brain& brain, const void* dna_blob, size_t dna_size,
                             void* out, size_t out_size, size_t* written) {
  const size_t state = state_bytes(brain);
  const size_t total = sizeof(SnapshotHeader) + align_up(dna_size) + align_up(state) +
                       brain.arena_used();
  if (written != nullptr) *written = total;
  if (out == nullptr || out_size < total) return SnapshotStatus::kSmallBuffer;

  uint8_t* base = static_cast<uint8_t*>(out);
  for (size_t i = 0; i < total; ++i) base[i] = 0;  // padding is never left as garbage

  SnapshotHeader h = {};
  h.magic = kSnapshotMagic;
  h.version = kSnapshotVersion;
  h.scalar_bytes = uint32_t(sizeof(Scalar));
  h.dna_size = uint32_t(dna_size);
  h.state_bytes = uint64_t(state);
  h.arena_bytes = uint64_t(brain.arena_used());
  h.tick = brain.network().tick();
  h.state_hash = brain.network().state_hash();
  h.neuron_capacity = brain.network().total_capacity();
  h.module_count = brain.network().module_count();
  h.plasticity_events = brain.plasticity_events();
  copy_bytes(base, &h, sizeof(h));

  size_t at = sizeof(SnapshotHeader);
  copy_bytes(base + at, dna_blob, dna_size);
  at += align_up(dna_size);

  SnapshotWriter w(base + at, state);
  brain.save_state(w);
  if (!w.ok() || w.used() != state) return SnapshotStatus::kLayoutMismatch;
  at += align_up(state);

  copy_bytes(base + at, brain.arena_base(), brain.arena_used());
  return SnapshotStatus::kOk;
}

SnapshotStatus snapshot_header(const void* data, size_t size, SnapshotHeader& out) {
  if (data == nullptr || size < sizeof(SnapshotHeader)) {
    // Something far too small to be a header is usually the wrong file rather
    // than a damaged one, and "not a snapshot" sends the reader somewhere more
    // useful than "truncated" does.
    if (data != nullptr && size >= sizeof(uint32_t)) {
      uint32_t magic = 0;
      copy_bytes(&magic, data, sizeof(magic));
      if (magic != kSnapshotMagic) return SnapshotStatus::kBadMagic;
    }
    return SnapshotStatus::kTruncated;
  }
  copy_bytes(&out, data, sizeof(out));
  if (out.magic != kSnapshotMagic) return SnapshotStatus::kBadMagic;
  if (out.version != kSnapshotVersion) return SnapshotStatus::kBadVersion;
  if (out.scalar_bytes != uint32_t(sizeof(Scalar))) return SnapshotStatus::kBadScalar;

  const size_t total = sizeof(SnapshotHeader) + align_up(out.dna_size) +
                       align_up(size_t(out.state_bytes)) + size_t(out.arena_bytes);
  if (size < total) return SnapshotStatus::kTruncated;
  return SnapshotStatus::kOk;
}

SnapshotStatus snapshot_genome(const void* data, size_t size, const void** blob,
                               size_t* blob_size) {
  SnapshotHeader h;
  const SnapshotStatus s = snapshot_header(data, size, h);
  if (s != SnapshotStatus::kOk) return s;
  if (blob != nullptr) *blob = static_cast<const uint8_t*>(data) + sizeof(SnapshotHeader);
  if (blob_size != nullptr) *blob_size = h.dna_size;
  return SnapshotStatus::kOk;
}

SnapshotStatus load_snapshot(Brain& brain, const void* data, size_t size, void* memory,
                             size_t memory_size) {
  SnapshotHeader h;
  const SnapshotStatus s = snapshot_header(data, size, h);
  if (s != SnapshotStatus::kOk) return s;

  const uint8_t* base = static_cast<const uint8_t*>(data);
  const uint8_t* dna_blob = base + sizeof(SnapshotHeader);

  // Hatch first, from the snapshot's own genome. This is what reproduces the
  // arena layout — same bump allocator, same sizes, same order — and therefore
  // what makes the arena bytes below meaningful.
  const BrainStatus bs = brain.init(dna_blob, h.dna_size, memory, memory_size);
  if (bs == BrainStatus::kBadDna) return SnapshotStatus::kBadDna;
  if (bs != BrainStatus::kOk) return SnapshotStatus::kArenaTooSmall;

  // Three ways the file could be from a different build of the same genome,
  // all of which would leave a plausible-looking creature behind.
  if (brain.arena_used() != size_t(h.arena_bytes)) return SnapshotStatus::kLayoutMismatch;
  if (brain.network().total_capacity() != h.neuron_capacity) return SnapshotStatus::kLayoutMismatch;
  if (brain.network().module_count() != h.module_count) return SnapshotStatus::kLayoutMismatch;
  if (state_bytes(brain) != size_t(h.state_bytes)) return SnapshotStatus::kLayoutMismatch;

  size_t at = sizeof(SnapshotHeader) + align_up(h.dna_size);
  SnapshotReader r(base + at, size_t(h.state_bytes));
  at += align_up(size_t(h.state_bytes));

  // The arena is the caller's memory — the allocator bumps from its base and
  // never moves it — so this writes over exactly the region init() just filled.
  if (brain.arena_base() != memory) return SnapshotStatus::kLayoutMismatch;

  // Copied before the fields that describe it: load_state() restores the module
  // counts, and those are what say how much of the arena is alive.
  copy_bytes(memory, base + at, size_t(h.arena_bytes));
  brain.load_state(r);
  if (!r.ok() || r.used() != size_t(h.state_bytes)) return SnapshotStatus::kLayoutMismatch;

  // The end-to-end check, and the reason the hash is in the header: it is the
  // same lever G1 uses, applied across a file instead of across two runs.
  if (brain.network().state_hash() != h.state_hash) return SnapshotStatus::kStateMismatch;
  return SnapshotStatus::kOk;
}

}  // namespace aibaby
