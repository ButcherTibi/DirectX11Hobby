
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

void createTestScene_DeletePoly(nui::Window* window, nui::StoredElement* source, void* user_data)
{
	printf("Test Scene: Delete Poly \n");

	application.resetToHardcodedStartup();

	application.shading_normal = ShadingNormal::POLY;

	MeshDrawcall* drawcall = application.createDrawcall();
	drawcall->display_mode = DisplayMode::SOLID;

	// Triangle
	{
		CreateCubeInfo info;
		MeshInstance* inst = application.createCube(info, nullptr, drawcall);
		
		scme::SculptMesh& mesh = inst->instance_set->parent_mesh->mesh;
		mesh.deletePoly(0);
	}

	// Camera positions
	glm::vec2 center = { 0, 0 };

	application.setCameraPosition(center.x, center.y, 10);
	application.setCameraFocus(glm::vec3(center.x, center.y, 0));
}

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
	createTestScene_Triangle(nullptr, nullptr, nullptr);

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
		window_grid->size[0] = 100.f;
		window_grid->size[1] = 100.f;
		window_grid->orientation = nui::GridOrientation::COLUMN;

		nui::Menu* top_menu = window->createMenu(window_grid);
		top_menu->z_index = 1;
		top_menu->titles_background_color.setRGBA_UNORM(0.f, 0.05f, 0.05f, 1.f);
		top_menu->select_background_color.setRGBA_UNORM(1.f, 1.f, 1.f, 0.05f);
		{
			uint32_t side_padding = 5;

			nui::MenuItem* scene = top_menu->addItem(nullptr);
			scene->text = "Scene";

			// Scene
			nui::MenuItem* create = top_menu->addItem(scene);
			create->text = "New";

			// New Test Scene
			nui::MenuItem* new_test = top_menu->addItem(scene);
			new_test->text = "New Test Scene";

			nui::MenuItem* triangle_test = top_menu->addItem(new_test);
			nui::MenuItem* quad_test = top_menu->addItem(new_test);
			nui::MenuItem* cube_test = top_menu->addItem(new_test);
			nui::MenuItem* cube_delete_poly_test = top_menu->addItem(new_test);
			{
				triangle_test->text = "Triangle";
				quad_test->text = "Quad";
				cube_test->text = "Cube";
				cube_delete_poly_test->text = "Cube delete poly";
			}

			nui::MenuItem* load = top_menu->addItem(scene);
			load->text = "Load";

			nui::MenuItem* save = top_menu->addItem(scene);
			save->text = "Save";

			nui::MenuItem* save_as = top_menu->addItem(scene);
			save_as->text = "Save as";

			nui::MenuItem* instance_menu = top_menu->addItem(nullptr);
			instance_menu->text = "Instance";

			// Instance
			nui::MenuItem* create_inst = top_menu->addItem(instance_menu);
			create_inst->text = "Create";

			nui::MenuItem* mesh_menu = top_menu->addItem(nullptr);
			mesh_menu->text = "Mesh";

			nui::MenuItem* layer_menu = top_menu->addItem(nullptr);
			layer_menu->text = "Layer";

			// Common Styles
			std::array<nui::MenuItem*, 4> titles = {
				scene,
				instance_menu,
				mesh_menu,
				layer_menu
			};

			for (auto& title : titles) {
				title->font_size = 17;
				title->top_padding = 3;
				title->bot_padding = 3;

				title->left_padding = side_padding;
				title->right_padding = side_padding;

				title->menu_background_color.setRGBA_UNORM(0.f, 0.025f, 0.025f, 0.75f);
			}

			std::array<nui::MenuItem*, 10> items = {
				create,
				new_test,
				load,
				save,
				save_as,
				create_inst,
				triangle_test,
				quad_test,
				cube_test,
				cube_delete_poly_test
			};

			for (auto& item : items) {
				item->font_size = 17;
				item->top_padding = 3;
				item->bot_padding = 3;
				item->left_padding = side_padding;
				item->right_padding = side_padding;
				item->menu_background_color.setRGBA_UNORM(0.f, 0.025f, 0.025f, 0.75f);
			}
		}

		// Viewport Grid
		nui::Grid* viewport = window->createGrid(window_grid);
		viewport->size[0] = 90.f;
		viewport->size[1] = 90.f;
		viewport->coloring = nui::BackgroundColoring::RENDERING_SURFACE;
		viewport->setRenderingSurfaceEvent(geometryDraw);

		// Camera Rotation
		viewport->setKeyDownEvent(onCameraOrbitKeyDown, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
		viewport->setKeyHeldDownEvent(onCameraOrbitKeyHeld, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
		viewport->setKeyUpEvent(onCameraOrbitKeyUp, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);

		// Camera Pan
		viewport->setKeyDownEvent(onCameraPanKeyDown, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);
		viewport->setKeyHeldDownEvent(onCameraPanKeyHeld, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);
		viewport->setKeyUpEvent(onCameraPanKeyUp, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);

		// Camera Zoom
		viewport->setMouseScrollEvent(onCameraDollyScroll);

		// Shading Normal
		viewport->setKeyDownEvent(changeShadingNormal, nui::VirtualKeys::N);


		// Test Scenes
		viewport->setKeyDownEvent(createTestScene_Triangle, nui::VirtualKeys::F1);
		viewport->setKeyDownEvent(createTestScene_Quad, nui::VirtualKeys::F2);
		viewport->setKeyDownEvent(createTestScene_Cube, nui::VirtualKeys::F3);
		viewport->setKeyDownEvent(createTestScene_DeletePoly, nui::VirtualKeys::F4);
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
