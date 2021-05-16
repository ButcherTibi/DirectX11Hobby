
#include "MeshPixelIn.hlsli"

struct VertexIn {
	float3 pos : POSITION;
	float3 normal : NORMAL;
};


StructuredBuffer<Instance> instances;


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


PixelIn main(VertexIn vertex,
	uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID)
{
	PixelIn output;
	
	// discard vertex
	#pragma warning( disable : 3578 )
	if (vertex.normal.x == 999999.f) {
		output.pos.w = -1.f;
		return output;
	}
	#pragma warning( default : 3578 )
	
	Instance instance = instances.Load(instance_id);
	
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
	persp.z *= persp.w;
	output.pos = persp;

	// Output
	output.vertex_normal = vertex.normal;
	
	output.vertex_id = vertex_id;
	output.primitive_id = 0xFFFFFFFF;
	output.instance_id = instance_id;
	
	output.tess_edge = 0.12345;
	output.tess_edge_dir = 0.12345;
	
	return output;
}