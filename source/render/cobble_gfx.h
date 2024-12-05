/* cobble_gfx.h : date = November 13th 2024 5:28 pm */

#if !defined(COBBLE_GFX_H)

#include "ufbx/ufbx.h"

typedef struct gfx_handle_t {
    u32 id;
} gfx_handle_t;

typedef struct gfx_buffered_object_t {
    sg_buffer index;
    sg_buffer vertex;
    u32 num_indices;
    u64 file_hash;
    mat4 model_matrix;
} gfx_buffered_object_t;

#define BUFFERED_OBJECT_DEFAULT_COUNT kilo(1)
typedef struct gfx_buffered_object_buffer_t {
    gfx_buffered_object_t *ptr;
    u32 idx;
    u32 count;
} gfx_buffered_object_buffer_t;

typedef struct gfx_mesh_t {
    gfx_buffered_object_t *parts;
    u32 num_parts;
} gfx_mesh_t;

typedef struct gfx_render_pass_t {
    sg_shader shader;
	sg_pipeline pipeline; 
} gfx_render_pass_t;

static void gfx_init();
static void gfx_frame();
static void gfx_shutdown();

static gfx_handle_t gfx_make_buffered_object(u32 vertex_size, u8 *vertices, u32 index_size, u32 *indices, u64 hash, u32 num_indices);
static void gfx_render_mesh(const gfx_handle_t *handle, mat4 model_matrix);

#include "cobble_ufbx.h"

#define COBBLE_GFX_H
#endif //COBBLE_GFX_H
