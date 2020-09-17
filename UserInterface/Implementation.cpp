
#include "pch.h"

// Mine
#include "FileIO.hpp"
#include "GPU_ShaderTypes.hpp"

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
				new_instance.color = text->color;
				new_instance.pos = new_pos;
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
		inst.color = wrap->background_color;

		if (parent != nullptr) {	
			inst.pos = wrap->pos;
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
				inst.pos += pos;
			}
		}
		break;
	}

	case NodeType::WRAP: {
		Wrap* wrap = (Wrap*)node->elem;

		wrap->drawcall.instance.pos += pos;
		for (Node* child : node->children) {
			calcGlobalPositions(child, wrap->drawcall.instance.pos);
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
		this->char_idxs.resize(index_count);

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

				char_idxs[index_idx + 0] = vertex_idx + 0;
				char_idxs[index_idx + 1] = vertex_idx + 1;
				char_idxs[index_idx + 2] = vertex_idx + 2;

				char_idxs[index_idx + 3] = vertex_idx + 2;
				char_idxs[index_idx + 4] = vertex_idx + 3;
				char_idxs[index_idx + 5] = vertex_idx + 0;

				vertex_idx += 4;
				index_idx += 6;
			}
		}

		checkErrStack1(chars_vbuff.load(char_verts.data(), char_verts.size() * sizeof(GPU_CharacterVertex)));
		checkErrStack1(chars_idxbuff.load(char_idxs.data(), char_idxs.size() * sizeof(uint32_t)));
	}

	// Create Wrap Mesh
	{
		checkErrStack1(wrap_vbuff.load(wrap_verts.data(), wrap_verts.size() * sizeof(GPU_WrapVertex)));
		checkErrStack1(wrap_idxbuff.load(wrap_idxs.data(), wrap_idxs.size() * sizeof(uint32_t)));
	}

	// Create Node Instances
	{
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

ErrStack Window::draw()
{
	ErrStack err_stack;

	if (!rendering_configured) {

		rendering_configured = true;

		// Characters Vertex Buffer
		{
			vkw::BufferCreateInfo info;
			info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			dev.createBuffer(info, chars_vbuff);
		}

		// Characters Index Buffer
		{
			vkw::BufferCreateInfo info;
			info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			dev.createBuffer(info, chars_idxbuff);
		}

		// Character Instance Buffer
		{
			vkw::BufferCreateInfo info;
			info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			dev.createBuffer(info, chars_instabuff);
		}

		// Wrap Vertex Buffer
		{
			vkw::BufferCreateInfo info;
			info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			dev.createBuffer(info, wrap_vbuff);
		}

		// Wrap Index Buffer
		{
			vkw::BufferCreateInfo info;
			info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			dev.createBuffer(info, wrap_idxbuff);
		}

		// Wrap Instance Buffer
		{
			vkw::BufferCreateInfo info;
			info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			dev.createBuffer(info, wrap_instabuff);
		}

		// Character Uniform Buffer
		{
			vkw::BufferCreateInfo info;
			info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			dev.createBuffer(info, text_ubuff);

			common_uniform.screen_size.x = (float)dev.surface.width;
			common_uniform.screen_size.y = (float)dev.surface.height;
			checkErrStack1(text_ubuff.load(&common_uniform, sizeof(GPU_CommonsUniform)));
		}

		// Composition Image
		{
			vkw::ImageCreateInfo info;
			info.format = dev.surface.imageFormat;
			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
			checkErrStack(dev.createImage(info, composition_img),
				"failed to create composition image");
		}

		// Parents Clip Image
		{
			vkw::ImageCreateInfo info;
			info.format = VK_FORMAT_R32_UINT;
			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
			checkErrStack1(dev.createImage(info, parents_clip_mask_img));

			vkw::ImageViewCreateInfo view_info;
			checkErrStack1(parents_clip_mask_img.createView(view_info, parents_clip_mask_view));
		}

		// Next Parents Clip Image
		{
			vkw::ImageCreateInfo info;
			info.format = VK_FORMAT_R32_UINT;
			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
			checkErrStack1(dev.createImage(info, next_parents_clip_mask_img));

			vkw::ImageViewCreateInfo view_info;
			checkErrStack1(next_parents_clip_mask_img.createView(view_info, next_parents_clip_mask_view));
		}

		// Character Atlas Texture
		{
			vkw::ImageCreateInfo info;
			info.format = VK_FORMAT_R8_UNORM;
			info.width = instance->char_atlas.atlas.tex_size;
			info.height = instance->char_atlas.atlas.tex_size;
			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			checkErrStack(dev.createImage(info, char_atlas_tex),
				"failed to create character atlas texture");

			vkw::ImageViewCreateInfo view_info;
			checkErrStack(char_atlas_tex.createView(view_info, char_atlas_view),
				"failed to create character atlas view");

			TextureAtlas& char_atlas = instance->char_atlas.atlas;
			checkErrStack(char_atlas_view.load(char_atlas.colors.data(), char_atlas.colors.size(), VK_IMAGE_LAYOUT_GENERAL),
				"failed to load character atlas data into texture");
		}

		// Composition Image View
		{
			vkw::ImageViewCreateInfo info;
			checkErrStack(composition_img.createView(info, composition_view),
				"failed to create composition view");
		}

		// Sampler
		{
			vkw::SamplerCreateInfo info;
			checkErrStack(dev.createSampler(info, text_sampler),
				"failed to create sampler");
		}

		checkErrStack1(generateGPU_Data());

		std::vector<char> spirv;

		// Character Vertex Shader
		{
			FilePath path;
			checkErrStack1(path.recreateRelativeToSolution("UserInterface/Shaders/Text/vert.spv"));

			spirv.clear();
			checkErrStack1(path.read(spirv));

			checkErrStack1(dev.createShader(spirv, VK_SHADER_STAGE_VERTEX_BIT, text_vs));
		}

		// Character Fragment Shader
		{
			FilePath path;
			checkErrStack1(path.recreateRelativeToSolution("UserInterface/Shaders/Text/frag.spv"));

			spirv.clear();
			checkErrStack1(path.read(spirv));

			checkErrStack1(dev.createShader(spirv, VK_SHADER_STAGE_FRAGMENT_BIT, text_fs));
		}

		// Wrap Vertex Shader
		{
			FilePath path;
			checkErrStack1(path.recreateRelativeToSolution("UserInterface/Shaders/Wrap/vert.spv"));

			spirv.clear();
			checkErrStack1(path.read(spirv));

			checkErrStack1(dev.createShader(spirv, VK_SHADER_STAGE_VERTEX_BIT, wrap_vs));
		}

		// Wrap Fragment Shader
		{
			FilePath path;
			checkErrStack1(path.recreateRelativeToSolution("UserInterface/Shaders/Wrap/frag.spv"));

			spirv.clear();
			checkErrStack1(path.read(spirv));

			checkErrStack1(dev.createShader(spirv, VK_SHADER_STAGE_FRAGMENT_BIT, wrap_fs));
		}

		// Text Pass
		{
			dev.createDrawpass(text_pass);

			vkw::CombinedImageSamplerBinding sampler_info;
			text_pass.bindCombinedImageSampler(sampler_info);

			vkw::UniformBufferBinding ubuff_info;
			ubuff_info.set = 1;
			text_pass.bindUniformBuffer(ubuff_info);

			vkw::ReadAttachmentInfo parents_clip_info;
			parents_clip_info.format = parents_clip_mask_img.format;
			parents_clip_info.set = 2;
			text_pass.addReadColorAttachment(parents_clip_info);

			vkw::WriteAttachmentInfo compose_info;
			compose_info.format = composition_img.format;
			compose_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
			compose_info.blendEnable = true;
			text_pass.addWriteColorAttachment(compose_info);

			text_pass.vertex_inputs.push_back(GPU_CharacterVertex::getVertexInput());
			text_pass.vertex_inputs.push_back(GPU_CharacterInstance::getVertexInput(1));

			text_pass.setVertexShader(text_vs);

			text_pass.setFragmentShader(text_fs);

			checkErrStack1(text_pass.build());
		}

		// Text Pass Descriptors
		{
			vkw::CombinedImageSamplerDescriptor sampler_info;
			sampler_info.sampler = &text_sampler;
			sampler_info.view = &char_atlas_view;
			text_pass.updateCombinedImageSamplerDescriptor(0, 0, sampler_info);

			vkw::UpdateBufferDescriptor ubuff_info;
			ubuff_info.buffer = &text_ubuff;
			text_pass.updateUniformBufferDescriptor(1, 0, ubuff_info);

			vkw::InputAttachmentDescriptor parents_clip_info;
			parents_clip_info.view = &parents_clip_mask_view;
			text_pass.updateInputAttachmentDescriptor(2, 0, parents_clip_info);
		}

		// Text Framebuffer
		{
			vkw::FramebufferCreateInfo info;
			info.attachments = {
				&parents_clip_mask_view,
				&composition_view
			};
			checkErrStack1(text_pass.createFramebuffer(info, text_framebuff));
		}

		// Wrap Pass
		{
			dev.createDrawpass(wrap_pass);

			vkw::UniformBufferBinding ubuff_info;
			wrap_pass.bindUniformBuffer(ubuff_info);

			vkw::ReadAttachmentInfo parents_clip_info;
			parents_clip_info.format = parents_clip_mask_img.format;
			parents_clip_info.set = 1;
			wrap_pass.addReadColorAttachment(parents_clip_info);

			vkw::WriteAttachmentInfo compose_info;
			compose_info.format = composition_img.format;
			compose_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
			wrap_pass.addWriteColorAttachment(compose_info);

			vkw::WriteAttachmentInfo next_parents_clip_info;
			next_parents_clip_info.format = next_parents_clip_mask_img.format;
			next_parents_clip_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
			wrap_pass.addWriteColorAttachment(next_parents_clip_info);

			wrap_pass.vertex_inputs.push_back(GPU_WrapVertex::getVertexInput());
			wrap_pass.vertex_inputs.push_back(GPU_WrapInstance::getVertexInput(1));

			wrap_pass.setVertexShader(wrap_vs);

			wrap_pass.setFragmentShader(wrap_fs);

			checkErrStack1(wrap_pass.build());
		}

		// Wrap Pass Descriptors
		{
			vkw::UpdateBufferDescriptor ubuff_info;
			ubuff_info.buffer = &text_ubuff;
			wrap_pass.updateUniformBufferDescriptor(0, 0, ubuff_info);

			vkw::InputAttachmentDescriptor parents_clip_info;
			parents_clip_info.view = &parents_clip_mask_view;
			wrap_pass.updateInputAttachmentDescriptor(1, 0, parents_clip_info);
		}

		// Wrap Pass Framebuffer
		{
			vkw::FramebufferCreateInfo info;
			info.attachments = {
				&parents_clip_mask_view,
				&composition_view,
				&next_parents_clip_mask_view
			};
			checkErrStack1(wrap_pass.createFramebuffer(info, wrap_framebuff));
		}

		// Command List
		{
			vkw::CommandListCreateInfo info = {};
			info.surface = &dev.surface;

			checkErrStack1(dev.createCommandList(info, cmd_list));
			checkErrStack1(cmd_list.beginRecording());

			cmd_list.cmdClearFloatImage(composition_view);
			cmd_list.cmdClearUIntImage(parents_clip_mask_view);
			cmd_list.cmdClearUIntImage(next_parents_clip_mask_view);
			{
				std::array<vkw::ImageView*, 3> views = {
					&composition_view, &parents_clip_mask_view, &next_parents_clip_mask_view
				};
				cmd_list.cmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, (VkAccessFlagBits)0, views.size(), views.data());
			}

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

						cmd_list.cmdBeginRenderpass(text_pass, text_framebuff);
						{
							cmd_list.cmdBindVertexBuffers(chars_vbuff, chars_instabuff);
							cmd_list.cmdBindIndexBuffer(chars_idxbuff);

							for (CharacterDrawcall& drawcall : text->drawcalls) {
								cmd_list.cmdDrawIndexedInstanced(6, (uint32_t)drawcall.instances.size(),
									drawcall.chara->index_start_idx, drawcall.instance_start_idx);

							}
						}
						cmd_list.cmdEndRenderpass();
						break;
					}

					case NodeType::WRAP: {
						Wrap* wrap = (Wrap*)node->elem;

						cmd_list.cmdBeginRenderpass(wrap_pass, wrap_framebuff);
						{
							cmd_list.cmdBindVertexBuffers(wrap_vbuff, wrap_instabuff);
							cmd_list.cmdBindIndexBuffer(wrap_idxbuff);

							cmd_list.cmdDrawIndexedInstanced(6, 1, 0, wrap->drawcall.instance_idx);
						}
						cmd_list.cmdEndRenderpass();
						break;
					}
					}

					// TODO: barrier, wait for writes to complete for composition, and next parents

					for (Node* child : node->children) {
						next_nodes.push_back(child);
					}
				}

				now_nodes = next_nodes;
				next_nodes.clear();

				cmd_list.cmdCopyImageToImage(next_parents_clip_mask_view, parents_clip_mask_view);
				cmd_list.cmdClearUIntImage(next_parents_clip_mask_view);
			}	

			// Copy Compose Image to Surface
			{
				vkw::SurfaceBarrier dst;
				dst.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				dst.wait_for_access = 0;
				dst.wait_at_access = VK_ACCESS_TRANSFER_WRITE_BIT;
				cmd_list.cmdSurfacePipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, dst);

				cmd_list.cmdCopyImageToSurface(composition_view);

				dst = {};
				dst.new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				dst.wait_for_access = VK_ACCESS_TRANSFER_WRITE_BIT;
				dst.wait_at_access = 0;
				cmd_list.cmdSurfacePipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					dst);
			}

			checkErrStack1(cmd_list.endRecording());
		}
	}
	else {
		if (width != dev.surface.width || height != dev.surface.height) {

			// resize images and their views
			// resize renderpass
			// recreate c
		}
	}

	checkErrStack1(cmd_list.run());
	checkErrStack1(cmd_list.waitForExecution());

	return ErrStack();
}

ErrStack Window::draw2()
{
	ErrStack err_stack;
	HRESULT hr = 0;

	if (!rendering_configured) {

		rendering_configured = true;

		// Factory
		{
			checkHResult(CreateDXGIFactory2(0, IID_PPV_ARGS(factory.GetAddressOf())),
				"failed to create DXGI factory");
		}

		// Choose Adapter with most VRAM
		{
			uint32_t adapter_idx = 0;
			IDXGIAdapter* found_adapter;
			size_t adapter_vram = 0;

			while (SUCCEEDED(factory.Get()->EnumAdapters(adapter_idx, &found_adapter))) {

				DXGI_ADAPTER_DESC desc;
				checkHResult(found_adapter->GetDesc(&desc),
					"failed to get adapter description");

				if (desc.DedicatedVideoMemory > adapter_vram) {
					adapter = found_adapter;
				}
				adapter_idx++;
			}
		}

		// Device
		{
			std::array<D3D_FEATURE_LEVEL, 1> features = {
				D3D_FEATURE_LEVEL_11_1
			};

			checkHResult(D3D11CreateDevice(adapter,
				D3D_DRIVER_TYPE_UNKNOWN,
				NULL,
				D3D11_CREATE_DEVICE_DEBUG,// | D3D11_CREATE_DEVICE_DEBUGGABLE,
				features.data(), (uint32_t)features.size(),
				D3D11_SDK_VERSION,
				device.GetAddressOf(),
				NULL,
				im_ctx.GetAddressOf()),
				"failed to create device");

				checkHResult(device->QueryInterface<ID3D11Device5>(dev5.GetAddressOf()),
					"failed to obtain ID3D11Device5");

				checkHResult(im_ctx->QueryInterface<ID3D11DeviceContext4>(im_ctx4.GetAddressOf()),
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

		checkHResult(factory->CreateSwapChainForHwnd(dev5.Get(), hwnd,
			&desc, &full_desc, NULL, swapchain.GetAddressOf()),
			"failed to create swapchain");
		}

		// Present Target View
		{
			checkHResult(swapchain->GetBuffer(0, IID_PPV_ARGS(present_img.GetAddressOf())),
				"failed to get swapchain back buffer");

			checkHResult(dev5->CreateRenderTargetView(present_img.Get(), NULL, present_rtv.GetAddressOf()),
				"failed to create present RTV");
		}

		// Vertex Shader
		{
			FilePath path;
			checkErrStack1(path.recreateRelativeToSolution("UserInterface/CompiledShaders/VertexShader.cso"));
			checkErrStack(path.read(vertex_shader_cso),
				"failed to read shader code");

			checkHResult(dev5->CreateVertexShader(vertex_shader_cso.data(), vertex_shader_cso.size(),
				NULL, vertex_shader.GetAddressOf()),
				"failed to create shader");
		}

		// Pixel Shader
		{
			FilePath path;
			checkErrStack1(path.recreateRelativeToSolution("UserInterface/CompiledShaders/PixelShader.cso"));
			checkErrStack(path.read(pixel_shader_cso),
				"failed to read shader code");

			checkHResult(dev5->CreatePixelShader(pixel_shader_cso.data(), pixel_shader_cso.size(),
				NULL, pixel_shader.GetAddressOf()),
				"failed to create shader");
		}

		// Input Layout
		{
			auto input_elems = GPU_Vertex::getInputLayout();
			checkHResult(dev5->CreateInputLayout(input_elems.data(), input_elems.size(),
				vertex_shader_cso.data(), vertex_shader_cso.size(), vertex_il.GetAddressOf()),
				"failed to create input layout");
		}

		// Vertex Buffer
		{
			//D3D11_BUFFER_DESC desc = {};
			//desc.ByteWidth;
			//desc.Usage;

			//UINT ByteWidth;
			//D3D11_USAGE Usage;
			//UINT BindFlags;
			//UINT CPUAccessFlags;
			//UINT MiscFlags;
			//UINT StructureByteStride;

			//dev5->CreateBuffer()
		}
	}

	// Commands
	{
		float present_clear_color[4] = {
				1, 0, 0, 1
		};
		im_ctx4->ClearRenderTargetView(present_rtv.Get(), present_clear_color);
	}
	
	/*{
		DXGI_SWAP_CHAIN_DESC1 swapchain_desc;
		swapchain->GetDesc1(&swapchain_desc);

		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)swapchain_desc.Width;
		viewport.Height = (float)swapchain_desc.Height;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;


	}*/

	im_ctx4->OMSetRenderTargets(1, present_rtv.GetAddressOf(), NULL);

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

	// Vulkan
	vkw::InstanceCreateInfo info;
	checkErrStack(inst.create(info),
		"failed to create vulkan instance");

	// DirectX 11

	return err_stack;
}

ErrStack Instance::createWindow(WindowCrateInfo& info, Window*& r_window)
{
	ErrStack err_stack{};

	Window& window = windows.emplace_back();
	window.instance = this;

	window.hinstance = GetModuleHandle(NULL);

	const char CLASS_NAME[] = "Sample Window Class";

	WNDCLASS wc = {};
	wc.lpfnWndProc = windowProc;
	wc.hInstance = window.hinstance;
	wc.lpszClassName = CLASS_NAME;

	if (!RegisterClass(&wc)) {
		return ErrStack(code_location, "failed to register window class");
	}

	window.hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		"Window",                       // Window text
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,            // Window style		
		CW_USEDEFAULT, CW_USEDEFAULT, info.width, info.height, // Position and Size
		NULL,       // Parent window    
		NULL,       // Menu
		window.hinstance,  // Instance handle
		NULL        // Additional application data
	);

	if (window.hwnd == NULL) {
		return ErrStack(code_location, "failed to create window");
	}

	window.minimized = false;
	window.close = false;
	window.clip_mask_id = 0;
	window.rendering_configured = false;

	window.wrap_verts[0].pos = { 0, 0 };
	window.wrap_verts[1].pos = { 1, 0 };
	window.wrap_verts[2].pos = { 1, 1 };
	window.wrap_verts[3].pos = { 0, 1 };

	window.wrap_idxs = {
		0, 1, 3,
		1, 2, 3
	};

	// Rendering
	{
		vkw::DeviceCreateInfo dev_info;
		dev_info.hinstance = window.hinstance;
		dev_info.hwnd = window.hwnd;
		dev_info.features.independentBlend = VK_TRUE;

		checkErrStack(inst.createDevice(dev_info, window.dev),
			"failed to create device");
	}

	// Root UI Element
	{
		Node& new_node = window.nodes.emplace_back();
		new_node.parent = nullptr;

		Wrap* root = new_node.createWrap();
		root->node_comp.window = &window;
		root->node_comp.this_elem = &new_node;

		// Default Values
		root->pos = { 0, 0 };
		root->width.setAbsolute((float)window.dev.surface.width);
		root->height.setAbsolute((float)window.dev.surface.height);
		root->overflow = Overflow::CLIP;
		root->background_color = { 0, 0, 0, 1 };
	}

	r_window = &window;
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
