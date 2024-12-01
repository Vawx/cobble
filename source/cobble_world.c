
static world_t world;

static entity_t *world_make_entity() {
    entity_t *result = &world.entities.ptr[world.entities.idx];
    c_assert(result->id == -1);
    result->id = world.entities.idx;
    ++world.entities.idx;
    return result;
}

static void world_mesh_system(const component_t *components, const u32 count) {
    
}

static void world_physics_system(const component_t *components, const u32 count) {
    
}

static void world_init_component_mesh(component_t *comp, const dir_t *directory) {
    c_assert(comp->type == COMPONENT_TYPE_MESH);
    c_assert(directory->ptr != NULL && directory->len > 0);
    
    
}

static void world_init_component_physics(component_t *comp) {
    c_assert(comp->type == COMPONENT_TYPE_PHYSICS);
    
}

static component_t *world_make_component(entity_t *entity, component_type type) {
    c_assert(type >= COMPONENT_TYPE_MESH && type < COMPONENT_TYPE_COUNT);
    c_assert(entity != NULL && entity->id >= 0);
    component_t *comp = &world.system_components[type].components[world.system_components[type].idx++]; // increment here
    
    comp->entity_id = entity->id;
    comp->type = type;
    return comp;
}

static void world_init() {
    u8 *component_linear_buffer = c_alloc(sizeof(component_t) * (COMPONENT_INITIAL_COUNT * COMPONENT_TYPE_COUNT)); // initial size _per_ type
    
    for(s32 i = 0; i < COMPONENT_TYPE_COUNT; ++i) {
        world.system_components[i].type = (component_type)i;
        world.system_components[i].components = (component_t*)component_linear_buffer;
        world.system_components[i].idx = 0;
        world.system_components[i].count = COMPONENT_INITIAL_COUNT;
        
        component_linear_buffer += COMPONENT_INITIAL_COUNT;
    }
    
    world.entities.ptr = (entity_t*)c_alloc(sizeof(entity_t) * ENTITY_INITIAL_COUNT);
    for(s32 i = 0; i < ENTITY_INITIAL_COUNT; ++i) { // not my favorite thing to do here
        world.entities.ptr[i].id = -1;
    }
    world.entities.count = ENTITY_INITIAL_COUNT;
    world.entities.idx = 0;
    
    world.system[COMPONENT_TYPE_MESH] = world_mesh_system;
    world.system[COMPONENT_TYPE_PHYSICS]= world_physics_system;
    
    ufbx_init();
    gfx_init();
}

static void world_frame() {
    for(s32 i = 0; i < COMPONENT_TYPE_COUNT; ++i) {
        world.system[i](world.system_components[i].components, world.system_components[i].idx);
    }
}

static void world_shutdown() {
    
}

