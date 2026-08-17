// The voice.
//
// §5.3 is explicit that the network must not generate samples. It emits eight
// continuous vocal-tract parameters plus a voicing gate at 100 Hz, and this is
// the body those parameters drive: a glottal source through three formant
// resonators. Eight numbers a hundred times a second instead of sixteen
// thousand samples is the difference between a learnable motor output and an
// impossible one.

(function (global) {
  'use strict';

  function Voice() {
    this.ctx = null;
    this.ready = false;
    this.muted = false;
  }

  Voice.prototype.start = function () {
    if (this.ctx) return;
    var Ctx = global.AudioContext || global.webkitAudioContext;
    var ctx = new Ctx();
    this.ctx = ctx;

    // Glottal source: a sawtooth is a decent stand-in for a glottal pulse
    // train — rich in harmonics, and the formant filters do the shaping.
    this.glottis = ctx.createOscillator();
    this.glottis.type = 'sawtooth';
    this.glottis.frequency.value = 120;

    // Aspiration: white noise for the unvoiced case. Two seconds, looped.
    var noiseLength = ctx.sampleRate * 2;
    var buffer = ctx.createBuffer(1, noiseLength, ctx.sampleRate);
    var data = buffer.getChannelData(0);
    for (var i = 0; i < noiseLength; i++) data[i] = Math.random() * 2 - 1;
    this.noise = ctx.createBufferSource();
    this.noise.buffer = buffer;
    this.noise.loop = true;

    this.voicedGain = ctx.createGain();
    this.voicedGain.gain.value = 0;
    this.noiseGain = ctx.createGain();
    this.noiseGain.gain.value = 0;

    var source = ctx.createGain();
    this.glottis.connect(this.voicedGain).connect(source);
    this.noise.connect(this.noiseGain).connect(source);

    // Three formants in parallel, each a bandpass whose centre and bandwidth
    // are what the motor module is actually controlling.
    this.formants = [];
    var mix = ctx.createGain();
    var defaults = [500, 1500, 2500];
    var gains = [1.0, 0.6, 0.35];
    for (var f = 0; f < 3; f++) {
      var filter = ctx.createBiquadFilter();
      filter.type = 'bandpass';
      filter.frequency.value = defaults[f];
      filter.Q.value = defaults[f] / 90;
      var g = ctx.createGain();
      g.gain.value = gains[f];
      source.connect(filter).connect(g).connect(mix);
      this.formants.push({ filter: filter, gain: g });
    }

    this.master = ctx.createGain();
    this.master.gain.value = 0;
    mix.connect(this.master).connect(ctx.destination);

    this.glottis.start();
    this.noise.start();
    this.ready = true;
  };

  Voice.prototype.resume = function () {
    if (this.ctx && this.ctx.state === 'suspended') this.ctx.resume();
  };

  // One motor frame. `when` is the AudioContext time it should take effect;
  // frames arrive in batches and are scheduled ahead so the voice is smooth
  // even though telemetry only lands 30 times a second.
  Voice.prototype.apply = function (p, when, glide) {
    if (!this.ready) return;
    var t = when;
    var ramp = glide;

    this.glottis.frequency.setTargetAtTime(p.f0, t, ramp);
    for (var i = 0; i < 3; i++) {
      var freq = [p.f1, p.f2, p.f3][i];
      var bw = [p.bw1, p.bw2, p.bw3][i];
      this.formants[i].filter.frequency.setTargetAtTime(freq, t, ramp);
      // Q and bandwidth are the same statement about a resonator, said twice.
      this.formants[i].filter.Q.setTargetAtTime(Math.max(0.5, freq / Math.max(20, bw)),
                                                t, ramp);
    }

    var voiced = p.voicing > 0.5;
    this.voicedGain.gain.setTargetAtTime(voiced ? 1.0 : 0.0, t, ramp);
    this.noiseGain.gain.setTargetAtTime(voiced ? 0.05 : 0.25, t, ramp);

    var level = this.muted ? 0 : Math.min(1, Math.max(0, p.amp)) * 0.35;
    this.master.gain.setTargetAtTime(level, t, ramp);
  };

  Voice.prototype.setMuted = function (muted) {
    this.muted = muted;
    if (this.ready && muted) {
      // Cancelling first is not a tidiness measure, it is the whole fix.
      // apply() schedules gain targets up to half a second ahead of the clock,
      // so a fade started now is immediately overwritten by frames that were
      // already queued — and the last of those leaves the gain sitting at
      // babble level with no automation behind it to bring it back down. The
      // symptom is a voice that ducks for a moment when you press off and then
      // drones on one note forever.
      var t = this.ctx.currentTime;
      this.master.gain.cancelScheduledValues(t);
      this.master.gain.setTargetAtTime(0, t, 0.02);
    }
  };

  Voice.prototype.now = function () { return this.ctx ? this.ctx.currentTime : 0; };

  global.Voice = Voice;
})(window);
