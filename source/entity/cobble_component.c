
static component_buffer_t component_buffer;

static void component_init() {
    c_assert(component_buffer.components == NULL);
    
    component_buffer.components = (component_t*)c_alloc(sizeof(component_t) * COMPONENT_DEFAULT_COUNT);
    component_buffer.count = COMPONENT_DEFAULT_COUNT;
    component_buffer.components_idx = 0;
}

static void component_handle_mesh(component_t *comp) {
    c_assert(comp->type == COMPONENT_TYPE_MESH);
    
}

static void component_frame() {
    for(s32 i = 0; i < component_buffer.components_idx; ++i) {
        component_t *comp = &component_buffer.components[i];
        switch(comp->type) {
            case COMPONENT_TYPE_MESH: {
                
            } break;
        }
    }
}

static void component_shutdown() {
    c_free((u8*)component_buffer.components);
    
    component_buffer.components = NULL;
    component_buffer.components_idx = 0;
}

static void component_init_mesh(component_t *comp) {
    c_assert(comp->type == COMPONENT_TYPE_MESH);
    comp->mesh.handle.view_id = -1;
    comp->mesh.handle.mesh_id = -1;
}

static void component_mesh_assign(component_t *comp, dir_t *dir) {
    c_assert(dir->len > 0 && comp-> type == COMPONENT_TYPE_MESH);
    LOG_TELL("assigning mesh %s to component", dir->ptr);
    
    comp->mesh.handle = gfx_make_model_handle(dir);
    
    c_assert(comp->mesh.handle.view_id >= 0);
    c_assert(comp->mesh.handle.mesh_id >= 0);
}

static component_t *component_make(component_type type, s32 entity_id) {
    c_assert(component_buffer.components != NULL);
    if(component_buffer.components_idx >= component_buffer.count) {
        LOG_YELL("increasing component buffer size from %d to %d", component_buffer.count, component_buffer.count * 2);
        component_buffer.count *= 2;
        component_buffer.components = (component_t*)realloc(component_buffer.components, component_buffer.count);
    }
    
    component_t *result = &component_buffer.components[component_buffer.components_idx++]; // increment buffer count here
    result->type = type;
    result->entity_id = entity_id;
    switch(type) {
        case COMPONENT_TYPE_MESH: {
            component_init_mesh(result);
        } break;
    }
    return result;
}
