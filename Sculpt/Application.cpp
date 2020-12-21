
// Header
#include "Application.hpp"

#include "glm\gtx\matrix_decompose.hpp"


Application application;

MeshInstance& Application::createTriangleMesh(CreateTriangleInfo& info)
{
	scme::SculptMesh& new_mesh = this->meshes.emplace_back();
	new_mesh.createAsTriangle(info.size);

	MeshInstance& new_instance = this->instances.emplace_back();
	new_instance.mesh = &new_mesh;
	new_instance.pos = info.transform.pos;
	new_instance.rot = info.transform.rot;
	new_instance.scale = info.transform.scale;

	new_instance.mesh_shading_subprimitive = info.mesh_shading_subprimitive;
	new_instance.albedo_color = info.material.albedo_color;
	new_instance.roughness = info.material.roughness;
	new_instance.metallic = info.material.metallic;
	new_instance.specular = info.material.specular;

	MeshLayer& root = this->layers.front();
	root.instances.push_back(&new_instance);

	this->renderer.load_vertices = true;

	return new_instance;
}

MeshInstance& Application::createQuadMesh(CreateQuadInfo& info)
{
	scme::SculptMesh& new_mesh = this->meshes.emplace_back();
	new_mesh.createAsQuad(info.size);

	MeshInstance& new_instance = this->instances.emplace_back();
	new_instance.mesh = &new_mesh;
	new_instance.pos = info.transform.pos;
	new_instance.rot = info.transform.rot;
	new_instance.scale = info.transform.scale;

	new_instance.mesh_shading_subprimitive = info.mesh_shading_subprimitive;
	new_instance.albedo_color = info.material.albedo_color;
	new_instance.roughness = info.material.roughness;
	new_instance.metallic = info.material.metallic;
	new_instance.specular = info.material.specular;

	MeshLayer& root = this->layers.front();
	root.instances.push_back(&new_instance);

	this->renderer.load_vertices = true;

	return new_instance;
}

MeshInstance& Application::createCubeMesh(CreateCubeInfo& info)
{
	scme::SculptMesh& new_mesh = this->meshes.emplace_back();
	new_mesh.createAsCube(info.size);

	MeshInstance& new_instance = this->instances.emplace_back();
	new_instance.mesh = &new_mesh;
	new_instance.pos = info.transform.pos;
	new_instance.rot = info.transform.rot;
	new_instance.scale = info.transform.scale;

	new_instance.mesh_shading_subprimitive = info.mesh_shading_subprimitive;
	new_instance.albedo_color = info.material.albedo_color;
	new_instance.roughness = info.material.roughness;
	new_instance.metallic = info.material.metallic;
	new_instance.specular = info.material.specular;

	MeshLayer& root = this->layers.front();
	root.instances.push_back(&new_instance);

	this->renderer.load_vertices = true;

	return new_instance;
}

MeshInstance& Application::createCylinder(CreateCylinderInfo& info)
{
	scme::SculptMesh& new_mesh = this->meshes.emplace_back();
	new_mesh.createAsCylinder(info.height, info.diameter,
		info.vertical_sides, info.horizontal_sides, info.with_caps);

	MeshInstance& new_instance = this->instances.emplace_back();
	new_instance.mesh = &new_mesh;
	new_instance.pos = info.transform.pos;
	new_instance.rot = info.transform.rot;
	new_instance.scale = info.transform.scale;

	new_instance.mesh_shading_subprimitive = info.mesh_shading_subprimitive;
	new_instance.albedo_color = info.material.albedo_color;
	new_instance.roughness = info.material.roughness;
	new_instance.metallic = info.material.metallic;
	new_instance.specular = info.material.specular;

	MeshLayer& root = this->layers.front();
	root.instances.push_back(&new_instance);

	this->renderer.load_vertices = true;

	return new_instance;
}

MeshInstance& Application::createUV_Sphere(CreateUV_SphereInfo& info)
{
	scme::SculptMesh& new_mesh = this->meshes.emplace_back();
	new_mesh.createAsUV_Sphere(info.diameter, info.vertical_sides, info.horizontal_sides);

	MeshInstance& new_instance = this->instances.emplace_back();
	new_instance.mesh = &new_mesh;
	new_instance.pos = info.pos;
	new_instance.rot = info.rot;
	new_instance.scale = info.scale;

	new_instance.mesh_shading_subprimitive = info.mesh_shading_subprimitive;
	new_instance.albedo_color = info.albedo_color;
	new_instance.roughness = info.roughness;
	new_instance.metallic = info.metallic;
	new_instance.specular = info.specular;

	MeshLayer& root = this->layers.front();
	root.instances.push_back(&new_instance);

	this->renderer.load_vertices = true;

	return new_instance;
}

ErrStack Application::importGLTF_File(io::FilePath& path, GLTF_ImporterSettings& settings)
{
	ErrStack err_stack;

	gltf::Structure structure;

	err_stack = structure.importGLTF(path);
	if (err_stack.isBad()) {
		err_stack.pushError(code_location, "failed to import GLTF file at the level GLTF");
		return err_stack;
	}

	// Meshes
	std::vector<scme::SculptMesh*> new_meshes(structure.meshes.size());
	for (scme::SculptMesh*& new_mesh : new_meshes) {
		new_mesh = &this->meshes.emplace_back();
	}

	uint32_t i = 0;
	for (gltf::Mesh& mesh : structure.meshes) {

		gltf::Primitive& prim = mesh.primitives.front();
		scme::SculptMesh* new_mesh = new_meshes[i];

		if (prim.positions.size()) {

			if (prim.indexes.size()) {
				if (prim.normals.size()) {
					//new_mesh->addFromLists(prim.indexes, prim.positions, prim.normals, true);
				}
				else {
					//new_mesh->addFromLists(prim.indexes, prim.positions, true);
				}
			}
		}

		i++;
	}

	// Layers
	auto& root_children = this->layers.front().children;
	std::vector<MeshLayer*> new_layers(structure.nodes.size());

	for (MeshLayer*& new_layer : new_layers) {
		new_layer = &this->layers.emplace_back();
		root_children.push_back(new_layer);  // add new layers to root layer
	};

	i = 0;
	for (gltf::Node& node : structure.nodes) {

		MeshLayer* new_layer = new_layers[i];
		new_layer->name = node.name;
		new_layer->children.resize(node.children.size());

		// Child Index to Child Pointer conversion
		uint32_t child_idx = 0;
		for (MeshLayer* child_layer : new_layer->children) {

			child_layer = new_layers[node.children[child_idx]];
			child_idx++;
		}

		if (node.mesh != 0xFFFF'FFFF'FFFF'FFFF) {

			MeshInstance& new_instance = this->instances.emplace_back();
			// new_instance.shading = MeshShadingSubPrimitive::SMOOTH_VERTEX;
			
			if (node.uses_matrix) {

				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(node.matrix, new_instance.scale, new_instance.rot, new_instance.pos,
					skew, perspective);
			}
			else {
				new_instance.pos = node.translation;
				new_instance.rot = node.rotation;
				new_instance.scale = node.scale;
			}

			new_layer->instances.push_back(&new_instance);
		}

		i++;
	}

	this->renderer.load_vertices = true;

	return err_stack;
}

void Application::setCameraFocus(glm::vec3& new_focus)
{
	this->camera_focus = new_focus;
}

// TODO: ask stack exchange about below

/* Attempt 1: */
/*glm::quat rotation_x = glm::rotate(glm::quat(1, 0, 0, 0), toRad(deg_x), { 1, 0, 0 });
glm::quat rotation_y = glm::rotate(glm::quat(1, 0, 0, 0), toRad(deg_y), { 0, 1, 0 });
mesh.rot = glm::normalize(mesh.rot * rotation_x * rotation_y);*/

/* Attempt 2: */
/*mesh.rot = glm::rotate(mesh.rot, toRad(deg_x), { 1, 0, 0 });
mesh.rot = glm::rotate(mesh.rot, toRad(deg_y), { 0, 1, 0 });
mesh.rot = glm::normalize(mesh.rot);*/

/* Attempt 3: Rotation Around X Local, Rotation Around Y Screen */
/*mesh.rot = glm::rotate(mesh.rot, toRad(deg_x), { 1, 0, 0 });

glm::vec3 y_axis = { 0, 1, 0 };
y_axis = y_axis * mesh.rot;
mesh.rot = glm::rotate(mesh.rot, toRad(deg_y), y_axis);

mesh.rot = glm::normalize(mesh.rot);*/

/* Attempt 4: Rotation Around X Screen, Rotation Around Y Local */
/*glm::vec3 x_axis = { 1, 0, 0 };
x_axis = x_axis * mesh.rot;
mesh.rot = glm::rotate(mesh.rot, toRad(deg_x), x_axis);
mesh.rot = glm::rotate(mesh.rot, toRad(deg_y), { 0, 1, 0 });

mesh.rot = glm::normalize(mesh.rot);*/

/* Attempt 5: Rotation Around X Screen, Rotation Around Y Screen */
/*glm::vec3 x_axis = { 1, 0, 0 };
x_axis = x_axis * mesh.rot;
mesh.rot = glm::rotate(mesh.rot, toRad(deg_x), x_axis);

glm::vec3 y_axis = { 0, 1, 0 };
y_axis = y_axis * mesh.rot;
mesh.rot = glm::rotate(mesh.rot, toRad(deg_y), y_axis);

mesh.rot = glm::normalize(mesh.rot);*/

// if I rotate a quaternion by position then clusterfuck

void Application::arcballOrbitCamera(float deg_x, float deg_y)
{
	glm::vec3 camera_right = glm::normalize(glm::vec3{ 1, 0, 0 } *camera_quat_inv);
	glm::vec3 camera_up = glm::normalize(glm::vec3{ 0, 1, 0 } *camera_quat_inv);

	glm::quat delta_rot = { 1, 0, 0, 0 };
	delta_rot = glm::rotate(delta_rot, toRad(deg_y), camera_right);
	delta_rot = glm::rotate(delta_rot, toRad(deg_x), camera_up);
	delta_rot = glm::normalize(delta_rot);

	camera_pos = (camera_pos - camera_focus) * delta_rot;
	this->camera_pos += camera_focus;

	glm::quat reverse_rot = camera_quat_inv;
	reverse_rot = glm::rotate(reverse_rot, toRad(deg_y), camera_right);
	reverse_rot = glm::rotate(reverse_rot, toRad(deg_x), camera_up);

	this->camera_quat_inv = reverse_rot;

	this->camera_forward = glm::normalize(glm::vec3{ 0, 0, -1 } *camera_quat_inv);

	renderer.load_uniform = true;
}

void Application::pivotCameraAroundY(float deg_x, float deg_y)
{
	glm::vec3 camera_right = glm::normalize(glm::vec3{ 1, 0, 0 } * camera_quat_inv);
	glm::vec3 camera_up = glm::normalize(glm::vec3{ 0, 1, 0 } * camera_quat_inv);

	glm::quat delta_rot = { 1, 0, 0, 0 };
	delta_rot = glm::rotate(delta_rot, toRad(deg_x), camera_right);
	delta_rot = glm::rotate(delta_rot, toRad(deg_y), { 0, 1, 0 });
	delta_rot = glm::normalize(delta_rot);

	camera_pos = (camera_pos - camera_focus) * delta_rot;
	this->camera_pos += camera_focus;

	glm::quat reverse_rot = camera_quat_inv;
	reverse_rot = glm::rotate(reverse_rot, toRad(deg_x), camera_right);
	reverse_rot = glm::rotate(reverse_rot, toRad(deg_y), { 0, 1, 0 });

	this->camera_quat_inv = reverse_rot;

	renderer.load_uniform = true;
}

void Application::panCamera(float amount_x, float amount_y)
{
	glm::vec3 camera_right = glm::normalize(glm::vec3{ 1, 0, 0 } * camera_quat_inv);
	glm::vec3 camera_up = glm::normalize(glm::vec3{ 0, 1, 0 } * camera_quat_inv);

	float dist = glm::distance(camera_focus, camera_pos);
	if (!dist) {
		dist = 1;
	}

	this->camera_pos += amount_x * dist * camera_right + (-amount_y) * dist * camera_up;

	renderer.load_uniform = true;
}

void Application::dollyCamera(float amount)
{
	glm::vec3 camera_forward = glm::normalize(glm::vec3{ 0, 0, -1 } * camera_quat_inv);

	float dist = glm::distance(camera_focus, camera_pos);
	if (!dist) {
		dist = 1;
	}

	this->camera_pos += amount * dist * camera_forward;

	renderer.load_uniform = true;
}

void Application::setCameraPosition(float x, float y, float z)
{
	this->camera_pos = { x, y, z };

	renderer.load_uniform = true;
}

void Application::setCameraRotation(float x, float y, float z)
{
	glm::quat x_rot = glm::rotate(glm::quat{ 1, 0, 0, 0 }, x, { 1, 0, 0 });
	glm::quat y_rot = glm::rotate(glm::quat{ 1, 0, 0, 0 }, y, { 0, 1, 0 });
	glm::quat z_rot = glm::rotate(glm::quat{ 1, 0, 0, 0 }, z, { 0, 0, 1 });

	this->camera_quat_inv = glm::normalize(x_rot * y_rot * z_rot);

	this->camera_forward = glm::normalize(glm::vec3{ 0, 0, -1 } *camera_quat_inv);

	renderer.load_uniform = true;
}
