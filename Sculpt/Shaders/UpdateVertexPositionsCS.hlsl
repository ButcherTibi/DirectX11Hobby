
#include "MeshPixelIn.hlsli"


struct GPU_VertexPositionUpdateGroup {
	uint vertex_id[64];
	float3 new_pos[64];
};
StructuredBuffer<GPU_VertexPositionUpdateGroup> updates;

RWStructuredBuffer<Vertex> verts;


[numthreads(64, 1, 1)]
void main(uint3 group_ids : SV_GroupID, uint3 thread_ids : SV_GroupThreadID)
{
	GPU_VertexPositionUpdateGroup update = updates.Load(group_ids.x);
	
	uint vertex_idx = update.vertex_id[thread_ids.x];
	verts[vertex_idx].pos = update.new_pos[thread_ids.x];
}