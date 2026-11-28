# Worlds and zones

`[planned]`

This note covers what a world is made of while it runs: its zones, the instances that simulate them, the state that spans zones, the world's own owner, and how a person enters and leaves a world. It takes the position that a world is zones, each closed and simulated by one Sim, with an owner for what spans them, and that entering a world goes through Player. The world's owner, the steps of entering and how crossing stays seamless are open. Both programs today have one world with one zone, and the island's portal joins two places in one scene.

## Principles

**A zone is closed and simulated by its own Sim; a person is in one zone at a time.** It forbids state shared live between zones, and a person simulated in two places.

**An instance starts from the zone's definition or a save, never from a live copy.** It forbids copying a running zone.

**World-wide state is distinct from zone state.** It forbids a zone's Sim owning what must outlive or span zones.

**The browser offers the superset of mechanisms; a world's builder chooses the policies.** It forbids one world's rules built into the browser.

Inherits: [Strangers' worlds](strangers-worlds.md) (the world is the unit of isolation), [Isolation and ownership](isolation.md), [Parts and imports](parts-and-imports.md) (a portal is an imported part), [Arrival](arrival.md), [The person](person.md).

## The design

### Design choices

- **Entering a world runs one sequence, whatever starts it.** It forbids a portal, a link and a restored session each entering their own way. `[design choice]`
- **Entering a world goes through Player, whatever starts it:** a portal, a link or an address, or reopening the worlds that were open when the browser closed. It forbids a world, or a portal in it, entering another world on its own. `[design choice]`

### Worlds, zones and instances

- A world is the whole thing a builder publishes and a person enters. It has one or more zones. A zone is one closed, separately simulated piece of a world: an amusement park is a world and each ride is a zone. The manual calls a zone a scene.
- Each zone is simulated by its own Sim ([Sim](../actors/sim.md)). Zones are closed: nothing in one zone reaches into another, and the isolation line goes around the world, not around a zone.
- A person is in one zone at a time.
- An instance is one running copy of a zone. It starts from the zone's definition (its defaults), from the zone's saved state, or from a save. A running instance is never copied; the word is "instance", not "copy".
- Changing a world's code reloads the world ([Live editing](live-editing.md)); an instance is never copied or patched live.
- The browser offers the superset of mechanisms for zones and instances; a world's builder chooses the policies ([the index](_index.md)).

### World state and the world's owner

- State that spans zones, or must outlive them, is world-wide state, distinct from zone state. The person's possessions in a world travel from zone to zone and are world-wide state.
- A world has an owner beside its zone Sims. What it holds, where it runs and who starts it are open (see Open). Until worlds have more than one zone, Player starts a zone's Sim and asks the Loader to open the world for it; that job moves to the world's owner after ([Sim](../actors/sim.md)).

### Regions

A region is reserved for a later idea: part of a world too big for one server, drawn across its borders with porous rendering. A region is not a zone.

### The partition for play over a network

Zones are the partition for now. Each zone has its own body cell, the unit the network's snapshot is cut by. A grid of regions inside a zone is deferred; the crowd one zone can hold stays a real limit.

### Entering a world

- Crossing a portal is the entering sequence as it is settled so far, below; a link, an address and reopening the browser's worlds run the same steps from step 2. Whether the sequence checks an origin rule waits on safety and identity (see Open).

#### Design choices: crossing a portal

- **Preparing and entering are two requests a portal sends Player:** prepare world X at arrival R, and enter world X at arrival R. Both are actions in the browser's domain, among the short list a world may send. Player judges each and runs the rest; a world never starts a Sim or fetches another world itself. `[design choice]`
- **When a portal prepares.** On the person's ask, always. On proximity (the person entering an area the portal declares around its mouth), only when the far world is on an origin this world's session already talks to, so preparing tells no server anything it does not already know; a stranger's world on another origin is prepared only on the person's ask, as the rule above says. A portal declared `arrival: instant` that cannot be prepared crosses as `mayFade`. `[design choice]`
- **What preparing does.** Player starts Sim B for the far zone (whole for a local world; for a shared world it prepares the content only, and joins at entry, since joining hands over the person) and asks the Loader to open world X for Sim B. The Loader builds nearest to arrival R first. Sim B declares its parts, publishes the zone at its first step and holds there: a prepared zone does not run until someone is admitted (or, later, a window shows it). Preparing hands over nothing about the person. The Loader tells Player when B is ready: every build within a radius of R (30 m to start, tuned by measurement) has been delivered. A prepared world nobody enters is retired after a while, its content released. `[design choice]`
- **The order of crossing.** `[design choice]`
  1. Sim A senses the person's avatar crossing the portal's mouth (an area crossed, enforced as a gate) and sends Player "enter world X at arrival R" (event). From that step Sim A holds the avatar at the mouth and takes no more of the person's movement; the person stays in A.
  2. If B was not prepared, Player prepares it now (as above).
  3. Player sends Sim B "admit this person at arrival R" (event), with what Player allows world X to know of the person. For now that is the same as joining: a display name and a handle for this visit, no signed statement; a separate identity per world is deferred with identity ([The person](person.md)). B chooses the avatar from its declared avatars, the browser's plain figure if it declares none, and places it at R facing as R says. Nothing the person held in A crosses for now. For a shared world this is the joining sequence of [Sim](../actors/sim.md), connected play.
  4. Sim B admits at its next step and answers Player: admitted, avatar X′, camera Y′ (event).
  5. Player tells Render and Audio: inhabiting X′ with camera Y′, in zone B (event). Render draws from B's snapshot from its next frame; Player's samples go to B with a fresh sequence.
  6. Player tells Sim A: release the person (event). A removes the avatar at its next step and fires `Left`. Player then retires Sim A and tells the Loader A no longer uses its content; keys B also uses stay.
  Sim A is let go only once B has admitted the person, so the person is in exactly one zone at every step, and is never in none.
- **Cancelling and failing.** Until step 4, the browser's own controls (back, stop, home) cancel the crossing: Player retires B and tells A to let the person move again. If B cannot be reached or does not admit within a time limit (10 s to start), the same happens and Player says why. The browser's home world is the landing when there is no world to return to; until the browser has one, the landing is the world being left. Going back later is a new arrival, not an undo. `[design choice]`
- **What Render shows across the gap.** If B was ready, nothing: Render changes snapshot between one frame and the next, the head pose carried on, the body moved from the mouth to R. If not, Render fades A out (about 0.2 s) to the browser's own crossing space, a plain lit space drawn from the head pose so the headset never freezes or goes black, with a sign of progress; when the person is admitted and the parts near R are resident, it fades into B. A part of B not yet resident shows when it is, as everywhere ([Arrival](arrival.md)). `[design choice]`
- **The person's ask, for now, is a click on the portal** (its `Clicked`), which the portal answers with "prepare". Walking into the mouth before it is ready crosses as `mayFade`. The first build prepares only on the ask; proximity is the second case, built after and measured against it. `[design choice]`
- **No live view through a portal yet.** The first portals are closed doors (passage, no window), so a crossing is measured without a far zone running for its picture. A window needs the far zone stepping and Render drawing a second snapshot from the portal's camera once per eye; it comes after the crossing is measured ([Render](../actors/render.md), Open). `[design choice]`
- **What a crossing is measured by:** the time from the step that senses the mouth to the first frame drawn from B, and, unprepared, to the first frame the person controls in B. A prepared crossing is meant to take one frame. `[design choice]`

### Portals

- A world imports parts published elsewhere; a portal is such a part. It is made available by some world to others, placed in a host world, and managed as an asset separate from the world it leads to ([Parts and imports](parts-and-imports.md)).
- A portal's look is its own capability, not necessarily a live window.
- On proximity, a button or another trigger, a portal gets ready. On entry it sends an action telling the browser "we are entering a new world"; that goes to Player, which runs the entering.
- For now, a portal prepares its far world only on the person's explicit ask. Preparing fetches from the far world's server, which would otherwise learn the person's address before the person chose to go. A far world on an origin the session already talks to learns nothing new, so it may be prepared on proximity (Design choices: crossing a portal).

### Seamless crossing

Between worlds and between zones, crossing is meant to be seamless and rapid: a zone is the unit a person moves between without the pause of a loading screen. How that is met when each zone is closed and has its own Sim is open.

## What exists today

- **One zone, one world in the island** (`C:\src\pegasus3d`, master). The world is Cone code compiled in (`browser\src\bakedworld\bakedworld.cone`) and runs in one world actor (`worldactor.cone`, whose header says one world may come to hold several scenes). The wire carries a region id (`RegionState.region` in `simview.cone`); the island is region 0 and nothing creates another. The snapshot is split into a part shared by every client, encoded once, and a personal part per client. `[differs: a zone has no definition apart from the compiled code, and there is no second zone]`
- **The island's portal joins two places in one scene** (`browser\src\parts\portal.cone`, `portalsim.cone`, `viewsurface.cone`): the labyrinth lies 300 m under the island and the portal moves a being between two panes; crossing is decided inside `Being.step` by crossing a plane, in Sim and in prediction. `[differs: the design's portal joins two worlds, through Player]`
- **Entering in the island:** a client sends a name at joining (`simactor.cone`, `hello` and `admit`); going online from single player dials while play continues, and on possession the view resets Render, stops the in-process Sim and swaps its state (`viewactor.cone`, `renderactor.cone`). Identity is the name alone. `[differs: one path for a connection and none for a portal; the design has one sequence for every entry]`
- **One world per process in the studio** (`C:\src\pegasus3d-wt-incarch17\guidebrowser\src`, branch `incubator-archwalk17`): `--world NAME` or an address chooses a world file at start, and `Loader.arrive` has one call site (`guidebrowser.cone`), so there is no navigation from world to world and no portal. The incubator arranges projects as panes inside one world; its pages are not worlds. `[differs: entering a world goes through Player]`

## Open

### Proposals

- **What the entering sequence checks and hands over.** The steps are settled under Design choices. Whether the sequence checks an origin rule, and whether it hands over a separate identity for each world, wait on safety and identity ([Strangers' worlds](strangers-worlds.md), [The person](person.md)). What a person may carry from one world to another (the manual: only what the world grants and the destination accepts) waits with them.
- **Arrivals.** The manual's portals page says a far world publishes the places at which strangers may enter it, its arrivals, and a portal names one, so a world decides where strangers may enter it. Recommended: as the page says.
- **What a person sees through a portal.** Proposed: what the person sees through a portal into another world is drawn from that world's snapshot, and the person is not in it.

- **The word, and the promise of seamless crossing.** "Zone" is the word in these notes and is not final: the manual's "scene" names the same unit, and "scene" also means a registry type, a scene clock and the drawn scene. The manual's glossary uses "zoning" for a loading screen. The manual promises crossing between scenes without a pause by preparing the scene beyond a portal; whether that holds when each zone is closed with its own Sim is open. Recommended: "zone" for the simulated unit, with the manual's glossary entry changed; crossing stays seamless by preparing the next zone before the person reaches it, on the person's ask or by the world's declared neighbours; "region" stays reserved.
- **The world's owner.** Its role, what it holds (world-wide state, the person's possessions that travel between zones), where it runs, and who starts it. It takes over starting and retiring the zone Sims once worlds have more than one zone; until then Player starts them (see Design). Recommended for the rest: none yet.
- **Saved state.** What "saved" means in the language (a field's lifetime, or a list the engine owns), whose servers hold a person's home island, and a ledger for what a person owns. It belongs with the world's owner and the identity service ([Server](../actors/server.md)).
- **Navigation.** Addresses, history, back and home, and what an address names: a world, a zone, an arrival. Whether the way back and home is the browser's alone is open.
- **Showing the destination before crossing.** Proposed: the destination a portal names is shown by Player's own interface before crossing, as a browser shows a link's address, because a host world could dress any portal to look like another. Recommended: yes. It waits on the browser's own prompts, deferred with safety and identity ([Strangers' worlds](strangers-worlds.md)).
- **What a portal draws.** A portal part names the far world; Render needs, per portal, which world's snapshot to draw and from what camera. Options: a live view of the far world's snapshot; a still or slowly moving picture; a mirror until approached. Whether a far world has a Sim running behind a portal that was only prepared is open.
- **What the browser offers and what a builder chooses.** Other platforms offer these mechanisms and policies: kinds of instance (a public pool, friends, invited, a party, reserved by an address, personal); capacity; what happens to an empty instance (close, close after a grace period, freeze, save and rebuild on re-entry, fast-forward on re-entry); a world clock and a zone clock (which may follow, own, or freeze); messages between zones; a party as a group that moves together; entry gates. The list is from memory and unchecked. Recommended: declare a zone once and let policy produce instances; default an empty instance to freeze or save.
- **Moving the person's possessions between zones.** Options: the zone's Sim carries them; the world's owner holds them and hands them over atomically. Recommended: keep them outside any zone, with an atomic hand-off and one owner at a time; move the person's record and re-enter rather than migrating a running zone.
- **How many people one zone carries.** One Sim and one body cell bound it; the number is unmeasured. A grid of regions inside a zone is deferred.

## What lives elsewhere

- [Sim](../actors/sim.md): what a Sim simulates, and the world's owner beside it.
- [Player](../actors/player.md): the entering sequence, the session, the open worlds.
- [The person](person.md): the signed statement a world receives, the avatar a person brings.
- [Parts and imports](parts-and-imports.md): portals and avatars as imported parts, ids across worlds.
- [Local and connected](local-and-connected.md): the server Sim for a zone, prediction, riding between zones.
- [Arrival](arrival.md): how a world and its next zone get built and shown.
- [Strangers' worlds](strangers-worlds.md): the origin rule and why worlds cannot reach each other.
- [Server](../actors/server.md): the program that runs a shared world's zones.
- The manual's worlds and portals pages: scenes, arrivals, being ready, gates.
