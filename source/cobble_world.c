
static world_t world;

static entity_t *world_make_entity() {
    entity_t *result = &world.entities.ptr[world.entities.idx];
    c_assert(result->id == -1);
    glm_vec3_copy((vec3){0.f, 0.f, 0.f}, result->position);
    glm_vec3_copy((vec3){0.f, 0.f, 0.f}, result->rotation);
    glm_vec3_copy((vec3){1.f, 1.f, 1.f}, result->scale);
    result->id = world.entities.idx;
    LOG_TELL("making entity %d", world.entities.idx);
    ++world.entities.idx;
    return result;
}

static void world_mesh_system(const component_t *components, const u32 count) {
    for(s32 i = 0; i < world.system_components[COMPONENT_TYPE_MESH].idx; ++i) {
        component_t *comp = &world.system_components[COMPONENT_TYPE_MESH].ptr[i];
        c_assert(comp->ident.type == COMPONENT_TYPE_MESH);
        
        c_assert(comp->ident.entity_id >= 0 && comp->ident.entity_id < world.entities.idx);
        entity_t *e = &world.entities.ptr[comp->ident.entity_id];
        
        glm_mat4_identity(e->mat);
        
        MAT4(pos);
        MAT4(rot);
        MAT4(scale);
        
        glm_translated_to(e->mat, e->position, pos);
        glm_rotated_x(e->mat, e->rotation[0], rot);
        glm_rotated_y(e->mat, e->rotation[1], rot);
        glm_rotated_z(e->mat, e->rotation[2], rot);
        glm_scale_to(e->mat, e->scale, scale);
        
        MAT4(pr);
        glm_mat4_mul(pos, rot, pr);
        glm_mat4_mul(pr, scale, e->mat);
        
        gfx_render_mesh(&comp->mesh.gfx_handle, e->mat);
    }
}

static void world_physics_system(const component_t *components, const u32 count) {
    
}

static void world_init_component_mesh(component_t *comp, const dir_t *directory) {
    c_assert(comp->ident.type == COMPONENT_TYPE_MESH);
    c_assert(directory->ptr != NULL && directory->len > 0);
    
    comp->mesh.gfx_handle = gfx_load_model_fbx(directory);
    
    
    LOG_TELL("initializing mesh %s for component %d. entity id: %d", directory->ptr, comp->ident.id, comp->ident.entity_id);
}

static void world_init_component_physics(component_t *comp) {
    c_assert(comp->ident.type == COMPONENT_TYPE_PHYSICS);
    
}

static void world_set_entity_pos(entity_t *e, vec3 pos) {
    glm_vec3_copy(pos, e->position);
}

static void world_set_entity_rot(entity_t *e, vec3 rot) {
    glm_vec3_copy(rot, e->rotation);
}

static void world_set_entity_scale(entity_t *e, vec3 scale) {
    glm_vec3_copy(scale, e->scale);
}

static component_t *world_make_component(entity_t *entity, component_type type) {
    c_assert(type >= COMPONENT_TYPE_MESH && type < COMPONENT_TYPE_COUNT);
    c_assert(entity != NULL && entity->id >= 0);
    component_t *comp = &world.system_components[type].ptr[world.system_components[type].idx++]; // increment here
    
    comp->ident.entity_id = entity->id;
    comp->ident.id = world.system_components[type].idx - 1;
    comp->ident.type = type;
    
    LOG_TELL("making component %s for entity %d", component_type_str[type], entity->id);
    return comp;
}

static void world_init() {
    u8 *component_linear_buffer = c_alloc(sizeof(component_t) * (COMPONENT_INITIAL_COUNT * COMPONENT_TYPE_COUNT)); // initial size _per_ type
    
    for(s32 i = 0; i < COMPONENT_TYPE_COUNT; ++i) {
        world.system_components[i].type = (component_type)i;
        world.system_components[i].ptr = (component_t*)component_linear_buffer;
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
    
    LOG_TELL("world initialized");
}

static void world_frame() {
    for(s32 i = 0; i < COMPONENT_TYPE_COUNT; ++i) {
        world.system[i](world.system_components[i].ptr, world.system_components[i].idx);
    }
    
    gfx_frame();
}

static void world_shutdown() {
    
}

