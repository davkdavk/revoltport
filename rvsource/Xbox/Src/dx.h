//-----------------------------------------------------------------------------
// File: dx.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef DX_H
#define DX_H

#include "revolt.h"
#include "main.h"

#ifdef _XBOX360
#include "../../../rv360/dx360_backend.h"
// The old source names this device as IDirect3DDevice3. The actual 360 XDK
// exposes D3DDevice; this alias is temporary while dx.cpp moves to explicit
// D3DDevice_* calls.
typedef D3DDevice IDirect3DDevice3;
// Draw-prim flags are consumed by DRAW_PRIM only as opaque arguments on 360
// (the backend uploads raw strides). FVF codes already exist in the 360 XDK;
// only define what is genuinely missing so call sites parse.
#ifndef D3DFVF_XYZRHW
#define D3DFVF_XYZRHW 0x004
#endif
#ifndef D3DDP_DONOTUPDATEEXTENTS
#define D3DDP_DONOTUPDATEEXTENTS 0
#define D3DDP_DONOTCLIP 0
#endif
#endif

//$MODIFIED: changed D3DRENDERSTATE_* constants to D3DRS_*
//$MODIFIED: changed D3Ddevice->Set***() to D3DDevice_Set***()  <xbox-specific>

#define D3DRENDERSTATE_ALPHATESTENABLE D3DRS_ALPHATESTENABLE //$ADDITION
#define D3DRENDERSTATE_ALPHAREF        D3DRS_ALPHAREF //$ADDITION
#define D3DRENDERSTATE_ALPHAFUNC       D3DRS_ALPHAFUNC //$ADDITION

// macros

#define MAX_DISPLAY_MODES 64
#define MAX_DISPLAY_MODE_TEXT 32
//$REMOVED#define MAX_DRAW_DEVICES 3
//$REMOVED#define MAX_DRAW_DEVICE_NAME 128

enum {
    GAMMA_UNAVAILABLE,
    GAMMA_AVAILABLE,
    GAMMA_AUTO,
};

// render state macros

#ifdef _XBOX360
#define SET_RENDER_STATE(_s, _v) \
{ \
    rv360_set_render_state((_s), (_v)); \
    RenderStateChange++; \
}
#else
#define SET_RENDER_STATE(_s, _v) \
{ \
    D3DDevice_SetRenderState((_s), (_v)); \
    RenderStateChange++; \
}
#endif

#define SET_STAGE_STATE(_t, _s, _v) \
{ \
    D3DDevice_SetTextureStageState((_t), (_s), (_v)); \
    RenderStateChange++; \
}

#ifdef _XBOX360
#define SET_TEXTURE(_t, _tex) \
{ \
    rv360_set_texture((_t), (_tex)); \
    TextureStateChange++; \
}
#else
#define SET_TEXTURE(_t, _tex) \
{ \
    D3DDevice_SetTexture((_t), (_tex)); \
    TextureStateChange++; \
}
#endif

//$MODIFIED
//#define DRAW_PRIM D3DDevice->DrawPrimitive
//#define DRAW_PRIM_INDEX D3DDevice->DrawIndexedPrimitive
#ifndef _XBOX360
#define DRAW_PRIM(a,b,c,d,e) \
{ \
    D3DDevice_SetVertexShader( (b) ); \
    D3DDevice_DrawVerticesUP( (a), (d), (c), sizeof((c)[0]) ); \
}
#define DRAW_PRIM_INDEX(a,b,c,d,e,f,g) \
{ \
    D3DDevice_SetVertexShader( (b) ); \
    D3DDevice_DrawIndexedVerticesUP( (a), (f), (e), (c), sizeof((c)[0]) ); \
}
#else
#define DRAW_PRIM(a,b,c,d,e) \
{ \
    (void)(b); (void)(e); \
    rv360_draw_vertices_up((a), (c), sizeof((c)[0]), (d)); \
}
#define DRAW_PRIM_INDEX(a,b,c,d,e,f,g) \
{ \
    (void)(b); (void)(g); \
    rv360_draw_indexed_vertices_up((a), (c), sizeof((c)[0]), (d), (e), (f)); \
}
#endif
//$END_MODIFICATIONS

#define MIPMAP_LODBIAS(_n) \
{ \
    float _f = _n; \
    SET_STAGE_STATE(0, D3DTSS_MIPMAPLODBIAS, *(DWORD*)&_f); \
    SET_STAGE_STATE(1, D3DTSS_MIPMAPLODBIAS, *(DWORD*)&_f); \
}

//$MODIFIED
//#define TEXTURE_ADDRESS(_w) \
//{ \
//    SET_STAGE_STATE(0, D3DTSS_ADDRESS, _w); \
//    SET_STAGE_STATE(1, D3DTSS_ADDRESS, _w); \
//}
#define TEXTURE_ADDRESS(_w) \
{ \
    SET_STAGE_STATE(0, D3DTSS_ADDRESSU, _w); \
    SET_STAGE_STATE(0, D3DTSS_ADDRESSV, _w); \
    SET_STAGE_STATE(0, D3DTSS_ADDRESSW, _w); \
    SET_STAGE_STATE(1, D3DTSS_ADDRESSU, _w); \
    SET_STAGE_STATE(1, D3DTSS_ADDRESSV, _w); \
    SET_STAGE_STATE(1, D3DTSS_ADDRESSW, _w); \
}
//$END_MODIFICATIONS

#define FOG_ON() \
{ \
    if (!RenderFog) SET_RENDER_STATE(D3DRS_FOGENABLE, (RenderFog = TRUE)); \
}

#define FOG_OFF() \
{ \
    if (RenderFog) SET_RENDER_STATE(D3DRS_FOGENABLE, (RenderFog = FALSE)); \
}

#define BLEND_SRC(_a) \
{ \
    if (RenderBlendSrc != _a) SET_RENDER_STATE(D3DRS_SRCBLEND, (RenderBlendSrc = _a)); \
}

#define BLEND_DEST(_a) \
{ \
    if (RenderBlendDest != _a) SET_RENDER_STATE(D3DRS_DESTBLEND, (RenderBlendDest = _a)); \
}

#define BLEND_OFF() \
{ \
    if (RenderBlend) \
    { \
        SET_RENDER_STATE(D3DRS_ALPHABLENDENABLE, FALSE); \
        if (RenderBlend == 2) \
        { \
            SET_STAGE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); \
            SET_RENDER_STATE(D3DRS_ALPHAREF, AlphaRef); \
        } \
        RenderBlend = 0; \
    } \
}

#define BLEND_ON() \
{ \
    if (RenderBlend != 1) \
    { \
        if (!RenderBlend) SET_RENDER_STATE(D3DRS_ALPHABLENDENABLE, TRUE); \
        if (RenderBlend == 2) \
        { \
            SET_STAGE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); \
            SET_RENDER_STATE(D3DRS_ALPHAREF, AlphaRef); \
        } \
        RenderBlend = 1; \
    } \
}

#define BLEND_ALPHA() \
{ \
    if (RenderBlend != 2) \
    { \
        if (!RenderBlend) SET_RENDER_STATE(D3DRS_ALPHABLENDENABLE, TRUE); \
        SET_STAGE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE); \
        SET_RENDER_STATE(D3DRS_ALPHAREF, 0); \
        RenderBlend = 2; \
    } \
}

#define ZWRITE_ON() \
{ \
    if (!RenderZwrite) SET_RENDER_STATE(D3DRS_ZWRITEENABLE, (RenderZwrite = TRUE)); \
}

#define ZWRITE_OFF() \
{ \
    if (RenderZwrite) SET_RENDER_STATE(D3DRS_ZWRITEENABLE, (RenderZwrite = FALSE)); \
}

#define ZBUFFER_ON() \
{ \
    if (RenderZbuffer != D3DZB_TRUE) SET_RENDER_STATE(D3DRS_ZENABLE, (RenderZbuffer = D3DZB_TRUE)); \
}

#define ZBUFFER_OFF() \
{ \
    if (RenderZbuffer != D3DZB_FALSE) SET_RENDER_STATE(D3DRS_ZENABLE, (RenderZbuffer = D3DZB_FALSE)); \
}

#define ZCMP(_c) \
{ \
    if (RenderZcmp != _c) SET_RENDER_STATE(D3DRS_ZFUNC, (RenderZcmp = _c)); \
}

//$REMOVED (perspective-correction for textures is on by default in DX8)
//#define PERSPECTIVE_ON() \
//    SET_RENDER_STATE(D3DRS_TEXTUREPERSPECTIVE, DxState.Perspective)
//
//#define PERSPECTIVE_OFF() \
//    SET_RENDER_STATE(D3DRS_TEXTUREPERSPECTIVE, FALSE)
//$END_REMOVAL

#define DITHER_ON() \
    SET_RENDER_STATE(D3DRS_DITHERENABLE, DxState.Dither)

#define DITHER_OFF() \
    SET_RENDER_STATE(D3DRS_DITHERENABLE, FALSE)

//$MODIFIED
//#define COLORKEY_ON() \
//    SET_RENDER_STATE(D3DRS_COLORKEYENABLE, DxState.ColorKey)
#define COLORKEY_ON() \
{ \
    SET_STAGE_STATE(0, D3DTSS_COLORKEYOP,    DxState.ColorKey); \
    SET_STAGE_STATE(0, D3DTSS_COLORKEYCOLOR, DxState.ColorKeyColor); \
    SET_STAGE_STATE(1, D3DTSS_COLORKEYOP,    DxState.ColorKey); \
    SET_STAGE_STATE(1, D3DTSS_COLORKEYCOLOR, DxState.ColorKeyColor); \
}
  //$ADDITION(jedl) - added ColorKeyColor
  //$PERF(cprince): if ColorKeyColor never changes, then no need to to set it inside this macro?
//$END_MODIFICATIONS    

//$MODIFIED
//#define COLORKEY_OFF() \
//    SET_RENDER_STATE(D3DRS_COLORKEYENABLE, FALSE)
#define COLORKEY_OFF() \
{ \
    SET_STAGE_STATE(0, D3DTSS_COLORKEYOP, D3DTCOLORKEYOP_DISABLE); \
    SET_STAGE_STATE(1, D3DTSS_COLORKEYOP, D3DTCOLORKEYOP_DISABLE); \
}
//$END_MODIFICATION    

 #ifndef XBOX_NOT_YET_IMPLEMENTED
#define ANTIALIAS_ON() \
    SET_RENDER_STATE(D3DRS_ANTIALIAS, DxState.AntiAlias)

#define ANTIALIAS_OFF() \
    SET_RENDER_STATE(D3DRS_ANTIALIAS, D3DANTIALIAS_NONE)
 #endif // ! XBOX_NOT_YET_IMPLEMENTED

#define CULL_ON() \
    SET_RENDER_STATE(D3DRS_CULLMODE, D3DCULL_CCW)

#define CULL_OFF() \
    SET_RENDER_STATE(D3DRS_CULLMODE, D3DCULL_NONE)

#define SPECULAR_ON() \
    SET_RENDER_STATE(D3DRS_SPECULARENABLE, TRUE)

#define SPECULAR_OFF() \
    SET_RENDER_STATE(D3DRS_SPECULARENABLE, FALSE)

#define WIREFRAME_ON() \
    SET_RENDER_STATE(D3DRS_FILLMODE, DxState.Wireframe)

#define WIREFRAME_OFF() \
    SET_RENDER_STATE(D3DRS_FILLMODE, D3DFILL_SOLID)

//$MODIFIED: changed D3DTFN_*/D3DTFG_* to D3DTEXF_* here
#define TEXTUREFILTER_OFF() \
{ \
    SET_STAGE_STATE(0, D3DTSS_MINFILTER, D3DTEXF_POINT); \
    SET_STAGE_STATE(0, D3DTSS_MAGFILTER, D3DTEXF_POINT); \
    SET_STAGE_STATE(1, D3DTSS_MINFILTER, D3DTEXF_POINT); \
    SET_STAGE_STATE(1, D3DTSS_MAGFILTER, D3DTEXF_POINT); \
}

//$MODIFIED: changed D3DTFN_*/D3DTFG_* to D3DTEXF_* here
#define TEXTUREFILTER_ON() \
{ \
    switch (DxState.TextureFilter) \
    { \
        case 0: \
            SET_STAGE_STATE(0, D3DTSS_MINFILTER, D3DTEXF_POINT); \
            SET_STAGE_STATE(0, D3DTSS_MAGFILTER, D3DTEXF_POINT); \
            SET_STAGE_STATE(1, D3DTSS_MINFILTER, D3DTEXF_POINT); \
            SET_STAGE_STATE(1, D3DTSS_MAGFILTER, D3DTEXF_POINT); \
            break; \
        case 1: \
            SET_STAGE_STATE(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR); \
            SET_STAGE_STATE(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR); \
            SET_STAGE_STATE(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR); \
            SET_STAGE_STATE(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR); \
            break; \
        case 2: \
            SET_STAGE_STATE(0, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC); \
            SET_STAGE_STATE(0, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC); \
            SET_STAGE_STATE(1, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC); \
            SET_STAGE_STATE(1, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC); \
            break; \
    } \
}

//$MODIFIED: changed D3DTFP_NONE to D3DTEXF_NONE here
#define MIPMAP_OFF() \
{ \
    SET_STAGE_STATE(0, D3DTSS_MIPFILTER, D3DTEXF_NONE); \
    SET_STAGE_STATE(1, D3DTSS_MIPFILTER, D3DTEXF_NONE); \
}

//$MODIFIED
//#define MIPMAP_ON() \
//{ \
//    SET_STAGE_STATE(0, D3DTSS_MIPFILTER, DxState.MipMap + 1); \
//    SET_STAGE_STATE(1, D3DTSS_MIPFILTER, DxState.MipMap + 1); \
//}
#define MIPMAP_ON() \
{ \
    SET_STAGE_STATE(0, D3DTSS_MIPFILTER, DxState.MipMap); \
    SET_STAGE_STATE(1, D3DTSS_MIPFILTER, DxState.MipMap); \
}
//$NOTE: DxState.MipMap can have the values 0 (none), 1 (point), or 2 (linear)
/// These values match the Xbox D3DTEXF_* constants exactly, but they're off-by-one compared to the DX6 D3DTFP_* constants.

//$END_MODIFICATIONS

#define FOG_COLOR(_c) \
    SET_RENDER_STATE(D3DRS_FOGCOLOR, _c)

#define FOG_COLOR(_c) \
    SET_RENDER_STATE(D3DRS_FOGCOLOR, _c)

#define SORT_INDEPENDENT_ON() \
    SET_RENDER_STATE(D3DRS_TRANSLUCENTSORTINDEPENDENT, TRUE)

#define SORT_INDEPENDENT_OFF() \
    SET_RENDER_STATE(D3DRS_TRANSLUCENTSORTINDEPENDENT, FALSE)

#define SET_TPAGE(_tp) \
{ \
    if (RenderTP != _tp) \
    { \
        if ((RenderTP = _tp) == -1) \
        { \
            SET_TEXTURE(0, NULL); \
        } \
        else \
        { \
            SET_TEXTURE(0, TexInfo[RenderTP].Texture); \
        } \
    } \
}

#define SET_TPAGE2(_tp) \
{ \
    if (RenderTP2 != _tp) \
    { \
        if ((RenderTP2 = _tp) == -1) \
        { \
            SET_TEXTURE(1, NULL); \
        } \
        else \
        { \
            SET_TEXTURE(1, TexInfo[RenderTP].Texture); \
        } \
    } \
}

// structs

typedef struct {
    long WireframeEnabled, Wireframe;
//$REMOVED    long PerspectiveEnabled, Perspective;
    long TextureFilterFlag, TextureFilter;
    long MipMapFlag;
    long MipMap;
    long FogEnabled, Fog;
    long DitherEnabled, Dither;
    long ColorKeyEnabled, ColorKey;
    D3DCOLOR ColorKeyColor; //$ADDITION(jedl)
#ifndef XBOX_NOT_YET_IMPLEMENTED
    long AntiAliasEnabled, AntiAlias;
#endif // ! XBOX_NOT_YET_IMPLEMENTED
} DX_STATE;

typedef struct {
    DWORD Width, Height, Bpp, Refresh;
    char DisplayText[MAX_DISPLAY_MODE_TEXT];
} DISPLAY_MODE;

//$REMOVED
//typedef struct {
//    char Name[MAX_DRAW_DEVICE_NAME];
//    long DisplayModeNum, BestDisplayMode;
//    DISPLAY_MODE DisplayMode[MAX_DISPLAY_MODES];
//} DRAW_DEVICE;
//$END_REMOVAL

// prototypes

//$TODO(cprince): these are sucky names for the D3D-init functions.  Fix them someday, and perhaps merge them.
//$REMOVEDextern BOOL InitDX(void); //$RENAMED: was InitDD
extern BOOL InitD3D(DWORD width, DWORD height, DWORD bpp, DWORD refresh);
//$REMOVEDextern void ReleaseDX(void);
extern void ReleaseD3D(void);
extern void SetGamma(long brightness, long contrast);
extern void ErrorDX(HRESULT r, char *mess);
extern void SetBackgroundColor(long col);
//$REMOVEDextern HRESULT CALLBACK EnumZedBufferCallback(DDPIXELFORMAT *ddpf, void *user);
//$REMOVEDextern void CheckSurfaces(void);
extern void FlipBuffers(void);
extern void ClearBuffers(void);
//$REMOVEDextern void SetFrontBufferRGB(long rgb);
extern void SetupDxState(void);
//$REMOVEDextern void GetDrawDevices(void);
//$REMOVEDextern BOOL CALLBACK GetDrawDeviceCallback(GUID *lpGUID, LPSTR szName, LPSTR szDevice, LPVOID lParam);
//$REMOVEDextern BOOL CALLBACK CreateDrawDeviceCallback(GUID *lpGUID, LPSTR szName, LPSTR szDevice, LPVOID lParam);
//$REMOVEDextern HRESULT CALLBACK DisplayModesCallback(DDSURFACEDESC2 *Mode, void *UserArg);

// globals

extern DX_STATE DxState;
//$REMOVEDextern IDirectDraw4 *DD;
//$REMOVEDextern IDirectDrawSurface4 *FrontBuffer;
//$REMOVEDextern IDirectDrawSurface4 *BackBuffer;
//$REMOVEDextern IDirectDrawSurface4 *ZedBuffer;
//$REMOVEDextern IDirectDrawGammaControl *GammaControl;
//$REMOVEDextern IDirect3D3 *D3D;
extern IDirect3DDevice3 *D3Ddevice;
//$REMOVEDextern IDirect3DViewport3 *D3Dviewport;
//$REMOVEDextern D3DDEVICEDESC D3Dcaps;
#ifdef _XBOX360
extern D3DFORMAT ZedBufferFormat;
#else
extern DDPIXELFORMAT ZedBufferFormat;
#endif
extern DWORD ScreenXsize;
extern DWORD ScreenYsize;
extern DWORD ScreenBpp;
//$REMOVEDextern DWORD ScreenRefresh;
extern long GammaFlag;
extern long NoColorKey;
//$REMOVEDextern long DrawDeviceNum, CurrentDrawDevice;
//$REMOVEDextern long DisplayModeCount;
extern long RenderStateChange, TextureStateChange;
extern long BackBufferCount;
extern long BlendSubtract;
extern DWORD BackgroundColor;
//$REMOVEDextern DRAW_DEVICE DrawDevices[];
extern int RenderTP, RenderTP2;
extern short RenderFog, RenderBlend, RenderBlendSrc, RenderBlendDest;
extern short RenderZcmp, RenderZwrite, RenderZbuffer;

#ifdef _XBOX360
// 360 sampler-state migration (M4.5): fixed-function texture-stage state has
// no D3D9 equivalent. Sampler filtering/addressing moves to D3DSAMP_*;
// combiner state (COLOROP/ALPHAOP/ARGS) is owned by the 360 pixel shader;
// color-key textures must be recooked with alpha (deferred to M4.4).
#undef MIPMAP_LODBIAS
#define MIPMAP_LODBIAS(_n) \
{ \
    float _f = _n; \
    rv360_set_sampler_state(0, D3DSAMP_MIPMAPLODBIAS, *(DWORD*)&_f); \
    rv360_set_sampler_state(1, D3DSAMP_MIPMAPLODBIAS, *(DWORD*)&_f); \
}
#undef TEXTURE_ADDRESS
#define TEXTURE_ADDRESS(_w) \
{ \
    rv360_set_sampler_state(0, D3DSAMP_ADDRESSU, _w); \
    rv360_set_sampler_state(0, D3DSAMP_ADDRESSV, _w); \
    rv360_set_sampler_state(0, D3DSAMP_ADDRESSW, _w); \
    rv360_set_sampler_state(1, D3DSAMP_ADDRESSU, _w); \
    rv360_set_sampler_state(1, D3DSAMP_ADDRESSV, _w); \
    rv360_set_sampler_state(1, D3DSAMP_ADDRESSW, _w); \
}
#undef BLEND_OFF
#define BLEND_OFF() \
{ \
    if (RenderBlend) \
    { \
        SET_RENDER_STATE(D3DRS_ALPHABLENDENABLE, FALSE); \
        if (RenderBlend == 2) \
        { \
            SET_RENDER_STATE(D3DRS_ALPHAREF, AlphaRef); \
        } \
        RenderBlend = 0; \
    } \
}
#undef BLEND_ON
#define BLEND_ON() \
{ \
    if (RenderBlend != 1) \
    { \
        if (!RenderBlend) SET_RENDER_STATE(D3DRS_ALPHABLENDENABLE, TRUE); \
        if (RenderBlend == 2) \
        { \
            SET_RENDER_STATE(D3DRS_ALPHAREF, AlphaRef); \
        } \
        RenderBlend = 1; \
    } \
}
#undef BLEND_ALPHA
#define BLEND_ALPHA() \
{ \
    if (RenderBlend != 2) \
    { \
        if (!RenderBlend) SET_RENDER_STATE(D3DRS_ALPHABLENDENABLE, TRUE); \
        SET_RENDER_STATE(D3DRS_ALPHAREF, 0); \
        RenderBlend = 2; \
    } \
}
#undef COLORKEY_ON
#define COLORKEY_ON() \
{ \
    (void)DxState.ColorKey; \
}
#undef COLORKEY_OFF
#define COLORKEY_OFF() \
{ \
}
#undef TEXTUREFILTER_OFF
#define TEXTUREFILTER_OFF() \
{ \
    rv360_set_sampler_state(0, D3DSAMP_MINFILTER, D3DTEXF_POINT); \
    rv360_set_sampler_state(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT); \
    rv360_set_sampler_state(1, D3DSAMP_MINFILTER, D3DTEXF_POINT); \
    rv360_set_sampler_state(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT); \
}
#undef TEXTUREFILTER_ON
#define TEXTUREFILTER_ON() \
{ \
    switch (DxState.TextureFilter) \
    { \
        case 0: \
            rv360_set_sampler_state(0, D3DSAMP_MINFILTER, D3DTEXF_POINT); \
            rv360_set_sampler_state(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT); \
            rv360_set_sampler_state(1, D3DSAMP_MINFILTER, D3DTEXF_POINT); \
            rv360_set_sampler_state(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT); \
            break; \
        case 1: \
            rv360_set_sampler_state(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR); \
            rv360_set_sampler_state(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR); \
            rv360_set_sampler_state(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR); \
            rv360_set_sampler_state(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR); \
            break; \
        case 2: \
            rv360_set_sampler_state(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC); \
            rv360_set_sampler_state(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC); \
            rv360_set_sampler_state(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC); \
            rv360_set_sampler_state(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC); \
            break; \
    } \
}
#undef MIPMAP_OFF
#define MIPMAP_OFF() \
{ \
    rv360_set_sampler_state(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE); \
    rv360_set_sampler_state(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE); \
}
#undef MIPMAP_ON
#define MIPMAP_ON() \
{ \
    rv360_set_sampler_state(0, D3DSAMP_MIPFILTER, DxState.MipMap); \
    rv360_set_sampler_state(1, D3DSAMP_MIPFILTER, DxState.MipMap); \
}
// Fixed-function states removed on 360: fog is baked into vertex specular
// alpha by the game and applied by the transformed pixel shader; specular
// highlights, dithering, wireframe fill, antialias sort-independence, and
// color-key are shader/cooker concerns, not render states.
#undef FOG_ON
#define FOG_ON() { RenderFog = TRUE; }
#undef FOG_OFF
#define FOG_OFF() { RenderFog = FALSE; }
#undef FOG_COLOR
#define FOG_COLOR(_c) { rv360_set_fog_color(_c); }
#undef SPECULAR_ON
#define SPECULAR_ON() { }
#undef SPECULAR_OFF
#define SPECULAR_OFF() { }
#undef DITHER_ON
#define DITHER_ON() { }
#undef DITHER_OFF
#define DITHER_OFF() { }
#undef WIREFRAME_ON
#define WIREFRAME_ON() { }
#undef WIREFRAME_OFF
#define WIREFRAME_OFF() { }
#undef ANTIALIAS_ON
#define ANTIALIAS_ON() { SET_RENDER_STATE(D3DRS_MULTISAMPLEANTIALIAS, TRUE); }
#undef ANTIALIAS_OFF
#define ANTIALIAS_OFF() { SET_RENDER_STATE(D3DRS_MULTISAMPLEANTIALIAS, FALSE); }
#undef SORT_INDEPENDENT_ON
#define SORT_INDEPENDENT_ON() { }
#undef SORT_INDEPENDENT_OFF
#define SORT_INDEPENDENT_OFF() { }
#endif

#endif // DX_H

