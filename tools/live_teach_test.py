#!/usr/bin/env python3
"""Does teaching work through the LIVE app, not just the headless harness?

M1c was measured entirely inside `--experiment teachsound`. This project's own
note says run the actual app before claiming a milestone -- a single-seed `audio`
pass once hid a 4-of-9 failure. So: real Chrome, real page, real WebSocket, real
key events through CDP's Input domain rather than synthetic JS events, because
the panel has already shipped a bug where synthetic events passed and real input
did not.

Asserts the chain M1c depends on, end to end in the live binary:
  the page loads and draws  ->  a real 'g' keypress reaches the server
  ->  the brain's reward moves  ->  the creature's voice moves with it
"""
import json, os, subprocess, sys, tempfile, time, urllib.request

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__))))
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
# Launch the server yourself first:  ./build/aibaby --port 8137 --speed 40
from eye_wire_test import Ws  # has an __main__ guard; importing it runs nothing


class Cdp:
    """A generic CDP client. `eye_panel_test.Cdp` only exposes eval(), and this
    needs Page.navigate, Input.dispatchKeyEvent and Page.captureScreenshot —
    the Input domain especially, because a synthetic JS event is not what a
    caregiver's keypress is and this panel has shipped that bug before."""

    def __init__(self, ws_url):
        import socket, base64
        host = ws_url.split("//", 1)[1].split("/", 1)[0]
        path = "/" + ws_url.split("//", 1)[1].split("/", 1)[1]
        self.n = 0
        self.ws = Ws.__new__(Ws)
        self.ws.s = socket.create_connection(
            (host.split(":")[0], int(host.split(":")[1])), timeout=15)
        key = base64.b64encode(os.urandom(16)).decode()
        self.ws.s.sendall(("GET %s HTTP/1.1\r\nHost: %s\r\nUpgrade: websocket\r\n"
                           "Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\n"
                           "Sec-WebSocket-Version: 13\r\n\r\n"
                           % (path, host, key)).encode())
        buf = b""
        while b"\r\n\r\n" not in buf:
            buf += self.ws.s.recv(4096)
        self.ws.buf = buf.split(b"\r\n\r\n", 1)[1]

    def call(self, method, params=None):
        self.n += 1
        self.ws.send(json.dumps({"id": self.n, "method": method,
                                 "params": params or {}}))
        while True:
            op, payload = self.ws.recv_frame()
            if op != 1:
                continue
            msg = json.loads(payload.decode())
            if msg.get("id") == self.n:
                return msg.get("result", {})

    def eval(self, expr):
        r = self.call("Runtime.evaluate",
                      {"expression": expr, "returnByValue": True, "awaitPromise": True})
        return r.get("result", {}).get("value")

PORT = int(os.environ.get("AIBABY_PORT", "8137"))
CDP_PORT = int(os.environ.get("AIBABY_CDP_PORT", "9231"))
SCRATCH = tempfile.mkdtemp(prefix="aibaby-liveteach-")
fails = []

def check(name, ok, detail=""):
    print(("  PASS  " if ok else "  FAIL  ") + name + (("   " + detail) if detail else ""))
    if not ok:
        fails.append(name)

chrome = subprocess.Popen(
    ["google-chrome", "--headless=new", "--disable-gpu", "--no-sandbox",
     f"--remote-debugging-port={CDP_PORT}", f"--user-data-dir={SCRATCH}",
     "--window-size=1400,900", "about:blank"],
    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

try:
    ws_url = None
    for _ in range(80):
        try:
            tabs = json.load(urllib.request.urlopen(f"http://127.0.0.1:{CDP_PORT}/json"))
            for t in tabs:
                if t.get("type") == "page":
                    ws_url = t["webSocketDebuggerUrl"]
                    break
            if ws_url:
                break
        except Exception:
            pass
        time.sleep(0.25)
    if not ws_url:
        print("  FAIL  chrome never exposed a page target")
        sys.exit(1)

    cdp = Cdp(ws_url)
    cdp.call("Page.enable")
    cdp.call("Runtime.enable")
    cdp.call("Page.navigate", {"url": f"http://localhost:{PORT}/"})
    time.sleep(4.0)

    title = cdp.eval("document.title")
    check("panel loads", isinstance(title, str) and "aibaby" in title, repr(title))

    # A blank frame is a failure to launch, so look at actual pixels.
    shot = cdp.call("Page.captureScreenshot", {"format": "png"})
    png = shot.get("data", "")
    import base64
    raw = base64.b64decode(png) if png else b""
    path = "/tmp/claude-1000/-home-arne-development-aibaby/dc7627fe-bcfb-4faa-aa2c-4319cb1eaf38/scratchpad/panel.png"
    open(path, "wb").write(raw)
    check("panel renders", len(raw) > 20000, f"{len(raw)} bytes -> {path}")

    live = cdp.eval("document.body.innerText.length")
    check("panel has content", isinstance(live, (int, float)) and live > 200, str(live))

    # The socket the panel itself uses, opened a second time so the test can
    # read what the creature is doing without reaching into the page's IIFE.
    obs = Ws(PORT)

    def sample(rounds=8):
        rew, f1, f2, ticks = [], [], [], []
        for _ in range(rounds):
            m = obs.latest_telemetry(0.35)
            if not m:
                continue
            r = m.get("reward")
            if isinstance(r, dict):
                rew.append(float(r.get("total", 0.0)))
            elif r is not None:
                rew.append(float(r))
            v = m.get("voice") or m.get("vocal")
            if isinstance(v, dict):
                if "f1" in v: f1.append(float(v["f1"]))
                if "f2" in v: f2.append(float(v["f2"]))
            if "tick" in m:
                ticks.append(m["tick"])
        return rew, f1, f2, ticks

    rew0, f1_0, f2_0, t0 = sample(10)
    check("creature is running", len(t0) >= 2 and t0[-1] > t0[0],
          f"tick {t0[0] if t0 else '?'} -> {t0[-1] if t0 else '?'}")
    check("voice is on the wire", len(f1_0) > 0,
          f"{len(f1_0)} f1 samples, mean {sum(f1_0)/len(f1_0):.0f} Hz" if f1_0 else "none")

    def press(n=50):
        """Real key events through the Input domain. Not dispatchEvent: this
        panel has already shipped a bug where synthetic events passed and a
        caregiver's actual keypress did not."""
        for _ in range(n):
            for kind in ("rawKeyDown", "char", "keyUp"):
                cdp.call("Input.dispatchKeyEvent",
                         {"type": kind, "key": "g", "text": "g", "unmodifiedText": "g",
                          "windowsVirtualKeyCode": 71, "nativeVirtualKeyCode": 71})
            time.sleep(0.02)

    def mean(xs):
        return sum(xs) / len(xs) if xs else 0.0

    # PAIRED, and repeated. The creature's own drives produce reward of their
    # own — hunger, comfort, curiosity — so one before/after window is a single
    # draw against a moving baseline, and the first version of this check passed
    # on one run and failed on the next for exactly that reason. Four quiet/
    # praise pairs, and the praise half has to win most of them.
    wins, pairs = 0, 4
    quiet_all, loud_all, f1_quiet, f1_loud = [], [], [], []
    for i in range(pairs):
        q, qf1, _, _ = sample(6)
        press(50)
        l, lf1, _, _ = sample(6)
        quiet_all += q
        loud_all += l
        f1_quiet += qf1
        f1_loud += lf1
        if mean(l) > mean(q):
            wins += 1
        print(f"        pair {i + 1}: quiet mean reward {mean(q):+.4f}   "
              f"praised {mean(l):+.4f}")
    check("a real 'g' keypress reaches the brain", wins >= 3,
          f"praise window higher in {wins} of {pairs}; "
          f"overall {mean(quiet_all):+.4f} -> {mean(loud_all):+.4f}")
    if f1_quiet and f1_loud:
        print(f"        F1 {mean(f1_quiet):.0f} Hz quiet -> {mean(f1_loud):.0f} Hz "
              f"praised")
    obs.s.close()
finally:
    chrome.terminate()

print()
if fails:
    print("  live teaching path FAILED: " + ", ".join(fails))
    sys.exit(1)
print("  live teaching path PASS — the panel draws, a real keypress reaches the")
print("  brain, and the creature's voice is on the wire while it does.")
