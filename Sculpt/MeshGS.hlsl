
#include "MeshPixelIn.hlsli"

StructuredBuffer<MeshTriangle> mesh_triangles;

[maxvertexcount(3)]
void main(triangle PixelIn input[3], uint primitive_id : SV_PrimitiveID,
	inout TriangleStream<PixelIn> output)
{
	if (input[0].pos.w == 0) {
		return;
	}
	
	for (uint i = 0; i < 3; i++) {
		
		PixelIn vertex;
		
		// Vertex
		vertex.pos = input[i].pos;
		vertex.world_pos = input[i].world_pos;
		vertex.vertex_normal = input[i].vertex_normal;
		
		vertex.vertex_id = input[i].vertex_id;
		vertex.primitive_id = primitive_id;
		vertex.instance_id = input[i].instance_id;
		
		MeshTriangle tris = mesh_triangles.Load(primitive_id);
		if (input[i].vertex_id == tris.tess_vertex_0) {
			vertex.tess_edge = 1;
			vertex.tess_edge_dir = 0;
		}
		else if (input[i].vertex_id == tris.tess_vertex_1) {
			vertex.tess_edge = 1;
			vertex.tess_edge_dir = 1;
		}
		else {
			vertex.tess_edge = 0;
			vertex.tess_edge_dir = 0.12345;
		}
		
		output.Append(vertex);
	}
}