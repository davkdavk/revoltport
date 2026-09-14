# Re-Volt Xbox 360 port — plan (XDK route)

Target: Xbox 360 XDK build, tested on Xenia + RGH hardware.
Base: `rvsource/Xbox/Src/` (OG Xbox port), not PC `source/` — D3D macros already abstracted.

## Phases
1. **Foundation (portable, no XDK needed)** — fixed-width types, endian-safe I/O helpers, audit asset/network structs. [DOING NOW]
2. **Renderer** — `dx.h/dx.cpp` `D3DDevice_*` UP-calls → D3D9 vertex buffers + decls + tiny VS/PS; `D3DTSS_*` → `D3DSAMP_*`; `Flip/Clear/Gamma` → `Present/Clear/SetGammaRamp`; `texture.cpp/XBResource` → tiled `D3DFMT_A8R8G8B8` + `XGSetTextureHeader`.
3. **Audio** — `SoundEffectEngine/soundbank` → XAudio2 voices, `MusicManager` XWma stream → XMA, keep `SfxParse` IDs.
4. **Input** — `XBInput/gamepad.h` OG `BLACK/WHITE` pads → 360 `XInputGetState/SetState`, keep `KEY/CTRL` layer so `CON_*` is untouched.
5. **Data/endian** — byteswap every `fread(struct)` site (model/level/collision) + `network.h` `pshpack1` packets for big-endian Xenon; fix `long`/`__int64`/`VISIMASK` sizes.
6. **System** — `main/gameloop/timing` → 360 `XTime`, `load.h/Content` `D:\` paths → `game:\`, DLC enum, Live (`XSession`) behind a flag.
7. **Boot milestone** — front-end menu on screen, then gameplay.

## Rules
- Never edit the leak in place for experiments — new 360 shims live in `rv360/`, wired in only when compiling under `_XBOX360`.
- Keep `SET_RENDER_STATE / SET_TEXTURE / DRAW_PRIM` macro names so game code doesn't churn.
