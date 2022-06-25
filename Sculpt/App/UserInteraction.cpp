
#include "Input.hpp"

// Header
#include "Application.hpp"


//void createTestScene_EmptyScene(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//}
//
//void createTestScene_SingleTriangle(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	CreateTriangleInfo info;
//	application.createTriangle(info);
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_Triangle(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	std::array<MeshDrawcall*, 3> drawcalls;
//	for (auto& drawcall : drawcalls) {
//		drawcall = application.createDrawcall();
//	}
//
//	drawcalls[0]->display_mode = DisplayMode::SOLID;
//	drawcalls[1]->display_mode = DisplayMode::WIREFRAME_OVERLAY;
//	drawcalls[2]->display_mode = DisplayMode::WIREFRANE;
//
//	float pos_x = 0;
//	float pos_y = -2;
//	float step_x = 1.5;
//
//	// Meshes
//	uint32_t i = 0;
//	for (auto& drawcall : drawcalls) {
//
//		CreateTriangleInfo info;
//		info.transform.pos.x = pos_x;
//		MeshInstanceRef tris_ref = application.createTriangle(info, nullptr, drawcall);
//
//		MeshInstance* instance = application.copyInstance(tris_ref).get();
//		instance->transform.pos.x = pos_x;
//		instance->transform.pos.y = pos_y;
//
//		pos_x += step_x;
//		i++;
//	}
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_SingleQuad(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	CreateQuadInfo info;
//	application.createQuad(info);
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_Quad(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	std::array<MeshDrawcall*, 3> drawcalls;
//	for (auto& drawcall : drawcalls) {
//		drawcall = application.createDrawcall();
//	}
//
//	drawcalls[0]->display_mode = DisplayMode::SOLID;
//	drawcalls[1]->display_mode = DisplayMode::WIREFRAME_OVERLAY;
//	drawcalls[2]->display_mode = DisplayMode::WIREFRANE;
//
//	float pos_x = 0;
//	float pos_y = -2;
//	float step_x = 1.5;
//
//	// Meshes
//	for (auto& drawcall : drawcalls) {
//
//		CreateQuadInfo info;
//		info.transform.pos.x = pos_x;
//		application.createQuad(info, nullptr, drawcall);
//
//		pos_x += 1.5;
//	}
//
//	// Instances
//	{
//		CreateQuadInfo info;
//		info.transform.pos.x = 0;
//		info.transform.pos.y = pos_y;
//		MeshInstanceRef ref = application.createQuad(info, nullptr, drawcalls[0]);
//
//		pos_x = step_x;
//		for (uint32_t i = 1; i < drawcalls.size(); i++) {
//
//			MeshInstance* instance = application.copyInstance(ref).get();
//			instance->transform.pos.x = pos_x;
//			instance->transform.pos.y = pos_y;
//
//			pos_x += step_x;
//		}
//	}
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_WavyGrid(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	CreateWavyGridInfo info;
//	application.createWavyGrid(info);
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_Cube(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	std::array<MeshDrawcall*, 3> drawcalls;
//	for (auto& drawcall : drawcalls) {
//		drawcall = application.createDrawcall();
//	}
//
//	drawcalls[0]->display_mode = DisplayMode::SOLID;
//	drawcalls[1]->display_mode = DisplayMode::WIREFRAME_OVERLAY;
//	drawcalls[2]->display_mode = DisplayMode::WIREFRANE;
//
//	float pos_x = 0;
//
//	for (auto& drawcall : drawcalls) {
//
//		CreateCubeInfo info;
//		info.transform.pos.x = pos_x;
//		application.createCube(info, nullptr, drawcall);
//
//		pos_x += 2;
//	}
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_Cylinder(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	CreateCylinderInfo info;
//	info.rows = 32;
//	info.columns = 64;
//	application.createCylinder(info);
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_UV_Sphere(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	CreateUV_SphereInfo info;
//	info.rows = 32;
//	info.columns = 64;
//	application.createUV_Sphere(info);
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_ImportGLTF(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	io::Path file_path;
//	ErrStack err_stack = file_path.recreateFromRelativePath("Sculpt/Meshes/Journey/scene.gltf");
//	if (err_stack.isBad()) {
//		err_stack.debugPrint();
//		return;
//	}
//
//	GLTF_ImporterSettings settings;
//	err_stack = application.importMeshesFromGLTF_File(file_path, settings);
//	if (err_stack.isBad()) {
//		err_stack.debugPrint();
//		return;
//	}
//
//	// Camera
//	glm::vec2 center = { 0, 100 };
//	application.setPosition(center.x, center.y, 1000);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_DeletePoly(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	application.shading_normal = GPU_ShadingNormal::POLY;
//
//	// Triangle
//	{
//		CreateCubeInfo info;
//		MeshInstanceRef cube_ref = application.createCube(info, nullptr, nullptr);
//
//		scme::SculptMesh& mesh = cube_ref.get()->instance_set->parent_mesh->mesh;
//		mesh.deletePoly(0);
//	}
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_joinMeshes(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	// Same Parent
//	{
//		CreateTriangleInfo info;
//		MeshInstanceRef tris_ref = application.createTriangle(info);
//
//		MeshInstanceRef copy_ref = application.copyInstance(tris_ref);
//		MeshInstance* copy_inst = copy_ref.get();
//		copy_inst->transform.pos.x = 2;
//
//		std::vector<MeshInstanceRef> sources = {
//			tris_ref, copy_ref
//		};
//		application.joinMeshes(sources, 0);
//	}
//
//	// Different Parent
//	{
//		float y = -6;
//
//		CreateTriangleInfo tris_info;
//		tris_info.transform.pos.y = y;
//		MeshInstanceRef tris_ref = application.createTriangle(tris_info);
//
//		CreateQuadInfo quad_info;
//		quad_info.transform.pos.x = 2;
//		quad_info.transform.pos.y = y;
//		MeshInstanceRef quad_ref = application.createQuad(quad_info);
//
//		std::vector<MeshInstanceRef> sources = {
//			tris_ref, quad_ref
//		};
//		application.joinMeshes(sources, 0);
//	}
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_joinMeshes_Complex(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	io::Path file_path;
//	ErrStack err_stack = file_path.recreateFromRelativePath("Sculpt/Meshes/Journey/scene.gltf");
//	if (err_stack.isBad()) {
//		err_stack.debugPrint();
//		return;
//	}
//
//	std::vector<MeshInstanceRef> new_refs;
//
//	GLTF_ImporterSettings settings;
//	err_stack = application.importMeshesFromGLTF_File(file_path, settings, &new_refs);
//	if (err_stack.isBad()) {
//		err_stack.debugPrint();
//		return;
//	}
//
//	application.joinMeshes(new_refs, 0);
//
//	// Camera
//	glm::vec2 center = { 0, 100 };
//	application.setPosition(center.x, center.y, 1000);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//
//void createTestScene_CopyInstance(nui::Window*, nui::StoredElement2*, void*)
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
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createTestScene_DeleteInstance(nui::Window*, nui::StoredElement2*, void*)
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
//	application.deleteInstance(tris_ref);
//
//	// Camera positions
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//
//void createPerformanceTestScene_AABBs(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	io::Path file_path;
//	ErrStack err_stack = file_path.recreateFromRelativePath("Sculpt/Meshes/Journey/scene.gltf");
//	if (err_stack.isBad()) {
//		err_stack.debugPrint();
//		return;
//	}
//
//	std::vector<MeshInstanceRef> new_refs;
//
//	GLTF_ImporterSettings settings;
//	err_stack = application.importMeshesFromGLTF_File(file_path, settings, &new_refs);
//	if (err_stack.isBad()) {
//		err_stack.debugPrint();
//		return;
//	}
//
//	application.joinMeshes(new_refs, 0);
//
//	application.setAABB_RenderModeForDrawcall(
//		&application.getRootDrawcall(),
//		AABB_RenderMode::LEAF_ONLY
//	);
//
//	// Camera
//	glm::vec2 center = { 0, 100 };
//	application.setPosition(center.x, center.y, 1000);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createPerformanceTestScene_ManySpheres(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	float spacing = 2;
//
//	CreateUV_SphereInfo info;
//	info.rows = 32;
//	info.columns = 64;
//	MeshInstanceRef source_ref = application.createUV_Sphere(info);
//	MeshInstance* source = source_ref.get();
//	source->transform.pos.y = -spacing;
//
//	uint32_t amount = 20;
//
//	for (uint32_t depth = 0; depth < amount; depth++) {
//		for (uint32_t row = 0; row < amount; row++) {
//			for (uint32_t col = 0; col < amount; col++) {
//
//				MeshInstanceRef copy_ref = application.copyInstance(source_ref);
//				MeshInstance* copy = copy_ref.get();
//				copy->transform.pos.x = spacing * col;
//				copy->transform.pos.y = spacing * row;
//				copy->transform.pos.z = -spacing * depth;
//			}
//		}
//	}
//
//	// Camera
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createPerformanceTestScene_DenseSphere(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	CreateUV_SphereInfo info;
//	info.rows = 400;
//	info.columns = info.rows * 2;
//	MeshInstanceRef source_ref = application.createUV_Sphere(info);
//
//	// Camera
//	glm::vec2 center = { 0, 0 };
//	application.setPosition(center.x, center.y, 10);
//
//	glm::vec3 focus = { center.x, center.y, 0 };
//	application.setCameraFocus(focus);
//}
//
//void createInputTestScene_TabletMapping(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.resetToHardcodedStartup();
//
//	double ratio = 0.7;
//
//	double screen_aspect_ratio = 16 / 9;
//
//	// Y
//	double tablet_size_y = 13500;
//	double height = tablet_size_y * ratio;
//
//	double top = (tablet_size_y - height) / 2;
//	double bot = top + height;
//
//	// X
//	double tablet_size_x = 21600;
//	double width = tablet_size_x * ratio * screen_aspect_ratio;
//
//	double left = (tablet_size_x - width) / 2;
//	double right = left + width;
//
//	printf("top = %d \n", (uint32_t)(top));
//	printf("bot = %d \n", (uint32_t)(bot));
//	printf("left = %d \n", (uint32_t)(left));
//	printf("right = %d \n", (uint32_t)(right));
//}
//
//
//// From here on are real usages  /////////////////////////////////////////////////////////////////////////////
//
//void exit_application(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.main_window->win_messages.should_close = true;
//}
//
//void beginStandardBrushStroke(nui::Window*, nui::StoredElement2*, void*)
//{
//	Application& app = application;
//	SculptContext& sculpt = app.sculpt;
//
//	if (sculpt.stroke_started == true) {
//		return;
//	}
//
//	sculpt.stroke_started = true;
//
//	StandardBrushSettings& standard = sculpt.standard_brush;
//	standard.sample_count.calcStrokeValue(sculpt.global_sample_count);
//	standard.radius.calcStrokeValue(sculpt.global_brush_radius);
//	standard.strength.calcStrokeValue(sculpt.global_brush_strength);
//	standard.falloff.calcStrokeValue(sculpt.global_brush_falloff);
//
//	// Debug
//	//standard.strength.speed_influence.enable = true;
//	standard.strength.speed_influence.min_speed = 0;
//	standard.strength.speed_influence.max_speed = FLT_MAX;
//	standard.strength.speed_influence.min_factor = 0;
//	standard.strength.speed_influence.max_factor = FLT_MAX;
//
//	standard.raw_steps.clear();
//	standard.applied_steps.clear();
//}
//
//void applyStandardBrush(nui::Window*, nui::StoredElement2*, void*)
//{
//	Application& app = application;
//	SculptContext& sculpt = app.sculpt;
//	StandardBrushSettings& standard = sculpt.standard_brush;
//
//	if (sculpt.stroke_started == false) {
//		return;
//	}
//
//	MeshInstance* instance = sculpt.target.get();
//
//	// ray origin
//	glm::vec3 local_ray_origin = app.camera_pos;
//	instance->localizePosition(local_ray_origin);
//
//	// ray targets
//	uint32_t sample_count = standard.sample_count._stroke_value;
//	uint32_t start_step = standard.applied_steps.size();
//
//	for (auto& mouse_history : app.main_window->input.mouse_pos_history) {
//
//		BrushStep& new_raw_step = standard.raw_steps.emplace_back();
//		new_raw_step.mouse_pos = { mouse_history.x, mouse_history.y };
//
//		if (standard.raw_steps.size() % sample_count == 0) {
//
//			glm::vec2 average = {};
//
//			for (uint32_t i = 0; i < sample_count; i++) {
//
//				BrushStep& prev_step = standard.raw_steps[standard.raw_steps.size() - 1 - i];
//				average += prev_step.mouse_pos;
//			}
//
//			average /= sample_count;
//
//			glm::vec3 ray_target;
//			renderer.getPixelWorldPosition((uint32_t)roundl(average.x), (uint32_t)roundl(average.y), ray_target);
//
//			if (ray_target.x != FLT_MAX) {
//
//				// localize ray target
//				glm::vec3 local_ray_target = ray_target;
//				instance->localizePosition(local_ray_target);
//
//				BrushStep& new_applied_step = standard.applied_steps.emplace_back();
//				new_applied_step.mouse_pos = average;
//				new_applied_step.target = local_ray_target;
//
//				BrushProperty<float> strength = standard.strength;
//				strength._stroke_value *= sample_count;
//
//				scme::SculptMesh& sculpt_mesh = instance->getSculptMesh();
//				sculpt_mesh.applyStandardBrush(
//					local_ray_origin,
//					standard.sample_count, standard.radius, strength, standard.falloff,
//					standard.applied_steps, start_step
//				);
//			}
//		}
//	}
//}
//
//void endStandardBrushStroke(nui::Window*, nui::StoredElement2*, void*)
//{
//	Application& app = application;
//	SculptContext& sculpt = app.sculpt;
//
//	if (sculpt.stroke_started == false) {
//		return;
//	}
//
//	sculpt.stroke_started = false;
//}
//
//void onCameraOrbitKeyDown(nui::Window* window, nui::StoredElement2* source, void*)
//{
//	nui::Input& input = window->input;
//	glm::vec3 pixel_world_pos;
//	renderer.getPixelWorldPosition(input.mouse_x, input.mouse_y, pixel_world_pos);
//
//	//printf("%.2f %.2f %.2f \n", pixel_world_pos.x, pixel_world_pos.y, pixel_world_pos.z);
//
//	if (pixel_world_pos.x != FLT_MAX) {
//		application.setCameraFocus(pixel_world_pos);
//	}
//
//	auto viewport = source->get<nui::DirectX11_Viewport>();
//	viewport->state->events.beginMouseFixedDeltaEffect(viewport->state->box);
//}
//
//void onCameraOrbitKeyHeld(nui::Window* window, nui::StoredElement2*, void*)
//{
//	int32_t delta_x = window->input.mouse_delta_x;
//	int32_t delta_y = window->input.mouse_delta_y;
//
//	float scaling = application.camera_orbit_sensitivity * application.main_window->delta_time;
//	application.arcballOrbitCamera((float)delta_x * scaling, (float)delta_y * scaling);
//}
//
//void onCameraOrbitKeyUp(nui::Window* window, nui::StoredElement2*, void*)
//{
//	window->endMouseDeltaEffect();
//}
//
//void onCameraPanKeyDown(nui::Window*, nui::StoredElement2* source, void*)
//{
//	auto viewport = source->get<nui::DirectX11_Viewport>();
//	viewport->state->events.beginMouseLoopDeltaEffect(viewport->state->box);
//}
//
//void onCameraPanKeyHeld(nui::Window* window, nui::StoredElement2*, void*)
//{
//	int32_t delta_x = window->input.mouse_delta_x;
//	int32_t delta_y = window->input.mouse_delta_y;
//
//	float scaling = application.camera_pan_sensitivity * application.main_window->delta_time;
//	application.panCamera((float)-delta_x * scaling, (float)-delta_y * scaling);
//}
//
//void onCameraPanKeyUp(nui::Window* window, nui::StoredElement2*, void*)
//{
//	window->endMouseDeltaEffect();
//}
//
//void onCameraDollyScroll(nui::Window* window, nui::StoredElement2*, void*)
//{
//	nui::Input& input = window->input;
//	glm::vec3 pixel_world_pos;
//	renderer.getPixelWorldPosition(input.mouse_x, input.mouse_y, pixel_world_pos);
//
//	if (pixel_world_pos.x != FLT_MAX) {
//		application.setCameraFocus(pixel_world_pos);
//	}
//
//	application.dollyCamera(window->input.mouse_wheel_delta * application.camera_dolly_sensitivity);
//}
//
//void setBackCullingTrue(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.getRootDrawcall().is_back_culled = true;
//}
//
//void setBackCullingFalse(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.getRootDrawcall().is_back_culled = false;
//}
//
//void setShadingNormalVertex(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.setShadingNormal(GPU_ShadingNormal::VERTEX);
//}
//
//void setShadingNormalPoly(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.setShadingNormal(GPU_ShadingNormal::POLY);
//}
//
//void setShadingNormalTesselation(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.setShadingNormal(GPU_ShadingNormal::TESSELATION);
//}
//
//void setDisplayModeSolid(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.getRootDrawcall().display_mode = DisplayMode::SOLID;
//}
//
//void setDisplayModeWireframeOverlay(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.getRootDrawcall().display_mode = DisplayMode::WIREFRAME_OVERLAY;
//}
//
//void setDisplayModeWireframe(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.getRootDrawcall().display_mode = DisplayMode::WIREFRANE;
//}
//
//void changeShadingNormal(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.shading_normal = (application.shading_normal + 1) % 3;
//}
//
//void renderAABBs_Normal(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.setAABB_RenderModeForDrawcall(
//		&application.getRootDrawcall(),
//		AABB_RenderMode::NORMAL
//	);
//}
//
//void renderAABBs_LeafOnly(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.setAABB_RenderModeForDrawcall(
//		&application.getRootDrawcall(),
//		AABB_RenderMode::LEAF_ONLY
//	);
//}
//
//void hideAABBs(nui::Window*, nui::StoredElement2*, void*)
//{
//	application.setAABB_RenderModeForDrawcall(
//		&application.getRootDrawcall(),
//		AABB_RenderMode::NO_RENDER
//	);
//}
