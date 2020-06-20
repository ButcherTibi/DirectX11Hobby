#pragma once

// standard
#include <vector>

// GLM
#include <glm/vec3.hpp>

// mine
#include "CommonTypes.h"
#include "UIComponents.h"

#include "VulkanSystems.h"


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

	vks::DescriptorSetLayout uniform_descp_layout;

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

	// Border Rect Pass
	vks::StagingBuffer border_rect_staging_buff;
	vks::Buffer border_rect_vertex_buff;

	// Border Circles Pass
	vks::StagingBuffer border_circles_staging_buff;
	vks::Buffer border_circles_vertex_buff;

	// Padding Rect Pass
	vks::StagingBuffer padding_rect_staging_buff;
	vks::Buffer padding_rect_vertex_buff;

	// Padding Circle Pass
	vks::StagingBuffer padding_circles_staging_buff;
	vks::Buffer padding_circles_vertex_buff;

	// Compose Pass
	vks::Renderpass compose_renderpass;

	vks::DescriptorSetLayout compose_descp_layout;

	vks::ShaderModule compose_vert_module;
	vks::ShaderModule compose_frag_module;

	vks::PipelineLayout compose_pipe_layout;
	vks::GraphicsPipeline compose_pipe;

	// Copy Pass
	vks::Renderpass copy_renderpass;

	vks::DescriptorSetLayout copy_descp_layout;

	vks::ShaderModule copy_frag_module;

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
