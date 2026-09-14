# Local Xenon Build Environment

The existing 32-bit Wine prefix is now populated from the embedded `XDK/`
payload in `XDKSetupXenon7645.exe`.

## Verified Files

SDK root:

```text
/home/davey/.wine32/drive_c/Program Files/Microsoft Xbox 360 SDK
```

Verified:

- `include/xbox/xtl.h`
- `include/xbox/xgraphics.h`
- `include/xbox/d3d9.h`
- `include/xbox/xaudio2.h`
- `include/xbox/xonline.h`
- `lib/xbox/*.lib`
- `bin/win32/cl.exe`

The compiler identifies as:

```text
Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 14.00.7151 for PowerPC
```

## Build Commands

From the repository root:

```bash
# Focused compile-only checks (no XEX).
bash rv360/check_xenon.sh

# Complete offline development image, using fresh objects on every run.
bash rv360/build_xenon.sh
```

The image build compiles the 132 game translation units and four port backend
units, links using `/MACHINE:PPCBE /SUBSYSTEM:XBOX`, then runs `imagexex`.
It does not use `/FORCE` or ignore unresolved symbols. `xmcore.lib` supplies
XAudio2's lock-free queue functions; `d3dx9.lib` supplies texture loading.

Outputs are kept in a unique ignored `build/xenon.*` directory:

- `default.xex`: development image, not a playable release.
- `revolt.exe`: linked PowerPC PE intermediate.
- `revolt.map`: linker map.
- `objects/objects.rsp`: exact object list from that build only.
- `objects/*.log`, `link.log`, `imagexex.log`: compiler/tool diagnostics.
- `xex-info.txt`: image header/import inspection.

Override `WINEPREFIX`, `RV360_XDK_ROOT`, or `RV360_BUILD_ROOT` as needed.
The full image script currently uses release libraries; `RV360_DEBUG=1` is
supported by the compile checker, not the full image script.

## Wine Linker Runtime

The existing prefix needed `link.exe.manifest` alongside the XDK linker to
activate its VC80 runtime. Without it the loader reported MSVCP80 initialization
failure `c0000005`. The local repair used the SDK's own `cl.exe.manifest` as the
side-by-side dependency manifest for `link.exe`. No XDK binaries or runtime DLLs
are included in this repository. A new machine may need the same SDK-runtime
setup before linking.

## Compiler Defines

`_XBOX` selects Xbox headers; `_XBOX360` enables the port branches. The verified
build also defines `_M_PPC` for the legacy toolchain.

```text
/D_XBOX /D_M_PPC /D_XBOX360
```

The current scripts use `/X` to avoid host header contamination and include only:

1. Xbox 360 XDK `include/xbox`
2. Xbox 360 XDK `include/xbox/sys`, when present
3. Xbox 360 XDK `include/win32`, when present
4. The local OG Xbox source directory

## Probe

`rv360/xdk_probe.cpp` compiles with the Xenon compiler and validates `xtl.h`,
`xgraphics.h`, `d3d9.h`, `xaudio2.h`, `IDirect3DDevice9`, viewport state, and
sampler state.

## Historical Compile Findings

Compiling `Xbox/Src/dx.cpp` now reaches source errors rather than environment
errors. The first API forks are:

- OG `d3d8.h` include, guarded to `d3d9.h` under `_XBOX360`.
- OG `dsound.h`, guarded out under `_XBOX360` for the later XAudio2 port.
- `IDirect3DDevice3` and `DDPIXELFORMAT` in `dx.h`.
- `LPDIRECT3D...8` resource types in `XBResource.h`.
- DirectSound types in `SoundEffectEngine.h`.

Do not alias these blindly: they need real renderer/audio backend changes.
