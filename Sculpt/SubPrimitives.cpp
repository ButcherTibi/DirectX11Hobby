
// Header
#include "SculptMesh.hpp"

#include <glm\geometric.hpp>


using namespace scme;


//void Vertex::calcNormalFromEdges()
//{
//	normal = { 0, 0, 0 };
//
//	for (Edge* e : edges) {
//
//		if (e->v0 == this) {
//			normal += e->v0->pos - e->v1->pos;
//		}
//		else {
//			normal += e->v1->pos - e->v0->pos;
//		}
//	}
//
//	normal = glm::normalize(normal / (float)edges.size());
//}

void Vertex::calcNormal()
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

glm::vec3 calcNormalForTrisPositions(Vertex* v0, Vertex* v1, Vertex* v2)
{
	glm::vec3 dir_0 = glm::normalize(v1->pos - v0->pos);
	glm::vec3 dir_1 = glm::normalize(v2->pos - v0->pos);

	return glm::normalize(-glm::cross(dir_0, dir_1));
}

void Poly::calcNormalForTris()
{
	this->normal = calcNormalForTrisPositions(vs[0], vs[1], vs[2]);
}

void Poly::calcNormalForQuad()
{
	glm::vec3 n0;
	glm::vec3 n1;

	if (tesselation_type == 0) {
		n0 = calcNormalForTrisPositions(vs[0], vs[1], vs[2]);
		n1 = calcNormalForTrisPositions(vs[0], vs[2], vs[3]);
	}
	else {
		n0 = calcNormalForTrisPositions(vs[0], vs[1], vs[3]);
		n1 = calcNormalForTrisPositions(vs[1], vs[2], vs[3]);
	}

	this->tess_normals[0] = n0;
	this->tess_normals[1] = n1;
	this->normal = glm::normalize((n0 + n1) / 2.f);
}

void Poly::calcNormal()
{
	if (vs[3] == nullptr) {
		calcNormalForTris();
	}
	else {
		calcNormalForQuad();
	}
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

Poly* SculptMesh::addQuad(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3, bool tesselation_type)
{
	Poly& new_poly = this->polys.emplace_back();

	new_poly.vs[0] = v0;
	new_poly.vs[1] = v1;
	new_poly.vs[2] = v2;
	new_poly.vs[3] = v3;

	new_poly.edges[0] = addEdgeIfNone(v0, v1);
	new_poly.edges[1] = addEdgeIfNone(v1, v2);
	new_poly.edges[2] = addEdgeIfNone(v2, v3);
	new_poly.edges[3] = addEdgeIfNone(v3, v0);

	new_poly.tesselation_type = 1;

	// Edges
	new_poly.edges[0]->polys.push_back(&new_poly);
	new_poly.edges[1]->polys.push_back(&new_poly);
	new_poly.edges[2]->polys.push_back(&new_poly);
	new_poly.edges[3]->polys.push_back(&new_poly);

	return &new_poly;
}

void SculptMesh::stichVerticesToVertex(Vertex* target, std::vector<Vertex*>& vertices, bool loop)
{
	uint32_t last = vertices.size() - 1;
	for (uint32_t i = 0; i < last; i++) {
		Vertex* v = vertices[i];
		Vertex* v_next = vertices[i + 1];

		addTris(v, target, v_next);
	}

	if (loop) {
		addTris(vertices[last], target, vertices[0]);
	}
}