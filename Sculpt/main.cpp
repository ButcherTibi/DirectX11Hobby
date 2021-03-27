
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

//void onCameraPanKeyDown(nui::KeyDownEvent& event)
//{
//	nui::RenderingSurface* wrap = std::get_if<nui::RenderingSurface>(&event.source->elem);
//
//	wrap->beginMouseDelta();
//}
//
//void onCameraPanKeyHeld(nui::KeyHeldDownEvent& event)
//{
//	int32_t delta_x;
//	int32_t delta_y;
//	application.main_window->getMouseDelta(delta_x, delta_y);
//
//	float scaling = application.camera_pan_sensitivity * application.main_window->delta_time;
//	application.panCamera((float)-delta_x * scaling, (float)-delta_y * scaling);
//}
//
//void onCameraPanKeyUp(nui::KeyUpEvent& event)
//{
//	nui::RenderingSurface* wrap = std::get_if<nui::RenderingSurface>(&event.source->elem);
//
//	wrap->endMouseDelta();
//}
//
//void onCameraDollyScroll(nui::MouseScrollEvent& event)
//{
//	application.dollyCamera(event.scroll_delta * application.camera_dolly_sensitivity);
//}
//
//void onCameraResetKeyDown(nui::KeyDownEvent& event)
//{
//	application.setCameraPosition(0, 0, 10);
//	application.setCameraRotation(0, 0, 0);
//}


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
		application.camera_z_near = 100'000'000.f;
		application.camera_z_far = 0.1f;
		application.camera_pos = { 0, 0, 0 };
		application.camera_quat_inv = { 1, 0, 0, 0 };
		application.camera_forward = { 0, 0, -1 };
		application.camera_orbit_sensitivity = 50000;
		application.camera_pan_sensitivity = 250;
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
		window_grid->size[0] = 50.f;
		window_grid->size[1] = 50.f;
		window_grid->coloring = nui::BackgroundColoring::RENDERING_SURFACE;
		window_grid->setRenderingSurfaceEvent(geometryDraw);

		window_grid->setKeyDownEvent(onCameraOrbitKeyDown, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
		window_grid->setKeyHeldDownEvent(onCameraOrbitKeyHeld, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
		window_grid->setKeyUpEvent(onCameraOrbitKeyUp, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);

		window_grid->setMouseMoveEvent(onMouseMove);
	}

	// Tests
	{
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
		drawcalls[3]->name = "wire pure";

		std::vector<MeshInstance*> new_instances;
		{
			io::FilePath path;
			path.recreateRelative("Sculpt/Meshes/Journey/scene.gltf");

			GLTF_ImporterSettings settings;
			settings.dest_drawcall = drawcalls[0];

			err_stack = application.importMeshesFromGLTF_File(path, settings, &new_instances);
			if (err_stack.isBad()) {
				err_stack.debugPrint();
				return 1;
			}
		}

		{
			//glm::vec3 ray_origin = { 0.0f, 0.0f, 1.0f };
			//glm::vec3 ray_direction = { 0.0f, 0.0f, -1.0f };

			{
				//CreateQuadInfo info;

				//MeshInstance* inst = application.createQuadMesh(info, nullptr, drawcalls[0]);
				//inst->pos.x = 1.f;

				//application.rotateInstanceAroundY(inst, toRad(45.f));

				//uint32_t isect_poly;
				//float isect_distance;
				//glm::vec3 isect_point;

				//if (application.raycast(inst, ray_origin, ray_direction, isect_poly, isect_distance, isect_point)) {
				//	printf("ray hit \n");
				//}
				//else {
				//	printf("ray miss \n");
				//}
			}

			/*CreateLineInfo info;
			info.origin = ray_origin;
			info.direction = ray_direction;
			info.length = 10.f;

			MeshInstance* inst = application.createLine(info, nullptr, drawcalls[3]);
			inst->wireframe_colors.front_color = { 0.f, 0.f, 1.f };*/
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
	}

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
