// Snapshots on disk (§8). The core knows the format; this knows about files.
//
// Writing is atomic — a temporary alongside the target, then a rename — because
// the one time a snapshot matters is when something went wrong, and a run that
// dies mid-write would otherwise take the previous save down with it. Hours of
// a creature's life are usually the only copy.

#ifndef AIBABY_HOST_SNAPSHOT_FILE_H
#define AIBABY_HOST_SNAPSHOT_FILE_H

#include <cstdint>
#include <string>
#include <vector>

#include "aibaby/brain.h"

namespace aibaby_host {

// Serialises `brain` and replaces `path` with it. `dna_blob` is the genome the
// brain was hatched from; it is embedded, so the file is a whole creature.
bool save_brain(const std::string& path, const aibaby::Brain& brain,
                const std::vector<uint8_t>& dna_blob, std::string& error);

// Reads a snapshot into memory. The buffer has to outlive the Brain restored
// from it — the genome is referenced in place, and it lives in here.
bool read_snapshot(const std::string& path, std::vector<uint8_t>& bytes,
                   std::string& error);

bool file_exists(const std::string& path);

}  // namespace aibaby_host

#endif  // AIBABY_HOST_SNAPSHOT_FILE_H
