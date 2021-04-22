
// Header
#include "SculptMesh.hpp"


using namespace scme;


void Vertex::init()
{
	away_loop = 0xFFFF'FFFF;
	aabb = 0xFFFF'FFFF;
}

bool Vertex::isPoint()
{
	return away_loop == 0xFFFF'FFFF;
}

bool VertexBoundingBox::isUnused()
{
	return verts.size() - verts_deleted_count == 0 && children[0] == 0xFFFF'FFFF;
}

bool VertexBoundingBox::isLeaf()
{
	return children[0] == 0xFFFF'FFFF;
}

void SculptMesh::calcVertexNormal(Vertex* vertex)
{
	// Vertex& vertex = verts[v];

	if (vertex->away_loop == 0xFFFF'FFFF) {
		return;
	}

	uint32_t count = 0;
	vertex->normal = { 0, 0, 0 };

	uint32_t loop_idx = vertex->away_loop;
	Loop* loop = &loops[loop_idx];

	// Iterate around Vertex
	do {
		// Iterate around Loop
		uint32_t mirror_loop_idx = loop->mirror_loop;
		Loop* mirror_loop = &loops[mirror_loop_idx];

		do {
			count++;
			vertex->normal += polys[mirror_loop->poly].normal;

			mirror_loop_idx = mirror_loop->mirror_loop;
			mirror_loop = &loops[mirror_loop_idx];

		} while (mirror_loop_idx != loop->mirror_loop);
		
		loop_idx = loop->v_next_loop;
		loop = &loops[loop_idx];
	}
	while (loop_idx != vertex->away_loop);

	vertex->normal /= count;
	vertex->normal = glm::normalize(vertex->normal);
}

void SculptMesh::markVertexFullUpdate(uint32_t vertex)
{
	ModifiedVertex& modified_vertex = modified_verts.emplace_back();
	modified_vertex.idx = vertex;
	modified_vertex.state = ModifiedVertexState::UPDATE;
}

void SculptMesh::deleteVertex(uint32_t vertex_idx)
{
	Vertex* vertex = &verts[vertex_idx];

	// vertex is point
	if (vertex->away_loop == 0xFFFF'FFFF) {
		throw std::exception("TODO");
		return;
	}

	Loop* start_loop = &loops[vertex->away_loop];

	// iter around vertex
	// for each delete_loop
	//   remove delete_loop from mirror loop list

	// find all polygons atached
	// if triangle then delete
	// if quad then trim to triangle
}

void SculptMesh::transferVertexToAABB(uint32_t v, uint32_t dest_aabb)
{
	Vertex* vertex = &verts[v];
	VertexBoundingBox& destination_aabb = aabbs[dest_aabb];

	// remove from old AABB
	if (vertex->aabb != 0xFFFF'FFFF) {

		VertexBoundingBox& source_aabb = aabbs[vertex->aabb];

		source_aabb.verts_deleted_count -= 1;
		source_aabb.verts[vertex->idx_in_aabb] = 0xFFFF'FFFF;
	}

	vertex->aabb = dest_aabb;
	vertex->idx_in_aabb = destination_aabb.verts.size();
	destination_aabb.verts.push_back(v);
}

void SculptMesh::registerVertexToAABBs(uint32_t vertex_idx, uint32_t start_aabb)
{
	Vertex& vertex = verts[vertex_idx];

	uint32_t now_count = 1;
	std::array<uint32_t, 8> now_aabbs = {
		start_aabb
	};

	uint32_t next_count = 0;
	std::array<uint32_t, 8> next_aabbs;

	while (now_count) {

		for (uint32_t i = 0; i < now_count; i++) {

			uint32_t aabb_idx = now_aabbs[i];
			VertexBoundingBox* aabb = &aabbs[aabb_idx];

			if (aabb->aabb.isPositionInside(vertex.pos)) {

				// Leaf
				if (aabb->children[0] == 0xFFFF'FFFF) {

					uint32_t child_vertex_count = aabb->verts.size() - aabb->verts_deleted_count;

					// leaf not full
					if (child_vertex_count < max_vertices_in_AABB) {

						transferVertexToAABB(vertex_idx, aabb_idx);
						return;
					}
					// Subdivide
					else {
						uint32_t base_idx = aabbs.size();
						aabbs.resize(aabbs.size() + 8);

						aabb = &aabbs[aabb_idx];  // old AABB pointer invalidated because octrees resize
						glm::vec3& min = aabb->aabb.min;
						glm::vec3& max = aabb->aabb.max;

						float x_mid = (aabb->aabb.max.x + aabb->aabb.min.x) / 2.0f;
						float y_mid = (aabb->aabb.max.y + aabb->aabb.min.y) / 2.0f;
						float z_mid = (aabb->aabb.max.z + aabb->aabb.min.z) / 2.0f;

						/*	Top Down View
								   +------+------+   ^
								  /      /      /    |
								 /  0   /  1   /     Z X---->
								+------+------+
							   /      /      /       Y is Up
							  /   2  /   3  /        -Z is Forward
							 +------+------+         X is Right
						*/

						{
							// Top Forward Left
							AxisBoundingBox3D& box_0 = aabbs[base_idx].aabb;
							box_0.max = { x_mid, max.y, max.z };
							box_0.min = { min.x, y_mid, z_mid };

							// Top Forward Right
							AxisBoundingBox3D& box_1 = aabbs[base_idx + 1].aabb;
							box_1.max = { max.x, max.y, max.z };
							box_1.min = { x_mid, y_mid, z_mid };

							// Top Backward Left
							AxisBoundingBox3D& box_2 = aabbs[base_idx + 2].aabb;
							box_2.max = { x_mid, max.y, z_mid };
							box_2.min = { min.x, y_mid, min.z };

							// Top Backward Right
							AxisBoundingBox3D& box_3 = aabbs[base_idx + 3].aabb;
							box_3.max = { max.x, max.y, z_mid };
							box_3.min = { x_mid, y_mid, min.z };
						}

						{
							// Bot Forward Left
							AxisBoundingBox3D& box_0 = aabbs[base_idx + 4].aabb;
							box_0.max = { x_mid, y_mid, max.z };
							box_0.min = { min.x, min.y, z_mid };

							// Bot Forward Right
							AxisBoundingBox3D& box_1 = aabbs[base_idx + 5].aabb;
							box_1.max = { max.x, y_mid, max.z };
							box_1.min = { x_mid, min.y, z_mid };

							// Bot Backward Left
							AxisBoundingBox3D& box_2 = aabbs[base_idx + 6].aabb;
							box_2.max = { x_mid, y_mid, z_mid };
							box_2.min = { min.x, min.y, min.z };

							// Bot Backward Right
							AxisBoundingBox3D& box_3 = aabbs[base_idx + 7].aabb;
							box_3.max = { max.x, y_mid, z_mid };
							box_3.min = { x_mid, min.y, min.z };
						}

						bool found = false;

						for (i = 0; i < 8; i++) {

							uint32_t child_aabb_idx = base_idx + i;
							aabb->children[i] = child_aabb_idx;

							VertexBoundingBox& child_aabb = aabbs[child_aabb_idx];
							child_aabb.parent = aabb_idx;
							child_aabb.children[0] = 0xFFFF'FFFF;
							child_aabb.verts_deleted_count = 0;
							child_aabb.verts.reserve(max_vertices_in_AABB / 4);
							child_aabb._debug_show_tesselation = false;

							// transfer the vertex to one of child AABBs
							if (!found && child_aabb.aabb.isPositionInside(vertex.pos)) {
								transferVertexToAABB(vertex_idx, child_aabb_idx);
								found = true;
							}
						}

						assert_cond(found == true, "vertex was not found in subdivided leaf");

						// transfer the rest of vertices of the parent to children
						for (uint32_t aabb_vertex_idx : aabb->verts) {

							Vertex& aabb_vertex = verts[aabb_vertex_idx];

							for (i = 0; i < 8; i++) {

								uint32_t child_aabb_idx = base_idx + i;
								VertexBoundingBox& child_aabb = aabbs[child_aabb_idx];

								if (child_aabb.aabb.isPositionInside(aabb_vertex.pos)) {

									aabb_vertex.aabb = child_aabb_idx;
									aabb_vertex.idx_in_aabb = child_aabb.verts.size();

									child_aabb.verts.push_back(aabb_vertex_idx);
									break;
								}
							}
						}

						// remove vertices from parent
						aabb->verts_deleted_count = 0;
						aabb->verts.clear();
						return;
					}
				}
				// Schedule recursive call
				else {
					for (i = 0; i < 8; i++) {
						next_aabbs[next_count] = aabb->children[i];
						next_count++;
					}
					break;
				}
			}
		}

		now_aabbs.swap(next_aabbs);
		now_count = next_count;
		next_count = 0;

		// TODO:
		// - merge upward
		// - expand upward
		// - seek upward
	}
}

uint32_t SculptMesh::findLoopFromTo(uint32_t src_v, uint32_t target_v)
{
	Vertex& src_vertex = verts[src_v];

	// vertex is a point
	if (src_vertex.away_loop == 0xFFFF'FFFF) {
		return 0xFFFF'FFFF;
	}

	uint32_t loop_idx = src_vertex.away_loop;
	Loop* loop = &loops[loop_idx];

	do {
		if (loop->target_v == target_v) {
			return loop_idx;
		}

		loop_idx = loop->v_next_loop;
		loop = &loops[loop_idx];
	}
	while (loop_idx != src_vertex.away_loop);

	return 0xFFFF'FFFF;
}

glm::vec3 calcNormalForTrisPositions(Vertex* v0, Vertex* v1, Vertex* v2)
{
	glm::vec3 dir_0 = glm::normalize(v1->pos - v0->pos);
	glm::vec3 dir_1 = glm::normalize(v2->pos - v0->pos);

	return glm::normalize(-glm::cross(dir_0, dir_1));
}

void SculptMesh::registerLoopToSourceVertexList(uint32_t away_loop_idx, uint32_t vertex_idx)
{
	Loop& new_loop = loops[away_loop_idx];
	Vertex& vertex = verts[vertex_idx];

	// if vertex is point then vertex loop list is unused
	if (vertex.away_loop == 0xFFFF'FFFF) {

		new_loop.v_next_loop = away_loop_idx;
		vertex.away_loop = away_loop_idx;
	}
	else {
		Loop& old_loop = loops[vertex.away_loop];

		// new ---> next
		new_loop.v_next_loop = old_loop.v_next_loop;

		// old ---> new
		old_loop.v_next_loop = away_loop_idx;
	}
}

void SculptMesh::registerLoopToMirrorLoopList(uint32_t new_loop_idx, uint32_t existing_loop_idx)
{
	Loop& new_loop = loops[new_loop_idx];
	Loop& existing_loop = loops[existing_loop_idx];

	// new ---> next
	new_loop.mirror_loop = existing_loop.mirror_loop;

	// old ---> new
	existing_loop.mirror_loop = new_loop_idx;
}

uint32_t SculptMesh::createLoop(uint32_t src_v, uint32_t target_v)
{
	uint32_t new_loop_idx = (uint32_t)loops.size();
	Loop* new_loop = &loops.emplace_back();
	new_loop->target_v = src_v;
	new_loop->poly = 0xFFFF'FFFF;

	registerLoopToSourceVertexList(new_loop_idx, src_v);

	uint32_t reverse_loop = findLoopFromTo(target_v, src_v);
	if (reverse_loop != 0xFFFF'FFFF) {

		registerLoopToMirrorLoopList(new_loop_idx, reverse_loop);
	}
	else {
		new_loop->mirror_loop = new_loop_idx;  // point to itself
	}

	return new_loop_idx;
}

void SculptMesh::setLoop(uint32_t existing_loop_idx, uint32_t src_v, uint32_t target_v)
{
	Loop* existing_loop = &loops[existing_loop_idx];
	existing_loop->target_v = src_v;
	existing_loop->poly = 0xFFFF'FFFF;

	registerLoopToSourceVertexList(existing_loop_idx, src_v);

	uint32_t reverse_loop = findLoopFromTo(target_v, src_v);
	if (reverse_loop != 0xFFFF'FFFF) {

	}
	else {
		existing_loop->mirror_loop = 0xFFFF'FFFF;
	}
}

uint32_t SculptMesh::addLoop(uint32_t src_v, uint32_t target_v)
{
	uint32_t existing_loop = findLoopFromTo(src_v, target_v);
	if (existing_loop == 0xFFFF'FFFF) {
		return createLoop(src_v, target_v);
	}
	
	return existing_loop;
}

glm::vec3 SculptMesh::calcWindingNormal(Vertex* v0, Vertex* v1, Vertex* v2)
{
	return -glm::normalize(glm::cross(v1->pos - v0->pos, v2->pos - v0->pos));
}

void SculptMesh::recalcPolyNormal(Poly* poly)
{
	if (poly->is_tris) {

		std::array<Vertex*, 3> tris_vs;

		uint32_t i = 0;
		uint32_t loop_idx = poly->inner_loop;
		Loop* loop = &loops[loop_idx];

		do {
			tris_vs[i] = &verts[loop->target_v];
			i++;

			loop_idx = loop->v_next_loop;
			loop = &loops[loop_idx];
		}
		while (loop_idx != poly->inner_loop);

		poly->normal = calcWindingNormal(tris_vs[0], tris_vs[1], tris_vs[2]);
	}
	else {
		std::array<Vertex*, 4> quad_vs;

		uint32_t i = 0;
		uint32_t loop_idx = poly->inner_loop;
		Loop* loop = &loops[loop_idx];

		do {
			quad_vs[i] = &verts[loop->target_v];
			i++;

			loop_idx = loop->v_next_loop;
			loop = &loops[loop_idx];
		}
		while (loop_idx != poly->inner_loop);

		if (glm::distance(quad_vs[0]->pos, quad_vs[2]->pos) < glm::distance(quad_vs[1]->pos, quad_vs[3]->pos)) {

			poly->tesselation_type = 0;
			poly->tess_normals[0] = calcWindingNormal(quad_vs[0], quad_vs[1], quad_vs[2]);
			poly->tess_normals[1] = calcWindingNormal(quad_vs[0], quad_vs[2], quad_vs[3]);
		}
		else {
			poly->tesselation_type = 1;
			poly->tess_normals[0] = calcWindingNormal(quad_vs[0], quad_vs[1], quad_vs[3]);
			poly->tess_normals[1] = calcWindingNormal(quad_vs[1], quad_vs[2], quad_vs[3]);
		}

		poly->normal = glm::normalize((poly->tess_normals[0] + poly->tess_normals[1]) / 2.f);
	}
}

void SculptMesh::markPolyFullUpdate(uint32_t poly)
{
	ModifiedPoly& modified_poly = modified_polys.emplace_back();
	modified_poly.idx = poly;
	modified_poly.state = ModifiedPolyState::UPDATE;
}

uint32_t SculptMesh::addTris(uint32_t v0, uint32_t v1, uint32_t v2)
{
	uint32_t new_poly_idx;
	Poly& new_poly = this->polys.emplace(new_poly_idx);
	new_poly.is_tris = 1;

	uint32_t ls_idx[3];
	ls_idx[0] = addLoop(v0, v1);
	ls_idx[1] = addLoop(v1, v2);
	ls_idx[2] = addLoop(v2, v0);

	Loop* ls[3];
	ls[0] = &loops[ls_idx[0]];
	ls[1] = &loops[ls_idx[1]];
	ls[2] = &loops[ls_idx[2]];

	// Register poly to loops
	ls[0]->poly_next_loop = ls_idx[1];
	ls[0]->poly_prev_loop = ls_idx[2];

	ls[1]->poly_next_loop = ls_idx[2];
	ls[1]->poly_prev_loop = ls_idx[0];

	ls[2]->poly_next_loop = ls_idx[0];
	ls[2]->poly_prev_loop = ls_idx[1];

	for (uint8_t i = 0; i < 3; i++) {
		ls[i]->poly = new_poly_idx;
	}

	// Calculate tris normal
	Vertex* vertex_0 = &verts[ls[0]->target_v];
	Vertex* vertex_1 = &verts[ls[1]->target_v];
	Vertex* vertex_2 = &verts[ls[2]->target_v];

	new_poly.normal = calcWindingNormal(vertex_0, vertex_1, vertex_2);
	new_poly.inner_loop = ls_idx[0];

	markPolyFullUpdate(new_poly_idx);

	return new_poly_idx;
}

void SculptMesh::setTris(uint32_t tris, uint32_t l0, uint32_t l1, uint32_t l2,
	uint32_t v0, uint32_t v1, uint32_t v2)
{
	uint32_t new_poly_idx = tris;
	Poly& new_poly = polys[new_poly_idx];
	new_poly.is_tris = 1;
	new_poly.inner_loop = l0;

	setLoop(l0, v0, v1);
	setLoop(l1, v1, v2);
	setLoop(l2, v2, v0);

	Loop* ls[3];
	ls[0] = &loops[l0];
	ls[1] = &loops[l1];
	ls[2] = &loops[l2];

	// Register poly to loops
	ls[0]->poly_next_loop = l1;
	ls[0]->poly_prev_loop = l2;

	ls[1]->poly_next_loop = l2;
	ls[1]->poly_prev_loop = l0;

	ls[2]->poly_next_loop = l0;
	ls[2]->poly_prev_loop = l1;

	for (uint8_t i = 0; i < 3; i++) {
		ls[i]->poly = new_poly_idx;
	}

	// Calculate tris normal
	Vertex* vertex_0 = &verts[ls[0]->target_v];
	Vertex* vertex_1 = &verts[ls[1]->target_v];
	Vertex* vertex_2 = &verts[ls[2]->target_v];

	new_poly.normal = calcWindingNormal(vertex_0, vertex_1, vertex_2);

	markPolyFullUpdate(tris);
}

void SculptMesh::setQuad(uint32_t quad, uint32_t l0, uint32_t l1, uint32_t l2, uint32_t l3,
	uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
	uint32_t new_poly_idx = quad;
	Poly& new_poly = polys[new_poly_idx];
	new_poly.inner_loop = l0;
	new_poly.is_tris = 0;

	setLoop(l0, v0, v1);
	setLoop(l1, v1, v2);
	setLoop(l2, v2, v3);
	setLoop(l3, v3, v0);

	Loop* ls[4];
	ls[0] = &loops[l0];
	ls[1] = &loops[l1];
	ls[2] = &loops[l2];
	ls[3] = &loops[l3];

	// Register poly to loops
	ls[0]->poly_next_loop = l1;
	ls[0]->poly_prev_loop = l3;

	ls[1]->poly_next_loop = l2;
	ls[1]->poly_prev_loop = l0;

	ls[2]->poly_next_loop = l3;
	ls[2]->poly_prev_loop = l1;

	ls[3]->poly_next_loop = l0;
	ls[3]->poly_prev_loop = l2;

	for (uint8_t i = 0; i < 4; i++) {
		ls[i]->poly = new_poly_idx;
	}

	// Calculate quad normals
	Vertex* vertex_0 = &verts[ls[0]->target_v];
	Vertex* vertex_1 = &verts[ls[1]->target_v];
	Vertex* vertex_2 = &verts[ls[2]->target_v];
	Vertex* vertex_3 = &verts[ls[3]->target_v];

	if (glm::distance(vertex_0->pos, vertex_2->pos) < glm::distance(vertex_1->pos, vertex_3->pos)) {

		new_poly.tesselation_type = 0;
		new_poly.tess_normals[0] = calcWindingNormal(vertex_0, vertex_1, vertex_2);
		new_poly.tess_normals[1] = calcWindingNormal(vertex_0, vertex_2, vertex_3);
	}
	else {
		new_poly.tesselation_type = 1;
		new_poly.tess_normals[0] = calcWindingNormal(vertex_0, vertex_1, vertex_3);
		new_poly.tess_normals[1] = calcWindingNormal(vertex_1, vertex_2, vertex_3);
	}

	new_poly.normal = glm::normalize((new_poly.tess_normals[0] + new_poly.tess_normals[1]) / 2.f);

	markPolyFullUpdate(quad);
}

uint32_t SculptMesh::addQuad(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
	uint32_t new_poly_idx;
	Poly& new_poly = this->polys.emplace(new_poly_idx);
	new_poly.is_tris = 0;

	uint32_t ls_idx[4];
	ls_idx[0] = addLoop(v0, v1);
	ls_idx[1] = addLoop(v1, v2);
	ls_idx[2] = addLoop(v2, v3);
	ls_idx[3] = addLoop(v3, v0);

	Loop* ls[4];
	ls[0] = &loops[ls_idx[0]];
	ls[1] = &loops[ls_idx[1]];
	ls[2] = &loops[ls_idx[2]];
	ls[3] = &loops[ls_idx[3]];

	// Register poly to loops
	ls[0]->poly_next_loop = ls_idx[1];
	ls[1]->poly_next_loop = ls_idx[2];
	ls[2]->poly_next_loop = ls_idx[3];
	ls[3]->poly_next_loop = ls_idx[0];

	for (uint8_t i = 0; i < 4; i++) {
		ls[i]->poly = new_poly_idx;
	}

	// Calculate quad normals
	Vertex* vertex_0 = &verts[ls[0]->target_v];
	Vertex* vertex_1 = &verts[ls[1]->target_v];
	Vertex* vertex_2 = &verts[ls[2]->target_v];
	Vertex* vertex_3 = &verts[ls[3]->target_v];

	if (glm::distance(vertex_0->pos, vertex_2->pos) < glm::distance(vertex_1->pos, vertex_3->pos)) {

		new_poly.tesselation_type = 0;
		new_poly.tess_normals[0] = calcWindingNormal(vertex_0, vertex_1, vertex_2);
		new_poly.tess_normals[1] = calcWindingNormal(vertex_0, vertex_2, vertex_3);
	}
	else {
		new_poly.tesselation_type = 1;
		new_poly.tess_normals[0] = calcWindingNormal(vertex_0, vertex_1, vertex_3);
		new_poly.tess_normals[1] = calcWindingNormal(vertex_1, vertex_2, vertex_3);
	}

	new_poly.normal = glm::normalize((new_poly.tess_normals[0] + new_poly.tess_normals[1]) / 2.f);
	new_poly.inner_loop = ls_idx[0];

	markPolyFullUpdate(new_poly_idx);

	return new_poly_idx;
}

void SculptMesh::stichVerticesToVertexLooped(std::vector<uint32_t>& vertices, uint32_t target)
{
	uint32_t last = vertices.size() - 1;
	for (uint32_t i = 0; i < last; i++) {

		uint32_t v = vertices[i];
		uint32_t v_next = vertices[i + 1];
	
		addTris(v, target, v_next);
	}

	addTris(vertices[last], target, vertices[0]);
}

void SculptMesh::deletePoly(uint32_t poly)
{
	// @HERE
}
