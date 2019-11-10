

// Other
#include "ErrorStuff.h"

#include "VulkanContext.h"
#include "VulkanFrames.h"
#include "VulkanBuffers.h"

#include "VulkanPipeline.h"


void ShaderModule::init(Device* device, std::vector<char>& code,
	VkShaderStageFlagBits stage)
{
	std::cout << "ShaderModule init" << std::endl;

	this->dev = device;
	this->code = code;
	this->stage = stage;
}

ErrorStack ShaderModule::build()
{
	std::cout << "ShaderModule build" << std::endl;

	VkShaderModuleCreateInfo shader_module_info = {};
	shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_info.codeSize = code.size();
	shader_module_info.pCode = reinterpret_cast<uint32_t*>(code.data());

	VkResult res = vkCreateShaderModule(dev->logical_device, &shader_module_info, NULL, &this->sh_module);
	if (res != VK_SUCCESS) {
		return ErrorStack(res, code_location, "failed to create shader module");
	}
	return ErrorStack();
}

void ShaderModule::destroy()
{
	std::cout << "ShaderModule destroy" << std::endl;

	vkDestroyShaderModule(dev->logical_device, this->sh_module, NULL);
	sh_module = VK_NULL_HANDLE;
}

ShaderModule::~ShaderModule()
{
	if (this->dev != nullptr) {
		destroy();
	}
}

void GraphicsPipeline::init(Device* device, Swapchain* swapchain, Renderpass* renderpass, DescriptorSets* descp_sets,
	ShaderModule* vert_module, ShaderModule* frag_module)
{
	std::cout << "GraphicsPipeline init" << std::endl;

	this->dev = device;
	this->descp_sets = descp_sets;
	this->swapchain = swapchain;
	this->renderpass = renderpass;
	this->vert_module = vert_module;
	this->frag_module = frag_module;

	// Graphics Pipeline Setup
	{
		// Vertex input state
		{
			vertex_input_binding_descp = GPUVertex::getBindingDescription();

			for (auto& atribute_descp : GPUVertex::getAttributeDescriptions()) {
				vertex_input_atribute_descp.push_back(atribute_descp);
			}

			vert_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vert_input_stage_info.vertexBindingDescriptionCount = 1;
			vert_input_stage_info.pVertexBindingDescriptions = &vertex_input_binding_descp;
			vert_input_stage_info.vertexAttributeDescriptionCount = (uint32_t)vertex_input_atribute_descp.size();
			vert_input_stage_info.pVertexAttributeDescriptions = vertex_input_atribute_descp.data();
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
			raster_state_info.cullMode = cull_mode;
			raster_state_info.frontFace = front_face;
			raster_state_info.depthBiasEnable = VK_FALSE;
			raster_state_info.lineWidth = 1.0f;  // unused
		}

		// Multisample state
		{
			multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  // for MSAA ?
			multisample_state_info.sampleShadingEnable = VK_FALSE;
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
			color_blend_state_info.attachmentCount = (uint32_t)color_blend_attachments.size();
			color_blend_state_info.pAttachments = color_blend_attachments.data();
			color_blend_state_info.blendConstants[0] = 0.0f;
			color_blend_state_info.blendConstants[1] = 0.0f;
			color_blend_state_info.blendConstants[2] = 0.0f;
			color_blend_state_info.blendConstants[3] = 0.0f;
		}
	}
}

ErrorStack GraphicsPipeline::buildLayoutAndCache()
{
	std::cout << "GraphicsPipeline buildLayoutAndCache" << std::endl;

	VkResult vk_res;

	// Pipeline Layout
	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &descp_sets->descp_layout;
	pipeline_layout_info.pushConstantRangeCount = 0;

	vk_res = vkCreatePipelineLayout(dev->logical_device, &pipeline_layout_info, NULL, &pipeline_layout);
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, code_location, "failed to create pipeline layout");
	}

	// Pipeline Cache
	VkPipelineCacheCreateInfo pipeline_cache_info = {};
	pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipeline_cache_info.initialDataSize = 0;
	pipeline_cache_info.pInitialData = NULL;

	vk_res = vkCreatePipelineCache(dev->logical_device, &pipeline_cache_info, NULL, &pipeline_cache);
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, code_location, "failed to created pipeline cache");
	}

	return ErrorStack();
}

ErrorStack GraphicsPipeline::buildPipeline()
{
	std::cout << "GraphicsPipeline buildPipeline" << std::endl;

	/* Build time only data */

	// Shader Stage
	{
		VkPipelineShaderStageCreateInfo vert_shader_info = {};
		vert_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_info.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_info.module = vert_module->sh_module;
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
		viewport.width = (float)swapchain->resolution.width;
		viewport.height = (float)swapchain->resolution.height;

		scissor.extent = swapchain->resolution;
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
	pipeline_info.pDepthStencilState = NULL;
	pipeline_info.pColorBlendState = &color_blend_state_info;
	pipeline_info.pDynamicState = NULL;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = renderpass->renderpass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = NULL;

	// use cached pipeline
	VkResult vk_res = vkCreateGraphicsPipelines(dev->logical_device, NULL, 1, &pipeline_info, NULL, &pipeline);
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, code_location, "failed to create pipeline");
	}

	return ErrorStack();
}

void GraphicsPipeline::destroyPipeline()
{
	std::cout << "GraphicsPipeline destroyPipeline" << std::endl;

	vkDestroyPipeline(dev->logical_device, pipeline, NULL);
}

void GraphicsPipeline::destroyLayoutAndCache()
{
	std::cout << "GraphicsPipeline destroyLayoutAndCache" << std::endl;

	vkDestroyPipelineCache(dev->logical_device, pipeline_cache, NULL);
	vkDestroyPipelineLayout(dev->logical_device, pipeline_layout, NULL);
}

GraphicsPipeline::~GraphicsPipeline()
{
	if (this->dev != nullptr) {

		this->destroyPipeline();
		this->destroyLayoutAndCache();
	}	
}