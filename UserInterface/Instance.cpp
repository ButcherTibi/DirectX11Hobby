
#include "pch.h"

// Header
#include "NuiLibrary.hpp"

// Mine
#include "FilePath.hpp"


using namespace nui;


std::list<Window> nui::windows;


ErrStack Instance::create()
{
	ErrStack err_stack;

	_hinstance = GetModuleHandleA(NULL);
	_arrow_cursor = LoadCursorA(NULL, IDC_ARROW);

	checkErrStack1(io::readLocalFile("UserInterface/CompiledShaders/WrapVS.cso", _wrap_vs_cso));
	checkErrStack1(io::readLocalFile("UserInterface/CompiledShaders/CharsVS.cso", _chars_vs_cso));
	checkErrStack1(io::readLocalFile("UserInterface/CompiledShaders/AllVS.cso", _all_vs_cso));

	checkErrStack1(io::readLocalFile("UserInterface/CompiledShaders/WrapPS.cso", _wrap_ps_cso));
	checkErrStack1(io::readLocalFile("UserInterface/CompiledShaders/CharsPS.cso", _chars_ps_cso));
	checkErrStack1(io::readLocalFile("UserInterface/CompiledShaders/CopyParentsMaskPS.cso", _copy_parents_ps_cso));

	Font* font;
	checkErrStack1(_char_atlas.addFont("UserInterface/Fonts/Roboto-Regular.ttf", font));
	checkErrStack1(font->addSize(14));

	return err_stack;
}

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
	ErrStack err_stack;

	for (Window& wnd : windows) {
		if (wnd.hwnd == hwnd) {

			switch (uMsg) {

			case WM_SIZE: {

				switch (wParam) {
				case SIZE_MAXIMIZED:
				case SIZE_RESTORED: {
					wnd.minimized = false;

					wnd.width = getLowOrder((uint32_t)lParam);
					wnd.height = getHighOrder((uint32_t)lParam);

					RECT client_rect;
					GetClientRect(hwnd, &client_rect);
					wnd.surface_width = (client_rect.right - client_rect.left);
					wnd.surface_height = (client_rect.bottom - client_rect.top);

					/*if (wnd.surface_width && wnd.surface_height) {

						err_stack = wnd._draw();
						if (err_stack.isBad()) {
							err_stack.debugPrint();
						}
					}*/
					break;
				}

				case SIZE_MINIMIZED: {
					wnd.minimized = true;
					break;
				}
				}

				return 0;
			}

			case WM_MOUSEMOVE: {
				wnd.input.mouse_x = getLowOrder((uint32_t)lParam);
				wnd.input.mouse_y = getHighOrder((uint32_t)lParam);
				return 0;
			}

			case WM_KEYDOWN: {
				wnd.input.setKeyDownState((uint32_t)wParam, (uint32_t)lParam);
				return 0;
			}

			case WM_KEYUP: {
				wnd.input.setKeyUpState((uint32_t)wParam);
				return 0;
			}

			case WM_LBUTTONDOWN: {
				wnd.input.setKeyDownState(VirtualKeys::LEFT_MOUSE_BUTTON, 0);
				return 0;
			}

			case WM_LBUTTONUP: {
				wnd.input.setKeyUpState(VirtualKeys::LEFT_MOUSE_BUTTON);
				return 0;
			}

			case WM_RBUTTONDOWN: {
				wnd.input.setKeyDownState(VirtualKeys::RIGHT_MOUSE_BUTTON, 0);
				return 0;
			}

			case WM_RBUTTONUP: {
				wnd.input.setKeyUpState(VirtualKeys::RIGHT_MOUSE_BUTTON);
				return 0;
			}

			case WM_MBUTTONDOWN: {
				wnd.input.setKeyDownState(VirtualKeys::MIDDLE_MOUSE_BUTTON, 0);
				return 0;
			}

			case WM_MBUTTONUP: {
				wnd.input.setKeyUpState(VirtualKeys::MIDDLE_MOUSE_BUTTON);
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
					printf("failed to get raw input data \n %s \n %s \n", code_location, getLastError().c_str());
				}

				RAWINPUT* raw = (RAWINPUT*)raw_input.data();
				wnd.input.mouse_delta_x = raw->data.mouse.lLastX;
				wnd.input.mouse_delta_y = raw->data.mouse.lLastY;

				return 0;
			}

			case WM_QUIT:
			case WM_CLOSE: {
				wnd.close = true;
				return 0;
			}

			case WM_DESTROY: {
				exit(1);
			}
			}
		}
	}
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

ErrStack Instance::createWindow(WindowCreateInfo& info, Window*& r_window)
{
	ErrStack err_stack;
	HRESULT hr = S_OK;

	Window& w = windows.emplace_back();
	w.instance = this;

	// Initialize Input
	{
		SteadyTime now = std::chrono::steady_clock::now();

		for (uint16_t virtual_key = 0; virtual_key < w.input.key_list.size(); virtual_key++) {

			KeyState& key = w.input.key_list[virtual_key];
			key.virtual_key = virtual_key;
			key.is_down = false;
			key.first_message = false;
			key.first_frame = false;
			key.start_time = now;
			key.end_time = now;
		}

		w.input.mouse_delta_x = 0;
		w.input.mouse_delta_y = 0;

		// Raw Device Input for Mouse
		RAWINPUTDEVICE raw_input_dev;
		raw_input_dev.usUsagePage = 0x01;
		raw_input_dev.usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
		raw_input_dev.dwFlags = 0;  // NOTE: RIDEV_NOLEGACY where legacy means regular input like WM_KEYDOWN or WM_LBUTTONDOWN
		raw_input_dev.hwndTarget = 0;

		if (!RegisterRawInputDevices(&raw_input_dev, 1, sizeof(RAWINPUTDEVICE))) {
			return ErrStack(code_location, getLastError());
		};
	}

	// Create Window
	const char CLASS_NAME[] = "Sample Window Class";
	w.window_class = {};
	w.window_class.style = CS_OWNDC;
	w.window_class.lpfnWndProc = (WNDPROC)windowProc;
	w.window_class.cbClsExtra = 0;
	w.window_class.cbWndExtra = 0;
	w.window_class.hInstance = _hinstance;
	w.window_class.hCursor = _arrow_cursor;
	w.window_class.lpszClassName = CLASS_NAME;

	if (!RegisterClassA(&w.window_class)) {
		return ErrStack(code_location, "failed to register window class");
	}

	w.hwnd = CreateWindowExA(
		0,                              // Optional window styles
		CLASS_NAME,                     // Window class
		"Window",                       // Window text
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,            // Window style
		CW_USEDEFAULT, CW_USEDEFAULT, info.width, info.height, // Position and Size
		NULL,       // Parent window
		NULL,       // Menu
		_hinstance,  // Instance handle
		NULL        // Additional application data
	);

	if (w.hwnd == NULL) {
		return ErrStack(code_location, "failed to create window");
	}

	// Window Size
	w.width = info.width;
	w.height = info.height;

	// Surface size
	{
		RECT client_rect;
		if (!GetClientRect(w.hwnd, &client_rect)) {
			return ErrStack(code_location, "failed to retrieve window client rect");
		}

		w.surface_width = client_rect.right - client_rect.left;
		w.surface_height = client_rect.bottom - client_rect.top;
	}

	// Messages
	w.minimized = false;
	w.close = false;
	w.mouse_delta_owner = nullptr;

	// Rendering
	w.rendering_configured = false;
	w.wrap_verts[0].pos = { 0, 0 };
	w.wrap_verts[1].pos = { 1, 0 };
	w.wrap_verts[2].pos = { 1, 1 };
	w.wrap_verts[3].pos = { 0, 1 };
	w.wrap_idxs = {
		0, 1, 3,
		1, 2, 3
	};

	// Factory
	{
		checkHResult(CreateDXGIFactory2(0, IID_PPV_ARGS(w.factory.GetAddressOf())),
			"failed to create DXGI factory");
	}

	// Choose Adapter with most VRAM
	{
		uint32_t adapter_idx = 0;
		IDXGIAdapter* found_adapter;
		size_t adapter_vram = 0;

		while (SUCCEEDED(w.factory.Get()->EnumAdapters(adapter_idx, &found_adapter))) {

			DXGI_ADAPTER_DESC desc;
			checkHResult(found_adapter->GetDesc(&desc),
				"failed to get adapter description");

			if (desc.DedicatedVideoMemory > adapter_vram) {
				adapter_vram = desc.DedicatedVideoMemory;
				w.adapter = found_adapter;
			}
			adapter_idx++;
		}
	}

	// Device
	{
		std::array<D3D_FEATURE_LEVEL, 1> features = {
			D3D_FEATURE_LEVEL_11_1
		};

		checkHResult(D3D11CreateDevice(w.adapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN,
			NULL,
			D3D11_CREATE_DEVICE_DEBUG,
			features.data(), (uint32_t)features.size(),
			D3D11_SDK_VERSION,
			w._dev.GetAddressOf(),
			NULL,
			w._im_ctx.GetAddressOf()),
			"failed to create device");

		checkHResult(w._dev->QueryInterface<ID3D11Device5>(w.dev5.GetAddressOf()),
			"failed to obtain ID3D11Device5");

		checkHResult(w.dev5->CreateDeferredContext3(0, w.de_ctx3.GetAddressOf()),
			"failed to obtain deferred context 3");

		checkHResult(w._im_ctx->QueryInterface<ID3D11DeviceContext4>(w.im_ctx4.GetAddressOf()),
			"failed to obtain ID3D11DeviceContext4");
	}

	// Swapchain
	{
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = 0;
		desc.BufferDesc.Height = 0;
		desc.BufferDesc.RefreshRate.Numerator = 0;
		desc.BufferDesc.RefreshRate.Denominator = 0;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 2;
		desc.OutputWindow = w.hwnd;
		desc.Windowed = true;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		checkHResult(w.factory->CreateSwapChain(w.dev5.Get(), &desc, w.swapchain.GetAddressOf()),
			"failed to create swapchain");
	}

	// Present Target View
	{
		checkHResult(w.swapchain->GetBuffer(0, IID_PPV_ARGS(w.present_img.GetAddressOf())),
			"failed to get swapchain back buffer");

		checkHResult(w.dev5->CreateRenderTargetView(w.present_img.Get(), NULL, w.present_rtv.GetAddressOf()),
			"failed to create present RTV");
	}

	// Root Node
	{
		Node& root_node = w.nodes.emplace_back();
		root_node.collider.set(w.surface_width, w.surface_height);
		root_node.layer_idx = 0;
		root_node.parent = nullptr;

		Root* root = root_node.createRoot();
		root->EventComp::_create(&w, &root_node);
		root->NodeComp::_create(&w, &root_node);
	}

	// Delta Time
	w.start_time = std::chrono::steady_clock::now();

	r_window = &w;
	return ErrStack();
}

ErrStack Instance::update()
{
	ErrStack err_stack;

	// Input
	for (Window& window : windows) {	

		window.delta_time = fsec_cast(std::chrono::steady_clock::now() - window.start_time);

		WaitMessage();

		MSG msg{};
		while (PeekMessageA(&msg, window.hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		if (!window.minimized &&
			window.surface_width > 10 && window.surface_height > 10)
		{
			window.input.startFrame();

			checkErrStack1(window._updateCPU_Data());
			checkErrStack1(window._render());

			window.input.endFrame();
		}

		window.start_time = std::chrono::steady_clock::now();
	}
	
	return err_stack;
}

Instance::~Instance()
{
	windows.clear();
}
