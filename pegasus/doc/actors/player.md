# Player

`[planned]`

Player is the actor that acts for the human at this machine: it establishes who they are, holds what is attached to them, and turns what they do on their devices into actions for the actor each is meant for. This note takes the position that Player is one actor and the browser's privacy gate: a world learns only what Player hands over ([Strangers' worlds](../design/strangers-worlds.md)). Much of who Player exchanges what with, and in what kind, is still open. The rules about identity, settings, safety and avatars are in [The person](../design/person.md); this note says what Player does and owns. The island has an action layer in its shell and a name for a person; the studio has a hand-written input cascade and no identity.

## The design

### Design choices

- **Player holds the person's choice of avatar for each world, and the camera's settings.** It forbids a world or an avatar choosing for the person. `[design choice]`

### Role

Player acts on behalf of the human at this machine. It establishes who they are (a signed-in account, or a guest) and holds what is attached to them: their resources and settings, browser-wide and for each world they enter, such as which avatar they use there and how its camera is set. It forwards what they do on their input devices as actions to the simulation, and to the other actors where an action is meant for them (a page they are typing into, the browser's own controls).

The avatar is a part in the world, and Sim's; its camera is a part of the avatar. A character run by an AI is not a Player (where such a character lives is in [The person](../design/person.md)).

### Who it works with

The kinds of handshake are those of [Messages](messages.md). The kinds of the rows the first browser is built to are decided there; the kinds of the rest are in the proposed table of messages there.

**The devices** (keyboard, mouse, controllers, hands, headset).
- In: what the person does on them, of four kinds ([Actions and triggers](../design/actions-and-triggers.md)). The browser's own controls stay live while text goes to a focused field ([Actions and triggers](../design/actions-and-triggers.md)); which inputs are reserved from worlds is deferred (see Open). The owner of raw streams (the microphone) is open (see Open).
- Bindings are triggers ([Actions and triggers](../design/actions-and-triggers.md)); a world sees the actions, never the devices.

**The client Sim of the zone the person is in.**
- Out: the person's actions; Player and Sim talk in actions, aimed by domain ([Actions and triggers](../design/actions-and-triggers.md)).
- Out: the avatar choice, which Player shares with Sim.
- What else passes between them is proposed (see Open).

**Render.**
- Out: which avatar the person inhabits, and how its camera is set ([The person](../design/person.md)).
- The head pose is read last by Render and never travels as a message ([The frame and its deadlines](../design/frame-and-deadlines.md)). In the headset Render reads the runtime at the predicted display time; on the desktop it reads Player's mouse look, a latest value Player publishes. In: the frame's display time and the head and hands as drawn, a latest value, so a ray means what the person saw. Player owns the controllers and the actions ([Render](render.md)).

**Audio.**
- Out: which avatar the person inhabits, so Audio places the listener for 3-D sound ([The person](../design/person.md)).
- Anything to do with the microphone is open (see Open).

**The focused page or document (a Surface).**
- Out: text, straight to the focused field ([Strangers' worlds](../design/strangers-worlds.md)). How the pointer reaches the page is open (see Open).

**The identity service (on a server).** Player establishes who the person is: a signed-in account, or a guest. What passes between Player and the identity service is deferred (see Open).

**Entering a world** goes through Player, and runs one sequence ([Worlds and zones](../design/worlds-and-zones.md)). The sequence starts the world's Sim (Player starts it for now and asks the Loader to open the world for it; the Loader moves the world image and Sim units to that Sim, see [Sim](sim.md)), hands it what Player gives that world about the person, chooses the avatar, and, for a portal, has the old world's Sim release the avatar. What is checked first, under the rule about origins, is deferred (see Open).

**A portal.** A portal is an imported part: a world can import parts published elsewhere, and a portal is one, made available by some world, placed in a host world and managed as an asset apart from the world it leads to. Its look is its own capability, not necessarily a live window. On proximity, a button or another trigger it gets ready; on entry it sends an action telling the browser "we are entering a new world", which goes to Player. Until decided otherwise, a portal prepares its far world only on the person's explicit ask, because preparing fetches from the far world's server, which would otherwise learn the person's address before they chose to go.

### What it owns

- The session: signed in or guest. What else the session holds is proposed (see Open).
- The person's identity, settings, grants and bindings, browser-wide and for each world they enter, written only by the person ([The person](../design/person.md)).
- The devices.
- The choice of avatar for each world and the camera's settings (see Design choices).

Not Player's: the avatar and its camera as parts, which are Sim's. The avatar's behaviour is always the world's. A person's own avatar from their store lends its look, if the world allows it.

### How it spreads its work

Player is one actor: its work is a sequence of handoffs down one stack (who they are, entering worlds, acting within one), so one coordinator fits. Subcomponents can come later. How it spreads its work over cores is not settled, and its cost is unmeasured (see Open).

## What exists today

- **Pegasus, the headset steers the walk, `playeractor.cone` (W129, 28 Nov).** In `--xr` Player puts the head yaw Render drew last frame into the LookIntent every poll, so the person walks and faces where the head turns; on the desktop the mouse look is used as before. `steerYaw(xr, mouseYaw, headYaw)` is the choice; `--checks` has `check steer yaw` and mutant `walk-mouse-yaw` is caught. Not seen in a headset.
- **Pegasus skeleton, `pegasus/src/playeractor.cone` (W110, 28 Nov).** A `Player` actor: the shell (main thread, which owns the SDL window) sends it an `InputSample` each poll; it writes `MoveIntent` and `LookIntent` latest values (the look is the desktop's mouse-look slot Render reads) and turns a left click into an `Action` aimed along the `DrawnFrame`. It starts the zone's Sim in one function, `startZoneSim`, then sends the Loader `openWorld`. Headless (`--steps N`) it runs a script instead of input. Interactive input built but not yet tried by hand.
- The island's shell (`browser\src\browser.cone`, in `C:\src\pegasus3d`) reads window events on the main thread, turns them into actions with the Cone `input` package (bindings in `browser\src\walk.cone`), and builds the frame's input. Held state travels as numbered input samples and edges as reliable commands (`browser\src\simview.cone`). The keep: signals resolved through bindings into actions. `[differs: this is the shell, not an actor, and there are no contexts a world declares.]`
- The island's headset hands come from `browser\src\xrhands.cone`; Render reads the head at the predicted display time inside `drawXr` (`browser\src\renderactor.cone`) and returns the hands in the draw's reply, one frame late. The keep: Render reads the head. `[differs: hands travel through the shell into the next frame's input.]`
- The island's identity is a name string (`--name`) and a play token that authorises a firewall rule (`browser\server\request\`, `browser\src\requestplay.cone`); the token is not an identity the simulation sees. `[differs: no account, no guest, no per-world id.]`
- The studio (`guidebrowser\src`, branch `incubator-archwalk17` of `C:\src\pegasus3d-wt-incarch17`) tests input in a hand-written cascade in `fn event` (`guidebrowser.cone`): web focus, edit focus, palette, keyboard, desktop, chips, pages, then parts. The keep: the browser's own interface and the focused page win before the world. `[differs: a cascade of tests, not bindings; the palette acts by direct state changes; the file xr.cone duplicates the island's stick and trigger constants.]`
- The studio's visitor is a free-flying eye with no avatar, name or identity.

## Open

- **The form of the browser's own interface.** It is the palette, keyboard, HUD, error card and the prompts no world may draw. Options: a trusted local world with its own Sim, written in the content language, drawn above the worlds and tested first for clicks; or plain code inside Player. Recommended: the local world, so one model of parts and triggers serves everything. The cost is that the browser's most trusted prompts then depend on the content language's runtime being correct. Rows of [Messages](messages.md) that depend on this are marked.
- **Preparing and entering** are settled as two requests, with the crossing's order and when a portal prepares, in [Worlds and zones](../design/worlds-and-zones.md), crossing a portal. Still proposed: Player's own interface shows the destination a portal names before crossing, as a browser shows a link's address, because a host world could dress any portal to look like another; and an imported part's code runs in its host with the host's authority, except the portal's requests, which Player judges. Recommended: yes.
- **Navigation.** What an address names (a world, a zone, an arrival), history, back and home. Options: an address names a world and the world says where a person arrives. Recommended: none yet.
- **Who fetches the person's avatar file into a world, and how other people's browsers get it in a connected world.** Options: from the person's own store; through the world's server. This touches the Loader and the origin rule.

- **The microphone.** Options: Audio owns the device and Player decides who hears it; Player owns both. Recommended: Audio owns the device, because capture runs on the audio clock and echo cancellation needs the speaker's signal.
- **The order in which a signal is resolved.** Options: reserved inputs, then the browser's controls, then the focused page, then the world's actions in the contexts switched on; or another order. Recommended: that order.
- **Player's clock and cost.** Proposed: Player works on the frame's clock, with a light job each frame (reading signals, resolving them through bindings, forwarding). Recommended: this, to be measured.
- **The proposed handshakes with the devices.** In: signals (a key down, a stick's position, a hand's pose, a button), read each frame. Out: vibration. Recommended: this.
- **The proposed handshakes with the client Sim.** Out: movement and look intents, sampled with a sequence number, as state; the person's actions as events (press along a ray at the time of the frame drawn, a button or gesture action, the world's own actions bound to keys); grants the person has given this world (microphone, hands, eyes, face, the room's shape) and withdrawals. In: admitted or refused; a being offered to inhabit; requests for a grant, which Player answers in its own interface; the world's declared actions (names, value types, contexts, suggested inputs); a page was clicked and offers focus; which contexts are on (on foot, a menu, typing). Recommended: this.
- **The proposed handshakes with Render.** In, as a latest value: when each frame was drawn and where the head and hands were, so a point means what the person saw. Out: the inhabited avatar as an event, with a mode (eyes, chase, spectate); the browser's own interface, drawn over the worlds (its form is the first question above). Recommended: this.
- **The proposed handshakes with Audio.** Out: who may hear the microphone. In: the microphone as a stream, passed on only to those Player allows. These rest on who owns the microphone (above).
- **The pointer on a focused page.** Typing goes straight to the focused page ([Strangers' worlds](../design/strangers-worlds.md)). Proposed: the pointer goes straight to the focused surface too and through no world, so a world adds no step of delay and cannot read it; focus is Player's. Recommended: yes.
- **The browser's own controls.** Proposed: back, home, leave, open a world, mute, switch camera, settings, the safety menu and panic mode are handled inside Player. Recommended: yes.
- **What else the session and Player hold.** Proposed: the session holds the worlds open, which world the person is in, and focus; the person's settings are kept in layers; Player holds the worlds' declared actions, how each binding was resolved, the reserved inputs, the browser's own interface (the palette, keyboard, HUD), and device settings kept apart because they do not travel to another machine. A world's data about the person (inventory, progress, rights in that world) is not Player's: the world's server keeps it. Recommended: this.

### Deferred, with safety and identity

- **Identity.** Options: one browser key every world sees; a server token bound to a name; the browser's own identity service with a separate id per world, a signed statement to the world, and guests first-class. Recommended: the identity service, which lets a world be told only what Player hands over and still lets ownership and payment sit on the account. On it rest: sign-in by whichever way the person uses, which the browser draws and no world can; the account, synced settings and a signed statement for each world coming back; the signed statement handed to the client Sim of a world being joined, which the world's server checks itself; a world's data about the person kept under that world's own id for the person.
- **The rule about origins at entering.** Proposed: entering a world first checks the origin rule (the origin decides what an asset may do).
- **The browser's own prompts and reserved inputs.** Proposed: the browser draws its own prompts (sign-in, consent, permissions, the controls review), which no world may draw, and keeps its reserved inputs (the exit chord, the menu button, the summoning gesture). Question: which inputs are reserved?
- **Safety and comfort.** Proposed: blocks, mutes and the personal-space bubble follow the person into every world and are never shown to one; Player sends Render comfort effects to draw (a vignette), whom to hide and the bubble, and sends Audio whom to silence. Comfort reshapes some actions on the way (a smooth turn becomes snap turns).

## What lives elsewhere

- [The person](../design/person.md): identity, settings, grants, safety, comfort, avatars and where an AI character lives.
- [Strangers' worlds](../design/strangers-worlds.md): what a world may learn and the prompts the browser keeps.
- [Actions and triggers](../design/actions-and-triggers.md): the action, the trigger, the four domains and bindings.
- [Parts and imports](../design/parts-and-imports.md): portals and avatars as imported parts.
- [Worlds and zones](../design/worlds-and-zones.md): entering, zones, the world's owner.
- [Messages](messages.md): every message to and from Player, by kind.
- [Sim](sim.md), [Render](render.md), [Audio](audio.md), [Surfaces](surfaces.md): the partners.
- [Server](server.md): the identity service.
- The manual's visitors, avatars, outside and overlays pages.
