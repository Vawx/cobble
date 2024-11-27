/* cobble_gfx.h : date = November 13th 2024 5:28 pm */

#if !defined(COBBLE_GFX_H)

#include "ufbx/ufbx.h"

#define UFBX_MAX_PIECES_PER_MESH 8

typedef enum gfx_handle_type {
    GFX_HANDLE_NONE,
    GFX_HANDLE_MODEL,
    GFX_HANDLE_TEXTURE,
} gfx_handle_type;

typedef struct gfx_handle_t {
    gfx_handle_type type;
    s32 id;
} gfx_handle_t;

typedef struct gfx_vertex_t {
    vec3 position;
	vec3 normal;
	vec2 uv;
    r32 f_vertex_index;
} gfx_vertex_t;

typedef struct ufbx_mesh_piece_t {
    gfx_vertex_t *vertices;
    u32 num_vertices;
    u16 *indices;
    u32 num_indices;
} gfx_mesh_piece_t;

typedef struct gfx_mesh_object_t {
    gfx_mesh_piece_t *mesh_pieces;
    u32 mesh_pieces_count;
} gfx_mesh_object_t;

typedef struct gfx_mesh_part_t {
    sg_buffer vertex_buffer;
    sg_buffer index_buffer;
	u32 num_indices;
    s32 material_index;
    
    // TODO(kyle)
    // everything below should be removed and moved to an importing function / object.
    // no need to have this here as what is imported should just be the raw
    // vert and indice data, which directly is set in sg_buffers.
    u8 *vertices;
    u32 vertices_size;
    u8 *indices;
    u32 indices_size;
} gfx_mesh_part_t;

typedef enum gfx_texture_type {
    GFX_TEXTURE_DIFFUSE,
    
    GFX_TEXTURE_COUNT
} gfx_texture_type;

typedef struct gfx_texture_t {
    gfx_texture_type type;
    sg_image image;
    sg_sampler sampler;
    
    vec2 tiling;
    u32 w;
    u32 h;
    u32 c;
    
    u64 file_hash;
} gfx_texture_t;

typedef enum gfx_model_movement_type {
    GFX_MOVEMENT_STATIC,
    GFX_MOVEMENT_DYNAMIC,
} gfx_model_movement_type;

typedef struct gfx_mesh_t {
    s32 *indices;
    u32 num_indices; // TODO(Kyle) figure out what these are actually for.
    
	gfx_mesh_part_t *parts;
    u32 num_parts;
    
	bool aabb_is_local;
	vec3 aabb_min;
	vec3 aabb_max;
} gfx_mesh_t;

typedef struct gfx_model_t {
    gfx_mesh_t  mesh;
    gfx_model_movement_type movement_type;
    gfx_handle_t texture_handles[GFX_TEXTURE_COUNT];
    u64 file_hash;
} gfx_model_t;

typedef struct gfx_scene_t {
    gfx_model_t *models;
    u32 model_idx;
    
    gfx_texture_t *textures;
    u32 texture_idx;
    
    gfx_model_t *models_frame; // models to render in frame. 
    u32 models_frame_idx;
    
	vec3 aabb_min;
	vec3 aabb_max;
    
    u8 initialized;
} gfx_scene_t;

typedef struct gfx_static_draw_t {
    sg_shader shader;
	sg_pipeline pipeline;
} gfx_static_draw_t;

typedef struct gfx_shadow_draw_t {
    sg_shader shader;
	sg_pipeline pipeline;
} gfx_shadow_draw_t;

static void gfx_init();
static void gfx_frame();
static void gfx_end();

#define COBBLE_GFX_H
#endif //COBBLE_GFX_H
