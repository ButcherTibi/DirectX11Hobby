
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
		application.layers.emplace_back();

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
		application.camera_field_of_view = 90;
		application.camera_z_near = 0.1f;
		application.camera_z_far = 100;
		application.camera_pos = { 0, 0, 0 };
		application.camera_quat_inv = { 1, 0, 0, 0 };
		application.camera_forward = { 0, 0, -1 };
		application.camera_orbit_sensitivity = 50000;
		application.camera_pan_sensitivity = 250;
		application.camera_dolly_sensitivity = 0.001f;

		
		// Test
		//{
		//	CreateTriangleInfo info;
		//	info.transform.pos = { 0, -2, 0 };

		//	// info.mesh_shading_subprimitive = MeshShadingSubPrimitive::VERTEX;
		//	info.material.albedo_color = { 1, 0, 0 };
		//	info.material.roughness = 0.5f;
		//	info.material.metallic = 0.0f;
		//	info.material.specular = 0.04f;

		//	application.createTriangleMesh(info);
		//}

		//{
		//	CreateQuadInfo info;
		//	info.transform.pos = { 0, -4, 0 };

		//	// info.mesh_shading_subprimitive = MeshShadingSubPrimitive::TESSELATION;
		//	info.material.albedo_color = { 1, 0, 0 };
		//	info.material.roughness = 0.5f;
		//	info.material.metallic = 0.0f;
		//	info.material.specular = 0.04f;

		//	application.createQuadMesh(info);
		//}
		//
		//{
		//	CreateCubeInfo info;
		//	info.transform.pos = { 0, -6, 0};

		//	// info.mesh_shading_subprimitive = MeshShadingSubPrimitive::VERTEX;
		//	info.material.albedo_color = { 1, 0, 0 };
		//	info.material.roughness = 0.5f;
		//	info.material.metallic = 0.0f;
		//	info.material.specular = 0.04f;

		//	application.createCubeMesh(info);
		//}

		//{
		//	CreateCylinderInfo info;
		//	info.transform.pos = { 0, -8, 0 };

		//	info.vertical_sides = 2;
		//	info.horizontal_sides = 300;

		//	// info.mesh_shading_subprimitive = MeshShadingSubPrimitive::VERTEX;
		//	info.material.albedo_color = { 1, 0, 0 };
		//	info.material.roughness = 0.5f;
		//	info.material.metallic = 0.0f;
		//	info.material.specular = 0.04f;

		//	application.createCylinder(info);
		//}

		{
			CreateUV_SphereInfo info;
			info.pos = { 0, 0, 0 };
			info.rot = { 1, 0, 0, 0 };
			info.diameter = 1;

			info.vertical_sides = 400;
			info.horizontal_sides = 800;

			info.mesh_shading_subprimitive = MeshShadingSubPrimitive::POLY;
			info.albedo_color = { 1, 0, 0 };
			info.roughness = 0.5f;
			info.metallic = 0.0f;
			info.specular = 0.04f;

			auto& inst = application.createUV_Sphere(info);

			printf("vertex count = %ld \n", (uint32_t)inst.mesh->verts.size());
			printf("memory size = %lld \n", inst.mesh->getMemorySizeMegaBytes());
		}

		/*uint32_t rows = 7;
		uint32_t cols = 7;
		for (uint32_t row = 0; row < rows; row += 1) {
			for (uint32_t col = 0; col < cols; col += 1) {

				CreateUV_SphereInfo info;
				info.pos = { col * 2, row * 2, 0 };
				info.rot = { 1, 0, 0, 0 };
				info.diameter = 1;

				info.vertical_sides = 40;
				info.horizontal_sides = 80;

				info.mesh_shading_subprimitive = MeshShadingSubPrimitive::VERTEX;
				info.albedo_color = { 1, 0, 0 };
				info.roughness = (float)col / (cols - 1);
				info.metallic = (float)row / (rows - 1);;
				info.specular = 0.04f;

				application.createUV_Sphere(info);
			}
		}*/

		application.setCameraFocus(glm::vec3(0, 0, 0));
		application.setCameraPosition(0, 0, 10);
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

	// Camera Frame

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
