#include "host/journal.h"

#include <cstring>
#include <ctime>

namespace aibaby_host {
namespace {

struct TelemetryRecord {
  uint64_t tick;
  uint32_t spikes;
  float mean_rate;
  float hunger;
  float comfort;
  float fatigue;
};

struct CaregiverRecord {
  uint64_t tick;
  uint8_t action;
  uint8_t pad[3];
  float intensity;
};

struct RewardRecord {
  uint64_t tick;
  float external;
  float total;
};

// Variable-length records carry their own count so a reader can skip a frame
// from a genome with a different channel count instead of desynchronising.
struct VectorHeader {
  uint64_t tick;
  uint32_t count;
  uint32_t pad;
};

}  // namespace

Journal::~Journal() { close(); }

bool Journal::open(const std::string& path, const std::vector<uint8_t>& dna_blob,
                   std::string& error) {
  file_.open(path, std::ios::binary | std::ios::trunc);
  if (!file_) {
    error = "cannot open journal for writing: " + path;
    return false;
  }

  const uint32_t magic = kJournalMagic;
  const uint32_t version = kJournalVersion;
  const uint64_t created = uint64_t(std::time(nullptr));
  const uint32_t dna_size = uint32_t(dna_blob.size());

  file_.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
  file_.write(reinterpret_cast<const char*>(&version), sizeof(version));
  file_.write(reinterpret_cast<const char*>(&created), sizeof(created));
  file_.write(reinterpret_cast<const char*>(&dna_size), sizeof(dna_size));
  file_.write(reinterpret_cast<const char*>(dna_blob.data()), dna_size);

  bytes_ = sizeof(magic) + sizeof(version) + sizeof(created) + sizeof(dna_size) + dna_size;
  records_ = 0;
  file_.flush();
  return bool(file_);
}

void Journal::write_record(RecordType type, const void* payload, uint32_t size) {
  if (!file_.is_open()) return;
  const uint8_t tag = static_cast<uint8_t>(type);
  file_.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
  file_.write(reinterpret_cast<const char*>(&size), sizeof(size));
  file_.write(static_cast<const char*>(payload), size);
  bytes_ += sizeof(tag) + sizeof(size) + size;
  ++records_;
}

void Journal::record_telemetry(uint64_t tick, uint32_t spikes, float mean_rate,
                               float hunger, float comfort, float fatigue) {
  TelemetryRecord r{tick, spikes, mean_rate, hunger, comfort, fatigue};
  write_record(RecordType::kTelemetry, &r, sizeof(r));
}

void Journal::record_caregiver(uint64_t tick, CaregiverAction action, float intensity) {
  CaregiverRecord r{};
  r.tick = tick;
  r.action = static_cast<uint8_t>(action);
  r.intensity = intensity;
  write_record(RecordType::kCaregiverEvent, &r, sizeof(r));
  // Caregiver actions are rare and are the events a replay must not lose, so
  // they are flushed immediately rather than riding the periodic flush.
  file_.flush();
}

void Journal::write_vector(RecordType type, uint64_t tick, const float* values,
                           uint32_t count) {
  if (!file_.is_open()) return;
  VectorHeader head{tick, count, 0};
  const uint32_t size = uint32_t(sizeof(head) + sizeof(float) * count);
  const uint8_t tag = static_cast<uint8_t>(type);
  file_.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
  file_.write(reinterpret_cast<const char*>(&size), sizeof(size));
  file_.write(reinterpret_cast<const char*>(&head), sizeof(head));
  file_.write(reinterpret_cast<const char*>(values), std::streamsize(sizeof(float) * count));
  bytes_ += sizeof(tag) + sizeof(size) + size;
  ++records_;
}

void Journal::record_audio(uint64_t tick, const float* mel, uint32_t channels) {
  write_vector(RecordType::kSensoryAudio, tick, mel, channels);
}

void Journal::record_vision(uint64_t tick, const float* features, uint32_t count) {
  write_vector(RecordType::kSensoryVideo, tick, features, count);
}

void Journal::record_vocal(uint64_t tick, const float* params, uint32_t count) {
  write_vector(RecordType::kVocal, tick, params, count);
}

void Journal::record_reward(uint64_t tick, float external, float total) {
  RewardRecord r{tick, external, total};
  write_record(RecordType::kReward, &r, sizeof(r));
  // Reward is the scarcest thing in the file and the one a replay must land
  // on the exact tick, so it does not wait for the periodic flush.
  file_.flush();
}

void Journal::flush() {
  if (file_.is_open()) file_.flush();
}

void Journal::close() {
  if (file_.is_open()) {
    file_.flush();
    file_.close();
  }
}

}  // namespace aibaby_host
