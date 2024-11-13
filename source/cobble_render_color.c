static color_state color;

static color_state *get_color_pass() {
    return &color;
}

static void render_bind_image(sg_bindings *bind, bind_image_idx_type idx) {
    c_assert(idx < BIND_IMAGE_IDX_COUNT);
    bind->images[idx] = sg_alloc_image();
}

static void render_bind_sampler(sg_bindings *bind, bind_image_idx_type idx) {
    c_assert(idx < BIND_IMAGE_IDX_COUNT);
    bind->samplers[idx] = sg_make_sampler(&(sg_sampler_desc) {
                                              .min_filter = SG_FILTER_LINEAR,
                                              .mag_filter = SG_FILTER_LINEAR,
                                          });
}

static void render_bind_vertex(sg_bindings *bind, const char *label, sg_range *vertices_range, u32 idx) {
    c_assert(idx < 8);
    bind->vertex_buffers[idx] = sg_make_buffer(&(sg_buffer_desc){
                                                   .data = (sg_range){vertices_range->ptr, vertices_range->size},
                                                   .label = label
                                               });
}

static void render_bind_index(sg_bindings *bind, const char *label, sg_range *indices_range, u32 idx) {
    bind->index_buffer = sg_make_buffer(&(sg_buffer_desc){
                                            .type = SG_BUFFERTYPE_INDEXBUFFER,
                                            .data = (sg_range){indices_range->ptr, indices_range->size},
                                            .label = label
                                        });
}

static void render_bind_texture(sg_bindings *bind, bind_image_idx_type idx, const cobble_dir *dir) {
    int png_width, png_height, num_channels;
    const int desired_channels = 4;
    stbi_uc* pixels = stbi_load(dir->ptr, &png_width, &png_height, &num_channels, desired_channels);
    if (pixels) {
        sg_init_image(bind->images[idx], &(sg_image_desc){
                          .width = png_width,
                          .height = png_height,
                          .pixel_format = SG_PIXELFORMAT_RGBA8,
                          .data.subimage[0][0] = {
                              .ptr = pixels,
                              .size = (size_t)(png_width * png_height * 4),
                          }
                      });
        stbi_image_free(pixels);
    }
    
}

static void color_init() {
    
    // display pass action
    color.pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.25f, 0.25f, 0.5f, 1.0f}
        },
    };
    
    // a shader (use separate shader sources here
    color.shader = sg_make_shader(display_shader_desc(sg_query_backend()));
    
    // a pipeline object for the display pass
    color.pip = sg_make_pipeline(&(sg_pipeline_desc){
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
    
}

static void color_frame(mat4 *projection_view) {
    sg_apply_pipeline(color.pip);
    
    vec3 light_dir = {0};
    light_dir[0] = 50.f;
    light_dir[1] = 50.f;
    light_dir[2] = -50.f;
    
    fs_display_params_t fs_display_params = {0};
    glm_vec3_normalize_to(light_dir, fs_display_params.light_dir);
    glm_vec3_copy(get_current_view()->pos, fs_display_params.eye_pos);
    
    sg_apply_uniforms(UB_fs_display_params, &SG_RANGE(fs_display_params));
    shapes_render_pass(projection_view);
} 

static void color_end() {
    
}
