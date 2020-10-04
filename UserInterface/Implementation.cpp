
#include "pch.h"

// Header
#include "NuiLibrary.hpp"

// Mine
#include "FileIO.hpp"


using namespace nui;


std::list<Window> nui::windows;


ErrStack Instance::create()
{
	ErrStack err_stack;

	_hinstance = GetModuleHandleA(NULL);
	_arrow_cursor = LoadCursorA(NULL, IDC_ARROW);

	checkErrStack1(readLocalFile("UserInterface/CompiledShaders/WrapVS.cso", _wrap_vs_cso));
	checkErrStack1(readLocalFile("UserInterface/CompiledShaders/CharsVS.cso", _chars_vs_cso));
	checkErrStack1(readLocalFile("UserInterface/CompiledShaders/AllVS.cso", _all_vs_cso));

	checkErrStack1(readLocalFile("UserInterface/CompiledShaders/WrapPS.cso", _wrap_ps_cso));
	checkErrStack1(readLocalFile("UserInterface/CompiledShaders/CharsPS.cso", _chars_ps_cso));
	checkErrStack1(readLocalFile("UserInterface/CompiledShaders/CopyParentsMaskPS.cso", _copy_parents_ps_cso));

	Font* font;
	checkErrStack1(_char_atlas.addFont("UserInterface/Fonts/Roboto-Regular.ttf", font));
	checkErrStack1(font->addSize(14));

	return err_stack;
}

uint16_t getLowOrder(uint32_t param)
{
	return param & 0xFFFF;
}

uint16_t getHighOrder(uint32_t param)
{
	return param >> 16;
}

static void setKeyDownState(Window& wnd, uint32_t wParam, uint32_t lParam)
{
	KeyState& key = wnd.input.key_list[wParam];

	if (key.is_down) {
		key.end_time = std::chrono::steady_clock::now();
	}
	// key changed from UP to DOWN
	else {
		key.is_down = true;
		key.first_frame = true;
		key.start_time = std::chrono::steady_clock::now();
		key.end_time = key.start_time;
	}

	key.first_message = !(lParam & (1 << 30));
}

static void setKeyUpState(Window& wnd, uint32_t wParam)
{
	KeyState& key = wnd.input.key_list[wParam];

	key.is_down = false;
	//key.start_time = std::chrono::steady_clock::now();
	//key.end_time = key.start_time;
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

					GetClientRect(hwnd, &wnd.client_rect);

					wnd.surface_width = (wnd.client_rect.right - wnd.client_rect.left);
					wnd.surface_height = (wnd.client_rect.bottom - wnd.client_rect.top);

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
				wnd.mouse_x = getLowOrder((uint32_t)lParam);
				wnd.mouse_y = getHighOrder((uint32_t)lParam);
				return 0;
			}

			case WM_KEYDOWN: {
				setKeyDownState(wnd, (uint32_t)wParam, (uint32_t)lParam);
				return 0;
			}

			case WM_KEYUP: {
				setKeyUpState(wnd, (uint32_t)wParam);
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

ErrStack Instance::createWindow(WindowCrateInfo& info, Window*& r_window)
{
	ErrStack err_stack;
	HRESULT hr = S_OK;

	Window& w = windows.emplace_back();
	w.instance = this;

	const char CLASS_NAME[] = "Sample Window Class";

	w.window_class = {};
	w.window_class.style = CS_OWNDC;
	w.window_class.lpfnWndProc = (WNDPROC)windowProc;
	w.window_class.cbClsExtra = 0;
	w.window_class.cbWndExtra = 0;
	w.window_class.hInstance = _hinstance;
	w.window_class.hCursor = _arrow_cursor;
	w.window_class.lpszClassName = CLASS_NAME;

	w.input.create();  // create input before doing input handling

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

	w.width = info.width;
	w.height = info.height;

	if (!GetClientRect(w.hwnd, &w.client_rect)) {
		return ErrStack(code_location, "failed to retrieve window client rect");
	}
	w.surface_width = w.client_rect.right - w.client_rect.left;
	w.surface_height = w.client_rect.bottom - w.client_rect.top;

	// Messages
	w.minimized = false;
	w.close = false;

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
		root_node.event_comp.create(&w, &root_node);
		root_node.layer_idx = 0;

		root_node.parent = nullptr;

		Root* root = root_node.createRoot();
		root->node_comp.window = &w;
		root->node_comp.this_elem = &root_node;
	}

	r_window = &w;
	return ErrStack();
}

ErrStack Instance::update()
{
	ErrStack err_stack;

	// Input
	for (Window& window : windows) {

		// WaitMessage();

		MSG msg{};
		while (PeekMessageA(&msg, window.hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		
		window.input.startFrame();
		window.input.debugPrint();

		if (!window.minimized &&
			window.surface_width && window.surface_height)
		{
			checkErrStack1(window._updateCPU_Data());
			checkErrStack1(window._render());
		}

		window.input.endFrame();
	}
	
	return err_stack;
}

Instance::~Instance()
{
	windows.clear();
}
