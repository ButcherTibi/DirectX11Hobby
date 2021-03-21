
// Header
#include "Application.hpp"

// Mine
#include "Geometry.hpp"


Application application;


void Application::_deleteFromDrawcall(MeshInstance* instance)
{
	auto& mesh_instance_sets = instance->parent_drawcall->mesh_instance_sets;

	for (uint32_t i = 0; i < mesh_instance_sets.size(); i++) {

		MeshInstanceSet& mesh_instance_set = mesh_instance_sets[i];
		if (mesh_instance_set.mesh == instance->parent_mesh) {

			for (uint32_t j = 0; j < mesh_instance_set.instances.size(); j++) {

				MeshInstance* existing_instance = mesh_instance_set.instances[j];
				if (existing_instance == instance) {

					mesh_instance_set.instances.erase(mesh_instance_set.instances.begin() + j);

					if (!mesh_instance_set.instances.size()) {
						mesh_instance_sets.erase(mesh_instance_sets.begin() + i);
					}
				}
			}
		}
	}
}

MeshInBuffers* Application::createMesh()
{
	MeshInBuffers& new_mesh = this->meshes.emplace_back();

	scme::SculptMesh& sculpt_mesh = new_mesh.mesh;
	sculpt_mesh.max_vertices_in_AABB = 1024;

	new_mesh.mesh_vertex_start = 0xFFFF'FFFF;
	new_mesh.aabbs_vertex_start = 0xFFFF'FFFF;

	return &new_mesh;
}

MeshInstance* Application::createInstance(MeshInBuffers* mesh, MeshLayer* dest_layer, MeshDrawcall* parent_drawcall)
{
	MeshInstance* new_instance = &this->instances.emplace_back();
	new_instance->parent_layer = nullptr;
	new_instance->parent_drawcall = nullptr;
	new_instance->parent_mesh = mesh;
	
	new_instance->id = instance_id;
	instance_id++;

	new_instance->visible = true;
	new_instance->can_collide = true;
	new_instance->pos = { 0.f, 0.f, 0.f };
	new_instance->rot = { 1.f, 0.f, 0.f, 0.f };
	new_instance->scale = { 1.f, 1.f, 1.f };

	transferInstanceToLayer(new_instance, dest_layer);
	transferInstanceToDrawcall(new_instance, parent_drawcall);
	mesh->child_instances.insert(new_instance);

	return new_instance;
}

MeshInstance* Application::copyInstance(MeshInstance* source)
{
	MeshInstance* new_instance = &this->instances.emplace_back();
	new_instance->parent_layer = nullptr;
	new_instance->parent_drawcall = nullptr;
	new_instance->parent_mesh = source->parent_mesh;
	new_instance->name = source->name + " copy";
	new_instance->pos = source->pos;
	new_instance->rot = source->rot;
	new_instance->scale = source->scale;

	transferInstanceToLayer(new_instance, source->parent_layer);
	transferInstanceToDrawcall(new_instance, source->parent_drawcall);
	source->parent_mesh->child_instances.insert(new_instance);

	return new_instance;
}

void Application::deleteInstance(MeshInstance* inst)
{
	// remove from layers
	for (MeshLayer& layer : layers) {
		layer.instances.erase(inst);
	}

	// remove from drawcall
	_deleteFromDrawcall(inst);

	// remove from mesh
	inst->parent_mesh->child_instances.erase(inst);

	// remove mesh if that instance was the only one
	if (!inst->parent_mesh->child_instances.size())
	{
		for (auto it = meshes.begin(); it != meshes.end(); it++) {

			if (&(*it) == inst->parent_mesh) {
				meshes.erase(it);
				break;
			}
		}
	}

	// remove instance
	for (auto it = instances.begin(); it != instances.end(); it++) {

		if (&(*it) == inst) {
			instances.erase(it);
			break;
		}
	}
}

void Application::transferInstanceToLayer(MeshInstance* mesh_instance, MeshLayer* dest_layer)
{
	// unparent from original layer
	if (mesh_instance->parent_layer != nullptr) {

		MeshLayer* parent = mesh_instance->parent_layer;
		parent->instances.erase(mesh_instance);
	}

	mesh_instance->parent_layer = dest_layer;
	dest_layer->instances.insert(mesh_instance);
}

void Application::rotateInstanceAroundY(MeshInstance* mesh_instance, float radians)
{
	mesh_instance->rot = glm::rotate(mesh_instance->rot, radians, { 0.f, 1.f, 0.f });
}

void Application::searchForInstances(std::string& keyword, std::vector<MeshInstanceSearchResult>& r_results)
{
	for (MeshInstance& instance : instances) {

		if (instance.name[0] == keyword[0]) {

			MeshInstanceSearchResult& new_result = r_results.emplace_back();
			new_result.instance = &instance;
			new_result.char_count = 1;

			for (uint32_t char_idx = 1; char_idx < keyword.size(); char_idx++) {

				if (instance.name[char_idx] == keyword[char_idx]) {
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
	new_drawcall->_debug_show_octree = false;

	return new_drawcall;
}

void Application::transferInstanceToDrawcall(MeshInstance* mesh_instance, MeshDrawcall* dest_drawcall)
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

	transferLayer(new_layer, parent_layer);

	return new_layer;
}

void Application::transferLayer(MeshLayer* child_layer, MeshLayer* parent_layer)
{
	if (child_layer->parent != nullptr) {

		child_layer->parent->children.erase(child_layer);
	}

	child_layer->parent = parent_layer;
	parent_layer->children.insert(child_layer);
}

void Application::setLayerVisibility(MeshLayer* layer, bool visible_state)
{
	bool can_collide = visible_state ? false : true;

	for (MeshInstance* instance : layer->instances) {
		instance->visible = visible_state;
		instance->can_collide = can_collide;
	}

	for (MeshLayer* child_layer : layer->children) {
		setLayerVisibility(child_layer, visible_state);
	}
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
		info.rows, info.columns, info.with_caps);

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
	new_mesh->mesh.createAsUV_Sphere(info.diameter, info.rows, info.columns);

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
			transferLayer(new_layers[child_node], new_layer);
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

MeshInstance* Application::createLine(CreateLineInfo& info, MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	if (dest_layer == nullptr) {
		dest_layer = last_used_layer;
	}

	if (dest_drawcall == nullptr) {
		dest_drawcall = last_used_drawcall;
	}

	MeshInBuffers* new_mesh = createMesh();
	new_mesh->mesh.createAsLine(info.origin, info.direction, info.length);

	MeshInstance* new_instance = createInstance(new_mesh, dest_layer, dest_drawcall);
	new_instance->pos = { 0.f, 0.f, 0.f };
	new_instance->rot = { 1.f, 0.f, 0.f, 0.f };
	new_instance->scale = { 1.f, 1.f, 1.f };

	this->renderer.load_vertices = true;

	return new_instance;
}

void Application::lookupInstanceMask(uint32_t pixel_x, uint32_t pixel_y, uint32_t& r_instance_id)
{
	// TODO: optimize for multi lookup

	// Map
	renderer.im_ctx3->Map(renderer.instance_id_staging_tex.Get(), 0, D3D11_MAP_READ, 0, &_instance_id_mask);

	uint32_t idx = (pixel_y * _instance_id_mask.RowPitch) + (pixel_x * 4);
	
	uint32_t r = *(((uint8_t*)_instance_id_mask.pData) + idx);
	uint32_t g = *(((uint8_t*)_instance_id_mask.pData) + idx + 1);
	uint32_t b = *(((uint8_t*)_instance_id_mask.pData) + idx + 2);
	uint32_t a = *(((uint8_t*)_instance_id_mask.pData) + idx + 3);

	r_instance_id = (a << 24) | (b << 16) | (g << 8) | r;

	// Unmap
	renderer.im_ctx3->Unmap(renderer.instance_id_staging_tex.Get(), 0);
}

bool Application::raycast(MeshInstance* mesh_instance, glm::vec3& ray_origin, glm::vec3& ray_direction,
	uint32_t& r_isect_poly, float& r_isect_distance, glm::vec3& r_isect_point)
{
	glm::vec3 local_ray_origin = ray_origin - mesh_instance->pos;
	local_ray_origin = local_ray_origin * mesh_instance->rot;

	glm::vec3 local_ray_dir = ray_direction * mesh_instance->rot;
	
	// debug
	if (false) {

		for (MeshDrawcall& drawcall : drawcalls) {
			if (drawcall.name == "wire pure") {

				CreateLineInfo info;
				info.origin = local_ray_origin;
				info.direction = local_ray_dir;
				info.length = 100.f;
				
				MeshInstance* inst = createLine(info, nullptr, &drawcall);
				inst->wireframe_colors.front_color = { 1.f, 1, 1 };

				break;
			}
		}
	}

	scme::SculptMesh& mesh = mesh_instance->parent_mesh->mesh;

	return mesh.raycastPolys(local_ray_origin, local_ray_dir, r_isect_poly, r_isect_distance, r_isect_point);
}

struct RaytraceResult {
	uint32_t poly;
	glm::vec3 point;
	MeshInstance* instance;
};

MeshInstance* Application::mouseRaycastInstances(uint32_t& r_isect_poly, float& r_isect_distance,
	glm::vec3& r_isect_point)
{
	// TODO:
	// AABB for mesh instances with the raytrace optimization
	// frustrum culling
	// use instance mask pixels to find first mesh
	
	glm::vec3 ray_direction = camera_forward;

	nui::Window* window = application.main_window;
	float x_amount = (float)window->input.mouse_x / window->surface_width;
	float y_amount = 1.f - ((float)window->input.mouse_y / window->surface_height);
	float aspect_ratio = (float)window->surface_width / window->surface_height;
	float half_fov = toRad(camera_field_of_view) / 2.f;

	if (x_amount > 0.5f) {
		x_amount = remapAboveTo01(x_amount);
		ray_direction = glm::rotateY(ray_direction, -x_amount * half_fov * aspect_ratio);
	}
	else {
		x_amount = remapBelowTo01(x_amount);
		ray_direction = glm::rotateY(ray_direction, x_amount * half_fov * aspect_ratio);
	}

	if (y_amount > 0.5f) {
		y_amount = remapAboveTo01(y_amount);
		ray_direction = glm::rotateX(ray_direction, y_amount * half_fov);
	}
	else {
		y_amount = remapBelowTo01(y_amount);
		ray_direction = glm::rotateX(ray_direction, -y_amount * half_fov);
	}

	printf("ray dir = %.2f %.2f, %.2f \n",
		ray_direction.x, ray_direction.y, ray_direction.z);



	r_isect_distance = FLT_MAX;
	MeshInstance* r_isect_instance = nullptr;

	for (MeshInstance& instance : instances) {

		if (instance.visible) {

			uint32_t isect_poly;
			float isect_distance;
			glm::vec3 isect_point;

			if (application.raycast(&instance, camera_pos, ray_direction,
				isect_poly, isect_distance, isect_point))
			{
				if (isect_distance < r_isect_distance) {

					r_isect_poly = isect_poly;
					r_isect_distance = isect_distance;
					r_isect_point = isect_point;
					r_isect_instance = &instance;
				}
			}
		}
	}

	//// debug
	//if (true) {

	//	for (MeshDrawcall& drawcall : drawcalls) {
	//		if (drawcall.name == "wire pure") {

	//			CreateLineInfo info;
	//			info.origin = ray_origin;
	//			info.direction = ray_direction;
	//			info.length = 10.f;

	//			MeshInstance* inst = createLine(info, nullptr, &drawcall);
	//			inst->wireframe_colors.front_color = { 1, 1, 1 };

	//			break;
	//		}
	//	}
	//}

	return r_isect_instance;
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
	glm::vec3 camera_right = glm::normalize(glm::vec3{ 1, 0, 0 } * camera_quat_inv);
	glm::vec3 camera_up = glm::normalize(glm::vec3{ 0, 1, 0 } * camera_quat_inv);

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
	delta_rot = glm::rotate(delta_rot, toRad(deg_y), camera_right);
	delta_rot = glm::rotate(delta_rot, toRad(deg_x), { 0, 1, 0 });
	delta_rot = glm::normalize(delta_rot);

	camera_pos = (camera_pos - camera_focus) * delta_rot;
	this->camera_pos += camera_focus;

	glm::quat reverse_rot = camera_quat_inv;
	reverse_rot = glm::rotate(reverse_rot, toRad(deg_y), camera_right);
	reverse_rot = glm::rotate(reverse_rot, toRad(deg_x), { 0, 1, 0 });

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
	// glm::vec3 camera_forward_axis = glm::normalize(glm::vec3{ 0, 0, -1 } * camera_quat_inv);

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
