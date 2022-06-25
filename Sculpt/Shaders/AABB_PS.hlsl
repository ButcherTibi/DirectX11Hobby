#include "MeshPixelIn.hlsli"


StructuredBuffer<Instance> instances;


[earlydepthstencil]
float4 main(PixelIn input) : SV_TARGET0
{
	Instance instance = instances.Load(input.instance_id);
	
	if (input.vertex_normal.x != 1) {
		return float4(instance.wireframe_front_color, 1);
	}
	
	discard;
	return float4(1, 0, 1, 1);
}