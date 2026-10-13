#!/usr/bin/python3
# Pegasus3D request-to-play endpoint: listens on 127.0.0.1:8098 behind nginx (POST /play/request), runs as user pegreq.
# Body (text/plain, at most 1 KB): line 1 the name, line 2 an optional one-line message, line 3 an optional token.
# It appends the request to the list and answers; it executes nothing submitted. Only a valid token from a new address
# moves that player's own firewall rule, through the fixed root helper.
import hashlib, hmac, os, re, subprocess, time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

DIR = "/var/lib/pegasus-requests"
NAME_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9 ._-]{0,23}$")
IP_RE = re.compile(r"^\d{1,3}(\.\d{1,3}){3}$")
TOKEN_RE = re.compile(r"^[0-9a-f]{32}$")


def clean(s, n):
    return re.sub(r"[\x00-\x1f\x7f]", " ", s)[:n].strip()


def players():
    out = []
    try:
        for line in open(DIR + "/players.tsv"):
            p = line.rstrip("\n").split("\t")
            if len(p) == 4:
                out.append(p)
    except FileNotFoundError:
        pass
    return out


def log(ip, name, msg):
    with open(DIR + "/requests.log", "a") as f:
        f.write("%s\t%s\t%s\t%s\n" % (time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()), ip, name, msg))


class H(BaseHTTPRequestHandler):
    server_version = "pegreq"
    sys_version = ""

    def reply(self, code, text):
        b = (text + "\n").encode()
        self.send_response(code)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.send_header("Content-Length", str(len(b)))
        self.end_headers()
        self.wfile.write(b)

    def log_message(self, *a):
        pass

    def do_POST(self):
        if self.path != "/request":
            return self.reply(404, "not found")
        ip = self.headers.get("X-Real-IP", "")
        if not IP_RE.match(ip):
            return self.reply(400, "IPv4 only for now")
        try:
            n = int(self.headers.get("Content-Length", "-1"))
        except ValueError:
            n = -1
        if n < 0 or n > 1024:
            return self.reply(413, "body too large (1 KB at most)")
        try:
            lines = self.rfile.read(n).decode("utf-8").split("\n")
        except UnicodeDecodeError:
            return self.reply(400, "not UTF-8")
        lines += ["", "", ""]
        name, msg, token = lines[0].strip(), clean(lines[1], 200), lines[2].strip()
        if not NAME_RE.match(name):
            return self.reply(400, "a name is 1-24 letters, digits, spaces and . _ - ")
        rows = [r for r in players() if r[0].lower() == name.lower()]
        if token:
            if not TOKEN_RE.match(token) or not rows or not hmac.compare_digest(rows[0][1], hashlib.sha256(token.encode()).hexdigest()):
                log(ip, name, "[bad token]")
                return self.reply(403, "token not accepted")
            old = rows[0][2]
            if old == ip:
                return self.reply(200, "approved")
            r = subprocess.run(["/usr/bin/sudo", "-n", "/usr/local/sbin/pegasus-move-ip"],
                               input="%s %s %s" % (rows[0][0], ip, token), capture_output=True, text=True, timeout=20)
            ok = r.returncode == 0
            log(ip, name, "[token: moved from %s%s]" % (old, "" if ok else " FAILED: " + clean(r.stderr, 120)))
            return self.reply(200 if ok else 500, "approved; your address was updated" if ok else "could not update your address")
        # No token: never an approval. If this is the address that was approved, hand over the token once.
        claim = DIR + "/claims/" + name.lower().replace(" ", "_") + ".tok"
        if rows and os.path.exists(claim):
            cip, ctok = open(claim).read().split()
            if cip == ip:
                os.remove(claim)
                log(ip, name, "[token handed over]")
                return self.reply(200, "approved\ntoken " + ctok)
        log(ip, name, msg)
        self.reply(200, "approved" if rows and rows[0][2] == ip else "sent")


if __name__ == "__main__":
    ThreadingHTTPServer(("127.0.0.1", 8098), H).serve_forever()
