/* cobble_component.h : date = November 26th 2024 9:49 am */

#if !defined(COBBLE_COMPONENT_H)

typedef enum component_type {
    COMPONENT_TYPE_NONE,
    COMPONENT_TYPE_MESH,
    
    COMPONENT_TYPE_COUNT
} component_type;

typedef struct component_t {
    component_type type;
    union {
        struct {
            gfx_model_handle_t handle;
        } mesh;
    };
    
    s32 entity_id; // entity who owns this instance
} component_t;

#define COMPONENT_DEFAULT_COUNT kilo(1)
typedef struct component_buffer_t {
    component_t *components;
    u32 components_idx;
    u32 count;
} component_buffer_t;

static void component_init();
static void component_frame();
static void component_shutdown();

static void component_mesh_assign(component_t *comp, dir_t *dir);

static component_t *component_make(component_type type, s32 entity_id);


#define COBBLE_COMPONENT_H
#endif //COBBLE_COMPONENT_H
