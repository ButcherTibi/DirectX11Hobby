
#include "NuiLibrary.hpp"
#include "Application.hpp"


void onCameraOrbitKeyDown(nui::KeyDownEvent& event)
{
	nui::Surface* wrap = std::get_if<nui::Surface>(&event.source->elem);

	wrap->beginMouseDelta();

	printf("RMB down \n");
}

void onCameraOrbit(nui::KeyHeldDownEvent& event)
{
	int32_t delta_x;
	int32_t delta_y;
	application.window->getMouseDelta(delta_x, delta_y);

	float scaling = application.mouse_sensitivity * application.window->delta_time;
	application.arcballOrbitCamera((float)delta_y * scaling, (float)delta_x * scaling,
		application.mesh.pos);
}

void onCameraOrbitKeyUp(nui::KeyUpEvent& event)
{
	nui::Surface* wrap = std::get_if<nui::Surface>(&event.source->elem);

	wrap->endMouseDelta();

	printf("RMB up \n");
}

void onCameraResetKeyDown(nui::KeyDownEvent& event)
{
	application.setCameraPosition(0, 0, 0);
	application.setCameraRotation(0, 0, 0);
}

int WINAPI WinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
	ErrStack err_stack{};

	// Application
	{
		application.mesh.createAsCube(glm::vec3{ 0, 0, -10 }, glm::quat{ 1, 0, 0, 0 }, 1);

		application.field_of_view = 90;
		application.z_near = 0.1f;
		application.z_far = 100;
		application.camera_pos = { 0, 0, 0 };
		application.camera_quat_inv = { 1, 0, 0, 0 };
		application.mouse_sensitivity = 50000;
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
	surface->addKeyHeldDownEvent(onCameraOrbit, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
	surface->addKeyUpEvent(onCameraOrbitKeyUp, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);

	// Camera Pan

	// Camera Dolly

	// Camera Reset
	surface->addKeyDownEvent(onCameraResetKeyDown, nui::VirtualKeys::F);

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
