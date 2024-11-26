
static entity_buffer_t entity_buffer;

static void entity_init() {
    c_assert(entity_buffer.entities == NULL); // initializing twice?
    
    entity_buffer.entities = (entity_t*)c_alloc(sizeof(entity_t) * ENTITY_INITIAL_COUNT);
    entity_buffer.entities_idx = 0;
    entity_buffer.count = ENTITY_INITIAL_COUNT;
    
    component_init();
}

static void entity_frame() {
    component_frame();
    
    
}

static void entity_shutdown() {
    
}

static entity_t *entity_make() {
    if(entity_buffer.entities_idx >= ENTITY_INITIAL_COUNT) {
        LOG_YELL("increasing size of entity buffer from %d to %d", entity_buffer.count, entity_buffer.count * 2);
        entity_buffer.count *= 2;
        entity_buffer.entities = (entity_t*)realloc(entity_buffer.entities, entity_buffer.count);
    }
    
    entity_t *result = &entity_buffer.entities[entity_buffer.entities_idx++]; // increment entites idx here
    glm_vec3_copy((vec3){0.f, 0.f, 0.f}, result->location.position);
    glm_vec3_copy((vec3){0.f, 0.f, 0.f}, result->location.rotation);
    glm_vec3_copy((vec3){1.f, 1.f, 1.f}, result->location.scale);
    result->id = entity_buffer.entities_idx - 1;
    return result;
}

static entity_t *entity_make_positioned(vec3 position, vec3 rotation, vec3 scale) {
    entity_t *result = entity_make();
    glm_vec3_copy(position, result->location.position);
    glm_vec3_copy(rotation, result->location.rotation);
    glm_vec3_copy(scale, result->location.scale);
    
    const r32 scale_len =glm_vec3_norm(result->location.scale);
    c_assert(scale_len > 0.f); // setting entity scale to negative
    return result;
}

static component_t *entity_push_component(entity_t *entity, component_type type) {
    c_assert(type > COMPONENT_TYPE_NONE && type < COMPONENT_TYPE_COUNT);
    
    if(entity->components == NULL) {
        LOG_YELL("initializing new entity [%d] with components count: %d", entity->id, ENTITY_INITIAL_COMPONENT_COUNT);
        entity->components = (component_t**)c_alloc(sizeof(component_t*) * ENTITY_INITIAL_COMPONENT_COUNT);
        entity->components_count = ENTITY_INITIAL_COMPONENT_COUNT;
        entity->components_idx = 0;
    }
    
    if(entity->components_idx >= entity->components_count) {
        LOG_TELL("reallocating entity [%d] with components count %d", entity->id, entity->components_count * 2);
        entity->components_count *= 2;
        entity->components = (component_t**)realloc(entity->components, sizeof(component_t*) * entity->components_count);
    }
    
    component_t *comp = component_make(type, entity->id);
    entity->components[entity->components_idx] = comp;
    return comp;
}

static void entity_pop_component(entity_t *entity) {
    entity->components_count = c_max(entity->components_count - 1, 0);
}

static void entity_remove_component_by_type(entity_t *entity, component_type type, s32 remove_count) {
    c_assert(remove_count > 0);
    
    u32 count = 0;
    while(count < remove_count) {
        if(count >= entity->components_count) {
            LOG_YELL_COND(count > entity->components_count, "tried to remove %d number of components from an entity that only had %d", remove_count, entity->components_count);
            break;
        }
        
        component_t *comp = entity->components[count];
        if(comp->type == type) {
            memmove(entity->components[count], entity->components[count + 1], sizeof(component_t) * entity->components_count - count);
            --entity->components_count;
        }
        ++count;
    }
}

static void entity_remove_component(entity_t *entity, component_t *comp) {
    for(s32 i = 0; i < entity->components_count; ++i) {
        if(entity->components[i] == comp) {
            if(i + 1 == entity->components_count) {
                --entity->components_count;
                break;
            } else {
                memmove(entity->components[i], entity->components[i + 1], sizeof(component_t) * entity->components_count - i);
                --entity->components_count;
                break;
            }
        }
    }
}



#include "cobble_component.c"