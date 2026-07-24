"""
run.py — FM entry point.

Responsibilities of this file, and ONLY these:
    1. Serve the GUI (templates/fm-gui.html) on 127.0.0.1.
  2. Launch the compiled `core` binary once the server below is confirmed
      listening.
  3. Relay requests from the GUI to `core` via PyCppBridge, and relay
     core's responses back to the GUI. No parsing, validation, or
     business logic on the file contents happens here — that is core's job.

Message contract over the bridge (documented here since main.cpp/CORE
is a separate deliverable — adjust to match whatever `core` actually
expects/returns):

  GUI -> Flask -> core   (bridge.send(..., "request"))
    { "action": "list", "path": "<path>" }
    { "action": "run",  "command": "<raw fm command line>" }

  core -> Flask -> GUI   (bridge.receive("response"))
    whatever JSON-serializable object core sends back; it is forwarded
    to the browser byte-for-byte (as JSON), unmodified.
"""

import atexit
import os
import socket
import subprocess
import sys
import threading
import time
from pathlib import Path

from flask import Flask, abort, jsonify, render_template, request

from Bridge import Bridge

# --------------------------------------------------------------------------
# Config
# --------------------------------------------------------------------------
HOST = "127.0.0.1"          # loopback only — never 0.0.0.0
PORT = 5000

BASE_DIR = Path(__file__).resolve().parent
CORE_PATH = BASE_DIR / ("core.exe" if os.name == "nt" else "core")

BRIDGE_TIMEOUT_SECONDS = 15   # how long Flask waits for a reply from core
PORT_WAIT_TIMEOUT_SECONDS = 10  # how long the launcher waits for Flask to be up

ALLOWED_CLIENTS = {"127.0.0.1", "::1"}

# --------------------------------------------------------------------------
# Bridge setup
# --------------------------------------------------------------------------
# Default runtime folder ("runtime") is used, matching PyCppBridge's default.
# Only one request/response round trip may be in flight at a time (the
# bridge is a single JSON-file channel), so every call is serialized
# through _bridge_lock.
bridge = Bridge(BASE_DIR / "runtime")
_bridge_lock = threading.Lock()


def call_core(command: str) -> str:
    """Send `payload` to core and return whatever core sends back.

    Raises TimeoutError if core does not answer within BRIDGE_TIMEOUT_SECONDS.
    """
    with _bridge_lock:
        bridge.send(command, "request")
        return bridge.receive("response", timeout=BRIDGE_TIMEOUT_SECONDS)


# --------------------------------------------------------------------------
# Flask app
# --------------------------------------------------------------------------
app = Flask(__name__)


@app.before_request
def restrict_to_localhost():
    # Defense in depth on top of binding to 127.0.0.1 below: reject
    # anything that isn't coming from this machine's loopback interface.
    if request.remote_addr not in ALLOWED_CLIENTS:
        abort(403)


@app.route("/")
def index():
    return render_template("fm-gui.html")


@app.route("/api/list")
def api_list():
    path = request.args.get("path", ".")
    try:
        output = call_core(f'ls "{path.replace(chr(34), chr(92) + chr(34))}"')
    except TimeoutError:
        return jsonify({"ok": False, "output": "core did not respond"}), 504
    entries = []
    for line in output.splitlines():
        if line.startswith("[DIR] "):
            entries.append({"name": line[6:], "type": "dir"})
        elif line.startswith("[FILE] "):
            entries.append({"name": line[7:], "type": "file"})
    return jsonify({"path": path, "entries": entries})


@app.route("/api/run", methods=["POST"])
def api_run():
    body = request.get_json(silent=True) or {}
    command = body.get("command", "")
    if not command:
        return jsonify({"ok": False, "output": "missing command"}), 400
    try:
        output = call_core(command)
    except TimeoutError:
        return jsonify({"ok": False, "output": "core did not respond"}), 504
    return jsonify({"ok": not output.startswith("Error:") and not output.startswith("Usage:"), "output": output})


# --------------------------------------------------------------------------
# Launching core
# --------------------------------------------------------------------------
_core_process = None


def _wait_for_port(host: str, port: int, timeout: float) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            with socket.create_connection((host, port), timeout=0.5):
                return True
        except OSError:
            time.sleep(0.1)
    return False


def _launch_core_once_server_is_up():
    global _core_process
    if not _wait_for_port(HOST, PORT, PORT_WAIT_TIMEOUT_SECONDS):
        print(f"[run.py] server never came up on {HOST}:{PORT}; not launching core", file=sys.stderr)
        return
    if not CORE_PATH.exists():
        print(f"[run.py] core binary not found at {CORE_PATH} — compile main.cpp first", file=sys.stderr)
        return
    _core_process = subprocess.Popen([str(CORE_PATH)], cwd=str(BASE_DIR))
    print(f"[run.py] launched core (pid {_core_process.pid})")


@atexit.register
def _stop_core():
    if _core_process and _core_process.poll() is None:
        _core_process.terminate()


if __name__ == "__main__":
    threading.Thread(target=_launch_core_once_server_is_up, daemon=True).start()

    # debug=False is deliberate: this app can end up triggering `oscmd`
    # (raw OS command execution) on the core side, so the last thing it
    # needs is Werkzeug's debugger/reloader exposed on top of that.
    app.run(host=HOST, port=PORT, debug=False, threaded=True)
