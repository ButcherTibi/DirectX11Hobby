
struct VertexIn
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	
	float3 inst_pos : INSTANCE_POSITION;
	float4 inst_rot : INSTANCE_ROTATION;
};

cbuffer Uniform : register(b0)
{
	float3 camera_pos;
	float4 camera_quat_inv;
	float3 camera_forward;
	float4x4 perspective;
	//int parent_clip_id;
	//int child_clip_id;
};

struct VertexOut
{
	//int parent_clip_id;
	//int child_clip_id;
	float4 dx_pos : SV_POSITION;
	float3 normal : NORMAL;
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

VertexOut main(VertexIn input)
{
	VertexOut output;
	
	float3 pos = input.pos;
	pos = quatRotate(pos, input.inst_rot);
	pos += input.inst_pos;
	
	pos -= camera_pos;
	pos = quatRotate(pos, camera_quat_inv);
	
	float4 persp = mul(float4(pos, 0), perspective);  // perspective transform
	persp.z = -persp.z;  // left hand coordinate system
	output.dx_pos = persp;

	// Output
	output.normal = input.normal;
	
	return output;
}