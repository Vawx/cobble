/* cobble_world.h : date = November 29th 2024 9:40 pm */

#if !defined(COBBLE_WORLD_H)

typedef enum component_type {
    COMPONENT_TYPE_MESH,
    COMPONENT_TYPE_PHYSICS,
    
    COMPONENT_TYPE_COUNT
} component_type;

static const char *component_type_str[COMPONENT_TYPE_COUNT] = {
    "Mesh",
    "Physics",
};

typedef struct component_t {
    ident_t ident;
    union {
        struct {
            gfx_handle_t gfx_handle; // this to the model_buffer, which holds the model, mesh *, and parts *. parts hold the buffers.
        } mesh;
        struct {
            JPH_BodyID *id;
        } physics;
    };
} component_t;

#define COMPONENT_INITIAL_COUNT mega(1)// NOTE(Kyle): odds we actually use this much?
typedef struct component_system_buffer_t {
    component_type type;
    component_t *ptr;
    u32 idx;
    u32 count;
} component_system_buffer_t;

#define ENTITY_INITIAL_COUNT kilo(1)
typedef struct entity_t {
    u32 id;
    
    vec3 position;
    vec3 rotation;
    vec3 scale;
    mat4 mat;
} entity_t;
TYPE_BUFFER(entity_t);

typedef void (*component_system)(const component_t *components, const u32 count);

typedef struct world_t {
    component_system_buffer_t system_components[COMPONENT_TYPE_COUNT];
    entity_t_buffer_t entities;
    component_system system[COMPONENT_TYPE_COUNT];
} world_t;

static entity_t *world_make_entity();

static component_t *world_make_component(entity_t *entity, component_type type);
static void world_init_component_mesh(component_t *comp, const dir_t *directory);
static void world_init_component_physics(component_t *comp);

static void world_set_entity_pos(entity_t *e, vec3 pos);
static void world_set_entity_rot(entity_t *e, vec3 rot);
static void world_set_entity_scale(entity_t *e, vec3 scale);

static void world_init();
static void world_frame();
static void world_shutdown();

#define COBBLE_WORLD_H
#endif //COBBLE_WORLD_H
