#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input Attachments
layout(input_attachment_index = 0, set = 0, binding = 0)
    uniform subpassInput border_color_subpass;

layout(input_attachment_index = 0, set = 0, binding = 1)
    uniform subpassInput padding_color_subpass;

// Outputs
layout(location = 0) out vec4 outColor;

void main() {
    vec4 border_pixel = subpassLoad(border_color_subpass);
    vec4 padding_pixel = subpassLoad(padding_color_subpass);
    
    if (padding_pixel.a > 0) {
        
        if (padding_pixel.a < 1) {

            float src_alpha = padding_pixel.a;
            float src_alpha_rev = 1 - src_alpha;
            outColor = vec4(padding_pixel.xyz * src_alpha + border_pixel.xyz * src_alpha_rev, 1);
        }
        else {
            outColor = padding_pixel;
        }
    }
    else {
        outColor = border_pixel;
    }
}