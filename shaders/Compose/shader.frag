#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input Attachments
layout(input_attachment_index = 0, set = 0, binding = 0)
    uniform subpassInput g3d_subpass;

layout(input_attachment_index = 0, set = 0, binding = 1)
    uniform subpassInput ui_subpass;

// Outputs
layout(location = 0) out vec4 outColor;

void main() {
    vec4 scene_pixel = subpassLoad(g3d_subpass);
    vec4 ui_pixel = subpassLoad(ui_subpass);

    outColor = ui_pixel;
}