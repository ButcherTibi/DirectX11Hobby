
#include "pch.h"

// Mine
#include "FileIO.hpp"

// Header
#include "NuiLibrary.h"


using namespace nui;

std::vector<Window> nui::windows;


Text* Root::addText()
{
	Node& child_node = window->nodes.emplace_back();
	child_node.parent = this->node;

	this->node->children.push_back(&child_node);

	Text& new_text = child_node.elem.emplace<Text>();
	new_text.window = this->window;
	new_text.node = &child_node;

	return &new_text;
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	for (Window& wnd : windows) {
		if (wnd.hwnd == hwnd) {
			
			switch (uMsg) {
			case WM_SIZE: {

				if (wParam == SIZE_MAXIMIZED || wParam == SIZE_MINIMIZED ||
					wParam == SIZE_RESTORED) 
				{
					uint32_t l_param = static_cast<uint32_t>(lParam);
					wnd.width = l_param & 0xFFFF;
					wnd.height = l_param >> 16;
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

void Text::generateDrawcalls()
{
	this->drawcalls.clear();

	FontSize& font_size = window->instance->char_atlas.fonts[0].sizes[0];

	// Choose most similar font size


	glm::vec2 pen = pos;

	for (uint32_t unicode : text) {
		
		// add instance to existing or new drawcall
		CharacterDrawcall* drawcall;

		bool found = false;
		for (CharacterDrawcall& d : drawcalls) {

			if (d.chara->unicode == unicode) {

				drawcall = &d;
				found = true;
				break;
			}
		}

		if (!found) {

			drawcall = &drawcalls.emplace_back();

			for (Character& chara : font_size.chars) {

				if (chara.unicode == unicode) {

					drawcall->chara = &chara;
					break;
				}
			}
		}

		float scale = size / font_size.size;

		glm::vec2 new_pos = pen;
		new_pos.x += (float)drawcall->chara->bitmap_left * scale;

		int32_t char_height = drawcall->chara->zone->bb_pix.getHeight();
		int32_t char_top = drawcall->chara->bitmap_top;
		new_pos.y += (char_height - char_top) * scale;

		GPU_CharacterInstance& new_instance = drawcall->instances.emplace_back();
		new_instance.color = color;
		new_instance.pos = new_pos;
		new_instance.rasterized_size = (float)font_size.size;
		new_instance.size = size;
		
		pen.x += drawcall->chara->advance_X * scale;
	}
}

ErrStack Window::generateGPU_CharacterVertexData()
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

		Text* text = std::get_if<Text>(&node.elem);
		if (text != nullptr) {

			for (uint32_t unicode : text->text) {

				text->generateDrawcalls();
				instance_count++;
			}
		}
	}

	this->char_instances.resize(instance_count);
	uint32_t instance_idx = 0;

	for (Node& node : nodes) {

		Text* text = std::get_if<Text>(&node.elem);
		if (text != nullptr) {

			for (CharacterDrawcall& drawcall : text->drawcalls) {

				drawcall.instance_start_idx = instance_idx;
				
				std::memcpy(char_instances.data() + instance_idx, drawcall.instances.data(),
					drawcall.instances.size() * sizeof(GPU_CharacterInstance));

				instance_idx += drawcall.instances.size();
			}
		}
	}

	if (instance_count) {
		checkErrStack1(chars_instabuff.load(char_instances.data(), char_instances.size() * sizeof(GPU_CharacterInstance)));
	}

	return ErrStack();
}

ErrStack Window::draw()
{
	ErrStack err_stack;

	if (!rendering_configured) {

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

		checkErrStack1(generateGPU_CharacterVertexData());

		// Character Uniform Buffer
		{
			vkw::BufferCreateInfo info;
			info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			dev.createBuffer(info, text_ubuff);

			text_uniform.screen_size.x = (float)dev.surface.width;
			text_uniform.screen_size.y = (float)dev.surface.height;
			checkErrStack1(text_ubuff.load(&text_uniform, sizeof(GPU_TextUniform)));
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
		}

		// Character Atlas Image View
		{
			vkw::ImageViewCreateInfo info;
			checkErrStack(char_atlas_tex.createView(info, char_atlas_view),
				"failed to create character atlas view");

			TextureAtlas& char_atlas = instance->char_atlas.atlas;
			checkErrStack(char_atlas_view.load(char_atlas.colors.data(), char_atlas.colors.size()),
				"failed to load character atlas data into texture");

			vkw::CommandList* cmd_list = dev.cmd_list;

			cmd_list->beginRecording();
			{
				vkw::ImageBarrier barrier;
				barrier.view = &char_atlas_view;
				barrier.new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.wait_for_access = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.wait_at_access = VK_ACCESS_SHADER_READ_BIT;

				cmd_list->cmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					barrier);
			}
			cmd_list->finish();
		}

		// Sampler
		{
			vkw::SamplerCreateInfo info;
			checkErrStack(dev.createSampler(info, sampler),
				"failed to create sampler");
		}

		// Vertex Shader
		{
			FilePath path;
			checkErrStack1(path.recreateRelativeToSolution("UserInterface/Shaders/Test/vert.spv"));

			std::vector<char> spirv;
			checkErrStack1(path.read(spirv));

			checkErrStack1(dev.createShader(spirv, VK_SHADER_STAGE_VERTEX_BIT, text_vs));
		}

		// Fragment Shader
		{
			FilePath path;
			checkErrStack1(path.recreateRelativeToSolution("UserInterface/Shaders/Test/frag.spv"));

			std::vector<char> spirv;
			checkErrStack1(path.read(spirv));

			checkErrStack1(dev.createShader(spirv, VK_SHADER_STAGE_FRAGMENT_BIT, text_fs));
		}

		// Text Pass
		{
			vkw::DrawpassCreateInfo draw_info = {};
			dev.createDrawpass(draw_info, text_pass);

			vkw::CombinedImageSamplerBinding sampler_info;
			sampler_info.sampler = &sampler;
			sampler_info.tex_view = &char_atlas_view;
			text_pass.bindCombinedImageSampler(sampler_info);

			vkw::UniformBufferBinding ubuff_info;
			ubuff_info.buff = &text_ubuff;
			ubuff_info.set = 1;
			text_pass.bindUniformBuffer(ubuff_info);

			vkw::PresentAttachmentInfo present_info;
			present_info.blendEnable = VK_TRUE;
			present_info.clear_value.color.float32[0] = 0;
			present_info.clear_value.color.float32[1] = 0;
			present_info.clear_value.color.float32[2] = 0;
			present_info.clear_value.color.float32[3] = 0;
			text_pass.addPresentAttachment(present_info);

			text_pass.vertex_inputs.push_back(GPU_CharacterVertex::getVertexInput());
			text_pass.vertex_inputs.push_back(GPU_CharacterInstance::getVertexInput(1));

			text_pass.setVertexShader(text_vs);

			text_pass.setFragmentShader(text_fs);

			checkErrStack1(text_pass.build());
		}

		// Command List
		{
			vkw::CommandListCreateInfo info = {};
			info.surface = &dev.surface;

			checkErrStack1(dev.createCommandList(info, cmd_list));

			cmd_list.beginRecording();

			for (Node& node : nodes) {

				Text* text = std::get_if<Text>(&node.elem);
				if (text != nullptr) {

					cmd_list.cmdBeginRenderpass(text_pass);
					{
						cmd_list.cmdBindVertexBuffers(chars_vbuff, chars_instabuff);
						cmd_list.cmdBindIndexBuffer(chars_idxbuff);

						for (CharacterDrawcall& drawcall : text->drawcalls) {
							cmd_list.cmdDrawIndexedInstanced(6, (uint32_t)drawcall.instances.size(),
								drawcall.chara->index_start_idx, drawcall.instance_start_idx);

						}
					}
					cmd_list.cmdEndRenderpass(text_pass);
				}
			}

			cmd_list.endRecording();
		}

		rendering_configured = true;
	}

	cmd_list.run();
	cmd_list.waitForExecution();

	return ErrStack();
}

Root* Window::getRoot()
{
	Node& root_node = nodes.front();

	return std::get_if<Root>(&root_node.elem);
}

Window::~Window()
{
	if (hwnd != NULL) {
		DestroyWindow(hwnd);
	}
}

ErrStack Instance::create()
{
	ErrStack err_stack{};

	// Create Character Atlas
	{
		FilePath path;
		checkErrStack1(path.recreateRelativeToSolution("UserInterface/Fonts/Roboto-Regular.ttf"));

		std::vector<uint32_t> sizes = {
			20
		};

		Font* font;
		checkErrStack1(char_atlas.addFont(path, sizes, font));
	}

	vkw::InstanceCreateInfo info;
	inst.create(info);

	return err_stack;
}

ErrStack Instance::createWindow(WindowCrateInfo& info, Window*& r_window)
{
	ErrStack err_stack{};

	Window& window = windows.emplace_back();
	window.instance = this;
	window.class_name = std::to_string(windows.size());
	window.window_name = "window name";
	window.width = info.width;
	window.height = info.height;
	window.close = false;

	if (!GetModuleHandleExA(0, NULL, &window.hinstance)) {
		return ErrStack(code_location, "");
	};

	window.window_class = {};
	window.window_class.cbSize = sizeof(WNDCLASSEXA);
	window.window_class.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	window.window_class.lpfnWndProc = &windowProc;
	window.window_class.hInstance = window.hinstance;
	window.window_class.hCursor;
	window.window_class.lpszClassName = window.class_name.data();

	if (!RegisterClassExA(&window.window_class)) {
		return ErrStack(code_location, "failed to register window class");
	}

	window.hwnd = CreateWindowExA(
		WS_EX_LEFT,
		window.class_name.data(),
		window.window_name.data(),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,

		// x, y, width, height
		CW_USEDEFAULT, CW_USEDEFAULT, info.width, info.height,

		NULL,       // Parent window
		NULL,       // Menu
		window.hinstance,  // Instance handle
		NULL        // Additional application data
	);

	if (window.hwnd == NULL) {
		return ErrStack(code_location, "failed to create window");
	}

	vkw::VulkanDevice& dev = window.dev;

	// Rendering
	{
		vkw::DeviceCreateInfo info;
		info.hinstance = window.hinstance;
		info.hwnd = window.hwnd;

		checkErrStack(inst.createDevice(info, window.dev),
			"failed to create device");
	}

	// Root UI Element
	{
		Node& new_node = window.nodes.emplace_back();
		new_node.parent = nullptr;

		Root& new_root = new_node.elem.emplace<Root>();
		new_root.window = &window;
		new_root.node = &new_node;
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
		while (PeekMessageA(&msg, window.hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		checkErrStack1(window.draw());
	}
	
	return err_stack;
}

Instance::~Instance()
{
	windows.clear();
	windows.shrink_to_fit();
}
