# AGENTS.md — Re-Volt Xbox 360 port

Read this first, every session. Then read `rv360/NEXT.md` §1 and §6, and the last
three work-log entries in `rv360/ROADMAP_360.md`. That is your full context. Do
not explore the source tree to orient yourself.

## Project

Porting Re-Volt (OG Xbox source at `rvsource/Xbox/Src/`) to Xbox 360 via the
official XDK 7645 toolchain under Wine. Target is offline single-player on RGH
hardware. `rvsource/source/` is the PC tree, kept for reference only.

The build compiles and links. **Nothing has ever been run.** Treat every
subsystem as unvalidated until a log line from hardware says otherwise.

## Hard rules

1. **Additive-first.** New 360 code goes in `rv360/`. The leak is modified only
   inside `#ifdef _XBOX360` branches. Never modify an `#else` branch — the PC and
   OG Xbox paths must stay byte-for-byte identical.
2. **Done means observed.** A task is complete when a named log line appears from
   hardware or something is visible on screen. Compiling is a checkpoint, not
   completion. Do not write "complete" in the work log for a compile result.
3. **No invented ABIs.** If an XDK symbol's signature is unknown, stop and ask.
   Never fabricate an implementation to satisfy the linker. `/FORCE` is banned.
4. **No guessing at one-way doors.** See `NEXT.md` §4. If a task needs one of
   those decisions, stop and write the question into the work log.
5. **Work log every session.** Append a dated entry to `## Work log` in
   `ROADMAP_360.md` before exiting. State what was observed, not what was
   attempted. This is mandatory and is never skipped to save tokens.
6. **One milestone per session.** Finish it, log it, exit. Do not roll into the
   next task in a compacted context — start a fresh session instead.
7. **Commit per unit of work.** Small commits; a failed experiment should be a
   `git reset`, not a debugging session.

## Context discipline

- Never read a raw build log. Use the filtered commands below.
- Never `cat` a file over 500 lines. `grep -n` for the symbol, then read a
  window around it.
- When a compile pass fails, collect **all** unique errors, group by file, fix
  the whole group, rebuild **once**. Never rebuild per error.
- Do not re-derive facts already written in `NEXT.md` §1. If something there is
  wrong, fix the document rather than working around it.

## Build

```bash
# Compile check (filtered — do not read the raw logs)
bash rv360/check_xenon.sh 2>&1 | grep -E '^(FAIL|[0-9]+ checked)'

# Unique errors only, capped
grep -hE 'error C[0-9]+' "$RV360_OBJECT_DIR"/*.log | sort -u | head -40

# Full development image
bash rv360/build_xenon.sh 2>&1 | tail -5
```

Compiler defines: `/D_XBOX /D_M_PPC /D_XBOX360`. `/X` is used deliberately to
prevent host header contamination — do not remove it.

Deployment for hardware: `build/rgh-current/` must contain the XEX **and**
`rv360/shaders/*.xvu` at the relative path `dx360_backend.cpp` loads from
(`game:\rv360\shaders\`). A shader-load failure here is indistinguishable from a
renderer bug — verify the layout before blaming the renderer.

## Out of scope

Xbox Live, voice chat, DLC/content packages, system link, multiplayer. These are
stubbed in `rv360/network360_stub.cpp` and `rv360/offline_ui_link_stubs.cpp` by
deliberate decision. Do not implement them. Do not "improve" the stubs.

Xenia is a convenience, not the target. If Xenia and RGH hardware disagree,
hardware wins and the Xenia result is discarded rather than debugged.

## Facts worth not rediscovering

- Disk data and wire format are little-endian x86; Xenon is big-endian PPC. All
  asset loaders already read explicit LE under `_XBOX360` (M1/M2 complete).
- `CXBFont` under `_XBOX360` is a no-op stub — 260 `DrawText` call sites render
  nothing. `CXBPackedResource` returns `E_NOTIMPL`.
- `RV360_MapPath` (`load.cpp:21-35`) maps legacy roots to `game:\` but is called
  only from `BKK_fopen`. 482 `D:\` literals and 141 direct file-open sites
  bypass it, including the main texture loader at `texture.cpp:846`.
- `dx.cpp` is compiled on 360 but the OG D3D8 fixed-function device path is not;
  the 360 renderer lives in `rv360/dx360_backend.cpp`.
- `_controlfp` is used instead of a fabricated `_control87`. That is the correct
  pattern for any similar gap.
