// Append-only interaction log.
//
// The journal embeds the genome in its header, so a journal file is a
// complete, self-contained recipe for one life: replaying it against the core
// must reproduce the brain bit-for-bit (requirement G1). Without this we
// could never re-run an experiment and would be tuning by vibes.

#ifndef AIBABY_HOST_JOURNAL_H
#define AIBABY_HOST_JOURNAL_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace aibaby_host {

constexpr uint32_t kJournalMagic = 0x4A424941;  // "AIBJ"
constexpr uint32_t kJournalVersion = 1;

enum class RecordType : uint8_t {
  kTelemetry = 1,
  kCaregiverEvent = 2,
  kSensoryAudio = 3,
  kSensoryVideo = 4,
  kReward = 5,
  kVocal = 6,
};

enum class CaregiverAction : uint8_t {
  kFeed = 1,
  kPoke = 2,
  kTickle = 3,
  kStroke = 4,
};

class Journal {
 public:
  ~Journal();

  bool open(const std::string& path, const std::vector<uint8_t>& dna_blob,
            std::string& error);
  void record_telemetry(uint64_t tick, uint32_t spikes, float mean_rate,
                        float hunger, float comfort, float fatigue);
  void record_caregiver(uint64_t tick, CaregiverAction action, float intensity);

  // The mel frame, not the PCM. This is exactly what the core was given, so a
  // replay reproduces the brain without having to reproduce the DSP — and it
  // is ~6 KB/s instead of 32.
  void record_audio(uint64_t tick, const float* mel, uint32_t channels);

  // The retinal responses, not the pixels — for the same two reasons as the
  // mel frame above. It is what the core was actually given, and it is 176
  // floats rather than a video stream.
  void record_vision(uint64_t tick, const float* features, uint32_t count);

  // Explicit caregiver feedback and the total reward it became.
  void record_reward(uint64_t tick, float external, float total);

  // The motor side of the loop. Without it we could see that the baby was
  // rewarded but not what it was rewarded *for*.
  void record_vocal(uint64_t tick, const float* params, uint32_t count);

  void flush();
  void close();

  bool is_open() const { return file_.is_open(); }
  uint64_t records() const { return records_; }
  uint64_t bytes() const { return bytes_; }

 private:
  void write_record(RecordType type, const void* payload, uint32_t size);
  void write_vector(RecordType type, uint64_t tick, const float* values, uint32_t count);

  std::ofstream file_;
  uint64_t records_ = 0;
  uint64_t bytes_ = 0;
};

}  // namespace aibaby_host

#endif  // AIBABY_HOST_JOURNAL_H
