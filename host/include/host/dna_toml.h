// Compiles the readable genome (TOML) into the core's binary DNA blob.
//
// This lives host-side on purpose: the core must never need a text parser.
// On a microcontroller the compiled blob is flashed and the core mmaps it.

#ifndef AIBABY_HOST_DNA_TOML_H
#define AIBABY_HOST_DNA_TOML_H

#include <cstdint>
#include <string>
#include <vector>

namespace aibaby_host {

// Returns false and fills `error` on any parse or validation failure.
bool compile_dna_toml(const std::string& path, std::vector<uint8_t>& out,
                      std::string& error);

}  // namespace aibaby_host

#endif  // AIBABY_HOST_DNA_TOML_H
