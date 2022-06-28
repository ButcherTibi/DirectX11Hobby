#include "../MeshPixelIn.hlsli"


struct PixelOutput {
	float depth_mask : SV_Target0;
	float4 world_pos : SV_Target1;
};

PixelOutput main(PixelIn input)
{
	PixelOutput output;
	output.depth_mask = input.pos.z;
	output.world_pos = float4(input.world_pos, 0.12345);
	
	return output;
}