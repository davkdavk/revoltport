#ifndef RV360_RENDER_API_H
#define RV360_RENDER_API_H

// Renderer boundary for the Xbox 360 port.
// This intentionally contains no XDK types. The XDK backend will implement
// these operations once the complete compiler/header installation is present.

#include <stddef.h>
#include <stdint.h>

typedef struct rv360_render_vertex {
    float x, y, z, rhw;
    uint32_t diffuse;
    uint32_t specular;
    float u, v;
} rv360_render_vertex;

typedef enum rv360_sampler_state {
    RV360_SAMPLER_ADDRESS_U,
    RV360_SAMPLER_ADDRESS_V,
    RV360_SAMPLER_ADDRESS_W,
    RV360_SAMPLER_MIN_FILTER,
    RV360_SAMPLER_MAG_FILTER,
    RV360_SAMPLER_MIP_FILTER,
    RV360_SAMPLER_LOD_BIAS,
    RV360_SAMPLER_MAX_ANISOTROPY
} rv360_sampler_state;

typedef enum rv360_render_state {
    RV360_RENDER_FOG_ENABLE,
    RV360_RENDER_FOG_COLOR,
    RV360_RENDER_ALPHA_BLEND_ENABLE,
    RV360_RENDER_SOURCE_BLEND,
    RV360_RENDER_DEST_BLEND,
    RV360_RENDER_ALPHA_TEST_ENABLE,
    RV360_RENDER_ALPHA_REFERENCE,
    RV360_RENDER_ALPHA_FUNCTION,
    RV360_RENDER_Z_ENABLE,
    RV360_RENDER_Z_WRITE_ENABLE,
    RV360_RENDER_Z_FUNCTION,
    RV360_RENDER_CULL_MODE,
    RV360_RENDER_FILL_MODE,
    RV360_RENDER_DITHER_ENABLE,
    RV360_RENDER_SPECULAR_ENABLE
} rv360_render_state;

// Backend functions are declarations only until the XDK-specific backend is
// added. Keeping the signatures here prevents game code from depending on
// OG-Xbox D3D8 names during the renderer migration.
void rv360_set_render_state(rv360_render_state state, uint32_t value);
void rv360_set_sampler_state(unsigned stage, rv360_sampler_state state, uint32_t value);
void rv360_set_texture(unsigned stage, void *texture);
void rv360_draw_vertices(unsigned primitive_type,
                         const rv360_render_vertex *vertices,
                         size_t vertex_count);
void rv360_draw_indexed_vertices(unsigned primitive_type,
                                 const rv360_render_vertex *vertices,
                                 size_t vertex_count,
                                 const uint16_t *indices,
                                 size_t index_count);

#endif
