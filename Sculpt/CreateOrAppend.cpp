
// Header
#include "SculptMesh.hpp"


using namespace scme;


void SculptMesh::createAsTriangle(float size)
{
	VertexBoundingBox& root = aabbs.emplace_back();
	root.vs.resize(3);

	auto& vs = root.vs;

	float half = size / 2;
	vs[0].pos = { 0, half, 0 };
	vs[0].normal = { 0, 0, 1 };

	vs[1].pos = { half, -half, 0 };
	vs[1].normal = { 0, 0, 1 };

	vs[2].pos = { -half, -half, 0 };
	vs[2].normal = { 0, 0, 1 };

	Edge* e0 = addEdge(&vs[0], &vs[1]);
	Edge* e1 = addEdge(&vs[1], &vs[2]);
	Edge* e2 = addEdge(&vs[2], &vs[0]);

 	Poly& tris = addTris(vs[0], vs[1], vs[2], *e0, *e1, *e2);
	tris.normal = { 0, 0, 1 };
}

/*
		4--------5
	   /|       /|
	  / |      / |
	 0--------1  |
	 |  7-----|--6
	 | /      | /
	 |/       |/
	 3--------2
*/

void SculptMesh::createAsCube(float size)
{
	VertexBoundingBox& root = aabbs.emplace_back();
	root.parent = nullptr;
	root.vs.resize(8);

	auto& vs = root.vs;

	float half = size / 2;

	// Front
	vs[0].pos = { -half,  half, half };
	vs[1].pos = {  half,  half, half };
	vs[2].pos = {  half, -half, half };
	vs[3].pos = { -half, -half, half };

	// Back
	vs[4].pos = { -half,  half, -half };
	vs[5].pos = {  half,  half, -half };
	vs[6].pos = {  half, -half, -half };
	vs[7].pos = { -half, -half, -half };

	// Front
	Edge* e01 = addEdge(&vs[0], &vs[1]);
	Edge* e12 = addEdge(&vs[1], &vs[2]);
	Edge* e23 = addEdge(&vs[2], &vs[3]);
	Edge* e30 = addEdge(&vs[3], &vs[0]);

	// Back
	Edge* e45 = addEdge(&vs[4], &vs[5]);
	Edge* e56 = addEdge(&vs[5], &vs[6]);
	Edge* e67 = addEdge(&vs[6], &vs[7]);
	Edge* e74 = addEdge(&vs[7], &vs[4]);

	// Left
	Edge* e04 = addEdge(&vs[0], &vs[4]);
	Edge* e37 = addEdge(&vs[3], &vs[7]);

	// Right
	Edge* e15 = addEdge(&vs[1], &vs[5]);
	Edge* e26 = addEdge(&vs[2], &vs[6]);

	Poly& front = addQuad(vs[0], vs[1], vs[2], vs[3],
		*e01, *e12, *e23, *e30);

	Poly& right = addQuad(vs[1], vs[5], vs[6], vs[2],
		*e15, *e56, *e26, *e12);

	Poly& back = addQuad(vs[5], vs[4], vs[7], vs[6],
		*e45, *e74, *e67, *e56);

	Poly& left = addQuad(vs[4], vs[0], vs[3], vs[7],
		*e04, *e30, *e37, *e74);

	Poly& top = addQuad(vs[0], vs[4], vs[5], vs[1],
		*e04, *e45, *e15, *e01);

	Poly& bot = addQuad(vs[3], vs[2], vs[6], vs[7],
		*e23, *e26, *e67, *e37);

	front.normal = { 0, 0, 1 };
	back.normal = { 0, 0, -1 };
	right.normal = { 1, 0, 0 };
	left.normal = { -1, 0, 0 };
	top.normal = { 0, 1, 0 };
	bot.normal = { 0, -1, 0 };

	// Vertex Normals
	for (Vertex& v : vs) {
		v.calcNormalFromPolyNormals();
	}
}

void SculptMesh::addFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
	bool flip_winding)
{
	VertexBoundingBox& root = aabbs.front();
	uint32_t old_size = root.vs.size();
	root.vs.resize(old_size + positions.size());

	std::vector<Vertex>& vs = root.vs;

	for (uint32_t i = 0; i < positions.size(); i++) {

		Vertex& v = vs[old_size + i];
		v.pos = positions[i];
		v.normal = normals[i];
	}

	if (flip_winding) {
		for (uint32_t i = 0; i < indexes.size(); i += 3) {

			uint32_t idx_0 = old_size + indexes[i];
			uint32_t idx_1 = old_size + indexes[i + 1];
			uint32_t idx_2 = old_size + indexes[i + 2];
			addTrisNormalWinding(&vs[idx_2], &vs[idx_1], &vs[idx_0]);
		}
	}
	else {
		for (uint32_t i = 0; i < indexes.size(); i += 3) {

			uint32_t idx_0 = old_size + indexes[i];
			uint32_t idx_1 = old_size + indexes[i + 1];
			uint32_t idx_2 = old_size + indexes[i + 2];
			addTrisNormalWinding(&vs[idx_0], &vs[idx_1], &vs[idx_2]);
		}
	}
}

void SculptMesh::addFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions,
	bool flip_winding)
{
	VertexBoundingBox& root = aabbs.front();
	uint32_t old_size = root.vs.size();
	root.vs.resize(old_size + positions.size());

	std::vector<Vertex>& vs = root.vs;

	for (uint32_t i = 0; i < positions.size(); i++) {

		Vertex& v = vs[old_size + i];
		v.pos = positions[i];
	}

	if (flip_winding) {
		for (uint32_t i = 0; i < indexes.size(); i += 3) {

			uint32_t idx_0 = old_size + indexes[i];
			uint32_t idx_1 = old_size + indexes[i + 1];
			uint32_t idx_2 = old_size + indexes[i + 2];
			Poly* new_tris = addTris(&vs[idx_2], &vs[idx_1], &vs[idx_0]);
			new_tris->calcNormalForTris();
		}
	}
	else {
		for (uint32_t i = 0; i < indexes.size(); i += 3) {

			uint32_t idx_0 = old_size + indexes[i];
			uint32_t idx_1 = old_size + indexes[i + 1];
			uint32_t idx_2 = old_size + indexes[i + 2];
			Poly* new_tris = addTris(&vs[idx_0], &vs[idx_1], &vs[idx_2]);
			new_tris->calcNormalForTris();
		}
	}
	
	for (uint32_t i = old_size; i < vs.size(); i++) {
		vs[i].calcNormalFromPolyNormals();
	}
}
