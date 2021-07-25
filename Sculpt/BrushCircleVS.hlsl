
#include "MeshPixelIn.hlsli"


StructuredBuffer<GPU_BrushCircleVertex> verts;
StructuredBuffer<GPU_BrushCircleInstance> instances;


float4 main(uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID) : SV_POSITION
{
	return pos;
}