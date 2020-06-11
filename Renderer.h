#pragma once

// standard
#include <vector>

// GLM
#include <glm/vec3.hpp>

// mine
#include "CommonTypes.h"
#include "UIComponents.h"

#include "VulkanSystems.h"


struct RenderingContent {
	// Surface
	HINSTANCE* hinstance = nullptr;
	HWND* hwnd = nullptr;

	// Swapchain
	uint32_t width = 0;
	uint32_t height = 0;
};

class VulkanRenderer {
public:
	vks::Instance instance;
	vks::Surface surface;

	vks::PhysicalDevice phys_dev;
	vks::LogicalDevice logical_dev;

	vks::Swapchain swapchain;
	vks::CommandPool cmd_pool;

	vks::Image border_color_img;
	vks::Image padding_color_img;
	vks::Image compose_color_img;
	
	vks::ImageView border_color_view;
	vks::ImageView padding_color_view;
	vks::ImageView compose_color_view;

	vks::ShaderModule rect_vert_module;
	vks::ShaderModule rect_frag_module;
	vks::ShaderModule circles_vert_module;
	vks::ShaderModule circles_frag_module;

	// Common Stuff
	vks::StagingBuffer uniform_staging_buff;
	vks::Buffer uniform_buff;

	vks::DescriptorSetLayout uniform_descp_layout;
	vks::DescriptorPool uniform_descp_pool;
	vks::DescriptorSet uniform_descp_set;

	std::vector<GPU_ElementsLayer> layers;

	// Rect
	vks::Renderpass rect_renderpass;
	vks::PipelineLayout rect_pipe_layout;
	vks::GraphicsPipeline rect_pipe;

	// Circle
	vks::Renderpass circles_renderpass;
	vks::PipelineLayout circles_pipe_layout;
	vks::GraphicsPipeline circles_pipe;

	// Border Rect Pass
	vks::StagingBuffer border_rect_staging_buff;
	vks::Buffer border_rect_vertex_buff;
	uint32_t border_rect_vertex_count;

	std::vector<vks::Framebuffer> border_rect_frames;

	// Border Circles Pass
	vks::StagingBuffer border_circles_staging_buff;
	vks::Buffer border_circles_vertex_buff;

	std::vector<vks::Framebuffer> border_circles_frames;

	// Padding Rect Pass
	vks::StagingBuffer padding_rect_staging_buff;
	vks::Buffer padding_rect_vertex_buff;

	std::vector<vks::Framebuffer> padding_rect_frames;

	// Padding Circle Pass
	vks::StagingBuffer padding_circles_staging_buff;
	vks::Buffer padding_circles_vertex_buff;

	std::vector<vks::Framebuffer> padding_circles_frames;

	// Compose Pass
	vks::Renderpass compose_renderpass;
	std::vector<vks::Framebuffer> compose_frames;

	vks::DescriptorSetLayout compose_descp_layout;
	vks::DescriptorPool compose_descp_pool;
	vks::DescriptorSet compose_descp_set;

	vks::ShaderModule compose_vert_module;
	vks::ShaderModule compose_frag_module;

	vks::PipelineLayout compose_pipe_layout;
	vks::GraphicsPipeline compose_pipe;

	// Copy Pass
	vks::DescriptorSetLayout copy_descp_layout;
	vks::DescriptorPool copy_descp_pool;
	vks::DescriptorSet copy_descp_set;

	vks::ShaderModule copy_frag_module;

	vks::Renderpass copy_renderpass;
	std::vector<vks::Framebuffer> copy_frames;

	vks::PipelineLayout copy_pipe_layout;
	vks::GraphicsPipeline copy_pipe;

	// Command Buffer
	vks::RenderingComandBuffers render_cmd_buffs;

	vks::Semaphore img_acquired;
	vks::Semaphore rendering_ended_sem;

public:
	ErrStack recreate(RenderingContent& content);

	ErrStack calc(ui::UserInterface& user);

	ErrStack draw();

	ErrStack waitForRendering();
};

extern VulkanRenderer renderer;
