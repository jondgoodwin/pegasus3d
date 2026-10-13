#!/usr/bin/python3
# Pegasus3D request-to-play: the root half (approve, revoke, move). Run through the wrappers play-approve, play-revoke and
# pegasus-move-ip. See browser/SERVER.md. Executes nothing it was given: input is validated, ufw gets an argument list.
import fcntl, hashlib, hmac, ipaddress, os, pwd, re, secrets, subprocess, sys, time

DIR = "/var/lib/pegasus-requests"
LOG = DIR + "/requests.log"          # time <TAB> ip <TAB> name <TAB> message
PLAYERS = DIR + "/players.tsv"       # name <TAB> sha256(token) <TAB> ip <TAB> approved-at
CLAIMS = DIR + "/claims"             # <name>.tok: the token waiting to be handed to the approved address once
PORT = "40000"
NAME_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9 ._-]{0,23}$")
BAD_NETS = [ipaddress.ip_network(n) for n in
            ("0.0.0.0/8", "10.0.0.0/8", "100.64.0.0/10", "127.0.0.0/8", "169.254.0.0/16", "172.16.0.0/12",
             "192.168.0.0/16", "224.0.0.0/4", "240.0.0.0/4")]
# Documentation ranges (192.0.2/24, 198.51.100/24, 203.0.113/24) are allowed on purpose: they are what the tests use.


def die(msg, code=1):
    print("error: " + msg, file=sys.stderr)
    sys.exit(code)


def check_name(n):
    if not NAME_RE.match(n):
        die("a name is 1-24 letters, digits, spaces and . _ - (first one not a space)")
    return n


def check_ip(s):
    if not re.match(r"^\d{1,3}(\.\d{1,3}){3}$", s):
        die("not a dotted IPv4 address: the server socket is IPv4 only")
    try:
        ip = ipaddress.ip_address(s)
    except ValueError:
        die("not an address")
    if any(ip in n for n in BAD_NETS):
        die("address %s is not a public address" % s)
    return str(ip)


def lock():
    f = open(DIR + "/.lock", "a")
    fcntl.flock(f, fcntl.LOCK_EX)
    return f


def claim_path(name):
    return CLAIMS + "/" + name.lower().replace(" ", "_") + ".tok"


def read_players():
    rows = []
    try:
        for line in open(PLAYERS):
            p = line.rstrip("\n").split("\t")
            if len(p) == 4:
                rows.append(p)
    except FileNotFoundError:
        pass
    return rows


def write_players(rows):
    tmp = PLAYERS + ".new"
    with open(tmp, "w") as f:
        for r in rows:
            f.write("\t".join(r) + "\n")
    os.chmod(tmp, 0o644)
    os.replace(tmp, PLAYERS)


def find(rows, name):
    for i, r in enumerate(rows):
        if r[0].lower() == name.lower():
            return i
    return -1


def ufw(*args):
    r = subprocess.run(["/usr/sbin/ufw"] + list(args), capture_output=True, text=True)
    if r.returncode != 0:
        die("ufw %s: %s" % (" ".join(args), (r.stdout + r.stderr).strip()))


def allow(ip, name):
    ufw("allow", "from", ip, "to", "any", "port", PORT, "proto", "udp", "comment", "pegasus3d player " + name)


def unallow(ip):
    ufw("delete", "allow", "from", ip, "to", "any", "port", PORT, "proto", "udp")


def request_ips(name):
    # the distinct addresses that asked as this name (a plain request, not a bad-token or bracketed event), oldest first
    ips = []
    try:
        for line in open(LOG):
            p = line.rstrip("\n").split("\t")
            if len(p) >= 4 and p[2].lower() == name.lower() and not p[3].startswith("[") and p[1] not in ips:
                ips.append(p[1])
    except FileNotFoundError:
        pass
    return ips


def approve(name, want=None):
    check_name(name)
    with lock():
        rows = read_players()
        if find(rows, name) >= 0:
            die("%s is already approved (play-revoke first to re-issue)" % name)
        ips = request_ips(name)
        if not ips:
            die("no request from %s in %s" % (name, LOG))
        if want is None and len(ips) > 1:
            die("%s asked from several addresses (%s): name the one to approve: play-approve NAME IP" % (name, ", ".join(ips)))
        ip = check_ip(want if want else ips[0])
        if ip not in ips:
            die("%s never asked from %s (asked from %s)" % (name, ip, ", ".join(ips)))
        token = secrets.token_hex(16)
        allow(ip, name)
        rows.append([name, hashlib.sha256(token.encode()).hexdigest(), ip, time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())])
        write_players(rows)
        os.makedirs(CLAIMS, exist_ok=True)
        u = pwd.getpwnam("pegreq")
        os.chown(CLAIMS, u.pw_uid, u.pw_gid)
        c = claim_path(name)
        with open(c, "w") as f:
            f.write(ip + " " + token + "\n")
        os.chown(c, u.pw_uid, u.pw_gid)
        os.chmod(c, 0o600)
    print("approved %s from %s: ufw allow UDP %s; the token goes to that address on its next request" % (name, ip, PORT))


def revoke(name):
    check_name(name)
    with lock():
        rows = read_players()
        i = find(rows, name)
        if i < 0:
            die("%s is not an approved player" % name)
        unallow(rows[i][2])
        del rows[i]
        write_players(rows)
        c = claim_path(name)
        if os.path.exists(c):
            os.remove(c)
    print("revoked %s" % name)


def move():
    # stdin: one line "name ip token". Nothing else is trusted: the token is checked here against the stored hash.
    parts = sys.stdin.read(256).strip().split(" ")
    if len(parts) < 3:
        die("expected: name ip token")
    token, ip, name = parts[-1], parts[-2], " ".join(parts[:-2])
    check_name(name)
    ip = check_ip(ip)
    if not re.match(r"^[0-9a-f]{32}$", token):
        die("bad token form")
    with lock():
        rows = read_players()
        i = find(rows, name)
        if i < 0 or not hmac.compare_digest(rows[i][1], hashlib.sha256(token.encode()).hexdigest()):
            die("no such player or wrong token", 3)
        old = rows[i][2]
        if old == ip:
            print("unchanged")
            return
        allow(ip, rows[i][0])
        unallow(old)
        rows[i][2] = ip
        write_players(rows)
    print("moved %s from %s to %s" % (name, old, ip))


if __name__ == "__main__":
    os.umask(0o022)
    if os.geteuid() != 0:
        die("run through sudo")
    cmd = sys.argv[1] if len(sys.argv) > 1 else ""
    if cmd == "approve" and len(sys.argv) in (3, 4):
        approve(sys.argv[2], sys.argv[3] if len(sys.argv) == 4 else None)
    elif cmd == "revoke" and len(sys.argv) == 3:
        revoke(sys.argv[2])
    elif cmd == "move" and len(sys.argv) == 2:
        move()
    else:
        die("usage: players.py approve NAME | revoke NAME | move (stdin)")
