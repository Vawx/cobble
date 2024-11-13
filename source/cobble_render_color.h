/* cobble_render_color.h : date = November 12th 2024 4:02 pm */

#if !defined(COBBLE_RENDER_COLOR_H)

typedef struct color_state {
    sg_pass_action pass_action;
    sg_pipeline pip;
    sg_bindings bind;
    sg_shader shader;
    r32 ry;
} color_state;

typedef struct {
    float x, y, z;
    int16_t u, v;
} vertex_t;

typedef enum bind_image_idx_type {
    BIND_IMAGE_IDX_UNDEFINED = 0,
    BIND_IMAGE_IDX_DIFFUSE   = 1,
    BIND_IMAGE_IDX_NORMAL    = 2,
    BIND_IMAGE_IDX_SPEC      = 3,
    
    BIND_IMAGE_IDX_COUNT     = 4
} bind_image_idx_type;

static color_state *get_color_pass();

static void render_bind_image(sg_bindings *bind, bind_image_idx_type idx);
static void render_bind_sampler(sg_bindings *bind, bind_image_idx_type idx);
static void render_bind_vertex(sg_bindings *bind, const char *label, sg_range *vertices_range, u32 idx);
static void render_bind_index(sg_bindings *bind, const char *label, sg_range *indices_range, u32 idx);
static void render_bind_texture(sg_bindings *bind, bind_image_idx_type idx, const cobble_dir *dir);
static void color_init();
static void color_frame(mat4 *projection_view);
static void color_end();

#define COBBLE_RENDER_COLOR_H
#endif //COBBLE_RENDER_COLOR_H
