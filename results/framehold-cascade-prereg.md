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
