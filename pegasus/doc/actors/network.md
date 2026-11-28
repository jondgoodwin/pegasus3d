# Network

`[planned]`

Network is not an actor. It is the library the client Sim and the Loader both use to talk to servers: the client Sim for a shared world's state and events, the Loader for assets. This note says what the browser needs from that library and what crosses the wire. The island engine uses it for play over UDP at protocol 7 with eight players; the studio fetches one world over HTTP and has no network play.

The Cone I/O library's own design is not described here.

## The design

### Design choices

- **State, event and input traffic and asset traffic never block each other:** they use separate connections, or separate streams with priorities. `[design choice]`

### Role

Network carries two kinds of traffic: state, event and input traffic, which is the client Sim's, and asset traffic, which is the Loader's. A path's scheme says where an asset lives, on disk or on a server, and the Loader picks the transport.

### What the browser needs from it

The kinds are those of [Messages](messages.md).

- **Reliable messages**, in order and delivered once: events.
- **Latest-value streams**, where only the newest value matters: a writer overwrites; a reader takes the newest and never waits.
- **Many connections.** A server supports many more connections than the island's eight, and may host several zones, each with its own Sim; nothing assumes one world per server.
- **Zones are the partition.** A zone is the natural unit of interest: each zone has its own body cell, and a connection is bound to the cells of the zone its person is in. Filtering by region inside a zone is a later question (see Open).
- **A browser tier.** The per-frame traffic is designed for both the native library and WebTransport on HTTP/3, because a browser cannot use raw UDP. The native library's side is built first; datagram frames over QUIC are later.
- **One shared part per zone, one personal part per connection.** The server's snapshot of a zone's parts is split into a shared part encoded once per snapshot sent for everyone, and a personal part for each client that carries the last input sequence number the server applied ([Sim](sim.md), connected play). The shared part is held in a shared latest-value cell (a one-writer cell readers on any worker take without waiting), and each personal part in that connection's own cell.
- Decided provisionally under delegation:
  - **Input in a per-connection cell.** A person's input is read from a per-connection latest-value cell once a step, not as one message per sample.
  - **A value that does not fit in one packet is refused**, and the application splits it.
  - **Bounds on a hostile peer.** The library limits streams per connection, connections per address and admissions per second, and counts each kind of refusal.

### What crosses the wire

- **For play, between client Sim and server Sim:** a state and event conversation. Going to the server, the person's input (in the per-connection cell above) and discrete actions as events; coming back, the shared part and the personal part, and events. At joining the server Sim sends the declarations as bulk. What each side sends, what a snapshot holds, and how the client merges, predicts and reconciles are in [Sim](sim.md), connected play, and [Local and connected](../design/local-and-connected.md).
- **For assets, between the Loader and an origin's server:** the world's description, as text with embedded data, and its other files. Meshes and textures never cross the wire; they exist only on the machine that built them, and the transfer from the Loader to Render is local. See [Arrival](../design/arrival.md).
- **Not carried here:** the browser's sign-in with the identity service (see [Server](server.md)), and voice between people, which is Open.

### What it owns

Not settled (see Open).

### How it spreads its work

The library's I/O work scales across cores, first among its requirements; each connection lives in an actor that does its own reads, writes and encryption on a worker. That is the library's own design. A server holds many connections and may run several zone Sims, so the work spreads by zone and by connection.

## What exists today

- **Pegasus, assets over https; the join gives a client its world's address (W150, 28 Nov; `worldfetch.cone`, `simactor.cone`, `netplay.cone`).** The Loader's `Fetcher` fetches a world from an `https://` address with `actorhttp` (TLS, the operating system's verifier, HTTP/2 offered; GitHub's raw host answered over HTTP/2) on its own client, so asset traffic is a different connection from the `netmsg` socket of play. The Welcome's world name is the address the server runs the world from; a client started with `--join HOST:PORT --world-from-server` (no copy, Hello with no world name) fetches it from there, then the definition-sum check holds it to the server's. A server run from a file refuses such a client. Detail and figures: [Loader](loader.md). `[differs: no encryption or authentication of the play connection itself; HTTP/3 not used (the client offers it only for origins listed as speaking it)]`

- **Two-client test script, `pegasus/tools/net-test.ps1` (W129, 28 Nov).** `net-test.ps1 loop|loss|glide|glideloss|avatar|rejoin|refuse` runs a server and headless clients as processes from a built tree and prints their net lines; `glide` ends with `GLIDE-CHECK ... MATCH` for the clients that moved the rook. Output is in `pegasus/build/run-<scenario>`.
- **Pegasus, connected play over `netmsg` (W117, 28 Nov; `pegasus/src/netplay.cone`, `simactor.cone`).** One UDP connection per client carries: reliable Hello, presses, and the join (Welcome, parts and shapes in messages under 1,100 bytes, Joined); latest-wins stream 1 (the client's unacknowledged input samples, 481 bytes for 30), stream 2 (the shared snapshot, largest 49 bytes with two people) and stream 4 (the personal part, 28 bytes). Tested on loopback and through `netmsg`'s `LossyRelay` (`--impair 50 20 10`: 5% loss, 20 ms and up to 10 more each way): 0 messages did not parse, no input sample was lost for good, 0 corrections. `[differs: floats, not quantised; no encryption; the declarations of a world larger than the reliable window (1,024 messages) are not chunked further.]`

- **Pegasus, the world's parts in the shared snapshot (W123, 28 Nov; `pegasus/src/netplay.cone`, `partsync.cone`).** Wire version 2. The join no longer sends the parts (Welcome carries their count and a sum); the client's input packet starts with the newest part version it applied (8 bytes); the shared snapshot on stream 2 is the people half, encoded once (9 + 20 a person bytes), then a per-client half: a count byte, and when parts changed, the version the snapshot is complete through (8) and 69 bytes a part (id, field mask, 16 floats), as many as fit one 1,100-byte packet (15 beside one person). Measured: 70 bytes with three people and nothing changed, 147 with the rook moving, 70 again once acknowledged. Silent failure: a change in a lost snapshot never resent. The server measures against what the client acknowledged, never what it sent; a driven check (`--checks`, the snapshot at step 4 lost) and a live one under 5% loss catch the mutant `delta-vs-sent` (one of two clients never saw the rook move).
- **Island, `browser/src`.** `netmsg` over UDP, one connection per client, carrying both reliable events and latest-wins streams on numbered keys (`netlink.cone`, `NetSide`). Reliable messages up to 1173 bytes, latest values up to 1171 (`netcodec.cone`); an oversize send is dropped and counted, with no fragmentation. `PROTOCOL_VERSION` 7 is checked at the client's hello (`netcodec.cone`). Single player uses the same messages over an in-process pair, plus a turn-taking pair of messages that makes it lockstep. `[differs: the design has one model of play with no lockstep messages]`.
- **Island, streams (`simview.cone`).** Stream 1 is the client's input (the last 10 samples with sequence numbers, up to 30 while unapplied). Stream 2 is the zone's shared body (`RegionState`, encoded once per publish by `encodeRegion` in `simactor.cone`; the island is region 0). Stream 4 is a client's personal part (`PersonalState`). The snapshot is sent every other tick, 30 a second on a server. Positions are quantised. Reliable messages carry joins, possession, the world's requests and weather, clicks, commands and plays. A late joiner receives a replay log in which a later request for the same part replaces an earlier one.
- **Island, not found.** Per-connection input cells, a bound on players beyond the constant of eight (`MAX_PLAYERS`, `simactor.cone`), and more than one zone on a server. `[differs: the design has many connections and several zones per server]`. No encryption.
- **Island, test tools.** `--impair LOSS LATENCY JITTER` (a lossy relay), `tools/netcheck.py`, `tools/stagingcheck.py`.
- **Measured, two headless clients on loopback with two mannequins, the villagers and 11 props.** A snapshot averages 375 bytes and the largest is 469 (the limit is 1171). The largest reliable message is 1086 and the largest request 70. Per client over the 20-second run: 599 snapshots (225,183 bytes) and 16 reliable messages (3,983 bytes); on the wire 11.7 KiB/s in and 1.9 KiB/s out. A being is about 30 bytes quantised, so one packet holds the body of roughly 30 to 35 beings.
- **Studio, `guidebrowser/src/loader.cone`.** An HTTP GET through `actorhttp.Client` with a 30-second limit, or a file read through aio. No `netmsg`, no cache, no streams.
- **Measured, fetching over HTTP/1.1** from the project's site: about 35 ms a round trip warm and about 150 ms for a cold connect; a world of 30 to 40 files costs about 1.4 s of waiting, one round trip per file.

## Open

- **What Network owns.** Proposed: no state of its own; a connection serves one user, the client Sim for state and events, the Loader for assets. Recommended: this.
- **The packet's size.** A packet is about 1,200 bytes, the size a datagram can have over WebTransport, which is the only browser interface with unreliable datagrams. Recommended: design values to fit it.
- **Body larger than one packet.** A zone with more beings than one packet's body holds (roughly 30 to 35). Options: a body that spans packets; sending each person only part of the zone; filtering by region inside a zone. A crowded single zone stays a real limit. Recommended: region filtering when a crowded zone is measured.
- **Assets and state on one connection or two.** Proposed: the Loader gets its own connection (or a lower-priority stream). To measure first: whether `netmsg`'s streams on one UDP socket already keep a large transfer from delaying a snapshot. Recommended: measure, then separate if they do.
- **How files travel.** Options: HTTP/2 multiplexing on one connection (about one round trip plus the transfer for a 30 to 40 file world); several HTTP/1.1 connections; packing a world into fewer files, which fails for files from other sites. Recommended: HTTP/2 if round trips dominate the measured world arrival; the memory ownership of Cone's I/O is fixed first.
- **Live traffic on QUIC.** Unreliable datagrams give no head-of-line blocking and match latest-value traffic; reliable streams on one connection carry events. Recommended: build on an existing QUIC implementation, not our own, after the I/O ownership work and after the native library carries play.
- **Encryption and authentication.** Nothing on the wire is encrypted or authenticated, and a name is not identity. Options: encryption and authentication in the native library, QUIC's TLS later. Recommended: both, because shared worlds with accounts depend on them.
- **Scale and admission.** Eight players are built; the plan is a hundred to a thousand, and public access for invited visits. The library's admission limits exist; the number a server should accept per core is unmeasured. See [Server](server.md).
- **Voice and text chat between people.** Who carries it, over which stream, and who may mute whom. Voice is small packets judged by tail latency.
- **Reconnecting.** A rejoin is a fresh join; whether a session resumes is open.

## What lives elsewhere

- [Local and connected](../design/local-and-connected.md): prediction, interpolation, reconciliation, riding platforms.
- [Arrival](../design/arrival.md): what the Loader fetches and in what order.
- [Worlds and zones](../design/worlds-and-zones.md): zones as the unit of interest, entering a zone.
- [Server](server.md): admission, the server's Sims, identity.
- [Sim](sim.md), [Loader](loader.md), [Messages](messages.md), [Audio](audio.md) (voice); the manual's distribution page.
