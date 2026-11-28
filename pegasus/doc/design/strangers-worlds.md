# Strangers' worlds

`[planned]`

This note covers trust, privacy and safety: what a world's code may reach, what a world may learn about the person, what a world may cost, and what the browser always keeps for itself. The position is that the world is the unit of isolation, and that the language and a time budget guard the engine inside it. Much of this subject is open: how a stranger's code is turned into something that runs, what rights an asset's origin gives, and what the browser keeps for its own prompts and inputs. Today the island's world is trusted compiled code with no watchdog, and the studio's worlds carry almost no code.

## Principles

- **A slow or hostile world can hurt itself, not the browser and not another world; privacy, security and trust are part of this.** It forbids any path by which one world stalls, reads or changes another world or the browser.
- **A world may be local, built and reshaped on its owner's own machine.** It forbids isolation that assumes every world comes from a server.
- **Within a world, the language protects the engine, as with JavaScript.** It forbids world code that touches an actor's data other than through the language.

Inherits: [The content language](content-language.md) (ConeScript is resilient; what a world can name), [The person](person.md) (what the person's side keeps from a world).

## The design

### Design choices

- **The isolation line goes around the world, as browsers isolate sites, not between script and physics.** It forbids isolating a world's code from its physics inside one world. `[design choice]`
- **A world's code runs on a time budget with a watchdog.** It forbids world code that runs unbounded. `[design choice]`
- **A world learns only what Player hands over.** It forbids a world reading identity, devices, typing or body data on its own. `[design choice]`
- **A stranger's world is never run as native code it supplied.** It forbids a server or a world sending compiled code for the browser to run. `[design choice]`
- **A world that hangs stalls only itself:** its own Sims and nothing outside the world. It forbids a hung world stalling another world or the browser. `[design choice]`
- **The smallest unit that can stall is a zone's Sim,** and a handler's budget bounds the stall to one step. A world's zones are separate Sims and do not stall each other. `[design choice]`
- **What the person types into a focused page goes from Player straight to that page,** through no world. It forbids a world reading what the person types into a page. `[design choice]`

### The unit is the world

- The line goes around the whole world: its zones and their Sims, its code and its memory.
- A world may be local, with no network. A person building their own space on their own disk is in a world like any other, and the same line surrounds it.

### What a world's code may reach

- A world's handlers reach parts only through the language's interface, as a script in a web page reaches the page only through the browser's interface. The budget on a handler is a named quantity, unmeasured. A handler that overruns it is stopped and its changes dropped, the world is told, and the rest of the step runs ([Sim](../actors/sim.md)).
- A world's requests into the browser and person domains are a short list ([Actions and triggers](actions-and-triggers.md)); everything else about the person is Player's to give.

### What a world learns

- What Player hands over is in [The person](person.md).
- For now, a far world is contacted only when the person asks to go there or to look, because contacting a world's server tells that server the person's address. This may be relaxed.

### Strangers that are not worlds

- A web page shown on a part is a stranger. The engine that renders it brings its own sandbox.

### Failure

ConeScript's resilience ([The content language](content-language.md)) means, for isolation, that a failure stays inside the part, step or handler that failed.

- For now, a part with an unrecoverable error does not exist, and its error is reported.
- A failure in the GPU is reported and is never a panic.

## What exists today

- The island's world holds only a `Door`: it reserves ids, sends requests and reads mail, and never sees a scene table, the render system or a GPU handle (`browser/src/worldactor.cone`, `browser/src/bakedworld/bakedworld.cone`, `browser/src/parts/parts.cone`). It is trusted compiled Cone code in the same process. `[differs: no time budget and no watchdog on a handler; Sim awaits the world's turn in single player, so a hung world freezes everything]`
- The island has one world, and its portal joins two places in one scene (`browser/src/parts/portal.cone`). `[differs: no portal between worlds, no admission of an avatar into another world]`
- The island's identity is a name string checked at join for length, characters and uniqueness; there is no login and no encryption (`browser/src/simactor.cone`, `browser/SERVER.md`).
- The studio's `.ddd` worlds can only produce a program of parts, fields, areas, derives, click rules and moves, with no handler code (`guidebrowser/src/understand.cone`, `guidebrowser/src/world.cone`). Its Incubator is trusted Cone code that touches the whole machine: files, injected input, web pages and the session folder.
- The studio hands typing to a focused web page or document before any world sees it (`fn event` in `guidebrowser/src/guidebrowser.cone`).
- The studio's web pages run with the engine's sandbox off (`guidebrowser/cef/src/cefshim.cc`).

## Open

### Designs not yet settled

- **A world's memory.** Proposed: the memory a world's code uses (its behaviour state) belongs to its zone's Sim, with that Sim's own collector, and no other actor reads it. Recommended: yes.
- **Secrets.** Proposed: a world that wants a secret keeps it in a field or rule that lives only on the server, out of the description anyone can download. Recommended: yes.
- **What a world receives at entry.** Options: a statement from Player (a world's own id for the person, a chosen name and avatar, claims such as guest or an age band), with the world able to ask for grants; a conversation with the identity service. Recommended: the statement, and a world never reads identity on its own.
- **Packages and imports.** Options: a world never names a package (packages are Cone code shipped with the browser) and imports parts, which are content under the same limits; or a world names approved packages. Recommended: the first, because a world that could name any package could name a network library.
- **The web page sandbox.** Recommended: switched on for every stranger's page.
- **How a stranger's code runs.** Three statements stand together: the language must not feel like two languages; the content language is much simpler than Cone and may get a generator of its own; the Loader compiles behaviour code to native code. Options: Cone's compiler restricted to what the language allows; a small generator of the content language's own; an interpreter first. Whatever turns a world's description into runnable form is attack surface and is treated so (fuzzed, budgeted). Recommended: the description always arrives as source and the browser's own generator, which emits only what the language allows and inserts the budget checks, turns it into runnable form. Which generator is not settled.
- **Failure beyond one part.** What a person sees and what a world is told when a part fails to build, a handler is dropped, a zone's Sim dies or the server goes away; how a fault in one world's code is confined to it; how the browser shuts down cleanly; what recovery after a part's error means.
- **Preparing and entering through a portal.** Whether preparing a far world and entering it are two requests to Player; what an imported portal part may do inside its host beyond its one request. Recommended: two requests.
- **What a registry fetch reveals** about which parts and avatars are near whom. Recommended: caching close to readers, which hides most of it.

### Deferred, with safety and identity

Not decided; taken up with safety and identity.

- **The origin decides what an asset may do.** An asset's rights would follow from where it came from and stay the same wherever it is placed. It would forbid rights that depend on where an asset is used. Questions: who checks it at entry (Player is the candidate); what a world read from the person's own disk may do that a remote world may not.
- **The browser draws its own prompts and keeps its reserved inputs.** It would forbid a world drawing sign-in or consent, or taking the way out. Questions: the list of reserved inputs (the exit, the menu, the summoning gesture, the permission prompts, a panic mode); which prompts only the browser may draw; whether the destination a portal names is shown by the browser's own interface before crossing, because a host world could dress any portal as another.
- **The browser's own interface.** Options: a trusted local world with its own Sim, written in the content language, drawn above the worlds and tested first for clicks; or plain code inside Player. Recommended: the local world, with the cost that the browser's most trusted prompts then depend on the content language's runtime being correct.

## What lives elsewhere

- [Parts and imports](parts-and-imports.md): what an imported part is and whose authority it runs with.
- [The person](person.md): identity, grants, the private body, safety and comfort.
- [The content language](content-language.md): resilience, and why a world can name no thread, socket or file.
- [Worlds and zones](worlds-and-zones.md): entering a world.
- [The frame and its deadlines](frame-and-deadlines.md): why a world's code cannot delay the picture.
- [Player](../actors/player.md): what Player hands over and the browser's own controls.
- [Sim](../actors/sim.md): handlers, their budget and the world's heap.
- [Surfaces](../actors/surfaces.md): web pages as strangers.
- The manual's outside, packages and overlays pages: what content sees of the person, what a package is, what a world cannot cover.
