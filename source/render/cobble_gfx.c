#include "cobble_shader.h"

static gfx_render_pass_t static_draw;

static gfx_buffered_object_buffer_t buffered_objects;
static gfx_buffered_object_buffer_t frame_buffered_objects;

static const char *DEFAULT_DIFFUSE_NAME = "default_diffuse.png";
static sg_image default_image;
static sg_sampler default_sampler;

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

static void gfx_init() {
    buffered_objects.ptr = (gfx_buffered_object_t*)c_alloc(sizeof(gfx_buffered_object_t) * BUFFERED_OBJECT_DEFAULT_COUNT);
    buffered_objects.idx = 0;
    buffered_objects.count = BUFFERED_OBJECT_DEFAULT_COUNT;
    
    frame_buffered_objects.ptr = (gfx_buffered_object_t*)c_alloc(sizeof(gfx_buffered_object_t) * BUFFERED_OBJECT_DEFAULT_COUNT);
    frame_buffered_objects.idx = 0;
    frame_buffered_objects.count = BUFFERED_OBJECT_DEFAULT_COUNT;
    
    sg_backend backend = sg_query_backend();
    static_draw.shader = sg_make_shader(static_lit_shader_desc(backend));
    
    sg_pipeline_desc desc = {0};
    desc.shader = static_draw.shader;
    desc.layout = mesh_vertex_layout;
    desc.index_type = SG_INDEXTYPE_UINT32;
    desc.face_winding = SG_FACEWINDING_CCW;
    desc.cull_mode = SG_CULLMODE_BACK;
    desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    desc.depth.write_enabled = true;
    
	static_draw.pipeline = sg_make_pipeline(&desc);
    
	default_image = sg_alloc_image();
    
    dir_t default_dir = dir_get_for(DEFAULT_DIFFUSE_NAME, SUBDIR_TEXTURE);
    int x,y,n;
    unsigned char *data = stbi_load(default_dir.ptr, &x, &y, &n, 4);
    c_assert(data != NULL);
    
    sg_image_desc img_desc = {0};
    img_desc.width = x;
    img_desc.height = y;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.data.subimage[0][0].ptr = data;
    img_desc.data.subimage[0][0].size = x * y * 4;
    sg_init_image(default_image, &img_desc);
    
    stbi_image_free(data);
    
    sg_sampler_desc sampler_desc = {0};
    sampler_desc.min_filter = SG_FILTER_LINEAR,
    sampler_desc.mag_filter = SG_FILTER_LINEAR,
    default_sampler = sg_make_sampler(&sampler_desc);
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
    
    for(s32 i = 0; i < frame_buffered_objects.idx; ++i) {
        gfx_buffered_object_t *obj = &frame_buffered_objects.ptr[i];
        
        uv_tiling_ubo_t tiling_ubo = {
            .tile_x = 1.f,
            .tile_y = 1.f
        };
        sg_apply_uniforms(UB_uv_tiling_ubo, &SG_RANGE(tiling_ubo));
        
        MAT4(ident);
        MAT4(proj);
        MAT4(view);
        
        glm_lookat((vec3){0.f, 10.f, -10.f}, (vec3){0.f, 1.f, 0.f}, GLM_YUP, view);
        glm_perspective(70.f, sapp_widthf() / sapp_heightf(), 0.01f, 100.f, proj);
        
        mesh_vertex_ubo_t mesh_ubo = {0};
        glm_mat4_copy(ident, mesh_ubo.geometry_to_world);
        glm_mat4_copy(ident, mesh_ubo.normal_to_world);
        glm_mat4_mul(proj, view, mesh_ubo.world_to_clip);
        mesh_ubo.f_num_blend_shapes = 0;
        sg_apply_uniforms(0, SG_RANGE_REF(mesh_ubo));
        
		sg_bindings binds = {
			.vertex_buffers[0] = obj->vertex,
			.index_buffer = obj->index,
            .images[IMG_diffuse_texture] = default_image,
            .samplers[SMP_diffuse_sampler] = default_sampler
		};
		sg_apply_bindings(&binds);
        
		sg_draw(0, obj->num_indices, 1);
    }
    
    sg_end_pass();
    sg_commit();
    
    frame_buffered_objects.idx =0;
}

static void gfx_shutdown() {
    
}

static gfx_handle_t gfx_make_buffered_object(u32 vertex_size, u8 *vertices, u32 index_size, u32 *indices, u64 hash, u32 num_indices) {
    c_assert(buffered_objects.ptr != NULL);
    
    gfx_buffered_object_t *obj = &buffered_objects.ptr[buffered_objects.idx++]; // increment objects here.
    
    obj->index = sg_make_buffer(&(sg_buffer_desc){
                                    .size = index_size,
                                    .type = SG_BUFFERTYPE_INDEXBUFFER,
                                    .data = { indices, index_size },
                                });
    
    obj->vertex = sg_make_buffer(&(sg_buffer_desc){
                                     .size = vertex_size,
                                     .type = SG_BUFFERTYPE_VERTEXBUFFER,
                                     .data = { vertices, vertex_size },
                                 });
    
    obj->file_hash = hash;
    obj->num_indices = num_indices;
    
    gfx_handle_t result = {0};
    result.id = buffered_objects.idx - 1;
    return result;
}

static void gfx_render_mesh(const gfx_handle_t *handle) {
    c_assert(handle->id >= 0 && handle->id < buffered_objects.idx);
    
    gfx_buffered_object_t *buffer = &buffered_objects.ptr[handle->id];
    memcpy(&frame_buffered_objects.ptr[frame_buffered_objects.idx], buffer, sizeof(gfx_buffered_object_t));
    ++frame_buffered_objects.idx;
}

#include "cobble_render_imgui.c"
#include "cobble_ufbx.c"