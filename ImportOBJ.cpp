

// OBJ
#include "tiny_obj_loader.h"

// Header
#include "Importer.h"


ErrStack importOBJMeshes(Path path, std::vector<LinkageMesh>& meshes)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.toWindowsPath().c_str())) {
		return ErrStack(code_location, "failed to load OBJ, " + warn + err);
	}

	std::vector<Vertex*> verts;

	meshes.resize(shapes.size());
	for (size_t mesh_idx = 0; mesh_idx < shapes.size(); mesh_idx++) {

		tinyobj::mesh_t& obj_mesh = shapes[mesh_idx].mesh;

		verts.resize(obj_mesh.indices.size());

		for (size_t i = 0; i < obj_mesh.indices.size(); i++) {

			tinyobj::index_t& idx = obj_mesh.indices[i];

			verts[i] = initBlankVertex(meshes[mesh_idx]);

			verts[i]->pos = {
				attrib.vertices[3 * idx.vertex_index + 0],
				attrib.vertices[3 * idx.vertex_index + 1],
				attrib.vertices[3 * idx.vertex_index + 2]
			};

			verts[i]->normal = {
				attrib.normals[3 * idx.normal_index + 0],
				attrib.normals[3 * idx.normal_index + 1],
				attrib.normals[3 * idx.normal_index + 2]
			};

			verts[i]->uv = {
				attrib.texcoords[3 * idx.texcoord_index + 0],
				1 - attrib.texcoords[3 * idx.texcoord_index + 1]
			};

			if (i && i % 2 == 0) {
				fabricateTris(meshes[mesh_idx], verts[i - 2], verts[i - 1], verts[i]);
			}
		}
	}

	return ErrStack();
}
