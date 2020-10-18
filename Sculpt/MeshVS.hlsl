
struct VertexIn
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	
	float3 inst_pos : INSTANCE_POSITION;
	float3 inst_rot : INSTANCE_ROTATION;
};

cbuffer Uniform : register(b0)
{
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

VertexOut main(VertexIn input)
{
	VertexOut output;
	
	float4 dx_v = float4(input.pos, 0);
    dx_v = mul(dx_v, perspective);
	output.dx_pos = dx_v;

	// Output
    output.normal = input.normal;
	
	return output;
}