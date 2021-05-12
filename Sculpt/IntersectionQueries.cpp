
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

//
//bool SculptMesh::raycastPoly(glm::vec3& ray_origin, glm::vec3& ray_direction, uint32_t poly_idx, glm::vec3& r_point)
//{
//	Poly* poly = &polys[poly_idx];
//
//	if (poly->is_tris) {
//
//		Loop* l0 = &loops[poly->inner_loop];
//		Loop* l1 = &loops[l0->poly_next_loop];
//		Loop* l2 = &loops[l1->poly_next_loop];
//
//		Vertex* v0 = &verts[l0->target_v];
//		Vertex* v1 = &verts[l1->target_v];
//		Vertex* v2 = &verts[l2->target_v];
//
//		return raycastTrisMollerTrumbore(ray_origin, ray_direction,
//			v0->pos, v1->pos, v2->pos, r_point);
//	}
//
//	Loop* l0 = &loops[poly->inner_loop];
//	Loop* l1 = &loops[l0->poly_next_loop];
//	Loop* l2 = &loops[l1->poly_next_loop];
//	Loop* l3 = &loops[l2->poly_next_loop];
//
//	Vertex* v0 = &verts[l0->target_v];
//	Vertex* v1 = &verts[l1->target_v];
//	Vertex* v2 = &verts[l2->target_v];
//	Vertex* v3 = &verts[l3->target_v];
//
//	if (poly->tesselation_type == 0) {
//
//		if (raycastTrisMollerTrumbore(ray_origin, ray_direction,
//			v0->pos, v1->pos, v2->pos, r_point))
//		{
//			return true;
//		}
//
//		return raycastTrisMollerTrumbore(ray_origin, ray_direction,
//			v0->pos, v2->pos, v2->pos, r_point);
//	}
//
//	if (raycastTrisMollerTrumbore(ray_origin, ray_direction,
//		v0->pos, v1->pos, v3->pos, r_point))
//	{
//		return true;
//	}
//
//	return raycastTrisMollerTrumbore(ray_origin, ray_direction,
//		v1->pos, v2->pos, v3->pos, r_point);
//}
//
//bool SculptMesh::raycastPolys(glm::vec3& ray_origin, glm::vec3& ray_direction,
//	uint32_t& r_isect_poly, float& r_isect_distance, glm::vec3& r_isect_position)
//{
//	std::vector<VertexBoundingBox*>& now_aabbs = _now_aabbs;
//	std::vector<VertexBoundingBox*>& next_aabbs = _next_aabbs;
//	std::vector<VertexBoundingBox*>& traced_aabbs = _traced_aabbs;
//
//	now_aabbs.resize(1);
//	now_aabbs[0] = &aabbs[root_aabb];
//
//	traced_aabbs.clear();
//
//	// gather AABBs that intersect with ray
//	while (now_aabbs.size()) {
//		next_aabbs.clear();
//
//		for (VertexBoundingBox* now_aabb : now_aabbs) {
//
//			if (now_aabb->aabb.isRayIsect(ray_origin, ray_direction)) {
//
//				if (now_aabb->isLeaf()) {
//					traced_aabbs.push_back(now_aabb);
//				}
//				else {
//					// Schedule next
//					for (uint32_t child_idx : now_aabb->children) {
//						next_aabbs.push_back(&aabbs[child_idx]);
//					}
//				}
//			}
//		}
//
//		now_aabbs.swap(next_aabbs);
//	}
//
//	// Depth sort traced AABBs ( LEAF AABBs DO NOT OVERLAP so they can depth discarded )
//	std::sort(traced_aabbs.begin(), traced_aabbs.end(), [&](VertexBoundingBox* a, VertexBoundingBox* b) {
//		float dist_a = glm::distance(ray_origin, a->aabb.max);
//		float dist_b = glm::distance(ray_origin, b->aabb.max);
//		return dist_a < dist_b;
//	});
//
//	// Find closest triangle
//	for (VertexBoundingBox* aabb : traced_aabbs) {
//
//		uint32_t closest_poly;
//		float closest_distance = FLT_MAX;
//		glm::vec3 closest_isect_position;
//
//		for (uint32_t v_idx : aabb->verts) {
//
//			if (v_idx != 0xFFFF'FFFF) {
//
//				Vertex* vertex = &verts[v_idx];
//				
//				// Loop around the vertex and check each polygon, store if closer
//				if (!vertex->isPoint()) {
//
//					uint32_t loop_idx = vertex->away_loop;
//					Loop* loop = &loops[loop_idx];
//
//					do {
//						glm::vec3 isect_position;
//						if (raycastPoly(ray_origin, ray_direction, loop->poly, isect_position)) {
//
//							float dist = glm::distance(ray_origin, isect_position);
//							if (dist < closest_distance) {
//								closest_distance = dist;
//								closest_poly = loop->poly;
//								closest_isect_position = isect_position;
//							}
//						}
//
//						loop_idx = loop->v_next_loop;
//						loop = &loops[loop_idx];
//					}
//					while (loop_idx != vertex->away_loop);
//				}
//			}
//		}
//
//		if (closest_distance != FLT_MAX) {
//			
//			r_isect_poly = closest_poly;
//			r_isect_distance = closest_distance;
//			r_isect_position = closest_isect_position;
//			return true;
//		}
//	}
//
//	return false;
//}