
static void render_init() {
    cobble_view *v = view_make_new((vec3){-10.f, 0.f, 0.f}, (vec3){0.f, 0.f, -1.f}, 0.f, 0.f, 1);
    view_set_current(v);
    
    shadow_init();
    color_init();
    shapes_init();
}

static void render_frame(const vs_display_params_t *params) {
    { // SHADOW
        shadow_frame();
    }
    
    { // COLOR
        sg_begin_pass(&(sg_pass){ .action = get_color_pass()->pass_action, .swapchain = sglue_swapchain() });
        mat4 pv = {0};
        glm_mat4_mul(get_current_view()->projection, get_current_view()->view, pv);
        color_frame(&pv);
    }
    
    //imgui_frame(state.pass_action);
    
    
    sg_end_pass();
    sg_commit();
}

static void render_close() {
    shapes_close();
}

#include "cobble_render_shadow.c"
#include "cobble_render_color.c"
#include "cobble_render_shapes.c"
#include "cobble_render_imgui.c"
#include "cobble_view.c"