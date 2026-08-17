#include "host/snapshot_file.h"

#include <cstdio>
#include <fstream>

#include "aibaby/snapshot.h"

namespace aibaby_host {

bool file_exists(const std::string& path) {
  std::ifstream f(path, std::ios::binary);
  return f.good();
}

bool save_brain(const std::string& path, const aibaby::Brain& brain,
                const std::vector<uint8_t>& dna_blob, std::string& error) {
  std::vector<uint8_t> bytes(aibaby::snapshot_bytes(brain, dna_blob.size()));
  size_t written = 0;
  const aibaby::SnapshotStatus s = aibaby::save_snapshot(
      brain, dna_blob.data(), dna_blob.size(), bytes.data(), bytes.size(), &written);
  if (s != aibaby::SnapshotStatus::kOk) {
    error = std::string("snapshot: ") + aibaby::snapshot_status_string(s);
    return false;
  }

  const std::string tmp = path + ".tmp";
  {
    std::ofstream file(tmp, std::ios::binary | std::ios::trunc);
    if (!file) {
      error = "cannot open snapshot for writing: " + tmp;
      return false;
    }
    file.write(reinterpret_cast<const char*>(bytes.data()), long(written));
    file.flush();
    if (!file) {
      error = "snapshot write failed: " + tmp;
      std::remove(tmp.c_str());
      return false;
    }
  }
  if (std::rename(tmp.c_str(), path.c_str()) != 0) {
    error = "cannot replace snapshot: " + path;
    std::remove(tmp.c_str());
    return false;
  }
  return true;
}

bool read_snapshot(const std::string& path, std::vector<uint8_t>& bytes,
                   std::string& error) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    error = "cannot open snapshot: " + path;
    return false;
  }
  const std::streamoff size = file.tellg();
  if (size <= 0) {
    error = "snapshot is empty: " + path;
    return false;
  }
  file.seekg(0);
  bytes.resize(size_t(size));
  file.read(reinterpret_cast<char*>(bytes.data()), size);
  if (!file) {
    error = "snapshot read failed: " + path;
    return false;
  }
  return true;
}

}  // namespace aibaby_host
