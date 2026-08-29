cmake_minimum_required(VERSION 3.16)
project(aibaby CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# --- Core: portable, freestanding-ish, no host dependencies ----------------
# The flags are the portability contract, not a preference. If the core ever
# stops building with -fno-exceptions -fno-rtti, it has grown a host dependency
# — and a core that can throw, allocate or reach a syscall is a core whose state
# changes are no longer all visible to the pinned hash.
#
# -ffp-contract=off, and no -ffast-math: determinism (G1) is the only real
# testing lever this project has, and fast-math lets the compiler reassociate
# and contract float arithmetic differently at different optimisation levels.
# A brain that hashes differently under -O2 and -O3 would cost far more than
# the handful of percent the flag buys.
add_library(aibaby_core STATIC
  core/src/dna.cpp
  core/src/network.cpp
  core/src/senses.cpp
  core/src/critic.cpp
  core/src/brain.cpp
  core/src/snapshot.cpp
)
target_include_directories(aibaby_core PUBLIC core/include)
target_compile_options(aibaby_core PRIVATE
  -Wall -Wextra -Wpedantic
  -fno-exceptions -fno-rtti
  -ffp-contract=off
)

# --- Host: desktop only. STL, sockets, files all live here ------------------
add_executable(aibaby
  host/src/main.cpp
  host/src/dna_toml.cpp
  host/src/journal.cpp
  host/src/snapshot_file.cpp
  host/src/ws_server.cpp
  host/src/audio.cpp
  host/src/mel.cpp
  host/src/vision.cpp
  host/src/wav.cpp
  host/src/experiments.cpp
  host/src/experiments_milestones.cpp
  host/src/experiments_probes.cpp
)
target_include_directories(aibaby PRIVATE host/include)
target_link_libraries(aibaby PRIVATE aibaby_core)
target_compile_options(aibaby PRIVATE -Wall -Wextra -ffp-contract=off)
