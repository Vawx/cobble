/* cobble_gfx.h : date = November 13th 2024 5:28 pm */

#if !defined(COBBLE_GFX_H)

#include "cobble_view.h"
#include "ufbx/ufbx.h"

typedef enum gfx_view_type {
    VIEW_TYPE_DEFAULT,
    VIEW_TYPE_ONE,
    VIEW_TYPE_TWO,
    VIEW_TYPE_THREE,
    VIEW_TYPE_FOUR,
    VIEW_TYPE_FIVE,
    VIEW_TYPE_SIX,
    VIEW_TYPE_SEVEN,
    VIEW_TYPE_COUNT,
} gfx_view_type;

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

typedef struct gfx_model_handle_t {
    gfx_handle_t view_handle;
    gfx_handle_t mesh_handle;
} gfx_model_handle_t;

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

#define GFX_MAX_SCENE_COUNT_PER_VIEWER kilo(2)
#define GFX_MAX_TEXTURES_PER_VIEWER kilo(1)
#define GFX_MAX_MODELS_PER_VIEWER kilo(1)
typedef struct gfx_viewer_t {
    gfx_scene_t *scenes;
    u32 scenes_idx;
    u32 scenes_current;
} gfx_viewer_t;

typedef struct gfx_viewer_buffer_t {
    gfx_viewer_t views[VIEW_TYPE_COUNT]; // Note(Kyle) this is entirely arbitrary and if scenes need more than 8, change this.
    u8 views_idx;
} gfx_viewer_buffer_t;

static gfx_model_t *gfx_retrieve_asset(gfx_viewer_t *viewer, gfx_handle_t *handle);

static void gfx_load_scene(gfx_scene_t *vs, const dir_t *dir, u8 keep_raw_data);
static gfx_handle_t gfx_load_texture_asset(gfx_viewer_t *viewer, const dir_t *dir);
static gfx_handle_t gfx_load_mesh_asset(gfx_viewer_t *viewer, const dir_t *dir);

static gfx_model_handle_t gfx_make_model_handle(const dir_t *dir);
static gfx_model_handle_t gfx_make_model_handle_from_specific_view(const dir_t *dir, gfx_viewer_t *viewer);

static void gfx_push_to_render(gfx_model_handle_t *model_handle, vec3 pos, vec3 rot, vec3 scale);

static void gfx_init();
static void gfx_frame();
static void gfx_end();

#define COBBLE_GFX_H
#endif //COBBLE_GFX_H
