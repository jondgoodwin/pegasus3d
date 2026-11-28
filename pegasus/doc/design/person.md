# The person

`[planned]`

This note covers who the person is to the browser and to a world: what Player holds for them, how their avatar and its
camera are chosen, and where an AI character stands. It takes the position that Player acts for the human at this
machine, that the avatar is the world's part and the person's choice, and that the person's own data is written only by
the person. How the person is identified, what a world is told at entry, and how safety and comfort are enforced are
open. The island asks for a name at join and the studio has no identity at all, so almost all of this is unbuilt.

## Principles

- **Player acts on behalf of the human at this machine.** It forbids anything that is not the human at this machine
  being a Player.
- **The avatar is a part in the world and its behaviour is always the world's; which avatar, and how its camera is
  set, are the person's.** It forbids an avatar that brings its own rules into a world.

Inherits: [Strangers' worlds](strangers-worlds.md) (what a world may learn of the person).

## The design

### Design choices

- **The person's identity, settings, grants and bindings are written only by the person.** A world may declare, ask
  and read what Player hands it. It forbids a world writing a setting, taking a grant or rebinding a key.
  `[design choice]`
- **The body is private by default.** It forbids a world receiving hands, eyes, face, voice or the room's shape
  without a grant the person gave that world. `[design choice]`
- **Taking over another being, spectating and riding are each a change of what the person inhabits; Player tells
  Render, and Audio for 3-D sound, which avatar is inhabited and in which way.** It forbids a separate mechanism for
  each. `[design choice]`

### The person and Player
The person is the human. Player is the actor that acts for the person at this machine. It establishes who they are (a
signed-in account, or a guest) and holds what is attached to them: their resources and settings, browser-wide and for
each world they enter, such as which avatar they use there and how its camera is set. It forwards what they do on their
devices as actions to the zone's Sim, and to the other actors where an action is meant for them (a page they are typing
into, the browser's own controls). What a world learns of the person is what Player hands over
([Strangers' worlds](strangers-worlds.md)). Its handshakes are in [Player](../actors/player.md).

The manual's authors meet the person as a visitor.

### The avatar
- An avatar is a part in the world like any other, visible or not. It belongs to the zone's Sim, and its camera is a
  part of it (below).
- A world's own avatar is defined wholly in the world. A person's own avatar, from their store, lends its look if the
  world allows it. The avatar's behaviour is always the world's.
- A person's own avatar is an imported part, like a portal. Its definition (mesh, procgen, rendering) lives in the
  person's store; a world that accepts it imports it and defines its behaviour: physics, size, orientation and
  movement rules. Animation is the hard part (see Open). How parts are imported is in
  [Parts and imports](parts-and-imports.md).
- Avatars are published in the registry of parts. The author keeps sole authorship, and the renderable form is
  available to everyone, so other people's browsers fetch it like any imported part.
- A person may keep several avatars and chooses one per world. The choice is a per-world setting, and it belongs to
  Player ([Player](../actors/player.md)). Player shares the choice with the Sim.

### The camera and inhabiting
- The camera is a part of the avatar: the place to look out of, the eyes (two, for stereo). A chase camera is an option
  on the part. Player holds the camera's settings ([Player](../actors/player.md)).

### AI characters
An AI is not a Player: Player acts for the human at this machine. Where an AI character lives is open (below).

## What exists today

- **Identity.** The island's identity is a name string (`browser/src/browser.cone`, option `--name`, default the
  machine's name). The server checks its length, characters and uniqueness at join (`browser/src/simactor.cone`,
  `nameProblem`) and keeps no account, login or signature (`browser/SERVER.md`). The studio has no identity and no
  avatar (`guidebrowser/src/guidebrowser.cone`).
- **Avatar.** The island's avatar is a `Being` in the `Cast` (`browser/src/being.cone`) wearing a person built from the
  humanbody package (`personasset.cone`, `personview.cone`), with a wooden mannequin as fallback (`mannequin.cone`).
  `[differs: a person cannot bring their own avatar from a registry]` The studio's visitor is a free-flying eye with a
  position and a yaw (`guidebrowser.cone`, `visitor: Fly`).
- **Camera.** The island's camera is the View's `ViewRig` (`browser/src/being.cone`), first or third person, following
  the View's predicted body. In the headset the head pose is placed on the avatar's feet and the person's own avatar is
  not drawn. `[differs: the camera is a part of the avatar]`
- **Inhabiting.** `Cast.possess` and `swap` move the person to another being; Tab does it in single player only, and a
  connected client's being is fixed at join (`browser/src/simactor.cone`, `browser/src/being.cone`). `[differs:
  takeover, spectating and riding are one change of what is inhabited]`
- **Settings.** The island reads command-line options and keeps caches. The studio reads two files, `incubator.toml`
  (the person's preferences, palette sets) and `room.toml` (where things are), by polling and whole-text compare
  (`guidebrowser/src/incubatorcfg.cone`, `incubator.cone`). `[differs: no per-world settings, no grants]`
- **AI.** The island's villagers and creatures are code inside Sim (`browser/src/npcs.cone`, `creatures*.cone`). The
  studio's companion is a Claude Code session reached through files in a folder (`guidebrowser/src/bridge.cone`), with
  no avatar.
- **The manual.** Its visitors page says a visitor drives a being and content never names a player. Its outside page
  says content sees nothing of identity `[differs: a world learns what Player hands over]`. Its frames and cameras
  pages say the viewer is the camera, not the player `[differs: the camera is a part of the avatar]`. Its avatars page
  keeps the person's profile as a portable description the world applies to its own avatar `[differs: the person's
  avatar is an imported part whose look a world may accept]`. The manual's `person` type is a recipe for a body, not
  the human.

## Open

### Designs not yet settled

- **Where an AI character lives.** What gives it an identity, what it may perceive, where it runs, and how its actions
  reach Sim. Options: it is a being whose mind is a service the world's server runs; it is a person's companion acting
  through Player. Recommended: a world's character belongs to the world, a being whose mind is a service the world's
  server runs, sending the same actions through the same door and perceiving what the world gives it; a person's
  companion acts under the person's grants, through Player.
- **Whether the person's settings are wholly theirs,** with policy from a guardian or organisation above them.
- **Layers of settings.** Recommended, lowest to highest: the browser's defaults; the person's own everywhere
  (language, comfort, dominant hand, accessibility, privacy defaults); the person's own for one world (grants, binding
  changes, comfort changes, the avatar chosen there); this session only; and policy from a guardian or an organisation,
  shown locked and by whom. Device settings (play area, lens spacing, controller calibration) are kept apart because
  they do not travel to another machine. Synced settings are encrypted so the service cannot read them, with the cost
  of a lost passphrase decided in advance.
- **Grants for the body.** Which kinds of body data, and how long a grant lasts. Recommended: per world and per kind
  (microphone, hands, eyes, face, the room's shape), asked for when used and after a real gesture, revocable, expiring
  if unused. A world is never given where the person looks: it is told that they pressed at a target. What of the
  person's body moves their avatar is a grant with a plain warning that others will see it.
- **The contract between a person's avatar and a world's animation.** Options: a standard humanoid skeleton the world
  animates and the avatar's mesh is bound to; the avatar brings its own animations; the world supplies a fallback.
  Recommended: a standard skeleton, with a fallback for what is not humanoid; a performance budget on avatars that
  arrive at run time by the person's choice; each viewer's Player may show a plain stand-in for any avatar it does not
  want drawn.
- **A world's data about the person** (inventory, progress, rights in that world). Recommended: the world's server
  keeps it, under the world's own record of the person. Persistence in general is open in
  [Worlds and zones](worlds-and-zones.md).
- **How bindings are resolved.** In [Actions and triggers](actions-and-triggers.md).

### Deferred, with safety and identity

Not decided; taken up with safety and identity.

- **Each world sees its own identity for the person.** It would forbid two worlds joining their records to find the
  same person. Question: is a stable id across worlds ever given, and on whose consent?
- **Safety and comfort follow the person into every world and are enforced on their side.** It would forbid a world
  ignoring a block, a mute or a comfort setting, or learning who is blocked. Question: is enforcement by Player, as a
  filter, the right place? Recommended: blocks, mutes and the personal-space bubble are applied by Player as a filter,
  so a hostile world cannot ignore them and never learns who is blocked; a world may add rules above that floor, never
  below it. Comfort (snap or smooth turning, vignette, seated mode, height) is reshaped by Player, and worlds declare
  how intense they are. Anything that can take control of the person is visible and ends when they leave.
- **The identity model.** What a world is told of the person. Recommended: the browser's makers run one identity
  service that holds the person's account, what they own and what they have paid for; each world is given its own id
  for the person by default; at entry Player hands the world a signed statement, and a world is told what the person
  owns by a claim in it; a world may ask Player for a grant; a guest is a first-class way to enter, needing no account.
- **What the signed statement holds,** and whether a stable id across worlds is ever given. Options: this world's id
  for the person, the display name and avatar chosen for this world, and separate claims (guest, age band, what the
  person owns); or more. Recommended: those fields only, checked by the world's server against the service's
  signature; a stable id across worlds only on the person's consent, per world.
- **The identity service.** Which ways in it accepts (passkey, email link, another company's login), whether a second
  factor is asked and when, what recovery is, and how a guest becomes an account. Recommended: other companies' logins
  are only ways in, never the identity; a passkey is the easy path and an authenticator code the fallback; a second
  factor is asked before account changes, not before entering a world; recovery is designed with the care of sign-in;
  a guest has a local identity, a random id per world, nothing synced, and a way to become an account that keeps chosen
  per-world state. Where the service runs and what it holds is in [Server](../actors/server.md).

## What lives elsewhere

- [Player](../actors/player.md): the person's session, devices, bindings and the browser's own interface, handshake by
  handshake.
- [Strangers' worlds](strangers-worlds.md): what a world may learn, the reserved inputs, the browser's own prompts.
- [Parts and imports](parts-and-imports.md): avatars as imported parts and the registry of parts.
- [Worlds and zones](worlds-and-zones.md): the one sequence that enters a world, and what travels between zones.
- [Actions and triggers](actions-and-triggers.md): bindings as triggers, and the person's actions.
- [Render](../actors/render.md): the camera and the eye. [Audio](../actors/audio.md): the listener and the microphone.
- The manual's visitors, avatars and outside pages, which this note rules over, and its frames and cameras pages.
