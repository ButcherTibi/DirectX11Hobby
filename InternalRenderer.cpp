
// Header
#include "Internals.h"


using namespace nui_int;
using namespace nui;


ErrStack Internals::create(HWND hwnd)
{
	ErrStack err_stack{};
	HRESULT hr{};

	DX11Renderer& dxr = dx11_renderer;

	// Device, Swapchain
	{
		std::array<D3D_FEATURE_LEVEL, 1> feature_lvl = {
			D3D_FEATURE_LEVEL_11_1
		};

		DXGI_MODE_DESC buff_desc = {};
		buff_desc.RefreshRate.Numerator = 60;
		buff_desc.RefreshRate.Denominator = 1;
		buff_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
		swapchain_desc.BufferDesc = buff_desc;
		swapchain_desc.SampleDesc.Count = 1;
		swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchain_desc.BufferCount = 1;
		swapchain_desc.OutputWindow = hwnd;
		swapchain_desc.Windowed = true;
		swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		checkHResult(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
			D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG,
			feature_lvl.data(), feature_lvl.size(), D3D11_SDK_VERSION,
			&swapchain_desc,
			dxr.swapchain.GetAddressOf(),
			dxr.device.GetAddressOf(), NULL,
			dxr.imediate_ctx.GetAddressOf()),
			"failed to create device and swapchain");

		
		checkHResult(dxr.device->QueryInterface(__uuidof (dxr.dx11_debug), reinterpret_cast<void**> (dxr.dx11_debug.GetAddressOf())),
			"failed to get debug interface");

	}

	// Defered Context
	{
		checkHResult(dxr.device->CreateDeferredContext(0, dxr.deferred_ctx.GetAddressOf()),
			"failed to create deferred context");
	}

	// Render Targets
	{
		ComPtr<ID3D11Texture2D> back_buff;
		checkHResult(dxr.swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)back_buff.GetAddressOf()),
			"failed to get swapchain's back buffer image");

		DXGI_SWAP_CHAIN_DESC swapchain_desc;
		dxr.swapchain->GetDesc(&swapchain_desc);

		dxr.width = swapchain_desc.BufferDesc.Width;
		dxr.height = swapchain_desc.BufferDesc.Height;
		checkHResult(dxr.device->CreateRenderTargetView(back_buff.Get(), NULL, &dxr.swapchain_view),
			"failed to create for back buffer render target");
	}

	// Create Root Element
	{
		float width = (float)dxr.width;
		float height = (float)dxr.height;

		Flex elem = {};
		elem.width.setAbsolute(width);
		elem.height.setAbsolute(height);
		elem.background_color = { 0, 0, 0, 1 };
		elem.border_color = { 0, 0, 0, 1 };

		elem._contentbox_width = width;
		elem._contentbox_height = height;

		elem._paddingbox_width = width;
		elem._paddingbox_height = height;

		elem._borderbox_width = width;
		elem._borderbox_height = height;

		// Assign to node
		Element& new_root = user_interface.elems.emplace_back();
		new_root.parent = nullptr;
		new_root.elem = elem;
	}

	//  Buffers
	{
		BufferCreateInfo info = {};
		info.usage = D3D11_USAGE_DYNAMIC;
		info.bind_flags = D3D11_BIND_VERTEX_BUFFER;
		info.cpu_access_flags = D3D11_CPU_ACCESS_WRITE;
		dxr.vertex_buff.create(dxr.device.Get(), dxr.imediate_ctx.Get(), info);

		info = {};
		info.usage = D3D11_USAGE_DYNAMIC;
		info.bind_flags = D3D11_BIND_SHADER_RESOURCE;
		info.cpu_access_flags = D3D11_CPU_ACCESS_WRITE;
		info.misc_flags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		dxr.rect_props_buff.create(dxr.device.Get(), dxr.imediate_ctx.Get(), info);

		info = {};
		info.usage = D3D11_USAGE_DYNAMIC;
		info.bind_flags = D3D11_BIND_SHADER_RESOURCE;
		info.cpu_access_flags = D3D11_CPU_ACCESS_WRITE;
		info.misc_flags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		dxr.circle_props_buff.create(dxr.device.Get(), dxr.imediate_ctx.Get(), info);

		info = {};
		info.usage = D3D11_USAGE_DYNAMIC;
		info.bind_flags = D3D11_BIND_SHADER_RESOURCE;
		info.cpu_access_flags = D3D11_CPU_ACCESS_WRITE;
		info.misc_flags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		dxr.common_buff.create(dxr.device.Get(), dxr.imediate_ctx.Get(), info);
	}

	// Shaders and Input Layout
	{
		checkErrStack(dxr.rect_VS.create(dxr.device.Get(), "/x64/Debug/RectangleVS.cso"),
			"failed to create vertex shader");

		std::vector<D3D11_INPUT_ELEMENT_DESC> layouts{ 2 };
		layouts[0] = {};
		layouts[0].SemanticName = "POSITION";
		layouts[0].SemanticIndex = 0;
		layouts[0].Format = DXGI_FORMAT_R32G32_FLOAT;
		layouts[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		layouts[1] = {};
		layouts[1].SemanticName = "IDX";
		layouts[1].SemanticIndex = 0;
		layouts[1].Format = DXGI_FORMAT_R32_UINT;
		layouts[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		layouts[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		checkHResult(dxr.device->CreateInputLayout(layouts.data(), layouts.size(),
			dxr.rect_VS.code.data(), dxr.rect_VS.code.size(),
			dxr.vertex_input.GetAddressOf()),
			"failed to create rects input layout");
	}

	// Rasterizer
	{
		D3D11_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D11_CULL_BACK;
		desc.FillMode = D3D11_FILL_SOLID;

		checkHResult(dxr.device->CreateRasterizerState(&desc, dxr.rasterizer_state.GetAddressOf()),
			"failed to create rasterizer state");
	}

	// Pixel Shaders
	checkErrStack(dxr.rect_PS.create(dxr.device.Get(), "/x64/Debug/RectanglePS.cso"),
		"failed to create rectangle shader");
	checkErrStack(dxr.circle_PS.create(dxr.device.Get(), "/x64/Debug/CirclePS.cso"),
		"failed to create circle shader");

	return err_stack;
}

ErrStack Internals::draw()
{
	ErrStack err_stack{};
	HRESULT hr{};

	DX11Renderer& dxr = dx11_renderer;
	UserInterface& ui = user_interface;

	ui.calcGraphLayout();
	checkErrStack1(generateGPU_Data());

	{
		// Vertex Buffer
		bool vertex_buff_redone;
		checkErrStack(dxr.vertex_buff.fill(dxr.vertices, vertex_buff_redone),
			"failed to fill vertex buffer");

		if (vertex_buff_redone) {
			std::vector<uint32_t> strides = {
			dxr.vertex_buff.stride
			};
			std::vector<uint32_t> offsets = {
				0
			};
			dxr.deferred_ctx->IASetVertexBuffers(0, 1, dxr.vertex_buff.buff.GetAddressOf(), strides.data(), offsets.data());
		}

		// Common Buffer
		bool common_buff_redone;
		checkErrStack(dxr.common_buff.fill(dxr.common_stuff, common_buff_redone),
			"failed to fill common stuff buffer");

		if (common_buff_redone) {

			if (dxr.common_buff_view != nullptr) {
				dxr.common_buff_view->Release();
			}

			checkHResult(dxr.device->CreateShaderResourceView(dxr.common_buff.buff.Get(), NULL, dxr.common_buff_view.GetAddressOf()),
				"failed to create shader resource view for common_stuff buffer");

			dxr.deferred_ctx->VSSetShaderResources(0, 1, dxr.common_buff_view.GetAddressOf());
		}

		// Rect Props Buffer
		bool rect_buff_redone;
		checkErrStack(dxr.rect_props_buff.fill(dxr.rect_props, rect_buff_redone),
			"failed to fill rect buffer");

		if (rect_buff_redone) {

			if (dxr.rect_props_view != nullptr) {
				dxr.rect_props_view->Release();
			}

			checkHResult(dxr.device->CreateShaderResourceView(dxr.rect_props_buff.buff.Get(), NULL, dxr.rect_props_view.GetAddressOf()),
				"failed to create shader resource view for rect props buffer");
		}

		// Circle Props Buffer
		bool circle_buff_redone;
		checkErrStack(dxr.circle_props_buff.fill(dxr.circle_props, circle_buff_redone),
			"failed to create circle buffer");

		if (circle_buff_redone) {

			if (dxr.circle_props_view != nullptr) {
				dxr.circle_props_view->Release();
			}

			checkHResult(dxr.device->CreateShaderResourceView(dxr.circle_props_buff.buff.Get(), NULL, dxr.circle_props_view.GetAddressOf()),
				"failed to create shader resource view for circle props buffer");
		}
	}

	float bg_color[] = { 0, 0, 0, 0 };
	dxr.deferred_ctx->ClearRenderTargetView(dxr.swapchain_view.Get(), bg_color);


	dxr.deferred_ctx->IASetInputLayout(dxr.vertex_input.Get());
	dxr.deferred_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxr.deferred_ctx->VSSetShader(dxr.rect_VS.shader.Get(), NULL, 0);

	// Rasterizer
	{
		D3D11_VIEWPORT viewport = {};
		viewport.Width = (float)dxr.width;
		viewport.Height = (float)dxr.height;
		viewport.MaxDepth = 1.0f;
		dxr.deferred_ctx->RSSetViewports(1, &viewport);

		dxr.deferred_ctx->RSSetState(dxr.rasterizer_state.Get());
	}

	// Output Merger
	{
		std::vector<ID3D11RenderTargetView*> render_targets = {
			dxr.swapchain_view.Get()
		};
		dxr.deferred_ctx->OMSetRenderTargets(render_targets.size(), render_targets.data(), NULL);

	}

	for (ElementRendering& elem_render : dxr.elements) {

		// Padding Rect Pass
		dxr.deferred_ctx->PSSetShaderResources(0, 1, dxr.rect_props_view.GetAddressOf());
		dxr.deferred_ctx->PSSetShader(dxr.rect_PS.shader.Get(), NULL, 0);
		dxr.deferred_ctx->Draw(elem_render.padding_rect.count, elem_render.padding_rect.start);
		printf("draw \n");

		// Padding Circle Passes
		dxr.deferred_ctx->PSSetShaderResources(0, 1, dxr.circle_props_view.GetAddressOf());
		dxr.deferred_ctx->PSSetShader(dxr.circle_PS.shader.Get(), NULL, 0);

		if (elem_render.padding_tl_circle.count) {
			dxr.deferred_ctx->Draw(elem_render.padding_tl_circle.count, elem_render.padding_tl_circle.start);
			printf("draw \n");
		}
	}

	printf("finish \n");
	checkHResult(dxr.deferred_ctx->FinishCommandList(true, dxr.command_list.GetAddressOf()),
		"failed to finish command list");

	dxr.imediate_ctx->ExecuteCommandList(dxr.command_list.Get(), true);

	checkHResult(dxr.swapchain->Present(1, NULL),
		"failed to present");

	dxr.dx11_debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

	return err_stack;
}


ErrStack Internals::generateGPU_Data()
{
	ErrStack err_stack{};

	DX11Renderer& dxr = dx11_renderer;
	UserInterface& ui = user_interface;

	// Common Stuff
	{
		dxr.common_stuff.width = dxr.width;
		dxr.common_stuff.height = dxr.height;
	}

	auto createChamferedRectangle = [](xm_float2 origin, float width, float height,
		float tl_radius, float tr_radius, float br_radius, float bl_radius,
		std::vector<GPU_Vertex>& rect_verts, size_t idx)
	{
		xm_float2 vec2_origin{ origin.x, origin.y };

		xm_float2 top_left_up = vec2_origin;
		xm_float2 top_left_down = vec2_origin;

		xm_float2 top_right_up = vec2_origin;
		xm_float2 top_right_down = vec2_origin;

		xm_float2 bot_right_up = vec2_origin;
		xm_float2 bot_right_down = vec2_origin;

		xm_float2 bot_left_up = vec2_origin;
		xm_float2 bot_left_down = vec2_origin;

		// Box positions
		top_left_up.x += tl_radius;
		top_left_down.y += tl_radius;

		top_right_up.x += width - tr_radius;
		top_right_down.x += width;
		top_right_down.y += tr_radius;

		bot_right_up.x += width;
		bot_right_up.y += height - br_radius;
		bot_right_down.x += width - br_radius;
		bot_right_down.y += height;

		bot_left_up.y += height - bl_radius;
		bot_left_down.x += bl_radius;
		bot_left_down.y += height;

		// Box triangles
		rect_verts[idx + 0].pos = top_left_up;
		rect_verts[idx + 1].pos = top_right_up;
		rect_verts[idx + 2].pos = top_right_down;

		rect_verts[idx + 3].pos = top_left_up;
		rect_verts[idx + 4].pos = top_right_down;
		rect_verts[idx + 5].pos = bot_right_up;

		rect_verts[idx + 6].pos = top_left_up;
		rect_verts[idx + 7].pos = bot_right_up;
		rect_verts[idx + 8].pos = bot_right_down;

		rect_verts[idx + 9].pos = top_left_up;
		rect_verts[idx + 10].pos = bot_right_down;
		rect_verts[idx + 11].pos = bot_left_down;

		rect_verts[idx + 12].pos = top_left_up;
		rect_verts[idx + 13].pos = bot_left_down;
		rect_verts[idx + 14].pos = bot_left_up;

		rect_verts[idx + 15].pos = top_left_up;
		rect_verts[idx + 16].pos = bot_left_up;
		rect_verts[idx + 17].pos = top_left_down;
	};

	auto createTopLeftCircle = [](xm_float2 origin, float radius,
		std::vector<GPU_Vertex>& verts, size_t idx)
	{
		xm_float2 vec2_origin{ origin.x, origin.y };

		xm_float2 top_left = vec2_origin;
		xm_float2 top_right = vec2_origin;
		xm_float2 bot_left = vec2_origin;

		top_right.x += radius;
		bot_left.y += radius;

		// Position
		verts[idx + 0].pos = top_left;
		verts[idx + 1].pos = top_right;
		verts[idx + 2].pos = bot_left;
	};

	//auto createTopRightCircle = [](glm::vec2 origin, float radius,
	//	std::vector<GPU_Circles_Vertex>& verts, size_t idx)
	//{
	//	glm::vec2 vec2_origin{ origin.x, origin.y };

	//	glm::vec2 top_left = vec2_origin;
	//	glm::vec2 top_right = vec2_origin;
	//	glm::vec2 bot_right = vec2_origin;

	//	top_right.x += radius;
	//	bot_right += radius;

	//	// Position
	//	verts[idx + 0].pos = top_left;
	//	verts[idx + 1].pos = top_right;
	//	verts[idx + 2].pos = bot_right;

	//	// Center and Radius
	//	glm::vec2 center = vec2_origin;
	//	center.y += radius;

	//	for (auto i = idx; i < idx + 3; i++) {
	//		verts[i].center = center;
	//		verts[i].radius = radius;
	//	}
	//};

	//auto createBotRightCircle = [](glm::vec2 origin, float radius,
	//	std::vector<GPU_Circles_Vertex>& verts, size_t idx)
	//{
	//	glm::vec2 vec2_origin{ origin.x, origin.y };

	//	glm::vec2 top_right = vec2_origin;
	//	glm::vec2 bot_right = vec2_origin;
	//	glm::vec2 bot_left = vec2_origin;

	//	top_right.x += radius;
	//	bot_right += radius;
	//	bot_left.y += radius;

	//	// Position
	//	verts[idx + 0].pos = top_right;
	//	verts[idx + 1].pos = bot_right;
	//	verts[idx + 2].pos = bot_left;

	//	// Center and Radius
	//	glm::vec2 center = vec2_origin;

	//	for (auto i = idx; i < idx + 3; i++) {
	//		verts[i].center = center;
	//		verts[i].radius = radius;
	//	}
	//};

	//auto createBotLeft = [](glm::vec2 origin, float radius,
	//	std::vector<GPU_Circles_Vertex>& verts, size_t idx)
	//{
	//	glm::vec2 vec2_origin{ origin.x, origin.y };

	//	glm::vec2 top_left = vec2_origin;
	//	glm::vec2 bot_right = vec2_origin;
	//	glm::vec2 bot_left = vec2_origin;

	//	bot_right += radius;
	//	bot_left.y += radius;

	//	// Position
	//	verts[idx + 0].pos = top_left;
	//	verts[idx + 1].pos = bot_right;
	//	verts[idx + 2].pos = bot_left;

	//	// Center and Radius
	//	glm::vec2 center = vec2_origin;
	//	center.x += radius;

	//	for (auto i = idx; i < idx + 3; i++) {
	//		verts[i].center = center;
	//		verts[i].radius = radius;
	//	}
	//};

	dxr.vertices.clear();
	dxr.rect_props.clear();
	dxr.circle_props.clear();
	dxr.elements.clear();

	auto ui_it = ui.layers.begin();
	uint32_t vertex_count = 0;

	for (auto l = 0; l < ui.layers.size(); l++) {

		ElementsLayer& ui_layer = *ui_it;

		for (Element* elem : ui_layer.elems) {

			Flex* flex;
			Paragraph* par;
			BoxModel* box;

			switch (elem->elem.index()) {
			case 0: {
				flex = std::get_if<Flex>(&elem->elem);
				box = flex;
				break;
			}

			case 1: {
				par = std::get_if<Paragraph>(&elem->elem);
				box = par;
				break;
			}

			default:
				printf(code_location);
			}

			ElementRendering& elem_rendering = dxr.elements.emplace_back();

			xm_float2 border_origin = { box->_origin.x, box->_origin.y };

			xm_float2 padding_origin = border_origin;

			// Padding Box
			{
				size_t last_idx = dxr.vertices.size();
				dxr.vertices.resize(dxr.vertices.size() + 18);

				padding_origin.x += box->_border_left_thick;
				padding_origin.y += box->_border_top_thick;

				createChamferedRectangle(padding_origin, box->_paddingbox_width, box->_paddingbox_height,
					box->_padding_tl_radius, box->_padding_tr_radius, box->_padding_br_radius, box->_padding_bl_radius,
					dxr.vertices, last_idx);

				// Vertex Props
				for (auto i = last_idx; i < dxr.vertices.size(); i++) {
					dxr.vertices[i].idx = dxr.rect_props.size();
				}

				GPU_RectProps& props = dxr.rect_props.emplace_back();
				props.color = {
					box->background_color.r,
					box->background_color.g,
					box->background_color.b,
					box->background_color.a
				};

				// Draw Props
				elem_rendering.padding_rect.start = vertex_count;
				elem_rendering.padding_rect.count = 18;
				vertex_count += 18;
			}

			// Padding Top Left Circle
			if (box->padding_tl_radius) {

				size_t last_idx = dxr.vertices.size();
				dxr.vertices.resize(dxr.vertices.size() + 3);

				createTopLeftCircle(padding_origin, box->_padding_tl_radius,
					dxr.vertices, last_idx);

				// Vertex Props
				for (auto i = last_idx; i < dxr.vertices.size(); i++) {
					dxr.vertices[i].idx = dxr.circle_props.size();
				}

				GPU_CircleProps& props = dxr.circle_props.emplace_back();
				props.center_radius.x = padding_origin.x + box->padding_tl_radius;
				props.center_radius.y = padding_origin.y + box->padding_tl_radius;
				props.center_radius.z = box->padding_tl_radius;

				// Draw Props
				elem_rendering.padding_tl_circle.start = vertex_count;
				elem_rendering.padding_tl_circle.count = 3;
				vertex_count += 3;
			}
		}

		ui_it++;
	}

	return err_stack;
}
