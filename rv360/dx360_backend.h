#ifndef RV360_DX360_BACKEND_H
#define RV360_DX360_BACKEND_H

#ifdef _XBOX360
#include <xtl.h>
#include <d3d9.h>
#include <d3dx9tex.h>

// Thin state boundary used by the 360 fork of dx.h. The OG Xbox macros call
// D3DDevice_* without a device argument; the 360 XDK requires one explicitly.
extern D3DDevice *g_rv360_device;

static inline void rv360_set_render_state(D3DRENDERSTATETYPE state, DWORD value)
{
    D3DDevice_SetRenderState(g_rv360_device, state, value);
}

static inline void rv360_set_sampler_state(DWORD sampler,
                                           D3DSAMPLERSTATETYPE state,
                                           DWORD value)
{
    D3DDevice_SetSamplerState(g_rv360_device, sampler, state, value);
}

static inline void rv360_set_texture(DWORD stage, D3DBaseTexture *texture)
{
    D3DDevice_SetTexture(g_rv360_device, stage, texture, 0);
}

void rv360_init_renderer(D3DDevice *device, DWORD width, DWORD height);
HRESULT rv360_load_default_shaders(void);
void rv360_shutdown_renderer(void);
void rv360_clear(DWORD color, DWORD flags);
void rv360_present(void);
void rv360_set_fog_color(DWORD color);
HRESULT rv360_load_texture_file(const char *filename, UINT width, UINT height,
                                UINT mip_levels, D3DTexture **texture);
void rv360_set_sampler_linear(DWORD stage, BOOL mipmapped, BOOL anisotropic);

HRESULT rv360_bind_transformed_pipeline(const DWORD *vertex_shader_code,
                                        const DWORD *pixel_shader_code);
void rv360_unbind_transformed_pipeline(void);
// Dual-texture variant (40-byte stride, second UV set) for the pickup-flash
// effect. Caller binds both textures; backend binds the TEX2 declaration,
// shaders, uploads, and draws.
HRESULT rv360_bind_transformed2_pipeline(const DWORD *vertex_shader_code,
                                         const DWORD *pixel_shader_code);
HRESULT rv360_draw_TEX2_vertices_up(D3DPRIMITIVETYPE primitive,
                                    const void *vertices, DWORD vertex_count);

// Upload-and-draw bridge for replacing the OG Xbox Draw*UP macros. The caller
// supplies the already-transformed vertex bytes and their stride; shader and
// vertex-fetch declaration binding is deliberately a separate concern.
HRESULT rv360_draw_vertices_up(D3DPRIMITIVETYPE primitive,
                               const void *vertices, DWORD stride,
                               DWORD vertex_count);
HRESULT rv360_draw_indexed_vertices_up(D3DPRIMITIVETYPE primitive,
                                        const void *vertices, DWORD stride,
                                        DWORD vertex_count,
                                        const WORD *indices, DWORD index_count);

// Adapter for old UI files that bypass DRAW_PRIM and call D3DDevice_* using
// OG signatures. SetVertexShader receives an FVF in that path; it is retained
// solely to disambiguate 24-byte position+UV vertices from game TEX0 vertices.
void rv360_legacy_set_vertex_format(DWORD fvf);
void rv360_legacy_set_texture_stage_state(DWORD stage, DWORD state, DWORD value);
void rv360_legacy_get_texture_stage_state(DWORD stage, DWORD state, DWORD *value);
HRESULT rv360_legacy_draw_vertices_up(D3DPRIMITIVETYPE primitive,
                                      DWORD vertex_count, const void *vertices,
                                      DWORD stride);
HRESULT rv360_legacy_draw_indexed_vertices_up(D3DPRIMITIVETYPE primitive,
                                              DWORD index_count,
                                              const WORD *indices,
                                              const void *vertices,
                                              DWORD stride);

// OG xgraphics.h matrix type removed on 360. Compatible layout keeps the
// car matrix-palette code parsing; real skinning constants move to the 360
// shader backend (M4 remainder).
struct XGMATRIX {
    float _11, _12, _13, _14;
    float _21, _22, _23, _24;
    float _31, _32, _33, _34;
    float _41, _42, _43, _44;
};

// Row-major 4x4 multiply matching D3DXMatrixMultiply/XGMatrixMultiply
// out = A * B semantics used by the OG GPU effect path.
inline XGMATRIX operator*(const XGMATRIX &a, const XGMATRIX &b)
{
    XGMATRIX o;
    const float *A = &a._11, *B = &b._11;
    float *O = &o._11;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            O[r * 4 + c] = A[r * 4 + 0] * B[0 * 4 + c] +
                           A[r * 4 + 1] * B[1 * 4 + c] +
                           A[r * 4 + 2] * B[2 * 4 + c] +
                           A[r * 4 + 3] * B[3 * 4 + c];
    return o;
}

inline void XGMatrixMultiply(XGMATRIX *pOut, const XGMATRIX *pA,
                             const XGMATRIX *pB)
{
    *pOut = (*pA) * (*pB);
}

// OG xgraphics vector type removed on 360; layout-compatible placeholder
// for the world GPU effect constants until the effect rewrite lands.
struct XGVECTOR4 {
    float x, y, z, w;
    XGVECTOR4() {}
    XGVECTOR4(float _x, float _y, float _z, float _w)
        : x(_x), y(_y), z(_z), w(_w) {}
};

// OG depth-buffer max constant (D24S8) used to scale the software projection.
#ifndef D3DZ_MAX_D24S8
#define D3DZ_MAX_D24S8 ((FLOAT)0xFFFFFF)
#endif

// OG viewport alias lives in Xbox/Src/windows.h (D3DVIEWPORT2->D3DVIEWPORT9
// under _XBOX360); not repeated here to avoid macro/typedef collisions.

// PIX debug markers for xbxray-style tracing are not wired yet.
#define D3DDevice_SetDebugMarker(_m) ((void)0)

#endif

#endif
