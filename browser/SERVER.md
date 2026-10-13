# The island server (Linux)

`browser --serve PORT` is the same program as the browser with no window, GPU or sound: the simulation, the world's
code, and every connected browser's mannequin. On the VM `jondgoodwin-us` (Ubuntu 24.04, 40.87.21.70) it runs as the
systemd service `pegasus3d-server` on **UDP 40000**, as user `ubuntu`. There is no login: a client says a name and the
server refuses one already in use (case ignored), one with anything but letters, digits, spaces and `. _ -`, one over 24
letters, and a ninth player. Nothing is encrypted.

**Protocol version.** `PROTOCOL_VERSION` (netcodec.cone) must be bumped whenever the wire format changes (a message's
fields, an encoding's range or origin such as `POS_ORIGIN_Y`, a new message). A client sends it after its name in `Hello`; a
server sends its own after the being in `Possessed`. A build from before versions existed sends none and reads as version 0.
A server refuses a client whose version differs ("Your browser is out of date. Please download the latest from
cone.jondgoodwin.com." for an older client, "The server is being updated. Please try again shortly." for a newer one); a
client refuses a server whose `Possessed` carries a different version, with the same two messages. Deploy the server and
publish the browser together.

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

## Request to play (a friend asks, the owner approves)

A friend runs `browser.exe --request-play --name Ann --message "hi, it's Ann"` (`--message` is optional; `--name` defaults
to the machine's name). It sends one HTTPS POST to `https://cone.jondgoodwin.com/play/request` and prints "Request sent;
you'll be able to join once it's approved." In the browser, **F9** (or **J**) opens the "Play with others" panel: a name field
(from `--name`, else the name this machine was approved under, else the machine's), an optional line to the owner, and
**Request to play**; the same code sends the request, from its own thread, and the panel shows the answer. Once approved it
says "You are approved" and its button reads **Join**: pressing it dials the server (`40.87.21.70:40000`, under the panel's name)
and switches the running browser from single player to connected, with no restart (the browser's README, "Play with others";
`--connect 40.87.21.70:40000 --name NAME` still starts a browser connected). A server that does not answer, or refuses the
name, leaves the player in single player with the reason in the panel. The server is not changed: a browser that joined
this way is a late joiner like any other. Tests and scripts: `PEGASUS_JOIN_ADDRESS` (Join dials this instead; an automated
run with none dials a port nothing listens on), `PEGASUS_REQUEST_URL` (post elsewhere) and `PEGASUS_PLAY_DIR` (keep
play-token.txt elsewhere); an automated run never posts to the live server unless `PEGASUS_REQUEST_URL` says so.

```
friend: browser --request-play  ->  nginx /play/request (1 KB, 6 a minute per address, 3 burst)  ->  pegasus-request on 127.0.0.1:8098
        -> one line in /var/lib/pegasus-requests/requests.log (UTC time, source IP, name, message), nothing else happens
owner:  sudo play-approve Ann [IP]   -> ufw allow from IP to UDP 40000 (comment "pegasus3d player Ann"), a random token issued
friend: browser --request-play again (from the approved address) -> "approved" and the token, kept in
        %LOCALAPPDATA%\pegasus3d\play-token.txt (else ~/.local/share/pegasus3d/), once only
later:  a request that carries the token from a NEW address -> that player's rule moves (new added, old deleted)
owner:  sudo play-revoke Ann         -> the rule and the token are gone
```

- **On the VM:** `sudo play-approve` with no name shows the last 10 requests. If one name asked from several addresses, say
  which: `sudo play-approve Ann 198.51.100.4` (a stranger can type a friend's name, so the owner chooses the address). Only a
  public IPv4 address is accepted. `sudo ufw status numbered` shows the rules, one per player, each with the player's name.
- **No token, no approval.** A typed name is not an identity: a request without a valid token never changes a firewall rule;
  it is a line in the list for the owner to read. A wrong token is logged as `[bad token]` and refused (403).
- **Where things are:** list `/var/lib/pegasus-requests/requests.log` (tab separated), approved players
  `/var/lib/pegasus-requests/players.tsv` (name, SHA-256 of the token, IP, UTC time; the token itself is not kept after it is
  handed over), pending hand-overs `claims/`. Code in `/usr/local/lib/pegasus/` (`request.py` the endpoint, `players.py` the
  root half), commands in `/usr/local/sbin/` (`play-approve`, `play-revoke`, `pegasus-move-ip`). Sources in `browser/server/request/`.
- **Endpoint:** a Python 3 standard-library server, systemd `pegasus-request`, as the system user `pegreq` (no shell, no
  login). nginx: the zone `playreq` and `location = /play/request` in `/etc/nginx/conf.d/cone.conf` (an exact match; the
  playground's `/play/` routes are untouched; backup `~/cone.conf.bak-2026-10-13-prerequest`). `sudo nginx -t` before a reload.
- **Root, narrowly:** `/etc/sudoers.d/pegasus-request` holds one line, `pegreq ALL=(root) NOPASSWD: /usr/local/sbin/pegasus-move-ip ""`.
  The empty `""` means no arguments at all. The helper reads `name ip token` from stdin, checks the name and the IPv4 address
  by pattern, checks the token against the stored hash itself, and calls ufw with an argument list (no shell). It can only
  move a player's own rule, never add a first one.
- **Firewalls:** this adds nothing to the Azure NSG. Until the NSG's `allow-pegasus3d-udp` source is widened from the owner's
  address, an approved friend's packets still stop at Azure; widening it is the owner's step (ufw is then the gate: default deny, no rule for
  UDP 40000 from anywhere except the approved addresses).
- **IPv6:** the game socket is IPv4; a request from an IPv6 address is refused with a message.
- **Client cost:** `--request-play` uses Cone's `httpclient` and `tls`, which link OpenSSL: `libssl-3.dll` and `libcrypto-3.dll`
  (7.3 MB) now sit beside `browser.exe`, and the zip must carry them. Build with OpenSSL's DLLs on `LIB`
  (`C:\src\cone\deps\openssl`).

## Connecting

The port is open only to the addresses in the Azure rule `allow-pegasus3d-udp` and the VM's ufw. **Adding a player means
adding their address to both** (the owner's decision each time; `play-approve` does the ufw half). From an allowed address:

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
