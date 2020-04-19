
// Standard
#include <chrono>

// Mine
#include "Input.h"
#include "Primitives.h"
#include "Importer.h"
#include "VulkanSystems.h"
#include "TextRendering.h"

// Image Loading
#include "stb_image.h"

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
			// ErrStack(ExtraError::FAILED_TO_GET_RAW_INPUT_DATA, code_location, "failed to get raw input", getLastError());
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

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	printf("WinMain \n");

	app_level.window_width = 1024;
	app_level.window_height = 720;

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
			CW_USEDEFAULT, CW_USEDEFAULT, app_level.window_width, app_level.window_height,

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

	ErrStack err;

	// Load Shader Code
	std::vector<char> g3d_vert_shader_code;
	std::vector<char> g3d_frag_shader_code;
	std::vector<char> ui_vert_shader_code;
	std::vector<char> ui_frag_shader_code;
	std::vector<char> comp_vert_shader_code;
	std::vector<char> comp_frag_shader_code;
	{
		FileSysPath path;
		err = path.recreateRelative("shaders/3D/vert.spv");
		err = path.read(g3d_vert_shader_code);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}

		path.recreateRelative("shaders/3D/frag.spv");
		err = path.read(g3d_frag_shader_code);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}

		path.recreateRelative("shaders/UI/vert.spv");
		err = path.read(ui_vert_shader_code);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}


		path.recreateRelative("shaders/UI/frag.spv");
		err = path.read(ui_frag_shader_code);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}
		
		path.recreateRelative("shaders/Compose/vert.spv");
		err = path.read(comp_vert_shader_code);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}

		path.recreateRelative("shaders/Compose/frag.spv");
		err = path.read(comp_frag_shader_code);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}
	}

	// Mesh
	std::vector<LinkageMesh> meshes;
	{
		FileSysPath path;
		path.recreateRelative("meshes/DamagedHelmet/DamagedHelmet.gltf");

		err = importGLTFMeshes(path, meshes);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}
	}

	Camera camera;
	camera.position.z = 1.5;

	// Mesh Diffuse Texture
	BasicBitmap mesh_diffuse;
	{
		FileSysPath tex_path;
		tex_path.recreateRelative("meshes/DamagedHelmet/Default_albedo.jpg");
		
		int32_t width, height, channels;

		uint8_t* tex_pixels = stbi_load(tex_path.toWindowsPath().c_str(),
			&width, &height, &channels, STBI_rgb_alpha);

		mesh_diffuse.width = width;
		mesh_diffuse.height = height;
		mesh_diffuse.channels = 4;
		mesh_diffuse.colors.resize(mesh_diffuse.calcMemSize());

		memcpy(mesh_diffuse.colors.data(), tex_pixels, mesh_diffuse.calcMemSize());

		stbi_image_free(tex_pixels);
	}

	// Text Rendering
	TextStuff text_stuff;
	{
		std::vector<uint8_t> roboto_font_ttf;
		{
			FileSysPath font_path;
			font_path.recreateRelative("/UI/Fonts/Roboto/Roboto-Regular.ttf");
			
			err = font_path.read(roboto_font_ttf);
			if (err.isBad()) {
				err.debugPrint();
				return 1;
			}
		}	

		FontInfo info;
		info.family_name = "roboto";
		info.style_name = "regular";
		info.sizes_px = {100};

		err = text_stuff.addFont(roboto_font_ttf, info);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}

		err = text_stuff.rebindToAtlas(1024);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}

		ui::CharSeq seq;	
		seq.setCharacters("Finally . . . Hello World from the GPU", 16, "roboto", "regular", { 0, 0.5f });

		err = text_stuff.addInstances(seq, app_level.window_width, app_level.window_height);
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}
	}

	RenderingContent render_content;
	render_content.hinstance = &hinstance;
	render_content.hwnd = &app_level.hwnd;
	render_content.width = app_level.window_width;
	render_content.height = app_level.window_height;

	// 3D
	render_content.mesh_diffuse = &mesh_diffuse;
	render_content.meshes = &meshes;
	render_content.camera = &camera;
	render_content.g3d_vert_shader_code = &g3d_vert_shader_code;
	render_content.g3d_frag_shader_code = &g3d_frag_shader_code;

	// UI
	render_content.char_atlas_changed = true;
	render_content.text_rendering = &text_stuff;
	render_content.ui_vert_shader_code = &ui_vert_shader_code;
	render_content.ui_frag_shader_code = &ui_frag_shader_code;

	// Compose
	render_content.comp_vert_shader_code = &comp_vert_shader_code;
	render_content.comp_frag_shader_code = &comp_frag_shader_code;

	err = renderer.recreate(render_content);
	if (err.isBad()) {
		err.debugPrint();
		return 1;
	}

	steady_time frame_start;
	float delta_time = 1;
	
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

		// Controls
		float rotate_shortcut = input.rotate_camera.duration;
		float zoom_shortcut = input.zoom_camera.duration;
		float pan_shortcut = input.pan_camera.duration;
		float focus_shortcut = input.focus_camera.duration;

		if (rotate_shortcut) {

			float rotation_sensitivity = 20.0f;
			float delta_pitch = (float)input.mouse_delta_y * rotation_sensitivity * delta_time;
			float delta_yaw = (float)input.mouse_delta_x * rotation_sensitivity * delta_time;

			camera.orbitCameraArcball({ 0, 0, 0 }, delta_pitch, delta_yaw);
		}
		else if (zoom_shortcut) {

			float zoom_sensitivity = 0.5f;
			float zoom_amount = (float)input.mouse_delta_y * zoom_sensitivity * delta_time;

			camera.zoomCamera({ 0, 0, 0 }, zoom_amount);
		}
		else if (pan_shortcut) {

			float pan_sensitivity = 1.0f;
			float delta_vertical = (float)input.mouse_delta_y * pan_sensitivity * delta_time;
			float delta_horizontal = (float)input.mouse_delta_x * pan_sensitivity * delta_time;

			camera.panCamera(delta_vertical, delta_horizontal);
		}
		else if (focus_shortcut) {
			camera.position = { 0, 0, 5 };
			camera.rotation = { 1, 0, 0, 0 };
		}

		// Begin render comands
		renderer.waitForRendering();
		renderer.draw();

		delta_time = fsec_cast(std::chrono::steady_clock::now() - frame_start);
	}

	if (err.isBad()) {
		err.debugPrint();
		return 1;
	}

	vkDeviceWaitIdle(renderer.logical_dev.logical_device);
	DestroyWindow(app_level.hwnd);

	return 0;
}