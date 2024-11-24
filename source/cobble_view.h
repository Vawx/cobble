/* view_t.h : date = November 7th 2024 8:49 pm */

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
} view_t;

#define VIEW_MOVE_SPEED 4
#define VIEW_ROTATION_SPEED 2
#define VIEW_DEFAULT_FOV 70
#define MAX_VIEWS 64

static view_t *get_current_view();
static view_t *view_make_new(vec3 pos, vec3 dir, r32 yaw, r32 pitch, u8 input_commanded);
static void view_set_current(view_t *view);
static void view_set_current_idx(u8 idx);
static view_t *view_get(u8 idx);
static void view_update(view_t *view, vec2 screen_size);
static void view_frame(vec2 screen_size, mat4 *out_view_projection);

#define COBBLE_VIEW_H
#endif //COBBLE_VIEW_H
