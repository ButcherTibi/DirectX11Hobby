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

/* AABB Resize
- modified vertices cause their AABB and ancestors to resize

AABB Split
- split occurs when too many vertices belong to single AABB
- split happens on average vertex position recursivelly until there is no AABB that has
  too many vertices

AABB Merge
1. if a AABB has too few vertices then it's merged with its closest sibling
- if all siblings have too many vertices then box gets parented upward and retry (1)
*/


namespace scme {

	class Vertex {
	public:
		glm::vec3 pos;
		glm::vec3 normal;

		uint32_t away_loop;  // starting vertices should ha this set to 0xFFFF'FFFF
	};


	/*struct VertexBoundingBox {
		VertexBoundingBox* parent;
		std::list<VertexBoundingBox*> children;

		AxisBoundingBox3D aabb;

		std::vector<Vertex> vs;
	};*/


	/* Half-edge data structure */
	class Loop {
	public:
		uint32_t target_v;
		uint32_t v_prev_loop;
		uint32_t v_next_loop;

		uint32_t poly;
		uint32_t poly_next_loop;

		uint32_t mirror_loop;
	};


	class Poly {
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
		// std::list<VertexBoundingBox> aabbs;

		// vector<uint32_t> empty_verts;
		std::vector<Vertex> verts;

		// vector<uint32_t> empty_edges;
		std::vector<Loop> loops;

		// vector<uint32_t> empty_polys;
		std::vector<Poly> polys;

		// GPU Rendering
		uint32_t _vertex_start;
		uint32_t _vertex_count;

	public:
		// Vertex ///////////////////////////////////////////////////

		void calcVertexNormal(uint32_t vertex);

		// addLoneVertex(GPU_Vertex new_vertex)
		// addLoneVertices(GPU_Vertex[] new_vertices)

		// addVertices(CPU_Vertex new_vertices)

		// attachVertex(existing_vertex, new_vertex)
		// attachVertex(existing_vertices, new_vertex)
		// attachVertices(existing_vertex, new_vertices)

		// boundVerticesTogether
		// boundVerticesSeparated

		// redistributeRootVertices

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

		////////////////////////////////////////////////////////////

		glm::vec3 calcWindingNormal(Vertex& v0, Vertex& v1, Vertex& v2);

		/* creates a triangle from existing vertices and edges */
		//Poly& addTris(Vertex& v0, Vertex& v1, Vertex& v2,
		//	Edge& e0, Edge& e1, Edge& e2);

		/* creates a new triangle from existing vertices, creates new loops between the vertices 
		if they are not already present */
		uint32_t addTris(uint32_t v0, uint32_t v1, uint32_t v2);

		/* assemble the quad using blank loops and existing vertices */
		void setQuad(uint32_t quad, uint32_t l0, uint32_t l1, uint32_t l2, uint32_t l3,
			uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);

		/* creates a new quad from existing vertices, creates new loops between the vertices
		if they are not already present */
		uint32_t addQuad(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);

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
		void createAsCylinder(float height, float diameter, uint32_t vertical_sides, uint32_t horizontal_sides, bool capped = true);
		void createAsUV_Sphere(float diameter, uint32_t vertical_sides, uint32_t horizontal_sides);

		//void addFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
		//	bool flip_winding = false);
		//void addFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions,
		//	bool flip_winding = false);

		size_t getMemorySizeMegaBytes();
	};
}
