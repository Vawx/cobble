/* cobble_entity.h : date = November 25th 2024 8:48 pm */

#if !defined(COBBLE_ENTITY_H)

#include "cobble_component.h"

// entities are expected to have a world location.
// by default it is a static world location.
// components can override what position looks like: such as physics

#define ENTITY_INITIAL_COMPONENT_COUNT 8
typedef struct entity_t {
    union {
        struct {
            vec3 position;
            vec3 rotation;
            vec3 scale;
        } location;
    };
    
    const component_t **components; // ptr to components in component_buffer
    u32 components_idx;
    u32 components_count;
    s32 id;
} entity_t;

#define ENTITY_INITIAL_COUNT kilo(1)
typedef struct entity_buffer_t {
    entity_t *entities;
    u32 entities_idx;
    u32 count;
} entity_buffer_t;

static void entity_init();
static void entity_frame();
static void entity_shutdown();

static entity_t *entity_make();
static entity_t *entity_make_positioned(vec3 position, vec3 rotation, vec3 scale);

static component_t *entity_push_component(entity_t *entity, component_type type);
static void entity_pop_component(entity_t *entity);
static void entity_remove_component_by_type(entity_t *entity, component_type type, s32 remove_count);
static void entity_remove_component(entity_t *entity, component_t *comp);

#define COBBLE_ENTITY_H
#endif //COBBLE_ENTITY_H
