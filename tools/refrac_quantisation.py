#!/usr/bin/env python3
"""What per-neuron spread does `refractory_sigma` actually produce, after rounding?

WHY THIS RUNS BEFORE ANY BUILD. v71's threshold ladder (0.00 / 0.05 / 0.15 / 0.30) is the
obvious thing to reuse for a third intrinsic constant, and for `refractory_ms` it may be
VACUOUS. The genome is `refractory_ms = 3.0` with `dt_ms = 1.0`, and the network stores

    refrac_ticks_[i] = uint16_t(refractory_ms * exp(sigma * z) / dt_ms + 0.5)

(`core/src/network.cpp:639`) -- an INTEGER number of ticks, three of them. A log-normal
spread narrow enough to round back to 3 for every neuron changes nothing at all, so the
dose this knob delivers is not its sigma. This script reports the realised lattice
exactly, by integrating the normal over each rounding bin, so the ladder is chosen from
arithmetic instead of copied from the parameter it is being compared against.

THE SECOND COLUMN IS THE ONE THAT MATTERS. A refractory tick is not merely a blocked
spike: `network.cpp:2487` clamps the membrane to rest while `tick_ < refrac_until_`, and
the inbox slot was already drained above it, so the input arriving in that window is
DISCARDED. The quantity a neuron's refractory period actually controls is therefore the
fraction of its input it throws away, which is `rate * refractory_ms`, and at the vocal
module's 5 Hz setpoint three milliseconds is 1.5% of the time. That is the dose, and it is
reported here against the threshold's +/-30% so the comparison is made before the run and
not after it.

Usage: refrac_quantisation.py [rate_hz]
"""
import math
import sys

REFRACTORY_MS = 3.0
DT_MS = 1.0
SIGMAS = [0.05, 0.15, 0.30, 0.60, 1.00]
RATE_HZ = float(sys.argv[1]) if len(sys.argv) > 1 else 5.0


def phi(x):
    """Standard normal CDF."""
    return 0.5 * (1.0 + math.erf(x / math.sqrt(2.0)))


def ticks_for(z, sigma):
    """Exactly what the network computes, including the clamp and the uint16_t truncation."""
    return int(clamped(REFRACTORY_MS * math.exp(sigma * z)) / DT_MS + 0.5)


def clamped(value_ms):
    """The one-tick floor `het_scaled` is called with (lo = dt_ms).

    Without it a wide sigma produces neurons with a ZERO-tick refractory period, which is
    the absence of the mechanism rather than a short version of it. The floor merges what
    would have been the 0 bin into the 1 bin, so the lattice below has no 0 and the 1 bin
    is correspondingly larger -- at sigma 1.00 that is 24% of neurons rather than 4% + 21%.
    """
    return max(DT_MS, value_ms)


def lattice(sigma, kmax=24):
    """P(refrac_ticks == k) for each k, by inverting the rounding bins.

    ticks == k iff the continuous value lands in [k - 0.5, k + 0.5), so the bin edge in
    z-space is z = log((k +/- 0.5) * dt / refractory) / sigma. Exact, not sampled: a
    sampled answer at n=782 neurons would carry its own noise into a pre-flight whose
    whole job is to be arithmetic.
    """
    out = {}
    for k in range(1, kmax + 1):
        lo_val, hi_val = (k - 0.5) * DT_MS, (k + 0.5) * DT_MS
        # The floor sends everything below one tick into k == 1, so that bin's lower edge
        # is -inf rather than its nominal 0.5 ms.
        z_lo = -math.inf if k == 1 else math.log(lo_val / REFRACTORY_MS) / sigma
        z_hi = math.log(hi_val / REFRACTORY_MS) / sigma
        p = phi(z_hi) - phi(z_lo)
        if p > 5e-5:
            out[k] = p
    return out


print(__doc__.split("Usage:")[0].strip())
print("\ngenome: refractory_ms = %.1f, dt_ms = %.1f -> %d ticks for every neuron at sigma 0"
      % (REFRACTORY_MS, DT_MS, ticks_for(0.0, 0.0)))
print("input blackout at %.1f Hz: %d ticks x %.1f ms x %.1f Hz = %.2f%% of input discarded\n"
      % (RATE_HZ, ticks_for(0.0, 0.0), DT_MS, RATE_HZ,
         100.0 * ticks_for(0.0, 0.0) * DT_MS * RATE_HZ / 1000.0))

hdr = "%-7s %9s %7s %7s %9s   %s" % ("sigma", "unchanged", "mean", "CV", "blackout", "lattice")
print(hdr)
print("-" * 92)
for sigma in SIGMAS:
    lat = lattice(sigma)
    tot = sum(lat.values())
    mean = sum(k * p for k, p in lat.items()) / tot
    var = sum((k - mean) ** 2 * p for k, p in lat.items()) / tot
    cv = math.sqrt(var) / mean if mean > 0 else 0.0
    # The spread in the quantity that actually reaches the neuron: the share of its input
    # thrown away, which is what differs between neurons once the operating point is matched.
    bl_lo = min(lat) * DT_MS * RATE_HZ / 1000.0
    bl_hi = max(lat) * DT_MS * RATE_HZ / 1000.0
    shown = " ".join("%d:%.0f%%" % (k, 100.0 * p) for k, p in sorted(lat.items()) if p >= 0.005)
    print("%-7.2f %8.1f%% %7.3f %7.3f  %4.1f-%4.1f%%   %s"
          % (sigma, 100.0 * lat.get(3, 0.0) / tot, mean, cv, 100.0 * bl_lo, 100.0 * bl_hi, shown))

print("\nthe comparison the ladder has to survive")
print("-" * 92)
print("  threshold_sigma 0.30 moves a neuron's threshold by a CONTINUOUS +/-30%,")
print("  and cost 61%% of the lesson. refractory_sigma 0.30 moves %d%% of neurons off"
      % round(100.0 * (1.0 - lattice(0.30).get(3, 0.0))))
print("  the median onto a %d-point lattice, worth %.1f%%-%.1f%% of input discarded"
      % (len([k for k, p in lattice(0.30).items() if p >= 0.005]),
         100.0 * min(lattice(0.30)) * RATE_HZ / 1000.0,
         100.0 * max(lattice(0.30)) * RATE_HZ / 1000.0))
print("  against 1.5% at the median. EQUAL SIGMA IS NOT EQUAL DOSE.")
