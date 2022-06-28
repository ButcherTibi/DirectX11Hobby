#include "./SculptMesh.hpp"


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

bool VertexBoundingBox::isLeaf()
{
	return children[0] == 0xFFFF'FFFF;
}

bool VertexBoundingBox::hasVertices()
{
	return verts.size() - verts_deleted_count > 0;
}

uint32_t VertexBoundingBox::inWhichChildDoesPositionReside(glm::vec3& pos)
{
	assert_cond(isLeaf() == false, "should not be called for leafs because leafs don't have children");

	// above
	if (pos.y >= mid.y) {

		// forward
		if (pos.z >= mid.z) {

			// left
			if (pos.x <= mid.x) {
				return children[0];
			}
			// right
			else {
				return children[1];
			}
		}
		// back
		else {
			// left
			if (pos.x <= mid.x) {
				return children[2];
			}
			// right
			else {
				return children[3];
			}
		}
	}
	// below
	else {
		// forward
		if (pos.z >= mid.z) {

			// left
			if (pos.x <= mid.x) {
				return children[4];
			}
			// right
			else {
				return children[5];
			}
		}
		// back
		else {
			// left
			if (pos.x <= mid.x) {
				return children[6];
			}
		}
	}

	// right
	return children[7];
}

uint32_t& Edge::nextEdgeOf(uint32_t vertex_idx)
{
	if (v0 == vertex_idx) {
		return v0_next_edge;
	}

	assert_cond(v1 == vertex_idx, "malformed iteration of edge around vertex");

	return v1_next_edge;
}

uint32_t& Edge::prevEdgeOf(uint32_t vertex_idx)
{
	if (v0 == vertex_idx) {
		return v0_prev_edge;
	}

	assert_cond(v1 == vertex_idx, "malformed iteration of edge around vertex");

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

void SculptMesh::markVertexFullUpdate(uint32_t vertex)
{
	ModifiedVertex& modified_vertex = modified_verts.emplace_back();
	modified_vertex.idx = vertex;
	modified_vertex.state = ModifiedVertexState::UPDATE;

}

void SculptMesh::markVertexMoved(uint32_t vertex)
{
	ModifiedVertex& modified_vertex = modified_verts.emplace_back();
	modified_vertex.idx = vertex;
	modified_vertex.state = ModifiedVertexState::UPDATE;
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

void SculptMesh::unregisterEdgeFromVertex(Edge* delete_edge, uint32_t vertex_idx, Vertex* vertex)
{
	if (vertex->edge != 0xFFFF'FFFF) {

		uint32_t prev_edge_idx = delete_edge->prevEdgeOf(vertex_idx);
		uint32_t next_edge_idx = delete_edge->nextEdgeOf(vertex_idx);

		Edge& prev_edge = edges[prev_edge_idx];
		Edge& next_edge = edges[next_edge_idx];

		// prev <---> next
		prev_edge.nextEdgeOf(vertex_idx) = next_edge_idx;
		next_edge.prevEdgeOf(vertex_idx) = prev_edge_idx;

		// make sure that the edge list has proper start or else infinite looping can occur
		// because start_edge is not in edge list
		vertex->edge = next_edge_idx;
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

void SculptMesh::markPolyFullUpdate(uint32_t poly)
{
	ModifiedPoly& modified_poly = modified_polys.emplace_back();
	modified_poly.idx = poly;
	modified_poly.state = ModifiedPolyState::UPDATE;
}

void SculptMesh::printVerices()
{
	for (auto iter = verts.begin(); iter != verts.end(); iter.next()) {

		Vertex& vertex = iter.get();

		printf("vertex[%d].pos = { %.2f, %.2f %.2f } \n",
			iter.index(),
			vertex.pos.x,
			vertex.pos.y,
			vertex.pos.z
		);
	}
}
