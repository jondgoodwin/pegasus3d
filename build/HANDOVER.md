# W148 boids: HANDOVER (parked by the coordinator, 28 Nov)

Brief: `C:\Users\jondg\MyDrive\Incubator\3D Web\Sessions\brief-w148-boids-2026-11-28.md`. Branch `pegasus-boids`.
Reference for the steering: `C:\src\cone\packages\boidflock\src\flock.cone` (stepBird).

## Language gaps found (as the world needs them)
Today's subset has no runtime code beyond derives (field/occupied/if), keys rules and pick-and-place. For boids:
1. No per-step rule: nothing runs every step with the step's length.
2. Functions take no parameters and hold no statements (`fn NAME() TYPE { let ...; EXPR }`, folded or vocabulary calls only).
3. No loop over other parts while the world runs (`each` is unrolled when the world is read).
4. No vector arithmetic at run time (points add, scale, length, dot).
5. No reading another part's place or a per-copy field at run time.
6. No per-copy mutable point state (velocity): fields are bool/number/part only.
7. No act that moves or turns a part from code (only keys/glide animators).
None of this is written into `Incubator\3D Web\WI\content-language.md` yet: that is left to do.

## What was added (all marked FOR REVIEW in the code: the language is Jon's)
Forms taken from the manual (animators.html "Your own animator: a step function", control-flow.html, fields.html):
- `field velocity: place = (x, y, z), mutable: true`: a place field, own to each copy (expand keeps it on copies).
- `on(created, start(stepAnimator(b, step: steer)))`: runs `steer(b, dt)` every step from the first (created only).
- `fn steer(b: part, dt: duration) bool { STMTS; EXPR }`: a step function. Statements: `let`, `var`, `NAME = EXPR;`,
  `if/else/else if`, `each o in P.parts { }` (runtime loop over P's parts), acts `set(p.place, v)`, `set(p.facing, v)`,
  `set(p.FIELD, v)`. Values: numbers (plain, m/cm/mm/s/ms), truths, points `(x,y,z)`, parts; `p.place`, `p.FIELD`,
  `v.x/.y/.z`, `+ - * /` on points and numbers, comparisons, `&& || !`, `length(v)`, `dot(a,b)`.
- Names appended to the standard table (lexer.cone, 175-181): var, place, stepAnimator, step, duration, length, dot.
- Lowering (`steplower.cone`, called from `lowerSim`): step fn = unit (part, f64) -> bool; stepper = unit (f64) -> bool,
  new entry kind `ENTRY_STEP = 3` (a = ordinal, b = part slot) calling it; place-field start = unit () -> (f64,f64,f64),
  entry ENTRY_FIXED (a = slot, b = field number). No new ops: points are 3-slot tuples, loops are block/loop/br_if
  with an i64 counter.
- Verifier rule 5a extended: ENTRY_STEP must be (f64) -> bool and may act; unknown kind now > ENTRY_STEP.
- Host imports (namespace host; Sim table in pegasus/src/simrules.cone, weight 1 each): `place(part)->point`,
  `place_field(part,i64)->point` (traps if the part lacks it), `child(part,i64)->part`, acting `set_place`,
  `set_place_field`, `set_facing`. Writes staged in handle 9 and applied after every stepper of the step has run
  (each reads the world as it stood at the step's start, as flock.cone's stepBird does).
- Sim: `SimRules.stepAll(step, stepNs)` and `stepReport()` (median/p95/max time per step, fuel per step and per run,
  runs stopped); `RuleHost.stepSetup/refreshPlaces/stepRun/stepWrites/setPlaceField`; `Slots.atOf/parentOf`.

New public names: steps.cone (ST_*, SX_*, SS_*, SExpr, SStmt, StepFn, StepperInfo, PlaceField, ENTRY_STEP, HOST_PLACE,
HOST_PLACE_FIELD, HOST_CHILD, HOST_SET_PLACE, HOST_SET_PLACE_FIELD, HOST_SET_FACING, the six *Signature fns,
stepperSignature, placeNumber); parser.cone (ParamDecl, Stmt, STMT_*; FnDecl.params/body); Understood.steps/steppers/
placeFields/placeNames; lexer N_VAR..N_DOT; simrules stepAll/stepReport; simslots atOf/parentOf.

## State
- conescript compiles (`build\congo.ps1 C:\src\pegasus3d-wt-boids\conescript build --release`, a helper here).
- pegasus NOT yet compiled with the simrules changes (written, not built). Expect small fixes: `self.hs.data[X] = list`
  may need `hs.data.set(X, list)`; `atan2/asin` may need f32 forms; `Instant` import in simrules.
- Not wired: simactor.cone needs `rules.stepAll(step, STEP_NS);` after `rules.animate` (line ~312) and
  `rules.stepReport();` beside `rules.profileReport(step)` in endRun (~337).
- Nothing run; no timings, no fuel numbers, no shot.

## Left
1. Build pegasus, fix compile errors, wire simactor.
2. Write `pegasus\worlds\boids.ddd`: a bird design (procgen shape: e.g. a lofted/lathed body + two flat wings in
   `fn form() shape`), `field velocity: place`, `part flock` with `each i in 0 ..< N { part b[i]: designs.bird(velocity: ...), at: ... }`,
   `each b in flock.parts { on(created, start(stepAnimator(b, step: steer))) }`, and `steer` with separation (<1.8 m,
   x3.5 / d2), alignment (mean velocity within 12 m, x1.0), cohesion (to centre, x0.9), a home leash, speed clamp,
   `set(b.velocity, v); set(b.place, b.place + v * dt); set(b.facing, v)`. Count parameter: a timing script that writes
   variants for 50/200/1000 (expand caps a loop at 10000).
3. Tests: `conescript\tests\steps.cone` driven checks (read text with Vocabulary.empty(), lowerSim, verify against a
   test table, run a stepper), a mutant each; congo test; checkhashes.py, checkworlds.py, conformance.py, --checks, press.
4. Timings at 50/200/1000 (PEGASUS_PROFILE_RULES for interpreter time), asset build time, Render frame time, one shot.
5. content-language.md gap list entry; actor notes "What exists today".

## Judgement calls
- One stepper per bird (from `each b in flock.parts`), not one for the flock: fuel is per run, so per-bird runs cost
  ~4 fuel x N each (about 4000 at 1000 birds, under RULE_FUEL 100000); one flock-wide run would need ~4M and be dropped.
- Step function mistakes go to `later` (lose the function and its steppers, not the part).
- `place` = at + offset in the holder's frame; `set(p.place, v)` sets offset = v - at.
- `facing` turns +z along v, top kept up (yaw then pitch); spelling borrowed from arrivals' `facing:`.
- Step functions fold with an empty env so a design field cannot shadow a local.
