
// Header
#include "SculptMesh.hpp"

#include <glm\geometric.hpp>


using namespace scme;


void Vertex::calcNormalFromEdges()
{
	normal = { 0, 0, 0 };

	for (Edge* e : edges) {

		if (e->v0 == this) {
			normal += e->v0->pos - e->v1->pos;
		}
		else {
			normal += e->v1->pos - e->v0->pos;
		}
	}

	normal = glm::normalize(normal / (float)edges.size());
}

void Vertex::calcNormalFromPolyNormals()
{
	uint32_t count = 0;
	normal = { 0, 0, 0 };

	for (Edge* e : edges) {
		for (Poly* poly : e->polys) {
			normal += poly->normal;
			count++;
		}
	}

	normal = glm::normalize(normal / (float)count);
}

void Poly::calcNormalForTris()
{
	glm::vec3 dir_0 = glm::normalize(vs[1]->pos - vs[0]->pos);
	glm::vec3 dir_1 = glm::normalize(vs[2]->pos - vs[0]->pos);

	this->normal = glm::normalize(glm::cross(dir_0, dir_1));  // TODD: might be negative, ensure clockwise front-facing
}

Edge* SculptMesh::addEdge(Vertex* v0, Vertex* v1)
{
	Edge& new_edge = this->edges.emplace_back();
	new_edge.v0 = v0;
	new_edge.v1 = v1;

	// Vertices
	v0->edges.push_back(&new_edge);
	v1->edges.push_back(&new_edge);

	return &new_edge;
}

Edge* SculptMesh::addEdgeIfNone(Vertex* v0, Vertex* v1)
{
	for (Edge* e0 : v0->edges) {

		if (e0 != nullptr) {
			for (Edge* e1 : v1->edges) {

				if (e1 != nullptr && e0 == e1) {
					return e0;
				}
			}
		}
	}

	return addEdge(v0, v1);
}

Poly& SculptMesh::addTris(Vertex& v0, Vertex& v1, Vertex& v2,
	Edge& e0, Edge& e1, Edge& e2)
{
	Poly& new_poly = this->polys.emplace_back();

	new_poly.vs[0] = &v0;
	new_poly.vs[1] = &v1;
	new_poly.vs[2] = &v2;
	new_poly.vs[3] = nullptr;

	new_poly.edges[0] = &e0;
	new_poly.edges[1] = &e1;
	new_poly.edges[2] = &e2;
	new_poly.edges[3] = nullptr;

	// Edges
	e0.polys.push_back(&new_poly);
	e1.polys.push_back(&new_poly);
	e2.polys.push_back(&new_poly);

	return new_poly;
}

Poly* SculptMesh::addTris(Vertex* v0, Vertex* v1, Vertex* v2)
{
	Poly& new_poly = this->polys.emplace_back();

	new_poly.vs[0] = v0;
	new_poly.vs[1] = v1;
	new_poly.vs[2] = v2;
	new_poly.vs[3] = nullptr;

	new_poly.edges[0] = addEdgeIfNone(v0, v1);
	new_poly.edges[1] = addEdgeIfNone(v1, v2);
	new_poly.edges[2] = addEdgeIfNone(v2, v0);
	new_poly.edges[3] = nullptr;

	// Edges
	new_poly.edges[0]->polys.push_back(&new_poly);
	new_poly.edges[1]->polys.push_back(&new_poly);
	new_poly.edges[2]->polys.push_back(&new_poly);

	return &new_poly;
}

Poly* SculptMesh::addTrisNormalWinding(Vertex* v0, Vertex* v1, Vertex* v2)
{
	glm::vec3 avg_normal = (v0->normal + v1->normal + v2->normal);
	avg_normal /= 3;
	avg_normal = glm::normalize(avg_normal);

	glm::vec3 dir_0 = v1->pos - v0->pos;
	glm::vec3 dir_1 = v2->pos - v0->pos;
	glm::vec3 winding_normal = glm::cross(dir_0, dir_1);

	if (glm::dot(avg_normal, winding_normal) > 0) {

		addTris(v0, v1, v2);
	}

	return addTris(v2, v1, v0);
}

//Poly* SculptMesh::addTrisPositionWinding(Vertex* v0, Vertex* v1, Vertex* v2)
//{
//	
//}

Poly& SculptMesh::addQuad(Vertex& v0, Vertex& v1, Vertex& v2, Vertex& v3,
	Edge& e0, Edge& e1, Edge& e2, Edge& e3, bool tesselation_type)
{
	Poly& new_poly = this->polys.emplace_back();

	new_poly.vs[0] = &v0;
	new_poly.vs[1] = &v1;
	new_poly.vs[2] = &v2;
	new_poly.vs[3] = &v3;

	new_poly.edges[0] = &e0;
	new_poly.edges[1] = &e1;
	new_poly.edges[2] = &e2;
	new_poly.edges[3] = &e3;

	new_poly.tesselation_type = tesselation_type;

	// Edges
	e0.polys.push_back(&new_poly);
	e1.polys.push_back(&new_poly);
	e2.polys.push_back(&new_poly);
	e3.polys.push_back(&new_poly);

	return new_poly;
}