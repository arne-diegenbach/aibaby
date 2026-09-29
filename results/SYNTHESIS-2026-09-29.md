# What the creature does, and the one thing that binds it

Written 2026-09-29, after the ring line closed. Every number here is from a
recorded run; where two numbers disagree the disagreement is stated.

## THE CHAIN, STAGE BY STAGE, WITH ITS MEASURED NUMBER

    stage                        measurement                          verdict
    1  hearing                   ear one-of-eight 0.981 / 1.000       NOT the limit
    2  representation            a-vs-i context separation 0.999      NOT the limit
    3  conditionality            directional naming 0.82, +6.0 SE     NOT the limit
    4  production RANGE          centroid free-runs 0.068 .. 0.880    NOT the limit
    5  production AIM            teachable centroid swing +-0.063     <<< THE LIMIT
    6  delivered F1              82.5 Hz vs 95 Hz predicted           the same limit
    7  what naming needs         ~230 Hz absolute                     2.8x short

Stage 1 is settled by `coderprobe`: the auditory module carries one-of-eight at
**0.981 against the signal's own 1.000**, so there is no headroom for a better
auditory code and `vocab`'s 0.371 is a fact about what the creature can SAY.

Stages 2 and 3 are settled positively. The creature derives its own context, and
under a shuffled order a-vs-i separation is 0.999. Directional naming — does the
voice move the RIGHT WAY for the word it heard — is MET at 0.82, 6.0 SE over a
matched-marginal control, on two seed families.

## STAGE 4 AND 5 ARE THE SAME ORGAN AND THEY DISAGREE, WHICH IS THE WHOLE STORY

The centroid **free-runs across 0.068 to 0.880** — nearly its entire range. The
creature can already put its tongue anywhere. But the swing a LESSON can install
is **+-0.063**, and since `lerp` spans 750 Hz that is 95 Hz of delivered F1,
measured at 82.5.

**So the creature has reach and no aim.** It is not short of range, it is short of
the ability to CHOOSE a point in the range and hold it. Two independent results
say the same thing from opposite directions:

  - `vowel-space`: the centroid moves but **never dwells**, and the 800 ms
    articulator inertia throws away 4.2x of the movement it does make;
  - DNA v62, the velocity decoder: delivered F1 excursion **275.7 Hz vs 82.5**,
    breaking the identity bound outright — and naming still REFUSED, conditional
    gap +18.9 +/- 40.1 while its matched control tripled to 66.5 Hz, which is the
    decoder's own wander. **More reach, no more aim.**

That is why fourteen routes at the naming ceiling all returned x1.1, and why
`ceiling-is-an-exponent` reads dF1 ~ aligned^0.61: moving delivered F1 by 1.68x
needs the aligned bias to move 2.34x, and every knob gives 1.1x.

## AIM REQUIRES AN ERROR SIGNAL, AND THE CREATURE USES NONE

Aiming a continuous articulator means comparing what you produced against what you
meant and correcting. That is DIVA's forward model, and `critic-is-not-a-forward-
model` refused the cheap version here. `selfloop` then measured whether the
creature uses auditory feedback to control F1 at all: altered auditory feedback
gives a coherent response of **+0.01 Hz +/- 0.23** — a dead null, every phase lag
indistinguishable from control. It also explains v62's free drift.

**AND THE RING LINE JUST PROVED A FEEDBACK LOOP EXISTS AND WORKS — ON THE WRONG
VARIABLE.** The 3 Hz ring is a genuine closed loop through the creature's own ear,
with two lags whose product sets its frequency; a derived 86 ms delay relocated it
to three separately predicted frequencies with 12/12 seeds agreeing. The machinery
for sensorimotor feedback is present, closed, and quantitatively understood. **It
carries amplitude and not F1.**

## SO THE BINDING CONSTRAINT, IN ONE SENTENCE

**The creature can produce any vowel and cannot aim at one, because the only
sensorimotor loop it owns carries loudness rather than formants.**

## AND THE NEXT QUESTION IS CHEAP AND HAS NEVER BEEN ASKED

`selfloop` measured the creature's RESPONSE to altered feedback and found nothing.
It did not measure whether the SIGNAL is there. Those imply completely different
things:

  - **if the ear does not encode the creature's own F1**, no learning rule can
    build an F1 controller, `selfloop`'s null is structural, and the fix is
    upstream — the creature is too quiet to itself, or its own voice is masked;
  - **if the ear does encode it**, the signal is present and unused, the null
    means the MAPPING is missing, and "can reward install the loop" — filed as the
    one untested move in this family — becomes worth its cost.

The probe: decode the creature's OWN produced F1 from its auditory population,
cross-validated, against three controls — a time-shuffled pairing (chance), the
same decode of the CAREGIVER's F1 (positive control, known good at 0.981), and a
deaf arm at `self_gain` 0 (the structural null, must read chance).

**Refusal-first, and it gates an expensive build rather than following one.**

---

## ANSWERED THE SAME DAY: THE SIGNAL IS THERE

`selfcode`, two independent wiring families, n=18 each:

    self                0.623 +/- 0.017   0.625 +/- 0.016
    fixed F1 (matched)  0.500 +/- 0.013   0.511 +/- 0.012   <- exactly chance
    caregiver (pos ctl) 0.982 +/- 0.006   0.959 +/- 0.016
    self - fixedF1      +0.124 +/- 0.020  +0.114 +/- 0.021   (6.2 / 5.4 SE)

The point estimate HELD as the sample tripled (+0.111 at n=6), which is the
stopping condition this project uses.

**So the ear encodes the creature's own F1, and the creature does not use it.**
`selfloop`'s null is a MISSING MAPPING rather than a missing signal, and the fix is
not upstream. The one untested move in the vocal family — whether reward can
INSTALL the loop — is now the live question, and it has a carrier to run on.

**The bar for it, set before the run:** a present signal is a carrier, not a
behaviour. `credgate` converted a PERFECT index into +4.8 SE and no more; `ctxfour`
holds four distinctions but not four targets. A loop that improves AIM has to show
up as reduced F1 variance around a taught target, not as a bigger excursion —
excursion is what v62 already bought and it did not become naming.
