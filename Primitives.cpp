
// Standard
#include <array>

// GLM
#include <glm/vec3.hpp>

// Other
#include "Meshes.h"

#include "Primitives.h"


/*
       0
      / \
     /   \
    2-----1
*/
void createTriangleMesh(CreateTriangleInfo info, LinkageMesh& me)
{
	// Mesh Data
	me.position = info.pos;
	me.rotation = info.rotation;
	me.scale = info.scale;

	// Verts
	Vertex* v0 = createLoneVertex(me, 0.0f, 0.0f, 1.0f);
	Vertex* v1 = createLoneVertex(me, 0.0f, 0.5f, 0.0f);
	Vertex* v2 = createLoneVertex(me, 0.0f, -0.5f, 0.0f);

	// Edge
	Edge* e0 = bridgeVerts(me, v0, v1);
	Edge* e1 = bridgeVerts(me, v1, v2);
	Edge* e2 = bridgeVerts(me, v2, v0);

	// Polys
	createTris(me, v0, v1, v2, e0, e1, e2);
}

/*
	Verts          Edges
	    4-------5    4
	   /       /|  7   5
	  / |     / |    6
	 /       /  |        8   9
	0--------1  |
	|   7 - -|--6       11   10
	|  /     | /     0
	|        |/    3   1
	3--------2       2
*/
void createCubeMesh(CreateCubeInfo info, LinkageMesh& me)
{
	// Mesh data
	me.position = info.pos;
	me.rotation = info.rotation;
	me.scale = info.scale;

	float x_dim = info.x_dim;
	float y_dim = info.y_dim;
	float z_dim = info.z_dim;

	// Front Verts
	Vertex* v0 = createLoneVertex(me, -x_dim, +y_dim, z_dim);
	Vertex* v1 = createLoneVertex(me, +x_dim, +y_dim, z_dim);
	Vertex* v2 = createLoneVertex(me, +x_dim, -y_dim, z_dim);
	Vertex* v3 = createLoneVertex(me, -x_dim, -y_dim, z_dim);

	// Back Verts
	Vertex* v4 = createLoneVertex(me, -x_dim, +y_dim, -z_dim);
	Vertex* v5 = createLoneVertex(me, +x_dim, +y_dim, -z_dim);
	Vertex* v6 = createLoneVertex(me, +x_dim, -y_dim, -z_dim);
	Vertex* v7 = createLoneVertex(me, -x_dim, -y_dim, -z_dim);

	// Front Edges
	Edge* e0 = bridgeVerts(me, v0, v1);
	Edge* e1 = bridgeVerts(me, v1, v2);
	Edge* e2 = bridgeVerts(me, v2, v3);
	Edge* e3 = bridgeVerts(me, v3, v0);

	// Back Edges
	Edge* e4 = bridgeVerts(me, v4, v5);
	Edge* e5 = bridgeVerts(me, v5, v6);
	Edge* e6 = bridgeVerts(me, v6, v7);
	Edge* e7 = bridgeVerts(me, v7, v4);

	// Middle Edges
	Edge* e8 = bridgeVerts(me, v0, v4);
	Edge* e9 = bridgeVerts(me, v1, v5);
	Edge* e10 = bridgeVerts(me, v2, v6);
	Edge* e11 = bridgeVerts(me, v3, v7);

	// Polys
	Poly* front_p = createQuad(me, v0, v1, v2, v3,
		e0, e1, e2, e3);
	Poly* back_p = createQuad(me, v7, v6, v5, v4,
		e7, e6, e5, e4);

	Poly* top_p = createQuad(me, v4, v5, v1, v0,
		e4, e9, e0, e8);
	Poly* bot_p = createQuad(me, v7, v3, v2, v6,
		e11, e2, e10, e6);
	
	Poly* right_p = createQuad(me, v1, v5, v6, v2,
		e9, e5, e10, e1);
	Poly* left_p = createQuad(me, v0, v3, v7, v4,
		e3, e11, e7, e8);
}

void createCoordinateCubeMesh(CreateCoordinateCubeInfo info, LinkageMesh& me)
{
	// Mesh Data
	me.position = info.pos;
	me.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	me.scale = info.scale;

	float x_dim = info.x_dim / 2.0f;
	float y_dim = info.y_dim / 2.0f;
	float z_dim = info.z_dim / 2.0f;	

	auto blue = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	auto blue_lite = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);

	auto red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	auto red_lite = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);

	auto green = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	auto green_lite = glm::vec4(0.5f, 1.0f, 0.5f, 1.0f);

	std::array<glm::vec3, 8> v;

	// Front Verts
	v[0] = glm::vec3(-x_dim, +y_dim, z_dim);
	v[1] = glm::vec3(+x_dim, +y_dim, z_dim);
	v[2] = glm::vec3(+x_dim, -y_dim, z_dim);
	v[3] = glm::vec3(-x_dim, -y_dim, z_dim);

	// Back Verts
	v[4] = glm::vec3(-x_dim, +y_dim, -z_dim);
	v[5] = glm::vec3(+x_dim, +y_dim, -z_dim);
	v[6] = glm::vec3(+x_dim, -y_dim, -z_dim);
	v[7] = glm::vec3(-x_dim, -y_dim, -z_dim);

	// Z Axis
	Poly* front_p = createLoneQuad(me, v[0], v[1], v[2], v[3], blue_lite);
	Poly* back_p = createLoneQuad(me, v[7], v[6], v[5], v[4], blue);

	// X Axis
	Poly* right_p = createLoneQuad(me, v[1], v[5], v[6], v[2], red);
	Poly* left_p = createLoneQuad(me, v[0], v[3], v[7], v[4], red_lite);

	// Y Axis
	Poly* top_p = createLoneQuad(me, v[4], v[5], v[1], v[0], green);
	Poly* bot_p = createLoneQuad(me, v[7], v[3], v[2], v[6], green_lite);

	// Inside cube (for checking culling)
	{
		for (glm::vec3& v_pos : v) {
			v_pos /= 2.0f;
		}

		auto pink = glm::vec4(1.0f, 0.5f, 1.0f, 1.0f);

		// Z Axis
		Poly* front_p = createLoneQuad(me, v[3], v[2], v[1], v[0], pink);
		Poly* back_p = createLoneQuad(me, v[4], v[5], v[6], v[7], pink);

		// X Axis
		Poly* right_p = createLoneQuad(me, v[2], v[6], v[5], v[1], pink);
		Poly* left_p = createLoneQuad(me, v[4], v[7], v[3], v[0], pink);

		// Y Axis
		Poly* top_p = createLoneQuad(me, v[0], v[1], v[5], v[4], pink);
		Poly* bot_p = createLoneQuad(me, v[6], v[2], v[3], v[7], pink);
	}
}
