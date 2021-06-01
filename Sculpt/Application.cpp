
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

MeshInstance* MeshInstanceRef::get()
{
	return &instance_set->instances[index_in_buffer];
}

MeshInstanceRef Application::copyInstance(MeshInstanceRef& source_ref)
{
	MeshInstance* original = source_ref.get();
	MeshInstanceSet* dest_set = original->instance_set;

	uint32_t index_in_buffer;
	MeshInstance& new_instance = dest_set->instances.emplace(index_in_buffer);
	new_instance.instance_set = dest_set;
	new_instance.index_in_buffer = index_in_buffer;
	new_instance.parent_layer = nullptr;
	new_instance.visible = true;
	new_instance.markFullUpdate();

	// pointer is invalid
	original = source_ref.get();

	MeshInstanceRef new_ref;
	new_ref.instance_set = dest_set;
	new_ref.index_in_buffer = index_in_buffer;

	transferInstanceToLayer(new_ref, original->parent_layer);

	return new_ref;
}

void Application::deleteInstance(MeshInstanceRef& ref)
{	
	_unparentInstanceFromParentLayer(ref);

	MeshInstance* inst = ref.get();
	MeshInstanceSet* instance_set = inst->instance_set;
	instance_set->instances.erase(ref.index_in_buffer);

	ModifiedMeshInstance& deleted_inst = instance_set->modified_instances.emplace_back();
	deleted_inst.idx = ref.index_in_buffer;
	deleted_inst.type = ModifiedInstanceType::DELETED;

	// Delete Set
	if (instance_set->instances.size() == 0) {

		Mesh* mesh = instance_set->parent_mesh;

		for (auto iter = mesh->sets.begin(); iter != mesh->sets.end(); iter++) {

			if (&(*iter) == instance_set) {
				mesh->sets.erase(iter);
				break;
			}
		}
	}

	// Delete Mesh
}

void Application::transferInstanceToLayer(MeshInstanceRef& ref, MeshLayer* dest_layer)
{
	MeshInstance* inst = ref.get();

	// unparent from original layer
	if (inst->parent_layer != nullptr) {
		_unparentInstanceFromParentLayer(ref);
	}

	inst->parent_layer = dest_layer;
	dest_layer->instances.push_back(ref);
}

//MeshInstanceRef Application::moveInstanceToDrawcall(MeshInstanceRef& instance, MeshDrawcall* dest_drawcall)
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

void Application::rotateInstanceAroundY(MeshInstance* mesh_instance, float radians)
{
	mesh_instance->transform.rot = glm::rotate(mesh_instance->transform.rot, radians, { 0.f, 1.f, 0.f });
}

MeshDrawcall* Application::createDrawcall()
{
	MeshDrawcall* new_drawcall = &drawcalls.emplace_back();
	new_drawcall->display_mode = DisplayMode::SOLID;
	new_drawcall->is_back_culled = false;
	new_drawcall->render_aabbs = false;

	return new_drawcall;
}

MeshDrawcall& Application::getRootDrawcall()
{
	return drawcalls.front();
}

void Application::shouldRenderAABBsForDrawcall(MeshDrawcall* drawcall, bool yes_or_no)
{
	drawcall->render_aabbs = yes_or_no;

	for (Mesh& mesh : meshes) {
		for (MeshInstanceSet& set : mesh.sets) {

			if (set.drawcall == drawcall) {
				mesh.render_aabbs = yes_or_no;

				// Free CPU Memory
				if (yes_or_no == false) {
					mesh.mesh.gpu_aabb_verts.clear();
					mesh.mesh.gpu_aabb_verts.shrink_to_fit();
				}
				break;
			}
		}
	}
}

MeshLayer* Application::createLayer()
{
	MeshLayer* new_layer = &this->layers.emplace_back();
	new_layer->_parent = nullptr;
	return new_layer;
}

void Application::transferLayer(MeshLayer* child_layer, MeshLayer* parent_layer)
{
	if (child_layer->_parent != nullptr) {
		child_layer->_parent->_children.erase(child_layer);
	}

	child_layer->_parent = parent_layer;
	parent_layer->_children.insert(child_layer);
}

void Application::setLayerVisibility(MeshLayer* layer, bool visible_state)
{
	for (MeshInstanceRef& ref : layer->instances) {
		ref.get()->visible = visible_state;
	}

	for (MeshLayer* child_layer : layer->_children) {
		setLayerVisibility(child_layer, visible_state);
	}
}

Mesh& Application::_createMesh()
{
	Mesh& new_mesh = this->meshes.emplace_back();

	scme::SculptMesh& sculpt_mesh = new_mesh.mesh;
	sculpt_mesh.gpu_triangles_srv = nullptr;
	sculpt_mesh.max_vertices_in_AABB = 1024;

	return new_mesh;
}

MeshInstanceRef Application::_addInstance(Mesh& mesh, MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	if (dest_layer == nullptr) {
		dest_layer = last_used_layer;
	}

	if (dest_drawcall == nullptr) {
		dest_drawcall = last_used_drawcall;
	}

	MeshInstanceSet& new_set = mesh.sets.emplace_back();
	new_set.parent_mesh = &mesh;
	new_set.drawcall = dest_drawcall;

	uint32_t index_in_buffer;
	MeshInstance& new_instance = new_set.instances.emplace(index_in_buffer);
	new_instance.instance_set = &new_set;
	new_instance.index_in_buffer = index_in_buffer;
	new_instance.parent_layer = nullptr;
	new_instance.visible = true;
	new_instance.markFullUpdate();

	MeshInstanceRef new_ref;
	new_ref.instance_set = &new_set;
	new_ref.index_in_buffer = index_in_buffer;

	transferInstanceToLayer(new_ref, dest_layer);

	return new_ref;
}

void Application::_unparentInstanceFromParentLayer(MeshInstanceRef& ref)
{
	MeshInstance* inst = ref.get();

	auto& instances = inst->parent_layer->instances;
	for (auto iter = instances.begin(); iter != instances.end(); iter++) {

		if (iter->instance_set == inst->instance_set &&
			iter->index_in_buffer == inst->index_in_buffer)
		{
			instances.erase(iter);
			break;
		}
	}
}

MeshInstanceRef Application::createEmptyMesh(MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	return _addInstance(_createMesh(), dest_layer, dest_drawcall);
}

MeshInstanceRef Application::createTriangle(CreateTriangleInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	MeshInstanceRef new_ref = createEmptyMesh(dest_layer, dest_drawcall);
	MeshInstance* new_instance = new_ref.get();
	new_instance->transform = info.transform;

	Mesh* mesh = new_instance->instance_set->parent_mesh;
	mesh->mesh.createAsTriangle(info.size);

	return new_ref;
}

MeshInstanceRef Application::createQuad(CreateQuadInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	MeshInstanceRef new_ref = createEmptyMesh(dest_layer, dest_drawcall);
	MeshInstance* new_instance = new_ref.get();
	new_instance->transform = info.transform;

	Mesh* mesh = new_instance->instance_set->parent_mesh;
	mesh->mesh.createAsQuad(info.size);

	return new_ref;
}

MeshInstanceRef Application::createCube(CreateCubeInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	MeshInstanceRef new_ref = createEmptyMesh(dest_layer, dest_drawcall);
	MeshInstance* new_instance = new_ref.get();
	new_instance->transform = info.transform;

	Mesh* mesh = new_instance->instance_set->parent_mesh;
	mesh->mesh.createAsCube(info.size);

	return new_ref;
}

MeshInstanceRef Application::createCylinder(CreateCylinderInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	MeshInstanceRef new_ref = createEmptyMesh(dest_layer, dest_drawcall);
	MeshInstance* new_instance = new_ref.get();
	new_instance->transform = info.transform;

	Mesh* mesh = new_instance->instance_set->parent_mesh;
	mesh->mesh.createAsCylinder(info.height, info.diameter,
		info.rows, info.columns, info.with_caps);

	return new_ref;
}

MeshInstanceRef Application::createUV_Sphere(CreateUV_SphereInfo& info,
	MeshLayer* dest_layer, MeshDrawcall* dest_drawcall)
{
	MeshInstanceRef new_ref = createEmptyMesh(dest_layer, dest_drawcall);
	MeshInstance* new_instance = new_ref.get();
	new_instance->transform = info.transform;

	Mesh* mesh = new_instance->instance_set->parent_mesh;
	mesh->mesh.createAsUV_Sphere(info.diameter, info.rows, info.columns);

	return new_ref;
}

ErrStack Application::importMeshesFromGLTF_File(io::FilePath& path, GLTF_ImporterSettings& settings,
	std::vector<MeshInstanceRef>* r_instances)
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
		err_stack.pushError(code_location, "failed to import GLTF file at the level of GLTF");
		return err_stack;
	}

	// Meshes
	std::vector<Mesh*> new_meshes(structure.meshes.size());

	for (uint32_t i = 0; i < structure.meshes.size(); i++) {

		gltf::Mesh& gltf_mesh = structure.meshes[i];
		gltf::Primitive& gltf_prim = gltf_mesh.primitives.front();

		new_meshes[i] = &_createMesh();
		scme::SculptMesh& sculpt_mesh = new_meshes[i]->mesh;

		if (gltf_prim.positions.size()) {

			if (gltf_prim.indexes.size()) {
				if (gltf_prim.normals.size()) {
					sculpt_mesh.createFromLists(gltf_prim.indexes, gltf_prim.positions, gltf_prim.normals);
				}
				else {
					//new_mesh->addFromLists(prim.indexes, prim.positions, true);
					throw std::exception();
				}
			}
		}
	}

	// Layers (create a layer for each nodes)
	std::vector<MeshLayer*> new_layers(structure.nodes.size());

	for (uint32_t i = 0; i < structure.nodes.size(); i++) {
	
		gltf::Node& node = structure.nodes[i];
		new_layers[i] = createLayer();
		new_layers[i]->name = node.name;
	
		transferLayer(new_layers[i], settings.dest_layer);
	}

	// Instances (assign instances to layers and parent layers among each other)
	for (uint32_t i = 0; i < structure.nodes.size(); i++) {

		gltf::Node& node = structure.nodes[i];
		MeshLayer* new_layer = new_layers[i];

		for (uint64_t child_node : node.children) {
			transferLayer(new_layers[child_node], new_layer);
		}

		if (node.mesh != 0xFFFF'FFFF'FFFF'FFFF) {

			Mesh* new_mesh = new_meshes[node.mesh];

			MeshInstanceRef new_ref = _addInstance(*new_mesh, new_layer, settings.dest_drawcall);

			MeshInstance* new_instance = new_ref.get();
			new_instance->name = node.name;

			if (node.uses_matrix) {

				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(node.matrix,
					new_instance->transform.scale, new_instance->transform.rot, new_instance->transform.pos,
					skew, perspective);
			}
			else {
				new_instance->transform.pos = node.translation;
				new_instance->transform.rot = node.rotation;
				new_instance->transform.scale = node.scale;
			}

			if (r_instances != nullptr) {
				r_instances->push_back(new_ref);
			}
		}
	}

	return err_stack;
}
/*
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
}*/

void Application::joinMeshes(std::vector<MeshInstanceRef>& children, MeshInstanceRef& parent)
{
	MeshInstance* parent_inst = parent.get();
	scme::SculptMesh& parent_mesh = parent_inst->instance_set->parent_mesh->mesh;

	uint32_t vertex_idx;
	uint32_t edge_idx;
	uint32_t poly_idx;
	{
		uint32_t vertex_count = parent_mesh.verts.size();
		uint32_t edge_count = parent_mesh.edges.size();
		uint32_t poly_count = parent_mesh.polys.size();
		for (MeshInstanceRef& ref : children) {

			MeshInstance* child_inst = ref.get();
			scme::SculptMesh& child_mesh = child_inst->instance_set->parent_mesh->mesh;

			vertex_count += child_mesh.verts.size() - 1;  // account for deleted vertex
			edge_count += child_mesh.edges.size();
			poly_count += child_mesh.polys.size();
		}

		// Save where to start adding before starting resizing
		vertex_idx = parent_mesh.verts.lastIndex() + 1;
		edge_idx = parent_mesh.edges.lastIndex() + 1;
		poly_idx = parent_mesh.polys.lastIndex() + 1;

		parent_mesh.verts.resize(vertex_count);
		parent_mesh.edges.resize(edge_count);
		parent_mesh.polys.resize(poly_count);
	}
	
	for (MeshInstanceRef& ref : children) {

		MeshInstance* child_inst = ref.get();
		scme::SculptMesh& child_mesh = child_inst->instance_set->parent_mesh->mesh;

		glm::vec3 offset = child_inst->transform.pos - parent_inst->transform.pos;

		// TODO: account for rotation on position and normal

		uint32_t i = 0;
		{
			auto v_iter = child_mesh.verts.begin();
			v_iter.next();

			for (; v_iter != child_mesh.verts.begin(); v_iter.next()) {

				scme::Vertex& src_vertex = v_iter.get();

				scme::Vertex& dest_vertex = parent_mesh.verts[vertex_idx + i];
				dest_vertex.pos = src_vertex.pos + offset;
				dest_vertex.normal = src_vertex.normal;
				dest_vertex.edge = src_vertex.edge + edge_idx;
				// aabb invalid
				// idx_in_aabb invalid

				parent_mesh.markVertexFullUpdate(v_iter.index());

				i++;
			}
		}

		i = 0;
		for (auto iter = child_mesh.edges.begin(); iter != child_mesh.edges.begin(); iter.next()) {

			scme::Edge& src_edge = iter.get();

			scme::Edge& dest_edge = parent_mesh.edges[edge_idx + i];
			dest_edge.v0 = src_edge.v0 + vertex_idx;
			dest_edge.v0_next_edge = src_edge.v0_next_edge + edge_idx;
			dest_edge.v0_prev_edge = src_edge.v0_prev_edge + edge_idx;

			dest_edge.v1 = src_edge.v1 + vertex_idx;
			dest_edge.v1_next_edge = src_edge.v1_next_edge + edge_idx;
			dest_edge.v1_prev_edge = src_edge.v1_prev_edge + edge_idx;

			if (src_edge.p0 != 0xFFFF'FFFF) {
				dest_edge.p0 = src_edge.p0 + poly_idx;
			}
			else {
				dest_edge.p0 = src_edge.p0;
			}
			
			if (src_edge.p1 != 0xFFFF'FFFF) {
				dest_edge.p1 = src_edge.p1 + poly_idx;
			}
			else {
				dest_edge.p1 = src_edge.p1;
			}

			i++;
		}

		i = 0;
		for (auto iter = child_mesh.polys.begin(); iter != child_mesh.polys.begin(); iter.next()) {

			scme::Poly& src_poly = iter.get();
			scme::Poly& dest_poly = parent_mesh.polys[poly_idx + i];
			dest_poly.normal = src_poly.normal;
			dest_poly.tess_normals[0] = src_poly.tess_normals[0];
			dest_poly.tess_normals[2] = src_poly.tess_normals[1];
			dest_poly.edges[0] = edge_idx + src_poly.edges[0];
			dest_poly.edges[1] = edge_idx + src_poly.edges[1];
			dest_poly.edges[2] = edge_idx + src_poly.edges[2];

			if (src_poly.is_tris == false) {
				dest_poly.edges[3] = edge_idx + src_poly.edges[3];
			}

			dest_poly.tesselation_type = src_poly.tesselation_type;
			dest_poly.is_tris = src_poly.is_tris;
			dest_poly.flip_edge_0 = src_poly.flip_edge_0;
			dest_poly.flip_edge_1 = src_poly.flip_edge_1;
			dest_poly.flip_edge_2 = src_poly.flip_edge_2;
			dest_poly.flip_edge_3 = src_poly.flip_edge_3;

			i++;
		}

		vertex_idx += child_mesh.verts.size() - 1;
		edge_idx += child_mesh.edges.size();
		poly_idx += child_mesh.polys.size();

		deleteInstance(ref);
	}

	parent_mesh.recreateAABBs();
}

bool Application::mouseRaycastInstances(MeshInstanceRef& r_isect_inst, uint32_t& r_isect_poly,
	glm::vec3& r_isect_point)
{
	nui::Input& input = main_window->input;
	renderer.getPixelWorldPosition(input.mouse_x, input.mouse_y, r_isect_point);

	if (r_isect_point.x == FLT_MAX) {
		return false;
	}

	for (Mesh& mesh : meshes) {

		scme::SculptMesh& sculpt_mesh = mesh.mesh;
		
		for (MeshInstanceSet& set : mesh.sets) {
			for (auto iter = set.instances.begin(); iter != set.instances.end(); iter.next()) {

				MeshInstance& inst = iter.get();
				glm::vec3 ray_origin = camera_pos - inst.transform.pos;
				glm::vec3 ray_target = r_isect_point - inst.transform.pos;
				glm::vec3 ray_dir = glm::normalize(ray_target - ray_origin);

				// TODO: account for rotation


			}
		}
	}

	return false;
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
	last_used_layer->_parent = nullptr;

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

void Application::setShadingNormal(uint32_t new_shading_normal)
{
	this->shading_normal = new_shading_normal;
}
