/* cobble_gfx.h : date = November 13th 2024 5:28 pm */

#if !defined(COBBLE_GFX_H)
#include "cobble_render_imgui.h"

typedef union gfx_vertex {
    r32 x, y, z;
    u16 u, v;
} gfx_vertex;

typedef struct gfx_state {
    sg_pass_action action;
    sg_pipeline pipeline;
    sg_bindings bindings;
    struct {
        sg_image diffuse_map;
        sg_sampler diffuse_sampler;
    } display;
} gfx_state;

static gfx_state *get_gfx();
static void gfx_init();
static void gfx_frame();
static void gfx_end();

#define COBBLE_GFX_H
#endif //COBBLE_GFX_H
