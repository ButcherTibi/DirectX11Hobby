#include "MeshPixelIn.hlsli"


cbuffer Dummny : register(b0)
{
	uint root_aabb_size;
	uint aabb_levels;
};

StructuredBuffer<Vertex> verts;

struct GPU_UnplacedVertices {
	uint vert_idxs[21];
};
StructuredBuffer<GPU_UnplacedVertices> unplaced_verts;

struct GPU_PlacedVertices {
	int vert_idxs[21][3][5];
};
RWStructuredBuffer<GPU_PlacedVertices> r_placements;


[numthreads(21, 3, 1)]
void main(uint3 group_ids : SV_GroupID, uint3 thread_ids : SV_GroupThreadID)
{		
	float dim;
	{
		GPU_UnplacedVertices group_verts = unplaced_verts.Load(group_ids.x);
		Vertex vertex = verts.Load(group_verts.vert_idxs[thread_ids.x]);
		dim = vertex.pos[thread_ids.y];
	}
	
	// dimension of the position to figure out were to place in AABBs
	uint aabb_size = root_aabb_size;
	
	// recursivelly group the dimension into smaller AABBs
	[unroll(5)]
	for (uint level = 0; level < aabb_levels; level++) {
		
		r_placements[group_ids.x].vert_idxs[thread_ids.x][thread_ids.y][level] =
			(int)(dim / (float)aabb_size);

		dim %= (float)aabb_size;
		aabb_size /= 2;
	}
}
