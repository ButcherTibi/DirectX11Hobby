#include "../SculptMesh.hpp"


using namespace scme;


void SculptMesh::calcVertexNormal(uint32_t vertex_idx)
{
	Vertex* vertex = &verts[vertex_idx];

	if (vertex->edge == 0xFFFF'FFFF) {
		return;
	}

	uint32_t count = 0;
	vertex->normal = { 0, 0, 0 };

	uint32_t edge_idx = vertex->edge;
	Edge* edge = &edges[edge_idx];

	do {
		Poly* poly;
		if (edge->p0 != 0xFFFF'FFFF) {
			poly = &polys[edge->p0];
			vertex->normal += poly->normal;
			count++;
		}

		if (edge->p1 != 0xFFFF'FFFF) {
			poly = &polys[edge->p1];
			vertex->normal += poly->normal;
			count++;
		}

		// Iter
		edge_idx = edge->nextEdgeOf(vertex_idx);
		edge = &edges[edge_idx];
	} while (edge_idx != vertex->edge);

	vertex->normal /= count;
	vertex->normal = glm::normalize(vertex->normal);
}

void SculptMesh::deleteVertex(uint32_t)
{
	//Vertex* vertex = &verts[vertex_idx];

	//// vertex is point
	//if (vertex->away_loop == 0xFFFF'FFFF) {
	//	throw std::exception("TODO");
	//	return;
	//}

	//Loop* start_loop = &loops[vertex->away_loop];

	// iter around vertex
	// for each delete_loop
	//   remove delete_loop from mirror loop list

	// find all polygons atached
	// if triangle then delete
	// if quad then trim to triangle
}

void SculptMesh::printEdgeListOfVertex(uint32_t vertex_idx)
{
	Vertex& vertex = verts[vertex_idx];

	uint32_t edge_idx = vertex.edge;
	Edge* edge = &edges[edge_idx];

	printf("[");
	do {
		printf("%d, ", edge_idx);

		// Iter
		edge_idx = edge->nextEdgeOf(vertex_idx);
		edge = &edges[edge_idx];
	} while (edge_idx != vertex.edge);

	printf("]\n");
}
