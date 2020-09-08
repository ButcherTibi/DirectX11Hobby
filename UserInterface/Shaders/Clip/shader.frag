#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput elem_sub;
layout (input_attachment_index = 1, set = 1, binding = 0) uniform usubpassInput clip_mask_sub;
layout (input_attachment_index = 2, set = 2, binding = 0) uniform usubpassInput parent_clip_masks_sub;

// Outputs
layout(location = 0) out vec4 compose_out;
layout(location = 1) out uint next_parent_clip;


void main() {
    vec4 elem_color = subpassLoad(elem_sub).rgba;
    uint clip_mask = uint(subpassLoad(clip_mask_sub).r);
    uint parent_clip_masks = uint(subpassLoad(parent_clip_masks_sub).r);

    if (clip_mask == parent_clip_masks) {
        compose_out = elem_color;
        next_parent_clip = clip_mask;
        return;
    }
    discard;
}