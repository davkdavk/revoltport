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

## Compiler Defines

The critical target define is `_M_PPC`. `_XBOX` selects Xbox headers; `_XBOX360`

```text
/D_XBOX /D_M_PPC /D_XBOX360
```

The include order that successfully compiled `rv360/xdk_probe.cpp` is:

1. Xbox 360 XDK `include/xbox`
2. Xbox 360 XDK `include/win32`
3. Visual Studio 9 VC `include`
4. Windows SDK 6.0A `Include`

## Probe

`rv360/xdk_probe.cpp` compiles with the Xenon compiler and validates `xtl.h`,
`xgraphics.h`, `d3d9.h`, `xaudio2.h`, `IDirect3DDevice9`, viewport state, and
sampler state.

## First Source Compile Result

Compiling `Xbox/Src/dx.cpp` now reaches source errors rather than environment
errors. The first API forks are:

- OG `d3d8.h` include, guarded to `d3d9.h` under `_XBOX360`.
- OG `dsound.h`, guarded out under `_XBOX360` for the later XAudio2 port.
- `IDirect3DDevice3` and `DDPIXELFORMAT` in `dx.h`.
- `LPDIRECT3D...8` resource types in `XBResource.h`.
- DirectSound types in `SoundEffectEngine.h`.

Do not alias these blindly: they need real renderer/audio backend changes.
