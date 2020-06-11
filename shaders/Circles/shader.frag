#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in vec3 color_in;
layout(location = 1) in vec2 center_in;
layout(location = 2) in float radius_in;

// Outputs
layout(location = 0) out vec4 outColor;

void main() {
    vec2 pixel_pos = vec2(gl_FragCoord.x, gl_FragCoord.y);

    // Sample Points
    vec2 top_left = pixel_pos + vec2(-0.5, -0.5);
    vec2 top_right = pixel_pos + vec2(0.5, -0.5);
    vec2 bot_right = pixel_pos + vec2(0.5, 0.5);
    vec2 bot_left = pixel_pos + vec2(-0.5, 0.5);

    float count = 0;
    if (distance(pixel_pos, center_in) < radius_in) {
        count++;
    }
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
    
    if (count > 0) {
        float alpha = count / 5;
        outColor = vec4(color_in, alpha);
    }
    else {
        discard;
    }
}