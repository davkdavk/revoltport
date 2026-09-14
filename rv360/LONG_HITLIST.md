# `long` → fixed-width hit-list (M1)

`long` is 32-bit on Win32-x86, OG Xbox, and 360 XDK — but it must never appear in
disk/network structs. Convert struct *fields* to `rv_s32`; leave function
params/return values alone unless they touch the wire.

## model.h (MODEL_POLY_LOAD / MODEL_VERTEX / LEVEL_MODEL)
- `model.h:183` `long c0,c1,c2,c3` → `rv_s32` (disk struct, swapped in `rv_read_pollload`)
- `model.h:200` `long color,specular` → `rv_s32` (color DWORD)
- `model.h:202` `long r,g,b,a` → `rv_s32`
- `model.h:240` `long ID,RefCount` → `rv_s32`
- `model.h:130-170` `*(long*)&mrgb->rgb[n]` color-punning macros → `*(rv_s32*)` (BE-safe after field swap)

## level.h (records + LEVELINFO)
- `level.h:43` `long Time` → `rv_s32`
- `level.h:49` `long SplitTime[]` → `rv_s32`
- `level.h:63,66,69-72` `NormalStartGrid/ReverseStartGrid/FogColor/WorldRGBper/ModelRGBper/InstanceRGBper/MirrorType` → `rv_s32`

## edfield.h / NewColl.h (track data on disk)
- `edfield.h:28` `long Type` → `rv_s32`
- `NewColl.h:97,101-103,113,251-252,307,339-340` `Type/SkidColour/Corrugation/DustType/SparkType/Material/CollType/NCollPolys/NWorldPolys` → `rv_s32`

## typedefs.h
- `bool=long` (Xbox `typedefs.h:48`), `MEM8..32 long data[N]` (220-246) → keep for RAM logic, but never `fread` as a unit; FLOAT bit-puns (`*(DWORD*)&f` in `dx.h` LODBIAS) stay DWORD.

## network.h
- `SetSendMsgHeaderTcp(... int size)` + `WORD Size` field: pack LE explicitly via `rv_pack_msg_tcp` (done in `rv360_lefile.h`).
