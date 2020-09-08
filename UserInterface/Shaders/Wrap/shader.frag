#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in flat vec4 inst_color_in;
layout(location = 1) in flat uint inst_parent_clip_id;
layout(location = 2) in flat uint inst_child_clip_id;

layout(input_attachment_index = 0, set = 1, binding = 0) uniform usubpassInput parents_clip_mask_sub;

// Outputs
layout(location = 0) out vec4 color_out;
layout(location = 1) out uint next_parents_clip_mask;


void main() {
    uint parents_clip_mask = subpassLoad(parents_clip_mask_sub).r;
    
    if (inst_parent_clip_id == parents_clip_mask) {
        color_out = inst_color_in;
        next_parents_clip_mask = inst_child_clip_id;
        return;
    }
    discard;
}