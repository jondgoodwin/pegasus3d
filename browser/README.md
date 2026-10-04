# browser: Pegasus3D in Cone

A native 3-D browser written in [Cone](https://github.com/jondgoodwin/cone), in the spirit of the
Acorn-era Pegasus3D in `src/`. This first cut is the shell and the walkable plane: an endless brown
ground under a sky-blue sky, in metres, walked on at eye height (1.7 m), with a post at the origin to
walk towards.

## Building and running

The browser is a Congo package that lives outside the Cone repository and imports Cone's own packages
(`frame`, `render`, `gpu`, `input`, `controls`, `geomath`, `mesh`, `pool`, `sdl` ...) by name. Congo
always searches the `packages/` folder of the Cone repository it runs from, so nothing has to be
configured or copied: run the Congo of a built Cone checkout from this folder.

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
browser.exe --script --shots shots    the same, saving three views as BMPs into shots/
browser.exe --headless --script       the same with no window or GPU: the scene and the walk only
browser.exe --frames 600              600 frames on frame's synthetic clock
```

`--script` pushes key events into SDL's own queue (never the operating system's), so nothing reaches
any other window, and a run of a fixed number of frames ignores the real keyboard and mouse. It
checks the walk against what its events must give (exact on the synthetic clock), and that the
scene applied, refused and removed what it was sent. It exits 0 only if every check passed, no Vulkan
call failed and the validation layer said nothing. `shots/` holds the views of one run (`start`,
`walked`, `turned`), converted to PNG.

## How it is put together

| File | What it is |
| --- | --- |
| `src/browser.cone` | the browser, a world that frame's loop runs (`mod browser is World`): input, the walk in fixed steps, the apply point, drawing |
| `src/scene.cone` | the scene: one id registry for parts, aspect tables keyed by id, and the inbox of requests |
| `src/rendersys.cone` | the render system: owns how parts look and the sky, and turns them into render's draw list |
| `src/shell.cone` | what the browser puts in every scene: the ground, the sky and the post, sent as requests |
| `src/walk.cone` | walking on the ground: the controller and its key bindings |
| `src/script.cone` | the scripted run and its checks |

**Parts are ids.** One registry (a generational `pool.Pool`) hands out every part's id, a slot and a
generation, so an id whose part is gone is detected rather than mistaken for the slot's next occupant.
What a part is lives in tables keyed by its id (sparse sets), each owned by one system: the scene owns
placement; the render system owns what a part is drawn as and the sky.

**Changes are requests.** Whoever wants the scene changed reserves an id and sends requests (`Spawn`,
`Place`, `Show`, `Sky`, `Despawn`) into the scene's inbox. Once a frame, at the publish phase, the
browser applies the inbox in order, routing each request to the system that owns what it changes; a
request naming a part that is gone is refused and counted. The ground and the sky go in this way too:
the ground is an ordinary part whose render row is *anchored*, drawn under the viewer in steps of its
checkerboard's period, so it never ends.
