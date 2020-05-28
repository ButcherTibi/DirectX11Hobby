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

    if (distance(pixel_pos, center_in) <= radius_in) {
        outColor = vec4(color_in, 1);
    }
    else {
        discard;
    }
}