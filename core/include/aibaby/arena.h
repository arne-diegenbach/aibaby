// Bump allocator over host-provided memory.
//
// The kernel allocates exactly once, at build time, and never again. Growth
// (M4) takes fresh space from the same arena, which is why the DNA carries a
// per-module n_max: the arena is sized for the maximum brain the DNA permits,
// so a growing brain can never fail to allocate mid-life.

#ifndef AIBABY_ARENA_H
#define AIBABY_ARENA_H

#include "aibaby/config.h"

namespace aibaby {

class Arena {
 public:
  void init(void* memory, size_t capacity) {
    base_ = static_cast<uint8_t*>(memory);
    capacity_ = capacity;
    used_ = 0;
    failed_ = false;
  }

  // Returns nullptr on exhaustion and latches the failure flag — no
  // exceptions in the core, so callers check ok() once after a build phase
  // rather than testing every single allocation.
  void* alloc(size_t bytes, size_t align) {
    size_t base = reinterpret_cast<size_t>(base_);
    size_t cur = base + used_;
    size_t aligned = (cur + (align - 1)) & ~(align - 1);
    size_t padding = aligned - cur;
    if (bytes > capacity_ - used_ - padding || used_ + padding > capacity_) {
      failed_ = true;
      return nullptr;
    }
    used_ += padding + bytes;
    return reinterpret_cast<void*>(aligned);
  }

  template <typename T>
  T* alloc_array(size_t count) {
    return static_cast<T*>(alloc(sizeof(T) * count, alignof(T)));
  }

  // Zero-initialising variant. The kernel relies on membrane state and
  // eligibility traces starting at rest, so most arrays want this.
  template <typename T>
  T* alloc_zeroed(size_t count) {
    T* p = alloc_array<T>(count);
    if (p == nullptr) return nullptr;
    uint8_t* bytes = reinterpret_cast<uint8_t*>(p);
    for (size_t i = 0; i < sizeof(T) * count; ++i) bytes[i] = 0;
    return p;
  }

  void reset() {
    used_ = 0;
    failed_ = false;
  }

  bool ok() const { return !failed_; }
  size_t used() const { return used_; }
  size_t capacity() const { return capacity_; }

  // Where the host's memory starts. The snapshot (§8) copies [base, used) out
  // and back in wholesale: the arena holds no pointers, only values, so its
  // bytes are portable between two builds of the same brain.
  const void* base() const { return base_; }

 private:
  uint8_t* base_ = nullptr;
  size_t capacity_ = 0;
  size_t used_ = 0;
  bool failed_ = false;
};

}  // namespace aibaby

#endif  // AIBABY_ARENA_H
