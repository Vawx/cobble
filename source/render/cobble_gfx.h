/* cobble_gfx.h : date = November 13th 2024 5:28 pm */

#if !defined(COBBLE_GFX_H)

#include "cobble_view.h"
#include "ufbx/ufbx.h"

#define UFBX_MAX_PIECES_PER_MESH 8

typedef enum gfx_handle_type {
    GFX_HANDLE_NONE,
    GFX_HANDLE_MODEL,
    GFX_HANDLE_TEXTURE,
} gfx_handle_type;

typedef struct gfx_handle {
    gfx_handle_type type;
    s32 id;
} gfx_handle;

typedef struct mesh_vertex {
    vec3 position;
	vec3 normal;
	vec2 uv;
    r32 f_vertex_index;
} mesh_vertex;

typedef struct ufbx_mesh_piece {
    mesh_vertex *vertices;
    u32 num_vertices;
    u16 *indices;
    u32 num_indices;
} ufbx_mesh_piece;

typedef struct ufbx_mesh_object {
    ufbx_mesh_piece *mesh_pieces;
    u32 mesh_pieces_count;
} ufbx_mesh_object;

typedef struct gfx_node {
    u32 parent_index;
    
	mat4 geometry_to_node;
	mat4 node_to_parent;
	mat4 node_to_world;
	mat4 geometry_to_world;
	mat4 normal_to_world;
} gfx_node;

typedef struct gfx_mesh_part {
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
} gfx_mesh_part;

typedef enum gfx_texture_type {
    GFX_TEXTURE_DIFFUSE,
    
    GFX_TEXTURE_COUNT
} gfx_texture_type;

typedef struct gfx_texture {
    gfx_texture_type type;
    sg_image image;
    sg_sampler sampler;
    
    vec2 tiling;
    u32 w;
    u32 h;
    u32 c;
    
    u64 file_hash;
} gfx_texture;

typedef enum gfx_model_movement_type {
    GFX_MOVEMENT_STATIC,
    GFX_MOVEMENT_DYNAMIC,
} gfx_model_movement_type;

typedef struct gfx_mesh {
    s32 *indices;
    u32 num_indices; // TODO(Kyle) figure out what these are actually for.
    
	gfx_mesh_part *parts;
    u32 num_parts;
    
	bool aabb_is_local;
	vec3 aabb_min;
	vec3 aabb_max;
} gfx_mesh;

typedef struct gfx_model {
    gfx_mesh  mesh;
    gfx_model_movement_type movement_type;
    gfx_handle texture_handles[GFX_TEXTURE_COUNT];
    
    vec3 position;
    vec3 rotation;
    vec3 scale;
} gfx_model;

typedef struct gfx_scene {
    gfx_model *models;
    u32 model_idx;
    
    gfx_texture *textures;
    u32 texture_idx;
    
	vec3 aabb_min;
	vec3 aabb_max;
    
    u8 initialized;
} gfx_scene;

typedef struct gfx_static_draw {
    sg_shader shader;
	sg_pipeline pipeline;
} gfx_static_draw;

typedef struct gfx_shadow_draw {
    sg_shader shader;
	sg_pipeline pipeline;
} gfx_shadow_draw;

#define GFX_MAX_SCENE_COUNT_PER_VIEWER kilo(2)
#define GFX_MAX_TEXTURES_PER_VIEWER kilo(1)
#define GFX_MAX_MODELS_PER_VIEWER kilo(1)
typedef struct gfx_viewer {
    gfx_scene *scenes;
    u32 scenes_idx;
    u32 scenes_current;
    
    gfx_static_draw static_draw;
    gfx_shadow_draw shadow_draw;
} gfx_viewer;

static void gfx_set_model_position(gfx_model *model, vec3 pos);
static void gfx_set_model_rotation(gfx_model *model, vec3 rot);
static void gfx_set_model_scale(gfx_model *model, vec3 scale);

static void gfx_load_scene(gfx_scene *vs, const cobble_dir *dir, u8 keep_raw_data);
static gfx_handle gfx_load_mesh(gfx_viewer *viewer, const cobble_dir *dir, u8 keep_raw_data);
static gfx_handle gfx_load_texture(gfx_viewer *viewer, const cobble_dir *dir);

static gfx_handle gfx_load_texture_asset(gfx_viewer *viewer, const cobble_dir *dir);
static gfx_handle gfx_load_mesh_asset(gfx_viewer *viewer, const cobble_dir *dir);

static void gfx_init();
static void gfx_frame();
static void gfx_end();

#define COBBLE_GFX_H
#endif //COBBLE_GFX_H
