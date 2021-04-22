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
#include "DeferredVector.hpp"
#include "GPU_ShaderTypesMesh.hpp"


/* Brush Pipeline
- ray is cast from camera screen
- ray isects with aabbs
- ray isects with (closest/all tris if passthru) tris => sphere origin
- sphere isects aabbs
- aabbs vertices are sent to GPU for modification
- modified vertices replace old ones
*/

namespace scme {

	enum class ModifiedVertexState {
		UPDATE,
		DELETED
	};

	struct ModifiedVertex {
		uint32_t idx;
		ModifiedVertexState state;
	};


	/*
		away_loop == 0xFFFF'FFFF, vertex is a point, not connected to anything
		aabb == 0xFFFF'FFFF, vertex does not belong to any AABB
	*/
	struct Vertex {
	public:
		glm::vec3 pos;
		glm::vec3 normal;

		uint32_t away_loop;  // any loop attached to vertex that is pointing away from it

		uint32_t aabb;
		uint32_t idx_in_aabb;

	public:
		Vertex() {};

		void init();

		bool isPoint();
	};


	// NOTE_TO_SELF: Making the AABB actully contain the vertices is a terrible idea if you ever
	// need to update the mesh because you need to update all primitives that reference that vertex
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
		// source_v is obtained from the previous loop around the polygon

		// next loop around vertex, if there is no other loop then point to itself
		uint32_t v_next_loop;

		uint32_t poly;  // each loop can only belong to ONE polygon
		uint32_t poly_next_loop;  // next loop around polygon
		uint32_t poly_prev_loop;

		// next loop that is parallel i.e the loop of the adjacent polygon,
		// if there is no other mirror loop the point to self
		uint32_t mirror_loop;

		// index into the gpu vertex buffer where the gpu vertex resides
		// index is placed here because for each CPU vertex there is a GPU vertex for each polygon shared,
		// because indexed rendering is not used
		// uint32_t gpu_v;  

		Loop() {};
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

		uint32_t inner_loop;  // any loop inside polygon

		uint8_t tesselation_type : 1,  // split from 0 to 2 or from 1 to 3
			is_tris : 1,  // is it a triangle or quad
			temp_flag_0 : 1,  // not used
			_pad : 5;

		glm::vec3 tess_normals[2];

		Poly() {};
	};


	// Version 1 & 2: Naive implementation with vectors allocated per element
	// Version 3: Edge list inspired allocation-less primitives with AABBs (top down only search)
	// Version 4: Partial GPU updatable buffer with change log, merged rendering data
	class SculptMesh {
	public:
		uint32_t root_aabb;
		std::vector<VertexBoundingBox> aabbs;

		// TODO:
		// ScultMesh cache locality stupid solution :
		// just store the vertices in verts vector grouped by AABB
		// simple to maintain and fast

		// the first vertex is not part of the mesh, it is only used to denote a deleted polygon in the
		// index buffer
		std::vector<Vertex> verts;
		std::vector<ModifiedVertex> modified_verts;
		dx11::ArrayBuffer<GPU_MeshVertex> gpu_verts;

		std::vector<Loop> loops;

		DeferredVector<Poly> polys;
		std::vector<ModifiedPoly> modified_polys;
		dx11::ArrayBuffer<uint32_t> gpu_indexes;
		dx11::ArrayBuffer<GPU_MeshTriangle> gpu_triangles;
		ComPtr<ID3D11ShaderResourceView> gpu_triangles_srv;

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


		// Internal Data Structures for primitives //////////////////

		// iterate loops around the src vertex, until a loop is found pointing to target vertex
		// return 0xFFFF'FFFF if not found
		uint32_t findLoopFromTo(uint32_t src_vertex, uint32_t target_vertex);

		// appends to the loop list around the vertex
		void registerLoopToSourceVertexList(uint32_t away_loop, uint32_t vertex);

		// appends to the mirror list of the loop
		void registerLoopToMirrorLoopList(uint32_t new_loop, uint32_t existing_loop);


		// Vertex ///////////////////////////////////////////////////

		// calculates vertex normal based on the average normals of the connected polygons
		void calcVertexNormal(Vertex* vertex);

		// schedule a vertex to have it's data updated on the GPU side
		void markVertexFullUpdate(uint32_t vertex);

		//
		void deleteVertex(uint32_t vertex);

		// TODO: move vertex, register vertex if no AABB or nothing if vertex is still in its own AABB

		// Loop ////////////////////////////////////////////////////

		// create a loop starting from source to target
		uint32_t createLoop(uint32_t src_vertex, uint32_t target_vertex);

		void setLoop(uint32_t existing_loop, uint32_t src_vertex, uint32_t target_vertex);

		// creates or returns existing loop from source to target
		uint32_t addLoop(uint32_t src_vertex, uint32_t target_vertex);


		// Poly ////////////////////////////////////////////////////

		glm::vec3 calcWindingNormal(Vertex* v0, Vertex* v1, Vertex* v2);

		// recalculates a polygons normal
		void recalcPolyNormal(Poly* poly);

		// schedule a poly to have it's data updated on the GPU side
		void markPolyFullUpdate(uint32_t poly);

		// creates a new blank triangle
		// uint32_t addTris(Poly*& idx);

		// creates a new triangle from existing vertices, creates new loops between the vertices 
		// if they are not already present
		uint32_t addTris(uint32_t v0, uint32_t v1, uint32_t v2);

		// assemble the tris using blank loops and existing vertices
		void setTris(uint32_t tris, uint32_t l0, uint32_t l1, uint32_t l2,
			uint32_t v0, uint32_t v1, uint32_t v2);

		// creates a new quad from existing vertices, creates new loops between the vertices
		// if they are not already present
		uint32_t addQuad(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);

		// assemble the quad using blank loops and existing vertices
		void setQuad(uint32_t quad, uint32_t l0, uint32_t l1, uint32_t l2, uint32_t l3,
			uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);

		void stichVerticesToVertexLooped(std::vector<uint32_t>& vertices, uint32_t vertex);

		void deletePoly(uint32_t poly);


		// Queries ////////////////////////////////////////////

		bool raycastPoly(glm::vec3& ray_origin, glm::vec3& ray_direction, uint32_t poly, glm::vec3& r_point);

		// TODO: mark vertices that are processed to avoid duplicate runs and increse performance
		bool raycastPolys(glm::vec3& ray_origin, glm::vec3& ray_direction,
			uint32_t& r_isect_poly, float& r_isect_distance, glm::vec3& r_isect_position);


		////////////////////////////////////////////////////////////

		void createAsTriangle(float size);
		void createAsQuad(float size);
		void createAsCube(float size);
		void createAsCylinder(float height, float diameter, uint32_t rows, uint32_t columns, bool capped = true);
		void createAsUV_Sphere(float diameter, uint32_t rows, uint32_t columns);

		// Line (not sure to even bother to set up AABB)
		void createAsLine(glm::vec3& origin, glm::vec3& direction, float length);
		void changeLineOrigin(glm::vec3& new_origin);
		void changeLineDirection(glm::vec3& new_direction);

		void createFromLists(std::vector<uint32_t>& indexes, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals);

		size_t getMemorySizeMegaBytes();
	};
}
