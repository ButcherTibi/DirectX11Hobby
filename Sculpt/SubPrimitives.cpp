
// Header
#include "SculptMesh.hpp"


using namespace scme;


void Vertex::init()
{
	edge = 0xFFFF'FFFF;
	aabb = 0xFFFF'FFFF;
}

bool Vertex::isPoint()
{
	return edge == 0xFFFF'FFFF;
}

bool VertexBoundingBox::isUnused()
{
	return verts.size() - verts_deleted_count == 0 && children[0] == 0xFFFF'FFFF;
}

bool VertexBoundingBox::isLeaf()
{
	return children[0] == 0xFFFF'FFFF;
}

uint32_t& Edge::nextEdgeOf(uint32_t vertex_idx)
{
	if (v0 == vertex_idx) {
		return v0_next_edge;
	}

	return v1_next_edge;
}

uint32_t& Edge::prevEdgeOf(uint32_t vertex_idx)
{
	if (v0 == vertex_idx) {
		return v0_prev_edge;
	}

	return v1_prev_edge;
}

void Edge::setPrevNextEdges(uint32_t vertex_idx, uint32_t prev_edge_idx, uint32_t next_edge_idx)
{
	if (v0 == vertex_idx) {
		v0_prev_edge = prev_edge_idx;
		v0_next_edge = next_edge_idx;
	}
	else {
		v1_prev_edge = prev_edge_idx;
		v1_next_edge = next_edge_idx;
	}
}

void SculptMesh::_deleteVertexMemory(uint32_t vertex_idx)
{
	verts.erase(vertex_idx);

	ModifiedVertex& modified_vertex = modified_verts.emplace_back();
	modified_vertex.idx = vertex_idx;
	modified_vertex.state = ModifiedVertexState::DELETED;
}

void SculptMesh::_deleteEdgeMemory(uint32_t edge_idx)
{
	edges.erase(edge_idx);
}

void SculptMesh::_deletePolyMemory(uint32_t poly_idx)
{
	polys.erase(poly_idx);

	ModifiedPoly& modified_poly = modified_polys.emplace_back();
	modified_poly.idx = poly_idx;
	modified_poly.state = ModifiedPolyState::DELETED;
}

void SculptMesh::calcVertexNormal(uint32_t vertex_idx)
{
	Vertex* vertex = &verts[vertex_idx];

	if (vertex->edge == 0xFFFF'FFFF) {
		return;
	}

	uint32_t count = 0;
	vertex->normal = { 0, 0, 0 };

	uint32_t edge_idx = vertex->edge;
	Edge* edge = &edges[edge_idx];

	iterEdgesAroundVertexStart;
	{
		Poly* poly;
		if (edge->p0 != 0xFFFF'FFFF) {
			poly = &polys[edge->p0];
			vertex->normal += poly->normal;
			count++;
		}

		if (edge->p1 != 0xFFFF'FFFF) {
			poly = &polys[edge->p1];
			vertex->normal += poly->normal;
			count++;
		}
	}
	iterEdgesAroundVertexEnd(vertex_idx, vertex->edge);

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
	//Vertex* vertex = &verts[vertex_idx];

	//// vertex is point
	//if (vertex->away_loop == 0xFFFF'FFFF) {
	//	throw std::exception("TODO");
	//	return;
	//}

	//Loop* start_loop = &loops[vertex->away_loop];

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



uint32_t SculptMesh::findEdgeBetween(uint32_t v0_idx, uint32_t v1_idx)
{
	Vertex& vertex_0 = verts[v0_idx];
	Vertex& vertex_1 = verts[v1_idx];

	if (vertex_0.edge == 0xFFFF'FFFF || vertex_1.edge == 0xFFFF'FFFF) {
		return 0xFFFF'FFFF;
	}

	uint32_t edge_idx = vertex_0.edge;
	Edge* edge = &edges[edge_idx];

	iterEdgesAroundVertexStart;
	{
		if ((edge->v0 == v0_idx && edge->v1 == v1_idx) ||
			(edge->v0 == v1_idx && edge->v1 == v0_idx))
		{
			return edge_idx;
		}
	}
	iterEdgesAroundVertexEnd(v0_idx, vertex_0.edge);

	edge_idx = vertex_1.edge;
	edge = &edges[edge_idx];

	iterEdgesAroundVertexStart;
	{
		if ((edge->v0 == v0_idx && edge->v1 == v1_idx) ||
			(edge->v0 == v1_idx && edge->v1 == v0_idx))
		{
			return edge_idx;
		}
	}
	iterEdgesAroundVertexEnd(v1_idx, vertex_1.edge);

	return 0xFFFF'FFFF;
}

glm::vec3 calcNormalForTrisPositions(Vertex* v0, Vertex* v1, Vertex* v2)
{
	glm::vec3 dir_0 = glm::normalize(v1->pos - v0->pos);
	glm::vec3 dir_1 = glm::normalize(v2->pos - v0->pos);

	return glm::normalize(-glm::cross(dir_0, dir_1));
}

void SculptMesh::registerEdgeToVertexList(uint32_t new_edge_idx, uint32_t vertex_idx)
{
	Edge& new_edge = edges[new_edge_idx];
	Vertex& vertex = verts[vertex_idx];

	// if vertex is point then vertex loop list is unused
	if (vertex.edge == 0xFFFF'FFFF) {

		new_edge.setPrevNextEdges(vertex_idx, new_edge_idx, new_edge_idx);
		vertex.edge = new_edge_idx;
	}
	else {
		uint32_t prev_edge_idx = vertex.edge;
		Edge& prev_edge = edges[prev_edge_idx];

		uint32_t next_edge_idx = prev_edge.nextEdgeOf(vertex_idx);
		Edge& next_edge = edges[next_edge_idx];

		// prev <--- new ---> next
		new_edge.setPrevNextEdges(vertex_idx, prev_edge_idx, next_edge_idx);

		// prev ---> new <--- next
		prev_edge.nextEdgeOf(vertex_idx) = new_edge_idx;
		next_edge.prevEdgeOf(vertex_idx) = new_edge_idx;
	}
}

void SculptMesh::unregisterEdgeFromVertexList(Edge* delete_edge, uint32_t vertex_idx, Vertex* vertex)
{
	if (vertex->edge != 0xFFFF'FFFF) {

		uint32_t prev_edge_idx = delete_edge->prevEdgeOf(vertex_idx);
		uint32_t next_edge_idx = delete_edge->nextEdgeOf(vertex_idx);

		Edge& prev_edge = edges[prev_edge_idx];
		Edge& next_edge = edges[next_edge_idx];

		// prev <---> next
		prev_edge.nextEdgeOf(vertex_idx) = next_edge_idx;
		next_edge.prevEdgeOf(vertex_idx) = prev_edge_idx;
	}
}

void SculptMesh::registerPolyToEdge(uint32_t new_poly_idx, uint32_t edge_idx)
{
	Edge& edge = edges[edge_idx];
	if (edge.p0 == 0xFFFF'FFFF) {
		edge.p0 = new_poly_idx;
	}
	else {
		edge.p1 = new_poly_idx;
	}
}

void SculptMesh::unregisterPolyFromEdge(uint32_t delete_poly_idx, uint32_t edge_idx)
{
	Edge& edge = edges[edge_idx];
	if (edge.p0 == delete_poly_idx) {
		edge.p0 = 0xFFFF'FFFF;
	}
	else {
		edge.p1 = 0xFFFF'FFFF;
	}
}

uint32_t SculptMesh::createEdge(uint32_t v0, uint32_t v1)
{
	uint32_t new_loop_idx;;
	edges.emplace(new_loop_idx);

	setEdge(new_loop_idx, v0, v1);
	return new_loop_idx;
}

void SculptMesh::setEdge(uint32_t existing_edge_idx, uint32_t v0_idx, uint32_t v1_idx)
{
	Edge* existing_edge = &edges[existing_edge_idx];
	existing_edge->v0 = v0_idx;
	existing_edge->v1 = v1_idx;
	existing_edge->p0 = 0xFFFF'FFFF;
	existing_edge->p1 = 0xFFFF'FFFF;

	registerEdgeToVertexList(existing_edge_idx, v0_idx);
	registerEdgeToVertexList(existing_edge_idx, v1_idx);
}

uint32_t SculptMesh::addEdge(uint32_t v0, uint32_t v1)
{
	uint32_t existing_loop = findEdgeBetween(v0, v1);
	if (existing_loop == 0xFFFF'FFFF) {
		return createEdge(v0, v1);
	}
	
	return existing_loop;
}

glm::vec3 SculptMesh::calcWindingNormal(Vertex* v0, Vertex* v1, Vertex* v2)
{
	return -glm::normalize(glm::cross(v1->pos - v0->pos, v2->pos - v0->pos));
}

void SculptMesh::recalcPolyNormal(Poly* poly)
{
	/*if (poly->is_tris) {

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
	}*/
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
	new_poly.edges[0] = addEdge(v0, v1);
	new_poly.edges[1] = addEdge(v1, v2);
	new_poly.edges[2] = addEdge(v2, v0);

	Edge* edge = &edges[new_poly.edges[0]];
	new_poly.flip_edge_0 = edge->v0 != v0;

	edge = &edges[new_poly.edges[1]];
	new_poly.flip_edge_1 = edge->v0 != v1;

	edge = &edges[new_poly.edges[2]];
	new_poly.flip_edge_2 = edge->v0 != v2;

	registerPolyToEdge(new_poly_idx, new_poly.edges[0]);
	registerPolyToEdge(new_poly_idx, new_poly.edges[1]);
	registerPolyToEdge(new_poly_idx, new_poly.edges[2]);

	// Calculate tris normal
	Vertex* vertex_0 = &verts[v0];
	Vertex* vertex_1 = &verts[v1];
	Vertex* vertex_2 = &verts[v2];
	new_poly.normal = calcWindingNormal(vertex_0, vertex_1, vertex_2);

	markPolyFullUpdate(new_poly_idx);

	return new_poly_idx;
}

void SculptMesh::setQuad(uint32_t new_quad_idx, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
	Poly& new_quad = polys[new_quad_idx];
	new_quad.is_tris = false;
	new_quad.edges[0] = addEdge(v0, v1);
	new_quad.edges[1] = addEdge(v1, v2);
	new_quad.edges[2] = addEdge(v2, v3);
	new_quad.edges[3] = addEdge(v3, v0);

	Edge* edge = &edges[new_quad.edges[0]];
	new_quad.flip_edge_0 = edge->v0 != v0;

	edge = &edges[new_quad.edges[1]];
	new_quad.flip_edge_1 = edge->v0 != v1;

	edge = &edges[new_quad.edges[2]];
	new_quad.flip_edge_2 = edge->v0 != v2;

	edge = &edges[new_quad.edges[3]];
	new_quad.flip_edge_3 = edge->v0 != v3;

	registerPolyToEdge(new_quad_idx, new_quad.edges[0]);
	registerPolyToEdge(new_quad_idx, new_quad.edges[1]);
	registerPolyToEdge(new_quad_idx, new_quad.edges[2]);
	registerPolyToEdge(new_quad_idx, new_quad.edges[3]);

	// Calculate quad normals
	Vertex* vertex_0 = &verts[v0];
	Vertex* vertex_1 = &verts[v1];
	Vertex* vertex_2 = &verts[v2];
	Vertex* vertex_3 = &verts[v3];

	if (glm::distance(vertex_0->pos, vertex_2->pos) < glm::distance(vertex_1->pos, vertex_3->pos)) {

		new_quad.tesselation_type = 0;
		new_quad.tess_normals[0] = calcWindingNormal(vertex_0, vertex_1, vertex_2);
		new_quad.tess_normals[1] = calcWindingNormal(vertex_0, vertex_2, vertex_3);
	}
	else {
		new_quad.tesselation_type = 1;
		new_quad.tess_normals[0] = calcWindingNormal(vertex_0, vertex_1, vertex_3);
		new_quad.tess_normals[1] = calcWindingNormal(vertex_1, vertex_2, vertex_3);
	}

	new_quad.normal = glm::normalize((new_quad.tess_normals[0] + new_quad.tess_normals[1]) / 2.f);

	markPolyFullUpdate(new_quad_idx);
}

uint32_t SculptMesh::addQuad(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
	uint32_t new_quad_idx;
	polys.emplace(new_quad_idx);
	
	setQuad(new_quad_idx, v0, v1, v2, v3);

	return new_quad_idx;
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

void SculptMesh::deletePoly(uint32_t delete_poly_idx)
{
	Poly* delete_poly = &polys[delete_poly_idx];
	
	uint32_t count;
	if (delete_poly->is_tris) {
		count = 3;
	}
	else {
		count = 4;
	}

	for (uint8_t i = 0; i < count; i++) {

		uint32_t edge_idx = delete_poly->edges[i];
		Edge* edge = &edges[edge_idx];

		Vertex* vertex = &verts[edge->v0];
		unregisterEdgeFromVertexList(edge, edge->v0, vertex);

		if (edge->nextEdgeOf(edge->v0) == edge_idx) {
			_deleteVertexMemory(edge->v0);
		}

		vertex = &verts[edge->v1];
		unregisterEdgeFromVertexList(edge, edge->v1, vertex);

		if (edge->nextEdgeOf(edge->v1) == edge_idx) {
			_deleteVertexMemory(edge->v1);
		}

		// unregister poly from edge
		// because edge is part of poly one poly reference will reference poly
		if (edge->p0 == delete_poly_idx) {
			edge->p0 = 0xFFFF'FFFF;
		}
		else {
			edge->p1 = 0xFFFF'FFFF;
		}

		// edge is wire
		if (edge->p0 == edge->p1) {
			_deleteEdgeMemory(edge_idx);
		}
	}

	// Now no edges reference the poly so we can safely delete the polygon
	_deletePolyMemory(delete_poly_idx);
}
