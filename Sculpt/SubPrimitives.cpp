
// Header
#include "SculptMesh.hpp"

#include <glm\geometric.hpp>


using namespace scme;


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
		if (loop->mirror_loop != 0xFFFF'FFFF) {

			uint32_t mirror_loop_idx = loop->mirror_loop;
			Loop* mirror_loop = &loops[mirror_loop_idx];

			do {
				count++;
				vertex->normal += polys[mirror_loop->poly].normal;

				mirror_loop_idx = mirror_loop->mirror_loop;
				mirror_loop = &loops[mirror_loop_idx];

			} while (mirror_loop_idx != loop->mirror_loop);
		}
		
		loop_idx = loop->v_next_loop;
		loop = &loops[loop_idx];
	}
	while (loop_idx != vertex->away_loop);

	vertex->normal /= count;
	vertex->normal = glm::normalize(vertex->normal);
}

void SculptMesh::transferVertexToAABB(uint32_t v, uint32_t dest_aabb)
{
	Vertex* vertex = &verts[v];
	VertexBoundingBox& destination_aabb = aabbs[dest_aabb];

	// remove from old AABB
	if (vertex->aabb != 0xFFFF'FFFF) {

		VertexBoundingBox& source_aabb = aabbs[vertex->aabb];

		source_aabb.deleted_count -= 1;
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
	uint32_t now_aabbs[64];

	uint32_t next_count = 0;
	uint32_t next_aabbs[64];

	now_aabbs[0] = start_aabb;

	do {
		for (uint32_t i = 0; i < now_count; i++) {

			uint32_t aabb_idx = now_aabbs[i];
			VertexBoundingBox& aabb = aabbs[aabb_idx];

			if (aabb.aabb.isPositionInside(vertex.pos)) {

				uint32_t child_vertex_count = aabb.verts.size() - aabb.deleted_count;

				// Leaf
				if (child_vertex_count) {

					// leaf not full
					if (child_vertex_count < max_vertices_in_AABB) {

						transferVertexToAABB(vertex_idx, aabb_idx);
						return;
					}
					// Subdivide
					else {
						uint32_t idx = aabbs.size();
						aabbs.resize(aabbs.size() + 8);

						glm::vec3& min = aabb.aabb.min;
						glm::vec3& max = aabb.aabb.max;

						float x_mid = (aabb.aabb.max.x + aabb.aabb.min.x) / 2.0f;
						float y_mid = (aabb.aabb.max.y + aabb.aabb.min.y) / 2.0f;
						float z_mid = (aabb.aabb.max.z + aabb.aabb.min.z) / 2.0f;

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
							AxisBoundingBox3D& box_0 = aabbs[idx].aabb;
							box_0.max = { x_mid, max.y, max.z };
							box_0.min = { min.x, y_mid, z_mid };

							// Top Forward Right
							AxisBoundingBox3D& box_1 = aabbs[idx + 1].aabb;
							box_1.max = { max.x, max.y, max.z };
							box_1.min = { x_mid, y_mid, z_mid };

							// Top Backward Left
							AxisBoundingBox3D& box_2 = aabbs[idx + 2].aabb;
							box_2.max = { x_mid, max.y, z_mid };
							box_2.min = { min.x, y_mid, min.z };

							// Top Backward Right
							AxisBoundingBox3D& box_3 = aabbs[idx + 3].aabb;
							box_3.max = { max.x, max.y, z_mid };
							box_3.min = { x_mid, y_mid, min.z };
						}

						{
							// Bot Forward Left
							AxisBoundingBox3D& box_0 = aabbs[idx + 4].aabb;
							box_0.max = { x_mid, y_mid, max.z };
							box_0.min = { min.x, min.y, z_mid };

							// Bot Forward Right
							AxisBoundingBox3D& box_1 = aabbs[idx + 5].aabb;
							box_1.max = { max.x, y_mid, max.z };
							box_1.min = { x_mid, min.y, z_mid };

							// Bot Backward Left
							AxisBoundingBox3D& box_2 = aabbs[idx + 6].aabb;
							box_2.max = { x_mid, y_mid, z_mid };
							box_2.min = { min.x, min.y, min.z };

							// Bot Backward Right
							AxisBoundingBox3D& box_3 = aabbs[idx + 7].aabb;
							box_3.max = { max.x, y_mid, z_mid };
							box_3.min = { x_mid, min.y, min.z };
						}

						bool found = false;

						for (uint8_t i = 0; i < 8; i++) {

							uint32_t child_aabb_idx = idx + i;
							aabb.children[i] = child_aabb_idx;

							VertexBoundingBox& child_aabb = aabbs[child_aabb_idx];
							child_aabb.parent = aabb_idx;
							child_aabb.deleted_count = 0;
							child_aabb.verts.reserve(max_vertices_in_AABB / 4);

							// transfer the vertex to one of child AABBs
							if (!found && child_aabb.aabb.isPositionInside(vertex.pos)) {

								vertex.aabb = child_aabb_idx;
								vertex.idx_in_aabb = child_aabb.verts.size();

								child_aabb.verts.push_back(vertex_idx);
								found = true;
							}
						}

						// transfer the rest of vertices of the parent to children
						for (uint32_t aabb_vertex_idx : aabb.verts) {

							Vertex& aabb_vertex = verts[aabb_vertex_idx];

							for (uint8_t i = 0; i < 8; i++) {

								uint32_t child_aabb_idx = idx + i;
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
						aabb.deleted_count = 0;
						aabb.verts.clear();
						return;
					}
				}
				// Schedule recursive call
				else {
					for (uint8_t i = 0; i < 8; i++) {
						next_aabbs[next_count] = aabb.children[i];
						next_count++;
					}
				}
			}
		}

		std::memcpy(now_aabbs, next_aabbs, 8 * sizeof(uint32_t));
		now_count = next_count;
		next_count = 0;

		// TODO: 
		// searched_aabbs.push_back(now_aabbs[0].parent)
		// now_aabbs.push_back(now_aabbs[0].parent.parent)
		// 
		// if aabb not in searched_aabbs then . . .
	}
	while (now_count);
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

//void Vertex::calcNormal()
//{
//	uint32_t count = 0;
//	normal = { 0, 0, 0 };
//
//	for (Edge* e : edges) {
//		for (Poly* poly : e->polys) {
//			normal += poly->normal;
//			count++;
//		}
//	}
//
//	normal = glm::normalize(normal / (float)count);
//}

glm::vec3 calcNormalForTrisPositions(Vertex* v0, Vertex* v1, Vertex* v2)
{
	glm::vec3 dir_0 = glm::normalize(v1->pos - v0->pos);
	glm::vec3 dir_1 = glm::normalize(v2->pos - v0->pos);

	return glm::normalize(-glm::cross(dir_0, dir_1));
}
//
//void Poly::calcNormalForTris()
//{
//	this->normal = calcNormalForTrisPositions(vs[0], vs[1], vs[2]);
//}
//
//void Poly::calcNormalForQuad()
//{
//	glm::vec3 n0;
//	glm::vec3 n1;
//
//	if (tesselation_type == 0) {
//		n0 = calcNormalForTrisPositions(vs[0], vs[1], vs[2]);
//		n1 = calcNormalForTrisPositions(vs[0], vs[2], vs[3]);
//	}
//	else {
//		n0 = calcNormalForTrisPositions(vs[0], vs[1], vs[3]);
//		n1 = calcNormalForTrisPositions(vs[1], vs[2], vs[3]);
//	}
//
//	this->tess_normals[0] = n0;
//	this->tess_normals[1] = n1;
//	this->normal = glm::normalize((n0 + n1) / 2.f);
//}
//
//void Poly::calcNormal()
//{
//	if (vs[3] == nullptr) {
//		calcNormalForTris();
//	}
//	else {
//		calcNormalForQuad();
//	}
//}

//void SculptMesh::calcVertexNormal(Vertex* v)
//{
//	uint32_t count = 0;
//	v->normal = { 0, 0, 0 };
//	
//
//
//	for (Edge* e : edges) {
//		for (Poly* poly : e->polys) {
//			normal += poly->normal;
//			count++;
//		}
//	}
//	
//	v->normal = glm::normalize(v->normal / (float)count);
//}

//Edge* SculptMesh::addEdge(Vertex* v0, Vertex* v1)
//{
//	Edge& new_edge = this->edges.emplace_back();
//	new_edge.v0 = v0;
//	new_edge.v1 = v1;
//
//	// Vertices
//	v0->edges.push_back(&new_edge);
//	v1->edges.push_back(&new_edge);
//
//	return &new_edge;
//}

//Edge* SculptMesh::addEdgeIfNone(Vertex* v0, Vertex* v1)
//{
//	for (Edge* e0 : v0->edges) {
//
//		if (e0 != nullptr) {
//			for (Edge* e1 : v1->edges) {
//
//				if (e1 != nullptr && e0 == e1) {
//					return e0;
//				}
//			}
//		}
//	}
//
//	return addEdge(v0, v1);
//}

void SculptMesh::registerLoopToSourceVertexList(uint32_t away_loop_idx, uint32_t vertex_idx)
{
	Loop& new_loop = loops[away_loop_idx];
	Vertex& vertex = verts[vertex_idx];

	// if vertex is point then vertex loop list is unused
	if (vertex.away_loop == 0xFFFF'FFFF) {

		vertex.away_loop = away_loop_idx;

		new_loop.v_next_loop = away_loop_idx;
	}
	else {
		Loop& old_loop = loops[vertex.away_loop];
		Loop& next_loop = loops[old_loop.v_next_loop];

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

	new_loop.mirror_loop = existing_loop.mirror_loop;
	existing_loop.mirror_loop = new_loop_idx;
}

uint32_t SculptMesh::setLoop(uint32_t loop, uint32_t src_v, uint32_t target_v)
{
	uint32_t new_loop_idx;
	Loop* new_loop = nullptr;

	uint32_t existing_loop = findLoopFromTo(src_v, target_v);
	if (existing_loop != 0xFFFF'FFFF) {

		// reuse wire into loop
		Loop* loop = &loops[existing_loop];
		if (loop->poly == 0xFFFF'FFFF) {
			new_loop_idx = existing_loop;
			new_loop = loop;
		}
	}

	if (new_loop == nullptr) {
		new_loop_idx = loop;
		new_loop = &loops[new_loop_idx];

		new_loop->target_v = target_v;
		new_loop->v_next_loop = new_loop_idx;
	}

	registerLoopToSourceVertexList(new_loop_idx, src_v);

	if (existing_loop != 0xFFFF'FFFF) {
		registerLoopToMirrorLoopList(new_loop_idx, existing_loop);
	}
	else {
		existing_loop = findLoopFromTo(target_v, src_v);

		if (existing_loop != 0xFFFF'FFFF) {
			registerLoopToMirrorLoopList(new_loop_idx, existing_loop);
		}
		else {
			new_loop->mirror_loop = new_loop_idx;
		}
	}

	return new_loop_idx;
}

uint32_t SculptMesh::addLoop(uint32_t src_v, uint32_t target_v)
{
	uint32_t new_loop_idx;
	Loop* new_loop = nullptr;

	uint32_t existing_loop = findLoopFromTo(src_v, target_v);
	if (existing_loop != 0xFFFF'FFFF) {

		// reuse wire into loop
		Loop* loop = &loops[existing_loop];
		if (loop->poly == 0xFFFF'FFFF) {
			new_loop_idx = existing_loop;
			new_loop = loop;
		}
	}

	if (new_loop == nullptr) {
		new_loop_idx = (uint32_t)loops.size();
		new_loop = &loops.emplace_back();

		new_loop->target_v = target_v;
		new_loop->v_next_loop = new_loop_idx;
	}

	registerLoopToSourceVertexList(new_loop_idx, src_v);

	if (existing_loop != 0xFFFF'FFFF) {
		registerLoopToMirrorLoopList(new_loop_idx, existing_loop);
	}
	else {
		existing_loop = findLoopFromTo(target_v, src_v);

		if (existing_loop != 0xFFFF'FFFF) {
			registerLoopToMirrorLoopList(new_loop_idx, existing_loop);
		}
		else {
			new_loop->mirror_loop = new_loop_idx;
		}
	}

	return new_loop_idx;
}

glm::vec3 SculptMesh::calcWindingNormal(Vertex* v0, Vertex* v1, Vertex* v2)
{
	return -glm::normalize(glm::cross(v1->pos - v0->pos, v2->pos - v0->pos));
}

//Poly& SculptMesh::addTris(Vertex& v0, Vertex& v1, Vertex& v2,
//	Edge& e0, Edge& e1, Edge& e2)
//{
//	Poly& new_poly = this->polys.emplace_back();
//
//	new_poly.vs[0] = &v0;
//	new_poly.vs[1] = &v1;
//	new_poly.vs[2] = &v2;
//	new_poly.vs[3] = nullptr;
//
//	new_poly.edges[0] = &e0;
//	new_poly.edges[1] = &e1;
//	new_poly.edges[2] = &e2;
//	new_poly.edges[3] = nullptr;
//
//	// Edges
//	e0.polys.push_back(&new_poly);
//	e1.polys.push_back(&new_poly);
//	e2.polys.push_back(&new_poly);
//
//	return new_poly;
//}

uint32_t SculptMesh::addTris(uint32_t v0, uint32_t v1, uint32_t v2)
{
	uint32_t new_poly_idx = (uint32_t)this->polys.size();
	Poly& new_poly = this->polys.emplace_back();

	uint32_t ls_idx[4];
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

	// Calculate quad normals
	Vertex* vertex_0 = &verts[ls[0]->target_v];
	Vertex* vertex_1 = &verts[ls[1]->target_v];
	Vertex* vertex_2 = &verts[ls[2]->target_v];

	new_poly.normal = calcWindingNormal(vertex_0, vertex_1, vertex_2);
	new_poly.inner_loop = ls_idx[0];
	new_poly.is_tris = 1;

	return new_poly_idx;
}

void SculptMesh::setQuad(uint32_t quad, uint32_t l0, uint32_t l1, uint32_t l2, uint32_t l3,
	uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
	uint32_t new_poly_idx = quad;
	Poly& new_poly = polys[new_poly_idx];

	uint32_t ls_idx[4];
	ls_idx[0] = setLoop(l0, v0, v1);
	ls_idx[1] = setLoop(l1, v1, v2);
	ls_idx[2] = setLoop(l2, v2, v3);
	ls_idx[3] = setLoop(l3, v3, v0);

	Loop* ls[4];
	ls[0] = &loops[ls_idx[0]];
	ls[1] = &loops[ls_idx[1]];
	ls[2] = &loops[ls_idx[2]];
	ls[3] = &loops[ls_idx[3]];

	// Register poly to loops
	ls[0]->poly_next_loop = ls_idx[1];
	ls[0]->poly_prev_loop = ls_idx[3];

	ls[1]->poly_next_loop = ls_idx[2];
	ls[1]->poly_prev_loop = ls_idx[0];

	ls[2]->poly_next_loop = ls_idx[3];
	ls[2]->poly_prev_loop = ls_idx[1];

	ls[3]->poly_next_loop = ls_idx[0];
	ls[3]->poly_prev_loop = ls_idx[2];

	for (uint8_t i = 0; i < 4; i++) {
		ls[i]->poly = new_poly_idx;
	}

	// Calculate quad normals
	Vertex*vertex_0 = &verts[ls[0]->target_v];
	Vertex*vertex_1 = &verts[ls[1]->target_v];
	Vertex*vertex_2 = &verts[ls[2]->target_v];
	Vertex*vertex_3 = &verts[ls[3]->target_v];

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
	new_poly.is_tris = 0;
}

uint32_t SculptMesh::addQuad(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
	uint32_t new_poly_idx = (uint32_t)this->polys.size();
	Poly& new_poly = this->polys.emplace_back();

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
	new_poly.is_tris = 0;

	return new_poly_idx;
}

//Poly* SculptMesh::addTrisNormalWinding(Vertex* v0, Vertex* v1, Vertex* v2)
//{
//	glm::vec3 avg_normal = (v0->normal + v1->normal + v2->normal);
//	avg_normal /= 3;
//	avg_normal = glm::normalize(avg_normal);
//
//	glm::vec3 dir_0 = v1->pos - v0->pos;
//	glm::vec3 dir_1 = v2->pos - v0->pos;
//	glm::vec3 winding_normal = glm::cross(dir_0, dir_1);
//
//	if (glm::dot(avg_normal, winding_normal) > 0) {
//
//		addTris(v0, v1, v2);
//	}
//
//	return addTris(v2, v1, v0);
//}

//void SculptMesh::stichVerticesToVertex(Vertex* target, std::vector<Vertex*>& vertices, bool loop)
//{
//	uint32_t last = vertices.size() - 1;
//	for (uint32_t i = 0; i < last; i++) {
//		Vertex* v = vertices[i];
//		Vertex* v_next = vertices[i + 1];
//
//		addTris(v, target, v_next);
//	}
//
//	if (loop) {
//		addTris(vertices[last], target, vertices[0]);
//	}
//}

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
