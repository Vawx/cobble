@ctype vec3 vec3
@ctype vec4 vec4
@ctype mat4 mat4

@block vertex_shared

layout(binding=0) uniform mesh_vertex_ubo {
    mat4 geometry_to_world;
    mat4 normal_to_world;
    mat4 world_to_clip;
    vec4 blend_weights[16];
    float f_num_blend_shapes;
};

layout(binding=0) uniform sampler blend_shapes_sampler;
layout(binding=0) uniform texture2D blend_shapes_texture;

vec3 evaluate_blend_shape(int vertex_index)
{
    ivec2 coord = ivec2(vertex_index & (2048 - 1), vertex_index >> 11);
    int num_blend_shapes = int(f_num_blend_shapes);
    vec3 offset = vec3(0.0);
    for (int i = 0; i < num_blend_shapes; i++) {
        vec4 packed = blend_weights[i >> 2];
        float weight = packed[i & 3];
        //offset += weight * texelFetch(blend_shapes_sampler, coord, 0).xyz;
    }
    return offset;
}

@end

@vs static_vertex

@include_block vertex_shared

layout(location=0) in vec3 a_position;
layout(location=1) in vec3 a_normal;
layout(location=2) in vec2 a_uv;
layout(location=3) in float a_vertex_index;

out vec3 v_normal;
out vec2 v_uv;

void main()
{
    vec3 local_pos = a_position;
    local_pos += evaluate_blend_shape(int(a_vertex_index));

    vec3 world_pos = (geometry_to_world * vec4(local_pos, 1.0)).xyz;
    gl_Position = world_to_clip * vec4(world_pos, 1.0);
    v_normal = normalize((normal_to_world * vec4(a_normal, 0.0)).xyz);
    v_uv = a_uv;
}

@end

@vs skinned_vertex

@include_block vertex_shared

layout(binding=1) uniform skin_vertex_ubo {
    mat4 bones[64];
};

layout(location=0) in vec3 a_position;
layout(location=1) in vec3 a_normal;
layout(location=2) in vec2 a_uv;
layout(location=3) in float a_vertex_index;
#if SOKOL_GLSL
    layout(location=4) in vec4 a_bone_indices;
#else
    layout(location=4) in ivec4 a_bone_indices;
#endif
layout(location=5) in vec4 a_bone_weights;

out vec3 v_normal;
out vec2 v_uv;

void main()
{
    mat4 bind_to_world
        = bones[int(a_bone_indices.x)] * a_bone_weights.x
        + bones[int(a_bone_indices.y)] * a_bone_weights.y
        + bones[int(a_bone_indices.z)] * a_bone_weights.z
        + bones[int(a_bone_indices.w)] * a_bone_weights.w;

    vec3 local_pos = a_position;
    local_pos += evaluate_blend_shape(int(a_vertex_index));
    vec3 world_pos = (bind_to_world * vec4(local_pos, 1.0)).xyz;
    vec3 world_normal = (bind_to_world * vec4(a_normal, 0.0)).xyz;

    gl_Position = world_to_clip * vec4(world_pos, 1.0);
    v_normal = normalize(world_normal);
    v_uv = a_uv;
}

@end

@fs lit_pixel

layout(binding=2) uniform uv_tiling_ubo {
	float tile_x;
	float tile_y;
};

layout(binding=0) uniform sampler diffuse_sampler;
layout(binding=0) uniform texture2D diffuse_texture;

in vec3 v_normal;
in vec2 v_uv;

out vec4 o_color;

void main()
{
	vec2 tiling = vec2(max(1.0, tile_x) * v_uv.x, max(1.0, tile_y) * v_uv.y);
    o_color = texture(sampler2D(diffuse_texture, diffuse_sampler), tiling)  * vec4(1.0, 1.0, 1.0, 1.0);
}
@end

@program static_lit static_vertex lit_pixel
@program skinned_lit skinned_vertex lit_pixel