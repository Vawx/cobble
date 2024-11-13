/* cobble_core.h : date = November 8th 2024 5:11 pm */

#if !defined(COBBLE_CORE_H)

#define STB_DS_IMPLEMENTATION
#include "stb/stb_ds.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

static void core_input_callback(sapp_event *event); // used by external systems for input, like IMGUI
#include "util/cobble_input.h"

#include "cobble_render.h"
#include "cobble_render.c"

typedef struct cobble_time {
    r64 delta;
    r64 seconds;
    u64 ticks;
    
    u64 freq;
} cobble_time;

static cobble_time time;

static void core_init(void) {
    sg_setup(&(sg_desc){
                 .environment = sglue_environment(),
                 .logger.func = slog_func,
             });
    
    printf("[cobble]: initializing root directory %s\n", root_dir.ptr);
    render_init();
    
    jolt_init();
    jolt_make_dynamic_box(V3(100, 1, 100), V3(0.f, -1, 0), NOT_MOVING);
    shapes_push_desc(&(render_shape_description) {
                         .color = V4(0.3f, 0.3f, 0.3f, 1.f),
                         .shape_type = RENDER_SHAPES_BOX,
                         .desc_type = RENDER_SHAPE_DESC_TYPE_COLORED,
                         .shape_movement = RENDER_SHAPES_MOVEMENT_STATIC,
                         .pos = V3(0.f, -0.5f, 0.f),
                         .rot = V3d(0.f),
                         .scale = V3(100, 1, 100)
                     });
    
#if _WIN32
    LARGE_INTEGER ii;
    QueryPerformanceFrequency(&ii);
    time.freq = ii.QuadPart;
#endif
    
    imgui_init();
    
    //shapes_test();
    sfetch_setup(&(sfetch_desc_t){
                     .max_requests = 1,
                     .num_channels = 1,
                     .num_lanes = 1,
                     .logger.func = slog_func,
                 });
}

#if _WIN32
static u64 performance_counter() {
    LARGE_INTEGER ii;
    QueryPerformanceCounter(&ii);
    return ii.QuadPart;
}
#else
static u64 performance_counter() {c_assert_break();}
#endif

static void core_frame(void) {
    u64 start = performance_counter();
    sfetch_dowork();
    
    jolt_step(time.delta);
    
    vec2 screen_size = {sapp_widthf(), sapp_heightf()};
    
    vs_display_params_t params;
    view_frame(screen_size, &params);
    render_frame(&params);
    input_frame();
    
    u64 end = performance_counter();
    time.delta = (r64)(end - start) / (r64)time.freq;
    time.seconds += time.delta;
    ++time.ticks;
    
}

void core_cleanup(void) {
    render_close();
    sg_shutdown();
}

static void core_input_callback(sapp_event *event) {
    imgui_input_callback(event);
}

#define COBBLE_CORE_H
#endif //COBBLE_CORE_H
