
struct VertexInput
{
	float2 pos : POSITION;
	
	// Instance
	float2 inst_pos : INSTANCE_POSITION;
	float2 size : SIZE;
	float4 color : COLOR;
	uint parent_clip_id : PARENT_CLIP_ID;
	uint child_clip_id : CHILD_CLIP_ID;
};

cbuffer Commons : register(b0)
{
	float4 screen_size;
};

struct VertexOutput
{
	float4 vertex_pos : SV_POSITION;
	float4 color : COLOR;
	uint parent_clip_id : PARENT_CLIP_ID;
	uint child_clip_id : CHILD_CLIP_ID;
};

VertexOutput main(VertexInput input)
{
	float screen_width = screen_size.x;
	float screen_height = screen_size.y;
	
	float2 local_pos = input.pos;
	local_pos *= input.size;
	local_pos += input.inst_pos;

	float4 dx11_pos = float4(
		local_pos.x / screen_width * 2 - 1,
		local_pos.y / screen_height * 2 - 1,
		0,
		1
	);
	dx11_pos.y *= -1;
	
	VertexOutput output;
	output.vertex_pos = dx11_pos;
	output.color = input.color;
	output.parent_clip_id = input.parent_clip_id;
	output.child_clip_id = input.child_clip_id;
	return output;
}