
// Header
#include "SculptMesh.hpp"

#include "Renderer.hpp"


using namespace scme;



void SculptMesh::_transferVertexToAABB(uint32_t v, uint32_t dest_aabb)
{
	Vertex* vertex = &verts[v];
	VertexBoundingBox& destination_aabb = aabbs[dest_aabb];

	// remove from old AABB
	if (vertex->aabb != 0xFFFF'FFFF) {

		VertexBoundingBox& source_aabb = aabbs[vertex->aabb];

		source_aabb.verts_deleted_count -= 1;
		source_aabb.verts[vertex->idx_in_aabb] = 0xFFFF'FFFF;

		// NOTE: the process of merging empty leafs or under ocupied leafs into parent AABB has been deliberatly omited
		// it is expected that the AABB graph will be recreated overy so often
		// making merging not be worth while in terms of execution speed/time/lag
	}

	vertex->aabb = dest_aabb;
	vertex->idx_in_aabb = destination_aabb.verts.size();
	destination_aabb.verts.push_back(v);
}

void SculptMesh::_recreateAABBs()
{
	aabbs.resize(1);
	root_aabb_idx = 0;

	VertexBoundingBox& root = aabbs[root_aabb_idx];
	root.parent = 0xFFFF'FFFF;
	root.children[0] = 0xFFFF'FFFF;
	root.aabb.max = { 0, 0, 0 };
	root.aabb.min = { FLT_MAX, FLT_MAX, FLT_MAX };
	root.verts_deleted_count = 0;
	root.verts.clear();

#undef max
#undef min

	auto& max = root.aabb.max;
	auto& min = root.aabb.min;

	for (auto iter = verts.begin(); iter != verts.end(); iter.next()) {

		Vertex& vert = iter.get();
		vert.aabb = 0xFFFF'FFFF;

		if (vert.pos.x > max.x) {
			max.x = vert.pos.x;
		}

		if (vert.pos.y > max.y) {
			max.y = vert.pos.y;
		}

		if (vert.pos.z > max.z) {
			max.z = vert.pos.z;
		}

		if (vert.pos.x < min.x) {
			min.x = vert.pos.x;
		}

		if (vert.pos.y < min.y) {
			min.y = vert.pos.y;
		}

		if (vert.pos.z < min.z) {
			min.z = vert.pos.z;
		}
	}

	float size_x = root.aabb.sizeX();
	float size_y = root.aabb.sizeY();
	float size_z = root.aabb.sizeZ();
	float grow_size;

	if (size_x > size_y) {
		grow_size = size_x;
	}
	else {
		grow_size = size_y;
	}

	if (grow_size < size_z) {
		grow_size = size_z;
	}

	root.mid.x = root.aabb.midX();
	root.mid.y = root.aabb.midY();
	root.mid.z = root.aabb.midZ();
	root.aabb.min = {
		root.mid.x - grow_size,
		root.mid.y - grow_size,
		root.mid.z - grow_size
	};
	root.aabb.max = {
		root.mid.x + grow_size,
		root.mid.y + grow_size,
		root.mid.z + grow_size
	};

	for (auto iter = verts.begin(); iter != verts.end(); iter.next()) {
		moveVertexInAABBs(iter.index());
	}
}

void SculptMesh::moveVertexInAABBs(uint32_t vertex_idx)
{
	Vertex& vertex = verts[vertex_idx];

	// did it even leave the original AABB heuristic
	if (vertex.aabb != 0xFFFF'FFFF) {

		VertexBoundingBox& original_aabb = aabbs[vertex.aabb];

		// no significant change in position
		if (original_aabb.aabb.isPositionInside(vertex.pos)) {
			return;
		}
	}

	// TODO: refactor using spatial split planes
	uint32_t now_count = 1;
	std::array<uint32_t, 8> now_aabbs = {
		root_aabb_idx
	};

	uint32_t next_count = 0;
	std::array<uint32_t, 8> next_aabbs;

	while (now_count) {

		for (uint32_t i = 0; i < now_count; i++) {

			uint32_t aabb_idx = now_aabbs[i];
			VertexBoundingBox* aabb = &aabbs[aabb_idx];

			if (aabb->aabb.isPositionInside(vertex.pos)) {

				// Leaf
				if (aabb->isLeaf()) {

					uint32_t child_vertex_count = aabb->verts.size() - aabb->verts_deleted_count;

					// leaf not full
					if (child_vertex_count < max_vertices_in_AABB) {

						_transferVertexToAABB(vertex_idx, aabb_idx);
						return;
					}
					// Subdivide
					else {
						uint32_t base_idx = aabbs.size();
						aabbs.resize(aabbs.size() + 8);

						aabb = &aabbs[aabb_idx];  // old AABB pointer invalidated because octrees resize

						aabb->aabb.subdivide(
							aabbs[base_idx + 0].aabb, aabbs[base_idx + 1].aabb,
							aabbs[base_idx + 2].aabb, aabbs[base_idx + 3].aabb,

							aabbs[base_idx + 4].aabb, aabbs[base_idx + 5].aabb,
							aabbs[base_idx + 6].aabb, aabbs[base_idx + 7].aabb,
							aabb->mid
						);

						bool found = false;

						for (i = 0; i < 8; i++) {

							uint32_t child_aabb_idx = base_idx + i;
							aabb->children[i] = child_aabb_idx;

							VertexBoundingBox& child_aabb = aabbs[child_aabb_idx];
							child_aabb.parent = aabb_idx;
							child_aabb.children[0] = 0xFFFF'FFFF;
							child_aabb.verts_deleted_count = 0;
							child_aabb.verts.reserve(max_vertices_in_AABB / 4);  // just a guess

							// transfer the excess vertex to one of child AABBs
							if (!found && child_aabb.aabb.isPositionInside(vertex.pos)) {
								_transferVertexToAABB(vertex_idx, child_aabb_idx);
								found = true;
							}
						}

						assert_cond(found == true, "vertex was not found in subdivided leaf");

						// transfer the rest of vertices of the parent to children
						for (uint32_t aabb_vertex_idx : aabb->verts) {

							// skip deleted vertex index from AABB
							if (aabb_vertex_idx == 0xFFFF'FFFF) {
								continue;
							}

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
				// Schedule going down the graph call
				else {
					uint32_t child_idx = aabb->inWhichChildDoesPositionReside(vertex.pos);

					// unlikelly only float point errors
					if (child_idx == 0xFFFF'FFFF) {

						for (i = 0; i < 8; i++) {
							next_aabbs[next_count] = aabb->children[i];
							next_count++;
						}
					}
					else {
						next_aabbs[next_count] = child_idx;
						next_count++;
					}
					break;
				}
			}
		}

		now_aabbs.swap(next_aabbs);
		now_count = next_count;
		next_count = 0;
	}

	// At this point position is not found in the AABB graph

	VertexBoundingBox& old_root = aabbs[root_aabb_idx];
	float root_size = old_root.aabb.sizeX();

	// would the position fit inside the graph if it would be enlarged by one level up
	if (old_root.aabb.min.x - root_size < vertex.pos.x &&
		vertex.pos.x < old_root.aabb.max.x + root_size &&
		// Y
		old_root.aabb.min.y - root_size < vertex.pos.y &&
		vertex.pos.y < old_root.aabb.max.y + root_size &&
		// Z
		old_root.aabb.min.z - root_size < vertex.pos.z &&
		vertex.pos.z < old_root.aabb.max.z + root_size)
	{
		// NOTE: taking this path is quicker but increases the graph traversal cost for all vertices by one level

		uint32_t new_root_idx = aabbs.size();
		VertexBoundingBox& new_root = aabbs.emplace_back();
		new_root.parent = 0xFFFF'FFFF;
		new_root.verts_deleted_count = 0;

		/* create a new AABB that is twice as big and is positioned so that
		  it contains the vertex

		  2D ZX Top Down example:
		  +---+---+    +---+---+    +---+---+    +---+---+
		  | O |   |	   |   |   |    | O |   |    |   | O |
		  +---+---+	   +---+---+    +---+---+    +---+---+
		  |   |   |	   | O |   |    |   |   |    |   |   |
		  +---+---+	   +---+---+    +---+---+    +---+---+

		  O is the old root AABB
		*/
		old_root.parent = new_root_idx;
		{
			if (vertex.pos.y > old_root.aabb.max.y) {
				new_root.aabb.min.y = old_root.aabb.min.y;
				new_root.aabb.max.y = new_root.aabb.min.y + root_size;
			}
			else {
				new_root.aabb.max.y = old_root.aabb.max.y;
				new_root.aabb.min.y = new_root.aabb.max.y - root_size;
			}

			if (vertex.pos.z > old_root.aabb.max.z) {
				new_root.aabb.min.z = old_root.aabb.min.z;
				new_root.aabb.max.z = new_root.aabb.min.z + root_size;
			}
			else {
				new_root.aabb.max.z = old_root.aabb.max.z;
				new_root.aabb.min.z = new_root.aabb.max.z - root_size;;
			}

			if (vertex.pos.x > old_root.aabb.max.x) {
				new_root.aabb.min.x = old_root.aabb.min.x;
				new_root.aabb.max.x = new_root.aabb.min.x + root_size;
			}
			else {
				new_root.aabb.max.x = old_root.aabb.max.x;
				new_root.aabb.min.x = new_root.aabb.max.x - root_size;
			}
		}

		AxisBoundingBox3D<> boxes[8];

		new_root.aabb.subdivide(boxes[0], boxes[1], boxes[2], boxes[3],
			boxes[4], boxes[5], boxes[6], boxes[7],
			new_root.mid);

		// it's cheaper to check for a bool than is position inside
		bool aabb_placed = false;
		bool vertex_placed = false;

		// create and link children
		for (uint32_t i = 0; i < 8; i++) {

			uint32_t child_idx;

			if (aabb_placed == false &&
				boxes[i].isPositionInside(old_root.mid))
			{
				child_idx = root_aabb_idx;

				aabb_placed = true;
			}
			else {
				child_idx = aabbs.size();

				VertexBoundingBox& child_aabb = aabbs.emplace_back();
				child_aabb.parent = new_root_idx;
				child_aabb.children[0] = 0xFFFF'FFFF;
				child_aabb.aabb = boxes[i];
				child_aabb.mid = { boxes->midX(), boxes->midY(), boxes->midZ() };
				child_aabb.verts_deleted_count = 0;

				if (vertex_placed == false &&
					child_aabb.aabb.isPositionInside(vertex.pos))
				{
					child_aabb.verts.push_back(vertex_idx);

					vertex.aabb = child_idx;
					vertex.idx_in_aabb = 0;

					vertex_placed = true;
				}
			}

			new_root.children[i] = child_idx;
		}

		root_aabb_idx = new_root_idx;
	}
	// recreate the graph big enough to fit the vertex and a bit more
	else {
		_recreateAABBs();
	}
}

void SculptMesh::recreateAABBs(uint32_t new_max_vertices_in_AABB)
{
	if (new_max_vertices_in_AABB) {
		this->max_vertices_in_AABB = new_max_vertices_in_AABB;
	}

	assert_cond(this->max_vertices_in_AABB > 0, "maximum number of vertices for AABB is not specified");

	_recreateAABBs();
}

void SculptMesh::standardBrush(StandardBrushInfo&)
{

}
