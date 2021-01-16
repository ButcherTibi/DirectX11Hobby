
// Header
#include "Application.hpp"

// Standard
#include <algorithm>

#include "glm\gtx\matrix_decompose.hpp"


Application application;

MeshInBuffers* Application::createMesh()
{
	MeshInBuffers& new_mesh = this->meshes.emplace_back();

	scme::SculptMesh& sculpt_mesh = new_mesh.mesh;
	sculpt_mesh.max_vertices_in_AABB = 1024;

	new_mesh.mesh_vertex_start = 0xFFFF'FFFF;
	new_mesh.aabb_vertex_start = 0xFFFF'FFFF;
	new_mesh.aabb_index_start = 0xFFFF'FFFF;

	return &new_mesh;
}

MeshInstance* Application::createInstance(MeshInBuffers* mesh, MeshLayer* dest_layer, MeshDrawcall* parent_drawcall)
{
	MeshInstance* new_instance = &this->instances.emplace_back();
	new_instance->parent_drawcall = nullptr;
	new_instance->parent_mesh = mesh;
	new_instance->name = "New Instance";
	new_instance->pos = { 0.f, 0.f, 0.f };
	new_instance->rot = { 1.f, 0.f, 0.f, 0.f };
	new_instance->scale = { 1.f, 1.f, 1.f };
	new_instance->instance_id = instance_id++;

	assignInstanceToLayer(new_instance, dest_layer);
	transferToDrawcall(new_instance, parent_drawcall);

	return new_instance;
}

MeshInstance* Application::copyInstance(MeshInstance* source, MeshLayer* dest_layer)
{
	if (dest_layer == nullptr) {
		dest_layer = last_used_layer;
	}

	MeshInstance* new_instance = &this->instances.emplace_back();
	new_instance->parent_drawcall = nullptr;
	new_instance->parent_mesh = source->parent_mesh;
	new_instance->name = source->name + " copy";
	new_instance->pos = source->pos;
	new_instance->rot = source->rot;
	new_instance->scale = source->scale;

	assignInstanceToLayer(new_instance, dest_layer);
	transferToDrawcall(new_instance, source->parent_drawcall);

	return new_instance;
}

void Application::assignInstanceToLayer(MeshInstance* mesh_instance, MeshLayer* dest_layer)
{
	dest_layer->instances.insert(mesh_instance);
	last_used_layer = dest_layer;
}

void Application::searchForInstances(std::string& search, std::vector<MeshInstanceSearchResult>& r_results)
{
	for (MeshInstance& instance : instances) {

		if (instance.name[0] == search[0]) {

			MeshInstanceSearchResult& new_result = r_results.emplace_back();
			new_result.instance = &instance;
			new_result.char_count = 1;

			for (uint32_t char_idx = 1; char_idx < search.size(); char_idx++) {

				if (instance.name[char_idx] == search[char_idx]) {
					new_result.char_count += 1;
				}
				else {
					break;
				}
			}
		}
	}

	// sort results
	std::sort(r_results.begin(), r_results.end(), [](MeshInstanceSearchResult& a, MeshInstanceSearchResult& b) {
		return a.char_count > b.char_count;
	});
}

MeshDrawcall* Application::createDrawcall(std::string& name)
{
	MeshDrawcall* new_drawcall = &drawcalls.emplace_back();
	if (name != "") {
		new_drawcall->name = name;
	}
	else {
		new_drawcall->name = "New Drawcall";
	}

	new_drawcall->rasterizer_mode = DisplayMode::SOLID;
	new_drawcall->is_back_culled = false;
	new_drawcall->show_aabbs = false;

	return new_drawcall;
}

void Application::transferToDrawcall(MeshInstance* mesh_instance, MeshDrawcall* dest_drawcall)
{
	if (mesh_instance->parent_drawcall != nullptr) {

		auto& mesh_instance_sets = mesh_instance->parent_drawcall->mesh_instance_sets;

		for (uint32_t i = 0; i < mesh_instance_sets.size(); i++) {

			MeshInstanceSet& mesh_instance_set = mesh_instance_sets[i];
			if (mesh_instance_set.mesh == mesh_instance->parent_mesh) {

				for (uint32_t j = 0; j < mesh_instance_set.instances.size(); j++) {

					MeshInstance* instance = mesh_instance_set.instances[j];
					if (instance == mesh_instance) {

						mesh_instance_set.instances.erase(mesh_instance_set.instances.begin() + j);

						if (!mesh_instance_set.instances.size()) {
							mesh_instance_sets.erase(mesh_instance_sets.begin() + i);
						}
					}
				}
			}
		}
	}

	mesh_instance->parent_drawcall = dest_drawcall;
	last_used_drawcall = dest_drawcall;

	// add to existing set of mesh instances
	for (MeshInstanceSet& mesh_instances : dest_drawcall->mesh_instance_sets) {
		if (mesh_instances.mesh == mesh_instance->parent_mesh) {
			mesh_instances.mesh_insta_start = 0xFFFF'FFFF;
			mesh_instances.instances.push_back(mesh_instance);
			return;
		}
	}

	// else create a new set of mesh instances
	MeshInstanceSet& mesh_instances = dest_drawcall->mesh_instance_sets.emplace_back();
	mesh_instances.mesh = mesh_instance->parent_mesh;
	mesh_instances.mesh_insta_start = 0xFFFF'FFFF;
	mesh_instances.instances.push_back(mesh_instance);
}

MeshLayer* Application::createLayer(std::string& name, MeshLayer* parent_layer)
{
	if (parent_layer == nullptr) {
		parent_layer = last_used_layer;
	}

	MeshLayer* new_layer = &this->layers.emplace_back();
	if (name != "") {
		new_layer->name = name;
	}
	else {
		new_layer->name = "New Layer";
	}

	parentLayer(new_layer, parent_layer);

	return new_layer;
}

void Application::parentLayer(MeshLayer* child_layer, MeshLayer* parent_layer)
{
	child_layer->parent = parent_layer;
	parent_layer->children.insert(child_layer);
}

MeshInstance* Application::createTriangleMesh(CreateTriangleInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	if (dest_layer == nullptr) {
		dest_layer = last_used_layer;
	}

	if (dest_drawcall == nullptr) {
		dest_drawcall = last_used_drawcall;
	}

	MeshInBuffers* new_mesh = createMesh();
	new_mesh->mesh.createAsTriangle(info.size);

	MeshInstance* new_instance = createInstance(new_mesh, dest_layer, dest_drawcall);
	new_instance->pos = info.transform.pos;
	new_instance->rot = info.transform.rot;
	new_instance->scale = info.transform.scale;

	this->renderer.load_vertices = true;

	return new_instance;
}

MeshInstance* Application::createQuadMesh(CreateQuadInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	if (dest_layer == nullptr) {
		dest_layer = last_used_layer;
	}

	if (dest_drawcall == nullptr) {
		dest_drawcall = last_used_drawcall;
	}

	MeshInBuffers* new_mesh = createMesh();
	new_mesh->mesh.createAsQuad(info.size);

	MeshInstance* new_instance = createInstance(new_mesh, dest_layer, dest_drawcall);
	new_instance->pos = info.transform.pos;
	new_instance->rot = info.transform.rot;
	new_instance->scale = info.transform.scale;

	this->renderer.load_vertices = true;

	return new_instance;
}

MeshInstance* Application::createCubeMesh(CreateCubeInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	if (dest_layer == nullptr) {
		dest_layer = last_used_layer;
	}

	if (dest_drawcall == nullptr) {
		dest_drawcall = last_used_drawcall;
	}

	MeshInBuffers* new_mesh = createMesh();
	new_mesh->mesh.createAsCube(info.size);

	MeshInstance* new_instance = createInstance(new_mesh, dest_layer, dest_drawcall);
	new_instance->pos = info.transform.pos;
	new_instance->rot = info.transform.rot;
	new_instance->scale = info.transform.scale;

	this->renderer.load_vertices = true;

	return new_instance;
}

MeshInstance* Application::createCylinder(CreateCylinderInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	if (dest_layer == nullptr) {
		dest_layer = last_used_layer;
	}

	if (dest_drawcall == nullptr) {
		dest_drawcall = last_used_drawcall;
	}

	MeshInBuffers* new_mesh = createMesh();
	new_mesh->mesh.createAsCylinder(info.height, info.diameter,
		info.vertical_sides, info.horizontal_sides, info.with_caps);

	MeshInstance* new_instance = createInstance(new_mesh, dest_layer, dest_drawcall);
	new_instance->pos = info.transform.pos;
	new_instance->rot = info.transform.rot;
	new_instance->scale = info.transform.scale;

	this->renderer.load_vertices = true;

	return new_instance;
}

MeshInstance* Application::createUV_Sphere(CreateUV_SphereInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	if (dest_layer == nullptr) {
		dest_layer = last_used_layer;
	}

	if (dest_drawcall == nullptr) {
		dest_drawcall = last_used_drawcall;
	}

	MeshInBuffers* new_mesh = createMesh();
	new_mesh->mesh.createAsUV_Sphere(info.diameter, info.vertical_sides, info.horizontal_sides);

	MeshInstance* new_instance = createInstance(new_mesh, dest_layer, dest_drawcall);
	new_instance->pos = info.transform.pos;
	new_instance->rot = info.transform.rot;
	new_instance->scale = info.transform.scale;

	this->renderer.load_vertices = true;

	return new_instance;
}

ErrStack Application::importMeshesFromGLTF_File(io::FilePath& path, GLTF_ImporterSettings& settings,
	std::vector<MeshInstance*>* r_instances)
{
	ErrStack err_stack;

	if (settings.dest_layer == nullptr) {
		settings.dest_layer = last_used_layer;
	}

	if (settings.dest_drawcall == nullptr) {
		settings.dest_drawcall = last_used_drawcall;
	}

	gltf::Structure structure;

	err_stack = structure.importGLTF(path);
	if (err_stack.isBad()) {
		err_stack.pushError(code_location, "failed to import GLTF file at the level GLTF");
		return err_stack;
	}

	// Meshes
	std::vector<MeshInBuffers*> new_meshes(structure.meshes.size());

	for (uint32_t i = 0; i < structure.meshes.size(); i++) {

		gltf::Mesh& gltf_mesh = structure.meshes[i];
		gltf::Primitive& gltf_prim = gltf_mesh.primitives.front();

		new_meshes[i] = createMesh();
		scme::SculptMesh& sculpt_mesh = new_meshes[i]->mesh;

		if (gltf_prim.positions.size()) {

			if (gltf_prim.indexes.size()) {
				if (gltf_prim.normals.size()) {
					sculpt_mesh.createFromLists(gltf_prim.indexes, gltf_prim.positions, gltf_prim.normals);
				}
				else {
					//new_mesh->addFromLists(prim.indexes, prim.positions, true);
				}
			}
		}
	}

	// Layers (create a layer for each nodes)
	std::vector<MeshLayer*> new_layers(structure.nodes.size());

	for (uint32_t i = 0; i < structure.nodes.size(); i++) {
	
		gltf::Node& node = structure.nodes[i];
		new_layers[i] = createLayer(node.name, settings.dest_layer);
	}

	// Instances (assign instances to layers and parent layers among each other)
	for (uint32_t i = 0; i < structure.nodes.size(); i++) {

		gltf::Node& node = structure.nodes[i];
		MeshLayer* new_layer = new_layers[i];

		for (uint64_t child_node : node.children) {
			parentLayer(new_layers[child_node], new_layer);
		}

		if (node.mesh != 0xFFFF'FFFF'FFFF'FFFF) {

			MeshInBuffers* new_mesh = new_meshes[node.mesh];
			MeshInstance* new_instance = createInstance(new_mesh, new_layer, settings.dest_drawcall);
			new_instance->name = node.name;

			if (node.uses_matrix) {

				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(node.matrix, new_instance->scale, new_instance->rot, new_instance->pos,
					skew, perspective);
			}
			else {
				new_instance->pos = node.translation;
				new_instance->rot = node.rotation;
				new_instance->scale = node.scale;
			}

			if (r_instances != nullptr) {
				r_instances->push_back(new_instance);
			}
		}
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
