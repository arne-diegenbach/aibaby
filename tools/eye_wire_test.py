#!/usr/bin/env python3
"""Wire-protocol test for the eye port.

Speaks the WebSocket handshake and frame format directly against a real host,
because there is no `websockets` module on this machine and because the point
is to test the protocol rather than a client library's idea of it.

Traps this harness is built around (all of them have made a working feature
look broken here before):
  - read the LATEST telemetry frame, not the next queued one. The host
    broadcasts at 30 Hz whether or not anyone reads.
  - keep the stimulus streaming while sampling. Sensors fade by design.
  - run the host at --speed 1, or a 10 Hz camera becomes 2.5 Hz in baby time.
"""

import base64, hashlib, json, os, socket, struct, subprocess, sys, tempfile, time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRATCH = tempfile.mkdtemp(prefix="aibaby-eyewire-")
PORT = int(os.environ.get("AIBABY_PORT", "8099"))
FRAME_VIDEO = 2

fails = []


def check(name, ok, detail=""):
    print(("  PASS  " if ok else "  FAIL  ") + name + (("   " + detail) if detail else ""))
    if not ok:
        fails.append(name)


# --- the smallest WebSocket client that can drive this host ------------------

class Ws:
    def __init__(self, port):
        self.s = socket.create_connection(("127.0.0.1", port), timeout=5)
        key = base64.b64encode(os.urandom(16)).decode()
        self.s.sendall(
            ("GET /ws HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\n"
             "Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\n"
             "Sec-WebSocket-Version: 13\r\n\r\n" % key).encode())
        buf = b""
        while b"\r\n\r\n" not in buf:
            buf += self.s.recv(4096)
        head, rest = buf.split(b"\r\n\r\n", 1)
        assert b"101" in head.split(b"\r\n")[0], head
        want = base64.b64encode(hashlib.sha1(
            (key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode()).digest()).decode()
        assert want.encode() in head, "bad accept key"
        self.buf = rest

    def send(self, payload, opcode=1):
        if isinstance(payload, str):
            payload = payload.encode()
        n = len(payload)
        head = bytes([0x80 | opcode])
        if n < 126:
            head += bytes([0x80 | n])
        elif n < 65536:
            head += bytes([0x80 | 126]) + struct.pack(">H", n)
        else:
            head += bytes([0x80 | 127]) + struct.pack(">Q", n)
        mask = os.urandom(4)
        masked = bytes(b ^ mask[i % 4] for i, b in enumerate(payload))
        self.s.sendall(head + mask + masked)

    def _fill(self, n):
        while len(self.buf) < n:
            chunk = self.s.recv(65536)
            if not chunk:
                raise EOFError
            self.buf += chunk

    def recv_frame(self):
        self._fill(2)
        b1, b2 = self.buf[0], self.buf[1]
        opcode = b1 & 0x0F
        n = b2 & 0x7F
        off = 2
        if n == 126:
            self._fill(4)
            n = struct.unpack(">H", self.buf[2:4])[0]
            off = 4
        elif n == 127:
            self._fill(10)
            n = struct.unpack(">Q", self.buf[2:10])[0]
            off = 10
        self._fill(off + n)
        payload = self.buf[off:off + n]
        self.buf = self.buf[off + n:]
        return opcode, payload

    def latest_telemetry(self, seconds=0.6):
        """Drain for a while and keep the last text frame. The backlog in front
        of it is from before whatever we just did."""
        deadline = time.time() + seconds
        last = None
        self.s.settimeout(0.15)
        while time.time() < deadline:
            try:
                op, payload = self.recv_frame()
            except (socket.timeout, EOFError):
                continue
            if op == 1:
                last = json.loads(payload.decode())
        self.s.settimeout(5)
        return last


def gray_frame(size, cx, cy, r):
    """A disc, as the browser would send it: one grayscale byte per pixel.

    It has to clear `contrast_floor` — the genome's own statement of what counts
    as seeing something — or the controller correctly refuses to chase it. A 5 px
    disc at 220/110 reads 0.036 against a floor of 0.06 and looks exactly like a
    broken port."""
    out = bytearray(size * size)
    for y in range(size):
        for x in range(size):
            d2 = (x + 0.5 - cx) ** 2 + (y + 0.5 - cy) ** 2
            out[y * size + x] = 235 if d2 <= r * r else 70
    return bytes(out)


# --- drive it ---------------------------------------------------------------

def main():
    proc = subprocess.Popen(
        [os.path.join(ROOT, "build/aibaby"), "--port", str(PORT), "--speed", "1",
         "--journal", os.path.join(SCRATCH, "eyewire.aibj"),
         "--web", os.path.join(ROOT, "web")],
        cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    time.sleep(1.2)

    try:
        ws = Ws(PORT)
        frame = ws.latest_telemetry()
        check("telemetry carries a gaze block", frame is not None and "gaze" in frame)
        g = frame["gaze"]
        size = frame["retinaShape"]["frame"]
        check("the eye starts internal and centred",
              g["mount"] == "internal" and abs(g["x"]) < 0.01 and abs(g["y"]) < 0.01,
              "mount=%s x=%.2f y=%.2f" % (g["mount"], g["x"], g["y"]))
        check("a fresh eye reports no lag and no reports",
              g["lag"] == 0 and g["reports"] == 0 and g["stale"] is False)
        check("the panel has something to draw a crosshair with",
              all(k in g for k in ("x", "y", "cx", "cy", "fx", "fy", "seq", "moves",
                                   "mount", "stale", "lag", "reports", "stalls")),
              "keys=%s" % ",".join(sorted(g)))

        # --- steering, in pixels and in frame fractions --------------------------
        ws.send(json.dumps({"cmd": "look", "x": 8, "y": -4}))
        g = ws.latest_telemetry()["gaze"]
        check("look in px moves an internal eye",
              abs(g["x"] - 8) < 0.01 and abs(g["y"] + 4) < 0.01,
              "x=%.2f y=%.2f" % (g["x"], g["y"]))
        check("the command follows the eye it teleported",
              abs(g["cx"] - 8) < 0.01 and g["seq"] > 0, "cx=%.2f seq=%d" % (g["cx"], g["seq"]))

        first_seq = g["seq"]
        ws.send(json.dumps({"cmd": "look", "fx": -0.25, "fy": 0.125}))
        g = ws.latest_telemetry()["gaze"]
        check("look in frame fractions is the same instruction",
              abs(g["x"] + 0.25 * size) < 0.01 and abs(g["y"] - 0.125 * size) < 0.01,
              "x=%.2f y=%.2f (frame %d)" % (g["x"], g["y"], size))
        # A device watching for new work polls the seq. An instruction that
        # moves the eye without advancing it is one the device cannot see, and
        # that is exactly what teleporting before commanding used to do.
        check("every instruction advances the sequence number",
              g["seq"] > first_seq, "seq %d -> %d" % (first_seq, g["seq"]))
        check("the command is published device-neutral too",
              abs(g["fx"] + 0.25) < 0.001, "fx=%.4f" % g["fx"])

        # --- feedback is ignored until the mount says who owns the eye -----------
        ws.send(json.dumps({"cmd": "gaze", "x": 20, "y": 20, "seq": 1}))
        g = ws.latest_telemetry()["gaze"]
        check("a report to an internal eye is refused, visibly",
              g["reports"] == 0 and abs(g["x"] + 0.25 * size) < 0.01,
              "reports=%d x=%.2f" % (g["reports"], g["x"]))

        ws.send(json.dumps({"cmd": "eye", "mount": "external"}))
        g = ws.latest_telemetry()["gaze"]
        check("the mount switches to external", g["mount"] == "external")

        ws.send(json.dumps({"cmd": "gaze", "x": 6, "y": -2, "seq": g["seq"]}))
        g = ws.latest_telemetry()["gaze"]
        check("feedback moves an external eye",
              g["reports"] >= 1 and abs(g["x"] - 6) < 0.01 and abs(g["y"] + 2) < 0.01,
              "reports=%d x=%.2f y=%.2f" % (g["reports"], g["x"], g["y"]))

        ws.send(json.dumps({"cmd": "gaze", "fx": 0.0625, "fy": 0.0, "seq": g["seq"]}))
        g = ws.latest_telemetry()["gaze"]
        check("feedback in fractions lands in the same place",
              abs(g["x"] - 0.0625 * size) < 0.01, "x=%.2f" % g["x"])

        # --- the closed loop, with a camera actually running ---------------------
        # The controller only exists on frame boundaries, so nothing above has
        # exercised it. Stream a disc off to one side and act as the device.
        # Home the COMMAND as well as the position. Reporting a position while a
        # stale command still points 16 px away just means the device dutifully
        # drives back to the stale command on the next frame.
        ws.send(json.dumps({"cmd": "eye", "mount": "internal"}))
        ws.send(json.dumps({"cmd": "look", "x": 0, "y": 0}))
        ws.send(json.dumps({"cmd": "eye", "mount": "external"}))
        ws.send(json.dumps({"cmd": "gaze", "x": 0, "y": 0, "seq": 0}))
        dev_x, dev_y = 0.0, 0.0
        seen_seq = 0
        contrasts = []
        for i in range(60):
            # The device has aimed, so the world arrives displaced by the opposite.
            ws.send(bytes([FRAME_VIDEO]) + gray_frame(size, size * 0.72 - dev_x,
                                                      size * 0.5 - dev_y, 9), opcode=2)
            time.sleep(0.1)
            frame = ws.latest_telemetry(0.05)
            if not frame:
                continue
            g = frame["gaze"]
            contrasts.append(frame.get("visionContrast", 0.0))
            dev_x, dev_y = g["cx"], g["cy"]
            seen_seq = g["seq"]
            ws.send(json.dumps({"cmd": "gaze", "x": dev_x, "y": dev_y, "seq": seen_seq}))
        print("        (peak contrast seen: %.4f, floor is contrast_floor)" % max(contrasts or [0]))
        g = ws.latest_telemetry()["gaze"]
        check("the controller drives an external eye toward the toy",
              g["moves"] > 0 and g["x"] > 3.0,
              "moves=%d x=%.1f (toy 14 px right of centre)" % (g["moves"], g["x"]))
        check("a prompt device measures as prompt", g["lag"] <= 2, "lag=%d" % g["lag"])
        check("nothing was held while the device was answering",
              g["stalls"] == 0, "stalls=%d" % g["stalls"])

        # --- the lag instrument, against a device that is deliberately slow ------
        # The eye has to keep being asked for something or there is no new seq to
        # acknowledge and the measurement has nothing to measure, so the toy sweeps.
        # The device answers the command it saw two frames ago, which is exactly the
        # `dead_frames` the servo sweep says is the dangerous parameter.
        # The jitter stays INSIDE the fovea. Swinging the toy out into the rings
        # instead just reproduces the known peripheral-acquisition failure: the
        # controller cannot see the displacement, issues no command, and the lag
        # instrument reads zero for want of anything to time.
        pending = []
        for i in range(24):
            toy = size * 0.5 + dev_x + (4 if i % 2 else -4)
            ws.send(bytes([FRAME_VIDEO]) + gray_frame(size, toy - dev_x,
                                                      size * 0.5 - dev_y, 9), opcode=2)
            time.sleep(0.1)
            frame = ws.latest_telemetry(0.05)
            if not frame:
                continue
            pending.append((frame["gaze"]["cx"], frame["gaze"]["cy"], frame["gaze"]["seq"]))
            if len(pending) > 2:
                dev_x, dev_y, old_seq = pending.pop(0)
                ws.send(json.dumps({"cmd": "gaze", "x": dev_x, "y": dev_y, "seq": old_seq}))
        g = ws.latest_telemetry()["gaze"]
        check("a two-frame device is measured as two frames behind",
              1 <= g["lag"] <= 4, "lag=%d frames" % g["lag"])

        # --- stop answering, keep the camera running -----------------------------
        held_before = g["stalls"]
        for i in range(20):
            ws.send(bytes([FRAME_VIDEO]) + gray_frame(size, size * 0.72 - dev_x,
                                                      size * 0.5 - dev_y, 9), opcode=2)
            time.sleep(0.1)
        g = ws.latest_telemetry()["gaze"]
        check("a silent device reads as stale", g["stale"] is True)
        check("and the controller says how often it held",
              g["stalls"] > held_before, "stalls=%d" % g["stalls"])
        frozen = (g["x"], g["y"])

        for i in range(10):
            ws.send(bytes([FRAME_VIDEO]) + gray_frame(size, size * 0.72 - dev_x,
                                                      size * 0.5 - dev_y, 9), opcode=2)
            time.sleep(0.1)
        g = ws.latest_telemetry()["gaze"]
        check("the eye stays exactly where the device left it",
              abs(g["x"] - frozen[0]) < 0.01 and abs(g["y"] - frozen[1]) < 0.01,
              "x=%.2f was %.2f" % (g["x"], frozen[0]))

        # --- and back ------------------------------------------------------------
        ws.send(json.dumps({"cmd": "gaze", "x": dev_x, "y": dev_y, "seq": seen_seq}))
        g = ws.latest_telemetry()["gaze"]
        check("one report clears the staleness", g["stale"] is False)

        ws.send(json.dumps({"cmd": "eye", "mount": "internal"}))
        g = ws.latest_telemetry()["gaze"]
        check("the mount switches back", g["mount"] == "internal")
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()

    print()
    if fails:
        print("FAILED: " + ", ".join(fails))
        sys.exit(1)
    print("eye wire protocol: all checks pass")


if __name__ == "__main__":
    main()
