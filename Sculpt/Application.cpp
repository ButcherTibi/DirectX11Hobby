
// Header
#include "Application.hpp"

// Mine
#include "Geometry.hpp"
#include "GLTF_File.hpp"


Application application;


void MeshInstance::markFullUpdate()
{
	ModifiedMeshInstance& modified_inst = instance_set->modified_instances.emplace_back();
	modified_inst.idx = index_in_buffer;
	modified_inst.type = ModifiedInstanceType::UPDATE;
}

MeshInstance* Application::copyInstance(MeshInstance* source, MeshDrawcall* dest_drawcall)
{
	MeshInstanceSet* instance_set = nullptr;

	if (dest_drawcall == nullptr) {	
		instance_set = source->instance_set;
	}
	else {
		bool found = false;

		// Does mesh have a set with that type of drawcall ?
		Mesh* mesh = source->instance_set->parent_mesh;
		for (MeshInstanceSet& set : mesh->sets) {

			if (set.drawcall == dest_drawcall) {
				instance_set = &set;
				found = true;
				break;
			}
		}

		if (found == false) {
			instance_set = &mesh->sets.emplace_back();
			instance_set->parent_mesh = mesh;
			instance_set->drawcall = dest_drawcall;
		}
	}

	uint32_t index_in_buffer;
	MeshInstance& new_instance = instance_set->instances.emplace(index_in_buffer);
	new_instance.instance_set = instance_set;
	new_instance.index_in_buffer = index_in_buffer;
	new_instance.parent_layer = nullptr;
	new_instance.visible = true;

	new_instance.markFullUpdate();

	transferInstanceToLayer(&new_instance, source->parent_layer);

	return &new_instance;
}

//void Application::deleteInstance(MeshInstance* inst)
//{
//	// delete from parent layer
//	inst->parent_layer->instances.erase(inst);
//
//	for (MeshInstanceSet& set : inst->parent_mesh->sets) {
//
//		if (set.drawcall = inst->parent_drawcall) {
//
//			//set.instances.erase(inst->instance_index_in_set);
//		}
//	}
//}

//MeshInstance* Application::transferInstanceToDrawcall(MeshInstance* mesh_instance, MeshDrawcall* dest_drawcall)
//{
//	last_used_drawcall = dest_drawcall;
//
//	if (mesh_instance->parent_drawcall == dest_drawcall) {
//		return mesh_instance;
//	}
//
//	Mesh* mesh = mesh_instance->parent_mesh;
//	MeshInstanceSet* dest_set = nullptr;
//
//	// find or create destination instance set
//	{
//		for (MeshInstanceSet& set : mesh->sets) {
//			if (set.drawcall == dest_drawcall) {
//				dest_set = &set;
//				break;
//			}
//		}
//
//		if (dest_set == nullptr) {
//			MeshInstanceSet& new_set = mesh->sets.emplace_back();
//			new_set.drawcall = dest_drawcall;
//		}
//	}
//	
//	// create new instance
//	uint32_t instance_index_in_set;
//	MeshInstance& new_instance = dest_set->instances.emplace(instance_index_in_set);
//	new_instance = *mesh_instance;
//	new_instance.instance_index_in_set = instance_index_in_set;
//	new_instance.parent_drawcall = dest_drawcall;
//
//	// delete old instance
//	if (mesh_instance->parent_drawcall != nullptr) {
//		
//		for (MeshInstanceSet& set : mesh->sets) {
//			if (set.drawcall == mesh_instance->parent_drawcall) {
//				
//				set.instances.erase(mesh_instance->instance_index_in_set);
//				break;
//			}
//		}
//	}
//
//	return &new_instance;
//}

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
	mesh_instance->transform.rot = glm::rotate(mesh_instance->transform.rot, radians, { 0.f, 1.f, 0.f });
}

MeshDrawcall* Application::createDrawcall()
{
	MeshDrawcall* new_drawcall = &drawcalls.emplace_back();
	new_drawcall->display_mode = DisplayMode::SOLID;
	new_drawcall->is_back_culled = false;
	new_drawcall->_debug_show_octree = false;

	return new_drawcall;
}

MeshLayer* Application::createLayer()
{
	MeshLayer* new_layer = &this->layers.emplace_back();
	new_layer->parent = nullptr;
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
	for (MeshInstance* instance : layer->instances) {
		instance->visible = visible_state;
	}

	for (MeshLayer* child_layer : layer->children) {
		setLayerVisibility(child_layer, visible_state);
	}
}

MeshInstance* Application::createEmptyMesh(MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	if (dest_layer == nullptr) {
		dest_layer = last_used_layer;
	}

	if (dest_drawcall == nullptr) {
		dest_drawcall = last_used_drawcall;
	}

	Mesh& new_mesh = this->meshes.emplace_back();

	scme::SculptMesh& sculpt_mesh = new_mesh.mesh;
	sculpt_mesh.gpu_triangles_srv = nullptr;
	sculpt_mesh.max_vertices_in_AABB = 1024;

	MeshInstanceSet& new_set = new_mesh.sets.emplace_back();
	new_set.parent_mesh = &new_mesh;
	new_set.drawcall = dest_drawcall;

	uint32_t index_in_buffer;
	MeshInstance& new_instance = new_set.instances.emplace(index_in_buffer);
	new_instance.instance_set = &new_set;
	new_instance.index_in_buffer = index_in_buffer;
	new_instance.parent_layer = nullptr;

	new_instance.visible = true;

	new_instance.markFullUpdate();

	transferInstanceToLayer(&new_instance, dest_layer);

	return &new_instance;
}

MeshInstance* Application::createTriangle(CreateTriangleInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	MeshInstance* new_instance = createEmptyMesh(dest_layer, dest_drawcall);
	new_instance->transform = info.transform;

	Mesh* mesh = new_instance->instance_set->parent_mesh;
	mesh->mesh.createAsTriangle(info.size);

	return new_instance;
}

MeshInstance* Application::createQuad(CreateQuadInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	MeshInstance* new_instance = createEmptyMesh(dest_layer, dest_drawcall);
	new_instance->transform = info.transform;

	Mesh* mesh = new_instance->instance_set->parent_mesh;
	mesh->mesh.createAsQuad(info.size);

	return new_instance;
}

MeshInstance* Application::createCube(CreateCubeInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	MeshInstance* new_instance = createEmptyMesh(dest_layer, dest_drawcall);
	new_instance->transform = info.transform;

	Mesh* mesh = new_instance->instance_set->parent_mesh;
	mesh->mesh.createAsCube(info.size);

	return new_instance;
}
/*
MeshInstance* Application::createCylinder(CreateCylinderInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	if (dest_layer == nullptr) {
		dest_layer = last_used_layer;
	}

	if (dest_drawcall == nullptr) {
		dest_drawcall = last_used_drawcall;
	}

	MeshInBuffer* new_mesh = createMesh();
	new_mesh->mesh.createAsCylinder(info.height, info.diameter,
		info.rows, info.columns, info.with_caps);

	MeshInstance* new_instance = createInstance(new_mesh, dest_layer, dest_drawcall);
	new_instance->pos = info.transform.pos;
	new_instance->rot = info.transform.rot;
	new_instance->scale = info.transform.scale;

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

	MeshInBuffer* new_mesh = createMesh();
	new_mesh->mesh.createAsUV_Sphere(info.diameter, info.rows, info.columns);

	MeshInstance* new_instance = createInstance(new_mesh, dest_layer, dest_drawcall);
	new_instance->pos = info.transform.pos;
	new_instance->rot = info.transform.rot;
	new_instance->scale = info.transform.scale;

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
	std::vector<MeshInBuffer*> new_meshes(structure.meshes.size());

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

			MeshInBuffer* new_mesh = new_meshes[node.mesh];
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

	MeshInBuffer* new_mesh = createMesh();
	new_mesh->mesh.createAsLine(info.origin, info.direction, info.length);

	MeshInstance* new_instance = createInstance(new_mesh, dest_layer, dest_drawcall);
	new_instance->pos = { 0.f, 0.f, 0.f };
	new_instance->rot = { 1.f, 0.f, 0.f, 0.f };
	new_instance->scale = { 1.f, 1.f, 1.f };

	return new_instance;
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

MeshInstance* Application::mouseRaycastInstances(uint32_t& r_isect_poly, glm::vec3& r_isect_point)
{
	// TODO:
	// AABB for mesh instances with the raytrace optimization
	// frustrum culling
	// use instance mask pixels to find first mesh

	nui::Window* window = application.main_window;

	glm::vec3& ray_origin = camera_pos;
	glm::vec3 ray_direction = camera_forward;
	{

		float x_amount = (float)window->input.mouse_x / window->surface_width;
		float y_amount = 1.f - ((float)window->input.mouse_y / window->surface_height);
		float aspect_ratio = (float)window->surface_width / window->surface_height;
		float half_fov = toRad(camera_field_of_view) / 2.f;

		// rotate camera vector left
		if (x_amount > 0.5f) {
			x_amount = remapAboveTo01(x_amount);
			ray_direction = glm::rotateY(ray_direction, -x_amount * half_fov * aspect_ratio);
		}
		else {
			// rotate camera vector right
			x_amount = remapBelowTo01(x_amount);
			ray_direction = glm::rotateY(ray_direction, x_amount * half_fov * aspect_ratio);
		}

		// rotate camera vector up
		if (y_amount > 0.5f) {
			y_amount = remapAboveTo01(y_amount);
			ray_direction = glm::rotateX(ray_direction, y_amount * half_fov);
		}
		else {
			// rotate camera vector down
			y_amount = remapBelowTo01(y_amount);
			ray_direction = glm::rotateX(ray_direction, -y_amount * half_fov);
		}

		// Perspective Distort
		glm::mat4x4 persp = glm::perspectiveFovRH_ZO(camera_field_of_view,
			(float)window->surface_width, (float)window->surface_height,
			camera_z_near, camera_z_far);

		ray_direction = persp * glm::vec4(ray_direction, 1.f);
	}

	uint32_t instance_id;
	renderer.instance_poly_id_staging_tex.readbackAtPixel(window->input.mouse_x, window->input.mouse_y,
		instance_id, r_isect_poly);

	if (instance_id == 0) {
		return nullptr;
	}

	MeshInstance* mesh_instance;
	for (MeshInstance& instance : instances) {
		if (instance.id == instance_id) {
			mesh_instance = &instance;
			break;
		}
	}

	scme::SculptMesh& mesh = mesh_instance->parent_mesh->mesh;
	mesh.raycastPoly(ray_origin, ray_direction, r_isect_poly, r_isect_point);

	if constexpr (false) {

		for (MeshInstance& instance : instances) {
			if (instance.name == "Debug Line") {

				scme::SculptMesh& line_mesh = instance.parent_mesh->mesh;
				line_mesh.changeLineOrigin(ray_origin);
				line_mesh.changeLineDirection(glm::vec3{ 0, 0, 1 });
				break;
			}
		}
	}

	return mesh_instance;
}*/

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

void Application::resetToHardcodedStartup()
{
	// Meshes
	meshes.clear();

	// Drawcalls
	drawcalls.clear();
	last_used_drawcall = createDrawcall();

	// Layers
	layers.clear();
	last_used_layer = &layers.emplace_back();
	last_used_layer->parent = nullptr;

	// Shading
	shading_normal = GPU_ShadingNormal::POLY;

	// Lighting
	lights[0].normal = toNormal(45, 45);
	lights[0].color = { 1, 1, 1 };
	lights[0].intensity = 1.f;

	lights[1].normal = toNormal(-45, 45);
	lights[1].color = { 1, 1, 1 };
	lights[1].intensity = 1.f;

	lights[2].normal = toNormal(45, -45);
	lights[2].color = { 1, 1, 1 };
	lights[2].intensity = 1.f;

	lights[3].normal = toNormal(-45, -45);
	lights[3].color = { 1, 1, 1 };
	lights[3].intensity = 1.f;

	lights[4].intensity = 0.f;
	lights[5].intensity = 0.f;
	lights[6].intensity = 0.f;
	lights[7].intensity = 0.f;

	ambient_intensity = 0.03f;

	// Camera
	camera_focus = { 0, 0, 0 };
	camera_field_of_view = 15.f;
	camera_z_near = 0.1f;
	camera_z_far = 100'000.f;
	camera_pos = { 0, 0, 0 };
	camera_quat_inv = { 1, 0, 0, 0 };
	camera_forward = { 0, 0, -1 };

	camera_orbit_sensitivity = 0.01f;
	camera_pan_sensitivity = 0.0001f;
	camera_dolly_sensitivity = 0.001f;
}
