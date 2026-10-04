# browser: Pegasus3D in Cone

A native 3-D browser written in [Cone](https://github.com/jondgoodwin/cone), in the spirit of the
Acorn-era Pegasus3D in `src/`. It is the shell and the walkable plane (an endless brown ground under a
sky-blue sky, in metres, walked on at eye height, 1.7 m) and a baked-in world standing on it: a chess
rook, an urn, a flower, a boulder, a horn, a chimney stack and a hot-air balloon with its burner alight,
built from Cone's sculpt and vfx packages and sent in as if downloaded.

## Building and running

The browser is a Congo package that lives outside the Cone repository and imports Cone's own packages
(`frame`, `render`, `gpu`, `input`, `controls`, `geomath`, `mesh`, `pool`, `sculpt`, `vfx`, `noise`,
`sdl` ...) by name. Congo always searches the `packages/` folder of the Cone repository it runs from, so
nothing has to be configured or copied: run the Congo of a built Cone checkout from this folder.

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

## Keys

| Key | Does |
| --- | --- |
| W or Up | walk forward (1.5 m/s) |
| S or Down | walk back |
| A or Left | turn left (a quarter turn in 1.5 s) |
| D or Right | turn right |
| Shift (held) | walk four times as fast |
| F11 | fullscreen |
| Escape | quit |

## Automated runs

```
browser.exe --script                  walk and turn with no one at the keyboard, checked
browser.exe --script --shots shots    the same, saving four views as BMPs into shots/
browser.exe --headless --script       the same with no window or GPU: the scene, the world and the walk
browser.exe --frames 600              600 frames on frame's synthetic clock
```

`--script` pushes key events into SDL's own queue (never the operating system's), so nothing reaches
any other window, and a run of a fixed number of frames ignores the real keyboard and mouse. It walks
up to the rook, the urn and the flower, turns, and runs to the balloon. It checks the walk against what
its events must give (exact on the synthetic clock); that the scene applied, refused and removed what it
was sent; and that every part the world sent is live, placed and (with a GPU) drawn or giving off
particles, with every request sent accounted for as applied or refused. It exits 0 only if every check
passed, no Vulkan call failed and the validation layer said nothing. `shots/` holds the views of one
run (`start`, `walked`, `turned`, `balloon`), converted to PNG.

Every run prints, at the end, each object's triangles and the time it took to hydrate and to send, and
along the way when the first frame was presented and when every object was first on screen.

## How it is put together

| File | What it is |
| --- | --- |
| `src/browser.cone` | the browser, a world that frame's loop runs (`mod browser is World`): input, the walk in fixed steps, the world's next object and the apply point, drawing |
| `src/parts/parts.cone` | the module `parts`: one id registry for parts, aspect tables keyed by id, the requests, the Door senders are given, and the Scene |
| `src/rendersys.cone` | the render system: owns how parts look, their effects (particles) and the sky, and turns them into render's draw list |
| `src/shell.cone` | what the browser puts in every scene: the ground and the sky, sent as requests |
| `src/walk.cone` | walking on the ground: the controller and its key bindings |
| `src/script.cone` | the scripted run and its checks |
| `src/bakedworld/` | the module `bakedworld`, the baked-in world: its objects' descriptions, hydrating and injecting them |

**Parts are ids.** One registry (a generational `pool.Pool`) hands out every part's id, a slot and a
generation, so an id whose part is gone is detected rather than mistaken for the slot's next occupant.
What a part is lives in tables keyed by its id (sparse sets), each owned by one system: the scene owns
placement; the render system owns what a part is drawn as, the particles it gives off, and the sky.

**Changes are requests.** Whoever wants the scene changed reserves an id and sends requests (`Spawn`,
`Place`, `Show`, `Effect`, `Sky`, `Despawn`) into the scene's inbox. Once a frame, at the publish
phase, the browser applies the inbox in order, routing each request to the system that owns what it
changes; a request naming a part that is gone is refused and counted. The ground and the sky go in this
way too: the ground is an ordinary part whose render row is *anchored*, drawn under the viewer in steps
of its checkerboard's period, so it never ends. A `Show` may carry a painted image (the balloon's
envelope, the basket's wicker), which its colour multiplies; an `Effect` is a part's particle
emitters (the vfx package's), held at one moment until something sends the time on.

**A world is given the Door, not the scene.** The `parts` module keeps the registry, the inbox and the
placement table private, and hands a sender a `Door`, which only reserves ids and sends requests. The
baked-in world is a sister module of `parts` and gets `&mut scene.door`; it cannot name the browser's
scene, any table, the render system or a GPU handle, so fetching worlds later changes where a world
comes from, not what it can do.

**Hydrate, then inject.** Each object of the baked-in world is a description in Cone, ported from the
example programs of Cone's `sculpt` and `vfx` packages (examples are not importable, and this content
belongs to the browser). Hydrating one runs it through those libraries into plain data, a `Hydrated`:
meshes, paint images, emitters and placements, with no ids. Injecting it reserves an id per part and
sends its requests. The `Hydrated` is what a world actor would send; today one object is hydrated and
injected a frame, at the start of the publish phase, so frames keep coming while the world arrives.

| Object | Ported from | Size | Triangles |
| --- | --- | --- | --- |
| chess rook | `sculpt/examples/rook.cone` | 1.4 m | 738 |
| urn | `sculpt/examples/vase.cone` | 1 m | 12,672 |
| flower | `sculpt/examples/flower.cone` | 1.8 m across | 33,600 |
| boulder | `sculpt/examples/pebble.cone` (one more subdivision) | 2 m | 768 |
| horn | `sculpt/examples/hornfamily.cone`, stage 0 | 3 m along | 46,204 |
| chimney stack | `sculpt/examples/chimney.cone` | 3.5 m | 1,216 |
| hot-air balloon (envelope, skirt, burner frame, cables, basket, passengers) | `sculpt/examples/balloon.cone`, the chevrons | 25 m | 74,264 |
| burner flame (core, tongue, embers, pilot) | `vfx/examples/burner.cone`, held 1.25 s into a burst | 3 m | particles |
