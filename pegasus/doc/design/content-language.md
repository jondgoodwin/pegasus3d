# The content language

`[planned]`

This note is the seam between the browser and the ConeScript manual. It takes the position that the manual is the detailed specification of what a world's author writes and what it means, that these notes rule over it, that the specification is public, and that the language itself, not a separate sandbox, protects the engine. Today the studio interprets a subset of the language and the island uses none.

## Principles

**The specification of world content is public, as HTML is.** It forbids a private or browser-specific format.

**ConeScript is resilient: a world's code runs as well as it can regardless of mistakes. Whatever fails is isolated (a failing part, tool step or handler is dropped or skipped), the rest runs, and the browser produces very few error messages.** It forbids a mistake that stops a world loading or running, and stopping at the first error as Cone does.

Inherits: [Strangers' worlds](strangers-worlds.md) (within a world the language protects the engine, as with JavaScript), [Local and connected](local-and-connected.md), [Actions and triggers](actions-and-triggers.md).

## The design

### Design choices

- **Content states requirements, never mechanisms, as a working hypothesis.** A world says what it needs (this is solid, this is heard, this responds when touched), and the browser decides how. It forbids a world naming a thread, a socket, a file or a GPU feature. `[design choice]`

### The manual and the notes

- The manual is the detailed specification: exactly what a world's author writes and what it means. These notes say what the browser must do and why. The notes rule over the manual, and where a decided design changes what a page says, the page changes with it.
- The pages this note rules over are the conventions, the introduction and the open questions; the index maps every other page to its note.
- A world arrives as a program with embedded data ([Arrival](arrival.md)); the language that expresses both is the content description language.
- New capability reaches content as new names in the browser's vocabulary, never as new syntax, as a new HTML tag can appear while a style sheet still applies. The vocabulary has no name for a thread, a socket, a file or memory, so a world cannot ask for them. This too is a working hypothesis.

### What resilience means

- A part that fails does not exist, and the rest of the world arrives.
- A tool step that fails is skipped, and the rest of the part is built.
- A handler that fails is dropped, and the rest of the world runs.
- The manual's rule that a tool's refusal stops the world loading changes to match.
- The browser produces very few error messages; a failure it cannot survive is the exception, not the rule.

### What the browser owes the language

- **Where a rule lives** (on the server, on the client or on both) is open in [Local and connected](local-and-connected.md).
- **The order of a step.** What happens in what order within one step of a zone's Sim is owned by [Sim](../actors/sim.md), which decides a reactive order; the manual's events and subscriptions pages follow it.
- **What a handler may reach.** A handler reaches parts only through the language's interface, under a time budget with a watchdog. See [Strangers' worlds](strangers-worlds.md).

### How a world's code becomes something that runs

A world arrives as a program with embedded data ([Arrival](arrival.md)). The browser compiles the world's source itself and never runs native code a stranger supplied ([Strangers' worlds](strangers-worlds.md)). The compiler is part of the Loader. Its intermediate form is the browser's own, and there is no LLVM in the browser. The first working version runs on an interpreter; a native backend, written in Cone, comes later. Changing a world's code reloads the world ([Live editing](live-editing.md)). The stages, the intermediate form, the calls into Cone libraries and the cache are in [Loader](../actors/loader.md), most of them open there.

## What exists today

- **The studio's interpreter** (`C:\src\pegasus3d-wt-incarch17\guidebrowser\src`, branch `incubator-archwalk17`). A world file passes through `lexer.cone`, `parser.cone`, `expand.cone`, `understand.cone` (the checker, with readable errors) and `build.cone` ("the interpreter": it walks the checked values and calls the builders they name), producing parts and a program for the world actor (`world.cone`). Limits: two triggers only, `created` and `clicked(part)` (`understand.cone`); areas feed derived values but raise no entered or left event; a field may live only on the client; `solid` is set aside (`world.cone`); no handler code, no timers. `[differs: the design's language has handlers under a budget]`
- **The island uses no content language** (`C:\src\pegasus3d`, master). Its world is Cone code compiled in (`browser\src\bakedworld\bakedworld.cone`) that answers mail with requests through a door (`Request` and `Message` in `browser\src\parts\parts.cone`). The files named `script.cone` are scripted test runs, not content. It has no budget or watchdog on a handler.
- **The manual** (`C:\src\conescript-manual`). Its conventions ask every page to open with a status banner, and the pages carry none; only the draft story page and the actions proposal pages do. Status appears inline ("proposed", "working hypothesis") and in each page's open questions. The manual names none of the browser's actors; it speaks of the browser, the engines, the server and each visitor's machine.

## Open

### Proposals

- **Where the line falls between a requirement and a mechanism** (a level of detail, a quality tier, a priority), and whether a world may state priorities and qualities of experience.
- **What "public" commits to.** Recommended: any browser may implement the specification and any author may write to it.

- **One language or two.** The language must ultimately not feel like two languages, yet ConeScript is much simpler than Cone, and it serves two kinds of code with different needs: Sim code (reactive, latency matters) and procedural code (bulk mesh building, throughput matters, identical results on every machine). Options: one language and one semantics for both, run by an interpreter first and a native backend later; or two languages or two semantics, one for each kind. Recommended: one language and one semantics; code whose inputs are fixed is evaluated at load and leaves data, so a world is mostly data and only the live remainder is code, and build-time evaluation uses the same interpreter as run-time code. The detail is in [Loader](../actors/loader.md).
- **Versioning.** Three stories do not meet: packages with semantic versions and several live at once; one vocabulary version per browser, with a minimum a world may state; a layout digest with at most two live versions. Recommended: one story: the vocabulary has one version per browser with a minimum a world may state; parts are pinned by content; live editing uses the digest. To be argued, not assumed.
- **One part changing another.** The manual holds two working hypotheses side by side: a part never changes another part's state directly, it sends a message and the receiver decides; and anything in the world can find and change anything else by its path. Both are open.
- **The actions proposal.** Its pages are a proposal that would change the manual's actions, events, subscriptions and visitors pages if accepted: one trigger form, a handler whose effects take effect together or not at all, a different order of a step, timers, and a record for every action.
- **Whether a world's source is always viewable.** Options: always, as a page's source is; or at the world's author's choice.
- **Whether the language's `number` is a 64-bit float.** Leaning yes; undecided.

## What lives elsewhere

- [Local and connected](local-and-connected.md): where a rule lives, what crosses the network.
- [Sim](../actors/sim.md): the order of a step, handlers and their budget.
- [Strangers' worlds](strangers-worlds.md): what a handler may reach, what a world learns.
- [Actions and triggers](actions-and-triggers.md): the action, the trigger, the reactive pages of the manual.
- [Arrival](arrival.md): worlds as programs with embedded data.
- [Loader](../actors/loader.md): how a world's code becomes something that runs.
- The manual: every page; the index maps pages to notes.
