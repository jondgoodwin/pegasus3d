# browser: Pegasus3D in Cone

A native 3-D browser written in [Cone](https://github.com/jondgoodwin/cone), in the spirit of the
Acorn-era Pegasus3D in `src/`. It is the shell and the walkable plane (an endless brown ground under a
sky-blue sky, in metres, walked on at eye height, 1.7 m) and a baked-in world standing on it: two chess
rooks, a horn-flower, a horn, a brick chimney stack with a black-figure amphora on one side of it and a
boulder on the other, a hot-air balloon with its burner alight and,
hovering beyond them, Elizabeth's skeletal dragon starship, built from Cone's sculpt, vfx, sdf and sdfmesh
packages and sent in as if downloaded. Things move: a ball bounces and spins, the burner fires in bursts,
and a launch button by the path sends the balloon up (and, pressed again, brings it down). A click
picks what it lands on and highlights it.

## Building and running

The browser is a Congo package that lives outside the Cone repository and imports Cone's own packages
(`frame`, `render`, `gpu`, `input`, `controls`, `geomath`, `mesh`, `pool`, `sculpt`, `vfx`, `noise`,
`sdf`, `sdfmesh`, `gpuwork`, `sdl` ...) by name. Congo always searches the `packages/` folder of the Cone
repository it runs from, so nothing has to be configured or copied: run the Congo of a built Cone
checkout from this folder.

```
cd browser
set LIB=C:\libs\SDL3-3.4.16\lib\x64;%LIB%
python C:\src\cone\tools\congo\congo.py build --release
build\release\browser.exe
```

(`C:\src\cone\tools\congo\congo.bat run` builds and runs in one step; `-- args` passes arguments.)
It needs `conec` built in the Cone checkout (`build\x64-release\conec.exe`), Python 3.11 or later, a
GPU driver with Vulkan 1.3, and SDL3: `SDL3.lib` on `LIB` to link (Congo copies `SDL3.dll` beside the
program). With the Vulkan SDK installed, the validation layer is on.
`GPU_POWER_PREFERENCE=low-power` picks the integrated GPU where there are two.

The build also compiles the browser's `gpu/` folder (the starship's distance field and kernels) for the
GPU, into `build\release\browser.spv`, beside `sdfmesh.spv`; the program reads both at run time. The
first run after a driver's shader cache is cleared makes the kernels in about 4 s; after that, 0.4 s.

## Keys

| Key | Does |
| --- | --- |
| W or Up | walk forward (3 m/s) |
| S or Down | walk back |
| A or Left | turn left (a quarter turn in 1.5 s) |
| D or Right | turn right |
| Shift (held) | walk four times as fast (12 m/s) |
| Left click | pick what is under the pointer: it is named on the console and highlighted; the launch button launches or lands the balloon |
| P | pause scene time, or start it again (the world's motions, the effects, the timers and the day stop; the walk does not) |
| F | run scene time ten times as fast, or back to normal (for testing the day: dusk comes 50 s after the start instead of 12.5 min) |
| F11 | fullscreen |
| Escape | quit |

## Automated runs

```
browser.exe --script                  walk and turn with no one at the keyboard, checked
browser.exe --script --shots shots    the same, saving its views as BMPs into shots/
browser.exe --headless --script       the same with no window: the scene, the world and the walk
browser.exe --checks                  the scene core's own checks, on scenes of their own (below)
browser.exe --frames 600              600 frames on frame's synthetic clock
browser.exe --soak 12000              12000 frames, checking nothing grows (below)
browser.exe --watch 300               300 s in real time, a line a second and a key probe (below)
```

`--script` pushes key events into SDL's own queue (never the operating system's), so nothing reaches
any other window, and a run of a fixed number of frames ignores the real keyboard and mouse (and the
window losing focus, which would otherwise let go of the script's keys). It walks up to the rooks and
the horn-flower, clicks on the rook and then on the sky, turns, runs to the balloon, clicks its
launch button, then draws six close-ups from eyes of its own (the amphora's through a 20 degree lens,
straight on, as its photograph shows it). Its clicks are pushed mouse events at
the place the script works out the target is drawn. It checks the walk against what its events must
give (exact on the synthetic clock); what each click picked and highlighted; that the world heard the
button and the balloon (every one of its parts) rose as its formula says; the ball's height against
its formula; every burst of the burner sent was applied; that the scene applied, refused and removed
what it was sent; and that every part the world sent is live, placed and (with a GPU) drawn or giving
off particles, with every request sent accounted for as applied or refused; and that the amphora and
the chimney stack are the sizes they were built to (the amphora 1.484 m to its tip, 0.3368 m at its
widest; the stack 47 courses, 3.525 m, and 5 x 4 bricks at its top course) and drawn physically based.
It also presses P (frames 256 and 271) and F (276 and 286) while the walker stands, and checks that
scene time stood still while paused and ran ten times as fast at x10; that the world's alarm (a timer
for 3 s) rang at the first frame at or after 3 s; that the world was told the walker came up to the
launch button (an area) and is still there; the day clock's reading; that the balloon is one tree as
built; and that the two rooks share one material (one mesh more than materials defined, and, with a
GPU, both drawn in the same material).
It exits 0 only if every check passed, no Vulkan call failed and the validation layer said nothing.

`--checks` runs the scene core's checks on scenes of their own, with no window, GPU or frames: a
three-level placement tree made children first, its world matrices against ones worked out by hand, a
placement changed recomputing only its own row, a reparent to the root and back under the top, a cycle
refused, and the top despawned with its subtree; timers on the scene's clock in 1/60 s steps at x1,
paused and at x10, each rung at the first step at or after its time, a cancel, and dusk at 750 s and
dawn at 1290 s; a sphere area and a box area under a scaled parent entered and left, and a second
being coming in moving the first out; and possession, the user's intents moving only the possessed
being and the camera following it. It exits 0 only if every check passed.
`shots/` holds the views of one run (`start`, `picked`, `walked`, `turned`, `balloon`, and the close-ups
`rooks`, `chimney`, `starship`, `rising`, `amphora`, `hornflower`), converted to PNG, and
`amphora-vs-photo.png`, the amphora's close-up beside the photograph it was traced from.

`--soak N` runs N frames on the synthetic clock and, once the world is in, checks every frame that the
scene's inbox is empty after the apply point and that the parts, the rows drawn and the particle
emitters stay as many as they were, and every 3000 frames that the browser's own work a frame (input,
simulate, publish, render; not the present's wait for the display) is no more than twice the first
window's plus 0.5 ms. It exits 1 if any check fails.

`--watch S` runs S seconds on the real clock in a window, printing a line a second (frames, frame time,
each phase's mean, the present and the acquire apart, fixed steps a frame, events, the window's own
events, queue and table sizes) and every 5 s a probe: a W or S pushed into SDL's queue and timed until
the walk it moves is presented.

Every run prints, at the end, each object's triangles and the time it took to hydrate and to send, and
along the way when the first frame was presented and when every object was first on screen.

## How it is put together

| File | What it is |
| --- | --- |
| `src/browser.cone` | the browser, a world that frame's loop runs (`mod browser is World`): input, the beings in fixed steps, the scene's events, the world's next object and the apply point, the snapshot, drawing |
| `src/parts/parts.cone` | the module `parts`: one id registry for parts and resources, aspect tables keyed by id, the requests and messages, the Door senders are given, and the Scene |
| `src/parts/placement.cone` | the placement tree: parents before children, world matrices in one pass over what changed |
| `src/parts/events.cone` | the scene's clock and the day, timers on the timewheel, areas |
| `src/parts/snapshot.cone` | what the scene publishes each frame for drawing |
| `src/rendersys.cone` | the render system: owns the meshes and materials defined, how parts are drawn (in batches by mesh and material), their effects (particles) and the sky, and turns them, with the snapshot, into render's draw list (render draws each batch as one instanced draw); a physical material is render's physically based one; a shaded one, a custom pipeline |
| `src/look.cone` | the look: light in physical units (the sun in lux, the sky and glows in nits), the exposure from the sun's height (EV 15 at noon), AgX tone mapping and bloom through render's Post |
| `src/shaders.cone` | the browser's own shaders, which a shaded Look names: the heightfield (`heightfield.slang`); after editing a `.slang`, run Cone's `tools/shaders/shaders.py src` to compile and embed it |
| `src/shell.cone` | what the browser puts in every scene: the ground and the sky, sent as requests |
| `src/being.cone` | beings and possession: intents, the user's controller, the Cast |
| `src/walk.cone` | a walker's body on the ground, and the user's key bindings |
| `src/script.cone` | the scripted run, its close-ups and its checks |
| `src/checks.cone` | the scene core's headless checks (`--checks`) |
| `src/picking.cone` | the picking system: each mesh's triangles, which mesh each part is, and the ray a click casts |
| `src/watch.cone` | the watched real-time run and the soak |
| `src/bakedworld/` | the module `bakedworld`, the baked-in world: its objects' descriptions, hydrating and injecting them, and its motions (`motions.cone`) |
| `src/dragonship/` | the module `dragonship`: the starship's description and its meshing on the GPU, on a device of the world's own |
| `gpu/shipfield.cone` | the starship's GPU half: its distance field and kernels, compiled into the browser and into `browser.spv` |

**Parts are ids.** One registry (a generational `pool.Pool`) hands out every part's id, a slot and a
generation, so an id whose part is gone is detected rather than mistaken for the slot's next occupant.
What a part is lives in tables keyed by its id (sparse sets), each owned by one system: the scene owns
placement, listeners, areas and timers; the render system owns what a part is drawn as, the particles
it gives off, and the sky. Meshes and materials take their ids from the same registry (its record says
what kind of thing an id names) and are shared: many parts name one mesh and one material.

**Changes are requests.** Whoever wants the scene changed reserves an id and sends requests into the
scene's inbox:

| Request | Does |
| --- | --- |
| `Spawn` | a reserved part becomes live, with a name |
| `Despawn` | a part goes, with its whole subtree and every aspect they have |
| `Place` | a part placed, in its parent's frame (in the world's, with none) |
| `Parent` | a part put under another (or under none), keeping its own placement; refused if it would make a cycle |
| `DefineMesh` | a mesh, its id reserved by `Door.defineMesh` |
| `DefineMaterial` | a material: a look, a painted image and a physical look's maps, its id reserved by `Door.defineMaterial` |
| `Show` | a part drawn as a mesh in a material, both defined before; optionally anchored under the viewer |
| `Effect`, `Fire` | a part's particle emitters, and a burst of them |
| `Listen` | a part's clicks wanted: posted to the sender as `Clicked` |
| `Highlight`, `Sky` | the browser's own: what was picked, and the sky |
| `Timer`, `Cancel` | a timer on the scene's clock, ringing once or every period, posted back as `Rang` with the sender's token; and all timers with a token disarmed |
| `Area` | a part is an area (a sphere or a box in its own frame): the possessed being coming in or going out is posted as `Entered` or `Left` |

Once a frame, at the publish phase, the browser applies the inbox in order, routing each request to the
system that owns what it changes; a request naming a part that is gone is refused and counted. The
ground and the sky go in this way too: the ground is an ordinary part whose render row is *anchored*,
drawn under the viewer in steps of its checkerboard's period, so it never ends. A material may carry a
painted image (the balloon's envelope, the basket's wicker), which its colour multiplies, and, for a
*physical* look, a normal map and an ORM map (the amphora's glaze glossy and its clay matt; the
chimney's mortar recessed), drawn with render's physically based material; an `Effect`
is a part's particle emitters (the vfx package's), fired from a start to a stop on the scene's clock
and drawn as they are at the scene's time each frame; a `Fire` fires it again (a burst). The render
system keeps the parts it draws in batches by (mesh, material), ready for instancing.

**Placement is a tree.** The placement table is kept parents before children, so after the apply point
the world matrices are composed (world = parent's world x local) in one pass from the first row that
changed. The balloon is one tree: its envelope is the root, with the skirt, the cables, the burner frame
(and under it the flame and the pilot light) and the basket (and under it the passengers) beneath, so
launching it moves only the envelope. The renderer never walks the tree: each frame the scene publishes
a **snapshot** (`parts/snapshot.cone`), a plain struct of every placed part's world matrix, the scene's
time, the day clock and the possessed being's eyes, and the draw phase works from that alone (when the
scene and render are actors, it is what moves between them).

**Beings and possession.** The walker is a *being*: a part (`you`) with a body (the walk) that moves by
the *intents* it is given each step. The user's *controller* turns the input package's actions into
intents for whichever being the user *possesses*, and the camera follows the possessed being's eyes.
Input, camera and walker are not wired together, so another being (T2's mannequin) can be possessed,
or driven by another controller, without touching either (`src/being.cone`).

**The scene's time** is the scene's own clock: frame's fixed steps at its rate, 1, or 0 paused (P), or
10 (F); exact on the synthetic clock. A day is 24 minutes of it, the day clock reading 07:00 at the
start, dusk at 19:30 (750 s) and dawn at 04:30 (1290 s), each posted to the world as `Dusk` and `Dawn`.
Timers run on Cone's `timewheel`, driven by scene time. Each
frame the world reads it and sends its motions as ordinary `Place` and `Fire` requests: the ball's
height `1.6 |sin(pi t / 1.1)|` m and its spin, 3 rad/s; the button's cap sinking 3 cm for a quarter of
a second when pressed; the balloon on the ground, its burner firing 3 s in every 4; pressed, rising at
2.5 m/s with the burner held on to 30 m, then bobbing 0.6 m either way every 7 s with a 2 s burst every
6 s; pressed again, the burner out, sinking at 1.5 m/s to land.

**A click picks.** The picking system keeps a copy of each mesh's triangles (taken when its
`DefineMesh` is applied) and which mesh each shown part is (not the anchored ground). A left click casts a ray from the eye through the
pointer, takes it into each part's own frame by the inverse of its placement, tests the part's box,
and, where the box is entered nearer than the best hit so far, each triangle; the nearest triangle
hit names the part. The browser prints it and sends itself a `Highlight` (the render system draws the
part in its colour half-way to yellow, and lets the last one go).

**Buttons are messages to the world.** A world that wants a part's clicks sends `Listen` for it; when
a click picks that part, the browser posts `Message.Clicked` into the world's Door, and the world reads
its mail at its next tick, at the publish phase of the same frame, and answers with requests. The
browser never reaches into the world, and the world never into the scene's tables: the Door's mail is
the world's inbox, as an actor's would be. Timers (`Rang`), dusk and dawn, and areas (`Entered`,
`Left`) come back the same way: the baked-in world sets an alarm for 3 s and keeps an area of 5 m
about the launch button, and prints what it is told.

**A world is given the Door, not the scene.** The `parts` module keeps the registry, the inbox and the
placement table private, and hands a sender a `Door`, which only reserves ids and sends requests. The
baked-in world is a sister module of `parts` and gets `&mut scene.door`; it cannot name the browser's
scene, any table, the render system or a GPU handle, so fetching worlds later changes where a world
comes from, not what it can do. To mesh the starship it makes a GPU device of its own (an instance and
a device with no surface), never the browser's.

**Hydrate, then inject.** Each object of the baked-in world is a description in Cone, ported from the
example programs of Cone's `sculpt` and `vfx` packages and from the `starship` package (examples and
executables are not importable, and this content belongs to the browser). Hydrating one runs it through
those libraries into plain data, a `Hydrated`: meshes, materials (with their paint images), parts
naming them with their parents and placements, and emitters, with no ids. Injecting it defines its
meshes and materials (a material the world shares across objects, the rooks' ivory, only once),
reserves an id per part and sends its requests. The `Hydrated` is what a world actor
would send; today one object is hydrated and injected a frame, at the start of the publish phase, so
frames keep coming while the world arrives.

| Object | Ported from | Size | Triangles | Hydrated in (ms) |
| --- | --- | --- | --- | --- |
| chess rook | `sculpt/examples/rook.cone` (a lathe and three booleans), 96 steps round | 1.4 m | 2,020 | 92 |
| lathed rook, beside it | the same rook with no booleans: one lathe, four capped merlons joined on | 1.4 m | 2,320 | 0.75 |
| amphora | new (`amphora.cone`): an outline traced from a photograph, lathed 128 steps round; two handles swept along a traced centreline; its bands and patterns painted, 2048 x 2048, and an ORM map | 1.5 m | 63,832 | 100 (paint 83) |
| horn-flower (stalk, leaves, petals, heart) | new (`hornflower.cone`): the horn, 2.4 m and cut short; `sculpt/examples/flower.cone`'s petals on the cut; five swept leaves | 2 m | 99,956 | 39 |
| boulder | `sculpt/examples/pebble.cone` (one more subdivision) | 2 m | 768 | 0.4 |
| horn | `sculpt/examples/hornfamily.cone`, stage 0 | 3 m along | 46,204 | 18 |
| chimney stack (stack, flaunching, pot) | new (`chimney.cone`): brick masonry, 4 x 3 bricks, 47 courses; boxes, its bricks a 1620 x 1620 colour, normal and ORM map | 4 m to the pot's rim | 808 | 254 (maps) |
| hot-air balloon (envelope, skirt, burner frame, cables, basket, passengers) | `sculpt/examples/balloon.cone`, the chevrons | 25 m | 74,264 | 52 |
| burner flame (core, tongue, embers) and pilot light | `vfx/examples/burner.cone`, live, fired in bursts | 3 m | particles | (in the balloon's) |
| launch button (pedestal, cap) | new: a box and a lathed disc; the cap listens for clicks | 1 m | 140 | 0.06 |
| ball | new: a sphere, checkered so its spin shows | 1 m across | 1,984 | 0.03 |
| mound | new: a 65 x 65 grid lifted on the GPU by a heightfield (the heightfield shader, a custom pipeline), green rising to a faintly glowing sandy hump | 4 m square | 8,192 | 0.14 |
| starship (frame, membranes, wing lights, ports, eyes, mouth) | `packages/starship`, meshed on the GPU at 0.03 a cell | 156 m, 10 m a unit | 458,324 | 700 (0.4 s of it making the kernels) |
