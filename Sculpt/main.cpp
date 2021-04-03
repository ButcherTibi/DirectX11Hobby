
#include "RenderDocIntegration.hpp"
#include "NuiLibrary.hpp"
#include "Application.hpp"


void renderDocTriggerCapture(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	render_doc.triggerCapture();
}

void onMouseMove(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	/*uint32_t isect_poly;
	float isect_distance;
	glm::vec3 isect_point;

	MeshInstance* inst = application.mouseRaycastInstances(isect_poly, isect_distance, isect_point);
	if (inst != nullptr) {
		printf("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH \n");
	}*/

	uint32_t x = application.main_window->input.mouse_x;
	uint32_t y = application.main_window->input.mouse_y;
	uint32_t id;
	application.lookupInstanceMask(x, y, id);

	printf("%d \n", id);
}

void onCameraOrbitKeyDown(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	nui::Grid* grid = std::get_if<nui::Grid>(source);
	grid->beginMouseDelta();
}

void onCameraOrbitKeyHeld(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	int32_t delta_x = window->input.mouse_delta_x;
	int32_t delta_y = window->input.mouse_delta_y;

	float scaling = application.camera_orbit_sensitivity * application.ui_instance.delta_time;
	application.arcballOrbitCamera((float)delta_x * scaling, (float)delta_y * scaling);
}

void onCameraOrbitKeyUp(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	// nui::Grid* grid = std::get_if<nui::Grid>(source);
	window->endMouseDelta();
}

void onCameraPanKeyDown(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	auto grid = std::get_if<nui::Grid>(source);
	grid->beginMouseDelta();
}

void onCameraPanKeyHeld(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	int32_t delta_x = window->input.mouse_delta_x;
	int32_t delta_y = window->input.mouse_delta_y;

	float scaling = application.camera_pan_sensitivity * application.ui_instance.delta_time;
	application.panCamera((float)-delta_x * scaling, (float)-delta_y * scaling);
}

void onCameraPanKeyUp(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	window->endMouseDelta();
}

void onCameraDollyScroll(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	application.dollyCamera(window->input.mouse_wheel_delta * application.camera_dolly_sensitivity);
}

//void onCameraResetKeyDown(nui::KeyDownEvent& event)
//{
//	application.setCameraPosition(0, 0, 10);
//	application.setCameraRotation(0, 0, 0);
//}

void createTestScene_01()
{
	application.shading_normal = ShadingNormal::TESSELATION;

	std::array<MeshDrawcall*, 10> drawcalls;
	for (MeshDrawcall*& drawcall : drawcalls) {
		drawcall = application.createDrawcall();
	}

	drawcalls[0]->display_mode = DisplayMode::SOLID;
	drawcalls[1]->display_mode = DisplayMode::SOLID_WITH_WIREFRAME_FRONT;
	drawcalls[2]->display_mode = DisplayMode::SOLID_WITH_WIREFRAME_NONE;
	drawcalls[3]->display_mode = DisplayMode::WIREFRANE;
	drawcalls[4]->display_mode = DisplayMode::WIREFRANE_PURE;

	drawcalls[5]->display_mode = DisplayMode::SOLID;
	drawcalls[5]->is_back_culled = true;

	drawcalls[6]->display_mode = DisplayMode::SOLID_WITH_WIREFRAME_FRONT;
	drawcalls[6]->is_back_culled = true;

	drawcalls[7]->display_mode = DisplayMode::SOLID_WITH_WIREFRAME_NONE;
	drawcalls[7]->is_back_culled = true;

	drawcalls[8]->display_mode = DisplayMode::WIREFRANE;
	drawcalls[8]->is_back_culled = true;

	drawcalls[9]->display_mode = DisplayMode::WIREFRANE_PURE;
	drawcalls[9]->is_back_culled = true;

	std::array<MeshInstance*, 5> primitives;

	float gap_space = 120;
	float primitive_size = 50;

	// Triangle
	{
		CreateTriangleInfo info;
		info.size = primitive_size;
		info.transform.pos.x = 0;

		primitives[0] = application.createTriangle(info, nullptr, drawcalls[0]);
	}
	
	// Quad
	{
		CreateQuadInfo info;
		info.size = primitive_size;
		info.transform.pos.x = gap_space;

		primitives[1] = application.createQuad(info, nullptr, drawcalls[1]);
	}

	// Cube
	{
		CreateCubeInfo info;
		info.size = primitive_size;
		info.transform.pos.x = gap_space * 2;

		primitives[2] = application.createCube(info, nullptr, drawcalls[2]);
	}

	// Cylinder
	{
		CreateCylinderInfo info;
		info.transform.pos.x = gap_space * 3;
		info.diameter = primitive_size;
		info.height = primitive_size;
		info.rows = 16;
		info.columns = 16;

		primitives[3] = application.createCylinder(info, nullptr, drawcalls[3]);
	}

	// Sphere
	{
		CreateUV_SphereInfo info;
		info.transform.pos.x = gap_space * 4;
		info.diameter = primitive_size;
		info.rows = 16;
		info.columns = 16;
		
		primitives[4] = application.createUV_Sphere(info, nullptr, drawcalls[4]);
	}

	auto move_meshes = [](std::vector<MeshInstance*>& instances, glm::vec3 pos) {
		for (MeshInstance* inst : instances) {
			inst->pos = pos;
		}
	};

	float meshes_y = -250;

	// Loaded Mesh
	std::vector<MeshInstance*> meshes;
	{
		io::FilePath path;
		path.recreateRelative("Sculpt/Meshes/Journey/scene.gltf");

		GLTF_ImporterSettings settings;
		settings.dest_drawcall = drawcalls[0];

		ErrStack err_stack = application.importMeshesFromGLTF_File(path, settings, &meshes);
		if (err_stack.isBad()) {
			err_stack.debugPrint();
			throw std::exception();
		}

		move_meshes(meshes, { 0, meshes_y, 0 });
	}

	for (uint32_t i = 1; i < 5; i++) {

		glm::vec3 new_pos = { gap_space * i, meshes_y, 0 };

		for (MeshInstance* inst : meshes) {

			MeshInstance* copy = application.copyInstance(inst);
			copy->pos = new_pos;

			application.transferInstanceToDrawcall(copy, drawcalls[i]);
		}
	}

	meshes_y -= 100;

	// Duplicates
	{
		for (uint32_t i = 0; i < 5; i++) {

			glm::vec3 new_pos = { gap_space * i, meshes_y, 0 };

			MeshInstance* copy = application.copyInstance(primitives[1]);
			copy->pos = new_pos;

			application.transferInstanceToDrawcall(copy, drawcalls[5 + i]);
		}
	}

	meshes_y -= 250;

	for (uint32_t i = 0; i < 5; i++) {

		glm::vec3 new_pos = { gap_space * i, meshes_y, 0 };

		for (MeshInstance* inst : meshes) {

			MeshInstance* copy = application.copyInstance(inst);
			copy->pos = new_pos;

			application.transferInstanceToDrawcall(copy, drawcalls[5 + i]);
		}
	}

	// Camera positions
	glm::vec2 center = { gap_space * (primitives.size() / 2), 0 };

	application.setCameraPosition(center.x, center.y, 2000);
	application.setCameraFocus(glm::vec3(center.x, center.y, 0));
}

void createTestScene_Triangle()
{
	application.shading_normal = ShadingNormal::TESSELATION;

	MeshDrawcall* drawcall = application.createDrawcall();
	drawcall->display_mode = DisplayMode::SOLID;

	// Triangle
	{
		CreateTriangleInfo info;
		info.size = 1;
		info.transform.pos.x = 0;

		application.createTriangle(info, nullptr, drawcall);
	}

	// Camera positions
	glm::vec2 center = { 0, 0 };

	application.setCameraPosition(center.x, center.y, 5);
	application.setCameraFocus(glm::vec3(center.x, center.y, 0));
}


int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	ErrStack err_stack;

	render_doc.init();

	// Application
	{
		// Instances
		application.instance_id = 1;

		// Drawcalls
		application.last_used_drawcall = application.createDrawcall();

		// Layers
		application.last_used_layer = &application.layers.emplace_back();
		application.last_used_layer->parent = nullptr;

		// Shading
		application.shading_normal = ShadingNormal::TESSELATION;

		// Lighting
		application.lights[0].normal = toNormal(45, 45);
		application.lights[0].color = { 1, 1, 1 };
		application.lights[0].intensity = 1.f;

		application.lights[1].normal = toNormal(-45, 45);
		application.lights[1].color = { 1, 1, 1 };
		application.lights[1].intensity = 1.f;

		application.lights[2].normal = toNormal(45, -45);
		application.lights[2].color = { 1, 1, 1 };
		application.lights[2].intensity = 1.f;

		application.lights[3].normal = toNormal(-45, -45);
		application.lights[3].color = { 1, 1, 1 };
		application.lights[3].intensity = 1.f;

		application.lights[4].intensity = 0.f;
		application.lights[5].intensity = 0.f;
		application.lights[6].intensity = 0.f;
		application.lights[7].intensity = 0.f;

		application.ambient_intensity = 0.03f;

		// Camera
		application.camera_focus = { 0, 0, 0 };
		application.camera_field_of_view = 15.f;
		application.camera_z_near = 0.1f;
		application.camera_z_far = 100'000.f;
		application.camera_pos = { 0, 0, 0 };
		application.camera_quat_inv = { 1, 0, 0, 0 };
		application.camera_forward = { 0, 0, -1 };
		application.camera_orbit_sensitivity = 0.01f;
		application.camera_pan_sensitivity = 0.0001f;
		application.camera_dolly_sensitivity = 0.001f;
	}

	// User Interface
	{
		nui::Instance& instance = application.ui_instance;
		instance.create();

		// Create Window
		nui::Window* window;
		{
			nui::WindowCreateInfo info;
			info.width = 1027;
			info.height = 720;

			application.main_window = instance.createWindow(info);
			window = application.main_window;
		}
		window->setKeyDownEvent(renderDocTriggerCapture, nui::VirtualKeys::F11);

		nui::Grid* window_grid = window->createGrid();
		window_grid->size[0] = 90.f;
		window_grid->size[1] = 90.f;
		window_grid->coloring = nui::BackgroundColoring::RENDERING_SURFACE;
		window_grid->setRenderingSurfaceEvent(geometryDraw);

		// Camera Rotation
		window_grid->setKeyDownEvent(onCameraOrbitKeyDown, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
		window_grid->setKeyHeldDownEvent(onCameraOrbitKeyHeld, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
		window_grid->setKeyUpEvent(onCameraOrbitKeyUp, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);

		// Camera Pan
		window_grid->setKeyDownEvent(onCameraPanKeyDown, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);
		window_grid->setKeyHeldDownEvent(onCameraPanKeyHeld, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);
		window_grid->setKeyUpEvent(onCameraPanKeyUp, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);

		// Camera Zoom
		window_grid->setMouseScrollEvent(onCameraDollyScroll);

		window_grid->setMouseMoveEvent(onMouseMove);
	}

	// Tests
	{
		createTestScene_01();
		//createTestScene_Triangle();
	}

	application.ui_instance.min_frame_duration_ms = 16;

	nui::WindowMessages& win_messages = application.main_window->win_messages;
	while (!win_messages.should_close) {
		application.ui_instance.update();
	}

	return 0;
}

int main(int argc, char** argv)
{
	return WinMain(nullptr, nullptr, "", 0);
}
