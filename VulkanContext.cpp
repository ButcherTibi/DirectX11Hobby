
// Header
#include "VulkanSystems.hpp"

using namespace vks;


ErrStack VulkanFence::waitAndReset(uint64_t max_wait_time)
{
	VkResult vk_res{};

	checkVkRes(vkWaitForFences(context->logical_dev.logical_device, 1, &fence, VK_TRUE, max_wait_time),
		"failed to wait on fence");

	checkVkRes(vkResetFences(context->logical_dev.logical_device, 1, &fence),
		"failed to reset fence");

	return ErrStack();
}

void VulkanFence::destroy()
{
	vkDestroyFence(context->logical_dev.logical_device, fence, NULL);
	fence = VK_NULL_HANDLE;
}

VulkanFence::~VulkanFence()
{
	if (fence != VK_NULL_HANDLE) {
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

	if (texture->layout == VK_IMAGE_LAYOUT_UNDEFINED) {
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
	else if (texture->layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
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

	barrier.oldLayout = texture->layout;
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

ErrStack CommandBuffer::finishRecording()
{
	ErrStack err_stack{};
	VkResult vk_res{};

	checkVkRes(vkEndCommandBuffer(cmd_buff),
		"failed to end single use command buffer");

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd_buff;

	checkVkRes(vkQueueSubmit(context->logical_dev.queue, 1, &submitInfo, fence.fence),
		"failed to submit single use command buffer");

	checkErrStack1(fence.waitAndReset());

	return err_stack;
}

ErrStack Texture::addView(const TextureViewCreateInfo& info)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	VkComponentMapping component_mapping = {};
	VkImageSubresourceRange sub_range = {};
	sub_range.aspectMask = info.aspect;
	sub_range.levelCount = 1;
	sub_range.layerCount = 1;

	VkImageViewCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vk_info.image = img;
	vk_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	vk_info.format = format;
	vk_info.components = component_mapping;
	vk_info.subresourceRange = sub_range;

	checkVkRes(vkCreateImageView(context->logical_dev.logical_device, &vk_info, NULL, &view),
		"failed to create image view");

	return err_stack;
}

void Texture::destroy()
{
	if (this->load_type == LoadType::MEMCPY) {
		vmaUnmapMemory(context->logical_dev.allocator, alloc);
	}

	vmaDestroyImage(context->logical_dev.allocator, img, alloc);
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

	checkVkRes(vmaCreateBuffer(context->logical_dev.allocator, &buff_info, &vma_info,
		&buff, &buff_alloc, &vma_r_info),
		"failed to create buffer");

	VkMemoryPropertyFlags mem_flags;
	vmaGetMemoryTypeProperties(context->logical_dev.allocator, vma_r_info.memoryType, &mem_flags);
	if (mem_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
		load_type = LoadType::MEMCPY;

		checkVkRes(vmaMapMemory(context->logical_dev.allocator, buff_alloc, &mem),
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

void RawBuffer::destroy()
{
	if (this->load_type == LoadType::MEMCPY) {
		vmaUnmapMemory(context->logical_dev.allocator, this->buff_alloc);
	}

	vmaDestroyBuffer(context->logical_dev.allocator, this->buff, this->buff_alloc);
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
	framebuffer.context = context;

	for (uint32_t i = 0; i < context->swapchain.images.size(); i++) {

		VkFramebufferCreateInfo framebuff_info = {};
		framebuff_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuff_info.flags = info.flags;
		framebuff_info.renderPass = renderpass;
		framebuff_info.attachmentCount = info.atachments.size();
		framebuff_info.pAttachments = info.atachments.data();
		framebuff_info.width = context->width;
		framebuff_info.height = context->height;
		framebuff_info.layers = info.layers;

		checkVkRes(vkCreateFramebuffer(context->logical_dev.logical_device, &framebuff_info, NULL,
			&framebuffer.framebuffs[i]), "failed to create framebuffer");
	}

	return err_stack;
}

void Framebuffers::destroy()
{
	for (VkFramebuffer& frame_buff : framebuffs) {

		vkDestroyFramebuffer(context->logical_dev.logical_device, frame_buff, NULL);
	}
	framebuffs.clear();
}

Framebuffers::~Framebuffers()
{
	destroy();
}

void Renderpass2::destroy()
{
	vkDestroyRenderPass(context->logical_dev.logical_device, renderpass, NULL);
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
	vkDestroyDescriptorSetLayout(context->logical_dev.logical_device, layout, NULL);
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
	vkDestroyPipelineLayout(context->logical_dev.logical_device, layout, NULL);
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

	checkVkRes(vkCreateShaderModule(context->logical_dev.logical_device, &shader_module_info, NULL, &shader),
		"failed to create shader module");

	return ErrStack();
}

void Shader::destroy()
{
	vkDestroyShaderModule(context->logical_dev.logical_device, shader, NULL);
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
	vkDestroyPipeline(context->logical_dev.logical_device, pipeline, NULL);
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

	vkUpdateDescriptorSets(pool->context->logical_dev.logical_device,
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

	checkVkRes(vkCreateDescriptorPool(context->logical_dev.logical_device, &descp_pool_info, NULL, &pool),
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

	checkVkRes(vkAllocateDescriptorSets(context->logical_dev.logical_device, &descp_sets_info, descp_sets.data()),
		"failed to allocate descriptor sizes");

	// sets
	for (uint32_t i = 0; i < layouts.size(); i++) {
		sets[i].set = descp_sets[i];
	}
	
	return ErrStack();
}

void DescriptorPool2::destroy()
{
	vkDestroyDescriptorPool(context->logical_dev.logical_device, pool, NULL);
	pool = VK_NULL_HANDLE;
}

DescriptorPool2::~DescriptorPool2()
{
	if (pool != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack Context::create(HWND hwnd, HINSTANCE hinstance)
{
	ErrStack err_stack{};

	checkErrStack1(instance.create());
	checkErrStack1(surface.create(&instance, hinstance, hwnd));
	checkErrStack1(phys_dev.create(&instance, &surface));
	checkErrStack1(logical_dev.create(&instance, &phys_dev));

	checkErrStack1(command_pool.create(&logical_dev, &phys_dev));
	checkErrStack1(createCommandBuffer(command_buff));

	checkErrStack1(getPhysicalSurfaceResolution(width, height));
	checkErrStack1(swapchain.create(&surface, &phys_dev, &logical_dev,
		width, height));

	return err_stack;
}

ErrStack Context::getPhysicalSurfaceResolution(uint32_t& width, uint32_t& height)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	VkSurfaceCapabilitiesKHR capabilities = {};
	checkVkRes(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev.physical_device, surface.surface, &capabilities),
		"failed to get physical device surface capabilities");

	VkExtent2D min_img_extent = capabilities.minImageExtent;
	VkExtent2D max_img_extent = capabilities.maxImageExtent;

	width = glm::clamp(width, min_img_extent.width, max_img_extent.width);
	height = glm::clamp(height, min_img_extent.height, max_img_extent.height);

	return err_stack;
}

ErrStack Context::createFence(VulkanFence& fence, VkFenceCreateFlags flags)
{
	VkResult vk_res{};

	fence = {};
	fence.context = this;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = flags;

	checkVkRes(vkCreateFence(logical_dev.logical_device, &fence_info, NULL, &fence.fence),
		"failed to create fence");

	return ErrStack();
}

ErrStack Context::createCommandBuffer(CommandBuffer& command_buffer)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	command_buffer = {};
	command_buffer.context = this;

	// Fence
	checkErrStack1(createFence(command_buffer.fence));

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = command_pool.cmd_pool;
	allocInfo.commandBufferCount = 1;

	checkVkRes(vkAllocateCommandBuffers(logical_dev.logical_device, &allocInfo, &command_buffer.cmd_buff),
		"failed to create command buffer");

	return err_stack;
}

ErrStack Context::createTexture(TextureCreateInfo& info, Texture& texture)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	texture = {};
	texture.context = this;
	texture.format = info.format;

	if (!info.width) {
		texture.width = width;
		texture.height = height;
	}
	else {
		texture.width = info.width;
		texture.height = info.height;
	}
	texture.layout = info.layout;

	VmaAllocationCreateInfo alloc_create_info = {};
	alloc_create_info.usage = info.mem_usage;

	VkImageCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	vk_info.imageType = VK_IMAGE_TYPE_2D;
	vk_info.format = info.format;
	vk_info.extent.width = info.width;
	vk_info.extent.height = info.height;
	vk_info.extent.depth = 1;
	vk_info.mipLevels = 0;
	vk_info.arrayLayers = 1;
	vk_info.samples = VK_SAMPLE_COUNT_1_BIT;
	vk_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	vk_info.usage = info.usage;
	vk_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vk_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	checkVkRes(vmaCreateImage(logical_dev.allocator, &vk_info, &alloc_create_info,
		&texture.img, &texture.alloc, &texture.alloc_info),
		"failed to create image");

	VkMemoryPropertyFlags mem_flags;
	vmaGetMemoryTypeProperties(logical_dev.allocator, texture.alloc_info.memoryType, &mem_flags);

	if (mem_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {

		texture.load_type = LoadType::MEMCPY;
		checkVkRes(vmaMapMemory(logical_dev.allocator, texture.alloc, &texture.mem), "");
	}
	else {
		texture.load_type = LoadType::STAGING;
		texture.mem = nullptr;
	}

	if (info.layout != VK_IMAGE_LAYOUT_UNDEFINED) {
		checkErrStack1(command_buff.beginRecording());
		command_buff.changeTextureLayout(&texture, info.layout);
		checkErrStack1(command_buff.finishRecording());
	}

	return err_stack;
}

ErrStack Context::createRenderpass(RenderpassCreateInfo& info, Renderpass2& renderpass)
{
	VkResult vk_res{};

	renderpass = {};
	renderpass.context = this;

	std::vector<VkAttachmentDescription> atach_descps(info.atach_descps.size());
	for (uint32_t i = 0; i < info.atach_descps.size(); i++) {

		VkAttachmentDescription& vk_descp = atach_descps[i];
		AtachmentCreateInfo& descp = info.atach_descps[i];

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

	checkVkRes(vkCreateRenderPass(logical_dev.logical_device, &vk_info, NULL, &renderpass.renderpass),
		"failed to create renderpass");

	return ErrStack();
}

ErrStack Context::createDescriptorSetLayout(DescriptorSetLayoutCreateInfo& info, DescriptorSetLayout2& descp_set_layout)
{
	VkResult vk_res{};

	descp_set_layout = {};
	descp_set_layout.context = this;

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

	checkVkRes(vkCreateDescriptorSetLayout(logical_dev.logical_device, &descp_layout_info, NULL, &descp_set_layout.layout),
		"failed to create descriptor set layout");
	
	return ErrStack();
}

ErrStack Context::createPipelineLayout(PipelineLayoutCreateInfo& info, PipelineLayout2& pipe_layout)
{
	VkResult vk_res{};

	pipe_layout = {};
	pipe_layout.context = this;

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

	checkVkRes(vkCreatePipelineLayout(logical_dev.logical_device, &vk_info, NULL, &pipe_layout.layout),
		"failed to create pipeline layout");

	return ErrStack();
}

ErrStack Context::createVertexShader(std::vector<char>& code, Shader& vertex_shader)
{
	vertex_shader = {};
	vertex_shader.context = this;
	vertex_shader.stage = VK_SHADER_STAGE_VERTEX_BIT;

	return vertex_shader.create(code);
}

ErrStack Context::createFragmentShader(std::vector<char>& code, Shader& fragment_shader)
{
	fragment_shader = {};
	fragment_shader.context = this;
	fragment_shader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

	return fragment_shader.create(code);
}

ErrStack Context::createGraphicsPipeline(GraphicsPipelineCreateInfo& info, GraphicsPipeline2& graphics_pipe)
{
	VkResult vk_res{};
	
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
			vk_viewports[i].width = (float)width;
			vk_viewports[i].height = (float)height;
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
			vk_scissors[i].extent.width = width;
			vk_scissors[i].extent.height = height;
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

	checkVkRes(vkCreateGraphicsPipelines(logical_dev.logical_device, NULL, 1, &vk_info, NULL, &graphics_pipe.pipeline),
		"failed to create graphics pipeline");

	return ErrStack();
}

void Context::createDescriptorPool(DescriptorPoolCreateInfo& info, DescriptorPool2& descp_pool)
{
	descp_pool = {};
	descp_pool.flags = info.flags;
}
