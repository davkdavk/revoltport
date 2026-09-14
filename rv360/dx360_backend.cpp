#include "dx360_backend.h"
#include <string.h>

#ifdef _XBOX360

D3DDevice *g_rv360_device = NULL;

static DWORD g_rv360_fog_color = 0xFF000000;

void rv360_set_fog_color(DWORD color)
{
    g_rv360_fog_color = color;
}

static void rv360_push_fog_constant(void)
{
    // Fog factor arrives in vertex specular alpha (game bakes it on CPU);
    // fog color arrives here from FOG_COLOR and is consumed by the
    // transformed pixel shader's c0 constant.
    float fog[4];
    fog[0] = (float)(g_rv360_fog_color & 0xFF) / 255.0f;
    fog[1] = (float)((g_rv360_fog_color >> 8) & 0xFF) / 255.0f;
    fog[2] = (float)((g_rv360_fog_color >> 16) & 0xFF) / 255.0f;
    fog[3] = (float)((g_rv360_fog_color >> 24) & 0xFF) / 255.0f;
    D3DDevice_SetPixelShaderConstantF(g_rv360_device, 0, fog, 1);
}

static D3DVertexBuffer *g_rv360_vertex_buffer = NULL;
static D3DIndexBuffer *g_rv360_index_buffer = NULL;
static D3DVertexDeclaration *g_rv360_vertex_declaration = NULL;
static D3DVertexDeclaration *g_rv360_vertex_declaration2 = NULL;
static D3DVertexShader *g_rv360_vertex_shader = NULL;
static D3DPixelShader *g_rv360_pixel_shader = NULL;
static D3DPixelShader *g_rv360_pixel_shader2 = NULL;
static DWORD g_rv360_legacy_fvf = 0;
static const DWORD RV360_VERTEX_BUFFER_BYTES = 1024 * 1024;
static const DWORD RV360_INDEX_BUFFER_BYTES = 256 * 1024;

static HRESULT rv360_ensure_draw_buffers(void)
{
    if (!g_rv360_device) return E_FAIL;

    if (!g_rv360_vertex_buffer)
        g_rv360_vertex_buffer = D3DDevice_CreateVertexBuffer(
            RV360_VERTEX_BUFFER_BYTES, D3DUSAGE_WRITEONLY, D3DPOOL_DEFAULT);
    if (!g_rv360_index_buffer)
        g_rv360_index_buffer = D3DDevice_CreateIndexBuffer(
            RV360_INDEX_BUFFER_BYTES, D3DUSAGE_WRITEONLY,
            D3DFMT_INDEX16, D3DPOOL_DEFAULT);

    return (g_rv360_vertex_buffer && g_rv360_index_buffer) ? S_OK : E_OUTOFMEMORY;
}

void rv360_init_renderer(D3DDevice *device, DWORD width, DWORD height)
{
    (void)width;
    (void)height;
    g_rv360_device = device;
}

void rv360_shutdown_renderer(void)
{
    rv360_unbind_transformed_pipeline();
    if (g_rv360_vertex_buffer) {
        D3DVertexBuffer_Release(g_rv360_vertex_buffer);
        g_rv360_vertex_buffer = NULL;
    }
    if (g_rv360_index_buffer) {
        D3DIndexBuffer_Release(g_rv360_index_buffer);
        g_rv360_index_buffer = NULL;
    }
    g_rv360_device = NULL;
}

HRESULT rv360_bind_transformed_pipeline(const DWORD *vertex_shader_code,
                                        const DWORD *pixel_shader_code)
{
    if (!g_rv360_device || !vertex_shader_code || !pixel_shader_code)
        return E_INVALIDARG;

    static const D3DVERTEXELEMENT9 elements[] = {
        { 0,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT,
          D3DDECLUSAGE_POSITION, 0 },
        { 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,
          D3DDECLUSAGE_COLOR, 0 },
        { 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,
          D3DDECLUSAGE_COLOR, 1 },
        { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,
          D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    if (!g_rv360_vertex_declaration)
        g_rv360_vertex_declaration = D3DDevice_CreateVertexDeclaration(elements);
    if (!g_rv360_vertex_shader)
        g_rv360_vertex_shader = D3DDevice_CreateVertexShader(vertex_shader_code);
    if (!g_rv360_pixel_shader)
        g_rv360_pixel_shader = D3DDevice_CreatePixelShader(pixel_shader_code);

    if (!g_rv360_vertex_declaration || !g_rv360_vertex_shader ||
        !g_rv360_pixel_shader)
        return E_OUTOFMEMORY;

    D3DDevice_SetVertexDeclaration(g_rv360_device, g_rv360_vertex_declaration);
    D3DDevice_SetVertexShader(g_rv360_device, g_rv360_vertex_shader);
    D3DDevice_SetPixelShader(g_rv360_device, g_rv360_pixel_shader);
    return S_OK;
}

void rv360_unbind_transformed_pipeline(void)
{
    if (g_rv360_vertex_declaration) {
        D3DVertexDeclaration_Release(g_rv360_vertex_declaration);
        g_rv360_vertex_declaration = NULL;
    }
    if (g_rv360_vertex_shader) {
        D3DVertexShader_Release(g_rv360_vertex_shader);
        g_rv360_vertex_shader = NULL;
    }
    if (g_rv360_pixel_shader) {
        D3DPixelShader_Release(g_rv360_pixel_shader);
        g_rv360_pixel_shader = NULL;
    }
    if (g_rv360_vertex_declaration2) {
        D3DVertexDeclaration_Release(g_rv360_vertex_declaration2);
        g_rv360_vertex_declaration2 = NULL;
    }
    if (g_rv360_pixel_shader2) {
        D3DPixelShader_Release(g_rv360_pixel_shader2);
        g_rv360_pixel_shader2 = NULL;
    }
}

HRESULT rv360_bind_transformed2_pipeline(const DWORD *vertex_shader_code,
                                         const DWORD *pixel_shader_code)
{
    if (!g_rv360_device || !vertex_shader_code || !pixel_shader_code)
        return E_INVALIDARG;

    static const D3DVERTEXELEMENT9 elements[] = {
        { 0,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT,
          D3DDECLUSAGE_POSITION, 0 },
        { 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,
          D3DDECLUSAGE_COLOR, 0 },
        { 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,
          D3DDECLUSAGE_COLOR, 1 },
        { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,
          D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,
          D3DDECLUSAGE_TEXCOORD, 1 },
        D3DDECL_END()
    };

    if (!g_rv360_vertex_declaration2)
        g_rv360_vertex_declaration2 = D3DDevice_CreateVertexDeclaration(elements);
    if (!g_rv360_vertex_shader)
        g_rv360_vertex_shader = D3DDevice_CreateVertexShader(vertex_shader_code);
    if (!g_rv360_pixel_shader2)
        g_rv360_pixel_shader2 = D3DDevice_CreatePixelShader(pixel_shader_code);

    if (!g_rv360_vertex_declaration2 || !g_rv360_vertex_shader ||
        !g_rv360_pixel_shader2)
        return E_OUTOFMEMORY;

    D3DDevice_SetVertexDeclaration(g_rv360_device, g_rv360_vertex_declaration2);
    D3DDevice_SetVertexShader(g_rv360_device, g_rv360_vertex_shader);
    D3DDevice_SetPixelShader(g_rv360_device, g_rv360_pixel_shader2);
    return S_OK;
}

#define RV360_TEX2_STRIDE 40

HRESULT rv360_draw_TEX2_vertices_up(D3DPRIMITIVETYPE primitive,
                                    const void *vertices, DWORD vertex_count)
{
    if (!vertices || !vertex_count) return E_INVALIDARG;
    if (vertex_count > RV360_VERTEX_BUFFER_BYTES / RV360_TEX2_STRIDE)
        return E_OUTOFMEMORY;
    if (rv360_ensure_draw_buffers() != S_OK) return E_FAIL;

    const DWORD bytes = RV360_TEX2_STRIDE * vertex_count;
    void *destination = D3DVertexBuffer_Lock(g_rv360_vertex_buffer, 0, bytes, 0);
    if (!destination) return E_FAIL;
    memcpy(destination, vertices, bytes);
    D3DVertexBuffer_Unlock(g_rv360_vertex_buffer);

    D3DDevice_SetStreamSource(g_rv360_device, 0, g_rv360_vertex_buffer,
                              0, RV360_TEX2_STRIDE, 0);
    rv360_push_fog_constant();
    D3DDevice_DrawVertices(g_rv360_device, primitive, 0, vertex_count);
    return S_OK;
}

void rv360_clear(DWORD color, DWORD flags)
{
    D3DDevice_Clear(g_rv360_device, 0, NULL, flags, color, 1.0f, 0, FALSE);
}

void rv360_present(void)
{
    D3DDevice_Present(g_rv360_device);
}

HRESULT rv360_load_texture_file(const char *filename, UINT width, UINT height,
                                UINT mip_levels, D3DTexture **texture)
{
    if (!g_rv360_device || !filename || !texture) return E_INVALIDARG;
    return D3DXCreateTextureFromFileExA(
        g_rv360_device, filename, width, height, mip_levels, 0,
        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
        0, NULL, NULL, texture);
}

void rv360_set_sampler_linear(DWORD stage, BOOL mipmapped, BOOL anisotropic)
{
    if (!g_rv360_device) return;
    const DWORD filter = anisotropic ? D3DTEXF_ANISOTROPIC : D3DTEXF_LINEAR;
    D3DDevice_SetSamplerState(g_rv360_device, stage, D3DSAMP_MINFILTER, filter);
    D3DDevice_SetSamplerState(g_rv360_device, stage, D3DSAMP_MAGFILTER, filter);
    D3DDevice_SetSamplerState(g_rv360_device, stage, D3DSAMP_MIPFILTER,
                              mipmapped ? D3DTEXF_LINEAR : D3DTEXF_NONE);
}

HRESULT rv360_draw_vertices_up(D3DPRIMITIVETYPE primitive,
                               const void *vertices, DWORD stride,
                               DWORD vertex_count)
{
    if (!vertices || !stride || !vertex_count) return E_INVALIDARG;
    if (stride > RV360_VERTEX_BUFFER_BYTES ||
        vertex_count > RV360_VERTEX_BUFFER_BYTES / stride)
        return E_OUTOFMEMORY;
    if (rv360_ensure_draw_buffers() != S_OK) return E_FAIL;

    const DWORD bytes = stride * vertex_count;
    void *destination = D3DVertexBuffer_Lock(g_rv360_vertex_buffer, 0, bytes, 0);
    if (!destination) return E_FAIL;
    memcpy(destination, vertices, bytes);
    D3DVertexBuffer_Unlock(g_rv360_vertex_buffer);

    D3DDevice_SetStreamSource(g_rv360_device, 0, g_rv360_vertex_buffer,
                              0, stride, 0);
    rv360_push_fog_constant();
    D3DDevice_DrawVertices(g_rv360_device, primitive, 0, vertex_count);
    return S_OK;
}

HRESULT rv360_draw_indexed_vertices_up(D3DPRIMITIVETYPE primitive,
                                       const void *vertices, DWORD stride,
                                       DWORD vertex_count,
                                       const WORD *indices, DWORD index_count)
{
    if (!indices || !index_count) return E_INVALIDARG;
    if (!vertices || !stride || !vertex_count) return E_INVALIDARG;
    if (stride > RV360_VERTEX_BUFFER_BYTES ||
        vertex_count > RV360_VERTEX_BUFFER_BYTES / stride)
        return E_OUTOFMEMORY;
    if (rv360_ensure_draw_buffers() != S_OK) return E_FAIL;

    const DWORD vertex_bytes = stride * vertex_count;
    void *vertex_destination = D3DVertexBuffer_Lock(
        g_rv360_vertex_buffer, 0, vertex_bytes, 0);
    if (!vertex_destination) return E_FAIL;
    memcpy(vertex_destination, vertices, vertex_bytes);
    D3DVertexBuffer_Unlock(g_rv360_vertex_buffer);
    D3DDevice_SetStreamSource(g_rv360_device, 0, g_rv360_vertex_buffer,
                              0, stride, 0);

    const DWORD bytes = sizeof(WORD) * index_count;
    if (bytes > RV360_INDEX_BUFFER_BYTES) return E_OUTOFMEMORY;
    void *destination = D3DIndexBuffer_Lock(g_rv360_index_buffer, 0, bytes, 0);
    if (!destination) return E_FAIL;
    memcpy(destination, indices, bytes);
    D3DIndexBuffer_Unlock(g_rv360_index_buffer);
    D3DDevice_SetIndices(g_rv360_device, g_rv360_index_buffer);
    rv360_push_fog_constant();
    D3DDevice_DrawIndexedVertices(g_rv360_device, primitive, 0, 0, index_count);
    return S_OK;
}

void rv360_legacy_set_vertex_format(DWORD fvf)
{
    g_rv360_legacy_fvf = fvf;
}

void rv360_legacy_set_texture_stage_state(DWORD stage, DWORD state, DWORD value)
{
    // Combiner operations are implemented by the selected pixel shader.
    // Only sampler states have a direct 360 equivalent.
    switch (state) {
        case 1001: rv360_set_sampler_state(stage, D3DSAMP_ADDRESSU, value); break;
        case 1002: rv360_set_sampler_state(stage, D3DSAMP_ADDRESSV, value); break;
        case 1003: rv360_set_sampler_state(stage, D3DSAMP_ADDRESSW, value); break;
        case 1004: rv360_set_sampler_state(stage, D3DSAMP_MAGFILTER, value); break;
        case 1005: rv360_set_sampler_state(stage, D3DSAMP_MINFILTER, value); break;
        case 1006: rv360_set_sampler_state(stage, D3DSAMP_MIPFILTER, value); break;
        default: break;
    }
}

void rv360_legacy_get_texture_stage_state(DWORD stage, DWORD state, DWORD *value)
{
    (void)stage; (void)state;
    if (value) *value = 0;
}

struct RV360_CANONICAL_VERTEX {
    float x, y, z, rhw;
    DWORD diffuse, specular;
    float u, v;
};

static HRESULT rv360_convert_position_uv(const void *vertices, DWORD count,
                                         RV360_CANONICAL_VERTEX **converted)
{
    struct POSITION_UV { float x, y, z, rhw, u, v; };
    RV360_CANONICAL_VERTEX *out =
        (RV360_CANONICAL_VERTEX*)malloc(sizeof(RV360_CANONICAL_VERTEX) * count);
    if (!out) return E_OUTOFMEMORY;
    const POSITION_UV *in = (const POSITION_UV*)vertices;
    for (DWORD i = 0; i < count; i++) {
        out[i].x = in[i].x; out[i].y = in[i].y;
        out[i].z = in[i].z; out[i].rhw = in[i].rhw;
        out[i].diffuse = 0xFFFFFFFF; out[i].specular = 0;
        out[i].u = in[i].u; out[i].v = in[i].v;
    }
    *converted = out;
    return S_OK;
}

HRESULT rv360_legacy_draw_vertices_up(D3DPRIMITIVETYPE primitive,
                                      DWORD vertex_count, const void *vertices,
                                      DWORD stride)
{
    const BOOL position_uv =
        (g_rv360_legacy_fvf & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX1 &&
        !(g_rv360_legacy_fvf & (D3DFVF_DIFFUSE | D3DFVF_SPECULAR));
    if (stride == 24 && position_uv) {
        RV360_CANONICAL_VERTEX *converted = NULL;
        HRESULT hr = rv360_convert_position_uv(vertices, vertex_count, &converted);
        if (FAILED(hr)) return hr;
        hr = rv360_draw_vertices_up(primitive, converted, sizeof(*converted), vertex_count);
        free(converted);
        return hr;
    }
    return rv360_draw_vertices_up(primitive, vertices, stride, vertex_count);
}

HRESULT rv360_legacy_draw_indexed_vertices_up(D3DPRIMITIVETYPE primitive,
                                              DWORD index_count,
                                              const WORD *indices,
                                              const void *vertices,
                                              DWORD stride)
{
    DWORD vertex_count = 0;
    for (DWORD i = 0; i < index_count; i++)
        if ((DWORD)indices[i] + 1 > vertex_count) vertex_count = indices[i] + 1;
    return rv360_draw_indexed_vertices_up(primitive, vertices, stride,
                                          vertex_count, indices, index_count);
}

#endif
