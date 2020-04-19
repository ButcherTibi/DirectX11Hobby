#version 450
#extension GL_ARB_separate_shader_objects : enable


// Inputs
layout(location = 0) in vec2 in_pos;
layout(location = 1) in uint in_vert_idx;  // local vertex index

// Storage buffer
struct Instance {
    vec2 pos;
    float scale;
    float pad;
    vec2 uvs[6];
};

layout(std430, set = 0, binding = 1) readonly buffer StorageBuff {
    Instance insts[];
} sbuf;

// Outputs
layout(location = 0) out vec2 out_uv;

void main()
{
    vec2 pos = sbuf.insts[gl_InstanceIndex].pos;
    float scale = sbuf.insts[gl_InstanceIndex].scale;

    vec2 global_pos = in_pos * scale + pos;

    // remap UV space to vulkan space
    vec4 to_vk_pos = vec4(
        global_pos.x * 2 - 1,
        -(global_pos.y * 2 - 1), 
        0,
        1);

    gl_Position = to_vk_pos;

    // Outputs
    {
        vec2 uv = sbuf.insts[gl_InstanceIndex].uvs[in_vert_idx];
        uv.y = -uv.y;
        out_uv = uv;
    }
}