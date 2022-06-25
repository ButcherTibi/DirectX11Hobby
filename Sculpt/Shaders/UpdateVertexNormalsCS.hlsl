
#include "MeshPixelIn.hlsli"


struct GPU_VertexNormalUpdateGroup {
	uint vertex_id[64];
	float3 new_normal[64];
};
StructuredBuffer<GPU_VertexNormalUpdateGroup> updates;

RWStructuredBuffer<Vertex> verts;


[numthreads(64, 1, 1)]
void main(uint3 group_ids : SV_GroupID, uint3 thread_ids : SV_GroupThreadID)
{
	GPU_VertexNormalUpdateGroup update = updates.Load(group_ids.x);
	
	uint vertex_idx = update.vertex_id[thread_ids.x];
	verts[vertex_idx].normal = update.new_normal[thread_ids.x];
}