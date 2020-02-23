#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in vec2 in_uv;

// Input Attachments
layout(input_attachment_index = 0, set = 0, binding = 0) 
    uniform subpassInput g3d_scene_pixel;

// Samplers
layout(set = 0, binding = 1) uniform sampler2D symbol_sampler;

// Outputs
layout(location = 0) out vec4 outColor;

void main() {
    vec4 scene_pixel = subpassLoad(g3d_scene_pixel);
    vec4 tex_color = texture(symbol_sampler, in_uv);

    if (tex_color.x > 0) {
        outColor = vec4(tex_color.x, 0, 0, in_uv);
    }
    else {
        outColor = scene_pixel;
    }
}