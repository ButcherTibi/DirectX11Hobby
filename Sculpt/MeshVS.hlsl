
struct VertexIn
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	
	float3 inst_pos : INSTANCE_POSITION;
	float3 inst_rot : INSTANCE_ROTATION;
};

cbuffer Uniform : register(b0)
{
	float3 camera_pos;
	float4 camera_quat;
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

//float4 quatRotate(float3 pos, float4 quat)
//{
//	vec < 3, T, Q > const QuatVector( q.x, q.y, q.z);
//	vec < 3, T, Q > const uv( glm::cross(QuatVector, v));
//	vec < 3, T, Q > const uuv( glm::cross(QuatVector, uv));

//	return v + ((uv * q.w) + uuv) * static_cast < T > (2);
//}

VertexOut main(VertexIn input)
{
	VertexOut output;
	
	float3 pos = input.pos;
	pos += input.inst_pos;
	pos -= camera_pos;
	
	float4 persp = mul(float4(pos, 1), perspective);
	output.dx_pos = persp;

	// Output
    output.normal = input.normal;
	
	return output;
}