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

typedef struct {
    u32 vertices_size;
    u32 indices_size;
    u8 *vertices;
    u8 *indices;
} asset_mesh_instance;

typedef struct {
    u32 instances_count;
    asset_mesh_instance *instances;
} asset_mesh;

typedef struct {
    u32 w;
    u32 h;
    u32 channels;
    u32 desired_channels;
    u8 *ptr;
} asset_texture;

typedef struct asset {
    asset_type type;
    u64 file_hash;
    union {
        asset_mesh mesh;
        asset_texture texture;
    };
} asset;

static asset asset_make(const cobble_dir *dir);
static asset asset_load(const cobble_dir *dir);

#define COBBLE_SERIALIZE_H
#endif //COBBLE_SERIALIZE_H