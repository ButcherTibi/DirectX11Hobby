
// Header
#include "SculptMesh.hpp"


using namespace scme;


void SculptMesh::createAsTriangle(float size)
{
	verts.resize(4);

	float half = size / 2;
	verts[0].normal.x = 999'999.f;
	verts[1].pos = { 0, half, 0 };
	verts[2].pos = { half, -half, 0 };
	verts[3].pos = { -half, -half, 0 };

	for (uint8_t i = 1; i < 4; i++) {
		verts[i].init();
		verts[i].normal = { 0, 0, 1 };

		markVertexFullUpdate(i);
	}

	addTris(1, 2, 3);

	VertexBoundingBox& root = aabbs.emplace_back();
	root.parent = 0xFFFF'FFFF;
	root.children[0] = 0xFFFF'FFFF;
	root.aabb.max = { half, half, half };
	root.aabb.min = { -half, -half, -half };
	root.verts_deleted_count = 0;

	root_aabb = 0;

	for (uint32_t i = 1; i < verts.size(); i++) {
		registerVertexToAABBs(i, 0);
	}
}

void SculptMesh::createAsQuad(float size)
{
	verts.resize(5);

	float half = size / 2;
	verts[0].normal.x = 999'999.f;
	verts[1].pos = { -half, half, 0 };
	verts[2].pos = { half, half, 0 };
	verts[3].pos = { half, -half, 0 };
	verts[4].pos = { -half, -half, 0 };

	for (uint8_t i = 1; i < 5; i++) {
		verts[i].init();
		verts[i].normal = { 0, 0, 1 };

		markVertexFullUpdate(i);
	}

	addQuad(1, 2, 3, 4);

	VertexBoundingBox& octree = aabbs.emplace_back();
	octree.parent = 0xFFFF'FFFF;
	octree.children[0] = 0xFFFF'FFFF;
	octree.aabb.max = { half, half, half };
	octree.aabb.min = { -half, -half, -half };
	octree.verts_deleted_count = 0;

	root_aabb = 0;

	for (uint32_t i = 1; i < verts.size(); i++) {
		registerVertexToAABBs(i, 0);
	}
}

/*
		5--------6
	   /|       /|
	  / |      / |
	 1--------2  |
	 |  8-----|--7
	 | /      | /
	 |/       |/
	 4--------3
*/

void SculptMesh::createAsCube(float size)
{
	verts.resize(9);

	float half = size / 2;

	// Front
	verts[0].normal.x = 999'999.f;
	verts[1].pos = { -half,  half, half };
	verts[2].pos = {  half,  half, half };
	verts[3].pos = {  half, -half, half };
	verts[4].pos = { -half, -half, half };
	
	// Back
	verts[5].pos = { -half,  half, -half };
	verts[6].pos = {  half,  half, -half };
	verts[7].pos = {  half, -half, -half };
	verts[8].pos = { -half, -half, -half };

	for (uint8_t i = 1; i < verts.size(); i++) {
		verts[i].init();

		markVertexFullUpdate(i);
	}

	addQuad(1, 2, 3, 4);  // front
	addQuad(2, 6, 7, 3);  // right
	addQuad(6, 5, 8, 7);  // back
	addQuad(5, 1, 4, 8);  // left
	addQuad(1, 5, 6, 2);  // top
	addQuad(4, 3, 7, 8);  // bot

	VertexBoundingBox& octree = aabbs.emplace_back();
	octree.parent = 0xFFFF'FFFF;
	octree.children[0] = 0xFFFF'FFFF;
	octree.aabb.max = { half, half, half };
	octree.aabb.min = { -half, -half, -half };
	octree.verts_deleted_count = 0;

	root_aabb = 0;

	for (uint32_t i = 1; i < verts.size(); i++) {
		registerVertexToAABBs(i, 0);
	}
}

void SculptMesh::createAsCylinder(float height, float diameter, uint32_t rows, uint32_t cols, bool capped)
{
	float radius = diameter / 2.f;

	// AABB
	{
		VertexBoundingBox& octree = aabbs.emplace_back();
		octree.parent = 0xFFFF'FFFF;
		octree.children[0] = 0xFFFF'FFFF;
		octree.aabb.max = { radius, radius, radius };
		octree.aabb.min = { -radius, -radius, -radius };
		octree.verts_deleted_count = 0;
	}

	uint32_t vertex_count = rows * cols + 1;
	uint32_t quad_count = (rows - 1) * cols;
	uint32_t tris_count = 0;

	if (capped) {
		vertex_count += 2;
		tris_count = cols * 2;
	}

	verts.resize(vertex_count);
	polys.reserve(quad_count + tris_count);
	polys.resize(quad_count);

	float height_step = height / (rows - 1);
	float y = 0;

	for (uint32_t row = 0; row < rows; row++) {

		uint32_t row_offset = row * cols;

		for (uint32_t col = 0; col < cols; col++) {

			uint32_t idx = 1 + row_offset + col;
			Vertex& v = verts[idx];

			float col_ratio = ((float)col / cols) * (2.f * glm::pi<float>());
			float cosine = std::cosf(col_ratio);
			float sine = std::sinf(col_ratio);

			v.init();
			v.pos.x = cosine * radius;
			v.pos.z = -(sine * radius);
			v.pos.y = y;

			registerVertexToAABBs(idx);
			markVertexFullUpdate(idx);
		}

		y -= height_step;
	}

	if (capped) {

		uint32_t top_idx = verts.size() - 2;
		
		Vertex& top_vertex = verts[top_idx];
		top_vertex.init();
		top_vertex.pos = { 0, 0, 0 };

		registerVertexToAABBs(top_idx);
		markVertexFullUpdate(top_idx);

		// top cap
		std::vector<uint32_t> rim(cols);
		{
			for (uint32_t col = 0; col < cols; col++) {
				rim[col] = 1 + col;
			}

			stichVerticesToVertexLooped(rim, top_idx);
		}

		// bot cap
		uint32_t bot_idx = verts.size() - 1;

		Vertex& bot_vertex = verts[bot_idx];
		bot_vertex.init();
		bot_vertex.pos = { 0, -height, 0 };

		registerVertexToAABBs(bot_idx);
		markVertexFullUpdate(bot_idx);
		{
			uint32_t row_offset = 1 + (rows - 1) * cols;

			uint32_t col = cols - 1;
			for (auto& v : rim) {
				v = row_offset + col;
				col--;
			}

			stichVerticesToVertexLooped(rim, bot_idx);
		}
	}

	// make origin top -> center
	float half_heigth = height / 2.f;

	for (auto iter = verts.begin(); iter != verts.end(); iter.next()) {
		Vertex& v = iter.get();
		v.pos.y += half_heigth;
	}

	// Create Quads
	uint32_t quad_idx = 0;

	for (uint32_t row = 0; row < rows - 1; row++) {
		for (uint32_t col = 0; col < cols - 1; col++) {

			uint32_t v0_idx = 1 + (row * cols) + col;
			uint32_t v1_idx = 1 + (row * cols) + col + 1;
			uint32_t v2_idx = 1 + (row + 1) * cols + col + 1;
			uint32_t v3_idx = 1 + (row + 1) * cols + col;

			setQuad(quad_idx, v0_idx, v1_idx, v2_idx, v3_idx);
			
			quad_idx++;
		}

		uint32_t v0_idx = 1 + (row * cols) + cols - 1;
		uint32_t v1_idx = 1 + (row * cols);
		uint32_t v2_idx = 1 + (row + 1) * cols;
		uint32_t v3_idx = 1 + (row + 1) * cols + cols - 1;

		setQuad(quad_idx, v0_idx, v1_idx, v2_idx, v3_idx);

		quad_idx++;
	}

	// make sure I got the counts right
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
	}

	uint32_t vertex_count = 1 + rows * cols + 2;
	uint32_t quad_count = (rows - 1) * cols;
	uint32_t tris_count = cols * 2;

	verts.resize(vertex_count);

	// can't be bothered to manually pass indexes when doing cap stiching
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
			uint32_t v_idx = 1 + row_offset + col;
			Vertex& v = verts[v_idx];

			float col_ratio = ((float)col / cols) * (2.f * glm::pi<float>());
			float cosine = std::cosf(col_ratio);
			float sine = std::sinf(col_ratio);
		
			v.init();
			v.pos.x = cosine * row_radius;
			v.pos.z = -(sine * row_radius);
			v.pos.y = y;

			registerVertexToAABBs(v_idx, 0);
			markVertexFullUpdate(v_idx);
		}
	}

	// top cap
	std::vector<uint32_t> rim(cols);
	{
		for (uint32_t col = 0; col < cols; col++) {
			rim[col] = 1 + col;
		}

		uint32_t v_idx = verts.size() - 2;
		Vertex& top_vertex = verts[v_idx];
		top_vertex.init();
		top_vertex.pos = { 0, radius, 0};

		stichVerticesToVertexLooped(rim, v_idx);

		registerVertexToAABBs(v_idx, 0);
		markVertexFullUpdate(v_idx);
	}

	// bot cap
	{
		uint32_t row_offset = 1 + (rows - 1) * cols;

		uint32_t col = cols - 1;
		for (auto& v : rim) {
			v = row_offset + col;
			col--;
		}

		uint32_t v_idx = verts.size() - 1;
		Vertex& bot_vertex = verts[v_idx];
		bot_vertex.init();
		bot_vertex.pos = { 0, -radius, 0 };

		stichVerticesToVertexLooped(rim, v_idx);

		registerVertexToAABBs(v_idx, 0);
		markVertexFullUpdate(v_idx);
	}

	// Create Quads
	uint32_t quad_idx = 0;

	for (uint32_t row = 0; row < rows - 1; row++) {
		for (uint32_t col = 0; col < cols - 1; col++) {

			uint32_t v0_idx = 1 + (row * cols) + col;
			uint32_t v1_idx = 1 + (row * cols) + col + 1;
			uint32_t v2_idx = 1 + (row + 1) * cols + col + 1;
			uint32_t v3_idx = 1 + (row + 1) * cols + col;

			setQuad(quad_idx, v0_idx, v1_idx, v2_idx, v3_idx);

			quad_idx++;
		}

		uint32_t v0_idx = 1 + (row * cols) + cols - 1;
		uint32_t v1_idx = 1 + (row * cols);
		uint32_t v2_idx = 1 + (row + 1) * cols;
		uint32_t v3_idx = 1 + (row + 1) * cols + cols - 1;

		setQuad(quad_idx, v0_idx, v1_idx, v2_idx, v3_idx);

		quad_idx++;
	}

	// make sure I got the counts right
	assert_cond(polys.capacity() == quad_count + tris_count, "");
}

void SculptMesh::createFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals)
{
	verts.resize(positions.size());
	polys.resize(indexes.size() / 3);

	float root_aabb_size = 0;

	for (uint32_t i = 0; i < positions.size(); i++) {

		Vertex& v = verts[i];
		v.init();
		v.pos = positions[i];
		v.normal = normals[i];

		markVertexFullUpdate(i);

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

		root_aabb = 0;

		for (uint32_t i = 0; i < verts.size(); i++) {
			registerVertexToAABBs(i, 0);
		}
	}

	uint32_t tris_idx = 0;
	for (uint32_t i = 0; i < indexes.size(); i += 3) {
		
		uint32_t v0_idx = indexes[i];
		uint32_t v1_idx = indexes[i + 1];
		uint32_t v2_idx = indexes[i + 2];

		Vertex* v0 = &verts[v0_idx];
		Vertex* v1 = &verts[v1_idx];
		Vertex* v2 = &verts[v2_idx];

		glm::vec3 normal = (v0->normal + v1->normal + v2->normal) / 3.f;
		glm::vec3 winding_normal = calcWindingNormal(v0, v1, v2);

		if (glm::dot(normal, winding_normal) > 0) {
			setTris(tris_idx, v0_idx, v1_idx, v2_idx);
		}
		else {
			setTris(tris_idx, v2_idx, v1_idx, v0_idx);
		}

		tris_idx++;
	}

	[]() {};
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
		vertex.init();
		vertex.normal = { 0.f, 0.f, 0.f };
	}

	addTris(0, 1, 2);
}

void SculptMesh::changeLineOrigin(glm::vec3& new_origin)
{
	verts[0].pos = new_origin;
	verts[1].pos = new_origin;
}

void SculptMesh::changeLineDirection(glm::vec3& new_direction)
{
	glm::vec3& origin = verts[0].pos;
	float length = glm::distance(verts[0].pos, verts[2].pos);

	glm::vec3 target = origin + new_direction * length;

	verts[2].pos = target;
}
