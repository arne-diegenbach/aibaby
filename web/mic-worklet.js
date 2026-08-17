// Microphone tap.
//
// Runs on the audio thread so a busy main thread cannot drop samples. It does
// no processing at all — the cochlea lives in the host, where it can be
// aligned to simulation ticks and written to the journal. All this does is
// hand over blocks of mono float and report a peak for the level meter.

class MicTap extends AudioWorkletProcessor {
  constructor() {
    super();
    this.muted = true;
    this.port.onmessage = (event) => {
      if (event.data && typeof event.data.muted === 'boolean') {
        this.muted = event.data.muted;
      }
    };
  }

  process(inputs) {
    const input = inputs[0];
    if (!input || input.length === 0) return true;
    const channel = input[0];
    if (!channel || channel.length === 0) return true;

    if (this.muted) return true;

    // Copy: the render quantum buffer is reused underneath us.
    const block = new Float32Array(channel.length);
    let peak = 0;
    for (let i = 0; i < channel.length; i++) {
      const s = channel[i];
      block[i] = s;
      const a = s < 0 ? -s : s;
      if (a > peak) peak = a;
    }
    this.port.postMessage({ block: block, peak: peak }, [block.buffer]);
    return true;
  }
}

registerProcessor('mic-tap', MicTap);
