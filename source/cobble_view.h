/* cobble_view.h : date = November 7th 2024 8:49 pm */

#if !defined(COBBLE_VIEW_H)

typedef struct {
    vec3 pos;
    vec3 dir;
    vec3 right;
    vec3 up;
    
    r32 yaw;
    r32 pitch;
    r32 fov;
    mat4 projection;
    mat4 view;
    u8 is_input_commanded;
    u8 id;
} cobble_view;

#define COBBLE_VIEW_MOVE_SPEED 4
#define COBBLE_VIEW_ROTATION_SPEED 2
#define COBBLE_VIEW_DEFAULT_FOV 70
#define MAX_COBBLE_VIEWS 64

static cobble_view *get_current_view();
static cobble_view *view_make_new(vec3 pos, vec3 dir, r32 yaw, r32 pitch, u8 input_commanded);
static void view_set_current(cobble_view *view);
static void view_set_current_idx(u8 idx);
static cobble_view *view_get(u8 idx);
static void view_update(cobble_view *view, vec2 screen_size);
static void view_frame(vec2 screen_size, mat4 *out_view_projection);

#define COBBLE_VIEW_H
#endif //COBBLE_VIEW_H
