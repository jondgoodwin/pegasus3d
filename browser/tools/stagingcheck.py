#!/usr/bin/env python3
"""A fresh --connect must not outgrow the GPU's staging memory.

A client that joins is told the whole world at once (the server's replay log: every Object, mesh and material), several
times the 16 MB a frame's uploads can take (gpu's staging block doubles from 64 KB and gives up on its 9th), so the view
spreads what render is asked to upload over frames (viewactor.cone, UPLOAD_BUDGET). This starts one local server
(--serve --loopback; never another machine's) and connects N times in a row, each a real GPU run of a few seconds, and checks
in every one:

  * exit code 0, "Vulkan failures: 0", no "outgrew its staging memory"
  * the burst really was spread: an "Upload: a burst of ..." line (the burst was over the budget and held back)

Usage: stagingcheck.py [--exe PATH] [--connects N] [--seconds S] [--port P]
Exit code 0 if every connect holds.
"""
import argparse, os, re, subprocess, sys, tempfile, time

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_EXE = os.path.join(HERE, '..', 'build', 'release', 'browser.exe')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe', default=DEFAULT_EXE)
    ap.add_argument('--connects', type=int, default=10)
    ap.add_argument('--seconds', type=int, default=8)
    ap.add_argument('--port', type=int, default=40123)
    a = ap.parse_args()
    exe = os.path.abspath(a.exe)
    cwd = os.path.dirname(exe)
    tmp = tempfile.mkdtemp(prefix='stagingcheck')
    slog = os.path.join(tmp, 'server.log')
    server = subprocess.Popen([exe, '--serve', str(a.port), '--loopback', '--run-for', str(60 + a.connects * (a.seconds + 15))],
                              stdout=open(slog, 'w'), stderr=subprocess.STDOUT, cwd=cwd)
    ok = True
    try:
        end = time.time() + 120
        while time.time() < end:
            if 'Serving on UDP port' in open(slog, errors='replace').read():
                break
            time.sleep(0.25)
        else:
            print('FAIL: the server did not come up'); return 1
        time.sleep(3)   # (the world builds as it serves; the replay log is whole after a while)
        for n in range(1, a.connects + 1):
            log = os.path.join(tmp, 'client%d.log' % n)
            p = subprocess.run([exe, '--connect', '127.0.0.1:%d' % a.port, '--name', 'stg%d' % n, '--run-for', str(a.seconds)],
                               stdout=open(log, 'w'), stderr=subprocess.STDOUT, cwd=cwd, timeout=a.seconds + 60)
            text = open(log, errors='replace').read()
            fails = re.search(r'Vulkan failures: (\d+)', text)
            outgrew = 'outgrew its staging' in text
            spread = re.search(r'Upload: a burst of (\d+) MB \(estimated\) spread over (\d+) frames, done at frame (\d+), [\d.]+ ms after it began', text)
            good = p.returncode == 0 and fails and fails.group(1) == '0' and not outgrew and spread
            print('connect %2d: exit %d, Vulkan failures %s, %s' % (n, p.returncode, fails.group(1) if fails else '?',
                  spread.group(0)[len('Upload: '):] if spread else 'NO BURST HELD BACK'))
            if not good:
                ok = False
                print('  (log kept: %s)' % log)
    finally:
        server.terminate()
    print('stagingcheck: %s (%d connects)' % ('ok' if ok else 'FAILED', a.connects))
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
