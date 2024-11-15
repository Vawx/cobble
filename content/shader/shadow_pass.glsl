@ctype mat4 mat4
@ctype vec3 vec3
@ctype vec2 vec2

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

//=== shadow pass
@vs vs_shadow

layout(binding=0) uniform vs_shadow_params {
    mat4 mvp;
};

in vec4 pos;
out vec2 proj_zw;

void main() {
    gl_Position = mvp * pos;
    proj_zw = gl_Position.zw;
}
@end

@fs fs_shadow
@include_block util

in vec2 proj_zw;
out vec4 frag_color;

void main() {
    float depth = proj_zw.x / proj_zw.y;
    frag_color = encode_depth(depth);
}
@end

@program shadow vs_shadow fs_shadow