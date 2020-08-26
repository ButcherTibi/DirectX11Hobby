
// Header
#include "VulkanNext.hpp"


using namespace vnx;


ErrStack Renderpass::addReadColorAttachment(ReadAttachmentInfo& info)
{
	Image* img = info.view->image;

	// Renderpass
	uint32_t atach_idx = atach_descps.size();
	VkAttachmentDescription& atach_descp = atach_descps.emplace_back();
	atach_descp.flags = 0;
	atach_descp.format = img->format;
	atach_descp.samples = img->samples;
	atach_descp.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	atach_descp.storeOp = info.store_op;
	atach_descp.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	atach_descp.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	atach_descp.initialLayout = info.initial_layout;
	atach_descp.finalLayout = info.final_fayout;

	VkAttachmentReference& atach_ref = atach_refs.emplace_back();
	atach_ref.attachment = atach_idx;
	atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	input_atachs.push_back(atach_ref);

	// Framebuffs
	if (img_views.size() != atach_idx) {
		return ErrStack(code_location, "renderpass atach to framebuffs atach indexing missmatch");
	}
	img_views.push_back(info.view->view);

	// Descriptor Layout
	DescriptorLayout* layout;

	if (layouts.size() < info.set) {
		return ErrStack(code_location, "previous set not created");
	}
	else if (layouts.size() == info.set) {
		layout = &layouts.emplace_back();
	}
	else {
		layout = &layouts.back();
	}
	
	VkDescriptorSetLayoutBinding& binding = layout->bindings.emplace_back();
	binding.binding = info.binding;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	binding.descriptorCount = info.descriptor_count;
	binding.stageFlags = info.stages;
	binding.pImmutableSamplers = NULL;

	// Write Descriptor
	VkDescriptorImageInfo img_info = {};
	img_info.imageView = info.view->view;
	img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	DescriptorWriteResource& resource = layout->resources.emplace_back();
	resource.value = img_info;

	VkWriteDescriptorSet& write = layout->writes.emplace_back();
	write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstBinding = info.binding;
	write.dstArrayElement = info.dst_array_element;
	write.descriptorCount = info.descriptor_count;
	write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	write.pImageInfo = std::get_if<VkDescriptorImageInfo>(&resource.value);

	// Descriptor Pool
	[&]() {
		for (auto& size : descp_sizes) {
			if (size.type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
				size.descriptorCount += info.descriptor_count;
				return;
			}
		}

		auto& new_size = descp_sizes.emplace_back();
		new_size.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		new_size.descriptorCount = info.descriptor_count;
	}();

	// Command Renderpass
	VkClearValue& clear_value = clear_values.emplace_back();
	clear_value = info.clear_value;

	return ErrStack();
}

ErrStack Renderpass::addWriteColorAttachment(WriteAttachmentInfo& info)
{
	Image* img = info.view->image;

	// Renderpass
	uint32_t atach_idx = atach_descps.size();
	VkAttachmentDescription& atach_descp = atach_descps.emplace_back();
	atach_descp.flags = 0;
	atach_descp.format = img->format;
	atach_descp.samples = img->samples;
	atach_descp.loadOp = info.load_op;
	atach_descp.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	atach_descp.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	atach_descp.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	atach_descp.initialLayout = info.initial_layout;
	atach_descp.finalLayout = info.final_layout;

	VkAttachmentReference& atach_ref = atach_refs.emplace_back();
	atach_ref.attachment = atach_idx;
	atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	color_atachs.push_back(atach_ref);

	// Framebuffs
	if (img_views.size() != atach_idx) {
		return ErrStack(code_location, "renderpass atach to framebuffs atach indexing missmatch");
	}
	img_views.push_back(info.view->view);

	// Color Blending
	auto& blend = blend_atachs.emplace_back();
	blend.blendEnable = info.blendEnable;
	blend.srcColorBlendFactor = info.srcColorBlendFactor;
	blend.dstColorBlendFactor = info.dstColorBlendFactor;
	blend.colorBlendOp = info.colorBlendOp;
	blend.srcAlphaBlendFactor = info.srcAlphaBlendFactor;
	blend.dstAlphaBlendFactor = info.dstAlphaBlendFactor;
	blend.alphaBlendOp = info.alphaBlendOp;
	blend.colorWriteMask = info.colorWriteMask;

	// Command Renderpass
	VkClearValue& clear_value = clear_values.emplace_back();
	clear_value = info.clear_value;

	return ErrStack();
}

ErrStack Renderpass::addReadWriteColorAttachment(ReadWriteAttachment& info)
{
	Image* img = info.view->image;

	// Renderpass
	uint32_t atach_idx = atach_descps.size();
	VkAttachmentDescription& atach_descp = atach_descps.emplace_back();
	atach_descp.flags = 0;
	atach_descp.format = img->format;
	atach_descp.samples = img->samples;
	atach_descp.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	atach_descp.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	atach_descp.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	atach_descp.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	atach_descp.initialLayout = info.initial_layout;
	atach_descp.finalLayout = info.final_layout;

	VkAttachmentReference& read_ref = atach_refs.emplace_back();
	read_ref.attachment = atach_idx;
	read_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference& write_ref = atach_refs.emplace_back();
	write_ref.attachment = atach_idx;
	write_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	input_atachs.push_back(read_ref);
	color_atachs.push_back(write_ref);

	// Framebuffs
	if (img_views.size() != atach_idx) {
		return ErrStack(code_location, "renderpass atach to framebuffs atach indexing missmatch");
	}
	img_views.push_back(info.view->view);

	// Descriptor Layout
	DescriptorLayout* layout;

	if (layouts.size() < info.set) {
		return ErrStack(code_location, "previous set not created");
	}
	else if (layouts.size() == info.set) {
		layout = &layouts.emplace_back();
	}
	else {
		layout = &layouts.back();
	}

	VkDescriptorSetLayoutBinding& binding = layout->bindings.emplace_back();
	binding.binding = info.binding;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	binding.descriptorCount = info.descriptor_count;
	binding.stageFlags = info.stages;
	binding.pImmutableSamplers = NULL;

	// Write Descriptor
	VkDescriptorImageInfo img_info = {};
	img_info.imageView = info.view->view;
	img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	DescriptorWriteResource& resource = layout->resources.emplace_back();
	resource.value = img_info;

	VkWriteDescriptorSet& write = layout->writes.emplace_back();
	write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = NULL;
	write.dstBinding = info.binding;
	write.dstArrayElement = info.dst_array_element;
	write.descriptorCount = info.descriptor_count;
	write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	write.pImageInfo = std::get_if<VkDescriptorImageInfo>(&resource.value);

	// Descriptor Pool
	[&]() {
		for (auto& size : descp_sizes) {
			if (size.type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
				size.descriptorCount += info.descriptor_count;
				return;
			}
		}

		auto& new_size = descp_sizes.emplace_back();
		new_size.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		new_size.descriptorCount = info.descriptor_count;
	}();

	// Color Blending
	auto& blend = blend_atachs.emplace_back();
	blend.blendEnable = info.blendEnable;
	blend.srcColorBlendFactor = info.srcColorBlendFactor;
	blend.dstColorBlendFactor = info.dstColorBlendFactor;
	blend.colorBlendOp = info.colorBlendOp;
	blend.srcAlphaBlendFactor = info.srcAlphaBlendFactor;
	blend.dstAlphaBlendFactor = info.dstAlphaBlendFactor;
	blend.alphaBlendOp = info.alphaBlendOp;
	blend.colorWriteMask = info.colorWriteMask;

	// Command Renderpass
	VkClearValue& clear_value = clear_values.emplace_back();
	clear_value = info.clear_value;

	return ErrStack();
}

ErrStack Renderpass::bindStorageBuffer(StorageBufferBinding& info)
{
	// Descriptor Layout
	DescriptorLayout* layout;

	if (layouts.size() < info.set) {
		return ErrStack(code_location, "previous set not created");
	}
	else if (layouts.size() == info.set) {
		layout = &layouts.emplace_back();
	}
	else {
		layout = &layouts.back();
	}

	VkDescriptorSetLayoutBinding& binding = layout->bindings.emplace_back();
	binding.binding = info.binding;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	binding.descriptorCount = info.descriptor_count;
	binding.stageFlags = info.stages;
	binding.pImmutableSamplers = NULL;

	// Write Descriptor
	VkDescriptorBufferInfo buff_info = {};
	buff_info.buffer = info.buff->buff;
	buff_info.offset = info.offset;
	buff_info.range = info.range;

	DescriptorWriteResource& resource = layout->resources.emplace_back();
	resource.value = buff_info;

	VkWriteDescriptorSet& write = layout->writes.emplace_back();
	write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = NULL;
	write.dstBinding = info.binding;
	write.dstArrayElement = info.dst_array_element;
	write.descriptorCount = info.descriptor_count;
	write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	write.pBufferInfo = std::get_if<VkDescriptorBufferInfo>(&resource.value);

	// Descriptor Pool
	[&]() {
		for (auto& size : descp_sizes) {
			if (size.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
				size.descriptorCount += info.descriptor_count;
				return;
			}
		}

		auto& new_size = descp_sizes.emplace_back();
		new_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		new_size.descriptorCount = info.descriptor_count;
	}();

	return ErrStack();
}

ErrStack Renderpass::bindUniformBuffer(UniformBufferBinding& info)
{
	// Descriptor Layout
	DescriptorLayout* layout;

	if (layouts.size() < info.set) {
		return ErrStack(code_location, "previous set not created");
	}
	else if (layouts.size() == info.set) {
		layout = &layouts.emplace_back();
	}
	else {
		layout = &layouts.back();
	}

	VkDescriptorSetLayoutBinding& binding = layout->bindings.emplace_back();
	binding.binding = info.binding;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	binding.descriptorCount = info.descriptor_count;
	binding.stageFlags = info.stages;
	binding.pImmutableSamplers = NULL;

	// Write Descriptor
	VkDescriptorBufferInfo buff_info = {};
	buff_info.buffer = info.buff->buff;
	buff_info.offset = info.offset;
	buff_info.range = info.range;

	DescriptorWriteResource& resource = layout->resources.emplace_back();
	resource.value = buff_info;

	VkWriteDescriptorSet& write = layout->writes.emplace_back();
	write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstBinding = info.binding;
	write.dstArrayElement = info.dst_array_element;
	write.descriptorCount = info.descriptor_count;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write.pBufferInfo = std::get_if<VkDescriptorBufferInfo>(&resource.value);

	// Descriptor Pool
	[&]() {
		for (auto& size : descp_sizes) {
			if (size.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				size.descriptorCount += info.descriptor_count;
				return;
			}
		}

		auto& new_size = descp_sizes.emplace_back();
		new_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		new_size.descriptorCount = info.descriptor_count;
	}();

	return ErrStack();
}

void Renderpass::setVertexShader(Shader& vs)
{
	auto& shader = shaders.emplace_back();
	shader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader.module = vs.shader;
	shader.pName = "main";
}

void Renderpass::setFragmentShader(Shader& fs)
{
	auto& shader = shaders.emplace_back();
	shader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader.module = fs.shader;
	shader.pName = "main";
}

ErrStack Renderpass::build()
{
	ErrStack err_stack{};
	VkResult vk_res{};

	// Renderpass
	{
		VkSubpassDescription subpass_descp = {};
		subpass_descp.flags = 0;
		subpass_descp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_descp.inputAttachmentCount = input_atachs.size();
		subpass_descp.pInputAttachments = input_atachs.data();
		subpass_descp.colorAttachmentCount = color_atachs.size();
		subpass_descp.pColorAttachments = color_atachs.data();
		// subpass_descp.pResolveAttachments;
		// subpass_descp.pDepthStencilAttachment;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = atach_descps.size();
		info.pAttachments = atach_descps.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpass_descp;
		info.dependencyCount = 0;
		info.pDependencies = NULL;

		checkVkRes(vkCreateRenderPass(dev->logical_dev, &info, NULL, &renderpass),
			"failed to create renderpass");
	}
	
	// Framebuffs
	{
		framebuffs.resize(surface->minImageCount);

		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.flags = 0;
		info.renderPass = renderpass;
		info.attachmentCount = img_views.size();
		info.pAttachments = img_views.data();
		info.width = surface->width;
		info.height = surface->height;
		info.layers = 1;

		for (VkFramebuffer& framebuff : framebuffs) {

			checkVkRes(vkCreateFramebuffer(dev->logical_dev, &info, NULL, &framebuff),
				"failed to create framebuffer");
		}
	}

	// Descriptor Layout
	{
		for (auto& layout : layouts) {

			VkDescriptorSetLayoutCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = layout.bindings.size();
			info.pBindings = layout.bindings.data();

			checkVkRes(vkCreateDescriptorSetLayout(dev->logical_dev, &info, NULL, &layout.layout),
				"failed to create descriptor set layout");
		}
	}

	// Descriptor Pool
	{
		uint32_t max_sets = 0;
		for (auto& size : descp_sizes) {
			max_sets += size.descriptorCount;
		}

		VkDescriptorPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.maxSets = max_sets;
		info.poolSizeCount = descp_sizes.size();
		info.pPoolSizes = descp_sizes.data();

		checkVkRes(vkCreateDescriptorPool(dev->logical_dev, &info, NULL, &descp_pool),
			"failed to create descriptor pool");
	}

	// Pipeline Layout
	{
		std::vector<VkDescriptorSetLayout> vk_layouts(layouts.size());
		for (uint32_t i = 0; i < layouts.size(); i++) {
			vk_layouts[i] = layouts[i].layout;
		}

		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = vk_layouts.size();
		info.pSetLayouts = vk_layouts.data();

		checkVkRes(vkCreatePipelineLayout(dev->logical_dev, &info, NULL, &pipe_layout),
			"failed to create pipeline layout");
	}

	// Pipeline Layout
	{
		VkPipelineVertexInputStateCreateInfo vi_info = {};
		vi_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi_info.vertexBindingDescriptionCount = 1;
		vi_info.pVertexBindingDescriptions = &vertex_input_state.binding;
		vi_info.vertexAttributeDescriptionCount = vertex_input_state.atributes.size();
		vi_info.pVertexAttributeDescriptions = vertex_input_state.atributes.data();

		VkPipelineInputAssemblyStateCreateInfo ia_info = {};
		ia_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia_info.topology = input_assembly_state.topology;
		ia_info.primitiveRestartEnable = input_assembly_state.primitiveRestartEnable;
	
		std::vector<VkViewport> vk_viewports(viewport_state.viewports.size());
		for (uint32_t i = 0; i < vk_viewports.size(); i++) {

			Viewport& viewport = viewport_state.viewports[i];

			vk_viewports[i].x = viewport.x;
			vk_viewports[i].y = viewport.y;

			if (!viewport.width) {
				vk_viewports[i].width = (float)surface->width;
				vk_viewports[i].height = (float)surface->height;
			}
			else {
				vk_viewports[i].width = viewport.width;
				vk_viewports[i].height = viewport.height;
			}

			vk_viewports[i].x = viewport.x;
			vk_viewports[i].x = viewport.x;
		}

		std::vector<VkRect2D> vk_scissors(viewport_state.scissors.size());
		for (uint32_t i = 0; i < vk_scissors.size(); i++) {

			Scissor& scissor = viewport_state.scissors[i];

			vk_scissors[i].offset = scissor.offset;

			if (!scissor.extent.width) {
				vk_scissors[i].extent.width = surface->width;
				vk_scissors[i].extent.height = surface->height;
			}
			else {
				vk_scissors[i].extent = scissor.extent;
			}
		}

		VkPipelineViewportStateCreateInfo vp_info = {};
		vp_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vp_info.viewportCount = vk_viewports.size();
		vp_info.pViewports = vk_viewports.data();
		vp_info.scissorCount = vk_scissors.size();
		vp_info.pScissors = vk_scissors.data();

		VkPipelineRasterizationStateCreateInfo rs_info = {};
		rs_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs_info.depthClampEnable = rasterization_state.depthClampEnable;
		rs_info.rasterizerDiscardEnable = rasterization_state.rasterizerDiscardEnable;
		rs_info.polygonMode = rasterization_state.polygonMode;
		rs_info.cullMode = rasterization_state.cullMode;
		rs_info.frontFace = rasterization_state.frontFace;
		rs_info.depthBiasEnable = rasterization_state.depthBiasEnable;
		rs_info.depthBiasConstantFactor = rasterization_state.depthBiasConstantFactor;
		rs_info.depthBiasClamp = rasterization_state.depthBiasClamp;
		rs_info.depthBiasSlopeFactor = rasterization_state.depthBiasSlopeFactor;
		rs_info.lineWidth = rasterization_state.lineWidth;

		VkPipelineMultisampleStateCreateInfo ms_info = {};
		ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms_info.rasterizationSamples = multisample_state.rasterizationSamples;
		ms_info.sampleShadingEnable = multisample_state.sampleShadingEnable;
		ms_info.minSampleShading = multisample_state.minSampleShading;

		if (multisample_state.sample_mask.size()) {
			ms_info.pSampleMask = multisample_state.sample_mask.data();
		}
		else {
			ms_info.pSampleMask = NULL;
		}
		
		ms_info.alphaToCoverageEnable = multisample_state.alphaToCoverageEnable;
		ms_info.alphaToOneEnable = multisample_state.alphaToOneEnable;

		VkPipelineDepthStencilStateCreateInfo ds_info = {};
		ds_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds_info.depthTestEnable = depth_stencil_state.depthTestEnable;
		ds_info.depthWriteEnable = depth_stencil_state.depthWriteEnable;
		ds_info.depthCompareOp = depth_stencil_state.depthCompareOp;
		ds_info.depthBoundsTestEnable = depth_stencil_state.depthBoundsTestEnable;
		ds_info.stencilTestEnable = depth_stencil_state.stencilTestEnable;
		ds_info.front = depth_stencil_state.front;
		ds_info.back = depth_stencil_state.back;
		ds_info.minDepthBounds = depth_stencil_state.minDepthBounds;
		ds_info.maxDepthBounds = depth_stencil_state.maxDepthBounds;

		VkPipelineColorBlendStateCreateInfo bs_info = {};
		bs_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		bs_info.logicOpEnable = color_blend_state.logicOpEnable;
		bs_info.logicOp = color_blend_state.logicOp;
		bs_info.attachmentCount = blend_atachs.size();
		bs_info.pAttachments = blend_atachs.data();
		bs_info.blendConstants[0] = color_blend_state.blendConstants[0];
		bs_info.blendConstants[1] = color_blend_state.blendConstants[1];
		bs_info.blendConstants[2] = color_blend_state.blendConstants[2];
		bs_info.blendConstants[3] = color_blend_state.blendConstants[3];

		VkPipelineDynamicStateCreateInfo dy_info = {};
		dy_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dy_info.dynamicStateCount = dynamic_state.size();
		dy_info.pDynamicStates = dynamic_state.data();

		VkGraphicsPipelineCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		info.stageCount = shaders.size();
		info.pStages = shaders.data();
		info.pVertexInputState = &vi_info;
		info.pInputAssemblyState = &ia_info;
		info.pTessellationState = NULL;
		info.pViewportState = &vp_info;
		info.pRasterizationState = &rs_info;
		info.pMultisampleState = &ms_info;
		info.pDepthStencilState = &ds_info;
		info.pColorBlendState = &bs_info;
		info.pDynamicState = &dy_info;
		info.layout = pipe_layout;
		info.renderPass = renderpass;
		info.subpass = 0;
		info.basePipelineHandle = NULL;
		info.basePipelineIndex = NULL;

		checkVkRes(vkCreateGraphicsPipelines(dev->logical_dev, NULL, 1, &info, NULL, &pipeline),
			"failed to create graphics pipeline");
	}

	return err_stack;
}
