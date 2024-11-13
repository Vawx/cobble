/* cobble_render_shapes.h : date = November 9th 2024 10:50 am */

#if !defined(COBBLE_RENDER_SHAPES_H)

#define SOKOL_SHAPE_IMPL
#include "sokol/util/sokol_shape.h"

typedef enum render_shapes_type {
    RENDER_SHAPES_BOX,
    RENDER_SHAPES_PLANE,
    RENDER_SHAPES_SPHERE,
    RENDER_SHAPES_CYLINDER,
    RENDER_SHAPES_TORUS,
    
    RENDER_SHAPES_COUNT
} render_shapes_type;

typedef enum render_shapes_movement  {
    RENDER_SHAPES_MOVEMENT_STATIC, 
    RENDER_SHAPES_MOVEMENT_DYNAMIC,
} render_shapes_movement;

typedef enum render_shape_desc_type {
    RENDER_SHAPE_DESC_TYPE_COLORED,
    RENDER_SHAPE_DESC_TYPE_BINDED,
} render_shape_desc_type;

typedef struct {
    render_shapes_type shape_type;
    render_shape_desc_type desc_type;
    render_shapes_movement shape_movement;
    union {
        v4 color;
        sg_bindings bindings;
    };
    union {
        struct {
            v3 pos;
            v3 rot;
            v3 scale;
        };
        struct {
            JPH_BodyID id;
        };
    };
} render_shape_description;

typedef struct {
    render_shape_description *shape_descriptions;
    u32 shape_descriptions_idx; // count of descriptions for this type
    
    render_shapes_type type;
    sshape_element_range_t draw;
} render_shapes;

typedef struct {
    sg_pipeline pip;
    sg_buffer vbuf;
    sg_buffer ibuf;
    render_shapes *shapes;
    vs_params_shapes_t params_shapes;
} render_shapes_state;

static render_shapes_state *get_shapes_pass();
static void shapes_init();
static void shapes_render_object_pass(mat4 *model, mat4 *projection_view, void *ptr);
static void shapes_render_pass(mat4 *projection_view);
static void shapes_push_desc(render_shape_description *desc);
static void shapes_close();
static void shapes_test();

#define COBBLE_RENDER_SHAPES_H
#endif //COBBLE_RENDER_SHAPES_H
