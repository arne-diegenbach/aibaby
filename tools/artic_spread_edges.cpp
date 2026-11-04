// The structural edge cases of `artic_spread`, the separability decomposition
// that `echoname` and `exsweep` share. It existed as two inline copies until
// 2026-11-04; this file is the half of the dedup gate that a short experiment
// run CANNOT give you, because the branches that matter are the ones real rows
// almost never take.
//
//   g++ -std=c++17 -O2 -I . -I core/include -I host/include \
//       -o /tmp/artic_edges tools/artic_spread_edges.cpp && /tmp/artic_edges
//
// WHY IT IS KEPT. The two former copies agreed on every ordinary creature and
// disagreed in exactly one place: when every scored row is identical within its
// word, `within` is 0, the ratio is undefined, and `echoname` published the
// partial decomposition with `sep` 0 while `exsweep` published nothing. That is
// not a theoretical branch -- the taught arm runs at a voiced fraction of 0.082
// and an all-zero articulator row is what silence looks like here. So the
// struct carries BOTH flags (`decomposed` and `ok`) and each caller keeps the
// behaviour it had. Case 5 below is that branch. A bit-equality check on real
// rows passed 51 of 51 cells and every one of them was case 9.
//
// The expected column is the contract, and it is duplicated in the header's own
// comment on purpose: if these two ever disagree, read the code, not either note.
#include "host/src/experiments_common.h"
#include <cstdio>

using aibaby_host::ArticSpread;
using aibaby_host::artic_spread;

static int fails = 0;

static void check(const char* what, const ArticSpread& s, bool dec, bool ok,
                  double mu, double within, double between, double sep) {
  const bool good = s.decomposed == dec && s.ok == ok && s.mu == mu &&
                    s.within == within && s.between == between && s.sep == sep;
  if (!good) ++fails;
  std::printf("  %-28s dec %d ok %d  mu %-6g within %-6g between %-6g sep %-6g  %s\n",
              what, s.decomposed ? 1 : 0, s.ok ? 1 : 0, s.mu, s.within, s.between,
              s.sep, good ? "ok" : "**WRONG**");
}

int main() {
  const uint32_t W = 2;
  // 1-4: the refusals. Nothing is populated, so both callers leave their fields
  // at zero -- which is what the inline copies did when their guards failed.
  check("1 empty", artic_spread({}, {}, W), false, false, 0, 0, 0, 0);
  check("2 words == 0", artic_spread({{1.0}}, {0}, 0), false, false, 0, 0, 0, 0);
  check("3 rows/labels mismatch", artic_spread({{1.0}, {2.0}}, {0}, W), false, false,
        0, 0, 0, 0);
  check("4 word 1 has one repeat", artic_spread({{1.0}, {2.0}, {5.0}}, {0, 0, 1}, W),
        false, false, 0, 0, 0, 0);
  // 5: THE DEGENERATE BRANCH. within == 0 exactly, so `sep` is undefined and
  // `ok` is false -- but mu and between ARE populated and `decomposed` is true.
  check("5 within == 0", artic_spread({{1, 1}, {1, 1}, {3, 3}, {3, 3}}, {0, 0, 1, 1}, W),
        true, false, 2.0, 0.0, 4.0, 0.0);
  // 6-8: a row that must be SKIPPED rather than counted. All three reduce to the
  // same four surviving rows, which is why all three must agree exactly.
  check("6 empty row skipped",
        artic_spread({{1, 1}, {2, 2}, {}, {3, 3}, {4, 4}}, {0, 0, 1, 1, 1}, W), true,
        true, 2.5, 0.5, 4.0, 8.0);
  check("7 ragged row skipped",
        artic_spread({{1, 1}, {2, 2}, {9}, {3, 3}, {4, 4}}, {0, 0, 1, 1, 1}, W), true,
        true, 2.5, 0.5, 4.0, 8.0);
  check("8 label out of range skipped",
        artic_spread({{1, 1}, {2, 2}, {3, 3}, {4, 4}, {9, 9}}, {0, 0, 1, 1, 7}, W), true,
        true, 2.5, 0.5, 4.0, 8.0);
  // 9: the ordinary case every real cell takes. within 0.5, between 16, so the
  // ratio is 32 -- worth writing out because it is easy to misread `sep` as
  // `between` when checking by hand.
  check("9 ordinary", artic_spread({{1, 1}, {2, 2}, {5, 5}, {6, 6}}, {0, 0, 1, 1}, W),
        true, true, 3.5, 0.5, 16.0, 32.0);
  std::printf("\n  %s -- %d of 9 cases wrong\n", fails ? "FAIL" : "PASS", fails);
  return fails ? 1 : 0;
}
