# Server

`[planned]`

The server is the program that runs a shared world. It holds the truth of the world: it runs one server Sim for each zone it hosts, and admits and serves the people who join. Most of its design is undecided. The island engine's `--serve` mode already runs one headless server on one machine for eight players with admission by a firewall rule per address; the studio has no server.

## The design

### Role

The server owns the truth of a shared world. A world with no server runs whole on the person's machine; a shared world's outcomes are decided on the server, not by a client's word. A server may host several zones, each with its own Sim, and nothing assumes one world per server. A world also has a world-level owner beside its zone Sims; its role, what it holds and where it runs are Open.

The server runs the Sim program in the server role, with a Loader that builds only what Sim needs ([Sim](sim.md), connected play).

### Who it works with

The kinds of handshake are those of [Messages](messages.md); the proposed kinds are in the proposed table of messages there.

- **Client Sims**, over [Network](network.md). Each server Sim carries a state and event conversation with the client Sims. The server supports many connections.
  - Each zone's snapshot goes out as a shared part encoded once for everyone and a personal part for each client ([Sim](sim.md), connected play; the cells that hold them are in [Network](network.md)).
  - Decided provisionally under delegation: each person's input is read from that connection's own latest-value cell once a step (see [Network](network.md)).
- **An identity service.** How a person is identified to a world's server, and what the server learns of them, is deferred (see Open).
- **The Loader** ([Loader](loader.md)). The server's Loader builds only what the server's Sim needs.

### What it owns

- The zones' Sims and the truth they hold.
- The world's saved state, where the world is saved on the server: Open.

### How it spreads its work

Each zone it hosts has its own Sim, as a separate actor doing its own simulation. How a single zone's work spreads is in [Sim](sim.md); how many connections are served across cores is the network library's design.

## What exists today

- **Pegasus, `pegasus-server`, a program of its own (W149, 28 Nov; `pegasusserver/`, `pegasuscore/`, `pegasus/tools/net-test.ps1`).** `pegasusserver/src/pegasusserver.cone` is a small executable (about 130 lines: arguments, the world's key count, a stand-in actor `Mute`, and the start-up Player did for `--serve`); it builds with `pegasusserver\build.ps1` into `build\release\pegasus-server.exe`. Usage: `pegasus-server --port PORT --world FILE [--steps N] [--loopback] [--until-empty] [--impair LOSS LATENCY JITTER]`, and `--checks` (the shared code's checks: connected play, rules, picking, blend, mesh cache, world fetch). It runs the same Sim (server role) and Loader (`simOnly`) as the browser, because both are in the library package `pegasuscore/` which the browser, `pegasus/`, also imports; Sim and the Loader send to the display and the sound through two actor traits (`RenderSink`, `AudioSink`), which Render and Audio meet by their behaviours and the server's `Mute` meets by taking everything. It imports no window, Render, Player, Audio, XR or SDL code, and the Windows program starts with no `SDL3.dll` beside it (the SDL binding still compiles in, below Cone's `sculpt` package). `pegasus --serve` is retired: it prints a pointer and exits 2. `net-test.ps1` glide, glideloss, lamp, light and rejoin run against it (MATCH); `-ServerExe` names another server (a `pegasus.exe` is run as `--serve`, for the old program's number). Measured on the laptop, `worlds\first.ddd`, cold mesh cache: both the old `--serve` and `pegasus-server` run at 25 to 27 MB peak and build the 70 meshes in 280 to 390 ms: **the server's Loader still builds each mesh in full**, and `shapesOf` takes the collision and pick boxes from the built mesh's bounds (W121); there is no shape-only builder yet (a design question for browser-arch). Open: the compile of the server needs 1,781 MB peak (release, Windows working set) of which `pegasuscore` is 1,930 MB and a program that only imports the same dependencies is 1,030 MB, so the server does not yet fit the Linux server's 1200 MB cap; see the lane's handover.

- **Island, `browser/src/server.cone`, `simactor.cone`.** `--serve PORT` runs the same `SimActor` as single player, with no window, GPU or sound. The main thread loop collects fixed steps, calls `serveTick`, blocks until the tick is served, and sleeps 2 ms. One world, one zone, eight players (`MAX_PLAYERS`). `[differs: the design has several zones and many connections]`.
- **Island, admission.** A client sends `Hello` with a name within 600 ticks or is refused. The name is at most 24 characters, letters, digits, space and `. _ -`, unique ignoring case. The protocol version (7) is checked. A refused client is hung up 30 ticks later. A leaver's being is despawned; a rejoin is a fresh join. There is no login and no encryption, and a name is not identity.
- **Island, deployment, `browser/server/`.** A headless Linux build through an SDL stand-in (`sdl3shim.c`), `deploy.sh`, `build-on-vm.sh`, and a systemd unit. It runs on one cloud machine (Ubuntu 24.04, 2 vCPU, 4 GB) that also serves the project's websites, on one UDP port, as a non-root user, logging to journald.
- **Island, measured on that machine:** idle at 2.9% of a core and 101 MB resident; about 9 s of world build at start; the scripted two-client test passes against it with round trips of 39 to 64 ms and no corrections.
- **Island, admission by approval.** The UDP port is open only to approved addresses, enforced by the firewall on the machine and by the cloud network rule. The browser's "Play with others" panel (`requestpanel.cone`, `requestplay.cone`) sends an HTTPS request to a small endpoint (`browser/server/request/`) that records the name, a message, the address and the time; a body of 1 KB at most, six requests a minute per address with a burst of three. It changes no firewall rule by itself. An operator approves a name, which adds the address to the firewall and issues a random token the browser stores; a later request carrying a valid token moves the allowed address. `Join` switches a running single-player browser to the server (`goOnline`).
- **Island, known defects** in the server's own status notes, not rechecked on master: a being's slot leaks on join, the rate limit counts all connections and not one address, and the spawn counter only grows.
- **Studio.** No server, no network play. A world is fetched from a site that serves plain files.

## Open

- **The world's owner.** Its role, the state it holds (world-wide state, what a person carries from zone to zone), where it runs, and who starts the world's zone Sims. Recommended: none yet; it is designed with Sim.
- **What a server runs besides Sim.** Proposed: a server has no Audio, Surfaces or Player, and, as [Render](render.md) decides, no Render. Recommended: yes.
- **The server's clock and what it keeps.** Proposed: the server runs on its own step clock, not the clients' frames; the server Sim keeps the catch-up a late joiner receives. Recommended: this.
- **What serves a world's files.** Options: the project's existing web server first; a Cone server in files mode later; one general server (`--serve ADDRESS`) that does both. Recommended: the existing web server first.
- **Admission and scale.** Options: approval by firewall rule (today); a token per person with an address that moves; public access for invited visits; encrypted and authenticated packets. The number of players a server accepts per core is unmeasured; the network plan is a hundred to a thousand. Recommended: tokens first, then the rest as the network library gains encryption.
- **Saved state.** What "saved" means in the language, whose servers hold a home island, and a ledger for ownership and credits. Recommended: three records per person on the project's servers (who they are, what they own, their island), with the world told what the person owns by a claim in the statement, and a world's own data kept by that world's server under the world's own id; the business side is the author's to decide.
- **Who may run a server.** Whether strangers may host worlds with our identity service, and what a stranger's server may ask of it.
- **An AI character's home.** Where a character's mind runs and how it reaches the zone's Sim (a service the world's server runs is one option) is open.
- **One server, many zones.** How a person moves from one zone's cells to another's on one endpoint, are designed in outline only.

### Deferred, with safety and identity

- **Identity.** Proposed: our own identity service, with other companies' logins only as ways into it; a separate id for the person in each world by default; a signed statement for the world they join, carrying that world's id for the person and claims such as "guest" and what the person owns, which the world's server checks itself; guests first-class; ownership and payment on the person's account. Reason: every platform studied converges on it, it lets a world learn only what is handed over, and it still lets a marketplace work; it sets the business's account model. Recommended: this. The service itself is undesigned: accounts and sign-in ways (passkey, email, linked logins), recovery (designed with the same care as sign-in), a second factor, parental links, where it runs, and how the world's server learns the service's signing key. No source designs it.

## What lives elsewhere

- [Local and connected](../design/local-and-connected.md): server truth, prediction, what rules run where.
- [Worlds and zones](../design/worlds-and-zones.md): zones, the world's owner, instances and saves.
- [The person](../design/person.md): the signed statement, per-world identity.
- [Network](network.md): the library, the wire, admission limits.
- [Sim](sim.md), [Loader](loader.md), [Player](player.md), [Messages](messages.md); the manual's distribution page.

- **Pegasus, `pegasus-server` takes a world at an address (W149b, 28 Nov; `pegasusserver.cone`, `pegasuscore/src/xrlog.cone`, `tools\https-test.ps1`).** After the merge of the https work (W150): `--world` may be an `http://` or `https://` address (it is not fetched by the program's main; the Loader fetches it, and the server counts no keys then). A client started with `--join HOST:PORT --world-from-server` against `pegasus-server --world https://ddd.jondgoodwin.com/pegasus/worlds/first.ddd` was told the address, fetched the world itself, joined, and its checks passed (0 mismatches, 200 snapshots, 0 hash differences). `https-test.ps1`'s connected test now starts `pegasus-server.exe` (no `--keys-from`). The XR log's two helpers `xrLog` and `vecText` moved to `pegasuscore/src/xrlog.cone` because Sim writes a line to it for each press.
