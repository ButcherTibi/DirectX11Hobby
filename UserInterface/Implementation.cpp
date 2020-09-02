
#include "pch.h"

// Mine
#include "FileIO.hpp"

// Header
#include "NuiLibrary.h"


using namespace nui;

std::list<Window> nui::windows;


Text* NodeComponent::addText()
{
	Node& child_node = window->nodes.emplace_back();

	Text* new_text = child_node.createText();
	new_text->node_comp.window = this->window;
	new_text->node_comp.this_elem = &child_node;
	new_text->node_comp.elem_id = window->node_unique_id;
	window->node_unique_id++;

	new_text->size = 14.0f;
	new_text->line_height = 1.15f;
	new_text->color = { 1, 1, 1, 1 };

	// Parent ---> Child
	this->this_elem->children.push_back(&child_node);

	return new_text;
}

Wrap* NodeComponent::addWrap()
{
	Node& child_node = window->nodes.emplace_back();

	Wrap* child_wrap = child_node.createWrap();
	child_wrap->node_comp.window = this->window;
	child_wrap->node_comp.this_elem = &child_node;
	child_wrap->node_comp.elem_id = window->node_unique_id;
	window->node_unique_id++;

	child_wrap->width = 100;
	child_wrap->height = 100;
	child_wrap->overflow = Overflow::OVERFLOw;
	child_wrap->background_color = { 0, 0, 0, 0 };

	// Parent ---> Child
	this->this_elem->children.push_back(&child_node);

	return child_wrap;
}

Flex* NodeComponent::addFlex()
{
	Node& child_node = window->nodes.emplace_back();

	Flex* child_flex = child_node.createFlex();
	child_flex->node_comp.window = this->window;

	return child_flex;
}


Text* Root::addText()
{
	return node_comp.addText();
}

Wrap* Root::addWrap()
{
	return node_comp.addWrap();
}

Flex* Root::addFlex()
{
	return node_comp.addFlex();
}

Text* Wrap::addText()
{
	return node_comp.addText();
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

Root* Node::createRoot()
{
	this->type = NodeType::ROOT;
	this->elem = new Root();
	return static_cast<Root*>(elem);
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
		case nui::NodeType::ROOT:
			delete (Root*)elem;
			break;

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

ErrStack Window::generateGPU_CharacterData()
{
	ErrStack err_stack;

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
		for (Character& chara: font_size.chars) {

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

	uint32_t instance_count = 0;
	
	for (Node& node : nodes) {

		switch (node.type) {
		case NodeType::TEXT: {
			Text* text = (Text*)node.elem;

			//text->generateDrawcalls();

			FontSize* font_size = nullptr;
			float min_diff = 9999;

			// Choose most similar font size
			for (FontSize& fsize : text->node_comp.window->instance->char_atlas.fonts[0].sizes) {

				float diff = abs(text->size - fsize.size);
				if (diff < min_diff) {

					font_size = &fsize;
					min_diff = diff;
				}
			}

			glm::vec2 pen = text->pos;

			for (uint32_t unicode : text->text) {

				float scale = text->size / font_size->size;

				switch (unicode) {
				case 0x000A:  // newline
					pen.x = text->pos.x;
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
					new_instance.elem_id = text->node_comp.elem_id;

					pen.x += drawcall->chara->advance_X * scale;
				}
			}

			instance_count += text->text.size();
			break;
		}
		}
	}

	this->char_instances.resize(instance_count);
	uint32_t instance_idx = 0;

	for (Node& node : nodes) {

		switch (node.type) {
		case NodeType::TEXT: {
			Text* text = (Text*)node.elem;

			for (CharacterDrawcall& drawcall : text->drawcalls) {

				drawcall.instance_start_idx = instance_idx;

				std::memcpy(char_instances.data() + instance_idx, drawcall.instances.data(),
					drawcall.instances.size() * sizeof(GPU_CharacterInstance));

				instance_idx += drawcall.instances.size();
			}
			break;
		}
		}
	}

	if (instance_count) {
		checkErrStack1(chars_instabuff.load(char_instances.data(), char_instances.size() * sizeof(GPU_CharacterInstance)));
	}

	return ErrStack();
}

ErrStack Window::generateGPU_WrapData()
{
	ErrStack err_stack;
	checkErrStack1(wrap_vbuff.load(wrap_verts.data(), wrap_verts.size() * sizeof(GPU_WrapVertex)));
	checkErrStack1(wrap_idxbuff.load(wrap_idxs.data(), wrap_idxs.size() * sizeof(uint32_t)));

	uint32_t instance_count = 0;
	for (Node& node : nodes) {

		switch (node.type) {
		case NodeType::WRAP: {
			instance_count++;
			break;
		}
		}
	}

	this->wrap_instances.resize(instance_count);
	uint32_t instance_idx = 0;
	for (Node& node : nodes) {

		switch (node.type) {
		case NodeType::WRAP: {
			
			Wrap* wrap = (Wrap*)node.elem;
			wrap->drawcall.instance_idx = instance_idx;

			auto& wrap_instance = wrap_instances[instance_idx];
			wrap_instance.pos = wrap->pos;
			wrap_instance.size = {wrap->width, wrap->height};
			wrap_instance.color = wrap->background_color;
			wrap_instance.elem_id = wrap->node_comp.elem_id;

			instance_idx++;
			break;
		}
		}
	}

	if (instance_count) {
		checkErrStack1(wrap_instabuff.load(wrap_instances.data(), wrap_instances.size() * sizeof(GPU_WrapInstance)));
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

		checkErrStack1(generateGPU_CharacterData());

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

		checkErrStack1(generateGPU_WrapData());

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

		// Elem Image
		{
			vkw::ImageCreateInfo info;
			info.format = VK_FORMAT_R8G8B8A8_UNORM;
			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
			checkErrStack1(dev.createImage(info, elem_img));

			vkw::ImageViewCreateInfo view_info;
			checkErrStack1(elem_img.createView(view_info, elem_view));
		}

		// Elem Mask Image
		{
			vkw::ImageCreateInfo info;
			info.format = VK_FORMAT_R32_UINT;
			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
			checkErrStack1(dev.createImage(info, elem_mask_img));

			vkw::ImageViewCreateInfo view_info;
			checkErrStack1(elem_mask_img.createView(view_info, elem_mask_view));
		}

		// Parents Id Image
		{
			vkw::ImageCreateInfo info;
			info.format = VK_FORMAT_R8G8B8A8_UINT;
			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
			checkErrStack1(dev.createImage(info, parents_id_img));

			vkw::ImageViewCreateInfo view_info;
			checkErrStack1(parents_id_img.createView(view_info, parents_id_view));
		}

		// Next Parents Id Image
		{
			vkw::ImageCreateInfo info;
			info.format = VK_FORMAT_R8G8B8A8_UINT;
			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
			checkErrStack1(dev.createImage(info, next_parents_id_img));

			vkw::ImageViewCreateInfo view_info;
			checkErrStack1(next_parents_id_img.createView(view_info, next_parents_id_view));
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
			sampler_info.sampler = &text_sampler;
			sampler_info.tex_view = &char_atlas_view;
			text_pass.bindCombinedImageSampler(sampler_info);

			vkw::UniformBufferBinding ubuff_info;
			ubuff_info.buff = &text_ubuff;
			ubuff_info.set = 1;
			text_pass.bindUniformBuffer(ubuff_info);

			vkw::WriteAttachmentInfo elem_info;
			elem_info.view = &elem_view;
			elem_info.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
			vkw::clearColorFloatValue(elem_info.clear_value);
			text_pass.addWriteColorAttachment(elem_info);

			vkw::WriteAttachmentInfo elem_mask_info;
			elem_mask_info.view = &elem_mask_view;
			elem_mask_info.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
			vkw::clearColorUIntValue(elem_mask_info.clear_value);
			text_pass.addWriteColorAttachment(elem_mask_info);

			text_pass.vertex_inputs.push_back(GPU_CharacterVertex::getVertexInput());
			text_pass.vertex_inputs.push_back(GPU_CharacterInstance::getVertexInput(1));

			text_pass.setVertexShader(text_vs);

			text_pass.setFragmentShader(text_fs);

			checkErrStack1(text_pass.build());
		}

		// Wrap Pass
		{
			dev.createDrawpass(wrap_pass);

			vkw::UniformBufferBinding ubuff_info;
			ubuff_info.buff = &text_ubuff;
			wrap_pass.bindUniformBuffer(ubuff_info);

			vkw::WriteAttachmentInfo elem_info;
			elem_info.view = &elem_view;
			elem_info.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
			vkw::clearColorFloatValue(elem_info.clear_value);
			wrap_pass.addWriteColorAttachment(elem_info);

			vkw::WriteAttachmentInfo elem_mask_info;
			elem_mask_info.view = &elem_mask_view;
			elem_mask_info.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
			vkw::clearColorUIntValue(elem_mask_info.clear_value);
			wrap_pass.addWriteColorAttachment(elem_mask_info);

			wrap_pass.vertex_inputs.push_back(GPU_WrapVertex::getVertexInput());
			wrap_pass.vertex_inputs.push_back(GPU_WrapInstance::getVertexInput(1));

			wrap_pass.setVertexShader(wrap_vs);

			wrap_pass.setFragmentShader(wrap_fs);

			checkErrStack1(wrap_pass.build());
		}

		// Command List
		{
			vkw::CommandListCreateInfo info = {};
			info.surface = &dev.surface;

			checkErrStack1(dev.createCommandList(info, cmd_list));
			checkErrStack1(cmd_list.beginRecording());

			// Clear Compose Image
			{
				vkw::ImageBarrier bar;
				bar.view = &composition_view;
				bar.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				bar.wait_for_access = 0;
				bar.wait_at_access = VK_ACCESS_TRANSFER_WRITE_BIT;
				cmd_list.cmdPipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, bar);

				cmd_list.cmdClearFloatImage(composition_view);

				bar = {};
				bar.view = &composition_view;
				bar.new_layout = VK_IMAGE_LAYOUT_GENERAL;
				bar.wait_for_access = VK_ACCESS_TRANSFER_WRITE_BIT;
				bar.wait_at_access = 0;
				cmd_list.cmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, bar);
			}

			//  Clear Parents Id Image
			{
				vkw::ImageBarrier bar;
				bar.view = &parents_id_view;
				bar.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				bar.wait_for_access = 0;
				bar.wait_at_access = VK_ACCESS_TRANSFER_WRITE_BIT;
				cmd_list.cmdPipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, bar);

				cmd_list.cmdClearUIntImage(parents_id_view, 0);

				bar = {};
				bar.view = &parents_id_view;
				bar.new_layout = VK_IMAGE_LAYOUT_GENERAL;
				bar.wait_for_access = VK_ACCESS_TRANSFER_WRITE_BIT;
				bar.wait_at_access = 0;
				cmd_list.cmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, bar);
			}

			// Clear Next Parents Id Image
			{
				vkw::ImageBarrier bar;
				bar.view = &next_parents_id_view;
				bar.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				bar.wait_for_access = 0;
				bar.wait_at_access = VK_ACCESS_TRANSFER_WRITE_BIT;
				cmd_list.cmdPipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, bar);

				cmd_list.cmdClearUIntImage(next_parents_id_view, 0);

				bar = {};
				bar.view = &next_parents_id_view;
				bar.new_layout = VK_IMAGE_LAYOUT_GENERAL;
				bar.wait_for_access = VK_ACCESS_TRANSFER_WRITE_BIT;
				bar.wait_at_access = 0;
				cmd_list.cmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, bar);
			}

			// render root
			// elem mask image clear to root

			// Text/Wrap/XXX Pass
			// draw elem -> elem img
			//           -> elem_mask img

			// Clip Pass
			// in: elem img, elem_mask img, parents_mask img, next_parents_mask
			// out: compose img
			// if (elem_mask == parents_mask) then
			//   compose img = elem_mask(elem_img)
			//   next_parents_mask = elem_mask

			// at end of layer parents_mask img = next_parents_mask img,
			// next_parents_mask clear to blank

			std::vector<Node*> now_nodes;
			std::vector<Node*> next_nodes;

			// Render the root
			{
				// Root* root = reinterpret_cast<Root*>(&nodes.front().elem);

				// nothing yet to render maybe background color

				for (Node* child : nodes.front().children) {
					now_nodes.push_back(child);
				}
			}

			while (now_nodes.size()) {

				for (Node* node : now_nodes) {

					switch (node->type) {
					case NodeType::TEXT: {
						Text* text = (Text*)node->elem;

						cmd_list.cmdBeginRenderpass(text_pass);
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

						cmd_list.cmdBeginRenderpass(wrap_pass);
						{
							cmd_list.cmdBindVertexBuffers(wrap_vbuff, wrap_instabuff);
							cmd_list.cmdBindIndexBuffer(wrap_idxbuff);

							cmd_list.cmdDrawIndexedInstanced(6, 1, 0, wrap->drawcall.instance_idx);
						}
						cmd_list.cmdEndRenderpass();
						break;
					}
					}

					for (Node* child : node->children) {
						next_nodes.push_back(child);
					}
				}

				now_nodes.clear();
				now_nodes = next_nodes;
				next_nodes.clear();
			}	

			// Copy Compose Image to Surface
			{
				vkw::ImageBarrier src;
				src.view = &composition_view;
				src.new_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				src.wait_for_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				src.wait_at_access = VK_ACCESS_TRANSFER_READ_BIT;
				cmd_list.cmdPipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, src);

				vkw::SurfaceBarrier dst;
				dst.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				dst.wait_for_access = 0;
				dst.wait_at_access = VK_ACCESS_TRANSFER_WRITE_BIT;
				cmd_list.cmdSurfacePipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, dst);

				cmd_list.cmdCopyImageToSurface(composition_view);

				src = {};
				src.view = &composition_view;
				src.new_layout = VK_IMAGE_LAYOUT_GENERAL;
				src.wait_for_access = VK_ACCESS_TRANSFER_READ_BIT;
				src.wait_at_access = 0;
				cmd_list.cmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, src);

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

	checkErrStack1(cmd_list.run());
	checkErrStack1(cmd_list.waitForExecution());

	return ErrStack();
}

Root* Window::getRoot()
{
	assert_cond(nodes.front().type == NodeType::ROOT, "");

	return (Root*)nodes.front().elem;
}

ErrStack Instance::create()
{
	ErrStack err_stack{};

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

	vkw::InstanceCreateInfo info;
	checkErrStack(inst.create(info),
		"failed to create vulkan instance");

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

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, info.width, info.height,

		NULL,       // Parent window    
		NULL,       // Menu
		window.hinstance,  // Instance handle
		NULL        // Additional application data
	);

	if (window.hwnd == NULL) {
		return ErrStack(code_location, "failed to create window");
	}

	//ShowWindow(window.hwnd, 0);

	window.minimized = false;
	window.close = false;
	window.node_unique_id = 0;
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

		checkErrStack(inst.createDevice(dev_info, window.dev),
			"failed to create device");
	}

	// Root UI Element
	{
		Node& new_node = window.nodes.emplace_back();

		Root* root = new_node.createRoot();
		root->node_comp.window = &window;
		root->node_comp.elem_id = window.node_unique_id;
		root->node_comp.this_elem = &new_node;

		window.node_unique_id++;
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

		if (!window.minimized) {
			checkErrStack1(window.draw());
		}
	}
	
	return err_stack;
}

Instance::~Instance()
{
	windows.clear();
}
