// aibaby panel.
//
// Receives one JSON telemetry frame per ~33 ms plus binary batches of motor
// parameters, and sends the microphone back as binary PCM and the camera as
// binary grayscale. The raster scrolls one pixel column per frame, so the
// visible window is about sixteen seconds of brain.

(function () {
  'use strict';

  var MODULE_COLORS = [
    [94, 200, 242],   // central
    [127, 216, 143],  // auditory
    [242, 163, 94],   // vision
    [199, 146, 234],  // somato
    [242, 121, 168],  // vocal
    [232, 212, 122],  // expression
  ];

  var FRAME_AUDIO = 1;   // browser -> host
  var FRAME_VIDEO = 2;   // browser -> host
  var FRAME_VOCAL = 1;   // host -> browser
  var VOCAL_FLOATS = 9;
  var SAMPLE_RATE = 16000;

  var $ = function (id) { return document.getElementById(id); };

  // A 2d context, or null if that canvas is not in the page. Reaching straight
  // through `$('x').getContext(...)` at load time makes every canvas a single
  // point of failure for the whole panel: one missing element throws before
  // the socket is even opened, and the symptom is not "the retina is missing",
  // it is a dead page where nothing works — no telemetry, no microphone, no
  // buttons. Nothing here is important enough to take the rest down with it.
  var ctx2d = function (canvas) { return canvas ? canvas.getContext('2d') : null; };

  // Write a readout, if the page has one. render() is a single chain: an
  // absent <span> a third of the way down used to throw and take every canvas
  // below it with it, so a missing number in the stats table presented itself
  // as a dead ears display.
  var text = function (id, value) {
    var el = $(id);
    if (el) el.textContent = value;
    return el;
  };

  // Wire a handler only if the element exists. Same reasoning as ctx2d: no
  // single control is worth taking the rest of the panel down for.
  var on = function (id, event, fn) {
    var el = $(id);
    if (el) el.addEventListener(event, fn);
    return el;
  };

  var statusEl = $('status');

  // A panel that fails silently is worse than one that fails loudly. Every bug
  // found in this UI so far presented as "feature X stopped working" with no
  // indication that anything had thrown — and a single exception at load time
  // stops the whole script, so the symptom lands somewhere unrelated to the
  // cause: an unwired camera button reads as a broken microphone. Installed
  // first, before anything that could throw.
  window.addEventListener('error', function (e) {
    if (!statusEl) return;
    statusEl.textContent = 'panel error: ' + (e.message || 'unknown') +
      ' (' + (e.filename || '?').replace(/^.*\//, '') + ':' + (e.lineno || '?') + ')';
    statusEl.className = '';
  });
  window.addEventListener('unhandledrejection', function (e) {
    if (!statusEl) return;
    var why = e.reason && (e.reason.message || e.reason.name) || e.reason;
    statusEl.textContent = 'panel error: ' + why;
    statusEl.className = '';
  });

  var legendEl = $('legend');
  var raster = $('raster'), rasterCtx = ctx2d(raster);
  var trace = $('trace'), traceCtx = ctx2d(trace);
  var melCanvas = $('mel'), melCtx = ctx2d(melCanvas);
  var retinaCanvas = $('retina'), retinaCtx = ctx2d(retinaCanvas);
  var formantCanvas = $('formants'), formantCtx = ctx2d(formantCanvas);
  var rewardCanvas = $('reward'), rewardCtx = ctx2d(rewardCanvas);
  var avatarCanvas = $('avatar'), avatarCtx = ctx2d(avatarCanvas);

  var layout = null;
  var lastFrame = null;
  var socket = null;
  var voice = new Voice();
  var speakerOn = false;
  var rewardHistory = [];
  var vocalCursor = 0;   // AudioContext time the next motor frame belongs at

  // --- canvas plumbing ------------------------------------------------------

  function fit(canvas) {
    if (!canvas) return false;
    var ratio = window.devicePixelRatio || 1;
    var width = canvas.clientWidth;
    if (!width) return false;
    var changed = canvas.width !== Math.floor(width * ratio);
    if (changed) {
      canvas.width = Math.floor(width * ratio);
      canvas.height = Math.floor(canvas.getAttribute('height') * ratio);
      canvas.getContext('2d').setTransform(ratio, 0, 0, ratio, 0, 0);
    }
    return changed;
  }

  function clear(ctx, canvas) {
    var w = canvas.clientWidth;
    var h = parseInt(canvas.getAttribute('height'), 10);
    ctx.clearRect(0, 0, w, h);
    ctx.fillStyle = '#0a0d12';
    ctx.fillRect(0, 0, w, h);
    return { w: w, h: h };
  }

  // --- raster ---------------------------------------------------------------

  function buildLayout(modules) {
    if (!raster) return null;
    var bins = modules[0].bins.length;
    var height = parseInt(raster.getAttribute('height'), 10);
    var gutter = 4;
    var available = height - gutter * (modules.length - 1);
    var perModule = Math.floor(available / modules.length);
    var rows = [];
    var y = 0;
    for (var i = 0; i < modules.length; i++) {
      rows.push({ name: modules[i].name, top: y, height: perModule, bins: bins });
      y += perModule + gutter;
    }
    return { rows: rows, bins: bins };
  }

  function renderLegend(modules) {
    if (!legendEl) return;
    legendEl.innerHTML = '';
    modules.forEach(function (m, i) {
      var c = MODULE_COLORS[i % MODULE_COLORS.length];
      var span = document.createElement('span');
      var sw = document.createElement('span');
      sw.className = 'swatch';
      sw.style.background = 'rgb(' + c[0] + ',' + c[1] + ',' + c[2] + ')';
      span.appendChild(sw);
      span.appendChild(document.createTextNode(m.name + ' (' + m.n + ')'));
      legendEl.appendChild(span);
    });
  }

  function drawRasterColumn(modules) {
    if (!rasterCtx || !layout) return;
    var w = raster.clientWidth;
    var h = parseInt(raster.getAttribute('height'), 10);

    rasterCtx.drawImage(raster, -1, 0, w, h);
    rasterCtx.fillStyle = '#0a0d12';
    rasterCtx.fillRect(w - 1, 0, 1, h);

    for (var i = 0; i < modules.length; i++) {
      var row = layout.rows[i];
      if (!row) continue;
      var color = MODULE_COLORS[i % MODULE_COLORS.length];
      var bins = modules[i].bins;
      var binHeight = row.height / bins.length;

      // Normalise per module per frame: absolute spike counts differ a lot
      // between a 400-neuron module and a 64-neuron one, and we care about
      // where the activity is, not how big the module happens to be.
      var peak = 1, b;
      for (b = 0; b < bins.length; b++) if (bins[b] > peak) peak = bins[b];

      for (b = 0; b < bins.length; b++) {
        if (bins[b] === 0) continue;
        var alpha = 0.15 + 0.85 * (bins[b] / peak);
        rasterCtx.fillStyle =
          'rgba(' + color[0] + ',' + color[1] + ',' + color[2] + ',' + alpha + ')';
        rasterCtx.fillRect(w - 1, row.top + b * binHeight, 1, Math.max(1, binHeight));
      }
    }
  }

  // --- traces ---------------------------------------------------------------

  function drawTrace(values, threshold) {
    if (!traceCtx) return;
    var d = clear(traceCtx, trace);
    if (!values || values.length < 2) return;

    var thr = threshold > 0 ? threshold : 1.0;
    var pad = 10, top = pad, bottom = d.h - pad;
    // Scaled to the threshold rather than to constants, so the window still
    // frames the trace after intrinsic plasticity has moved the threshold, and
    // still holds the drawn spike. It reaches much further below rest than
    // above it because inhibition is the stronger swing: the membrane spends a
    // quarter of its time under -0.35, which the old fixed window drew off the
    // bottom edge of the canvas.
    var vMin = -2.0 * thr, vMax = 1.3 * thr;
    var toY = function (v) { return bottom - ((v - vMin) / (vMax - vMin)) * (bottom - top); };

    traceCtx.strokeStyle = '#3a4453';
    traceCtx.setLineDash([3, 3]);
    traceCtx.beginPath();
    traceCtx.moveTo(0, toY(thr));
    traceCtx.lineTo(d.w, toY(thr));
    traceCtx.stroke();
    traceCtx.setLineDash([]);

    traceCtx.strokeStyle = '#5ec8f2';
    traceCtx.lineWidth = 1.25;
    traceCtx.beginPath();
    for (var i = 0; i < values.length; i++) {
      var x = (i / (values.length - 1)) * d.w;
      var y = toY(values[i]);
      if (i === 0) traceCtx.moveTo(x, y); else traceCtx.lineTo(x, y);
    }
    traceCtx.stroke();
  }

  function drawMel(mel) {
    if (!melCtx) return;
    var d = clear(melCtx, melCanvas);
    if (!mel || !mel.length) return;
    var barW = d.w / mel.length;
    for (var i = 0; i < mel.length; i++) {
      var v = Math.max(0, Math.min(1, mel[i]));
      var height = v * (d.h - 8);
      melCtx.fillStyle = 'rgba(127,216,143,' + (0.25 + 0.75 * v) + ')';
      melCtx.fillRect(i * barW + 0.5, d.h - 4 - height, Math.max(1, barW - 1), height);
    }
    melCtx.fillStyle = '#4a5563';
    melCtx.font = '9px ui-monospace, monospace';
    melCtx.fillText('low', 2, d.h - 1);
    melCtx.fillText('high', d.w - 22, d.h - 1);
  }

  // The retina view. Cells are laid out from the shape the host sends rather
  // than from constants here, but the *enumeration* — fovea row-major, then
  // each ring row-major with its middle skipped — has to match the sampler in
  // host/src/vision.cpp. If the two ever disagree the picture comes out
  // scrambled, which is at least a symptom you cannot miss.
  function retinaLayout(shape) {
    var cells = [];
    var centre = shape.frame / 2;
    var pitch = shape.fovea / shape.foveaGrid;
    var origin = centre - shape.fovea / 2;
    var gx, gy;
    for (gy = 0; gy < shape.foveaGrid; gy++) {
      for (gx = 0; gx < shape.foveaGrid; gx++) {
        cells.push({ x: origin + gx * pitch, y: origin + gy * pitch, size: pitch });
      }
    }
    var hole = shape.ringGrid / 2;
    var lo = (shape.ringGrid - hole) / 2, hi = lo + hole;
    for (var r = 1; r <= shape.rings; r++) {
      var win = shape.fovea * Math.pow(2, r);
      var p = win / shape.ringGrid;
      var o = centre - win / 2;
      for (gy = 0; gy < shape.ringGrid; gy++) {
        for (gx = 0; gx < shape.ringGrid; gx++) {
          if (gx >= lo && gx < hi && gy >= lo && gy < hi) continue;
          cells.push({ x: o + gx * p, y: o + gy * p, size: p });
        }
      }
    }
    return cells;
  }

  var retinaCells = null;

  function drawRetina(values, shape, gaze) {
    if (!retinaCtx) return;
    var d = clear(retinaCtx, retinaCanvas);
    if (!values || !values.length || !shape) return;
    if (!retinaCells || retinaCells.length !== values.length) {
      retinaCells = retinaLayout(shape);
    }
    // Square, centred: the field of view is square, and stretching it to the
    // card would put the fovea somewhere other than the middle.
    var side = Math.min(d.w, d.h) - 6;
    var ox = (d.w - side) / 2, oy = (d.h - side) / 2;
    var scale = side / shape.frame;

    retinaCtx.strokeStyle = '#1b222c';
    for (var i = 0; i < retinaCells.length && i < values.length; i++) {
      var c = retinaCells[i];
      var v = Math.max(-1, Math.min(1, values[i]));
      var x = ox + c.x * scale, y = oy + c.y * scale, s = c.size * scale;
      // Warm for ON (light on a dark surround), cool for OFF. A cell with
      // nothing to say is nearly invisible, which is the point: an empty field
      // should look empty.
      var mag = Math.abs(v);
      var col = v >= 0 ? '242,163,94' : '94,200,242';
      retinaCtx.fillStyle = 'rgba(' + col + ',' + (0.04 + 0.9 * mag) + ')';
      retinaCtx.fillRect(x, y, Math.max(1, s - 0.5), Math.max(1, s - 0.5));
    }
    // The fovea boundary, so the acuity gradient is visible rather than implied.
    var fo = (shape.frame - shape.fovea) / 2;
    retinaCtx.strokeStyle = '#3a4453';
    retinaCtx.setLineDash([2, 2]);
    retinaCtx.strokeRect(ox + fo * scale, oy + fo * scale,
                         shape.fovea * scale, shape.fovea * scale);
    retinaCtx.setLineDash([]);

    // Where the eye is pointing, in the frame the camera sent. Drawn over the
    // cells rather than beside them: the whole retinal layout slides with the
    // gaze, so a number in a table cannot say what a mark on the picture says.
    if (!gaze) return;
    var half = shape.frame / 2;
    var mark = function (px, py) {
      return { x: ox + (half + px) * scale, y: oy + (half + py) * scale };
    };
    // The command first, so the actual position draws on top of it. They sit on
    // each other on an ideal eye and separate on a slow one, which is the
    // clearest thing this display can show about a motor.
    var far = Math.abs(gaze.cx - gaze.x) > 0.75 || Math.abs(gaze.cy - gaze.y) > 0.75;
    if (far) {
      var c = mark(gaze.cx, gaze.cy);
      retinaCtx.strokeStyle = 'rgba(242,163,94,0.45)';
      retinaCtx.beginPath();
      retinaCtx.arc(c.x, c.y, 4, 0, Math.PI * 2);
      retinaCtx.stroke();
    }
    var g = mark(gaze.x, gaze.y);
    // Red once the device has stopped answering: the controller freezes there,
    // so the crosshair is telling the truth by not moving and the colour is the
    // only thing that can say the difference between that and nothing to look at.
    retinaCtx.strokeStyle = gaze.stale ? 'rgba(242,94,94,0.9)' : 'rgba(94,200,242,0.9)';
    retinaCtx.beginPath();
    retinaCtx.moveTo(g.x - 6, g.y);
    retinaCtx.lineTo(g.x + 6, g.y);
    retinaCtx.moveTo(g.x, g.y - 6);
    retinaCtx.lineTo(g.x, g.y + 6);
    retinaCtx.stroke();
  }

  // The vowel plane. Where a sound sits in F1/F2 is what makes it that vowel
  // rather than another one, so this is the plot that will matter for M3.
  function drawFormants(v) {
    if (!formantCtx) return;
    var d = clear(formantCtx, formantCanvas);
    if (!v) return;
    var pad = 16;
    var f1Range = [200, 1100], f2Range = [600, 2800];
    var x = pad + ((v.f2 - f2Range[0]) / (f2Range[1] - f2Range[0])) * (d.w - 2 * pad);
    var y = pad + ((v.f1 - f1Range[0]) / (f1Range[1] - f1Range[0])) * (d.h - 2 * pad);
    x = Math.max(pad, Math.min(d.w - pad, x));
    y = Math.max(pad, Math.min(d.h - pad, y));

    formantCtx.strokeStyle = '#232b36';
    formantCtx.strokeRect(pad, pad, d.w - 2 * pad, d.h - 2 * pad);
    formantCtx.fillStyle = '#4a5563';
    formantCtx.font = '9px ui-monospace, monospace';
    formantCtx.fillText('F2 →', d.w - 34, d.h - 4);
    formantCtx.fillText('F1 ↓', 2, 10);

    var voiced = v.voicing > 0.5;
    var r = 3 + 9 * Math.max(0, Math.min(1, v.amp));
    formantCtx.beginPath();
    formantCtx.arc(x, y, r, 0, Math.PI * 2);
    formantCtx.fillStyle = voiced ? 'rgba(242,121,168,0.75)' : 'rgba(125,135,148,0.35)';
    formantCtx.fill();
  }

  function drawReward(r) {
    if (!rewardCtx) return;
    if (r) {
      rewardHistory.push(r.total);
      if (rewardHistory.length > 240) rewardHistory.shift();
    }
    var d = clear(rewardCtx, rewardCanvas);
    var mid = d.h / 2;
    rewardCtx.strokeStyle = '#2a323d';
    rewardCtx.beginPath();
    rewardCtx.moveTo(0, mid);
    rewardCtx.lineTo(d.w, mid);
    rewardCtx.stroke();
    if (rewardHistory.length < 2) return;

    var peak = 0.05;
    for (var i = 0; i < rewardHistory.length; i++) {
      peak = Math.max(peak, Math.abs(rewardHistory[i]));
    }
    var barW = d.w / rewardHistory.length;
    for (var j = 0; j < rewardHistory.length; j++) {
      var v = rewardHistory[j] / peak;
      var h = Math.abs(v) * (mid - 6);
      rewardCtx.fillStyle = v >= 0 ? 'rgba(127,216,143,0.8)' : 'rgba(242,121,121,0.8)';
      rewardCtx.fillRect(j * barW, v >= 0 ? mid - h : mid, Math.max(1, barW - 0.5), h);
    }
  }

  // --- avatar ---------------------------------------------------------------
  //
  // Driven by the expression module (B6) and the drives, which is the whole
  // point of having an expression module: the face is an output of the brain,
  // not a rendering of a state machine.
  function drawAvatar(frame) {
    if (!avatarCtx) return;
    var d = clear(avatarCtx, avatarCanvas);
    var e = frame.expression || { valence: 0.5, arousal: 0 };
    var cx = d.w / 2, cy = d.h / 2;
    var radius = Math.min(d.w, d.h) * 0.32;

    var warmth = Math.max(0, Math.min(1, e.valence));
    var hue = 200 + (warmth - 0.5) * 90;      // cool when unhappy, warm when content
    var tired = Math.max(0, Math.min(1, frame.drives.fatigue));
    var asleep = !!frame.asleep;

    avatarCtx.beginPath();
    avatarCtx.arc(cx, cy, radius * (1 + 0.06 * e.arousal), 0, Math.PI * 2);
    avatarCtx.fillStyle = 'hsl(' + hue + ',35%,' + (26 - 8 * tired) + '%)';
    avatarCtx.fill();
    avatarCtx.strokeStyle = 'hsl(' + hue + ',45%,45%)';
    avatarCtx.stroke();

    // Eyes: they narrow as fatigue rises and shut completely once asleep.
    var eyeY = cy - radius * 0.2;
    var eyeDx = radius * 0.38;
    var open = asleep ? 0 : 1 - 0.7 * tired;
    avatarCtx.fillStyle = '#d7dde5';
    for (var s = -1; s <= 1; s += 2) {
      if (open <= 0.02) {
        avatarCtx.strokeStyle = '#8b94a1';
        avatarCtx.lineWidth = 2;
        avatarCtx.beginPath();
        avatarCtx.moveTo(cx + s * eyeDx - radius * 0.11, eyeY);
        avatarCtx.lineTo(cx + s * eyeDx + radius * 0.11, eyeY);
        avatarCtx.stroke();
      } else {
        avatarCtx.beginPath();
        avatarCtx.ellipse(cx + s * eyeDx, eyeY, radius * 0.11,
                          Math.max(0.6, radius * 0.14 * open), 0, 0, Math.PI * 2);
        avatarCtx.fill();
      }
    }

    // Mouth: opens with vocal amplitude, curves with valence.
    var v = frame.vocal || { amp: 0, voicing: 0 };
    var mouthY = cy + radius * 0.38;
    var openness = Math.max(0.06, Math.min(1, v.amp)) * radius * 0.34;
    var curve = (warmth - 0.5) * radius * 0.5;
    avatarCtx.beginPath();
    avatarCtx.moveTo(cx - radius * 0.32, mouthY);
    avatarCtx.quadraticCurveTo(cx, mouthY + curve + openness, cx + radius * 0.32, mouthY);
    avatarCtx.quadraticCurveTo(cx, mouthY + curve - openness, cx - radius * 0.32, mouthY);
    avatarCtx.fillStyle = v.voicing > 0.5 ? 'rgba(242,121,168,0.85)' : 'rgba(60,68,80,0.9)';
    avatarCtx.fill();

    var mood = asleep ? 'asleep — ears shut, larynx closed'
             : frame.drives.hunger > 0.75 ? 'hungry'
             : tired > 0.7 ? 'sleepy'
             : frame.drives.comfort < 0.35 ? 'unhappy'
             : frame.drives.comfort > 0.65 ? 'content' : 'settled';
    text('mood',
      mood + ' · valence ' + e.valence.toFixed(2) + ' · arousal ' + e.arousal.toFixed(2));
  }

  // --- rendering ------------------------------------------------------------

  function setDrive(id, value) {
    var bar = $(id), label = $(id + '-v');
    if (bar) bar.style.width = (value * 100).toFixed(1) + '%';
    if (label) label.textContent = value.toFixed(2);
  }

  function render(frame) {
    if (statusEl &&
        (statusEl.className === 'live' || statusEl.className === 'asleep')) {
      statusEl.textContent = frame.asleep ? 'connected — asleep' : 'connected';
      statusEl.className = frame.asleep ? 'asleep' : 'live';
    }
    text('tick', frame.tick.toLocaleString());
    text('neurons', frame.neurons.toLocaleString());
    text('synapses', frame.synapses.toLocaleString());
    text('rate', frame.rate.toFixed(2) + ' Hz');
    text('weight', frame.weight.toFixed(4));
    text('vocalizations', frame.vocalizations.toLocaleString());

    // dt is 1 ms, so ticks are milliseconds of the baby's life.
    var seconds = frame.tick / 1000;
    var mm = Math.floor(seconds / 60), ss = Math.floor(seconds % 60);
    text('clock', mm + ':' + (ss < 10 ? '0' : '') + ss);

    setDrive('hunger', frame.drives.hunger);
    setDrive('comfort', frame.drives.comfort);
    setDrive('fatigue', frame.drives.fatigue);
    setDrive('miclevel', Math.min(1, frame.micPeak || 0));
    setDrive('viscontrast', Math.min(1, (frame.visionContrast || 0) * 3));

    // The eye's own readouts. `re-aims` is the did-it-run guard the headless
    // probe needed for the same reason: a controller that never fires and one
    // that fires and lands nowhere look identical in every other number here.
    var eye = frame.gaze;
    if (eye) {
      // Through num() rather than straight off the wire: render() is a single
      // chain and a `.toFixed` on a field an older host does not send throws
      // half way down it, which presents as the ears display dying.
      var num = function (v) { return typeof v === 'number' ? v : 0; };
      text('gaze-v', num(eye.x).toFixed(1) + ', ' + num(eye.y).toFixed(1) + ' px');
      text('gaze-moves', num(eye.moves).toLocaleString() +
                         (eye.stalls ? ' (' + eye.stalls + ' held)' : ''));
      var label = eye.mount === 'external'
          ? (eye.stale ? 'eye lost' : 'device' + (eye.lag ? ' +' + eye.lag + 'f' : ''))
          : (eye.moves ? 'tracking' : 'fixed');
      var mountPill = text('eye-mount', label);
      if (mountPill) {
        mountPill.className = 'pill' + (eye.stale ? '' : (eye.moves ? ' on' : ''));
      }
    }

    // The camera cannot open until we know how big a frame the genome expects,
    // and that arrives with the first telemetry frame. Enabling the button
    // before then would let someone start a stream the host will silently
    // discard for being the wrong size.
    if (frame.retinaShape && !cam.size) {
      cam.size = frame.retinaShape.frame;
      cam.hz = frame.retinaShape.hz || 10;
      paintCameraButton();
    }

    var v = frame.vocal;
    text('f0', v.f0.toFixed(0) + ' Hz');
    text('f1', v.f1.toFixed(0) + ' Hz');
    text('f2', v.f2.toFixed(0) + ' Hz');
    text('f3', v.f3.toFixed(0) + ' Hz');
    text('amp', v.amp.toFixed(2));
    var voicingPill = text('voicing', v.voicing > 0.5 ? 'voicing' : 'silent');
    if (voicingPill) voicingPill.className = 'pill' + (v.voicing > 0.5 ? ' on' : '');

    var r = frame.reward;
    text('r-ext', r.external.toFixed(4));
    text('r-hun', r.hunger.toFixed(5));
    text('r-com', r.comfort.toFixed(5));
    text('r-cur', r.curiosity.toFixed(5));
    text('elig', frame.elig.toFixed(5));

    // Structural plasticity (§9). Guarded with a default rather than assumed
    // present: a panel served against an older host would otherwise throw here
    // and take every canvas below it down, and the symptom would look like a
    // broken raster.
    var st = frame.structure || {};
    if (frame.structure) {
      // Live over capacity, with the growth counter in brackets. The old
      // "grown / cap" pairing read as live-over-capacity and invited exactly
      // the wrong conclusion: "40 / 9216" is a *cumulative growth* counter over
      // the *arena ceiling*, which is fixed at hatch and can never move, so a
      // healthy creature at 1142 of 9216 looked like a network stuck at 0.4%
      // of its size and refusing to grow.
      text('struct-grown', (frame.neurons || 0).toLocaleString() + ' / ' +
                           (st.capacity || 0).toLocaleString() + ' (' +
                           (st.grown || 0).toLocaleString() + ' grown)');
      text('struct-pruned', (st.prunedSynapses || 0).toLocaleString() + ' syn, ' +
                            (st.prunedNeurons || 0).toLocaleString() + ' cells');
      text('struct-passes', (st.consolidations || 0).toLocaleString());
      text('struct-replays', (st.replays || 0).toLocaleString());
      text('struct-windows', (st.windows || 0).toLocaleString() +
                             (st.plateau ? ' (plateau)' : ''));
      text('struct-plasticity', ((st.plasticity || 1) * 100).toFixed(0) + '% of eta');

      // One pill for what the machinery is doing right now, because the
      // counters above only say what it has already done.
      var state = 'stable', cls = 'pill';
      if (st.replaying) { state = 'replaying a memory'; cls = 'pill on'; }
      else if (frame.asleep) { state = 'consolidating'; cls = 'pill on'; }
      else if (st.plateau) { state = 'plateau — growth allowed'; cls = 'pill on'; }
      var pill = text('struct-state', state);
      if (pill) pill.className = cls;
    }

    if (!layout) {
      layout = buildLayout(frame.modules);
      renderLegend(frame.modules);
    }
    drawRasterColumn(frame.modules);
    drawTrace(frame.trace, frame.threshold);
    drawMel(frame.mel);
    drawRetina(frame.retina, frame.retinaShape, frame.gaze);
    drawFormants(v);
    drawReward(r);
    drawAvatar(frame);
  }

  // --- motor parameter playback --------------------------------------------

  function handleVocalPacket(buffer) {
    var view = new DataView(buffer);
    if (view.byteLength < 2 || view.getUint8(0) !== FRAME_VOCAL) return;
    var count = view.getUint8(1);
    if (!voice.ready || !speakerOn) return;

    // Frames are emitted at 100 Hz but arrive in ~30 Hz batches. Scheduling
    // them forward from a cursor keeps the voice continuous instead of
    // stepping three times and then waiting.
    var frameDt = 0.01;
    var now = voice.now();
    if (vocalCursor < now + 0.02) vocalCursor = now + 0.05;

    for (var i = 0; i < count; i++) {
      var off = 2 + i * VOCAL_FLOATS * 4;
      if (off + VOCAL_FLOATS * 4 > view.byteLength) break;
      voice.apply({
        f0: view.getFloat32(off, true),
        f1: view.getFloat32(off + 4, true),
        f2: view.getFloat32(off + 8, true),
        f3: view.getFloat32(off + 12, true),
        bw1: view.getFloat32(off + 16, true),
        bw2: view.getFloat32(off + 20, true),
        bw3: view.getFloat32(off + 24, true),
        amp: view.getFloat32(off + 28, true),
        voicing: view.getFloat32(off + 32, true)
      }, vocalCursor, 0.012);
      vocalCursor += frameDt;
    }
    // Do not let the schedule run away if the socket bursts.
    if (vocalCursor > now + 0.5) vocalCursor = now + 0.5;
  }

  // --- microphone -----------------------------------------------------------

  // Everything the microphone owns, so that turning it off can release all of
  // it. Keeping only the worklet and muting it leaves the browser's recording
  // indicator lit, which is a promise the UI would be breaking.
  var mic = { stream: null, ctx: null, node: null, on: false, muted: true };

  function send(obj) {
    if (socket && socket.readyState === WebSocket.OPEN) socket.send(JSON.stringify(obj));
  }

  function sendPcm(block) {
    if (!socket || socket.readyState !== WebSocket.OPEN) return;
    var payload = new ArrayBuffer(1 + block.length * 2);
    var view = new DataView(payload);
    view.setUint8(0, FRAME_AUDIO);
    for (var i = 0; i < block.length; i++) {
      var s = Math.max(-1, Math.min(1, block[i]));
      view.setInt16(1 + i * 2, s < 0 ? s * 0x8000 : s * 0x7FFF, true);
    }
    socket.send(payload);
  }

  function paintMicButtons() {
    var btn = $('mic');
    if (btn) {
      btn.disabled = false;
      btn.textContent = mic.on ? 'microphone on' : 'enable microphone';
      btn.className = mic.on ? 'on' : '';
    }
    var ptt = $('ptt');
    if (!ptt) return;
    ptt.disabled = !mic.on;
    ptt.className = mic.on && !mic.muted ? 'on' : '';
    ptt.textContent = !mic.on ? 'hold to talk'
                    : mic.muted ? 'hold to talk' : 'listening…';
  }

  function micOff() {
    setMicMuted(true);
    // Stopping the tracks is what actually releases the device and clears the
    // browser's recording indicator.
    if (mic.stream) mic.stream.getTracks().forEach(function (t) { t.stop(); });
    if (mic.ctx) mic.ctx.close();
    mic.stream = null;
    mic.ctx = null;
    mic.node = null;
    mic.on = false;
    // Tell the host the stream has ended, so the cochlea drops any partial
    // window rather than joining it to whatever is said next.
    send({ cmd: 'mic', amount: 0 });
    paintMicButtons();
  }

  function micOn() {
    var btn = $('mic');
    btn.disabled = true;
    btn.textContent = 'requesting…';

    navigator.mediaDevices.getUserMedia({
      audio: {
        channelCount: 1,
        // The baby has to hear the room, including its own voice through the
        // speakers. Browser speech processing would remove exactly that.
        echoCancellation: false,
        noiseSuppression: false,
        autoGainControl: false
      }
    }).then(function (stream) {
      mic.stream = stream;
      var Ctx = window.AudioContext || window.webkitAudioContext;
      // The genome says 16 kHz; asking for it here avoids a resample.
      mic.ctx = new Ctx({ sampleRate: SAMPLE_RATE });
      return mic.ctx.audioWorklet.addModule('mic-worklet.js').then(function () {
        var source = mic.ctx.createMediaStreamSource(stream);
        mic.node = new AudioWorkletNode(mic.ctx, 'mic-tap');
        mic.node.port.onmessage = function (event) {
          if (event.data && event.data.block) sendPcm(event.data.block);
        };
        source.connect(mic.node);
        // A worklet with no downstream connection may be culled; a silent
        // sink keeps it scheduled without making noise.
        var sink = mic.ctx.createGain();
        sink.gain.value = 0;
        mic.node.connect(sink).connect(mic.ctx.destination);

        if (mic.ctx.sampleRate !== SAMPLE_RATE) {
          statusEl.textContent = 'connected — mic at ' + mic.ctx.sampleRate +
            ' Hz, genome expects ' + SAMPLE_RATE;
        }
        mic.on = true;
        setMicMuted(true);   // permission granted; push to talk to actually send
        paintMicButtons();
      });
    }).catch(function (err) {
      mic.on = false;
      paintMicButtons();
      $('mic').textContent = 'microphone blocked';
      statusEl.textContent = 'microphone denied: ' + (err && err.name ? err.name : err);
    });
  }

  function setMicMuted(muted) {
    mic.muted = muted;
    if (mic.node) mic.node.port.postMessage({ muted: muted });
    if (muted) send({ cmd: 'mic', amount: 0 });
    paintMicButtons();
  }

  // --- camera ---------------------------------------------------------------
  //
  // Everything the camera owns, so that turning it off can release all of it.
  // A paused timer with a live MediaStream behind it leaves the browser's
  // camera indicator lit, and an indicator that lies about whether something
  // is watching you is not a small bug.
  // `size` and `hz` are the genome's, learned from the first telemetry frame:
  // the retina's shape is body plan, not a browser preference.
  var cam = {
    stream: null, video: null, canvas: null, ctx: null,
    timer: null, on: false, size: 0, hz: 10
  };

  // There is no push-to-talk equivalent here on purpose. Sound is something
  // you do *to* the baby in bursts; a visual field is simply there, and a
  // camera that only worked while a button was held would be a creature that
  // blinks for a living. The on/off switch is the whole of the consent model,
  // so it has to genuinely release the device.
  function paintCameraButton() {
    var btn = $('camera');
    if (!btn) return;
    btn.disabled = !cam.size;
    btn.textContent = cam.on ? 'camera on'
                    : !cam.size ? 'camera (waiting for genome)'
                    : 'enable camera';
    btn.className = cam.on ? 'on' : '';
  }

  function sendFrame() {
    if (!cam.on || !socket || socket.readyState !== WebSocket.OPEN) return;
    if (!cam.video || cam.video.readyState < 2) return;   // no decoded frame yet

    // Centre-crop to a square before scaling, or a 4:3 camera would hand the
    // baby a squashed world and every centre-surround cell would be measuring
    // an aspect ratio.
    var vw = cam.video.videoWidth, vh = cam.video.videoHeight;
    if (!vw || !vh) return;
    var side = Math.min(vw, vh);
    cam.ctx.drawImage(cam.video, (vw - side) / 2, (vh - side) / 2, side, side,
                      0, 0, cam.size, cam.size);

    var pixels = cam.ctx.getImageData(0, 0, cam.size, cam.size).data;
    var payload = new Uint8Array(1 + cam.size * cam.size);
    payload[0] = FRAME_VIDEO;
    for (var i = 0, p = 0; i < pixels.length; i += 4, p++) {
      // Rec.601 luma. The retina is monochrome (§5.1); colour is a channel the
      // creature does not have and would only be noise in the mean.
      payload[1 + p] =
        (0.299 * pixels[i] + 0.587 * pixels[i + 1] + 0.114 * pixels[i + 2]) | 0;
    }
    socket.send(payload.buffer);
  }

  function camOff() {
    if (cam.timer) clearInterval(cam.timer);
    // Stopping the tracks is what actually releases the device and clears the
    // browser's camera indicator.
    if (cam.stream) cam.stream.getTracks().forEach(function (t) { t.stop(); });
    if (cam.video) cam.video.srcObject = null;
    cam.timer = null;
    cam.stream = null;
    cam.video = null;
    cam.canvas = null;
    cam.ctx = null;
    cam.on = false;
    // Tell the host the stream has ended, so its retina clears rather than
    // holding the last frame the baby saw.
    send({ cmd: 'camera', amount: 0 });
    paintCameraButton();
  }

  function camOn() {
    var btn = $('camera');
    btn.disabled = true;
    btn.textContent = 'requesting…';

    navigator.mediaDevices.getUserMedia({
      video: { width: { ideal: 320 }, height: { ideal: 240 } }
    }).then(function (stream) {
      cam.stream = stream;
      cam.video = document.createElement('video');
      cam.video.srcObject = stream;
      cam.video.muted = true;
      cam.video.playsInline = true;
      return cam.video.play().then(function () {
        cam.canvas = document.createElement('canvas');
        cam.canvas.width = cam.size;
        cam.canvas.height = cam.size;
        // willReadFrequently: every frame is read straight back out, which is
        // the pattern the flag exists for.
        cam.ctx = cam.canvas.getContext('2d', { willReadFrequently: true });
        cam.on = true;
        // The genome says frame_hz; sending faster would only queue frames the
        // host drops, and sending slower would starve the encoder.
        cam.timer = setInterval(sendFrame, 1000 / cam.hz);
        paintCameraButton();
      });
    }).catch(function (err) {
      cam.on = false;
      paintCameraButton();
      $('camera').textContent = 'camera blocked';
      statusEl.textContent = 'camera denied: ' + (err && err.name ? err.name : err);
    });
  }

  // --- wiring ---------------------------------------------------------------

  function connect() {
    var proto = location.protocol === 'https:' ? 'wss' : 'ws';
    socket = new WebSocket(proto + '://' + location.host + '/ws');
    socket.binaryType = 'arraybuffer';

    socket.onopen = function () {
      statusEl.textContent = 'connected';
      statusEl.className = 'live';
      // Lift the mute the close handler applied, or a reconnected panel would
      // show a speaker that says on and stay silent.
      voice.setMuted(!speakerOn);
    };
    socket.onclose = function () {
      statusEl.textContent = 'disconnected — retrying';
      statusEl.className = '';
      // Motor frames stop arriving but the gain target they left behind does
      // not expire, so a dropped host would otherwise leave the voice holding
      // its last note until the panel is reloaded.
      voice.setMuted(true);
      setTimeout(connect, 1000);
    };
    socket.onerror = function () { socket.close(); };
    socket.onmessage = function (event) {
      if (typeof event.data !== 'string') {
        handleVocalPacket(event.data);
        return;
      }
      try {
        lastFrame = JSON.parse(event.data);
      } catch (e) {
        return;
      }
      render(lastFrame);
    };
  }

  document.querySelectorAll('button[data-cmd]').forEach(function (btn) {
    btn.onclick = function () { send({ cmd: btn.dataset.cmd, amount: 0.35 }); };
  });

  on('praise', 'click', function () { send({ cmd: 'praise', amount: 1.0 }); });
  on('scold', 'click', function () { send({ cmd: 'praise', amount: -1.0 }); });

  window.addEventListener('keydown', function (e) {
    if (e.target.tagName === 'INPUT' || e.repeat) return;
    if (e.key === 'g') send({ cmd: 'praise', amount: 1.0 });
    if (e.key === 'b') send({ cmd: 'praise', amount: -1.0 });
  });

  on('mic', 'click', function () { mic.on ? micOff() : micOn(); });
  on('camera', 'click', function () { cam.on ? camOff() : camOn(); });

  // Push-to-talk captures the pointer instead of watching whether it is still
  // over the button. Watching was the bug. Pressing relabels the button to
  // "listening…", which makes it narrower, which let the wrapped actions row
  // fit on one line, which moved the button out from under a finger that had
  // not moved at all. `mouseleave` then fired about four milliseconds after
  // `mousedown` and holding the button sent essentially no sound. With the
  // pointer captured the button's geometry stops mattering: it goes on
  // receiving events wherever the pointer ends up, and the press ends when
  // the press ends rather than when the layout twitches.
  on('ptt', 'pointerdown', function (e) {
    e.preventDefault();
    if (!mic.on) return;
    // Capture is a nicety, not a requirement — if it is refused the release
    // handlers below still fire on this element.
    try { this.setPointerCapture(e.pointerId); } catch (err) { /* ignore */ }
    setMicMuted(false);
  });
  ['pointerup', 'pointercancel', 'lostpointercapture'].forEach(function (ev) {
    on('ptt', ev, function () { if (mic.on) setMicMuted(true); });
  });
  // A pointer that disappears without a pointerup — window switched away, tab
  // hidden — must not leave the microphone open behind your back.
  window.addEventListener('blur', function () { if (mic.on) setMicMuted(true); });
  document.addEventListener('visibilitychange', function () {
    if (document.hidden && mic.on) setMicMuted(true);
  });

  on('speaker', 'click', function () {
    // Audio contexts start suspended until a gesture, so the voice is built
    // here rather than on load.
    voice.start();
    voice.resume();
    speakerOn = !speakerOn;
    voice.setMuted(!speakerOn);
    this.textContent = speakerOn ? 'on' : 'off';
    this.className = speakerOn ? 'on' : '';
  });

  window.addEventListener('resize', function () {
    if (fit(raster)) layout = null;  // bin geometry depends on canvas size
    [trace, melCanvas, retinaCanvas, formantCanvas, rewardCanvas, avatarCanvas].forEach(fit);
    if (lastFrame) render(lastFrame);
  });

  [raster, trace, melCanvas, retinaCanvas, formantCanvas, rewardCanvas,
   avatarCanvas].forEach(fit);

  // Pin the buttons that relabel themselves to the width they are born with,
  // which is their widest label. A control you hold down must not resize under
  // your hand: in a wrapping row a few pixels are enough to move it to another
  // line. Measured rather than guessed at, because the width depends on the
  // font the browser actually found.
  ['ptt', 'mic', 'camera'].forEach(function (id) {
    var el = $(id);
    if (el) el.style.minWidth = el.getBoundingClientRect().width + 'px';
  });

  paintCameraButton();
  connect();
})();
