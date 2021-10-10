module;

// Windows
#undef RELATIVE
#undef ABSOLUTE

// DirectX 11
#include "DX11Wrapper.hpp"

// GLM
#include "glm\vec2.hpp"

module NuiLibrary;


using namespace nui;


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
	}

	// Shaders
	{
		std::vector<char> read_buffer;

		// Simple
		dx11::createVertexShaderFromPath("UserInterface/CompiledShaders/SimpleVS.cso",
			dev5.Get(), simple_vs.GetAddressOf(), &read_buffer);

		// Gradient
		/*dx11::createPixelShaderFromPath("UserInterface/CompiledShaders/RectFlatFillPS.cso",
			dev5.Get(), rect_flat_fill_ps.GetAddressOf(), &read_buffer);

		dx11::createPixelShaderFromPath("UserInterface/CompiledShaders/RectLinearGradient.cso",
			dev5.Get(), rect_gradient_linear_ps.GetAddressOf(), &read_buffer);*/
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

	// Text Renderer
	{
		// Vertex
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

			text_renderer.vbuff.create(dev5.Get(), im_ctx3.Get(), desc);
		}

		// Index
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

			text_renderer.idxbuff.create(dev5.Get(), im_ctx3.Get(), desc);
		}

		// Instance
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

			text_renderer.instances_buff.create(dev5.Get(), im_ctx3.Get(), desc);
		}

		// Character Atlas
		{
			Font* font;
			text_renderer.char_atlas.addFont("UserInterface/Fonts/Roboto-Regular.ttf", font);

			FontSize* font_size;
			text_renderer.char_atlas.addSizeToFont(font, 14, font_size);

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

				text_renderer.char_atlas_tex.create(dev5.Get(), im_ctx3.Get(), desc);
			}

			// SRV
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
				desc.Format = DXGI_FORMAT_R8_UNORM;
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MostDetailedMip = 0;
				desc.Texture2D.MipLevels = 1;

				text_renderer.char_atlas_tex.createShaderResourceView(desc);
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

				throwDX11(dev5->CreateSamplerState(&desc, text_renderer.char_atlas_sampler.GetAddressOf()));
			}
		}

		// Shaders
		{
			dx11::createVertexShaderFromPath("UserInterface/CompiledShaders/CharVS.cso",
				dev5.Get(), text_renderer.char_vs.GetAddressOf());

			dx11::createPixelShaderFromPath("UserInterface/CompiledShaders/CharPS.cso",
				dev5.Get(), text_renderer.char_ps.GetAddressOf());
		}
	}

	// Rect Renderer
	{
		// Vertex
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

			rect_renderer.vbuff.create(dev5.Get(), im_ctx3.Get(), desc);
		}

		// Index
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

			rect_renderer.idxbuff.create(dev5.Get(), im_ctx3.Get(), desc);
		}

		// Instance
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

			rect_renderer.instances_buff.create(dev5.Get(), im_ctx3.Get(), desc);
		}

		// Shaders
		{
			// Rect
			dx11::createPixelShaderFromPath("UserInterface/CompiledShaders/RectPS.cso",
				dev5.Get(), rect_renderer.rect_ps.GetAddressOf());
		}
	}

	// Circle
	{
		// Vertex
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

			circle_renderer.vbuff.create(dev5.Get(), im_ctx3.Get(), desc);
		}

		// Index
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

			circle_renderer.idxbuff.create(dev5.Get(), im_ctx3.Get(), desc);
		}

		// Instance
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

			circle_renderer.instances_buff.create(dev5.Get(), im_ctx3.Get(), desc);
		}

		// Shaders
		{
			// Circle
			dx11::createPixelShaderFromPath("UserInterface/CompiledShaders/CirclePS.cso",
				dev5.Get(), circle_renderer.circle_ps.GetAddressOf());
		}
	}
}

void Instance::findAndPositionGlyphs(TextProps& props,
	int32_t start_pos_x, int32_t start_pos_y,
	uint32_t& r_width, uint32_t& r_height,
	std::vector<PositionedCharacter>& r_chars)
{
	r_chars.resize(props.text.size());

	FontSize* font_size;
	text_renderer.char_atlas.ensureFontWithSize(props.font_family, props.font_style, props.font_size, font_size);

	r_width = 0;

	// if line height is unspecified use default from font file
	uint32_t line_height;
	if (props.line_height == 0xFFFF'FFFF) {
		line_height = font_size->ascender;
	}
	else {
		line_height = props.line_height;
	}

	glm::ivec2 pen = { start_pos_x, start_pos_y };
	pen.y += line_height;

	uint32_t i = 0;;
	for (uint32_t unicode : props.text) {

		switch (unicode) {
			// New Line
		case 0x000A: {
			pen.x = 0;  // Carriage Return
			pen.y += line_height;  // Line Feed
			break;
		}

		default: {
			Character* chara = font_size->findCharacter(unicode);
			r_chars[i].pos = { pen.x, pen.y };
			r_chars[i].chara = chara;

			// Move the writing pen to the next character
			pen.x += chara->advance_X;

			uint32_t new_width = pen.x - start_pos_x;

			if (new_width > r_width) {
				r_width = new_width;
			}
		}
		}

		i++;
	}

	r_height = (pen.y + font_size->descender) - start_pos_y;
}

void Instance::drawTexts(Window* window, ClipZone& clip_zone, std::vector<TextInstance*>& instances)
{
	if (instances.size() == 0) {
		return;
	}

	// Generate GPU Data
	{
		// Counting
		{
			uint32_t vertex_count = 0;
			uint32_t index_count = 0;

			for (TextInstance* inst : instances) {
				for (auto& positioned_char : inst->chars) {

					if (positioned_char.chara->zone != nullptr) {
						vertex_count += 4;
						index_count += 6;
					}
				}
			}

			text_renderer.verts.resize(vertex_count);
			text_renderer.indexes.resize(index_count);
			text_renderer.instances.resize(instances.size());
		}

		uint32_t vertex_idx = 0;
		uint32_t index_idx = 0;
		uint32_t instance_idx = 0;

		for (TextInstance* inst : instances) {
			for (auto& positioned_char : inst->chars) {

				Character* chara = positioned_char.chara;

				if (chara->zone != nullptr) {

					uint32_t bitmap_width = chara->zone->bb_pix.getWidth();
					uint32_t bitmap_height = chara->zone->bb_pix.getHeight();

					int32_t char_top = chara->bitmap_top;

					glm::ivec2 character_pos = { positioned_char.pos[0], positioned_char.pos[1] };
					character_pos.x += chara->bitmap_left;
					character_pos.y += bitmap_height - char_top;

					// Top Left
					glm::ivec2 pos = character_pos;
					pos.y -= bitmap_height;

					GPU_CharacterVertex* v = &text_renderer.verts[vertex_idx];
					v->pos = toXM(pos);
					v->uv.x = chara->zone->bb_uv.x0;
					v->uv.y = chara->zone->bb_uv.y0;
					v->instance_id = instance_idx;

					// Top Right
					pos = character_pos;
					pos.x += bitmap_width;
					pos.y -= bitmap_height;

					v = &text_renderer.verts[vertex_idx + 1];
					v->pos = toXM(pos);
					v->uv.x = chara->zone->bb_uv.x1;
					v->uv.y = chara->zone->bb_uv.y0;
					v->instance_id = instance_idx;

					// Bot Right
					pos = character_pos;
					pos.x += bitmap_width;

					v = &text_renderer.verts[vertex_idx + 2];
					v->pos = toXM(pos);
					v->uv.x = chara->zone->bb_uv.x1;
					v->uv.y = chara->zone->bb_uv.y1;
					v->instance_id = instance_idx;

					// Bot Left
					pos = character_pos;

					v = &text_renderer.verts[vertex_idx + 3];
					v->pos = toXM(pos);
					v->uv.x = chara->zone->bb_uv.x0;
					v->uv.y = chara->zone->bb_uv.y1;
					v->instance_id = instance_idx;

					// Tesselation 0 to 2
					text_renderer.indexes[index_idx + 0] = vertex_idx + 0;
					text_renderer.indexes[index_idx + 1] = vertex_idx + 1;
					text_renderer.indexes[index_idx + 2] = vertex_idx + 2;

					text_renderer.indexes[index_idx + 3] = vertex_idx + 2;
					text_renderer.indexes[index_idx + 4] = vertex_idx + 3;
					text_renderer.indexes[index_idx + 5] = vertex_idx + 0;

					vertex_idx += 4;
					index_idx += 6;
				}
			}

			// Text Instance
			GPU_TextInstance& tex_inst = text_renderer.instances[instance_idx];
			tex_inst.color = toXM(inst->color.rgba);

			instance_idx += 1;
		}
	}

	// Draw
	{
		text_renderer.vbuff.upload(text_renderer.verts);
		text_renderer.idxbuff.upload(text_renderer.indexes);
		text_renderer.instances_buff.upload(text_renderer.instances);

		// Input Assembly
		{
			im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			im_ctx3->IASetIndexBuffer(text_renderer.idxbuff.get(), DXGI_FORMAT_R32_UINT, 0);
		}

		// Vertex Shader
		{
			std::array<ID3D11ShaderResourceView*, 1> srvs = {
				text_renderer.vbuff.getSRV()
			};
			im_ctx3->VSSetShaderResources(0, srvs.size(), srvs.data());

			std::array<ID3D11Buffer*, 1> cbuffs = {
				window->cbuff.get()
			};
			im_ctx3->VSSetConstantBuffers(0, cbuffs.size(), cbuffs.data());

			im_ctx3->VSSetShader(text_renderer.char_vs.Get(), nullptr, 0);
		}

		// Rasterizer
		{
			D3D11_VIEWPORT viewport;
			viewport.TopLeftX = (float)clip_zone.pos[0];
			viewport.TopLeftY = (float)clip_zone.pos[1];
			viewport.Width = (float)clip_zone.size[0];
			viewport.Height = (float)clip_zone.size[1];
			viewport.MinDepth = window->viewport.MinDepth;
			viewport.MaxDepth = window->viewport.MaxDepth;

			im_ctx3->RSSetViewports(1, &viewport);
			im_ctx3->RSSetState(solid_back_rs.Get());
		}

		// Pixel Shader
		{
			std::array<ID3D11SamplerState*, 1> samplers = {
				text_renderer.char_atlas_sampler.Get()
			};
			im_ctx3->PSSetSamplers(0, samplers.size(), samplers.data());

			std::array<ID3D11ShaderResourceView*, 2> srvs = {
				text_renderer.char_atlas_tex.getSRV(), text_renderer.instances_buff.getSRV()
			};
			im_ctx3->PSSetShaderResources(0, srvs.size(), srvs.data());

			im_ctx3->PSSetShader(text_renderer.char_ps.Get(), nullptr, 0);
		}

		// Output Merger
		{
			std::array<float, 4> blend_factor = {
				0, 0, 0, 0
			};
			im_ctx3->OMSetBlendState(blend_state.Get(), blend_factor.data(), 0xFFFF'FFFF);

			std::array<ID3D11RenderTargetView*, 1> srv = {
				window->present_rtv.Get()
			};
			im_ctx3->OMSetRenderTargets(srv.size(), srv.data(), nullptr);
		}

		// Draw
		im_ctx3->DrawIndexed(text_renderer.indexes.size(),
			0, 0);
	}
}

void Instance::drawTexts(Window* window, std::vector<TextInstance*>& instances)
{
	ClipZone clip_zone;
	clip_zone.pos = { 0, 0 };
	clip_zone.size = { (uint32_t)window->viewport.Width, (uint32_t)window->viewport.Height };

	drawTexts(window, clip_zone, instances);
}

void Instance::_drawRects(Window* window, RectInstance** instances_data, size_t instances_count)
{
	// Generate GPU Data
	{
		rect_renderer.verts.resize(instances_count * 4);
		rect_renderer.indexes.resize(instances_count * 6);
		rect_renderer.instances.resize(instances_count);

		uint32_t vertex_idx = 0;
		uint32_t index_idx = 0;

		for (uint32_t i = 0; i < instances_count; i++) {

			RectInstance* inst = instances_data[i];

			// Vertices
			GPU_SimpleVertex& tl_v = rect_renderer.verts[vertex_idx + 0];
			GPU_SimpleVertex& tr_v = rect_renderer.verts[vertex_idx + 1];
			GPU_SimpleVertex& br_v = rect_renderer.verts[vertex_idx + 2];
			GPU_SimpleVertex& bl_v = rect_renderer.verts[vertex_idx + 3];

			glm::ivec2 pos = { inst->pos[0], inst->pos[1] };
			tl_v.pos = toXM(pos);
			tr_v.pos = toXM(pos.x + inst->size[0], pos.y);
			br_v.pos = toXM(pos.x + inst->size[0], pos.y + inst->size[1]);;
			bl_v.pos = toXM(pos.x, pos.y + inst->size[1]);

			tl_v.instance_id = i;
			tr_v.instance_id = i;
			br_v.instance_id = i;
			bl_v.instance_id = i;

			// Indexes
			rect_renderer.indexes[index_idx + 0] = vertex_idx + 0;
			rect_renderer.indexes[index_idx + 1] = vertex_idx + 1;
			rect_renderer.indexes[index_idx + 2] = vertex_idx + 2;

			rect_renderer.indexes[index_idx + 3] = vertex_idx + 2;
			rect_renderer.indexes[index_idx + 4] = vertex_idx + 3;
			rect_renderer.indexes[index_idx + 5] = vertex_idx + 0;

			// Instance
			GPU_RectInstance& instance = rect_renderer.instances[i];
			instance.color = toXM(inst->color.rgba);

			vertex_idx += 4;
			index_idx += 6;
		}
	}

	// Draw
	{
		rect_renderer.vbuff.upload(rect_renderer.verts);
		rect_renderer.idxbuff.upload(rect_renderer.indexes);
		rect_renderer.instances_buff.upload(rect_renderer.instances);

		// Input Assembly
		{
			im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			im_ctx3->IASetIndexBuffer(rect_renderer.idxbuff.get(), DXGI_FORMAT_R32_UINT, 0);
		}

		// Vertex Shader
		{
			std::array<ID3D11ShaderResourceView*, 1> srvs = {
				rect_renderer.vbuff.getSRV()
			};
			im_ctx3->VSSetShaderResources(0, srvs.size(), srvs.data());

			std::array<ID3D11Buffer*, 1> cbuffs = {
				window->cbuff.get()
			};
			im_ctx3->VSSetConstantBuffers(0, cbuffs.size(), cbuffs.data());

			im_ctx3->VSSetShader(simple_vs.Get(), nullptr, 0);
		}

		// Rasterizer
		{
			im_ctx3->RSSetViewports(1, &window->viewport);
			im_ctx3->RSSetState(solid_back_rs.Get());
		}

		// Pixel Shader
		{
			std::array<ID3D11ShaderResourceView*, 1> ps_srv = {
				rect_renderer.instances_buff.getSRV()
			};
			im_ctx3->PSSetShaderResources(0, ps_srv.size(), ps_srv.data());

			im_ctx3->PSSetShader(rect_renderer.rect_ps.Get(), nullptr, 0);
		}

		// Output Merger
		{
			std::array<float, 4> blend_factor = {
				0, 0, 0, 0
			};
			im_ctx3->OMSetBlendState(blend_state.Get(), blend_factor.data(), 0xFFFF'FFFF);

			std::array<ID3D11RenderTargetView*, 1> srv = {
				window->present_rtv.Get()
			};
			im_ctx3->OMSetRenderTargets(srv.size(), srv.data(), nullptr);
		}

		im_ctx3->DrawIndexed(rect_renderer.indexes.size(),
			0, 0);
	}
}

void Instance::drawRect(Window* window, RectInstance* instance)
{
	_drawRects(window, &instance, 1);
}

void Instance::drawRects(Window* window, std::vector<RectInstance*>& instances)
{
	_drawRects(window, instances.data(), instances.size());
}

void Instance::drawArrows(Window* window, std::vector<ArrowInstance*>& instances)
{
	if (instances.size() == 0) {
		return;
	}

	// Generate GPU Data
	{
		rect_renderer.verts.resize(instances.size() * 3);
		rect_renderer.instances.resize(instances.size());

		uint32_t vertex_idx = 0;

		for (uint32_t i = 0; i < instances.size(); i++) {

			ArrowInstance* inst = instances[i];

			// Vertices
			GPU_SimpleVertex& v0 = rect_renderer.verts[vertex_idx + 0];
			GPU_SimpleVertex& v1 = rect_renderer.verts[vertex_idx + 1];
			GPU_SimpleVertex& v2 = rect_renderer.verts[vertex_idx + 2];

			glm::ivec2 pos = { inst->screen_pos[0], inst->screen_pos[1] };
			v0.pos = toXM(pos);
			v1.pos = toXM(pos.x + inst->size[0], pos.y + (inst->size[1] / 2));
			v2.pos = toXM(pos.x, pos.y + inst->size[1]);

			v0.instance_id = i;
			v1.instance_id = i;
			v2.instance_id = i;

			// Instance
			GPU_RectInstance& instance = rect_renderer.instances[i];
			instance.color = toXM(inst->color.rgba);

			vertex_idx += 3;
		}
	}

	// Draw
	{
		rect_renderer.vbuff.upload(rect_renderer.verts);
		rect_renderer.instances_buff.upload(rect_renderer.instances);

		// Input Assembly
		{
			im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}

		// Vertex Shader
		{
			std::array<ID3D11ShaderResourceView*, 1> srvs = {
				rect_renderer.vbuff.getSRV()
			};
			im_ctx3->VSSetShaderResources(0, srvs.size(), srvs.data());

			std::array<ID3D11Buffer*, 1> cbuffs = {
				window->cbuff.get()
			};
			im_ctx3->VSSetConstantBuffers(0, cbuffs.size(), cbuffs.data());

			im_ctx3->VSSetShader(simple_vs.Get(), nullptr, 0);
		}

		// Rasterizer
		{
			im_ctx3->RSSetViewports(1, &window->viewport);
			im_ctx3->RSSetState(solid_back_rs.Get());
		}

		// Pixel Shader
		{
			std::array<ID3D11ShaderResourceView*, 1> ps_srv = {
				rect_renderer.instances_buff.getSRV()
			};
			im_ctx3->PSSetShaderResources(0, ps_srv.size(), ps_srv.data());

			im_ctx3->PSSetShader(rect_renderer.rect_ps.Get(), nullptr, 0);
		}

		// Output Merger
		{
			std::array<float, 4> blend_factor = {
				0, 0, 0, 0
			};
			im_ctx3->OMSetBlendState(blend_state.Get(), blend_factor.data(), 0xFFFF'FFFF);

			std::array<ID3D11RenderTargetView*, 1> srv = {
				window->present_rtv.Get()
			};
			im_ctx3->OMSetRenderTargets(srv.size(), srv.data(), nullptr);
		}

		im_ctx3->Draw(rect_renderer.verts.size(), 0);
	}
}

void Instance::drawBorder(Window* window, std::vector<BorderInstance*>& instances)
{
	if (instances.size() == 0) {
		return;
	}

	static std::vector<RectInstance*> rect_instances;
	rect_instances.clear();

	for (BorderInstance* inst : instances) {

		RectInstance top_inst;
		top_inst.pos = {
			inst->screen_pos[0],
			inst->screen_pos[1]
		};
		top_inst.size = {
			inst->size[0],
			inst->thickness
		};
		top_inst.color = inst->color;

		RectInstance bot_inst;
		bot_inst.pos = {
			inst->screen_pos[0],
			(int32_t)(inst->screen_pos[1] + inst->size[1] - inst->thickness)
		};
		bot_inst.size = {
			inst->size[0],
			inst->thickness
		};
		bot_inst.color = inst->color;

		RectInstance left_inst;
		left_inst.pos = {
			inst->screen_pos[0],
			inst->screen_pos[1]
		};
		left_inst.size = {
			inst->thickness,
			inst->size[1]
		};
		left_inst.color = inst->color;

		RectInstance right_inst;
		right_inst.pos = {
			(int32_t)(inst->screen_pos[0] + inst->size[0] - inst->thickness),
			inst->screen_pos[1]
		};
		right_inst.size = {
			inst->thickness,
			inst->size[1]
		};
		right_inst.color = inst->color;

		if (inst->top) {
			rect_instances.push_back(&top_inst);
		}

		if (inst->bot) {
			rect_instances.push_back(&bot_inst);
		}

		if (inst->left) {
			rect_instances.push_back(&left_inst);
		}

		if (inst->right) {
			rect_instances.push_back(&right_inst);
		}
	}

	drawRects(window, rect_instances);
}

void Instance::drawCircles(Window* window, CircleInstance** instances_data, size_t instances_count)
{
	if (instances_count == 0) {
		return;
	}

	// Generate GPU Data
	{
		circle_renderer.verts.resize(instances_count * 4);
		circle_renderer.indexes.resize(instances_count * 6);
		circle_renderer.instances.resize(instances_count);

		uint32_t vertex_idx = 0;
		uint32_t index_idx = 0;

		for (uint32_t i = 0; i < instances_count; i++) {

			CircleInstance* inst = instances_data[i];

			// Vertices
			GPU_SimpleVertex& tl_v = circle_renderer.verts[vertex_idx + 0];
			GPU_SimpleVertex& tr_v = circle_renderer.verts[vertex_idx + 1];
			GPU_SimpleVertex& br_v = circle_renderer.verts[vertex_idx + 2];
			GPU_SimpleVertex& bl_v = circle_renderer.verts[vertex_idx + 3];

			glm::ivec2 pos = { inst->pos[0], inst->pos[1] };
			tl_v.pos = toXM(pos.x - inst->radius, pos.y - inst->radius);
			tr_v.pos = toXM(pos.x + inst->radius, pos.y - inst->radius);
			br_v.pos = toXM(pos.x + inst->radius, pos.y + inst->radius);
			bl_v.pos = toXM(pos.x - inst->radius, pos.y + inst->radius);

			tl_v.instance_id = i;
			tr_v.instance_id = i;
			br_v.instance_id = i;
			bl_v.instance_id = i;

			// Indexes
			circle_renderer.indexes[index_idx + 0] = vertex_idx + 0;
			circle_renderer.indexes[index_idx + 1] = vertex_idx + 1;
			circle_renderer.indexes[index_idx + 2] = vertex_idx + 2;

			circle_renderer.indexes[index_idx + 3] = vertex_idx + 2;
			circle_renderer.indexes[index_idx + 4] = vertex_idx + 3;
			circle_renderer.indexes[index_idx + 5] = vertex_idx + 0;

			// Instance
			GPU_CircleInstance& instance = circle_renderer.instances[i];
			instance.pos.x = (float)inst->pos[0];
			instance.pos.y = (float)inst->pos[1];
			instance.radius = (float)inst->radius - 1;  // account for multi sampling
			instance.color = toXM(inst->color.rgba);

			vertex_idx += 4;
			index_idx += 6;
		}
	}

	// Draw
	{
		circle_renderer.vbuff.upload(circle_renderer.verts);
		circle_renderer.idxbuff.upload(circle_renderer.indexes);
		circle_renderer.instances_buff.upload(circle_renderer.instances);

		// Input Assembly
		{
			im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			im_ctx3->IASetIndexBuffer(circle_renderer.idxbuff.get(), DXGI_FORMAT_R32_UINT, 0);
		}

		// Vertex Shader
		{
			std::array<ID3D11ShaderResourceView*, 1> srvs = {
				circle_renderer.vbuff.getSRV()
			};
			im_ctx3->VSSetShaderResources(0, srvs.size(), srvs.data());

			std::array<ID3D11Buffer*, 1> cbuffs = {
				window->cbuff.get()
			};
			im_ctx3->VSSetConstantBuffers(0, cbuffs.size(), cbuffs.data());

			im_ctx3->VSSetShader(simple_vs.Get(), nullptr, 0);
		}

		// Rasterizer
		{
			im_ctx3->RSSetViewports(1, &window->viewport);
			im_ctx3->RSSetState(solid_back_rs.Get());
		}

		// Pixel Shader
		{
			std::array<ID3D11ShaderResourceView*, 1> ps_srv = {
				circle_renderer.instances_buff.getSRV()
			};
			im_ctx3->PSSetShaderResources(0, ps_srv.size(), ps_srv.data());

			im_ctx3->PSSetShader(circle_renderer.circle_ps.Get(), nullptr, 0);
		}

		// Output Merger
		{
			std::array<float, 4> blend_factor = {
				0, 0, 0, 0
			};
			im_ctx3->OMSetBlendState(blend_state.Get(), blend_factor.data(), 0xFFFF'FFFF);

			std::array<ID3D11RenderTargetView*, 1> srv = {
				window->present_rtv.Get()
			};
			im_ctx3->OMSetRenderTargets(srv.size(), srv.data(), nullptr);
		}

		im_ctx3->DrawIndexed(circle_renderer.indexes.size(),
			0, 0);
	}
}

void Instance::drawCircle(Window* window, CircleInstance* instance)
{
	drawCircles(window, &instance, 1);
}

//void Instance::registerStyleFile(io::Path& path)
//{
//	ErrStack err_stack;
//
//	StyleFile& new_style_file = style_files.emplace_back();
//	new_style_file.file.create(path);
//
//	tryErrStack1(new_style_file.file.openForParsing());
//}

void Instance::_loadCharacterAtlasToTexture()
{
	TextureAtlas& atlas = text_renderer.char_atlas.atlas;

	if (atlas.tex_size) {
		text_renderer.char_atlas_tex.load(atlas.colors.data(), atlas.tex_size, atlas.tex_size);
	}
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
	_created_windows.push_back(&w);

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

	// Frame Rate
	{
		nui::frame_start_time = std::chrono::steady_clock::now();
		w.min_frame_duration_ms = 16;
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
	w.win_messages.should_close = false;

	// Root Node
	{
		StoredElement2& stored_root = w.retained_elements.emplace_back();

		w.root = &stored_root.specific_elem.emplace<Root>();
		w.root->_window = &w;
		w.root->_parent = nullptr;
		w.root->_self = &stored_root;

		stored_root.base_elem = w.root;

		w.root->_events._init(&w);

		// Put root in drawstack or else window does not handle input
		w.draw_stacks[0].push_back(w.root);
	}

	// Delta Trap
	{
		w.delta_owner_elem = nullptr;
	}

	// Events
	{
		w.finalEvent = nullptr;
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

	// Constant Buffer
	{
		w.cbuff.addUint();
		w.cbuff.addUint();
		w.cbuff.create(dev5.Get(), im_ctx3.Get(), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	}

	return &w;
}