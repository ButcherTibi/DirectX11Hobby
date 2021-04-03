
// Header
#include "SculptMesh.hpp"


using namespace scme;


void SculptMesh::createAsTriangle(float size)
{
	verts.resize(3);

	float half = size / 2;
	verts[0].pos = { 0, half, 0 };
	verts[1].pos = { half, -half, 0 };
	verts[2].pos = { -half, -half, 0 };

	for (uint8_t i = 0; i < 3; i++) {
		verts[i].normal = { 0, 0, 1 };
		verts[i].away_loop = 0xFFFF'FFFF;
		verts[i].aabb = 0xFFFF'FFFF;
	}

	addTris(0, 1, 2);

	VertexBoundingBox& root = aabbs.emplace_back();
	root.parent = 0xFFFF'FFFF;
	root.children[0] = 0xFFFF'FFFF;
	root.aabb.max = { half, half, half };
	root.aabb.min = { -half, -half, -half };
	root.verts_deleted_count = 0;

	root_aabb = 0;

	for (uint32_t i = 0; i < verts.size(); i++) {
		registerVertexToAABBs(i, 0);
	}
}

void SculptMesh::createAsQuad(float size)
{
	verts.resize(4);

	float half = size / 2;
	verts[0].pos = { -half, half, 0 };
	verts[1].pos = { half, half, 0 };
	verts[2].pos = { half, -half, 0 };
	verts[3].pos = { -half, -half, 0 };

	for (uint8_t i = 0; i < 4; i++) {
		verts[i].normal = { 0, 0, 1 };
		verts[i].away_loop = 0xFFFF'FFFF;
		verts[i].aabb = 0xFFFF'FFFF;
	}

	addQuad(0, 1, 2, 3);

	VertexBoundingBox& octree = aabbs.emplace_back();
	octree.parent = 0xFFFF'FFFF;
	octree.children[0] = 0xFFFF'FFFF;
	octree.aabb.max = { half, half, half };
	octree.aabb.min = { -half, -half, -half };
	octree.verts_deleted_count = 0;

	for (uint32_t i = 0; i < verts.size(); i++) {
		registerVertexToAABBs(i, 0);
	}
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
	verts.resize(8);

	float half = size / 2;

	// Front
	verts[0].pos = { -half,  half, half };
	verts[1].pos = {  half,  half, half };
	verts[2].pos = {  half, -half, half };
	verts[3].pos = { -half, -half, half };
	
	// Back
	verts[4].pos = { -half,  half, -half };
	verts[5].pos = {  half,  half, -half };
	verts[6].pos = {  half, -half, -half };
	verts[7].pos = { -half, -half, -half };

	for (uint8_t i = 0; i < 8; i++) {
		verts[i].away_loop = 0xFFFF'FFFF;
	}

	addQuad(0, 1, 2, 3);  // front
	addQuad(1, 5, 6, 2);  // right
	addQuad(5, 4, 7, 6);  // back
	addQuad(4, 0, 3, 7);  // left
	addQuad(0, 4, 5, 1);  // top
	addQuad(3, 2, 6, 7);  // bot

	for (uint8_t i = 0; i < 8; i++) {
		calcVertexNormal(&verts[i]);
	}
}

void SculptMesh::createAsCylinder(float height, float diameter, uint32_t rows, uint32_t cols, bool capped)
{
	uint32_t vertex_count = rows * cols;
	uint32_t quad_count = (rows - 1) * cols;
	uint32_t tris_count = 0;

	if (capped) {
		vertex_count += 2;
		tris_count = cols * 2;
	}

	verts.resize(vertex_count);

	// can't be bothered to manually pass indexes when doing cap stiching
	loops.reserve(quad_count * 4 + tris_count * 3);
	loops.resize(quad_count * 4);

	polys.reserve(quad_count + tris_count);
	polys.resize(quad_count);

	float radius = diameter / 2.f;
	float height_step = height / (rows - 1);
	float y = 0;

	for (uint32_t row = 0; row < rows; row++) {

		uint32_t row_offset = row * cols;

		for (uint32_t col = 0; col < cols; col++) {

			Vertex& v = verts[row_offset + col];

			float col_ratio = ((float)col / cols) * (2.f * glm::pi<float>());
			float cosine = std::cosf(col_ratio);
			float sine = std::sinf(col_ratio);

			v.pos.x = cosine * radius;
			v.pos.z = -(sine * radius);
			v.pos.y = y;
			v.away_loop = 0xFFFF'FFFF;
		}

		y -= height_step;
	}

	if (capped) {

		Vertex& top_vertex = verts[verts.size() - 2];
		top_vertex.pos = { 0, 0, 0 };
		top_vertex.init();

		Vertex& bot_vertex = verts.back();
		bot_vertex.pos = { 0, -height, 0 };
		bot_vertex.init();

		// top cap
		std::vector<uint32_t> rim(cols);
		{
			for (uint32_t col = 0; col < cols; col++) {
				rim[col] = col;
			}

			stichVerticesToVertexLooped(rim, verts.size() - 2);
		}

		// bot cap
		{
			uint32_t row_offset = (rows - 1) * cols;

			uint32_t col = cols - 1;
			for (auto& v : rim) {
				v = row_offset + col;
				col--;
			}

			stichVerticesToVertexLooped(rim, verts.size() - 1);
		}
	}

	// make origin top -> center
	float half_heigth = height / 2.f;
	for (Vertex& v : verts) {
		v.pos.y += half_heigth;
	}

	// Create Quads
	uint32_t loop_idx = 0;
	uint32_t quad_idx = 0;

	for (uint32_t row = 0; row < rows - 1; row++) {
		for (uint32_t col = 0; col < cols - 1; col++) {

			uint32_t v0_idx = row * cols + col;
			uint32_t v1_idx = row * cols + col + 1;
			uint32_t v2_idx = (row + 1) * cols + col + 1;
			uint32_t v3_idx = (row + 1) * cols + col;

			setQuad(quad_idx, loop_idx, loop_idx + 1, loop_idx + 2, loop_idx + 3,
				v0_idx, v1_idx, v2_idx, v3_idx);
			
			loop_idx += 4;
			quad_idx++;
		}

		uint32_t v0_idx = row * cols + cols - 1;
		uint32_t v1_idx = row * cols;
		uint32_t v2_idx = (row + 1) * cols;
		uint32_t v3_idx = (row + 1) * cols + cols - 1;

		setQuad(quad_idx, loop_idx, loop_idx + 1, loop_idx + 2, loop_idx + 3,
			v0_idx, v1_idx, v2_idx, v3_idx);

		loop_idx += 4;
		quad_idx++;
	}

	for (uint32_t i = 0; i < verts.size(); i++) {
		calcVertexNormal(&verts[i]);
	}

	// make sure I got the counts right
	assert_cond(verts.capacity() == vertex_count, "");
	assert_cond(loops.capacity() == quad_count * 4 + tris_count * 3, "");
	assert_cond(polys.capacity() == quad_count + tris_count, "");
}

void SculptMesh::createAsUV_Sphere(float diameter, uint32_t rows, uint32_t cols)
{
	assert_cond(rows > 1, "");
	assert_cond(cols > 1, "");

	float radius = diameter / 2.f;

	// AABB
	{
		VertexBoundingBox& octree = aabbs.emplace_back();
		octree.parent = 0xFFFF'FFFF;
		octree.children[0] = 0xFFFF'FFFF;
		octree.aabb.max = { radius, radius, radius };
		octree.aabb.min = { -radius, -radius, -radius };
		octree.verts_deleted_count = 0;
		octree._debug_show_tesselation = false;
	}

	uint32_t vertex_count = rows * cols + 2;
	uint32_t quad_count = (rows - 1) * cols;
	uint32_t tris_count = cols * 2;

	verts.resize(vertex_count);

	// can't be bothered to manually pass indexes when doing cap stiching
	loops.reserve(quad_count * 4 + tris_count * 3);
	loops.resize(quad_count * 4);

	polys.reserve(quad_count + tris_count);
	polys.resize(quad_count);

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
			uint32_t v_idx = row_offset + col;
			Vertex& v = verts[v_idx];

			float col_ratio = ((float)col / cols) * (2.f * glm::pi<float>());
			float cosine = std::cosf(col_ratio);
			float sine = std::sinf(col_ratio);
	
			v.pos.x = cosine * row_radius;
			v.pos.z = -(sine * row_radius);
			v.pos.y = y;
			v.away_loop = 0xFFFF'FFFF;
			v.aabb = 0xFFFF'FFFF;

			registerVertexToAABBs(v_idx, 0);
		}
	}

	// top cap
	std::vector<uint32_t> rim(cols);
	{
		for (uint32_t col = 0; col < cols; col++) {
			rim[col] = col;
		}

		uint32_t v_idx = verts.size() - 2;
		Vertex& top_vertex = verts[v_idx];
		top_vertex.pos = { 0, radius, 0};
		top_vertex.away_loop = 0xFFFF'FFFF;
		top_vertex.aabb = 0xFFFF'FFFF;

		stichVerticesToVertexLooped(rim, verts.size() - 2);

		registerVertexToAABBs(v_idx, 0);
	}

	// bot cap
	{
		uint32_t row_offset = (rows - 1) * cols;

		uint32_t col = cols - 1;
		for (auto& v : rim) {
			v = row_offset + col;
			col--;
		}

		Vertex& bot_vertex = verts.back();
		bot_vertex.pos = { 0, -radius, 0 };
		bot_vertex.away_loop = 0xFFFF'FFFF;
		bot_vertex.aabb = 0xFFFF'FFFF;

		stichVerticesToVertexLooped(rim, verts.size() - 1);

		registerVertexToAABBs(verts.size() - 1, 0);
	}

	// Create Quads
	uint32_t loop_idx = 0;
	uint32_t quad_idx = 0;

	for (uint32_t row = 0; row < rows - 1; row++) {
		for (uint32_t col = 0; col < cols - 1; col++) {

			uint32_t v0_idx = row * cols + col;
			uint32_t v1_idx = row * cols + col + 1;
			uint32_t v2_idx = (row + 1) * cols + col + 1;
			uint32_t v3_idx = (row + 1) * cols + col;

			setQuad(quad_idx, loop_idx, loop_idx + 1, loop_idx + 2, loop_idx + 3,
				v0_idx, v1_idx, v2_idx, v3_idx);

			loop_idx += 4;
			quad_idx++;
		}

		uint32_t v0_idx = row * cols + cols - 1;
		uint32_t v1_idx = row * cols;
		uint32_t v2_idx = (row + 1) * cols;
		uint32_t v3_idx = (row + 1) * cols + cols - 1;

		setQuad(quad_idx, loop_idx, loop_idx + 1, loop_idx + 2, loop_idx + 3,
			v0_idx, v1_idx, v2_idx, v3_idx);

		loop_idx += 4;
		quad_idx++;
	}

	for (uint32_t i = 0; i < verts.size(); i++) {
		calcVertexNormal(&verts[i]);
	}

	// make sure I got the counts right
	assert_cond(verts.capacity() == vertex_count, "");
	assert_cond(loops.capacity() == quad_count * 4 + tris_count * 3, "");
	assert_cond(polys.capacity() == quad_count + tris_count, "");
}

void SculptMesh::createFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals)
{
	verts.resize(positions.size());
	loops.resize(indexes.size());
	polys.resize(indexes.size() / 3);

	float root_aabb_size = 0;

	for (uint32_t i = 0; i < positions.size(); i++) {

		Vertex& v = verts[i];
		v.pos = positions[i];
		v.normal = normals[i];
		v.away_loop = 0xFFFF'FFFF;
		v.aabb = 0xFFFF'FFFF;

		if (std::abs(v.pos.x) > root_aabb_size) {
			root_aabb_size = v.pos.x;
		}

		if (std::abs(v.pos.y) > root_aabb_size) {
			root_aabb_size = v.pos.y;
		}

		if (std::abs(v.pos.z) > root_aabb_size) {
			root_aabb_size = v.pos.z;
		}
	}

	// AABBs
	{
		VertexBoundingBox& root = aabbs.emplace_back();
		root.parent = 0xFFFF'FFFF;
		root.children[0] = 0xFFFF'FFFF;
		root.aabb.min = { -root_aabb_size, -root_aabb_size, -root_aabb_size };
		root.aabb.max = { root_aabb_size, root_aabb_size, root_aabb_size };
		root.verts_deleted_count = 0;
		root._debug_show_tesselation = false;

		root_aabb = 0;

		for (uint32_t i = 0; i < verts.size(); i++) {
			registerVertexToAABBs(i, 0);
		}
	}

	for (uint32_t i = 0; i < indexes.size(); i += 3) {
		
		uint32_t tris = i / 3;

		uint32_t l0 = i;
		uint32_t l1 = i + 1;
		uint32_t l2 = i + 2;

		uint32_t v0_idx = indexes[i];
		uint32_t v1_idx = indexes[i + 1];
		uint32_t v2_idx = indexes[i + 2];

		Vertex* v0 = &verts[v0_idx];
		Vertex* v1 = &verts[v1_idx];
		Vertex* v2 = &verts[v2_idx];

		glm::vec3 normal = (v0->normal + v1->normal + v2->normal) / 3.f;
		glm::vec3 winding_normal = calcWindingNormal(v0, v1, v2);

		if (glm::dot(normal, winding_normal) > 0) {
			setTris(tris, l0, l1, l2, v0_idx, v1_idx, v2_idx);
		}
		else {
			setTris(tris, l0, l1, l2, v2_idx, v1_idx, v0_idx);
		}
	}
}

void SculptMesh::createAsLine(glm::vec3& origin, glm::vec3& direction, float length)
{
	glm::vec3 target = origin + direction * length;

	verts.resize(3);
	verts[0].pos = origin;
	verts[1].pos = origin;
	verts[2].pos = target;

	VertexBoundingBox& aabb = aabbs.emplace_back();
	aabb.parent = 0xFFFF'FFFF;
	aabb.children[0] = 0xFFFF'FFFF;
	aabb.aabb.min.x = std::min(origin.x, target.x);
	aabb.aabb.min.y = std::min(origin.y, target.y);
	aabb.aabb.min.z = std::min(origin.z, target.z);
	aabb.aabb.max.x = std::max(origin.x, target.x);
	aabb.aabb.max.y = std::max(origin.y, target.y);
	aabb.aabb.max.z = std::max(origin.z, target.z);
	aabb.children[0] = 0xFFFF'FFFF;
	aabb.verts_deleted_count = 0;
	aabb.verts = { 0, 1, 2 };

	for (uint32_t i = 0; i < 3; i++) {

		Vertex& vertex = verts[i];
		vertex.normal = { 0.f, 0.f, 0.f };
		vertex.away_loop = 0xFFFF'FFFF;
		vertex.aabb = 0xFFFF'FFFF;
	}

	addTris(0, 1, 2);
}

//void SculptMesh::addFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions,
//	bool flip_winding)
//{
//	VertexBoundingBox& root = aabbs.front();
//	uint32_t old_size = root.vs.size();
//	root.vs.resize(old_size + positions.size());
//
//	std::vector<Vertex>& vs = root.vs;
//
//	for (uint32_t i = 0; i < positions.size(); i++) {
//
//		Vertex& v = vs[old_size + i];
//		v.pos = positions[i];
//	}
//
//	if (flip_winding) {
//		for (uint32_t i = 0; i < indexes.size(); i += 3) {
//
//			uint32_t idx_0 = old_size + indexes[i];
//			uint32_t idx_1 = old_size + indexes[i + 1];
//			uint32_t idx_2 = old_size + indexes[i + 2];
//			Poly* new_tris = addTris(&vs[idx_2], &vs[idx_1], &vs[idx_0]);
//			new_tris->calcNormalForTris();
//		}
//	}
//	else {
//		for (uint32_t i = 0; i < indexes.size(); i += 3) {
//
//			uint32_t idx_0 = old_size + indexes[i];
//			uint32_t idx_1 = old_size + indexes[i + 1];
//			uint32_t idx_2 = old_size + indexes[i + 2];
//			Poly* new_tris = addTris(&vs[idx_0], &vs[idx_1], &vs[idx_2]);
//			new_tris->calcNormalForTris();
//		}
//	}
//	
//	for (uint32_t i = old_size; i < vs.size(); i++) {
//		vs[i].calcNormal();
//	}
//}

size_t SculptMesh::getMemorySizeMegaBytes()
{
	size_t aabbs_size = 0;
	for (auto& aabb : aabbs) {
		aabbs_size += aabb.verts.size() * sizeof(uint32_t) +
			sizeof(VertexBoundingBox);
	}

	size_t size_in_bytes = aabbs_size +
		verts.size() * sizeof(Vertex) +
		loops.size() * sizeof(Loop) +
		polys.size() * sizeof(Poly) +
		sizeof(SculptMesh);

	return size_in_bytes / (1024 * 1024);
}
