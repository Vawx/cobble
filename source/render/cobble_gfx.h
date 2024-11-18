/* cobble_gfx.h : date = November 13th 2024 5:28 pm */

#if !defined(COBBLE_GFX_H)
#include "cobble_shader.h"
#include "cobble_render_imgui.h"
#include "sokol/util/sokol_shape.h"

#include "cobble_view.h"

static struct {
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
    } display;
    struct {
        sg_pipeline pip;
        sg_bindings bind;
    } dbg;
} state;

static void gfx_init();
static void gfx_frame();
static void gfx_end();

#define COBBLE_GFX_H
#endif //COBBLE_GFX_H
