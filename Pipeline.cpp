
// Header
#include "VulkanSystems.h"


ErrStack vks::PipelineLayout::create(LogicalDevice* logical_dev, VkPipelineLayoutCreateInfo* info)
{
	this->logical_dev = logical_dev;

	VkResult vk_res = vkCreatePipelineLayout(logical_dev->logical_device, info, NULL, &pipe_layout);
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
	return logical_dev->setDebugName(
		reinterpret_cast<uint64_t>(sh_module), VK_OBJECT_TYPE_DESCRIPTOR_SET, name);
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

ErrStack vks::GraphicsPipeline::create(LogicalDevice* logical_dev, VkGraphicsPipelineCreateInfo* info)
{
	this->logical_dev = logical_dev;

	VkResult vk_res = vkCreateGraphicsPipelines(logical_dev->logical_device, NULL, 1, info, NULL, &pipeline);
	if (vk_res != VK_SUCCESS) {
		return ErrStack(vk_res, code_location, "failed to create pipeline");
	}

	return ErrStack();
}

ErrStack vks::GraphicsPipeline::setDebugName(std::string name)
{
	return logical_dev->setDebugName(
		reinterpret_cast<uint64_t>(pipeline), VK_OBJECT_TYPE_PIPELINE, name);
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
