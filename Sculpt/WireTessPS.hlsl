#include "MeshPixelIn.hlsli"


[earlydepthstencil]
float4 main(PixelIn input) : SV_TARGET
{	
	if (input.tess_edge >= 1) {
		
		float stripe = calcStripe(input.tess_edge_dir, input.wireframe_tess_split_count, input.wireframe_tess_gap);
		
		if (stripe > 0.5) {
			return float4(input.wireframe_tess_front_color, 1);
		}
		
		discard;
		return float4(1, 0, 1, 1);
	}
	
	return float4(input.wireframe_front_color, 1);
}