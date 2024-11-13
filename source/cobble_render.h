/* cobble_render.h : date = November 8th 2024 5:14 pm */

#if !defined(COBBLE_RENDER_H)

#include "color_pass.h"
#include "shadow_pass.h"
#include "shapes.h"

#include "cobble_render_color.h"
#include "cobble_render_shadow.h"
#include "cobble_render_shapes.h"
#include "cobble_render_imgui.h"
#include "cobble_view.h"

static void render_init();
static void render_frame(const vs_display_params_t *params);
static void render_close();

#define COBBLE_RENDER_H
#endif //COBBLE_RENDER_H
