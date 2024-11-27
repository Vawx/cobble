/* cobble_world.h : date = November 27th 2024 2:07 pm */

#if !defined(COBBLE_WORLD_H)

typedef enum componnet_type {
    COMPONENT_TYPE_NONE,
    COMPONENT_TYPE_MESH,
    COMPONENT_TYPE_SIMULATED,
    
    COMPONENT_TYPE_COUNT;
} component_type;

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

typedef struct view_buffter_t {
    view_t *views;
    u32 idx;
    u32 count;
} view_buffter_t;

typedef struct scene_t {
    view_buffer_t views;
} scene_t;

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
} world_t;

static void world_init();
static void world_frame();
static void world_shutdown();

#define COBBLE_WORLD_H
#endif //COBBLE_WORLD_H
