# Isolation and ownership

`[planned]`

This note covers where to isolate, by which means, and who owns each kind of data. The position is that the means of isolation is chosen per structure, and that local data and local ownership come first. Several rows of the ownership table, and the whole memory model, are open. Today the island moves data between actors and several structures are built or held twice.

## Principles

- **Where to isolate, and by which means, is chosen per structure:** an actor where the processing, not just the data, must be isolated; a lock where data is shared independently of processing; other structures transit freely. It forbids one mechanism applied everywhere, and an actor used only to protect data.
- **Local data and local ownership are preferred.** It forbids a shared structure where an owned one would do.

Inherits: [Strangers' worlds](strangers-worlds.md) (a stranger's memory stays inside its world).

## The design

### The means

- **An actor, reached only by messages,** where the processing as well as the data must be isolated. One owner and no locks rules out lock deadlocks but not other deadlocks, and it is a slow way to protect data.
- **A lock,** where two must share a structure independently of their processing. A lock is a faster way to share than messages.
- **Free transit** for structures that are not an actor's own. One form is a latest value, where only the newest version matters: one writer overwrites, readers take the newest and never wait, and a missed version costs nothing. The head pose is one.

Both models exist in Cone, and choosing between them is done structure by structure.

### Design choices: who owns what

Each row is a per-structure decision `[design choice]`. Parts are Sim's and meshes and textures are Render's. A structure no actor changes has no owner. "Sim" is a zone's Sim. A row marked (open) is proposed and rests on an item in Open.

| Data | Owner | How it is reached or shared |
|---|---|---|
| Parts: ids, tree, placement, visibility, look, and references to their content, surface and sounds | A zone's Sim; the client Sim holds its copy and the server's wins | Creation and edits are requests to Sim; Render and Audio read through the snapshot |
| Snapshot of parts after a step | Nobody once published; the Sim that made it | A latest value of immutable data; Render keeps the two newest; read by Render and Audio ([Sim](../actors/sim.md)) |
| Draw meshes | Render | Moved in from the Loader as bulk under the upload budget; an edit sends Render a copy |
| Textures, from worlds and from surfaces | Render | Loader bulk, or a surface's latest value |
| Materials | Render | Moved in from the Loader |
| Collision and pick shapes | Sim | The Loader builds each once and moves it; immutable, so physics, navigation and picking share it inside Sim |
| Navigation mesh | Sim | Built by the Loader and moved |
| Physics world: bodies, contacts | A zone's Sim | Inner to the step |
| Beings: bodies, intents, the predicted body and its history | The client Sim (predicted), the server Sim (authority) | The step and reconciliation; others see them through the snapshot |
| Platform poses and their short history (open) | Sim | Published in the snapshot with velocity, and with each part's frame |
| Timers, areas, the zone's clock | A zone's Sim | Triggers into Sim's handlers |
| Behaviour code (open: its form) | Sim | Built by the Loader and moved; lives while its part lives |
| Behaviour state: the world's heap (open) | A zone's Sim, with its own collector | Handlers only |
| World-wide state, and what travels between zones (open: the owner's role) | The world's owner | (open) |
| Head pose | Render reads the headset at the frame's display time and publishes it as drawn; on the desktop, Render reads Player's mouse look | A latest value; Player and the client Sim take the published copy; whether Audio does is open ([Audio](../actors/audio.md)) |
| Controllers, keyboard, mouse, focus | Player | OS signals go to Player only |
| Account, privileges, avatars owned (open: the identity service) | The identity service on a server; Player at this machine | A world gets only what Player hands over |
| Which avatar is inhabited, and its camera | Player decides; the avatar and its camera are parts, Sim's | Player tells Render, Audio and the client Sim; Sim confirms |
| The Loader's catalog: content key to what was built, who holds it, how many parts use it (open) | The Loader | Messages from Sim and Render |
| Content on disk: worlds, assets, the bake cache (open) | The Loader, the only writer | The Loader's messages |
| Large immutable inputs: the island's heightfield, fonts, a world's embedded data | Nobody | By reference |
| Sound: mixer state, decoded samples, listener | Audio | Samples moved in from the Loader; the listener's pose is open ([Audio](../actors/audio.md)) |
| The microphone device and its stream (open) | Audio | Player decides who may hear it |
| A web page's picture | Its surface | A latest value to Render |
| The desktop picture (open: the lock) | Its surface | The capture thread's three buffers swapped under a lock; a latest value to Render |
| A document: text, parsed form, layout, picture, and its one source file (open) | Its surface | The picture is a latest value to Render |
| Room layout in the studio | The studio's Sim | Edits are messages; the Loader writes the world back |
| Browser preferences | Player | Written through Player |
| Start-up options | Nobody | Read only |
| Replay of a zone for late joiners (open) | The server Sim | Published to joining clients |
| Network connections | The client Sim (state, events); the Loader (assets) | Through the network library ([Network](../actors/network.md)) |
| Per-frame scratch (open: the memory model) | Each actor | A frame arena |

### Memory

How memory is chosen for a structure follows from how long it lives and who changes it. The models that choose memory are not decided. What is measured: the island makes about 640 allocations and 1.8 MB of allocation a frame on one small world (counted at the new size, so overstated), and no cost in time has been shown for it.

## What exists today

- The island moves data between actors by value. The snapshot is built by the View and moved to Render and back each frame (`browser/src/parts/snapshot.cone`, `browser/src/viewactor.cone`); the registry is lent to the world for each turn (`browser/src/worldactor.cone`).
- The island keeps two placement trees, Sim's and the View's (`browser/src/parts/parts.cone`, `browser/src/parts/viewscene.cone`), builds each object twice, and computes collision solids separately in Sim, in the View, in the village and on join. `[differs: one build, moved to its one user]`
- The View keeps a CPU copy of the draw meshes for picking (`browser/src/viewactor.cone`, `browser/src/picking.cone`).
- The island's baked land is made once and shared read only (`browser/src/browser.cone`).
- Part data is in tables by id, with a sparse index; placement is flat arrays with parents before children (`browser/src/parts/parts.cone`, `browser/src/parts/placement.cone`).
- GPU content is never freed when a part despawns (`browser/src/parts/parts.cone`).
- One connection carries everything (`browser/src/netlink.cone`). `[differs: asset traffic and state traffic never block each other]`
- The studio's desktop capture runs on its own thread with three buffers swapped under a lock and taken by the frame without waiting (`guidebrowser/shim/deskshim.c`, `guidebrowser/src/desk.cone`).
- The studio's World actor holds part values by slot and publishes copies (`guidebrowser/src/world.cone`, `guidebrowser/src/scene.cone`). Its frame package passes a scratch arena that nothing uses (`guidebrowser/src/guidebrowser.cone`).
- The studio's documents write their own file through a temporary file and a rename (`guidebrowser/src/docpanel.cone`).

## Open

### Designs not yet settled

These are the rows of the table marked (open). Each is recommended as the table states it.

- **Immutable data shared by reference** as a form of free transit, for what many read and none change: snapshots, built shapes, large inputs, fonts, start-up options. Recommended: yes.
- **The Loader's catalog** builds each thing once, knows what exists, frees GPU content nobody uses, re-supplies after eviction, so that no shared library exists. Recommended: yes.
- **The world's heap.** Behaviour state belongs to a zone's Sim, with its own collector. Recommended: yes.
- **The microphone.** Audio owns the device; Player decides who hears it. Recommended: yes, since capture runs on the audio clock and echo cancellation needs the speaker's signal.
- **Disk.** The Loader owns content (worlds, assets, the bake cache); a surface owns its one source; one origin rule covers both. Recommended: yes.
- **The desktop picture's lock.** None was found between actors; the one lock is inside the desktop capture's buffer swap. Recommended: keep it there, and add none until a structure needs one.
- **The replay of a zone for late joiners.** Recommended: the server Sim's.
- **The world's owner.** What state it holds beyond what spans zones, where it runs, who starts it and the zone Sims. The table's row waits on it.
- **The whole memory model.** The ideas in hand: ids as weak references into tables; resource scopes with use counts and deferred GPU destruction; a frame arena in each actor; a collector per actor for irregular graphs. None is decided.
- **A long session.** Nothing has run longer than a short measured run, and GPU content is never freed by despawn in the island. The catalog is the proposed mechanism; the measurement is owed.
- **The per-frame allocation count.** Whether it matters, or is left until a cost is shown.
- **The form of behaviour code,** which depends on how a stranger's code runs ([Strangers' worlds](strangers-worlds.md)).
- **The identity service,** and what the client holds of it.
- **Where an AI character's identity, perception and mind are held.** No row of the table is for it.

## What lives elsewhere

- [Arrival](arrival.md): building each thing once, the catalog, the budget under which products move.
- [The frame and its deadlines](frame-and-deadlines.md): the head pose and the snapshot on the frame's clock.
- [Strangers' worlds](strangers-worlds.md): why a world's heap stays inside it.
- [Messages](../actors/messages.md): the kinds of handoff (state, event, bulk, move, latest value) and who sends what to whom.
- [Sim](../actors/sim.md), [Render](../actors/render.md), [Audio](../actors/audio.md), [Loader](../actors/loader.md), [Player](../actors/player.md), [Surfaces](../actors/surfaces.md): each actor's own data.
