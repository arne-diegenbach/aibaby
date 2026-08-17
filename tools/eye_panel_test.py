#!/usr/bin/env python3
"""Does the panel actually draw where the eye is looking?

Real Chrome, real page, real socket. Asserted through the DOM and through
getImageData on the retina canvas, because the page's logic is inside an IIFE
and there is nothing to call into.
"""

import base64, json, os, socket, subprocess, sys, tempfile, time, urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from eye_wire_test import Ws  # the raw WebSocket client, reused to speak CDP

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRATCH = tempfile.mkdtemp(prefix="aibaby-eyepanel-")
PORT = int(os.environ.get("AIBABY_PORT", "8098"))
CDP = int(os.environ.get("AIBABY_CDP_PORT", "9223"))

fails = []


def check(name, ok, detail=""):
    print(("  PASS  " if ok else "  FAIL  ") + name + (("   " + detail) if detail else ""))
    if not ok:
        fails.append(name)


class Cdp:
    def __init__(self, ws_url):
        host = ws_url.split("//", 1)[1].split("/", 1)[0]
        path = "/" + ws_url.split("//", 1)[1].split("/", 1)[1]
        self.n = 0
        self.ws = Ws.__new__(Ws)
        self.ws.s = socket.create_connection((host.split(":")[0], int(host.split(":")[1])),
                                             timeout=10)
        key = base64.b64encode(os.urandom(16)).decode()
        self.ws.s.sendall(("GET %s HTTP/1.1\r\nHost: %s\r\nUpgrade: websocket\r\n"
                           "Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\n"
                           "Sec-WebSocket-Version: 13\r\n\r\n" % (path, host, key)).encode())
        buf = b""
        while b"\r\n\r\n" not in buf:
            buf += self.ws.s.recv(4096)
        self.ws.buf = buf.split(b"\r\n\r\n", 1)[1]

    def eval(self, expr):
        self.n += 1
        self.ws.send(json.dumps({"id": self.n, "method": "Runtime.evaluate",
                                 "params": {"expression": expr, "returnByValue": True,
                                            "awaitPromise": True}}))
        while True:
            op, payload = self.ws.recv_frame()
            if op != 1:
                continue
            msg = json.loads(payload.decode())
            if msg.get("id") == self.n:
                res = msg.get("result", {}).get("result", {})
                if "exceptionDetails" in msg.get("result", {}):
                    raise RuntimeError(msg["result"]["exceptionDetails"])
                return res.get("value")


host_proc = subprocess.Popen(
    [os.path.join(ROOT, "build/aibaby"), "--port", str(PORT), "--speed", "1",
     "--journal", os.path.join(SCRATCH, "eyepanel.aibj"), "--web", os.path.join(ROOT, "web")],
    cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
chrome = subprocess.Popen(
    ["google-chrome", "--headless=new", "--disable-gpu", "--no-sandbox",
     "--remote-debugging-port=%d" % CDP, "--window-size=1400,1200",
     "http://localhost:%d/" % PORT],
    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
time.sleep(4)

try:
    targets = json.load(urllib.request.urlopen("http://127.0.0.1:%d/json/list" % CDP))
    page = [t for t in targets if t["type"] == "page"][0]
    cdp = Cdp(page["webSocketDebuggerUrl"])
    time.sleep(2)

    check("the panel connected", cdp.eval("document.getElementById('status').textContent")
          not in (None, "", "disconnected"),
          "status=%r" % cdp.eval("document.getElementById('status').textContent"))
    check("the eye readouts exist and are live",
          cdp.eval("document.getElementById('gaze-v').textContent") != "—",
          "gaze=%r" % cdp.eval("document.getElementById('gaze-v').textContent"))
    check("the mount pill reads fixed on a still eye",
          cdp.eval("document.getElementById('eye-mount').textContent") == "fixed")

    # Move the eye from outside, the way an attention system would.
    drv = Ws(PORT)
    drv.send(json.dumps({"cmd": "look", "fx": 0.25, "fy": -0.125}))
    time.sleep(1.0)

    check("the readout follows the eye",
          cdp.eval("document.getElementById('gaze-v').textContent") == "16.0, -8.0 px",
          "gaze=%r" % cdp.eval("document.getElementById('gaze-v').textContent"))

    # And the crosshair itself. Nothing is streaming, so every cell is drawn at
    # alpha 0.04 and the only strong blue on the canvas is the mark.
    found = cdp.eval("""(function () {
      var c = document.getElementById('retina');
      var ctx = c.getContext('2d');
      var ratio = c.width / c.clientWidth;
      var img = ctx.getImageData(0, 0, c.width, c.height).data;
      var n = 0, sx = 0, sy = 0;
      for (var i = 0; i < img.length; i += 4) {
        if (img[i + 2] > 150 && img[i + 3] > 200 && img[i] < 150) {
          var p = (i / 4) | 0;
          sx += p % c.width; sy += (p / c.width) | 0; n++;
        }
      }
      var h = parseInt(c.getAttribute('height'), 10);
      var side = Math.min(c.clientWidth, h) - 6;
      var ox = (c.clientWidth - side) / 2, oy = (h - side) / 2, scale = side / 64;
      return { n: n, x: n ? sx / n / ratio : -1, y: n ? sy / n / ratio : -1,
               wantX: ox + (32 + 16) * scale, wantY: oy + (32 - 8) * scale };
    })()""")
    check("a crosshair is drawn", found["n"] > 20, "%d px" % found["n"])
    check("and it is drawn where the eye is",
          abs(found["x"] - found["wantX"]) < 3 and abs(found["y"] - found["wantY"]) < 3,
          "at %.1f,%.1f want %.1f,%.1f" % (found["x"], found["y"],
                                           found["wantX"], found["wantY"]))

    # An external eye that goes quiet has to look different, or the display is
    # showing a frozen number with no way to know it is frozen.
    drv.send(json.dumps({"cmd": "eye", "mount": "external"}))
    time.sleep(0.5)
    check("the pill names the device",
          cdp.eval("document.getElementById('eye-mount').textContent") in ("device", "eye lost"),
          "pill=%r" % cdp.eval("document.getElementById('eye-mount').textContent"))
finally:
    for p in (chrome, host_proc):
        p.terminate()
        try:
            p.wait(timeout=5)
        except subprocess.TimeoutExpired:
            p.kill()

print()
if fails:
    print("FAILED: " + ", ".join(fails))
    sys.exit(1)
print("eye panel: all checks pass")
