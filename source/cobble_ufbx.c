
static gfx_mesh_piece_t ufbx_convert_mesh_part(ufbx_mesh_part *part, ufbx_mesh *mesh) {
    gfx_mesh_piece_t result = {0};
    
    size_t num_triangles = part->num_triangles;
    result.vertices = (gfx_vertex_t*)c_alloc(num_triangles * 3 * sizeof(gfx_vertex_t));
    result.num_vertices = 0;
    
    // Reserve space for the maximum triangle indices.
    size_t num_tri_indices = mesh->max_face_triangles * 3;
    u32 *tri_indices = (u32*)c_alloc(num_tri_indices * sizeof(u32));
    
    // Iterate over each face using the specific material.
    for (size_t face_ix = 0; face_ix < part->num_faces; face_ix++) {
        ufbx_face face = mesh->faces.data[part->face_indices.data[face_ix]];
        
        // Triangulate the face into `tri_indices[]`.
        u32 num_tris = ufbx_triangulate_face(tri_indices, num_tri_indices, mesh, face);
        
        // Iterate over each triangle corner contiguously.
        for (size_t i = 0; i < num_tris * 3; i++) {
            u32 index = tri_indices[i];
            
            gfx_vertex_t *v = &result.vertices[result.num_vertices++];
            ufbx_vec3 pos = ufbx_get_vertex_vec3(&mesh->vertex_position, index);
            ufbx_vec3 norm = ufbx_get_vertex_vec3(&mesh->vertex_normal, index);
            ufbx_vec2 uv = ufbx_get_vertex_vec2(&mesh->vertex_uv, index);;
            
            v->position[0] = pos.x;
            v->position[1] = pos.y;
            v->position[2] = pos.z;
            v->normal[0] = norm.x;
            v->normal[1] = norm.y;
            v->normal[2] = norm.z;
            v->uv[0] = uv.x;
            v->uv[1] = uv.y;
        }
    }
    
    // Should have written all the vertices.
    c_free((u8*)tri_indices);
    c_assert(result.num_vertices == num_triangles * 3);
    
    // Generate the index buffer.
    ufbx_vertex_stream streams[1] = {
        { result.vertices, result.num_vertices, sizeof(gfx_vertex_t) },
    };
    result.num_indices = num_triangles * 3;
    result.indices = (u16*)c_alloc(result.num_indices * sizeof(u16));
    
    // This call will deduplicate vertices, modifying the arrays passed in `streams[]`,
    // indices are written in `indices[]` and the number of unique vertices is returned.
    result.num_vertices = ufbx_generate_indices(streams, 1, (u32*)result.indices, result.num_indices, NULL, NULL);
    return result;
}

static gfx_mesh_object_t ufbx_load(const dir_t *dir) {
    c_assert(dir->ptr != NULL);
    gfx_mesh_object_t result = {0};
    result.mesh_pieces = (gfx_mesh_piece_t*)c_alloc(sizeof(gfx_mesh_piece_t) * UFBX_MAX_PIECES_PER_MESH);
    result.mesh_pieces_count = 0;
    
    ufbx_load_opts opts = {0};
    opts.target_unit_meters = 1.0f;
    
    ufbx_error error;
    ufbx_scene *scene = ufbx_load_file(dir->ptr, &opts, &error);
    if (!scene) {
        c_assert_break(); // Failed to load scene
    }
    
    for (size_t i = 0; i < scene->meshes.count; i++) {
        ufbx_mesh *mesh = scene->meshes.data[i];
        
        for(size_t j = 0; j < mesh->material_parts.count; ++j) {
            ufbx_mesh_part *mesh_part = &mesh->material_parts.data[j];
            gfx_mesh_piece_t piece = ufbx_convert_mesh_part(mesh_part, mesh);
            memcpy(&result.mesh_pieces[result.mesh_pieces_count++], &piece, sizeof(gfx_mesh_piece_t));
        }
        for(size_t j = 0; j < mesh->face_group_parts.count; ++j) {
            ufbx_mesh_part *mesh_part = &mesh->face_group_parts.data[j];
            gfx_mesh_piece_t piece = ufbx_convert_mesh_part(mesh_part, mesh);
            memcpy(&result.mesh_pieces[result.mesh_pieces_count++], &piece, sizeof(gfx_mesh_piece_t));
        }
    }
    
    ufbx_free_scene(scene);
    return result;
}