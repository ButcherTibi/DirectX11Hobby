#include "MeshPixelIn.hlsli"


[earlydepthstencil]
float4 main(PixelIn input) : SV_TARGET
{	
	if (input.tess_edge != 1) {
		return float4(input.wireframe_front_color, 1);
	}
	
	discard;
	return float4(1, 0, 1, 1);
}