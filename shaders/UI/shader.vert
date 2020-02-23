#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;

// Outputs
layout(location = 0) out vec2 out_uv;

void main()
{
    // remap screen/texture space to vulkan space
    vec4 to_vk_pos = vec4(in_pos.x * 2 - 1, in_pos.y * 2 - 1, 0, 1);

    gl_Position = to_vk_pos;

    // Outputs
    out_uv = in_uv;
}