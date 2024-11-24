/* cobble_serialize.h : date = November 21st 2024 1:57 pm */

#if !defined(COBBLE_SERIALIZE_H)

typedef enum asset_type {
    ASSET_TYPE_NONE,
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_MESH,
    ASSET_TYPE_AUDIO,
    ASSET_TYPE_SHADER_SRC,
    ASSET_TYPE_SHADER_GEN,
} asset_type;

typedef struct asset_mesh_instance_t {
    u32 vertices_size;
    u32 indices_size;
    u8 *vertices;
    u8 *indices;
} asset_mesh_instance_t;

typedef struct asset_mesh_t {
    u32 instances_count;
    asset_mesh_instance_t *instances;
} asset_mesh_t;

typedef struct asset_texture_t {
    u32 w;
    u32 h;
    u32 channels;
    u32 desired_channels;
    u8 *ptr;
} asset_texture_t;

typedef struct asset_t {
    asset_type type;
    u64 file_hash;
    union {
        asset_mesh_t mesh;
        asset_texture_t texture;
    };
} asset_t;

static asset_t asset_make(const dir_t *dir);
static asset_t asset_load(const dir_t *dir);

#define COBBLE_SERIALIZE_H
#endif //COBBLE_SERIALIZE_H