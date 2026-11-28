# Render

`[planned]`

Render puts the picture on the display and the headset on time, every frame. It owns the GPU copies of meshes and textures and the display, and it draws; it does not build content or simulate. It never blocks on the other actors, and it takes new content under a budget; how it is fed is decided (see Design choices), and what remains undecided is listed in Open. The island engine has a Render actor that owns the GPU, but the data it draws from is assembled by the actor that also predicts the body; the studio draws on its main loop thread.

The frame's deadline rules are in [The frame and its deadlines](../design/frame-and-deadlines.md).

## The design

### Design choices

- **Render's deadline is every display frame,** on the display's clock (in the headset, the runtime's wait for the next frame), on its own high-priority thread. A server has no Render. `[design choice]`
- **Render draws from Sim's snapshot and never waits for it:** it keeps the two newest and blends between them by the display time, then composes the eye last: the platform's pose, the body in the platform, the avatar's camera part, the newest head pose ([Sim](sim.md)). `[design choice]`
- **Render reads the head pose itself, last, before drawing,** because only the reader that knows the display time can ask the runtime for the right moment. In the headset it reads OpenXR at the predicted display time; on the desktop it reads Player's mouse look, a latest value. It publishes the frame's display time and the head and hands as drawn, a latest value Player and Sim read, so a ray means what the person saw. Player owns the controllers and the actions. `[design choice]`
- **Render is the only code that submits to the GPU, for now.** The Loader moves meshes, textures and materials to Render as CPU-side buffers; Render uploads them on a transfer queue beside drawing, under a per-frame upload budget, and a part shows when its content is resident. Whether other owners record their own GPU work for Render to submit is left until a surface or a GPU procgen job needs it. `[design choice]`
- **Render is one actor that divides its work into jobs and runs them in parallel inside itself,** rather than dividing it across several actors. `[design choice]`
- **Render keeps no CPU copy of a mesh:** picking uses the pick shape Sim owns. Render tells the Loader what it dropped (eviction, a lost device), and the Loader's catalog supplies it again if it is still used. `[design choice]`

### Role

Render puts the picture on time: the world the person is in and the live 2-D pictures shown on parts, to the display or to the headset. Render never blocks waiting for Sim, the Loader or a world's code ([The frame and its deadlines](../design/frame-and-deadlines.md)).

### Who it works with

The kinds of handshake are those of [Messages](messages.md); the kinds named below are the decided ones, and the proposed kinds of the rest are in the proposed table of messages there.

- **Sim** ([Sim](sim.md)).
  - In: the snapshot of the zone after each step, a latest value never changed after publishing; Render keeps the two newest and never waits for it (see Design choices).
  - Out: the frame's display time and the head and hands as drawn, a latest value Sim reads (see Design choices).
- **Player** ([Player](player.md)).
  - In: which avatar the person inhabits and how its camera is set ([The person](../design/person.md)). The camera is a part of the avatar.
  - In, on the desktop: the mouse look, a latest value. Out: the same published head pose and hands, which Player reads. The head pose is read last by Render and never travels as a message ([The frame and its deadlines](../design/frame-and-deadlines.md)). What else Player sends is Open.
- **Loader** ([Loader](loader.md)).
  - In: meshes, textures and materials built by the Loader, each built once and moved to Render, its one user ([Arrival](../design/arrival.md)), as CPU-side buffers uploaded under the per-frame upload budget (unmeasured beyond the built figure of 4 MB); arrival is governed ([The frame and its deadlines](../design/frame-and-deadlines.md)).
- **Surfaces** ([Surfaces](surfaces.md)).
  - In: each surface's picture, on the surface's own clock, as a texture under the upload budget; ideally shared on the GPU without a copy.
- **The display and headset runtime.** In: the frame clock. Out: the finished frame.

### What it owns

- The GPU copies of meshes and textures. An editor that changes a mesh hands Render a copy; Render owns that copy like any other.
- The display and the headset.
- The upload budget, in the sense that Render decides how much new content it accepts in a frame.

Render is the only submitter to the GPU for now, so the GPU's queues and memory budget are its to manage (see Design choices). Whether other owners record their own GPU work for Render to submit is Open.

### How it spreads its work

Render is one actor that divides its work into jobs and runs them in parallel inside itself (see Design choices). Whether recording needs to fan out is Open, because the island is bound by GPU time, not by recording.

## What exists today

- **Pegasus skeleton, `pegasus/src/renderactor.cone` (W110, 28 Nov).** A `Render` actor over Cone's `render`, `gpu` and `window` packages (not `browser/src`), its device made on the shell's thread. Its frame loop runs on a worker at the display's rate (vsynced present; headless, a 72 Hz timer) and never waits for Sim: it keeps the two newest snapshots and blends the person between them at the display time less a step, takes the Loader's meshes under a 4 MB per-frame budget and keeps no CPU copy, reads the head pose last in one function (`readHeadPose`: Player's mouse-look slot), and publishes a `DrawnFrame` latest value. `--shot FILE` draws one frame off screen. Measured headless with Sim stalled 200 ms: Render drew 13 frames over 181 ms with no new snapshot, longest gap between frames 15.2 ms. The frame's work is in `RenderCore`, a plain struct a check drives without a GPU. One frame's draw list is plain parts; no lights, shadows or post beyond the render package's defaults.
- **Island, `C:\src\pegasus3d`, `browser/src/renderactor.cone`.** A `RenderActor` owns the GPU (the `Gfx` struct: instance, device, swapchain, renderer, post effects, render system, sky, ground cover, HUD, headset). It draws from the snapshot alone. `[differs: the snapshot is built by the view actor (`viewactor.cone`, `publishSnapshot`), not by the simulation; it is moved to Render in `DrawOrder` and moved back in `Presented`]`.
- **Island, the headset.** `drawXr` reads the head and hands at the predicted display time. The hands come back in `Presented.xr` and fold into the next frame's input, one frame late (`xrhands.cone`).
- **Island, uploads.** The view holds uploads to 4 MB a frame when connected (`viewactor.cone`, `UPLOAD_BUDGET`). `[differs: the design has Render accept content under its budget]`.
- **Island, picking and content.** The view keeps a CPU copy of the draw meshes for picking (`PickSystem`), and each object is built a second time there. `[differs: Render holds the only draw meshes]`. Despawning a part does not free its GPU meshes (`parts/parts.cone`).
- **Studio, `guidebrowser/src`.** There is no Render actor: the loop thread owns the device, renderer and scene (`guidebrowser.cone`). Pages are drawn as unlit quads over the finished picture (`PagePass`). Document pages are uploaded in 3 ms slices (`pageup.cone`), but drawing a page on the GPU makes the main thread wait for the device to go idle (`docpanel.cone`, `device.waitIdle`; also `incubator.cone`).
- **Measured, island, at the Quest 3's eye size.** About 80% of an eye's GPU time scales with pixels; a million triangles cost about 0.55 ms an eye. At 72 Hz one eye has about 6.9 ms. Frames at eye size measure 7.28 ms (the beach) and 9.40 ms (the lawn).
- **Measured, synchronisation.** A frame has 37 passes, 77 image transitions and 37 global barriers. Narrowing every barrier to the stages involved bought 0.00 to 0.01 ms an eye; removing all compute barriers (incorrect, a bound only) bought 0.19 ms. The overlap of independent passes is not measured.
- **Measured, pipelining.** Overlapping the next step's simulation with this frame's draw gains at most 0.31 ms on the desktop and nothing in the headset, where the frame is pinned to the runtime's clock, and it costs a frame of age on the head position. The frame is not pipelined; the boundary between Sim and Render keeps the option.
- **Reported, studio in the headset.** Render p95 of 26 ms against an 11 ms budget (jank and flicker).
- **Cone's `gpu` package** synchronises conservatively (each texture rests in one layout, global barriers around compute) and gives each resource its own allocation, with no suballocation or aliasing. The pass chain is ordered by hand (`render/fx.cone`).

## Open

These are proposed settlements, not decided design. Each says what it proposes and the reason.

- **A bounded wait, as an optimisation to measure later.** Whether Render waiting briefly for the step due this frame (under 1 ms suggested) is worth saving a step of age on the world's things. Not built first; measure once the browser runs.
- **Controllers in the headset.** To check: whether OpenXR lets one thread run the frame loop while another syncs actions on the same session; if not, Render reads the actions on its thread and publishes them to Player.
- **Other GPU submitters.** When a surface or a GPU procgen job needs the GPU: each records its own work and Render submits it under its budgets, or each submits under Render's rules. Lean: the first, so no second submitter waits on the device.
- **Surfaces.** Proposed: surfaces run on their own threads and hand Render pictures as latest values; Render tells each whether it is visible and how large it is drawn, so a hidden or distant page stops painting. Recommended: this.
- **How the browser's own interface reaches Render.** Options: a trusted local world with its own Sim that publishes a snapshot Render draws as a top layer, or plain code inside Player that hands Render a list of quads. Recommended: the local world, so one model of parts and triggers serves everything; the cost is that the browser's most trusted prompts then depend on the content language's runtime.
- **Whether to fan out command recording, or move to GPU-driven rendering.** Options: parallel recording over chunks of equal cost; GPU-driven culling and indirect draws, with mesh shaders as the later form. Neither touches the per-pixel cost that dominates the island. Recommended: measure CPU recording time against GPU time per pass in the headset, then study the two before choosing.
- **Synchronisation and memory.** Options: leave barriers as built (they cost at most about 0.2 ms an eye); derive barriers from the accesses each pass declares, which changes the public shape of Cone's `gpu` package; later, suballocation and aliasing for a standalone headset. Recommended: leave as built until the overlap of independent passes is measured.
- **Speed in the headset.** Options: fixed foveated shading, both eyes in one pass, shared shadow cascades. The effect of each in this browser is unmeasured.
- **A portal's view of a far zone.** Render could draw the far zone from its latest snapshot through a camera the portal part names, once per eye. Recommended: when the part asks for a live view.
- **Local models that use the GPU.** Speech and language models run in their own process on the same device. Options: leave them outside Render's ownership under a video memory budget (about 2 GB of 8 is the working figure), or run them as jobs under Render's budgets. Recommended: outside, with a budget.
- **One-off effects** that are not part state (a burst of particles): an event from Sim, or a short-lived part in the snapshot.

### Deferred, with safety and identity

- **What else Player sends Render:** comfort effects to draw (a vignette), people to hide, the personal-space bubble. Recommended: Player sends them as a latest value. This rests on safety and comfort following the person into every world, which is deferred.

## What lives elsewhere

- [The frame and its deadlines](../design/frame-and-deadlines.md): Render's wait on Sim, the head pose, the upload governor, measured costs.
- [Arrival](../design/arrival.md): the content key and when a part shows.
- [Isolation and ownership](../design/isolation.md): moved data, latest values, and how each structure is shared.
- [Local and connected](../design/local-and-connected.md): the platform frame and why the eye is composed last.
- [Sim](sim.md): the snapshot and the step. [Loader](loader.md), [Surfaces](surfaces.md), [Audio](audio.md), [Player](player.md), [Messages](messages.md).
- The manual's frames, cameras and overlays pages.
