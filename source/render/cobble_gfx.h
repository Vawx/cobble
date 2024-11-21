/* cobble_gfx.h : date = November 13th 2024 5:28 pm */

#if !defined(COBBLE_GFX_H)

#include "cobble_ufbx.h"
#include "cobble_view.h"

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
} gfx_mesh_part;

typedef struct gfx_mesh {
    s32 *instance_node_indices;
    u32 num_instances;
    
	gfx_mesh_part *parts;
    u32 num_parts;
    
	bool aabb_is_local;
	vec3 aabb_min;
	vec3 aabb_max;
} gfx_mesh;

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
} gfx_texture;

typedef enum gfx_model_movement_type {
    GFX_MOVEMENT_STATIC,
    GFX_MOVEMENT_DYNAMIC,
} gfx_model_movement_type;

typedef struct gfx_model {
    gfx_mesh  mesh;
    gfx_model_movement_type movement_type;
    gfx_handle texture_handles[GFX_TEXTURE_COUNT];
    
    vec3 position;
    vec3 rotation;
    vec3 scale;
} gfx_model;

typedef struct gfx_scene {
    gfx_node *nodes;
    u32 num_nodes;
    
	gfx_model *models;
    u32 num_models;
    
    gfx_handle texture_handles[GFX_TEXTURE_COUNT];
    u32 num_texture_handles;
    
	vec3 aabb_min;
	vec3 aabb_max;
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
typedef struct gfx {
    gfx_scene *scenes;
    u32 scenes_idx;
    
    gfx_texture *textures;
    u32 textures_idx;
    
    gfx_static_draw static_draw;
    gfx_shadow_draw shadow_draw;
} gfx_viewer;

static void gfx_set_model_position(gfx_model *model, vec3 pos);
static void gfx_set_model_rotation(gfx_model *model, vec3 rot);
static void gfx_set_model_scale(gfx_model *model, vec3 scale);

static gfx_handle gfx_load_mesh(gfx_viewer *viewer, const cobble_dir *dir);
static gfx_handle gfx_load_texture(gfx_viewer *viewer, const cobble_dir *dir);

static void gfx_init();
static void gfx_frame();
static void gfx_end();

#define COBBLE_GFX_H
#endif //COBBLE_GFX_H
