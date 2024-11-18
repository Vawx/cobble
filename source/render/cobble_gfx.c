/**
NOTES:

This is cobbled together using examples from sokol git: https://floooh.github.io/sokol-html5/index.html

As for the order of initialization: I have no clue if the order matters, though it _feels_ like it does.
*/
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
    state.vbuf = sg_make_buffer(&(sg_buffer_desc){
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
    state.ibuf = sg_make_buffer(&(sg_buffer_desc){
                                    .type = SG_BUFFERTYPE_INDEXBUFFER,
                                    .data = SG_RANGE(scene_indices),
                                    .label = "cube-indices"
                                });
    
    // shadow map pass action: clear the shadow map to (1,1,1,1)
    state.shadow.pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 1.0f, 1.0f, 1.0f, 1.0f },
        }
    };
    
    // display pass action
    state.display.pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.25f, 0.5f, 0.25f, 1.0f}
        },
    };
    
    // a regular RGBA8 render target image as shadow map
    state.shadow_map = sg_make_image(&(sg_image_desc){
                                         .render_target = true,
                                         .width = 2048,
                                         .height = 2048,
                                         .pixel_format = SG_PIXELFORMAT_RGBA8,
                                         .sample_count = 1,
                                         .label = "shadow-map",
                                     });
    
    // ...we also need a separate depth-buffer image for the shadow pass
    sg_image shadow_depth_img = sg_make_image(&(sg_image_desc){
                                                  .render_target = true,
                                                  .width = 2048,
                                                  .height = 2048,
                                                  .pixel_format = SG_PIXELFORMAT_DEPTH,
                                                  .sample_count = 1,
                                                  .label = "shadow-depth-buffer",
                                              });
    
    // a regular sampler with nearest filtering to sample the shadow map
    state.shadow_sampler = sg_make_sampler(&(sg_sampler_desc){
                                               .min_filter = SG_FILTER_NEAREST,
                                               .mag_filter = SG_FILTER_NEAREST,
                                               .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                               .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                               .label = "shadow-sampler",
                                           });
    
    // the render pass object for the shadow pass
    state.shadow.atts = sg_make_attachments(&(sg_attachments_desc){
                                                .colors[0].image = state.shadow_map,
                                                .depth_stencil.image = shadow_depth_img,
                                                .label = "shadow-pass",
                                            });
    
    // a pipeline object for the shadow pass
    state.shadow.pip = sg_make_pipeline(&(sg_pipeline_desc){
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
                                            .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
                                            .depth = {
                                                .pixel_format = SG_PIXELFORMAT_DEPTH,
                                                .compare = SG_COMPAREFUNC_LESS_EQUAL,
                                                .write_enabled = true,
                                            },
                                            .label = "shadow-pipeline"
                                        });
    
    // resource bindings to render shadow scene
    state.shadow.bind = (sg_bindings) {
        .vertex_buffers[0] = state.vbuf,
        .index_buffer = state.ibuf,
    };
    
    // a pipeline object for the display pass
    state.display.pip = sg_make_pipeline(&(sg_pipeline_desc){
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
    state.display.bind = (sg_bindings) {
        .vertex_buffers[0] = state.vbuf,
        .index_buffer = state.ibuf,
        .images[IMG_shadow_map] = state.shadow_map,
        .samplers[SMP_shadow_sampler] = state.shadow_sampler,
    };
    
    // a vertex buffer, pipeline and sampler to render a debug visualization of the shadow map
    float dbg_vertices[] = { 0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f };
    sg_buffer dbg_vbuf = sg_make_buffer(&(sg_buffer_desc){
                                            .data = SG_RANGE(dbg_vertices),
                                            .label = "debug-vertices"
                                        });
    state.dbg.pip = sg_make_pipeline(&(sg_pipeline_desc){
                                         .layout = {
                                             .attrs[ATTR_dbg_pos].format = SG_VERTEXFORMAT_FLOAT2,
                                         },
                                         .shader = sg_make_shader(dbg_shader_desc(sg_query_backend())),
                                         .primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP,
                                         .label = "debug-pipeline",
                                     });
    sg_sampler dbg_smp = sg_make_sampler(&(sg_sampler_desc){
                                             .min_filter = SG_FILTER_NEAREST,
                                             .mag_filter = SG_FILTER_NEAREST,
                                             .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                             .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                             .label = "debug-sampler"
                                         });
    state.dbg.bind = (sg_bindings){
        .vertex_buffers[0] = dbg_vbuf,
        .images[IMG_dbg_tex] = state.shadow_map,
        .samplers[SMP_dbg_smp] = dbg_smp,
    };
    
    view_make_new((vec3){0.f, 5.f, 0.f}, (vec3){0.f, 0.f, -1.f}, 0.f, 0.f, 1);
    view_set_current_idx(0);
}

static void gfx_frame() {
    
    static MAT4(view_projection);
    view_frame((vec2){sapp_widthf(), sapp_heightf()}, &view_projection);
    
    const float t = (float)(sapp_frame_duration() * 10.0);
    state.ry += 0.2f * t;
    
    //vec3 eye_pos = {5.0f, 5.0f, 5.0f};
    vec3 plane_color = {1.0f, 1.0f, 0.0f};
    vec3 cube_color = {1.0f, 0.0f, 1.0f};
    MAT4(plane_model);
    MAT4(cube_model);
    glm_translated(cube_model, (vec3){0.f, 1.5f, 0.f});
    
    // calculate matrices for shadow pass
    MAT4(rym);
    MAT4(light_view);
    MAT4(light_proj);
    MAT4(light_view_proj);
    
    glm_rotated(rym, state.ry, (vec3){0.f, 1.f, 0.f});
    vec4 light_pos = {0};
    glm_mat4_mulv(rym, (vec4){50.f, 50.f, -50.f, 1.f}, light_pos); // light pos
    
    glm_lookat((vec3){light_pos[0], light_pos[1], light_pos[2]}, (vec3){0.f, 1.5f, 0.f}, (vec3){0.f, 1.f, 0.f}, light_view); // light view
    glm_ortho(-5.f, 5.f, -5.f, 5.f, 0.f, 100.f, light_proj); // light proj
    glm_mat4_mul(light_proj, light_view, light_view_proj); // light view proj
    
    vs_shadow_params_t cube_vs_shadow_params = {0};
    glm_mat4_mul(light_view_proj, cube_model, cube_vs_shadow_params.mvp); // shadow mvp
    
    // calculate matrices for display pass
    MAT4(proj);
    MAT4(view);
    MAT4(view_proj);
    
    //glm_perspective(70.f, sapp_widthf() / sapp_heightf(), 0.01f, 500.f, proj); // proj
    //glm_lookat(eye_pos, (vec3){0.f, 0.f, 0.f}, (vec3){0.f, 1.f, 0.f}, view); // view
    //glm_mat4_mul(proj, view, view_proj); // view proj
    
    cobble_view *current_view = get_current_view();
    c_assert(current_view != NULL);
    glm_mat4_copy(current_view->projection, proj);
    glm_mat4_copy(current_view->view, view);
    glm_mat4_mul(proj, view, view_proj);
    
    fs_display_params_t fs_display_params = {0};
    glm_vec3_copy((vec3){light_pos[0], light_pos[1], light_pos[2]}, fs_display_params.light_dir); // light dir
    glm_vec3_normalize(fs_display_params.light_dir);
    glm_vec3_copy(current_view->pos, fs_display_params.eye_pos); // eye pos
    
    vs_display_params_t plane_vs_display_params = {0};
    glm_mat4_mul(view_proj, plane_model, plane_vs_display_params.mvp); // plane mvp
    glm_mat4_copy(plane_model, plane_vs_display_params.model); // plane model
    glm_mat4_mul(light_view_proj, plane_model, plane_vs_display_params.light_mvp); // plane light mvp
    glm_vec3_copy(plane_color, plane_vs_display_params.diff_color); // plane color
    
    vs_display_params_t cube_vs_display_params = {0};
    glm_mat4_mul(view_proj, cube_model, cube_vs_display_params.mvp); // cube mvp
    glm_mat4_copy(cube_model, cube_vs_display_params.model); // cube model
    glm_mat4_mul(light_view_proj, cube_model, cube_vs_display_params.light_mvp); // cube light mvp
    glm_vec3_copy(cube_color, cube_vs_display_params.diff_color); // cube color
    
    // the shadow map pass, render scene from light source into shadow map texture
    sg_begin_pass(&(sg_pass){ .action = state.shadow.pass_action, .attachments = state.shadow.atts });
    sg_apply_pipeline(state.shadow.pip);
    sg_apply_bindings(&state.shadow.bind);
    sg_apply_uniforms(UB_vs_shadow_params, &SG_RANGE(cube_vs_shadow_params));
    sg_draw(0, 36, 1);
    sg_end_pass();
    
    // the display pass, render scene from camera and sample the shadow map
    sg_begin_pass(&(sg_pass){ .action = state.display.pass_action, .swapchain = sglue_swapchain() });
    sg_apply_pipeline(state.display.pip);
    sg_apply_bindings(&state.display.bind);
    sg_apply_uniforms(UB_fs_display_params, &SG_RANGE(fs_display_params));
    // render plane
    sg_apply_uniforms(UB_vs_display_params, &SG_RANGE(plane_vs_display_params));
    sg_draw(36, 6, 1);
    // render cube
    sg_apply_uniforms(UB_vs_display_params, &SG_RANGE(cube_vs_display_params));
    sg_draw(0, 36, 1);
    // render debug visualization of shadow-map
    sg_apply_pipeline(state.dbg.pip);
    sg_apply_bindings(&state.dbg.bind);
    sg_apply_viewport(sapp_width() - 150, 0, 150, 150, false);
    sg_draw(0, 4, 1);
    
    sg_end_pass();
    sg_commit();
}

static void gfx_end() {
    
}

#include "cobble_view.c"
#include "cobble_render_imgui.c"
