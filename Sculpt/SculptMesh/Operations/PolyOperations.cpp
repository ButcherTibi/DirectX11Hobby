#include "../SculptMesh.hpp"

using namespace scme;


glm::vec3 SculptMesh::calcWindingNormal(Vertex* v0, Vertex* v1, Vertex* v2)
{
	return -glm::normalize(glm::cross(v1->pos - v0->pos, v2->pos - v0->pos));
}

void SculptMesh::calcPolyNormal(Poly* poly)
{
	if (poly->is_tris) {

		std::array<scme::Vertex*, 3> vs;
		getTrisPrimitives(poly, vs);

		// Triangle Normal
		poly->normal = calcWindingNormal(vs[0], vs[1], vs[2]);
		poly->tess_normals[0] = poly->normal;
		poly->tess_normals[1] = poly->normal;
	}
	else {
		std::array<scme::Vertex*, 4> vs;
		getQuadPrimitives(poly, vs);

		// Tesselation and Normals
		if (glm::distance(vs[0]->pos, vs[2]->pos) < glm::distance(vs[1]->pos, vs[3]->pos)) {

			poly->tesselation_type = 0;
			poly->tess_normals[0] = calcWindingNormal(vs[0], vs[1], vs[2]);
			poly->tess_normals[1] = calcWindingNormal(vs[0], vs[2], vs[3]);
		}
		else {
			poly->tesselation_type = 1;
			poly->tess_normals[0] = calcWindingNormal(vs[0], vs[1], vs[3]);
			poly->tess_normals[1] = calcWindingNormal(vs[1], vs[2], vs[3]);
		}
		poly->normal = glm::normalize((poly->tess_normals[0] + poly->tess_normals[1]) / 2.f);
	}
}

uint32_t SculptMesh::addTris(uint32_t v0, uint32_t v1, uint32_t v2)
{
	uint32_t new_tris_idx;
	polys.emplace(new_tris_idx);

	initTris(new_tris_idx, v0, v1, v2);

	return new_tris_idx;
}

uint32_t SculptMesh::addQuad(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
	uint32_t new_quad_idx;
	polys.emplace(new_quad_idx);

	initQuad(new_quad_idx, v0, v1, v2, v3);

	return new_quad_idx;
}

void SculptMesh::deletePoly(uint32_t delete_poly_idx)
{
	Poly* delete_poly = &polys[delete_poly_idx];

	uint32_t count;
	if (delete_poly->is_tris) {
		count = 3;
	}
	else {
		count = 4;
	}

	for (uint8_t i = 0; i < count; i++) {

		uint32_t edge_idx = delete_poly->edges[i];
		Edge* edge = &edges[edge_idx];

		Vertex* vertex = &verts[edge->v0];
		unregisterEdgeFromVertex(edge, edge->v0, vertex);

		if (edge->nextEdgeOf(edge->v0) == edge_idx) {
			_deleteVertexMemory(edge->v0);
		}

		vertex = &verts[edge->v1];
		unregisterEdgeFromVertex(edge, edge->v1, vertex);

		if (edge->nextEdgeOf(edge->v1) == edge_idx) {
			_deleteVertexMemory(edge->v1);
		}

		// unregister poly from edge
		// because edge is part of poly one poly reference will reference poly
		if (edge->p0 == delete_poly_idx) {
			edge->p0 = 0xFFFF'FFFF;
		}
		else {
			edge->p1 = 0xFFFF'FFFF;
		}

		// edge is wire
		if (edge->p0 == edge->p1) {
			_deleteEdgeMemory(edge_idx);
		}
	}

	// Now no edges reference the poly so we can safely delete the polygon
	_deletePolyMemory(delete_poly_idx);
}

void SculptMesh::getTrisPrimitives(Poly* poly,
	std::array<uint32_t, 3>& r_vs_idxs, std::array<Vertex*, 3>& r_vs)
{
	std::array<Edge*, 3> r_es;
	r_es[0] = &edges[poly->edges[0]];
	r_es[1] = &edges[poly->edges[1]];
	r_es[2] = &edges[poly->edges[2]];

	r_vs_idxs[0] = poly->flip_edge_0 ? r_es[0]->v1 : r_es[0]->v0;
	r_vs_idxs[1] = poly->flip_edge_1 ? r_es[1]->v1 : r_es[1]->v0;
	r_vs_idxs[2] = poly->flip_edge_2 ? r_es[2]->v1 : r_es[2]->v0;

	r_vs[0] = &verts[r_vs_idxs[0]];
	r_vs[1] = &verts[r_vs_idxs[1]];
	r_vs[2] = &verts[r_vs_idxs[2]];
}

void SculptMesh::getTrisPrimitives(Poly* poly, std::array<uint32_t, 3>& r_vs_idxs)
{
	std::array<Edge*, 3> r_es;
	r_es[0] = &edges[poly->edges[0]];
	r_es[1] = &edges[poly->edges[1]];
	r_es[2] = &edges[poly->edges[2]];

	r_vs_idxs[0] = poly->flip_edge_0 ? r_es[0]->v1 : r_es[0]->v0;
	r_vs_idxs[1] = poly->flip_edge_1 ? r_es[1]->v1 : r_es[1]->v0;
	r_vs_idxs[2] = poly->flip_edge_2 ? r_es[2]->v1 : r_es[2]->v0;
}

void SculptMesh::getTrisPrimitives(Poly* poly, std::array<Vertex*, 3>& r_vs)
{
	std::array<Edge*, 3> r_es;
	r_es[0] = &edges[poly->edges[0]];
	r_es[1] = &edges[poly->edges[1]];
	r_es[2] = &edges[poly->edges[2]];

	std::array<uint32_t, 3> r_vs_idxs;
	r_vs_idxs[0] = poly->flip_edge_0 ? r_es[0]->v1 : r_es[0]->v0;
	r_vs_idxs[1] = poly->flip_edge_1 ? r_es[1]->v1 : r_es[1]->v0;
	r_vs_idxs[2] = poly->flip_edge_2 ? r_es[2]->v1 : r_es[2]->v0;

	r_vs[0] = &verts[r_vs_idxs[0]];
	r_vs[1] = &verts[r_vs_idxs[1]];
	r_vs[2] = &verts[r_vs_idxs[2]];
}

void SculptMesh::getQuadPrimitives(Poly* poly,
	std::array<uint32_t, 4>& r_vs_idxs, std::array<Vertex*, 4>& r_vs)
{
	std::array<Edge*, 4> r_es;
	r_es[0] = &edges[poly->edges[0]];
	r_es[1] = &edges[poly->edges[1]];
	r_es[2] = &edges[poly->edges[2]];
	r_es[3] = &edges[poly->edges[3]];

	r_vs_idxs[0] = poly->flip_edge_0 ? r_es[0]->v1 : r_es[0]->v0;
	r_vs_idxs[1] = poly->flip_edge_1 ? r_es[1]->v1 : r_es[1]->v0;
	r_vs_idxs[2] = poly->flip_edge_2 ? r_es[2]->v1 : r_es[2]->v0;
	r_vs_idxs[3] = poly->flip_edge_3 ? r_es[3]->v1 : r_es[3]->v0;

	r_vs[0] = &verts[r_vs_idxs[0]];
	r_vs[1] = &verts[r_vs_idxs[1]];
	r_vs[2] = &verts[r_vs_idxs[2]];
	r_vs[3] = &verts[r_vs_idxs[3]];
}

void SculptMesh::getQuadPrimitives(Poly* poly, std::array<uint32_t, 4>& r_vs_idxs)
{
	std::array<Edge*, 4> r_es;
	r_es[0] = &edges[poly->edges[0]];
	r_es[1] = &edges[poly->edges[1]];
	r_es[2] = &edges[poly->edges[2]];
	r_es[3] = &edges[poly->edges[3]];

	r_vs_idxs[0] = poly->flip_edge_0 ? r_es[0]->v1 : r_es[0]->v0;
	r_vs_idxs[1] = poly->flip_edge_1 ? r_es[1]->v1 : r_es[1]->v0;
	r_vs_idxs[2] = poly->flip_edge_2 ? r_es[2]->v1 : r_es[2]->v0;
	r_vs_idxs[3] = poly->flip_edge_3 ? r_es[3]->v1 : r_es[3]->v0;
}

void SculptMesh::getQuadPrimitives(Poly* poly, std::array<Vertex*, 4>& r_vs)
{
	std::array<Edge*, 4> r_es;
	r_es[0] = &edges[poly->edges[0]];
	r_es[1] = &edges[poly->edges[1]];
	r_es[2] = &edges[poly->edges[2]];
	r_es[3] = &edges[poly->edges[3]];

	std::array<uint32_t, 4> r_vs_idxs;
	r_vs_idxs[0] = poly->flip_edge_0 ? r_es[0]->v1 : r_es[0]->v0;
	r_vs_idxs[1] = poly->flip_edge_1 ? r_es[1]->v1 : r_es[1]->v0;
	r_vs_idxs[2] = poly->flip_edge_2 ? r_es[2]->v1 : r_es[2]->v0;
	r_vs_idxs[3] = poly->flip_edge_3 ? r_es[3]->v1 : r_es[3]->v0;

	r_vs[0] = &verts[r_vs_idxs[0]];
	r_vs[1] = &verts[r_vs_idxs[1]];
	r_vs[2] = &verts[r_vs_idxs[2]];
	r_vs[3] = &verts[r_vs_idxs[3]];
}