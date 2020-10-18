#pragma once

// Standard
#include <vector>
#include <array>
#include <list>

// GLM
#include "glm\vec3.hpp"

#include "GPU_ShaderTypes.hpp"
#include "ErrorStack.hpp"
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


	struct GPU_Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
	};

	struct CPU_Vertex {
		GPU_Vertex* gpu_vertex;

		std::list<Edge*> edges;
	};


	struct VertexBoundingBox {
		AxisBoundingBox3D aabb;

		std::vector<GPU_Vertex> gpu_vs;
		std::vector<CPU_Vertex> cpu_vs;

		VertexBoundingBox* parent;
		std::list<VertexBoundingBox*> children;
	};


	class Edge {
	public:
		CPU_Vertex* v0;
		CPU_Vertex* v1;

		std::list<Poly*> polys;
	};


	class Poly {
	public:
		glm::vec3 normal;

		std::vector<CPU_Vertex*> vs;
		std::vector<Edge*> edges;

		std::array<uint8_t, 6> tesselation;
		std::array<glm::vec3, 2> tess_normals;

	public:
		void calcNormalForTris();
	};


	class SculptMesh {
	public:
		glm::vec3 pos;
		glm::vec3 rot;

		std::list<VertexBoundingBox> aabbs;
		std::list<Edge> edges;
		std::list<Poly> polys;

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
		// add edge
		Edge& addEdge(CPU_Vertex& v0, CPU_Vertex& v1);

		////////////////////////////////////////////////////////////
		Poly& addTris(CPU_Vertex& v0, CPU_Vertex& v1, CPU_Vertex& v2, Edge& e0, Edge& e1, Edge& e2);
		// addLoneQuad
		// addTris
		// addQuad
		// stichTrisToOne(existing_v, new_v0, new_v1)
		// stichTrisToTwo(existing_v0, existing_v1, new_v)
		// stichQuadToOne()
		// stichQuadToTwo()
		// stichQuadToThree()

		////////////////////////////////////////////////////////////
		// delete vertex
		// delete edge
		// delete poly

		// ray query

		void createAsTriangle(glm::vec3& pos, glm::vec3& rot);
		ErrStack createFromGLTF(std::vector<char>& gltf_file);
		//nui::ErrStack createFromOBJ(std::vector<uint8_t> obj_file);
	};
}
