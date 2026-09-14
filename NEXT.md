# Re-Volt 360 — Execution Directive (supersedes ad-hoc task selection)

Status date: 2026-09-15. This document is the authoritative task queue.
`ROADMAP_360.md` remains the historical record and milestone map; where the two
disagree about *what to do next*, this file wins.

---

## 1. Situation assessment

### What is genuinely done

- XDK 7645 environment is functional: Xenon `cl.exe`, `link.exe`, `imagexex`,
  headers and libs, all verified under Wine.
- 132/132 `rvsource/Xbox/Src/*.cpp` translation units compile under
  `/D_XBOX /D_M_PPC /D_XBOX360`.
- 136 objects link with no unresolved symbols; `default.xex` packages; image
  header inspects clean at base `0x82000000`.
- M1/M2 endian work is real and correctly `_XBOX360`-guarded. PC/OG paths are
  byte-for-byte untouched in the `#else` branches.
- M4.1/M4.2 boundary work is real: `DRAW_PRIM`/`DRAW_PRIM_INDEX` (50 call sites)
  route through `rv360/dx360_backend.cpp` dynamic VB/IB upload. Shader pair
  compiled to `.xvu`. Vertex declaration bound.
- M5 source-level path: XAudio2 init, mastering voice, PCM16 playback, `.xwp`
  manifest parse, 17 generated per-level sound enum headers.
- M6 source-level path: XInput user-index polling in `gamepad.cpp`.
- N0 is observed: `rv360/build_xenon.sh` produces `build/rgh-current/Revolt/default.xex`
  plus `rv360/shaders/*.xvu`; the current cached/parallel build completed 136/136
  units in 28.6 seconds on the development host.

### What is not done, stated plainly

**Nothing in this repository has ever executed.** No emulator boot, no hardware
boot, zero runtime observations. Every claim above is a claim about compilation.

Compile-cleanliness was reached in part by stubbing. The three that determine
whether a boot produces anything visible:

| Stub | Location | Consequence |
|---|---|---|
| `CXBFont` 360 branch | `XBFont.cpp:46-82` | All `DrawText` overloads `return S_OK` and draw nothing. **260 call sites.** No text renders anywhere in the game. |
| `CXBPackedResource` | `XBResource.cpp:18-64` | `Create`/`PatchAll` return `E_NOTIMPL`, `GetData` returns `NULL`. `font.xpr`, `resource.xpr`, `gamepad.xpr` load nothing. |
| Network / Live / DLC | `rv360/network360_stub.cpp`, `rv360/offline_ui_link_stubs.cpp` | Intentional and correct for the offline target. Not a defect. |

Therefore: a *perfect* boot today yields a black or single-colour screen with no
text. This is the expected state, but it means the `RGH_TEST.md` goal of
"reaches the title/menu" **cannot be satisfied by the current build** regardless
of how well the XEX loads.

### The under-weighted blocker: path mapping is 1% complete

`RV360_MapPath` (`load.cpp:21-35`) maps `D:\` / `T:\` / `N:\` to `game:\`, and
is invoked **only** from `BKK_fopen`. Everything else opens raw legacy paths.

Measured in this tree:

- **482** `D:\` string literals under `rvsource/Xbox/Src/`
- **141** direct `fopen` / `CreateFile` / `_open` sites outside `load.cpp`
- **176** `.bmp` literals

Known runtime-path offenders that bypass `BKK_fopen` entirely:

- `texture.cpp:846` — `D3DXCreateTextureFromFileExA(D3Ddevice, tex, ...)` where
  `tex` is a legacy path from the texture tables. **This is the main game
  texture loader.**
- `draw.cpp:1340` — same call, runtime texture load
- `XBUtil.cpp:222` — `D3DXCreateTextureFromFileEx`
- `ui_Help.cpp:174` — hardcoded `"D:\\gfx\\fxpage2.bmp"`
- `ui_SelectCar.cpp:591` — hardcoded `"D:\\cars\\ufo\\carbox.bmp"`
- `LevelInfo.cpp:48+` — the entire level path table (`"D:\\levels\\NHood1"` …)

**Every texture load on first boot will fail.** This is mechanical work and the
highest value-per-token task remaining.

### Known defect to fix before the first console trip

`build_xenon.sh` copies the `.xvu` shaders to `$build/package/rv360/shaders/`.
`RGH_TEST.md` instructs the operator to copy `build/rgh-current/`, which the
script never creates. `dx360_backend.cpp:84-85` loads from
`game:\rv360\shaders\transformed_vs.xvu` / `transformed_ps.xvu`. If the deployed
layout does not place the shaders at `<xex dir>\rv360\shaders\`, shader binding
fails at init and the cause will look like a renderer bug. Fix the script/doc
mismatch as part of N0.

---

## 2. Strategic directive

**Stop breadth-first porting. Optimise for runtime evidence per console trip.**

The project has saturated "does it compile" as a metric. Continuing to add
compile-clean subsystems produces unvalidated work with an unknown redo rate.
The scarce resources now are (a) human time per hardware boot and (b) agent
tokens spent on rebuild/rediscovery loops — not source-porting throughput.

Consequences, in priority order:

1. Make the build loop cheap before doing more porting work.
2. Build an information channel (logging) before building more features.
3. Get *something* on screen on real hardware, then let observed failures order
   all subsequent work.
4. Never batch more than one console trip's worth of speculative fixes.

---

## 3. Standing rules (non-negotiable)

**R1 — Additive-first.** New 360 code lives in `rv360/`. The leak is touched only
with `_XBOX360`-guarded branches. `#else` branches are never modified. This rule
has been followed well so far; it does not relax.

**R2 — "Done" means observed, never compiled.** A task is complete when a named
line appears in a log captured from hardware, or a named thing is visible on
screen. "It compiles" is a checkpoint, not an acceptance criterion. This rule
exists because M2/M5 were previously declared complete on compile success alone.

**R3 — Work log is mandatory.** Every session ends by appending a dated entry to
`ROADMAP_360.md` and updating the status block in this file. No exceptions. This
is the single mechanism that keeps a fresh session cheap; skipping it to save
tokens costs an order of magnitude more on the next session.

**R4 — No log spam into context.** Never read a raw build log. Use the filtered
forms in §7. Never `cat` a file over 500 lines — `grep -n` for the symbol.

**R5 — One milestone per session.** Finish, log, exit. Do not continue into the
next milestone in a compacted context.

**R6 — Escalate, don't decide.** The one-way doors in §4 are human decisions.
If a task requires one, stop and write the question into the work log.

**R7 — No invented ABIs.** Precedent already set correctly in this repo
(`_controlfp` over a fabricated `_control87`). If an XDK symbol's shape is
unknown, stop; do not guess a signature to satisfy the linker. `/FORCE` is
banned.

**R8 — Commit per unit.** A failed experiment is `git reset`, not a debugging
session.

---

## 4. Human-only decisions (agent must stop and ask)

These are one-way doors. An agent choosing one of these wrong costs days.

1. **XPR vs loose files.** Recook `.xpr` bundles for 360 tiling, or abandon XPR
   and load loose `.bmp`/`.dds` through D3DX? Affects N4 entirely.
2. **Fixed 720p vs video-mode enumeration.** Affects `dx.cpp` init, safe area,
   and every UI coordinate.
3. **Offline permanently vs system link later.** Determines whether M3.2/M3.3
   serialisation work is ever justified.
4. **Texture cooker: offline tool vs runtime conversion.** `xdx.cpp` is the
   reference cooker; a 360 equivalent is a separate build target if chosen.
5. **`Src/` vs `FrontEndSrc/` as authoritative front end.** Still unresolved in
   M7 and blocks the menu work.

---

## 5. Model tiering

Token budget is wasted mostly on using a strong model for mechanical edits and a
weak model for architecture. Tag every task before starting it.

| Tier | Use for | Tasks in this directive |
|---|---|---|
| **Cheap** | Mechanical, pattern-repetitive, verifiable by compile | N0 (script work), N3 (path rewrite), N4.1 (literal sweep) |
| **Strong** | Design decisions, unfamiliar API surfaces, debugging without a stack trace | N1, N4.2 (font renderer), N5, all first-boot failure triage |

Never use a strong model to apply a sweep. Never use a cheap model to diagnose a
hardware failure with no symbols.

---

## 6. Task queue

### N0 — Make the build loop cheap · Tier: Cheap · 1 session

**Status: DONE 2026-09-15.**

**Why first:** `check_xenon.sh` does `mkdir "$out"` and deliberately refuses
reuse, then loops `cl.exe` serially under Wine. A one-line change triggers a full
136-object serial rebuild. Every subsequent task in this directive pays that
cost. This is not a porting task and it is the largest single cost lever
remaining.

Tasks:

- Add content-hash-based object caching (hash of preprocessed source + flag set).
  Reuse objects on match; recompile on miss. Keep the "refuse stale objects in a
  link" safety property — cache validity must be provable, not assumed by mtime
  alone.
- Parallelise compilation (`xargs -P` over the TU list). Wine tolerates parallel
  `cl.exe` instances; verify the object dir has no write contention.
- Add a `--changed-only` path that relinks without recompiling untouched TUs.
- Fix the deployment mismatch: `build_xenon.sh` must produce
  `build/rgh-current/` with `default.xex` (named per `RGH_TEST.md`) **and**
  `rv360/shaders/*.xvu` in the exact relative layout that
  `dx360_backend.cpp:84-85` expects. Update `RGH_TEST.md` if the name changes.

Acceptance observed: cached/parallel rebuild produced a fresh XEX in 28.6
seconds, and `build/rgh-current/Revolt/` contains the XEX plus shaders at the
exact relative path `rv360/shaders/*.xvu`.

---

### N1 — Bring-up mode and the logging channel · Tier: Strong · 1–2 sessions

**Why:** This is the information channel. Without it, every console trip returns
one bit of data ("black screen") and the next task is guesswork.

**N1.1 — Log sink.** `WriteLogEntry` already exists and `BKK_fopen` already logs
`Loading: <path>: Found|Not Found` for every file. That is precisely the
telemetry needed; it just has to survive to somewhere readable.

- Route `WriteLogEntry` under `_XBOX360` to both `DbgPrint`/`OutputDebugString`
  and an append-mode file on a writable device.
- Resolve the writable path explicitly — the XEX directory may be read-only
  depending on launch method. Determine the correct writable root and log which
  one succeeded as the first line of output.
- Flush after every entry. A crash must not lose the buffer; an unflushed log
  from a hard hang is worthless.
- Log a boot banner: build id, screen mode actually obtained, D3D init HRESULT,
  shader load HRESULT, XInput pad-0 presence.

**N1.2 — Bring-up gate.** Add `RV360_BRINGUP` (compile-time define, default on
for now) that short-circuits `main` after device init and runs a minimal loop:

- Clear to a known non-black colour (proves present + video mode).
- Draw one untextured quad via `DRAW_PRIM` (proves the M4.2 upload backend,
  vertex declaration, and shader binding).
- Draw one textured quad from a known-good loose `.bmp` (proves file root,
  path mapping, and `D3DXCreateTextureFromFileExA` on 360).
- Poll pad 0; change the clear colour while A is held (proves XInput and that
  the loop is live rather than hung on frame 1).
- Log one line per frame for the first 10 frames, then every 60th.

Acceptance: `default.xex` + assets deployed; on hardware the screen shows the
clear colour, the textured quad, and responds to A; the log file is retrievable
and contains the boot banner. **If any stage fails, that failure is the N2 task
list** — that is a successful N1, not a failed one.

---

### N2 — First console trip and triage · Tier: Strong · human-gated

Operator runs the checklist in `RGH_TEST.md`, captures the log file and a photo.
Agent's job is triage only:

- Record verbatim results in the work log under a `## Runtime observations`
  heading. This section is append-only and never edited afterward.
- Convert every failure into a specific task appended to this queue.
- Do not fix and re-deploy one item at a time. Batch all fixes derivable from a
  single trip's evidence, then deploy once.

**Expected first failures**, in likelihood order — pre-reading these is cheaper
than discovering them: shader `.xvu` not found at the expected relative path;
writable-log-path failure (silent, looks like a hang); wrong/unset video mode;
`D3DXCreateTextureFromFileExA` failing on a legacy `D:\` path; vertex
declaration mismatch against the `.xvu` inputs producing a blank or garbage quad.

---

### N3 — Path unification · Tier: Cheap · 1 session

Driven by the `Not Found` list captured in N2, not applied speculatively to all
482 literals.

- Promote `RV360_MapPath` out of `load.cpp` into `rv360/rv360_path.h` as
  `RV360_Path()`, keeping `BKK_fopen` behaviour identical.
- Apply at every file-touching API on the runtime path, not just `fopen`:
  `texture.cpp:846`, `draw.cpp:1340`, `XBUtil.cpp:222`, `ui_Help.cpp:174`,
  `ui_SelectCar.cpp:591`, and any `CreateFile` site reached from level load.
- Leave `Edit*.cpp` and offline tool paths alone — they are not in the runtime
  path and touching them is pure cost.
- Add a debug counter: number of path mappings applied, number of opens that
  still failed. Log at level-load completion.

Acceptance: log from a subsequent boot shows zero `Not Found` entries for the
front-end asset set.

---

### N4 — Resources and font · Tier: mixed · 2–3 sessions

**Blocked on human decision #1 (XPR vs loose files).** Do not start until
resolved.

**N4.1 — Resource path** (Cheap, if "loose files" is chosen): replace
`CXBPackedResource` consumers with direct D3DX loads of loose files. If "recook
XPR" is chosen instead, this becomes a Strong-tier cooker task modelled on
`xdx.cpp` and should be scheduled as its own milestone.

**N4.2 — Font renderer** (Strong): implement `CXBFont::DrawText` against the
existing transformed pipeline in `dx360_backend.cpp`. The OG implementation
(`XBFont.cpp:113-750`) is the behavioural reference for glyph metrics, scale,
and slant — reimplement its output, do not port its D3D8 vertex-buffer
mechanics. Preserve all 260 call-site signatures unchanged.

Acceptance: title screen renders readable text on hardware; a photo is attached
to the work log.

---

### N5 — Renderer completion (M4.3 / M4.4 / M4.5) · Tier: Strong

**Ordered by N2/N4 evidence, not by roadmap numbering.** Do not start before the
menu is visible — bucket batching and texture tiling are performance and
correctness work whose bugs are indistinguishable from bring-up bugs when
nothing renders at all.

Carried forward from `ROADMAP_360.md`: bucket batching
(`FlushPolyBuckets/Env/SemiList/NearClip`), texture format/tiling via
`XGSetTextureHeader`, NPOT/square rules, `Present`/`Clear`/`SetGammaRamp`, 720p
and safe area. Colour-key removal already has a documented alpha-texture
replacement in `RENDERER_M4_1.md` — implement that, do not emulate fixed-function
colour key.

---

### N6 — First race · Tier: Strong

Boot → front end → single race, 2 cars, 1 lap, offline. Acceptance is the
existing M8 checklist, with the addition that the log must contain zero
`Not Found` entries and zero failed texture loads for the duration.

---

## 7. Session protocol

**Start of every session** — read only these, in order:

1. This file, §1 (status) and §6 (task queue)
2. The last 3 entries of the work log in `ROADMAP_360.md`
3. The `## Runtime observations` section, if it exists

Do not re-explore the source tree to establish context. If the above three are
insufficient, that is a defect in the documents — fix the documents as part of
the session.

**Build commands** (never read raw logs):

```bash
# Compile check, filtered
bash rv360/check_xenon.sh 2>&1 | grep -E '^(FAIL|[0-9]+ checked)'

# Errors only, capped
grep -hE 'error C[0-9]+' "$RV360_OBJECT_DIR"/*.log | sort -u | head -40

# Full image
bash rv360/build_xenon.sh 2>&1 | tail -5
```

**Fix batching:** when a compile pass fails, collect all unique errors first,
group by file, then fix the whole group and rebuild once. Never rebuild per
error.

**End of every session** — mandatory:

1. Append a dated entry to the `## Work log` in `ROADMAP_360.md`, stating what
   was *observed*, not what was attempted.
2. Update §1 of this file if status changed.
3. Update §6 if tasks were added, removed, or reordered.
4. Commit.
5. Exit. Do not begin the next task.

---

## 8. Anti-patterns — do not do these

- Declaring a milestone complete on compile success (see R2).
- Adding a new subsystem while an earlier one has never been observed running.
- Aliasing OG D3D8/DirectDraw types to D3D9 equivalents to make code compile.
  Precedent: `dx.cpp` was correctly excluded rather than forced through aliases.
- Speculatively rewriting all 482 `D:\` literals before the log says which ones
  matter.
- Chasing Xenia-specific failures. Xenia is a convenience, not the target. RGH
  hardware is the acceptance environment; if the two disagree, hardware wins and
  the Xenia result is discarded, not debugged.
- Implementing Live, voice, DLC, or system link. Explicitly out of scope until
  human decision #3 says otherwise.
- Deleting or rewriting the work log to save context. It is the asset.
