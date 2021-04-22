
struct VertexIn {
	float3 pos : POSITION;
	float3 normal : NORMAL;
};

struct InstanceIn {
	float3 inst_pos : INSTANCE_POSITION;
	float4 inst_rot : INSTANCE_ROTATION;
	
	float3 albedo_color : ALBEDO_COLOR;
	float roughness : ROUGHNESS;
	float metallic : METALLIC;
	float specular : SPECULAR;
	
	float3 wireframe_front_color : WIREFRAME_FRONT_COLOR;
	float4 wireframe_back_color : WIREFRAME_BACK_COLOR;
	float3 wireframe_tess_front_color : WIREFRAME_TESS_FRONT_COLOR;
	float4 wireframe_tess_back_color : WIREFRAME_TESS_BACK_COLOR;
	float wireframe_tess_split_count : WIREFRAME_TESS_SPLIT_COUNT;
	float wireframe_tess_gap : WIREFRAME_TESS_GAP;
	
	uint instance_id : INSTANCE_ID;
};

struct CameraLight {
	float3 normal;
	float3 color;
	float radiance;
};

cbuffer FrameUniforms : register(b0) {
	float3 camera_pos;
	float4 camera_quat_inv;
	float3 camera_forward;
	matrix perspective;
	float z_near;
	float z_far;
	
	CameraLight lights[4];
	float ambient_intensity;
	uint shading_normal;
};

struct VertexOut {
	float4 dx_pos : SV_POSITION;
	float3 normal : NORMAL;
	
	nointerpolation float3 albedo_color : ALBEDO_COLOR;
	nointerpolation float roughness : ROUGHNESS;
	nointerpolation float metallic : METALLIC;
	nointerpolation float specular : SPECULAR;
	
	nointerpolation float3 wireframe_front_color : WIREFRAME_FRONT_COLOR;
	nointerpolation float4 wireframe_back_color : WIREFRAME_BACK_COLOR;
	nointerpolation float3 wireframe_tess_front_color : WIREFRAME_TESS_FRONT_COLOR;
	nointerpolation float4 wireframe_tess_back_color : WIREFRAME_TESS_BACK_COLOR;
	nointerpolation float wireframe_tess_split_count : WIREFRAME_TESS_SPLIT_COUNT;
	nointerpolation float wireframe_tess_gap : WIREFRAME_TESS_GAP;
	
	nointerpolation uint instance_id : INSTANCE_ID;

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

VertexOut main(VertexIn vertex, InstanceIn instance)
{
	VertexOut output;
	
	// World Position
	float3 pos = vertex.pos;
	pos = quatRotate(pos, instance.inst_rot);
	pos += instance.inst_pos;
	
	// Camera Position
	pos -= camera_pos;
	pos = quatRotate(pos, camera_quat_inv);
	
	// Viewport Position
	float4 persp = mul(perspective, float4(pos, 1.f));  // perspective transform
	
	// Fix Depth
	persp.z = getLerp(z_near, persp.w, z_far);
	//persp.z = log(persp.z * 2);
	
	//float C = 1;
	//persp.z = 2.0*log(persp.w * C + 1)/log(z_far * C + 1) - 1;
	
	//persp.z = 1.0 - persp.z;
	persp.z *= persp.w;
	output.dx_pos = persp;

	// Output
	output.normal = vertex.normal;

	output.albedo_color = instance.albedo_color;
	output.roughness = instance.roughness;
	output.metallic = instance.metallic;
	output.specular = instance.specular;
	
	output.wireframe_front_color = instance.wireframe_front_color;
	output.wireframe_back_color = instance.wireframe_back_color;
	output.wireframe_tess_front_color = instance.wireframe_tess_front_color;
	output.wireframe_tess_back_color = instance.wireframe_tess_back_color;
	output.wireframe_tess_split_count = instance.wireframe_tess_split_count;
	output.wireframe_tess_gap = instance.wireframe_tess_gap;
	
	output.instance_id = instance.instance_id;
	
	// Debug
	//output.camera_pos = camera_pos;
	//output.camera_quat = camera_quat;
	//output.camera_forward = camera_forward;
	//output.perspective_matrix = perspective_matrix;
	//output.z_near = z_near;
	//output.z_far = z_far;
	
	return output;
}