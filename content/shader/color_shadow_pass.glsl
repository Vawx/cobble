@ctype mat4 mat4

@vs vs
layout(binding=0) uniform vs_params {
    mat4 mvp;
};

in vec3 pos;
in vec2 texcoord0;

out vec2 uv;

void main() {
    gl_Position = mvp * vec4(pos, 1.0);
    uv = texcoord0;
}
@end

@fs fs
layout(binding=0) uniform texture2D diffuse_map;
layout(binding=0) uniform sampler diffuse_sampler;

in vec2 uv;
out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(diffuse_map, diffuse_sampler), uv) * vec4(1.0, 1.0, 1.0, 1.0);
}
@end

@program color_shadow_pass vs fs