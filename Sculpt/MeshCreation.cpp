
// Header
#include "SculptMesh.hpp"


using namespace scme;

void SculptMesh::createAsTriangle(float size, uint32_t max_vertices_AABB)
{
	verts.resize(3);

	float half = size / 2;
	verts[0].pos = { 0, half, 0 };
	verts[1].pos = { half, -half, 0 };
	verts[2].pos = { -half, -half, 0 };

	for (uint8_t i = 0; i < 3; i++) {
		verts[i].init();

		markVertexFullUpdate(i);
	}

	addTris(0, 1, 2);

	recreateAABBs(max_vertices_AABB);
}

void SculptMesh::createAsQuad(float size, uint32_t max_vertices_AABB)
{
	verts.resize(4);

	float half = size / 2;
	verts[0].pos = { -half, half, 0 };
	verts[1].pos = { half, half, 0 };
	verts[2].pos = { half, -half, 0 };
	verts[3].pos = { -half, -half, 0 };

	for (uint8_t i = 0; i < 4; i++) {
		verts[i].init();

		markVertexFullUpdate(i);
	}

	addQuad(0, 1, 2, 3);

	recreateAABBs(max_vertices_AABB);
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

void SculptMesh::createAsCube(float size, uint32_t max_vertices_AABB)
{
	verts.resize(8);

	float half = size / 2;

	// Front
	verts[0].pos = { -half,  half, half };
	verts[1].pos = { half,  half, half };
	verts[2].pos = { half, -half, half };
	verts[3].pos = { -half, -half, half };
	
	// Back
	verts[4].pos = { -half,  half, -half };
	verts[5].pos = { half,  half, -half };
	verts[6].pos = { half, -half, -half };
	verts[7].pos = { -half, -half, -half };

	for (uint8_t i = 0; i < verts.size(); i++) {
		verts[i].init();

		markVertexFullUpdate(i);
	}

	addQuad(0, 1, 2, 3);  // front
	addQuad(1, 5, 6, 2);  // right
	addQuad(5, 4, 7, 6);  // back
	addQuad(4, 0, 3, 7);  // left
	addQuad(0, 4, 5, 1);  // top
	addQuad(3, 2, 6, 7);  // bot

	recreateAABBs(max_vertices_AABB);
}

void SculptMesh::createAsCylinder(float height, float diameter, uint32_t rows, uint32_t cols, bool capped,
	uint32_t max_vertices_AABB)
{
	float radius = diameter / 2.f;

	uint32_t vertex_count = rows * cols;
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

			uint32_t idx = row_offset + col;
			Vertex& v = verts[idx];

			float col_ratio = ((float)col / cols) * (2.f * glm::pi<float>());
			float cosine = std::cosf(col_ratio);
			float sine = std::sinf(col_ratio);

			v.init();
			v.pos.x = cosine * radius;
			v.pos.z = -(sine * radius);
			v.pos.y = y;

			markVertexFullUpdate(idx);
		}

		y -= height_step;
	}

	if (capped) {

		std::vector<uint32_t> rim(cols);

		auto iter = verts.end();
		iter.prev();

		// bot cap
		{
			uint32_t bot_idx = iter.index();

			Vertex& bot_vertex = verts[bot_idx];
			bot_vertex.init();
			bot_vertex.pos = { 0, -height, 0 };

			markVertexFullUpdate(bot_idx);
			{
				uint32_t row_offset = (rows - 1) * cols;

				uint32_t col = cols - 1;
				for (auto& v : rim) {
					v = row_offset + col;
					col--;
				}

				stichVerticesToVertexLooped(rim, bot_idx);
			}
		}	

		iter.prev();

		// top cap
		{
			uint32_t top_idx = iter.index();

			Vertex& top_vertex = verts[top_idx];
			top_vertex.init();
			top_vertex.pos = { 0, 0, 0 };

			markVertexFullUpdate(top_idx);

			for (uint32_t col = 0; col < cols; col++) {
				rim[col] = col;
			}

			stichVerticesToVertexLooped(rim, top_idx);
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

			uint32_t v0_idx = (row * cols) + col;
			uint32_t v1_idx = (row * cols) + col + 1;
			uint32_t v2_idx = (row + 1) * cols + col + 1;
			uint32_t v3_idx = (row + 1) * cols + col;

			setQuad(quad_idx, v0_idx, v1_idx, v2_idx, v3_idx);
			
			quad_idx++;
		}

		uint32_t v0_idx = (row * cols) + cols - 1;
		uint32_t v1_idx = (row * cols);
		uint32_t v2_idx = (row + 1) * cols;
		uint32_t v3_idx = (row + 1) * cols + cols - 1;

		setQuad(quad_idx, v0_idx, v1_idx, v2_idx, v3_idx);

		quad_idx++;
	}

	recreateAABBs(max_vertices_AABB);

	// make sure I got the counts right
	assert_cond(polys.capacity() == quad_count + tris_count, "");
}

void SculptMesh::createAsUV_Sphere(float diameter, uint32_t rows, uint32_t cols, uint32_t max_vertices_AABB)
{
	assert_cond(rows > 1, "");
	assert_cond(cols > 1, "");

	float radius = diameter / 2.f;

	uint32_t vertex_count = rows * cols + 2;
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
			uint32_t v_idx = row_offset + col;
			Vertex& v = verts[v_idx];

			float col_ratio = ((float)col / cols) * (2.f * glm::pi<float>());
			float cosine = std::cosf(col_ratio);
			float sine = std::sinf(col_ratio);
		
			v.init();
			v.pos.x = cosine * row_radius;
			v.pos.z = -(sine * row_radius);
			v.pos.y = y;

			markVertexFullUpdate(v_idx);
		}
	}

	// Capped
	{
		std::vector<uint32_t> rim(cols);

		auto iter = verts.end();
		iter.prev();

		// bot cap
		{
			uint32_t bot_idx = iter.index();

			Vertex& bot_vertex = verts[bot_idx];
			bot_vertex.init();
			bot_vertex.pos = { 0, -radius, 0 };

			markVertexFullUpdate(bot_idx);
			{
				uint32_t row_offset = (rows - 1) * cols;

				uint32_t col = cols - 1;
				for (auto& v : rim) {
					v = row_offset + col;
					col--;
				}

				stichVerticesToVertexLooped(rim, bot_idx);
			}
		}

		iter.prev();

		// top cap
		{
			uint32_t top_idx = iter.index();

			Vertex& top_vertex = verts[top_idx];
			top_vertex.init();
			top_vertex.pos = { 0, radius, 0 };

			markVertexFullUpdate(top_idx);

			for (uint32_t col = 0; col < cols; col++) {
				rim[col] = col;
			}

			stichVerticesToVertexLooped(rim, top_idx);
		}
	}

	// Create Quads
	uint32_t quad_idx = 0;

	for (uint32_t row = 0; row < rows - 1; row++) {
		for (uint32_t col = 0; col < cols - 1; col++) {

			uint32_t v0_idx = (row * cols) + col;
			uint32_t v1_idx = (row * cols) + col + 1;
			uint32_t v2_idx = (row + 1) * cols + col + 1;
			uint32_t v3_idx = (row + 1) * cols + col;

			setQuad(quad_idx, v0_idx, v1_idx, v2_idx, v3_idx);

			quad_idx++;
		}

		uint32_t v0_idx = (row * cols) + cols - 1;
		uint32_t v1_idx = (row * cols);
		uint32_t v2_idx = (row + 1) * cols;
		uint32_t v3_idx = (row + 1) * cols + cols - 1;

		setQuad(quad_idx, v0_idx, v1_idx, v2_idx, v3_idx);

		quad_idx++;
	}

	recreateAABBs(max_vertices_AABB);

	// make sure I got the counts right
	assert_cond(polys.capacity() == quad_count + tris_count, "");
}

void SculptMesh::createFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions,
	std::vector<glm::vec3>& normals, uint32_t max_vertices_AABB)
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

	recreateAABBs(max_vertices_AABB);
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
