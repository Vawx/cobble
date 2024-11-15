
#include "color_pass.h"
#include "shadow_pass.h"

static gfx_state gfx;
static gfx_state *get_gfx() {return &gfx;}

static gfx_vertex vertices[] = {
    -1.0f, -1.0f, -1.0f,    0.0f, 0.0f, -1.0f,  0, 0,
    1.0f, -1.0f, -1.0f,    0.0f, 0.0f, -1.0f, 1, 0,
    1.0f,  1.0f, -1.0f,    0.0f, 0.0f, -1.0f, 1, 1,
    -1.0f,  1.0f, -1.0f,    0.0f, 0.0f, -1.0f, 0, 1,
    
    -1.0f, -1.0f,  1.0f,    0.0f, 0.0f, 1.0f,   0, 0,
    1.0f, -1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1, 0,
    1.0f,  1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1, 1,
    -1.0f,  1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 0, 1,
    
    -1.0f, -1.0f, -1.0f,    -1.0f, 0.0f, 0.0f,  0, 0,
    -1.0f,  1.0f, -1.0f,    -1.0f, 0.0f, 0.0f,1, 0, 
    -1.0f,  1.0f,  1.0f,    -1.0f, 0.0f, 0.0f,1, 1, 
    -1.0f, -1.0f,  1.0f,    -1.0f, 0.0f, 0.0f, 0, 1,
    
    1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 0, 0,  
    1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1, 0, 
    1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.0f, 1, 1, 
    1.0f, -1.0f,  1.0f,    1.0f, 0.0f, 0.0f, 0, 1,
    
    -1.0f, -1.0f, -1.0f,    0.0f, -1.0f, 0.0f, 0, 0, 
    -1.0f, -1.0f,  1.0f,    0.0f, -1.0f, 0.0f, 1, 0, 
    1.0f, -1.0f,  1.0f,    0.0f, -1.0f, 0.0f, 1, 1,
    1.0f, -1.0f, -1.0f,    0.0f, -1.0f, 0.0f, 0, 1, 
    
    -1.0f,  1.0f, -1.0f,    0.0f, 1.0f, 0.0f, 0, 0,
    -1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1, 0,
    1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1, 1,
    1.0f,  1.0f, -1.0f,    0.0f, 1.0f, 0.0f, 0, 1,
    
    -5.0f,  0.0f, -5.0f,    0.0f, 1.0f, 0.0f, 0, 0,
    -5.0f,  0.0f,  5.0f,    0.0f, 1.0f, 0.0f, 1, 0,
    5.0f,  0.0f,  5.0f,    0.0f, 1.0f, 0.0f, 1, 1, 
    5.0f,  0.0f, -5.0f,    0.0f, 1.0f, 0.0f, 0, 1,
};

static u16 indices[] = {
    0, 1, 2,  0, 2, 3,
    6, 5, 4,  7, 6, 4,
    8, 9, 10,  8, 10, 11,
    14, 13, 12,  15, 14, 12,
    16, 17, 18,  16, 18, 19,
    22, 21, 20,  23, 22, 20,
    26, 25, 24,  27, 26, 24
};

static sg_sampler gfx_make_sampler() {
    sg_sampler_desc desc = {0};
    desc.min_filter = SG_FILTER_LINEAR;
    desc.mag_filter = SG_FILTER_LINEAR;
    return sg_make_sampler(&desc);
}

static sg_buffer gfx_make_buffer(void *ptr, u64 size, const char *label) {
    sg_buffer_desc desc = {0};
    desc.data.ptr = ptr;
    desc.data.size = size;
    desc.label = label;
    return sg_make_buffer(&desc);
}

static sg_buffer gfx_make_idx_buffer(void *ptr, u64 size, const char *label) {
    sg_buffer_desc desc = {0};
    desc.type = SG_BUFFERTYPE_INDEXBUFFER;
    desc.data.ptr = ptr;
    desc.data.size = size;
    desc.label = label;
    return sg_make_buffer(&desc);
}

static void gfx_init_image(const char *file_name, sg_image *img) {
    s32 x = 0;
    s32 y = 0;
    s32 w = 0;
    s32 desired_channels = 4;
    
    cobble_dir dir = dir_get_for(file_name, SUBDIR_TEXTURE);
    stbi_uc *pixels = stbi_load(dir.ptr, &x, &y, &w, desired_channels);
    c_assert(pixels != NULL);
    
    sg_image_desc image_desc = {0};
    image_desc.width = x;
    image_desc.height = y;
    image_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    image_desc.data.subimage[0][0].ptr = pixels;
    image_desc.data.subimage[0][0].size = (size_t)(x * y * desired_channels);
    sg_init_image(*img, &image_desc);
    
    stbi_image_free(pixels);
}

static void gfx_init() {
    gfx.vbuf= gfx_make_buffer(vertices, sizeof(vertices), "color_pass_vertices");
    gfx.ibuf = gfx_make_idx_buffer(indices, sizeof(indices), "color_pass_indices");
    
    gfx.shadow_map = sg_make_image(&(sg_image_desc){
                                       .render_target = true,
                                       .width = 2048,
                                       .height = 2048,
                                       .pixel_format = SG_PIXELFORMAT_RGBA8,
                                       .sample_count = 1,
                                       .label = "shadow-map",
                                   });
    
    sg_image shadow_depth_img = sg_make_image(&(sg_image_desc){
                                                  .render_target = true,
                                                  .width = 2048,
                                                  .height = 2048,
                                                  .pixel_format = SG_PIXELFORMAT_DEPTH,
                                                  .sample_count = 1,
                                                  .label = "shadow-depth-buffer",
                                              });
    
    gfx.shadow_sampler = sg_make_sampler(&(sg_sampler_desc){
                                             .min_filter = SG_FILTER_NEAREST,
                                             .mag_filter = SG_FILTER_NEAREST,
                                             .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                             .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                             .label = "shadow-sampler",
                                         });
    
    { // color pass init
        gfx.display.action.colors[0].load_action = SG_LOADACTION_CLEAR;
        gfx.display.action.colors[0].clear_value = (sg_color){1.f, 0.25f, 0.1f, 1.f};
        
        gfx.display.bindings.images[IMG_diffuse_map] = sg_alloc_image();
        gfx.display.bindings.samplers[SMP_diffuse_sampler] = gfx_make_sampler();
        gfx.display.bindings.images[IMG_shadow_map] = gfx.shadow_map;
        gfx.display.bindings.samplers[SMP_shadow_sampler] = gfx.shadow_sampler;
        gfx.display.bindings.vertex_buffers[0] = gfx.vbuf;
        gfx.display.bindings.index_buffer = gfx.ibuf;
        
        sg_pipeline_desc pipeline_desc = {0};
        pipeline_desc.shader = sg_make_shader(color_pass_shader_desc(sg_query_backend()));
        pipeline_desc.layout.attrs[ATTR_color_pass_pos].format = SG_VERTEXFORMAT_FLOAT3;
        pipeline_desc.layout.attrs[ATTR_color_pass_norm].format = SG_VERTEXFORMAT_FLOAT3;
        pipeline_desc.layout.attrs[ATTR_color_pass_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
        pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
        pipeline_desc.cull_mode = SG_CULLMODE_BACK;
        pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
        pipeline_desc.depth.write_enabled = true;
        pipeline_desc.label = "color_pass_pipeline";
        gfx.display.pipeline = sg_make_pipeline(&pipeline_desc);
        
        gfx_init_image("test.png", &gfx.display.bindings.images[IMG_diffuse_map]);
    }
    
    { // shadow pass init
        gfx.shadow.action.colors[0].load_action = SG_LOADACTION_CLEAR;
        gfx.shadow.action.colors[0].clear_value = (sg_color){1.f, 1.f, 1.f, 1.f};
        
        gfx.shadow.bindings.vertex_buffers[0] = gfx.vbuf;
        gfx.shadow.bindings.index_buffer = gfx.ibuf;
        
        sg_attachments_desc shadow_attachments_desc = {0};
        shadow_attachments_desc.colors[0].image = gfx.shadow_map,
        shadow_attachments_desc.depth_stencil.image = shadow_depth_img,
        shadow_attachments_desc.label = "shadow-pass",
        gfx.shadow.attachments = sg_make_attachments(&shadow_attachments_desc);
        
        sg_pipeline_desc shadow_pipeline_desc = {0};
        shadow_pipeline_desc.layout.buffers[0].stride = 6 * sizeof(float);
        shadow_pipeline_desc.layout.attrs[ATTR_shadow_pos].format = SG_VERTEXFORMAT_FLOAT3;
        shadow_pipeline_desc.shader = sg_make_shader(shadow_shader_desc(sg_query_backend()));
        shadow_pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
        shadow_pipeline_desc.cull_mode = SG_CULLMODE_FRONT;
        shadow_pipeline_desc.sample_count = 1;
        shadow_pipeline_desc.colors[0].pixel_format = SG_PIXELFORMAT_RGBA8;
        shadow_pipeline_desc.depth.pixel_format = SG_PIXELFORMAT_DEPTH;
        shadow_pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
        shadow_pipeline_desc.depth.write_enabled = true;
        shadow_pipeline_desc.label = "shadow-pipeline";
        gfx.shadow.pipeline = sg_make_pipeline(&shadow_pipeline_desc);
    } 
}

static void gfx_frame() {
    const float t = (float)(sapp_frame_duration() * 5.0);
    fs_display_params_t fs_display_params = {0};
    vs_display_params_t vs_display_params = {0};
    
    vec3 eye_pos = {5.f, 5.f, 5.f};
    vec4 light_pos = {50, 50, -50, 1.f};
    MAT4(light_view);
    MAT4(light_proj);
    MAT4(light_proj_view);
    MAT4(model);
    
    { // shadow
        glm_lookat(light_pos, (vec3){0.f, 0.f, 0.f}, (vec3){0.f, 1.f, 0.f}, light_view);
        glm_ortho(-5, 5, -5, 5, 0.01f, 500.f, light_proj);
        glm_mat4_mul(light_proj, light_view, light_proj_view);
        
        vs_shadow_params_t model_vs_shadow_params = {0};
        glm_mat4_mul(light_proj_view, model, model_vs_shadow_params.mvp);
        
        sg_begin_pass(&(sg_pass){ .action = gfx.shadow.action, .attachments = gfx.shadow.attachments });
        sg_apply_pipeline(gfx.shadow.pipeline);
        sg_apply_bindings(&gfx.shadow.bindings);
        sg_apply_uniforms(UB_vs_shadow_params, &SG_RANGE(model_vs_shadow_params));
        sg_draw(0, 36, 1);
        sg_end_pass();
        
        vec3 light_dir = {light_pos[0], light_pos[1], light_pos[2]};
        glm_vec3_normalize(light_dir);
        glm_vec3_copy(light_dir, fs_display_params.light_dir);
        glm_vec3_copy(eye_pos, fs_display_params.eye_pos);
    }
    
    { // color
        MAT4(proj);
        MAT4(view);
        MAT4(proj_view);
        
        const r32 aspect = sapp_widthf() / sapp_heightf();
        glm_perspective(70, aspect, 0.01, 1000.f, proj);
        glm_lookat((vec3){0.f, 1.f, 6.f}, (vec3){0.f, 0.f, 0.f}, (vec3){0.f, 1.f, 0.f}, view);
        glm_mat4_mul(proj, view, proj_view);
        
        glm_mat4_mul(proj_view, model, vs_display_params.mvp);
        glm_mat4_copy(model, vs_display_params.model);
        glm_mat4_mul(light_proj_view, model, vs_display_params.light_mvp);
        glm_vec4_copy((vec4){1.f, 1.f, 0.f, 1.f}, vs_display_params.diff_color);
        
        sg_begin_pass(&(sg_pass){ .action = gfx.display.action, .swapchain = sglue_swapchain() });
        sg_apply_pipeline(gfx.display.pipeline);
        sg_apply_bindings(&gfx.display.bindings);
        sg_apply_uniforms(UB_fs_display_params, &SG_RANGE(fs_display_params));
        sg_apply_uniforms(UB_vs_display_params, &SG_RANGE(vs_display_params));
        sg_draw(0, 36, 1);
        sg_end_pass();
    }
    sg_commit();
}

static void gfx_end() {
    
}

#include "cobble_render_imgui.c"