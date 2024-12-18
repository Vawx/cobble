@ctype mat4 mat4

@block util

vec4 encode_depth(float v) {
    vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
    enc = fract(enc);
    enc -= enc.yzww * vec4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);
    return enc;
}

float decodeDepth(vec4 rgba) {
    return dot(rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0));
}

float sample_shadow(texture2D tex, sampler smp, vec2 uv, float compare) {
    float depth = decodeDepth(texture(sampler2D(tex, smp), vec2(uv.x, uv.y)));
    return step(compare, depth);
}

float sample_shadow_pcf(texture2D tex, sampler smp, vec3 uv_depth, vec2 sm_size) {
    float result = 0.0;
    for (int x = -2; x <= 2; x++) {
        for (int y =- 2; y <= 2; y++) {
            vec2 offset = vec2(x, y) / sm_size;
            result += sample_shadow(tex, smp, uv_depth.xy + offset, uv_depth.z);
        }
    }
    return result / 25.0;
}

@end

@vs vs
layout(binding=0) uniform vs_display_params {
    mat4 mvp;
    mat4 model;
    mat4 light_mvp;
    vec3 diff_color;
};
in vec4 pos;
in vec3 norm;
in vec2 texcoord0;

out vec2 uv;
out vec3 normal;
out vec3 color;
out vec4 light_proj_pos;
out vec4 world_pos;
out vec3 world_norm;

void main() {
 gl_Position = mvp * pos;
    light_proj_pos = light_mvp * pos;
    #if !SOKOL_GLSL
        light_proj_pos.y = -light_proj_pos.y;
    #endif
    world_pos = model * pos;
    world_norm = normalize((model * vec4(norm, 0.0)).xyz);
    color = diff_color;
    uv = texcoord0;
	normal = norm;
}
@end

@fs fs
@include_block util

layout(binding=1) uniform fs_display_params {
    vec3 light_dir;
    vec3 eye_pos;
};

layout(binding=2) uniform texture2D shadow_map;
layout(binding=2) uniform sampler shadow_sampler;
layout(binding=3) uniform texture2D diffuse_map;
layout(binding=3) uniform sampler diffuse_sampler;

in vec2 uv;
in vec3 normal;
in vec3 color;
in vec4 light_proj_pos;
in vec4 world_pos;
in vec3 world_norm;

out vec4 frag_color;

vec4 gamma(vec4 c) {
    float p = 1.0 / 2.2;
    return vec4(pow(c.xyz, vec3(p)), c.w);
}

vec4 diffuse_color() {
	return texture(sampler2D(diffuse_map, diffuse_sampler), uv) * vec4(1.0, 1.0, 1.0, 1.0);
}

void main() {
	vec2 sm_size = textureSize(sampler2D(shadow_map, shadow_sampler), 0);
    float spec_power = 2.2;
    float ambient_intensity = 0.25;
    vec3 l = normalize(light_dir);
    vec3 n = normalize(world_norm);
    float n_dot_l = dot(n, l);
    if (n_dot_l > 0.0) {

        vec3 light_pos = light_proj_pos.xyz / light_proj_pos.w;
        vec3 sm_pos = vec3((light_pos.xy + 1.0) * 0.5, light_pos.z);
        float s = sample_shadow_pcf(shadow_map, shadow_sampler, sm_pos, sm_size);
        float diff_intensity = max(n_dot_l * s, 0.0);

        vec3 v = normalize(eye_pos - world_pos.xyz);
        vec3 r = reflect(-l, n);
        float r_dot_v = max(dot(r, v), 0.0);
        float spec_intensity = pow(r_dot_v, spec_power) * n_dot_l * s;

        frag_color = diffuse_color() * vec4(vec3(spec_intensity) + (diff_intensity + ambient_intensity) * color, 1.0);
    } else {
        frag_color = diffuse_color() * vec4(color * ambient_intensity, 1.0);
    }
    frag_color = diffuse_color() * gamma(frag_color);
}
@end

@program color_pass vs fs