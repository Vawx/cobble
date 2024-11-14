
#include "color_shadow_pass.h"

static cobble_gfx gfx;
static cobble_gfx *get_gfx() {return &gfx;}

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
    
    shape_type types[10] = {
        BOX,
        SPHERE,
        SPHERE,
        CYLINDER,
        TORUS,
        BOX,
        BOX,
        SPHERE,
        CYLINDER,
        TORUS,
    };
    
    for(s32 i = 0 ; i < 10; ++i) {
        shapes_push_desc(&(shape_description) {
                             .pos = pos[i],
                             .rot = rot[i],
                             .scale = V3(1.f, 1.f, 1.f),
                             .color = col[i],
                             .shape_type = types[i],
                             .desc_type = SHAPE_DESC_TYPE_COLORED,
                             .shape_movement = SHAPE_MOVEMENT_STATIC
                         });
    }
}


static void gfx_shapes_init() {
    
    // shader and pipeline object
    gfx.display.primitives.pip = sg_make_pipeline(&(sg_pipeline_desc){
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
    
    
    for(s32 i = 0; i < NUM_SHAPES; ++i) {
        gfx.display.primitives.shapes[i].shape_descriptions = (shape_description*)c_alloc(sizeof(shape_description) * kilo(1));
        gfx.display.primitives.shapes[i].type = (shape_type)i;
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
    gfx.display.primitives.shapes[BOX].draw = sshape_element_range(&buf);
    buf = sshape_build_plane(&buf, &(sshape_plane_t){
                                 .width = 1.0f,
                                 .depth = 1.0f,
                                 .tiles = 10,
                                 .random_colors = false,
                             });
    gfx.display.primitives.shapes[PLANE].draw = sshape_element_range(&buf);
    buf = sshape_build_sphere(&buf, &(sshape_sphere_t) {
                                  .radius = 0.75f,
                                  .slices = 36,
                                  .stacks = 20,
                                  .random_colors = false,
                              });
    gfx.display.primitives.shapes[SPHERE].draw = sshape_element_range(&buf);
    buf = sshape_build_cylinder(&buf, &(sshape_cylinder_t) {
                                    .radius = 0.5f,
                                    .height = 1.5f,
                                    .slices = 36,
                                    .stacks = 10,
                                    .random_colors = false,
                                });
    gfx.display.primitives.shapes[CYLINDER].draw = sshape_element_range(&buf);
    buf = sshape_build_torus(&buf, &(sshape_torus_t) {
                                 .radius = 0.5f,
                                 .ring_radius = 0.3f,
                                 .rings = 36,
                                 .sides = 18,
                                 .random_colors = false,
                             });
    gfx.display.primitives.shapes[TORUS].draw = sshape_element_range(&buf);
    c_assert(buf.valid);
    
    // one vertex/index-buffer-pair for all shapes
    const sg_buffer_desc vbuf_desc = sshape_vertex_buffer_desc(&buf);
    const sg_buffer_desc ibuf_desc = sshape_index_buffer_desc(&buf);
    gfx.display.primitives.vbuf = sg_make_buffer(&vbuf_desc);
    gfx.display.primitives.ibuf = sg_make_buffer(&ibuf_desc);
    
    shapes_test();
}

static void gfx_init() {
    // vertex buffer for a cube and plane
    const float scene_vertices[] = {
        // pos                  normals
        -1.0f, -1.0f, -1.0f,    0.0f, 0.0f, -1.0f,  //CUBE BACK FACE
        1.0f, -1.0f, -1.0f,    0.0f, 0.0f, -1.0f,
        1.0f,  1.0f, -1.0f,    0.0f, 0.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f, -1.0f,
        
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f, 1.0f,   //CUBE FRONT FACE
        1.0f, -1.0f,  1.0f,    0.0f, 0.0f, 1.0f,
        1.0f,  1.0f,  1.0f,    0.0f, 0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 0.0f, 1.0f,
        
        -1.0f, -1.0f, -1.0f,    -1.0f, 0.0f, 0.0f,  //CUBE LEFT FACE
        -1.0f,  1.0f, -1.0f,    -1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    -1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    -1.0f, 0.0f, 0.0f,
        
        1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f,   //CUBE RIGHT FACE
        1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f,
        1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.0f,
        1.0f, -1.0f,  1.0f,    1.0f, 0.0f, 0.0f,
        
        -1.0f, -1.0f, -1.0f,    0.0f, -1.0f, 0.0f,  //CUBE BOTTOM FACE
        -1.0f, -1.0f,  1.0f,    0.0f, -1.0f, 0.0f,
        1.0f, -1.0f,  1.0f,    0.0f, -1.0f, 0.0f,
        1.0f, -1.0f, -1.0f,    0.0f, -1.0f, 0.0f,
        
        -1.0f,  1.0f, -1.0f,    0.0f, 1.0f, 0.0f,   //CUBE TOP FACE
        -1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f,
        1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f,
        1.0f,  1.0f, -1.0f,    0.0f, 1.0f, 0.0f,
        
        -5.0f,  0.0f, -5.0f,    0.0f, 1.0f, 0.0f,   //PLANE GEOMETRY
        -5.0f,  0.0f,  5.0f,    0.0f, 1.0f, 0.0f,
        5.0f,  0.0f,  5.0f,    0.0f, 1.0f, 0.0f,
        5.0f,  0.0f, -5.0f,    0.0f, 1.0f, 0.0f,
    };
    gfx.vbuf = sg_make_buffer(&(sg_buffer_desc){
                                  .data = SG_RANGE(scene_vertices),
                                  .label = "cube-vertices"
                              });
    
    // ...and a matching index buffer for the scene
    const uint16_t scene_indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20,
        26, 25, 24,  27, 26, 24
    };
    gfx.ibuf = sg_make_buffer(&(sg_buffer_desc){
                                  .type = SG_BUFFERTYPE_INDEXBUFFER,
                                  .data = SG_RANGE(scene_indices),
                                  .label = "cube-indices"
                              });
    
    // shadow map pass action: only clear depth buffer, don't configure color and stencil actions,
    // because there are no color and stencil targets
    gfx.shadow.pass_action = (sg_pass_action){
        .depth = {
            .load_action = SG_LOADACTION_CLEAR,
            .store_action = SG_STOREACTION_STORE,
            .clear_value = 1.0f,
        },
    };
    
    // display pass action
    gfx.display.pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.25f, 0.25f, 0.25f, 1.0f}
        },
    };
    
    // a shadow map render target which will serve as depth buffer in the shadow pass
    gfx.shadow_map = sg_make_image(&(sg_image_desc){
                                       .render_target = true,
                                       .width = 2048,
                                       .height = 2048,
                                       .pixel_format = SG_PIXELFORMAT_DEPTH,
                                       .sample_count = 1,
                                       .label = "shadow-map",
                                   });
    
    // a comparison sampler which is used to sample the shadow map texture in the display pass
    gfx.shadow_sampler = sg_make_sampler(&(sg_sampler_desc){
                                             .min_filter = SG_FILTER_LINEAR,
                                             .mag_filter = SG_FILTER_LINEAR,
                                             .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                             .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                             .compare = SG_COMPAREFUNC_LESS,
                                             .label = "shadow-sampler",
                                         });
    
    // a depth-only pass object for the shadow pass
    gfx.shadow.atts = sg_make_attachments(&(sg_attachments_desc){
                                              .depth_stencil.image = gfx.shadow_map,
                                              .label = "shadow-pass",
                                          });
    
    // a pipeline object for the shadow pass
    gfx.shadow.pip = sg_make_pipeline(&(sg_pipeline_desc){
                                          .layout = {
                                              // need to provide vertex stride, because normal component is skipped in shadow pass
                                              .buffers[0].stride = 6 * sizeof(float),
                                              .attrs = {
                                                  [ATTR_shadow_pos].format = SG_VERTEXFORMAT_FLOAT3,
                                              },
                                          },
                                          .shader = sg_make_shader(shadow_shader_desc(sg_query_backend())),
                                          .index_type = SG_INDEXTYPE_UINT16,
                                          // render back-faces in shadow pass to prevent shadow acne on front-faces
                                          .cull_mode = SG_CULLMODE_FRONT,
                                          .sample_count = 1,
                                          .depth = {
                                              .pixel_format = SG_PIXELFORMAT_DEPTH,
                                              .compare = SG_COMPAREFUNC_LESS_EQUAL,
                                              .write_enabled = true,
                                          },
                                          // important: 'deactivate' the default color target for 'depth-only-rendering'
                                          .colors[0].pixel_format = SG_PIXELFORMAT_NONE,
                                          .label = "shadow-pipeline"
                                      });
    
    // resource bindings to render shadow scene
    gfx.shadow.bind = (sg_bindings) {
        .vertex_buffers[0] = gfx.vbuf,
        .index_buffer = gfx.ibuf,
    };
    
    // a pipeline object for the display pass
    gfx.display.pip = sg_make_pipeline(&(sg_pipeline_desc){
                                           .layout = {
                                               .attrs = {
                                                   [ATTR_display_pos].format = SG_VERTEXFORMAT_FLOAT3,
                                                   [ATTR_display_norm].format = SG_VERTEXFORMAT_FLOAT3,
                                               }
                                           },
                                           .shader = sg_make_shader(display_shader_desc(sg_query_backend())),
                                           .index_type = SG_INDEXTYPE_UINT16,
                                           .cull_mode = SG_CULLMODE_BACK,
                                           .depth = {
                                               .compare = SG_COMPAREFUNC_LESS_EQUAL,
                                               .write_enabled = true,
                                           },
                                           .label = "display-pipeline",
                                       });
    
    // resource bindings to render display scene
    gfx.display.bind = (sg_bindings) {
        .vertex_buffers[0] = gfx.vbuf,
        .index_buffer = gfx.ibuf,
        .images[IMG_shadow_map] = gfx.shadow_map,
        .samplers[SMP_shadow_sampler] = gfx.shadow_sampler,
    };
    
    // a vertex buffer, pipeline and sampler to render a debug visualization of the shadow map
    float dbg_vertices[] = { 0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f };
    sg_buffer dbg_vbuf = sg_make_buffer(&(sg_buffer_desc){
                                            .data = SG_RANGE(dbg_vertices),
                                            .label = "debug-vertices"
                                        });
    gfx.dbg.pip = sg_make_pipeline(&(sg_pipeline_desc){
                                       .layout = {
                                           .attrs[ATTR_dbg_pos].format = SG_VERTEXFORMAT_FLOAT2,
                                       },
                                       .shader = sg_make_shader(dbg_shader_desc(sg_query_backend())),
                                       .primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP,
                                       .label = "debug-pipeline",
                                   });
    // note: use a regular sampling-sampler to render the shadow map,
    // need to use nearest filtering here because of portability restrictions
    // (e.g. WebGL2)
    sg_sampler dbg_smp = sg_make_sampler(&(sg_sampler_desc){
                                             .min_filter = SG_FILTER_NEAREST,
                                             .mag_filter = SG_FILTER_NEAREST,
                                             .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                             .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                             .label = "debug-sampler"
                                         });
    gfx.dbg.bind = (sg_bindings){
        .vertex_buffers[0] = dbg_vbuf,
        .images[IMG_dbg_tex] = gfx.shadow_map,
        .samplers[SMP_dbg_smp] = dbg_smp,
    };
    
    gfx_shapes_init();
}

static void shapes_frame(mat4 *projection_view) {
    sg_apply_pipeline(gfx.display.primitives.pip);
    sg_apply_bindings(&(sg_bindings) {
                          .vertex_buffers[0] = gfx.display.primitives.vbuf,
                          .index_buffer = gfx.display.primitives.ibuf
                      });
    for (s32 i = 0; i < NUM_SHAPES; i++) {
        for(s32 j = 0; j < gfx.display.primitives.shapes[i].shape_descriptions_idx; ++j) {
            shape_description *desc = &gfx.display.primitives.shapes[i].shape_descriptions[j];
            mat4 model = {0};
            
            switch(desc->shape_movement) {
                case SHAPE_MOVEMENT_STATIC: {
                    glm_mat4_identity(model);
                    
                    vec3 rotation = {0};
                    glm_vec3_copy(rotation, vec3_from_v3(desc->rot));
                    
                    glm_rotated(model, rotation[0], (vec3){1.f, 0.f, 0.f});
                    glm_rotated(model, rotation[1], (vec3){0.f, 1.f, 0.f});
                    glm_rotated(model, rotation[2], (vec3){0.f, 0.f, 1.f});
                    glmc_translate(model, vec3_from_v3(desc->pos));
                    glmc_scale(model, vec3_from_v3(desc->scale));
                } break;
                case SHAPE_MOVEMENT_DYNAMIC: {
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
            
            glm_vec4_copy(vec4_from_v4(desc->color), gfx.display.primitives.vs_params.custom_color);
            glm_mat4_mul(*projection_view, model, gfx.display.primitives.vs_params.mvp);
            
            sg_apply_uniforms(UB_vs_params_shapes, &SG_RANGE(gfx.display.primitives.vs_params));
            sg_draw(gfx.display.primitives.shapes[i].draw.base_element, gfx.display.primitives.shapes[i].draw.num_elements, 1);
        }
    }
}

static void gfx_frame() {
    const float t = (float)(sapp_frame_duration() * 60.0);
    gfx.ry += 0.02f * t;
    
    static vec3 eye_pos = {5.0f, 5.0f, 5.0f};
    
    MAT4(plane_model);
    
    MAT4(cube_model);
    glm_translated(cube_model, (vec3){0.f, 1.5f, 0.f});
    
    vec3 plane_color = {1.f, 0.f, 0.f};
    vec3 cube_color = {0.f, 1.f, 0.f};
    
    // calculate matrices for shadow pass
    MAT4(rym);
    glm_rotated(rym, gfx.ry, (vec3){0.f, 1.f, 0.f});
    
    vec4 light_pos = {0};
    glm_mat4_mulv(rym, (vec4){50.f, 50.f, -50.f, 1.f}, light_pos);
    
    MAT4(light_view);
    glm_lookat((vec3){light_pos[0], light_pos[1], light_pos[2]}, (vec3){0.f, 1.f, 0.f}, (vec3){0.f, 1.f, 0.f}, light_view);
    
    MAT4(light_proj);
    glm_ortho(-5.f, 5.f, -5.f, 5.f, 0.01f, 100.f, light_proj); 
    
    MAT4(light_proj_view);
    glm_mat4_mul(light_proj, light_view, light_proj_view);
    
    vs_shadow_params_t cube_vs_shadow_params = {0};
    glm_mat4_identity(cube_vs_shadow_params.mvp);
    glm_mat4_mul(light_proj_view, cube_model, cube_vs_shadow_params.mvp);
    
    MAT4(proj);
    MAT4(view);
    MAT4(proj_view);
    
    fs_display_params_t fs_display_params = {0};
    {
        const r32 aspect = (r32)sapp_width() / (r32)sapp_height();
        glm_perspective(70.f, aspect, 0.01f, 100.f, proj);
        glm_lookat(eye_pos, (vec3){0.f, 0.f, 0.f}, (vec3){0.f, 1.f, 0.f}, view);
        glm_mat4_mul(proj, view, proj_view);
        
        glm_vec3_copy(light_pos, fs_display_params.light_dir);
        glm_vec3_normalize(fs_display_params.light_dir);
        glm_vec3_copy(eye_pos, fs_display_params.eye_pos);
    }
    
    vs_display_params_t plane_vs_display_params = {0};
    {
        glm_mat4_identity(plane_vs_display_params.mvp);
        glm_mat4_identity(plane_vs_display_params.model);
        glm_mat4_identity(plane_vs_display_params.light_mvp);
        
        glm_mat4_mul(proj_view, plane_model, plane_vs_display_params.mvp);
        glm_mat4_copy(plane_model, plane_vs_display_params.model);
        glm_mat4_mul(light_proj_view, plane_model, plane_vs_display_params.light_mvp);
        glm_vec4_copy(plane_color, plane_vs_display_params.diff_color);
    }
    
    vs_display_params_t cube_vs_display_params = {0};
    {
        glm_mat4_identity(cube_vs_display_params.mvp);
        glm_mat4_identity(cube_vs_display_params.model);
        glm_mat4_identity(cube_vs_display_params.light_mvp);
        
        glm_mat4_mul(proj_view, cube_model, cube_vs_display_params.mvp);
        glm_mat4_copy(cube_model, cube_vs_display_params.model);
        glm_mat4_mul(light_proj_view, cube_model, cube_vs_display_params.light_mvp);
        glm_vec4_copy(cube_color, cube_vs_display_params.diff_color);
    }
    
    // the shadow map pass, render scene from light source into shadow map texture
    sg_begin_pass(&(sg_pass){ .action = gfx.shadow.pass_action, .attachments = gfx.shadow.atts });
    sg_apply_pipeline(gfx.shadow.pip);
    sg_apply_bindings(&gfx.shadow.bind);
    sg_apply_uniforms(UB_vs_shadow_params, &SG_RANGE(cube_vs_shadow_params));
    sg_draw(0, 36, 1);
    sg_end_pass();
    
    // the display pass, render scene from camera, compare-sample shadow map texture
    sg_begin_pass(&(sg_pass){ .action = gfx.display.pass_action, .swapchain = sglue_swapchain() });
    shapes_frame(&proj_view);
    
    sg_apply_pipeline(gfx.display.pip);
    sg_apply_bindings(&gfx.display.bind);
    sg_apply_uniforms(UB_fs_display_params, &SG_RANGE(fs_display_params));
    // render plane
    sg_apply_uniforms(UB_vs_display_params , &SG_RANGE(plane_vs_display_params));
    sg_draw(36, 6, 1);
    // render cube
    sg_apply_uniforms(UB_vs_display_params, &SG_RANGE(cube_vs_display_params));
    sg_draw(0, 36, 1);
    // render debug visualization of shadow-map
    sg_apply_pipeline(gfx.dbg.pip);
    sg_apply_bindings(&gfx.dbg.bind);
    sg_apply_viewport(sapp_width() - 150, 0, 150, 150, false);
    sg_draw(0, 4, 1);
    
    imgui_frame();
    
    sg_end_pass();
    sg_commit();
    
    eye_pos[2] -= 0.02f;
}

static void shapes_close() {
    for (s32 i = 0; i < NUM_SHAPES; i++) {
        for(s32 j = 0; j < gfx.display.primitives.shapes[i].shape_descriptions_idx; ++j) {
            c_free((u8*)gfx.display.primitives.shapes[i].shape_descriptions);
        }
    }
}


static void gfx_end() {
    shapes_close();
}


static void shapes_push_desc(shape_description *desc) {
    c_assert(desc->shape_type >= 0 && desc->shape_type < NUM_SHAPES);
    
    render_shape *ptr = &gfx.display.primitives.shapes[desc->shape_type];
    c_assert(ptr != NULL);
    
    shape_description *shape_to = &ptr->shape_descriptions[ptr->shape_descriptions_idx];
    shape_to->pos = desc->pos;
    shape_to->rot = desc->rot;
    shape_to->scale = desc->scale;
    shape_to->desc_type = desc->desc_type;
    shape_to->shape_movement = desc->shape_movement;
    shape_to->shape_type = desc->shape_type;
    switch(desc->desc_type) {
        case SHAPE_DESC_TYPE_COLORED: {
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
        case SHAPE_DESC_TYPE_BINDED: {
            memcpy(&shape_to->bindings, &desc->bindings, sizeof(sg_bindings));
        } break;
        default:
        c_assert_break(); // invalid
    }
    
    ++ptr->shape_descriptions_idx;
}

#include "cobble_view.c"
#include "cobble_render_imgui.c"
