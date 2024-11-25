
static entity_buffer_t entity_buffer;

static void entity_init() {
    entity_buffer.entities = (entity_t*)c_alloc(ENTITY_START_COUNT * sizeof(entity_t));
    entity_buffer.entities_idx = 0;
}

static void entity_frame() {
    for(s32 i = 0; i < entity_buffer.entities_idx; ++i) {
        entity_t *e = &entity_buffer.entities[i];
        for(s32 j = 0; j < e->components_idx; ++j) {
            component_t *compo = &e->components[j];
            
            
        }
    }
}

static void entity_shutdown() {
    c_free((u8*)entity_buffer.entities);
}

static entity_t *entity_make() {
    c_assert_msg(entity_buffer.entities == NULL, "entities have not been initialized");
    
    entity_t *result = &entity_buffer.entities[entity_buffer.entities_idx++]; // increment entities here
    return result;
}

static void entity_store(entity_t *e) {
    c_assert_msg(e != NULL, "trying to store a NULL entity");
    
}

static void entity_delete(entity_t *e) {
    c_assert_msg(e != NULL, "trying to delete a NULL entity");
    
}

static void entity_push_component(entity_t *e, component_t *comp) {
    c_assert(e->components_idx < ENTITY_COMPONENT_MAX);
}

static void entity_pop_component(entity_t *e) {
    c_assert_msg(e->components_idx > 0, "trying to pop component from already empty entity");
    --e->components_idx;
}

static void entity_remove_component(entity_t *e, component_t *comp) {
    c_assert(e->components_idx < ENTITY_COMPONENT_MAX);
    memcpy(&e->components[e->components_idx++], comp, sizeof(component_t));
}

