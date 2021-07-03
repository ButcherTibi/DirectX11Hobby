
#include "MeshPixelIn.hlsli"


struct GPU_PolyNormalUpdateGroup {
	uint tess_idxs[32][2];  // idx of tess triangles to update
	uint poly_verts[32][4];  // vertices of polygon
	uint tess_type[32];
	uint tess_split_vertices[32][2];
};
StructuredBuffer<GPU_PolyNormalUpdateGroup> updates;


StructuredBuffer<Vertex> verts;


RWStructuredBuffer<MeshTriangle> triangles;


struct GPU_Result_PolyNormalUpdateGroup {
	float3 poly_normal[32];
	float3 tess_normals[32][2];
};
RWStructuredBuffer<GPU_Result_PolyNormalUpdateGroup> results;


float3 calcWindingNormal(in Vertex v0, in Vertex v1, in Vertex v2)
{
	float3 axis_0 = v1.pos - v0.pos;
	float3 axis_1 = v2.pos - v0.pos;
	float3 normal = cross(axis_0, axis_1);
	
	return -normalize(normal);
}


[numthreads(32, 1, 1)]
void main(uint3 group_ids : SV_GroupID, uint3 thread_ids : SV_GroupThreadID)
{
	GPU_PolyNormalUpdateGroup update = updates.Load(group_ids.x);
	
	uint t0_idx;
	uint t1_idx;
	
	Vertex v0;
	Vertex v1;
	Vertex v2;
	Vertex v3;
	{
		t0_idx = update.tess_idxs[thread_ids.x][0];
		if (t0_idx == 0xFFFFFFFF) {
			// excess thread call
			return;
		}
		
		t1_idx = update.tess_idxs[thread_ids.x][1];
		
		v0 = verts[update.poly_verts[thread_ids.x][0]];
		v1 = verts[update.poly_verts[thread_ids.x][1]];
		v2 = verts[update.poly_verts[thread_ids.x][2]];
		v3 = verts[update.poly_verts[thread_ids.x][3]];
	}
	
	// Poly is Triangle (the second triangle exists is next but is unused and never rendered, so ignored)
	if (t1_idx == 0xFFFFFFFF) {
		
		// update GPU data
		float3 normal = calcWindingNormal(v0, v1, v2);
		triangles[t0_idx].poly_normal = normal;
		triangles[t0_idx].tess_normal = normal;
		// tess type unused
		
		// remember changes
		results[group_ids.x].poly_normal[thread_ids.x] = normal;
		results[group_ids.x].tess_normals[thread_ids.x][0] = normal;
		results[group_ids.x].tess_normals[thread_ids.x][1] = normal;
	}
	// Poly is Quad
	else {
		float3 tess_normal_0;
		float3 tess_normal_1;
		uint tesselation_type = update.tess_type[thread_ids.x];
		
		if (tesselation_type == 0) {
			
			tess_normal_0 = calcWindingNormal(v0, v2, v3);
			tess_normal_1 = calcWindingNormal(v0, v1, v2);
		}
		else {
			tess_normal_0 = calcWindingNormal(v0, v1, v3);
			tess_normal_1 = calcWindingNormal(v1, v2, v3);
		}
		
		float3 poly_normal = normalize((tess_normal_0 + tess_normal_1) / 2.0);
		
		triangles[t0_idx].poly_normal = poly_normal;
		triangles[t0_idx].tess_normal = tess_normal_0;
		triangles[t0_idx].tess_vertex_0 = update.tess_split_vertices[thread_ids.x][0];
		triangles[t0_idx].tess_vertex_1 = update.tess_split_vertices[thread_ids.x][1];
		
		triangles[t1_idx].poly_normal = poly_normal;
		triangles[t1_idx].tess_normal = tess_normal_1;
		triangles[t1_idx].tess_vertex_0 = update.tess_split_vertices[thread_ids.x][0];
		triangles[t1_idx].tess_vertex_1 = update.tess_split_vertices[thread_ids.x][1];
		
		results[group_ids.x].poly_normal[thread_ids.x] = poly_normal;
		results[group_ids.x].tess_normals[thread_ids.x][0] = tess_normal_0;
		results[group_ids.x].tess_normals[thread_ids.x][1] = tess_normal_1;
	}
}
