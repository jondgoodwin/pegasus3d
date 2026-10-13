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
Exit code 0 if every check holds.
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
    ap.add_argument('--out', default=os.path.join(HERE, '..', '..', 'netcheck-out'))
    args = ap.parse_args()
    exe = os.path.abspath(args.exe)
    os.makedirs(args.out, exist_ok=True)
    tag = 'impaired' if args.impair else 'loopback'
    slog = os.path.join(args.out, f'{tag}-server.log')
    alog = os.path.join(args.out, f'{tag}-clientA.log')
    blog = os.path.join(args.out, f'{tag}-clientB.log')

    server_cmd = [exe, '--serve', str(args.port), '--loopback', '--run-for', str(args.seconds + 50), '--trace']
    if args.impair:
        server_cmd += ['--impair'] + [str(v) for v in args.impair]
    procs = []
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
        time.sleep(11)
        common = [exe, '--headless', '--connect', f'127.0.0.1:{port}', '--trace', '--run-for', str(args.seconds)]
        b, f3 = start(common, blog)
        procs.append((b, f3))
        a, f2 = start(common + ['--autowalk'], alog)
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
    ticks, pos = path[a_being]
    # The first 3 s after B first drew anything are not measured: B is building the world (one frame of seconds) and
    # its clock estimate is settling
    q0 = tv[0][1] + 3 * (freq or 10000000.0) if tv else 0
    errs = []
    lags = []
    moved = 0.0
    last = None
    for at, qpc, p in tv:
        if qpc < q0 or at < ticks[0] or at > ticks[-1]:
            continue
        sp = interp(ticks, pos, at)
        errs.append(((p[0] - sp[0]) ** 2 + (p[1] - sp[1]) ** 2 + (p[2] - sp[2]) ** 2) ** 0.5)
        if freq and wall[0] and wall[0][0] <= qpc <= wall[0][-1]:
            tick_now = interp(wall[0], wall[1], qpc)[0]
            lags.append(tick_now - at)
        if last is not None:
            moved += ((p[0] - last[0]) ** 2 + (p[2] - last[2]) ** 2) ** 0.5
        last = p
    check(bool(errs), f'B has {len(errs)} measured samples of A, who moved {moved:.1f} m')
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
        vq0 = vtv[0][1] + 3 * (freq or 10000000.0) if vtv else 0
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
