#include "MeshPixelIn.hlsli"


Texture2D<float> mesh_depth_tex;

[earlydepthstencil]
float4 main(PixelIn input) : SV_TARGET
{
	float mesh_depth = mesh_depth_tex.Load(input.dx_pos.xyz);
	float wireframe_depth = input.dx_pos.z;
	
	if (mesh_depth < 1.) {
		
		// back
		if (wireframe_depth > mesh_depth) {
		
			// tesselation edge
			if (input.tess_edge >= 1) {
				float stripe = calcStripe(input.tess_edge_dir, input.wireframe_tess_split_count, input.wireframe_tess_gap);
		
				if (stripe > 0.5) {
					return float4(input.wireframe_tess_back_color);
				}
		
				discard;
				return float4(1, 0, 1, 1);
			}
		
			return float4(input.wireframe_back_color);
		}
	
		// front
		if (input.tess_edge >= 1) {
			float stripe = calcStripe(input.tess_edge_dir, input.wireframe_tess_split_count, input.wireframe_tess_gap);
		
			if (stripe > 0.5) {
				return float4(input.wireframe_tess_front_color.rgb, 1);
			}
		
			discard;
			return float4(1, 0, 1, 1);
		}
	
		return float4(input.wireframe_front_color, 1);
	}
	
	discard;
	return float4(1, 0, 1, 1);
}