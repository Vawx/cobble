@ctype mat4 mat4

@vs vs_shape
layout(binding=0) uniform vs_params_shapes {
    mat4 mvp;
	vec4 custom_color;
};

layout(location=0) in vec4 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;
layout(location=3) in vec4 color0;

out vec4 color;

void main() {
    color = custom_color;
    gl_Position = mvp * position;
}
@end

@fs fs_shape
in vec4 color;
out vec4 frag_color;

void main() {
    frag_color = color * vec4(1.f);
}
@end

@program shapes vs_shape fs_shape