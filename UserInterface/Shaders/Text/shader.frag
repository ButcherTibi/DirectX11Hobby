#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in vec2 uv_in;
layout(location = 1) in flat vec4 inst_color;
layout(location = 2) in flat uint inst_parent_clip_id;

// Input Bindings
layout(set = 0, binding = 0) uniform sampler2D atlas_sampler;
layout(input_attachment_index = 0, set = 2, binding = 0) uniform usubpassInput parents_clip_mask_sub;

// Outputs
layout(location = 0) out vec4 color_out;


void main() {
    uint parents_clip_mask = subpassLoad(parents_clip_mask_sub).r;

    if (inst_parent_clip_id == parents_clip_mask) {

        vec4 tex = texture(atlas_sampler, uv_in);
        vec4 color = inst_color;
        color.a *= tex.r;
        
        color_out = color;
        return;
    }
    discard;
}