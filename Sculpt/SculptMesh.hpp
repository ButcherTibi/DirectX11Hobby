#pragma once

// Standard
#include <vector>
#include <array>
#include <list>

// GLM
#include "glm\vec3.hpp"

#include "ErrorStack.hpp"
#include "GLTF_File.hpp"
#include "Geometry.hpp"


/* Brush Pipeline
- ray is cast from camera screen
- ray isects with aabbs
- ray isects with (closest/all tris if passthru) tris => sphere origin
- sphere isects aabbs
- aabbs vertices are sent to GPU for modification
- modified vertices replace old ones
*/

namespace scme {

	/*
		normal.x == 1024, normal not calculated
		normal.y == 1024, vertex is marked as deleted
		normal.xyz == 0, normal is unused
		away_loop == 0xFFFF'FFFF, vertex is a point, not connected to anything
		aabb == 0xFFFF'FFFF, vertex does not belong to any AABB
	*/
	struct Vertex {
	public:
		glm::vec3 pos;
		glm::vec3 normal;

		uint32_t away_loop;  // any loop attached to vertex

		uint32_t aabb;
		uint32_t idx_in_aabb;

	public:
		Vertex() {};

		bool isPoint();
	};


	struct VertexBoundingBox {
		uint32_t parent;
		uint32_t children[8];

		AxisBoundingBox3D aabb;

		uint32_t verts_deleted_count;
		std::vector<uint32_t> verts;

		bool _debug_show_tesselation;  // TODO:

	public:
		bool isUnused();
		bool isLeaf();
	};


	/* Half-edge data structure */
	struct Loop {
	public:
		uint32_t target_v;
		uint32_t v_next_loop;  // next loop around vertex

		uint32_t poly;
		uint32_t poly_next_loop;  // next loop around polygon
		uint32_t poly_prev_loop;

		uint32_t mirror_loop;

		Loop() {};
	};


	struct Poly {
	public:
		glm::vec3 normal;
		uint32_t inner_loop;  // any loop inside polygon

		uint8_t tesselation_type : 1;
		uint8_t is_tris : 1;
		uint8_t temp_flag_0 : 1;  // not used
		uint8_t _pad : 5;

		glm::vec3 tess_normals[2];

		Poly() {};
	};


	/*struct EmptyGap {
		uint32_t idx;
		uint32_t count;
	};*/


	/* Version 3: Allocation-less primitives with AABBs down only */
	class SculptMesh {
	public:
		uint32_t root_aabb;
		std::vector<VertexBoundingBox> aabbs;
		
		// TODO:
		// ScultMesh cache locality stupid solution :
		// just store the vertices in verts vector grouped by AABB
		// simple to mantain and fast
		std::vector<Vertex> verts;

		// vector<uint32_t> empty_edges;
		std::vector<Loop> loops;

		// vector<uint32_t> empty_polys;
		std::vector<Poly> polys;

		// Settings
		uint32_t max_vertices_in_AABB;

	public:
		// Memory caching for intersections
		std::vector<VertexBoundingBox*> _now_aabbs;
		std::vector<VertexBoundingBox*> _next_aabbs;
		std::vector<VertexBoundingBox*> _traced_aabbs;

	public:
		// Axis Aligned Bounding Box ////////////////////////////////

		void transferVertexToAABB(uint32_t vertex, uint32_t destination_aabb);

		void registerVertexToAABBs(uint32_t vertex, uint32_t starting_aabb = 0);

		// Vertex ///////////////////////////////////////////////////

		void calcVertexNormal(Vertex* vertex);

		// TODO: move vertex, register vertex if no AABB or nothing if vertex is still in its own AABB

		// Loop ////////////////////////////////////////////////////

		uint32_t findLoopFromTo(uint32_t src_vertex, uint32_t target_vertex);

		void registerLoopToSourceVertexList(uint32_t away_loop, uint32_t vertex);

		void registerLoopToMirrorLoopList(uint32_t new_loop, uint32_t existing_loop);


		/* assemble a loop from vertices, unless there is already a loop
		in which case that loop is used and returned */
		uint32_t setLoop(uint32_t loop, uint32_t src_vertex, uint32_t target_vertex);

		uint32_t addLoop(uint32_t src_vertex, uint32_t target_vertex);

		// Poly ////////////////////////////////////////////////////

		glm::vec3 calcWindingNormal(Vertex* v0, Vertex* v1, Vertex* v2);

		/* creates a new triangle from existing vertices, creates new loops between the vertices 
		if they are not already present */
		uint32_t addTris(uint32_t v0, uint32_t v1, uint32_t v2);

		/* assemble the tris using blank loops and existing vertices */
		void setTris(uint32_t tris, uint32_t l0, uint32_t l1, uint32_t l2,
			uint32_t v0, uint32_t v1, uint32_t v2);

		/* creates a new quad from existing vertices, creates new loops between the vertices
		if they are not already present */
		uint32_t addQuad(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);

		/* assemble the quad using blank loops and existing vertices */
		void setQuad(uint32_t quad, uint32_t l0, uint32_t l1, uint32_t l2, uint32_t l3,
			uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);

		void stichVerticesToVertexLooped(std::vector<uint32_t>& vertices, uint32_t vertex);

		// Queries ////////////////////////////////////////////

		bool raycastPoly(glm::vec3& ray_origin, glm::vec3& ray_direction, uint32_t poly, glm::vec3& r_point);

		bool raycastPolys(glm::vec3& ray_origin, glm::vec3& ray_direction,
			uint32_t& r_isect_poly, float& r_isect_distance, glm::vec3& r_isect_position);

		////////////////////////////////////////////////////////////

		void createAsTriangle(float size);
		void createAsQuad(float size);
		void createAsCube(float size);
		void createAsCylinder(float height, float diameter, uint32_t rows, uint32_t columns, bool capped = true);
		void createAsUV_Sphere(float diameter, uint32_t rows, uint32_t columns);

		void createFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals);
		void createAsLine(glm::vec3& origin, glm::vec3& direction, float length);

		size_t getMemorySizeMegaBytes();
	};
}
