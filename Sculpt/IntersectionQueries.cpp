
// Header
#include "SculptMesh.hpp"


using namespace scme;

///* Geometry Solution
//* https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
//*/
//bool rayTriangleIntersection(glm::vec3& orig, glm::vec3& dir,
//	glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& N,
//	glm::vec3& r_P)
//{
//	// Step 1: finding P
//
//	// check if ray and plane are parallel ?
//	float NdotRayDirection = glm::dot(N, dir);
//	if (fabs(NdotRayDirection) < 1e-8) {
//		return false; // they are parallel so they don't intersect !
//	}
//
//	// compute d parameter using equation 2
//	float d = glm::dot(N, v0);
//
//	// compute t (equation 3)
//	float t = (glm::dot(N, orig) + d) / NdotRayDirection;
//
//	// check if the triangle is in behind the ray
//	if (t < 0) {
//		return false; // the triangle is behind
//	}
//
//	// compute the intersection point using equation 1
//	r_P = orig + t * dir;
//
//	// Step 2: inside-outside test
//	glm::vec3 C; // vector perpendicular to triangle's plane 
//
//	// edge 0
//	glm::vec3 edge0 = v1 - v0;
//	glm::vec3 vp0 = r_P - v0;
//	C = glm::cross(edge0, vp0);
//	if (glm::dot(N, C) < 0) {
//		return false; // P is on the right side
//	}
//
//	// edge 1
//	glm::vec3 edge1 = v2 - v1;
//	glm::vec3 vp1 = r_P - v1;
//	C = glm::cross(edge1, vp1);
//	if (glm::dot(N, C) < 0) {
//		return false; // P is on the right side
//	}
//
//	// edge 2
//	glm::vec3 edge2 = v0 - v2;
//	glm::vec3 vp2 = r_P - v2;
//	C = glm::cross(edge2, vp2);
//	if (glm::dot(N, C) < 0) {
//		return false; // P is on the right side; 
//	}
//
//	return true; // this ray hits the triangle 
//}

/* Möller-Trumbore algorithm
* https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
*/
bool raycastTrisMollerTrumbore(glm::vec3& orig, glm::vec3& dir,
	glm::vec3& v0, glm::vec3& v1, glm::vec3& v2,
	glm::vec3& r_intersection_point)
{
	glm::vec3 v0v1 = v1 - v0;
	glm::vec3 v0v2 = v2 - v0;
	glm::vec3 pvec = glm::cross(dir, v0v2);
	float det = glm::dot(v0v1, pvec);

	// ray and triangle are parallel if det is close to 0
	if (fabs(det) < 1e-8) {
		return false;
	}

	float invDet = 1.0f / det;

	glm::vec3 tvec = orig - v0;
	float u = glm::dot(tvec, pvec) * invDet;

	if (u < 0.0f || u > 1.0f) {
		return false;
	}

	glm::vec3 qvec = glm::cross(tvec, v0v1);
	float v = glm::dot(dir, qvec) * invDet;

	if (v < 0.0f || u + v > 1.0f) {
		return false;
	}

	float t = glm::dot(v0v2, qvec) * invDet;
	r_intersection_point = orig + dir * t;

	return true;
}


#pragma warning(disable : 4702)
bool SculptMesh::raycastPoly(glm::vec3& ray_origin, glm::vec3& ray_direction, uint32_t poly_idx, glm::vec3& r_point)
{
	Poly* poly = &polys[poly_idx];

	if (poly->is_tris) {

		std::array<Vertex*, 3> vs;
		getTrisPrimitives(poly, vs);

		return raycastTrisMollerTrumbore(ray_origin, ray_direction,
			vs[0]->pos, vs[1]->pos, vs[2]->pos, r_point);
	}
	else {
		std::array<Vertex*, 4> vs;
		getQuadPrimitives(poly, vs);

		if (poly->tesselation_type == 0) {

			//  Tesselated Polygon 0 - 2
			//  0-----------1
			//  | \         |
			//  |   \       |
			//  |     \     |
			//  |       \   |
			//  |         \ |
			//  3-----------2
			if (raycastTrisMollerTrumbore(ray_origin, ray_direction,
				vs[0]->pos, vs[1]->pos, vs[2]->pos, r_point))
			{
				return true;
			}

			return raycastTrisMollerTrumbore(ray_origin, ray_direction,
				vs[0]->pos, vs[2]->pos, vs[3]->pos, r_point);
		}
		else {

			//  Tesselated Polygon 1 - 3
			//  0-----------1
			//  |         / |
			//  |       /   |
			//  |     /     |
			//  |   /       |
			//  | /         |
			//  3-----------2
			if (raycastTrisMollerTrumbore(ray_origin, ray_direction,
				vs[0]->pos, vs[1]->pos, vs[3]->pos, r_point))
			{
				return true;
			}

			return raycastTrisMollerTrumbore(ray_origin, ray_direction,
				vs[1]->pos, vs[2]->pos, vs[3]->pos, r_point);
		}
	}


	return false;
}
#pragma warning(default : 4702)

void SculptMesh::_raytraceAABB(glm::vec3& ray_origin, glm::vec3& ray_direction)
{
	std::vector<VertexBoundingBox*>& now_aabbs = _now_aabbs;
	std::vector<VertexBoundingBox*>& next_aabbs = _next_aabbs;
	std::vector<VertexBoundingBox*>& traced_aabbs = _traced_aabbs;

	now_aabbs.resize(1);
	now_aabbs[0] = &aabbs[root_aabb_idx];

	traced_aabbs.clear();

	// gather AABBs that intersect with ray
	while (now_aabbs.size()) {
		next_aabbs.clear();

		for (VertexBoundingBox* now_aabb : now_aabbs) {

			if (now_aabb->aabb.isRayIsect(ray_origin, ray_direction)) {

				if (now_aabb->isLeaf()) {
					traced_aabbs.push_back(now_aabb);
				}
				else {
					// Schedule next
					for (uint32_t child_idx : now_aabb->children) {
						next_aabbs.push_back(&aabbs[child_idx]);
					}
				}
			}
		}

		now_aabbs.swap(next_aabbs);
	}

	// Depth sort traced AABBs
	std::sort(traced_aabbs.begin(), traced_aabbs.end(), [&](VertexBoundingBox* a, VertexBoundingBox* b) {
		float dist_a = glm::distance(ray_origin, a->aabb.max);
		float dist_b = glm::distance(ray_origin, b->aabb.max);
		return dist_a < dist_b;
	});
}

// print raycastPolys performance stats
#define raycastPolys_enable_performance_stats 1

// TODO: does not work properly
bool SculptMesh::raycastPolys(glm::vec3& ray_origin, glm::vec3& ray_direction,
	uint32_t& r_isect_poly, glm::vec3& r_isect_position)
{
#if raycastPolys_enable_performance_stats
	SteadyTime start = std::chrono::steady_clock::now();
	SteadyTime ray_aabb_isect_time;
	SteadyTime ray_poly_isect_time;
#endif

	std::vector<VertexBoundingBox*>& now_aabbs = _now_aabbs;
	std::vector<VertexBoundingBox*>& next_aabbs = _next_aabbs;
	std::vector<VertexBoundingBox*>& traced_aabbs = _traced_aabbs;

	now_aabbs.resize(1);
	now_aabbs[0] = &aabbs[root_aabb_idx];

	traced_aabbs.clear();

	// gather AABBs that intersect with ray
	while (now_aabbs.size()) {
		next_aabbs.clear();

		for (VertexBoundingBox* now_aabb : now_aabbs) {

			if (now_aabb->aabb.isRayIsect(ray_origin, ray_direction)) {

				if (now_aabb->isLeaf()) {
					traced_aabbs.push_back(now_aabb);
				}
				else {
					// Schedule next
					for (uint32_t child_idx : now_aabb->children) {
						next_aabbs.push_back(&aabbs[child_idx]);
					}
				}
			}
		}

		now_aabbs.swap(next_aabbs);
	}

#if raycastPolys_enable_performance_stats
	ray_aabb_isect_time = std::chrono::steady_clock::now();
#endif

	// Depth sort traced AABBs
	std::sort(traced_aabbs.begin(), traced_aabbs.end(), [&](VertexBoundingBox* a, VertexBoundingBox* b) {
		float dist_a = glm::distance(ray_origin, a->aabb.max);
		float dist_b = glm::distance(ray_origin, b->aabb.max);
		return dist_a < dist_b;
	});

	// mark all tested edges as untested for next function call
	auto unmark_tested_edges = [&]() {

		for (uint32_t edge_idx : _tested_edges) {
			edges[edge_idx].was_raycast_tested = false;
		}
	};

	// Find the closest poly in the closest AABB
	for (VertexBoundingBox* aabb : traced_aabbs) {

		_tested_edges.clear();

		uint32_t closest_poly;
		float closest_distance = FLT_MAX;
		glm::vec3 closest_isect_position;

		for (uint32_t v_idx : aabb->verts) {

			if (v_idx != 0xFFFF'FFFF) {

				Vertex* vertex = &verts[v_idx];
				
				// vertex -> edges -> polys[2]
				if (vertex->isPoint() == false) {

					uint32_t edge_idx = vertex->edge;
					Edge* edge = &edges[edge_idx];

					do {
						if (edge->was_raycast_tested == false) {

							if (edge->p0 != 0xFFFF'FFFF) {

								glm::vec3 isect_position;
								if (raycastPoly(ray_origin, ray_direction, edge->p0, isect_position)) {

									float dist = glm::distance(ray_origin, isect_position);
									if (dist < closest_distance) {
										closest_distance = dist;
										closest_poly = edge->p0;
										closest_isect_position = isect_position;
									}
								}
							}

							if (edge->p1 != 0xFFFF'FFFF) {

								glm::vec3 isect_position;
								if (raycastPoly(ray_origin, ray_direction, edge->p1, isect_position)) {

									float dist = glm::distance(ray_origin, isect_position);
									if (dist < closest_distance) {
										closest_distance = dist;
										closest_poly = edge->p1;
										closest_isect_position = isect_position;
									}
								}
							}

							// mark edge as tested
							edge->was_raycast_tested = true;
							_tested_edges.push_back(edge_idx);
						}

						// Iter
						edge_idx = edge->nextEdgeOf(v_idx);
						edge = &edges[edge_idx];
					}
					while (edge_idx != vertex->edge);
				}
			}
		}


		// stop at the first (closest) AABB for hit
		if (closest_distance != FLT_MAX) {
			
			r_isect_poly = closest_poly;
			r_isect_position = closest_isect_position;

			unmark_tested_edges();

#if raycastPolys_enable_performance_stats
			ray_poly_isect_time = std::chrono::steady_clock::now();

			printf(
				"Raycast performance stats: \n"
				"  AABB raycast duration = %zd \n"
				"  poly raycast duration = %zd \n",
				std::chrono::duration_cast<std::chrono::microseconds>(ray_aabb_isect_time - start).count(),
				std::chrono::duration_cast<std::chrono::microseconds>(ray_poly_isect_time - ray_aabb_isect_time).count()
			);
#endif
			return true;
		}
	}

	unmark_tested_edges();
	return false;
}

void SculptMesh::sphereIsectAABBs(glm::vec3& origin, float radius)
{
	std::vector<VertexBoundingBox*>& now_aabbs = _now_aabbs;
	std::vector<VertexBoundingBox*>& next_aabbs = _next_aabbs;
	std::vector<VertexBoundingBox*>& traced_aabbs = _traced_aabbs;

	now_aabbs.resize(1);
	now_aabbs[0] = &aabbs[root_aabb_idx];

	traced_aabbs.clear();

	// gather AABBs that intersect with ray
	while (now_aabbs.size()) {

		next_aabbs.clear();

		for (VertexBoundingBox* now_aabb : now_aabbs) {

			if (now_aabb->aabb.isSphereIsect(origin, radius)) {

				if (now_aabb->isLeaf()) {
					traced_aabbs.push_back(now_aabb);
				}
				else {
					// Schedule next
					for (uint32_t child_idx : now_aabb->children) {
						next_aabbs.push_back(&aabbs[child_idx]);
					}
				}
			}
		}

		now_aabbs.swap(next_aabbs);
	}
}
