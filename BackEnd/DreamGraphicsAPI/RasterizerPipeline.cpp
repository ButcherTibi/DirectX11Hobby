// Header
#include "DreamGraphicsAPI.hpp"

using namespace dga;


void VulkanPipelineInfo::_create()
{
	// Input Assembly
	vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.pVertexBindingDescriptions = nullptr;
	vertex_input_info.vertexAttributeDescriptionCount = 0;
	vertex_input_info.pVertexAttributeDescriptions = nullptr;

	input_assembly_info = {};
	input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = false;

	// Vertex Shader
	vertex_stage = {};
	vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_stage.module = nullptr;
	vertex_stage.pName = "main";

	// Rasterizer
	viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = FLT_MAX;
	viewport.height = FLT_MAX;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = 0xFFFF'FFFF;
	scissor.extent.height = 0xFFFF'FFFF;

	viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = false;
	rasterizer.rasterizerDiscardEnable = false;
	rasterizer.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = false;
	rasterizer.depthBiasConstantFactor = 0;
	rasterizer.depthBiasClamp = 0;
	rasterizer.depthBiasSlopeFactor = 0;
	rasterizer.lineWidth = 1.0;

	multisample = {};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample.sampleShadingEnable = false;
	multisample.minSampleShading = 1.0f;
	multisample.pSampleMask = nullptr;
	multisample.alphaToCoverageEnable = false;
	multisample.alphaToOneEnable = false;

	// Pixel Shader
	pixel_stage = {};
	pixel_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pixel_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pixel_stage.module = nullptr;
	pixel_stage.pName = "main";
}

void RasterizerPipeline::_create()
{
	pipe_info._create();
}

void RasterizerPipeline::setPrimitiveTopology(VkPrimitiveTopology topology)
{
	pipe_info.input_assembly_info.topology = topology;
}

void RasterizerPipeline::setVertexShader(VertexShader* vertex_shader)
{
	if (pipe_info.vertex_stage.sType != 0) {
		throw;
	}

	pipe_info.vertex_stage = {};
	pipe_info.vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipe_info.vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipe_info.vertex_stage.module = vertex_shader->shader;
	pipe_info.vertex_stage.pName = "main";

	pipe_info.stages.push_back(pipe_info.vertex_stage);
}

void RasterizerPipeline::setViewportPosition(float x, float y)
{
	pipe_info.viewport.x = x;
	pipe_info.viewport.y = y;
}

void RasterizerPipeline::setViewportSize(float width, float height)
{
	pipe_info.viewport.width = width;
	pipe_info.viewport.height = height;
}

void RasterizerPipeline::setViewportDepth(float min_depth, float max_depth)
{
	pipe_info.viewport.minDepth = min_depth;
	pipe_info.viewport.maxDepth = max_depth;
}

void RasterizerPipeline::setScissorOffset(int32_t x, int32_t y)
{
	pipe_info.scissor.offset.x = x;
	pipe_info.scissor.offset.y = y;
}

void RasterizerPipeline::setScissorExtend(uint32_t width, uint32_t height)
{
	pipe_info.scissor.extent.width = width;
	pipe_info.scissor.extent.height = height;
}

void RasterizerPipeline::setPolygonMode(VkPolygonMode mode)
{
	pipe_info.rasterizer.polygonMode = mode;
}

void RasterizerPipeline::setCullMode(VkCullModeFlags flags)
{
	pipe_info.rasterizer.cullMode = flags;
}

void RasterizerPipeline::setFrontFace(VkFrontFace front_face)
{
	pipe_info.rasterizer.frontFace = front_face;
}

void RasterizerPipeline::setMultisampleCount(VkSampleCountFlagBits flags)
{
	pipe_info.multisample.rasterizationSamples = flags;
}

void RasterizerPipeline::setPixelShader(PixelShader* pixel_shader)
{
	if (pipe_info.pixel_stage.sType != 0) {
		throw;
	}

	pipe_info.pixel_stage = {};
	pipe_info.pixel_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipe_info.pixel_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipe_info.pixel_stage.module = pixel_shader->shader;
	pipe_info.pixel_stage.pName = "main";

	pipe_info.stages.push_back(pipe_info.pixel_stage);
}