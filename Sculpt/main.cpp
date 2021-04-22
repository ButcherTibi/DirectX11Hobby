
#include "RenderDocIntegration.hpp"
#include "NuiLibrary.hpp"
#include "Application.hpp"

#include "DeferredVector.hpp"


void renderDocTriggerCapture(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	render_doc.triggerCapture();
}

void onMouseMove(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	/*uint32_t isect_poly;
	glm::vec3 isect_point;
	if (application.mouseRaycastInstances(isect_poly, isect_point) != nullptr) {
		printf("point = %.3f %.3f %.3f \n", isect_point.x, isect_point.y, isect_point.z);
	}*/
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

void changeShadingNormal(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	application.shading_normal = (application.shading_normal + 1) % 3;

	switch (application.shading_normal)	{
	case ShadingNormal::VERTEX: {
		printf("Shading Normal = VERTEX \n");
		break;
	}
	case ShadingNormal::POLY: {
		printf("Shading Normal = POLY \n");
		break;
	}
	case ShadingNormal::TESSELATION: {
		printf("Shading Normal = TESSELATION \n");
		break;
	}
	}
}

//void onCameraResetKeyDown(nui::KeyDownEvent& event)
//{
//	application.setCameraPosition(0, 0, 10);
//	application.setCameraRotation(0, 0, 0);
//}
//
//void createTestScene_01()
//{
//	application.shading_normal = ShadingNormal::TESSELATION;
//
//	std::array<MeshDrawcall*, 10> drawcalls;
//	for (MeshDrawcall*& drawcall : drawcalls) {
//		drawcall = application.createDrawcall();
//	}
//
//	drawcalls[0]->display_mode = DisplayMode::SOLID;
//	drawcalls[1]->display_mode = DisplayMode::SOLID_WITH_WIREFRAME_FRONT;
//	drawcalls[2]->display_mode = DisplayMode::SOLID_WITH_WIREFRAME_NONE;
//	drawcalls[3]->display_mode = DisplayMode::WIREFRANE;
//	drawcalls[4]->display_mode = DisplayMode::WIREFRANE;
//
//	drawcalls[5]->display_mode = DisplayMode::SOLID;
//	drawcalls[5]->is_back_culled = true;
//
//	drawcalls[6]->display_mode = DisplayMode::SOLID_WITH_WIREFRAME_FRONT;
//	drawcalls[6]->is_back_culled = true;
//
//	drawcalls[7]->display_mode = DisplayMode::SOLID_WITH_WIREFRAME_NONE;
//	drawcalls[7]->is_back_culled = true;
//
//	drawcalls[8]->display_mode = DisplayMode::WIREFRANE;
//	drawcalls[8]->is_back_culled = true;
//
//	drawcalls[9]->display_mode = DisplayMode::WIREFRANE;
//	drawcalls[9]->is_back_culled = true;
//
//	std::array<MeshInstance*, 5> primitives;
//
//	float gap_space = 120;
//	float primitive_size = 50;
//
//	// Triangle
//	{
//		CreateTriangleInfo info;
//		info.size = primitive_size;
//		info.transform.pos.x = 0;
//
//		primitives[0] = application.createTriangle(info, nullptr, drawcalls[0]);
//	}
//	
//	// Quad
//	{
//		CreateQuadInfo info;
//		info.size = primitive_size;
//		info.transform.pos.x = gap_space;
//
//		primitives[1] = application.createQuad(info, nullptr, drawcalls[1]);
//	}
//
//	// Cube
//	{
//		CreateCubeInfo info;
//		info.size = primitive_size;
//		info.transform.pos.x = gap_space * 2;
//
//		primitives[2] = application.createCube(info, nullptr, drawcalls[2]);
//	}
//
//	// Cylinder
//	{
//		CreateCylinderInfo info;
//		info.transform.pos.x = gap_space * 3;
//		info.diameter = primitive_size;
//		info.height = primitive_size;
//		info.rows = 16;
//		info.columns = 16;
//
//		primitives[3] = application.createCylinder(info, nullptr, drawcalls[3]);
//	}
//
//	// Sphere
//	{
//		CreateUV_SphereInfo info;
//		info.transform.pos.x = gap_space * 4;
//		info.diameter = primitive_size;
//		info.rows = 16;
//		info.columns = 16;
//		
//		primitives[4] = application.createUV_Sphere(info, nullptr, drawcalls[4]);
//	}
//
//	auto move_meshes = [](std::vector<MeshInstance*>& instances, glm::vec3 pos) {
//		for (MeshInstance* inst : instances) {
//			inst->pos = pos;
//		}
//	};
//
//	float meshes_y = -250;
//
//	// Loaded Mesh
//	std::vector<MeshInstance*> meshes;
//	{
//		io::FilePath path;
//		path.recreateRelative("Sculpt/Meshes/Journey/scene.gltf");
//
//		GLTF_ImporterSettings settings;
//		settings.dest_drawcall = drawcalls[0];
//
//		ErrStack err_stack = application.importMeshesFromGLTF_File(path, settings, &meshes);
//		if (err_stack.isBad()) {
//			err_stack.debugPrint();
//			throw std::exception();
//		}
//
//		move_meshes(meshes, { 0, meshes_y, 0 });
//	}
//
//	for (uint32_t i = 1; i < 5; i++) {
//
//		glm::vec3 new_pos = { gap_space * i, meshes_y, 0 };
//
//		for (MeshInstance* inst : meshes) {
//
//			MeshInstance* derive = application.copyInstance(inst);
//			derive->pos = new_pos;
//
//			application.transferInstanceToDrawcall(derive, drawcalls[i]);
//		}
//	}
//
//	meshes_y -= 100;
//
//	// Duplicates
//	{
//		for (uint32_t i = 0; i < 5; i++) {
//
//			glm::vec3 new_pos = { gap_space * i, meshes_y, 0 };
//
//			MeshInstance* derive = application.copyInstance(primitives[1]);
//			derive->pos = new_pos;
//
//			application.transferInstanceToDrawcall(derive, drawcalls[5 + i]);
//		}
//	}
//
//	meshes_y -= 250;
//
//	for (uint32_t i = 0; i < 5; i++) {
//
//		glm::vec3 new_pos = { gap_space * i, meshes_y, 0 };
//
//		for (MeshInstance* inst : meshes) {
//
//			MeshInstance* derive = application.copyInstance(inst);
//			derive->pos = new_pos;
//
//			application.transferInstanceToDrawcall(derive, drawcalls[5 + i]);
//		}
//	}
//
//	// Camera positions
//	glm::vec2 center = { gap_space * (primitives.size() / 2), 0 };
//
//	application.setCameraPosition(center.x, center.y, 2000);
//	application.setCameraFocus(glm::vec3(center.x, center.y, 0));
//}

void createTestScene_Triangle(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	printf("Test Scene: Create Triangle \n");

	application.resetToHardcodedStartup();

	application.shading_normal = ShadingNormal::POLY;

	MeshDrawcall* drawcall = application.createDrawcall();
	drawcall->display_mode = DisplayMode::SOLID;

	// Triangle
	{
		CreateTriangleInfo info;
		application.createTriangle(info, nullptr, drawcall);
	}

	// Camera positions
	glm::vec2 center = { 0, 0 };

	application.setCameraPosition(center.x, center.y, 10);
	application.setCameraFocus(glm::vec3(center.x, center.y, 0));
}

void createTestScene_Quad(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	printf("Test Scene: Create Quad \n");

	application.resetToHardcodedStartup();

	application.shading_normal = ShadingNormal::POLY;

	MeshDrawcall* drawcall = application.createDrawcall();
	drawcall->display_mode = DisplayMode::SOLID;

	// Triangle
	{
		CreateQuadInfo info;
		application.createQuad(info, nullptr, drawcall);
	}

	// Camera positions
	glm::vec2 center = { 0, 0 };

	application.setCameraPosition(center.x, center.y, 10);
	application.setCameraFocus(glm::vec3(center.x, center.y, 0));
}

void createTestScene_Cube(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	printf("Test Scene: Create Cube \n");

	application.resetToHardcodedStartup();

	application.shading_normal = ShadingNormal::POLY;

	MeshDrawcall* drawcall = application.createDrawcall();
	drawcall->display_mode = DisplayMode::SOLID;
	drawcall->is_back_culled = true;

	// Triangle
	{
		CreateCubeInfo info;
		application.createCube(info, nullptr, drawcall);
	}

	// Camera positions
	glm::vec2 center = { 0, 0 };

	application.setCameraPosition(center.x, center.y, 10);
	application.setCameraFocus(glm::vec3(center.x, center.y, 0));
}

void createTestScene_DeleteVertex(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	printf("Test Scene: Delete Vertex \n");

	application.resetToHardcodedStartup();

	application.shading_normal = ShadingNormal::POLY;

	MeshDrawcall* drawcall = application.createDrawcall();
	drawcall->display_mode = DisplayMode::SOLID;

	// Triangle
	{
		CreateQuadInfo info;
		MeshInstance* inst = application.createQuad(info, nullptr, drawcall);
		
		scme::SculptMesh& mesh = inst->instance_set->parent_mesh->mesh;
	}

	// Camera positions
	glm::vec2 center = { 0, 0 };

	application.setCameraPosition(center.x, center.y, 10);
	application.setCameraFocus(glm::vec3(center.x, center.y, 0));
}

//
//void createTestScene_Cube()
//{
//	application.shading_normal = ShadingNormal::TESSELATION;
//
//	MeshDrawcall* drawcall = application.createDrawcall();
//	drawcall->display_mode = DisplayMode::SOLID_WITH_WIREFRAME_NONE;
//
//	// Triangle
//	{
//		CreateCubeInfo info;
//		info.size = 1;
//		info.transform.pos.x = 0;
//
//		application.createCube(info, nullptr, drawcall);
//	}
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//
//	application.setCameraPosition(center.x, center.y, 10);
//	application.setCameraFocus(glm::vec3(center.x, center.y, 0));
//}
//
//void createDebugLine()
//{
//	MeshDrawcall* drawcall = application.createDrawcall();
//	drawcall->name = "Debug Line Drawcall";
//	drawcall->display_mode = DisplayMode::WIREFRAME_PURE;
//
//	CreateLineInfo info;
//	info.origin = { 0, 0, 0 };
//	info.direction = { 0, 0, 1 };
//	info.length = 1000.f;
//
//	MeshInstance* inst = application.createLine(info, nullptr, drawcall);
//	inst->name = "Debug Line";
//	inst->wireframe_colors.front_color = { 1, 0, 1 };
//}


int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	ErrStack err_stack;

	render_doc.init();

	// Application
	application.resetToHardcodedStartup();

	//createTestScene_Triangle();
	createTestScene_Cube(nullptr, nullptr, nullptr);

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

		// Shading Normal
		window_grid->setKeyDownEvent(changeShadingNormal, nui::VirtualKeys::N);


		// Test Scenes
		window_grid->setKeyDownEvent(createTestScene_Triangle, nui::VirtualKeys::F1);
		window_grid->setKeyDownEvent(createTestScene_Quad, nui::VirtualKeys::F2);
		window_grid->setKeyDownEvent(createTestScene_Cube, nui::VirtualKeys::F3);
	}

	// Alternative Vector
	{

		/*DeferredVector<GPU_MeshVertex> sparse_vector;
		{
			GPU_MeshVertex& ref = sparse_vector.emplace();
			ref.poly_id = 0xBEEF'CAFE;
		}
		
		for (auto iter = sparse_vector.begin(); iter != sparse_vector.end(); iter.next()) {
			printf("[%d] = %X \n", iter.index(), iter.get().poly_id);
		}*/
		/*
		sparse_vector.erase(0);
		sparse_vector.erase(1);
		sparse_vector.erase(3);
		sparse_vector.erase(4);

		printf("\n");
		for (auto iter = sparse_vector.begin(); iter != sparse_vector.end(); iter.next()) {
			printf("[%d] = %d \n", iter.index(), iter.get());
		}

		auto& added = sparse_vector.emplace();
		added = 5;

		printf("\n");
		for (auto iter = sparse_vector.begin(); iter != sparse_vector.end(); iter.next()) {
			printf("[%d] = %d \n", iter.index(), iter.get());
		}*/
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
