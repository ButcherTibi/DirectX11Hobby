#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in vec2 in_uv;

// Samplers
layout(set = 0, binding = 0) uniform sampler2D symbol_sampler;

// Outputs
layout(location = 0) out vec4 outColor;

void main() {
    vec4 tex_color = texture(symbol_sampler, in_uv);

    outColor = vec4(tex_color.x, 0, 0, 1);
}