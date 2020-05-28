
// Mine
#include "VulkanSystems.h"

// Header
#include "AppLevel.h"


AppLevel app_level;


// handles Windows's window messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// std::cout << "WindowProc" << std::endl;

	switch (uMsg) {
	case WM_SIZE: {

		if (wParam == SIZE_MAXIMIZED || wParam == SIZE_MINIMIZED || wParam == SIZE_RESTORED) {

			uint32_t l_param = static_cast<uint32_t>(lParam);
			app_level.display_width = l_param & 0xFFFF;
			app_level.display_height = l_param >> 16;
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

	app_level.display_width = 1024;
	app_level.display_height = 720;

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
			CW_USEDEFAULT, CW_USEDEFAULT, app_level.display_width, app_level.display_height,

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

	ErrStack err;

	RenderingContent render_content;
	render_content.hinstance = &hinstance;
	render_content.hwnd = &app_level.hwnd;
	render_content.width = app_level.display_width;
	render_content.height = app_level.display_height;

	err = renderer.recreate(render_content);
	if (err.isBad()) {
		err.debugPrint();
		return 1;
	}
	
	// Message loop
	while (app_level.run_app_loop)
	{
		// Get Window Messages
		MSG msg = { };
		while (PeekMessageA(&msg, app_level.hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		// Begin render comands
		err = renderer.waitForRendering();
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}

		err = renderer.draw();
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}
	}

	if (err.isBad()) {
		err.debugPrint();
		return 1;
	}

	vkDeviceWaitIdle(renderer.logical_dev.logical_device);
	DestroyWindow(app_level.hwnd);

	return 0;
}