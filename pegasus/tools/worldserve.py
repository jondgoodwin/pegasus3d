"""A local file server that serves a world's files over a slowed link, for arrival tests.

    python worldserve.py ROOT_DIR [--port N] [--mbit RATE] [--latency-ms MS] [--delay NAME=MS ...] [--tls CERT.pem KEY.pem]

--tls serves https:// (HTTP/1.1 over TLS) with the given certificate, for the loopback tests of the https path; pegasus trusts a
self-signed one only through its test switch PEGASUS_TEST_TRUST_CA (see tools\selfsigned.ps1 for making one).
Binds 127.0.0.1 only. GET /PATH answers the file ROOT_DIR/PATH (HTTP/1.1, keep-alive, Content-Length). The link is ONE pipe
shared by every connection: the bytes of all responses together go at --mbit megabits a second (0: no limit), so asking for
files in parallel does not make the link faster. --latency-ms is added once before each response's first byte (the time a
request takes to get there and back on a link of that round trip). Prints "serving ROOT on PORT" when it is ready, and one line
per request to stderr. Stop it by its process id.
"""
import os
import sys
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

CHUNK = 1460


class Link:
    """The shared pipe: each chunk reserves its time on it, in the order asked."""

    def __init__(self, mbit):
        self.bytesPerSecond = mbit * 1_000_000 / 8 if mbit > 0 else 0
        self.free = time.monotonic()
        self.lock = threading.Lock()

    def carry(self, n):
        if not self.bytesPerSecond:
            return
        with self.lock:
            now = time.monotonic()
            start = max(now, self.free)
            self.free = start + n / self.bytesPerSecond
            wait = self.free - now
        if wait > 0:
            time.sleep(wait)


def main():
    args = sys.argv[1:]
    port, mbit, latency = 0, 0.0, 0.0
    for flag, name in (("--port", "port"), ("--mbit", "mbit"), ("--latency-ms", "latency")):
        if flag in args:
            i = args.index(flag)
            value = args[i + 1]
            del args[i:i + 2]
            if name == "port":
                port = int(value)
            elif name == "mbit":
                mbit = float(value)
            else:
                latency = float(value)
    tls = None
    if "--tls" in args:
        i = args.index("--tls")
        tls = (args[i + 1], args[i + 2])
        del args[i:i + 3]
    delays = {}                       # --delay NAME=MS (repeatable): a file whose response starts MS later than the rest
    while "--delay" in args:
        i = args.index("--delay")
        name, _, ms = args[i + 1].partition("=")
        delays[name] = float(ms)
        del args[i:i + 2]
    root = os.path.abspath(args[0])
    link = Link(mbit)

    class Handler(BaseHTTPRequestHandler):
        protocol_version = "HTTP/1.1"

        def do_GET(self):
            path = os.path.normpath(os.path.join(root, self.path.split("?")[0].lstrip("/")))
            if not path.startswith(root) or not os.path.isfile(path):
                self.send_response(404)
                self.send_header("Content-Length", "0")
                self.end_headers()
                return
            with open(path, "rb") as f:
                body = f.read()
            if latency > 0:
                time.sleep(latency / 1000.0)
            late = delays.get(os.path.basename(path), 0.0)
            if late > 0:
                time.sleep(late / 1000.0)
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            at = 0
            while at < len(body):
                piece = body[at:at + CHUNK]
                link.carry(len(piece))
                self.wfile.write(piece)
                at += len(piece)
            self.wfile.flush()

        def log_message(self, fmt, *a):
            sys.stderr.write("%.3f GET %s\n" % (time.monotonic(), self.path))

    server = ThreadingHTTPServer(("127.0.0.1", port), Handler)
    server.daemon_threads = True
    if tls:
        import ssl
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        ctx.load_cert_chain(tls[0], tls[1])
        server.socket = ctx.wrap_socket(server.socket, server_side=True)
    print("serving %s on %d" % (root, server.server_address[1]), flush=True)
    server.serve_forever()


main()
