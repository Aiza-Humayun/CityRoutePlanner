

import atexit
import mimetypes
import os
import subprocess
import sys
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

# ----------------------------------------------------------------- settings
BRIDGE_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(BRIDGE_DIR)          # the folder that contains backend, data, web
WEB_DIR = os.path.join(PROJECT_ROOT, "web")
HOST = "127.0.0.1"                                  # only this computer can connect
PORT = 8000
READY_TIMEOUT_SECONDS = 10                          # how long we wait for the READY line
REPLY_TIMEOUT_SECONDS = 15                          # how long we wait for one engine reply
MAX_BODY_BYTES = 4096                               # a command line is tiny; refuse anything bigger

ENGINE_NOT_RUNNING = '{"ok":false,"error":"Engine not running"}'


def json_error(message):
    """Builds {"ok":false,"error":"..."} (used only for bridge-side errors)."""
    escaped = message.replace("\\", "\\\\").replace('"', '\\"')
    return '{"ok":false,"error":"' + escaped + '"}'


# ------------------------------------------------------------------ engine
class Engine:
    """Owns the C++ process and talks to it through its pipes."""

    def __init__(self):
        self.process = None
        self.lock = threading.Lock()                # one request at a time

    def find_executable(self):
        """Returns the path of the compiled engine, or None if it is missing."""
        for name in ("planner.exe", "planner"):     # .exe on Windows, no extension on Linux/Mac
            path = os.path.join(PROJECT_ROOT, "backend", name)
            if os.path.isfile(path):
                return path
        return None

    def start(self):
        """Starts the engine once and waits for READY. Returns True on success."""
        path = self.find_executable()
        if path is None:
            print("ERROR: backend/planner.exe not found. Build it first:")
            print("       g++ -std=c++17 -Wall -o backend/planner.exe backend/main.cpp")
            return False

        # cwd = project root, so the engine finds data/map.txt.
        # stderr is NOT piped: engine warnings show up in this console.
        self.process = subprocess.Popen(
            [path],
            cwd=PROJECT_ROOT,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            bufsize=1,                              # line buffered
        )

        first_line = self._read_line_with_timeout(READY_TIMEOUT_SECONDS)
        if first_line is None or '"READY"' not in first_line:
            print("ERROR: the engine did not print READY (got: %r)" % first_line)
            self.stop()
            return False
        print("Engine is ready.")
        return True

    def _read_line_with_timeout(self, seconds):
        """Reads one line from the engine; returns None if nothing arrives in time."""
        result = []

        def reader():
            try:
                result.append(self.process.stdout.readline())
            except (OSError, ValueError):
                result.append("")

        thread = threading.Thread(target=reader, daemon=True)
        thread.start()
        thread.join(seconds)
        if thread.is_alive():
            return None                             # timed out
        return result[0].strip() if result else None

    def is_running(self):
        return self.process is not None and self.process.poll() is None

    def send(self, command):
        """Sends one command line, returns the engine's one reply line (a string)."""
        command = command.strip()
        if command == "":
            return json_error("Empty command")
        if "\n" in command or "\r" in command:
            # A second line would be a second command and the replies would get out of step.
            return json_error("A command must be a single line")
        if command.upper() == "EXIT":
            # Would stop the engine for everybody. Only closing the server does that.
            return json_error("EXIT is not allowed from the web page")

        with self.lock:                             # two requests can never mix
            if not self.is_running():
                return ENGINE_NOT_RUNNING
            try:
                self.process.stdin.write(command + "\n")
                self.process.stdin.flush()
            except (OSError, ValueError):           # broken pipe: the engine died
                return ENGINE_NOT_RUNNING

            reply = self._read_line_with_timeout(REPLY_TIMEOUT_SECONDS)
            if reply is None:
                return json_error("Engine did not answer in time")
            if reply == "":
                return ENGINE_NOT_RUNNING           # end of output: the engine stopped
            return reply

    def stop(self):
        """Shuts the engine down politely (EXIT), then forcefully if needed."""
        if self.process is None:
            return
        try:
            if self.process.poll() is None:
                try:
                    self.process.stdin.write("EXIT\n")
                    self.process.stdin.flush()
                except (OSError, ValueError):
                    pass
                try:
                    self.process.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    self.process.kill()
        finally:
            self.process = None


engine = Engine()


# ------------------------------------------------------------- HTTP handler
class Handler(BaseHTTPRequestHandler):
    server_version = "CityRoutePlannerBridge/1.0"

    def log_message(self, format, *args):
        # Keep the console quiet: no line for every file request.
        pass

    # ---- helpers
    def send_bytes(self, status, content_type, body):
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def send_json_text(self, text, status=200):
        self.send_bytes(status, "application/json; charset=utf-8", text.encode("utf-8"))

    # ---- GET: serve the web page files
    def do_GET(self):
        path = self.path.split("?", 1)[0].split("#", 1)[0]
        if path == "/":
            path = "/index.html"

        # Build the file path inside web/ and make sure it cannot escape the folder (../).
        relative = path.lstrip("/").replace("/", os.sep)
        full = os.path.abspath(os.path.join(WEB_DIR, relative))
        inside = full == WEB_DIR or full.startswith(WEB_DIR + os.sep)
        if not inside or not os.path.isfile(full):
            self.send_bytes(404, "text/plain; charset=utf-8", b"404 Not found")
            return

        content_type = mimetypes.guess_type(full)[0] or "application/octet-stream"
        if content_type.startswith("text/") or content_type in ("application/javascript", "application/json"):
            content_type += "; charset=utf-8"
        with open(full, "rb") as f:
            self.send_bytes(200, content_type, f.read())

    # ---- POST /api: one command in, one JSON line out
    def do_POST(self):
        if self.path.split("?", 1)[0] != "/api":
            self.send_bytes(404, "text/plain; charset=utf-8", b"404 Not found")
            return

        try:
            length = int(self.headers.get("Content-Length", "0"))
        except ValueError:
            length = -1
        if length < 0 or length > MAX_BODY_BYTES:
            self.send_json_text(json_error("Request too large"), 413)
            return

        body = self.rfile.read(length)
        try:
            command = body.decode("utf-8")
        except UnicodeDecodeError:
            self.send_json_text(json_error("Command must be UTF-8 text"), 400)
            return

        reply = engine.send(command)
        self.send_json_text(reply)


# --------------------------------------------------------------------- main
def main():
    port = PORT
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            print("Usage: python bridge/server.py [port]")
            return 1

    if not engine.start():
        return 1
    atexit.register(engine.stop)                    # always close the engine when we exit

    if not os.path.isdir(WEB_DIR):
        print("Warning: the folder web/ does not exist, so the page cannot be shown.")

    try:
        server = ThreadingHTTPServer((HOST, port), Handler)
    except OSError as error:
        print("ERROR: cannot use port %d (%s). Is the server already running?" % (port, error))
        return 1
    server.daemon_threads = True

    print("Open  http://localhost:%d  in your browser. Press Ctrl+C to stop." % port)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        server.server_close()
        engine.stop()
    return 0


if __name__ == "__main__":
    sys.exit(main())
