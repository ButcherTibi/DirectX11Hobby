#pragma once

// Standard
#include <vector>
#include <array>
#include <list>

// GLM
#include "glm\vec3.hpp"

// DirectX 11
#include "DX11Wrapper.hpp"

#include "ErrorStack.hpp"
#include "Geometry.hpp"
#include "SparseVector.hpp"
#include "GPU_ShaderTypesMesh.hpp"


// Needs:
// uint32_t edge_idx;
// Edge* edge;
#define iterEdgesAroundVertexStart \
	do { 

// edges around a vertex don't have consitent orientation because they are shared by different polygons
// so check at what end of the edge to iterate around
// so that you always iterate around the same vertex
#define iterEdgesAroundVertexEnd(vertex_idx, start_edge_idx) \
		if (edge->v0 == vertex_idx) { \
			edge_idx = edge->v0_next_edge; \
		} \
		else { \
			edge_idx = edge->v1_next_edge; \
		} \
		edge = &edges[edge_idx]; \
	} while (edge_idx != start_edge_idx);


namespace scme {

	enum class ModifiedVertexState {
		UPDATE,
		DELETED
	};

	struct ModifiedVertex {
		uint32_t idx;
		ModifiedVertexState state;
	};


	// edge == 0xFFFF'FFFF, vertex is a point, not connected to anything
	// aabb == 0xFFFF'FFFF, vertex does not belong to any AABB
	struct Vertex {
	public:
		glm::vec3 pos;
		glm::vec3 normal;

		uint32_t edge;  // any edge attached to vertex

		uint32_t aabb;
		uint32_t idx_in_aabb;

	public:
		Vertex() {};

		void init();

		bool isPoint();
	};


	// NOTES:
	// - Making the AABB actully contain the vertices is a terrible idea if you ever
	//   need to update the mesh because you need to update all primitives that reference that vertex
	// - All AABBs get divided into 8, even if child AABBs are unused,
	//   this is to not require storing which vertices belong to each child in a buffer before assigning them to
	//   to the child AABB, as well as to enable resizing the AABB vector only once for all 8 children
	struct VertexBoundingBox {
		uint32_t parent;
		uint32_t children[8];

		AxisBoundingBox3D aabb;

		uint32_t verts_deleted_count;
		std::vector<uint32_t> verts;

		//bool _debug_show_tesselation;  // TODO:

	public:
		bool isLeaf();
		bool hasVertices();
	};


	/* Winged-edge data structure */
	struct Edge {
	public:
		// Double Linked list of edges around vertices
		uint32_t v0;
		uint32_t v0_next_edge;
		uint32_t v0_prev_edge;

		uint32_t v1;
		uint32_t v1_next_edge;
		uint32_t v1_prev_edge;

		// Polygons
		uint32_t p0;
		uint32_t p1;

		Edge() {};

		// edges don't have consistent orientation so a list edges around a vertex
		// may not have all v0 == vertex_around
		uint32_t& nextEdgeOf(uint32_t vertex);
		uint32_t& prevEdgeOf(uint32_t vertex);
		void setPrevNextEdges(uint32_t vertex, uint32_t prev_edge, uint32_t next_edge);
	};


	enum class ModifiedPolyState {
		UPDATE,
		DELETED
	};
	
	struct ModifiedPoly {
		uint32_t idx;
		ModifiedPolyState state;
	};


	struct Poly {
	public:
		// the normal of the triangle or 
		// the average normal of the 2 triangles composing the quad
		glm::vec3 normal;
		glm::vec3 tess_normals[2];

		uint32_t edges[4];

		uint8_t tesselation_type : 1,  // split from 0 to 2 or from 1 to 3
			is_tris : 1,  // is it a triangle or quad	
			flip_edge_0 : 1,  // orientation of the edges for faster iteration of vertices around poly
			flip_edge_1 : 1,  // do not replace with edges[i].v0 == edges[i + 1].v0 stuff, it's slower
			flip_edge_2 : 1,
			flip_edge_3 : 1, : 2;

		Poly() {};
	};
	// NOTE TO SELF: wrong bit field syntax breaks MSVC hard


	// History:
	// Version 1 & 2: Naive implementation with vectors allocated per element
	// Version 3: Edge list inspired allocation-less primitives with AABBs (top down only search)
	// Version 4: Partial GPU updatable buffer with change log, merged rendering data
	// Version 5: Moved from Helf Edge to Winged Edge Data Structure, 
	//   removed support for multiple connected polygons
	class SculptMesh {
	public:
		uint32_t root_aabb;
		std::vector<VertexBoundingBox> aabbs;

		SparseVector<Vertex> verts;
		std::vector<ModifiedVertex> modified_verts;
		dx11::ArrayBuffer<GPU_MeshVertex> gpu_verts;

		SparseVector<Edge> edges;

		SparseVector<Poly> polys;
		std::vector<ModifiedPoly> modified_polys;
		dx11::ArrayBuffer<uint32_t> gpu_indexes;
		dx11::ArrayBuffer<GPU_MeshTriangle> gpu_triangles;
		ComPtr<ID3D11ShaderResourceView> gpu_triangles_srv;

		// AABBs
		std::vector<GPU_MeshVertex> gpu_aabb_verts;
		dx11::Buffer aabb_vbuff;

		// Settings
		uint32_t max_vertices_in_AABB;

	public:
		// Memory caching for intersections
		std::vector<VertexBoundingBox*> _now_aabbs;
		std::vector<VertexBoundingBox*> _next_aabbs;
		std::vector<VertexBoundingBox*> _traced_aabbs;

		void _deleteVertexMemory(uint32_t vertex);
		void _deleteEdgeMemory(uint32_t edge);
		void _deletePolyMemory(uint32_t poly);

		void printEdgeListOfVertex(uint32_t vertex_idx);

	public:
		// Axis Aligned Bounding Box ////////////////////////////////

		void _transferVertexToAABB(uint32_t vertex, uint32_t destination_aabb);

		void moveVertexInAABBs(uint32_t vertex, uint32_t starting_aabb = 0);

		void recreateAABBs(uint32_t max_vertices_in_AABB = 0);


		// Internal Data Structures for primitives //////////////////

		// return 0xFFFF'FFFF if not found
		uint32_t findEdgeBetween(uint32_t vertex_0, uint32_t vertex_1);

		// appends to the edge list around the vertex
		void registerEdgeToVertexList(uint32_t new_edge, uint32_t vertex);

		// removes from edge list around the vertex
		// trying to unregister the last edge has no effect
		void unregisterEdgeFromVertex(Edge* delete_edge, uint32_t vertex_idx, Vertex* vertex);

		void registerPolyToEdge(uint32_t new_poly, uint32_t edge);

		void unregisterPolyFromEdge(uint32_t delete_poly, uint32_t edge);

		// Vertex ///////////////////////////////////////////////////

		// calculates vertex normal based on the average normals of the connected polygons
		// only called when updating vertex gpu data
		void calcVertexNormal(uint32_t vertex);

		// schedule a vertex to have it's data updated on the GPU side
		void markVertexFullUpdate(uint32_t vertex);

		//
		void deleteVertex(uint32_t vertex);

		// TODO: move vertex, register vertex if no AABB or nothing if vertex is still in its own AABB

		// Loop ////////////////////////////////////////////////////

		// create a edge between vertices
		uint32_t createEdge(uint32_t v0, uint32_t v1);

		void setEdge(uint32_t blank_edge, uint32_t v0, uint32_t v1);

		// creates or returns existing edge between vertex 0 and vertex 1
		uint32_t addEdge(uint32_t v0, uint32_t v1);


		// Poly ////////////////////////////////////////////////////

		// calculate a normal based on the winding order of the vertices
		// only called when updating poly gpu data
		glm::vec3 calcWindingNormal(Vertex* v0, Vertex* v1, Vertex* v2);

		// schedule a poly to have it's data updated on the GPU side
		void markPolyFullUpdate(uint32_t poly);

		// creates a new triangle from existing vertices, creates new edges between the vertices 
		// if they are not already present
		uint32_t addTris(uint32_t v0, uint32_t v1, uint32_t v2);

		// like addTris but doesn't create a new polygon
		void setTris(uint32_t blank_tris, uint32_t v0, uint32_t v1, uint32_t v2);

		// creates a new quad from existing vertices, creates new edges between the vertices
		// if they are not already present
		uint32_t addQuad(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);

		// like addQuad but doesn't create a new polygon
		void setQuad(uint32_t blank_quad, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);

		// think capping a cylinder
		void stichVerticesToVertexLooped(std::vector<uint32_t>& vertices, uint32_t vertex);

		void deletePoly(uint32_t poly);

		void getTrisPrimitives(Poly* poly, std::array<uint32_t, 3>& r_vertex_indexes, std::array<Vertex*, 3>& r_vertices);
		void getTrisPrimitives(Poly* poly, std::array<uint32_t, 3>& r_vertex_indexes);
		void getTrisPrimitives(Poly* poly, std::array<Vertex*, 3>& r_vertices);

		void getQuadPrimitives(Poly* poly, std::array<uint32_t, 4>& r_vertex_indexes, std::array<Vertex*, 4>& r_vertices);
		void getQuadPrimitives(Poly* poly, std::array<uint32_t, 4>& r_vertex_indexes);
		void getQuadPrimitives(Poly* poly, std::array<Vertex*, 4>& r_vertices);


		// Queries ////////////////////////////////////////////

		bool raycastPoly(glm::vec3& ray_origin, glm::vec3& ray_direction, uint32_t poly, glm::vec3& r_point);

		// TODO: mark vertices that are processed to avoid duplicate runs and increse performance
		bool raycastPolys(glm::vec3& ray_origin, glm::vec3& ray_direction,
			uint32_t& r_isect_poly, glm::vec3& r_isect_position);


		// Creation //////////////////////////////////////////////////////////
		void createAsTriangle(float size, uint32_t max_vertices_in_AABB);
		void createAsQuad(float size, uint32_t max_vertices_in_AABB);
		void createAsCube(float size, uint32_t max_vertices_AABB);
		void createAsCylinder(float height, float diameter, uint32_t rows, uint32_t columns, bool capped, uint32_t max_vertices_AABB);
		void createAsUV_Sphere(float diameter, uint32_t rows, uint32_t columns, uint32_t max_vertices_AABB);

		// Line (not sure to even bother to set up AABB)
		void createAsLine(glm::vec3& origin, glm::vec3& direction, float length);
		void changeLineOrigin(glm::vec3& new_origin);
		void changeLineDirection(glm::vec3& new_direction);

		void createFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions,
			std::vector<glm::vec3>& normals, uint32_t max_vertices_AABB);
	
		
		// Prints /////////////////////////////////////////////////////////////

		void printVerices();


		// Other
	};
}
