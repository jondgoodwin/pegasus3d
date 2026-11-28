# Live editing

`[planned]`

This note covers changing a world while it runs: creating and destroying parts, editing in place, saving back, and what a change to a world's code does. It takes the position that editing is a first-class part of play, on a person's machine and on a server, and that a change to a world's code reloads the world. Today the studio re-reads its configuration and documents when files change, and the island creates nothing after start-up.

## Principles

**Editing is live, in the world; creating and destroying objects on the fly are first-class.** It forbids a world that must restart to gain or lose an object.

**Changing a world's code reloads the world.** It forbids swapping code under a running instance.

Inherits: [Parts and imports](parts-and-imports.md) (a part is an id, and Sim owns the live parts), [Worlds and zones](worlds-and-zones.md) (an instance starts from the zone's definition or a save), [Arrival](arrival.md) (new content reaches a part as any content does).

## The design

### Design choices

- **Editing, creating and destroying objects live are requests to Sim, which asks the Loader to build anything new.** It forbids another actor changing a part, or building content for it, on its own. `[design choice]`

### Creating, destroying and editing

- The studio is one case of live editing. Servers edit too, as people change their own island or character. Games often lack this; the browser needs it.
- A created part's id is allocated by Sim at once, with no content yet, and the part shows when its content is resident ([Sim](../actors/sim.md), [Loader](../actors/loader.md)).
- Render owns the meshes and textures it draws; an editor hands Render a copy ([Render](../actors/render.md)).
- The studio is a local world whose data comes live from local files and which its owner reshapes locally.

### Changing a world's code

Changing a world's code reloads the world: the running instance ends and a new instance starts from the changed definition ([Worlds and zones](worlds-and-zones.md)). Nothing swaps code under a running instance, and an instance is never copied live. What state survives a reload is open.

## What exists today

- **The studio re-reads files** (`C:\src\pegasus3d-wt-incarch17\guidebrowser\src`, branch `incubator-archwalk17`). It polls its two configuration files every 250 ms and compares whole text (`incubator.cone`, `incubatorcfg.cone`): one holds what exists and where it is meant to be, the other what the person did in the room; the newer entry per item wins, a bad edit keeps the last good layout and shows the error, and the person's moves are written back one second after the last change through a temporary file renamed into place. Documents reload on a change of modification time, with margin marks on changed blocks, and keep the last good page when a change fails to parse (`docpanel.cone`). Reloading is done by the loop thread; who does it in the design is open. `[differs: closing a page stalls on the GPU (a wait for device idle in incubator.cone), and nothing may delay the picture]`
- **The studio loads a world file once per process:** `Loader.arrive` has one call site (`guidebrowser.cone`). `[differs: a changed world file does not reload the world]`
- **The island creates nothing after start-up** (`C:\src\pegasus3d`, master): the world builds its objects one a turn and then only answers mail; Sim spawns people, creatures and the portal puppet itself (`browser\src\simactor.cone`). A despawn does not free the part's GPU meshes (`browser\src\parts\parts.cone`). The replay log drops a despawned part's entries (`replaylog.cone`). `[differs: creating and destroying are requests to Sim for anyone, and freeing content is proposed below]`
- **Changes by file come from an editing session:** the studio's changes to documents and to the room arrive as edited files that the studio reloads (`incubator.cone`, `docpanel.cone`); the person's click positions go back as a log file.

## Open

- **Who asks, and where.** Whether a world's own code, an editor and a person's tool all ask Sim the same way; and whether, on a person's machine, the request goes to the Sim (the whole role in a local world, the client Sim otherwise) and, in a shared world, to the server Sim, which owns the truth. Recommended: yes to both.
- **Who owns a mesh being edited.** Recommended: the editor that edits it, handing Render a copy.
- **What counts as code.** A change to a world's definition may be a change to its code or to its data (a part's parameters, a document). Options: any change reloads the world; or data changes apply as edits to the live parts while only code changes reload. Recommended: compare the new part tree with the running one by qualified path and send only the added, changed and removed parts, so unchanged parts keep their state; a mistake is shown and the old world keeps running.
- **What state survives a reload.** Options: nothing, so the new instance starts from the definition or a save; the viewer's own state (position, what is open) outside what reloads; state a world marks as lasting. Recommended: state outside what reloads, stable names for parts, the swap between frames, and a bad load refused so the last good one stays.
- **How a change to a local world's file reaches the running world.** Recommended: the Loader watches the world's origin and sends Sim new content for the changed key; a surface watches its own one source.
- **Freeing content.** Recommended: the Loader's catalog counts the parts that use each content key, frees what nobody uses, and re-supplies after eviction; Sim tells the Loader that a destroyed part has let go of its content.
- **Saving back.** Recommended: an edit saved is written back to the world's origin by the Loader, as bulk; a studio's room layout is the world's content.
- **Who may edit.** On a shared world the server Sim accepts edits only from a person with the privilege to edit that world. Recommended: an edit carries the person's standing with it, and a world's own rules cannot give themselves it.
- **Editing by an AI companion.** Whether a companion edits through the person's tools and grants, or as a character of the world. Recommended: through a few tools, changing data rather than writing code in the live loop.

## What lives elsewhere

- [Sim](../actors/sim.md): requests to create, destroy and edit parts.
- [Loader](../actors/loader.md): building what is new, the catalog, writing back.
- [Render](../actors/render.md): the copy of an edited mesh.
- [Surfaces](../actors/surfaces.md): documents and pages that reload their own source.
- [Parts and imports](parts-and-imports.md): ids and content keys.
- [Strangers' worlds](strangers-worlds.md): what an editor may do inside a world.
- The manual's actions and designs pages: how a world's code spawns and removes parts.
