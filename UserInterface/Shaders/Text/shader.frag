#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in vec2 uv_in;
layout(location = 1) in flat vec4 inst_color_in;
layout(location = 2) in flat uint inst_elem_id_in;

// Outputs
layout(location = 0) out vec4 color_out;
layout(location = 1) out uint mask_out;

//Bindings
layout(binding = 0) uniform sampler2D atlas_sampler;


void main() {
    vec4 tex = texture(atlas_sampler, uv_in);
    vec4 color = inst_color_in;
    color.a *= tex.r;

    color_out = color;
    mask_out = inst_elem_id_in;
}