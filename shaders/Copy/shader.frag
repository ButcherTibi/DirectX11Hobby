#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input Attachments
layout(input_attachment_index = 0, set = 0, binding = 0)
    uniform subpassInput compose_subpass;

// Outputs
layout(location = 0) out vec4 outColor;

void main() {
    vec4 compose_pix = subpassLoad(compose_subpass);
    outColor = compose_pix;
}