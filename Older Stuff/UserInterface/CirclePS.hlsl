
struct PixelInput
{
	float4 pixel_pos : SV_POSITION;
	uint instance_id : INSTANCE_ID;
};

struct Instance {
	float2 pos;
	float radius;

	float4 color;
};
StructuredBuffer<Instance> sbuff;

float4 main(PixelInput input) : SV_Target
{
	Instance inst = sbuff.Load(input.instance_id);
	
	float2 top_left = float2(
		input.pixel_pos.x + 0.25,
		input.pixel_pos.y + 0.25
	);
	float2 top_right = float2(
		input.pixel_pos.x + 0.75,
		input.pixel_pos.y + 0.25
	);
	float2 bot_right = float2(
		input.pixel_pos.x + 0.75,
		input.pixel_pos.y + 0.75
	);
	float2 bot_left = float2(
		input.pixel_pos.x + 0.25,
		input.pixel_pos.y + 0.75
	);
	
	int coverge_count = 0;
	if (distance(inst.pos, top_left) < inst.radius) {
		coverge_count++;
	}
	if (distance(inst.pos, top_right) < inst.radius) {
		coverge_count++;
	}
	if (distance(inst.pos, bot_right) < inst.radius) {
		coverge_count++;
	}
	if (distance(inst.pos, bot_left) < inst.radius) {
		coverge_count++;
	}
	
	float alpha = (float)coverge_count / 4.0;
	
	return float4(inst.color.rgb, inst.color.a * alpha);
}
