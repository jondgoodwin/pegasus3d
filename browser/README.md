# browser: Pegasus3D in Cone

A native 3-D browser written in [Cone](https://github.com/jondgoodwin/cone), in the spirit of the
Acorn-era Pegasus3D in `src/`. It is the shell and the walkable ground (an island in the sea, below; in metres, walked on at eye
height, 1.7 m) and a baked-in world standing on it: two chess
rooks, a horn-flower, a horn, a forest cottage (board and batten, a brick chimney up its right eave wall)
with three pathway lanterns along its path, a black-figure amphora and a boulder in front of it, a
hot-air balloon with its burner alight and,
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

## The mannequin

You start as an artist's wooden mannequin, 1.75 m tall, seen from behind and above (V for its eyes),
on the landing beach, facing -z up the path; a second, 1.62 m and clay red, stands idle (breathing, shifting its
weight) a few metres off, turned towards you. Each is a 19-bone skeleton (Cone's `bonepose` package)
under one smooth body (`src/mannequin.cone`): the trunk, neck and head one tube of elliptical rings, each
leg and arm one tube through its knee or elbow, flat-soled feet and mitten hands, lofted along the bones
by Cone's `sculpt` and skinned on the GPU by `render` (linear blend skinning, each vertex's weights
written at build time from where it lies along its bones), so knees, elbows and the waist bend as one
surface. The being's part is shown as the body, posed each step by a `Bones` request of its skinning
matrices. Each step Cone's
`walkgait` writes the pose: a walk whose phase follows the distance travelled, so the feet do not
skate, feet that roll heel, flat and toe, two-bone IK on the legs reaching each ankle to a place on the
ground, the pelvis as high as the legs allow, the arms against the legs, and when standing, breathing
and shifting weight. The body is a kinematic capsule on `groundHeight` (`src/ground.cone`: the island's,
below) with gravity: a landing at 7 m/s or more (a 2.5 m drop) is
HARD, the pelvis crouches, and the scene mails a `HardLanding` to the world; one above 10 m/s hurts the
being's health, to nothing left at 24 m/s.

## The sky and the birds

The sky is the day's, drawn by one custom pipeline (`src/skyshader.slang`, a full-screen draw at the far plane,
`src/skyrig.cone`) over the sky of Cone's `analyticdaylight` package: Preetham, Shirley and Smits's analytic daylight
(coefficients read from the paper, not from memory), a twilight patch for the sun below 6 degrees, a night floor,
the sun's disc, a moon that is always up at night with a phase and seas, and a field of stars that turns with
the hour; with two layers of cloud (a low one 1800 m up, cirrus at 7500 m) drifting on scene time, lit by the
sun's colour so they redden at dusk and by the moon at night. The day is **45 degrees north on 20 May** (so the
clock's dawn at 4:30 and dusk at 19:30 are the sun's centre at the horizon), the sun rising in the east (+x) and
setting in the west, due south (+z) at noon, 64.8 degrees up. Turbidity 2.5.

The scene is lit from the same `DaySky` on the CPU, each frame, from the scene's day clock: the sun's irradiance
(100 000 lux at noon, reddening through the air it crosses), the moon's (0.18 lux at night, the directional light
once the sun's is the less), and the ambient radiance, the sky averaged over the hemisphere (10 000 nits at noon).
The exposure is scripted from the sun's height (`ev100ForSun`, `look.cone`): EV 15 at noon, falling to -1 at night,
where the moon and the ambient floor show the ground dark but readable. `--no-sky` runs with the static sky and
light and no birds, to time what the day costs.

Forty birds (`boidflock`) keep to within 90 m of the cottage (`BIRD_HOME_X/Z`, a parameter), flock by
separation, alignment and cohesion on their 7 nearest neighbours, steer round the ground and the hill (the
flock is asked the ground's height, `ground.cone`), and beat their wings at 7 Hz on the level, up to 13 Hz
climbing and down to 4.5 Hz diving, gliding between bursts. A bird is **one mesh drawn once, instanced** (a body,
head and beak, a forked tail and two wings of ten stations with three primaries each, 0.54 m across): the flap
is done in the vertex stage (`src/birdshader.slang`) from the phase and amplitude the flock gives each instance,
so there is no CPU vertex work and one draw for all of them. The flock steps on scene time (paused with P, x10
with F).

## Glow and glass

**Nothing casts light yet: no point lights.** The lamps glow, glass is see-through, and the light a lamp would
throw on the ground is painted there. All of it is the world's lights (`src/bakedworld/lights.cone`):

- **What glows:** the five lanterns' bulbs and frosted glass chimneys, and a lit room behind every window and
  door pane (`src/interior.slang`: a card 9 mm behind the glass, a lampshade a little left of centre, its warm
  light falling off round it, the dark back of a sofa across the lower third, curtains gathered at both sides).
  Their glow is their material's emissive colour, which the picture's bloom spreads. The lamps are about 2700 K
  (linear (1, 0.40, 0.095)); the rooms a deeper amber (1, 0.25, 0.028), deep enough that AgX does not pale it
  to peach.
- **On at dusk, off at dawn:** the world reads the scene's `Dusk` and `Dawn` mail and fades the lamps over
  15 s of scene time (a quarter of an hour of the day). The fade is counted from the time the message was for,
  so a clock set past dusk finds them already on.
- **How bright follows the clock,** as the scripted exposure does: a bulb gives off about 2800 nits at dusk,
  falling to 1.3 nits from 22:00 (`nitsAt`).
  - The exposure rises about 5000-fold over the same hours.
  - The bulb is about 1.2 of the display's white at 19:30, 2.7 at 19:50 and 3 at night: brighter as the night
    deepens, as the eye adapting sees a lamp.
  - Every other glow is a share of the bulb's: the glass 0.22, a room 0.4, a lantern's pool 0.04, a window's
    pool 0.05.
- **Nearness:** each path lantern's stake is an `Area` 4 m round. The possessed being coming into it brightens
  that lantern's glass, bulb and pool 1.45 times over 0.4 s (a `Tint` on the three parts), and going out dims
  them back. Only lit lanterns brighten.
- **Pools of light** (`src/lightpool.slang`) are see-through, added light: a grid lying on the ground, its
  vertices at `groundHeight` plus 3 cm, so it follows the terrain. They are soft from the middle out,
  (1 - d^2)^2 times a bright core, with no rim. There is one under each path lantern (1.3 m round); one under
  each wall lantern and before each lit porch window and the door, on the deck; and one on the ground before
  the wing's windows.
- **Glass:** the window and door panes are physically based and see-through (alpha 0.1, a faint grey-green, so
  they reflect the sky at a glancing angle). The lanterns' chimneys are frosted (alpha 0.45), so the bulb shows
  through as a paler oval.

The requests are `Glow` (a material's emissive colour, or a shaded look's first three parameters) and `Tint` (a
part's colour and glow multiplied). A `Look` is see-through with `.translucent(alpha)` and cut out with
`.cutOut(cutoff)`, through render's alpha modes.

## Keys

| Key | Does |
| --- | --- |
| W or Up | walk forward (the mannequin 1.8 m/s; the plain walker of a scripted run 3 m/s) |
| S or Down | walk back |
| A or Left | turn left (a quarter turn in 1.5 s) |
| D or Right | turn right |
| Shift (held) | run (the mannequin 4.5 m/s; the plain walker four times as fast) |
| Space | jump (a mannequin: 4.2 m/s up, 0.9 m) |
| **V** | **first person or third person**: the camera at the mannequin's eyes socket, or 3.2 m behind it and above, looking at its chest |
| **Tab** | possess the next mannequin: the controls and the camera go to it, and the one left behind stands |
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
browser.exe --checks                  the scene core's own checks, on scenes of their own (below), and the sky's
browser.exe --no-sky                  the static sky and light, no birds (to time what the day costs)
browser.exe --frames 600              600 frames on frame's synthetic clock
browser.exe --soak 12000              12000 frames, checking nothing grows (below)
browser.exe --watch 300               300 s in real time, a line a second and a key probe (below)
```

`--script` pushes key events into SDL's own queue (never the operating system's), so nothing reaches
any other window, and a run of a fixed number of frames ignores the real keyboard and mouse (and the
window losing focus, which would otherwise let go of the script's keys). It starts on the island's landing
beach and runs 12 m up the path (on the ground, inside the fence, checked), is set down at the old flat
world's start on the lowland, walks up to the rooks and the horn-flower, clicks on the rook and then on the sky, turns, runs to the balloon, clicks its
launch button, then draws eight close-ups from eyes of its own (the amphora's through a 20 degree lens,
straight on, as its photograph shows it; the cottage's from the reference photograph's viewpoint and
lens, and from the walker's eye on the approach). Its clicks are pushed mouse events at
the place the script works out the target is drawn. It checks the walk against what its events must
give (exact on the synthetic clock); what each click picked and highlighted; that the world heard the
button and the balloon (every one of its parts) rose as its formula says; the ball's height against
its formula; every burst of the burner sent was applied; that the scene applied, refused and removed
what it was sent; and that every part the world sent is live, placed and (with a GPU) drawn or giving
off particles, with every request sent accounted for as applied or refused; and that the amphora and
the chimney stack are the sizes they were built to (the amphora 1.484 m to its tip, 0.3368 m at its
widest; the stack 47 courses, 3.525 m, and 5 x 4 bricks at its top course) and drawn physically based;
and that the cottage is as measured: its door 2.0 m by 0.8 m with its floor 0.6 m up, its ridge at 7.0 m
and its wing's far wall 5.3 m left of the door, its chimney stack standing on its shaft at the eave
against the right eave wall, all 26 of its parts one tree under its plinth, the five lanterns' glass,
bulbs and metal one shared material each, and its panes drawn as parts of their own.
It also presses P (frames 256 and 271) and F (276 and 286) while the walker stands, and checks that
scene time stood still while paused and ran ten times as fast at x10; that the world's alarm (a timer
for 3 s) rang at the first frame at or after 3 s; that the world was told the walker came up to the
launch button (an area) and is still there; the day clock's reading; that the balloon is one tree as
built; and that the two rooks share one material (11 more meshes than materials defined: the rooks'
one, the cottage's 22 meshes in 16 materials, the lanterns' 4 meshes in the cottage's 3; each
mannequin is one skinned mesh in one material; and, with a GPU, both rooks drawn in the same material).
The scripted run starts as the plain walker (so its walk checks stay exact) with the two mannequins
standing off to the right; from frame 660 it presses Tab (the first mannequin is possessed, in third
person), walks it with W for 60 steps, presses V (first person, the camera at its eyes socket) and
back, presses Tab again, and lifts the second 6 m into the air to fall and land hard: it checks the gait's
speed against the intent, the camera's place in each view, the possession, and that the hard landing
was mailed and read by the world and hurt by (speed - 10) / 14. It saves `mannequin-third`,
`mannequin-side` and `mannequin-side2` (a side view mid-stride), `mannequin-first`, `mannequin-second`,
`mannequin-falling` and `mannequin-landing`. Last, from frame 940, it looks at the cottage from the
reference photograph's place and from the walker's eye on the approach.
After that (frames 960 to 1130, `src/skyscript.cone`) it sets the day clock to a series of hours, forward from
noon to the next morning, and saves the sky from beside the cottage in windows of ten frames: `sky-noon`, four
birds from beside, above and ahead (`sky-bird-1` to `-4`), the flock from the ground (`sky-flock`),
`sky-afternoon`, the sunset (`sky-sunset-cottage`, `sky-sunset-sun`), dusk (`sky-dusk-cottage`, `sky-dusk-west`,
`sky-deepdusk`), night (`sky-night-cottage`, `sky-night-moon`, `sky-night-stars`) and dawn (`sky-dawn`,
`sky-morning-cottage`).
It exits 0 only if every check passed, no Vulkan call failed and the validation layer said nothing.

`--checks` runs the scene core's checks on scenes of their own, with no window, GPU or frames: a
three-level placement tree made children first, its world matrices against ones worked out by hand, a
placement changed recomputing only its own row, a reparent to the root and back under the top, a cycle
refused, and the top despawned with its subtree; timers on the scene's clock in 1/60 s steps at x1,
paused and at x10, each rung at the first step at or after its time, a cancel, and dusk at 750 s and
dawn at 1290 s; a sphere area and a box area under a scaled parent entered and left, and a second
being coming in moving the first out; and possession, the user's intents moving only the possessed
being and the camera following it; and the mannequin: a capsule dropped from 0.5, 3.5 and 8 m landing
at v = sqrt(2 g h), the hard landings mailed once each and the hurt (nothing to 10 m/s, 0.18 at
12.5 m/s) taken from health, a 6 s walk whose gait speed follows the intent and whose planted foot does
not move more than 1 cm, its skinned sole (skinned by render's CPU twin of the GPU's skinning) within
1 cm of where the planted sole socket carries it, not sliding and on the ground, standing with both feet
flat and level, the third-person camera 3.2 m behind
and the first-person one at the eyes socket, and Tab possessing the other mannequin with the camera and
the controls following it; and the sky (`src/skychecks.cone`): the sun at the clock's noon (64.8 degrees,
due south), dusk and dawn (-0.84 degrees, its centre at the horizon), the sun below the horizon and the
moon above it every quarter hour from 20:00 to 4:30, the light (noon: 100 000 lux and a few thousand nits of
ambient; midnight: the moon's 0.18 lux, never black), the exposure (EV 15 at noon, -1 at night, only falling
through the evening) and forty birds staying over the cottage. It exits 0 only if every check passed.
`shots/` holds the views of one run (`start`, `picked`, `walked`, `turned`, `balloon`, and the close-ups
`rooks`, `chimney`, `starship`, `rising`, `amphora`, `hornflower`, the mannequins' `mannequin-*`,
`cottage-photo`, `cottage-approach`), converted to PNG, `amphora-vs-photo.png`, the amphora's close-up
beside the photograph it was traced from, and `cottage-vs-photo.png`, the cottage from the photograph's
viewpoint beside the photograph.

After the sky's views, from frame 1170 (`src/glowscript.cone`), the clock is set to the next evening and night
for the lights:
- `glow-dusk-approach`, `glow-night-approach`: the cottage at 19:50 and at 23:00;
- `glow-glass-dusk`, `glow-glass-night`: the porch's right bay window close up;
- `glow-dusk-photo`, and `glow-dusk-vs-photo.png` beside the reference photograph: the photograph's view at
  19:50;
- `glow-lantern-night`: the nearest path lantern close up. The walker was set down within its area two windows
  before, so the lantern is brightened.

At frame 1222 the lights are checked: on, the bulb at the hour's brightness, the lantern beside the walker
brightened 1.45 times, and the next one not. `--checks` runs the same lights on a scene of their own: off by day
(and not brightened by nearness), half way through the fade at half its time, at the hour's brightness after
it, brightened on coming near at night and dimmed on leaving, and off after dawn.

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
| `src/shaders.cone` | the browser's own shaders, which a shaded Look names: the heightfield (`heightfield.slang`), a pool of light (`lightpool.slang`, see-through) and a lit room behind a window (`interior.slang`); after editing a `.slang`, run Cone's `tools/shaders/shaders.py src` to compile and embed it |
| `src/shell.cone` | what the browser puts in every scene: the island's land and sea and the sky, sent as requests |
| `src/island/island.cone` | the module `island`: the baked heightfield, its path, fence, height query and meshes (below) |
| `src/terrain.slang`, `src/water.slang`, `src/islandshaders.cone` | the island's two shaders and the custom pipelines made from them |
| `src/being.cone` | beings and possession: intents, the user's controller, the view rig (first and third person), the Cast |
| `src/mannequin.cone` | the mannequin: its figure (skeleton, pose, gait, kinematic capsule), its meshes, one a bone, and spawning it |
| `src/ground.cone` | the ground's height under a point: the island's, the one function everything standing on the ground asks |
| `src/walk.cone` | a walker's body on the ground, and the user's key bindings |
| `src/script.cone` | the scripted run, its close-ups and its checks |
| `src/checks.cone` | the scene core's headless checks (`--checks`) |
| `src/picking.cone` | the picking system: each mesh's triangles, which mesh each part is, and the ray a click casts |
| `src/watch.cone` | the watched real-time run and the soak |
| `src/bakedworld/` | the module `bakedworld`, the baked-in world: its objects' descriptions, hydrating and injecting them, its motions (`motions.cone`), and its lights (`lights.cone`: glow on at dusk and off at dawn, nearness, pools of light) |
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
| `Glow` | a material gives off a radiance (nits) from now on: a lamp switched on, dimmed or off |
| `Tint` | a part's colour and glow multiplied by a colour from now on: one lamp of many brightened |

Once a frame, at the publish phase, the browser applies the inbox in order, routing each request to the
system that owns what it changes; a request naming a part that is gone is refused and counted. The
land, the sea and the sky go in this way too (an *anchored* part, a non-zero `anchor` in `Show`, is drawn
under the viewer in steps of that many metres; a negative one is placed where it is but not pickable, the
island's). A material may carry a
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
| cottage (26 parts: plinth, walls, battens, roofs, trim, window frames and panes, door and its panes, porch deck, skirt, posts, roof and steps, chimney shaft, stack, flaunching and pot, two wall lanterns) | new (`cottage.cone`, `chimney.cone`): measured from the reference frame with the door as the ruler (below); boxes, prisms and slabs; board, shingle, deck and flagstone maps from the `surfacepatterns` texture kit; the chimney is the stack of step 4 on a brick shaft from the ground, 3.3 m + 4 m | 7 m to the ridge, 7.3 m to the pot | 5,044 | 870 (the texture kit's four bakes 610, the chimney's maps 260) |
| path lanterns (three: stake, glass, bulb, cap each) | new (`cottage.cone`): four lathes and a sphere in three shared materials | 0.5 m | 2,892 | 0.24 |
| hot-air balloon (envelope, skirt, burner frame, cables, basket, passengers) | `sculpt/examples/balloon.cone`, the chevrons | 25 m | 74,264 | 52 |
| burner flame (core, tongue, embers) and pilot light | `vfx/examples/burner.cone`, live, fired in bursts | 3 m | particles | (in the balloon's) |
| launch button (pedestal, cap) | new: a box and a lathed disc; the cap listens for clicks | 1 m | 140 | 0.06 |
| ball | new: a sphere, checkered so its spin shows | 1 m across | 1,984 | 0.03 |
| mound | new: a 65 x 65 grid lifted on the GPU by a heightfield (the heightfield shader, a custom pipeline), green rising to a faintly glowing sandy hump | 4 m square | 8,192 | 0.14 |
| starship (frame, membranes, wing lights, ports, eyes, mouth) | `packages/starship`, meshed on the GPU at 0.03 a cell | 156 m, 10 m a unit | 458,324 | 700 (0.4 s of it making the kernels) |

## The island

An island about 1.3 km across (x east, z south, sea level y = 0, its middle near the origin) in water, baked
once on the CPU at start-up (about 0.6 s, `src/island/island.cone`; `browser.exe --island FILE.bmp` writes a
picture of it, the fence and the path). Inlets and peninsulas round the coast (a smoothed signed distance of a
body, two peninsulas and a cove, warped and roughened by noise near the coast only) rise to a **150 m** hill
in the middle, its summit and ridges ridged noise and its flanks **eroded** by a per-point filter on the
hill alone (after runevision's erosion filter, built on `noise`'s phacelle: four octaves of gullies from 140 m
down to 17 m, masked to steep ground, faded at the summit and foot, each octave steered by the slope the
last left). The cove on the south shore has the landing beach at its head (the start: (-22, 499), facing
north); 170 m inland, on a flat pad, stands the cottage (its front wall's middle at (-16, 330)), the hill
rising behind it, the old flat world shifted rigidly onto the lowland with it (`WORLD_DX`, `WORLD_DZ`), every
object standing on the ground there. The lowland rolls (the farther from the pad, the more, up to about 3 m
over 70 m) so there is up and down to practise on.

**One grid.** The land is N = 1025 samples a side, 2 m apart, covering 2048 m (4.2 MB, one r32float texture).
The GPU draws it with the same numbers: a fine mesh with a vertex a sample over x from -400 to 400 and z from
-64 to 704 (the walkable country and the hill's foot; 307 000 triangles) and a coarse one with a vertex every
4th sample over the whole grid, with a hole where the fine one is (113 000), the fine mesh's border vertices
set on the line between the coarse ones, so they meet with no crack. No levels of detail beyond these two;
the fence keeps you in the fine one. `groundHeight` (`island.cone`) is **triangle-exact**: the quads are split
by one rule (`diagonalFlipped`), the index buffers use it, and the query finds the triangle the mesh has
(`--checks`: 4000 points against the index buffer, within 0.4 mm; plain bilinear would be 0.5 m off).

**The path** is a Catmull-Rom curve from the beach (heading north for its first 22 m) to the cottage's steps,
205 points a metre apart (`pathCentreline`), its height smoothed along it and pressed into the ground in a
ribbon (the steepest metre 9 degrees); the distance to it is a second field the ground shader reads.
**The fence** is a polygon round the beach, the path, the forest and the lawn (`constrain`, called by every
walk and mannequin step, which slide along it) and a wading limit of 0.6 m. **The forest** (`forestDensity`)
is where trees may stand: above the beach, below 85 m, clear of the path and of 34 to 58 m round the lawn.

**The ground** (`src/terrain.slang`) is coloured per fragment from procedural noise: sand by the water (wetter
where the swash reaches, rippled), grass in two greens with blades, rock on the steep slopes (triplanar
noise with strata), forest litter from a painted mask (the one image: red the forest's density) and the path's packed
earth with pebbles and an uneven edge, blended by height, slope, the mask and the path's distance field.
**The sea** (`src/water.slang`) is a flat grid and a ring to 8 km moved by five Gerstner waves on scene time
(Finch, GPU Gems 1 ch. 1: L = 60, 34, 14, 8, 4.5 m, A = 0.16 to 0.014 m, Q = 0.8, w = sqrt(g k), their heights
and steepness shrinking over shallows; the two longest move the 8 m grid's vertices, all five the normal,
fading with distance so nothing shimmers), tinted by the depth of the same heightfield (Beer-Lambert: red goes
first) over a sand bed, with Schlick Fresnel reflection of the frame's sky, a sun glint, ripples, and foam in
a band that comes and goes along the shoreline.

## The cottage, measured from the reference frame

The frame is `forest-cottage-dusk.jpg` (640 x 1136; `C:\src\reference-images\cottage`). The ruler is its
front door, taken as 2.0 m high (a standard door; 0.85 m wide is not what the frame shows, it reads 0.70 m, so
the model's leaf is 0.8 m): 137 px (612 to 749), 68.5 px a metre at the front wall. Pixels were read off
enlarged, gridded crops of the frame. Each feature was then back-projected onto the plane it stands in (the
front wall, z = 0; the posts, z = 1.45; the porch roof's edge, z = 1.9) through the camera that least squares
fits the planar features (a fit of the front-wall points, the horizon read from two receding lines at row 679,
a 55 degree vertical lens assumed): **1.63 m up, 15.3 m from the front wall, 4.25 m left of the door, turned 12
degrees to the right, tilted 5.8 degrees up**, 3.8 px rms. The cottage's frame: the ground at the middle of the
main front wall, +x right, +z towards the viewer. The porch floor is the door's sill, 0.63 m measured, 0.6 built.

| Feature | Pixels (x; y) | Metres (x; y above the ground) | Built |
| --- | --- | --- | --- |
| Door, the ruler | 360 to 408; 612 to 749 | -0.41 to 0.31; floor 0.63 to 2.6 (2.0 high by 0.72) | -0.4 to 0.4; 0.6 to 2.6 |
| Gable's apex | 400; 318 | 0.2; 7.0 | 0; 7.0 |
| Gable's feet (the eaves' ends) | 203; 565 and 560; 567 | -2.68; 3.23 and 2.72; 3.31 | -2.7 and 2.7; 3.3 |
| Gable's tall window | 358 to 412; 442 to 530 | -0.43 to 0.4; 3.8 to 5.11 (0.83 by 1.31) | -0.42 to 0.4; 3.8 to 5.11 |
| Attic opening | 377 to 398; 385 to 415 | -0.13 to 0.19; 5.52 to 5.98 | the same |
| Left porch bay window | 268 to 308; 610 to 697 | -1.75 to -1.17; 1.37 to 2.61 | the same |
| Right porch bay window | 461 to 502; 616 to 697 | 1.13 to 1.76; 1.36 to 2.56 | 1.15 to 1.77; 1.37 to 2.61 |
| Wing's three tall windows | 125 to 192; 610 to 695 | -3.74 to -2.82; 1.41 to 2.58 | -3.74 to -2.82; 1.41 to 2.58 |
| Wing's narrow corner window | 50 to 62; 617 to 677 | -4.75 to -4.59; 1.65 to 2.47 | the same |
| Dormer's window | 87 to 106; 465 to 525 | -4.3 to -4.04; 3.79 to 4.65 (face 0.3 m behind the wall) | the same |
| Dormer's roof top | 100; 390 | -4.15; 5.75 | 5.4 (see below) |
| Wing's eave | 60; 562 and 200; 572 | -4.64; 3.23 and -2.72; 3.13 | 3.3 |
| Porch's left post | 326; 600 to 750 | -1.22 (z = 1.45) | -1.25 |
| Light column right of the right bay | 503 to 515; 610 to 690 | 1.25 (z = 1.45, by symmetry) | a post at 1.25 |
| Pilaster beside the door | 412 to 428; 600 to 745 | 0.5 to 0.6 (z = 0) | 0.47 to 0.59, and its mirror |
| Porch's right end post | 606; 650 | 2.71 (z = 1.45) | 2.6 |
| Porch roof's ends, its front edge | 245 and 612; 583 and 594 | -2.35 and 2.59 (z = 1.9) | -2.5 to 2.8; edge 2.9 up |
| Deck's right end | 612; 768 | 2.67 | 2.7 |
| Steps | 387 to 477; 780 to 797 | -0.47 to 0.52 (z = 2.0); 0.3 | -0.55 to 0.65; two risers of 0.2 |
| Wall lanterns | 337; 640 and 445; 645 | -0.78 and 0.83; 2.15 | -0.78 and 0.83 (bracket at 2.4) |
| Eave to ridge, the gable's rise | 247 to 254 px | 3.6 to 3.7 | 3.7 (53.9 degrees) |

Left of the door the frame shows one steep slate slope with a small dormer in it, a low eave, and three
tall windows and a narrow one under the eave. This is built as a **wing** with its own ridge running across
(7.0 m up on the main block, 6.5 m on the wing), the dormer in the wing's front slope, so it reads as the frame
does from the front; the earlier note in `WI\other-exemplars.md` read it as the main roof's left slope running
on down (a catslide). The two cannot be told apart from this one view; the wing was chosen because the slope's top
edge in the frame runs across, not back.

**What is real geometry and what is material.** The battens are strips (5 cm wide, 2.5 cm proud, every 0.3 m on
the front walls, the wing's end and the main block's right and left walls, cut round the door and windows);
the boards between them are the wall material's maps (surfacepatterns' siding, as plain boards 0.3 m wide, the
tile's seams slid to lie under the battens). The shingles are the roof material's maps (surfacepatterns' cedar,
courses 14 cm deep, tinted slate grey) on slabs 12 cm thick, not rows of geometry. The windows are real: a casing, a sash with muntins, and a pane
that is a part of its own (all the windows' panes are one part, the door's another). Every surface kind is one
material defined in `ctMaterials`, shared by every part of that kind: board, batten, shingle, trim, window
frame, pane, door, timber, deck and stone. Four of them are baked from the `surfacepatterns` texture kit (a
physically based material with colour, normal and ORM maps) in `ctBakeBoards` (siding, 512 x 512 over 1.8 m),
`ctBakeShingles` (cedar, 256 over 1.12 m), `ctBakeDeck` (decking, 256 over 1.68 m) and `ctBakeStone` (garden
flagstones, 512 over 2 m); the meshes' uvs are metres over each tile's side. The four bakes take about 0.6 s of
the cottage's hydration. The renderer gives every texture its whole mip chain and samples it trilinearly and
anisotropically, so the roof and deck no longer shimmer as the camera moves (a 1 cm shift of the
camera changes the roof's pixels by 8.0 grey levels on average before, 1.9 to 2.3 after, on the
two GPUs; the deck's, 6.3 before, 1.7 to 2.3 after; the mips cost about 0.1 s on the first frame the
world is published). The brick chimney is `chimney.cone`'s stack, unchanged, standing on a shaft of 44
more courses of the same bond from the ground to the eave, on the right eave wall; its pot is 0.3 m above the
ridge. The three path lanterns and the two wall lanterns are one family (stake or bracket, glass, bulb, cap), the
glass and the bulb each its own part, in three materials the world shares.
