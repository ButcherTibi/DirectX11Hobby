#include "../SculptMesh.hpp"

using namespace scme;


uint32_t SculptMesh::addEdge(uint32_t v0, uint32_t v1)
{
	uint32_t existing_loop = findEdgeBetween(v0, v1);
	if (existing_loop == 0xFFFF'FFFF) {
		return createEdge(v0, v1);
	}

	return existing_loop;
}


uint32_t SculptMesh::findEdgeBetween(uint32_t v0_idx, uint32_t v1_idx)
{
	Vertex& vertex_0 = verts[v0_idx];
	Vertex& vertex_1 = verts[v1_idx];

	if (vertex_0.edge == 0xFFFF'FFFF || vertex_1.edge == 0xFFFF'FFFF) {
		return 0xFFFF'FFFF;
	}

	uint32_t edge_idx = vertex_0.edge;
	Edge* edge = &edges[edge_idx];

	iterEdgesAroundVertexStart;
	{
		if ((edge->v0 == v0_idx && edge->v1 == v1_idx) ||
			(edge->v0 == v1_idx && edge->v1 == v0_idx))
		{
			return edge_idx;
		}
	}
	iterEdgesAroundVertexEnd(v0_idx, vertex_0.edge);

	edge_idx = vertex_1.edge;
	edge = &edges[edge_idx];

	iterEdgesAroundVertexStart;
	{
		if ((edge->v0 == v0_idx && edge->v1 == v1_idx) ||
			(edge->v0 == v1_idx && edge->v1 == v0_idx))
		{
			return edge_idx;
		}
	}
	iterEdgesAroundVertexEnd(v1_idx, vertex_1.edge);

	return 0xFFFF'FFFF;
}