# Surfaces

`[planned]`

Surfaces are the live 2-D pictures shown on parts: documents, web pages and the desktop. Each has its own source and its own clock, and each hands Render a picture; how surfaces are threaded and how they talk to Sim are open. The studio shows all three; its desktop capture runs on its own thread as the design proposes, while its pages and documents run on the main thread and stall the GPU. The island has no surfaces.

## The design

### Role

A surface is a live 2-D picture with its own source and clock, shown on a part. Documents, web pages and the laptop's desktop are surfaces. Each is a 2-D producer that runs on its own clock and hands Render textures under the upload budget; ideally the texture is shared on the GPU without a copy. A web page is a stranger: it runs in CEF (the embedded Chromium), which brings Chromium's sandbox.

### Who it works with

The kinds of handshake are those of [Messages](messages.md); the proposed kinds are in the proposed table of messages there.

- **Render** ([Render](render.md)).
  - Out: the surface's picture, on the surface's own clock, as a texture Render accepts under its per-frame upload budget (unmeasured beyond the built slices in "What exists today").
  - In: whether the surface is visible and how large it is drawn is proposed (see Open).
- **Player** ([Player](player.md)).
  - In: what the person types, forwarded by Player straight to the focused page ([Strangers' worlds](../design/strangers-worlds.md), [Actions and triggers](../design/actions-and-triggers.md)); the reserved keys stay live while a page has focus.
  - Sending input to the desktop controls the whole laptop; whether that is a grant is Open.
- **Sim** ([Sim](sim.md)).
  - A surface is shown on a part, and the part is Sim's. How Sim opens, closes and points a surface, and what a surface reports back (a page's title changed), are Open.
- **The Loader** ([Loader](loader.md)). Whether they exchange anything is Open.

### What it owns

- Each producer's source and its picture.

### How it spreads its work

Each surface runs on its own clock, not the frame's. How threads and the GPU queue are split is proposed in Open.

## What exists today

All in the studio, `guidebrowser/src`; the island has no surfaces.

- **Desktop, `desk.cone`, `shim/deskshim.c`.** A capture thread on its own D3D11 device, at most 30 Hz, with three buffers swapped under a lock and taken by the frame without waiting. This is the latest-value pattern, already isolated. Input to the desktop goes out through `SendInput`.
- **Web pages, `webpanel.cone`, `cef/src/cefshim.cc`.** CEF runs windowless with its sandbox off. `[differs: the design has the sandbox on for a stranger's page]`. Its message loop is pumped once a frame on the main thread, so the paint callback runs there and copies the whole frame. `[differs: its own thread]`. The changed rectangle is converted and uploaded, at most 30 paints a second and 40 rows a frame. Hidden tabs are skipped.
- **Documents, `docpanel.cone`, `pageup.cone`.** On the main thread: a file check every 250 ms; on change a whole-file read, parse, diff against the old tree and re-layout. The page is drawn on the GPU and read back, and the main thread waits for the device to go idle (`device.waitIdle`; also in `guidebrowser.cone` and `incubator.cone`). `[differs: no surface stalls the frame]`. The picture is then converted and uploaded in time slices under a 3 ms budget. A document writes its own file through a temporary file and a rename.
- **Typing.** The studio's `fn event` (`guidebrowser.cone`) is a priority cascade (web focus, edit focus, palette, keyboard, desktop, chips, pages, parts) on the main thread. The studio's worlds carry no code that could read it.
- **Other.** Markdown and Mermaid pages, a code view, and the studio's palette are drawn as quads by `PagePass`.

## Open

These are proposed settlements, not decided design.

- **What each surface owns.** Proposed: a document owns its text, its parsed form, its layout and its picture; a web page owns its page through CEF; the desktop owns its capture. Each surface is small. Recommended: this.
- **Surfaces on their own threads, with the sandbox on.** Proposed: each surface runs on its own threads, CEF's pump included, and CEF's sandbox is on for a stranger's page; Render tells each surface whether it is visible and how large it is drawn, so a hidden or distant page stops painting. Recommended: yes.
- **Who owns disk.** Proposed: the Loader owns content (worlds, assets, the bake cache) and a surface owns its one source, so a document being edited can save its own file, and the Loader and a surface exchange nothing. Recommended: yes; routing every save through the Loader buys nothing.
- **Who submits a surface's GPU work.** For now Render is the only code that submits GPU work, and a surface hands Render a picture as a texture ([Render](render.md)). Proposed, for when a surface draws on the GPU: it records its own work and Render submits it under Render's queue and budget, with no waiting on the device. Recommended: yes.
- **The pointer on a page.** Typing goes straight to the focused page ([Strangers' worlds](../design/strangers-worlds.md)). Proposed: Player routes the pointer straight to the focused surface too, through no world; Player decides which surface has focus, and Sim learns only that focus moved. Recommended: yes.
- **Input to the desktop.** Proposed: sending input to the desktop is a privilege Player grants. Recommended: yes, since it controls the whole machine.
- **How Sim and a surface talk.** Proposed: Sim opens or closes a surface for a part and navigates it (event); a surface tells Sim its content changed (event, a trigger); a click on a page gives Sim a hit with the page's coordinates and Sim tells the surface where. Recommended: this.
- **Sharing the texture without a copy.** The desktop could share a D3D11 texture with Vulkan, and a page could share its GPU surface; both are platform-specific. Recommended: later, after the copy path is measured.
- **What an AI character may see of a document.** Perception limited to what Player grants (the documents in view, the clicks) depends on where an AI character lives, which is open.
- **The upload budget for pictures:** unmeasured. A page that changes in full every frame needs a limit on how often it may repaint; unmeasured.

### Deferred, with safety and identity

- **The origin rule for surfaces.** Proposed: one origin rule governs both what the Loader fetches and the one source a surface owns. This rests on the origin deciding what an asset may do, which is deferred.

## What lives elsewhere

- [Render](render.md): the upload budget and the GPU's queue.
- [Player](player.md): focus and routing of typing.
- [Live editing](../design/live-editing.md): documents edited in place, and the studio as the first case.
- [Strangers' worlds](../design/strangers-worlds.md): the origin rule and why a page is a stranger.
- [Isolation and ownership](../design/isolation.md): latest values and the one lock in the desktop's buffer swap.
- [Sim](sim.md), [Loader](loader.md), [Messages](messages.md); the manual's overlays page.
