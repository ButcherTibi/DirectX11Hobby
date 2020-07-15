
// Mine
#include "FileIO.h"
#include "DX11Renderer.h"

// new
#include "Nui.h"

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

	nui::Nui ui;
	err = ui.create(app_level.hwnd);
	if (err.isBad()) {
		err.debugPrint();
		return 1;
	}

	nui::Element& root = ui.getRoot();
	{
		nui::Flex& root_elem = ui.getRootElement();

		nui::Flex f0 = {};
		f0.background_color = { 1, 0, 0, 1 };
		f0.box_sizing = nui::BoxSizing::BORDER;
		f0.width.setRelative(0.5);
		f0.height.setRelative(0.5);
		f0.padding_tl_radius = 50;
		f0.padding_tr_radius = 50;
		f0.padding_br_radius = 50;
		f0.padding_bl_radius = 50;
		nui::Element& f0_elem = ui.addElement(root, f0);
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

		err = ui.draw();
		if (err.isBad()) break;
	}

	if (err.isBad()) {
		err.debugPrint();
		return 1;
	}

	DestroyWindow(app_level.hwnd);

	return 0;
}