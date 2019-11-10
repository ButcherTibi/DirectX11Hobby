#pragma once

// Standard
#include <array>
#include <vector>
#include <forward_list>
#include <list>
#include <atomic>

// GLM
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>


// Forward decl
class Edge;
class Poly;
class LinkageMesh;


class Vertex
{
public:
	std::forward_list<Vertex>::iterator it;

	// Data
	glm::vec3 pos;
	glm::vec4 color;

	// Linkage
	uint32_t link_edges_count;
	std::forward_list<Edge*> link_edges;

	// Generated when rendering
	uint32_t gpu_idx;
public:
	Vertex(float x, float y, float z);
	Vertex(glm::vec3 const& pos);
	Vertex(glm::vec3 const& pos, glm::vec4 const& color);
};


class Edge
{
public:
	std::forward_list<Edge>::iterator it;

	// Data
	std::array<Vertex*, 2> verts;

	// Linkage
	uint32_t link_polys_count;
	std::forward_list<Poly*> link_polys;
};


struct Side
{
	Vertex* v;
	Edge* e;

public:
	Side(Vertex* v, Edge* e);
};

struct TriangulationTris
{
	std::array<Vertex*, 3> vs;
};

class Poly
{
public:
	std::forward_list<Poly>::iterator it;

	// Data
	uint32_t poly_sides_count;
	std::forward_list<Side> poly_sides;

	// bool clock_wise;
	std::vector<TriangulationTris> tess_tris;

public:
	bool getWinding();

	void tesselate(LinkageMesh* me);
};


/* Mutable Linkage Mesh
 *  */
class LinkageMesh
{
public:
	//uint32_t id;

	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

public:
	std::atomic_uint32_t verts_size;
	std::atomic_uint32_t edges_size;
	std::atomic_uint32_t polys_size;
	std::atomic_uint32_t ttris_count;

	std::forward_list<Vertex> verts;
	std::forward_list<Edge> edges;
	std::forward_list<Poly> polys;
};

/* Vertices */

// Creates a Vertex that is not attached to anything
Vertex* createLoneVertex(LinkageMesh& me, float const& x, float const& y, float const& z);
Vertex* createLoneVertex(LinkageMesh& me, glm::vec3 const& pos);
Vertex* createLoneVertex(LinkageMesh& me, glm::vec3 const& pos, glm::vec4 const& color);


/* Edges */

// Creates a Edge from existing vertices
Edge* bridgeVerts(LinkageMesh& me, Vertex* v0, Vertex* v1);


/* Polygons */

// Creates a Triangle from existing vertices and edges
Poly* createTris(LinkageMesh& me, Vertex* v0, Vertex* v1, Vertex* v2, 
	Edge* e0, Edge* e1, Edge* e2);

// Attaches a triangle to an edge, v is the other vertex
Poly* createTris(LinkageMesh& me, Edge* e, Vertex* v);


// Creates a Quad from existing vertices and edges
Poly* createQuad(LinkageMesh& me, Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3,
	Edge* e0, Edge* e1, Edge* e2, Edge* e3);

// Creates a Quad from scratch that is not attached to anything
Poly* createLoneQuad(LinkageMesh& me, glm::vec3 const& v0, glm::vec3 const& v1, 
	glm::vec3 const& v2, glm::vec3 const& v3, glm::vec4 const& color);
