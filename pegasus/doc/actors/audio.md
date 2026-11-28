# Audio

`[planned]`

Audio puts sound out on time. It owns the speakers and the mixer, and it never blocks waiting for Sim, the Loader or a world's code. Most of its design (where its sounds come from, its clock and thread, the microphone) is proposed, and is in Open. The island engine mixes on a thread of its own with a lock-free queue, but the mixer is driven from the view actor and there is no microphone; the studio has no audio of its own.

## The design

### Role

Audio mixes and plays the sound the person hears, on time. A missed buffer is a click, heard at once. Audio never blocks waiting for Sim, the Loader or a world's code ([The frame and its deadlines](../design/frame-and-deadlines.md)).

### Who it works with

The kinds of handshake are those of [Messages](messages.md); the proposed kinds are in the proposed table of messages there.

- **Player** ([Player](player.md)).
  - In: which avatar the person inhabits, so Audio places the listener for 3-D sound ([The person](../design/person.md)).
- **Sim** ([Sim](sim.md)).
  - In: the newest snapshot Sim publishes after each step, which never changes after publishing; emitters are parts in it ([Sim](sim.md)). Audio runs on its own buffer clock and never holds a buffer back for Sim.
- **Loader** ([Loader](loader.md)).
  - A sound the Loader builds is built once and moved to Audio, its one user ([Arrival](../design/arrival.md)). What else comes from the Loader is Open.
- **The speakers.** In: the device's buffer clock. Out: the mixed buffer.

### What it owns

- The mixer.
- The speakers.

Who owns the microphone as a device is Open.

### How it spreads its work

Not settled (see Open).

## What exists today

- **Island, `browser/src/audioview.cone`.** Audio is not an actor: `AudioView` lives in the view actor's state, and `audioFrame` runs once per frame from `publishSnapshot`. `[differs: the design has Audio as its own actor]`.
- **Island, sources of sound.** Sound is derived from the view's own state each frame, and the one world-driven sound is the balloon's burner, recognised by the part's name. The listener is the view's eyes, or the headset's head in the headset.
- **Island, the mixer.** Commands go into a lock-free single-producer ring consumed by the feeder thread of the device package. Automated runs mix in-thread to a null device, with a fixed 48000/60 samples per step so runs repeat. A server has no audio. The sounds are recipes, not files.
- **Cone packages `audiomixer` and `audiodevice`.** Voices through three buses (music, effects, ambience) and a master; priority voice stealing; spatial sound with distance attenuation, a pan law, a head-shadow low-pass filter on the far ear (no interaural delay, no head-related transfer function), Doppler shift, and occlusion as a number from 0 to 1 that the caller sets. Synthesised recipes for wind, brook, fire, footsteps on grass, wood and stone, and a bird. WAV decode. A test counts heap allocations across 600 rendered blocks. A bench measures a 10 ms block of 32 voices at about 0.2 ms with sample voices and about 0.9 ms with procedural voices (release build). The device keeps 40 ms of sound queued plus the device's own buffer.
- **Studio.** No audio code. A separate voice program owns the microphone and speakers and speaks the companion's replies.

## Open

These are proposed settlements, not decided design.

- **What Audio plays, and its deadline.** Proposed: the world's sounds placed in 3-D around the listener, the browser's own sounds, and the voices of other people; its deadline is every audio buffer, a few milliseconds. Recommended: this.
- **Where Audio runs.** Proposed: on the person's machine only; a server has no Audio. Recommended: yes.
- **How Audio spreads its work.** Proposed: a thread of its own at high priority, small and always ready; the mixer's render never allocates and never locks, and shares a single-producer queue of plain-data commands with the rest of the program, where a full queue refuses a command and counts it; mixing a few dozen sounds is light work. Recommended: this, as the packages are built.
- **What the mixer holds.** Proposed: voices, buses, fades, the choice of which voice to drop when there are too many, and the listener's state. Recommended: this.
- **Whether Audio reads the published head pose.** Proposed: Audio reads the head pose as drawn as the listener's pose, the latest value Render publishes, which Render's note names for Player and Sim only ([Render](render.md)). Recommended: yes.
- **Sound from the Loader.** Proposed: the Loader sends decoded sound as bulk; sounds that are recipes, not files (wind, a brook, footsteps), are made by Audio's own synthesis. Recommended: this.
- **Who owns the microphone.** Proposed: Audio owns the device and Player decides who hears it. Reason: capture runs on the audio buffer's clock, and echo cancellation needs the signal being played; every platform separates the device from the permission. Recommended: this. The stream goes to Player (bulk, on Audio's clock), which passes it only to those allowed.
- **How a world's sounds reach Audio.** Options: a handler's sound goes as an event naming a part and a sound, with footsteps and weather derived by Audio from the snapshot; or every sound is part state in the snapshot and Audio derives all of it. Recommended: events for what a handler asks for, derivation from the snapshot for ambient sound.
- **Occlusion.** Sound blocked by geometry needs a number the mixer takes, and the geometry is Sim's collision shapes. Options: Sim sets occlusion per emitter in the snapshot; or Audio reads the immutable shapes. No source recommends.
- **People talking to each other.** Voice and text chat between people in a zone: who carries the stream, which codec, how the listener hears a speaker placed at their avatar, who may mute whom. Player's side (who hears the microphone, whom to silence) is drafted; the transport, which belongs with [Network](network.md), and the world's side are not.
- **Where an AI character's speech originates and how it is heard.** Depends on where an AI character lives, which is open.
- **Latency target.** The built feeder holds 40 ms queued; a deadline of a few milliseconds per buffer needs a smaller queue or a callback on the device's own thread. The target is unmeasured, and the callback depends on Cone letting a foreign thread call into it.
- **The browser tier.** The package design uses Web Audio in a browser, but nothing is compiled for it. Recommended: keep the mixer's core as portable arithmetic and do not let the tier shape the actor.
- **Not yet built:** streamed music, Ogg Vorbis and Opus decoding (Opus is the likely voice codec), a measured head-related transfer function, reverb zones.

### Deferred, with safety and identity

- **Muting.** Whether Player tells Audio whom to silence, or the snapshot omits that person's emitter. Recommended: Player tells Audio, since a mute should follow the person into every world. This rests on safety following the person into every world, which is deferred.

## What lives elsewhere

- [The frame and its deadlines](../design/frame-and-deadlines.md): why Audio never blocks on the other actors.
- [Isolation and ownership](../design/isolation.md): how a structure read by two actors is shared.
- [The person](../design/person.md): who may hear the microphone, muting, the body's privacy.
- [Sim](sim.md), [Render](render.md), [Player](player.md), [Loader](loader.md), [Network](network.md), [Messages](messages.md).
