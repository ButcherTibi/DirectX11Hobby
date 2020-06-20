#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input Attachments
layout(input_attachment_index = 0, set = 0, binding = 0)
    uniform subpassInput border_color_subpass;

layout(input_attachment_index = 0, set = 0, binding = 1)
    uniform subpassInput padding_color_subpass;

layout(input_attachment_index = 0, set = 0, binding = 2)
    uniform subpassInput border_mask_subpass;

layout(input_attachment_index = 0, set = 0, binding = 3)
    uniform subpassInput padding_mask_subpass;

// Outputs
layout(location = 0) out vec4 outColor;

void main() {
    vec4 border_pixel = subpassLoad(border_color_subpass);
    vec4 padding_pixel = subpassLoad(padding_color_subpass);

    /* red = MSAA falloff
     * green = is part of surface */
    vec4 border_mask_pixel = subpassLoad(border_mask_subpass);
    vec4 padding_mask_pixel = subpassLoad(padding_mask_subpass);
    
    if (padding_mask_pixel.g == 1) {

        if (padding_mask_pixel.r != 1) {

            outColor = vec4(mix(border_pixel.rgb, padding_pixel.rgb, padding_mask_pixel.r),
                padding_pixel.a
            );
            return;
        }

        outColor = vec4(padding_pixel.rgb, padding_pixel.a * padding_mask_pixel.r);
        return;
    }
    else {
        outColor = vec4(border_pixel.rgb, border_pixel.a * border_mask_pixel.r);
        return;
    }

    discard;
}