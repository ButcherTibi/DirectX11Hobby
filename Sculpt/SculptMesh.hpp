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

	// Forward declarations
	class Edge;
	class Poly;


	class Vertex {
	public:
		glm::vec3 pos;
		glm::vec3 normal;

		std::vector<Edge*> edges;

	public:
		void calcNormal();
	};


	struct VertexBoundingBox {
		VertexBoundingBox* parent;
		std::list<VertexBoundingBox*> children;

		AxisBoundingBox3D aabb;

		std::vector<Vertex> vs;
	};


	class Edge {
	public:
		Vertex* v0;
		Vertex* v1;

		std::vector<Poly*> polys;
	};


	class Poly {
	public:
		glm::vec3 normal;

		std::array<Vertex*, 4> vs;
		std::array<Edge*, 4> edges;

		bool tesselation_type;
		std::array<glm::vec3, 2> tess_normals;

	public:
		void calcNormalForTris();
		void calcNormalForQuad();
		void calcNormal();
	};


	/*  */
	class SculptMesh {
	public:
		std::list<VertexBoundingBox> aabbs;
		std::list<Edge> edges;
		std::list<Poly> polys;

		// GPU Rendering
		uint32_t _vertex_start;
		uint32_t _vertex_count;

	public:
		//void createTriangle();

		////////////////////////////////////////////////////////////
		// addLoneVertex(GPU_Vertex new_vertex)
		// addLoneVertices(GPU_Vertex[] new_vertices)

		// addVertices(CPU_Vertex new_vertices)

		// attachVertex(existing_vertex, new_vertex)
		// attachVertex(existing_vertices, new_vertex)
		// attachVertices(existing_vertex, new_vertices)

		// boundVerticesTogether
		// boundVerticesSeparated

		// redistributeRootVertices 

		////////////////////////////////////////////////////////////

		/* always creates a edge between existing vertices */
		Edge* addEdge(Vertex* v0, Vertex* v1);

		/* only creates a edge if there is no edge between the vertices */
		Edge* addEdgeIfNone(Vertex* v0, Vertex* v1);

		////////////////////////////////////////////////////////////

		/* creates a triangle from existing vertices and edges */
		Poly& addTris(Vertex& v0, Vertex& v1, Vertex& v2,
			Edge& e0, Edge& e1, Edge& e2);

		/* creates a new triangle from existing vertices, creates new edges between the vertices 
		if they are not already present */
		Poly* addTris(Vertex* v0, Vertex* v1, Vertex* v2);

		/* creates a new triangle from existing vertices, creates new edges between the vertices 
		if they are not already present, winding is set based on average normal */
		Poly* addTrisNormalWinding(Vertex* v0, Vertex* v1, Vertex* v2);
		
		// Poly* addTrisPositionWinding(Vertex* v0, Vertex* v1, Vertex* v2);

		Poly& addQuad(Vertex& v0, Vertex& v1, Vertex& v2, Vertex& v3,
			Edge& e0, Edge& e1, Edge& e2, Edge& e3, bool tesselation_type = 1);

		Poly* addQuad(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3, bool tesselation_type = 1);

		void stichVerticesToVertex(Vertex* v, std::vector<Vertex*>& vertices, bool loop = false);

		// addLoneQuad
		// addTris
		// addQuad
		// stichTrisToOne(existing_v, new_v0, new_v1)
		// stichTrisToTwo(existing_v0, existing_v1, new_v)
		// stichQuadToOne()
		// stichQuadToTwo()
		// stichQuadToThree()

		////////////////////////////////////////////////////////////
		void calculateAllNormalsFromWindings();

		////////////////////////////////////////////////////////////
		// delete vertex
		// delete edge
		// delete poly

		// ray query

		void createAsTriangle(float size);
		void createAsCube(float size);
		void createAsCylinder(float height, float diameter, uint32_t vertical_sides, uint32_t horizontal_sides);
		void createAsUV_Sphere(float diameter, uint32_t vertical_sides, uint32_t horizontal_sides);

		void addFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
			bool flip_winding = false);
		void addFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions,
			bool flip_winding = false);
	};
}
