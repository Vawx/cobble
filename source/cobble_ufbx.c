
static void ufbx_init() {
    model_buffer.ptr = (ufbx_model_t*)c_alloc(sizeof(ufbx_model_t) * UFBX_MODEL_DEFAULT_COUNT);
    model_buffer.count = UFBX_MODEL_DEFAULT_COUNT;
    model_buffer.idx = 0;
}

static void gfx_read_mesh(ufbx_mesh_t *vmesh, ufbx_mesh *mesh, u64 file_hash) {
	// Count the number of needed parts and temporary buffers
	u32 max_parts = 0;
	u32 max_triangles = 0;
    
	// We need to render each material of the mesh in a separate part, so let's
	// count the number of parts and maximum number of triangles needed.
	for (u32 pi = 0; pi < mesh->material_parts.count; pi++) {
		ufbx_mesh_part *part = &mesh->material_parts.data[pi];
		if (part->num_triangles == 0) continue;
		max_parts += 1;
		max_triangles = max_sz(max_triangles, part->num_triangles);
	}
    
	// Temporary buffers
	u32 num_tri_indices = mesh->max_face_triangles * 3;
	u32 *tri_indices = (u32*)c_alloc(sizeof(u32) * num_tri_indices);
	gfx_mesh_vertex_t *vertices = (gfx_mesh_vertex_t*)c_alloc(sizeof(gfx_mesh_vertex_t) * max_triangles * 3);
	u32 *indices = (u32*)c_alloc(sizeof(u32) * max_triangles * 3);
    
	// Result buffers
	ufbx_mesh_part_t *parts = (ufbx_mesh_part_t*)c_alloc(sizeof(ufbx_mesh_part_t) * max_parts);
	u32 num_parts = 0;
    
	// In FBX files a single mesh can be instanced by multiple nodes. ufbx handles the connection
	// in two ways: (1) `ufbx_node.mesh/light/camera/etc` contains pointer to the data "attribute"
	// that node uses and (2) each element that can be connected to a node contains a list of
	// `ufbx_node*` instances eg. `ufbx_mesh.instances`.
	vmesh->num_instances = mesh->instances.count;
	vmesh->instance_node_indices = (s32*)c_alloc(sizeof(s32) * mesh->instances.count);
	for (u32 i = 0; i < mesh->instances.count; i++) {
		vmesh->instance_node_indices[i] = (s32)mesh->instances.data[i]->typed_id;
	}
    
	// Our shader supports only a single material per draw call so we need to split the mesh
	// into parts by material. `ufbx_mesh_part` contains a handy compact list of faces
	// that use the material which we use here.
	for (u32 pi = 0; pi < mesh->material_parts.count; pi++) {
		ufbx_mesh_part *mesh_part = &mesh->material_parts.data[pi];
		if (mesh_part->num_triangles == 0) continue;
        
		ufbx_mesh_part_t *part = &parts[num_parts++];
		u32 num_indices = 0;
        
		// First fetch all vertices into a flat non-indexed buffer, we also need to
		// triangulate the faces
		for (u32 fi = 0; fi < mesh_part->num_faces; fi++) {
			ufbx_face face = mesh->faces.data[mesh_part->face_indices.data[fi]];
			u32 num_tris = ufbx_triangulate_face(tri_indices, num_tri_indices, mesh, face);
            
			ufbx_vec2 default_uv = { 0 };
            
			// Iterate through every vertex of every triangle in the triangulated result
			for (u32 vi = 0; vi < num_tris * 3; vi++) {
				u32 ix = tri_indices[vi];
				gfx_mesh_vertex_t *vert = &vertices[num_indices];
                
				ufbx_vec3 pos = ufbx_get_vertex_vec3(&mesh->vertex_position, ix);
				ufbx_vec3 normal = ufbx_get_vertex_vec3(&mesh->vertex_normal, ix);
				ufbx_vec2 uv = mesh->vertex_uv.exists ? ufbx_get_vertex_vec2(&mesh->vertex_uv, ix) : default_uv;
                
				vert->position[0] = pos.x;
                vert->position[1] = pos.y;
                vert->position[2] = pos.z;
                vert->normal[0] = normal.x;
                vert->normal[1] = normal.y;
                vert->normal[2] = normal.z;
                glm_normalize(vert->normal);
                vert->uv[0] = uv.x;
                vert->uv[1] = uv.y;
                
				vert->f_vertex_index = (r32)mesh->vertex_indices.data[ix];
                
				num_indices++;
			}
		}
        
		ufbx_vertex_stream streams[2];
		u32 num_streams = 1;
        
		streams[0].data = vertices;
		streams[0].vertex_count = num_indices;
		streams[0].vertex_size = sizeof(gfx_mesh_vertex_t);
        
		// Optimize the flat vertex buffer into an indexed one. `ufbx_generate_indices()`
		// compacts the vertex buffer and returns the number of used vertices.
		ufbx_error error;
		u32 num_vertices = ufbx_generate_indices(streams, num_streams, indices, num_indices, NULL, &error);
		if (error.type != UFBX_ERROR_NONE) {
			LOG_PANIC("Failed to generate index buffer %s", error.description);
        }
        
		if (mesh_part->index < mesh->materials.count) {
			ufbx_material *material =  mesh->materials.data[mesh_part->index];
			part->material_index = (int32_t)material->typed_id;
		} else {
			part->material_index = -1;
		}
        
        part->obj_handle = gfx_make_buffered_object(num_vertices * sizeof(gfx_mesh_vertex_t), (u8*)vertices, num_indices * sizeof(u32), indices, file_hash, num_indices);
	}
    
	// Free the temporary buffers
	c_free((u8*)tri_indices);
	c_free((u8*)vertices);
	c_free((u8*)indices);
    
	// Compute bounds from the vertices
	vmesh->aabb_is_local = mesh->skinned_is_local;
	vmesh->aabb_min[0] = +INFINITY;
    vmesh->aabb_min[1] = +INFINITY;
    vmesh->aabb_min[2] = +INFINITY;
    
	vmesh->aabb_max[0] = -INFINITY;
    vmesh->aabb_max[1] = -INFINITY;
    vmesh->aabb_max[2] = -INFINITY;
    
	for (u32 i = 0; i < mesh->num_vertices; i++) {
		vec3 pos = {0};
        pos[0] = mesh->skinned_position.values.data[i].x;
        pos[1] = mesh->skinned_position.values.data[i].y;
        pos[2] = mesh->skinned_position.values.data[i].z;
        
        glm_vec3_copy(vec3_min(vmesh->aabb_min, pos), vmesh->aabb_min);
        glm_vec3_copy(vec3_max(vmesh->aabb_max, pos), vmesh->aabb_max);
	}
    
	vmesh->parts = parts;
	vmesh->num_parts = num_parts;
}

static void ufbx_mat4_to_mat4(const ufbx_matrix *m, mat4 out) {
    out[0][0] = m->m00;
    out[0][1] = m->m01;
    out[0][2] = m->m02;
    out[0][3] = m->m03;
    
    out[1][0] = m->m11;
    out[1][1] = m->m11;
    out[1][2] = m->m12;
    out[1][3] = m->m13;
    
    out[2][0] = m->m20;
    out[2][1] = m->m21;
    out[2][2] = m->m22;
    out[2][3] = m->m23;
    
    out[3][0] = 0.f;
    out[3][1] = 0.f;
    out[3][2] = 0.f;
    out[3][3] = 1;
}

void gfx_read_node(ufbx_node_t *vnode, ufbx_node *node) {
	vnode->parent_index = node->parent ? node->parent->typed_id : -1;
	ufbx_mat4_to_mat4(&node->node_to_parent, vnode->node_to_parent);
    ufbx_mat4_to_mat4(&node->node_to_world, vnode->node_to_world);
    ufbx_mat4_to_mat4(&node->geometry_to_node, vnode->geometry_to_node);
    ufbx_mat4_to_mat4(&node->geometry_to_world, vnode->geometry_to_world);
    
    ufbx_matrix mm = ufbx_matrix_for_normals(&node->geometry_to_world);
    ufbx_mat4_to_mat4(&mm, vnode->node_to_parent);
}

void gfx_read_scene(ufbx_model_t *vs, ufbx_scene *scene, u64 file_hash) {
	vs->num_nodes = scene->nodes.count;
	vs->nodes = (ufbx_node_t*)c_alloc(sizeof(ufbx_node_t) * vs->num_nodes);
	for (u32 i = 0; i < vs->num_nodes; i++) {
		gfx_read_node(&vs->nodes[i], scene->nodes.data[i]);
	}
    
    vs->num_meshes = scene->meshes.count;
	vs->meshes = (ufbx_mesh_t*)c_alloc(sizeof(ufbx_mesh_t) * vs->num_meshes);
	for (u32 i = 0; i < vs->num_meshes; i++) {
		gfx_read_mesh(&vs->meshes[i], scene->meshes.data[i], file_hash);
	}
}

static gfx_handle_t gfx_load_model_fbx(dir_t *dir) {
    // check if already created
    u64 file_hash = dir_hash(dir);
    for(s32 i = 0; i < model_buffer.idx; ++i) {
        if(file_hash == model_buffer.ptr[i].file_hash) {
            gfx_handle_t handle = {0};
            handle.id = i;
            return handle; // found created, give that one back instead of loading it again.
            // expected here that the model_buffer is initialized with sokol/gpu
        }
    }
    
    // wasnt found, load it.
    c_assert(model_buffer.ptr != NULL);
    ufbx_model_t *model = &model_buffer.ptr[model_buffer.idx++]; // increment here
    c_assert(model_buffer.idx < model_buffer.count);
    
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
	if (!scene) {
		LOG_PANIC("Failed to load scene %s", error.description);
        gfx_handle_t r = {-1};
		return r;
	}
    
    model->file_hash = dir_hash(dir);
	gfx_read_scene(model, scene, model->file_hash);
    
	// Compute the world-space bounding box
	model->aabb_min[0] = +INFINITY;
    model->aabb_min[1] = +INFINITY;
    model->aabb_min[2] = +INFINITY;
    
	model->aabb_max[0] = -INFINITY;
    model->aabb_max[1] = -INFINITY;
    model->aabb_max[2] = -INFINITY;
    
	for (u32 mesh_ix = 0; mesh_ix < model->num_meshes; mesh_ix++) {
		ufbx_mesh_t *mesh = &model->meshes[mesh_ix];
        vec3 mesh_aabb_max = {0};
        glm_vec3_copy(mesh->aabb_max, mesh_aabb_max);
        vec3 mesh_aabb_min = {0};
        glm_vec3(mesh->aabb_min, mesh_aabb_min);
        
        vec3 aabb_origin = {0};
        vec3 max_min_add_aabb = {0};
        glm_vec3_add(mesh->aabb_max, mesh->aabb_min, max_min_add_aabb);
        glm_vec3_scale(max_min_add_aabb, 0.5f, aabb_origin);
        
        vec3 max_sub_min_aabb = {0};
        glm_vec3_sub(mesh->aabb_max, mesh->aabb_min, max_sub_min_aabb);
        
        vec3 aabb_extent = {0};
        glm_vec3_scale(max_sub_min_aabb, 0.5f, aabb_extent);
        
		if (mesh->aabb_is_local) {
			for (u32 inst_ix = 0; inst_ix < mesh->num_instances; inst_ix++) {
				ufbx_node_t *node = &model->nodes[mesh->instance_node_indices[inst_ix]];
                
                vec3 world_origin = {0};
                vec3_transform_point(node->geometry_to_world, aabb_origin, world_origin);
                
                vec3 world_extent = {0};
                vec3_transform_extent(node->geometry_to_world, aabb_extent, world_extent);
				
                vec3 origin_sub_extent = {0};
                glm_vec3_sub(world_origin, world_extent, origin_sub_extent);
                vec3 origin_add_extent = {0};
                glm_vec3_add(world_origin, world_extent, origin_add_extent);
                
                glm_vec3_copy(vec3_min(model->aabb_min, origin_sub_extent), model->aabb_min);
                glm_vec3_copy(vec3_max(model->aabb_max, origin_add_extent), model->aabb_max);
			}
		} else {
            glm_vec3_copy(vec3_min(model->aabb_min, mesh->aabb_min), model->aabb_min);
			glm_vec3_copy(vec3_max(model->aabb_max, mesh->aabb_max), model->aabb_max);
		}
	}
    
	ufbx_free_scene(scene);
    
    gfx_handle_t result = {0};
    result.id = model_buffer.idx - 1;
    return result;
}