/* cobble_gfx.h : date = November 13th 2024 5:28 pm */

#if !defined(COBBLE_GFX_H)
#include "cobble_render_imgui.h"

typedef union gfx_vertex {
    r32 x, y, z;
    r32 xn, yn, zn;
    u16 u, v;
} gfx_vertex;

typedef struct gfx_state {
    sg_buffer vbuf;
    sg_buffer ibuf;
    sg_image shadow_map;
    sg_sampler shadow_sampler;
    struct {
        sg_pass_action action;
        sg_pipeline pipeline;
        sg_bindings bindings;
        sg_image diffuse_map;
        sg_sampler diffuse_sampler;
    } display;
    struct {
        sg_bindings bindings;
        sg_pass_action action;
        sg_pipeline pipeline;
        sg_attachments attachments;
    } shadow;
} gfx_state;

static gfx_state *get_gfx();
static void gfx_init();
static void gfx_frame();
static void gfx_end();

#define COBBLE_GFX_H
#endif //COBBLE_GFX_H
