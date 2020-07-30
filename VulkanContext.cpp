
#include <execution>

// Header
#include "VulkanSystems.hpp"

using namespace vks;


void Surface2::destroy()
{
	vkDestroySurfaceKHR(instance->instance, surface, NULL);
	surface = VK_NULL_HANDLE;
}

Surface2::~Surface2()
{
	if (surface != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack LogicalDevice2::setDebugName(uint64_t obj, VkObjectType obj_type, std::string name)
{
	VkResult vk_res{};

	VkDebugUtilsObjectNameInfoEXT info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectType = obj_type;
	info.objectHandle = obj;
	info.pObjectName = name.c_str();

	checkVkRes(phys_dev->instance->set_vkdbg_name_func(logical_device, &info),
		"failed to set debug name for vulkan object")

	return ErrStack();
}

ErrStack LogicalDevice2::createSwapchain(SwapchainCreateInfo& info, Swapchain2& swapchain)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	Surface2* default_surface = this->phys_dev->instance->default_surface;

	VkSurfaceCapabilitiesKHR capabilities;
	checkErrStack1(phys_dev->getSurfaceCapabilities(*default_surface, capabilities));

	if (!info.minImageCount) {
		info.minImageCount = capabilities.minImageCount;
	}

	if (info.imageFormat == VK_FORMAT_MAX_ENUM) {

		uint32_t format_counts;
		checkVkRes(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev->physical_device, default_surface->surface, &format_counts, NULL),
			"failed to find physical surface format count");

		std::vector<VkSurfaceFormatKHR> formats(format_counts);
		checkVkRes(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev->physical_device, default_surface->surface, &format_counts, formats.data()),
			"failed to find physical surface format");

		bool found = false;
		for (VkSurfaceFormatKHR format : formats) {
			if (format.format == VK_FORMAT_R8G8B8A8_UNORM ||
				format.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				info.imageFormat = format.format;
				found = true;
				break;
			}
		}

		if (!found) {
			return ErrStack(code_location, "failed to find suitable surface format");
		}
	}

	if (!info.width) {
		info.width = capabilities.currentExtent.width;
		info.height = capabilities.currentExtent.height;
	}

	swapchain = {};
	swapchain.dev = this;
	swapchain.info = info;

	VkSwapchainCreateInfoKHR vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vk_info.pNext = info.pNext;
	vk_info.flags = info.flags;
	vk_info.surface = default_surface->surface;
	vk_info.minImageCount = info.minImageCount;
	vk_info.imageFormat = info.imageFormat;
	vk_info.imageColorSpace = info.imageColorSpace;
	vk_info.imageExtent.width = info.width;
	vk_info.imageExtent.height = info.height;
	vk_info.imageArrayLayers = info.imageArrayLayers;
	vk_info.imageUsage = info.imageUsage;
	vk_info.imageSharingMode = info.imageSharingMode;
	vk_info.queueFamilyIndexCount = info.queue_family_indices.size();
	vk_info.pQueueFamilyIndices = info.queue_family_indices.data();
	vk_info.preTransform = info.preTransform;
	vk_info.compositeAlpha = info.compositeAlpha;
	vk_info.presentMode = info.presentMode;
	vk_info.clipped = info.clipped;
	vk_info.oldSwapchain = info.oldSwapchain;

	checkVkRes(vkCreateSwapchainKHR(logical_device, &vk_info, NULL, &swapchain.swapchain),
		"failed to create swapchain");

	// Image and Image View of the swapchain
	uint32_t image_count = 0;
	checkVkRes(vkGetSwapchainImagesKHR(logical_device, swapchain.swapchain, &image_count, NULL),
		"failed to retrieve swap chain image count");

	swapchain.images.resize(image_count);
	checkVkRes(vkGetSwapchainImagesKHR(logical_device, swapchain.swapchain, &image_count, swapchain.images.data()),
		"failed to retrieve swap chain images");

	VkComponentMapping component_mapping = {};
	component_mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	component_mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	component_mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	component_mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	VkImageSubresourceRange res_range = {};
	res_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	res_range.baseMipLevel = 0;
	res_range.levelCount = 1;
	res_range.baseArrayLayer = 0;
	res_range.layerCount = 1;;

	swapchain.views.resize(image_count);
	for (uint32_t i = 0; i < image_count; i++) {

		VkImageViewCreateInfo view_info = {};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = swapchain.images[i];
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = info.imageFormat;
		view_info.components = component_mapping;
		view_info.subresourceRange = res_range;

		checkVkRes(vkCreateImageView(logical_device, &view_info, NULL, &swapchain.views[i]),
			"failed to create swapchain image views");
	}

	// Default
	if (this->default_swapchain == nullptr) {
		default_swapchain = &swapchain;
	}

	return err_stack;
}

ErrStack LogicalDevice2::createFence(FenceCreateInfo& info, Fence2& fence)
{
	VkResult vk_res{};

	fence = {};
	fence.dev = this;

	VkFenceCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vk_info.pNext = info.pNext;
	vk_info.flags = info.flags;

	checkVkRes(vkCreateFence(logical_device, &vk_info, NULL, &fence.fence),
		"failed to create fence");

	return ErrStack();
}

ErrStack LogicalDevice2::createSemaphore(SemaphoreCreateInfo& info, Semaphore2& semaphore)
{
	VkResult vk_res{};

	VkSemaphoreCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vk_info.pNext = info.pNext;
	vk_info.flags = info.flags;

	checkVkRes(vkCreateSemaphore(logical_device, &vk_info, NULL, &semaphore.semaphore),
		"failed to create semaphore");

	return ErrStack();
}

ErrStack LogicalDevice2::createCommandPool(CommandPoolCreateInfo& info, CommandPool2& command_pool)
{
	VkResult vk_res{};

	command_pool = {};
	command_pool.dev = this;

	VkCommandPoolCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vk_info.flags = info.flags;

	if (info.queueFamilyIndex == -1) {
		vk_info.queueFamilyIndex = default_queue_family;
	}
	else {
		vk_info.queueFamilyIndex = info.queueFamilyIndex;
	}

	checkVkRes(vkCreateCommandPool(logical_device, &vk_info, NULL, &command_pool.pool),
		"failed to create command pool");

	return ErrStack();
}


ErrStack LogicalDevice2::createTexture(TextureCreateInfo& info, Texture& texture)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	texture = {};
	texture.dev = this;

	if (info.format == VK_FORMAT_MAX_ENUM) {
		info.format = default_swapchain->info.imageFormat;
	}

	if (!info.width) {
		info.width = default_swapchain->info.width;
		info.height = default_swapchain->info.height;
	}
	else {
		info.width = info.width;
		info.height = info.height;
	}

	VmaAllocationCreateInfo alloc_create_info = {};
	alloc_create_info.usage = info.mem_usage;

	VkImageCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	vk_info.flags = info.flags;
	vk_info.imageType = info.imageType;
	vk_info.format = info.format;
	vk_info.extent.width = info.width;
	vk_info.extent.height = info.height;
	vk_info.extent.depth = info.depth;
	vk_info.mipLevels = info.mipLevels;
	vk_info.arrayLayers = info.arrayLayers;
	vk_info.samples = info.samples;
	vk_info.tiling = info.tiling;
	vk_info.usage = info.usage;
	vk_info.sharingMode = info.sharingMode;
	vk_info.queueFamilyIndexCount = info.queueFamilyIndexCount;
	vk_info.pQueueFamilyIndices = info.pQueueFamilyIndices;
	vk_info.initialLayout = info.initialLayout;

	checkVkRes(vmaCreateImage(allocator, &vk_info, &alloc_create_info,
		&texture.img, &texture.alloc, &texture.alloc_info),
		"failed to create texture");

	texture.info = info;
	texture.current_layout = info.initialLayout;

	VkMemoryPropertyFlags mem_flags;
	vmaGetMemoryTypeProperties(allocator, texture.alloc_info.memoryType, &mem_flags);

	if (mem_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {

		texture.load_type = LoadType::MEMCPY;
		checkVkRes(vmaMapMemory(allocator, texture.alloc, &texture.mem), "");
	}
	else {
		texture.load_type = LoadType::STAGING;
		texture.mem = nullptr;
	}

	return err_stack;
}

ErrStack LogicalDevice2::createRawBuffer(size_t size, BufferCreateInfo& info, RawBuffer& raw_buffer)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	raw_buffer = {};
	raw_buffer.dev = this;

	checkErrStack1(raw_buffer.create(size, info));

	return err_stack;
}

ErrStack LogicalDevice2::createRenderpass(RenderpassCreateInfo& info, Renderpass2& renderpass)
{
	VkResult vk_res{};

	renderpass = {};
	renderpass.dev = this;

	std::vector<VkAttachmentDescription> atach_descps(info.atach_descps.size());
	for (uint32_t i = 0; i < info.atach_descps.size(); i++) {

		VkAttachmentDescription& vk_descp = atach_descps[i];
		AtachmentCreateInfo& descp = info.atach_descps[i];

		if (descp.format == VK_FORMAT_UNDEFINED) {
			descp.format = default_swapchain->info.imageFormat;
		}

		vk_descp.flags = descp.flags;
		vk_descp.format = descp.format;
		vk_descp.samples = descp.samples;
		vk_descp.loadOp = descp.loadOp;
		vk_descp.storeOp = descp.storeOp;
		vk_descp.stencilLoadOp = descp.stencilLoadOp;
		vk_descp.stencilStoreOp = descp.stencilStoreOp;
		vk_descp.initialLayout = descp.initialLayout;
		vk_descp.finalLayout = descp.finalLayout;
	}

	std::vector<VkSubpassDescription> subpasses(info.subpasses.size());
	for (uint32_t i = 0; i < info.subpasses.size(); i++) {

		VkSubpassDescription& vk_subpass = subpasses[i];
		SubpassCreateInfo& subpass = info.subpasses[i];

		vk_subpass.flags = subpass.flags;
		vk_subpass.pipelineBindPoint = subpass.bind_point;
		vk_subpass.inputAttachmentCount = subpass.input_atachs.size();
		vk_subpass.pInputAttachments = subpass.input_atachs.data();
		vk_subpass.colorAttachmentCount = subpass.color_atachs.size();
		vk_subpass.pColorAttachments = subpass.color_atachs.data();
		vk_subpass.pResolveAttachments = NULL;
		vk_subpass.pDepthStencilAttachment = NULL;
		vk_subpass.preserveAttachmentCount = 0;
	}

	std::vector<VkSubpassDependency> subpass_deps(info.subpass_deps.size());
	for (uint32_t i = 0; i < info.subpass_deps.size(); i++) {

		VkSubpassDependency& vk_subpass_dep = subpass_deps[i];
		SubpassDependencyCreateInfo& subpass_dep = info.subpass_deps[i];

		vk_subpass_dep.srcSubpass = subpass_dep.srcSubpass;
		vk_subpass_dep.dstSubpass = subpass_dep.dstSubpass;
		vk_subpass_dep.srcStageMask = subpass_dep.srcStageMask;
		vk_subpass_dep.dstStageMask = subpass_dep.dstStageMask;
		vk_subpass_dep.srcAccessMask = subpass_dep.srcAccessMask;
		vk_subpass_dep.dstAccessMask = subpass_dep.dstAccessMask;
		vk_subpass_dep.dependencyFlags = subpass_dep.dependencyFlags;
	}

	VkRenderPassCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	vk_info.flags = info.flags;
	vk_info.attachmentCount = atach_descps.size();
	vk_info.pAttachments = atach_descps.data();
	vk_info.subpassCount = subpasses.size();
	vk_info.pSubpasses = subpasses.data();
	vk_info.dependencyCount = subpass_deps.size();
	vk_info.pDependencies = subpass_deps.data();

	checkVkRes(vkCreateRenderPass(logical_device, &vk_info, NULL, &renderpass.renderpass),
		"failed to create renderpass");

	return ErrStack();
}

ErrStack LogicalDevice2::createDescriptorSetLayout(DescriptorSetLayoutCreateInfo& info, DescriptorSetLayout2& descp_set_layout)
{
	VkResult vk_res{};

	descp_set_layout = {};
	descp_set_layout.dev = this;

	std::vector<VkDescriptorSetLayoutBinding> bindings(info.bindings.size());
	for (uint32_t i = 0; i < info.bindings.size(); i++) {

		VkDescriptorSetLayoutBinding& vk_binding = bindings[i];
		DescriptorSetLayoutBinding& binding = info.bindings[i];

		vk_binding.binding = binding.binding;
		vk_binding.descriptorType = binding.descriptorType;
		vk_binding.descriptorCount = binding.descriptorCount;
		vk_binding.stageFlags = binding.stageFlags;
		vk_binding.pImmutableSamplers = binding.pImmutableSamplers;
	}

	VkDescriptorSetLayoutCreateInfo descp_layout_info = {};
	descp_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descp_layout_info.flags = info.flags;
	descp_layout_info.bindingCount = (uint32_t)(bindings.size());
	descp_layout_info.pBindings = bindings.data();

	checkVkRes(vkCreateDescriptorSetLayout(logical_device, &descp_layout_info, NULL, &descp_set_layout.layout),
		"failed to create descriptor set layout");

	return ErrStack();
}

ErrStack LogicalDevice2::createPipelineLayout(PipelineLayoutCreateInfo& info, PipelineLayout2& pipe_layout)
{
	VkResult vk_res{};

	pipe_layout = {};
	pipe_layout.dev = this;

	std::vector<VkDescriptorSetLayout> set_layouts(info.layouts.size());
	for (auto i = 0; i < set_layouts.size(); i++) {
		set_layouts[i] = info.layouts[i]->layout;
	}

	std::vector<VkPushConstantRange> vk_pushes(info.push_const_ranges.size());
	for (auto i = 0; i < vk_pushes.size(); i++) {
		vk_pushes[i].stageFlags = info.push_const_ranges[i].stageFlags;
		vk_pushes[i].offset = info.push_const_ranges[i].offset;
		vk_pushes[i].size = info.push_const_ranges[i].size;
	}

	VkPipelineLayoutCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vk_info.flags = info.flags;
	vk_info.setLayoutCount = set_layouts.size();
	vk_info.pSetLayouts = set_layouts.data();
	vk_info.pushConstantRangeCount = vk_pushes.size();
	vk_info.pPushConstantRanges = vk_pushes.data();

	checkVkRes(vkCreatePipelineLayout(logical_device, &vk_info, NULL, &pipe_layout.layout),
		"failed to create pipeline layout");

	return ErrStack();
}

ErrStack LogicalDevice2::createVertexShader(std::vector<char>& code, Shader& vertex_shader)
{
	vertex_shader = {};
	vertex_shader.dev = this;
	vertex_shader.stage = VK_SHADER_STAGE_VERTEX_BIT;

	return vertex_shader.create(code);
}

ErrStack LogicalDevice2::createFragmentShader(std::vector<char>& code, Shader& fragment_shader)
{
	fragment_shader = {};
	fragment_shader.dev = this;
	fragment_shader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

	return fragment_shader.create(code);
}

ErrStack LogicalDevice2::createGraphicsPipeline(GraphicsPipelineCreateInfo& info, GraphicsPipeline2& graphics_pipe)
{
	VkResult vk_res{};

	graphics_pipe = {};
	graphics_pipe.dev = this;

	// Vertex Input
	std::vector<VkPipelineShaderStageCreateInfo> vk_stages(info.stages.size());
	for (auto i = 0; i < vk_stages.size(); i++) {

		VkPipelineShaderStageCreateInfo& vk_stage = vk_stages[i];
		ShaderStageCreteInfo& stage = info.stages[i];

		vk_stage.flags = stage.flags;
		vk_stage.stage = stage.stage;
		vk_stage.module = stage.module;
		vk_stage.pName = stage.pName;
		vk_stage.pSpecializationInfo = stage.pSpecializationInfo;
	}

	VkPipelineVertexInputStateCreateInfo VI_info = {};
	VertexInput& VI = info.vertex_input.vertex_input;
	VI_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VI_info.flags = info.vertex_input.flags;
	VI_info.vertexBindingDescriptionCount = 1;
	VI_info.pVertexBindingDescriptions = &VI.binding;
	VI_info.vertexAttributeDescriptionCount = VI.atributes.size();
	VI_info.pVertexAttributeDescriptions = VI.atributes.data();

	// Input Assembly
	VkPipelineInputAssemblyStateCreateInfo IA_info = {};
	IA_info.flags = info.input_assembly.flags;
	IA_info.topology = info.input_assembly.topology;
	IA_info.primitiveRestartEnable = info.input_assembly.primitiveRestartEnable;

	// Viewport
	std::vector<VkViewport> vk_viewports(info.viewport_state.viewports.size());
	for (auto i = 0; i < vk_viewports.size(); i++) {

		ViewportCreateInfo& viewport = info.viewport_state.viewports[i];
		vk_viewports[i].x = viewport.x;
		vk_viewports[i].y = viewport.y;

		if (!viewport.width) {
			vk_viewports[i].width = (float)default_swapchain->info.width;
			vk_viewports[i].height = (float)default_swapchain->info.height;
		}
		else {
			vk_viewports[i].width = viewport.width;
			vk_viewports[i].height = viewport.height;
		}

		vk_viewports[i].minDepth = viewport.minDepth;
		vk_viewports[i].maxDepth = viewport.maxDepth;
	}

	std::vector<VkRect2D> vk_scissors(info.viewport_state.scissors.size());
	for (auto i = 0; i < vk_scissors.size(); i++) {

		ScissorsCreateInfo& scissor = info.viewport_state.scissors[i];
		vk_scissors[i].offset = scissor.offset;

		if (!vk_scissors[i].extent.width) {
			vk_scissors[i].extent.width = default_swapchain->info.width;
			vk_scissors[i].extent.height = default_swapchain->info.height;
		}
		else {
			vk_scissors[i].extent.width = scissor.extent.width;
			vk_scissors[i].extent.height = scissor.extent.height;
		}
	}

	VkPipelineViewportStateCreateInfo viewport_info = {};
	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.flags = info.viewport_state.flags;
	viewport_info.viewportCount = vk_viewports.size();
	viewport_info.pViewports = vk_viewports.data();
	viewport_info.scissorCount = vk_scissors.size();
	viewport_info.pScissors = vk_scissors.data();

	// Rasterization
	VkPipelineRasterizationStateCreateInfo vk_raster = {};
	vk_raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	vk_raster.flags = info.rasterization.flags;
	vk_raster.depthClampEnable = info.rasterization.depthClampEnable;
	vk_raster.rasterizerDiscardEnable = info.rasterization.rasterizerDiscardEnable;
	vk_raster.polygonMode = info.rasterization.polygonMode;
	vk_raster.cullMode = info.rasterization.cullMode;
	vk_raster.frontFace = info.rasterization.frontFace;
	vk_raster.depthBiasEnable = info.rasterization.depthBiasEnable;
	vk_raster.depthBiasConstantFactor = info.rasterization.depthBiasConstantFactor;
	vk_raster.depthBiasClamp = info.rasterization.depthBiasClamp;
	vk_raster.depthBiasSlopeFactor = info.rasterization.depthBiasSlopeFactor;
	vk_raster.lineWidth = info.rasterization.lineWidth;

	// Multisample
	VkPipelineMultisampleStateCreateInfo vk_multi = {};
	vk_multi.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	vk_multi.flags = info.multisample.flags;
	vk_multi.rasterizationSamples = info.multisample.rasterizationSamples;
	vk_multi.sampleShadingEnable = info.multisample.sampleShadingEnable;
	vk_multi.minSampleShading = info.multisample.minSampleShading;
	vk_multi.pSampleMask = info.multisample.pSampleMask;
	vk_multi.alphaToCoverageEnable = info.multisample.alphaToCoverageEnable;
	vk_multi.alphaToOneEnable = info.multisample.alphaToOneEnable;

	// Depth Stencil
	VkPipelineDepthStencilStateCreateInfo vk_depth = {};
	vk_depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	vk_depth.flags = info.depth_stencil.flags;
	vk_depth.depthTestEnable = info.depth_stencil.depthTestEnable;
	vk_depth.depthWriteEnable = info.depth_stencil.depthWriteEnable;
	vk_depth.depthCompareOp = info.depth_stencil.depthCompareOp;
	vk_depth.depthBoundsTestEnable = info.depth_stencil.depthBoundsTestEnable;
	vk_depth.stencilTestEnable = info.depth_stencil.stencilTestEnable;
	vk_depth.front = info.depth_stencil.front;
	vk_depth.back = info.depth_stencil.back;
	vk_depth.minDepthBounds = info.depth_stencil.minDepthBounds;
	vk_depth.maxDepthBounds = info.depth_stencil.maxDepthBounds;

	// Color Blend
	std::vector<VkPipelineColorBlendAttachmentState> vk_atachs(info.color_blend.attachments.size());
	for (auto i = 0; i < vk_atachs.size(); i++) {

		VkPipelineColorBlendAttachmentState& vk_atach = vk_atachs[i];
		ColorBlendAtachmentCreateInfo& atach = info.color_blend.attachments[i];

		vk_atach.blendEnable = atach.blendEnable;
		vk_atach.srcColorBlendFactor = atach.srcColorBlendFactor;
		vk_atach.dstColorBlendFactor = atach.dstColorBlendFactor;
		vk_atach.colorBlendOp = atach.colorBlendOp;
		vk_atach.srcAlphaBlendFactor = atach.srcAlphaBlendFactor;
		vk_atach.dstAlphaBlendFactor = atach.dstAlphaBlendFactor;
		vk_atach.alphaBlendOp = atach.alphaBlendOp;
		vk_atach.colorWriteMask = atach.colorWriteMask;
	}

	VkPipelineColorBlendStateCreateInfo vk_blend = {};
	vk_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	vk_blend.flags = info.color_blend.flags;
	vk_blend.logicOpEnable = info.color_blend.logicOpEnable;
	vk_blend.logicOp = info.color_blend.logicOp;
	vk_blend.attachmentCount = vk_atachs.size();
	vk_blend.pAttachments = vk_atachs.data();
	vk_blend.blendConstants[0] = info.color_blend.blendConstants[0];
	vk_blend.blendConstants[1] = info.color_blend.blendConstants[1];
	vk_blend.blendConstants[2] = info.color_blend.blendConstants[2];
	vk_blend.blendConstants[3] = info.color_blend.blendConstants[3];

	// Dynamic State
	VkPipelineDynamicStateCreateInfo vk_dynamic = {};
	vk_dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	vk_dynamic.flags = info.dynamic_state.flags;
	vk_dynamic.dynamicStateCount = info.dynamic_state.dynamic_states.size();
	vk_dynamic.pDynamicStates = info.dynamic_state.dynamic_states.data();

	VkGraphicsPipelineCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vk_info.flags = vk_info.flags;
	vk_info.stageCount = vk_stages.size();
	vk_info.pStages = vk_stages.data();
	vk_info.pVertexInputState = &VI_info;
	vk_info.pInputAssemblyState = &IA_info;
	vk_info.pTessellationState = NULL;
	vk_info.pViewportState = &viewport_info;
	vk_info.pRasterizationState = &vk_raster;
	vk_info.pMultisampleState = &vk_multi;
	vk_info.pDepthStencilState = &vk_depth;
	vk_info.pColorBlendState = &vk_blend;
	vk_info.pDynamicState = &vk_dynamic;
	vk_info.layout = info.layout->layout;
	vk_info.renderPass = info.renderpass->renderpass;
	vk_info.subpass = info.subpass;
	vk_info.basePipelineHandle = info.basePipelineHandle;
	vk_info.basePipelineIndex = info.basePipelineIndex;

	checkVkRes(vkCreateGraphicsPipelines(logical_device, NULL, 1, &vk_info, NULL, &graphics_pipe.pipeline),
		"failed to create graphics pipeline");

	return ErrStack();
}

void LogicalDevice2::createDescriptorPool(DescriptorPoolCreateInfo& info, DescriptorPool2& descp_pool)
{
	descp_pool = {};
	descp_pool.flags = info.flags;
}

ErrStack LogicalDevice2::createRenderingCommandBuffers(RenderingCommandBuffersCreateInfo& info, RenderingCommandBuffers2& cmd_buffs)
{
	cmd_buffs = {};
	cmd_buffs.dev = this;

	if (!info.size) {
		info.size = default_swapchain->views.size();
	}
	cmd_buffs.tasks.resize(info.size);

	for (uint32_t i = 0; i < info.size; i++) {
		cmd_buffs.tasks[i].idx = i;
	}

	std::for_each(std::execution::seq, cmd_buffs.tasks.begin(), cmd_buffs.tasks.end(), [&](RenderingCommandTask& task) {

		task.err_stack = createCommandPool(info.pool_info, task.pool);
		if (task.err_stack.isBad()) {
			task.err_stack.pushError(code_location, "failed to create rendering command pool");
			return;
		}

		task.err_stack = task.pool.createCommandBuffer(info.buff_info, task.buff);
		if (task.err_stack.isBad()) {
			task.err_stack.pushError(code_location, "failed to create rendering command buffer");
			return;
		}
	});

	ErrStack err_stack{};
	for (auto& task : cmd_buffs.tasks) {
		checkErrStack1(task.err_stack);
	}

	return err_stack;
}

void LogicalDevice2::destroy()
{
	vkDestroyDevice(logical_device, NULL);
	logical_device = VK_NULL_HANDLE;
}

LogicalDevice2::~LogicalDevice2()
{
	if (logical_device != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack PhysicalDevice2::getQueueFamilyIdxWithSurfaceSupport(Surface2& surface, uint32_t& r_found_idx)
{
	VkResult vk_res{};

	uint32_t family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, queue_families.data());

	for (uint32_t i = 0; i < family_count; i++) {

		VkQueueFamilyProperties& family_prop = queue_families[i];

		VkBool32 supported;
		checkVkRes(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface.surface, &supported),
			"failed to check if pshysical device can present");

		if (family_prop.queueCount > 0 && family_prop.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
			supported == VK_TRUE)
		{
			r_found_idx = i;
			return ErrStack();
		}
	}

	return ErrStack(code_location, "queue family idx that supports surface not found");
}

ErrStack PhysicalDevice2::checkForExtensions(ExtensionsSupportInfo& info)
{
	uint32_t extension_count;
	if (vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extension_count, NULL) != VK_SUCCESS) {
		return ErrStack(code_location, "failed to retrieve enumerate physical device extension properties count");
	}

	std::vector<VkExtensionProperties> available_extensions(extension_count);
	if (vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extension_count, available_extensions.data()) != VK_SUCCESS) {
		return ErrStack(code_location, "failed to retrieve enumerate physical device extension properties");
	}

	uint32_t count = 0;
	for (const char* req_extension : info.extensions) {
		for (VkExtensionProperties extension : available_extensions) {

			if (!std::strcmp(req_extension, extension.extensionName)) {
				count++;
			}
		}
	}

	if (count < info.extensions.size()) {
		return ErrStack(code_location, "not all requested physical device extensions found");
	}
	return ErrStack();
}

ErrStack PhysicalDevice2::checkForFeatures(FeaturesSupportInfo& info)
{
	VkPhysicalDeviceFeatures available_features;
	vkGetPhysicalDeviceFeatures(physical_device, &available_features);

	if (info.features.samplerAnisotropy &&
		info.features.samplerAnisotropy != available_features.samplerAnisotropy)
	{
		return ErrStack(code_location, "physical device feature not found");
	}

	return ErrStack();
}

ErrStack PhysicalDevice2::getSurfaceCapabilities(Surface2& surface, VkSurfaceCapabilitiesKHR& capabilities)
{
	VkResult vk_res{};

	checkVkRes(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface.surface, &capabilities),
		"failed to get physical device surface capabilities");

	return ErrStack();
}

ErrStack PhysicalDevice2::createLogicalDevice(LogicalDeviceCreateInfo& info, LogicalDevice2& logical_device)
{
	VkResult vk_res{};

	logical_device = {};
	logical_device.phys_dev = this;

	std::vector<VkDeviceQueueCreateInfo> vk_queue_infos(info.queue_infos.size());
	for (auto i = 0; i < vk_queue_infos.size(); i++) {
		VkDeviceQueueCreateInfo& vk_queue_info = vk_queue_infos[i];
		DeviceQueueCreateInfo& queue_info = info.queue_infos[i];

		vk_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		vk_queue_info.pNext = queue_info.pNext;
		vk_queue_info.flags = queue_info.flags;
		vk_queue_info.queueFamilyIndex = queue_info.queueFamilyIndex;
		vk_queue_info.queueCount = queue_info.queue_priorities.size();
		vk_queue_info.pQueuePriorities = queue_info.queue_priorities.data();
	}

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = info.pNext;
	device_info.flags = info.flags;
	device_info.queueCreateInfoCount = vk_queue_infos.size();
	device_info.pQueueCreateInfos = vk_queue_infos.data();
	device_info.enabledLayerCount = info.layers.size();
	device_info.ppEnabledLayerNames = info.layers.data();
	device_info.enabledExtensionCount = info.extensions.extensions.size();
	device_info.ppEnabledExtensionNames = info.extensions.extensions.data();
	device_info.pEnabledFeatures = &info.features.features;

	checkVkRes(vkCreateDevice(physical_device, &device_info, NULL, &logical_device.logical_device),
		"failed to create logical device");
	
	// VMA Allocator
	{
		VmaAllocatorCreateInfo alloc_info = {};
		alloc_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		alloc_info.instance = instance->instance;
		alloc_info.physicalDevice = physical_device;
		alloc_info.device = logical_device.logical_device;

		VkResult vk_res = vmaCreateAllocator(&alloc_info, &logical_device.allocator);
		if (vk_res != VK_SUCCESS) {
			return ErrStack(code_location, "failed to create allocator");
		}
	}

	// Default Queue
	logical_device.default_queue_family = vk_queue_infos[0].queueFamilyIndex;
	vkGetDeviceQueue(logical_device.logical_device, vk_queue_infos[0].queueFamilyIndex,
		0, &logical_device.default_queue);

	return ErrStack();
}

static ErrStack find_layers(const std::vector<VkLayerProperties>& layer_props, const std::vector<const char*>& layers)
{
	for (const char* layer : layers) {

		bool found = false;
		for (VkLayerProperties layer_prop : layer_props) {
			if (!strcmp(layer, layer_prop.layerName)) {
				found = true;
				break;
			}
		}

		if (!found) {
			return ErrStack(code_location, "validation layer = " + std::string(layer) + " not found");
		}
	}
	return ErrStack();
}

static ErrStack find_extensions(const std::vector<VkExtensionProperties>& ext_props, const std::vector<const char*>& extensions)
{
	for (const char* extension : extensions) {

		bool found = false;
		for (VkExtensionProperties ext_prop : ext_props) {
			if (!strcmp(extension, ext_prop.extensionName)) {
				found = true;
				break;
			}
		}

		if (!found) {
			return ErrStack(code_location, "instance extension = " + std::string(extension) + " not found");
		}
	}
	return ErrStack();
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	printf("Vulkan Debug: %s \n\n", pCallbackData->pMessage);

	return VK_FALSE;
}

ErrStack Instance2::create(CreateInstanceInfo& info)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	// Find Layers
	{
		uint32_t layer_props_count = 0;

		checkVkRes(vkEnumerateInstanceLayerProperties(&layer_props_count, NULL),
			"could not retrieve validation layer props count");

		std::vector<VkLayerProperties> layer_props(layer_props_count);

		checkVkRes(vkEnumerateInstanceLayerProperties(&layer_props_count, layer_props.data()),
			"could not retrieve validation layer props");

		checkErrStack1(find_layers(layer_props, info.validation_layers));
	}

	// Find Extensions
	{
		uint32_t ext_props_count = 0;

		checkVkRes(vkEnumerateInstanceExtensionProperties(NULL, &ext_props_count, NULL),
			"could not retrieve instance extension props count");

		std::vector<VkExtensionProperties> ext_props(ext_props_count);

		checkVkRes(vkEnumerateInstanceExtensionProperties(NULL, &ext_props_count, ext_props.data()),
			"could not retrieve instance extension props");

		checkErrStack1(find_extensions(ext_props, info.instance_extensions));
	}

	// Create Instance
	{
		// AppInfo
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = info.app_info.pNext;
		app_info.pApplicationName = info.app_info.pApplicationName;
		app_info.applicationVersion = info.app_info.applicationVersion;
		app_info.pEngineName = info.app_info.pEngineName;
		app_info.engineVersion = info.app_info.engineVersion;
		app_info.apiVersion = info.app_info.apiVersion;

		// Instance Creation info
		VkInstanceCreateInfo inst_info = {};
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = info.pNext;
		inst_info.flags = info.flags;
		inst_info.pApplicationInfo = &app_info;
		inst_info.enabledLayerCount = info.validation_layers.size();
		inst_info.ppEnabledLayerNames = info.validation_layers.data();
		inst_info.enabledExtensionCount = info.instance_extensions.size();
		inst_info.ppEnabledExtensionNames = info.instance_extensions.data();

		checkVkRes(vkCreateInstance(&inst_info, NULL, &instance),
			"failed to create instance");
	}

	// Debug callback
	{
		VkDebugUtilsMessengerCreateInfoEXT dbg_info = {};
		dbg_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		dbg_info.pNext = info.debug_pnext;
		dbg_info.flags = info.debug_flags;
		dbg_info.messageSeverity = info.debug_msg_severity;
		dbg_info.messageType = info.debug_msg_type;
		dbg_info.pfnUserCallback = debugCallback;
		dbg_info.pUserData = NULL;

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != NULL) {

			checkVkRes(func(this->instance, &dbg_info, NULL, &callback),
				"debug callback creation failed");
		}
		else {
			return ErrStack(code_location, "debug extension not present");
		}
	}

	// External Functions
	{
		set_vkdbg_name_func = (PFN_vkSetDebugUtilsObjectNameEXT)
			vkGetInstanceProcAddr(this->instance, "vkSetDebugUtilsObjectNameEXT");
		if (set_vkdbg_name_func == NULL) {
			return ErrStack(code_location,
				"failed to retrieve function pointer for "
				"vkSetDebugUtilsObjectNameEXT");
		}
	}

	return err_stack;
}

ErrStack Instance2::createWin32Surface(SurfaceCreateInfo& info, Surface2& surface)
{
	VkResult vk_res{};

	surface = {};
	surface.instance = this;

	VkWin32SurfaceCreateInfoKHR vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vk_info.pNext = info.pNext;
	vk_info.flags = info.flags;
	vk_info.hinstance = info.hinstance;
	vk_info.hwnd = info.hwnd;

	checkVkRes(vkCreateWin32SurfaceKHR(instance, &vk_info, NULL, &surface.surface),
		"failed to create vulkan surface");

	if (default_surface == nullptr) {
		default_surface = &surface;
	}

	return ErrStack();
}

ErrStack Instance2::getPhysicalDevices(std::vector<PhysicalDevice2>& phys_devs)
{
	VkResult vk_res{};

	uint32_t deviceCount = 0;
	checkVkRes(vkEnumeratePhysicalDevices(instance, &deviceCount, NULL),
		"failed to enumerate physical devices");

	if (deviceCount == 0) {
		return ErrStack(code_location, "failed to find GPUs with Vulkan support");
	}

	std::vector<VkPhysicalDevice> vk_phys_devs(deviceCount);
	checkVkRes(vkEnumeratePhysicalDevices(instance, &deviceCount, vk_phys_devs.data()),
		"failed to enumerate physical devices");

	phys_devs.resize(vk_phys_devs.size());
	for (auto i = 0; i < phys_devs.size(); i++) {
		phys_devs[i].instance = this;
		phys_devs[i].physical_device = vk_phys_devs[i];
	}

	return ErrStack();
}

void Instance2::destroy()
{
	if (this->callback != VK_NULL_HANDLE) {

		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != NULL) {
			func(instance, callback, NULL);
		}
		else {
			printf("%s debug callback already freed \n", code_location);
		}
		callback = VK_NULL_HANDLE;
	}

	vkDestroyInstance(instance, NULL);
	this->instance = VK_NULL_HANDLE;
}

Instance2::~Instance2()
{
	if (instance != VK_NULL_HANDLE) {
		destroy();
	}
}

void Swapchain2::destroy()
{
	for (size_t i = 0; i < views.size(); i++) {
		vkDestroyImageView(dev->logical_device, views[i], NULL);
	}
	views.clear();

	vkDestroySwapchainKHR(dev->logical_device, swapchain, NULL);
	swapchain = VK_NULL_HANDLE;
}

Swapchain2::~Swapchain2()
{
	if (swapchain != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack Fence2::waitAndReset(uint64_t max_wait_time)
{
	VkResult vk_res{};

	checkVkRes(vkWaitForFences(dev->logical_device, 1, &fence, VK_TRUE, max_wait_time),
		"failed to wait on fence");

	checkVkRes(vkResetFences(dev->logical_device, 1, &fence),
		"failed to reset fence");

	return ErrStack();
}

void Fence2::destroy()
{
	vkDestroyFence(dev->logical_device, fence, NULL);
	fence = VK_NULL_HANDLE;
}

Fence2::~Fence2()
{
	if (fence != VK_NULL_HANDLE) {
		destroy();
	}
}

void Semaphore2::destroy()
{
	vkDestroySemaphore(dev->logical_device, semaphore, NULL);
	semaphore = VK_NULL_HANDLE;
}

Semaphore2::~Semaphore2()
{
	if (semaphore != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack CommandBuffer::beginRecording(VkCommandBufferUsageFlags flags = 0)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = flags;

	checkVkRes(vkBeginCommandBuffer(cmd_buff, &beginInfo),
		"failed to begin single use command buffer");

	return err_stack;
}

void CommandBuffer::copyBuffer(RawBuffer* src, RawBuffer* dst)
{
	VkBufferCopy cpy_info = {};
	cpy_info.srcOffset = 0;
	cpy_info.dstOffset = 0;
	cpy_info.size = src->vma_r_info.size;

	vkCmdCopyBuffer(cmd_buff, dst->buff, dst->buff, 1, &cpy_info);
}

void CommandBuffer::changeTextureLayout(Texture* texture, VkImageLayout new_layout)
{
	auto setUndefined = [](VkAccessFlags& access_mask, VkPipelineStageFlags& stage) {
		access_mask = 0;
		stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	};

	auto setTransferDst = [](VkAccessFlags& access_mask, VkPipelineStageFlags& stage) {
		access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
		stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	};

	auto setColorAttachWrite = [](VkAccessFlags& access_mask, VkPipelineStageFlags& stage) {
		access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	};

	auto setShaderReadFragment = [](VkAccessFlags& access_mask, VkPipelineStageFlags& stage) {
		access_mask = VK_ACCESS_SHADER_READ_BIT;
		stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	};

	VkPipelineStageFlags src_stage;
	VkPipelineStageFlags dst_stage;

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

	if (texture->current_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
		setUndefined(barrier.srcAccessMask, src_stage);

		if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			setTransferDst(barrier.dstAccessMask, dst_stage);
		}
		else if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			setColorAttachWrite(barrier.dstAccessMask, dst_stage);
		}
		else {
			printf("unsupported image layout transition");
		}
	}
	else if (texture->current_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		setTransferDst(barrier.srcAccessMask, src_stage);

		if (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			setShaderReadFragment(barrier.dstAccessMask, dst_stage);
		}
		else if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			setColorAttachWrite(barrier.dstAccessMask, dst_stage);
		}
		else {
			printf("unsupported image layout transition");
		}
	}
	else {
		printf("unsupported image layout transition");
	}

	barrier.oldLayout = texture->current_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = texture->img;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		cmd_buff,
		src_stage, dst_stage,
		0,
		0, NULL,
		0, NULL,
		1, &barrier
	);
}

ErrStack CommandBuffer::finishRecording(VkQueue queue = VK_NULL_HANDLE)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	checkVkRes(vkEndCommandBuffer(cmd_buff),
		"failed to end single use command buffer");

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd_buff;

	if (queue == VK_NULL_HANDLE) {
		checkVkRes(vkQueueSubmit(pool->dev->default_queue, 1, &submitInfo, fence.fence),
			"failed to submit single use command buffer");
	}
	else {
		checkVkRes(vkQueueSubmit(queue, 1, &submitInfo, fence.fence),
			"failed to submit single use command buffer");
	}
	
	checkErrStack1(fence.waitAndReset());

	return err_stack;
}

ErrStack CommandPool2::createCommandBuffer(CommandBufferCreateInfo& info, CommandBuffer& command_buffer)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	command_buffer = {};
	command_buffer.pool = this;

	FenceCreateInfo fence_info = {};

	checkErrStack1(dev->createFence(fence_info, command_buffer.fence));

	VkCommandBufferAllocateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vk_info.commandPool = pool;
	vk_info.level = info.level;
	vk_info.commandBufferCount = 1;

	checkVkRes(vkAllocateCommandBuffers(dev->logical_device, &vk_info, &command_buffer.cmd_buff),
		"failed to create command buffer");

	return err_stack;
}

void CommandPool2::destroy()
{
	vkDestroyCommandPool(dev->logical_device, pool, NULL);
	pool = VK_NULL_HANDLE;
}

CommandPool2::~CommandPool2()
{
	if (pool != VK_NULL_HANDLE) {
		destroy();
	}
}

void TextureView::destroy()
{
	vkDestroyImageView(texture->dev->logical_device, view, NULL);
	view = VK_NULL_HANDLE;
}

TextureView::~TextureView()
{
	if (view != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack Texture::createView(TextureViewCreateInfo& info, TextureView& view)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	view = {};
	view.texture = this;

	VkImageViewCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vk_info.pNext =  info.pNext;
	vk_info.flags = info.flags;
	vk_info.image = img;
	vk_info.viewType = info.viewType;
	vk_info.format = this->info.format;
	vk_info.components = info.components;
	vk_info.subresourceRange.aspectMask = info.sub_res.aspectMask;
	vk_info.subresourceRange.baseMipLevel = info.sub_res.baseMipLevel;
	vk_info.subresourceRange.levelCount = info.sub_res.levelCount;
	vk_info.subresourceRange.baseArrayLayer = info.sub_res.baseArrayLayer;
	vk_info.subresourceRange.layerCount = info.sub_res.layerCount;

	checkVkRes(vkCreateImageView(dev->logical_device, &vk_info, NULL, &view.view),
		"failed to create image view");

	return err_stack;
}

ErrStack Texture::setDebugName(std::string name)
{
	return dev->setDebugName(
		reinterpret_cast<uint64_t>(img), VK_OBJECT_TYPE_IMAGE, name + " VkImage");
}

void Texture::destroy()
{
	if (this->load_type == LoadType::MEMCPY) {
		vmaUnmapMemory(dev->allocator, alloc);
	}

	vmaDestroyImage(dev->allocator, img, alloc);
	img = VK_NULL_HANDLE;
}

Texture::~Texture()
{
	if (img != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack RawBuffer::create(size_t size, BufferCreateInfo& info)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	this->info = info;
	this->size = size;

	VkBufferCreateInfo buff_info = {};
	buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buff_info.flags = info.flags;
	buff_info.size = size;
	buff_info.usage = info.usage;
	buff_info.sharingMode = info.sharingMode;
	buff_info.queueFamilyIndexCount = info.queueFamilyIndexCount;
	buff_info.pQueueFamilyIndices = info.pQueueFamilyIndices;

	VmaAllocationCreateInfo vma_info = {};
	vma_info.usage = info.vma_usage;

	checkVkRes(vmaCreateBuffer(dev->allocator, &buff_info, &vma_info,
		&buff, &buff_alloc, &vma_r_info),
		"failed to create buffer");

	VkMemoryPropertyFlags mem_flags;
	vmaGetMemoryTypeProperties(dev->allocator, vma_r_info.memoryType, &mem_flags);
	if (mem_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
		load_type = LoadType::MEMCPY;

		checkVkRes(vmaMapMemory(dev->allocator, buff_alloc, &mem),
			"failed to map buffer");
	}
	else {
		load_type = LoadType::STAGING;
		this->mem = nullptr;
	}

	return err_stack;
}

ErrStack RawBuffer::resize(size_t new_size)
{
	ErrStack err_stack{};

	destroy();
	checkErrStack1(create(new_size, info));

	return err_stack;
}

ErrStack RawBuffer::loadStagedWait(size_t size, void* data, CommandBuffer& cmd_buff, RawBuffer& staging_buff)
{
	ErrStack err_stack{};

	// Copy to transfer
	std::memcpy(staging_buff.mem, data, size);

	checkErrStack1(cmd_buff.beginRecording());
	cmd_buff.copyBuffer(&staging_buff, this);
	checkErrStack1(cmd_buff.finishRecording());

	return err_stack;
}

void RawBuffer::loadMemcpy(size_t size, void* data)
{
	std::memcpy(mem, data, size);
}

size_t RawBuffer::getRequestedSize()
{
	return size;
}

size_t RawBuffer::getAllocatedSize()
{
	return vma_r_info.size;
}

void RawBuffer::destroy()
{
	if (this->load_type == LoadType::MEMCPY) {
		vmaUnmapMemory(dev->allocator, this->buff_alloc);
	}

	vmaDestroyBuffer(dev->allocator, this->buff, this->buff_alloc);
	buff = VK_NULL_HANDLE;
}

RawBuffer::~RawBuffer()
{
	if (buff != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack Renderpass2::createFramebuffers(FramebuffersCreateInfo& info, Framebuffers& framebuffer)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	framebuffer = {};
	framebuffer.dev = this->dev;

	if (!info.size) {
		info.size = dev->default_swapchain->images.size();
	}
	framebuffer.framebuffs.resize(info.size);

	if (!info.width) {
		info.width = dev->default_swapchain->info.width;
		info.height = dev->default_swapchain->info.height;
	}

	std::vector<VkImageView> vk_views(info.atachments.size());
	for (uint32_t i = 0; i < vk_views.size(); i++) {
		vk_views[i] = info.atachments[i]->view;
	}

	for (uint32_t i = 0; i < info.size; i++) {

		VkFramebufferCreateInfo framebuff_info = {};
		framebuff_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuff_info.flags = info.flags;
		framebuff_info.renderPass = renderpass;
		framebuff_info.attachmentCount = vk_views.size();
		framebuff_info.pAttachments = vk_views.data();
		framebuff_info.width = info.width;
		framebuff_info.height = info.height;
		framebuff_info.layers = info.layers;

		checkVkRes(vkCreateFramebuffer(dev->logical_device, &framebuff_info, NULL,
			&framebuffer.framebuffs[i]), "failed to create framebuffer");
	}

	return err_stack;
}

void Framebuffers::destroy()
{
	for (VkFramebuffer& frame_buff : framebuffs) {

		vkDestroyFramebuffer(dev->logical_device, frame_buff, NULL);
	}
	framebuffs.clear();
}

Framebuffers::~Framebuffers()
{
	destroy();
}

void Renderpass2::destroy()
{
	vkDestroyRenderPass(dev->logical_device, renderpass, NULL);
	renderpass = VK_NULL_HANDLE;
}

Renderpass2::~Renderpass2()
{
	if (renderpass != VK_NULL_HANDLE) {
		destroy();
	}
}

void DescriptorSetLayout2::destroy()
{
	vkDestroyDescriptorSetLayout(dev->logical_device, layout, NULL);
	layout = VK_NULL_HANDLE;
}

DescriptorSetLayout2::~DescriptorSetLayout2()
{
	if (layout != VK_NULL_HANDLE) {
		destroy();
	}
}

void PipelineLayout2::destroy()
{
	vkDestroyPipelineLayout(dev->logical_device, layout, NULL);
	layout = VK_NULL_HANDLE;
}

PipelineLayout2::~PipelineLayout2()
{
	if (layout != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack Shader::create(std::vector<char>& code)
{
	VkResult vk_res{};

	VkShaderModuleCreateInfo shader_module_info = {};
	shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_info.codeSize = code.size();
	shader_module_info.pCode = reinterpret_cast<uint32_t*>(code.data());

	checkVkRes(vkCreateShaderModule(dev->logical_device, &shader_module_info, NULL, &shader),
		"failed to create shader module");

	return ErrStack();
}

void Shader::destroy()
{
	vkDestroyShaderModule(dev->logical_device, shader, NULL);
	shader = VK_NULL_HANDLE;
}

Shader::~Shader()
{
	if (shader != VK_NULL_HANDLE) {
		destroy();
	}
}

void GraphicsPipeline2::destroy()
{
	vkDestroyPipeline(dev->logical_device, pipeline, NULL);
	pipeline = VK_NULL_HANDLE;
}

GraphicsPipeline2::~GraphicsPipeline2()
{
	if (pipeline != VK_NULL_HANDLE) {
		destroy();
	}
}

void DescriptorSet2::update(std::vector<DescriptorSetWriteInfo> writes)
{
	std::vector<VkWriteDescriptorSet> vk_writes(writes.size());
	for (uint32_t i = 0; i < writes.size(); i++) {

		VkWriteDescriptorSet& vk_write = vk_writes[i];
		DescriptorSetWriteInfo& write = writes[i];

		vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vk_write.dstSet = set;
		vk_write.dstBinding = write.dstBinding;
		vk_write.dstArrayElement = write.dstArrayElement;
		vk_write.descriptorCount = write.descriptorCount;
		vk_write.descriptorType = write.descriptorType;

		switch (write.resource.index()) {
		case 0: {
			auto resource_info = std::get_if<DescriptorSetImageInfo>(&write.resource);

			write._image_info.sampler = resource_info->sampler;
			write._image_info.imageView = resource_info->imageView;
			write._image_info.imageLayout = resource_info->imageLayout;
			vk_write.pImageInfo = &write._image_info;
			break;
		}
		case 1: {
			auto resource_info = std::get_if<DescriptorSetBufferInfo>(&write.resource);

			write._buff_info.buffer = resource_info->buffer;
			write._buff_info.offset = resource_info->offset;
			write._buff_info.range = resource_info->range;
			vk_write.pBufferInfo = &write._buff_info;
			break;
		}
		}
	}

	vkUpdateDescriptorSets(pool->dev->logical_device,
		vk_writes.size(), vk_writes.data(), 0, NULL);
}

void DescriptorPool2::createDescriptorSet(DescriptorSetLayout2* layout, DescriptorSet2& set)
{
	this->layouts.push_back(layout);

	auto add = [&](DescriptorSetLayoutBinding& binding) {

		// increment existing size
		for (auto& size : sizes) {
			if (size.type == binding.descriptorType) {
				size.descriptorCount++;
				return;
			}
		}

		// add new size
		auto& new_size = sizes.emplace_back();
		new_size.type = binding.descriptorType;
		new_size.descriptorCount = 1;
	};

	// for each binding of the implemented layout
	for (auto& binding : layout->bindings) {
		add(binding);
	}

	set = sets.emplace_back();
	set = {};
	set.pool = this;
}

ErrStack DescriptorPool2::allocate()
{
	VkResult vk_res{};

	uint32_t max_sets = 0;
	for (auto& size : sizes) {
		max_sets += size.descriptorCount;
	}

	VkDescriptorPoolCreateInfo descp_pool_info = {};
	descp_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descp_pool_info.flags = flags;
	descp_pool_info.maxSets = max_sets;
	descp_pool_info.poolSizeCount = sizes.size();
	descp_pool_info.pPoolSizes = sizes.data();

	checkVkRes(vkCreateDescriptorPool(dev->logical_device, &descp_pool_info, NULL, &pool),
		"failed to create descriptor pool");

	// Allocate Sets from pool
	std::vector<VkDescriptorSetLayout> vk_layouts(layouts.size());
	for (uint32_t i = 0; i < vk_layouts.size(); i++) {
		vk_layouts[i] = layouts[i]->layout;
	}

	std::vector<VkDescriptorSet> descp_sets(layouts.size());

	VkDescriptorSetAllocateInfo descp_sets_info = {};
	descp_sets_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descp_sets_info.descriptorPool = pool;
	descp_sets_info.descriptorSetCount = vk_layouts.size();
	descp_sets_info.pSetLayouts = vk_layouts.data();

	checkVkRes(vkAllocateDescriptorSets(dev->logical_device, &descp_sets_info, descp_sets.data()),
		"failed to allocate descriptor sizes");

	// sets
	for (uint32_t i = 0; i < layouts.size(); i++) {
		sets[i].set = descp_sets[i];
	}
	
	return ErrStack();
}

void DescriptorPool2::destroy()
{
	vkDestroyDescriptorPool(dev->logical_device, pool, NULL);
	pool = VK_NULL_HANDLE;
}

DescriptorPool2::~DescriptorPool2()
{
	if (pool != VK_NULL_HANDLE) {
		destroy();
	}
}
