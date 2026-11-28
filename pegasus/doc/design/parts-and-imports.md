# Parts and imports

`[planned]`

This note covers parts as the unit a world is made of: what a part is, who owns the live ones, how a world reaches them,
and how a world imports parts published elsewhere (portals and a person's avatar among them), with the technical side
of the registry that holds them. It takes the position that worlds are built of parts, their own and imported, that a
part is an id, and that Sim owns the live ones. What a published part is, how ids cross worlds and how a portal's
requests are judged are open. Neither program has any import today, and both keep a part's data in tables owned by one
actor.

## Principles

- **Sim owns the live parts and everything attached to them.** It forbids another actor holding or changing a part.
- **A world can import parts published elsewhere; the author keeps authorship and the renderable form is available to
  everyone.** It forbids a part visible only to those who hold its source.

Inherits: [Strangers' worlds](strangers-worlds.md) (a world's code reaches parts only through the language's
interface), [Isolation and ownership](isolation.md) (where to isolate).

## The design

### Design choices
- **An imported part runs with its host world's authority.** It forbids an imported part carrying rights of its own.
  `[design choice]`

### What a part is
- A part is the unit a world is made of: a node in the world's tree, with an id. A part can be a subpart of another,
  and both have ids. A collection of particles is a single part.
- A part is an id, and it refers to its content (meshes, surfaces, sounds) by a content key, never by a GPU handle.
- A being is a part that acts; an avatar is the being a person inhabits.
- The client Sim has its copy of the live parts, and the server's copy wins in a shared world.
- Render and Audio reach parts only through the snapshot Sim publishes after each step, which never changes after publishing ([Sim](../actors/sim.md)).

### How parts come to exist
- The Loader parses a world and moves the world image and Sim units to Sim. Sim allocates a part's id at once, when the part is declared or created and before its content exists; the part shows when its content is resident ([Sim](../actors/sim.md)).
- Editing, creating and destroying objects while a world runs: [Live editing](live-editing.md).

### How a world reaches parts
A world never reaches into the scene: it reaches parts only through its door. What a world may ask is the language's
interface, under the limits in [Strangers' worlds](strangers-worlds.md).

### Importing parts
- A world can import parts published elsewhere, like a library: the pieces of a chess set, a portal, a person's avatar.
  People build parts that others import and refine.

### Portals
- A portal is an imported part: made available by some world to others, placed in a host world, and managed as an
  asset separate from the world it leads to.
- Its look is its own capability and is not necessarily a live window.
- On proximity, a button or another trigger, it gets ready: the far world is made ready and visible through it.
- On entry it sends an action telling the browser that a new world is being entered. The action goes to Player, and
  Player runs the entering sequence (see [Worlds and zones](worlds-and-zones.md)).
- For now, a portal prepares its far world only when the person explicitly asks. Preparing fetches from the far world's
  server, which would otherwise learn the person's address before the person chose to go.

### Avatars
A person's own avatar is an imported part, like a portal; how it lends its look and who decides its behaviour is in
[The person](person.md).

### The registry of parts
- Parts are published in a registry, much like a registry of packages, with a store where they live: the registry
  points to a part held elsewhere, or holds it.
- The author keeps sole authorship. The renderable form is available to everyone, so other people's browsers fetch it
  from the registry, as they fetch any imported part. Avatars are published in the same registry.

## What exists today

The island is `C:\src\pegasus3d` master (`browser/src/`); the studio is the branch `incubator-archwalk17`
(`guidebrowser/src/`).

- **Parts and ids.** The island's registry is a pool of part records with generational ids (`parts/parts.cone`,
  `Registry`), and each aspect of a part (placement, look, sounds) is a table keyed by id and owned by one system
  (`Table[T]`, sparse-set indexed, `parts/parts.cone`). The placement tree is flat parallel arrays (`parts/placement.cone`).
  `[differs: a second placement tree is kept by the View (`parts/viewscene.cone`), and the registry is lent to the world
  each turn, not owned by Sim alone]` The studio keeps a part as a slot in a list, with parallel lists of values,
  positions, rotations and tints in the World actor (`world.cone`).
- **The door.** The island's world is handed a `Door` (`parts/parts.cone`) to reserve ids, send `Request`s and read
  mail, and never sees a scene table, the render system or a GPU handle (`worldactor.cone`, `bakedworld/bakedworld.cone`).
  The world is compiled Cone code in the same process, built one object a turn. `[differs: the studio's world is an
  interpreted program of parts, fields and rules with no door]`
- **Content.** The island builds each object a second time in the View and the despawn of a part never frees its GPU
  mesh (`parts/parts.cone`, `viewactor.cone`). `[differs: a part refers to its content by key]` A Loader catalog
  exists only as an animated demonstration in the studio (`archdata.cone`).
- **Import.** Neither program has any. The studio can fetch a whole world by URL (`loader.cone`) but nothing inside a
  world names another, and its `part p: d(...)` design copy works only within one file (`expand.cone`). Cone packages
  are build-time dependencies by name (`congo.toml`). `[differs: no import, no registry, no published part]`
- **Portals.** The island's portal is a looking-glass between two places in one scene, the labyrinth sitting far below
  the island (`parts/portal.cone`, `portalsim.cone`); Sim spawns the portal's puppet itself. `[differs: not an imported
  part, and not a door between two worlds]` The studio has no portal and no navigation from world to world.
- **The manual.** Its parts page makes a part's path its identity, and its designs page copies a design within a world.
  Its packages page says a world names the browser's tools and never a package, and has no import line. `[differs: a
  world imports parts]` The portals page makes a portal part of the language with published arrivals `[differs: a
  portal is an imported part whose entry goes to Player]`.

## Open

### Decisions to confirm
- **The content key.** Is it assigned by the Loader, and what does it hold (what was built, from what, with what
  arguments)? See [Arrival](arrival.md).
- **What an imported part is.** Recommended: content in the same language under the same limits as the host's; a world
  never names a package, which is Cone code shipped with the browser, so a world that could name any package could name
  a network library.
- **Whether a part may ask for more than its host holds,** and who then judges. Recommended: no, except a portal's one
  request (below).
- **Who builds and who frees content.** Recommended: the Loader's catalog builds each thing once, knows what exists,
  frees GPU content nobody uses and re-supplies after eviction.
- **Ids.** Options: one registry with a global pool of ids, shared by everything; or two layers, a dense
  generation-checked id inside a world, and for what crosses a boundary a world id (a domain and path for authored
  parts, an authority's namespace and a counter for spawned ones). Recommended: ids belong to the world, each zone's Sim
  allocating within it, and what crosses a boundary is a handle the browser maps, because the isolation line around a
  world forbids a pool shared with a stranger's world and two worlds met in a portal may number their parts alike.
  Related: authored parts' ids must not depend on the order content arrives, since arrival is progressive; the
  declarations would carry their numbers explicitly.
- **The form of a published part, and what happens when its author changes it.** What a part is as published; how a
  version is pinned; whether a world sees a changed part; how a registry fetch is kept from revealing who is near whom.
  Recommended: a part is pinned by its content; study package registries and a marketplace's next-owner permissions
  (copy, modify, transfer) first.
- **A portal's requests.** Proposed: getting ready and entering are two requests to Player, because starting a Sim is
  Player's to ask; preparing hands over no identity; the portal's one request is judged by Player. Whether the
  destination a portal names is shown by Player's own interface before crossing is deferred with the browser's own
  prompts ([Strangers' worlds](strangers-worlds.md)).
- **Arrivals.** The manual's portals page has a far world publish the landing places a portal can name. Recommended:
  they stay.
- **Preparing a far world before the person asks.** Options: only on the person's explicit ask; also on proximity; or
  as the world declares its neighbours. Recommended: only on the ask until a reason to allow more is measured.
- **Who fetches a person's avatar into a world,** and how other people's browsers get it in a connected world: from the
  person's own store, from the registry, or through the world's server. It touches the Loader. What a registry fetch
  reveals (which avatars are near whom) depends on how the registry serves files; caching close to readers hides most
  of it.

## What lives elsewhere

- [The person](person.md): a person's avatar, its camera, and the registry choice per world.
- [Arrival](arrival.md): the content key, and a part showing when its content is resident.
- [Worlds and zones](worlds-and-zones.md): entering a world, zones, and what travels with the person.
- [Strangers' worlds](strangers-worlds.md): what an imported part's code may reach.
- [The content language](content-language.md): how a world's parts are written.
- [Sim](../actors/sim.md) and [Loader](../actors/loader.md): owners of the live parts and of what is built.
- The manual's parts, designs and packages pages, which this note rules over.
