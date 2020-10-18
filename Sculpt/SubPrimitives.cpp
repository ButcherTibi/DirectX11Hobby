
// Header
#include "SculptMesh.hpp"

#include <glm\geometric.hpp>


using namespace scme;


void Poly::calcNormalForTris()
{
	glm::vec3 dir_0 = vs[1]->gpu_vertex->pos - vs[0]->gpu_vertex->pos;
	glm::vec3 dir_1 = vs[2]->gpu_vertex->pos - vs[0]->gpu_vertex->pos;

	this->normal = glm::cross(dir_0, dir_1);  // TODD: might be negative, ensure clockwise front-facing
}

Edge& SculptMesh::addEdge(CPU_Vertex& v0, CPU_Vertex& v1)
{
	Edge& new_edge = this->edges.emplace_back();
	new_edge.v0 = &v0;
	new_edge.v1 = &v1;

	// Vertices
	v0.edges.push_back(&new_edge);
	v1.edges.push_back(&new_edge);

	return new_edge;
}

Poly& SculptMesh::addTris(CPU_Vertex& v0, CPU_Vertex& v1, CPU_Vertex& v2, Edge& e0, Edge& e1, Edge& e2)
{
	Poly& new_poly = this->polys.emplace_back();

	new_poly.vs.resize(3);
	new_poly.vs[0] = &v0;
	new_poly.vs[1] = &v1;
	new_poly.vs[2] = &v2;

	new_poly.edges.resize(3);
	new_poly.edges[0] = &e0;
	new_poly.edges[1] = &e1;
	new_poly.edges[2] = &e2;

	new_poly.calcNormalForTris();

	// Edges
	e0.polys.push_back(&new_poly);
	e1.polys.push_back(&new_poly);
	e2.polys.push_back(&new_poly);

	return new_poly;
}