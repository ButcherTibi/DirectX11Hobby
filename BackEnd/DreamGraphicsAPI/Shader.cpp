// Header
#include "DreamGraphicsAPI.hpp"

using namespace dga;


Shader::~Shader()
{
	if (device != nullptr) {
		vkDestroyShaderModule(device, shader, nullptr);
	}
}