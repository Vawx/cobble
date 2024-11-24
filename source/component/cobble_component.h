/* component_t.h : date = November 23rd 2024 9:00 pm */

#if !defined(COBBLE_COMPONENT_H)

typedef enum component_type {
    COMPONENT_TYPE_NONE,
    COMPONENT_TYPE_MESH,
    COMPONENT_TYPE_VIEW,
    
    COMPONENT_TYPE_COUNT,
} component_type;

typedef struct component_t {
    component_type type;
    union {
        struct {
            vec3 position;
            vec3 rotation;
            vec3 scale;
        } mesh;
        struct {
            u32 idx; // id of view list.
            u8 follow_parent_position;
        } view;
    };
} component_t;

#define ENTITY_COMPONENT_MAX 8
typedef struct entity_t {
    component_t components[ENTITY_COMPONENT_MAX];
    u32 components_idx;
    u64 id;
} entity_t;

#define ENTITY_START_COUNT kilo(1) // used for initial allocation. will expand if needed
typedef struct entity_buffer_t {
    entity_t *entities;
    u32 entities_idx;
} entity_buffer_t;

static void entity_init();
static entity_t *entity_make();
static void entity_store(entity_t *e);
static void entity_delete(entity_t *e);

#define COBBLE_COMPONENT_H
#endif //COBBLE_COMPONENT_H
