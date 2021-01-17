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
		away_loop == 0xFFFF'FFFF, vertex is a point, not connected to anything
	*/
	struct Vertex {
	public:
		glm::vec3 pos;
		glm::vec3 normal;

		uint32_t away_loop;

		uint32_t aabb;
		uint32_t idx_in_aabb;
	};


	struct VertexBoundingBox {
		uint32_t parent;
		uint32_t children[8];

		AxisBoundingBox3D aabb;

		uint32_t deleted_count;
		std::vector<uint32_t> verts;

		bool _debug_show_tesselation;  // TODO:

	public:
		bool is_unused();
	};


	/* Half-edge data structure */
	struct Loop {
	public:
		uint32_t target_v;
		uint32_t v_next_loop;

		uint32_t poly;
		uint32_t poly_next_loop;
		uint32_t poly_prev_loop;

		uint32_t mirror_loop;
	};


	struct Poly {
	public:
		glm::vec3 normal;
		uint32_t inner_loop;

		uint8_t tesselation_type : 1;
		uint8_t is_tris : 1;
		uint8_t _pad : 6;

		glm::vec3 tess_normals[2];
	};


	/*struct EmptyGap {
		uint32_t idx;
		uint32_t count;
	};*/


	/* Version 3: Allocation-less primitives */
	class SculptMesh {
	public:
		std::vector<VertexBoundingBox> aabbs;
		
		std::vector<Vertex> verts;

		// vector<uint32_t> empty_edges;
		std::vector<Loop> loops;

		// vector<uint32_t> empty_polys;
		std::vector<Poly> polys;

		// Settings
		uint32_t max_vertices_in_AABB;

	public:
		// Axis Aligned Bounding Box ////////////////////////////////

		void transferVertexToAABB(uint32_t vertex, uint32_t destination_aabb);

		void registerVertexToAABBs(uint32_t vertex, uint32_t starting_aabb = 0);

		// Vertex ///////////////////////////////////////////////////

		void calcVertexNormal(Vertex* vertex);

		// Loop ////////////////////////////////////////////////////

		uint32_t findLoopFromTo(uint32_t src_vertex, uint32_t target_vertex);

		void registerLoopToSourceVertexList(uint32_t away_loop, uint32_t vertex);

		void registerLoopToMirrorLoopList(uint32_t new_loop, uint32_t existing_loop);

		/* always creates a edge between existing vertices */
		//Edge* addEdge(Vertex* v0, Vertex* v1);

		/* only creates a edge if there is no edge between the vertices */
		//Edge* addEdgeIfNone(Vertex* v0, Vertex* v1);

		/* assemble a loop from vertices, unless there is already a loop
		in which case that loop is used and returned */
		uint32_t setLoop(uint32_t loop, uint32_t src_vertex, uint32_t target_vertex);

		uint32_t addLoop(uint32_t src_vertex, uint32_t target_vertex);

		// Poly ////////////////////////////////////////////////////

		glm::vec3 calcWindingNormal(Vertex* v0, Vertex* v1, Vertex* v2);

		/* creates a triangle from existing vertices and edges */
		//Poly& addTris(Vertex& v0, Vertex& v1, Vertex& v2,
		//	Edge& e0, Edge& e1, Edge& e2);

		/* creates a new triangle from existing vertices, creates new loops between the vertices 
		if they are not already present */
		uint32_t addTris(uint32_t v0, uint32_t v1, uint32_t v2);

		/* assemble the quad using blank loops and existing vertices */
		void setTris(uint32_t tris, uint32_t l0, uint32_t l1, uint32_t l2,
			uint32_t v0, uint32_t v1, uint32_t v2);

		/* creates a new quad from existing vertices, creates new loops between the vertices
		if they are not already present */
		uint32_t addQuad(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);

		/* assemble the quad using blank loops and existing vertices */
		void setQuad(uint32_t quad, uint32_t l0, uint32_t l1, uint32_t l2, uint32_t l3,
			uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);

		//void stichVerticesToVertex(Vertex* v, std::vector<Vertex*>& vertices, bool loop = false);
		void stichVerticesToVertexLooped(std::vector<uint32_t>& vertices, uint32_t vertex);

		// addLoneQuad
		// addTris
		// addQuad
		// stichTrisToOne(existing_v, new_v0, new_v1)
		// stichTrisToTwo(existing_v0, existing_v1, new_v)
		// stichQuadToOne()
		// stichQuadToTwo()
		// stichQuadToThree()

		////////////////////////////////////////////////////////////

		void createAsTriangle(float size);
		void createAsQuad(float size);
		void createAsCube(float size);
		void createAsCylinder(float height, float diameter, uint32_t rows, uint32_t columns, bool capped = true);
		void createAsUV_Sphere(float diameter, uint32_t rows, uint32_t columns,
			uint32_t max_vertices_in_AABB);

		void createFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
			uint32_t max_vertices_in_AABB);
		//void addFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions,
		//	bool flip_winding = false);

		size_t getMemorySizeMegaBytes();
	};
}
