
#include "NuiLibrary.hpp"
#include "Application.hpp"


void onCameraOrbitKeyDown(nui::KeyDownEvent& event)
{
	nui::Surface* wrap = std::get_if<nui::Surface>(&event.source->elem);

	wrap->beginMouseDelta();
}

void onCameraOrbitKeyHeld(nui::KeyHeldDownEvent& event)
{
	int32_t delta_x;
	int32_t delta_y;
	application.window->getMouseDelta(delta_x, delta_y);

	float scaling = application.camera_orbit_sensitivity * application.window->delta_time;
	application.arcballOrbitCamera((float)delta_x * scaling, (float)delta_y * scaling);
}

void onCameraOrbitKeyUp(nui::KeyUpEvent& event)
{
	nui::Surface* wrap = std::get_if<nui::Surface>(&event.source->elem);

	wrap->endMouseDelta();
}

void onCameraPanKeyDown(nui::KeyDownEvent& event)
{
	nui::Surface* wrap = std::get_if<nui::Surface>(&event.source->elem);

	wrap->beginMouseDelta();
}

void onCameraPanKeyHeld(nui::KeyHeldDownEvent& event)
{
	int32_t delta_x;
	int32_t delta_y;
	application.window->getMouseDelta(delta_x, delta_y);

	float scaling = application.camera_pan_sensitivity * application.window->delta_time;
	application.panCamera((float)-delta_x * scaling, (float)-delta_y * scaling);
}

void onCameraPanKeyUp(nui::KeyUpEvent& event)
{
	nui::Surface* wrap = std::get_if<nui::Surface>(&event.source->elem);

	wrap->endMouseDelta();
}

void onCameraDollyScroll(nui::MouseScrollEvent& event)
{
	application.dollyCamera(event.scroll_delta * application.camera_dolly_sensitivity);
}

void onCameraResetKeyDown(nui::KeyDownEvent& event)
{
	application.setCameraPosition(0, 0, 10);
	application.setCameraRotation(0, 0, 0);
}

int WINAPI WinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
	ErrStack err_stack;

	// Application
	{
		// Layers
		application.last_used_layer = &application.layers.emplace_back();
		application.last_used_layer->parent = nullptr;
		application.last_used_layer->name = "Root Layer";

		// Instances
		application.instance_id = 0;

		// Drawcalls
		application.last_used_drawcall = &application.drawcalls.emplace_back();
		application.last_used_drawcall->name = "Root Drawcall";
		application.last_used_drawcall->rasterizer_mode = DisplayMode::SOLID;
		application.last_used_drawcall->_debug_show_octree = false;

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
		application.camera_field_of_view = 15;
		application.camera_z_near = 100'000'000.f;
		application.camera_z_far = 0.1f;
		application.camera_pos = { 0, 0, 0 };
		application.camera_quat_inv = { 1, 0, 0, 0 };
		application.camera_forward = { 0, 0, -1 };
		application.camera_orbit_sensitivity = 50000;
		application.camera_pan_sensitivity = 250;
		application.camera_dolly_sensitivity = 0.001f;

		// Tests	
		application.setCameraPosition(0, 0, 10);
		application.setCameraFocus(glm::vec3(0, 0, 0));

		MeshDrawcall* drawcalls[4];

		drawcalls[0] = application.createDrawcall();
		drawcalls[1] = application.createDrawcall();
		drawcalls[2] = application.createDrawcall();
		drawcalls[3] = application.createDrawcall();

		drawcalls[0]->rasterizer_mode = DisplayMode::SOLID;
		drawcalls[0]->_debug_show_octree = true;

		drawcalls[1]->rasterizer_mode = DisplayMode::SOLID_WITH_WIREFRAME_FRONT;
		drawcalls[2]->rasterizer_mode = DisplayMode::SOLID_WITH_WIREFRAME_NONE;
		drawcalls[3]->rasterizer_mode = DisplayMode::WIREFRANE_PURE;

		std::vector<MeshInstance*> new_instances;
		{
			io::FilePath path;
			path.recreateRelative("Sculpt/Meshes/Journey/scene.gltf");

			GLTF_ImporterSettings settings;
			settings.max_vertices_in_AABB = 128;

			err_stack = application.importMeshesFromGLTF_File(path, settings, &new_instances);
			if (err_stack.isBad()) {
				err_stack.debugPrint();
				return 1;
			}
		}
		
		uint32_t target_inst = 4;

		for (uint32_t i = 0; i < new_instances.size(); i++) {

			if (i == target_inst) {
				application.transferToDrawcall(new_instances[target_inst], drawcalls[0]);
			}
			else {
				application.deleteInstance(new_instances[i]);
			}
		}

		/*float sphere_y = 250.f;
		float sphere_diameter = 50.f;
		float spacing = 100;*/

		/*{
			CreateUV_SphereInfo info;
			info.transform.pos = { 0, sphere_y, 0 };
			info.diameter = sphere_diameter;

			info.vertical_sides = 15;
			info.horizontal_sides = 15;

			MeshInstance* inst = application.createUV_Sphere(info, nullptr, drawcalls[0]);
		}*/

		/*for (uint32_t i = 1; i < 4; i++) {

			{
				CreateUV_SphereInfo info;
				info.transform.pos = { i * spacing, sphere_y, 0 };
				info.diameter = sphere_diameter;

				info.vertical_sides = 15;
				info.horizontal_sides = 15;

				MeshInstance* inst = application.createUV_Sphere(info, nullptr, drawcalls[i]);
			}

			for (MeshInstance*& inst : new_instances) {
				inst = application.copyInstance(inst);
				inst->pos.x = i * spacing;
				application.transferToDrawcall(inst, drawcalls[i]);
			}
		}*/
	}

	nui::Instance instance;
	err_stack = instance.create();
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return 1;
	}

	nui::WindowCreateInfo info;
	info.width = 1027;
	info.height = 720;

	err_stack = instance.createWindow(info, application.window);
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return 1;
	}

	// UI code
	nui::Surface* surface = application.window->addSurface();
	surface->gpu_callback = geometryDraw;
	surface->user_data = &application.renderer;

	// Camera Orbit
	surface->addKeyDownEvent(onCameraOrbitKeyDown, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
	surface->addKeyHeldDownEvent(onCameraOrbitKeyHeld, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
	surface->addKeyUpEvent(onCameraOrbitKeyUp, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);

	// Camera Pan
	surface->addKeyDownEvent(onCameraPanKeyDown, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);
	surface->addKeyHeldDownEvent(onCameraPanKeyHeld, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);
	surface->addKeyUpEvent(onCameraPanKeyUp, nui::VirtualKeys::MIDDLE_MOUSE_BUTTON);

	// Camera Dolly
	surface->setMouseScrollEvent(onCameraDollyScroll);

	// Camera Reset
	surface->addKeyDownEvent(onCameraResetKeyDown, nui::VirtualKeys::R);

	while (!application.window->close) {

		err_stack = instance.update();
		if (err_stack.isBad()) {
			err_stack.debugPrint();
			return 1;
		}
	}

	return 0;
}

int main(int argc, char** argv)
{
	return WinMain(GetModuleHandleA(NULL), NULL, "", 1);
}
