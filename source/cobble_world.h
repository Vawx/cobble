/* cobble_world.h : date = November 27th 2024 2:07 pm */

#if !defined(COBBLE_WORLD_H)

typedef struct entity_t {
    union {
        struct {
            vec3 position;
            vec3 rotation;
            vec3 scale;
        };
        struct {
            mat4 matrix;
        };
    };
    s32 id;
} entity_t;

#define WORLD_DEFAULT_ENTITY_COUNT kilo(1)
typedef struct entity_buffer_t {
    entity_t *entities;
    u32 idx;
    u32 count;
} entity_buffer_t;

typedef struct view_t {
    mat4 projection;
    mat4 view;
    
    vec3 position;
    vec3 forward;
    vec3 right;
    vec3 up;
    r32 yaw;
    r32 pitch;
} view_t;

#define WORLD_DEFAULT_VIEW_COUNT 8
typedef struct view_buffer_t {
    view_t *views;
    u32 idx;
    u32 count;
} view_buffer_t;

typedef struct scene_t {
    view_buffer_t views;
} scene_t;

#define WORLD_DEFAULT_SCENE_COUNT 8
typedef struct scene_buffer_t {
    scene_t *scenes;
    u32 idx;
    u32 count;
    entity_buffer_t entities;
} scene_buffer_t;

// each _scene_ holds n-views _and_ n-entities.
// each _world_ holds n_scenes
typedef struct world_t {
    scene_buffer_t scenes;
    u32 current_scene;
} world_t;

static entity_t *world_make_entity(entity_buffer_t *entities);
static view_t *world_make_view(view_buffer_t *views);
static scene_t *world_make_scene(scene_buffer_t *scenes);

static void world_init();
static void world_frame();
static void world_shutdown();

#define COBBLE_WORLD_H
#endif //COBBLE_WORLD_H
