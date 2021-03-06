
#include "pch.h"

// Header
#include "VulkanWrapper.hpp"

using namespace nui;
using namespace vkw;


Framebuffer::~Framebuffer()
{
	for (auto& framebuff : framebuffs) {
		vkDestroyFramebuffer(dev->logical_dev, framebuff, NULL);
	}
}

void Drawpass::addDescriptorTypeToPool(VkDescriptorType type, uint32_t count)
{
	for (auto& size : descp_sizes) {
		if (size.type == type) {
			size.descriptorCount += count;
			return;
		}
	}

	auto& new_size = descp_sizes.emplace_back();
	new_size.type = type;
	new_size.descriptorCount = count;
}

ErrStack Drawpass::addReadColorAttachment(ReadAttachmentInfo& info)
{
	// Renderpass
	uint32_t atach_idx = (uint32_t)atach_descps.size();
	VkAttachmentDescription& atach_descp = atach_descps.emplace_back();
	atach_descp.flags = 0;

	if (info.example_view != nullptr) {
		Image* img = info.example_view->image;
		atach_descp.format = img->format;
		atach_descp.samples = img->samples;
	}
	else {
		atach_descp.format = info.format;
		atach_descp.samples = info.samples;
	}

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

	// Descriptor Pool
	addDescriptorTypeToPool(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, info.descriptor_count);

	// Set Debug Name
	layout->name = info.name;

	return ErrStack();
}

ErrStack Drawpass::addWriteColorAttachment(WriteAttachmentInfo& info)
{
	// Renderpass
	uint32_t atach_idx = (uint32_t)atach_descps.size();
	VkAttachmentDescription& atach_descp = atach_descps.emplace_back();
	atach_descp.flags = 0;

	if (info.example_view != nullptr) {
		Image* img = info.example_view->image;
		atach_descp.format = img->format;
		atach_descp.samples = img->samples;
	}
	else {
		atach_descp.format = info.format;
		atach_descp.samples = info.samples;
	}

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

	return ErrStack();
}

nui::ErrStack Drawpass::addPresentAttachment(PresentAttachmentInfo& info)
{
	// Renderpass
	uint32_t atach_idx = (uint32_t)atach_descps.size();
	VkAttachmentDescription& atach_descp = atach_descps.emplace_back();
	atach_descp.flags = 0;
	atach_descp.format = dev->surface.imageFormat;
	atach_descp.samples = VK_SAMPLE_COUNT_1_BIT;
	atach_descp.loadOp = info.load_op;
	atach_descp.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	atach_descp.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	atach_descp.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	atach_descp.initialLayout = info.initial_layout;
	atach_descp.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference& atach_ref = atach_refs.emplace_back();
	atach_ref.attachment = atach_idx;
	atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	color_atachs.push_back(atach_ref);

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

	return ErrStack();
}

ErrStack Drawpass::addReadWriteColorAttachment(ReadWriteAttachment& info)
{
	// Renderpass
	uint32_t atach_idx = (uint32_t)atach_descps.size();
	VkAttachmentDescription& atach_descp = atach_descps.emplace_back();
	atach_descp.flags = 0;

	if (info.example_view != nullptr) {
		Image* img = info.example_view->image;
		atach_descp.format = img->format;
		atach_descp.samples = img->samples;
	}
	else {
		atach_descp.format = info.format;
		atach_descp.samples = info.samples;
	}

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

	// Descriptor Pool
	addDescriptorTypeToPool(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, info.descriptor_count);

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

	return ErrStack();
}

ErrStack Drawpass::bindStorageBuffer(StorageBufferBinding& info)
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

	// Descriptor Pool
	addDescriptorTypeToPool(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, info.descriptor_count);

	return ErrStack();
}

ErrStack Drawpass::bindUniformBuffer(UniformBufferBinding& info)
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

	// Descriptor Pool
	addDescriptorTypeToPool(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, info.descriptor_count);

	return ErrStack();
}

nui::ErrStack Drawpass::bindCombinedImageSampler(CombinedImageSamplerBinding& info)
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
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.descriptorCount = info.descriptor_count;
	binding.stageFlags = info.stages;
	binding.pImmutableSamplers = NULL;

	// Descriptor Pool
	addDescriptorTypeToPool(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, info.descriptor_count);

	return ErrStack();
}

void Drawpass::setVertexShader(Shader& vs)
{
	auto& shader = shaders.emplace_back();
	shader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader.module = vs.shader;
	shader.pName = "main";
}

void Drawpass::setFragmentShader(Shader& fs)
{
	auto& shader = shaders.emplace_back();
	shader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader.module = fs.shader;
	shader.pName = "main";
}

ErrStack Drawpass::build()
{
	ErrStack err_stack;
	VkResult vk_res;

	// Renderpass
	{
		VkSubpassDescription subpass_descp = {};
		subpass_descp.flags = 0;
		subpass_descp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_descp.inputAttachmentCount = (uint32_t)input_atachs.size();
		subpass_descp.pInputAttachments = input_atachs.data();
		subpass_descp.colorAttachmentCount = (uint32_t)color_atachs.size();
		subpass_descp.pColorAttachments = color_atachs.data();
		// subpass_descp.pResolveAttachments;
		// subpass_descp.pDepthStencilAttachment;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = (uint32_t)atach_descps.size();
		info.pAttachments = atach_descps.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpass_descp;
		info.dependencyCount = 0;
		info.pDependencies = NULL;

		checkVkRes(vkCreateRenderPass(dev->logical_dev, &info, NULL, &renderpass),
			"failed to create renderpass");
	}

	if (layouts.size()) {

		// Descriptor Layout
		{
			for (auto& layout : layouts) {

				VkDescriptorSetLayoutCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				info.bindingCount = (uint32_t)layout.bindings.size();
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
			info.poolSizeCount = (uint32_t)descp_sizes.size();
			info.pPoolSizes = descp_sizes.data();

			checkVkRes(vkCreateDescriptorPool(dev->logical_dev, &info, NULL, &descp_pool),
				"failed to create descriptor pool");
		}

		// Descriptor Sets
		{
			descp_layouts.resize(layouts.size());
			for (uint32_t i = 0; i < descp_layouts.size(); i++) {
				descp_layouts[i] = layouts[i].layout;
			}

			descp_sets.resize(layouts.size());

			VkDescriptorSetAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.descriptorPool = descp_pool;
			alloc_info.descriptorSetCount = (uint32_t)descp_layouts.size();
			alloc_info.pSetLayouts = descp_layouts.data();

			checkVkRes(vkAllocateDescriptorSets(dev->logical_dev, &alloc_info, descp_sets.data()),
				"failed to allocate descriptor sets");

			// Debug Name
			for (uint32_t i = 0; i < descp_layouts.size(); i++) {
				if (layouts[i].name.length()) {
					dev->setDebugName(reinterpret_cast<uint64_t>(descp_sets[i]), VK_OBJECT_TYPE_DESCRIPTOR_SET,
						layouts[i].name);
				}
			}
		}

		// Pipeline Layout
		{
			std::vector<VkDescriptorSetLayout> vk_layouts(layouts.size());
			for (uint32_t i = 0; i < layouts.size(); i++) {
				vk_layouts[i] = layouts[i].layout;
			}

			VkPipelineLayoutCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.setLayoutCount = (uint32_t)vk_layouts.size();
			info.pSetLayouts = vk_layouts.data();

			checkVkRes(vkCreatePipelineLayout(dev->logical_dev, &info, NULL, &pipe_layout),
				"failed to create pipeline layout");
		}
	}
	else {
		// Pipeline Layout
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		checkVkRes(vkCreatePipelineLayout(dev->logical_dev, &info, NULL, &pipe_layout),
			"failed to create pipeline layout");
	}
	
	// Vertex Input State
	std::vector<VkVertexInputBindingDescription> vi_bindings;
	std::vector<VkVertexInputAttributeDescription> vi_attributes;
	VkPipelineVertexInputStateCreateInfo vi_info = {};
	{
		uint32_t vi_attribute_idx = 0;

		vi_bindings.resize(vertex_inputs.size());
		for (uint32_t i = 0; i < vi_bindings.size(); i++) {
			vi_bindings[i] = vertex_inputs[i].binding;

			for (VkVertexInputAttributeDescription& atribute : vertex_inputs[i].atributes) {
				if (atribute.location == 0xFFFF'FFFF) {
					atribute.location = vi_attribute_idx;
				}
				vi_attribute_idx++;
			}
		}

		vi_attributes.resize(vi_attribute_idx);
		vi_attribute_idx = 0;

		for (uint32_t i = 0; i < vi_bindings.size(); i++) {

			auto& attributes = vertex_inputs[i].atributes;

			std::memcpy(vi_attributes.data() + vi_attribute_idx, attributes.data(),
				attributes.size() * sizeof(VkVertexInputAttributeDescription));

			vi_attribute_idx += attributes.size();
		}
	
		vi_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi_info.vertexBindingDescriptionCount = (uint32_t)vi_bindings.size();
		vi_info.pVertexBindingDescriptions = vi_bindings.data();
		vi_info.vertexAttributeDescriptionCount = (uint32_t)vi_attributes.size();
		vi_info.pVertexAttributeDescriptions = vi_attributes.data();
	}

	// Input Assembly State
	VkPipelineInputAssemblyStateCreateInfo ia_info = {};
	{
		ia_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia_info.topology = input_assembly_state.topology;
		ia_info.primitiveRestartEnable = input_assembly_state.primitiveRestartEnable;
	}

	// Viewport State
	std::vector<VkViewport> vk_viewports;
	std::vector<VkRect2D> vk_scissors;
	VkPipelineViewportStateCreateInfo vp_info = {};
	{
		if (viewport_state.viewports.size()) {

			vk_viewports.resize(viewport_state.viewports.size());
			for (uint32_t i = 0; i < vk_viewports.size(); i++) {

				Viewport& viewport = viewport_state.viewports[i];

				vk_viewports[i].x = viewport.x;
				vk_viewports[i].y = viewport.y;

				if (!viewport.width) {
					vk_viewports[i].width = (float)dev->surface.width;
					vk_viewports[i].height = (float)dev->surface.height;
				}
				else {
					vk_viewports[i].width = viewport.width;
					vk_viewports[i].height = viewport.height;
				}

				vk_viewports[i].minDepth = viewport.minDepth;
				vk_viewports[i].maxDepth = viewport.maxDepth;
			}
		}
		else {
			auto& viewport = vk_viewports.emplace_back();
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = (float)dev->surface.width;
			viewport.height = (float)dev->surface.height;
			viewport.minDepth = 0;
			viewport.maxDepth = 1;
		}

		if (viewport_state.scissors.size()) {

			vk_scissors.resize(viewport_state.scissors.size());
			for (uint32_t i = 0; i < vk_scissors.size(); i++) {

				Scissor& scissor = viewport_state.scissors[i];

				vk_scissors[i].offset = scissor.offset;

				if (!scissor.extent.width) {
					vk_scissors[i].extent.width = dev->surface.width;
					vk_scissors[i].extent.height = dev->surface.height;
				}
				else {
					vk_scissors[i].extent = scissor.extent;
				}
			}
		}
		else {
			auto& scissor = vk_scissors.emplace_back();
			scissor.offset = { 0, 0 };
			scissor.extent.width = dev->surface.width;
			scissor.extent.height = dev->surface.height;
		}
		
		vp_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vp_info.viewportCount = (uint32_t)vk_viewports.size();
		vp_info.pViewports = vk_viewports.data();
		vp_info.scissorCount = (uint32_t)vk_scissors.size();
		vp_info.pScissors = vk_scissors.data();
	}

	// Rasterization State
	VkPipelineRasterizationStateCreateInfo rs_info = {};
	{	
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
	}

	// Multisample State
	VkPipelineMultisampleStateCreateInfo ms_info = {};
	{	
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
	}

	// Depth Stencil State
	VkPipelineDepthStencilStateCreateInfo ds_info = {};
	{
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
	}

	// Color Blending State
	VkPipelineColorBlendStateCreateInfo bs_info = {};
	{
		bs_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		bs_info.logicOpEnable = color_blend_state.logicOpEnable;
		bs_info.logicOp = color_blend_state.logicOp;
		bs_info.attachmentCount = (uint32_t)blend_atachs.size();
		bs_info.pAttachments = blend_atachs.data();
		bs_info.blendConstants[0] = color_blend_state.blendConstants[0];
		bs_info.blendConstants[1] = color_blend_state.blendConstants[1];
		bs_info.blendConstants[2] = color_blend_state.blendConstants[2];
		bs_info.blendConstants[3] = color_blend_state.blendConstants[3];
	}

	// Dynamic State
	VkPipelineDynamicStateCreateInfo dy_info = {};
	{
		dy_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dy_info.dynamicStateCount = (uint32_t)dynamic_state.size();
		dy_info.pDynamicStates = dynamic_state.data();
	}

	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.stageCount = (uint32_t)shaders.size();
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

	return err_stack;
}

nui::ErrStack Drawpass::createFramebuffer(FramebufferCreateInfo& info, Framebuffer& frame)
{
	VkResult vk_res{};

	frame.dev = this->dev;

	if (info.width) {
		frame.width = info.width;
		frame.height = info.height;
	}
	else {
		frame.width = dev->surface.width;
		frame.height = dev->surface.height;
	}

	if (info.framebuffer_count) {
		frame.framebuffs.resize(info.framebuffer_count);
	}
	else {
		frame.framebuffs.resize(dev->surface.minImageCount);
	}
	
	std::vector<VkImageView> views(info.attachments.size());

	for (uint32_t i = 0; i < frame.framebuffs.size(); i++) {

		for (uint32_t j = 0; j < info.attachments.size(); j++) {
			ImageView* img_view = info.attachments[j];

			if (img_view == nullptr) {
				views[j] = dev->surface.swapchain_views[i];
			}
			else {
				views[j] = img_view->view;
			}
		}

		VkFramebufferCreateInfo vk_info = {};
		vk_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		vk_info.renderPass = renderpass;
		vk_info.attachmentCount = (uint32_t)views.size();
		vk_info.pAttachments = views.data();
		vk_info.width = frame.width;
		vk_info.height = frame.height;
		vk_info.layers = info.layers;

		checkVkRes(vkCreateFramebuffer(dev->logical_dev, &vk_info, NULL, &frame.framebuffs[i]),
			"failed to create framebuffer");
	}

	return ErrStack();
}

void Drawpass::updateStorageBufferDescriptor(uint32_t set_idx, uint32_t binding, UpdateBufferDescriptor& info)
{
	VkDescriptorBufferInfo resource = {};
	resource.buffer = info.buffer->buff;
	resource.offset = info.offset;
	resource.range = info.range;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descp_sets[set_idx];
	write.dstBinding = binding;
	write.dstArrayElement = info.dst_array_element;
	write.descriptorCount = info.descriptor_count;
	write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	write.pBufferInfo = &resource;

	vkUpdateDescriptorSets(dev->logical_dev,
		1, &write,
		0, NULL);

	if (layouts[set_idx].name.length()) {
		dev->setDebugName(reinterpret_cast<uint64_t>(descp_sets[set_idx]), VK_OBJECT_TYPE_DESCRIPTOR_SET,
			layouts[set_idx].name);
	}
}

void Drawpass::updateUniformBufferDescriptor(uint32_t set_idx, uint32_t binding, UpdateBufferDescriptor& info)
{
	VkDescriptorBufferInfo resource = {};
	resource.buffer = info.buffer->buff;
	resource.offset = info.offset;
	resource.range = info.range;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descp_sets[set_idx];
	write.dstBinding = binding;
	write.dstArrayElement = info.dst_array_element;
	write.descriptorCount = info.descriptor_count;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write.pBufferInfo = &resource;

	vkUpdateDescriptorSets(dev->logical_dev,
		1, &write,
		0, NULL);
}

void Drawpass::updateInputAttachmentDescriptor(uint32_t set_idx, uint32_t binding, InputAttachmentDescriptor& info)
{
	VkDescriptorImageInfo resource = {};
	resource.sampler = NULL;
	resource.imageView = info.view->view;
	resource.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descp_sets[set_idx];
	write.dstBinding = binding;
	write.dstArrayElement = info.dst_array_element;
	write.descriptorCount = info.descriptor_count;
	write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	write.pImageInfo = &resource;

	vkUpdateDescriptorSets(dev->logical_dev,
		1, &write,
		0, NULL);
}

void Drawpass::updateCombinedImageSamplerDescriptor(uint32_t set_idx, uint32_t binding,
	CombinedImageSamplerDescriptor& info)
{
	VkDescriptorImageInfo resource = {};
	resource.sampler = info.sampler->sampler;
	resource.imageView = info.view->view;
	resource.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descp_sets[set_idx];
	write.dstBinding = binding;
	write.dstArrayElement = info.dst_array_element;
	write.descriptorCount = info.descriptor_count;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.pImageInfo = &resource;

	vkUpdateDescriptorSets(dev->logical_dev,
		1, &write,
		0, NULL);
}

Drawpass::~Drawpass()
{
	if (dev != nullptr) {
		vkDestroyRenderPass(dev->logical_dev, renderpass, NULL);
		
		vkDestroyPipelineLayout(dev->logical_dev, pipe_layout, NULL);

		vkDestroyPipeline(dev->logical_dev, pipeline, NULL);

		vkDestroyDescriptorPool(dev->logical_dev, descp_pool, NULL);

		for (auto& layout : layouts) {
			vkDestroyDescriptorSetLayout(dev->logical_dev, layout.layout, NULL);
		}
	}
}
