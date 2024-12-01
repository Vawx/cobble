#include "cobble_shader.h"

static gfx_render_pass_t static_draw;

static gfx_buffered_object_buffer_t buffered_objects;

static const char *DEFAULT_DIFFUSE_NAME = "default_diffuse.asset";
static gfx_handle_t default_texture_handle;

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
    
    sg_end_pass();
    sg_commit();
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

#include "cobble_render_imgui.c"
#include "cobble_ufbx.c"