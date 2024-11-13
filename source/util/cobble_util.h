/* cobble_util.h : date = November 7th 2024 9:55 pm */

#if !defined(COBBLE_UTIL_H)

#include <stdio.h>
#include <string.h>

#define SOKOL_FETCH_IMPL
#include "sokol_fetch.h"

#define clamp(v, min, max) (v) < (min) ? (min) : (v); (v) > (max) ? (max) : (v);

#define DIRECTORY_MAX_LEN 255
typedef struct{
    char ptr[DIRECTORY_MAX_LEN];
    u32 len;
} cobble_dir;

typedef struct {
    u8 *ptr;
    u32 len;
    cobble_dir dir;
} cobble_file;

static cobble_dir root_dir;

static void root_dir_init(const char *ptr) {
    static u8 ran = false;
    if(!ran) {
        char *p = strstr(ptr, "\\build");
        c_assert(p != NULL);
        
        memcpy(root_dir.ptr, ptr, p - ptr);
        for(s32 i  = 0; i < (u32)(p - ptr); ++i) {
            if(root_dir.ptr[i] == '\\') { 
                root_dir.ptr[i] = '/';
            }
        }
        
        root_dir.len = strlen(root_dir.ptr);
        ran = true;
    }
}

static cobble_file load_file(cobble_dir *dir) {
    c_assert(dir->ptr != NULL);
    
    FILE *f = fopen(dir->ptr, "rb");
    c_assert(f != NULL);
    
    cobble_file result = {0};
    fseek(f, 0, SEEK_END);
    result.len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    result.ptr = (u8*)malloc(result.len);
    fread(result.ptr, 1, result.len, f);
    fclose(f);
    
    return result;
}

// expected response func sig: void response_callback(const sfetch_response_t* response)
static void load_file_async(cobble_dir *dir, void *response_func, u8 *ptr, u64 ptr_size) {
    sfetch_send(&(sfetch_request_t) {
                    .path = dir->ptr,
                    .callback = response_func,
                    .buffer = {
                        .ptr = ptr,
                        .size = ptr_size
                    },
                });
}

#define SUBDIR_TEXTURE "/content/texture/"
#define SUBDIR_MESH "/content/mesh/"
#define SUBDIR_AUDIO "/content/audio/"
#define SUBDIR_SHADER_SRC "/content/shader/"
#define SUBDIR_SHADER_GEN "/source/shader/"

static cobble_dir dir_get_for(const char *file_name, const char *subdir) {
    cobble_dir dir = {0};
    char *ptr = dir.ptr;
    u32 len = root_dir.len;
    memcpy(ptr, root_dir.ptr, len);
    ptr += len;
    if(subdir) {
        len = strlen(subdir);
        memcpy(ptr, subdir, len);
        ptr += len;
    }
    len = strlen(file_name);
    memcpy(ptr, file_name, len);
    ptr += len;
    dir.len = ptr - &dir.ptr[0];
    return dir;
}

#if _WIN32
static u8 *c_alloc(u64 size) {
    return (u8*)VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

static void c_free(u8 *ptr) {
    VirtualFree((void*)ptr, 0, MEM_RELEASE);
}
#else
static u8 *c_alloc(u64 size) {
    return (u8*)malloc(size);
}

static void c_free(u8 *ptr) {
    free(ptr);
}
#endif

// math types that are easier to assign and pass around
// conversions to cglm below
typedef union {
    struct {
        r32 x, y;
    };
    r32 elements[2];
} v2;

typedef union {
    union {
        v2 _v2;
        r32 _z;
        struct {
            r32 x, y, z;
        };
    };
    r32 elements[3];
} v3;

typedef union {
    union {
        v3 _v3;
        r32 _w;
        struct {
            r32 x, y, z, w;
        };
    };
    r32 elements[4];
} v4;

static inline v2 V2(r32 x, r32 y) {
    v2 result = {0};
    result.x = x;
    result.y = y;
    return result;
}

static inline v3 V3(r32 x, r32 y, r32 z) {
    v3 result = {0};
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

static inline v3 V3d(r32 d) {
    v3 result = {0};
    result.x = d;
    result.y = d;
    result.z = d;
    return result;
}

static inline v4 V4(r32 x, r32 y, r32 z, r32 w) {
    v4 result = {0};
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}

typedef union {
    v4 columns[4];
    r32 elements[4][4];
} m4;

static m4 M4() {
    m4 result = {0};
    result.elements[0][0] = 1.f;
    result.elements[0][1] = 0;
    result.elements[0][2] = 0;
    result.elements[0][3] = 0;
    
    result.elements[1][0] = 0;
    result.elements[1][1] = 1.f;
    result.elements[1][2] = 0;
    result.elements[1][3] = 0;
    
    result.elements[2][0] = 0;
    result.elements[2][1] = 0;
    result.elements[2][2] = 1.f;
    result.elements[2][3] = 0;
    
    result.elements[3][0] = 0;
    result.elements[3][1] = 0;
    result.elements[3][2] = 0;
    result.elements[3][3] = 1.f;
    return result;
}

static m4 M4d(r32 d) {
    m4 result = {0};
    result.elements[0][0] = d;
    result.elements[0][1] = 0;
    result.elements[0][2] = 0;
    result.elements[0][3] = 0;
    
    result.elements[1][0] = 0;
    result.elements[1][1] = d;
    result.elements[1][2] = 0;
    result.elements[1][3] = 0;
    
    result.elements[2][0] = 0;
    result.elements[2][1] = 0;
    result.elements[2][2] = d;
    result.elements[2][3] = 0;
    
    result.elements[3][0] = 0;
    result.elements[3][1] = 0;
    result.elements[3][2] = 0;
    result.elements[3][3] = d;
    return result;
}

#define vec2_from_v2(in) ((vec2){in.elements[0], in.elements[1]})
#define vec3_from_v3(in) ((vec3){in.elements[0], in.elements[1], in.elements[2]})
#define vec4_from_v4(in) ((vec4){in.elements[0], in.elements[1], in.elements[2], in.elements[3]})

#define jph_vec2_from_v2(in) ((JPH_Vec2){in.elements[0], in.elements[1]})
#define jph_vec3_from_v3(in) ((JPH_Vec3){in.elements[0], in.elements[1], in.elements[2]})
#define jph_vec4_from_v4(in) ((JPH_Vec4){in.elements[0], in.elements[1], in.elements[2], in.elements[3]})

#define COBBLE_UTIL_H
#endif //COBBLE_UTIL_H
