
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

	UserInterface user_interface = {};
	user_interface.recreateGraph(1, 1);

	Flex* root = std::get_if<Flex>(&user_interface.elems.front().elem);
	root->axis_align = FlexAxisAlign::SPACE_BETWEEN;
	root->cross_axis_align = FlexCrossAxisAlign::CENTER;
	
	{
		Flex basic_elem_0 = {};
		basic_elem_0.box_sizing = BoxSizing::BORDER;
		basic_elem_0.width.setRelative(0.25);
		basic_elem_0.height.setRelative(0.5);

		basic_elem_0.background_color = { 1, 0, 0, 1 };
		basic_elem_0.padding_right.setAbsolute(20);
		basic_elem_0.padding_tl_radius = 50;
		basic_elem_0.padding_tr_radius = 150;
		basic_elem_0.padding_br_radius = 150;

		basic_elem_0.border_right.setAbsolute(2);
		basic_elem_0.border_bot.setAbsolute(2);

		basic_elem_0.border_color = { 0, 1, 0, 1 };
		basic_elem_0.border_tl_radius = 50;
		basic_elem_0.border_tr_radius = 150;
		basic_elem_0.border_br_radius = 150;

		user_interface.addElement(&user_interface.elems.front(), basic_elem_0);
	}
	
	{
		Flex basic_elem_0 = {};
		basic_elem_0.box_sizing = BoxSizing::BORDER;
		basic_elem_0.width.setRelative(0.25);
		basic_elem_0.height.setRelative(0.25);

		basic_elem_0.background_color = { 1, 0, 0, 1 };
		basic_elem_0.padding_right.setAbsolute(20);
		basic_elem_0.padding_tl_radius = 50;
		basic_elem_0.padding_tr_radius = 150;
		basic_elem_0.padding_br_radius = 150;

		basic_elem_0.border_right.setAbsolute(2);
		basic_elem_0.border_bot.setAbsolute(2);

		basic_elem_0.border_color = { 0, 1, 0, 1 };
		basic_elem_0.border_tl_radius = 50;
		basic_elem_0.border_tr_radius = 150;
		basic_elem_0.border_br_radius = 150;

		basic_elem_0.flex_cross_axis_align_self = FlexCrossAxisAlign::START;

		user_interface.addElement(&user_interface.elems.front(), basic_elem_0);
	}

	{
		Flex basic_elem_0 = {};
		basic_elem_0.box_sizing = BoxSizing::BORDER;
		basic_elem_0.width.setRelative(0.25);
		basic_elem_0.height.setRelative(0.25);

		basic_elem_0.background_color = { 1, 0, 0, 1 };
		basic_elem_0.padding_right.setAbsolute(20);
		basic_elem_0.padding_tl_radius = 50;
		basic_elem_0.padding_tr_radius = 150;
		basic_elem_0.padding_br_radius = 150;

		basic_elem_0.border_right.setAbsolute(2);
		basic_elem_0.border_bot.setAbsolute(2);

		basic_elem_0.border_color = { 0, 1, 0, 1 };
		basic_elem_0.border_tl_radius = 50;
		basic_elem_0.border_tr_radius = 150;
		basic_elem_0.border_br_radius = 150;

		user_interface.addElement(&user_interface.elems.front(), basic_elem_0);
	}
	

	// Renderer
	err = renderer.createContext(&hinstance, &app_level.hwnd);
	if (err.isBad()) {
		err.debugPrint();
		return 1;
	}

	uint32_t requested_width = app_level.display_width;
	uint32_t requested_height = app_level.display_height;
	uint32_t rendering_width;
	uint32_t rendering_height;
	err = renderer.getPhysicalSurfaceResolution(rendering_width, rendering_height);
	if (err.isBad()) {
		err.debugPrint();
		return 1;
	}

	// we now have the actual resolution of the rendering surface so calculate user interface layout
	user_interface.changeResolution((float)rendering_width, (float)rendering_height);
	renderer.user_interface = &user_interface;

	err = renderer.recreate(rendering_width, rendering_height);
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

		if (app_level.display_width != requested_width ||
			app_level.display_height != requested_height)
		{
			err = renderer.getPhysicalSurfaceResolution(rendering_width, rendering_height);
			if (err.isBad()) {
				err.debugPrint();
				return 1;
			}

			user_interface.changeResolution((float)rendering_width, (float)rendering_height);
		}

		// Begin render comands
		err = renderer.waitForRendering();
		if (err.isBad()) {
			err.debugPrint();
			return 1;
		}

		if (app_level.display_width != requested_width ||
			app_level.display_height != requested_height)
		{
			err = renderer.changeResolution(rendering_width, rendering_height);
			if (err.isBad()) {
				err.debugPrint();
				return 1;
			}

			requested_width = app_level.display_width;
			requested_height = app_level.display_height;

			printf("resolution = (%d, %d) \n", requested_width, requested_height);
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