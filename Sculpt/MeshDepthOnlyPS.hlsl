#include "MeshPixelIn.hlsli"


struct PixelOutput {
	float depth_mask : SV_Target0;
	uint instance_id : SV_Target1;
};

PixelOutput main(PixelIn input)
{
	PixelOutput output;
	output.depth_mask = input.dx_pos.z;
	output.instance_id = input.instance_id;
	return output;
}