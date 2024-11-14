/* cobble_gfx.h : date = November 13th 2024 5:28 pm */

#if !defined(COBBLE_GFX_H)

#include "cobble_view.h"
#include "cobble_render_imgui.h"
#include "sokol/util/sokol_shape.h"
#include "shapes.h"

typedef enum shape_type {
    BOX,
    PLANE,
    SPHERE,
    CYLINDER,
    TORUS,
    
    NUM_SHAPES
} shape_type;

typedef enum shape_movement  {
    SHAPE_MOVEMENT_STATIC, 
    SHAPE_MOVEMENT_DYNAMIC,
} shape_movement;

typedef enum render_shape_desc_type {
    SHAPE_DESC_TYPE_COLORED,
    SHAPE_DESC_TYPE_BINDED,
} shape_desc_type;

typedef struct {
    shape_type shape_type;
    shape_desc_type desc_type;
    shape_movement shape_movement;
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
} shape_description;

typedef struct {
    shape_description *shape_descriptions;
    u32 shape_descriptions_idx; // count of descriptions for this type
    
    shape_type type;
    sshape_element_range_t draw;
} render_shape;

typedef struct cobble_gfx {
    sg_image shadow_map;
    sg_sampler shadow_sampler;
    sg_buffer vbuf;
    sg_buffer ibuf;
    float ry;
    struct {
        sg_pass_action pass_action;
        sg_attachments atts;
        sg_pipeline pip;
        sg_bindings bind;
    } shadow;
    struct {
        sg_pass_action pass_action;
        sg_pipeline pip;
        sg_bindings bind;
        struct {
            sg_pass_action pass_action;
            sg_attachments atts;
            sg_pipeline pip;
            sg_buffer vbuf;
            sg_buffer ibuf;
            render_shape shapes[NUM_SHAPES];
            vs_params_shapes_t vs_params;
            float rx, ry;
        } primitives;
    } display;
    struct {
        sg_pipeline pip;
        sg_bindings bind;
    } dbg;
} cobble_gfx;

static cobble_gfx *get_gfx();
static void gfx_init();
static void gfx_frame();
static void gfx_end();
static void shapes_push_desc(shape_description *desc);

#define COBBLE_GFX_H
#endif //COBBLE_GFX_H
