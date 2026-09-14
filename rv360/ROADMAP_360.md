# Re-Volt Xbox 360 Port — Full Roadmap (XDK route)

Base: `rvsource/Xbox/Src/` (OG Xbox). Target: 360 XDK, Xenia + RGH test.
Rule: additive work in `rv360/` first; touch the leak only with `_XBOX360`-guarded includes.
Disk data + wire protocol are little-endian x86; Xenon is big-endian PPC.

## M0 — Foundation & hygiene ✅ DONE
- [x] Extract rar (official unrar, `rvsource/` 1.6G, All OK)
- [x] Inventory PC (`source/`) vs Xbox (`Xbox/Src`, `Xbox/FrontEndSrc`)
- [x] `rv360/README_360.md`, `rv360/rv360_endian.h`, `rv360/AUDIT_endian.md`
- Acceptance: tree builds untouched; docs present.

## M1 — Fixed-width types + endian-safe I/O (portable, no XDK) � eminent
- [x] `rv360_endian.h`: swap16/32/64, swapf, `rv_fread_u16le/u32le/f32le`, `rv_s32/u32/s16/u16` doctrine
- [x] `rv360/rv360_lefile.h`: LE readers for `MODEL_HEADER`, `MODEL_POLY_LOAD`, `MODEL_VERTEX_LOAD`, `FILE_FIELD`, `ONE_RECORD_ENTRY`, `NEWCOLLPOLYHDR`
- [x] gcc `-fsyntax-only` check on both headers from Linux
- [x] `long`→`rv_s32` hit-list for file/network structs (`rv360/LONG_HITLIST.md`)
- Acceptance: headers compile on PC; zero edits to game files yet.

## M2 — Asset loaders → explicit LE reads
Files: `source/model.cpp` (.prm/.m), `source/level.cpp` (.fld/.ncp/.cam), `source/light.cpp`, `source/NewColl.cpp`, `Xbox/Src/LevelLoad.cpp`, `Xbox/Src/texture.cpp`.
- [x] M2.1 model: `MODEL_HEADER` + per-poly/per-vert LE reads (`model.h:175-237`; fread sites `model.cpp:136,186,315,2408,2413,2421`)
  - Done 2026-09-13 in `Xbox/Src/model.cpp` (`LoadModel` + `SetModelFrames`): `_XBOX360`-guarded include of `rv360_endian.h`/`rv360_lefile.h`, LE header/poly/vert reads with identical PC path otherwise.
- [x] M2.2 level fields: `FILE_FIELD` loop + `LEVELINFO` floats/longs (`level.cpp:917,934`; `level.h:42-76`)
  - Done 2026-09-13 in `Xbox/Src/LevelLoad.cpp` (`LoadLevelFields`): LE `nFields` + per-field `rv_read_filefield` copy, PC path untouched.
- [x] M2.3 collision: `NEWCOLLPOLYHDR/NEWCOLLPOLY/COLLGRID_DATA` (`NewColl.cpp:906,921,982,1043,1099`)
  - Done 2026-09-13 in `Xbox/Src/newcoll.cpp` (_PC path): `LoadNewCollPolys` header+polys, `LoadGridInfo` grid data/counts/indices, `LoadConvex` hdrs/bbox/vec/faces, `LoadSpheres` count+spheres — all LE-guarded.
  - Plus `Xbox/Src/instance.cpp` (`LoadInstances`): `InstanceNum` + `FILE_INSTANCE` LE-guarded.
- [x] M2.3b world mesh: `Xbox/Src/world.cpp` world header/chunk/vert/boundary/texanim reads (~12 sites)
  - Done 2026-09-13 in `Xbox/Src/world.cpp` (`LoadWorld`): `WORLD_HEADER`/big-cube counts, `CUBE_HEADER_LOAD`, poly/vert via shared LE readers, big-cube idx, `TexAnimNum`/frames, env rgb — all LE-guarded.
- [x] M2.4 records/times: `ONE_RECORD_ENTRY/RECORD_ENTRY` (`level.h:42-53`)
  - Done 2026-09-13 in `Xbox/Src/timing.cpp`: `InitTrackRecords` / `SaveTrackTimes` read/write of `TrackRecords` and `record` wrapped in LE-guarded `rv360_read_recordentry` / `rv360_write_recordentry`.
- [x] M2.4b track data: AI nodes/zones, triggers, camera nodes, objects
  - Done 2026-09-13, all LE-guarded: `ainode.cpp` (`AiSingleNodeNum`/`AiLinkNodeNum`, `FILE_AINODE`, `AiStartNode`, `AiNodeTotalDist`), `aizone.cpp` (`AiZoneNum`, `FILE_ZONE`), `trigger.cpp` (`TriggerNum`, `FILE_TRIGGER`), `camera.cpp` (`LoadCameraNodes` count + `FILE_CAM_NODE`), `obj_init.cpp` (`LoadFileObjects` + `CountFileStars`, `FILE_OBJECT`).
- [x] M2.4c ghost + position nodes
  - Done 2026-09-13, LE-guarded: `ghost.cpp` (`GHOST_INFO`/`GHOST_DATA` read+write helpers, CRC field; `GHOST_HEADER` is pure char[] so byte-identical), `posnode.cpp` (`PosNodeNum`/`PosStartNode`/`PosTotalDist` + `FILE_POSNODE`).
- [x] M2.5 textures: `TEXINFO` pixel path → no endian work needed
  - Audited 2026-09-13: active path is `D3DXCreateTextureFromFileExA` (`texture.cpp` `LoadMipTexture`) + `LoadTextureGPU`/`XBResource`; D3DX parses the BMP container itself. The old DirectDraw `Lock`/`lpSurface` pixel loops are dead code inside `#ifndef XBOX_NOT_YET_IMPLEMENTED`. Tiling/swizzle is a 360 GPU-format task → deferred to M4.4, not an endian fix.
- Acceptance: on PC, LE-reader build loads every stock track/car bit-identical; on 360, no raw `fread(struct)` remains in these files.

## M3 — Network wire format (BE-safe) + Live plan
Files: `Xbox/Src/network.h/.cpp` (`MSG_TYPE/MSG_TYPE_EXT`, `pshpack1` structs:150-166, header set/get:172-176), `net_xonline.h`, `XBOnline.h`, voice/friends.
- [x] M3.1 wire-order policy decided + header routed through it
  - Done 2026-09-13 in `Xbox/Src/network.cpp`: `MSG_HEADER_TCP.Size` now goes through `RV360_TCP_SIZE_GET/SET` (4 sites: `SetSendMsgHeaderTcp`, `GetSingleMessageTCP`, 2x `ProcessMessage` dispatch).
  - `MSG_HEADER`/`MSG_HEADER_EXT` need no work: `TypeAndID`/`ExtendedType` are single bytes (`PackMsgTypeAndID` packs type+ID into one BYTE).
  - **Policy**: default = native byte order → 360↔360 system link works out of the box. `-DRV360_NET_LE_WIRE` = canonical LE for PC cross-play, which requires converting *every* field of *every* packet struct (they are blitted raw). Half-converting is worse than either.
- [ ] M3.2 (only if cross-play wanted) full LE serialize for `NET_PLAYER`, `NET_SESSION`, `RESTART_DATA`, `JOIN_INFO`, `RACE_TIME_INFO`, car/weapon/object payloads + `REMOTE_QUAT/VEL/ANGVEL_SCALE` quant paths
- [ ] M3.3 Replace `pshpack1/poppack` dependency with explicit `#pragma pack` + compile-time size asserts
- [ ] M3.4 360 Live map: `XNet + XSession` replace `XOnline OG + GAME_PORT 1000`; voice → GameChat; guard behind `RV360_LIVE` flag (system-link first)
- Acceptance: packet sizeof/layout table documented; 360↔360 system link parity.

## M4 — Renderer → D3D9/360 (COMPILING FOUNDATION; RUNTIME INCOMPLETE)
Files: `Xbox/Src/dx.h/.cpp` (macros 36-90, `DRAW_PRIM`), `draw.h/.cpp`, `drawobj.h/.cpp`, `texture.h/.cpp`, `XBResource.h/.cpp`, `XBUtil.h/.cpp`, `xdx.cpp`, `mirror/shadow/text`.
- [x] M4.1 compile bridge: sampler state moved to `D3DSAMP_*`; removed fixed-function states routed to shader/no-op equivalents
- [x] M4.2 compile bridge: `DRAW_PRIM(_INDEX)` routes through dynamic VBs/IBs and transformed VS/PS
- [ ] M4.3 buckets: `BUCKET_TEX0/TEX1/ENV`, `FlushPolyBuckets/Env/SemiList/NearClip` (`draw.h` XYZRHW `FVF_TEX0/1/2`) → batched draws
- [ ] M4.4 textures: `TPAGE_*`, `PickTextureFormat=A8R8G8B8`, `LoadTextureClever/Mip/GPU` + swizzle/`.xpr` → `XGSetTextureHeader` tiled; NPOT/square rules; fonts (`XBFont`, `TPAGE_FONT/SPRU/LOADING`)
- [ ] M4.5 frame: `FlipBuffers/ClearBuffers/SetGamma/InitD3D/BackBufferCount/BackgroundColor` → `Present/Clear/SetGammaRamp`, 720p + safe area
- Acceptance: front-end menu renders on Xenia; no `Draw*UP` left in game path.

## M5 — Audio → XAudio2/XMA (SFX FOUNDATION; MUSIC INCOMPLETE)
Files: `source/inc/mss.h`, `source/inc/sfx.h`, `Xbox/Src/sfx.h/.cpp`, `SoundEffectEngine.h/.cpp`, `soundbank.h/.cpp`, `MusicManager.h/.cpp`, `tools/SfxParse/`.
- [x] Compile bridge: DirectSound engine isolated; generated SDF IDs and PCM16/WAV/XWP XAudio2 path compile
- [ ] Real concurrent SFX voices, SDF variation/volume mapping, X3DAudio rolloff, completion callbacks
- [ ] `CMusicManager` (`XWmaFileMediaObject + IDirectSoundStream[2]`, `PACKET_*` thread) → XAudio2 streaming + XMA decode
- Acceptance: menu SFX + one streamed track; volumes/pans sane.

## M6 — Input → XInput 360 (COMPILES; RUNTIME UNVALIDATED)
Files: `Xbox/Src/XBInput.h/.cpp`, `gamepad.h/.cpp`, `input.h/.cpp`, `ctrlread.h/.cpp`, `control.h/.cpp`.
- [x] OG input shape mapped from 360 user-index XInput (A/B/X/Y, shoulders, triggers, sticks, pressed/repeat state)
- [ ] Rumble and target-runtime validation
- [ ] Keep `KEY/CTRL/FUNCTION_KEY/KeyTable`, `CRD_LocalInput/CON_*` untouched
- Acceptance: car steers/throttle/brake + menu nav on pad 0 in Xenia.

## M7 — System, paths, front-end
Files: `Xbox/Src/main.h/.cpp`, `gameloop.h/.cpp`, `timing.h/.cpp`, `XBUtil.cpp Timer`, `load.h/.cpp` (`BKK_fopen`), `Content.h/.cpp`, `settings.h`, `FrontEndSrc/` vs `Src/` choice.
- [ ] `BKK_fopen` + `D:\CARS\...` absolutes → `game:\` + content-package enum (`content_packages/.../packagefiles/*.prm,*.bmq,Parameters.txt,subscription.txt,metadata.xbx`)
- [ ] `GAMETYPE_*` (Xbox extended list) + `GameSettings.GameType/MultiType` wiring; pick `Src/` as authoritative game, `FrontEndSrc/` only if keeping OG flow
- [ ] Timing `TIMER_*/XBUtil_Timer/BackgroundColor/BackBufferCount/present interval` → `XTime/QueryPerformanceCounter`, 60Hz/HDTV modes
- Acceptance: cold boot → front-end → load one track offline, no network required.

## M8 — Bring-up & validation (needs XDK + Xenia/RGH)
- [ ] XDK project from `revolt_xbox.dsw`/`revolt_src.dsp` file lists; `/W3`, no `/GX`, PPC-safe warnings-as-errors pass on game code
- [ ] Boot checklist: menu → single race (2 cars, 1 lap) → finish → no crash; log `BKK_fopen` misses
- [ ] Perf: buckets batched, texture tiling validated, 30fps floor at 720p; voice/Live explicitly deferred
- [ ] Regression: PC LE-reader build still bit-identical loads
- Acceptance: playable offline single race on hardware/emulator.

## Work log
- 2026-09-13: M0 done. M1 started (`rv360_endian.h` + audit). Next: `rv360_lefile.h` + gcc check, then M2.1.
- 2026-09-13: **M1 + M2 complete.** Every runtime asset loader now reads little-endian explicitly under `_XBOX360`, PC/OG-Xbox paths byte-for-byte unchanged (`#else` branches untouched).
  - Files touched: `model.cpp`, `LevelLoad.cpp`, `newcoll.cpp`, `instance.cpp`, `world.cpp`, `timing.cpp`, `ainode.cpp`, `aizone.cpp`, `trigger.cpp`, `camera.cpp`, `obj_init.cpp`, `ghost.cpp`, `posnode.cpp`, `network.cpp`.
  - Readers/writers live in `rv360/rv360_lefile.h` (+ primitives in `rv360_endian.h`), gcc `-Wall -Wextra -fsyntax-only` clean on every change.
  - M2.5 audited as no-op (D3DX parses BMPs; old DDraw pixel loops are dead code) — tiling deferred to M4.4.
  - M3.1 done; M3.2+ gated on the cross-play decision above.
- **M4.1 audit complete.** Added `RENDERER_M4_1.md` and `rv360_render_api.h`. Render states are mostly portable; sampler states must move from `D3DTSS_*` to `D3DSAMP_*`; color-key state needs alpha-texture replacement. The new boundary is XDK-independent and syntax-checks clean.
- **M4.2 designed.** Added `RENDERER_M4_2.md`. All roughly 50 game draw sites funnel through `DRAW_PRIM` / `DRAW_PRIM_INDEX`, so migration can remain localized to `dx.h` plus the backend. Implementation is blocked by missing XDK development headers/libs.
- 2026-09-14: **XDK development payload restored.** `XDKSetupXenon7645.exe` contained the complete `XDK/include`, `XDK/lib`, and compiler payload, but the Wine installer deployed only its Minimal/tools profile. Extracted the embedded `XDK/` tree into the existing `.wine32` SDK directory. Verified `xtl.h`, `xgraphics.h`, `d3d9.h`, `xaudio2.h`, `xonline.h`, Xbox libraries, and Xenon `cl.exe` (PowerPC compiler version 14.00.7151). M4.2 is no longer environment-blocked.
- 2026-09-14: Added `XDK_BUILD.md` and `xdk_probe.cpp`. The Xenon compiler successfully compiles the probe with `/D_XBOX /D_M_PPC /D_XBOX360`. First `dx.cpp` compile reaches expected OG API forks (`IDirect3DDevice3`, `DDPIXELFORMAT`, D3D8 resource types, DirectSound), confirming the toolchain is functional.
- 2026-09-14: First source compile also confirmed the OG `dx.cpp` must not be forced through D3D9 with aliases. Its D3D8 fixed-function state calls and viewport/capability structures are incompatible. M4.2 now uses a dedicated `rv360/dx360_backend.cpp`; compatibility aliases remain limited to resource metadata and omitted sound headers are explicit stubs until M5.
- 2026-09-14: Added `rv360/dx360_backend.h/.cpp` with XDK-verified render-state, sampler, texture, clear, and present wrappers. The backend compiles with the Xenon compiler; it is not wired into game rendering yet.
- 2026-09-14: Added dynamic vertex/index upload buffers to `dx360_backend.cpp`, verified with the Xenon compiler. `_XBOX360` `DRAW_PRIM` and `DRAW_PRIM_INDEX` now route through the upload backend; OG Xbox macros remain unchanged. Shader/declaration binding is the next renderer subtask.
- 2026-09-14: The upload backend now allocates/reuses 1 MiB vertex and 256 KiB 16-bit index buffers, locks/copies caller data, binds stream/index buffers, and issues Xenon `D3DDevice_DrawVertices`/`DrawIndexedVertices` calls. Backend compilation remains clean after the macro integration.
- 2026-09-14: Added `_XBOX360` XInput user-index polling to `gamepad.cpp`; OG `XInputOpen` device handling remains in the non-360 branch. Xenon compilation passes for the gamepad unit.
- 2026-09-14: Added and XeDK-compiled `rv360/shaders/transformed.hlsl` (`vs_3_0` and `ps_3_0`) plus checked-in `.xvu` intermediates. The shader pair preserves software-transformed screen coordinates, colors, and two UV sets; backend resource binding remains next.
- 2026-09-14: Added XDK-compiled transformed vertex declaration and shader lifecycle to `dx360_backend.cpp` (`rv360_bind_transformed_pipeline` / `rv360_unbind_transformed_pipeline`). The backend now has a concrete position/color/specular/UV declaration and binds compiled VS/PS objects.
- 2026-09-14: Added XDK-compiled texture and sampler helpers to `dx360_backend.cpp`: `rv360_load_texture_file` uses D3DX9 with `A8R8G8B8`, and `rv360_set_sampler_linear` uses `D3DSAMP_*` state. The old D3D8 texture loader remains isolated until integration.
- 2026-09-14: Added `audio360_backend.h/.cpp` with XDK-compiled XAudio2 initialization, mastering voice, PCM16 source voice playback, loop control, stop, and shutdown. Generated sound-bank replacement and lifetime-safe streaming remain M5 asset work.
- 2026-09-14: Added an `_XBOX360` implementation branch to `SoundEffectEngine.cpp`, replacing the DirectSound-dependent OG implementation with the XAudio2 lifecycle/instance interface. The unit now compiles with the Xenon compiler; SDF/XWP sample decoding and per-effect PCM submission remain the next audio integration step.
- 2026-09-14: Finished the source-level M5 bank path: `audio360_backend` now parses `.xwp` manifests, resolves relative WAV entries, caches PCM16 effects, and dispatches effect IDs through XAudio2. `SoundEffectEngine::LoadSounds` converts legacy `.sfx` paths to `.xwp`, and `Play2DSound` dispatches the loaded effect. Audio and sound-engine units compile cleanly with XeDK.
- 2026-09-14: Xenon compile audit: `Xbox/Src/load.cpp` now compiles cleanly under `_XBOX360`; `main.cpp` reaches the next expected OG utility/UI forks (`XBUtil.h` D3D8 helper types, online constants, and legacy player declarations).
- 2026-09-14: Generated 17 per-level sound enum headers from the included `.sdf` definitions under `rv360/generated_sounds/` and switched the 360 sound engine to use them. Sound engine compilation now passes; sample-bank decoding remains separate from enum generation.
- 2026-09-14: Added repeatable `rv360/check_xenon.sh`; release and debug checks pass for renderer/audio/music/voice/online bridge units. Fixed PCM ownership on failed voice creation and made unsupported music/voice output deterministic.
- 2026-09-14: `dx.cpp` now compiles with 360 presentation/viewport/gamma/state handling and initializes `dx360_backend`. Legacy `network.cpp` is explicitly excluded on 360; `network360_stub.cpp` supplies offline single-player globals/functions. Multiplayer remains a separate system-link rewrite.
