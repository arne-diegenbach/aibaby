# PRE-REGISTRATION: is the 3 Hz ring the cascade of two lags?

Written 2026-09-28, BEFORE arms B, C and D were run. The two baseline cells
(A and the gate-030 cell) already exist and were run before this hypothesis
was formed -- they are what suggested it, so they are not evidence for it.

## THE HYPOTHESIS

`read_group` reads `net.rate_fast`, whose estimator time constant was hardcoded
at 50 ms (`network.cpp`, `rate_fast_alpha_ = dt_ms / 50`). The larynx then
smooths that output AGAIN at `gate_smoothing_ms` = 60 ms (`senses.cpp`,
`group_activity_ += gate_smooth_ * (...)`). Two cascaded first-order lags sit in
series on the amplitude path, which is exactly the signal `framehold` measures.

If the ring's frequency is set by that pair, it scales as the GEOMETRIC MEAN:

    f ~ 1 / (2*pi*sqrt(tau_rate * tau_gate))

and moving ONE of the two moves f with exponent 0.5, not 1.0.

## WHY THIS IS NOT THE EXPONENT ALREADY REPORTED

`aibaby-three-hertz-resonance` refused `gate_smoothing_ms` as the origin at
exponent 0.42 against the 1.0 a single pole requires. That 0.42 was fitted on
ONE leg of the sweep (60 -> 30). Over the full range:

    tau 120 -> 60   argmax 2 -> 3 Hz    exponent 0.585
    tau  60 -> 30   argmax 3 -> 4 Hz    exponent 0.415
    tau 120 -> 30   argmax 2 -> 4 Hz    exponent 0.500

The legs bracket 0.5, which is what quantising a square-root law onto an integer
grid of arms does. The refusal tested against 1.0 and never against 0.5.

## THE ARMS, AND THE PREDICTION FOR EACH

Primary statistic: the EARLY-window SNR argmax over the seven `heard` arms --
the same column the existing sweep was read on.

    cell   tau_rate  tau_gate   predicted f   predicted argmax   status
    A            50        60       2.91 Hz            3          HAVE (5.49 at h3)
    -            50        30       4.11 Hz            4          HAVE (5.97 at h4)
    B            25        60       4.11 Hz            4          TO RUN
    C           100        30       2.91 Hz            3          TO RUN  <- decisive
    D            25        30       5.81 Hz        5 or 6         TO RUN

C IS THE ARM THAT CARRIES THE DISCRIMINATION. Both of its constants are changed
and each one ALONE moves the argmax to 4; their product is unchanged, so the
cascade predicts it lands back on 3. No account of the form "perturbing the
creature shifts the ring" can produce a return to baseline.

## WHAT REFUSES IT

  - B's argmax stays at 3  ->  the hardcoded lag does not move the ring at all,
    the cascade account is dead, and the ring survives intact.
  - C's argmax is 4        ->  the product law is dead; whatever moves the ring
    is not sqrt(tau_rate * tau_gate).
  - D's argmax is 4 or below -> dead.

Any one of these refuses it. All three must land for it to stand.

## WHAT I WILL NOT QUOTE EVEN IF IT LANDS

The absolute agreement. sqrt(50*60) = 54.8 ms is a corner at 2.906 Hz against a
ring measured at 3 Hz, which is a 3% match and looks like a confirmation. It is
not one: two cascaded low-passes are OVERDAMPED and do not ring, so a visible
ring needs loop gain K around them, and with gain the natural frequency rises as
sqrt(1+K). The clean 2.91 Hz agreement therefore requires K ~ 0 while the ring's
existence requires K >> 1. Those cannot both hold. The SCALING is the testable
claim; the absolute number is a coincidence until a loop is identified, and no
loop is identified -- vocal's `norm_gain` is 0.0, so divisive normalisation, the
one delayed negative feedback whose lag IS `rate_fast`, is OFF in the larynx.

## THE DEFLATIONARY READING, STATED IN ADVANCE

If this lands, the ring's frequency is set by two READOUT filters. That is
consistent with there being no oscillator at all -- the peak would be the shape
of an SNR ratio taken through a low-pass cascade. Every existing defence of the
ring (a deaf creature still rings; the transient fix selectively killed heard-1;
2 and 4 Hz fully tiled yet weakest) attacks a TRANSIENT artefact and leaves a
FILTER-SHAPE artefact untouched. So a positive result here is bad news for
"the first timekeeping structure in this project", and it is being run anyway.

## MULTIPLICITY, carried forward from the entry's own warning

The argmax is a maximum over 7 arms and the grid is integer-spaced, so single
contrasts at +2 to +3 SE in this experiment are scatter. The claim rests on
THREE argmaxes landing where they were predicted, not on any one SE.

---

# ROUND 2, pre-registered 2026-09-28 after round 1 and before these arms ran

Round 1 refused the product law as an EXTRAPOLATION: cell D (25/30, product 750)
did not move the ring to its predicted 5.81 Hz, and at peak/next 1.08 it has no
peak at any location. D moved BOTH constants, so it cannot say whether the death
is about the product or about one constant being too fast.

## THE TWO CELLS, one on each side

    cell   tau_rate  tau_gate  product   predicted   note
    E          50.0      15.0      750     5.81 Hz    gate fast, rate SHIPPED
    F          12.5      60.0      750     5.81 Hz    rate fast, gate SHIPPED

Same product as D, same predicted frequency, opposite splits.

## WHAT EACH OUTCOME MEANS, fixed in advance

  - BOTH flat (peak/next near 1.0, as D)  ->  the death follows the PRODUCT, so
    the ring stops existing once the law predicts a frequency past some ceiling.
    The product law survives as a law about frequency and gains a stated range.
  - BOTH peak at 5 or 6  ->  D was the fluke, not the law; the product law
    extends to 750 and round 1's refusal was a max-of-7 accident. This is the
    outcome that would REVERSE round 1's refusal, and it is why two cells are
    being run rather than one.
  - EXACTLY ONE flat  ->  the death is about a single constant being too fast,
    not about the product. Which one is flat names it, and the product law is
    then narrower than the two matched pairs suggest.

## THE STATISTIC, fixed this time

PRIMARY: peak/next — the highest `heard` early-window SNR divided by the second
highest. Round 1 pre-registered the ARGMAX and that was a mistake: an argmax is
defined on a flat profile and would have scored D as "the ring held at 4". The
argmax is reported second, and only for cells whose peak/next says there is a
peak to locate.

Reference values from round 1, quoted as quantities with no cut applied:
clear peaks 1.91 / 1.95 / 1.98, marginal 1.49, flat 1.08 and 1.13.

## A CORRECTION MADE BEFORE THIS RAN, NOT AFTER

The round-1 write-up said D's predicted 5.81 Hz "exceeds the free-running vocal
rate of ~4.9 Hz". That misreads `adaptclock`: its 4.93 is the `rate` column, the
module's mean FIRING rate, not a rhythm. The free-running voice's spectral peak
is 1.90 +/- 0.25 Hz — BELOW the ring — so it supplies no ceiling at 4-5 Hz.
**The ceiling, if these cells find one, has no independently measured number to
agree with, and nothing here should be made to agree with 4.9.**

---

# ROUND 2 OUTCOME: "BOTH FLAT" — the death follows the PRODUCT

    cell        rate  gate  product  pred Hz   median  arg   seeds peaking there   p
    gate120       50   120     6000     2.05     1.59    2         6/12          0.054 (4-arm grid)
    A shipped     50    60     3000     2.91     2.64    3         7/12          0.0005
    C            100    30     3000     2.91     1.92    3         9/12          <1e-5
    -             50    30     1500     4.11     2.21    4         8/12          0.00005
    B             25    60     1500     4.11     1.08    4         6/12          0.0036
    D             25    30      750     5.81     1.05    4         3/12          0.24
    E             50    15      750     5.81     1.06    5         3/12          0.24
    F           12.5    60      750     5.81     1.15    4         4/12          0.08

"Seeds peaking there" counts how many of the 12 seeds independently put their OWN
maximum on that arm; the null is Binomial(12, 1/7) with expectation 1.7.

**Every cell at product 1500-3000 concentrates (p <= 0.004). All three at product
750 fail (0.24, 0.24, 0.08).** Two of those three keep one constant at its shipped
value, so the death is NOT one constant being too fast -- it follows the product.

**E LOOKED LIKE THE EXCEPTION AND WAS ONE SEED.** Its mean peak/next was 1.62 with
an argmax at the predicted 5 Hz. Per seed, heard-5 reads 28.97 on seed 1 and
0.34-2.82 on the other eleven. Median ratio 1.06, 3/12 seeds. Flat.

## SO THE LAW, WITH THE RANGE IT EARNED

The ring's frequency reads the PRODUCT of the two cascaded constants across
products 6000 -> 1500, an argmax ladder of 2 -> 3 -> 4 Hz against predictions of
2.05 / 2.91 / 4.11. Two MATCHED PAIRS confirm it is the product and not either
constant: at 3000 the constants differ by 2x, at 1500 by 4x, and each pair agrees.
Below 1500 the ring CEASES TO EXIST rather than moving higher. The cut-off sits
between 4.11 Hz (alive) and 5.81 Hz (dead) and has no independently measured
number to match.

## MY PRIMARY STATISTIC FAILED TWICE, DIFFERENTLY EACH TIME

  round 1   ARGMAX -- defined on a flat profile, so it reports a location for a
            peak that is not there. D would have scored "the ring held at 4".
  round 2   PEAK/NEXT ON MEANS -- a ratio of means, which one outlier seed
            manufactures. E scored 1.62 off a single seed's 28.97.

Both failures are the same shape: a summary that is DEFINED whether or not the
effect exists. The fix in both cases is to ask whether the effect is present PER
SEED before asking where it is. The seed-vote count is immune to both and should
have been the primary from the start; it is what the third column above reports.

---

# ROUND 3: NAMING THE LOOP. Pre-registered 2026-09-28, before these arms ran.

## THE ARGUMENT THAT MAKES THIS DECIDABLE

`gate_smoothing_ms` appears in exactly TWO places in the whole system:

  1. it smooths `group_activity_` -> `params_.amplitude`, which goes to the
     synthesiser and therefore INTO THE MEASUREMENT; and
  2. `audio.cpp` renders `v.amplitude * self_gain_` into the cochlea, which is
     the ONLY path by which it re-enters the brain.

Everywhere else the gate smoother is a TERMINAL READOUT. A filter that is not in
a loop cannot set the frequency of an oscillation -- it can only attenuate one.
Yet rounds 1 and 2 showed the ring's frequency tracks it at exponent ~0.5.

So exactly one of these is true, and `self_gain = 0` separates them.

## THE CELLS

    cell  self_gain  gate   status
    A           0.5    60    HAVE -- peak 3 Hz, 7/12 seeds, p 0.0005
    -           0.5    30    HAVE -- peak 4 Hz, 8/12 seeds, p 0.00005
    G           0.0    60    TO RUN
    H           0.0    30    TO RUN

`rate_fast_tau_ms` stays at its shipped 50 in all four: this round moves the
LOOP, not the cascade.

## THE THREE OUTCOMES, NAMED IN ADVANCE

  1. THE DEAF CELLS STILL SHIFT 3 -> 4 WITH THE GATE.
     Then gate_smoothing sets the frequency while provably outside any loop, so
     there is NO LOOP through it and the "ring frequency" is a property of the
     MEASUREMENT CHAIN. This is the artefact outcome and it would close the 3 Hz
     resonance as a structure rather than name its loop.

  2. THE DEAF CELLS BOTH PEAK ON THE SAME ARM, wherever that is.
     Then the frequency is set WITHOUT the gate, i.e. by an INTERNAL loop whose
     lag is `rate_fast`; and self-hearing is what inserts the gate smoother as a
     SECOND lag when it is on. THE LOOP IS THEN NAMED: internal, closed through
     the larynx-ear path only when the creature can hear itself. This is the
     outcome that answers the question asked.

  3. THE DEAF CELLS BOTH SCATTER (3-4 of 12, as product 750 did).
     Then the ring needs self-hearing to exist at all, and the loop IS the
     larynx -> ear -> brain -> larynx path. NOTE THIS WOULD QUALIFY A STANDING
     RESULT: `aibaby-three-hertz-resonance` records "a DEAF creature still rings
     at +3.1 SE", but that was measured AT 3 Hz ONLY -- the self_gain sweep never
     scanned frequency, so it could not see a ring that moved or thinned.

## THE STATISTIC, primary from the start this time

Per-seed vote: how many of the 12 seeds independently put their own maximum on
the same arm, null Binomial(12, 1/7), expectation 1.7. Round 1 pre-registered an
argmax and round 2 a ratio of means, and BOTH were fooled -- an argmax is defined
on a flat profile, a ratio of means is manufactured by one outlier seed. Means
and argmaxes are reported alongside but decide nothing.

Reference: concentrated cells ran 6-9 of 12 (p 0.0036 to <1e-5), scattered cells
3-4 of 12 (p 0.24 to 0.08).

## WHAT IS NOT BEING CLAIMED

Outcome 2 names a loop but does not identify what closes it internally. Round 2
already excluded the obvious internal candidate: vocal's `norm_gain` is 0.0, so
divisive normalisation is off in the larynx.
