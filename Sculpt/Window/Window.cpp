#include "./Window.hpp"

#include <source_location>

#include <App/Application.hpp>


FORCEINLINE uint16_t getLowOrder(uint32_t param)
{
	return param & 0xFFFF;
}

FORCEINLINE uint16_t getHighOrder(uint32_t param)
{
	return param >> 16;
}

FORCEINLINE int16_t getSignedLowOrder(uint32_t param)
{
	return param & 0xFFFF;
}

FORCEINLINE int16_t getSignedHighOrder(uint32_t param)
{
	return param >> 16;
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Input& input = app.input;
	Window& w = app.window;

	switch (uMsg) {
	case WM_SIZE: {

		switch (wParam) {
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED: {
			w.win_messages.is_minimized = false;

			w.width = getLowOrder((uint32_t)lParam);
			w.height = getHighOrder((uint32_t)lParam);

			RECT client_rect;
			GetClientRect(hwnd, &client_rect);

			app.viewport.width = (client_rect.right - client_rect.left);
			app.viewport.height = (client_rect.bottom - client_rect.top);
			break;
		}

		case SIZE_MINIMIZED: {
			w.win_messages.is_minimized = true;
			break;
		}
		}

		return 0;
	}

	case WM_MOUSEMOVE: {
		input.mouse_x = getLowOrder((uint32_t)lParam);
		input.mouse_y = getHighOrder((uint32_t)lParam);

		auto& new_pos = input.mouse_pos_history.emplace_back();
		new_pos.x = input.mouse_x;
		new_pos.y = input.mouse_y;
		return 0;
	}

	case WM_MOUSEWHEEL: {
		input.mouse_wheel_delta += GET_WHEEL_DELTA_WPARAM(wParam);
		return 0;
	}

	case WM_KEYDOWN: {
		input.setKeyDownState((uint32_t)wParam, (uint32_t)lParam);
		return 0;
	}

	case WM_KEYUP: {
		input.setKeyUpState((uint32_t)wParam);
		return 0;
	}

	case WM_CHAR: {
		// These are NOT characters
		switch ((uint32_t)wParam) {
		case 27:  // Escape key
		case 13:  // Carriage Return
		case 8:  // Backspace key
		{
			return 0;
		}
		}

		CharacterKeyState& key = input.unicode_list.emplace_back();
		key.code_point = (uint32_t)wParam;
		key.down_transition = (lParam & (1 << 30)) != (1 << 30);
		return 0;
	}

	case WM_LBUTTONDOWN: {
		input.setKeyDownState(VirtualKeys::LEFT_MOUSE_BUTTON, 0);
		return 0;
	}

	case WM_LBUTTONUP: {
		input.setKeyUpState(VirtualKeys::LEFT_MOUSE_BUTTON);
		return 0;
	}

	case WM_RBUTTONDOWN: {
		input.setKeyDownState(VirtualKeys::RIGHT_MOUSE_BUTTON, 0);
		return 0;
	}

	case WM_RBUTTONUP: {
		input.setKeyUpState(VirtualKeys::RIGHT_MOUSE_BUTTON);
		return 0;
	}

	case WM_MBUTTONDOWN: {
		input.setKeyDownState(VirtualKeys::MIDDLE_MOUSE_BUTTON, 0);
		return 0;
	}

	case WM_MBUTTONUP: {
		input.setKeyUpState(VirtualKeys::MIDDLE_MOUSE_BUTTON);
		return 0;
	}

	case WM_INPUT: {
		uint32_t count;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &count,
			sizeof(RAWINPUTHEADER));

		std::vector<uint8_t> raw_input(count);
		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw_input.data(), &count,
			sizeof(RAWINPUTHEADER)) == (uint32_t)-1)
		{
			__debugbreak();
		}

		RAWINPUT* raw = (RAWINPUT*)raw_input.data();
		input.mouse_delta_x += raw->data.mouse.lLastX;
		input.mouse_delta_y += raw->data.mouse.lLastY;

		/*win32::printToOutput(std::format(
			L"delta = {} {} \n", input.mouse_delta_x, input.mouse_delta_y
		));*/
		return 0;
	}

	case WM_QUIT:
	case WM_CLOSE: {
		w.win_messages.should_close = true;
		return 0;
	}

	case WM_DESTROY: {
		// emergency exit do not save progress
		std::abort();
	}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Window::init()
{
	auto& w = app.window;

	cursor = LoadCursor(NULL, IDC_ARROW);

	const wchar_t class_name[] = L"Sample Window Class";
	w.window_class = {};
	w.window_class.lpfnWndProc = (WNDPROC)windowProc;
	w.window_class.cbClsExtra = 0;
	w.window_class.cbWndExtra = 0;
	w.window_class.hInstance = nullptr;
	w.window_class.hCursor = cursor;
	w.window_class.lpszClassName = class_name;

	if (!RegisterClass(&w.window_class)) {
		throw std::exception("failed to register window class");
	}

	const wchar_t window_name[] = L"Window";

	w.hwnd = CreateWindowEx(
		0,                  // Optional window styles
		class_name,                     // Window class
		window_name,                    // Window text
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,            // Window style
		CW_USEDEFAULT, CW_USEDEFAULT, app.window.width, app.window.height, // Position and Size
		NULL,       // Parent window
		NULL,       // Menu
		NULL,       // Instance handle
		NULL        // Additional application data
	);

	if (w.hwnd == NULL) {
		throw std::exception("failed to create window");
	}

	// DirectX 11 SRBG swapchain for presentation to a per pixel transparent Window
	// seems to be . . . lost knowledge

	//BLENDFUNCTION blend_func = {};
	//blend_func.BlendOp = AC_SRC_OVER;
	//blend_func.SourceConstantAlpha = 0xFF;

	//if (!UpdateLayeredWindow(w.hwnd,
	//	nullptr,  // HDC hdcDst,
	//	nullptr,  // POINT * pptDst,
	//	nullptr,  // SIZE * psize,
	//	nullptr,  // HDC hdcSrc,
	//	nullptr,  // POINT * pptSrc,
	//	RGB(0, 0, 0),  // COLORREF crKey
	//	&blend_func,  // BLENDFUNCTION * pblend,
	//	ULW_ALPHA))
	//{
	//	throw std::exception("failed to configure window for transparency");
	//}
}

RECT Window::getClientRectangle()
{
	RECT win_rect;
	GetWindowRect(hwnd, &win_rect);

	RECT client_rect;
	GetClientRect(hwnd, &client_rect);

	uint32_t border_thick = ((win_rect.right - win_rect.left) - client_rect.right) / 2;
	uint32_t header_height;
	{
		uint32_t win_height = win_rect.bottom - win_rect.top;
		uint32_t client_height = client_rect.bottom - client_rect.top;
		header_height = win_height - (client_height + border_thick * 2);
	}

	win_rect.left += border_thick;
	win_rect.right -= border_thick;
	win_rect.top += border_thick + header_height;
	win_rect.bottom -= border_thick;

	return win_rect;
}

bool Window::setLocalMousePosition(u32 x, u32 y)
{
	app.input.mouse_x = (uint16_t)x;
	app.input.mouse_y = (uint16_t)y;

	RECT client_rect = getClientRectangle();
	return SetCursorPos(client_rect.left + x, client_rect.top + y);
}

bool Window::trapMousePosition(u32 x, u32 y)
{
	RECT client_rect = getClientRectangle();
	RECT new_trap;
	new_trap.top = client_rect.top + y;
	new_trap.bottom = new_trap.top + 1;
	new_trap.left = client_rect.left + x;
	new_trap.right = new_trap.left + 1;

	return ClipCursor(&new_trap);
}

bool Window::untrapMousePosition()
{
	return ClipCursor(nullptr);
}

void Window::setMouseVisibility(bool is_visible)
{
	if (is_visible) {
		int32_t internal_display_counter = ShowCursor(true);
		while (internal_display_counter < 0) {
			internal_display_counter = ShowCursor(true);
		}
	}
	else {
		int32_t internal_display_counter = ShowCursor(false);
		while (internal_display_counter >= 0) {
			internal_display_counter = ShowCursor(false);
		}
	}
}
