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
	std::forward_list<Vertex>::iterator self_it;  // iterator to self allows independent deletion

	// Data
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec4 color;

	// Linkage
	uint32_t link_edges_count;
	std::forward_list<Edge*> link_edges;

	// Generated when rendering
	uint32_t gpu_idx;
public:
	Vertex();
	Vertex(float x, float y, float z);
	Vertex(glm::vec3 const& pos);
	Vertex(glm::vec3 const& pos, glm::vec4 const& color);

	void registerEdge(Edge* e);
};


class Edge
{
public:
	std::forward_list<Edge>::iterator self_it;

	// Data
	std::array<Vertex*, 2> verts;

	// Linkage
	uint32_t link_polys_count;
	std::forward_list<Poly*> link_polys;

public:
	void registerPoly(Poly* p);
};


struct Side
{
	Vertex* v;
	Edge* e;

public:
	Side(Vertex* v, Edge* e);
};

/* Because calc normals requires cross product consitent winding 
 * must be maintained to ensure all Tesselation Triangle normals point 
 * in the same relative direction */
struct TesselationTris
{
	std::array<Vertex*, 3> vs;
	glm::vec3 normal;

	// NOTE: check if needs flip
	void calcNormal();
};

class Poly
{
public:
	std::forward_list<Poly>::iterator self_it;

	// Data
	glm::vec3 normal;

	uint32_t poly_sides_count;
	std::forward_list<Side> poly_sides;

	// Generated when rendering
	std::vector<TesselationTris> tess_tris;

public:
	/* calculates the normal that is the average of tesselated tris normals */
	void calcNormal();

	/* finds what winding has the first neighboring polygon
	 * return true for Same Winding Direction */
	bool isNeighboringWindingDifferent();

	/* tesselateAnyWinding with the winding of the first neighbouring polygon */
	void tesselateFirstWinding(LinkageMesh* me);

	/* tesselation with no defined winding */
	void tesselateAnyWinding(LinkageMesh* me);

	void build(LinkageMesh* me);
};


/* Mutable Linkage Mesh
 *  */
class LinkageMesh {
public:
	glm::vec3 position = {0, 0, 0};
	glm::quat rotation = { 1, 0, 0, 0 };
	glm::vec3 scale = {1, 1, 1};

	uint32_t verts_size = 0;
	uint32_t edges_size = 0;
	uint32_t polys_size = 0;
	uint32_t ttris_count = 0;

	std::forward_list<Vertex> verts;
	std::forward_list<Edge> edges;
	std::forward_list<Poly> polys;
};


/* Vertices */

// Creates a Vertex that is not attached to anything
Vertex* initBlankVertex(LinkageMesh& me);
Vertex* initVertex(LinkageMesh& me, glm::vec3 const& pos);
Vertex* initVertex(LinkageMesh& me, float const& x, float const& y, float const& z);
Vertex* initVertex(LinkageMesh& me, glm::vec3 const& pos, glm::vec4 const& color);


/* Edges */

// Creates a Edge from existing vertices
Edge* initEdge(LinkageMesh& me, Vertex* v0, Vertex* v1);

// Create if no edge or return existing edge
Edge* ensureEdge(LinkageMesh& me, Vertex* v0, Vertex* v1);

// Find the Edge connecting the two verts else return nullptr
Edge* findEdge(Vertex* v0, Vertex* v1);


/* Polygons */

// Creates a Triangle from existing vertices and edges
Poly* initTris(LinkageMesh& me, Vertex* v0, Vertex* v1, Vertex* v2, 
	Edge* e0, Edge* e1, Edge* e2);

// Creates a Triangle and if needed it's edges
Poly* fabricateTris(LinkageMesh& me, Vertex* v0, Vertex* v1, Vertex* v2);

// Attaches a triangle to an edge, v is the other vertex
// Poly* addTris(LinkageMesh& me, Edge* e, Vertex* v);


// Creates a Quad from existing vertices and edges
Poly* initQuad(LinkageMesh& me, Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3,
	Edge* e0, Edge* e1, Edge* e2, Edge* e3);

// Creates a Quad as well as it's verts and edges
Poly* fabricateQuad(LinkageMesh& me, glm::vec3 const& v0, glm::vec3 const& v1, 
	glm::vec3 const& v2, glm::vec3 const& v3,
	glm::vec3 normal, glm::vec4 const& color);


/* Linkage Mesh */

struct VertexAtributes {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	// colors
};

// add Tris Polys to mesh, new tris polys are linked together but not linked to existing polys
void addTriangleListToMesh(LinkageMesh& mesh, std::vector<uint32_t>& indexes, VertexAtributes& attrs, bool flip_winding);

// iterates over all vertices and sets their color
void setMeshVertexColor(LinkageMesh& mesh, glm::vec4 new_color);
