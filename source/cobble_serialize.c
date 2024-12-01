static asset_buffer_t assets;

static void asset_init() {
    assets.ptr = (asset_t*)c_alloc(sizeof(asset_t) * ASSET_BUFFER_INIT_COUNT);
    assets.idx = 0;
    assets.count = ASSET_BUFFER_INIT_COUNT;
}

static asset_t *asset_get_empty() {
    if(assets.idx + 1 >= assets.count) {
        assets.ptr = (asset_t*)realloc(assets.ptr, sizeof(asset_t) * assets.count * 2);
        assets.count *= 2;
        
        LOG_YELL("assets are being resized from count: %d to: %d", assets.count / 2, assets.count);
    }
    
    asset_t *asset = &assets.ptr[assets.idx++]; // increment asset here
    return asset;
}

static asset_texture_t asset_generate_texture(const dir_t *dir) {
    c_assert(dir->ptr != NULL);
    asset_texture_t result = {0};
    
    return result;
}

static asset_mesh_t asset_generate_mesh(const dir_t *dir) {
    c_assert(dir->ptr != NULL);
    asset_mesh_t result = {0};
    
    return result;
}

static asset_type asset_type_from_path(const dir_t *dir) {
    for(s32 i = 0; i < SUBDIRS_COUNT; ++i) {
        char *ptr = strstr(dir->ptr, subdir_str[i]);
        if(ptr && (subdirs)i != NONE_SUBDIR) {
            return (asset_type)i;
        }
    }
    return ASSET_TYPE_NONE;
}

static asset_t *asset_make(const dir_t *dir) {
    // TODO(Kyle): make sure this asset doesnt already exist.
    // if it does, notify and ask to replace/override it.
    
    LOG_TELL("making new asset %s", dir->ptr);
    
    asset_t *asset = asset_get_empty();
    
    asset->type = asset_type_from_path(dir);
    c_assert(asset->type != ASSET_TYPE_NONE);
    
    switch(asset->type) {
        case ASSET_TYPE_TEXTURE: {
            asset->texture = asset_generate_texture(dir);
        } break;
        case ASSET_TYPE_MESH: {
            asset->mesh = asset_generate_mesh(dir);
        } break;
        case ASSET_TYPE_AUDIO: {
            
        } break;
        case ASSET_TYPE_SHADER_SRC: {
            
        } break;
        case ASSET_TYPE_SHADER_GEN: {
            
        } break;
    }
    
    asset->file_hash = wgpu_hash(dir->ptr, dir->len);
    return asset;
}

static bool asset_save(const dir_t *dir, asset_t *in_asset) {
    c_assert(dir->ptr != NULL);
    
    FILE *f = fopen(dir->ptr, "wb");
    if(f) {
        fwrite(&in_asset->type, sizeof(asset_type), 1, f);
        fwrite(&in_asset->file_hash, sizeof(u64), 1, f);
        
        switch(in_asset->type) {
            case ASSET_TYPE_TEXTURE: {
                fwrite(&in_asset->texture.w, sizeof(u32), 1, f);
                fwrite(&in_asset->texture.h, sizeof(u32), 1, f);
                fwrite(&in_asset->texture.channels, sizeof(u32), 1, f);
                fwrite(&in_asset->texture.desired_channels, sizeof(u32), 1, f);
                fwrite(in_asset->texture.ptr, sizeof(u8), in_asset->texture.w * in_asset->texture.h * in_asset->texture.desired_channels, f);
            } break;
            case ASSET_TYPE_MESH: {
                fwrite(&in_asset->mesh.instances_count, sizeof(u32), 1, f);
                
                for(s32 i = 0; i < in_asset->mesh.instances_count; ++i) {
                    asset_mesh_instance_t *instance = &in_asset->mesh.instances[i];
                    fwrite(&instance->vertices_size, sizeof(u32), 1, f);
                    fwrite(&instance->indices_size, sizeof(u32), 1, f);
                    fwrite(instance->vertices, sizeof(u8), instance->vertices_size, f);
                    fwrite(instance->indices, sizeof(u8), instance->indices_size, f);
                }
            } break;
            case ASSET_TYPE_AUDIO: {
                
            } break;
            case ASSET_TYPE_SHADER_SRC: {
                
            } break;
            case ASSET_TYPE_SHADER_GEN: {
                
            } break;
        }
        fclose(f);
        return true;
    }
    return false;
}

#define ASSIGN_AND_INCREMENT_BY_TYPE_SIZE(p, v, t) (v) = *(t*)p; p += sizeof(v)
#define ASSIGN_AND_INCREMENT_BY_PTR_SIZE(p, v, t, s) (v) = (t*)p; p += s

static asset_t *asset_load(const dir_t *dir) {
    c_assert(dir->ptr != NULL);
    
    u64 file_hash = wgpu_hash(dir->ptr, dir->len);
    for(s32 i = 0; i < assets.idx; ++i) {
        if(file_hash == assets.ptr[i].file_hash) {
            LOG_TELL("returning already loaded asset %s", dir->ptr);
            return &assets.ptr[i];
        }
    }
    
    asset_t *asset = asset_get_empty();
    
    FILE *f = fopen(dir->ptr, "rb");
    c_assert(f != NULL);
    
    fseek(f, 0, SEEK_END);
    u32 size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    u8 *ptr = (u8*)c_alloc(size);
    fread(ptr, size, 1, f);
    u8 *ptr_cpy = ptr;
    
    ASSIGN_AND_INCREMENT_BY_TYPE_SIZE(ptr_cpy, asset->type, asset_type);
    ASSIGN_AND_INCREMENT_BY_TYPE_SIZE(ptr_cpy, asset->file_hash, u64);
    
    switch(asset->type) {
        case ASSET_TYPE_TEXTURE: {
            ASSIGN_AND_INCREMENT_BY_TYPE_SIZE(ptr_cpy, asset->texture.w, u32);
            ASSIGN_AND_INCREMENT_BY_TYPE_SIZE(ptr_cpy, asset->texture.h, u32);
            ASSIGN_AND_INCREMENT_BY_TYPE_SIZE(ptr_cpy, asset->texture.channels, u32);
            ASSIGN_AND_INCREMENT_BY_TYPE_SIZE(ptr_cpy, asset->texture.desired_channels, u32);
            ASSIGN_AND_INCREMENT_BY_PTR_SIZE(ptr_cpy, asset->texture.ptr, u8, asset->texture.w * asset->texture.h * asset->texture.channels);
        } break;
        case ASSET_TYPE_MESH: {
            ASSIGN_AND_INCREMENT_BY_TYPE_SIZE(ptr_cpy, asset->mesh.instances_count, u32);
            asset->mesh.instances = (asset_mesh_instance_t*)c_alloc(sizeof(asset_mesh_instance_t) * asset->mesh.instances_count);
            for(s32 i = 0; i < asset->mesh.instances_count; ++i) {
                asset_mesh_instance_t *instance = &asset->mesh.instances[i];
                
                ASSIGN_AND_INCREMENT_BY_TYPE_SIZE(ptr_cpy, instance->vertices_size, u32);
                ASSIGN_AND_INCREMENT_BY_TYPE_SIZE(ptr_cpy, instance->indices_size, u32);
                ASSIGN_AND_INCREMENT_BY_PTR_SIZE(ptr_cpy, instance->vertices, u8, instance->vertices_size);
                ASSIGN_AND_INCREMENT_BY_PTR_SIZE(ptr_cpy, instance->indices, u8, instance->indices_size);
            }
        } break;
        case ASSET_TYPE_AUDIO: {
            
        } break;
        case ASSET_TYPE_SHADER_SRC: {
            
        } break;
        case ASSET_TYPE_SHADER_GEN: {
            
        } break;
    }
    
    fclose(f);
    return asset;
}
