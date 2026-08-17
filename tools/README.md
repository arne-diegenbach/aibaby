# Tooling

Two scripts for editing one field of a genome without touching anything else,
two for the eye port and the panel that displays it, and one pattern for running
a sweep that cannot silently lie to you.

## `eye_wire_test.py` — the eye port, end to end

```sh
tools/eye_wire_test.py          # needs build/aibaby; AIBABY_PORT overrides 8099
```

Starts a real host, speaks the WebSocket handshake and frame format directly,
and drives the whole eye API: steer in pixels and in frame fractions, switch the
mount, feed position reports back, run a closed loop with the harness acting as
the device, then stop answering and check the creature notices. Nineteen checks,
about a minute.

It exists because none of that is reachable from `--experiment`: the experiments
link the retina straight into a probe, so they can prove the port is correct
C++ and cannot prove a single byte of the protocol on top of it. See §6.3 in the
requirements for what else lives on that seam.

Three things it is built around, each of which has made a working feature look
broken:

- **Read the *latest* telemetry frame, not the next one.** The host broadcasts
  at 30 Hz whether or not anyone is reading, so there is a backlog in front of
  whatever you just did.
- **Clear `contrast_floor` with the test stimulus.** A 5 px disc at 220/110 on a
  64 px frame reads 0.036 against a floor of 0.06, so the controller correctly
  refuses to chase it — and a controller that is right to do nothing looks
  exactly like a port that is broken.
- **Keep test movement inside the fovea.** Swinging the toy out into the rings
  reproduces the known peripheral-acquisition failure instead of whatever you
  were trying to measure: no command is issued, so anything timed against a
  command reads zero.

## `eye_panel_test.py` — does the panel draw where the eye is looking?

```sh
tools/eye_panel_test.py         # needs google-chrome and build/aibaby
```

Real Chrome, headless, driven over CDP: loads the panel, moves the eye from a
second socket, and asserts both the readout text and the **pixels** — it counts
strong blue on the retina canvas and checks the centroid lands where the eye is,
rather than trusting that a draw call was made. Nothing is streaming during the
test, so every cell is drawn at alpha 0.04 and the crosshair is the only bright
thing on the canvas, which is what makes that measurable at all.

`--virtual-time-budget` with `--dump-dom` is flaky here; CDP is not. And if you
extend this to anything held, dragged or hovered, use `Input.dispatchMouseEvent`
at real coordinates — `el.click()` and synthetic `MouseEvent`s reach your
handlers but bypass hit testing, which is how push-to-talk stayed broken for a
week with the harness reporting PASS.

```sh
tools/genome_set_module.py     dna/default.toml out.toml central:norm_gain=1.0 vision:n_max=256
tools/genome_set_projection.py dna/default.toml out.toml "vision->central:density=0.12"
```

Both **assert that every requested edit matched exactly one line** and fail
loudly otherwise. That is the whole point of them. A sweep whose `sed` quietly
matches nothing measures the same genome N times and draws a flat line, and
this project has produced that flat line more than once — the first
`sensitivity` sweep did it because of a trailing comment, and a later one did it
because an anchor's indentation was off by two spaces.

## The sweep pattern

```sh
for v in 0.03 0.06 0.10; do
  tools/genome_set_projection.py dna/default.toml /tmp/v-$v.toml "central->vocal:density=$v" || exit 1

  # Never measure a brain the genome does not describe.
  if ./build/aibaby --dna /tmp/v-$v.toml --experiment babble --ticks 2000 2>&1 | grep -qi WARNING; then
    echo "dropped synapses at $v — aborting"; exit 1
  fi

  ./build/aibaby --dna /tmp/v-$v.toml --experiment g3probe --ticks 600000 > out-$v.txt 2>&1
done
```

**Never rebuild while a sweep is running.** A running process keeps the binary it
started with, but the *next* iteration of the loop picks up whatever is on disk
now — so a `cmake --build` half way through a two-arm comparison silently runs
the arms on different builds, and the difference between them is then partly your
edit and partly the creature. Nothing warns about this and the output looks
completely normal. Finish the sweep, then build.

Three things that pattern encodes, each of which cost a rerun to learn:

- **Run the whole sweep sequentially inside one background command.** A `&`
  inside a backgrounded wrapper dies with the wrapper and the runs are lost.
- **Check for dropped synapses with `babble`, not `audio`.** `audio` does not
  print that warning at all, so it reports nothing and the silence reads as an
  all-clear. Raising a projection's density can overflow the *target* module's
  `max_out_degree`, because the reverse index shares the per-neuron slicing.
- **Never change `n_max` inside a sweep.** It sets each module's global neuron
  index base, so editing it re-rolls the wiring of everything downstream —
  a nuisance shift that has been larger than the effect being measured.
  `max_out_degree` is free: raise it whenever synapses are being dropped.

## Recalibrating after a genome edit

Any edit that changes a module's drive changes its free-running rate, and a
`target_rate_hz` that no longer matches means intrinsic plasticity spends the
whole session hauling the module somewhere. `calibrate` prints which modules are
stale and what they actually run at; rewrite those targets and repeat until it
passes.

Two ways that loop misbehaves, both real:

- A module with `norm_gain > 0` has its **normalisation reference tied to
  `target_rate_hz`**, so the rate you measure depends on the target you are
  setting. It is a fixed-point problem, and for a drive-starved module it
  diverges toward zero rather than converging.
- A module inside a **positive feedback loop** diverges upward the same way:
  excitatory top-down feedback took central 8.48 → 17.48 Hz over five rounds and
  was still climbing. That is a verdict on the configuration, not a calibration
  failure.
