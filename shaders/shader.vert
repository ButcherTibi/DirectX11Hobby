#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs
layout(location = 0) in uint in_mesh_id;  // unused
layout(location = 1) in vec3 in_pos;
layout(location = 2) in vec3 in_vert_normal;
layout(location = 3) in vec3 in_tess_normal;
layout(location = 4) in vec3 in_poly_normal;
layout(location = 5) in vec2 in_uv;
layout(location = 6) in vec3 in_color;

// Uniform Buffer
layout(std140, set = 0, binding = 0) readonly uniform UniformBuffer {
    vec3 camera_pos;
    vec4 camera_rot;
    mat4 camera_perspective;
    vec3 camera_forward;
} ubuf;


/* Storage Buffer Types */

struct MeshProps {
    vec3 pos;
    vec4 rot;
};

layout(std140, set = 0, binding = 1) readonly buffer StorageBuffer {
    MeshProps meshes_props[];
} sbuf;


// Outputs
layout(location = 0) out vec3 out_vert_normal;
layout(location = 1) out vec3 out_tess_normal;
layout(location = 2) out vec3 out_poly_normal;
layout(location = 3) out vec2 out_uv;
layout(location = 4) out vec3 out_color;


/* Functions */

// rotate vector by quaternion
vec3 rotate(vec3 v, vec4 q)
{
    // copy paste from GLM operator* type_quat
    vec3 qvec = vec3(q.x, q.y, q.z);
    vec3 uv = cross(qvec, v);
    vec3 uuv = cross(qvec, uv);

    return v + ((uv * q.w) + uuv) * 2.0;
}

void main()
{
    vec3 pos;

    // Object
    pos = in_pos; 
    pos += sbuf.meshes_props[in_mesh_id].pos;  // local to global position

    // Camera
    pos -= ubuf.camera_pos;
    pos = rotate(pos, ubuf.camera_rot);

    // From this point on coordinate system is changed
    vec3 vulkan_pos = vec3(pos.x, -pos.y, pos.z);

    // perspective transform
    gl_Position = ubuf.camera_perspective * vec4(vulkan_pos, 1.0);

    // Outputs to next stage
    out_vert_normal = in_vert_normal;
    out_tess_normal = in_tess_normal;
    out_poly_normal = in_poly_normal;
    out_uv = in_uv;
    out_color = in_color;
}
