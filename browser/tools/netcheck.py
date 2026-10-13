#!/usr/bin/env python3
"""Two headless clients on one server: client A walks a script, client B watches.

Runs `browser.exe --serve`, then two `browser.exe --headless --connect` processes (all on this machine, over
127.0.0.1, or through the server's impairing relay with --impair), and checks, from the traces they print
(--trace):

  * A's own predicted position agrees with the server's after reconciliation: at every snapshot, the difference
    between the position A predicted after the last input the server applied and the server's (TR lines), and
    A's final position against the server's (FINAL lines)
  * B's view of A's mannequin follows A's server path: each position B drew for A (TV lines), at the server tick
    B was drawing, against the server's own path for A's being at that tick (TS lines), within --tolerance metres
    (after B has had 3 s of snapshots)
  * how far in the past B draws A, from the wall clock (the performance counter both sides print)
  * the bandwidth the server used for each client, and the client's own count of the wire

Usage: netcheck.py [--exe PATH] [--impair LOSS_PER_1000 LATENCY_MS JITTER_MS] [--seconds N] [--tolerance M]
                   [--connect HOST:PORT --server-log FILE]
Exit code 0 if every check holds.

AGAINST A SERVER ON ANOTHER MACHINE (--connect HOST:PORT --server-log FILE): the script starts no server. Start one
there yourself with `--serve PORT --trace --run-for N` (see SERVER.md) and let it build the world; run this script;
the server ends by its --run-for and the script waits for its log at FILE (copy it there, e.g. with scp). The two
clients run here. The check that needs the server's and the clients' clocks to be one clock (how far in the past B
draws) is skipped.
"""
import argparse, os, re, subprocess, sys, time, threading

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_EXE = os.path.join(HERE, '..', 'build', 'release', 'browser.exe')


def start(cmd, logname):
    log = open(logname, 'w')
    p = subprocess.Popen(cmd, stdout=log, stderr=subprocess.STDOUT, cwd=os.path.dirname(os.path.abspath(cmd[0])))
    return p, log


def wait_for(logname, pattern, timeout):
    rx = re.compile(pattern)
    end = time.time() + timeout
    while time.time() < end:
        try:
            text = open(logname, errors='replace').read()
        except OSError:
            text = ''
        m = rx.search(text)
        if m:
            return m
        time.sleep(0.25)
    return None


def interp(xs, ys, x):
    """Linear interpolation of ys over sorted xs at x (clamped)."""
    import bisect
    i = bisect.bisect_right(xs, x)
    if i <= 0:
        return ys[0]
    if i >= len(xs):
        return ys[-1]
    x0, x1 = xs[i - 1], xs[i]
    t = (x - x0) / (x1 - x0) if x1 > x0 else 0.0
    return tuple(a + (b - a) * t for a, b in zip(ys[i - 1], ys[i]))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe', default=DEFAULT_EXE)
    ap.add_argument('--impair', nargs=3, type=int, metavar=('LOSS', 'LATENCY', 'JITTER'))
    ap.add_argument('--seconds', type=int, default=28, help='how long each client runs')
    ap.add_argument('--tolerance', type=float, default=0.3, help='metres B may differ from the server path')
    ap.add_argument('--predict-tolerance', type=float, default=0.05, help='metres A\'s final position may differ')
    ap.add_argument('--port', type=int, default=7777)
    ap.add_argument('--lab', action='store_true',
                    help='the server sets each joiner down in the labyrinth\'s antechamber (--lab-spawn), 300 m under the island: '
                         'the same checks, and B must see A there')
    ap.add_argument('--portal', action='store_true', help='A runs through the cottage\'s looking-glass into the labyrinth and back (--portal-walk) '
                    'instead of the island walk; the portal is checked, the playground is not')
    ap.add_argument('--connect', metavar='HOST:PORT', help='a server already running on another machine (needs --server-log)')
    ap.add_argument('--server-log', metavar='FILE', help='with --connect: where the remote server\'s --trace log will be')
    ap.add_argument('--reuse', action='store_true', help='with --connect: run no clients; check the logs of a run already made')
    ap.add_argument('--settle', type=int, default=11, help='seconds to wait for a local server to build the world')
    ap.add_argument('--out', default=os.path.join(HERE, '..', '..', 'netcheck-out'))
    args = ap.parse_args()
    exe = os.path.abspath(args.exe)
    os.makedirs(args.out, exist_ok=True)
    remote = bool(args.connect)
    if remote and not args.server_log:
        print('FAIL: --connect needs --server-log'); return 2
    tag = 'remote' if remote else ('impaired' if args.impair else 'loopback')
    slog = args.server_log if remote else os.path.join(args.out, f'{tag}-server.log')
    alog = os.path.join(args.out, f'{tag}-clientA.log')
    blog = os.path.join(args.out, f'{tag}-clientB.log')

    procs = []
    server = None
    if remote and args.reuse:
        pass    # the clients' logs and the server's are already there: only the checks are run
    elif remote:
        common = [exe, '--headless', '--connect', args.connect, '--trace', '--run-for', str(args.seconds)]
        try:
            b, f3 = start(common + ['--name', 'netcheck-b'], blog)
            procs.append((b, f3))
            a, f2 = start(common + ['--name', 'netcheck-a', '--portal-walk' if args.portal else '--autowalk'], alog)
            procs.append((a, f2))
            a.wait(timeout=args.seconds + 60)
            b.wait(timeout=60)
        finally:
            for p, f in procs:
                if p.poll() is None:
                    p.kill()
                f.close()
        print(f'clients ended; waiting up to 300 s for the server log at {slog} to hold the end of the run')
        if not wait_for(slog, r'Server: \d+ ticks', 300):
            print('FAIL: the server log is missing or has not ended'); return 1
    else:
        server_cmd = [exe, '--serve', str(args.port), '--loopback', '--run-for', str(args.seconds + 50), '--trace']
        if args.impair:
            server_cmd += ['--impair'] + [str(v) for v in args.impair]
        if args.lab:
            server_cmd += ['--lab-spawn']
        server, f1 = start(server_cmd, slog)
        procs.append((server, f1))
        try:
            if not wait_for(slog, r'Serving on UDP port', 60):
                print('FAIL: the server did not start'); return 1
            port = args.port
            if args.impair:
                m = wait_for(slog, r'Relay: port (\d+)', 10)
                if not m:
                    print('FAIL: the relay did not start'); return 1
                port = int(m.group(1))
            # Let the server build the world before the clients ask for it
            time.sleep(args.settle)
            common = [exe, '--headless', '--connect', f'127.0.0.1:{port}', '--trace', '--run-for', str(args.seconds)]
            b, f3 = start(common + ['--name', 'netcheck-b'], blog)
            procs.append((b, f3))
            a, f2 = start(common + ['--name', 'netcheck-a', '--portal-walk' if args.portal else '--autowalk'], alog)
            procs.append((a, f2))
            a.wait(timeout=args.seconds + 60)
            b.wait(timeout=60)
            server.wait(timeout=args.seconds + 60)
        finally:
            for p, f in procs:
                if p.poll() is None:
                    p.kill()
                f.close()

    ok = True
    def check(cond, text):
        nonlocal ok
        print(('ok   ' if cond else 'FAIL ') + text)
        ok = ok and cond

    S = open(slog, errors='replace').read().splitlines()
    A = open(alog, errors='replace').read().splitlines()
    B = open(blog, errors='replace').read().splitlines()

    # The server's path: tick -> position for each being, and tick <-> wall clock
    path = {}      # being -> (ticks, positions)
    wall = ([], [])
    freq = None
    for line in S:
        if line.startswith('QPCFREQ'):
            freq = float(line.split()[1])
        elif line.startswith('TS '):
            _, tick, qpc, being, x, y, z, applied = line.split()
            tick, qpc, being = int(tick), int(qpc), int(being)
            ticks, pos = path.setdefault(being, ([], []))
            ticks.append(tick); pos.append((int(x) / 1000.0, int(y) / 1000.0, int(z) / 1000.0))
            if being == min(path):
                wall[0].append(qpc); wall[1].append((float(tick),))
    check(len(path) >= 2, f'the server saw {len(path)} client beings (want 2)')
    if len(path) < 2:
        return 1
    fa0 = [l.split() for l in A if l.startswith('FINAL ')]
    fb0 = [l.split() for l in B if l.startswith('FINAL ')]
    if not fa0 or not fb0:
        check(False, 'both clients ended and said which being was theirs')
        return 1
    a_being = int(fa0[-1][1])
    b_being = int(fb0[-1][1])

    # A's prediction (the first 2 seconds, while the client is still building the world and the server waits for it, are not counted)
    tr = [l.split() for l in A if l.startswith('TR ') and int(l.split()[1]) >= 120]
    errs = [int(t[3]) for t in tr]
    wrong = sum(1 for t in tr if t[4] == '1')
    if args.portal:
        # The one set-down (the server puts A before the glass: 170 m or so from where it was) is not a misprediction; a crossing the
        # prediction did not make with the server would be, by 300 m
        big = [e for e in errs if e > 100000]
        check(len(big) <= 1, f'A\'s prediction is out by more than 100 m {len(big)} time(s) (want at most the one set-down)')
        errs = [e for e in errs if e <= 100000]
    check(bool(errs), f'A compared its prediction with the server {len(tr)} times')
    if errs:
        print(f'     A: worst difference at a snapshot {max(errs)/1000.0:.3f} m, mean {sum(errs)/len(errs)/1000.0:.4f} m, '
              f'{wrong} corrections of {len(errs)} snapshots')
        check(max(errs) / 1000.0 <= args.predict_tolerance, f'A\'s prediction stays within {args.predict_tolerance} m of the server\'s at every snapshot')
    fa = [l.split() for l in A if l.startswith('FINAL ')]
    fs = [l.split() for l in S if l.startswith('FINAL ')]
    if fa and fs:
        ax, ay, az = (int(v) / 1000.0 for v in fa[-1][2:5])
        sx = [tuple(int(v) / 1000.0 for v in l[2:5]) for l in fs if int(l[1]) == a_being]
        if sx:
            d = ((ax - sx[0][0]) ** 2 + (ay - sx[0][1]) ** 2 + (az - sx[0][2]) ** 2) ** 0.5
            check(d <= args.predict_tolerance, f'A\'s final position ({ax:.3f}, {az:.3f}) is {d:.3f} m from the server\'s ({sx[0][0]:.3f}, {sx[0][2]:.3f})')

    # B's view of A
    tv = []
    for l in B:
        if l.startswith('TV '):
            _, at, qpc, being, x, y, z = l.split()
            if int(being) == a_being:
                tv.append((int(at) / 1000.0, int(qpc), (int(x) / 1000.0, int(y) / 1000.0, int(z) / 1000.0)))
    check(len(tv) > 600, f'B drew A {len(tv)} times')
    if args.lab and tv:
        ys = sorted(p[1] for _, _, p in tv)
        check(ys[0] < -299.0 and ys[-1] < -298.0 and ys[len(ys) // 2] < -299.0,
              f'B drew A in the labyrinth: heights {ys[0]:.2f} to {ys[-1]:.2f} m, median {ys[len(ys) // 2]:.2f} (want about -300)')
    ticks, pos = path[a_being]
    # The first 3 s after B first drew anything are not measured: B is building the world (one frame of seconds) and
    # its clock estimate is settling
    # (B's own clock: when the server is on another machine, its counter's frequency is not B's, so B says its own)
    cfreq = freq
    if remote:
        cfreq = None
        for l in B:
            if l.startswith('QPCFREQ'):
                cfreq = float(l.split()[1])
    q0 = tv[0][1] + 3 * (cfreq or 10000000.0) if tv else 0
    errs = []
    lags = []
    moved = 0.0
    last = None
    # A body that goes through a portal's gate jumps 300 m in one tick (the portal's whole point). B draws it a little late and takes the later
    # snapshot across the jump, so it and the server's path part company for a few ticks either side of it: those are not measured (and said)
    jumps = [ticks[i] for i in range(1, len(ticks))
             if ((pos[i][0] - pos[i - 1][0]) ** 2 + (pos[i][1] - pos[i - 1][1]) ** 2 + (pos[i][2] - pos[i - 1][2]) ** 2) ** 0.5 > 50.0]
    skipped = 0
    for at, qpc, p in tv:
        if qpc < q0 or at < ticks[0] or at > ticks[-1]:
            continue
        if any(abs(at - j) < 8 for j in jumps):
            skipped += 1
            continue
        sp = interp(ticks, pos, at)
        errs.append(((p[0] - sp[0]) ** 2 + (p[1] - sp[1]) ** 2 + (p[2] - sp[2]) ** 2) ** 0.5)
        if not remote and freq and wall[0] and wall[0][0] <= qpc <= wall[0][-1]:
            tick_now = interp(wall[0], wall[1], qpc)[0]
            lags.append(tick_now - at)
        if last is not None:
            moved += ((p[0] - last[0]) ** 2 + (p[2] - last[2]) ** 2) ** 0.5
        last = p
    check(bool(errs), f'B has {len(errs)} measured samples of A, who moved {moved:.1f} m')
    if jumps:
        print(f'     A went through a gate {len(jumps)} time(s) (a jump of 50 m or more in the server\'s path); {skipped} of B\'s samples within 8 ticks of one are not measured')
    if errs:
        errs.sort()
        print(f'     B vs the server path of A: worst {errs[-1]:.3f} m, 95th percentile {errs[int(len(errs)*0.95)]:.3f} m, mean {sum(errs)/len(errs):.3f} m')
        check(errs[-1] <= args.tolerance, f'B follows A\'s server path within {args.tolerance} m')
    if lags:
        lags.sort()
        mean = sum(lags) / len(lags)
        print(f'     B draws A about {mean/60*1000:.0f} ms in the past (the server\'s tick now less the one drawn, by the wall clock: '
              f'{lags[0]:.1f} to {lags[-1]:.1f} ticks, median {lags[len(lags)//2]:.1f})')

    # The villagers (npcs.cone), the simulation's own beings: B's drawing of each (TV) against the server's path for it (TN)
    npath = {}
    for line in S:
        if line.startswith('TN '):
            _, tick, qpc, being, x, y, z = line.split()
            ticks, pos = npath.setdefault(int(being), ([], []))
            ticks.append(int(tick)); pos.append((int(x) / 1000.0, int(y) / 1000.0, int(z) / 1000.0))
    check(len(npath) >= 1, f'the server walked {len(npath)} villagers')
    for vb in sorted(npath):
        vticks, vpos = npath[vb]
        vtv = []
        for l in B:
            if l.startswith('TV '):
                _, at, qpc, being, x, y, z = l.split()
                if int(being) == vb:
                    vtv.append((int(at) / 1000.0, int(qpc), (int(x) / 1000.0, int(y) / 1000.0, int(z) / 1000.0)))
        vq0 = vtv[0][1] + 3 * (cfreq or 10000000.0) if vtv else 0
        verrs = []
        vmoved = 0.0
        vlast = None
        for at, qpc, p in vtv:
            if qpc < vq0 or at < vticks[0] or at > vticks[-1]:
                continue
            sp = interp(vticks, vpos, at)
            verrs.append(((p[0] - sp[0]) ** 2 + (p[1] - sp[1]) ** 2 + (p[2] - sp[2]) ** 2) ** 0.5)
            if vlast is not None:
                vmoved += ((p[0] - vlast[0]) ** 2 + (p[2] - vlast[2]) ** 2) ** 0.5
            vlast = p
        if verrs:
            verrs.sort()
            sm = sum(((vpos[i][0] - vpos[i - 1][0]) ** 2 + (vpos[i][2] - vpos[i - 1][2]) ** 2) ** 0.5 for i in range(1, len(vpos)))
            print(f'     villager {vb}: the server walked it {sm:.1f} m; B drew it {len(verrs)} times, {vmoved:.1f} m, worst {verrs[-1]:.3f} m from the '
                  f'server\'s path, 95th percentile {verrs[int(len(verrs)*0.95)]:.3f} m')
            check(verrs[-1] <= args.tolerance, f'B follows villager {vb}\'s server path within {args.tolerance} m')
        else:
            check(False, f'B drew villager {vb}')

    # The portal (--portal): A asks at tick 200 to be put before the cottage's glass, which is opened for it; it runs through, into the room, turns,
    # and runs back out. The server counts the gates gone through; A's own prediction counted the same (and agreed, above: the TR lines); B was
    # told the glass opened and saw the thread laid
    if args.portal:
        sp = [l.split() for l in S if l.startswith('PORTAL ')]
        ap_ = [l.split() for l in A if l.startswith('PORTAL ')]
        bp = [l.split() for l in B if l.startswith('PORTAL ')]
        check(bool(sp) and int(sp[-1][1]) == 1, 'the server opened the glass')
        if sp:
            check(int(sp[-1][2]) >= 2, f'the server counted {sp[-1][2]} gates gone through (want at least 2: in and out)')
            check(int(sp[-1][3]) >= 2, f'the server laid {sp[-1][3]} thread points (want at least E and the end)')
        check(bool(ap_) and int(ap_[-1][1]) == 1 and int(ap_[-1][2]) >= 2, f'A\'s own prediction crossed as the server did ({ap_[-1][2] if ap_ else "no report"} gates)')
        check(bool(bp) and int(bp[-1][1]) == 1 and int(bp[-1][3]) >= 2,
              f'B was told the glass opened and shown the thread ({bp[-1][3] if bp else "no report"} points)')
        print('PASS' if ok else 'FAIL')
        return 0 if ok else 1

    # The playground (props.cone, spellsim.cone): A bowls at tick 600 and throws paint at 900; the server played both, and B saw the ball
    # where the server has it and was told of the paint
    sp = [l.split() for l in S if l.startswith('PLAY ')]
    bp = [l.split() for l in B if l.startswith('PLAY ')]
    ap_ = [l.split() for l in A if l.startswith('PLAY ')]
    check(bool(sp) and int(sp[0][1]) == 1 and int(sp[0][2]) == 1, 'the server played A\'s one bowl and one paintball')
    if sp and bp:
        props, bursts = int(bp[-1][1]), int(bp[-1][2])
        bx, by, bz = (int(v) / 1000.0 for v in bp[-1][4:7])
        sx, sy, sz = (int(v) / 1000.0 for v in sp[0][4:7])
        d = ((bx - sx) ** 2 + (by - sy) ** 2 + (bz - sz) ** 2) ** 0.5
        check(props == 11, f'B has all 11 props once the ball has moved ({props})')
        check(d <= 0.05, f'B has the ball at ({bx:.3f}, {bz:.3f}), {d:.3f} m from the server\'s ({sx:.3f}, {sz:.3f})')
        if int(sp[0][3]) > 0:
            check(bursts >= 1, f'B was told of the paint ({bursts} burst(s); the server had {sp[0][3]} splat(s))')
    else:
        check(False, 'the server and B both reported the playground')
    if ap_:
        check(int(ap_[-1][1]) == 11, f'A has all 11 props too ({ap_[-1][1]})')

    # Bandwidth
    secs = args.seconds
    for line in S:
        if line.startswith('  sent it:'):
            print('     server' + line[1:])
    for name, lines in (('A', A), ('B', B)):
        for l in lines:
            if l.startswith('NET '):
                m = re.search(r'rtt (\d+) ms, loss (\d+) per 1000; received (\d+) packets, (\d+) bytes; sent (\d+) packets, (\d+) bytes', l)
                if m:
                    rtt, loss, pr, br, ps, bs = (int(v) for v in m.groups())
                    print(f'     client {name}: rtt {rtt} ms; wire in {br} bytes ({br/secs/1024:.1f} KiB/s over the run, {pr} packets), out {bs} bytes ({bs/secs/1024:.2f} KiB/s, {ps} packets)')
    for name, lines in (('A', A), ('B', B)):
        for l in lines:
            if l.startswith('Link:'):
                print(f'     client {name}: {l}')
    for l in S:
        if l.startswith('Server:') or l.startswith('  a snapshot averages'):
            print('     ' + l.strip())
    print('PASS' if ok else 'FAIL')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
