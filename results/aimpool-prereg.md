# aimpool — is the F1 backlash PER-NEURON regulation, or regulation at all?

Written 2026-09-29, BEFORE any pooled arm ran. Three arms of one experiment
(`--experiment aimgain`) on three genomes differing in ONE vocal field each.

## The question, and the correction that produced it

`aimgain` measured 107.1 Hz of hysteresis in the F1 actuator with the larynx's
homeostat on and 2.0 Hz with it off (`results/aimgain-final.log`,
`results/aimgain-ip00.log`). I wrote up that result naming a **group-mean
homeostat** as "a mechanism nobody has built", carrying `stageprobe`'s
independent reason for it: a per-neuron rate homeostat drives every neuron toward
the same rate, so it flattens the TILT the centroid readout reads, while a
homeostat on the group's mean would hold the common mode and leave the tilt
intact.

**That was wrong, and the mechanism exists.** It is DNA v57, `ip_pool`, and it
has been in the genome since. `stageprobe`'s "that mechanism does not exist here"
was written on 2026-09-12 and v57 built it the same day; `ippool` then measured it
and found **-15.6 +/- 17.5 Hz of dF1 (-0.90 SE, 8/18)** with the learned bias
nearly halved. So the group-mean homeostat is built and is refused ON THE NAMING
CEILING.

**What `ippool` did NOT measure is hysteresis**, and that is a different quantity
from dF1. So the live question is narrow and cheap:

> Does pooling the homeostat remove the actuator's backlash, or is the backlash
> rate regulation as such?

`ipoff` prices the alternative: turning the homeostat off collapses vocal learning
(dF1 103.9 -> 17.7, -7.42 SE). So "just turn it off" is not available, and a
pooled homeostat that kept regulation while dropping the backlash would be the
only setting between the horns of the trade `stageprobe` named.

## The arms

| arm | genome | vocal field |
|---|---|---|
| per-neuron (shipped) | `dna/default.toml` | `ip_pool = 0` |
| POOLED | `results/aimgain-ippool9.toml` | `ip_pool = 9` |
| off | `results/aimgain-ip00.toml` | `ip_wake_scale = 0.0` |

`ip_pool = 9` gives one slice per vocal readout group. Vocal grows to 126
neurons and 126 = 9 x 14, so `(k * 9) / count` and `slice_begin(count, 9, g)`
partition identically with no boundary neuron regulated against its neighbour's
mean — an alignment that is exact here and would NOT be for a count indivisible
by 9.

All three arms are re-run with the same binary, because the primary statistic is
new: **hysteresis PAIRED per creature with an SE**. Ascending rep r and
descending rep r are the same seed, and the recorded 107.1 and 2.0 are
differences of means. A 54x gap survives that; a verdict on an intermediate
value would not.

## Pre-registered outcomes, with the cuts placed now

Primary: paired hysteresis of the POOLED arm, H, reported as its position on the
axis the other two arms span, `f = (H - H_off) / (H_neuron - H_off)`, measured on
the same binary in the same run set (not against the recorded 107.1 / 2.0).

- **f <= 0.25 — POOLING REMOVES THE BACKLASH.** Granularity is the cause. This is
  the first setting between the horns of the trade, and it makes the closed-loop
  question worth re-opening on a pooled larynx, because `aimhold`'s refusal was
  measured on a plant with 107 Hz of backlash.
- **f >= 0.65 — THE BACKLASH IS REGULATION AS SUCH.** Pooling is refused on this
  axis too, `aimhold`'s refusal stands on the only plant available, and the
  homeostat's granularity has now been refused on dF1 (`ippool`) and on backlash
  (here).
- **0.25 < f < 0.65 — PARTIAL, reported as a quantity with NO verdict.** A cut
  placed afterwards inside this band would be a threshold placed where the data
  is.

## The guard, and it is the one that matters

**A DEAD ACTUATOR HAS NO HYSTERESIS.** Zero backlash is exactly what a larynx
that cannot move F1 at all reads, so the primary is void unless the pooled arm
still has an actuator:

- **static swing >= 150 Hz in EACH direction** (shipped reads +186.1 / -192.2,
  off reads +222.5 / -232.4, so 150 sits below both known arms with margin).
- **rest F1 at k = 0 within 100 Hz of 629.7** (shipped) — a large shift means the
  operating point moved and the two curves are not the same plant sampled twice.

If either fails, the run reports only "pooling weakened the actuator" and the
hysteresis number is NOT interpretable in either direction.

## What this cannot say

It cannot say pooling helps learning — `ippool` already says it does not, at
-0.90 SE on 18 creatures. A backlash result here would be about the PLANT, and
would license re-measuring `aimhold` on a pooled larynx, nothing further.

---

# OUTCOME, appended after the run. Nothing above this line was edited.

**f = +0.03** (pooled 7.0 +/- 0.7, off 3.6 +/- 0.5, per-neuron 107.1 +/- 1.7, all
paired per creature on this binary). Under the 0.25 cut: **POOLING REMOVES THE
BACKLASH.** Guard passes in the direction that cannot be mistaken for a null --
pooled swing +226.8 / -239.8 is WIDER than shipped, rest F1 633.4 vs 629.7.

**AND THE "WHAT THIS LICENSES" CLAUSE ABOVE IS WRONG.** It says a pooled plant makes
the closed loop worth re-opening because `aimhold` ran on a 107 Hz-backlash plant.
`aimhold` had an IP-OFF arm all along, measured here at 3.6 Hz of backlash, and its
precision was already not information-specific. I could have known that when I wrote
the clause. Tested rather than argued: `aimhold` on the pooled plant returns the
30 Hz of reach the backlash was eating (36.0 -> 65.5 Hz) and collapses the
interaction (+26.9 -> +1.1 Hz), and precision reads **+0.3 +/- 0.4 against
scrambled** -- refused on the same bar, now on a third plant.

**A pre-registration can get its CONSEQUENCES wrong while getting its cuts and its
guard right.** The cuts and the guard did their job; the licence clause was a
prediction about a different experiment, made without re-reading that experiment's
own arms. Keep those clauses, and check them against the arms that already exist.
