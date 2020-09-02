#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in flat vec4 inst_color_in;
layout(location = 1) in flat uint inst_elem_id_in;

// Outputs
layout(location = 0) out vec4 color_out;
layout(location = 1) out uint mask_out;


void main() {
    color_out = inst_color_in;
    mask_out = inst_elem_id_in;
}