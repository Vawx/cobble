
#include "color_shadow_pass.h"

static gfx_state gfx;
static gfx_state *get_gfx() {return &gfx;}

static gfx_vertex vertices[] = {
    -1.0f, -1.0f, -1.0f,      0,     0 ,
    1.0f, -1.0f, -1.0f,  1,     0 ,
    1.0f,  1.0f, -1.0f,  1, 1 ,
    -1.0f,  1.0f, -1.0f,      0, 1 ,
    
    -1.0f, -1.0f,  1.0f,      0,     0 ,
    1.0f, -1.0f,  1.0f,  1,     0 ,
    1.0f,  1.0f,  1.0f,  1, 1 ,
    -1.0f,  1.0f,  1.0f,      0, 1 ,
    
    -1.0f, -1.0f, -1.0f,      0,     0 ,
    -1.0f,  1.0f, -1.0f,  1,     0 ,
    -1.0f,  1.0f,  1.0f,  1, 1 ,
    -1.0f, -1.0f,  1.0f,      0, 1 ,
    
    1.0f, -1.0f, -1.0f,      0,     0 ,
    1.0f,  1.0f, -1.0f,  1,     0 ,
    1.0f,  1.0f,  1.0f,  1, 1 ,
    1.0f, -1.0f,  1.0f,      0, 1 ,
    
    -1.0f, -1.0f, -1.0f,      0,     0 ,
    -1.0f, -1.0f,  1.0f,  1,     0 ,
    1.0f, -1.0f,  1.0f,  1, 1 ,
    1.0f, -1.0f, -1.0f,      0, 1 ,
    
    -1.0f,  1.0f, -1.0f,      0,     0 ,
    -1.0f,  1.0f,  1.0f,  1,     0 ,
    1.0f,  1.0f,  1.0f,  1, 1 ,
    1.0f,  1.0f, -1.0f,      0, 1 ,
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
    { // color pass init
        gfx.action.colors[0].load_action = SG_LOADACTION_CLEAR;
        gfx.action.colors[0].clear_value = (sg_color){1.f, 0.25f, 0.1f, 1.f};
        
        gfx.bindings.images[IMG_diffuse_map] = sg_alloc_image();
        gfx.bindings.samplers[SMP_diffuse_sampler] = gfx_make_sampler();
        gfx.bindings.vertex_buffers[0] = gfx_make_buffer(vertices, sizeof(vertices), "color_pass_vertices");
        gfx.bindings.index_buffer = gfx_make_idx_buffer(indices, sizeof(indices), "color_pass_indices");
        
        sg_pipeline_desc pipeline_desc = {0};
        pipeline_desc.shader = sg_make_shader(color_shadow_pass_shader_desc(sg_query_backend()));
        pipeline_desc.layout.attrs[ATTR_color_shadow_pass_pos].format = SG_VERTEXFORMAT_FLOAT3;
        pipeline_desc.layout.attrs[ATTR_color_shadow_pass_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
        pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
        pipeline_desc.cull_mode = SG_CULLMODE_BACK;
        pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
        pipeline_desc.depth.write_enabled = true;
        pipeline_desc.label = "color_pass_pipeline";
        gfx.pipeline = sg_make_pipeline(&pipeline_desc);
        
        gfx_init_image("test.png", &gfx.bindings.images[IMG_diffuse_map]);
    }
}

static void gfx_frame() {
    const float t = (float)(sapp_frame_duration() * 5.0);
    MAT4(proj);
    MAT4(view);
    MAT4(proj_view);
    MAT4(model);
    
    const r32 aspect = sapp_widthf() / sapp_heightf();
    glm_perspective(70, aspect, 0.01, 1000.f, proj);
    glm_lookat((vec3){0.f, 1.f, 6.f}, (vec3){0.f, 0.f, 0.f}, (vec3){0.f, 1.f, 0.f}, view);
    glm_mat4_mul(proj, view, proj_view);
    
    vs_params_t vs_params = {0};
    static r32 x = 0.f;
    glm_rotated(model, x, (vec3){0.1f, 0.3f, 0.4f});
    x += t;
    glm_mat4_mul(proj_view, model, vs_params.mvp);
    
    sg_begin_pass(&(sg_pass){ .action = gfx.action, .swapchain = sglue_swapchain() });
    sg_apply_pipeline(gfx.pipeline);
    sg_apply_bindings(&gfx.bindings);
    sg_apply_uniforms(UB_vs_params, &SG_RANGE(vs_params));
    sg_draw(0, 36, 1);
    sg_end_pass();
    sg_commit();
}

static void gfx_end() {
    
}

#include "cobble_render_imgui.c"