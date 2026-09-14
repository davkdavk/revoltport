# Renderer M4.1 Audit

## Keep

These OG-Xbox render states have direct D3D9/360 equivalents:

- `D3DRS_FOGENABLE`, `FOGCOLOR`, `FOGSTART`, `FOGEND`
- `D3DRS_ALPHABLENDENABLE`, `SRCBLEND`, `DESTBLEND`, `ALPHATESTENABLE`, `ALPHAREF`, `ALPHAFUNC`
- `D3DRS_ZENABLE`, `ZWRITEENABLE`, `ZFUNC`
- `D3DRS_CULLMODE`, `FILLMODE`, `DITHERENABLE`, `SPECULARENABLE`
- `D3DRS_COLORWRITEENABLE`, `BLENDOP`

## Move To Samplers

The following `D3DTSS_*` values are texture-sampler state in the 360 path:

| OG Xbox | 360 target |
|---|---|
| `D3DTSS_ADDRESSU/V/W` | `D3DSAMP_ADDRESSU/V/W` |
| `D3DTSS_MINFILTER` | `D3DSAMP_MINFILTER` |
| `D3DTSS_MAGFILTER` | `D3DSAMP_MAGFILTER` |
| `D3DTSS_MIPFILTER` | `D3DSAMP_MIPFILTER` |
| `D3DTSS_MIPMAPLODBIAS` | `D3DSAMP_MIPMAPLODBIAS` |
| `D3DTSS_MAXANISOTROPY` | `D3DSAMP_MAXANISOTROPY` |

The existing `SET_STAGE_STATE` macro should not be globally renamed: color/alpha combiner state and sampler state have different APIs. Add `SET_SAMPLER_STATE(stage, state, value)` and migrate only the sampler macros.

## Remove Or Replace

- `D3DTSS_COLORKEYOP` and `D3DTSS_COLORKEYCOLOR`: no D3D9 sampler equivalent. Replace color-key textures with alpha in the texture cooker, then use ordinary alpha-test/blend state.
- `D3DRS_TRANSLUCENTSORTINDEPENDENT`: no portable D3D9 equivalent. Treat as a no-op initially; preserve the existing bucket ordering.
- `D3DRS_ANTIALIAS`: replace with the 360 multisample render-target configuration, not a per-draw render state.
- `D3DRS_TEXTUREPERSPECTIVE`: perspective correction is always enabled.
- `D3DTSS_ALPHAOP` in `BLEND_ON/OFF/ALPHA`: replace with pixel-shader alpha behavior. The current fixed-function emulation must not be copied blindly.

## First Code Boundary

`rv360/rv360_render_api.h` defines semantic operations without requiring XDK headers. The eventual XDK implementation should provide:

- `rv360_set_render_state`
- `rv360_set_sampler_state`
- `rv360_set_texture`
- `rv360_draw_vertices`
- `rv360_draw_indexed_vertices`

No game source is switched to this boundary until the XDK header names and signatures are confirmed.

## Acceptance

- No new 360 code uses `D3DTSS_*` for sampler filtering/addressing.
- Color-key behavior has a documented alpha-texture replacement.
- `DRAW_PRIM` remains a separate M4.2 task because it currently relies on `DrawVerticesUP`.
