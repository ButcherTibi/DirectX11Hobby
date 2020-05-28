#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input Attachments
layout(input_attachment_index = 0, set = 0, binding = 0)
    uniform subpassInput rects_color_subpass;

layout(input_attachment_index = 0, set = 0, binding = 1)
    uniform subpassInput rects_depth_subpass;

layout(input_attachment_index = 0, set = 0, binding = 2)
    uniform subpassInput circles_color_subpass;

layout(input_attachment_index = 0, set = 0, binding = 3)
    uniform subpassInput circles_depth_subpass;

// Outputs
layout(location = 0) out vec4 outColor;

void main() {
    vec4 rects_color_pixel = subpassLoad(rects_color_subpass);
    vec4 rects_depth_pixel = subpassLoad(rects_depth_subpass);
    vec4 circles_color_pixel = subpassLoad(circles_color_subpass);
    vec4 circles_depth_pixel = subpassLoad(circles_depth_subpass);

    if (rects_depth_pixel.x > circles_depth_pixel.x) {
        outColor = rects_color_pixel;
    }
    else if (rects_depth_pixel.x < circles_depth_pixel.x) {
        outColor = circles_color_pixel;
    } 
    else {
        if (rects_color_pixel.a > 0) {
            outColor = rects_color_pixel;
        }
        else if (circles_color_pixel.a > 0) {
            outColor = circles_color_pixel;
        }
    }
}