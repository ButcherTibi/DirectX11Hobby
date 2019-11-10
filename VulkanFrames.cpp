
// Other
#include "VulkanContext.h"
#include "MathUtils.h"
#include "ErrorStuff.h"

#include "VulkanFrames.h"


void Swapchain::init(Device* device, Surface* surface)
{
	std::cout << "Swapchain init" << std::endl;

	this->dev = device;
	this->surf = surface;

	this->resolution.width = req_width;
	this->resolution.height = req_height;
}

ErrorStack Swapchain::build()
{
	std::cout << "Swapchain build" << std::endl;

	VkResult vk_res;

	// Trim resolution
	VkSurfaceCapabilitiesKHR surf_capbs;
	{		
		vk_res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev->physical_device, surf->surface, &surf_capbs);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, ExtraError::FAILED_TO_GET_SURFACE_CAPABILITIES, code_location, "failed to get physical device surface capabilities");
		}

		VkExtent2D min_img_extent = surf_capbs.minImageExtent;
		VkExtent2D max_img_extent = surf_capbs.maxImageExtent;

		this->resolution.width = clamp(req_width, min_img_extent.width, max_img_extent.width);
		this->resolution.height = clamp(req_height, min_img_extent.height, max_img_extent.height);
		
		/* when minimizing minImageExtent and maxImageExtent will be zero
		 * at that point vkCreateSwapchainKHR will not like ANY resolution
		 * this triggers fewer warnings */
		if (!resolution.width || !resolution.height) {
			this->resolution.width = req_width;
			this->resolution.height = req_height;
		}

		printf("%d %d \n", resolution.width, resolution.height);
	}

	VkSwapchainCreateInfoKHR swapchain_info = {};
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.surface = surf->surface;
	swapchain_info.minImageCount = surf_capbs.minImageCount;
	swapchain_info.imageFormat = dev->surface_format.format;
	swapchain_info.imageColorSpace = dev->surface_format.colorSpace;
	swapchain_info.imageExtent = resolution;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.preTransform = pre_transform;
	swapchain_info.compositeAlpha = composite_alpha;
	swapchain_info.presentMode = presentation_mode;
	swapchain_info.clipped = VK_TRUE;
	swapchain_info.oldSwapchain = NULL;

	vk_res = vkCreateSwapchainKHR(dev->logical_device, &swapchain_info, NULL, &swapchain);
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, code_location, "failed to create swapchain");
	}

	return ErrorStack();
}

void Swapchain::destroy()
{
	std::cout << "Swapchain destroy" << std::endl;
	vkDestroySwapchainKHR(dev->logical_device, swapchain, NULL);
	swapchain = VK_NULL_HANDLE;
}

Swapchain::~Swapchain()
{
	if (dev != nullptr) {
		this->destroy();
	}	
}

void ImageViews::init(Device* device, Swapchain* swapchain)
{
	std::cout << "ImageViews init" << std::endl;

	this->dev = device;
	this->swapchain = swapchain;
}

ErrorStack ImageViews::build()
{
	std::cout << "ImageViews build" << std::endl;

	VkResult vk_res;

	uint32_t image_count = 0;
	vk_res = vkGetSwapchainImagesKHR(dev->logical_device, swapchain->swapchain, &image_count, NULL);
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, code_location, "failed to retrieve swap chain image count");
	}

	images.resize(image_count);
	vk_res = vkGetSwapchainImagesKHR(dev->logical_device, swapchain->swapchain, &image_count, images.data());
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, code_location, "failed to retrieve swap chain images");
	}

	image_views.resize(image_count);
	for (uint32_t i = 0; i < image_count; i++) {

		VkComponentMapping component_mapping = {};
		component_mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		component_mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		component_mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		component_mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		VkImageSubresourceRange sub_resource = {};
		sub_resource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		sub_resource.baseMipLevel = 0;
		sub_resource.levelCount = 1;
		sub_resource.baseArrayLayer = 0;
		sub_resource.layerCount = 1;

		VkImageViewCreateInfo imageview_info = {};
		imageview_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageview_info.image = images[i];
		imageview_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageview_info.format = dev->surface_format.format;
		imageview_info.components = component_mapping;
		imageview_info.subresourceRange = sub_resource;

		vk_res = vkCreateImageView(dev->logical_device, &imageview_info, NULL, &image_views[i]);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, code_location, "failed to create image view");
		}
	}

	return ErrorStack();
}

void ImageViews::destroy()
{
	std::cout << "ImageViews destroy" << std::endl;
	for (uint32_t i = 0; i < image_views.size(); i++) {
		vkDestroyImageView(dev->logical_device, image_views[i], NULL);
	}
	image_views.clear();
}

ImageViews::~ImageViews()
{
	if (dev != nullptr) {
		this->destroy();
	}	
}

void Renderpass::init(Device* device)
{
	std::cout << "Renderpass init" << std::endl;
	this->dev = device;
}

ErrorStack Renderpass::build()
{
	std::cout << "Renderpass build" << std::endl;

	VkResult vk_res;

	VkAttachmentDescription color_attachment = {};
	color_attachment.format = dev->surface_format.format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;  // for MSAA ?
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attach_ref = {};
	color_attach_ref.attachment = 0;
	color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass_description = {};
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &color_attach_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderpass_info = {};
	renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpass_info.attachmentCount = 1;
	renderpass_info.pAttachments = &color_attachment;
	renderpass_info.subpassCount = 1;
	renderpass_info.pSubpasses = &subpass_description;
	renderpass_info.dependencyCount = 1;
	renderpass_info.pDependencies = &dependency;

	vk_res = vkCreateRenderPass(dev->logical_device, &renderpass_info, NULL, &renderpass);
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, code_location, "failed to create renderpass");
	}

	return ErrorStack();
}

void Renderpass::destroy()
{
	std::cout << "Renderpass destroy" << std::endl;
	vkDestroyRenderPass(dev->logical_device, renderpass, NULL);
	renderpass = VK_NULL_HANDLE;
}

Renderpass::~Renderpass()
{
	if (dev != nullptr) {
		this->destroy();
	}	
}

void FrameBuffers::init(Device* device, Swapchain* swapchain, ImageViews* img_views, Renderpass* renderpass)
{
	this->dev = device;
	this->swapchain = swapchain;
	this->img_views = img_views;
	this->renderpass = renderpass;
}

ErrorStack FrameBuffers::build()
{
	std::cout << "FrameBuffers build" << std::endl;

	size_t img_count = img_views->image_views.size();
	frame_buffers.resize(img_count);

	for (size_t i = 0; i < img_count; i++) {

		VkImageView attachments[] = { img_views->image_views[i] };

		VkFramebufferCreateInfo framebuff_info = {};
		framebuff_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuff_info.renderPass = renderpass->renderpass;
		framebuff_info.attachmentCount = 1;
		framebuff_info.pAttachments = attachments;
		framebuff_info.width = swapchain->resolution.width;
		framebuff_info.height = swapchain->resolution.height;
		framebuff_info.layers = 1;

		VkResult vk_res = vkCreateFramebuffer(dev->logical_device, &framebuff_info, NULL, &frame_buffers[i]);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, code_location, "failed to create framebuffer");
		}
	}

	return ErrorStack();
}

void FrameBuffers::destroy()
{
	std::cout << "FrameBuffers destroy" << std::endl;

	for (uint32_t i = 0; i < frame_buffers.size(); i++) {
		vkDestroyFramebuffer(dev->logical_device, frame_buffers[i], NULL);
	}
	frame_buffers.clear();
}

FrameBuffers::~FrameBuffers()
{
	if (dev != nullptr) {
		this->destroy();
	}
}