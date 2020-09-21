
#include "pch.h"

// Mine
#include "FileIO.hpp"

// Header
#include "NuiLibrary.hpp"


using namespace nui;

std::list<Window> nui::windows;


void ContentSize::setAbsolute(float new_size)
{
	this->type = ContentSizeType::ABSOLUTE_SIZE;
	this->size = new_size;
}

void ContentSize::setRelative(float new_size)
{
	this->type = ContentSizeType::RELATIVE_SIZE;
	this->size = new_size;
}

Text* NodeComponent::addText()
{
	Node& child_node = window->nodes.emplace_back();

	Text* new_text = child_node.createText();
	new_text->node_comp.window = this->window;
	new_text->node_comp.this_elem = &child_node;

	new_text->pos = { 0, 0 };
	new_text->size = 14.0f;
	new_text->line_height = 1.15f;
	new_text->color = { 1, 1, 1, 1 };

	// Parent ---> Child
	this->this_elem->children.push_back(&child_node);

	// Parent <--- Child
	child_node.parent = this->this_elem;

	return new_text;
}

Wrap* NodeComponent::addWrap()
{
	Node& child_node = window->nodes.emplace_back();

	Wrap* child_wrap = child_node.createWrap();
	child_wrap->node_comp.window = this->window;
	child_wrap->node_comp.this_elem = &child_node;

	child_wrap->pos = { 0, 0 };
	child_wrap->overflow = Overflow::OVERFLOw;
	child_wrap->background_color = { 0, 0, 0, 0 };

	// Parent ---> Child
	this->this_elem->children.push_back(&child_node);

	// Parent <--- Child
	child_node.parent = this->this_elem;

	return child_wrap;
}

Flex* NodeComponent::addFlex()
{
	Node& child_node = window->nodes.emplace_back();

	Flex* child_flex = child_node.createFlex();
	child_flex->node_comp.window = this->window;

	return child_flex;
}

Text* Wrap::addText()
{
	return node_comp.addText();
}

Wrap* Wrap::addWrap()
{
	return node_comp.addWrap();
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	for (Window& wnd : windows) {
		if (wnd.hwnd == hwnd) {
			
			switch (uMsg) {
			case WM_SIZE: {

				switch (wParam) {
				case SIZE_MAXIMIZED:
				case SIZE_RESTORED: {
					wnd.minimized = false;

					uint32_t l_param = static_cast<uint32_t>(lParam);
					wnd.width = l_param & 0xFFFF;
					wnd.height = l_param >> 16;

					RECT rect;
					GetClientRect(hwnd, &rect);

					wnd.surface_width = rect.right - rect.left;
					wnd.surface_height = rect.bottom - rect.top;
					break;
				}

				case SIZE_MINIMIZED:
					wnd.minimized = true;
					break;
				}
				break;
			}

			case WM_DESTROY:
			case WM_QUIT:
			case WM_CLOSE: {
				wnd.close = true;
				return 0;
			}
			}
			break;
		}
	}
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

Wrap* Node::createWrap()
{
	this->type = NodeType::WRAP;
	this->elem = new Wrap();
	return static_cast<Wrap*>(elem);
}

Flex* Node::createFlex()
{
	this->type = NodeType::FLEX;
	this->elem = new Flex();
	return static_cast<Flex*>(elem);
}

Text* Node::createText()
{
	this->type = NodeType::TEXT;
	this->elem = new Text();
	return static_cast<Text*>(elem);
}

Node::~Node()
{
	if (elem != nullptr) {
		switch (type) {
		case nui::NodeType::WRAP:
			delete (Wrap*)elem;
			break;

		case nui::NodeType::FLEX:
			delete (Flex*)elem;
			break;

		case nui::NodeType::TEXT:
			delete (Text*)elem;
			break;

		default:
			printf("missing type \n");
			break;
		}
	}
}

void Window::generateDrawcalls(Node* node, AncestorProps& ancestor,
	DescendantProps& r_descendant)
{
	switch (node->type) {
	case NodeType::TEXT:{

		Text* text = (Text*)node->elem;
		text->drawcalls.clear();

		FontSize* font_size = nullptr;
		{
			float min_diff = 9999;

			// Choose most similar font size
			for (FontSize& fsize : text->node_comp.window->instance->char_atlas.fonts[0].sizes) {

				float diff = abs(text->size - fsize.size);
				if (diff < min_diff) {

					font_size = &fsize;
					min_diff = diff;
				}
			}
		}

		float scale = text->size / font_size->size;
		float text_width = 0;

		glm::vec2 pen = text->pos;
		pen.y += font_size->ascender * scale;

		for (uint32_t unicode : text->text) {	

			switch (unicode) {
			case 0x000A:  // newline
				pen.x = 0;
				pen.y += font_size->line_spacing * text->line_height * scale;
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

					for (CharacterDrawcall& d : text->drawcalls) {
						if (d.chara->unicode == unicode) {
							return &d;
						}
					}

					CharacterDrawcall& new_drawcall = text->drawcalls.emplace_back();
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
				new_instance.color.x = text->color.r;
				new_instance.color.y = text->color.g;
				new_instance.color.z = text->color.b;
				new_instance.color.w = text->color.a;

				new_instance.pos.x = new_pos.x;
				new_instance.pos.y = new_pos.y;

				new_instance.rasterized_size = (float)font_size->size;
				new_instance.size = text->size;
				new_instance.parent_clip_mask = ancestor.clip_mask;

				pen.x += drawcall->chara->advance_X * scale;
			}

			if (pen.x > text_width) {
				text_width = pen.x;
			}
		}

		r_descendant.pos = &text->pos;
		r_descendant.width = text_width - text->pos.x;
		r_descendant.height = pen.y - text->pos.y + (font_size->descender * scale);

		this->char_instance_count += text->text.size();
		break;
	}

	case NodeType::WRAP: {

		Wrap* wrap = (Wrap*)node->elem;
		Node* parent = wrap->node_comp.this_elem->parent;

		AncestorProps child_ancs;

		// Size
		auto calcSize = [](ContentSize size, float ancestor_size) -> float {
			switch (size.type) {
			case ContentSizeType::ABSOLUTE_SIZE:
				return size.size;
			case ContentSizeType::RELATIVE_SIZE:
				return size.size * ancestor_size;
			}
			// FIT
			return ancestor_size;
		};

		child_ancs.width = calcSize(wrap->width, ancestor.width);
		child_ancs.height = calcSize(wrap->height, ancestor.height);

		// Child Clip Mask
		if (wrap->overflow == Overflow::CLIP) {
			child_ancs.clip_mask = this->clip_mask_id;
			this->clip_mask_id++;
		}
		else {
			child_ancs.clip_mask = ancestor.clip_mask;
		}

		std::vector<DescendantProps> child_props(node->children.size());
		auto child_it = node->children.begin();

		for (uint32_t i = 0; i < node->children.size(); i++, child_it++) {

			DescendantProps& child_prop = child_props[i];
			generateDrawcalls(*child_it, child_ancs, child_prop);
		}

		if (wrap->width.type == ContentSizeType::FIT) {

			float right_most = 0;
			for (DescendantProps& child_prop : child_props) {
				float right = child_prop.pos->x + child_prop.width;
				if (right_most < right) {
					right_most = right;
				}
			}

			r_descendant.width = right_most;
		}
		else {
			r_descendant.width = child_ancs.width;
		}

		if (wrap->height.type == ContentSizeType::FIT) {

			float bottom_most = 0;
			for (DescendantProps& child_prop : child_props) {
				float bottom = child_prop.pos->y + child_prop.height;
				if (bottom_most < bottom) {
					bottom_most = bottom;
				}
			}

			r_descendant.height = bottom_most;
		}
		else {
			r_descendant.height = child_ancs.height;
		}

		auto& inst = wrap->drawcall.instance;
		inst.color.x = wrap->background_color.r;
		inst.color.y = wrap->background_color.g;
		inst.color.z = wrap->background_color.b;
		inst.color.w = wrap->background_color.a;

		if (parent != nullptr) {	
			inst.pos.x = wrap->pos.x;
			inst.pos.y = wrap->pos.y;
			inst.size.x = r_descendant.width;
			inst.size.y = r_descendant.height;
			inst.parent_clip_id = ancestor.clip_mask;
			inst.child_clip_id = child_ancs.clip_mask;
		}
		else {
			inst.pos = { 0, 0 };
			inst.size.x = wrap->width.size;
			inst.size.y = wrap->height.size;
			inst.parent_clip_id = 0;
			inst.child_clip_id = 0;
		}

		r_descendant.pos = &wrap->pos;

		this->wrap_instance_count += 1;
		break;
	}

	case NodeType::FLEX: {
		// Flex* flex = (Flex*)node->elem;
		break;
	}
	}
}

void Window::calcGlobalPositions(Node* node, glm::vec2 pos)
{
	switch (node->type) {
	case NodeType::TEXT: {
		Text* text = (Text*)node->elem;

		for (CharacterDrawcall& drawcall : text->drawcalls) {
			for (GPU_CharacterInstance& inst : drawcall.instances) {
				inst.pos.x += pos.x;
				inst.pos.y += pos.y;
			}
		}
		break;
	}

	case NodeType::WRAP: {
		Wrap* wrap = (Wrap*)node->elem;

		wrap->drawcall.instance.pos.x += pos.x;
		wrap->drawcall.instance.pos.y += pos.y;

		for (Node* child : node->children) {
			calcGlobalPositions(child, { wrap->drawcall.instance.pos.x, wrap->drawcall.instance.pos.y });
		}
		break;
	}

	case NodeType::FLEX: {
		// Flex* flex = (Flex*)node->elem;
		break;
	}
	}
}

ErrStack Window::generateGPU_Data()
{
	ErrStack err_stack;

	// Create Character Meshes
	{
		Font& font = instance->char_atlas.fonts[0];

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
				char_verts[vertex_idx + 0].uv = chara.zone->bb_uv.getBotLeft();

				char_verts[vertex_idx + 1].pos = { 0, -h };
				char_verts[vertex_idx + 1].uv = chara.zone->bb_uv.getTopLeft();

				char_verts[vertex_idx + 2].pos = { w, -h };
				char_verts[vertex_idx + 2].uv = chara.zone->bb_uv.getTopRight();

				char_verts[vertex_idx + 3].pos = { w, 0 };
				char_verts[vertex_idx + 3].uv = chara.zone->bb_uv.getBotRight();

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

		checkErrStack1(chars_vbuff.load(char_verts.data(), char_verts.size() * sizeof(GPU_CharacterVertex)));
		checkErrStack1(chars_idxbuff.load(chars_idxs.data(), chars_idxs.size() * sizeof(uint32_t)));
	}

	// Create Node Instances
	{
		this->clip_mask_id = 0;
		this->char_instance_count = 0;
		this->wrap_instance_count = 0;

		AncestorProps ancestor;
		DescendantProps descendant;
		generateDrawcalls(&nodes.front(), ancestor, descendant);

		calcGlobalPositions(&nodes.front(), { 0, 0 });
	}

	// Index and Load Instances
	{
		this->char_instances.resize(this->char_instance_count);
		this->wrap_instances.resize(this->wrap_instance_count);

		uint32_t char_instance_idx = 0;
		uint32_t wrap_instance_idx = 0;

		for (Node& node : nodes) {

			switch (node.type) {
			case NodeType::TEXT: {
				Text* text = (Text*)node.elem;

				for (CharacterDrawcall& drawcall : text->drawcalls) {

					drawcall.instance_start_idx = char_instance_idx;

					std::memcpy(char_instances.data() + char_instance_idx, drawcall.instances.data(),
						drawcall.instances.size() * sizeof(GPU_CharacterInstance));

					char_instance_idx += drawcall.instances.size();
				}
				break;
			}

			case NodeType::WRAP: {
				Wrap* wrap = (Wrap*)node.elem;

				wrap->drawcall.instance_idx = wrap_instance_idx;

				wrap_instances[wrap_instance_idx] = wrap->drawcall.instance;

				wrap_instance_idx++;
				break;
			}
			}
		}

		if (this->char_instance_count) {
			checkErrStack1(chars_instabuff.load(char_instances.data(), char_instances.size() * sizeof(GPU_CharacterInstance)));
		}
		if (this->wrap_instance_count) {
			checkErrStack1(wrap_instabuff.load(wrap_instances.data(), wrap_instances.size() * sizeof(GPU_WrapInstance)));
		}
	}

	return err_stack;
}

ErrStack Window::draw2()
{
	ErrStack err_stack;
	HRESULT hr = 0;

	if (!rendering_configured) {

		rendering_configured = true;

		// Parents Clip Mask Images
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

			checkHResult(dev5->CreateTexture2D(&desc, NULL, parents_clip_mask_img.GetAddressOf()),
				"failed to create parents clip mask image");

			desc.BindFlags = D3D11_BIND_RENDER_TARGET;
			checkHResult(dev5->CreateTexture2D(&desc, NULL, next_parents_clip_mask_img.GetAddressOf()),
				"failed to create next parents clip mask image");
		}

		// Character Atlas
		{
			TextureAtlas& atlas = instance->char_atlas.atlas;

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

			checkErrStack1(dx11::singleLoad(im_ctx4.Get(), chars_atlas_tex.Get(),
				atlas.colors.data(), sizeof(uint8_t) * atlas.colors.size()));
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

		// Shader Resource View
		{
			checkHResult(dev5->CreateShaderResourceView(parents_clip_mask_img.Get(), NULL,
				parents_clip_mask_srv.GetAddressOf()),
				"failed to create parents clip mask srv");

			checkHResult(dev5->CreateShaderResourceView(chars_atlas_tex.Get(), NULL,
				chars_atlas_srv.GetAddressOf()),
				"failed to create character atlas srv");
		}

		// Render Target Views
		{
			checkHResult(dev5->CreateRenderTargetView(parents_clip_mask_img.Get(), NULL,
				parents_clip_mask_rtv.GetAddressOf()),
				"failed to create parents clip mask rtv");

			checkHResult(dev5->CreateRenderTargetView(next_parents_clip_mask_img.Get(), NULL,
				next_parents_clip_mask_rtv.GetAddressOf()),
				"failed to create next parents clip mask rtv");
		}

		// Vertex Shaders
		{
			FilePath path;
			checkErrStack1(path.recreateRelativeToSolution("UserInterface/CompiledShaders/WrapVS.cso"));
			checkErrStack(path.read(wrap_vs_cso),
				"failed to read wrap VS cso");

			checkHResult(dev5->CreateVertexShader(wrap_vs_cso.data(), wrap_vs_cso.size(),
				NULL, wrap_vs.GetAddressOf()),
				"failed to create wrap VS");

			checkErrStack1(path.recreateRelativeToSolution("UserInterface/CompiledShaders/CharsVS.cso"));
			checkErrStack(path.read(chars_vs_cso),
				"failed to read chars VS cso");

			checkHResult(dev5->CreateVertexShader(chars_vs_cso.data(), chars_vs_cso.size(),
				NULL, chars_vs.GetAddressOf()),
				"failed to create chars VS");
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
				wrap_vs_cso.data(), wrap_vs_cso.size(), wrap_input_layout.GetAddressOf()),
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
				chars_vs_cso.data(), chars_vs_cso.size(), chars_input_layout.GetAddressOf()),
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
			FilePath path;
			checkErrStack1(path.recreateRelativeToSolution("UserInterface/CompiledShaders/WrapPS.cso"));
			checkErrStack(path.read(wrap_ps_cso),
				"failed to read wrap PS cso");

			checkHResult(dev5->CreatePixelShader(wrap_ps_cso.data(), wrap_ps_cso.size(),
				NULL, wrap_ps.GetAddressOf()),
				"failed to create wrap PS");

			checkErrStack1(path.recreateRelativeToSolution("UserInterface/CompiledShaders/CharsPS.cso"));
			checkErrStack(path.read(chars_ps_cso),
				"failed to read chars PS cso");

			checkHResult(dev5->CreatePixelShader(chars_ps_cso.data(), chars_ps_cso.size(),
				NULL, chars_ps.GetAddressOf()),
				"failed to create chars PS");
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
			checkErrStack1(wrap_vbuff.load(wrap_verts.data(), sizeof(GPU_WrapVertex)* wrap_verts.size()));

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
			checkErrStack1(wrap_idxbuff.load(wrap_idxs.data(), sizeof(uint32_t) * wrap_idxs.size()));

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

			DXGI_SWAP_CHAIN_DESC1 swapchain_desc;
			swapchain->GetDesc1(&swapchain_desc);

			GPU_CommonsUniform commons;
			commons.screen_size.x = (float)swapchain_desc.Width;
			commons.screen_size.y = (float)swapchain_desc.Height;
			checkErrStack1(cbuff.load(&commons, sizeof(GPU_CommonsUniform)));
		}

		checkErrStack1(generateGPU_Data());
	}

	// resize
	DXGI_SWAP_CHAIN_DESC1 swapchain_desc;
	swapchain->GetDesc1(&swapchain_desc);
	if (surface_width != swapchain_desc.Width || surface_height != swapchain_desc.Height) {

		present_rtv->Release();
		parents_clip_mask_rtv->Release();
		next_parents_clip_mask_rtv->Release();

		parents_clip_mask_srv->Release();

		present_img->Release();
		parents_clip_mask_img->Release();
		next_parents_clip_mask_img->Release();

		// Images
		checkHResult(swapchain->ResizeBuffers(2, width, height,
			DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH),
			"failed to resize swapchain back buffers");

		checkHResult(swapchain->GetBuffer(0, IID_PPV_ARGS(present_img.GetAddressOf())),
			"failed to get swapchain back buffer");

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

		checkHResult(dev5->CreateTexture2D(&desc, NULL, parents_clip_mask_img.GetAddressOf()),
			"failed to create parents clip mask image");

		desc.BindFlags = D3D11_BIND_RENDER_TARGET;

		checkHResult(dev5->CreateTexture2D(&desc, NULL, next_parents_clip_mask_img.GetAddressOf()),
			"failed to create next parents clip mask image");

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

		Wrap* root = getRoot();
		root->width.setAbsolute((float)surface_width);
		root->height.setAbsolute((float)surface_height);

		checkErrStack1(generateGPU_Data());

		GPU_CommonsUniform commons;
		commons.screen_size.x = (float)surface_width;
		commons.screen_size.y = (float)surface_height;
		checkErrStack1(cbuff.load(&commons, sizeof(GPU_CommonsUniform)));
	}

	// Commands
	float clear_color[4] = {
		0, 0, 0, 0
	};
	im_ctx4->ClearRenderTargetView(present_rtv.Get(), clear_color);
	im_ctx4->ClearRenderTargetView(parents_clip_mask_rtv.Get(), clear_color);
	im_ctx4->ClearRenderTargetView(next_parents_clip_mask_rtv.Get(), clear_color);

	std::vector<Node*> now_nodes = {
		&nodes.front()
	};
	std::vector<Node*> next_nodes;
		
	while (now_nodes.size()) {

		for (Node* node : now_nodes) {

			switch (node->type) {
			case NodeType::TEXT: {
				Text* text = (Text*)node->elem;

				if (!text->drawcalls.size()) {
					break;
				}

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
				{
					D3D11_VIEWPORT viewport = {};
					viewport.TopLeftX = 0;
					viewport.TopLeftY = 0;
					viewport.Width = (float)swapchain_desc.Width;
					viewport.Height = (float)swapchain_desc.Height;
					viewport.MinDepth = 0;
					viewport.MaxDepth = 1;

					im_ctx4->RSSetViewports(1, &viewport);

					im_ctx4->RSSetState(rasterizer_state.Get());
				}

				// Pixel Shader
				std::array<ID3D11ShaderResourceView*, 2> srviews = {
					parents_clip_mask_srv.Get(), chars_atlas_srv.Get()
				};
				im_ctx4->PSSetShaderResources(0, srviews.size(), srviews.data());
				im_ctx4->PSSetSamplers(0, 1, chars_atlas_sampler.GetAddressOf());
				im_ctx4->PSSetShader(chars_ps.Get(), NULL, 0);

				// Output Merger
				im_ctx4->OMSetBlendState(blend_state.Get(), clear_color, 0xFFFF'FFFF);

				std::array<ID3D11RenderTargetView*, 1> rtvs;
				rtvs[0] = present_rtv.Get();
				im_ctx4->OMSetRenderTargets(rtvs.size(), rtvs.data(), NULL);

				for (CharacterDrawcall& drawcall : text->drawcalls) {
					im_ctx4->DrawIndexedInstanced(6, (uint32_t)drawcall.instances.size(),
						drawcall.chara->index_start_idx, 0, drawcall.instance_start_idx);
				}
				break;
			}

			case NodeType::WRAP: {
				Wrap* wrap = (Wrap*)node->elem;

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
				{
					D3D11_VIEWPORT viewport = {};
					viewport.TopLeftX = 0;
					viewport.TopLeftY = 0;
					viewport.Width = (float)swapchain_desc.Width;
					viewport.Height = (float)swapchain_desc.Height;
					viewport.MinDepth = 0;
					viewport.MaxDepth = 1;

					im_ctx4->RSSetViewports(1, &viewport);

					im_ctx4->RSSetState(rasterizer_state.Get());
				}

				// Pixel Shader
				im_ctx4->PSSetShaderResources(0, 1, parents_clip_mask_srv.GetAddressOf());
				im_ctx4->PSSetShader(wrap_ps.Get(), NULL, 0);

				// Output Merger
				im_ctx4->OMSetBlendState(blend_state.Get(), clear_color, 0xFFFF'FFFF);

				std::array<ID3D11RenderTargetView*, 2> rtvs;
				rtvs[0] = present_rtv.Get();
				rtvs[1] = next_parents_clip_mask_rtv.Get();
				im_ctx4->OMSetRenderTargets(rtvs.size(), rtvs.data(), NULL);

				im_ctx4->DrawIndexedInstanced(6, 1, 0, 0, wrap->drawcall.instance_idx);
				break;
			}
			}

			for (Node* child : node->children) {
				next_nodes.push_back(child);
			}
		}

		now_nodes = next_nodes;
		next_nodes.clear();

		im_ctx4->CopyResource(parents_clip_mask_img.Get(), next_parents_clip_mask_img.Get());
		im_ctx4->ClearRenderTargetView(next_parents_clip_mask_rtv.Get(), clear_color);
	}

	checkHResult(swapchain->Present(0, 0),
		"failed to present swapchain");

	return err_stack;
}

Wrap* Window::getRoot()
{
	assert_cond(nodes.front().type == NodeType::WRAP, "");

	return (Wrap*)nodes.front().elem;
}

ErrStack Instance::create()
{
	ErrStack err_stack;

	// Create Character Atlas
	{
		FilePath path;
		checkErrStack1(path.recreateRelativeToSolution("UserInterface/Fonts/Roboto-Regular.ttf"));

		std::vector<uint32_t> sizes = {
			14
		};

		Font* font;
		checkErrStack1(char_atlas.addFont(path, sizes, font));
	}

	return err_stack;
}

ErrStack Instance::createWindow(WindowCrateInfo& info, Window*& r_window)
{
	ErrStack err_stack;
	HRESULT hr = S_OK;

	Window& w = windows.emplace_back();
	w.instance = this;
	w.hinstance = GetModuleHandle(NULL);

	const char CLASS_NAME[] = "Sample Window Class";

	w.window_class = {};
	w.window_class.lpfnWndProc = windowProc;
	w.window_class.hInstance = w.hinstance;
	w.window_class.lpszClassName = CLASS_NAME;

	if (!RegisterClass(&w.window_class)) {
		return ErrStack(code_location, "failed to register window class");
	}

	w.hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		"Window",                       // Window text
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,            // Window style		
		CW_USEDEFAULT, CW_USEDEFAULT, info.width, info.height, // Position and Size
		NULL,       // Parent window    
		NULL,       // Menu
		w.hinstance,  // Instance handle
		NULL        // Additional application data
	);

	if (w.hwnd == NULL) {
		return ErrStack(code_location, "failed to create window");
	}

	w.width = info.width;
	w.height = info.height;

	RECT rect;
	if (!GetClientRect(w.hwnd, &rect)) {
		return ErrStack(code_location, "failed to retrieve window client rect");
	}
	w.surface_width = rect.right - rect.left;
	w.surface_height = rect.bottom - rect.top;

	w.minimized = false;
	w.close = false;
	w.clip_mask_id = 0;
	w.rendering_configured = false;

	w.wrap_verts[0].pos = { 0, 0 };
	w.wrap_verts[1].pos = { 1, 0 };
	w.wrap_verts[2].pos = { 1, 1 };
	w.wrap_verts[3].pos = { 0, 1 };

	w.wrap_idxs = {
		0, 1, 3,
		1, 2, 3
	};

	// Rendering
	{
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

			checkHResult(D3D11CreateDevice(w.adapter,
				D3D_DRIVER_TYPE_UNKNOWN,
				NULL,
				D3D11_CREATE_DEVICE_DEBUG,// | D3D11_CREATE_DEVICE_DEBUGGABLE,
				features.data(), (uint32_t)features.size(),
				D3D11_SDK_VERSION,
				w.device.GetAddressOf(),
				NULL,
				w.im_ctx.GetAddressOf()),
				"failed to create device");

			checkHResult(w.device->QueryInterface<ID3D11Device5>(w.dev5.GetAddressOf()),
				"failed to obtain ID3D11Device5");

			checkHResult(w.im_ctx->QueryInterface<ID3D11DeviceContext4>(w.im_ctx4.GetAddressOf()),
				"failed to obtain ID3D11DeviceContext4");
		}

		// Swapchain
		{
			DXGI_SWAP_CHAIN_DESC1 desc = {};
			desc.Width = 0;
			desc.Height = 0;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.Stereo = false;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			desc.BufferCount = 2;
			desc.Scaling = DXGI_SCALING_NONE;
			desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
			desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC full_desc = {};
			full_desc.RefreshRate.Numerator = 60;
			full_desc.RefreshRate.Denominator = 1;
			full_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			full_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			full_desc.Windowed = true;

			checkHResult(w.factory->CreateSwapChainForHwnd(w.dev5.Get(), w.hwnd,
				&desc, &full_desc, NULL, w.swapchain.GetAddressOf()),
				"failed to create swapchain");
		}

		// Present Target View
		{
			checkHResult(w.swapchain->GetBuffer(0, IID_PPV_ARGS(w.present_img.GetAddressOf())),
				"failed to get swapchain back buffer");

			checkHResult(w.dev5->CreateRenderTargetView(w.present_img.Get(), NULL, w.present_rtv.GetAddressOf()),
				"failed to create present RTV");
		}
	}

	// Root UI Element
	{
		DXGI_SWAP_CHAIN_DESC1 swapchain_desc;
		w.swapchain->GetDesc1(&swapchain_desc);

		Node& new_node = w.nodes.emplace_back();
		new_node.parent = nullptr;

		Wrap* root = new_node.createWrap();
		root->node_comp.window = &w;
		root->node_comp.this_elem = &new_node;

		// Default Values
		root->pos = { 0, 0 };
		root->width.setAbsolute((float)swapchain_desc.Width);
		root->height.setAbsolute((float)swapchain_desc.Height);
		root->overflow = Overflow::CLIP;
		root->background_color = { 0, 0, 0, 1 };
	}

	r_window = &w;
	return ErrStack();
}

ErrStack Instance::update()
{
	ErrStack err_stack;

	// Input
	for (Window& window : windows) {

		MSG msg{};
		while (PeekMessage(&msg, window.hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		/*if (!window.minimized) {
			checkErrStack1(window.draw());
		}*/

		checkErrStack1(window.draw2());
	}
	
	return err_stack;
}

Instance::~Instance()
{
	windows.clear();
}
