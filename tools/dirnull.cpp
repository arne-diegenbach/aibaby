// Does the naming score read its own chance level on a voice that carries
// NOTHING? It did not, and that is how the four-word off arm came to sit at
// 0.321 against a nominal chance of 0.250.
//
//   g++ -std=c++17 -O2 -I . -I core/include -I host/include \
//       -o /tmp/dirnull tools/dirnull.cpp && /tmp/dirnull
//
// Expected after the fix: ~0.250 on balanced labels AND on skewed ones, because
// `direction_null` is what the experiments actually compare against now. This
// file is the check itself, kept because the reasoning is easy to lose: the
// measure was wrong in a way that was invisible at k=2 by construction.
#include "host/src/experiments_common.h"
#include <cstdio>
#include <random>

int main() {
  using namespace aibaby_host;
  std::vector<std::pair<double,double>> tg;
  for (uint32_t q = 0; q < 4; ++q) tg.push_back({double(kWords[q].f1), double(kWords[q].f2)});

  // magnitudes of the unnormalised target directions the measure uses
  double t1=0,t2=0;
  for (auto&t:tg){t1+=std::log(t.first);t2+=std::log(t.second);}
  t1/=4;t2/=4;
  for (size_t c=0;c<4;++c){
    double a=std::log(tg[c].first)-t1,b=std::log(tg[c].second)-t2;
    std::printf("  target %zu  |d| = %.3f\n", c, std::sqrt(a*a+b*b));
  }

  for (int trial = 0; trial < 3; ++trial) {
    std::mt19937 rng(1234 + trial);
    std::normal_distribution<double> g(0.0, 1.0);
    const int N = 4000;
    std::vector<double> f1(N), f2(N);
    std::vector<int> w(N);
    // A creature babbling around one operating point, label drawn independently.
    for (int i = 0; i < N; ++i) {
      f1[i] = 500.0 * std::exp(0.20 * g(rng));
      f2[i] = 1500.0 * std::exp(0.20 * g(rng));
      w[i] = int(rng() % 4);
    }
    std::printf("  balanced labels, independent voice : %.3f\n",
                direction_accuracy(f1, f2, w, tg));
    // Same, but the label distribution is skewed 40/30/20/10.
    for (int i = 0; i < N; ++i) {
      const uint32_t r = rng() % 10;
      w[i] = r < 4 ? 0 : (r < 7 ? 1 : (r < 9 ? 2 : 3));
    }
    std::printf("  SKEWED labels, independent voice   : %.3f\n",
                direction_accuracy(f1, f2, w, tg));
  }
  return 0;
}
