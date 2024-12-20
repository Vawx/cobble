/** sokol SHDC:
./sokol-shdc --input ../../../../content/shader/cobble_shader.glsl --output ../../../../source/shader/cobble_shader.h --slang glsl430:hlsl5:metal_macos
*/

#pragma warning(disable: 4090) // C4090: 'function': different 'const' qualifiers

#define SOKOL_IMPL
#if defined(__APPLE__)
#define SOKOL_METAL
#elif defined(_WIN32)
#define SOKOL_D3D11
#elif defined(__EMSCRIPTEN__)
#define SOKOL_GLES2
#else
#define SOKOL_GLCORE33
#endif

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui/cimgui.h"

#include "cobble_types.h"

#include "cglm/cglm.h"
#include "util/cobble_util.h"
#include "cobble_jolt.c"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"

#include "cobble_core.h"

sapp_desc sokol_main(int argc, char* argv[]) {
    root_dir_init(argv[0]);
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = core_init,
        .frame_cb = core_frame,
        .event_cb = input_callback,
        .cleanup_cb = core_cleanup,
        .width = 1920,
        .height = 1080,
        .window_title = "cobble",
        .icon.sokol_default = false,
        .logger.func = slog_func,
        .enable_clipboard = true,
        .ios_keyboard_resizes_canvas = false,
        .win32_console_create = true,
        .sample_count = 4,
        .high_dpi = true
    };
}