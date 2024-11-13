
#include "cimgui/cimgui.h"
#define SOKOL_IMGUI_IMPL
#include "sokol/util/sokol_imgui.h"

static const u64 COBBLE_IMGUI_STATE_NONE          = 0x0;
static const u64 COBBLE_IMGUI_STATE_SHOW_DEFAULTS = 0x1;
static const u64 COBBLE_IMGUI_STATE_SHOW_CONTENT  = 0x2;
static const u64 COBBLE_IMGUI_STATE_SHOW_STATS    = 0x4;

static u64 imgui_state;

static void imgui_init() {
    simgui_setup(&(simgui_desc_t) {
                     .logger.func = slog_func
                 });
    imgui_state = COBBLE_IMGUI_STATE_NONE;
}

static void imgui_editor_defaults() {
    static bool defaults = false;
    defaults = imgui_state & COBBLE_IMGUI_STATE_SHOW_DEFAULTS;
    if(defaults) {
        igSetNextWindowSize((ImVec2){300, 600}, ImGuiCond_FirstUseEver);
        igBegin("Defaults", &defaults, 0);
        igEnd();
    }
}

static void imgui_editor_content() {
    static bool content = false;
    content = imgui_state & COBBLE_IMGUI_STATE_SHOW_CONTENT;
    if(content) {
        igSetNextWindowSize((ImVec2){800, 300}, ImGuiCond_FirstUseEver);
        igBegin("Content", &content, 0);
        igEnd();
    }
}

static void imgui_editor_stats() {
    static bool stats = false;
    stats = imgui_state & COBBLE_IMGUI_STATE_SHOW_STATS;
    if(stats) {
        igSetNextWindowSize((ImVec2){128, 64}, ImGuiCond_FirstUseEver);
        igBegin("Stats", &stats, 0);
        igEnd();
    }
}

static void imgui_frame() {
    const u32 width = sapp_width();
    const u32 height = sapp_height();
    const r64 delta = sapp_frame_duration();
    const r64 dpi = sapp_dpi_scale();
    simgui_new_frame(&(simgui_frame_desc_t) {
                         .width = width, 
                         .height = height,
                         .delta_time = delta,
                         .dpi_scale = dpi
                     });
    
    imgui_editor_defaults();
    imgui_editor_content();
    imgui_editor_stats();
    simgui_render();
}

static void imgui_end() {
    simgui_shutdown();
}

static void imgui_input_callback(sapp_event *event) {
    simgui_handle_event(event);
    
    // this is handled _along with_ normal input. as in, this event and
    // the input event, which handles things like is_key_pressed(), 
    // will be called in the same frame.
    switch(event->type) {
        case SAPP_EVENTTYPE_KEY_UP: { // key up so i dont have to worry about sticky keys sending many events
            switch(event->key_code) {
                case SAPP_KEYCODE_1: {
                    imgui_state ^= COBBLE_IMGUI_STATE_SHOW_DEFAULTS;
                } break;
                case SAPP_KEYCODE_2: {
                    imgui_state ^= COBBLE_IMGUI_STATE_SHOW_CONTENT;
                } break;
                case SAPP_KEYCODE_3: {
                    imgui_state ^= COBBLE_IMGUI_STATE_SHOW_STATS;
                } break;
            };
        } break;
    }
}
