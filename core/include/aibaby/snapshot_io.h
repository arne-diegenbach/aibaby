// The byte cursor a snapshot is written through, and nothing else.
//
// Split from snapshot.h so that Network and Critic can serialise themselves
// without including Brain: the cursor is the only part of the snapshot they
// need, and a module below the Brain that had to include the Brain to save its
// own fields would be a layering inversion in the one library that has to stay
// portable.
//
// A writer constructed over a null buffer counts instead of writing. That is
// how snapshot_bytes() knows the size before anything is allocated, and how
// both sides compute the layout fingerprint that guards the format.

#ifndef AIBABY_SNAPSHOT_IO_H
#define AIBABY_SNAPSHOT_IO_H

#include "aibaby/config.h"

namespace aibaby {

class SnapshotWriter {
 public:
  SnapshotWriter(void* base, size_t size)
      : base_(static_cast<uint8_t*>(base)), size_(size) {}

  void write(const void* src, size_t n) {
    if (base_ != nullptr) {
      if (used_ > size_ || n > size_ - used_) {
        overflow_ = true;
        return;
      }
      const uint8_t* s = static_cast<const uint8_t*>(src);
      for (size_t i = 0; i < n; ++i) base_[used_ + i] = s[i];
    }
    used_ += n;
  }

  template <typename T>
  void put(const T& v) {
    write(&v, sizeof(T));
  }

  bool ok() const { return !overflow_; }
  size_t used() const { return used_; }

 private:
  uint8_t* base_;
  size_t size_;
  size_t used_ = 0;
  bool overflow_ = false;
};

class SnapshotReader {
 public:
  SnapshotReader(const void* base, size_t size)
      : base_(static_cast<const uint8_t*>(base)), size_(size) {}

  // Short reads leave the destination untouched and latch the flag, so a
  // truncated file gives a creature that is refused rather than one that is
  // half restored.
  void read(void* dst, size_t n) {
    if (used_ > size_ || n > size_ - used_) {
      underflow_ = true;
      return;
    }
    uint8_t* d = static_cast<uint8_t*>(dst);
    for (size_t i = 0; i < n; ++i) d[i] = base_[used_ + i];
    used_ += n;
  }

  template <typename T>
  void get(T& v) {
    read(&v, sizeof(T));
  }

  bool ok() const { return !underflow_; }
  size_t used() const { return used_; }

 private:
  const uint8_t* base_;
  size_t size_;
  size_t used_ = 0;
  bool underflow_ = false;
};

}  // namespace aibaby

#endif  // AIBABY_SNAPSHOT_IO_H
