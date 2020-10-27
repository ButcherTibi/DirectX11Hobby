
// Header
#include "SculptMesh.hpp"

#include "GLTF_File.hpp"


using namespace scme;


void SculptMesh::createAsTriangle(glm::vec3& new_pos, glm::vec3& new_rot, float size)
{
	this->pos = new_pos;
	this->rot = new_rot;

	VertexBoundingBox& root = aabbs.emplace_back();
	root.gpu_vs.resize(3);
	root.cpu_vs.resize(3);

	auto& gpu_vs = root.gpu_vs;
	auto& cpu_vs = root.cpu_vs;

	float half = size / 2;
	gpu_vs[0].pos = { 0, half, 0 };
	gpu_vs[0].normal = { 0, 0, -1 };

	gpu_vs[1].pos = { half, -half, 0 };
	gpu_vs[1].normal = { 0, 0, -1 };

	gpu_vs[2].pos = { -half, -half, 0 };
	gpu_vs[2].normal = { 0, 0, -1 };

	cpu_vs[0].gpu_vertex = &gpu_vs[0];
	cpu_vs[1].gpu_vertex = &gpu_vs[1];
	cpu_vs[2].gpu_vertex = &gpu_vs[2];

	Edge& e0 = addEdge(cpu_vs[0], cpu_vs[1]);
	Edge& e1 = addEdge(cpu_vs[1], cpu_vs[2]);
	Edge& e2 = addEdge(cpu_vs[2], cpu_vs[0]);

	addTris(cpu_vs[0], cpu_vs[1], cpu_vs[2], e0, e1, e2);
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