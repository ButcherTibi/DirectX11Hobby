#include "MeshPixelIn.hlsli"


Texture2D<float> mesh_depth_tex;


[earlydepthstencil]
float4 main(PixelIn input) : SV_TARGET
{
	float mesh_mask_depth = mesh_depth_tex.Load(input.dx_pos.xyz);
	float depth = input.dx_pos.z;
	
	if (mesh_mask_depth != 0 && input.tess_edge != 1) {
		
		if (depth < mesh_mask_depth) {
			return float4(input.wireframe_back_color);
		}
		return float4(input.wireframe_front_color, 1);
	}
	
	discard;
	return float4(1, 0, 1, 1);
}