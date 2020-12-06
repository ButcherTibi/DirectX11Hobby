
// Header
#include "SculptMesh.hpp"


using namespace scme;


void SculptMesh::calculateAllNormalsFromWindings()
{
	for (Poly& p : polys) {
		p.calcNormal();
	}

	for (VertexBoundingBox& aabb : aabbs) {
		for (Vertex& v : aabb.vs) {
			v.calcNormal();
		}
	}
}

void SculptMesh::createAsTriangle(float size)
{
	VertexBoundingBox& root = aabbs.emplace_back();
	root.vs.resize(3);
	auto& vs = root.vs;

	float half = size / 2;
	vs[0].pos = { 0, half, 0 };
	vs[1].pos = { half, -half, 0 };
	vs[2].pos = { -half, -half, 0 };

	Edge* e0 = addEdge(&vs[0], &vs[1]);
	Edge* e1 = addEdge(&vs[1], &vs[2]);
	Edge* e2 = addEdge(&vs[2], &vs[0]);

	Poly& tris = addTris(vs[0], vs[1], vs[2], *e0, *e1, *e2);
	
	calculateAllNormalsFromWindings();
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
		*e01, *e12, *e23, *e30, 0);

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

	calculateAllNormalsFromWindings();
}

void SculptMesh::createAsCylinder(float height, float diameter, uint32_t rows, uint32_t cols)
{
	VertexBoundingBox& root = aabbs.emplace_back();
	root.parent = nullptr;
	root.vs.resize(rows * cols + 2);
	auto& vs = root.vs;

	float radius = diameter / 2.f;
	float height_step = height / (rows - 1);
	float y = 0;

	for (uint32_t row = 0; row < rows; row++) {

		uint32_t row_offset = row * cols;

		for (uint32_t col = 0; col < cols; col++) {

			Vertex& v = vs[row_offset + col];

			float col_ratio = ((float)col / cols) * (2.f * glm::pi<float>());
			float cosine = std::cosf(col_ratio);
			float sine = std::sinf(col_ratio);

			v.pos.x = cosine * radius;
			v.pos.z = -(sine * radius);
			v.pos.y = y;
		}

		y -= height_step;
	}

	// top cap
	std::vector<Vertex*> loop(cols);
	{	
		for (uint32_t col = 0; col < cols; col++) {
			loop[col] = &vs[col];
		}

		Vertex& top_vertex = vs[vs.size() - 2];
		stichVerticesToVertex(&top_vertex, loop, true);
	}

	// bot cap
	{
		uint32_t row_offset = (rows - 1) * cols;

		uint32_t col = cols - 1;
		for (auto& v : loop) {
			v = &vs[row_offset + col];
			col--;
		}

		Vertex& bot_vertex = vs.back();
		bot_vertex.pos.y = -height;
		stichVerticesToVertex(&bot_vertex, loop, true);
	}

	// make origin top -> center
	float half_heigth = height / 2.f;
	for (Vertex& v : vs) {
		v.pos.y += half_heigth;
	}

	// Create Quads
	for (uint32_t row = 0; row < rows - 1; row++) {
		for (uint32_t col = 0; col < cols - 1; col++) {

			Vertex& v0 = vs[row * cols + col];
			Vertex& v1 = vs[row * cols + col + 1];
			Vertex& v2 = vs[(row + 1) * cols + col + 1];
			Vertex& v3 = vs[(row + 1) * cols + col];

			Poly* new_poly = addQuad(&v0, &v1, &v2, &v3);
			new_poly->calcNormalForQuad();
		}

		Vertex& v0 = vs[row * cols + cols - 1];
		Vertex& v1 = vs[row * cols];
		Vertex& v2 = vs[(row + 1) * cols];
		Vertex& v3 = vs[(row + 1) * cols + cols - 1];

		Poly* new_poly = addQuad(&v0, &v1, &v2, &v3);
	}

	calculateAllNormalsFromWindings();
}

void SculptMesh::createAsUV_Sphere(float diameter, uint32_t rows, uint32_t cols)
{
	VertexBoundingBox& root = aabbs.emplace_back();
	root.parent = nullptr;
	root.vs.resize(rows * cols + 2);
	auto& vs = root.vs;

	float radius = diameter / 2.f;

	for (uint32_t row = 0; row < rows; row++) {

		uint32_t row_offset = row * cols;

		float row_radius;
		float y;
		{
			float v_ratio = (float)(row + 1) / (rows + 1);
			row_radius = std::sinf(v_ratio * glm::pi<float>()) * radius; //  * radius;
			y = std::cosf(v_ratio * glm::pi<float>()) * radius;
		}

		for (uint32_t col = 0; col < cols; col++) {

			/* From StackOverflow:
			x = sin(Pi * m / M) * cos(2Pi * n / N);
			y = sin(Pi * m / M) * sin(2Pi * n / N);
			z = cos(Pi * m / M);*/

			Vertex& v = vs[row_offset + col];

			float col_ratio = ((float)col / cols) * (2.f * glm::pi<float>());
			float cosine = std::cosf(col_ratio);
			float sine = std::sinf(col_ratio);
	
			v.pos.x = cosine * row_radius;
			v.pos.z = -(sine * row_radius);
			v.pos.y = y;
		}
	}

	// top cap
	std::vector<Vertex*> loop(cols);
	{
		for (uint32_t col = 0; col < cols; col++) {
			loop[col] = &vs[col];
		}

		Vertex& top_vertex = vs[vs.size() - 2];
		top_vertex.pos = { 0, radius, 0};
		stichVerticesToVertex(&top_vertex, loop, true);
	}

	// bot cap
	{
		uint32_t row_offset = (rows - 1) * cols;

		uint32_t col = cols - 1;
		for (auto& v : loop) {
			v = &vs[row_offset + col];
			col--;
		}

		Vertex& bot_vertex = vs.back();
		bot_vertex.pos = { 0, -radius, 0 };
		stichVerticesToVertex(&bot_vertex, loop, true);
	}

	// Create Quads
	for (uint32_t row = 0; row < rows - 1; row++) {
		for (uint32_t col = 0; col < cols - 1; col++) {

			Vertex& v0 = vs[row * cols + col];
			Vertex& v1 = vs[row * cols + col + 1];
			Vertex& v2 = vs[(row + 1) * cols + col + 1];
			Vertex& v3 = vs[(row + 1) * cols + col];

			Poly* new_poly = addQuad(&v0, &v1, &v2, &v3);
			new_poly->calcNormalForQuad();
		}

		Vertex& v0 = vs[row * cols + cols - 1];
		Vertex& v1 = vs[row * cols];
		Vertex& v2 = vs[(row + 1) * cols];
		Vertex& v3 = vs[(row + 1) * cols + cols - 1];

		Poly* new_poly = addQuad(&v0, &v1, &v2, &v3);
	}

	calculateAllNormalsFromWindings();
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
		vs[i].calcNormal();
	}
}
