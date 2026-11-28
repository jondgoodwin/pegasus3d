# Messages

`[planned]`

This note owns the kinds of handshake between actors and the table of who sends what to whom. The kinds, state included, are decided: the index's word list defines them. The rows the first browser is built to are decided; the rest of the table of messages is proposed and held in Open, with rows that depend on another open question marked. Today the island carries its exchanges as typed messages over one connection and the studio as channels; neither names kinds.

## The design

### The kinds

- **Event:** a message that must arrive once, in order: a click, "this part was created".
- **Bulk:** a large transfer accepted over many frames: a mesh, a texture. New content is accepted under a budget, because arrival is governed ([The frame and its deadlines](../design/frame-and-deadlines.md)).
- **Move:** handing a thing to a new owner; the sender has it no more. Each thing is built once and moved to its one user ([Arrival](../design/arrival.md)).
- **Latest value:** data whose newest version is all that matters: one writer overwrites, readers never wait.

An action travels inside a message. Everything reactive is an action; everything else between actors is a message ([Actions and triggers](../design/actions-and-triggers.md)).

State and event traffic and asset traffic never block each other ([Network](network.md)).

### Design choices

- **State is a fifth kind:** a message whose newest version supersedes the older, so a lost one costs nothing (movement samples, snapshots across the network). In one process it is a latest value with a sequence number. `[design choice]`
- **The rows the first browser is built to,** taken from the proposed table below. They sit behind one message module, so a name or a kind changes in one place. `[design choice]`
  - Player → client Sim: movement and look samples with a sequence number (state); a press along a ray at the drawn frame's time, and bound world actions (event, carrying an action).
  - Player → Render: inhabiting avatar X with camera Y (event).
  - Sim → Render, Audio: the snapshot after each step (latest value, never changed after publishing).
  - Render → Player, Sim: the frame's display time and the head and hands as drawn (latest value). Render reads the newest pose itself, last, before drawing.
  - Player → Loader: open world W for Sim S, which Player has started (event). Loader → Sim: the world image and Sim units (move).
  - Sim → Loader: build content K for part P at a priority set by nearness, where K is a mesh key or a material key ([Sim](sim.md)); part P no longer uses K (events).
  - Loader → Sim: collision and pick shapes for a mesh key (move). Loader → Render: the mesh for a mesh key; the material, with its textures, for a material key (bulk moves, under Render's upload budget). Loader → Render: release K, sent when K's last user leaves, so Render may drop its GPU content (event).
  - Client Sim ↔ server Sim, when connected: as in [Sim](sim.md), connected play.
- **An action in a message is small:** a verb, a receiver handle, its parameters, the time it was made and where it came from. The fuller record in the actions proposal waits until that proposal is judged. `[design choice]`

### Ownership of the data a row carries

A message names its sender and its receiver. Who owns the data it carries, and by what means it is shared, is decided per structure in [Isolation and ownership](../design/isolation.md).

## What exists today

- **Pegasus skeleton, `pegasus/src/messages.cone` (W110, 28 Nov).** The first browser's rows as one thin module: `MoveIntent`, `LookIntent`, `Action`, `Snapshot` (`PartState` with its platform, published as `Arc[imm, Snapshot]`), `ContentRequest`, `WorldImage`, `ContentShapes`, `MeshContent`, `DrawnFrame`, and the state kind as `StateWriter[T]`/`StateReader[T]` over the sync package's latest-value cell (the version is the sequence number). The events and moves are behaviours on the receiving actors, listed by row in the module's header. A pointer press arrives with no receiver; Sim's admit sets it by picking.
- **The island** (`browser\src\simview.cone`, `netcodec.cone`, `netlink.cone`, in `C:\src\pegasus3d`): reliable messages from the view to the simulation (`Hello`, `Click`, `Command`, `Look`, `Frame`, `Play`) and back (`Requests`, `Possessed`, `Refused`, `Weather`, `FrameDone`, `Burst`, `Portal`, `Creature`); latest-wins numbered streams for input samples, the shared region snapshot, a test stream and a per-person part. Reliable messages are at most 1173 bytes and latest-wins at most 1171, with no fragmentation. In single player the same messages cross an in-process pair. The keep: reliable events and latest-wins values as two kinds on one wire. `[differs: one connection carries everything, so state and assets cannot yet block each other because no asset traffic exists; the snapshot travels by move to Render and back (in renderactor.cone); the shell awaits each actor in single player.]`
- **The island's head pose and hands**: Render reads them at the predicted display time and returns them with the draw's reply, one frame late into the next frame's input (`browser\src\renderactor.cone`, `browser\src\browser.cone`). The keep: Render reads the head.
- **The studio** (`guidebrowser\src`, branch `incubator-archwalk17` of `C:\src\pegasus3d-wt-incarch17`): channels carry a built part from the Loader actor to the loop and published part states from the world actor (`loader.cone`, `world.cone`); the world actor's step is sent without being awaited. The desktop capture's three buffers swapped under a lock (`shim\deskshim.c`) are the one existing latest value of the kind this note names. `[differs: nothing is named by kind; uploads are not budgeted; there is no network traffic.]`

## Open

### The microphone stream

Proposed: a stream of microphone samples is bulk. Recommended: this.

### The proposed table of messages

The rows listed under Design choices are decided; the others are proposed, and are the working standard the actor notes' proposed handshakes follow until they are settled. The mark **T** in the last column is a trigger: a message that names its handler in the content language.

"Sim" is a zone's Sim. "client Sim" and "server Sim" are its two versions ([Sim](sim.md)). A row marked (open) depends on a question held in the note it names.

| From → to | Message | Kind | T |
|---|---|---|---|
| Player → client Sim | movement and look intents, sampled with a sequence number | state | |
| Player → client Sim | pointer pressed or released along a ray, at the drawn frame's time; a button or gesture action | event | T (`Clicked`, `Pressed`) |
| Player → client Sim | inhabit being X, with camera Y; leave | event | T (`Boarded`, `Left`) |
| Player → client Sim | the signed statement for the world being joined (open: [Player](player.md), identity) | event | |
| client Sim → Player | inhabit granted or refused; focus offered (a page was clicked) | event | |
| Player → Render | inhabiting avatar X, camera Y, mode (eyes, chase, spectate) | event | |
| Player → Render | the browser interface's snapshot (open: [Player](player.md)) | latest value | |
| Player → Render | mouse look on the desktop, which Render reads as the head pose | latest value | |
| Render → Player, client Sim | frame time; head and hands as drawn (whether Audio reads it is open: [Audio](audio.md)) | latest value | |
| Player → Audio | inhabiting X (the listener's avatar) | event | |
| Player → Audio | who may hear the microphone (open: [Player](player.md), the microphone) | event | |
| Audio → Player | the microphone, for whoever Player allows (open: [Player](player.md), the microphone; and [Audio](audio.md) and [Network](network.md), for how it reaches people in a zone) | bulk (stream) | |
| Player → Surface | keys, text, pointer on the focused surface | event | |
| Player ↔ identity service | sign-in; the account, synced settings, and a signed statement for a world (open: [Player](player.md), identity) | event | |
| client Sim → Render, Audio | the snapshot after each step, never changed after publishing | latest value | |
| client Sim → Audio | play sound S on part P | event | |
| client Sim → Render | one-off effects (a burst of particles) | event | |
| Sim → Loader | build content K for part P, at a priority set by nearness | event | |
| Sim → Loader | priorities changed; part P no longer uses K | state; event | |
| Sim → Loader | write this world back to its origin (an edit saved) (open: [Worlds and zones](../design/worlds-and-zones.md)) | bulk | |
| Player → Loader | open world W for Sim S, which Player has started | event | |
| Loader → Sim | the world image and Sim units | move | |
| Loader → Sim | collision shape, pick shape, navigation mesh, behaviour code for K | move | T (`Arrived` on P) |
| Loader → Sim | the origin's file changed (local worlds): new content for K | move | T (`Changed`) |
| Loader → Render | the mesh for a mesh key; the material and its textures for a material key | bulk (move) | |
| Render → Loader | K dropped (eviction, device lost): re-supply if still used; memory budget left | event; state | |
| Loader → Audio | decoded samples for K | bulk (move) | |
| Surface → Render | newest picture, with its changed rectangle (open: [Surfaces](surfaces.md)) | latest value | |
| Render → Surface | visible or not, size on screen (open: [Surfaces](surfaces.md)) | latest value | |
| Sim → Surface | open or close surface for part P; navigate | event | |
| Surface → Sim | the page's content changed (title, length) | event | T (`Changed`) |
| client Sim ↔ server Sim | input samples; the network snapshot (a shared part and a personal part, [Sim](sim.md)), which is not the snapshot published to Render | state | |
| client Sim ↔ server Sim | actions, joins, creations, possession, world events | event | T (server side) |
| server Sim → client Sim | on joining: declarations (ids, places, platforms, areas, content keys) | bulk | |
| client Loader ↔ an origin's server | assets | bulk, on its own connection or a lower-priority stream | |
| Sim (zone A) → Player → Sim (zone B) | crossing a portal: release the avatar, admit it with what identity Player allows (open: [Worlds and zones](../design/worlds-and-zones.md), the steps of the entering sequence) | event | T (`Entered` in B) |
| Sim ↔ world's owner | (open: [Sim](sim.md)) | | |
| Sim inside (between parts) | triggers: `Entered`, `Left`, `Rang`, `Dusk`, `Dawn`, a handler's message to another part | event | T |

Reading the table:
- A world's character, run by an AI, has no row: where it lives is open in [The person](../design/person.md).
- Who owns the microphone is open in [Player](player.md); the rows that depend on it are marked. The head pose itself is read last by Render and never travels as a message ([The frame and its deadlines](../design/frame-and-deadlines.md)); Render reads the headset and publishes the pose as drawn.
- Rows marked (open): each question is held in the note the row names: the browser interface as a local world (Player), the steps of crossing a portal (Worlds and zones), the world's owner (Sim), writing a world back to its origin (Worlds and zones), and how the microphone reaches other people (Audio, Network).

### Questions the table raises

- Which rows carry actions, given that everything reactive is an action and everything else between actors is a message.
- Which rows are moves and which are reads of a latest value.

## What lives elsewhere

- [Sim](sim.md), [Player](player.md), [Render](render.md), [Audio](audio.md), [Loader](loader.md), [Surfaces](surfaces.md): each actor's side of these rows.
- [Network](network.md) and [Server](server.md): what crosses the wire.
- [Actions and triggers](../design/actions-and-triggers.md): what an action is and which messages are not actions.
- [Isolation and ownership](../design/isolation.md): who owns each structure and by what means it is shared.
- [Arrival](../design/arrival.md): bulk under a budget and moves to the one user.
