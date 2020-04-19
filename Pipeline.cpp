
// Header
#include "VulkanSystems.h"


ErrStack vks::PipelineLayout::create(LogicalDevice* logical_dev, Descriptor* descp)
{
	this->logical_dev = logical_dev;
	
	// Pipeline Layout
	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &descp->descp_layout;
	pipeline_layout_info.pushConstantRangeCount = 0;

	VkResult vk_res = vkCreatePipelineLayout(logical_dev->logical_device, &pipeline_layout_info, NULL, &pipe_layout);
	if (vk_res != VK_SUCCESS) {
		return ErrStack(vk_res, code_location, "failed to create pipeline layout");
	}
	return ErrStack();
}

ErrStack vks::PipelineLayout::create(LogicalDevice* logical_dev)
{
	this->logical_dev = logical_dev;

	// Pipeline Layout
	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pSetLayouts = 0;
	pipeline_layout_info.pushConstantRangeCount = 0;

	VkResult vk_res = vkCreatePipelineLayout(logical_dev->logical_device, &pipeline_layout_info, NULL, &pipe_layout);
	if (vk_res != VK_SUCCESS) {
		return ErrStack(vk_res, code_location, "failed to create pipeline layout");
	}
	return ErrStack();
}

void vks::PipelineLayout::destroy()
{
	vkDestroyPipelineLayout(logical_dev->logical_device, pipe_layout, NULL);
	pipe_layout = VK_NULL_HANDLE;
}

vks::PipelineLayout::~PipelineLayout()
{
	if (pipe_layout != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack vks::ShaderModule::recreate(LogicalDevice* logical_dev, std::vector<char>& code, VkShaderStageFlagBits stage)
{
	if (this->sh_module != VK_NULL_HANDLE) {
		destroy();
	}

	this->logical_dev = logical_dev;
	this->stage = stage;

	VkShaderModuleCreateInfo shader_module_info = {};
	shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_info.codeSize = code.size();
	shader_module_info.pCode = reinterpret_cast<uint32_t*>(code.data());

	VkResult res = vkCreateShaderModule(logical_dev->logical_device, &shader_module_info, NULL, &this->sh_module);
	if (res != VK_SUCCESS) {
		return ErrStack(res, code_location, "failed to create shader module");
	}
	return ErrStack();
}

ErrStack vks::ShaderModule::setDebugName(std::string name)
{
	checkErrStack1(logical_dev->setDebugName(
		reinterpret_cast<uint64_t>(sh_module), VK_OBJECT_TYPE_DESCRIPTOR_SET, name));

	return ErrStack();
}

void vks::ShaderModule::destroy()
{
	vkDestroyShaderModule(logical_dev->logical_device, this->sh_module, NULL);
	sh_module = VK_NULL_HANDLE;
}

vks::ShaderModule::~ShaderModule()
{
	if (sh_module != VK_NULL_HANDLE) {
		destroy();
	}
}

vks::GraphicsPipeline::GraphicsPipeline()
{
	
}

void vks::GraphicsPipeline::setToDefault()
{
	// Vertex input state
	{
		vert_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_stage_info.vertexBindingDescriptionCount = 0;
		vert_input_stage_info.vertexAttributeDescriptionCount = 0;
	}

	// Input Assembly state
	{
		input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state_info.primitiveRestartEnable = VK_FALSE;
	}

	// Viewport state 
	{
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		scissor.offset = { 0, 0 };

		viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_info.viewportCount = 1;
		viewport_state_info.pViewports = &viewport;
		viewport_state_info.scissorCount = 1;
		viewport_state_info.pScissors = &scissor;
	}

	// Rasterization state
	{
		raster_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_state_info.depthClampEnable = VK_FALSE;
		raster_state_info.rasterizerDiscardEnable = VK_FALSE;
		raster_state_info.polygonMode = VK_POLYGON_MODE_FILL;
		raster_state_info.cullMode = VK_CULL_MODE_NONE;
		raster_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster_state_info.depthBiasEnable = VK_FALSE;
		raster_state_info.lineWidth = 1.0f;  // unused
	}

	// Multisample state
	{
		multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_info.sampleShadingEnable = VK_FALSE;
	}

	// Depth Stencil
	{
		depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_info.depthTestEnable = VK_FALSE;
		depth_stencil_state_info.depthWriteEnable = VK_FALSE;
		depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_info.stencilTestEnable = VK_FALSE;
	}

	// Color Blending state
	{
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		color_blend_attachments = { color_blend_attachment };

		color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_info.logicOpEnable = VK_FALSE;
		color_blend_state_info.logicOp = VK_LOGIC_OP_NO_OP;
		color_blend_state_info.attachmentCount = (uint32_t)color_blend_attachments.size();
		color_blend_state_info.pAttachments = color_blend_attachments.data();
		color_blend_state_info.blendConstants[0] = 0.0f;
		color_blend_state_info.blendConstants[1] = 0.0f;
		color_blend_state_info.blendConstants[2] = 0.0f;
		color_blend_state_info.blendConstants[3] = 0.0f;
	}
}

void vks::GraphicsPipeline::configureFor3D()
{
	// Vertex input state
	{
		vertex_input_binding_descp = GPU_3D_Vertex::getBindingDescription();

		for (auto& atribute_descp : GPU_3D_Vertex::getAttributeDescriptions()) {
			vertex_input_atribute_descp.push_back(atribute_descp);
		}

		vert_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_stage_info.vertexBindingDescriptionCount = 1;
		vert_input_stage_info.pVertexBindingDescriptions = &vertex_input_binding_descp;
		vert_input_stage_info.vertexAttributeDescriptionCount = (uint32_t)vertex_input_atribute_descp.size();
		vert_input_stage_info.pVertexAttributeDescriptions = vertex_input_atribute_descp.data();
	}

	// Depth Stencil
	{
		depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_info.depthTestEnable = VK_TRUE;
		depth_stencil_state_info.depthWriteEnable = VK_TRUE;
		depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_info.stencilTestEnable = VK_FALSE;
	}
}

void vks::GraphicsPipeline::configureForUserInterface()
{
	// Vertex input state
	{
		vertex_input_binding_descp = GPU_UI_Vertex::getBindingDescription();

		for (auto& atribute_descp : GPU_UI_Vertex::getAttributeDescriptions()) {
			vertex_input_atribute_descp.push_back(atribute_descp);
		}

		vert_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_stage_info.vertexBindingDescriptionCount = 1;
		vert_input_stage_info.pVertexBindingDescriptions = &vertex_input_binding_descp;
		vert_input_stage_info.vertexAttributeDescriptionCount = (uint32_t)vertex_input_atribute_descp.size();
		vert_input_stage_info.pVertexAttributeDescriptions = vertex_input_atribute_descp.data();
	}
}

ErrStack vks::GraphicsPipeline::recreate(LogicalDevice* logical_dev, ShaderModule* vertex_module, ShaderModule* frag_module,
	uint32_t width, uint32_t height, PipelineLayout* pipe_layout, Renderpass* renderpass, uint32_t subpass_idx)
{
	if (this->pipeline != VK_NULL_HANDLE) {
		destroy();
	}

	this->logical_dev = logical_dev;

	// Shader Stage
	{
		VkPipelineShaderStageCreateInfo vert_shader_info = {};
		vert_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_info.stage = vertex_module->stage;
		vert_shader_info.module = vertex_module->sh_module;
		vert_shader_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_info = {};
		frag_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_info.stage = frag_module->stage;
		frag_shader_info.module = frag_module->sh_module;
		frag_shader_info.pName = "main";

		shader_stages = { vert_shader_info, frag_shader_info };
	}

	// Viewport
	{
		viewport.width = (float)width;
		viewport.height = (float)height;

		scissor.extent.width = width;
		scissor.extent.height = height;
	}

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = (uint32_t)shader_stages.size();
	pipeline_info.pStages = shader_stages.data();
	pipeline_info.pVertexInputState = &vert_input_stage_info;
	pipeline_info.pInputAssemblyState = &input_assembly_state_info;
	pipeline_info.pTessellationState = NULL;
	pipeline_info.pViewportState = &viewport_state_info;
	pipeline_info.pRasterizationState = &raster_state_info;
	pipeline_info.pMultisampleState = &multisample_state_info;
	pipeline_info.pDepthStencilState = &depth_stencil_state_info;
	pipeline_info.pColorBlendState = &color_blend_state_info;
	pipeline_info.pDynamicState = NULL;
	pipeline_info.layout = pipe_layout->pipe_layout;
	pipeline_info.renderPass = renderpass->renderpass;
	pipeline_info.subpass = subpass_idx;
	pipeline_info.basePipelineHandle = NULL;

	VkResult vk_res = vkCreateGraphicsPipelines(logical_dev->logical_device, NULL, 1, &pipeline_info, NULL, &pipeline);
	if (vk_res != VK_SUCCESS) {
		return ErrStack(vk_res, code_location, "failed to create pipeline");
	}

	return ErrStack();
}

void vks::GraphicsPipeline::destroy()
{
	vkDestroyPipeline(logical_dev->logical_device, pipeline, NULL);
	pipeline = VK_NULL_HANDLE;
}

vks::GraphicsPipeline::~GraphicsPipeline()
{
	if (pipeline != VK_NULL_HANDLE) {
		destroy();
	}
}
