# Actions and triggers

`[planned]`

This note covers the reactive layer: what an action is, what a trigger is, how a device signal becomes an action, the
four domains an action may be aimed at, and what is not reactive. It takes the position that one data structure
carries every "do this" from a device to a world's code, and that a world never sees a device. The two programs encode
"do this" five ways between them, and the manual and the code give "action", "trigger", "event" and "message" two or
three meanings each.

## Principles

- **An action is one data structure everywhere: a verb, a receiver, parameters.** It forbids a second encoding of "do
  this" in any actor.
- **A trigger is a stimulus and its response, and the response is one or more actions.** It forbids reactions that are not
  expressible as triggers.
- **Everything reactive is an action; everything else between actors is a message.** It forbids snapshots, bulk
  transfers, lifecycle or replies dressed as actions.

Inherits: [Strangers' worlds](strangers-worlds.md) (a world learns only what Player hands over).

## The design

### Design choices
- **A world sees actions, never devices.** It forbids a world reading a key, a button or a raw pose, so a person can
  remap freely and a world can still say "press E" by asking which input is bound. `[design choice]`
- **An action travels inside an actor message; a trigger is a stimulus and one or more actions; Player's bindings are
  triggers whose stimuli are device signals.** It forbids a second kind of message for reactions. `[design choice]`
- **Player and Sim talk in actions. An action is aimed at one of four domains: world, page, browser or person. The
  browser's own controls are bound, so they are actions; a world may send only a short list of requests into the
  browser and person domains.** It forbids a world reaching the browser or the person except through that list.
  `[design choice]`
- **Device input is of four kinds:** signals mapped by bindings to actions; text to the focused field, through a typing
  context Player switches on with focus (SDL's key and text streams); body tracking under permission; raw streams.
  `[design choice]`

### What an action is
- An action is a universal data structure with its own type: a verb, a receiver handle and parameters.
- The outer message is the actor message and the inner message is the action. A message is what actors send each
  other; an action is what the reactive layer trades in.
- The receiver is a handle: typically a part, or a stand-in for one, somewhere.
- A verb is named by a string. The browser interns the name to an integer, actors exchange the integer, and the name
  can be shown again when a person or an author needs to see it.

### What a trigger is
- A trigger has two parts: a stimulus, and a response. The response is an action, or a list of them, and all of them
  are sent. When the response is code in the world's language it is a handler.
- Triggers belong to the content language. They fire on parts inside the zone's Sim.
- A handler that overruns its budget is stopped and its changes dropped, so a person's own movement never waits for a world's code. The step answers triggers in a reactive order, decided in [Sim](../actors/sim.md).
- A click shows the chain. Player sends an action: pressed, along this ray, at the time of the frame the person saw,
  addressed to the person's avatar's hand. Only Sim knows what lies under the ray, so Sim turns it into the trigger
  `Clicked` on the part hit. Actions go from Player to Sim; triggers fire inside Sim, on parts.
- Actions have several sources: devices through bindings, triggers in worlds, timers and animation.

### The four domains
- Each domain is the set of actors that take actions for it. Player routes an action to different actors by its domain.
- An action in the world domain goes to the zone's Sim and needs no routing. It cannot affect the browser, the person
  or another world.
- The list of requests a world may send into the browser and person domains is open (below).

### What is not reactive
Everything that is not reactive is an actor message: snapshots, bulk transfers, lifecycle events and replies. The kinds
of message are in [Messages](../actors/messages.md).

### Device input
Device input is of four kinds (see Design choices).
- **Body tracking** (the avatar's pose and pointing) is given to a world under permission
  ([The person](person.md): the body is private by default).
- **Raw streams** are the microphone. Who owns the device is open (below).

## What exists today

The island is `C:\src\pegasus3d` master (`browser/src/`); the studio is the branch `incubator-archwalk17`
(`guidebrowser/src/`).

- **The island's world speaks in two enums.** `Request` is what a world asks for (Spawn, Place, Show, Fire, Listen,
  Timer, Area, Seat and others, `parts/parts.cone`). `Message` is what a world is told (Clicked, Rang, Dusk, Dawn,
  Entered, Left, HardLanding, Read, `parts/parts.cone`), posted into the door's mail and read each turn by a `match`
  in `bakedworld/bakedworld.cone`. `[differs: two enums, neither an action: no receiver handle, no parameters
  record]`
- **The island's View and Sim talk in `ToSim` and `ToView`** (`simview.cone`): Hello, Click, Command (a `kind`), Look,
  Frame and Play one way; Requests, Possessed, Refused, FrameDone and others back. Held controls travel as numbered
  `InputSample`s on a latest-wins stream, and edges as reliable commands. `[differs: a third and fourth encoding]`
- **The island's bindings** are the `input` package's contexts, actions and bindings (`walk.cone`, `browser.cone`). In
  that package an "action" is a named, bound input, not a reaction. `[differs: the word names a signal]`
- **The studio's world has two triggers,** `created` and `clicked(part)` (`understand.cone`). Areas feed derived values
  and send no event. The loop picks a click through a hand-written priority cascade (`guidebrowser.cone`, `fn event`).
- **The studio's palette** holds action strings of the form `icon|Label|action`, dispatched by `palAct`
  (`incubator.cone`), and "want" slots the loop polls later. `[differs: strings and polled slots]`
- **Five encodings** of "do this" are found across the two programs: the island's `Request`, its `Command` kind, the
  studio's palette strings, its "want" fields, and a numeric `act` field (named in a survey of the code; not re-read).
- **Timers:** the island's are scene-time `Timer` requests on a timewheel (`parts/events.cone`); the studio's are
  wall-clock fields.
- **The manual.** Its actions page uses "action" for a step of an acting block and also for a being's named verb; its
  events page uses "trigger" for the stimulus alone and "handler" for the acting block; its visitors page declares a
  being's verbs with `action(...)` and receives them as triggers; "message" is a named typed value sent between
  parts. `[differs: one structure, and a trigger is a stimulus with its response]` The pages that change with the
  decided design are actions, events, subscriptions, behaviors and visitors.

## Open

### Other decisions to confirm
- **A world that needs a device signal itself** (a hand's pose for a game of its own). Is it then a grant, under
  [The person](person.md)'s body privacy?
- **The pointer on a focused surface.** Typing goes from Player straight to the page ([Strangers' worlds](strangers-worlds.md)).
  Options for the pointer: Player sends it straight to that surface, Sim learning only that focus moved; or it passes
  through the zone's Sim. Recommended: straight to the surface.
- **The click.** Player sends "pressed, along this ray, at the time of the frame the person saw" to the avatar's hand,
  and only Sim knows what lies under the ray and turns it into `Clicked` on the part hit. Whether this is the form the
  message table takes is to be settled with the table in [Messages](../actors/messages.md).
- **The microphone.** Options: Audio owns the device and Player decides who hears it; or Player owns both. Recommended:
  Audio owns the device, because capture runs on the audio clock and echo cancellation needs the speaker's signal;
  Player decides who hears it.

### The actions proposal
The manual's actions proposal pages offer a design for what the decided parts leave open. It is a proposal, not
judged. Its summary, with its "world" read as "zone":
- The record is 64 bytes: a 40-byte head (verb, domain, delivery kind, flags for person, predicted and more, chain
  depth, receiver handle, origin handle, moment, sequence number, cause) and 24 bytes of values.
- The origin is filled in by the trusted side (Player, the server from the connection, Sim for handlers and engine
  reports), never by the sender. The moment of a person's pointing is the world time of the frame they saw; a client's
  claim is accepted only inside a window.
- Delivery kinds: once (reliable, in order), held (a value that stands until replaced, one slot per origin, verb and
  receiver), cue (may be dropped, never repeated for a late arrival). Latest values, bulk and moves are not actions.
- Values come from a closed list with no pointers or code; an action may carry an action.
- A verb is an interned name, with one table per Sim and one fixed table for the three non-world domains; the proposal
  names its domains world, player, browser and surface.
- A receiver is an 8-byte handle (index and generation); a stale handle is dropped with a refusal.
- Triggers have one written form; every stimulus is an action arriving at a part; timers and an action track in
  animation are added.
- A step has six phases: admit, move, sense, react, derive, publish. The react queue is first in, first out, and the
  actions a handler sends join the back.
- Effects are staged: a handler's actions take effect together when it finishes, or are all dropped if it overruns, and
  a handler does not see its own writes.
- Caps, all proposed and unmeasured: 65,535 names and 1 MB of name text per table; 100,000 units of work per handler
  with a 2 ms backstop; 256 actions per handler; chain depth 8; 4 ms of reaction per zone per step; a queue of 16,384
  (1,024 reserved for persons); 64 waiting per part; 256 timers per part and 65,536 per zone; a person's once-actions
  60 a second with bursts of 20.
- Privileges: six kinds of origin (person, mind, rule, engine, interface, editor); checks at the device, the client
  to server boundary and the world to browser boundary; a person flag that a rule cannot set, needed for requests
  with consequences outside the world, found by following the cause back (proposed: within 1 s).
- On the network a person's once-actions are reliable and ordered, held values are newest-wins, state travels as
  snapshots and not as actions, and the client sends only the person's own actions. Prediction covers only the
  built-in responses to the person's held actions. Two logs: a run log for replay, off in normal running, and a join
  log.

### Its ten open questions, with its recommendation
1. Does a handler see its own writes? Recommended: no.
2. Are reactions answered after physics, so an animator a click starts writes its first value a step later?
   Recommended: after physics.
3. Are the browser and the person two domains with one actor behind them? Recommended: two domains, one actor for now.
4. What cap on a name table, and what happens at the cap? Recommended: refuse the load, loudly.
5. May an AI character's actions carry the person flag? Recommended: no.
6. Which standard being verbs carry between worlds? Recommended: move, look, jump, sit, stand, hail, use, press,
   release, grab, drop, speak, adding only when two unrelated worlds invent the same verb.
7. Is the input package's "action" renamed "signal"? Recommended: yes.
8. Is a verb declared with `verb(...)` and answered by its own name? Recommended: as proposed.
9. Does a verb call bound by `let` build or send? Recommended: build when the name is declared as an action, else send.
10. May a client act ahead of the server on more than its own body? Recommended: not at first.

### Its known inconsistencies
- It is written for one Sim per world and has no zones; crossing between the zones of one world is not among its flows.
- Its `created` trigger is sent at spawn in one page and on arrival of the part's code in two.
- Its handles assume authored parts are numbered in a fixed load order, which progressive arrival does not give.
- It changes the sense of "trigger", from the stimulus alone to a stimulus with its response, without saying so.
- Six of its changes to the manual's pages are not on its own list of changes.
- It names the domains world, player, browser and surface; the agreed names are world, page, browser and person.
- Its `cause` field holds a sequence number that may come from a different stream.

### Other open items
- **The short list of requests a world may send into the browser and person domains.** The proposal's verbs page lists
  seven: open a portal to another world; offer a link; offer focus on a page the person clicked; offer a being to
  inhabit; make a verb available for binding; buzz a hand; open, navigate or close a surface belonging to one of its
  own parts. Its privileges page adds asking for something of the person's identity. Recommended: that list, each
  judged by Player, anything else refused.
- **The browser's own interface:** a trusted local world with its own Sim, written in the content language, or plain
  code inside Player. It decides whether the browser and person domains have a world behind them. Recommended: the
  local world, one model of parts and triggers for everything; the cost is that the browser's most trusted prompts
  depend on the content language's runtime being correct. It is deferred with the browser's own prompts and reserved
  inputs ([Strangers' worlds](strangers-worlds.md)).
- **Reserved keys while typing,** deferred with the reserved inputs ([Strangers' worlds](strangers-worlds.md)).
  Recommended: the reserved keys stay live while a typing context is on.
- **How a world's declared actions become bindings.** A world declares actions (a name, a value type, a context such as
  on foot, in a menu or typing, and suggested inputs for each kind of device). Recommended resolution, first match
  wins: the person's choice for this world; the person's choice for the standard action of the same name; the world's
  suggestion if it collides with nothing; the browser's default; else unbound and shown as unbound. A world's
  suggestion is never applied silently over the person's own, and the person sees "this world suggests these controls"
  the first time. A context is a set of bindings switched on and off. The world sees the action, never the device, and
  can still say "press E" by asking which input is bound.

## What lives elsewhere

- [Player](../actors/player.md): the bindings, the reserved inputs and routing by domain, as handshakes.
- [Sim](../actors/sim.md): the order of a step, handlers and their budget, how triggers fire on parts.
- [Messages](../actors/messages.md): the kinds of handshake, and what is an actor message and not an action.
- [Strangers' worlds](strangers-worlds.md): what a world may reach, the browser's own prompts and reserved inputs.
- [The person](person.md): the person's settings and grants that bindings belong to.
- [The content language](content-language.md): where triggers are written.
- The manual's actions, events, subscriptions, behaviors and visitors pages, which this note rules over.
