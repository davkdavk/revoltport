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
void rv360_shutdown_renderer(void);
void rv360_clear(DWORD color, DWORD flags);
void rv360_present(void);
HRESULT rv360_load_texture_file(const char *filename, UINT width, UINT height,
                                UINT mip_levels, D3DTexture **texture);
void rv360_set_sampler_linear(DWORD stage, BOOL mipmapped, BOOL anisotropic);

HRESULT rv360_bind_transformed_pipeline(const DWORD *vertex_shader_code,
                                        const DWORD *pixel_shader_code);
void rv360_unbind_transformed_pipeline(void);

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

#endif

#endif
