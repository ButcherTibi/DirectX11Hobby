
#include "pch.h"

// Header
#include "NuiLibrary.hpp"


using namespace nui;


void Window::_calculateLayoutAndDrawcalls(Node* node, AncestorProps& ancestor,
	DescendantProps& r_descendant)
{
	switch (node->type) {
	case ElementType::TEXT: {

		Text* text = (Text*)node->elem;

		// Drawcalls
		text->_drawcalls.clear();

		FontSize* font_size = nullptr;
		{
			float min_diff = 9999;

			// Choose most similar font size
			for (FontSize& fsize : instance->_char_atlas.fonts[0].sizes) {

				float diff = (float)std::abs((int64_t)text->size - fsize.size);
				if (diff < min_diff) {

					font_size = &fsize;
					min_diff = diff;
				}
			}
		}

		float scale = (float)text->size / font_size->size;

		float line_height;
		if (text->line_height) {
			line_height = (float)text->line_height;
		}
		else {
			line_height = font_size->ascender * scale;
		}

		glm::vec2 pen = text->pos;
		pen.y += line_height;

		float text_width = 0;

		for (uint32_t unicode : text->text) {

			switch (unicode) {
			case 0x000A:  // newline
				pen.x = 0;
				pen.y += line_height;
				break;

			case 0x0020:  // white space

				for (Character& chara : font_size->chars) {

					if (chara.unicode == unicode) {

						pen.x += chara.advance_X * scale;
						break;
					}
				}
				break;

			default:  // regular character

				// add instance to existing or new drawcall
				CharacterDrawcall* drawcall = [&]() -> CharacterDrawcall* {

					for (CharacterDrawcall& d : text->_drawcalls) {
						if (d.chara->unicode == unicode) {
							return &d;
						}
					}

					CharacterDrawcall& new_drawcall = text->_drawcalls.emplace_back();
					for (Character& chara : font_size->chars) {

						if (chara.unicode == unicode) {
							new_drawcall.chara = &chara;
							return &new_drawcall;
						}
					}
					return nullptr;
				}();

				int32_t char_height = drawcall->chara->zone->bb_pix.getHeight();
				int32_t char_top = drawcall->chara->bitmap_top;

				glm::vec2 new_pos = pen;
				new_pos.x += (float)drawcall->chara->bitmap_left * scale;
				new_pos.y += (char_height - char_top) * scale;

				GPU_CharacterInstance& new_instance = drawcall->instances.emplace_back();
				new_instance.color.x = text->color.rgba.r;
				new_instance.color.y = text->color.rgba.g;
				new_instance.color.z = text->color.rgba.b;
				new_instance.color.w = text->color.rgba.a;

				new_instance.pos.x = new_pos.x;
				new_instance.pos.y = new_pos.y;

				new_instance.rasterized_size = (float)font_size->size;
				new_instance.size = (float)text->size;
				new_instance.parent_clip_mask = ancestor.clip_mask;

				pen.x += drawcall->chara->advance_X * scale;
			}

			if (pen.x > text_width) {
				text_width = pen.x;
			}
		}

		r_descendant.pos = &text->pos;
		r_descendant.width = (uint32_t)(text_width - text->pos.x);
		r_descendant.height = (uint32_t)(pen.y - text->pos.y + (font_size->descender * scale));

		this->char_instance_count += text->text.size();

		// Calculate Event Collider
		node->collider.x0 = text->pos.x;
		node->collider.x1 = std::min(text->pos.x + r_descendant.width, ancestor.clip_width);
		node->collider.y0 = text->pos.y;
		node->collider.y1 = std::min(text->pos.y + r_descendant.height, ancestor.clip_height);
		break;
	}

	case ElementType::WRAP: {

		Wrap* wrap = (Wrap*)node->elem;

		AncestorProps child_ancs;

		// Sizes
		auto calcSize = [](ElementSize size, uint32_t ancestor_size) -> uint32_t {
			switch (size.type) {
			case ElementSizeType::ABSOLUTE_SIZE:
				return (uint32_t)size.size;
			case ElementSizeType::RELATIVE_SIZE:
				return (uint32_t)(size.size * (float)ancestor_size);
			}
			return ancestor_size;  // FIT
		};

		child_ancs.width = calcSize(wrap->width, ancestor.width);
		child_ancs.height = calcSize(wrap->height, ancestor.height);

		// Collider Sizes and Child Clip Mask
		if (wrap->overflow == Overflow::CLIP) {
			child_ancs.clip_width = child_ancs.width;
			child_ancs.clip_height = child_ancs.height;

			this->clip_mask_id++;
			child_ancs.clip_mask = this->clip_mask_id;
		}
		else {
			child_ancs.clip_width = ancestor.clip_width;
			child_ancs.clip_height = ancestor.clip_height;

			child_ancs.clip_mask = ancestor.clip_mask;
		}

		// Calculate Children
		std::vector<DescendantProps> child_props(node->children.size());
		auto child_it = node->children.begin();

		for (uint32_t i = 0; i < node->children.size(); i++, child_it++) {

			DescendantProps& child_prop = child_props[i];
			_calculateLayoutAndDrawcalls(*child_it, child_ancs, child_prop);
		}

		r_descendant.pos = &wrap->pos;

		// Make size big enough to engulf all children
		if (wrap->width.type == ElementSizeType::FIT) {

			uint32_t right_most = 0;
			for (DescendantProps& child_prop : child_props) {
				uint32_t right = child_prop.pos->x + child_prop.width;
				if (right_most < right) {
					right_most = right;
				}
			}

			r_descendant.width = right_most;
		}
		else {
			r_descendant.width = child_ancs.width;
		}

		if (wrap->height.type == ElementSizeType::FIT) {

			uint32_t bottom_most = 0;
			for (DescendantProps& child_prop : child_props) {
				uint32_t bottom = child_prop.pos->y + child_prop.height;
				if (bottom_most < bottom) {
					bottom_most = bottom;
				}
			}

			r_descendant.height = bottom_most;
		}
		else {
			r_descendant.height = child_ancs.height;
		}

		// Drawcall
		auto& inst = wrap->_drawcall.instance;
		inst.pos = { (float)wrap->pos.x, (float)wrap->pos.y };
		inst.size = { (float)r_descendant.width, (float)r_descendant.height };
		inst.color = { wrap->background_color.rgba.r,
			wrap->background_color.rgba.g,
			wrap->background_color.rgba.b,
			wrap->background_color.rgba.a
		};
		inst.parent_clip_id = ancestor.clip_mask;
		inst.child_clip_id = child_ancs.clip_mask;

		this->wrap_instance_count += 1;

		// Calculate Event Collider
		node->collider.x0 = wrap->pos.x;
		node->collider.x1 = std::min(wrap->pos.x + r_descendant.width, ancestor.clip_width);
		node->collider.y0 = wrap->pos.y;
		node->collider.y1 = std::min(wrap->pos.y + r_descendant.height, ancestor.clip_height);
		break;
	}

	case ElementType::FLEX: {
		// Flex* flex = (Flex*)node->elem;
		break;
	}
	}
}

void Window::_calcGlobalPositions(Node* node, glm::uvec2 pos)
{
	switch (node->type) {
	case ElementType::TEXT: {
		Text* text = (Text*)node->elem;

		for (CharacterDrawcall& drawcall : text->_drawcalls) {
			for (GPU_CharacterInstance& inst : drawcall.instances) {
				inst.pos.x += pos.x;
				inst.pos.y += pos.y;
			}
		}

		// Collider
		node->collider.x0 += pos.x;
		node->collider.x1 += pos.x;
		node->collider.y0 += pos.y;
		node->collider.y1 += pos.y;
		break;
	}

	case ElementType::WRAP: {
		Wrap* wrap = (Wrap*)node->elem;

		wrap->_drawcall.instance.pos.x += pos.x;
		wrap->_drawcall.instance.pos.y += pos.y;

		for (Node* child : node->children) {
			_calcGlobalPositions(child, { wrap->_drawcall.instance.pos.x, wrap->_drawcall.instance.pos.y });
		}
		break;
	}

	case ElementType::FLEX: {
		// Flex* flex = (Flex*)node->elem;
		break;
	}
	}
}

ErrStack Window::_updateCPU_Data()
{
	ErrStack err_stack;

	std::list<Node*>& children = nodes.front().children;

	// Respond to Input
	{
		_emitEvents();
	}

	// Create Character Meshes
	{
		// Rasterize new font sizes
		Font& font = instance->_char_atlas.fonts[0];
		{
			std::set<uint32_t> sizes;

			for (Node& node : nodes) {
				if (node.type == ElementType::TEXT) {

					Text* text = (Text*)node.elem;
					sizes.insert((uint32_t)(text->size));
				}
			}

			// TODO: sort from smalles to biggest to better pack in atlas

			// delete already rasterized sizes
			for (FontSize& font_size : font.sizes) {
				sizes.erase(font_size.size);
			}

			for (uint32_t size : sizes) {
				checkErrStack1(font.addSize(size));
			}
		}

		uint32_t vertex_count = 0;
		uint32_t index_count = 0;

		for (FontSize& font_size : font.sizes) {
			vertex_count += font_size.chars.size() * 4;
			index_count += font_size.chars.size() * 6;
		}

		this->char_verts.resize(vertex_count);
		this->chars_idxs.resize(index_count);

		uint32_t vertex_idx = 0;
		uint32_t index_idx = 0;
		for (FontSize& font_size : font.sizes) {
			for (Character& chara : font_size.chars) {

				if (chara.zone == nullptr) {
					continue;
				}

				float w = (float)chara.zone->bb_pix.getWidth();
				float h = (float)chara.zone->bb_pix.getHeight();

				chara.vertex_start_idx = vertex_idx;
				chara.index_start_idx = index_idx;

				// Character origin is bottom left
				char_verts[vertex_idx + 0].pos = { 0, 0 };
				char_verts[vertex_idx + 0].uv = toXMFloat2(chara.zone->bb_uv.getBotLeft());

				char_verts[vertex_idx + 1].pos = { 0, -h };
				char_verts[vertex_idx + 1].uv = toXMFloat2(chara.zone->bb_uv.getTopLeft());

				char_verts[vertex_idx + 2].pos = { w, -h };
				char_verts[vertex_idx + 2].uv = toXMFloat2(chara.zone->bb_uv.getTopRight());

				char_verts[vertex_idx + 3].pos = { w, 0 };
				char_verts[vertex_idx + 3].uv = toXMFloat2(chara.zone->bb_uv.getBotRight());

				chars_idxs[index_idx + 0] = vertex_idx + 0;
				chars_idxs[index_idx + 1] = vertex_idx + 1;
				chars_idxs[index_idx + 2] = vertex_idx + 2;

				chars_idxs[index_idx + 3] = vertex_idx + 2;
				chars_idxs[index_idx + 4] = vertex_idx + 3;
				chars_idxs[index_idx + 5] = vertex_idx + 0;

				vertex_idx += 4;
				index_idx += 6;
			}
		}
	}

	// Calculate Layout and Drawcalls
	{
		this->clip_mask_id = 1;
		this->char_instance_count = 0;
		this->wrap_instance_count = 0;

		AncestorProps ancestor;
		ancestor.width = surface_width;
		ancestor.height = surface_height;
		ancestor.clip_width = surface_width;
		ancestor.clip_height = surface_height;
		ancestor.clip_mask = 1;

		DescendantProps descendant;	

		for (Node* child : children) {
			_calculateLayoutAndDrawcalls(child, ancestor, descendant);
		}

		for (Node* child : children) {
			_calcGlobalPositions(child, { 0, 0 });
		}
	}

	// Index Instances
	{
		this->char_instances.resize(this->char_instance_count);
		this->wrap_instances.resize(this->wrap_instance_count);

		uint32_t char_instance_idx = 0;
		uint32_t wrap_instance_idx = 0;

		for (Node& node : nodes) {

			switch (node.type) {
			case ElementType::TEXT: {
				Text* text = (Text*)node.elem;

				for (CharacterDrawcall& drawcall : text->_drawcalls) {

					drawcall.instance_idx = char_instance_idx;

					std::memcpy(char_instances.data() + char_instance_idx, drawcall.instances.data(),
						drawcall.instances.size() * sizeof(GPU_CharacterInstance));

					char_instance_idx += drawcall.instances.size();
				}
				break;
			}

			case ElementType::WRAP: {
				Wrap* wrap = (Wrap*)node.elem;

				wrap->_drawcall.instance_idx = wrap_instance_idx;

				wrap_instances[wrap_instance_idx] = wrap->_drawcall.instance;

				wrap_instance_idx++;
				break;
			}
			}
		}
	}

	// Commons
	{
		Node& root_node = nodes.front();
		root_node.collider.set(surface_width, surface_height);

		common_uniform.screen_size.x = (float)surface_width;
		common_uniform.screen_size.y = (float)surface_height;
	}

	return err_stack;
}

ErrStack Window::_loadCPU_DataToGPU()
{
	ErrStack err_stack;

	// Characters
	checkErrStack1(chars_vbuff.load(char_verts.data(), char_verts.size() * sizeof(GPU_CharacterVertex)));
	checkErrStack1(chars_idxbuff.load(chars_idxs.data(), chars_idxs.size() * sizeof(uint32_t)));

	if (this->char_instance_count) {
		checkErrStack1(chars_instabuff.load(char_instances.data(), char_instances.size() * sizeof(GPU_CharacterInstance)));
	}

	// Wrap
	if (this->wrap_instance_count) {
		checkErrStack1(wrap_instabuff.load(wrap_instances.data(), wrap_instances.size() * sizeof(GPU_WrapInstance)));
	}

	// Texture Atlas
	{
		TextureAtlas& atlas = instance->_char_atlas.atlas;

		checkErrStack1(dx11::singleLoad(im_ctx4.Get(), chars_atlas_tex.Get(),
			atlas.colors.data(), sizeof(uint8_t) * atlas.colors.size()));
	}

	// Commons Constant Buffer
	checkErrStack1(cbuff.load(&common_uniform, sizeof(GPU_CommonsUniform)));

	return err_stack;
}

ErrStack Window::_resizeAllAtachments()
{
	HRESULT hr = S_OK;
	ErrStack err_stack;

	present_rtv->Release();
	parents_clip_mask_rtv->Release();
	next_parents_clip_mask_rtv->Release();

	parents_clip_mask_srv->Release();
	next_parents_clip_mask_srv->Release();

	// Present Images
	present_img->Release();
	checkHResult(swapchain->ResizeBuffers(2, width, height,
		DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH),
		"failed to resize swapchain back buffers");

	checkHResult(swapchain->GetBuffer(0, IID_PPV_ARGS(present_img.GetAddressOf())),
		"failed to get swapchain back buffer");

	// Images
	checkErrStack1(dx11::resizeTexture2D(dev5.Get(), surface_width, surface_height, parents_clip_mask_img));
	checkErrStack1(dx11::resizeTexture2D(dev5.Get(), surface_width, surface_height, next_parents_clip_mask_img));

	// Render Target Views
	checkHResult(dev5->CreateRenderTargetView(present_img.Get(), NULL, present_rtv.GetAddressOf()),
		"failed to create present RTV");

	checkHResult(dev5->CreateRenderTargetView(parents_clip_mask_img.Get(), NULL,
		parents_clip_mask_rtv.GetAddressOf()),
		"failed to create next parents clip mask rtv");

	checkHResult(dev5->CreateRenderTargetView(next_parents_clip_mask_img.Get(), NULL,
		next_parents_clip_mask_rtv.GetAddressOf()),
		"failed to create next parents clip mask rtv");

	// Shader Resource Views
	checkHResult(dev5->CreateShaderResourceView(parents_clip_mask_img.Get(), NULL,
		parents_clip_mask_srv.GetAddressOf()),
		"failed to create parents clip mask srv");

	checkHResult(dev5->CreateShaderResourceView(next_parents_clip_mask_img.Get(), NULL,
		next_parents_clip_mask_srv.GetAddressOf()),
		"failed to create next parents clip mask srv");

	return err_stack;
}

ErrStack Window::_render()
{
	ErrStack err_stack;
	HRESULT hr = 0;

	if (!rendering_configured) {

		rendering_configured = true;

		// Masks
		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = surface_width;
			desc.Height = surface_height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R32_UINT;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			// Parents Clip Mask Images
			checkHResult(dev5->CreateTexture2D(&desc, NULL, parents_clip_mask_img.GetAddressOf()),
				"failed to create parents clip mask image");

			checkHResult(dev5->CreateShaderResourceView(parents_clip_mask_img.Get(), NULL,
				parents_clip_mask_srv.GetAddressOf()),
				"failed to create parents clip mask srv");

			checkHResult(dev5->CreateRenderTargetView(parents_clip_mask_img.Get(), NULL,
				parents_clip_mask_rtv.GetAddressOf()),
				"failed to create parents clip mask rtv");

			// Next Parents Clip Mask Images
			checkHResult(dev5->CreateTexture2D(&desc, NULL, next_parents_clip_mask_img.GetAddressOf()),
				"failed to create next parents clip mask image");

			checkHResult(dev5->CreateShaderResourceView(next_parents_clip_mask_img.Get(), NULL,
				next_parents_clip_mask_srv.GetAddressOf()),
				"failed to create next parents clip mask srv");

			checkHResult(dev5->CreateRenderTargetView(next_parents_clip_mask_img.Get(), NULL,
				next_parents_clip_mask_rtv.GetAddressOf()),
				"failed to create next parents clip mask rtv");
		}

		// Vertex Shaders
		{
			checkHResult(dev5->CreateVertexShader(instance->_wrap_vs_cso.data(), instance->_wrap_vs_cso.size(),
				NULL, wrap_vs.GetAddressOf()),
				"failed to create wrap VS");

			checkHResult(dev5->CreateVertexShader(instance->_chars_vs_cso.data(), instance->_chars_vs_cso.size(),
				NULL, chars_vs.GetAddressOf()),
				"failed to create chars VS");

			checkHResult(dev5->CreateVertexShader(instance->_all_vs_cso.data(), instance->_all_vs_cso.size(),
				NULL, all_vs.GetAddressOf()),
				"failed to create all VS");
		}

		// Wrap Input Layout
		{
			std::vector<D3D11_INPUT_ELEMENT_DESC> input_elems;

			auto vertex_elems = GPU_WrapVertex::getInputLayout();
			for (auto& elem : vertex_elems) {
				input_elems.push_back(elem);
			}

			auto instance_elems = GPU_WrapInstance::getInputLayout(1);
			for (auto& elem : instance_elems) {
				input_elems.push_back(elem);
			}

			checkHResult(dev5->CreateInputLayout(input_elems.data(), input_elems.size(),
				instance->_wrap_vs_cso.data(), instance->_wrap_vs_cso.size(), wrap_input_layout.GetAddressOf()),
				"failed to create wrap input layout");
		}

		// Chars Input Layout
		{
			std::vector<D3D11_INPUT_ELEMENT_DESC> input_elems;

			auto vertex_elems = GPU_CharacterVertex::getInputLayout();
			for (auto& elem : vertex_elems) {
				input_elems.push_back(elem);
			}

			auto instance_elems = GPU_CharacterInstance::getInputLayout(1);
			for (auto& elem : instance_elems) {
				input_elems.push_back(elem);
			}

			checkHResult(dev5->CreateInputLayout(input_elems.data(), input_elems.size(),
				instance->_chars_vs_cso.data(), instance->_chars_vs_cso.size(), chars_input_layout.GetAddressOf()),
				"failed to create chars input layout");
		}

		// Rasterizer
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

			checkHResult(dev5->CreateRasterizerState(&desc, rasterizer_state.GetAddressOf()),
				"failed to create rasterizer state");
		}

		// Pixel Shader
		{
			checkHResult(dev5->CreatePixelShader(instance->_wrap_ps_cso.data(), instance->_wrap_ps_cso.size(),
				NULL, wrap_ps.GetAddressOf()),
				"failed to create wrap PS");

			checkHResult(dev5->CreatePixelShader(instance->_chars_ps_cso.data(), instance->_chars_ps_cso.size(),
				NULL, chars_ps.GetAddressOf()),
				"failed to create chars PS");

			checkHResult(dev5->CreatePixelShader(instance->_copy_parents_ps_cso.data(), instance->_copy_parents_ps_cso.size(),
				NULL, copy_parents_ps.GetAddressOf()),
				"failed to create copy parents PS");
		}

		// Blend States
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

			desc.RenderTarget[1].BlendEnable = false;
			desc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			checkHResult(dev5->CreateBlendState(&desc, blend_state.GetAddressOf()),
				"failed to create wrap blend state");
		}

		// Vertex Buffers
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			wrap_vbuff.create(dev5.Get(), im_ctx4.Get(), desc);
			checkErrStack1(wrap_vbuff.load(wrap_verts.data(), sizeof(GPU_WrapVertex) * wrap_verts.size()));

			chars_vbuff.create(dev5.Get(), im_ctx4.Get(), desc);
		}

		// Index Buffers
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			wrap_idxbuff.create(dev5.Get(), im_ctx4.Get(), desc);
			checkErrStack1(wrap_idxbuff.load(wrap_idxs.data(), sizeof(uint16_t) * wrap_idxs.size()));

			chars_idxbuff.create(dev5.Get(), im_ctx4.Get(), desc);
		}

		// Instance Buffers
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			wrap_instabuff.create(dev5.Get(), im_ctx4.Get(), desc);
			chars_instabuff.create(dev5.Get(), im_ctx4.Get(), desc);
		}

		// Constant Buffer
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			cbuff.create(dev5.Get(), im_ctx4.Get(), desc);
		}

		// Character Atlas
		{
			TextureAtlas& atlas = instance->_char_atlas.atlas;

			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = atlas.tex_size;
			desc.Height = atlas.tex_size;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;

			checkHResult(dev5->CreateTexture2D(&desc, NULL, chars_atlas_tex.GetAddressOf()),
				"failed to create character atlas texture");

			checkHResult(dev5->CreateShaderResourceView(chars_atlas_tex.Get(), NULL,
				chars_atlas_srv.GetAddressOf()),
				"failed to create character atlas srv");
		}

		// Character Atlas Sampler
		{
			D3D11_SAMPLER_DESC desc;
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
			desc.MipLODBias = 0;
			desc.MaxAnisotropy = 1;
			desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			desc.BorderColor[0] = 0;
			desc.BorderColor[1] = 0;
			desc.BorderColor[2] = 0;
			desc.BorderColor[3] = 0;
			desc.MinLOD = 0;
			desc.MaxLOD = 0;

			checkHResult(dev5->CreateSamplerState(&desc, chars_atlas_sampler.GetAddressOf()),
				"failed to create character atlas sampler");
		}
	}

	// Window Resize
	DXGI_SWAP_CHAIN_DESC swapchain_desc;
	swapchain->GetDesc(&swapchain_desc);

	if (surface_width != swapchain_desc.BufferDesc.Width ||
		surface_height != swapchain_desc.BufferDesc.Height)
	{
		checkErrStack1(_resizeAllAtachments());
	}

	checkErrStack1(_loadCPU_DataToGPU());

	// Commands
	float zero_color[4] = {
		0, 0, 0, 0
	};
	float parent_init_value[4] = {
		1, 0, 0, 0
	};

	im_ctx4->ClearRenderTargetView(present_rtv.Get(), zero_color);
	im_ctx4->ClearRenderTargetView(parents_clip_mask_rtv.Get(), parent_init_value);
	im_ctx4->ClearRenderTargetView(next_parents_clip_mask_rtv.Get(), zero_color);

	std::vector<Node*> now_nodes = {
		&nodes.front()
	};
	std::vector<Node*> next_nodes;

	// Common State
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)swapchain_desc.BufferDesc.Width;
	viewport.Height = (float)swapchain_desc.BufferDesc.Height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	while (now_nodes.size()) {

		for (Node* node : now_nodes) {

			switch (node->type) {
			case ElementType::TEXT: {
				Text* text = (Text*)node->elem;

				if (!text->_drawcalls.size()) {
					break;
				}

				im_ctx4->ClearState();

				// Input Assembly
				im_ctx4->IASetInputLayout(chars_input_layout.Get());
				im_ctx4->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				std::array<ID3D11Buffer*, 2> vbuffs;
				vbuffs[0] = chars_vbuff.buff.Get();
				vbuffs[1] = chars_instabuff.buff.Get();

				std::array<uint32_t, 2> strides = {
					sizeof(GPU_CharacterVertex), sizeof(GPU_CharacterInstance)
				};

				std::array<uint32_t, 2> offsets = {
					0, 0
				};
				im_ctx4->IASetVertexBuffers(0, vbuffs.size(), vbuffs.data(), strides.data(), offsets.data());
				im_ctx4->IASetIndexBuffer(chars_idxbuff.buff.Get(), DXGI_FORMAT_R32_UINT, 0);

				// Vertex Shader
				im_ctx4->VSSetConstantBuffers(0, 1, cbuff.buff.GetAddressOf());
				im_ctx4->VSSetShader(chars_vs.Get(), NULL, 0);

				// Rasterizer
				im_ctx4->RSSetViewports(1, &viewport);
				im_ctx4->RSSetState(rasterizer_state.Get());

				// Pixel Shader
				std::array<ID3D11ShaderResourceView*, 2> srviews = {
					parents_clip_mask_srv.Get(), chars_atlas_srv.Get()
				};
				im_ctx4->PSSetShaderResources(0, srviews.size(), srviews.data());
				im_ctx4->PSSetSamplers(0, 1, chars_atlas_sampler.GetAddressOf());
				im_ctx4->PSSetShader(chars_ps.Get(), NULL, 0);

				// Output Merger
				im_ctx4->OMSetBlendState(blend_state.Get(), zero_color, 0xFFFF'FFFF);

				std::array<ID3D11RenderTargetView*, 1> rtvs;
				rtvs[0] = present_rtv.Get();
				im_ctx4->OMSetRenderTargets(rtvs.size(), rtvs.data(), NULL);

				for (CharacterDrawcall& drawcall : text->_drawcalls) {
					im_ctx4->DrawIndexedInstanced(6, (uint32_t)drawcall.instances.size(),
						drawcall.chara->index_start_idx, 0, drawcall.instance_idx);
				}
				break;
			}

			case ElementType::WRAP: {
				Wrap* wrap = (Wrap*)node->elem;

				im_ctx4->ClearState();

				// Input Assembly
				im_ctx4->IASetInputLayout(wrap_input_layout.Get());
				im_ctx4->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				std::array<ID3D11Buffer*, 2> vbuffs;
				vbuffs[0] = wrap_vbuff.buff.Get();
				vbuffs[1] = wrap_instabuff.buff.Get();

				std::array<uint32_t, 2> strides = {
					sizeof(GPU_WrapVertex), sizeof(GPU_WrapInstance)
				};

				std::array<uint32_t, 2> offsets = {
					0, 0
				};

				im_ctx4->IASetVertexBuffers(0, vbuffs.size(), vbuffs.data(), strides.data(), offsets.data());
				im_ctx4->IASetIndexBuffer(wrap_idxbuff.buff.Get(), DXGI_FORMAT_R16_UINT, 0);

				// Vertex Shader
				im_ctx4->VSSetConstantBuffers(0, 1, cbuff.buff.GetAddressOf());
				im_ctx4->VSSetShader(wrap_vs.Get(), NULL, 0);

				// Rasterizer
				im_ctx4->RSSetViewports(1, &viewport);
				im_ctx4->RSSetState(rasterizer_state.Get());

				// Pixel Shader
				im_ctx4->PSSetShaderResources(0, 1, parents_clip_mask_srv.GetAddressOf());
				im_ctx4->PSSetShader(wrap_ps.Get(), NULL, 0);

				// Output Merger
				im_ctx4->OMSetBlendState(blend_state.Get(), zero_color, 0xFFFF'FFFF);

				std::array<ID3D11RenderTargetView*, 2> rtvs;
				rtvs[0] = present_rtv.Get();
				rtvs[1] = next_parents_clip_mask_rtv.Get();
				im_ctx4->OMSetRenderTargets(rtvs.size(), rtvs.data(), NULL);

				im_ctx4->DrawIndexedInstanced(6, 1, 0, 0, wrap->_drawcall.instance_idx);
				break;
			}
			}

			for (Node* child : node->children) {
				next_nodes.push_back(child);
			}
		}

		now_nodes = next_nodes;
		next_nodes.clear();

		// Copy Next Parents to Parents
		{
			im_ctx4->ClearState();

			// Input Assempbly
			im_ctx4->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Vertex Shader
			im_ctx4->VSSetShader(all_vs.Get(), nullptr, 0);

			// Rasterizer
			im_ctx4->RSSetViewports(1, &viewport);
			im_ctx4->RSSetState(rasterizer_state.Get());

			// Pixel Shader
			std::array<ID3D11ShaderResourceView*, 2> srvs = {
				next_parents_clip_mask_srv.Get(),
				nullptr
			};
			im_ctx4->PSSetShaderResources(0, srvs.size(), srvs.data());
			im_ctx4->PSSetShader(copy_parents_ps.Get(), nullptr, 0);

			// Output Merger
			im_ctx4->OMSetRenderTargets(1, parents_clip_mask_rtv.GetAddressOf(), nullptr);

			im_ctx4->Draw(6, 0);
		}

		//im_ctx4->CopyResource(parents_clip_mask_img.Get(), next_parents_clip_mask_img.Get());
		im_ctx4->ClearRenderTargetView(next_parents_clip_mask_rtv.Get(), zero_color);
	}

	checkHResult(swapchain->Present(0, 0),
		"failed to present swapchain");

	return err_stack;
}
