
// Header
#include "GPU_ShaderTypes.h"


VkVertexInputBindingDescription GPU_3D_Vertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(GPU_3D_Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 7> GPU_3D_Vertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 7> attrs_descp = {};

	attrs_descp[0].binding = 0;
	attrs_descp[0].location = 0;
	attrs_descp[0].format = VK_FORMAT_R32_UINT;
	attrs_descp[0].offset = offsetof(GPU_3D_Vertex, mesh_id);

	attrs_descp[1].binding = 0;
	attrs_descp[1].location = 1;
	attrs_descp[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs_descp[1].offset = offsetof(GPU_3D_Vertex, pos);

	attrs_descp[2].binding = 0;
	attrs_descp[2].location = 2;
	attrs_descp[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs_descp[2].offset = offsetof(GPU_3D_Vertex, vertex_normal);

	attrs_descp[3].binding = 0;
	attrs_descp[3].location = 3;
	attrs_descp[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs_descp[3].offset = offsetof(GPU_3D_Vertex, tess_normal);

	attrs_descp[4].binding = 0;
	attrs_descp[4].location = 4;
	attrs_descp[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs_descp[4].offset = offsetof(GPU_3D_Vertex, poly_normal);

	attrs_descp[5].binding = 0;
	attrs_descp[5].location = 5;
	attrs_descp[5].format = VK_FORMAT_R32G32_SFLOAT;
	attrs_descp[5].offset = offsetof(GPU_3D_Vertex, uv);

	attrs_descp[6].binding = 0;
	attrs_descp[6].location = 6;
	attrs_descp[6].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs_descp[6].offset = offsetof(GPU_3D_Vertex, color);

	return attrs_descp;
}


VkVertexInputBindingDescription GPU_UI_Vertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(GPU_UI_Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> GPU_UI_Vertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 2> attrs_descp = {};

	attrs_descp[0].binding = 0;
	attrs_descp[0].location = 0;
	attrs_descp[0].format = VK_FORMAT_R32G32_SFLOAT;
	attrs_descp[0].offset = offsetof(GPU_UI_Vertex, local_pos);

	attrs_descp[1].binding = 0;
	attrs_descp[1].location = 1;
	attrs_descp[1].format = VK_FORMAT_R32_UINT;
	attrs_descp[1].offset = offsetof(GPU_UI_Vertex, local_vert_idx);

	return attrs_descp;
}
