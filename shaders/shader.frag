#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in vec3 in_vert_normal;
layout(location = 1) flat in vec3 in_tess_normal;
layout(location = 2) flat in vec3 in_poly_normal;
layout(location = 3) in vec2 in_uv;
layout(location = 4) in vec3 in_color;

// Uniform Buffer
layout(std140, set = 0, binding = 0) readonly uniform UniformBuffer {
    vec3 camera_pos;
    vec4 camera_rot;
    mat4 camera_perspective;
    vec3 camera_forward;
} ubuf;

// Sampler
layout(binding = 2) uniform sampler2D Sampler;

// Outputs
layout(location = 0) out vec4 outColor;

void main() {
    if (dot(in_poly_normal, ubuf.camera_forward) > 0) {
        discard;
    }
    //outColor = vec4(in_uv.x, in_uv.y, 0, 1.0);
    outColor = texture(Sampler, in_uv);
}