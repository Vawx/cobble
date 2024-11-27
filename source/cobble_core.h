/* cobble_core.h : date = November 8th 2024 5:11 pm */

#if !defined(COBBLE_CORE_H)

#define STB_DS_IMPLEMENTATION
#include "stb/stb_ds.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

static void core_input_callback(sapp_event *event); // used by external systems for input, like IMGUI
#include "util/cobble_input.h"

#include "util/cobble_log.h"
#include "util/cobble_log.c"

#include "cobble_serialize.h"

#include "render/cobble_gfx.h"
#include "render/cobble_gfx.c"

#include "cobble_world.h"
#include "cobble_world.c"

#include "cobble_serialize.c"

typedef struct timing_t {
    r64 delta;
    r64 seconds;
    u64 ticks;
    
    u64 freq;
} timing_t;

static timing_t time;

static void core_init(void) {
    logger_init();
    sg_setup(&(sg_desc){
                 .environment = sglue_environment(),
                 .logger.func = slog_func,
             });
    
    LOG_TELL("Setting root path to %s", root_dir.ptr);
    
    gfx_init();
    imgui_init();
    
    jolt_init();
    jolt_make_dynamic_box(V3(100, 1, 100), V3(0.f, -1, 0), NOT_MOVING);
    
#if _WIN32
    LARGE_INTEGER ii;
    QueryPerformanceFrequency(&ii);
    time.freq = ii.QuadPart;
#endif
    
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
    
    gfx_frame();
    
    input_frame();
    
    u64 end = performance_counter();
    time.delta = (r64)(end - start) / (r64)time.freq;
    time.seconds += time.delta;
    ++time.ticks;
}

void core_cleanup(void) {
    gfx_end();
    logger_shutdown();
    sg_shutdown();
}

static void core_input_callback(sapp_event *event) {
    imgui_input_callback(event);
}

#define COBBLE_CORE_H
#endif //COBBLE_CORE_H
