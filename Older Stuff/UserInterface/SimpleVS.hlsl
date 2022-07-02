struct VertexInput
{
	int2 pos : POSITION;
	uint instance_id : INSTANCE_ID;
};
StructuredBuffer<VertexInput> vertices;

cbuffer Commons : register(b0)
{
	int2 screen_size;
};

struct VertexOutput
{
	float4 dx11_pos : SV_POSITION;
	uint instance_id : INSTANCE_ID;
};

VertexOutput main(uint index : SV_VertexID)
{  
	VertexInput vertex = vertices.Load(index);
	
	float2 local_pos;
	local_pos.x = vertex.pos.x / (float)screen_size.x;
	local_pos.y = vertex.pos.y / (float)screen_size.y;

	float4 dx11_pos = float4(
		local_pos.x * 2 - 1,
		-(local_pos.y * 2 - 1),
		0,
		1
	);
	
	VertexOutput output;
	output.dx11_pos = dx11_pos;
	output.instance_id = vertex.instance_id;
	
	return output;
}