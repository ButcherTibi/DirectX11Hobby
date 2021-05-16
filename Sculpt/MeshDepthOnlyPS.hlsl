#include "MeshPixelIn.hlsli"


struct PixelOutput {
	float depth_mask : SV_Target0;
};

PixelOutput main(PixelIn input)
{
	PixelOutput output;
	output.depth_mask = input.pos.z;
	return output;
}