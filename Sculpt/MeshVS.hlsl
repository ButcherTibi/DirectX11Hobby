
struct VertexIn
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	
	float3 inst_pos : INSTANCE_POSITION;
	float4 inst_rot : INSTANCE_ROTATION;
	
	float3 albedo_color : ALBEDO_COLOR;
	float roughness : ROUGHNESS;
	float metallic : METALLIC;
	float specular : SPECULAR;
	
	float3 wireframe_front_color : WIREFRAME_FRONT_COLOR;
	float4 wireframe_back_color : WIREFRAME_BACK_COLOR;
};

struct CameraLight
{
	float3 normal;
	float3 color;
	float radiance;
};

cbuffer FrameUniforms : register(b0)
{
	float3 camera_pos;
	float4 camera_quat_inv;
	float3 camera_forward;
	matrix perspective;
	float z_near;
	float z_far;
	
	CameraLight lights[4];
	float ambient_intensity;
};

struct VertexOut
{
	//int parent_clip_id;
	//int child_clip_id;
	float4 dx_pos : SV_POSITION;
	float3 normal : NORMAL;
	
	float3 albedo_color : ALBEDO_COLOR;
	float roughness : ROUGHNESS;
	float metallic : METALLIC;
	float specular : SPECULAR;
	
	float3 wireframe_front_color : WIREFRAME_FRONT_COLOR;
	float4 wireframe_back_color : WIREFRAME_BACK_COLOR;
	
	//// Debug
	//float3 camera_pos : DEBUG_camera_pos;
	//float4 camera_quat : DEBUG_camera_quat;
	//float3 camera_forward : DEBUG_camera_forward;
	//matrix perspective_matrix : DEBUG_perspective_matrix;
	//float z_near : DEBUG_z_near;
	//float z_far : DEBUG_z_far;
};

float3 quatRotate(float3 v, float4 q)
{
//	vec < 3, T, Q > const QuatVector( q.x, q.y, q.z);
//	vec < 3, T, Q > const uv( glm::cross(QuatVector, v));
//	vec < 3, T, Q > const uuv( glm::cross(QuatVector, uv));

//	return v + ((uv * q.w) + uuv) * static_cast < T > (2);
	
	float3 quat_vector = float3(q.x, q.y, q.z);
	float3 uv = cross(quat_vector, v);
	float3 uuv = cross(quat_vector, uv);
	
	return v + ((uv * q.w) + uuv) * 2;
}

float3 quatRotate(float3 pos, float4 quat, float3 pivot)
{
	float3 p = pos - pivot;
	p = quatRotate(p, quat);
	return p + pivot;
}

float distPointAndPlane(float3 pos, float3 plane_pos, float3 plane_normal)
{
	return dot(plane_normal, pos - plane_pos);
}

float getLerp(float min, float center, float max)
{
	return (center - min) / (max - min);
}

VertexOut main(VertexIn input)
{
	VertexOut output;
	
	float3 pos = input.pos;
	pos = quatRotate(pos, input.inst_rot);
	pos += input.inst_pos;
	
	pos -= camera_pos;
	pos = quatRotate(pos, camera_quat_inv);
	
	float4 persp = mul(float4(pos, 1.0f), perspective); // perspective transform
	persp.z /= z_far;
	output.dx_pos = persp;

	// Output
	output.normal = input.normal;

	output.albedo_color = input.albedo_color;
	output.roughness = input.roughness;
	output.metallic = input.metallic;
	output.specular = input.specular;
	
	output.wireframe_front_color = input.wireframe_front_color;
	output.wireframe_back_color = input.wireframe_back_color;
	
	// Debug
	//output.camera_pos = camera_pos;
	//output.camera_quat = camera_quat;
	//output.camera_forward = camera_forward;
	//output.perspective_matrix = perspective_matrix;
	//output.z_near = z_near;
	//output.z_far = z_far;
	
	return output;
}