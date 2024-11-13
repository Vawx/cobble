
static render_shapes_state shape_state;

static render_shapes_state *get_shapes_pass() {
    return &shape_state;
}

static void shapes_init() {
    
    // shader and pipeline object
    shape_state.pip = sg_make_pipeline(&(sg_pipeline_desc){
                                           .shader = sg_make_shader(shapes_shader_desc(sg_query_backend())),
                                           .layout = {
                                               .buffers[0] = sshape_vertex_buffer_layout_state(),
                                               .attrs = {
                                                   [0] = sshape_position_vertex_attr_state(),
                                                   [1] = sshape_normal_vertex_attr_state(),
                                                   [2] = sshape_texcoord_vertex_attr_state(),
                                                   [3] = sshape_color_vertex_attr_state()
                                               }
                                           },
                                           .index_type = SG_INDEXTYPE_UINT16,
                                           .cull_mode = SG_CULLMODE_NONE,
                                           .depth = {
                                               .compare = SG_COMPAREFUNC_LESS_EQUAL,
                                               .write_enabled = true
                                           },
                                       });
    
    shape_state.shapes = (render_shapes*)c_alloc(sizeof(render_shapes) * RENDER_SHAPES_COUNT);
    for(s32 i = 0; i < RENDER_SHAPES_COUNT; ++i) {
        shape_state.shapes[i].shape_descriptions = (render_shape_description*)c_alloc(sizeof(render_shape_description) * kilo(1));
    }
    
    // generate shape geometries
    sshape_vertex_t vertices[6 * 1024];
    uint16_t indices[16 * 1024];
    sshape_buffer_t buf = {
        .vertices.buffer = SSHAPE_RANGE(vertices),
        .indices.buffer  = SSHAPE_RANGE(indices),
    };
    buf = sshape_build_box(&buf, &(sshape_box_t){
                               .width  = 1.0f,
                               .height = 1.0f,
                               .depth  = 1.0f,
                               .tiles  = 10,
                               .random_colors = false,
                           });
    shape_state.shapes[RENDER_SHAPES_BOX].draw = sshape_element_range(&buf);
    buf = sshape_build_plane(&buf, &(sshape_plane_t){
                                 .width = 1.0f,
                                 .depth = 1.0f,
                                 .tiles = 10,
                                 .random_colors = false,
                             });
    shape_state.shapes[RENDER_SHAPES_PLANE].draw = sshape_element_range(&buf);
    buf = sshape_build_sphere(&buf, &(sshape_sphere_t) {
                                  .radius = 1.f,
                                  .slices = 36,
                                  .stacks = 20,
                                  .random_colors = false,
                              });
    shape_state.shapes[RENDER_SHAPES_SPHERE].draw = sshape_element_range(&buf);
    buf = sshape_build_cylinder(&buf, &(sshape_cylinder_t) {
                                    .radius = 0.5f,
                                    .height = 1.5f,
                                    .slices = 36,
                                    .stacks = 10,
                                    .random_colors = false,
                                });
    shape_state.shapes[RENDER_SHAPES_CYLINDER].draw = sshape_element_range(&buf);
    buf = sshape_build_torus(&buf, &(sshape_torus_t) {
                                 .radius = 0.5f,
                                 .ring_radius = 0.3f,
                                 .rings = 36,
                                 .sides = 18,
                                 .random_colors = false,
                             });
    shape_state.shapes[RENDER_SHAPES_TORUS].draw = sshape_element_range(&buf);
    c_assert(buf.valid);
    
    // one vertex/index-buffer-pair for all shapes
    const sg_buffer_desc vbuf_desc = sshape_vertex_buffer_desc(&buf);
    const sg_buffer_desc ibuf_desc = sshape_index_buffer_desc(&buf);
    shape_state.vbuf = sg_make_buffer(&vbuf_desc);
    shape_state.ibuf = sg_make_buffer(&ibuf_desc);
}

static void shapes_render_object_pass(mat4 *model, mat4 *projection_view, void *ptr) {
    c_assert(ptr != NULL);
    
    render_shapes *shapes = (render_shapes*)ptr;
    render_shape_description *desc = shapes->shape_descriptions;
    
    glm_vec4_copy(vec4_from_v4(desc->color), shape_state.params_shapes.custom_color);
    glm_mat4_mul(*projection_view, *model, shape_state.params_shapes.mvp);
    
    sg_apply_uniforms(UB_vs_display_params, &SG_RANGE(shape_state.params_shapes));
    sg_draw(shapes->draw.base_element, shapes->draw.num_elements, 1);
}

static void shapes_render_pass(mat4 *projection_view) {
    sg_apply_pipeline(shape_state.pip);
    sg_apply_bindings(&(sg_bindings) {
                          .vertex_buffers[0] = shape_state.vbuf,
                          .index_buffer = shape_state.ibuf
                      });
    
    for (s32 i = 0; i < RENDER_SHAPES_COUNT; i++) {
        for(s32 j = 0; j < shape_state.shapes[i].shape_descriptions_idx; ++j) {
            render_shape_description *desc = &shape_state.shapes[i].shape_descriptions[j];
            mat4 model = {0};
            
            switch(desc->shape_movement) {
                case RENDER_SHAPES_MOVEMENT_STATIC: {
                    glm_mat4_identity(model);
                    
                    vec3 rotation = {0};
                    glm_vec3_copy(rotation, vec3_from_v3(desc->rot));
                    
                    glm_rotated(model, rotation[0], (vec3){1.f, 0.f, 0.f});
                    glm_rotated(model, rotation[1], (vec3){0.f, 1.f, 0.f});
                    glm_rotated(model, rotation[2], (vec3){0.f, 0.f, 1.f});
                    glmc_translate(model, vec3_from_v3(desc->pos));
                    glmc_scale(model, vec3_from_v3(desc->scale));
                } break;
                case RENDER_SHAPES_MOVEMENT_DYNAMIC: {
                    c_assert(desc->id > 0);
                    m4 d_model = jolt_get_dynamic_object_model_matrix(&desc->id);
                    model[0][0] = d_model.elements[0][0];
                    model[0][1] = d_model.elements[0][1];
                    model[0][2] = d_model.elements[0][2];
                    model[0][3] = d_model.elements[0][3];
                    
                    model[1][0] = d_model.elements[1][0];
                    model[1][1] = d_model.elements[1][1];
                    model[1][2] = d_model.elements[1][2];
                    model[1][3] = d_model.elements[1][3];
                    
                    model[2][0] = d_model.elements[2][0];
                    model[2][1] = d_model.elements[2][1];
                    model[2][2] = d_model.elements[2][2];
                    model[2][3] = d_model.elements[2][3];
                    
                    model[3][0] = d_model.elements[3][0];
                    model[3][1] = d_model.elements[3][1];
                    model[3][2] = d_model.elements[3][2];
                    model[3][3] = d_model.elements[3][3];
                } break;
            }
            shapes_render_object_pass(&model, projection_view, &shape_state.shapes[i]);
        }
    }
}

static void shapes_push_desc(render_shape_description *desc) {
    c_assert(desc->shape_type >= 0 && desc->shape_type < RENDER_SHAPES_COUNT);
    
    render_shapes *ptr = &shape_state.shapes[desc->shape_type];
    c_assert(ptr != NULL);
    
    render_shape_description *shape_to = &ptr->shape_descriptions[ptr->shape_descriptions_idx];
    shape_to->pos = desc->pos;
    shape_to->rot = desc->rot;
    shape_to->scale = desc->scale;
    shape_to->desc_type = desc->desc_type;
    shape_to->shape_movement = desc->shape_movement;
    switch(desc->desc_type) {
        case RENDER_SHAPE_DESC_TYPE_COLORED: {
            if(desc->color._v3.x > 1.f || desc->color._v3.y > 1.f || desc->color._v3.z > 1.f) {
                shape_to->color.x = clamp(desc->color.x / 255.f, 0, 255);
                shape_to->color.y = clamp(desc->color.y / 255.f, 0, 255);
                shape_to->color.z = clamp(desc->color.z / 255.f, 0, 255);
                shape_to->color.w = desc->color.w;
            } else {
                shape_to->color.x = desc->color.x;
                shape_to->color.y = desc->color.y;
                shape_to->color.z = desc->color.z;
                shape_to->color.w = desc->color.w;
            }
        } break;
        case RENDER_SHAPE_DESC_TYPE_BINDED: {
            memcpy(&shape_to->bindings, &desc->bindings, sizeof(sg_bindings));
        } break;
        default:
        c_assert_break(); // invalid
    }
    
    ++ptr->shape_descriptions_idx;
}

static void shapes_close() {
    for (s32 i = 0; i < RENDER_SHAPES_COUNT; i++) {
        for(s32 j = 0; j < shape_state.shapes[i].shape_descriptions_idx; ++j) {
            c_free((u8*)shape_state.shapes[i].shape_descriptions);
        }
    }
    c_free((u8*)shape_state.shapes);
}

static void shapes_test() {
    v3 pos[10] = {
        V3(30.f, 20.f, 20.f),
        V3(-4.f, 0.f, 30.f),
        V3(1.f, 0.f, 10.f),
        V3(9.f, 0.f, 1.f),
        V3(11.f, -10.f, 2.f),
        V3(-3.f, -20.f, 4.f),
        V3(-10.f, -12.f, -10.f),
        V3(4.f, 4.f, -20.f),
        V3(0.f, 14.f, -11.f),
        V3(7.f, 1.f, 2.f),
    };
    
    v3 rot[10] = {
        V3(30.f, 20.f, 20.f),
        V3(-4, 0.f, 30.f),
        V3(1.f, 0.f, 10.f),
        V3(9.f, 0.f, 1.f),
        V3(11.f, -10.f, 2.f),
        V3(-3.f, -20.f, 4.f),
        V3(-10.f, -12.f, -10.f),
        V3(4.f, 4.f, -20.f),
        V3(0.f, 14.f, -11.f),
        V3(7.f, 1.f, 2.f)
    };
    
    v3 col[10] = {
        V3(87, 192, 53),
        V3(13, 220, 177),
        V3(212, 89, 44),
        V3(101, 43, 255),
        V3(156, 34, 211),
        V3(72, 188, 128),
        V3(240, 17, 96),
        V3(59, 255, 59),
        V3(0, 103, 189),
        V3(145, 251, 28),
    };
    
    render_shapes_type types[10] = {
        RENDER_SHAPES_BOX,
        RENDER_SHAPES_SPHERE,
        RENDER_SHAPES_SPHERE,
        RENDER_SHAPES_CYLINDER,
        RENDER_SHAPES_TORUS,
        RENDER_SHAPES_BOX,
        RENDER_SHAPES_BOX,
        RENDER_SHAPES_SPHERE,
        RENDER_SHAPES_CYLINDER,
        RENDER_SHAPES_TORUS,
    };
    
    for(s32 i = 0 ; i < 10; ++i) {
        shapes_push_desc(&(render_shape_description) {
                             .pos = pos[i],
                             .rot = rot[i],
                             .scale = V3(1.f, 1.f, 1.f),
                             .color = col[i],
                             .shape_type = types[i],
                             .desc_type = RENDER_SHAPE_DESC_TYPE_COLORED,
                             .shape_movement = RENDER_SHAPES_MOVEMENT_STATIC
                         });
    }
}
