# The frame and its deadlines

`[planned]`

This note covers the picture and the sound on time: who may wait for whom and for how long, where the head pose comes from, and what a frame costs. The position is that nothing delays the picture: Render and Audio never block on the simulation, the Loader or a world's code. Which clock each actor keeps is mostly open. Today the island runs its actors in turn when alone and overlapped when connected, and the studio does nearly everything on one thread.

## Principles

- **Nothing delays the picture: not a world's code, the network, loading or memory cleanup.** It forbids any of those on the path of a frame.
- **Head movement always gets the newest pose.** It forbids a head pose that waits on another actor.

## The design

### Design choices

- **Render and Audio never block waiting for Sim, the Loader or a world's code.** It forbids a frame or a sound buffer that is late because another actor has not finished. `[design choice]`
- **Arrival is governed: loading a whole world never moves the tail of frame times.** It forbids unbudgeted uploads, and loading at a priority that can starve a frame. `[design choice]`
- **The Loader runs at low priority, not none, and cores are reserved if the operating system's priorities are too loose; Render accepts new content under a per-frame upload budget.** The test is that loading a whole world never moves the tail of frame times. `[design choice]`
- **The head pose is read last by Render and never travels as a message.** It forbids a pose delivered as a message, or read before the frame's last moment. `[design choice]`

### What may stall

- A world that hangs stalls only itself ([Strangers' worlds](strangers-worlds.md)).
- Render never waits for Sim: it blends the two newest snapshots by the frame's display time, at a cost of at most a step of age on the world's things and none on the head ([Render](../actors/render.md), [Sim](../actors/sim.md)).
- The state traffic of play and the asset traffic of arrival never block each other ([Network](../actors/network.md)).
- No pause stops every actor. Memory cleanup in one actor never stops the picture.
- A world's code runs on a time budget under a watchdog, as JavaScript does in a web browser. The budget is a named quantity, unmeasured. A handler that overruns it is dropped and reported, and the rest of the step runs ([Sim](../actors/sim.md)).

### The head pose

- The head pose is a latest value: one writer overwrites, readers take the newest and never wait. Render reads it last, just before it composes the eye.
- Render is the reader of the device: in the headset it reads the runtime at the predicted display time, on the desktop it reads Player's mouse look. It publishes the pose as drawn, with the frame's display time and the hands, for Player and Sim to read. Player owns the controllers and the actions ([Render](../actors/render.md)).

### The governor

- The governor is set out under Design choices. The island's connected mode uses 4 MB a frame.

### The budgets

- The budgets belong to the performance perspective: the response to an input (100 ms), the frame, and motion to photon. This note names them and does not restate them.

### What is measured

- These were measured in the island, on one small world (9 to 12 beings, 1,577 parts), in runs under 25 s, with the headset by simulator only. A hung script, a ride, a portal and connected play were not measured.
  - Overlapping the frame (Sim on the next step while Render draws this one) has a ceiling of 0.31 ms on the desktop. In the headset it gains no frame time, because the frame is pinned to the runtime's clock, and it costs a frame of age on the head position.
  - Encoding and decoding messages as bytes costs 0.006 ms a frame.
  - A Render that never waits for Sim costs a step of age on the world's things, and gains no frame time.
- In a headset the island runs at 36 Hz. With shadows off, its GPU time is 11.3 and 11.5 ms an eye, over the 13.9 ms both eyes must fit in at 72 Hz or the 11.1 ms at 90 Hz.
- The studio's headset build is reported, in one uncontrolled headset session, at a render time p95 of 26 ms against an 11 ms budget.
- On the desktop the island's input to presented frame measured about 17 ms at p99 on an RTX card and about 15 ms on an integrated one.

## What exists today

- The island steps physics at 1/60 s, banking time and spending whole steps; at 72 frames a second about one frame in six has no step.
- The island in single player takes turns: the shell blocks every frame until the View reports, and Sim awaits the world's turn (`browser/src/browser.cone`, `browser/src/simactor.cone`, `browser/src/viewactor.cone`). `[differs: Render and Audio never block waiting for Sim or a world's code]` A slow world turn lengthens the frame, and a hung one freezes the program.
- The island connected is overlapped: the client never awaits the server, predicts its own body and draws others about 100 ms in the past (`browser/src/viewactor.cone`). Uploads are held to 4 MB a frame (`UPLOAD_BUDGET`).
- In the headset the island's Render reads the head and hands at the predicted display time inside `drawXr` and returns them to the next frame's input, one frame stale (`browser/src/renderactor.cone`, `browser/src/xrhands.cone`). `[differs: the head pose is a latest value read last]`
- The island's mixer runs on its own thread behind a lock-free ring, and its sound is computed in the View each frame (`browser/src/audioview.cone`).
- The studio runs input, the fixed step, panels, the web engine's message pump, file polling and all GPU recording on the main thread, inside the frame package's loop (`guidebrowser/src/guidebrowser.cone`, `guidebrowser/src/webpanel.cone`). `[differs: Render never blocks waiting for a world's code]`
- The studio's pages wait for the GPU: `device.waitIdle` after drawing a document and on closing a page (`guidebrowser/src/docpanel.cone`, `guidebrowser/src/incubator.cone`). `[differs: nothing delays the picture]`
- The studio activates at most 64 built parts a frame and uploads page pictures in 3 ms slices (`guidebrowser/src/scene.cone`, `guidebrowser/src/pageup.cone`). Its World actor runs one step late, and the loop places each part between its last two states.
- The studio's desktop capture is on its own thread, at most 30 Hz, taken by the frame without waiting (`guidebrowser/shim/deskshim.c`, `guidebrowser/src/desk.cone`).

## Open

### Designs not yet settled

- **Each actor's clock.** Proposed: each actor runs on its own clock. Audio's is the sound buffer, a few milliseconds; Player's is each frame, lightly; each Surface keeps its own, the desktop's at most 30 Hz; the Loader has none. Render's clock is the display, every frame, and Sim's is the step, 60 a second; both are decided in [Render](../actors/render.md) and [Sim](../actors/sim.md). Recommended: yes.
- **Waiting without limit for another actor.** Whether it exists only as the test mode that makes a run repeatable. Recommended: yes.
- **Overlapping the frame.** Whether Sim prepares the next step while Render draws this one. The measurements above show almost no gain and a frame of age on the head position. Recommended: no; the boundary between Sim and Render keeps the option if heavy worlds change the numbers.
- **The upload budget,** the Loader's priority and any reserved cores: sizes unmeasured beyond the island's 4 MB.
- **The headset's frame:** the studio's reported p95 of 26 ms against 11 ms and the island's 36 Hz are not met; the means (foveated shading, one pass for both eyes) belong to Render.

## What lives elsewhere

- [Arrival](arrival.md): what arrives, in what order, under what budget.
- [Render](../actors/render.md): the GPU, the upload budget, composing the eye, how its work spreads.
- [Sim](../actors/sim.md): the step, handlers and their budget, the snapshot.
- [Audio](../actors/audio.md): the mixer and the microphone.
- [Strangers' worlds](strangers-worlds.md): the budget on a world's code.
- [Local and connected](local-and-connected.md): prediction and riders.
- The performance perspective: the budgets and how they are measured.
