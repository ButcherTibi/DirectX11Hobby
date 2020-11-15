
// Header
#include "SculptMesh.hpp"

#include "GLTF_File.hpp"


using namespace scme;


void SculptMesh::createAsTriangle(glm::vec3& new_pos, glm::quat& new_rot, float size)
{
	this->pos = new_pos;
	this->rot = new_rot;

	VertexBoundingBox& root = aabbs.emplace_back();
	root.vs.resize(3);

	auto& vs = root.vs;

	float half = size / 2;
	vs[0].pos = { 0, half, 0 };
	vs[0].normal = { 0, 0, -1 };

	vs[1].pos = { half, -half, 0 };
	vs[1].normal = { 0, 0, -1 };

	vs[2].pos = { -half, -half, 0 };
	vs[2].normal = { 0, 0, -1 };

	Edge& e0 = addEdge(vs[0], vs[1]);
	Edge& e1 = addEdge(vs[1], vs[2]);
	Edge& e2 = addEdge(vs[2], vs[0]);

	addTris(vs[0], vs[1], vs[2], e0, e1, e2);
}

/*
        4--------5
       /|       /|
      / |      / |
     0--------1  |
     |  7-----|--6
     | /      | /
     |/       |/
     3--------2
*/

void SculptMesh::createAsCube(glm::vec3& new_pos, glm::quat& new_rot, float size)
{
	this->pos = new_pos;
	this->rot = new_rot;

	VertexBoundingBox& root = aabbs.emplace_back();
	root.vs.resize(8);

	auto& vs = root.vs;

	float half = size / 2;

	// Front
	vs[0].pos = { -half,  half, -half };
	vs[1].pos = {  half,  half, -half };
	vs[2].pos = {  half, -half, -half };
	vs[3].pos = { -half, -half, -half };

	// Back
	vs[4].pos = { -half,  half, half };
	vs[5].pos = {  half,  half, half };
	vs[6].pos = {  half, -half, half };
	vs[7].pos = { -half, -half, half };

	// Front
	Edge& e01 = addEdge(vs[0], vs[1]);
	Edge& e12 = addEdge(vs[1], vs[2]);
	Edge& e23 = addEdge(vs[2], vs[3]);
	Edge& e30 = addEdge(vs[3], vs[0]);

	// Back
	Edge& e45 = addEdge(vs[4], vs[5]);
	Edge& e56 = addEdge(vs[5], vs[6]);
	Edge& e67 = addEdge(vs[6], vs[7]);
	Edge& e74 = addEdge(vs[7], vs[4]);

	// Left
	Edge& e04 = addEdge(vs[0], vs[4]);
	Edge& e37 = addEdge(vs[3], vs[7]);

	// Right
	Edge& e15 = addEdge(vs[1], vs[5]);
	Edge& e26 = addEdge(vs[2], vs[6]);

	// Front Face
	addQuad(vs[0], vs[1], vs[2], vs[3],
		e01, e12, e23, e30);

	// Right Face
	addQuad(vs[1], vs[5], vs[6], vs[2],
		e15, e56, e26, e12);

	// Back Face
	addQuad(vs[5], vs[4], vs[7], vs[6],
		e45, e74, e67, e56);

	// Left Face
	addQuad(vs[4], vs[0], vs[3], vs[7],
		e04, e30, e37, e74);

	// Top Face
	addQuad(vs[0], vs[4], vs[5], vs[1],
		e04, e45, e15, e01);

	// Bottom Face
	addQuad(vs[3], vs[2], vs[6], vs[7],
		e23, e26, e67, e37);
}

ErrStack SculptMesh::createFromGLTF(std::vector<char>& gltf_file)
{
	ErrStack err_stack;

	gltf::Structure gltf_struct;
	{
		json::Graph json_graph;
		checkErrStack(json_graph.importJSON(gltf_file),
			"failed to read json");

		checkErrStack(gltf_struct.importGLTF(json_graph),
			"failed to import gltf file");
	}

	// put all verts inside the box
	// connect them


	return err_stack;
}