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

	ui::UserInterface* user_interface;
};

class VulkanRenderer {
public:
	// Vulkan Systems
	vks::Instance instance;
	vks::Surface surface;

	vks::PhysicalDevice phys_dev;
	vks::LogicalDevice logical_dev;

	vks::CommandPool cmd_pool;
	vks::Buffer staging_buff;

	// Frame
	vks::Swapchain swapchain;

	vks::Image rects_color_img;
	vks::Image rects_depth_img;
	vks::Image circles_color_img;
	vks::Image circles_depth_img;
	vks::Renderpass renderpass;

	vks::ImageView rects_color_view;
	vks::ImageView rects_depth_view;
	vks::ImageView circles_color_view;
	vks::ImageView circles_depth_view;
	vks::Framebuffers frame_buffs;

	// Subpass Commons
	vks::Buffer uniform_buff;

	vks::DescriptorSetLayout uniform_descp_layout;
	vks::DescriptorPool uniform_descp_pool;
	vks::DescriptorSet uniform_descp_set;

	// Rects Subpass
	vks::Buffer rects_vertex_buff;
	uint32_t rects_vertex_count;

	vks::ShaderModule rects_vert_module;
	vks::ShaderModule rects_frag_module;

	vks::PipelineLayout rects_pipe_layout;
	vks::GraphicsPipeline rects_pipe;

	// Circles Subpass
	vks::Buffer circles_vertex_buff;
	uint32_t circles_vertex_count;

	vks::ShaderModule circles_vert_module;
	vks::ShaderModule circles_frag_module;

	vks::PipelineLayout circles_pipe_layout;
	vks::GraphicsPipeline circles_pipe;

	// Compose Subpass
	vks::DescriptorSetLayout compose_descp_layout;
	vks::DescriptorPool compose_descp_pool;
	vks::DescriptorSet compose_descp_set;

	vks::ShaderModule compose_vert_module;
	vks::ShaderModule compose_frag_module;

	vks::PipelineLayout compose_pipe_layout;
	vks::GraphicsPipeline compose_pipe;

	// Command Buffer
	vks::RenderingComandBuffers render_cmd_buffs;

	vks::Semaphore img_acquired;
	vks::Semaphore rendering_ended_sem;

public:
	ErrStack recreate(RenderingContent& content);

	ErrStack draw();

	ErrStack waitForRendering();
};

extern VulkanRenderer renderer;
