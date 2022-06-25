#include "MeshPixelIn.hlsli"


Texture2D<float> mesh_depth_tex;

[earlydepthstencil]
float4 main(PixelIn input) : SV_TARGET
{
	float mesh_depth = mesh_depth_tex.Load(input.dx_pos.xyz);
	float wireframe_depth = input.dx_pos.z;
	
	
	
	discard;
	return float4(1, 0, 1, 1);
}