// Header
#include "DreamGraphicsAPI.hpp"

using namespace dga;


void Device::_createShader(std::vector<char8_t>& code, Shader& r_shader)
{
	VkShaderModuleCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = code.size();
	info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	r_shader.device = device;

	if (vkCreateShaderModule(device, &info, nullptr, &r_shader.shader) != VK_SUCCESS) {
		throw;
	}
}

void Device::createVertexShader(std::vector<char8_t>& compiled_shader_code, VertexShader& r_vertex_shader)
{
	_createShader(compiled_shader_code, r_vertex_shader);
}

void Device::createPixelShader(std::vector<char8_t>& compiled_shader_code, PixelShader& r_pixel_shader)
{
	_createShader(compiled_shader_code, r_pixel_shader);
}

void Device::createRasterizerPipeline(RasterizerPipeline& r_pipeline)
{
	r_pipeline.device = device;
}

Device::~Device()
{
	vkDestroyDevice(device, nullptr);
}
