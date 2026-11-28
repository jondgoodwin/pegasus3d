# Pegasus browser design notes

Design context for work on the Pegasus 3D browser: what it must do, how it is shaped, and why. These notes are the
standard the code is measured against. They state what is decided, what of it exists, and what is open, and they
point at code rather than reproducing it.

Page in the note you need. Do not load the folder.

`[planned]` The browser these notes describe has no code yet. Two earlier programs carry parts of it: the island
engine (`browser\`) and the studio (`guidebrowser\`). Each note's "What exists today" section says what those two
already do that the design keeps, and what they do differently. When the new browser's code starts, that section
becomes "Built", and only divergences are marked from then on.

## What the browser is for

**A web of 3-D worlds, made by strangers, visited through one browser, in a headset or on a desktop.** Three lasting
measures, owned by the vision and the performance perspective and referenced here:

- **The web fills:** worlds worth visiting can be made at a cost that repeats.
- **It responds:** continually responsive to a tenth of a second, even when entering a completely new world.
- **Parts interoperate:** a part made for one world works in another, its behaviour included, with no coordination
  between the authors.

**The priorities are ranked:** how fast a world arrives over the network, and how fast a shared world answers, come
first; the time of each frame second; the speed of compiling third. Success is immersion: performance succeeds
when it is never noteworthy.

Every note below holds a position on a trade-off among these.

## Two kinds of note

```
doc/design/     topic notes: one concern followed across every actor
doc/actors/     actor notes: one per actor, plus the messages between them, the network and the server
```

**A topic note follows one concern across every actor it touches.** Its principles are about the 3-D web as a
platform. **An actor note describes how this browser is built.** Its principles are about this program. The test
that sorts a principle: if the browser were rewritten from scratch, would it survive? A topic note's would; an
actor note's would be thrown away with the code.

**Every note has the same sections, separated by headings and never by annotation sprinkled through prose:**
the principles it owns; the decided design those principles rule over; what exists today; what is open.

**An actor note's decided design has one shape:** the actor's role; who it works with (what comes in and what goes
out, each with its kind, clocks included); what it owns; how it spreads its work over cores.

## Principles: own them, or inherit them

**A note states the principles it owns and references the ones it inherits.** Writing out an inherited principle
is how two notes come to disagree about it.

**The owner is the note whose subject the principle is.** Where no note has it as its subject, this index owns it.

**Every principle names something it forbids or settles.** If it would reject nothing, it is decoration.

**A principle is one the author has stated or agreed, and the table below holds only those.** A rule chosen in
the design work to serve them is a design choice: it is written in its owner note's design, under "Design
choices", worded as decided and marked `[design choice]`. A design choice is a requirement that helps the design;
it yields to a principle wherever the two meet, and it is the first thing to question when a design fits badly.

### The principles this index owns

- **All the cores:** work overlaps wherever the data allows. It forbids a design in which only one actor can run
  at a time.
- **Repeatable on demand:** any run can be replayed exactly for testing, and normal running does not pay for
  that.

### The principle table

| Principle | Owner | What it forbids or settles |
|---|---|---|
| All the cores | this index | a design where one actor runs at a time |
| Repeatable on demand | this index | paying for exact replay in normal running |
| A world appears progressively, and loading never shares a deadline with a frame | [Arrival](arrival.md) | a loading screen that blocks the picture; a frame that waits for content |
| Worlds arrive as programs with embedded data, not meshes; the description crosses the network and is built on the person's machine | [Arrival](arrival.md) | sending built geometry or textures over the wire |
| Nothing delays the picture: not a world's code, the network, loading or memory cleanup | [The frame and its deadlines](frame-and-deadlines.md) | any of those on the path of a frame |
| Head movement always gets the newest pose | [The frame and its deadlines](frame-and-deadlines.md) | a head pose that waits on another actor |
| A slow or hostile world can hurt itself, not the browser and not another world; privacy, security and trust are part of this | [Strangers' worlds](strangers-worlds.md) | any path by which one world stalls, reads or changes another world or the browser |
| A world may be local, built and reshaped on its owner's own machine | [Strangers' worlds](strangers-worlds.md) | isolation that assumes every world comes from a server |
| Within a world, the language protects the engine, as with JavaScript | [Strangers' worlds](strangers-worlds.md) | world code that touches an actor's data other than through the language |
| Where to isolate, and by which means, is chosen per structure: an actor where the processing, not just the data, must be isolated; a lock where data is shared independently of processing; other structures transit freely | [Isolation and ownership](isolation.md) | one mechanism applied everywhere; an actor used only to protect data |
| Local data and local ownership are preferred | [Isolation and ownership](isolation.md) | a shared structure where an owned one would do |
| The server owns the truth of a shared world; a world with no server runs whole on the person's machine | [Local and connected](local-and-connected.md) | a client's word deciding a shared outcome; a world that needs a server to run alone |
| A body's position is relative to the moving thing it stands on, and the eyes are placed at the last moment | [Local and connected](local-and-connected.md) | a rider corrected because the platform moved; solving a moving platform by knowing its path |
| A zone is closed and simulated by its own Sim; a person is in one zone at a time | [Worlds and zones](worlds-and-zones.md) | state shared live between zones; a person simulated in two places |
| An instance starts from the zone's definition or a save, never from a live copy | [Worlds and zones](worlds-and-zones.md) | copying a running zone |
| World-wide state is distinct from zone state | [Worlds and zones](worlds-and-zones.md) | a zone's Sim owning what must outlive or span zones |
| The browser offers the superset of mechanisms; a world's builder chooses the policies | [Worlds and zones](worlds-and-zones.md) | one world's rules built into the browser |
| Player acts on behalf of the human at this machine | [The person](person.md) | anything that is not the human at this machine being a Player |
| The avatar is a part in the world and its behaviour is always the world's; which avatar, and how its camera is set, are the person's | [The person](person.md) | an avatar that brings its own rules into a world |
| An action is one data structure everywhere: a verb, a receiver, parameters | [Actions and triggers](actions-and-triggers.md) | a second encoding of "do this" in any actor |
| A trigger is a stimulus and its response, and the response is one or more actions | [Actions and triggers](actions-and-triggers.md) | reactions that are not expressible as triggers |
| Everything reactive is an action; everything else between actors is a message | [Actions and triggers](actions-and-triggers.md) | snapshots, bulk transfers, lifecycle or replies dressed as actions |
| Sim owns the live parts and everything attached to them | [Parts and imports](parts-and-imports.md) | another actor holding or changing a part |
| A world can import parts published elsewhere; the author keeps authorship and the renderable form is available to everyone | [Parts and imports](parts-and-imports.md) | a part visible only to those who hold its source |
| Editing is live, in the world; creating and destroying objects on the fly are first-class | [Live editing](live-editing.md) | a world that must restart to gain or lose an object |
| Changing a world's code reloads the world | [Live editing](live-editing.md) | swapping code under a running instance |
| The specification of world content is public, as HTML is | [The content language](content-language.md) | a private or browser-specific format |
| ConeScript is resilient: a world's code runs as well as it can regardless of mistakes. Whatever fails is isolated (a failing part, tool step or handler is dropped or skipped), the rest runs, and the browser produces very few error messages | [The content language](content-language.md) | a mistake that stops a world loading or running; stopping at the first error as Cone does |

## Topics

One note per concern the browser takes a position on. What earns a topic note is a position on a trade-off that
could have gone the other way.

| Note | The position | The distance today |
|---|---|---|
| [Arrival](arrival.md) | A world arrives as a small program and is built on the person's machine, off every frame | the island is compiled in and built twice at start-up; the studio fetches one world and builds it off the loop, uploading unbudgeted |
| [The frame and its deadlines](frame-and-deadlines.md) | Nothing delays the picture | the island runs its actors in turn when alone and overlapped when connected; the studio does nearly everything on one thread |
| [Strangers' worlds](strangers-worlds.md) | One world cannot affect another, or the browser | the island's world is trusted compiled code with no watchdog; the studio's worlds carry almost no code |
| [Isolation and ownership](isolation.md) | The means of isolation is chosen per structure | data is moved between actors in the island; several structures are built or held twice |
| [Local and connected](local-and-connected.md) | The server owns a shared world's truth; a world alone runs whole on one machine | two modes in the island; the studio has no network play |
| [Worlds and zones](worlds-and-zones.md) | A world is zones, each closed and simulated alone, with world-wide state apart | one world, one zone, in both programs; the island's portal joins two places in one scene |
| [The person](person.md) | Player acts for the human; the avatar is the world's part and the person's choice | a name at join in the island; no identity in the studio |
| [Actions and triggers](actions-and-triggers.md) | One reactive currency from device to world | five encodings of an action across the two programs |
| [Parts and imports](parts-and-imports.md) | Worlds are built of parts, their own and imported | no import in either program |
| [Live editing](live-editing.md) | Worlds change while they run; a code change reloads | the studio reloads on a file change; the island creates nothing after start-up |
| [The content language](content-language.md) | The seam with the ConeScript manual | the studio interprets a subset; the island uses none |

## Actors

| Actor | Job | Owns | Note |
|---|---|---|---|
| Audio | Sound out on time | the mixer and the speakers | [Audio](../actors/audio.md) |
| Render | The picture on time; the only submitter to the GPU; reads the head pose | the GPU's resources; the display and headset; draw meshes and textures | [Render](../actors/render.md) |
| Player | Acts for the human at this machine | the session, settings, devices and bindings | [Player](../actors/player.md) |
| Sim | Everything that changes the parts of one zone; one program in three roles: whole, server, client | parts and their state, the shapes, physics, the world's code | [Sim](../actors/sim.md) |
| Loader | Assets in, and built | transports, the build pipeline | [Loader](../actors/loader.md) |
| Surfaces | Live 2-D pictures: documents, web pages, the desktop | each producer's source and picture | [Surfaces](../actors/surfaces.md) |

Beside them: [Messages](../actors/messages.md), the kinds of handshake and who sends what to whom;
[Network](../actors/network.md), the library the client Sim and the Loader both use; [Server](../actors/server.md),
the program that runs a shared world.

## The words

One meaning each. Where the ConeScript manual uses a word differently, the entry says so.

| Word | Meaning |
|---|---|
| World | The whole thing a builder publishes and a person enters. |
| Zone | One closed, separately simulated piece of a world. A world has one or more. The manual calls this a scene; which word stays is open. |
| Instance | One running copy of a zone, started from its definition or a save. |
| Region | Reserved for a later idea: part of a world too big for one server, drawn across its borders. Not a zone. |
| Part | The unit a world is made of: a node in the world's tree. |
| Being | A part that acts: an avatar, a villager. |
| Avatar | The being a person inhabits. Its camera is a part of it. |
| Person | The human. The manual's authors meet them as a visitor. |
| Player | The actor that acts for the person at this machine. |
| Action | The data structure that says "do this": a verb, a receiver, parameters. |
| Trigger | A stimulus and its response; the response is one or more actions. |
| Design choice | A rule chosen in the design work to serve the principles; not a principle. |
| Whole, server and client Sim | The three roles of the one Sim program: whole (local play: it holds the truth and runs every rule), server (the truth of a shared zone), client (on the person's machine in a shared zone). |
| State | A message whose newest version replaces the older ones; a missed one costs nothing. |
| Platform | A part other bodies stand on or ride; its frame of reference is the platform's frame. "Frame" alone means one drawn picture. |
| Grant | A permission the person gives a world, such as the microphone. |
| Origin | Where a world or an asset came from. |
| Binding | A trigger whose stimulus is a device signal. |
| Handler | A trigger's response when it is code in the world's language. |
| Message | What actors send each other. An action travels inside one. |
| Snapshot | The state of a zone's parts after one step, published whole and never changed. |
| Latest value | Data whose newest version is all that matters: one writer overwrites, readers never wait. |
| Event | A message that must arrive once, in order. |
| Bulk | A large transfer accepted over many frames. |
| Move | Handing a thing to a new owner; the sender no longer has it. |
| Content key | The identity of a built thing: what was built, from what, with what arguments. A part refers to its content by key. |
| Surface | A live 2-D picture with its own source and clock, shown on a part. |
| Step | One fixed advance of a zone's simulation. |

## The manual

The ConeScript manual is the detailed specification of what a world's author writes and what it means. These notes
say what the browser must do and why. Where a decided design changes what a page says, the page changes with it.

| Manual pages | Ruled over by |
|---|---|
| actions, events, subscriptions, behaviors | [Actions and triggers](actions-and-triggers.md), [Sim](../actors/sim.md) |
| visitors, avatars, outside | [The person](person.md), [Strangers' worlds](strangers-worlds.md) |
| worlds, portals | [Worlds and zones](worlds-and-zones.md) |
| loading | [Arrival](arrival.md) |
| distribution | [Local and connected](local-and-connected.md) |
| parts, designs, packages, composing | [Parts and imports](parts-and-imports.md) |
| frames, cameras | [The person](person.md), [Render](../actors/render.md) |
| overlays | [Player](../actors/player.md), [Surfaces](../actors/surfaces.md) |
| the conventions, the introduction, open questions | [The content language](content-language.md) |

## Conventions

- **Status.** A note is `[planned]` as a whole until the new browser has code. Inside "What exists today", a claim
  names the program (the island, the studio) and the file that carries it.
- **`[differs: ...]`** marks where one of the two programs does something the design replaces; the design reads as
  ordinary prose and the divergence sits in the brackets.
- **No history.** A note says what is true now. Who decided what, when and why is in the decisions log; superseded
  reasoning and dated evidence are in the research folder.
- **`[design choice]`** marks a rule chosen in the design work and not stated by the author. It appears only
  under a note's "Design choices".
- **Open** holds what is undecided, each question with its options and, where there is one, the recommendation,
  marked as a recommendation. What is put off sits under a "Deferred" subheading that names what it waits for, and is not decided.
- **Budgets and measures** are owned by the performance perspective. A note names a budget and points there; a
  budget not yet measured is marked unmeasured.

## Open

- How each actor spreads its work over cores. Each actor note states what is known; none is settled in detail.

## Ripple: what a change here reaches

For decided facts with no code behind them, drift cannot be recovered by reading the code, so each is listed with
its consumers.

| Fact | Owner | Consumers |
|---|---|---|
| The six actors and what each owns | this index | every actor note; the message table |
| The meaning of zone, action and trigger | this index | every note; the manual's worlds, actions and events pages |
| What a snapshot holds and who reads it | [Sim](../actors/sim.md) | Render, Audio, Network |
| The order of a step | [Sim](../actors/sim.md) | Actions and triggers; The content language; the manual's events and subscriptions pages |
| Sim's three roles, and the protocol of connected play | [Sim](../actors/sim.md) | Local and connected, Network, Server |
| Render never waits for Sim, reads the head pose and is the only GPU submitter | [Render](../actors/render.md) | The frame and its deadlines, Player, Loader, Surfaces |

## Boundaries between notes are moment-in-time decisions

Which note carries which material was decided under one focus and is expected to move. Moving material and
redrawing boundaries is encouraged: propose it first and record it when it happens, so a note is never read as
having quietly lost something.

## What a decided-design change records

What thinking fed it, what prior art was surveyed, what was measured. Research is required where a decision has
prior art we do not know, not to re-justify what is settled.
