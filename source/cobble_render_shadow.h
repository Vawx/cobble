/* cobble_render_shadow.h : date = November 11th 2024 5:18 pm */

#if !defined(COBBLE_RENDER_SHADOW_H)

typedef struct {
    sg_pass_action pass_action;
    sg_attachments atts;
    sg_pipeline pip;
    sg_bindings bind;
    sg_image shadow_map;
    sg_sampler shadow_sampler;
} cobble_shadow;

static cobble_shadow *get_shadow_pass();
static void shadow_init();
static void shadow_frame();
static void shadow_end();

#define COBBLE_RENDER_SHADOW_H
#endif //COBBLE_RENDER_SHADOW_H
