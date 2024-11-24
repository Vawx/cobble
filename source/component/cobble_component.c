
static entity_buffer_t entity_buffer;

static void entity_init() {
    entity_buffer.entities = (entity_t*)c_alloc(ENTITY_START_COUNT * sizeof(entity_t));
    entity_buffer.entities_idx = 0;
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
