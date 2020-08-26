#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in vec4 color_in;
layout(location = 1) in vec2 center_in;
layout(location = 2) in float radius_in;

layout(input_attachment_index = 0, set = 1, binding = 0)
    uniform subpassInput mask;

// Outputs
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 out_mask;

void main() {  
    vec2 pixel_pos = vec2(gl_FragCoord.x, gl_FragCoord.y);

    // Sample Points
    float offset = 0.5;
    vec2 top_left = pixel_pos + vec2(-offset, -offset);
    vec2 top_right = pixel_pos + vec2(offset, -offset);
    vec2 bot_right = pixel_pos + vec2(offset, offset);
    vec2 bot_left = pixel_pos + vec2(-offset, offset);

    offset = 0.5;
    vec2 top = pixel_pos + vec2(0, -offset);
    vec2 right = pixel_pos + vec2(offset, 0);
    vec2 bot = pixel_pos + vec2(0, offset);
    vec2 left = pixel_pos + vec2(-offset, 0);

    // Center
    float count = 0;
    if (distance(pixel_pos, center_in) < radius_in) {
        count++;
    }

    // diagonal
    if (distance(top_left, center_in) < radius_in) {
        count++;
    }
    if (distance(top_right, center_in) < radius_in) {
        count++;
    }
    if (distance(bot_right, center_in) < radius_in) {
        count++;
    }
    if (distance(bot_left, center_in) < radius_in) {
        count++;
    }

    // sides
    if (distance(top, center_in) < radius_in) {
        count++;
    }
    if (distance(right, center_in) < radius_in) {
        count++;
    }
    if (distance(bot, center_in) < radius_in) {
        count++;
    }
    if (distance(left, center_in) < radius_in) {
        count++;
    }

    if (count > 0) {
        
        float alpha = count / 9;

        if (subpassLoad(mask).g == 1) {
            alpha = 1;
        }

        outColor = color_in;
        out_mask = vec2(alpha, 1);
        return;
    }
    discard;
}