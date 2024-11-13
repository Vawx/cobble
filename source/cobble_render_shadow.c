
static cobble_shadow shadow;

static cobble_shadow *get_shadow_pass() {
    return &shadow;
}

static void shadow_init() {
    // shadow map pass action: only clear depth buffer, don't configure color and stencil actions,
    // because there are no color and stencil targets
    shadow.pass_action = (sg_pass_action){
        .depth = {
            .load_action = SG_LOADACTION_CLEAR,
            .store_action = SG_STOREACTION_STORE,
            .clear_value = 1.0f,
        },
    };
    
    // a pipeline object for the shadow pass
    shadow.pip = sg_make_pipeline(&(sg_pipeline_desc){
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
    
    // a shadow map render target which will serve as depth buffer in the shadow pass
    shadow.shadow_map = sg_make_image(&(sg_image_desc){
                                          .render_target = true,
                                          .width = 2048,
                                          .height = 2048,
                                          .pixel_format = SG_PIXELFORMAT_DEPTH,
                                          .sample_count = 1,
                                          .label = "shadow-map",
                                      });
    
    // a comparison sampler which is used to sample the shadow map texture in the display pass
    shadow.shadow_sampler = sg_make_sampler(&(sg_sampler_desc){
                                                .min_filter = SG_FILTER_LINEAR,
                                                .mag_filter = SG_FILTER_LINEAR,
                                                .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                                .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                                .compare = SG_COMPAREFUNC_LESS,
                                                .label = "shadow-sampler",
                                            });
    
    // resource bindings to render display scene
    shadow.bind = (sg_bindings) {
        .images[IMG_shadow_map] = shadow.shadow_map,
        .samplers[SMP_shadow_sampler] = shadow.shadow_sampler,
    };
    
    // a depth-only pass object for the shadow pass
    shadow.atts = sg_make_attachments(&(sg_attachments_desc){
                                          .depth_stencil.image = shadow.shadow_map,
                                          .label = "shadow-pass",
                                      });
}

static void shadow_frame() {
    mat4 rym = {0};
    glm_rotated(rym, get_color_pass()->ry, (vec3){0.f, 1.f, 0.f});
    vec4 light_pos = {0};
    glm_mat4_mulv(rym, (vec4){50.f, 50.f, -50.f, 1.f}, light_pos);
    mat4 light_view = {0};
    glm_lookat((vec3){light_pos[0], light_pos[1], light_pos[2]}, (vec3){0.f, 1.f, 0.f}, (vec3){0.f, 1.f, 0.f}, light_view);
    mat4 light_proj = {0};
    glm_ortho(-5.f, 5.f, -5.f, 5.f, 0.f, 100.f, light_proj);
    mat4 light_view_proj = {0};
    glm_mat4_mul(light_proj, light_view, light_view_proj);
    
    vs_shadow_params_t shadow_params = {0};
    mat4 model = {0};
    mat4 mvp = {0};
    glm_mat4_mul(light_view_proj, model, shadow_params.mvp);
    
    sg_begin_pass(&(sg_pass){ .action = shadow.pass_action, .attachments = shadow.atts });
    sg_apply_pipeline(shadow.pip);
    sg_apply_bindings(&shadow.bind);
    
    vs_shadow_params_t vs_shadow_params = {
        0
    };
    sg_apply_uniforms(UB_vs_shadow_params, &SG_RANGE(vs_shadow_params));
    // render objects
    
    sg_end_pass();
    
}

static void shadow_end() {
    
}
