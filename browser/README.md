# browser: Pegasus3D in Cone

A native 3-D browser written in [Cone](https://github.com/jondgoodwin/cone), in the spirit of the
Acorn-era Pegasus3D in `src/`. It is the shell and the walkable ground (an island in the sea, below; in metres, walked on at eye
height, 1.7 m) and a baked-in world standing on it, assembled as one scene: you land on the beach and
a path (about 200 m, snaking through a forest of red maples and white pines, rocks, edging stones, hostas,
ferns and flowers, below) leads to a forest cottage (board and batten, a brick chimney up its right
eave wall) with twelve pathway lanterns in six pairs facing across its last 16 m and a black-figure amphora beside it
with the horn-flower; on its porch a lathed chess rook and two clay pots of flowers stand by the door; a ball
lies on the lawn where it was left; and behind the cottage, in the backyard, a hot-air balloon with its
burner alight waits, reached round the cottage's east end, its basket open at the front to walk into.
Things move: the burner fires in bursts, and the launch button in the basket sends the balloon on a
2 2/3-minute ride up round the island and back, carrying whoever stands in the basket. A click
picks what it lands on and highlights it. (Elizabeth's skeletal dragon starship, built from Cone's
sculpt, vfx, sdf and sdfmesh packages, hovered outside the fence until the scene was assembled; its
code stays in `src/dragonship/` and `gpu/` for a space scene, and nothing places it now.)

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
below) or a platform's floor (the balloon's basket, `src/support.cone`) with gravity: a landing at 7 m/s or more (a 2.5 m drop) is
HARD, the pelvis crouches, and the scene mails a `HardLanding` to the world; one above 10 m/s hurts the
being's health, to nothing left at 24 m/s.

## The sky and the birds

The sky is the day's, drawn by one custom pipeline (`src/skyshader.slang`, a full-screen draw at the far plane,
`src/skyrig.cone`) over the sky of Cone's `analyticdaylight` package: Preetham, Shirley and Smits's analytic daylight
(coefficients read from the paper, not from memory), a twilight patch for the sun below 6 degrees, a night floor,
the sun's disc, a moon that is always up at night with a phase and seas, and a field of stars that turns with
the hour; with two layers of cloud (a low one 1800 m up, cirrus at 7500 m) carried east (+x) by the world's wind
(7 m/s at the low layer, 11 m/s at the cirrus, so the layers slide past each other) while each octave of the cloud's
noise turns slowly on a small circle of its own, so the clouds grow, thin and break up as they go (both closed forms in
scene time, so a paused or scrubbed clock puts them where they were), lit by the
sun's colour so they redden at dusk and by the moon at night. The wind is one value for the world
(`src/parts/weather.cone`): the clouds, the chimney's smoke and the snow lean on it.

**The weather** (`src/parts/weather.cone`, press **T**) is six states: clear, fair, cloudy, overcast, rain and snow,
each a set of numbers (the two layers' cover, the clouds' density and light, how grey and how bright the sky is, how
much of the sun gets through, the fog's thickness, the ambient light's scale and tint, the exposure's offset in stops,
the rain's and the snow's strength). T crossfades to the next state over 12 s of scene time: the simulation keeps the
blend it left, the state it goes to and when it began; the view draws from the closed form, a smoothstep of scene
time. **Fair** is the look the browser had before there was weather. The sky shader takes the covers, the grey and
dimming, and a horizon haze; the render actor dims the sun, scales and tints the ambient and opens the exposure; the
fog is a stack of 48 see-through quads facing the camera, 8 m to 2.4 km away (`src/fogshader.slang`, drawn
alpha-blended with the depth test, so each veils only what lies beyond it, and none are drawn in clear or fair
weather); rain and snow are particle fields (the `vfx` smoke style) in a disc over the eye, so they move with the camera
and fall indoors too. Not done: wet ground and snow cover.

The day is **45 degrees north on 15 October** (day 288, autumn, to match the maples; so the
clock's dawn at 6:31 and dusk at 17:28 are the sun's centre at the horizon and the night is 13 of the 24 minutes), the sun rising in the east (+x) and
setting in the west, due south (+z) at noon, 36.0 degrees up. Turbidity 2.5.

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
head and beak, a forked tail and two wings of ten stations with three primaries each, `BIRD_SCALE` 2.2, so about 1.08 m across, dark grey-brown above and below so a flock reads as a silhouette against the sky): the flap
is done in the vertex stage (`src/birdshader.slang`) from the phase and amplitude the flock gives each instance,
so there is no CPU vertex work and one draw for all of them. The flock steps on scene time (paused with P, x10
with F).

## Glow and glass

**Nothing casts light yet: no point lights.** The lamps glow, glass is see-through, and the light a lamp would
throw on the ground is painted there. All of it is the world's lights (`src/bakedworld/lights.cone`):

- **What glows:** the fourteen lanterns' bulbs and clear glass panes (square lanterns, each lit from its bulb), and a lit room behind every window and
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
  - The bulb is about 1.2 of the display's white at 17:28, 2.7 at 17:48 and 3 at night: brighter as the night
    deepens, as the eye adapting sees a lamp.
  - Every other glow is a share of the bulb's: the glass 0.015, a room 0.4, a lantern's pool 0.04, a window's
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
  they reflect the sky at a glancing angle). The lanterns' panes are clear (alpha 0.2), so the bulb shows
  through them, and the dark frame and arched tracery read as silhouettes against its glow.

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
| PageUp / PageDown, or drag with the right mouse button held | look up / down: the camera's pitch (first and third person, and the plain walker), held within 80 degrees of level; the walking direction does not change; it starts level and stays where you leave it (mouse yaw is not bound: A/D or Left/Right turn) |
| **V** | **first person or third person**: the camera at the mannequin's eyes socket, or 3.2 m behind it and above, looking at its chest |
| **Tab** | possess the next mannequin (never a villager): the controls and the camera go to it, and the one left behind stands |
| Left click | pick what is under the pointer: it is named on the console and highlighted; the launch button in the basket sends the balloon on its ride |
| P | pause scene time, or start it again (the world's motions, the effects, the timers and the day stop; the walk does not) |
| F | run scene time ten times as fast, or back to normal (for testing the day: dusk comes 50 s after the start instead of 12.5 min) |
| T | the weather changes to the next of six states (clear, fair, cloudy, overcast, rain, snow), crossfading over 12 s of scene time |
| N | mute the sound, or un-mute it (the master gain fades over 80 ms) |
| H | hide the HUD, or show it again (below) |
| B | bowl the ball at the spot the camera looks at (the playground, below) |
| F1 | throw paint where the camera looks (30 m; trees, rocks, walls and the balloon's basket stop it) |
| F2 / F3 | zap / make dance the mannequin in front of the camera (the nearest to its middle within 22 m; locked on, and highlighted, for 1.5 s after) |
| F4 to F8 | hotbar slots with nothing in them yet |
| F11 | fullscreen |
| Escape | quit |

## Sound

The browser has sound (`src/audioview.cone`): Cone's `audiomixer` (a mixer in Cone: voices, buses, 3-D placement, procedural
recipes after Andy Farnell's *Designing Sound*) sent to the speakers by `audiodevice` (an SDL3 audio push stream fed by a Cone
thread). Nothing is a file; every sound is a recipe. It is the view's, never the simulation's: it reads what the view has (the
camera, the beings' feet, the weather, the world's burner bursts) and sends nothing back, so a server has none.

| Sound | How |
| --- | --- |
| wind | not placed: a breeze and a gale recipe crossfaded at constant power by the wind's strength, which is a function of the weather's numbers (clear 0.13 ... rain 0.87) so it follows the weather's own crossfade, and which rises with height over the land |
| the brook | placed along its course: the closest point of the course to the listener carries a trickle and a rush recipe, crossfaded by the water's speed there; the three nearest bed stones, the arch under the bridge and the cascade in the dingle are louder emitters of their own |
| the balloon's burner | a fire recipe at the burner's part, on while the world's burst lasts (the Effect and Fire requests the view already applies), following the balloon |
| footsteps | one step recipe each time a foot comes down (the walker's foot goes from swinging to planted), at the heel, by what is underfoot: grass, stone (rocks, edging stones), wood (the porch, the basket) or the bridge (stone with a hollow note under it) |

The listener is the player's head turned with the camera. A placed sound beyond its range plays no voice. Interactive runs use the
default playback device; every automated run (`--script`, `--checks`, `--soak`, `--audio-walk`) and any machine without a device
renders the mixer on the game thread, a fixed 48000 / 60 frames to each fixed step, so it is the same every time and needs no
device. Options: `--no-audio`; `--audio-device` (use the device even in an automated run, e.g. with `SDL_AUDIODRIVER=dummy`);
`--audio-muted` (open the device but start muted); `--audio-wav FILE` (record the offline mix); `--audio-walk FILE` (the
scripted walk of `src/audiowalk.cone`, past the brook and over the bridge, recorded to FILE and described by its spectrum;
add `--headless` for no window).

## The HUD

An overlay drawn over the finished picture, after tone mapping (`src/hud.cone`, `src/hudshader.slang`): a
**status panel** at the top left (the day clock and the weather, the frame time and frames a second, single
player or connected with the round trip), a **minimap** at the top right (the island from above, 2 m a texel,
painted at start-up from the height grid, the path, the brook and the trees; north is up, it shows 340 m
round the player; the player is the white dot, a small yellow tick 16 m ahead of it shows the heading, the
other characters are red dots) with the player's x and z under it, and a **hotbar** of eight slots at the
bottom, bound to F1 to F8. The layout and the draw list are Cone's `uilayout` (with its hotbar and minimap
widgets), the text is `msdftext`'s signed-distance-field text (sharp at any size), the shapes (rounded boxes, borders,
arcs, lines and the map's image) are one instanced pipeline, `hudshader.slang`. The HUD only presents: the view
actor hands the render actor, which owns the GPU and draws it, a small record each frame (the clock, the weather,
the frame time, where the beings are, which F-keys were pressed); no game state is in it. Clicks do not reach it
yet: they go to the world's picking.

It is laid out in logical units and drawn at the display's scale (`PEGASUS_UI_SCALE=1.5` to force 150 %), with
the text laid out at the pixel size. The font is Inter (Regular, and SemiBold which no label uses yet), from
`fonts/` (Rasmus Andersson and the Inter Project Authors, https://github.com/rsms/inter, version 4.1; SIL Open Font
License 1.1, in `fonts/Inter-LICENSE.txt`). The program looks for `fonts/Inter-Regular.ttf` beside the executable,
two folders above it (the source tree, where the executable is `build\release\browser.exe`) and in the working
folder; where none can be read it falls back to Segoe UI, or Arial, or Cascadia Mono from the system's font
folder, and draws no text where none can be read. `PEGASUS_HUD=off` starts it hidden (to time the
picture without it). A scripted run starts with it hidden, so its other views are as they were, and turns it on in
its last part (`src/hudscript.cone`); `browser.exe --hud-only --shots DIR` runs only that part, for looking at it.

## The mirror

A framed mirror hangs on the cottage's back wall, towards the chimney's end (1.0 m wide, 1.7 m tall, 0.55 m up: a
standing figure shows head to foot, the sky and the balloon behind the viewer show above it). It is the first **view
surface** (`src/viewsurface.cone`): a surface that shows the scene drawn again from another camera. The piece is three
steps and only the first is the mirror's: which camera (`cameraThrough`: the viewer's camera reflected about the surface's
plane, with an oblique near plane so nothing behind the wall is drawn: render's `Camera.reflectedAbout`), draw the scene
again from it into the surface's own texture (render's `RenderView`, drawn before the frame in its own submission, on
the layers the surface is not on), and show the picture (render's `ViewMaterials`). A portal would add a kind to
`cameraThrough`. A surface is a part whose look names `SHADER_MIRROR`; the part's world matrix places it (its mesh is
mesh's plane, its normal the matrix's +y), so the simulation places it like any part and the view only draws what it shows.
The picture is drawn only while the surface is in front of the eye and in the frustum, at the frame's size (remade when the
window changes). Temporal AA: the picture is jittered with the frame's own sequence (opposite in x, the picture being
mirrored), so a still mirror is anti-aliased; while the camera moves, the history is reprojected by the wall's motion and
the picture's edges soften (render treats a custom pipeline as static). Not in the reflection: the particles (the burner's
flame, smoke, rain), and the froxel fog (a pass over the main frame's depth). Shadows are right (the picture is drawn with
its own cascades; the cottage's shadow edge falls in the pane where it should). `--mirror-only --shots DIR` runs the
mirror's part of the script (`src/mirrorscript.cone`); `--mirror-only --soak N --mirror-hold 1|2` holds the eyes on the
mirror or turned away, for timing; `PEGASUS_MIRROR_TIMING=1` times each picture on the GPU. `--checks` works the
geometry out by hand (`src/mirrorchecks.cone`: the plane, the facing, the reflected eye, that points in front of the mirror
are drawn where the mirror shows them, the frustum test, the pane's size).

Cost (RTX 4060, soak, mirror in sight against turned away): frame p50 20.8 ms against 13.9 (vsync-bound; render phase
9 against 4.8 ms), the picture 3.6 ms on the GPU and 4 ms of CPU recording (76 draws, 0.9 M triangles). Intel UHD 770: 67 ms
against 55 ms (the picture 51 ms on the GPU in the timed run, which waits idle after it).

## Automated runs

```
browser.exe --script                  walk and turn with no one at the keyboard, checked
browser.exe --script --shots shots    the same, saving its views as BMPs into shots/
browser.exe --headless --script       the same with no window: the scene, the world and the walk
browser.exe --no-vegetation           the same without the trees, rocks and plants (to time what they cost)
browser.exe --checks                  the scene core's own checks, on scenes of their own (below), the sky's and the vegetation's layout
browser.exe --no-sky                  the static sky and light, no birds (to time what the day costs)
browser.exe --frames 600              600 frames on frame's synthetic clock
browser.exe --soak 12000              12000 frames, checking nothing grows (below)
browser.exe --watch 300               300 s in real time, a line a second and a key probe (below)
browser.exe --navpic PREFIX           the villagers' navigation mesh drawn top-down as PREFIX-region.bmp, -cottage.bmp and -bridge.bmp
```

`--script` pushes key events into SDL's own queue (never the operating system's), so nothing reaches
any other window, and a run of a fixed number of frames ignores the real keyboard and mouse (and the
window losing focus, which would otherwise let go of the script's keys). It starts on the island's landing
beach and runs 12 m up the path (on the ground, above the wading limit, checked), is set down on the lawn
10 m in front of the porch, clicks on the lathed rook on the porch deck and then on the sky, turns, runs a
few metres, then draws eight close-ups from eyes of its own (the porch's rook and pots, the chimney,
the ball on the lawn, the balloon from the lawn, the amphora's through a 20 degree lens,
straight on, as its photograph shows it; the horn-flower; and, later, the cottage's from the reference photograph's viewpoint and
lens, and from the walker's eye on the approach). Its clicks are pushed mouse events at
the place the script works out the target is drawn. It checks the walk against what its events must
give (exact on the synthetic clock); what each click picked and highlighted; that the ball lies still
where the table puts it; every burst of the burner sent was applied; that the scene applied, refused and removed
what it was sent; and that every part the world sent is live, placed and (with a GPU) drawn or giving
off particles, with every request sent accounted for as applied or refused; and that the amphora and
the chimney stack are the sizes they were built to (the amphora 1.484 m to its tip, 0.3368 m at its
widest; the stack 47 courses, 3.525 m, and 5 x 4 bricks at its top course) and drawn physically based;
and that the cottage is as measured: its door 2.0 m by 0.8 m with its floor 0.6 m up, its ridge at 7.0 m
and its wing's far wall 5.3 m left of the door, its chimney stack standing on its shaft at the eave
against the right eave wall, all 26 of its parts one tree under its plinth, the fourteen lanterns' glass,
bulbs and metal one shared material each, and its panes drawn as parts of their own.
It also presses P (frames 256 and 271) and F (276 and 286) while the walker stands, and checks that
scene time stood still while paused and ran ten times as fast at x10; that the world's alarm (a timer
for 3 s) rang at the first frame at or after 3 s; the day clock's reading; that the balloon is one tree as
built; and that the two porch pots share one clay (the cottage's meshes in fewer materials, the lanterns' 4 meshes in the cottage's 3; each
mannequin is one skinned mesh in one material; and, with a GPU, the pots drawn in one material and the rook in another).
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
paused and at x10, each rung at the first step at or after its time, a cancel, and dusk at 628.5 s and
dawn at 1411.5 s; a sphere area and a box area under a scaled parent entered and left, and a second
being coming in moving the first out; and possession, the user's intents moving only the possessed
being and the camera following it; and the mannequin: a capsule dropped from 0.5, 3.5 and 8 m landing
at v = sqrt(2 g h), the hard landings mailed once each and the hurt (nothing to 10 m/s, 0.18 at
12.5 m/s) taken from health, a 6 s walk whose gait speed follows the intent and whose planted foot does
not move more than 1 cm, its skinned sole (skinned by render's CPU twin of the GPU's skinning) within
1 cm of where the planted sole socket carries it, not sliding and on the ground, standing with both feet
flat and level, the third-person camera 3.2 m behind
and the first-person one at the eyes socket, and Tab possessing the other mannequin with the camera and
the controls following it; platforms: a mannequin standing on a deck (under a scaled parent) that climbs and
turns for 6 s stays on it, carried, its feet and planted heel within 1 mm of where they stood on it and its heading
on it within 0.01 degree; the deck's -x and back walls stop it at its radius; walking out of its open side it falls
at the deck's velocity plus its own, lands at sqrt(v0^2 + 2 g h), its health emptied, and 2 s later gets up on the
landing beach; a walker outside a basket is stopped outside its back wall, steps up onto its floor through its open
front, and does not stand on a floor lifted 3 m over it; the balloon's ride, on the baked island: 2 to 3 minutes, 120
to 200 m up, no faster than 10 m/s nor turning faster than 10 degrees a second, its basket at least 22 m over the
ground away from its spot, within 700 m of the island's middle, landing on its spot as it stood; and the sky (`src/skychecks.cone`): the sun at the clock's noon (36.0 degrees,
due south), dusk and dawn (-0.84 degrees, its centre at the horizon), the sun below the horizon and the
moon above it every quarter hour from 17:45 to 6:15, the light (noon: 81 000 lux and a few thousand nits of
ambient; midnight: the moon's 0.18 lux, never black), the exposure (EV 15 at noon, -1 at night, only falling
through the evening) and forty birds staying over the cottage. It exits 0 only if every check passed.
`shots/` holds the views of one run (`start`, `picked`, `walked`, `turned`, and the close-ups
`porch`, `chimney`, `ball`, `backyard`, `amphora`, `hornflower`, the mannequins' `mannequin-*`,
`cottage-photo`, `cottage-approach`), converted to PNG, `amphora-vs-photo.png`, the amphora's close-up
beside the photograph it was traced from, and `cottage-vs-photo.png`, the cottage from the photograph's
viewpoint beside the photograph.

After the sky's views, from frame 1170 (`src/glowscript.cone`), the clock is set to the next evening and night
for the lights:
- `glow-dusk-approach`, `glow-night-approach`: the cottage at 17:48 and at 23:00;
- `glow-glass-dusk`, `glow-glass-night`: the porch's right bay window close up;
- `glow-dusk-photo`, and `glow-dusk-vs-photo.png` beside the reference photograph: the photograph's view at
  17:48 (taken at 19:50 in May's evening; the reference photograph's light was then);
- `glow-lantern-dusk`, `glow-lantern-night`: the nearest path lantern close up, from 1.2 m at a three-quarter angle
  (like the front lantern of `C:\src\reference-images\lantern\solar-path-lanterns.jpg`, beside which `lantern-vs-photo.png` puts it).
  The walker was set down within its area, so the lantern is brightened.

At frame 1222 the lights are checked: on, the bulb at the hour's brightness, the lantern beside the walker
brightened 1.45 times, and the next one not. `--checks` runs the same lights on a scene of their own: off by day
(and not brightened by nearness), half way through the fade at half its time, at the hour's brightness after
it, brightened on coming near at night and dimmed on leaving, and off after dawn.

Last, from frame 1340 (`src/assembly.cone`), the plain walker lands on the beach again and the script steers it
(W, A, D and Shift pushed into SDL's queue as it needs them) the whole way: the path to the steps with eight
first-person pictures `walk-01` to `walk-08`; round the cottage's east end to the boarding spot, facing the basket's open
front (`balloon-ready`) and the burner close up (`ride-burner`); then the ride (`src/ridescript.cone`): Tab takes the
first mannequin over, set down there; it walks in at the open front onto the basket's floor (checked on the platform,
and the world told), turns about to face out, clicks the button on the back rail as the third person's camera shows
it, and with V rides the whole flight in first person, `ride-01` to `ride-06` from its eyes at six waypoints and
`ride-third` from outside the basket in the air (every frame its feet are checked to stay within 1 cm of where they
stood on the floor and its heading on it within 0.1 degree; landed, the balloon on its spot as it stood); it walks out
onto the lawn, back in, presses again, and at 70 m up walks off the open front: `fall-01` just off it, `fall-02`
falling, `fall-03` landed (its speed against the drop, its health emptied), `fall-04` up again on the landing beach;
the empty balloon finishes at x10 scene time and lands on its spot. Then the plain walker goes back to the steps at dusk
(`dusk-steps`, `dusk-path`); a night walk up the path's last 45 m (`night-01` to `-03`); and the specimen maple at noon
with the sun in front of and behind the eye (`maple-sun-ahead`, `maple-sun-behind`). Each phase prints what it
measured (the walk's farthest distance from the path's centre, the ride's, the fall's, the lamps' glow), and
the run fails if one does not finish.

After the assembly (`src/weatherscript.cone`) the clock is set to noon of a fourth day: `weather-sky-a` and
`weather-sky-b` are the sky from the lawn 60 s of scene time apart (the clouds carried east and changed), then the cottage
in each weather, `weather-clear`, `-fair`, `-cloudy`, `-overcast`, `-rain` and `-snow`; and T is pushed through SDL's queue
in snow and the weather is checked to be going to clear.

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

## The villagers

Four mannequins (beings 3 to 6, after the user's and the idle one) live on the island and walk by themselves. They
are **the simulation's** (`src/npcs.cone`): each is a being like the user's, walked by the same character
controller and posed by `walkgait`, whose intents (a turn and an ahead, never a key) come from a mind instead of
the keyboard. They cross in the snapshot as every being does, and the view draws them as it draws the idle mannequin
(single player: stepped from the snapshot's intents; connected: interpolated). Every mode has them from the start (a
server's, the user's) except a scripted run, which makes them at the end of its own part (`src/npcscript.cone`: four
pictures, `npc-villager`, `npc-gardener`, `npc-follower` and `npc-shy`), so that the shots of everything before are of
the world alone. **Tab skips them** (`Being.ai`).

| Villager | What it does | Made of |
| --- | --- | --- |
| villager (blue) | walks a round of the cottage's front, the bridge's crest and the beach, 8 s at each, by the path where there is one (ground off it costs three times as much a metre, so a route follows the path's windings, kept within 1.3 m of its centre, rather than cutting across the forest) | navmesh A* and funnel; `PathFollower` |
| gardener (green) | idles about the lawn by the horn-flower and the ball: walks and stands in turns, within 5 m of home | `npcbehave.Grazer` (a behaviour tree, Wander) |
| follower (ochre) | keeps about 6 m from the nearest player: trails him when he walks, backs off when he comes close | a route to the spot at that distance on the line from the player; `PathFollower` |
| shy villager (purple) | keeps away from the player (runs inside 5 m until he is beyond 9 m), goes home when he is not near it | `npcbehave` `evade`, then a route home |

**The navigation mesh** (`navmesh`, after Recast and Detour) is built at start-up in 0.2 to 0.5 s from the baked island
inside the window x -330 to 330, z 150 to 565 (about 2 s, 4520 polygons; it was x -110 to 90, z 290 to 515 in 0.3 s): the island's heightfield cells in it above the wading
line as triangles (the island's own diagonals), a circle obstacle for each trunk and rock and the balloon's basket, a
box obstacle for each box solid taller than 2.2 m (the cottage), and for each lower box solid (the porch deck and
steps, the bridge's crest, ramps and parapets) its top as walkable triangles, level or sloped; a disc every 2 m along the
path's centreline marks the ground under it as the path's area. The agent is the
capsule's: 0.3 m radius, 1.9 m tall, climbing 0.4 m (`STEP_UP`) and not ground steeper than 40 degrees; the voxels are
0.2 m by 0.1 m (0.3 m by 0.2 m left the bridge's 1.4 m road, between parapets, too narrow to cross). `browser.exe
--navpic PREFIX` draws it.

**Steering and containment.** A mind asks for an acceleration on the ground plane (npcbehave); its `Body2` is set to
where the real body is, and the velocity it then wishes (which builds up at the steering's acceleration) is held to the mesh (walked half a metre along it with
`moveAlongSurface`: a wall turns the wish along it or stops it) and turned into intents: a turn as fast as the body
turns (`TURN_RATE`), an ahead that falls as the turn needed grows, both rounded to 1/127 (the snapshot's resolution),
so the view stepping a villager from the snapshot's intents does what the simulation did. Villagers do not avoid each
other or the player except as the shy one flees: characters pass through one another, as every body does here.

A step of the four minds takes about 2 to 8 microseconds (measured by `--checks` and the run's last lines); a route
query a few tenths of a millisecond. `--no-npcs` leaves them out (a scripted run without them is byte-identical in its
shots to a run before they were added).

## The playground: bowling and three spells

In the balloon's backyard, 26 m west of the basket, ten amphoras (the amphora's mesh at 0.6, 0.89 m tall) stand in a triangle
at the end of a lane that runs west (`bakedworld/pins.cone`); the lane's head is 11 m east of the apex. The patrolling villager's round
now ends here (it used to end at the beach), so there is a mannequin to cast at. Everything in it is the simulation's: the view draws only what the
snapshot and the simulation's bursts say, so every connected player sees it.

- **Bowling (B).** The ball (the scene's own, 1 m across) is made a rigid body at the player's feet and rolled at the spot the camera's ray meets,
  at 13 m/s with a damping that stops it within about 25 m. The pins are rigid bodies (convex hulls of the amphora's outline, 3 kg each, the ball 25 kg) in
  the physics world (`props.cone`; layer 16, which mannequins and ragdolls do not see). Live bodies slower than 0.45 m/s for 0.8 s all go to sleep together
  (the world's own sleep is never satisfied by a ball creeping down a lawn); 5 s after the last body sleeps, every moved prop rises 0.7 m and glides home in
  1.4 s, and is made again there, asleep. The ball's home is the lane's head. From the first time any prop leaves its home all eleven are in every
  snapshot (14 bytes each).
- **Paint (F1).** A ray from the camera through the physics world's ground, solids and platforms and through every mannequin (four spheres each);
  the nearest is hit. A splat is a box decal (render's) with a ragged-blot texture, tinted one of four colours, kept 20 s and faded over the last 6 s. On a
  mannequin it is kept in the mannequin's frame (position and heading only: the honest version without a paint layer on the skin, so an arm that swings
  carries no splat with it). Decals need the image-quality passes, so with `PEGASUS_FX=off` there are none.
- **Zap (F2).** The target jerks about for a second: twelve bones each given a fresh random rotation twenty times a second, the pelvis thrown about, the whole dying
  away. A procedural overlay on the gait's pose (`spells.cone`), the same in the simulation and in every view because it is a function of the seconds since
  the spell began. The "ah!" is audiomixer's new `SYNTH_VOICE` (a glottal pulse train through three vowel formants, 0.35 s; a man's pitch for a tall
  mannequin, a woman's for a short one), played by each view at the target when the simulation's burst arrives. Nobody has listened to it.
- **Jig (F3).** The target dances four seconds at two beats a second: bounce, sway, knees up in turn, arms swinging, head nodding.
- A zapped or dancing mannequin ignores its own intents until the spell ends. The target must be in the open (a ray from the caster's eye to its chest meets no
  solid) and within 25 m; the simulation decides, the view's targeting (`worldinteract`'s cone-and-cost lock-on) only suggests.
- The hotbar's first three slots carry icons drawn by hand over them (a splat, a bolt, a dancer).

## Playing together: one server, two browsers

The browser is two programs in one: a **simulation** (the truth: the bodies, the platforms, the world's own
code, the clock) and a **view** (what is drawn, picked and followed). Single player runs both in this
process, joined by a connection of Cone's `netmsg` package (the in-process pair: the same messages, in the
same order, with nothing lost); connected, the simulation is a server's and the view dials it over UDP.
**The server owns the truth** (after Glenn Fiedler's *Gaffer On Games*):

- **Content stays local.** Every browser has the baked-in world, the island and the mannequin's mesh in its
  own program; none of it crosses. The server sends an `Object` request, which says *which* object and the
  ids it reserved for its parts, meshes and materials, and the view builds it itself (so its meshes and
  images are never in a packet: they would not fit in one). A client that joins late is told the objects
  and where everything stands (the replay log).
- **The user's own mannequin is predicted.** The view steps it at once by the same code the server runs (the
  gait, the collision, the platforms) from each fixed step's intents, so it answers a key in the frame it is
  pressed, whatever the link's delay. The intents go to the server numbered, the last 10 in every packet
  (more, up to 30, while the server has not yet applied them), so a lost packet loses nothing. Each snapshot says
  the last input the server applied; if the body the view predicted after that input is not where the server
  has it, the view takes the server's, plays its unapplied inputs again, and blends the difference away over
  0.1 s (a difference over 2 m at once). The camera follows this body, in the view.
- **The others are drawn in the past.** Every other mannequin is drawn 100 ms behind (or, when the link
  delivers fewer snapshots, behind enough to have two), on a cubic through the two snapshots either side of
  that moment (with the velocities they carry), and its gait is animated in the view from the speed and heading.
  The server's tick is estimated from each snapshot and the round-trip time netmsg measures.
- **Local and cosmetic:** the birds (each browser flies its own flock), the camera, the gait, the day (a
  function of scene time).

```
# the server: a headless simulation on a UDP port (no window, no GPU)
browser.exe --serve 7777

# each player, on this machine or another (a window each)
browser.exe --connect 127.0.0.1:7777
browser.exe --connect 127.0.0.1:7777
```

Each client that joins gets a mannequin of its own on the landing beach, and sees the others walk (the
second, idle mannequin stands where it did). The keys are the keys above, except that **Tab** (possess the
next being) does nothing connected, and **P**, **F** and **T** are the world's: any client changes the scene's
clock and the weather for everyone. A click on the launch button in the basket sends the balloon off for all.
A client that leaves takes its mannequin with it.

**Across machines.** The server binds UDP on all its addresses, so another machine can connect to its
address and port, *but the port must be opened on the server's host* (a firewall rule, and on a cloud VM its
security group): nothing in this program opens one. A server runs on the VM (`SERVER.md`: building it for Linux,
deploying, the log, connecting), with its port open to listed addresses only. There is no encryption or login
(netmsg's handshake proves only that a client can receive at the address it gives); a client says a name
(`--name`, default the machine's), shown over its mannequin to the others, and the server refuses one already in use.

Options: `--run-for S` stops a run after S seconds of real time; `--headless` with `--connect` runs a client with no window or
GPU; `--impair LOSS LATENCY JITTER` (with `--serve`) puts netmsg's lossy relay in front of the server and
prints its port (LOSS in 1000, LATENCY and JITTER in ms, each way); `--autowalk` and `--trace` are the
test's.

**The test** (`tools/netcheck.py`, which runs a server and two headless clients on 127.0.0.1 and reads their
traces): client A walks a script (a walk, a turn, a run, a jump, backing up), client B watches. It checks that
A's predicted position agrees with the server's at every snapshot, that B's drawing of A follows A's server
path within 0.3 m, and prints how far in the past B draws and the bandwidth each client uses.
`python tools/netcheck.py` is the loopback run, and `python tools/netcheck.py --impair 50 100 20` the same through
the relay with 5% of packets lost and 100 ms and up to 20 ms more each way.

## How it is put together

| File | What it is |
| --- | --- |
| `src/browser.cone` | the shell, on the main thread: the window, input and the user's controller, the clock and the fixed steps (frame's `Host`), the statistics; it makes the three actors and hands the scene each frame (below) |
| `src/simactor.cone` | the simulation actor, the truth: the scene's registry and tables, the beings' bodies (the physics), the clients and their inputs, the world actor's turn, the requests and snapshots it tells each client; it runs in this process (single player) or alone (`--serve`) |
| `src/viewactor.cone` | the view actor: the picking system, the placement tree and platforms for drawing, the snapshot for render; it predicts the being its user drives, interpolates the others, animates every gait, flies its own birds and follows with the camera |
| `src/simview.cone`, `src/netcodec.cone` | what crosses between them, as values and as the bytes of netmsg's messages (quantised state, input batches, requests); round-trip tests in `src/netchecks.cone` |
| `src/netlink.cone`, `src/replaylog.cone`, `src/server.cone` | the connections' two ends, what a late client is told, and `--serve` and its test script's controller |
| `src/worldactor.cone` | the world actor: the world's own code (the baked-in world) behind a Door of its own, its turn once a frame |
| `src/renderactor.cone` | the render actor: the device and everything on it; it applies render's requests, draws the snapshot and presents |
| `src/parts/parts.cone` | the module `parts`: one id registry for parts and resources, aspect tables keyed by id, the requests and messages, the Door senders are given, and the Scene |
| `src/parts/placement.cone` | the placement tree: parents before children, world matrices in one pass over what changed |
| `src/parts/events.cone` | the scene's clock and the day, timers on the timewheel, areas |
| `src/parts/platforms.cone` | platforms: a part's floor and walls, and how each moved since the last frame |
| `src/parts/snapshot.cone` | what the scene publishes each frame for drawing |
| `src/parts/weather.cone` | the wind, the six weather states and their crossfade, the rain and snow emitters |
| `src/fogshader.slang` | the fog's depth slabs (compiled and embedded in `skyrig.cone` by `tools/shaders/shaders.py`) |
| `src/weatherscript.cone` | the scripted run's last part: the sky 60 s apart, the cottage in each weather, the T key |
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
| `src/assembly.cone` | the scripted run's last part (frames 1340 on): the walk from the beach to the steps, round to the balloon, dusk at the steps and the night walk, steered by the script's own key events |
| `src/ridescript.cone` | the assembly's balloon ride: boarding, the ride, stepping out, the fall and getting up again |
| `src/support.cone` | what a body stands on (the ground or a platform's floor), platforms' walls, and carrying the bodies on a moving platform |
| `src/checks.cone` | the scene core's headless checks (`--checks`) |
| `src/npcs.cone`, `src/npcchecks.cone` | the villagers (below): the navigation mesh built from the island and its solids, the four minds, and the intents they give their beings; their checks (a 520 s run in the physics world) and `--navpic` |
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
| `Platform` | a part is a platform: a level floor and walls in its own frame, which beings stand on and are carried by |

Once a frame, at the publish phase, the scene actor applies the inbox in order, routing each request to
the system that owns what it changes (render's moved on to the render actor, in the same order); a
request naming a part that is gone is refused and counted. The
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
(and under it the burner's coil and valves, the flame and the pilot light) and the basket (and under it its
rim and the launch button) beneath, so its ride moves only the envelope. The renderer never walks the tree: each frame the scene publishes
a **snapshot** (`parts/snapshot.cone`), a plain struct of every placed part's world matrix, the scene's
time, the day clock, the possessed being's eyes and the birds, and the draw phase works from that alone:
the scene actor moves it to the render actor each frame, and render moves it back once it is presented.

**Beings and possession.** The walker is a *being*: a part (`you`) with a body (the walk) that moves by
the *intents* it is given each step. The user's *controller* turns the input package's actions into
intents for whichever being the user *possesses*, and the camera follows the possessed being's eyes.
Input, camera and walker are not wired together, so another being (T2's mannequin) can be possessed,
or driven by another controller, without touching either (`src/being.cone`).

**The scene's time** is the scene's own clock: frame's fixed steps at its rate, 1, or 0 paused (P), or
10 (F); exact on the synthetic clock. A day is 24 minutes of it, the day clock reading 07:00 at the
start, dusk at 17:28 (628.5 s) and dawn at 06:31 (1411.5 s), each posted to the world as `Dusk` and `Dawn`.
Timers run on Cone's `timewheel`, driven by scene time. Each
frame the world reads it and sends its motions as ordinary `Place` and `Fire` requests: the button's cap
sinking 2 cm for a quarter of a second when pressed; the balloon on the ground, its burner firing 3 s in
every 4; pressed, its ride (below). (The ball lies still.)

## The balloon ride

**The basket** holds two standing mannequins: 1.9 m square inside, its wicker walls 1.1 m above its floor
(the floor 0.12 m up, a step), a dark leather roll on each wall's top, and **open at the front** (its +z
side, turned to face south-south-east down the lawn route's end), so a walker comes round the cottage's
east end and steps straight in. The **burner** is one head, nothing pale or wood-toned, so it is not
taken for a mannequin's head: a dark charcoal steel can (sRGB 44, 46, 50) on four spokes of a charcoal
square frame 2.75 m above the floor (a 1.75 m figure's head is a metre under it), on uprights from the
basket's corners; a copper coil (sRGB 150, 68, 36) wound five times round the can; brass valve fittings
(sRGB 176, 136, 44): two valves on the spokes and a ring at the nozzle. The old two passengers' heads are
gone. The **launch button** is a red cap on a charcoal mount inside the basket on the back rail, to the
right of a rider facing out; the ground button is gone (its place in the world's table is kept as the
*boarding spot*, where the lawn route ends: nothing stands there).

**A part can be a platform** (`parts/platforms.cone`, the `Platform` request): a level floor and walls on
any of its edges, in the part's own frame, so it moves with the part. The basket is one. The physics lives
with the bodies in the scene actor (`src/support.cone`): a body stands on the highest of the ground and any
floor over it no more than a step above its feet (a floor overhead is not one to stand on); a floor's walls
stop a capsule from the side it came from (inside or out), while its height overlaps theirs; an edge with
no wall stops nothing. Each frame, once the world's requests are applied and propagated, the scene asks
how each platform moved since the frame before, and **carries** the bodies standing on it by that move:
their feet's point, their heading, the gait's planted feet and the camera following the possessed one,
turned about the platform's origin and moved with it (so a long ride does not drift), before the beings
are placed, so a rider and its floor are drawn together. A body is held up only by the floor: a mannequin
that walks off the open front falls, at the floor's velocity as well as its own, under the usual gravity
and hard landings. A fall that empties its health (24 m/s, a 30 m drop) lays it down for 2 s; then it
gets up whole on the landing beach. The shoreline holds only a body on or near the ground: a
rider carried by the basket goes where it goes, and one that falls lands where it falls.

**The ride** (`bakedworld/motions.cone`) is a centripetal Catmull-Rom spline through twelve waypoints,
travelled by its length on the scene's clock in 160 s, easing from rest over 20 s, steady, and to rest
over the last 20 s, turning as it goes so its open front faces each waypoint's view (a rider who walked in
and turned about faces them all). Each press while it flies is ignored. The waypoints (x, height above
the spot, z) and what the open front faces:

| Waypoint | Where | Height | Facing |
| --- | --- | --- | --- |
| the spot | (-10, 300) | 0, then 25 straight up | as it stands, the cottage's back |
| over the lawn | (25, 345) | 75 | the path, the beach and the cove |
| east lowland | (90, 430) | 130 | the beach's curve, the cove, the south-west peninsula |
| east shore | (70, 540) | 165 | west along the coast |
| over the cove (the top) | (-40, 610) | 180 | north: the beach, path, cottage and the hill behind |
| west coast | (-170, 560) | 170 | north-east over the lowland to the hill |
| west lowland | (-200, 420) | 150 | east: the cottage, the cove, the south-east peninsula |
| north-west | (-120, 300) | 110 | south-east over the cottage to the cove |
| coming down | (-45, 278) | 55 | over the cottage's roof |
| the spot | (-10, 300) | 25, then down | as it stands |

The burner fires 3.2 s in every 5 on the climb to the top, 1.5 s every 20 s after, and not at all for the
last 45 s as it sinks to land.

**A click picks.** The picking system keeps a copy of each mesh's triangles (taken when its
`DefineMesh` is applied) and which mesh each shown part is (not the anchored ground). A left click casts a ray from the eye through the
pointer, takes it into each part's own frame by the inverse of its placement, tests the part's box,
and, where the box is entered nearer than the best hit so far, each triangle; the nearest triangle
hit names the part. The browser prints it and sends itself a `Highlight` (the render system draws the
part in its colour half-way to yellow, and lets the last one go).

**Buttons are messages to the world.** A world that wants a part's clicks sends `Listen` for it; when
a click picks that part, the scene posts `Message.Clicked` into its mail for the world, which moves to
the world actor with the world's turn at the publish phase of the same frame; the world reads it and
answers with requests. The scene never reaches into the world, and the world never into the scene's
tables. Timers (`Rang`), dusk and dawn, and areas (`Entered`,
`Left`) come back the same way: the baked-in world sets an alarm for 3 s and keeps an area over the
inside of the balloon's basket, and prints what it is told.

**A world is given a Door, not the scene.** The `parts` module keeps the registry, the inbox and the
placement table private, and hands a sender a `Door`, which only reserves ids and sends requests. The
baked-in world is a sister module of `parts`, run by the world actor with a Door of its own; it cannot
name the browser's scene, any table, the render system or a GPU handle, so fetching worlds later
changes where a world comes from, not what it can do. The registry is lent to the world for its turn
and handed back with its requests, so ids are reserved in one place and in one order, without a lock.
(The starship, when the scene had it, made a GPU device of its own, an instance and a device with no
surface, to mesh itself, never the browser's.)

**Four actors.** The shell (the main thread, since SDL wants the window's events there), the scene, the
world and render each own their state and reach each other only by messages (Cone's actors). Each
frame the shell hands the scene its steps and the user's intents; the scene simulates, awaits the
world's turn, applies the requests, publishes the snapshot and awaits render's drawing of it; then it
reports the frame to the shell. The actors take turns, so a frame is the same sequence it was on one
thread, and no actor divides its own work across threads.

**Hydrate, then inject.** Each object of the baked-in world is a description in Cone, ported from the
example programs of Cone's `sculpt` and `vfx` packages and from the `starship` package (examples and
executables are not importable, and this content belongs to the browser). Hydrating one runs it through
those libraries into plain data, a `Hydrated`: meshes, materials (with their paint images), parts
naming them with their parents and placements, and emitters, with no ids. Injecting it defines its
meshes and materials (a material the world shares across objects, the cottage's and the maples', only once),
reserves an id per part and sends its requests. One object is hydrated and injected a frame, in the
world actor's turn at the publish phase, so frames keep coming while the world arrives.

| Object | Ported from | Size | Triangles | Hydrated in (ms) |
| --- | --- | --- | --- | --- |
| lathed rook, on the porch deck | `sculpt/examples/rook.cone`'s rook with no booleans: one lathe, four capped merlons joined on; at 0.32 of its size, standing on the deck at its east end | 0.56 m | 2,320 | 0.75 |
| porch pots (two pots and two clumps of flowers) | new (`porch.cone`): a clay pot lathed from an outline, 48 steps round; `leafrosette`'s flower clump (the beds' kind) planted in each, one violet, one yellow | 0.35 m | 1,500 | 3 |
| amphora | new (`amphora.cone`): an outline traced from a photograph, lathed 128 steps round; two handles swept along a traced centreline; its bands and patterns painted, 2048 x 2048, and an ORM map | 1.5 m | 63,832 | 100 (paint 83) |
| horn-flower (stalk, leaves, petals, heart) | new (`hornflower.cone`): the horn, 2.4 m and cut short; `sculpt/examples/flower.cone`'s petals on the cut; five swept leaves | 2 m | 99,956 | 39 |
| rocks (108 parts of six meshes) | new (`vegetation.cone`): `noiserock`'s faceted rocks in the kit's granite, scattered by `layout.cone`; they replace the one boulder | 0.3 to 1.6 m | 24,192 | 505 (granite bake 440) |
| specimen maples (two, 4 parts) | `weberpenn` red maples, fitted to the winter photograph, in full leaf (below) | 11.6 m and 10.6 m | 316,528 | 392 |
| pines (63, 126 parts of three variants) | `weberpenn` white pines, tiers of needle plumes | 13 to 15.5 m | 1,144,536 | 336 |
| forest maples (35, 70 parts of three variants) | `weberpenn` red maples, three levels, bigger plainer leaves | 8 to 10.5 m | 626,870 | 3 |
| edging stones (335, four variants) | `noiserock` low rounded stones in field-stone colours, one every 0.7 to 1.1 m along both edges just outside the walking surface (0.03 to 0.2 m clear of it, never on it), a quarter of their height sunk into the ground | 0.25 to 0.5 m across (the larger at bends) | 33,500 | 603 (bake 600) |
| hostas and ferns (118, two each) | `leafrosette` | 0.5 to 1 m | 60,048 | 0.5 |
| flowers (16 clumps, six variants) | `leafrosette` | 0.4 m | 7,040 | 0.2 |
| cottage (26 parts: plinth, walls, battens, roofs, trim, window frames and panes, door and its panes, porch deck, skirt, posts, roof and steps, chimney shaft, stack, flaunching and pot, two wall lanterns) | new (`cottage.cone`, `chimney.cone`): measured from the reference frame with the door as the ruler (below); boxes, prisms and slabs; board, shingle, deck and flagstone maps from the `surfacepatterns` texture kit; the chimney is the stack of step 4 on a brick shaft from the ground, 3.3 m + 4 m | 7 m to the ridge, 7.3 m to the pot | 5,044 | 870 (the texture kit's four bakes 610, the chimney's maps 260) |
| path lanterns (twelve, six pairs 2.8 m apart, 3 m across: stake, glass, bulb, cap each, and a pool of light) | new (`cottage.cone`): a lathed round post, and a square head of boxes (plinth, corner posts, arched tracery, flared hipped roof, solar-panel block), four clear panes and a bulb sphere, in three shared materials | 0.5 m | 17,712 with the pools | 0.24 |
| hot-air balloon (envelope, skirt, burner frame, cables, basket and rim, burner coil and valves, launch button) | `sculpt/examples/balloon.cone`, the chevrons; the basket (open-fronted, for two standing riders), the burner and the button new | 25 m | 77,200 | 51 |
| burner flame (core, tongue, embers) and pilot light | `vfx/examples/burner.cone`, live, fired in bursts | 3 m | particles | (in the balloon's) |
| (launch button) | nothing now: the button is in the balloon's basket; the object keeps its place in the list | | 0 | |
| ball | new: a sphere, checkered; it lies on the lawn where it was left | 1 m across | 1,984 | 0.03 |

Not in this scene (their code stays): the chess rook (`sculptures.cone`'s `rookMesh`, a lathe and three
booleans), the horn (`hornMesh`, which the horn-flower's stalk is cut from), the mound (a 65 x 65 grid lifted
on the GPU by the heightfield shader, `src/heightfield.slang`: the island's ground now covers its test), and the
starship (frame, membranes, wing lights, ports, eyes, mouth; `packages/starship` meshed on the GPU at 0.03 a
cell, 156 m, 458,324 triangles, 700 ms of it 0.4 s making the kernels), which `dragonship/`, `gpu/` and
`hydrateShip` keep for a space scene.

## The vegetation

The scene is 15 October, so the maples are in autumn colour: scarlet at the top and on the outer side, orange and gold in the middle, a few greenish-yellow leaves low and inside (the leaf palette's ramp, by each leaf's exposure); the pines stay green. The generators are Cone's own packages (`weberpenn`: Weber and
Penn's trees with an opposite-pair and whorl mode; `noiserock`: rocks; `leafrosette`: hostas, ferns and flower
clumps); this folder holds the scene's choices. Everything is a few meshes (variants by seed) of which many parts
are copies, so render draws each (mesh, material) pair as one instanced draw. **Every leaf is geometry, not a
cut-out** (render has no alpha-test material yet): a maple leaf is a blade of 7 to 24 triangles in its own
three-lobed outline, drawn from both sides, a pine's needles are tiers of drooping ribbons. When the cut-out
material arrives, only `weberpenn`'s `foliageMesh` and the leaf material change; the trees, parts and layout stay.

| File | What it is |
| --- | --- |
| `src/bakedworld/layout.cone` | where everything stands: the rules and the seed (`layOutVegetation` makes the list of `Item`s, declared in `src/island/island.cone`: kind, variant, x, y, z, yaw, scale, radius, once after the bake; the `Island` holds it and `vegetationLayout(isl)` returns a copy) |
| `src/bakedworld/vegetation.cone` | the meshes, the textures and the parts of each family, objects 9 (rocks) to 15 |
| `src/vegscript.cone` | the scripted run's eleven views (frames 1230 to 1339, after glow and glass's: `veg-maple`, `veg-maple-close`, `veg-path-beach`, `veg-path-forest`, `veg-cottage-trees`, `veg-cottage-above`, `veg-plants`, `veg-flowers`, `veg-rocks`, `veg-edging`, `veg-overview`), worked out from the layout |
| `src/vegchecks.cone` | the layout's checks, part of `--checks` |

**The rules.** Two specimen maples stand near the cottage and the path (-4, 341 and -31, 347, the first to the
right of the lane as you walk up it, as in the photograph that started this) and eight pines round the cottage's
back and sides; then pines (two in three) and forest maples by `island.forestDensity`, a dart-thrown scatter over
a jittered grid of candidates, nearest the path first, with a spacing that falls where the forest is thick (7 to
13 m), 4.5 m from the path's centre at the least, above the beach, off steep ground, inside the fence, and clear of
the cottage's footprint, the old world's objects and the lanterns, capped at 90 trees. Rocks: 22 on the beach and
shore about where you land, the rest on steep ground, and one every dozen metres or so at the path's side. Edging
stones along both edges of the path (jittered spacing 1.1 to 1.6 m, larger at its bends, a third sunk, overlapping
the path's edge) from where the beach ends to the steps. Hostas by the cottage's foundation and the path's last
30 m; two to four fern clumps round each tree and along the lane; flowers in beds on the porch front either side
of the steps, round the amphora and by the path's last 25 m. Deterministic from one seed.

**Interfaces for assembly.** A tree is two parts: the bark is the root (what moves a tree: send `Place` to it) and
the foliage a child with no placement of its own. `world.partOf(SPECIMENS, 2 * i)` is specimen i's root;
`PINES` and `FOREST_MAPLES` the same, two parts a tree, in layout order; `ROCKS`, `EDGING`, `GROUNDCOVER` and
`FLOWERS` one part an item. To clear an area, add a disc to `worldObjects()` in `layout.cone` (its x, radius, z),
or a rectangle to `COTTAGE_RECTS`, and every kind keeps off it; to move an anchor, change `anchors()`. `--no-vegetation`
leaves the lot out (to time what it costs).

**The red maple, measured against the winter photograph** (`winter-3-king-rhodes-2014.jpg`, the lone
single-trunk tree; `shots/maple-vs-winter-photo.png` puts it beside the model's bare skeleton, the photograph
Famartin, CC BY-SA 4.0). The numbers come from the photograph (the crown's width at 19 heights, by segmenting it
against the sky; the trunk's width by rows; the limbs' exit angles by eye on a crop) and the model's, over four
seeds and two views:

| Measure | Photograph | Model |
| --- | --- | --- |
| first limbs, as a fraction of the height | 0.24 | 0.21 to 0.24 |
| crown width over height, widest | 0.81 at 0.5 H | 0.76 at 0.48 H |
| crown width over height at 0.9 H | 0.39 | 0.46 |
| rms difference of the width profile | | 0.06 |
| trunk radius over height, at the ground | 0.020 (65 px of 1590) | 0.022 to 0.025 |
| trunk radius over height, mid-trunk | 0.0125 | 0.012 to 0.014 |
| limbs' exit angle from the vertical | about 10 to 57 degrees, the lowest widest | mean 30, standard deviation 24 |

Weber and Penn's dials were fitted to it by a random search (about 6000 trials, the crown profile and the exit
angles in its cost), not by hand; the fitted values are `redMaple`'s in `packages/weberpenn/src/species.cone`.
Where it falls short: the model's top is wider and its fine branching sparser and straighter than the
photograph's, and its limbs' angles vary more.

**Colours** (late May, none from Jon's autumn note): leaves a deep green in the crown's shade to a fresh
yellow-green in the sun, the very sunniest with a hint of spring's red, petioles red; the maple bark is the kit's
`BarkParams.redMaple`, the pine's `BarkParams.pine` greyed; the pine's needles deep blue-green; hostas one
blue-grey and one yellow-green, ferns two greens, flowers buttercup, white and violet.

**Glow and flutter** (render's foliage shading, set per material in `vegetation.cone`: `MAPLE_TRANSLUCENCY`,
`MAPLE_FLUTTER`, `PINE_TRANSLUCENCY`). With the sun behind a leaf, light comes through it in the leaf's own
colour, so a backlit crown glows coral and orange instead of going dark; the maples' leaves also turn up to about
seven degrees about their stems, each on its own phase and period (0.6 to 1.8 s), so they catch and lose the light.
The maples' leaf meshes carry each leaf's pivot and phase in their vertex colours (`FoliageParams.sway`). The
pines' needles have a little translucency and no flutter. `shots/foliage-*.png` shows the maple before and after,
with the sun behind the tree and behind the viewer, and two frames 0.25 s apart.

**Counts and cost.** 98 trees (63 pines, 35 forest maples) and 2 specimens, 106 rocks, 335 edging stones, 21
hostas, 97 ferns, 16 flower clumps: 656 parts in all, 2,959,446 triangles of the world's 3,530,352 drawn (with
the island, sky and cottage), against 1,336,138 and 67 parts without (`--no-vegetation`). Frame times at the
start view, `--watch`, the p50 of the whole frame with and without: on the RTX 4060 6.97 and 6.99 ms (its render
phase 2.69 and 2.25 ms), on the Intel UHD 770 32.2 and 33.5 ms, about 31 frames a second either way (its render
phase 2.8 and 2.1 ms): the Intel's frame was already waiting on the GPU or the display before the vegetation, and
it adds no more than the noise between runs (the machine was shared, so a clean difference could not be read).
The scripted run's phases, which include every close-up, were no cleaner: see the PR.

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
the fine one covers the whole island's land (it was x -400 to 400, z -64 to 704, with the hill beyond it drawn on the
coarse mesh up to 7.6 m off the walker's ground). `groundHeight` (`island.cone`) is **triangle-exact**: the quads are split
by one rule (`diagonalFlipped`), the index buffers use it, and the query finds the triangle the mesh has
(`--checks`: 4000 points against the index buffer, within 0.4 mm; plain bilinear would be 0.5 m off).

**The path** is a Catmull-Rom curve from the beach (heading north for its first 22 m) to the cottage's steps,
205 points a metre apart (`pathCentreline`), its height smoothed along it and pressed into the ground in a
ribbon (the steepest metre 9 degrees); the distance to it is a second field the ground shader reads.
**There is no fence.** A walker may go anywhere on the island, up the hill and round the coast; the one limit is
the **shoreline**: `constrain` (called by every walk and mannequin step) refuses ground more than 0.6 m under the sea
(`WADE_DEPTH`), sliding the step along the shore (by the ground's gradient), and lets a walker already in deeper water
walk to ground no deeper than its own. The old polygon is the **dressed area** (`insideDressed`): the vegetation's
layout and the brook's stones are placed only inside it, so the rest of the island is bare grass (the ground shader's
litter is painted everywhere). The fine mesh covers the whole island (x -640 to 784, z -696 to 704, about a million
triangles), so the ground a walker stands on is the ground drawn. **The forest** (`forestDensity`)
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

## The brook and the stone bridge

A small brook runs 435 m from a spring on the hill (outside the fence, seen from the balloon) down to the lawn,
under the stone bridge on the lawn route, west across the back yard, south down the west side of the clearing
through a dingle cut across the lowland's rim (the pad is a shallow bowl: its lowest saddle is 6.1 m, the water
at 4.3), and down a cascade to the sea 32 m west of where you land. Designed in `Research/brook-and-bridge.md`.

**The land carries it** (`src/island/brook.cone`, run after the path in `bake()`). The heightfield is 2 m a
sample and cannot hold a 1 m channel, so the land gets a wide shallow trench (the yard's flat bed is about 4.4 m
across, the banks 1 in 2.2, joined to the ground by a smooth minimum; the dingle is up to 2.8 m deep at the rim,
banks 1 in 3) and the water is a ribbon lying in it. The course is a Catmull-Rom curve through `BROOK_CONTROLS`,
a point about every metre (`brookPoints`); the water's level at each control is a table, held to the ground's
smoothed height less 0.3 m, never rising, falling at least 0.3% in the yard, and in steps of 0.3 m down the
cascade. `--checks` walk the course: the ground under the ribbon is never within 0.08 m of the water, the water
never rises, it keeps 8 m off the path and inside the fence, and the yard centreline stays 15 m behind the
cottage's front wall (a cottage turned a quarter turn, its side wall up to 12 m long running back, leaves 3.6 m
to the centreline). The distance to the course (8 m at most) is the ground mask's blue channel
(`brookDistanceAt`).

**The water** (`src/brook.slang`, `bakedworld/brook.cone`) is one see-through ribbon mesh, seven vertices across
a station every half metre, drawn after the opaque things (premultiplied alpha, depth tested, not written). The
vertex data is packed into the three streams (uv: across and along in metres; normal: speed, foam, half width).
Two layers of value noise scroll downstream at speeds that differ, the middle faster than the edges, their slopes
tilting the normal in the ribbon's own frame (found from screen derivatives); the colour is the sea's way: a faint
tea-green body, the sky by Schlick's Fresnel, the sun's glint on the ripples, foam round the bed stones and at the
foot of each step, an edge that fades and an end that fades away at the sea. The speed follows the fall (about
0.4 m/s in the yard, a metre in the dingle). Water is not solid and not part of `groundHeight`: a walker wades
along the bed.

**The bed and its edge** are items of the vegetation's layout (`layout.cone`, last, so existing placement only
shifts where the brook's clearing discs cut it): about 70 squat rocks in the water (tops 5 cm over the surface and
at most 0.36 m over the bed, so a walker wades over them: a low solid is a step), 100 edging stones along the
yard's lips, and ferns beside them; a disc every 3 m along the course (2.6 m, 3.2 m in the dingle) keeps every
tree, rock and plant out of the channel.

**The bridge** (`bakedworld/bridge.cone`) is a single low segmental arch of rubble masonry on the lawn route at
(-7.0, 314.4): 3.0 m clear span, 2.0 m wide over all, 1.4 m between its parapets, a flat crest 2.0 m long at
0.58 m over the banks, ramps of 2.5 m each side (13 degrees), parapets 0.55 m high with rough copings. Four
parts built flat-shaded from quads in `surfacepatterns` rock (0.35 m blocks, lichen; the underside mossier). Its
collision is nine `Solid`s built from the same numbers (`bridgeSolids`): the crest, the two ramps (a sloped
top: `Solid.ramp`, a box whose top rises linearly along x or z, general, usable for any ramp) and a parapet on
each side of each. `--checks`: a walker climbs the south ramp, crosses the crest at 0.58 m and comes down the
north ramp at the road's height; the parapets stop it on the crest, on a ramp, and from the bank beside it.

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
ridge. The twelve path lanterns and the two wall lanterns are one family, square in plan like a solar path light (post
or bracket, glass, bulb, and the head: bronze frame with arched tracery, flared hipped roof and solar panel), the
glass and the bulb each its own part, in three materials the world shares.
