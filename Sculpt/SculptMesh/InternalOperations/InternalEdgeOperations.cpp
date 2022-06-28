#include "../SculptMesh.hpp"

using namespace scme;


uint32_t SculptMesh::createEdge(VertexIndex v0, VertexIndex v1)
{
	uint32_t new_loop_idx;;
	edges.emplace(new_loop_idx);

	initEdge(new_loop_idx, v0, v1);
	return new_loop_idx;
}

void SculptMesh::initEdge(uint32_t existing_edge_idx, uint32_t v0_idx, uint32_t v1_idx)
{
	Edge* existing_edge = &edges[existing_edge_idx];
	existing_edge->v0 = v0_idx;
	existing_edge->v1 = v1_idx;
	existing_edge->p0 = 0xFFFF'FFFF;
	existing_edge->p1 = 0xFFFF'FFFF;
	existing_edge->was_raycast_tested = false;

	registerEdgeToVertexList(existing_edge_idx, v0_idx);
	registerEdgeToVertexList(existing_edge_idx, v1_idx);
}