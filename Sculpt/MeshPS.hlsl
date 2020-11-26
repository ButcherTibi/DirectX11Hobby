struct VertexInput
{
	float4 dx_pos : SV_POSITION;
	float3 normal : NORMAL;
};

cbuffer Uniform : register(b0)
{
	float3 camera_pos;
	float4 camera_quat_inv;
	float4x4 perspective;
	float3 camera_forward;
	//int parent_clip_id;
	//int child_clip_id;
};

float4 main(VertexInput input) : SV_TARGET
{
	//return float4(abs(input.normal.x), abs(input.normal.y), abs(input.normal.z), 1.0f);
	
	//if (dot(camera_forward, input.normal) > 0)
	//{
	//	return float4(input.normal.x, input.normal.y, input.normal.z, 1);
	//}
	//discard;
	//return float4(99, 99, 99, 99);
	
	return float4(input.normal.x, input.normal.y, input.normal.z, 1);
	//return float4(1, 1, 1, 1);
}