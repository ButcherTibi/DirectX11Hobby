
// Header
#include "GPU_ShaderTypes.h"


VkVertexInputBindingDescription GPU_Rects_Vertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(GPU_Rects_Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> GPU_Rects_Vertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 2> attrs_descp = {};

	attrs_descp[0].binding = 0;
	attrs_descp[0].location = 0;
	attrs_descp[0].format = VK_FORMAT_R32G32_SFLOAT;
	attrs_descp[0].offset = offsetof(GPU_Rects_Vertex, pos);

	attrs_descp[1].binding = 0;
	attrs_descp[1].location = 1;
	attrs_descp[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs_descp[1].offset = offsetof(GPU_Rects_Vertex, color);

	return attrs_descp;
}

VkVertexInputBindingDescription GPU_Circles_Vertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(GPU_Circles_Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> GPU_Circles_Vertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 4> attrs_descp = {};

	attrs_descp[0].binding = 0;
	attrs_descp[0].location = 0;
	attrs_descp[0].format = VK_FORMAT_R32G32_SFLOAT;
	attrs_descp[0].offset = offsetof(GPU_Circles_Vertex, pos);

	attrs_descp[1].binding = 0;
	attrs_descp[1].location = 1;
	attrs_descp[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs_descp[1].offset = offsetof(GPU_Circles_Vertex, color);

	attrs_descp[2].binding = 0;
	attrs_descp[2].location = 2;
	attrs_descp[2].format = VK_FORMAT_R32G32_SFLOAT;
	attrs_descp[2].offset = offsetof(GPU_Circles_Vertex, center);

	attrs_descp[3].binding = 0;
	attrs_descp[3].location = 3;
	attrs_descp[3].format = VK_FORMAT_R32_SFLOAT;
	attrs_descp[3].offset = offsetof(GPU_Circles_Vertex, radius);

	return attrs_descp;
}
