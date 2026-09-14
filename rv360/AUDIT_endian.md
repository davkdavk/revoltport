# Endian/alignment audit — must-fix sites for Xenon (big-endian PPC)

File data on disk is little-endian x86. Xenon needs swap + packing fixes.

## Asset loaders (raw fread of structs)
- `source/model.cpp:136,186,315,2408,2413,2421` — `fread(&mh/&mpl/&mvl, sizeof(...))` (.prm/.m). Struct defs: `source/inc/model.h:175-239`.
- `source/level.cpp:917,934` — `fread(&nFields/sizeof(long))`, `fread(&fileField/sizeof(FILE_FIELD))` (.fld/.ncp/.cam). `long` is 4B on x86/360 but verify; prefer `rv_s32`. Defs: `source/inc/level.h:42-56`.
- `source/light.cpp:1781,1793`, `source/NewColl.cpp:906,921,982,1043,1099` — collision/grid structs (`NEWCOLLPOLYHDR`, `NEWCOLLPOLY`, `COLLGRID_DATA`). Defs: `source/inc/NewColl.h`.
- `Xbox/Src/LevelLoad.cpp` — staged track loads, same structs + `short CarType` in `ONE_RECORD_ENTRY`.
- `Xbox/Src/texture.cpp` — `TPAGE_*` tpage cache, `TEXINFO` has `char File[128]` + texture pointers; `LoadTextureClever/Mip/GPU` does pixel copies needing swap for tiled 360 textures.

## Type hazards
- `typedefs.h` (both trees): `REAL=float` ok (IEEE both sides, only byte order swaps), but `bool=long` (line 48), `VISIMASK=unsigned __int64`, `long data[N]` in `MEM8..32`, `INDEX=short`. Audit every `long` in file/network structs → `rv_s32`.
- `SHORTVEC/SHORTMAT/SHORTQUAT/CHARQUAT` — 16/8-bit arrays also need per-element swap on load.

## Network packets (must be wire-stable)
- `Xbox/Src/network.h:92-176` — `MSG_TYPE`/`MSG_TYPE_EXT` enums + `#include <pshpack1.h>` packed structs, `SetSendMsgHeader*/GetRecvMsgHeader`, scales `REMOTE_QUAT_SCALE/VEL/ANGVEL`, `PACKET_BUFFER_SIZE 1300`. Every multi-byte field + float needs explicit LE serialize on 360.
- `Xbox/Src/net_xonline.h` — `XATTRIB_*` matchmaking attributes.

## Renderer-adjacent binary
- `Xbox/Src/xdx.cpp` — offline cooker reference; use it as the model for a 360 precooker (byteswap + swizzle once, not at runtime).
- `Xbox/art/**/*.xdx`, `*.xpr` (`ui/media/font.xpr,resource.xpr,gamepad.xpr`) — pre-swizzled OG Xbox blobs, must be recooked for 360 tiling.

## First conversion targets (smallest, highest value)
1. `model.cpp` mesh header read → `rv_fread_*le` helpers.
2. `level.cpp` `FILE_FIELD` loop → explicit field-by-field LE read.
3. `network.h` packet header set/get → explicit LE pack/unpack.
