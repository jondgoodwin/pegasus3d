# The island server (Linux)

`browser --serve PORT` is the same program as the browser with no window, GPU or sound: the simulation, the world's
code, and every connected browser's mannequin. On the VM `jondgoodwin-us` (Ubuntu 24.04, 40.87.21.70) it runs as the
systemd service `pegasus3d-server` on **UDP 40000**, as user `ubuntu`. There is no login: a client says a name and the
server refuses one already in use (case ignored), one with anything but letters, digits, spaces and `. _ -`, one over 24
letters, and a ninth player. Nothing is encrypted.

## Where things are on the VM

| What | Where |
| --- | --- |
| Cone compiler (Cone master `5da803c6`, shared libLLVM link, so it needs 104 MB to link, not 854) | `~/pegasus/cone/build/x64-release/conec`, with `libconestd.a` beside it; its `CMakeLists.txt` has the one change (`target_link_libraries(conec LLVM m)`; the original is `CMakeLists.txt.orig`). Not the playground's: that is `~/play/` and its container. |
| Server source, as sent by `deploy.sh` | `~/pegasus/src/browser` |
| Program and its libSDL3 stand-in | `~/pegasus/bin/pegasus-server`, `~/pegasus/bin/libSDL3.so` |
| The unit | `/etc/systemd/system/pegasus3d-server.service` (a copy is `server/pegasus3d-server.service`) |

## Building the server for Linux

The browser's packages name SDL3 at link time, and Ubuntu 24.04 has no SDL3. The server uses five of its functions (init
for events, the clock, the event poll for a quit, quit) and none of its windows, displays, Vulkan or audio, so it links
a tiny stand-in, `server/sdl3shim.c` (`libSDL3.so`: the five implemented, the rest answering "failed" with a reason).
A signal (SIGTERM, SIGINT) becomes SDL's quit event, so `systemctl stop` ends the run in order. Nothing in the program
changed for this. The rest of the browser compiles on Linux as it is.

```
# from the browser/ folder on the laptop (Git Bash); sends the committed source, builds on the VM, restarts the service
server/deploy.sh                 # or: server/deploy.sh --no-restart
```

By hand on the VM: `cc -O2 -fPIC -shared -o ~/pegasus/bin/libSDL3.so server/sdl3shim.c`, then in `~/pegasus/src/browser`
`LIBRARY_PATH=$HOME/pegasus/bin python3 ~/pegasus/cone/tools/congo/congo.py build --release`, and copy
`build/release/browser` to `~/pegasus/bin/pegasus-server`. A new Cone master for the compiler: replace `~/pegasus/cone`
the way `~/PLAYGROUND.md` says to for `~/play/src` (archive, change the one CMake line, `cmake` and `ninja`, about 2
minutes), and rebuild the server with it.

## Running it

```
sudo systemctl status pegasus3d-server
sudo systemctl restart pegasus3d-server        # the world takes about 10 s to build; clients may connect after that
sudo systemctl stop pegasus3d-server
journalctl -u pegasus3d-server -f              # the log (joins, leaves, refusals, what each client was sent)
journalctl -u pegasus3d-server --since today
```

It restarts on failure after 5 s, runs at nice 5 and may not pass 1.5 GB (it uses about 100 MB). Starting it builds the
world in about 9 s of one core; idle it uses about 3% of a core.

## Connecting

The port is open only to the addresses in the Azure rule `allow-pegasus3d-udp` and the VM's ufw. **Adding a player means
adding their address to both** (the owner's decision each time). From an allowed address:

```
browser.exe --connect 40.87.21.70:40000 --name Jon
```

`--name` defaults to the machine's name; two clients on one machine need different names. `HOST` must be a dotted address
or `localhost` (there is no name lookup). A refused name prints the reason and exits with code 3. The other players' names
float over their mannequins (the HUD's plates, `H` hides the HUD).

## Testing against the server

`tools/netcheck.py` runs a server and two headless clients on 127.0.0.1. Against the VM it needs the server's trace, which
the service does not print, so stop the service for the length of the test:

```
# on the VM: stop the service, start a traced server that ends itself
sudo systemctl stop pegasus3d-server
cd ~/pegasus/run && LD_LIBRARY_PATH=$HOME/pegasus/bin nohup ../bin/pegasus-server --serve 40000 --trace --run-for 50 > trace.log 2>&1 &
# on the laptop, 12 s later (the world has to build): two clients here, the checks against the log copied from there
python tools/netcheck.py --connect 40.87.21.70:40000 --server-log server-trace.log --seconds 28
scp ubuntu@40.87.21.70:pegasus/run/trace.log server-trace.log     # once the server has ended
# on the VM again
sudo systemctl start pegasus3d-server
```
The script waits for the log to appear and to hold the end of the run. The check of how far in the past B draws A is
skipped (it compares two machines' clocks). `--reuse` re-runs the checks on logs already made.
