#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

// Instance
layout(location = 2) in vec4 inst_color;
layout(location = 3) in vec2 inst_pos;
layout(location = 4) in float inst_raster_size;
layout(location = 5) in float inst_size;
layout(location = 6) in uint inst_parent_clip_id;

// Outputs
layout(location = 0) out vec2 uv_out;
layout(location = 1) out vec4 inst_color_out;
layout(location = 2) out uint inst_parent_clip_id_out;

layout(set = 1, binding = 0) uniform stub {
    vec4 screen_size;
} ubuff;


void main()
{   
    float screen_width = ubuff.screen_size.x;
    float screen_height = ubuff.screen_size.y;

    vec2 local_pos = pos;
    local_pos *= inst_size / inst_raster_size;  // scaling
    local_pos += inst_pos;  // character position

    vec4 vk_pos = vec4(
        local_pos.x / screen_width * 2 - 1,
        local_pos.y / screen_height * 2 - 1,
        0,
        1
    );

    gl_Position = vk_pos;

    uv_out = uv;
    inst_color_out = inst_color;
    inst_parent_clip_id_out = inst_parent_clip_id;
}