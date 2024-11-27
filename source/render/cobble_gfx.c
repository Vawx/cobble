#include "cobble_shader.h"

static gfx_static_draw_t static_draw;
static gfx_shadow_draw_t shadow_draw;

static const char *DEFAULT_DIFFUSE_NAME = "default_diffuse.asset";
static gfx_handle_t default_texture_handle;

#define ufbx_to_vec2(v) (vec2){v.x, v.y}
#define ufbx_to_vec3(v) (vec3){v.x, v.y, v.z}
#define ufbx_to_mat4(m) (mat4){m.m00, m.m01, m.m02, m.m03, m.m10, m.m11, m.m12, m.m13, m.m20, m.m21, m.m22, m.m23, 0, 0, 0, 1,}

size_t min_sz(size_t a, size_t b) { return a < b ? a : b; }
size_t max_sz(size_t a, size_t b) { return b < a ? a : b; }
size_t clamp_sz(size_t a, size_t min_a, size_t max_a) { return min_sz(max_sz(a, min_a), max_a); }

static const sg_vertex_layout_state mesh_vertex_layout = {
	.attrs = {
		{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3 },
		{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3 },
		{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT2 },
		{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT },
	},
};

static void read_mesh(gfx_mesh_t *vmesh, ufbx_mesh *mesh, u8 keep_raw_data) {
	// Count the number of needed parts and temporary buffers
	size_t max_parts = 0;
	size_t max_triangles = 0;
    
	// We need to render each material of the mesh in a separate part, so let's
	// count the number of parts and maximum number of triangles needed.
	for (size_t pi = 0; pi < mesh->material_parts.count; pi++) {
		ufbx_mesh_part *part = &mesh->material_parts.data[pi];
		if (part->num_triangles == 0) continue;
		max_parts += 1;
		max_triangles = max_sz(max_triangles, part->num_triangles);
	}
    
	// Temporary buffers
	size_t num_tri_indices = mesh->max_face_triangles * 3;
    u32 *tri_indices = (u32*)c_alloc(sizeof(u32) * num_tri_indices);
    gfx_vertex_t *vertices = (gfx_vertex_t*)c_alloc(sizeof(gfx_vertex_t) * max_triangles * 3);
    u32 *indices = (u32*)c_alloc(sizeof(u32) * max_triangles * 3);
    
	// Result buffers
	gfx_mesh_part_t *parts = (gfx_mesh_part_t*)c_alloc(sizeof(gfx_mesh_part_t) * max_parts);
	size_t num_parts = 0;
    
	// In FBX files a single mesh can be instanced by multiple nodes. ufbx handles the connection
	// in two ways: (1) `ufbx_node.mesh/light/camera/etc` contains pointer to the data "attribute"
	// that node uses and (2) each element that can be connected to a node contains a list of
	// `ufbx_node*` instances eg. `ufbx_mesh.instances`.
	vmesh->num_indices = mesh->instances.count;
	vmesh->indices = (s32*)c_alloc(sizeof(s32) * mesh->instances.count);
	for (size_t i = 0; i < mesh->instances.count; i++) {
		vmesh->indices[i] = (int32_t)mesh->instances.data[i]->typed_id;
	}
    
	// Our shader supports only a single material per draw call so we need to split the mesh
	// into parts by material. `ufbx_mesh_part` contains a handy compact list of faces
	// that use the material which we use here.
	for (size_t pi = 0; pi < mesh->material_parts.count; pi++) {
		ufbx_mesh_part *mesh_part = &mesh->material_parts.data[pi];
		if (mesh_part->num_triangles == 0) continue;
        
		gfx_mesh_part_t *part = &parts[num_parts++];
		size_t num_indices = 0;
        
		// First fetch all vertices into a flat non-indexed buffer, we also need to
		// triangulate the faces
		for (size_t fi = 0; fi < mesh_part->num_faces; fi++) {
			ufbx_face face = mesh->faces.data[mesh_part->face_indices.data[fi]];
			size_t num_tris = ufbx_triangulate_face(tri_indices, num_tri_indices, mesh, face);
            
			ufbx_vec2 default_uv = { 0 };
            
			// Iterate through every vertex of every triangle in the triangulated result
			for (size_t vi = 0; vi < num_tris * 3; vi++) {
				uint32_t ix = tri_indices[vi];
				gfx_vertex_t *vert = &vertices[num_indices];
                
				ufbx_vec3 pos = ufbx_get_vertex_vec3(&mesh->vertex_position, ix);
				ufbx_vec3 normal = ufbx_get_vertex_vec3(&mesh->vertex_normal, ix);
				ufbx_vec2 uv = mesh->vertex_uv.exists ? ufbx_get_vertex_vec2(&mesh->vertex_uv, ix) : default_uv;
                
                glm_vec3_copy(ufbx_to_vec3(pos), vert->position);
                
                vec3 norm = {0};
                glm_vec3_copy(ufbx_to_vec3(normal), norm);
                glm_normalize(norm);
                glm_vec3_copy(norm, vert->normal);
                
                glm_vec2_copy(ufbx_to_vec2(uv), vert->uv);
				
                vert->f_vertex_index = (r32)mesh->vertex_indices.data[ix];
                
				num_indices++;
			}
		}
        
		ufbx_vertex_stream streams[2];
		size_t num_streams = 1;
        
		streams[0].data = vertices;
		streams[0].vertex_count = num_indices;
		streams[0].vertex_size = sizeof(gfx_vertex_t);
        
		// Optimize the flat vertex buffer into an indexed one. `ufbx_generate_indices()`
		// compacts the vertex buffer and returns the number of used vertices.
		ufbx_error error;
		size_t num_vertices = ufbx_generate_indices(streams, num_streams, indices, num_indices, NULL, &error);
		if (error.type != UFBX_ERROR_NONE) {
            // "Failed to generate index buffer"
			c_assert_break();
		}
        
		part->num_indices = num_indices;
		if (mesh_part->index < mesh->materials.count) {
			ufbx_material *material =  mesh->materials.data[mesh_part->index];
			part->material_index = (int32_t)material->typed_id;
		} else {
			part->material_index = -1;
		}
        
		// Create the GPU buffers from the temporary `vertices` and `indices` arrays
		part->index_buffer = sg_make_buffer(&(sg_buffer_desc){
                                                .size = num_indices * sizeof(u32),
                                                .type = SG_BUFFERTYPE_INDEXBUFFER,
                                                .data = { indices, num_indices * sizeof(u32) },
                                            });
		part->vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
                                                 .size = num_vertices * sizeof(gfx_vertex_t),
                                                 .type = SG_BUFFERTYPE_VERTEXBUFFER,
                                                 .data = { vertices, num_vertices * sizeof(gfx_vertex_t) },
                                             });
        
        // this may get cleared below.
        part->vertices = (u8*)vertices;
        part->vertices_size = num_vertices * sizeof(gfx_vertex_t);
        
        part->indices = (u8*)indices;
        part->indices_size = num_indices * sizeof(u32);
    }
    
	// Free the temporary buffers
	c_free((u8*)tri_indices);
    
    // maybe keep others, this is for importing mesh assets.
    if(!keep_raw_data) {
        c_free((u8*)vertices);
        c_free((u8*)indices);
    } 
    
	// Compute bounds from the vertices
	vmesh->aabb_is_local = mesh->skinned_is_local;
	glm_vec3_copy((vec3){+INFINITY, +INFINITY, +INFINITY}, vmesh->aabb_min);
    glm_vec3_copy((vec3){-INFINITY, -INFINITY, -INFINITY}, vmesh->aabb_max);
    
	for (size_t i = 0; i < mesh->num_vertices; i++) {
		vec3 pos = {0};
        glm_vec3_copy(ufbx_to_vec3(mesh->skinned_position.values.data[i]), pos);
		glm_vec3_copy(vec3_min(vmesh->aabb_min, pos), vmesh->aabb_min);
        glm_vec3_copy(vec3_max(vmesh->aabb_max, pos), vmesh->aabb_max);
	}
    
	vmesh->parts = parts;
	vmesh->num_parts = num_parts;
}

static void vec3_transform_point(const mat4 *a, vec3 b, vec3 *r) {
	r32 a1 = a[0][0][0] * b[0];
    r32 a2 = a[0][0][1] * b[1];
    r32 a3 = a[0][0][2] * b[2];
    r32 a4 = a[0][0][3];
    
    r32 b1 = a[0][1][0] * b[0];
    r32 b2 = a[0][1][1] * b[1];
    r32 b3 = a[0][1][2] * b[2];
    r32 b4 = a[0][1][3];
    
    r32 c1 = a[0][2][0] * b[0];
    r32 c2 = a[0][2][1] * b[1];
    r32 c3 = a[0][2][2] * b[2];
    r32 c4 = a[0][2][3];
    
    r[0][0] = a1 + a1 + a3 + a4;
    r[0][1] = b1 + b2 + b3 + b4;
    r[0][2] = c1 + c2 + c3 + c4;
}

static void vec3_transform_extent(const mat4 *a, vec3 b, vec3 *r) {
	r32 a1 = abs(a[0][0][0]) * b[0];
    r32 a2 = abs(a[0][0][1]) * b[1];
    r32 a3 = abs(a[0][0][2]) * b[2];
    
    r32 b1 = abs(a[0][1][0]) * b[0];
    r32 b2 = abs(a[0][1][1]) * b[1];
    r32 b3 = abs(a[0][1][2]) * b[2];
    
    r32 c1 = abs(a[0][2][0]) * b[0];
    r32 c2 = abs(a[0][2][1]) * b[1];
    r32 c3 = abs(a[0][2][2]) * b[2];
    
    r[0][0] = a1 + a1 + a3;
    r[0][1] = b1 + b2 + b3;
    r[0][2] = c1 + c2 + c3;
}

static void read_scene(gfx_scene_t *vs, ufbx_scene *scene, u8 keep_raw_data) {
    c_assert(vs->initialized);
	
	vs->model_idx = scene->meshes.count;
	vs->models = (gfx_model_t*)c_alloc(sizeof(gfx_model_t) * vs->model_idx);
	for (size_t i = 0; i < vs->model_idx; i++) {
		read_mesh(&vs->models[i].mesh, scene->meshes.data[i], keep_raw_data);
	}
}

static void vec3_mulf(const vec3 *a, r32 b, vec3 *dest) {
    dest[0][0] = a[0][0] * b;
    dest[0][1] = a[0][1] * b;
    dest[0][2] = a[0][2] * b;
}

static void gfx_load_scene(gfx_scene_t *vs, const dir_t *dir, u8 keep_raw_data) {
    c_assert(vs->initialized);
    
    LOG_TELL("loading fbx scene %s", dir->ptr);
    
	ufbx_load_opts opts = {
		.load_external_files = true,
		.ignore_missing_external_files = true,
		.generate_missing_normals = true,
        
		// NOTE: We use this _only_ for computing the bounds of the scene!
		// The viewer contains a proper implementation of skinning as well.
		// You probably don't need this.
		.evaluate_skinning = true,
        
		.target_axes = {
			.right = UFBX_COORDINATE_AXIS_POSITIVE_X,
			.up = UFBX_COORDINATE_AXIS_POSITIVE_Y,
			.front = UFBX_COORDINATE_AXIS_POSITIVE_Z,
		},
		.target_unit_meters = 1.0f,
	};
	ufbx_error error;
	ufbx_scene *scene = ufbx_load_file(dir->ptr, &opts, &error);
    c_assert(scene != NULL);
    
	read_scene(vs, scene, keep_raw_data);
    
	// Compute the world-space bounding box
	glm_vec3_copy((vec3){+INFINITY, +INFINITY, +INFINITY}, vs->aabb_min);
    glm_vec3_copy((vec3){-INFINITY, -INFINITY, -INFINITY}, vs->aabb_max);
	for (size_t mesh_ix = 0; mesh_ix < vs->model_idx; mesh_ix++) {
		gfx_mesh_t *mesh = &vs->models[mesh_ix].mesh;
        vec3 aabb_origin = {0};
        vec3 aabb_extent = {0};
        
        vec3 mesh_aabb_max_min = {0};
        glm_vec3_add(mesh->aabb_max, mesh->aabb_min, mesh_aabb_max_min);
        vec3_mulf(&mesh_aabb_max_min, 0.5, &aabb_origin);
        
        vec3 mesh_sub_aabb_max_min = {0};
        glm_vec3_sub(mesh->aabb_max, mesh->aabb_min, mesh_sub_aabb_max_min);
        vec3_mulf(&mesh_sub_aabb_max_min, 0.5, &aabb_extent);
        
		if (mesh->aabb_is_local) {
			for (size_t inst_ix = 0; inst_ix < mesh->num_indices; inst_ix++) {
				
                vec3 world_origin = {0};
                MAT4(ident);
                vec3_transform_point(&ident, aabb_origin, &world_origin);
                
                vec3 world_extent = {0};
                vec3 origin_add_extent = {0};
                vec3 origin_sub_extent = {0};
                
                vec3_transform_extent(&ident, aabb_extent, &world_extent);
                
                glm_vec3_sub(world_origin, world_extent, origin_sub_extent);
                glm_vec3_add(world_origin, world_extent, origin_add_extent);
                glm_vec3_copy(vec3_min(origin_sub_extent, vs->aabb_min), vs->aabb_min);
                glm_vec3_copy(vec3_max(origin_add_extent, vs->aabb_max), vs->aabb_max);
			}
		} else {
            glm_vec3_copy(vec3_min(vs->aabb_min, mesh->aabb_min), vs->aabb_min);
			glm_vec3_copy(vec3_max(vs->aabb_max, mesh->aabb_max), vs->aabb_max);
		}
	}
    
	ufbx_free_scene(scene);
}

static void gfx_init() {
    sg_backend backend = sg_query_backend();
    
	static_draw.shader = sg_make_shader(static_lit_shader_desc(backend));
	static_draw.pipeline = sg_make_pipeline(&(sg_pipeline_desc){
                                                .shader = static_draw.shader,
                                                .layout = mesh_vertex_layout,
                                                .index_type = SG_INDEXTYPE_UINT32,
                                                .face_winding = SG_FACEWINDING_CCW,
                                                .cull_mode = SG_CULLMODE_BACK,
                                                .depth = {
                                                    .compare = SG_COMPAREFUNC_LESS_EQUAL,
                                                    .write_enabled = true,
                                                },
                                            });
}

static void draw_mesh() {
    /*
    view_t *v = get_current_view();
    MAT4(proj_view);
    glm_mat4_mul(v->projection, v->view, proj_view);
    
    MAT4(ident);
    
    mesh_vertex_ubo_t mesh_ubo = {0};
    glm_mat4_copy(ident, mesh_ubo.normal_to_world);
    glm_mat4_copy(ident, mesh_ubo.geometry_to_world);
    
    glm_mat4_identity(mesh_ubo.world_to_clip); // default mat4
    glm_mat4_mul(proj_view, mesh_ubo.world_to_clip, mat);
    
    mesh_ubo.f_num_blend_shapes = 0.f;
    
	sg_apply_uniforms(0, SG_RANGE_REF(mesh_ubo));
    
	for (size_t pi = 0; pi < model->mesh.num_parts; pi++) {
		gfx_mesh_part_t *part = &model->mesh.parts[pi];
		uv_tiling_ubo_t tiling_ubo = {
            .tile_x = view->scenes[view->scenes_current].textures[default_texture_handle.id].tiling[0],
            .tile_y = view->scenes[view->scenes_current].textures[default_texture_handle.id].tiling[1],
        };
        sg_apply_uniforms(UB_uv_tiling_ubo, &SG_RANGE(tiling_ubo));
        sg_bindings binds = {
			.vertex_buffers[0] = part->vertex_buffer,
            .index_buffer = part->index_buffer,
            .images[IMG_diffuse_texture] = view->scenes[view->scenes_current].textures[default_texture_handle.id].image,
            .samplers[SMP_diffuse_sampler] = view->scenes[view->scenes_current].textures[default_texture_handle.id].sampler
		};
        
		sg_apply_bindings(&binds);
		sg_draw(0, (s32)part->num_indices, 1);
	}
*/
}

static void gfx_draw_scene() {
}

static void gfx_frame() {
    sg_pass_action action = {
		.colors[0] = {
			.load_action = SG_LOADACTION_CLEAR,
			.clear_value = { 0.1f, 0.1f, 0.1f },
		},
	};
    sg_begin_pass(&(sg_pass){.action = action, .swapchain = sglue_swapchain()});
    sg_apply_pipeline(static_draw.pipeline);
    
    sg_end_pass();
    sg_commit();
}

static void gfx_end() {
    
}

#include "cobble_render_imgui.c"
#include "cobble_ufbx.c"