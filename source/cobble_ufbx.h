/* cobble_ufbx.h : date = November 18th 2024 6:41 pm */

#if !defined(COBBLE_UFBX_H)

#include "ufbx/ufbx.h"

typedef struct mesh_vertex_t {
    vec3 position;
    vec3 normal;
    vec2 uv;
    r32 f_vertex_index;
} gfx_mesh_vertex_t;

typedef struct ufbx_mesh_part_t {
    gfx_handle_t obj_handle; // to the buffered object, in gfx
    u32 material_index;
} ufbx_mesh_part_t;

typedef struct ufbx_mesh_t {
    u32 *instance_node_indices;
    u32 num_instances;
    
	ufbx_mesh_part_t *parts;
    u32 num_parts;
    
    u8 aabb_is_local;
	vec3 aabb_min;
	vec3 aabb_max;
} ufbx_mesh_t;

typedef struct ufbx_node_t {
    u32 parent_index;
	mat4 geometry_to_node;
	mat4 node_to_parent;
	mat4 node_to_world;
	mat4 geometry_to_world;
	mat4 normal_to_world;
} ufbx_node_t;

#define UFBX_MODEL_DEFAULT_COUNT kilo(1)
typedef struct ufbx_model_t {
	ufbx_node_t *nodes;
    u32 num_nodes;
    
    ufbx_mesh_t *meshes;
	u32 num_meshes;
    
	vec3 aabb_min;
	vec3 aabb_max;
    
    u64 file_hash;
} ufbx_model_t;
TYPE_BUFFER(ufbx_model_t);

static gfx_handle_t gfx_load_model_fbx(dir_t *dir);

#define COBBLE_UFBX_H
#endif //COBBLE_UFBX_H
