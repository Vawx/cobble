/* cobble_input.h : date = November 8th 2024 11:43 am */

#if !defined(COBBLE_INPUT_H)

const char *debug_sapp_eventtypes[SAPP_EVENTTYPE_CHAR] = {
    "INVALID",
    "KEY_DOWN",
    "KEY_UP",
    "KEY_HELD"
};

typedef struct {
    sapp_keycode keycode;
    sapp_event_type type;
} cobble_input_event;

#define MAX_ACTIVE_INPUT_EVENTS 8
typedef struct {
    vec2 mouse_pos;
    vec2 mouse_pos_delta;
    
    cobble_input_event events[MAX_ACTIVE_INPUT_EVENTS];
} cobble_input;

static cobble_input input;

static cobble_input_event *get_input_event(sapp_keycode code) {
    for(s32 i = 0; i < MAX_ACTIVE_INPUT_EVENTS; ++i) {
        if(input.events[i].keycode == code) {
            return &input.events[i];
        }
    }
    return NULL;
}

static u8 is_key_free(sapp_keycode code) {
    cobble_input_event *e = get_input_event(code);
    if(e) {
        return 0;
    }
    return 1;
}

static u8 is_key_pressed(sapp_keycode code) {
    cobble_input_event *e = get_input_event(code);
    if(e) {
        return e->type == SAPP_EVENTTYPE_KEY_DOWN ? 1 : 0;
    }
    return 0;
    
}

static u8 is_key_released(sapp_keycode code) {
    cobble_input_event *e = get_input_event(code);
    if(e) {
        return e->type == SAPP_EVENTTYPE_KEY_UP ? 1 : 0;
    }
    return 0;
}

static u8 is_key_held(sapp_keycode code) {
    cobble_input_event *e = get_input_event(code);
    if(e) {
        return e->type == SAPP_EVENTTYPE_KEY_HELD ? 1 : 0;
    }
    return 0;
}

static void input_frame() {
    for(s32 i = 0; i < MAX_ACTIVE_INPUT_EVENTS; ++i) {
        if(input.events[i].type == SAPP_EVENTTYPE_KEY_UP) {
            input.events[i].type = SAPP_EVENTTYPE_INVALID;
            input.events[i].keycode = SAPP_KEYCODE_INVALID;
        }
        if(input.events[i].type == SAPP_EVENTTYPE_KEY_DOWN) {
            input.events[i].type = SAPP_EVENTTYPE_KEY_HELD;
        }
    }
}

static void push_input_event(sapp_event *ptr) {
    switch(ptr->type) {
        case SAPP_EVENTTYPE_KEY_DOWN: {
            if(is_key_free(ptr->key_code)) {
                for(s32 i = 0; i < MAX_ACTIVE_INPUT_EVENTS; ++i) {
                    if(input.events[i].keycode == SAPP_KEYCODE_INVALID) {
                        input.events[i].keycode = ptr->key_code;
                        input.events[i].type = ptr->type;
                        break;
                    }
                }
            }
        } break;
        case SAPP_EVENTTYPE_KEY_UP: {
            if(!is_key_free(ptr->key_code)) {
                cobble_input_event *e = get_input_event(ptr->key_code);
                c_assert(e != NULL);
                e->type = SAPP_EVENTTYPE_KEY_UP;
            } else {
                printf("trying to release key that is not pressed %d\n", ptr->key_code);
            }
        } break;
        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            vec2 last_mouse_pos = {0};
            glm_vec2_copy(input.mouse_pos, last_mouse_pos);
            input.mouse_pos[0] = ptr->mouse_x;
            input.mouse_pos[1] = ptr->mouse_y;
            
            input.mouse_pos_delta[0] = input.mouse_pos[0] - last_mouse_pos[0];
            input.mouse_pos_delta[1] = input.mouse_pos[1] - last_mouse_pos[1];
        } break;
    }
}

void input_callback(const sapp_event *event) {
    if(event->key_code != SAPP_KEYCODE_INVALID) {
        push_input_event((sapp_event*)event);
    }
    core_input_callback((sapp_event*)event);
}

#define COBBLE_INPUT_H
#endif //COBBLE_INPUT_H
