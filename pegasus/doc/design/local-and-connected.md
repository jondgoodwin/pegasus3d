# Local and connected

`[planned]`

This note follows one concern across every actor: how a zone plays on one machine alone and on many machines together. It takes the position that the server owns the truth of a shared world, that a client Sim predicts what the person controls, that a world with no server runs whole on the person's machine, and that a rider is held in the frame of what it stands on. Much of the detail (interpolation delay, where a world's rules run, how a takeover runs) is open. Today the island plays alone by turn-taking and connected by overlapping, in two modes; the studio has no network play.

## Principles

**The server owns the truth of a shared world; a world with no server runs whole on the person's machine.** It forbids a client's word deciding a shared outcome, and a world that needs a server in order to run alone.

**A body's position is relative to the moving thing it stands on, and the eyes are placed at the last moment.** It forbids correcting a rider because the platform moved, and solving a moving platform by knowing its path.

Inherits: [the index](_index.md), [Worlds and zones](worlds-and-zones.md) (a Sim simulates one zone).

## The design

### Design choices

- **Local and connected play are one model, as a goal.** They differ in how messages travel and in who holds the truth, not in the code that plays the world. It forbids a second code path written for single player. `[design choice]`
- **A person's machine predicts only what that person controls.** It forbids predicting other beings, or the world's code, forward on a client. `[design choice]`

### The roles of Sim

- Sim is one program in three roles: whole, server and client ([Sim](../actors/sim.md)). The client Sim is lightweight: it predicts what this person controls (their avatar's body, a ship they steer), keeps the body history to reconcile with the server, and carries the state and event conversation with the server Sim. Local play is one Sim in the whole role, not a server and a client joined in-process.
- The conversation has two kinds of traffic: state, where only the newest matters, and events, which arrive once and in order. What each side sends, the snapshot the client Sim publishes from its merge, and the rule that a shared outcome waits for the server's word are in [Sim](../actors/sim.md), connected play. The wire protocol is in [Network](../actors/network.md).
- State traffic and asset traffic never block each other ([Network](../actors/network.md)).

### Who is right

- The server Sim holds the truth of a shared world and does its calculations; it may refuse a client's request.
- A world with no server needs none: it runs whole on the person's machine. Someone building their own space on their own drive is the case; the studio is an example.

### Prediction and reconciliation

- **Prediction:** the client runs ahead for what the person controls.
- **Reconciliation:** when the server's state for the person's body differs from what was predicted, the client Sim corrects it using the body history it keeps.

### Riding moving things

A body is held in the frame of what it stands on. Walking is predicted and compared in that frame, so a platform's motion never makes a correction for its rider. The eye is composed last: the platform's pose, then the body in the platform, then the avatar's camera part, then the newest head pose.

A world need not tell the browser how a platform will move: a platform may be steered by someone, and the design does not depend on knowing its path.

A world declares which of its crossings are routine and which are risky. The expected outcome of docking in order to disembark is a driver; where a leap may fail, ending in the water is less unexpected. A crossing is scored against what perfect timing would have given.

**The measured result,** from a standalone model of a ship and its riders, not the browser; the deck-frame strategies were not built in the browser:
- Held in the deck's frame, a rider walking needed no corrections; the baseline needed about 1,610 a minute.
- Near an exit, showing the ship predicted to the present did better than interpolating the frame being ridden (5.5% against 13.5%); on the deck the choice made no difference.
- Risky leaps: the client disagreed with the server in 12.4% of them with the baseline and 4.7% with the model's recommended choices (deck frame, the ship predicted near an exit, the server accepting the drawn pose within 1 m, declared routine crossings).
- Routine crossings from a docked ship failed in 0 of 320 for every strategy tried. Prediction failed in 21% of cases on a rocking ship unless the ship was declared docked.
- Rewinding to what the person saw agreed with the person and differed from perfect timing in 13 to 20% of risky leaps, and made observers see a pop. Near-misses were not fair: nearly all went against the person.

### Who steers, who stands where

Taking over another being, spectating and riding are each a change of what the person inhabits ([The person](person.md)). A change of who steers or who stands where is a first-class event: boarding, leaving a seat, docking and changing pilot each arrive once and in order, and are never side effects of position.

## What exists today

- **Two modes in the island** (`C:\src\pegasus3d`, master). Single player joins the client side and Sim over an in-process connection (`pairInProcess`, `browser\src\simactor.cone`) with the network's message kinds, plus a turn-taking pair (`Frame`, `FrameDone`): the frame passes from the shell to the view, Sim, the world, Sim, the view and Render in turn (`browser\src\browser.cone` loop, `viewactor.cone`). Connected, the view awaits no one (`viewactor.cone`, `frame`). `[differs: one model of play]`
- **Prediction and interpolation are the view's** (`browser\src\viewactor.cone`): it predicts the person's body with the server's own `Being.step` code, reconciles on the echoed input number and blends the difference over 0.1 s (`CORRECTION_SECONDS`), draws others 6 steps (100 ms, `DELAY_TICKS`) in the past, and builds the snapshot Render reads. In single player it also steps every other being itself (`predictOthers`). `[differs: the client Sim predicts only what the person controls]`
- **The server** is the simulation actor in `--serve` mode (`browser\src\server.cone`, `simactor.cone`): it holds all bodies, platforms, the world's code and the clock. Clients send their last ten input samples with sequence numbers, latest wins; the server publishes a snapshot every other step (30 a second), split into a shared part encoded once and a personal part per client (`RegionState`, `PersonalState` in `simview.cone`); events are reliable; a replay log catches a late joiner up (`replaylog.cone`). The world's code runs only where Sim runs, which is the server when connected. Tests run prediction through a lossy in-process link (`--impair`, netmsg's `LossyRelay`).
- **Platforms:** the balloon's pose is computed by the world from scene time and applied to both Sim's and the view's placement trees, each carrying its riders (`browser\src\support.cone`, `parts\placement.cone`); the server tells clients when a closed-form motion starts and the client evaluates the path on the shared clock. There is no steered platform in the island. `[differs: a rider's world position is carried by the platform's moves; the design holds the body in the platform's frame]`
- **Takeover:** Tab works only in single player; Sim moves possession to the next non-AI being and sends `Possessed` (`simactor.cone`, `being.cone`). Connected, each client's being is fixed at joining.
- **The studio** (`C:\src\pegasus3d-wt-incarch17\guidebrowser\src`, branch `incubator-archwalk17`) has no network play and no body: the visitor is a free-flying eye (`guidebrowser.cone`). It runs whole on the person's machine from a world file or an address, which is the local case. Its language accepts only fields that live on the client (`understand.cone`).

## Open

### Designs not yet settled

- **Where a world's rules run.** The manual's distribution page says a world is one program and each of its rules says where it lives: on the server, on the client or on both; the client runs what it needs to predict what the person controls; a world that wants a secret keeps it in a rule that runs only on the server, so the secret is never in the description anyone can download. Recommended: as the page says.
- **The takeover sequence when connected.** Proposed: Player asks the client Sim to inhabit the being, the server Sim checks the person's privilege and replies, Player tells Render and Audio, and the client Sim starts predicting the new body from Sim's state.
- **Crossing between frames.** Recommended, from the ship model: show the ship predicted to the present near an exit, and have the server accept the pose the person drew within 1 m.
- **Declared motion.** Whether a world may declare a motion as data (a balloon's path) as an optional smoothing. Recommended: yes, as an option only.
- **Interpolation delay and snapshot rate.** How far in the past others are drawn, and how often the server publishes, are unmeasured for the new browser; the island's constants are 100 ms and 30 a second. The delay is also the lever left on near-miss fairness: how old the person's view is.
- **Still hard for platforms.** Moving between frames; a pilot's correction, which moves everyone aboard (a decaying offset on the frame); firing from a deck; a ship's roll and comfort in a headset; whether a tilted deck works with the platform's collision shape. None is designed.
- **Failure.** What the person sees, and what a world is told, when the server goes away or a zone's Sim stops.

## What lives elsewhere

- [Sim](../actors/sim.md): the step, the client and server versions, the snapshot, handlers and their budget.
- [Network](../actors/network.md): what crosses the wire, state and event streams, the library both the client Sim and the Loader use.
- [Server](../actors/server.md): the program that runs a shared world.
- [Worlds and zones](worlds-and-zones.md): zones, instances, entering a world.
- [The frame and its deadlines](frame-and-deadlines.md): Render's deadline, the test mode, the head pose.
- [The person](person.md): inhabiting, the camera, the avatar.
- [The content language](content-language.md): where a rule lives, as an author writes it.
- The manual's distribution page: `lives`, what crosses the network, where logic runs.
