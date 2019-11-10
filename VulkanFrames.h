#pragma once

// Standard
#include <vector>


class Device;
class Surface;


class Swapchain
{
public:
	// Parents
	Device* dev = nullptr;
	Surface* surf = nullptr;

public:
	// Content
	uint32_t req_width = 800;
	uint32_t req_height = 600;
	VkExtent2D resolution;

	VkSurfaceTransformFlagBitsKHR pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	VkPresentModeKHR presentation_mode = VK_PRESENT_MODE_FIFO_KHR;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;

public:
	void init(Device* device, Surface* surface);

	ErrorStack build();

	void destroy();

	~Swapchain();
};


class ImageViews
{
public:
	// Parents
	Device* dev = nullptr;
	Swapchain* swapchain = nullptr;

public:
	//Content
	std::vector<VkImage> images;
	std::vector<VkImageView> image_views;

public:
	void init(Device* device, Swapchain* swapchain);

	ErrorStack build();

	void destroy();

	~ImageViews();
};


class Renderpass
{	
public:
	// Parents
	Device* dev;

public:
	// Content
	VkRenderPass renderpass = VK_NULL_HANDLE;

public:
	void init(Device* device);

	ErrorStack build();

	void destroy();

	~Renderpass();
};


class FrameBuffers
{
public:
	// Parents
	Device* dev = nullptr;
	Swapchain* swapchain = nullptr;
	ImageViews* img_views = nullptr;
	Renderpass* renderpass = nullptr;

public:
	// Content
	std::vector<VkFramebuffer> frame_buffers;

public:
	void init(Device* device, Swapchain* swapchain, ImageViews* img_views, Renderpass* renderpass);

	ErrorStack build();

	void destroy();

	~FrameBuffers();
};