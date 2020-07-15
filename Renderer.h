#pragma once

// standard
#include <vector>

// GLM
#include <glm/vec3.hpp>

// mine
#include "CommonTypes.h"
#include "UIComponents.h"

#include "VulkanSystems.h"


//UserInterface user_interface = {};
//{
//	FileSysPath path = {};
//	err = path.recreateRelative("UI/Fonts/Roboto/Roboto-Regular.ttf");
//	if (err.isBad()) {
//		err.debugPrint();
//		return 1;
//	}

//	std::vector<uint8_t> roboto_ttf;
//	err = path.read(roboto_ttf);
//	if (err.isBad()) {
//		err.debugPrint();
//		return 1;
//	}

//	FontInfo roboto_info = {};
//	roboto_info.family_name = "Roboto";
//	roboto_info.style_name = "normal";
//	roboto_info.sizes_px = {
//		50
//	};
//	err = user_interface.addFont(roboto_ttf, roboto_info);
//	if (err.isBad()) {
//		err.debugPrint();
//		return 1;
//	}

//	err = user_interface.rebindToAtlas(1024);
//	if (err.isBad()) {
//		err.debugPrint();
//		return 1;
//	}
//}

//user_interface.recreateGraph(1, 1);

//Flex* root = std::get_if<Flex>(&user_interface.elems.front().elem);
//root->direction = FlexDirection::COLUMN;
//root->wrap = FlexWrap::WRAP;
//root->axis_align = FlexAxisAlign::SPACE_BETWEEN;
//root->cross_axis_align = FlexCrossAxisAlign::START;
//root->lines_align = FlexLinesAlign::SPACE_BETWEEN;
//
//{
//	Flex basic_elem_0 = {};
//	basic_elem_0.width.setRelative(0.4);
//	basic_elem_0.height.setRelative(0.4);

//	basic_elem_0.background_color = { 1, 0, 0, 1 };

//	user_interface.addElement(&user_interface.elems.front(), basic_elem_0);
//}
//
//{
//	Flex basic_elem_0 = {};
//	basic_elem_0.width.setRelative(0.2);
//	basic_elem_0.height.setRelative(0.4);

//	basic_elem_0.background_color = { 0, 1, 0, 1 };

//	user_interface.addElement(&user_interface.elems.front(), basic_elem_0);
//}

//{
//	Flex basic_elem_0 = {};
//	basic_elem_0.width.setRelative(0.2);
//	basic_elem_0.height.setRelative(0.4);

//	basic_elem_0.background_color = { 0, 0, 1, 1 };

//	basic_elem_0.flex_cross_axis_align_self = FlexCrossAxisAlign::END;

//	user_interface.addElement(&user_interface.elems.front(), basic_elem_0);
//}

//{
//	Flex basic_elem_0 = {};
//	basic_elem_0.width.setRelative(0.4);
//	basic_elem_0.height.setRelative(0.4);

//	basic_elem_0.background_color = { 1, 0, 1, 1 };

//	user_interface.addElement(&user_interface.elems.front(), basic_elem_0);
//}
//

//// Renderer
//err = renderer.createContext(&hinstance, &app_level.hwnd);
//if (err.isBad()) {
//	err.debugPrint();
//	return 1;
//}

//uint32_t requested_width = app_level.display_width;
//uint32_t requested_height = app_level.display_height;
//uint32_t rendering_width;
//uint32_t rendering_height;
//err = renderer.getPhysicalSurfaceResolution(rendering_width, rendering_height);
//if (err.isBad()) {
//	err.debugPrint();
//	return 1;
//}

//// we now have the actual resolution of the rendering surface so calculate user interface layout
//user_interface.changeResolution((float)rendering_width, (float)rendering_height);
//renderer.user_interface = &user_interface;

//err = renderer.recreate(rendering_width, rendering_height);
//if (err.isBad()) {
//	err.debugPrint();
//	return 1;
//}


class VulkanRenderer {
public:
	UserInterface* user_interface;

public:
	vks::Instance instance;
	vks::Surface surface;

	vks::PhysicalDevice phys_dev;
	vks::LogicalDevice logical_dev;

	vks::Swapchain swapchain;
	vks::CommandPool cmd_pool;

	vks::Image border_color_img;
	vks::Image border_mask_img;
	vks::Image padding_color_img;
	vks::Image padding_mask_img;
	vks::Image compose_color_img;
	
	vks::ImageView border_color_view;
	vks::ImageView border_mask_view;
	vks::ImageView padding_color_view;
	vks::ImageView padding_mask_view;
	vks::ImageView compose_color_view;

	// Common Stuff
	vks::StagingBuffer common_staging_buff;
	vks::Buffer uniform_buff;
	vks::Buffer vertex_buff;

	vks::StagingBuffer storage_staging_buff;
	vks::Buffer storage_buff;

	vks::ShaderModule fullscreen_vert_module;

	vks::DescriptorSetLayout uniform_descp_layout;
	vks::DescriptorSetLayout storage_descp_layout;

	std::vector<GPU_ElementsLayer> layers;

	// Rect
	vks::Renderpass rect_renderpass;

	vks::ShaderModule rect_vert_module;
	vks::ShaderModule rect_frag_module;

	vks::PipelineLayout rect_pipe_layout;
	vks::GraphicsPipeline rect_pipe;

	// Circle
	vks::Renderpass circles_renderpass;

	vks::ShaderModule circles_vert_module;
	vks::ShaderModule circles_frag_module;

	vks::DescriptorSetLayout circles_descp_layout;

	vks::PipelineLayout circles_pipe_layout;
	vks::GraphicsPipeline circles_pipe;

	// Compose Pass
	vks::Renderpass compose_renderpass;

	vks::ShaderModule compose_frag_module;

	vks::DescriptorSetLayout compose_descp_layout;

	vks::PipelineLayout compose_pipe_layout;
	vks::GraphicsPipeline compose_pipe;

	// Copy Pass
	vks::Renderpass copy_renderpass;

	vks::ShaderModule copy_frag_module;

	vks::DescriptorSetLayout copy_descp_layout;

	vks::PipelineLayout copy_pipe_layout;
	vks::GraphicsPipeline copy_pipe;

	// Framebuffers
	std::vector<vks::Framebuffer> border_rect_frames;
	std::vector<vks::Framebuffer> border_circles_frames;
	std::vector<vks::Framebuffer> padding_rect_frames;
	std::vector<vks::Framebuffer> padding_circles_frames;

	std::vector<vks::Framebuffer> compose_frames;
	std::vector<vks::Framebuffer> copy_frames;

	// Descriptor Pool
	vks::DescriptorPool descp_pool;

	// Descriptor Sets
	vks::DescriptorSet uniform_descp_set;
	vks::DescriptorSet storage_descp_set;
	vks::DescriptorSet border_circles_descp_set;
	vks::DescriptorSet padding_circles_descp_set;
	vks::DescriptorSet compose_descp_set;
	vks::DescriptorSet copy_descp_set;

	// Command Buffer
	vks::RenderingComandBuffers render_cmd_buffs;

	vks::Semaphore img_acquired;
	vks::Semaphore rendering_ended_sem;

public:
	ErrStack recreateSwapchain(uint32_t width, uint32_t height);

	ErrStack recreateFrameImagesAndViews(uint32_t width, uint32_t height);

	void updateUniformDescriptorSet();
	void updateStorageDescriptorSet();
	void updateBorderCirclesDescriptorSet();
	void updatePaddingCirclesDescriptorSet();
	void updateComposeImagesDescriptorSet();
	void updateCopyDescriptorSet();

	ErrStack recreateFramebuffers();
	ErrStack recreateRenderingCommandBuffers();

public:
	ErrStack createContext(HINSTANCE* hinstance, HWND* hwnd);

	ErrStack getPhysicalSurfaceResolution(uint32_t& width, uint32_t& height);

	ErrStack recreate(uint32_t width, uint32_t height);

	ErrStack calc(UserInterface& user);

	ErrStack changeResolution(uint32_t new_width, uint32_t new_height);

	ErrStack draw();

	ErrStack waitForRendering();
};

extern VulkanRenderer renderer;
