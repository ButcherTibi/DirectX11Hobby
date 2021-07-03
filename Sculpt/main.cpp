
#include "RenderDocIntegration.hpp"
#include "NuiLibrary.hpp"
#include "Application.hpp"


void renderDocBeginCapture(nui::Window*, nui::StoredElement*, void*)
{
	render_doc.startCapture();
}

void renderDocEndCapture(nui::Window*, nui::StoredElement*, void*)
{
	render_doc.endCapture();
}

void onCameraOrbitKeyDown(nui::Window* window, nui::StoredElement* source, void*)
{
	nui::Input& input = window->input;
	glm::vec3 pixel_world_pos;
	renderer.getPixelWorldPosition(input.mouse_x, input.mouse_y, pixel_world_pos);

	//printf("%.2f %.2f %.2f \n", pixel_world_pos.x, pixel_world_pos.y, pixel_world_pos.z);

	if (pixel_world_pos.x != FLT_MAX) {
		application.setCameraFocus(pixel_world_pos);
	}

	nui::Flex* grid = std::get_if<nui::Flex>(source);
	grid->beginMouseFixedDeltaEffect();
}

void onCameraOrbitKeyHeld(nui::Window* window, nui::StoredElement*, void*)
{
	int32_t delta_x = window->input.mouse_delta_x;
	int32_t delta_y = window->input.mouse_delta_y;

	float scaling = application.camera_orbit_sensitivity * application.ui_instance.delta_time;
	application.arcballOrbitCamera((float)delta_x * scaling, (float)delta_y * scaling);
}

void onCameraOrbitKeyUp(nui::Window* window, nui::StoredElement*, void*)
{
	// nui::Flex* grid = std::get_if<nui::Flex>(source);
	window->endMouseDeltaEffect();
}

void onCameraPanKeyDown(nui::Window*, nui::StoredElement* source, void*)
{
	auto grid = std::get_if<nui::Flex>(source);
	grid->beginMouseLoopDeltaEffect();
}

void onCameraPanKeyHeld(nui::Window* window, nui::StoredElement*, void*)
{
	int32_t delta_x = window->input.mouse_delta_x;
	int32_t delta_y = window->input.mouse_delta_y;

	float scaling = application.camera_pan_sensitivity * application.ui_instance.delta_time;
	application.panCamera((float)-delta_x * scaling, (float)-delta_y * scaling);
}

void onCameraPanKeyUp(nui::Window* window, nui::StoredElement*, void*)
{
	window->endMouseDeltaEffect();
}

void onCameraDollyScroll(nui::Window* window, nui::StoredElement*, void*)
{
	nui::Input& input = window->input;
	glm::vec3 pixel_world_pos;
	renderer.getPixelWorldPosition(input.mouse_x, input.mouse_y, pixel_world_pos);

	if (pixel_world_pos.x != FLT_MAX) {
		application.setCameraFocus(pixel_world_pos);
	}

	application.dollyCamera(window->input.mouse_wheel_delta * application.camera_dolly_sensitivity);
}

void setBackCullingTrue(nui::Window*, nui::StoredElement*, void*)
{
	application.getRootDrawcall().is_back_culled = true;
}

void setBackCullingFalse(nui::Window*, nui::StoredElement*, void*)
{
	application.getRootDrawcall().is_back_culled = false;
}

void setShadingNormalVertex(nui::Window*, nui::StoredElement*, void*)
{
	application.setShadingNormal(GPU_ShadingNormal::VERTEX);
}

void setShadingNormalPoly(nui::Window*, nui::StoredElement*, void*)
{
	application.setShadingNormal(GPU_ShadingNormal::POLY);
}

void setShadingNormalTesselation(nui::Window*, nui::StoredElement*, void*)
{
	application.setShadingNormal(GPU_ShadingNormal::TESSELATION);
}

void setDisplayModeSolid(nui::Window*, nui::StoredElement*, void*)
{
	application.getRootDrawcall().display_mode = DisplayMode::SOLID;
}

void setDisplayModeWireframeOverlay(nui::Window*, nui::StoredElement*, void*)
{
	application.getRootDrawcall().display_mode = DisplayMode::WIREFRAME_OVERLAY;
}

void setDisplayModeWireframe(nui::Window*, nui::StoredElement*, void*)
{
	application.getRootDrawcall().display_mode = DisplayMode::WIREFRANE;
}

void changeShadingNormal(nui::Window*, nui::StoredElement*, void*)
{
	application.shading_normal = (application.shading_normal + 1) % 3;
}

void renderAABBs_Normal(nui::Window*, nui::StoredElement*, void*)
{
	application.setAABB_RenderModeForDrawcall(
		&application.getRootDrawcall(),
		AABB_RenderMode::NORMAL
	);
}

void renderAABBs_LeafOnly(nui::Window*, nui::StoredElement*, void*)
{
	application.setAABB_RenderModeForDrawcall(
		&application.getRootDrawcall(),
		AABB_RenderMode::LEAF_ONLY
	);
}

void hideAABBs(nui::Window*, nui::StoredElement*, void*)
{
	application.setAABB_RenderModeForDrawcall(
		&application.getRootDrawcall(),
		AABB_RenderMode::NO_RENDER
	);
}

void createTestScene_EmptyScene(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();
}

void createTestScene_CopyInstance(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	CreateTriangleInfo info;
	MeshInstanceRef tris_ref = application.createTriangle(info);

	MeshInstanceRef copy_ref = application.copyInstance(tris_ref);
	MeshInstance* copy_inst = copy_ref.get();
	copy_inst->transform.pos.x = 2;

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_DeleteInstance(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	CreateTriangleInfo info;
	MeshInstanceRef tris_ref = application.createTriangle(info);

	MeshInstanceRef copy_ref = application.copyInstance(tris_ref);
	MeshInstance* copy_inst = copy_ref.get();
	copy_inst->transform.pos.x = 2;

	application.deleteInstance(copy_ref);

	application.deleteInstance(tris_ref);

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_joinMeshes(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	// Same Parent
	{
		CreateTriangleInfo info;
		MeshInstanceRef tris_ref = application.createTriangle(info);

		MeshInstanceRef copy_ref = application.copyInstance(tris_ref);
		MeshInstance* copy_inst = copy_ref.get();
		copy_inst->transform.pos.x = 2;

		std::vector<MeshInstanceRef> sources = {
			tris_ref, copy_ref
		};
		application.joinMeshes(sources, 0);
	}

	// Different Parent
	{
		float y = -6;

		CreateTriangleInfo tris_info;
		tris_info.transform.pos.y = y;
		MeshInstanceRef tris_ref = application.createTriangle(tris_info);

		CreateQuadInfo quad_info;
		quad_info.transform.pos.x = 2;
		quad_info.transform.pos.y = y;
		MeshInstanceRef quad_ref = application.createQuad(quad_info);

		std::vector<MeshInstanceRef> sources = {
			tris_ref, quad_ref
		};
		application.joinMeshes(sources, 0);
	}

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_joinMeshes_Complex(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	io::FilePath file_path;
	ErrStack err_stack = file_path.recreateRelative("Sculpt/Meshes/Journey/scene.gltf");
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return;
	}

	std::vector<MeshInstanceRef> new_refs;

	GLTF_ImporterSettings settings;
	err_stack = application.importMeshesFromGLTF_File(file_path, settings, &new_refs);
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return;
	}

	application.joinMeshes(new_refs, 0);

	// Camera
	glm::vec2 center = { 0, 100 };
	application.setCameraPosition(center.x, center.y, 1000);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

//void createTestScene_DeleteAllInstancesInDrawcall(nui::Window*, nui::StoredElement*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	CreateTriangleInfo info;
//	MeshInstanceRef tris_ref = application.createTriangle(info);
//
//	MeshInstanceRef copy_ref = application.copyInstance(tris_ref);
//	MeshInstance* copy_inst = copy_ref.get();
//	copy_inst->transform.pos.x = 2;
//
//	application.deleteInstance(copy_ref);
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setCameraPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}

void createTestScene_SingleTriangle(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	CreateTriangleInfo info;
	application.createTriangle(info);

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_Triangle(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	std::array<MeshDrawcall*, 3> drawcalls;
	for (auto& drawcall : drawcalls) {
		drawcall = application.createDrawcall();
	}

	drawcalls[0]->display_mode = DisplayMode::SOLID;
	drawcalls[1]->display_mode = DisplayMode::WIREFRAME_OVERLAY;
	drawcalls[2]->display_mode = DisplayMode::WIREFRANE;

	float pos_x = 0;
	float pos_y = -2;
	float step_x = 1.5;

	// Meshes
	uint32_t i = 0;
	for (auto& drawcall : drawcalls) {

		CreateTriangleInfo info;
		info.transform.pos.x = pos_x;
		MeshInstanceRef tris_ref = application.createTriangle(info, nullptr, drawcall);

		MeshInstance* instance = application.copyInstance(tris_ref).get();
		instance->transform.pos.x = pos_x;
		instance->transform.pos.y = pos_y;

		pos_x += step_x;
		i++;
	}

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_SingleQuad(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	CreateQuadInfo info;
	application.createQuad(info);

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_Quad(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	std::array<MeshDrawcall*, 3> drawcalls;
	for (auto& drawcall : drawcalls) {
		drawcall = application.createDrawcall();
	}

	drawcalls[0]->display_mode = DisplayMode::SOLID;
	drawcalls[1]->display_mode = DisplayMode::WIREFRAME_OVERLAY;
	drawcalls[2]->display_mode = DisplayMode::WIREFRANE;

	float pos_x = 0;
	float pos_y = -2;
	float step_x = 1.5;

	// Meshes
	for (auto& drawcall : drawcalls) {

		CreateQuadInfo info;
		info.transform.pos.x = pos_x;
		application.createQuad(info, nullptr, drawcall);

		pos_x += 1.5;
	}

	// Instances
	{
		CreateQuadInfo info;
		info.transform.pos.x = 0;
		info.transform.pos.y = pos_y;
		MeshInstanceRef ref = application.createQuad(info, nullptr, drawcalls[0]);

		pos_x = step_x;
		for (uint32_t i = 1; i < drawcalls.size(); i++) {

			MeshInstance* instance = application.copyInstance(ref).get();
			instance->transform.pos.x = pos_x;
			instance->transform.pos.y = pos_y;

			pos_x += step_x;
		}
	}

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_WavyGrid(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	CreateWavyGridInfo info;
	application.createWavyGrid(info);

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_Cube(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	std::array<MeshDrawcall*, 3> drawcalls;
	for (auto& drawcall : drawcalls) {
		drawcall = application.createDrawcall();
	}

	drawcalls[0]->display_mode = DisplayMode::SOLID;
	drawcalls[1]->display_mode = DisplayMode::WIREFRAME_OVERLAY;
	drawcalls[2]->display_mode = DisplayMode::WIREFRANE;

	float pos_x = 0;

	for (auto& drawcall : drawcalls) {

		CreateCubeInfo info;
		info.transform.pos.x = pos_x;
		application.createCube(info, nullptr, drawcall);

		pos_x += 2;
	}

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_Cylinder(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	CreateCylinderInfo info;
	info.rows = 32;
	info.columns = 64;
	application.createCylinder(info);

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_UV_Sphere(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	CreateUV_SphereInfo info;
	info.rows = 32;
	info.columns = 64;
	application.createUV_Sphere(info);

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_DeletePoly(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	application.shading_normal = GPU_ShadingNormal::POLY;

	// Triangle
	{
		CreateCubeInfo info;
		MeshInstanceRef cube_ref = application.createCube(info, nullptr, nullptr);
		
		scme::SculptMesh& mesh = cube_ref.get()->instance_set->parent_mesh->mesh;
		mesh.deletePoly(0);
	}

	// Camera positions
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createTestScene_ImportGLTF(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	io::FilePath file_path;
	ErrStack err_stack = file_path.recreateRelative("Sculpt/Meshes/Journey/scene.gltf");
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return;
	}

	GLTF_ImporterSettings settings;
	err_stack = application.importMeshesFromGLTF_File(file_path, settings);
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return;
	}

	// Camera
	glm::vec2 center = { 0, 100 };
	application.setCameraPosition(center.x, center.y, 1000);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createPerformanceTestScene_AABBs(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	io::FilePath file_path;
	ErrStack err_stack = file_path.recreateRelative("Sculpt/Meshes/Journey/scene.gltf");
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return;
	}

	std::vector<MeshInstanceRef> new_refs;

	GLTF_ImporterSettings settings;
	err_stack = application.importMeshesFromGLTF_File(file_path, settings, &new_refs);
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return;
	}

	application.joinMeshes(new_refs, 0);

	application.setAABB_RenderModeForDrawcall(
		&application.getRootDrawcall(),
		AABB_RenderMode::LEAF_ONLY
	);

	// Camera
	glm::vec2 center = { 0, 100 };
	application.setCameraPosition(center.x, center.y, 1000);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createPerformanceTestScene_ManySpheres(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	float spacing = 2;

	CreateUV_SphereInfo info;
	info.rows = 32;
	info.columns = 64;
	MeshInstanceRef source_ref = application.createUV_Sphere(info);
	MeshInstance* source = source_ref.get();
	source->transform.pos.y = -spacing;

	uint32_t amount = 20;

	for (uint32_t depth = 0; depth < amount; depth++) {
		for (uint32_t row = 0; row < amount; row++) {
			for (uint32_t col = 0; col < amount; col++) {

				MeshInstanceRef copy_ref = application.copyInstance(source_ref);
				MeshInstance* copy = copy_ref.get();
				copy->transform.pos.x = spacing * col;
				copy->transform.pos.y = spacing * row;
				copy->transform.pos.z = -spacing * depth;
			}
		}
	}

	// Camera
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createPerformanceTestScene_DenseSphere(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	CreateUV_SphereInfo info;
	info.rows = 300;
	info.columns = info.rows * 2;
	MeshInstanceRef source_ref = application.createUV_Sphere(info);

	// Camera
	glm::vec2 center = { 0, 0 };
	application.setCameraPosition(center.x, center.y, 10);

	glm::vec3 focus = { center.x, center.y, 0 };
	application.setCameraFocus(focus);
}

void createInputTestScene_TabletMapping(nui::Window*, nui::StoredElement*, void*)
{
	application.resetToHardcodedStartup();

	double ratio = 0.7;

	double screen_aspect_ratio = 16 / 9;
	
	// Y
	double tablet_size_y = 13500;
	double height = tablet_size_y * ratio;

	double top = (tablet_size_y - height) / 2;
	double bot = top + height;

	// X
	double tablet_size_x = 21600;
	double width = tablet_size_x * ratio * screen_aspect_ratio;

	double left = (tablet_size_x - width) / 2;
	double right = left + width;

	printf("top = %d \n", (uint32_t)(top));
	printf("bot = %d \n", (uint32_t)(bot));
	printf("left = %d \n", (uint32_t)(left));
	printf("right = %d \n", (uint32_t)(right));
}

//void createUITestScene_Flex()
//{
//	application.resetScene();
//
//	application.navigateUp();
//}

void exit_application(nui::Window*, nui::StoredElement*, void*)
{
	application.main_window->win_messages.should_close = true;
}


// Mode title

void enterInstanceMode(nui::Window*, nui::StoredElement*, void*)
{

}

void enterSculptMode(nui::Window*, nui::StoredElement*, void*)
{
	application.navigateToChild(InteractionModes::SCULPT);
}


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// Debug Runtime Configuration
#if 0
	// Memory Debug
	{
		int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
		tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

		// Set flag to the new value.
		_CrtSetDbgFlag(tmpFlag);
	}
#endif

	// GPU Debugging using Render Doc
#ifndef NDEBUG
	render_doc.init();
#endif

	// Interactions Init
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

		auto& inters = application.interactions;
		{
			auto& nothing = inters[InteractionModes::NOTHING];
			nothing.parent = InteractionModes::ENUM_SIZE;
			nothing.children = {
				InteractionModes::DEFAULT
			};

			nothing.enter = []() {
				// do nothing
			};

			nothing.exit = []() {
				// do nothing
			};
		}

		{
			auto& default = inters[InteractionModes::DEFAULT];
			default.parent = InteractionModes::NOTHING;
			default.children = {
				InteractionModes::INSTANCE_SELECTION,
				InteractionModes::MESH,
				InteractionModes::SCULPT
			};

			default.enter = []() {

				nui::Window* window = application.main_window;
				UserInterface& ui = application.ui;

				nui::Flex* window_grid = window->createGrid();
				window_grid->setSize(100.f, 100.f);
				window_grid->setOrientation(nui::Flex::Orientation::COLUMN);

				nui::Menu* top_menu = window->createMenu(window_grid);
				top_menu->setZ_Index(2);
				top_menu->setTitleBackColor({ 0.f, 0.05f, 0.05f, 1.f });
				top_menu->setSelectBackColor({ 1.f, 1.f, 1.f, 0.05f });
				{
					nui::MenuStyle title_style;
					title_style.font_size = 15;
					title_style.top_padding = 4;
					title_style.bot_padding = title_style.top_padding;
					title_style.left_padding = 5;
					title_style.right_padding = title_style.left_padding;
					title_style.menu_background_color.setRGBA_UNORM(0.f, 0.025f, 0.025f, 0.75f);

					nui::MenuStyle menus_style;
					menus_style.font_size = 15;
					menus_style.top_padding = 4;
					menus_style.bot_padding = menus_style.top_padding;
					menus_style.left_padding = 7;
					menus_style.right_padding = menus_style.left_padding;
					menus_style.menu_background_color.setRGBA_UNORM(0.f, 0.025f, 0.025f, 0.75f);

					nui::MenuItem* scene = top_menu->addTitle(title_style);
					scene->setItemText("Scene");
					{
						nui::MenuItem* create = scene->addItem(menus_style);
						create->text = "New";

						nui::MenuItem* new_test = scene->addItem(menus_style);
						new_test->text = "New Creation Test Scene";
						{
							nui::MenuItem* empty_test = new_test->addItem(menus_style);
							empty_test->text = "Empty Scene";
							empty_test->label_callback = createTestScene_EmptyScene;

							nui::MenuItem* triangle_test = new_test->addItem(menus_style);
							triangle_test->text = "Single Triangle";
							triangle_test->label_callback = createTestScene_SingleTriangle;

							nui::MenuItem* tris_drawcalls = new_test->addItem(menus_style);
							tris_drawcalls->text = "Triangle";
							tris_drawcalls->label_callback = createTestScene_Triangle;

							nui::MenuItem* quad_test = new_test->addItem(menus_style);
							quad_test->text = "Single Quad";
							quad_test->label_callback = createTestScene_SingleQuad;

							nui::MenuItem* quad_drawcalls = new_test->addItem(menus_style);
							quad_drawcalls->text = "Quad";
							quad_drawcalls->label_callback = createTestScene_Quad;

							nui::MenuItem* wavy_grid = new_test->addItem(menus_style);
							wavy_grid->text = "Wavy Grid";
							wavy_grid->label_callback = createTestScene_WavyGrid;

							nui::MenuItem* cube_test = new_test->addItem(menus_style);
							cube_test->text = "Cube";
							cube_test->label_callback = createTestScene_Cube;

							nui::MenuItem* cylinder_test = new_test->addItem(menus_style);
							cylinder_test->text = "Cylinder";
							cylinder_test->label_callback = createTestScene_Cylinder;

							nui::MenuItem* uv_sphere_test = new_test->addItem(menus_style);
							uv_sphere_test->text = "UV Sphere";
							uv_sphere_test->label_callback = createTestScene_UV_Sphere;

							nui::MenuItem* import_gltf = new_test->addItem(menus_style);
							import_gltf->text = "Import GLTF file";
							import_gltf->label_callback = createTestScene_ImportGLTF;
						}

						nui::MenuItem* new_mesh_edit_test = scene->addItem(menus_style);
						new_mesh_edit_test->text = "New Mesh edit test scene";
						{
							nui::MenuItem* delete_poly_test = new_mesh_edit_test->addItem(menus_style);
							delete_poly_test->text = "Delete poly";
							delete_poly_test->label_callback = createTestScene_DeletePoly;

							nui::MenuItem* join_meshes_test = new_mesh_edit_test->addItem(menus_style);
							join_meshes_test->text = "Join Meshes Simple";
							join_meshes_test->label_callback = createTestScene_joinMeshes;

							nui::MenuItem* join_meshes_2_test = new_mesh_edit_test->addItem(menus_style);
							join_meshes_2_test->text = "Join Meshes Complex";
							join_meshes_2_test->label_callback = createTestScene_joinMeshes_Complex;
						}

						nui::MenuItem* new_instance_edit_test = scene->addItem(menus_style);
						new_instance_edit_test->text = "New Instance edit test scene";
						{
							nui::MenuItem* copy_instance_test = new_instance_edit_test->addItem(menus_style);
							copy_instance_test->text = "Copy Instance";
							copy_instance_test->label_callback = createTestScene_CopyInstance;

							nui::MenuItem* delete_instance_test = new_instance_edit_test->addItem(menus_style);
							delete_instance_test->text = "Delete Instance";
							delete_instance_test->label_callback = createTestScene_DeleteInstance;
						}

						nui::MenuItem* new_performance_test = scene->addItem(menus_style);
						new_performance_test->text = "New Performance test scene";
						{
							nui::MenuItem* aabbs = new_performance_test->addItem(menus_style);
							aabbs->text = "Octree";
							aabbs->label_callback = createPerformanceTestScene_AABBs;

							nui::MenuItem* many_spheres = new_performance_test->addItem(menus_style);
							many_spheres->text = "Many Spheres";
							many_spheres->label_callback = createPerformanceTestScene_ManySpheres;

							nui::MenuItem* dense_sphere = new_performance_test->addItem(menus_style);
							dense_sphere->text = "Dense Sphere";
							dense_sphere->label_callback = createPerformanceTestScene_DenseSphere;
						}

						nui::MenuItem* new_input_test = scene->addItem(menus_style);
						new_input_test->text = "New Input test scene";
						{
							nui::MenuItem* tablet_mapping = new_input_test->addItem(menus_style);
							tablet_mapping->text = "Tablet Mapping";
							tablet_mapping->label_callback = createInputTestScene_TabletMapping;
						}

						nui::MenuItem* load = scene->addItem(menus_style);
						load->text = "Load";

						nui::MenuItem* save = scene->addItem(menus_style);
						save->text = "Save";

						nui::MenuItem* save_as = scene->addItem(menus_style);
						save_as->text = "Save As";

						nui::MenuItem* exit_app = scene->addItem(menus_style);
						exit_app->text = "Exit";
						exit_app->label_callback = exit_application;
					}

					// Mode
					nui::MenuItem* mode_menu = top_menu->addTitle(title_style);
					mode_menu->text = "Mode";
					{
						
					}

					// Layer
					nui::MenuItem* layer_menu = top_menu->addTitle(title_style);
					layer_menu->text = "Layer";

					// Display
					nui::MenuItem* display_menu = top_menu->addTitle(title_style);
					display_menu->text = "Display";
					{
					nui::MenuItem* back_culling = display_menu->addItem(menus_style);
					back_culling->text = "Back Culling";
					{
						nui::MenuItem* back = back_culling->addItem(menus_style);
						back->text = "Back";
						back->label_callback = setBackCullingTrue;

						nui::MenuItem* none = back_culling->addItem(menus_style);
						none->text = "None";
						none->label_callback = setBackCullingFalse;
					}

					nui::MenuItem* shading_normal = display_menu->addItem(menus_style);
					shading_normal->text = "Shading Normal";
					{
						nui::MenuItem* vertex = shading_normal->addItem(menus_style);
						vertex->text = "Vertex";
						vertex->label_callback = setShadingNormalVertex;

						nui::MenuItem* poly = shading_normal->addItem(menus_style);
						poly->text = "Poly";
						poly->label_callback = setShadingNormalPoly;

						nui::MenuItem* tesselation = shading_normal->addItem(menus_style);
						tesselation->text = "Tesselation";
						tesselation->label_callback = setShadingNormalTesselation;
					}

					nui::MenuItem* display_mode = display_menu->addItem(menus_style);
					display_mode->text = "Display Mode";
					{
						nui::MenuItem* solid_mode = display_mode->addItem(menus_style);
						solid_mode->text = "Solid";
						solid_mode->label_callback = setDisplayModeSolid;

						nui::MenuItem* wire_overlay_mode = display_mode->addItem(menus_style);
						wire_overlay_mode->text = "Wireframe Overlay";
						wire_overlay_mode->label_callback = setDisplayModeWireframeOverlay;

						nui::MenuItem* wire_mode = display_mode->addItem(menus_style);
						wire_mode->text = "Wireframe";
						wire_mode->label_callback = setDisplayModeWireframe;
					}

					nui::MenuItem* aabbs = display_menu->addItem(menus_style);
					aabbs->text = "Octree";
					{
						nui::MenuItem* normal = aabbs->addItem(menus_style);
						normal->text = "Hierarchy";
						normal->label_callback = renderAABBs_Normal;

						nui::MenuItem* leaf_only = aabbs->addItem(menus_style);
						leaf_only->text = "Leaf Only";
						leaf_only->label_callback = renderAABBs_LeafOnly;

						nui::MenuItem* hide = aabbs->addItem(menus_style);
						hide->text = "Hide";
						hide->label_callback = hideAABBs;
					}
					}
				}

				// Viewport Flex
				ui.viewport = window->createGrid(window_grid);
				ui.viewport->size[0] = 90.f;
				ui.viewport->size[1] = 90.f;
				ui.viewport->coloring = nui::BackgroundColoring::RENDERING_SURFACE;
				ui.viewport->setRenderingSurfaceEvent(geometryDraw);

				// Camera Rotation
				ui.viewport->setKeyDownEvent(onCameraOrbitKeyDown, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
				ui.viewport->setKeyHeldDownEvent(onCameraOrbitKeyHeld, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
				ui.viewport->setKeyUpEvent(onCameraOrbitKeyUp, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);

				// Camera Pan
				ui.viewport->setKeyDownEvent(onCameraPanKeyDown, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);
				ui.viewport->setKeyHeldDownEvent(onCameraPanKeyHeld, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);
				ui.viewport->setKeyUpEvent(onCameraPanKeyUp, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);

				// Camera Zoom
				ui.viewport->setMouseScrollEvent(onCameraDollyScroll);

				// Shading Normal
				ui.viewport->setKeyDownEvent(changeShadingNormal, nui::VirtualKeys::N);
			};

			default.exit = []() {
				application.main_window->deleteAllElements();
				application.navigateUp();
			};

			// set this interaction as default
			application.now_interaction = InteractionModes::DEFAULT;
			default.enter();
		}

		{
			auto& sculpt = inters[InteractionModes::SCULPT];
			sculpt.parent = InteractionModes::DEFAULT;
			sculpt.children = {
				InteractionModes::STANDARD_BRUSH
			};

			sculpt.enter = []() {
				application.enterSculptMode();
			};

			sculpt.exit = []() {
				application.navigateUp();
			};
		}
	}

	// Scene Init
	{
		application.resetScene();
	}

	nui::WindowMessages& win_messages = application.main_window->win_messages;
	while (!win_messages.should_close) {
		application.ui_instance.update();
	}

	return 0;
}

int main(int, char**)
{
	// Disabled compiler warnings:
	// - 4201 nameless struct/union = needed by GLM
	// - 4239 default reference parameter
	// - 4267 uint32_t size = vector.size()
	// - 4701 potentially uninitialized local variable used
	// - 4703 potentially uninitialized local pointer variable used

	return WinMain(nullptr, nullptr, nullptr, 0);
}
