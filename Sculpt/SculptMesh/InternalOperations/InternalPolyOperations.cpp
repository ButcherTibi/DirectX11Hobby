#include "../SculptMesh.hpp"

using namespace scme;


void SculptMesh::initTris(uint32_t new_poly_idx, uint32_t v0, uint32_t v1, uint32_t v2)
{
	Poly& new_poly = polys[new_poly_idx];
	new_poly.is_tris = 1;
	new_poly.edges[0] = addEdge(v0, v1);
	new_poly.edges[1] = addEdge(v1, v2);
	new_poly.edges[2] = addEdge(v2, v0);

	Edge* edge = &edges[new_poly.edges[0]];
	new_poly.flip_edge_0 = edge->v0 != v0;

	edge = &edges[new_poly.edges[1]];
	new_poly.flip_edge_1 = edge->v0 != v1;

	edge = &edges[new_poly.edges[2]];
	new_poly.flip_edge_2 = edge->v0 != v2;

	registerPolyToEdge(new_poly_idx, new_poly.edges[0]);
	registerPolyToEdge(new_poly_idx, new_poly.edges[1]);
	registerPolyToEdge(new_poly_idx, new_poly.edges[2]);

	markPolyFullUpdate(new_poly_idx);
}

void SculptMesh::initQuad(uint32_t new_quad_idx, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
	Poly& new_quad = polys[new_quad_idx];
	new_quad.is_tris = false;
	new_quad.edges[0] = addEdge(v0, v1);
	new_quad.edges[1] = addEdge(v1, v2);
	new_quad.edges[2] = addEdge(v2, v3);
	new_quad.edges[3] = addEdge(v3, v0);

	Edge* edge = &edges[new_quad.edges[0]];
	new_quad.flip_edge_0 = edge->v0 != v0;

	edge = &edges[new_quad.edges[1]];
	new_quad.flip_edge_1 = edge->v0 != v1;

	edge = &edges[new_quad.edges[2]];
	new_quad.flip_edge_2 = edge->v0 != v2;

	edge = &edges[new_quad.edges[3]];
	new_quad.flip_edge_3 = edge->v0 != v3;

	registerPolyToEdge(new_quad_idx, new_quad.edges[0]);
	registerPolyToEdge(new_quad_idx, new_quad.edges[1]);
	registerPolyToEdge(new_quad_idx, new_quad.edges[2]);
	registerPolyToEdge(new_quad_idx, new_quad.edges[3]);

	markPolyFullUpdate(new_quad_idx);
}

void SculptMesh::stichVerticesToVertexLooped(std::vector<uint32_t>& vertices, uint32_t target)
{
	uint32_t last = vertices.size() - 1;
	for (uint32_t i = 0; i < last; i++) {

		uint32_t v = vertices[i];
		uint32_t v_next = vertices[i + 1];

		addTris(v, target, v_next);
	}

	addTris(vertices[last], target, vertices[0]);
}