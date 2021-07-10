#pragma once

// Standard
#include <vector>
#include <array>
#include <list>
#include <chrono>

// GLM
#include "glm\vec3.hpp"

// DirectX 11
#include "DX11Wrapper.hpp"

#include "ErrorStack.hpp"
#include "Geometry.hpp"
#include "SparseVector.hpp"
#include "GPU_ShaderTypesMesh.hpp"


using SteadyTime = std::chrono::time_point<std::chrono::steady_clock>;


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
		uint32_t idx;  // vertex that is changed and must be updated on the GPU
		ModifiedVertexState state;
	};


	// edge == 0xFFFF'FFFF, vertex is a point, not connected to anything
	// aabb == 0xFFFF'FFFF, vertex does not belong to any AABB
	struct Vertex {
	public:
		glm::vec3 pos;
		glm::vec3 normal;

		uint32_t edge;  // any edge attached to vertex

		uint32_t aabb;  // to leaf AABB does this vertex belong
		uint32_t idx_in_aabb;  // where to find vertex in parent AABB (makes AABB transfers faster)

	public:
		Vertex() {};

		void init();

		bool isPoint();  // is the vertex atached to anything
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

		AxisBoundingBox3D<> aabb;
		glm::vec3 mid;

		uint32_t verts_deleted_count;  // how many empty slots does this AABB have
		std::vector<uint32_t> verts;  // indexes of contained vertices

		//bool _debug_show_tesselation;  // TODO:

	public:
		bool isLeaf();
		bool hasVertices();

		uint32_t inWhichChildDoesPositionReside(glm::vec3& pos);
	};


	struct VertexBoundingBox2 {
		//AxisBoundingBox3D<> aabb;

		//uint32_t verts_deleted_count;  // how many empty slots does this AABB have
		std::vector<uint32_t> verts;  // indexes of contained vertices
	};


	/* Winged-edge data structure */
	struct Edge {
	public:
		// Double Linked list of edges around vertices
		uint32_t v0;
		uint32_t v0_next_edge;  // next/prev edge around vertex 0
		uint32_t v0_prev_edge;

		uint32_t v1;
		uint32_t v1_next_edge;  // next/prev edge around vertex 1
		uint32_t v1_prev_edge;

		// Polygons
		uint32_t p0;
		uint32_t p1;

		uint8_t was_raycast_tested : 1,  // used in poly raycasting to mark edge as tested
			: 7;

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
			flip_edge_3 : 1,
			: 2;

		Poly() {};
	};
	// NOTE TO SELF: wrong bit field syntax breaks MSVC hard


	struct StandardBrushInfo {
		SteadyTime last_sample_time;

		glm::vec3 last_pos;

		glm::vec3 end_pos;
		SteadyTime end_time;
		
		float diameter;
		float focus;
		float strength;
	};


	enum class TesselationModificationBasis {
		MODIFIED_POLYS,  // update the tesselation for each polygon

		// update the tesselation for each polygon around a modified vertex
		// this is slower and is used only when vertices are modified
		MODIFIED_VERTICES  
	};


	// History:
	// Version 1 & 2: Naive implementation with vectors allocated per element
	// Version 3: Edge list inspired allocation-less primitives with AABBs (top down only search)
	// Version 4: Partial GPU updatable buffer with change log, merged rendering data
	// Version 5: Moved from Helf Edge to Winged Edge Data Structure, 
	//   removed support for multiple connected polygons
	// Version 6: All updates are batched and send to compute shaders to be applied,
	//   vertex normal generation is on demand
	class SculptMesh {
	public:
		// AABBs
		uint32_t root_aabb_idx;
		std::vector<VertexBoundingBox> aabbs;
		std::vector<GPU_MeshVertex> aabb_verts;
		dx11::ArrayBuffer<GPU_MeshVertex> gpu_aabb_verts;

		// New AABBs
		//uint32_t root_aabb_size;  // the size of the root AABB that contains all other AABBs
		//uint32_t aabbs_levels;  // the depth of the AABB graph

		// Vertex
		SparseVector<Vertex> verts;
		std::vector<ModifiedVertex> modified_verts;
		dx11::ArrayBuffer<GPU_MeshVertex> gpu_verts;

		// Edge
		SparseVector<Edge> edges;

		// Poly
		SparseVector<Poly> polys;
		std::vector<ModifiedPoly> modified_polys;
		dx11::ArrayBuffer<uint32_t> gpu_indexes;
		dx11::ArrayBuffer<GPU_MeshTriangle> gpu_triangles;

		// Settings
		uint32_t max_vertices_in_AABB;

	public:
		// mark vertex as deleted in both CPU and GPU memory
		void _deleteVertexMemory(uint32_t vertex);

		// mark edge as deleted in CPU
		void _deleteEdgeMemory(uint32_t edge);

		// mark poly as deleted in both CPU and GPU memory
		void _deletePolyMemory(uint32_t poly);

		void printEdgeListOfVertex(uint32_t vertex_idx);

	public:
		void init();

		// Axis Aligned Bounding Box ////////////////////////////////

		void _transferVertexToAABB(uint32_t vertex, uint32_t destination_aabb);

		void _recreateAABBs();

		void moveVertexInAABBs(uint32_t vertex);

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

		//
		void deleteVertex(uint32_t vertex);


		// Edge ////////////////////////////////////////////////////

		// create a edge between vertices
		uint32_t createEdge(uint32_t v0, uint32_t v1);

		void setEdge(uint32_t blank_edge, uint32_t v0, uint32_t v1);

		// creates or returns existing edge between vertex 0 and vertex 1
		uint32_t addEdge(uint32_t v0, uint32_t v1);


		// Poly ////////////////////////////////////////////////////

		// calculate a normal based on the winding order of the vertices
		// only called when updating poly gpu data
		glm::vec3 calcWindingNormal(Vertex* v0, Vertex* v1, Vertex* v2);

		void calcPolyNormal(Poly* poly);

		

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
		std::vector<VertexBoundingBox*> _now_aabbs;
		std::vector<VertexBoundingBox*> _next_aabbs;
		std::vector<VertexBoundingBox*> _traced_aabbs;
		std::vector<uint32_t> _tested_edges;


		// Creation //////////////////////////////////////////////////////////
		void createAsTriangle(float size, uint32_t max_vertices_in_AABB);
		void createAsQuad(float size, uint32_t max_vertices_in_AABB);
		void createAsWavyGrid(float size, uint32_t max_vertices_in_AABB);
		void createAsCube(float size, uint32_t max_vertices_AABB);
		void createAsCylinder(float height, float diameter, uint32_t rows, uint32_t columns, bool capped, uint32_t max_vertices_AABB);
		void createAsUV_Sphere(float diameter, uint32_t rows, uint32_t columns, uint32_t max_vertices_AABB);

		// Line (not sure to even bother to set up AABB)
		void createAsLine(glm::vec3& origin, glm::vec3& direction, float length);
		void changeLineOrigin(glm::vec3& new_origin);
		void changeLineDirection(glm::vec3& new_direction);

		void createFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions,
			std::vector<glm::vec3>& normals, uint32_t max_vertices_AABB);
	
		
		// Sculpt /////////////////////////////////////////////////////////////

		void standardBrush(StandardBrushInfo& info);


		// GPU Updates

		// schedule a vertex to have it's data updated on the GPU side
		void markVertexFullUpdate(uint32_t vertex);

		// schedule a poly to have it's data updated on the GPU side
		void markPolyFullUpdate(uint32_t poly);

		void markAllVerticesForNormalUpdate();

		// upload vertex additions and removals to GPU
		void uploadVertexAddsRemoves();
		bool dirty_vertex_list;

		// upload vertex positions changes to GPU
		void uploadVertexPositions();
		bool dirty_vertex_pos;

		// upload vertex normals changes to GPU
		void uploadVertexNormals();
		bool dirty_vertex_normals;

		// upload poly additions and removals to GPU
		void uploadIndexBufferChanges();
		bool dirty_index_buff;

		// uploads which tesselation triangles have changed
		// computes on the GPU normals for polygons
		// downloads the results and applies them
		// 
		// the based_on parameter is used to determine on what basis should the tesselation be updated
		void uploadTesselationTriangles(
			TesselationModificationBasis based_on = TesselationModificationBasis::MODIFIED_POLYS);
		bool dirty_tess_tris;


		// Debug /////////////////////////////////////////////////////////////

		void printVerices();

		void validateCPU_GPU_Mirroring();
	};
}
