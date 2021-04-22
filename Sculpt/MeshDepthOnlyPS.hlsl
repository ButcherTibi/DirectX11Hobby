#include "MeshPixelIn.hlsli"


struct PixelOutput {
	float depth_mask : SV_Target0;
	uint2 instance_poly_id : SV_Target1;
};

PixelOutput main(PixelIn input)
{
	PixelOutput output;
	output.depth_mask = input.dx_pos.z;
	output.instance_poly_id[0] = input.instance_id;
	output.instance_poly_id[1] = input.poly_id;
	return output;
}