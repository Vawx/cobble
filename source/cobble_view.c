
static view_t views[MAX_VIEWS];
static u8 views_idx = 0;

static view_t *view_current = NULL;

static view_t *get_current_view() {
    return view_current;
}

static view_t *view_make_new(vec3 pos, vec3 dir, r32 yaw, r32 pitch, u8 input_commanded) {
    view_t *result = &views[views_idx];
    glm_vec3_copy(pos, result->pos);
    glm_vec3_copy(dir, result->dir);
    result->yaw = yaw;
    result->pitch = pitch;
    glm_mat4_identity(result->projection);
    glm_mat4_identity(result->view);
    result->id = views_idx;
    result->is_input_commanded = input_commanded;
    result->fov = VIEW_DEFAULT_FOV;
    ++views_idx;
    return result;
}

static void view_set_current(view_t *view) {
    if(view_current != view) {
        view_current = view;
    }
}

static void view_set_current_idx(u8 idx) {
    c_assert(idx < views_idx);
    view_current = &views[idx];
}

static view_t *view_get(u8 idx) {
    c_assert(idx < views_idx);
    return &views[idx];
}

static void view_update(view_t *view, vec2 screen_size) {
    vec3 front;
    front[0] = cos(c_rad(view->yaw)) * cos(c_rad(view->pitch));
    front[1] = sin(c_rad(view->pitch));
    front[2] = sin(c_rad(view->yaw)) * cos(c_rad(view->pitch));
    glm_vec3_normalize(front);
    glm_vec3_copy(front, view->dir);
    
    glm_vec3_cross(front, (vec3){0.f, 1.f, 0.f}, view->right);
    glm_vec3_cross(view->right, front, view->up);
    glm_vec3_normalize(view->right);
    glm_vec3_normalize(view->up);
    
    glm_perspective(view->fov, screen_size[0] / screen_size[1], 0.01f, 1000.f, view->projection);
    
    vec3 forward_pos = {0};
    glm_vec3_add(view->pos, view->dir, forward_pos);
    glm_lookat(view->pos, forward_pos, (vec3){0.f, 1.f, 0.f}, view->view);
}

static void view_frame(vec2 screen_size, mat4 *out_view_projection) {
    if(view_current && view_current->is_input_commanded) {
        view_update(view_current, screen_size);
        
        vec3 current_pos = {0};
        glm_vec3_copy(view_current->pos, current_pos);
        vec3 current_dir = {0};
        glm_vec3_copy(view_current->dir, current_dir);
        
        if(is_key_pressed(SAPP_KEYCODE_W) || is_key_held(SAPP_KEYCODE_W)) {
            vec3 new_pos_scaled = {0};
            glm_vec3_scale(current_dir, VIEW_MOVE_SPEED * 0.033f, new_pos_scaled);
            glm_vec3_add(current_pos, new_pos_scaled, view_current->pos);
        }
        
        if(is_key_pressed(SAPP_KEYCODE_S) || is_key_held(SAPP_KEYCODE_S)) {
            vec3 new_pos_scaled = {0};
            glm_vec3_scale(current_dir, VIEW_MOVE_SPEED * 0.033f, new_pos_scaled);
            glm_vec3_sub(current_pos, new_pos_scaled, view_current->pos);
        }
        
        if(is_key_pressed(SAPP_KEYCODE_A) || is_key_held(SAPP_KEYCODE_A)) {
            vec3 new_pos_scaled = {0};
            glm_vec3_scale(view_current->right, VIEW_MOVE_SPEED * 0.033f, new_pos_scaled);
            glm_vec3_sub(current_pos, new_pos_scaled, view_current->pos);
        }
        
        if(is_key_pressed(SAPP_KEYCODE_D) || is_key_held(SAPP_KEYCODE_D)) {
            vec3 new_pos_scaled = {0};
            glm_vec3_scale(view_current->right, VIEW_MOVE_SPEED * 0.033f, new_pos_scaled);
            glm_vec3_add(current_pos, new_pos_scaled, view_current->pos);
        }
        
        if(is_key_pressed(SAPP_KEYCODE_SPACE) || is_key_held(SAPP_KEYCODE_SPACE)) {
            vec3 new_pos_scaled = {0};
            glm_vec3_scale(view_current->up, VIEW_MOVE_SPEED * 0.033f, new_pos_scaled);
            glm_vec3_add(current_pos, new_pos_scaled, view_current->pos);
        }
        
        if(is_key_pressed(SAPP_KEYCODE_LEFT_CONTROL) || is_key_held(SAPP_KEYCODE_LEFT_CONTROL)) {
            vec3 new_pos_scaled = {0};
            glm_vec3_scale(view_current->up, VIEW_MOVE_SPEED * 0.033f, new_pos_scaled);
            glm_vec3_sub(current_pos, new_pos_scaled, view_current->pos);
        }
        
        if(is_key_pressed(SAPP_KEYCODE_RIGHT) || is_key_held(SAPP_KEYCODE_RIGHT)) {
            view_current->yaw += VIEW_ROTATION_SPEED;
        }
        
        if(is_key_pressed(SAPP_KEYCODE_LEFT) || is_key_held(SAPP_KEYCODE_LEFT)) {
            view_current->yaw -= VIEW_ROTATION_SPEED;
        }
        
        if(is_key_pressed(SAPP_KEYCODE_UP) || is_key_held(SAPP_KEYCODE_UP)) {
            view_current->pitch += VIEW_ROTATION_SPEED;
        }
        
        if(is_key_pressed(SAPP_KEYCODE_DOWN) || is_key_held(SAPP_KEYCODE_DOWN)) {
            view_current->pitch -= VIEW_ROTATION_SPEED;
        }
        
        static u32 x = 234324324;
        if(is_key_pressed(SAPP_KEYCODE_F1)) {
            srand(x);
            r32 randx = rand() % 10;
            r32 randy = rand() % 10;
            r32 randz = rand() % 10;
            jolt_dynamic_object obj = {0};
            r32 shape_r = rand() % 100;
            if(shape_r > 50) {
                obj = jolt_make_dynamic_box(V3d(0.5f), V3(10-randx, 10-randy, 10-randz), 1);
            } else {
                obj = jolt_make_dynamic_sphere(1.f, V3(10-randx, 10-randy, 10-randz), 1);
            }
            /*shapes_push_desc(&(render_shape_description) {
                                 .color = V4(1.f, 0.25, 0.3f, 1.f),
                                 .shape_type = shape_r > 50 ? RENDER_SHAPES_BOX : RENDER_SHAPES_SPHERE,
                                 .desc_type = RENDER_SHAPE_DESC_TYPE_COLORED,
                                 .shape_movement = RENDER_SHAPES_MOVEMENT_DYNAMIC,
                                 .id = obj.id
                             });*/
        }
        x += 1;
        // keep view from "flipping"
        if (view_current->pitch > 89.0f) {view_current->pitch = 89.0f;}
        if (view_current->pitch < -89.0f) {view_current->pitch = -89.0f;}
        
        mat4 perspective = {0};
        glm_mat4_copy(view_current->projection, perspective);
        mat4 view = {0};
        glm_mat4_copy(view_current->view, view);
        mat4 pv = {0};
        glm_mat4_mul(perspective, view, pv);
        
        glm_mat4_copy(pv, *out_view_projection);
    }
}
