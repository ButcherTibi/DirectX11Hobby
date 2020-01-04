
// Standard
#include <chrono>

// Mine
#include "Input.h"
#include "Primitives.h"
#include "Importer.h"

// new
#include "VulkanSystems.h"

// Header
#include "AppLevel.h"


AppLevel app_level;


// handles Windows's window messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// std::cout << "WindowProc" << std::endl;

	switch (uMsg) {
	// Input
	case WM_INPUT: {
		uint32_t cb_size;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &cb_size, sizeof(RAWINPUTHEADER));
		
		RAWINPUT raw_input;
		uint32_t input_size = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw_input, &cb_size,
			sizeof(RAWINPUTHEADER));

		if (input_size == (UINT)-1) {
			// ErrorStack(ExtraError::FAILED_TO_GET_RAW_INPUT_DATA, code_location, "failed to get raw input", getLastError());
		}
		else if (input_size) {
			input.mouse_delta_x = raw_input.data.mouse.lLastX;
			input.mouse_delta_y = raw_input.data.mouse.lLastY;
		}
		break;
	}

	// Exiting
	case WM_QUIT:
	case WM_CLOSE: {
		printf("X \n");
		app_level.run_app_loop = false;
		return 0;
	}	
	}

	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

ErrorStack AppLevel::response()
{
	// Controls
	float rotate_shortcut = input.rotate_camera.duration;
	float zoom_shortcut = input.zoom_camera.duration;
	float pan_shortcut = input.pan_camera.duration;
	float focus_shortcut = input.focus_camera.duration;

	if (rotate_shortcut) {
		
		float rotation_sensitivity = 20.0f;
		float delta_pitch = (float)input.mouse_delta_y * rotation_sensitivity * delta_time;
		float delta_yaw = (float)input.mouse_delta_x * rotation_sensitivity * delta_time;

		renderer.orbitCameraArcball(renderer.meshes[0].position, delta_pitch, delta_yaw);
	}
	else if (zoom_shortcut) {

		float zoom_sensitivity = 0.5f;
		float zoom_amount = (float)input.mouse_delta_y * zoom_sensitivity * delta_time;

		renderer.zoomCamera(renderer.meshes[0].position, zoom_amount);
	}
	else if (pan_shortcut) {

		float pan_sensitivity = 1.0f;
		float delta_vertical = (float)input.mouse_delta_y * pan_sensitivity * delta_time;
		float delta_horizontal = (float)input.mouse_delta_x * pan_sensitivity * delta_time;

		renderer.panCamera(delta_vertical, delta_horizontal);
	}
	else if (focus_shortcut) {
		renderer.camera.position = { 0, 0, 5 };
		renderer.camera.rotation = { 1, 0, 0, 0 };
	}

	// Begin render comands
	checkErrStack(renderer.waitForRendering(), "");
	renderer.generateGPUData();
	checkErrStack(renderer.loadGPUData(), "");
	checkErrStack(renderer.draw(), "");

	return ErrorStack();
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	printf("WinMain \n");

	// Window
	WNDCLASSEXA window_class = {};
	{
		const char win_class_name[] = "Window Class";

		// Register the window class.
		window_class.cbSize = sizeof(window_class);
		window_class.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
		window_class.lpfnWndProc = &WindowProc;
		window_class.hInstance = hinstance;
		window_class.lpszClassName = win_class_name;

		if (!RegisterClassExA(&window_class)) {
			printf("Error: \n"
				"failed to register window class \n"
				"Windows error: %s \n", getLastError().c_str());
			return 1;
		}	

		// Create the window.
		app_level.hwnd = CreateWindowExA(
			WS_EX_LEFT,
			win_class_name,
			"Vulkan aplication",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,

			// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT, 1024, 720,

			NULL,       // Parent window    
			NULL,       // Menu
			hinstance,  // Instance handle
			NULL        // Additional application data
		);

		if (app_level.hwnd == NULL) {
			printf("Error: \n"
				"failed to create window \n"
				"Windows error: %s \n", getLastError().c_str());
			return 1;
		}
	}

	// Input Setup
	{
		RAWINPUTDEVICE raw_input_dev;
		raw_input_dev.usUsagePage = 0x01;
		raw_input_dev.usUsage = 0x02;
		raw_input_dev.dwFlags = 0;
		raw_input_dev.hwndTarget = app_level.hwnd;

		if (!RegisterRawInputDevices(&raw_input_dev, 1, sizeof(RAWINPUTDEVICE))) {

			printf("Error: \n"
				"failed to register raw mouse input device \n"
				"Windows error: %s \n", getLastError().c_str());
			return 1;
		}

		input.addShortcut(&input.rotate_camera, &input.key_mouse_right);
		input.addShortcut(&input.zoom_camera, &input.key_mouse_middle);
		input.addShortcut(&input.pan_camera, &input.key_mouse_left, &input.key_mouse_right);
		input.addShortcut(&input.focus_camera, &input.key_f);
	}

	ErrorStack err;

	// Load Shader Code
	std::vector<char> vert_shader_code;
	std::vector<char> frag_shader_code;
	{
		Path shader_path;
		err = Path::getExePath(shader_path);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}

		shader_path.pop_back(3);
		shader_path.push_back("Sculpt/shaders");

		Path vert_shader_path = shader_path;
		vert_shader_path.push_back("vert.spv");
		
		err = vert_shader_path.read(vert_shader_code);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}

		Path frag_shader_path = shader_path;
		frag_shader_path.push_back("frag.spv");

		err = frag_shader_path.read(frag_shader_code);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}
	}

	// 
	{

	}

	// Scene setup
	{
		/*err = gltf::importMeshes(Path("E:/my_work/Vulkan/Sculpt/Sculpt/meshes/damaged_helmet/damaged_helmet.gltf"),
			renderer.meshes);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}*/

		CreateQuadInfo info = {};
		createQuadMesh(info, renderer.meshes.emplace_back());

		renderer.camera.position.z = 1.5;
	}

	// Render
	renderer.generateGPUData();

	err = renderer.create(hinstance, app_level.hwnd, 1024, 720,
		vert_shader_code, frag_shader_code);
	if (err.isBad()) {
		err.debugPrint();
		return 1;
	}

	steady_time frame_start;
	
	// Message loop
	while (app_level.run_app_loop)
	{
		frame_start = std::chrono::steady_clock::now();

		// reset these in case no WM_INPUT message is present
		input.mouse_delta_x = 0;
		input.mouse_delta_y = 0;

		// Get Window Messages
		MSG msg = { };
		while (PeekMessageA(&msg, app_level.hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		// Get Input
		err = input.update(frame_start);
		if (err.isBad()) {
			err.pushError(code_location, "failed to update input");
			break;
		}

		// Respond
		err = app_level.response();
		if (err.isBad()) {
			break;
		}

		//WaitMessage();  // pretty chill on CPU
		app_level.delta_time = fsec_cast(std::chrono::steady_clock::now() - frame_start);
	}

	DestroyWindow(app_level.hwnd);

	if (err.isBad()) {
		err.debugPrint();
		return 1;
	}

	vkDeviceWaitIdle(renderer.logical_dev.logical_device);

	return 0;
}