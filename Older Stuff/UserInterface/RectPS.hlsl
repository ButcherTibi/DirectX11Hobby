
struct PixelInput
{
	float4 pixel_pos : SV_POSITION;
	uint instance_id : INSTANCE_ID;
};

struct Instance {
	float4 color;
};
StructuredBuffer<Instance> sbuff;

float4 main(PixelInput input) : SV_Target
{
	Instance inst = sbuff.Load(input.instance_id);
	return inst.color;
}