#pragma once

// Standard
#include <vector>


class DescriptorSets;

class ShaderModule 
{
public:
	Device* dev = nullptr;
	std::vector<char> code;

	VkShaderModule sh_module = VK_NULL_HANDLE;
	VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

	void init(Device* device, std::vector<char> &code, VkShaderStageFlagBits stage);

	ErrorStack build();

	void destroy();

	~ShaderModule();
};


class GraphicsPipeline
{
public:
	// Parents
	Device* dev = nullptr;
	DescriptorSets* descp_sets = nullptr;
	Swapchain* swapchain = nullptr;
	Renderpass* renderpass = nullptr;
	ShaderModule* vert_module = nullptr;
	ShaderModule* frag_module = nullptr;

	// Pipeline Layout
	VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

	VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;

	// Pipeline setup
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	
	VkPipelineVertexInputStateCreateInfo vert_input_stage_info = {};
	VkVertexInputBindingDescription vertex_input_binding_descp;
	std::vector<VkVertexInputAttributeDescription> vertex_input_atribute_descp;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};

	// Viewport state 
	VkViewport viewport = {};
	VkRect2D scissor = {};
	VkPipelineViewportStateCreateInfo viewport_state_info = {};

	// Rasterization state
	VkCullModeFlagBits cull_mode = VK_CULL_MODE_BACK_BIT;
	VkFrontFace front_face = VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
	VkPipelineRasterizationStateCreateInfo raster_state_info = {};

	VkPipelineMultisampleStateCreateInfo multisample_state_info = {};

	// Color Blending state
	std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments;
	VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};

public:
	void init(Device *device, Swapchain* swapchain, Renderpass* renderpass, DescriptorSets* descp_sets,
		ShaderModule* vert_module, ShaderModule* frag_module);

	ErrorStack buildLayoutAndCache();
	ErrorStack buildPipeline();

	void destroyPipeline();
	void destroyLayoutAndCache();

	~GraphicsPipeline();
};
