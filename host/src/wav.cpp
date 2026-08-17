#include "host/wav.h"

#include <cstring>
#include <fstream>

namespace aibaby_host {
namespace {

void put_u32(std::vector<uint8_t>& out, uint32_t v) {
  out.push_back(uint8_t(v));
  out.push_back(uint8_t(v >> 8));
  out.push_back(uint8_t(v >> 16));
  out.push_back(uint8_t(v >> 24));
}

void put_u16(std::vector<uint8_t>& out, uint16_t v) {
  out.push_back(uint8_t(v));
  out.push_back(uint8_t(v >> 8));
}

void put_tag(std::vector<uint8_t>& out, const char* tag) {
  for (int i = 0; i < 4; ++i) out.push_back(uint8_t(tag[i]));
}

}  // namespace

bool write_wav(const std::string& path, const std::vector<float>& samples,
               uint32_t channels, uint32_t sample_rate, std::string& error) {
  if (channels == 0) {
    error = "wav: zero channels";
    return false;
  }
  const uint32_t frames = uint32_t(samples.size() / channels);
  const uint32_t data_bytes = frames * channels * 2;

  std::vector<uint8_t> header;
  header.reserve(44);
  put_tag(header, "RIFF");
  put_u32(header, 36 + data_bytes);
  put_tag(header, "WAVE");
  put_tag(header, "fmt ");
  put_u32(header, 16);                            // PCM chunk size
  put_u16(header, 1);                             // PCM
  put_u16(header, uint16_t(channels));
  put_u32(header, sample_rate);
  put_u32(header, sample_rate * channels * 2);    // byte rate
  put_u16(header, uint16_t(channels * 2));        // block align
  put_u16(header, 16);                            // bits per sample
  put_tag(header, "data");
  put_u32(header, data_bytes);

  std::vector<uint8_t> pcm(data_bytes);
  for (uint32_t i = 0; i < frames * channels; ++i) {
    float v = samples[i];
    if (v > 1.0f) v = 1.0f;
    if (v < -1.0f) v = -1.0f;
    const int32_t q = int32_t(v * 32767.0f);
    pcm[i * 2] = uint8_t(q & 0xff);
    pcm[i * 2 + 1] = uint8_t((q >> 8) & 0xff);
  }

  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  if (!file) {
    error = "cannot open wav for writing: " + path;
    return false;
  }
  file.write(reinterpret_cast<const char*>(header.data()), long(header.size()));
  file.write(reinterpret_cast<const char*>(pcm.data()), long(pcm.size()));
  file.flush();
  if (!file) {
    error = "wav write failed: " + path;
    return false;
  }
  return true;
}

}  // namespace aibaby_host
