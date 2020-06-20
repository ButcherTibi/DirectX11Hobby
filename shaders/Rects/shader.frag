#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in vec4 color_in;

// Outputs
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 out_mask;

void main() {
    outColor = color_in;
    out_mask = vec2(1, 1);
}