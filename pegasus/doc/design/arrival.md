# Arrival

`[planned]`

This note covers how a world gets from an address to the screen: what crosses the wire, where it is built, in what order, for whom and under what budget. The position is that a world arrives as a small program with its data embedded, and is built on the person's machine, off every frame. How loading is ordered and budgeted, and how built products are tracked, is mostly open. Today the island is compiled in and builds each object twice at start-up, and the studio fetches one world and builds it off the loop but uploads without a budget.

## Principles

- **A world appears progressively, and loading never shares a deadline with a frame.** It forbids a loading screen that blocks the picture, and a frame that waits for content.
- **Worlds arrive as programs with embedded data, not meshes; the description crosses the network and is built on the person's machine.** It forbids sending built geometry or textures over the wire.

Inherits: [The frame and its deadlines](frame-and-deadlines.md) (nothing delays the picture), [The content language](content-language.md) (what a description is).

## The design

### Design choices

- **Each thing is built once and moved to its one user.** It forbids two actors each building or holding the same product. `[design choice]`
- **A part shows when its content is resident; nobody waits for "ready".** It forbids a handshake that holds a world back until it is complete. `[design choice]`
- **A world of several files sends a manifest first.** It lists each file with its size and the hash of its content, and the path of the part whose subtree the file holds. The hash lets the Loader check what arrived and keep a file by its content, so a file two worlds share downloads once (the manual's construction page), and an unchanged file is not fetched again. The sizes let the Loader know when the world is complete and order its fetches, and the paths say which file declares which parts. A file holds whole subtrees; one file adding to a part that another file made is a question of the content language, held in Open. The spelling of the manifest is not chosen. `[design choice]`
- **The Loader reads each file as it arrives,** and Sim declares its parts then, so the near parts' builds start before the far files are in. Rules and derives start only when every file in the manifest is in, because a rule may name any part by its path. `[design choice]`
- **Compression is the transport's** (the content encoding of the connection), not a feature of the world's files. `[design choice]`

### What crosses the wire

- The wire carries the description of a world: a program in the content language, with its data embedded. Meshes and textures are built on the person's machine and stay there.
- A path's scheme says where an asset lives, on disk or on a server, and the Loader picks the transport. Above the transport the pipeline is the same.
- Asset traffic and state traffic never block each other ([Network](../actors/network.md)).
- For now, a far world's content is fetched ahead only when the person asks; this may be relaxed. [Strangers' worlds](strangers-worlds.md) says why.

### What happens to it

- The Loader fetches, parses, sets up what Sim declares its parts from (Sim allocates each part's id at once, before its content exists), runs procedural generation once to make meshes, textures and collision shapes, then compiles behaviour code to native code wherever it will run. How code becomes runnable is in [Loader](../actors/loader.md).
- Meshes and textures go to Render. Parts, collision shapes and behaviours go to Sim. On a server the Loader builds only what Sim needs.
- Player starts a zone's Sim for now and asks the Loader to open the world for it; the Loader moves the world image and Sim units to that Sim, which asks for each part's content by key ([Sim](../actors/sim.md)).
- Render is the only code that submits GPU work for now: the Loader moves meshes, textures and materials to it as CPU-side buffers ([Render](../actors/render.md)).
- Creating and editing parts while a world runs: [Live editing](live-editing.md).
- Arrival is governed: the Loader's priority and Render's upload budget are in [The frame and its deadlines](frame-and-deadlines.md).

### A partial world

- A renderer draws a partial world. This is a hard requirement of the platform, not a choice of the language: a person never waits for a whole world before seeing it.
- Nothing on the arrival path blocks the draw loop.

### The four layers of the latency architecture

1. **Do not send it.** The procedural description crosses the wire, in kilobytes where the built content would be megabytes.
2. **Hydrate where the compute is.** Meshes and textures are made on the GPU of the machine that shows them.
3. **Do not pause.** There is no stop-the-world pause that halts every actor.
4. **Do not pay per handshake.** The smallest saving; it is never the lead argument.

The ranked priorities are: the network latency of 3-D assets and the answer of a shared world first, the time of each frame second, the speed of compiling third.

### Budgets

The budgets that arrival answers to are owned by the performance perspective: the response to an input (100 ms), the world transition (time to the first controllable frame, with no frame in transit over the frame budget, counted as a maximum), and motion to photon. Arrival states the quantities it must meet and takes the numbers from there.

### Measured

- Hydrating (building meshes and textures from a description) a small cottage scene in a hydration experiment on one laptop took 4.3 s with its NVIDIA GPU and 4.7 s with its Intel GPU, to full. Hydration took 59% of that, and 95% of the hydration was painting textures.
- In a separate experiment, hydrating on the GPU took about 19 ms for a mesh the CPU took 325 ms for, and about 0.32 s for four meshes against about 34.6 s.
- In the island, on one small world (9 to 12 beings, 1577 parts), each object is built twice, taking 1.36 s and 1.6 s, and the worst frame during arrival was 2.2 s with every actor blocked. Those runs were under 25 s, with a headset by simulator only.
- Fetching a world's files took about 35 ms a round trip warm and about 150 ms cold, and about 1.4 s for 30 to 40 files over HTTP/1.1; the program and the network used are not recorded.
- Nothing has been measured about a long session.

## What exists today

- The island fetches nothing: its world is compiled in, and meshes and sounds are generated (`browser/src/bakedworld/bakedworld.cone`, `browser/src/audioview.cone`). `[differs: a world arrives as a description from an address]`
- The island's world builds one object a turn, so frames continue (`browser/src/bakedworld/bakedworld.cone`). Sim discards the meshes and the View builds each object again from its description (`browser/src/simactor.cone`, `browser/src/viewactor.cone`). `[differs: one build, moved to its one user]`
- The island has no Loader actor. A joining client gets the server's replay log and builds each object itself, with uploads held to 4 MB a frame when connected (`browser/src/replaylog.cone`, `browser/src/viewactor.cone`).
- The island's bake cache holds textures only, keyed by what made them, written atomically and checked (`browser/src/bakedworld/bakecache.cone`). It is filled from both builds; whether the Loader becomes its only writer is open.
- The studio's Loader actor reads a file or fetches a world by HTTP, parses it and builds its parts on a worker, sends them over a channel, and the loop makes at most 64 live a frame (`guidebrowser/src/loader.cone`, `guidebrowser/src/build.cone`, `guidebrowser/src/scene.cone`). It has one call site, so one world is loaded per process. `[differs: no budget on mesh uploads]` It has no nearest-first order, no catalog and no caching of fetched worlds, which are open.
- The studio uploads page pictures in time slices of 3 ms (`guidebrowser/src/pageup.cone`). The design keeps the idea of a budget.

## Open

### Designs not yet settled

- **A part whose content has not arrived.** How it is shown, and whether its behaviour and queued triggers wait for its content.
- **Whether every built product really has one user,** or some must be shared (see [Isolation and ownership](isolation.md)).
- **The order of loading.** Whether the Loader's work is ordered nearest first, and whether it has no deadline of its own. Recommended: nearest first, with no deadline of its own, at low priority.
- **The Loader's catalog.** It builds each thing once, is keyed by content key, knows what exists, frees GPU content nobody uses and re-supplies it after eviction. Recommended: yes.
- **The content key** (a part refers to its content by key; see [Parts and imports](parts-and-imports.md)). Proposed: a content key names a built thing, what was built, from what, and with what arguments. Open: who assigns it, and what makes it up, including the cache key of a built description. Options: a hash of the description plus the vocabulary version; the layout digest of live editing. Recommended: the Loader assigns it.
- **Crossing between zones and worlds** is settled in [Worlds and zones](worlds-and-zones.md), crossing a portal: preparing on the person's ask or on proximity, and the old world let go only once the new one has admitted the person. Open here: whether a world's declared neighbouring zones are prepared without a portal asking.
- **A file adding to a part another file made.** The content language has no way for one file to reopen a part declared in another; today a world's files are cut at part boundaries. Whether the language allows it, and by what rule, is the language's to decide. Recommended: not yet; files that hold whole subtrees meet arrival's needs.
- **Each file's stage.** Whether each file reports its stage (arrived, understood, built, registered, active).
- **How many fetches at once.** Today every file is asked for at once and the client's pool decides; a cap waits on a measurement against a real server, with TLS and HTTP/2.
- **GPU hydration against the frame budget.** Hydrating on the GPU competes with the frame. The test is that loading a whole world never moves the tail of frame times; the measurement is owed.
- **A long session.** The catalog is the mechanism for freeing content; nothing has run long enough to show it.
- **The upload budget** beyond the island's 4 MB a frame: unmeasured.

## What lives elsewhere

- [Loader](../actors/loader.md): the pipeline's internals, the transports, how code becomes runnable.
- [Render](../actors/render.md): what Render accepts, the upload budget, drawing a part that is not resident.
- [Sim](../actors/sim.md): declaring what exists, receiving products.
- [Network](../actors/network.md): the wire for assets and for play.
- [Parts and imports](parts-and-imports.md): parts, ids and content keys in parts.
- [Worlds and zones](worlds-and-zones.md): entering a world and crossing between zones.
- The manual's loading page: what a world's author sees of arrival.
- The performance perspective: the budgets and how they are measured.
