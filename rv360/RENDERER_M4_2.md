# Renderer M4.2 Plan

## Existing Contract

The game performs software transforms and submits screen-space vertices:

- `VERTEX_TEX0/1/2` use `x,y,z,rhw`, diffuse color, specular/fog color, and one or two UV sets.
- `DRAW_PRIM` is used for triangle fans, line lists, and point lists.
- `DRAW_PRIM_INDEX` is used for bucketed triangle lists, clipped fans, and film-line geometry.
- The OG Xbox implementation calls `D3DDevice_DrawVerticesUP` and `D3DDevice_DrawIndexedVerticesUP` for every draw.

There are roughly 50 call sites, but they all pass through the two macros in `Xbox/Src/dx.h`. This is the correct migration boundary.

## 360 Backend Design

1. Keep the game-facing `DRAW_PRIM` and `DRAW_PRIM_INDEX` macro signatures.
2. Convert each FVF variant to a fixed vertex declaration: `TEX0`, `TEX1`, and `TEX2` with the existing position/color/fog/UV fields.
3. Allocate dynamic write-combined vertex and index buffers for each frame or render thread.
4. Copy UP data into the dynamic buffers, bind the declaration/buffers, and issue indexed or non-indexed draws.
5. Use a small shader pair preserving transformed screen-space coordinates. Do not reintroduce world transforms in the first pass.
6. Keep existing primitive types. If the 360 path does not accept fans, expand fans to triangle lists in the adapter.

The OG `Xbox/Src/dx.cpp` will not be compiled for `_XBOX360`. It contains
fixed-function D3D8 initialization, capability structures, and state calls
that have no compatible 360 signature. The 360 implementation belongs in a
dedicated backend (`rv360/dx360_backend.cpp`) and will expose the same
game-facing operations through guarded `dx.h` macros.

## Important Detail

The adapter must use the actual vertex stride supplied by the macro argument, not infer it from the pointer type.

## Acceptance

- Menu quad, text quad, line list, point list, indexed world bucket, and clipped fan render through the new path.
- No runtime call reaches `D3DDevice_DrawVerticesUP` or `D3DDevice_DrawIndexedVerticesUP`.
- Dynamic buffer overflow flushes or fails safely, never writes past the allocation.
- Existing bucket counters and texture-state caches remain unchanged.

## Current Blocker

The installed SDK directories contain tools/assets but no compiler headers or libraries (`xtl.h`, `xgraphics.h`, D3D declarations, or `.lib` files). The adapter can be implemented structurally, but compiling the backend requires the missing XDK development payload.
