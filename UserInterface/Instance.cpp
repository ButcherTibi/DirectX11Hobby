
// Header
#include "NuiLibrary.hpp"

// Mine
#include "FilePath.hpp"


using namespace nui;


std::set<Window*> nui::_created_windows;


void Instance::create()
{
	hinstance = GetModuleHandleA(NULL);
	arrow_hcursor = LoadCursorA(NULL, IDC_ARROW);

	// Raw Device Input for Mouse
	{	
		RAWINPUTDEVICE raw_input_dev;
		raw_input_dev.usUsagePage = 0x01;
		raw_input_dev.usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
		raw_input_dev.dwFlags = 0;  // NOTE: RIDEV_NOLEGACY where legacy means regular input like WM_KEYDOWN or WM_LBUTTONDOWN
		raw_input_dev.hwndTarget = 0;

		if (!RegisterRawInputDevices(&raw_input_dev, 1, sizeof(RAWINPUTDEVICE))) {
			throw WindowsException("failed to register mouse raw input device");
		};
	}

	// Factory
	throwDX11(CreateDXGIFactory2(0, IID_PPV_ARGS(factory2.GetAddressOf())),
		"failed to create DXGI factory 2");

	throwDX11(factory2->QueryInterface<IDXGIFactory7>(factory7.GetAddressOf()),
		"failed to create DXGI factory 7");

	// Choose Adapter with most VRAM
	{
		uint32_t adapter_idx = 0;
		IDXGIAdapter* found_adapter;
		size_t adapter_vram = 0;

		while (SUCCEEDED(factory7.Get()->EnumAdapters(adapter_idx, &found_adapter))) {

			DXGI_ADAPTER_DESC desc;
			throwDX11(found_adapter->GetDesc(&desc), "failed to get adapter description");

			if (desc.DedicatedVideoMemory > adapter_vram) {
				adapter_vram = desc.DedicatedVideoMemory;
				adapter = found_adapter;
			}
			adapter_idx++;
		}
	}

	// Device and Context
	{
		uint32_t create_device_flag;
#ifdef _DEBUG
		create_device_flag = D3D11_CREATE_DEVICE_DEBUG;
#else
		create_device_flag = 0;
#endif

		std::array<D3D_FEATURE_LEVEL, 1> features = {
			D3D_FEATURE_LEVEL_11_1
		};

		throwDX11(D3D11CreateDevice(adapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN,  // driver type chosen by adapter
			NULL,  // no software adapter
			create_device_flag,
			features.data(), (uint32_t)features.size(),
			D3D11_SDK_VERSION,
			dev1.GetAddressOf(),
			NULL,  // dont care feature level
			im_ctx1.GetAddressOf()),
			"failed to create device");

		throwDX11(dev1->QueryInterface<ID3D11Device5>(dev5.GetAddressOf()),
			"failed to obtain ID3D11Device5");

		throwDX11(im_ctx1->QueryInterface<ID3D11DeviceContext3>(im_ctx3.GetAddressOf()),
			"failed to obtain immediate ID3D11DeviceContext3");
		
		/*throwDX11(dev5->CreateDeferredContext3(0, de_ctx3.GetAddressOf()),
			"failed to create deferred ID3D11DeviceContext3");*/
	}

	// Shaders
	{
		std::vector<char> read_buffer;

		dx11::createVertexShaderFromPath("UserInterface/CompiledShaders/CharVS.cso",
			dev5.Get(), char_vs.GetAddressOf(), &char_vs_cso);

		dx11::createPixelShaderFromPath("UserInterface/CompiledShaders/CharPS.cso",
			dev5.Get(), char_ps.GetAddressOf(), &read_buffer);

		dx11::createVertexShaderFromPath("UserInterface/CompiledShaders/RectVS.cso",
			dev5.Get(), rect_vs.GetAddressOf(), &rect_vs_cso);

		dx11::createPixelShaderFromPath("UserInterface/CompiledShaders/RectFlatFillPS.cso",
			dev5.Get(), rect_flat_fill_ps.GetAddressOf(), &read_buffer);

		dx11::createPixelShaderFromPath("UserInterface/CompiledShaders/RectLinearGradient.cso",
			dev5.Get(), rect_gradient_linear_ps.GetAddressOf(), &read_buffer);
	}

	// Character Vertex Input Layout
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> input_elems;
		
		auto vertex_elems = GPU_CharacterVertex::getInputLayout();
		for (auto& elem : vertex_elems) {
			input_elems.push_back(elem);
		}
		
		auto instance_elems = GPU_TextInstance::getInputLayout(1);
		for (auto& elem : instance_elems) {
			input_elems.push_back(elem);
		}
		
		throwDX11(dev5->CreateInputLayout(input_elems.data(), input_elems.size(),
			char_vs_cso.data(), char_vs_cso.size(), char_input_layout.GetAddressOf()));
	}

	// Rectangle Vertex Input Layout
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> input_elems;

		auto vertex_elems = GPU_RectVertex::getInputLayout();
		for (auto& elem : vertex_elems) {
			input_elems.push_back(elem);
		}

		/*auto instance_elems = GPU_RectInstance::getInputLayout(1);
		for (auto& elem : instance_elems) {
			input_elems.push_back(elem);
		}*/

		throwDX11(dev5->CreateInputLayout(input_elems.data(), input_elems.size(),
			rect_vs_cso.data(), rect_vs_cso.size(), rect_input_layout.GetAddressOf()));
	}

	// Character Atlas
	{
		Font* font;
		char_atlas.addFont("UserInterface/Fonts/Roboto-Regular.ttf", font);

		// Texture
		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = 0;
			desc.Height = 0;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;

			char_atlas_tex.create(dev5.Get(), im_ctx3.Get(), desc);
		}

		// SRV
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Format = DXGI_FORMAT_R8_UNORM;
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MostDetailedMip = 0;
			desc.Texture2D.MipLevels = 1;

			char_atlas_tex.createShaderResourceView(desc);
		}
		
		// Sampler
		{
			D3D11_SAMPLER_DESC desc = {};
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
			desc.MipLODBias = 0;
			desc.MaxAnisotropy = 1;
			desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			desc.BorderColor[0] = 1;
			desc.BorderColor[1] = 0;
			desc.BorderColor[2] = 1;
			desc.BorderColor[3] = 1;
			desc.MinLOD = 0;
			desc.MaxLOD = 0;

			throwDX11(dev5->CreateSamplerState(&desc, char_atlas_sampler.GetAddressOf()));
		}
	}

	// Rasterizer State
	{
		D3D11_RASTERIZER_DESC desc = {};
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		desc.FrontCounterClockwise = false;
		desc.DepthBias = 0;
		desc.DepthBiasClamp = 0;
		desc.SlopeScaledDepthBias = 0;
		desc.DepthClipEnable = false;
		desc.ScissorEnable = false;
		desc.MultisampleEnable = false;
		desc.AntialiasedLineEnable = false;
		
		throwDX11(dev5->CreateRasterizerState(&desc, solid_back_rs.GetAddressOf()));
	}

	// Blend State
	{
		D3D11_BLEND_DESC desc = {};
		desc.AlphaToCoverageEnable = false;
		desc.IndependentBlendEnable = true;

		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		throwDX11(dev5->CreateBlendState(&desc, blend_state.GetAddressOf()));
	}
}

void Instance::loadCharacterAtlasToTexture()
{
	TextureAtlas& atlas = char_atlas.atlas;

	if (atlas.tex_size) {
		char_atlas_tex.load(atlas.colors.data(), atlas.tex_size, atlas.tex_size);
	}
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

	for (Window* w : _created_windows) {
		if (w->hwnd == hwnd) {

			switch (uMsg) {

			case WM_SIZE: {

				switch (wParam) {
				case SIZE_MAXIMIZED:
				case SIZE_RESTORED: {
					w->win_messages.is_minimized = false;

					w->width = getLowOrder((uint32_t)lParam);
					w->height = getHighOrder((uint32_t)lParam);

					RECT client_rect;
					GetClientRect(hwnd, &client_rect);

					w->surface_width = (client_rect.right - client_rect.left);
					w->surface_height = (client_rect.bottom - client_rect.top);

					w->_updateCPU();
					w->_render();
					break;
				}

				case SIZE_MINIMIZED: {
					w->win_messages.is_minimized = true;
					break;
				}
				}

				return 0;
			}

			case WM_MOUSEMOVE: {
				w->input.mouse_x = getLowOrder((uint32_t)lParam);
				w->input.mouse_y = getHighOrder((uint32_t)lParam);
				return 0;
			}

			case WM_MOUSEWHEEL: {
				w->input.mouse_wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam);
				return 0;
			}

			case WM_KEYDOWN: {
				w->input.setKeyDownState((uint32_t)wParam, (uint32_t)lParam);
				return 0;
			}

			case WM_KEYUP: {
				w->input.setKeyUpState((uint32_t)wParam);
				return 0;
			}

			case WM_LBUTTONDOWN: {
				w->input.setKeyDownState(VirtualKeys::LEFT_MOUSE_BUTTON, 0);
				return 0;
			}

			case WM_LBUTTONUP: {
				w->input.setKeyUpState(VirtualKeys::LEFT_MOUSE_BUTTON);
				return 0;
			}

			case WM_RBUTTONDOWN: {
				w->input.setKeyDownState(VirtualKeys::RIGHT_MOUSE_BUTTON, 0);
				return 0;
			}

			case WM_RBUTTONUP: {
				w->input.setKeyUpState(VirtualKeys::RIGHT_MOUSE_BUTTON);
				return 0;
			}

			case WM_MBUTTONDOWN: {
				w->input.setKeyDownState(VirtualKeys::MIDDLE_MOUSE_BUTTON, 0);
				return 0;
			}

			case WM_MBUTTONUP: {
				w->input.setKeyUpState(VirtualKeys::MIDDLE_MOUSE_BUTTON);
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
				w->input.mouse_delta_x = raw->data.mouse.lLastX;
				w->input.mouse_delta_y = raw->data.mouse.lLastY;

				// printf("mouse delta = %d %d \n", wnd->input.mouse_delta_x, wnd->input.mouse_delta_y);
				return 0;
			}

			case WM_QUIT:
			case WM_CLOSE: {
				w->win_messages.should_close = true;
				return 0;
			}

			case WM_DESTROY: {
				// emergency exit do not save progress
				std::abort();
			}
			}
		}
	}
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

bool Instance::_bruteForceCreateSwapchain(Window& w, ComPtr<IDXGISwapChain1>& swapchain1)
{
	std::array<DXGI_FORMAT, 4> formats = {
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_B8G8R8A8_UNORM
	};

	std::array<uint32_t, 3> buffer_usages = {
		DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_DISCARD_ON_PRESENT,
		DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT,
		DXGI_USAGE_RENDER_TARGET_OUTPUT
	};

	std::array<DXGI_SCALING, 3> scalings = {
		DXGI_SCALING_NONE,
		DXGI_SCALING_ASPECT_RATIO_STRETCH,
		DXGI_SCALING_STRETCH,
	};

	std::array<DXGI_SWAP_EFFECT, 2> swap_effects = {
		DXGI_SWAP_EFFECT_DISCARD,
		DXGI_SWAP_EFFECT_FLIP_DISCARD,  // as expect the newer it is the slower it is
	};

	std::array<DXGI_ALPHA_MODE, 2> alpha_modes = {
		DXGI_ALPHA_MODE_IGNORE,
		DXGI_ALPHA_MODE_STRAIGHT,
		// DXGI_ALPHA_MODE_UNSPECIFIED,
		// DXGI_ALPHA_MODE_PREMULTIPLIED,
	};

	for (DXGI_FORMAT format : formats) {

		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.Format = format;
		desc.SampleDesc.Quality = 0;
		desc.SampleDesc.Count = 1;
		// desc.BufferUsage
		desc.BufferCount = 2;
		// desc.Scaling
		// desc.SwapEffect;
		// desc.AlphaMode;
		desc.Flags = 0;

		for (DXGI_ALPHA_MODE alpha_mode : alpha_modes) {
			desc.AlphaMode = alpha_mode;

			for (DXGI_SWAP_EFFECT swap_effect : swap_effects) {
				desc.SwapEffect = swap_effect;

				for (uint32_t buffer_usage : buffer_usages) {
					desc.BufferUsage = buffer_usage;

					for (DXGI_SCALING scaling : scalings) {
						desc.Scaling = scaling;

						HRESULT hr = factory7->CreateSwapChainForHwnd(dev5.Get(), w.hwnd,
							&desc, nullptr, nullptr, swapchain1.GetAddressOf());
						if (hr == S_OK) {


							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

Window* Instance::createWindow(WindowCreateInfo& info)
{
	Window& w = this->windows.emplace_back();
	_created_windows.insert(&w);

	w.instance = this;

	// Create Window
	{
		const char CLASS_NAME[] = "Sample Window Class";
		w.window_class = {};
		w.window_class.lpfnWndProc = (WNDPROC)windowProc;
		w.window_class.cbClsExtra = 0;
		w.window_class.cbWndExtra = 0;
		w.window_class.hInstance = hinstance;
		w.window_class.hCursor = arrow_hcursor;
		w.window_class.lpszClassName = CLASS_NAME;

		if (!RegisterClassA(&w.window_class)) {
			throw std::exception("failed to register window class");
		}

		w.hwnd = CreateWindowExA(
			0,                  // Optional window styles
			CLASS_NAME,                     // Window class
			"Window",                       // Window text
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,            // Window style
			CW_USEDEFAULT, CW_USEDEFAULT, info.width, info.height, // Position and Size
			NULL,       // Parent window
			NULL,       // Menu
			hinstance,  // Instance handle
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

	// Initialize Input
	{
		Input& input = w.input;
		input.mouse_x = 0;
		input.mouse_y = 0;
		input.mouse_delta_x = 0;
		input.mouse_delta_y = 0;
		input.mouse_wheel_delta = 0;

		for (nui::KeyState key : input.key_list) {
			key.is_down = false;
			key.start_time = frame_start_time;
			key.end_time = key.start_time;
		}
	}

	// Window Size
	w.width = info.width;
	w.height = info.height;

	// Surface size
	{
		RECT client_rect;
		if (!GetClientRect(w.hwnd, &client_rect)) {
			throw std::exception("failed to retrieve window client rect");
		}
		w.surface_width = client_rect.right - client_rect.left;
		w.surface_height = client_rect.bottom - client_rect.top;
	}

	// Messages
	w.win_messages.is_minimized = false;
	w.win_messages.should_close= false;

	// Root Node
	{
		StoredElement& root_elem = w.elements.emplace_back();
		auto& root = root_elem.emplace<Root>();
		root._window = &w;
		root._parent = nullptr;
		root._self = &root_elem;
		root.Element::_init();
	}

	// Swapchain
	if (!_bruteForceCreateSwapchain(w, w.swapchain1)) {
		throw std::exception("failed to create swapchain1 for hwnd");
	}

	// Present Target View
	{
		throwDX11(w.swapchain1->GetBuffer(0, IID_PPV_ARGS(w.present_img.GetAddressOf())),
			"failed to get swapchain back buffer");

		throwDX11(dev5->CreateRenderTargetView(w.present_img.Get(), NULL, w.present_rtv.GetAddressOf()),
			"failed to create present RTV");

		// Paint it Black . . . 
		float clear_color[4] = { 0, 0, 0, 0 };
		im_ctx3->ClearRenderTargetView(w.present_rtv.Get(), clear_color);

		throwDX11(w.swapchain1->Present(0, 0));
	}

	// Character Vertex Buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		w.char_vbuff.create(dev5.Get(), im_ctx3.Get(), desc);
	}

	// Character Index Buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		w.char_idxbuff.create(dev5.Get(), im_ctx3.Get(), desc);
	}

	// Text Instance Buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		w.text_instabuff.create(dev5.Get(), im_ctx3.Get(), desc);
	}

	// Rect Vertex Buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		w.rect_vbuff.create(dev5.Get(), im_ctx3.Get(), desc);
	}

	// Rect Index Buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		w.rect_idxbuff.create(dev5.Get(), im_ctx3.Get(), desc);
	}

	// Rect Drawcall Buffer
	{
		w.rect_dbuff.create(dev5.Get(), im_ctx3.Get(), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		w.rect_dbuff.addFloat4();
		w.rect_dbuff.addFloat4Array(8);
		w.rect_dbuff.addFloat4Array(8);	
		w.rect_dbuff.addFloat();
	}

	// Constant Buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		w.cbuff.create(dev5.Get(), im_ctx3.Get(), desc);
	}

	return &w;
}

void Instance::update()
{
	frame_start_time = std::chrono::steady_clock::now();
	delta_time = fsec_cast(frame_start_time - frame_end_time);

	// Input
	for (Window& window : windows) {	

		// Reset Input
		Input& input = window.input;
		{
			input.mouse_delta_x = 0;
			input.mouse_delta_y = 0;
			input.mouse_wheel_delta = 0;

			for (uint16_t virtual_key = 0; virtual_key < input.key_list.size(); virtual_key++) {

				KeyState& key = input.key_list[virtual_key];
				key.down_transition = false;
				key.up_transition = false;
			}
		}

		// Read Input
		MSG msg{};
		while (PeekMessageA(&msg, window.hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		// calculate time
		for (uint16_t virtual_key = 0; virtual_key < input.key_list.size(); virtual_key++) {

			KeyState& key = input.key_list[virtual_key];

			if (key.is_down) {
				key.end_time = frame_start_time;
			}
			else {
				key.end_time = key.start_time;
			}
		}

		// Update
		window._updateCPU();
		window._render();
	}

	// active frame time ended
	// TODO: sleep for remainder, try thread sleep

	frame_end_time = std::chrono::steady_clock::now();
}
