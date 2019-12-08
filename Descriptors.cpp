
// Standard
#include <array>

// Other
#include "ErrorStuff.h"
#include "VulkanContext.h"

#include "VulkanBuffers.h"


VkVertexInputBindingDescription GPUVertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(GPUVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> GPUVertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32_UINT;
	attributeDescriptions[0].offset = offsetof(GPUVertex, mesh_id);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(GPUVertex, pos);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(GPUVertex, color);

	return attributeDescriptions;
}

ErrorStack DescriptorSets::buildDescpSet()
{
	std::cout << "DescriptorSets build" << std::endl;

	VkResult res;

	// Descriptor Set Layout
	{
		std::array<VkDescriptorSetLayoutBinding, 2> bindings;

		VkDescriptorSetLayoutBinding uniform_bind = {};
		uniform_bind.binding = 0;
		uniform_bind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniform_bind.descriptorCount = 1;
		uniform_bind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[0] = uniform_bind;

		VkDescriptorSetLayoutBinding storage_bind = {};
		storage_bind.binding = 1;
		storage_bind.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		storage_bind.descriptorCount = 1;
		storage_bind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[1] = storage_bind;

		VkDescriptorSetLayoutCreateInfo descp_layout_info = {};
		descp_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descp_layout_info.bindingCount = (uint32_t)(bindings.size());
		descp_layout_info.pBindings = bindings.data();

		res = vkCreateDescriptorSetLayout(dev->logical_device, &descp_layout_info, NULL, &descp_layout);
		if (res != VK_SUCCESS) {
			return ErrorStack(res, code_location, "failed to create descriptor set layout");
		}
	}

	// Descriptor pool
	{
		std::array<VkDescriptorPoolSize, 2> pool_sizes;

		VkDescriptorPoolSize uniform_size = {};
		uniform_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniform_size.descriptorCount = 1;
		pool_sizes[0] = uniform_size;

		VkDescriptorPoolSize storage_size = {};
		storage_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		storage_size.descriptorCount = 1;
		pool_sizes[1] = storage_size;

		VkDescriptorPoolCreateInfo descp_pool_info = {};
		descp_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descp_pool_info.maxSets = 1;
		descp_pool_info.poolSizeCount = (uint32_t)(pool_sizes.size());
		descp_pool_info.pPoolSizes = pool_sizes.data();

		res = vkCreateDescriptorPool(dev->logical_device, &descp_pool_info, NULL, &descp_pool);
		if (res != VK_SUCCESS) {
			return ErrorStack(res, code_location, "failed to create descriptor pool");
		}
	}

	// Allocate Descriptor Sets
	{
		VkDescriptorSetAllocateInfo descp_sets_info = {};
		descp_sets_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descp_sets_info.descriptorPool = descp_pool;
		descp_sets_info.descriptorSetCount = 1;
		descp_sets_info.pSetLayouts = &descp_layout;

		res = vkAllocateDescriptorSets(dev->logical_device, &descp_sets_info, &descp_set);
		if (res != VK_SUCCESS) {
			return ErrorStack(res, code_location, "failed to allocate descriptor sets");
		}
	}

	return ErrorStack();
}

void DescriptorSets::update()
{
	std::cout << "DescriptorSets update" << std::endl;

	descp_writes.clear();

	if (uniform_update) {

		VkDescriptorBufferInfo uniform_buff_info = {};
		uniform_buff_info.buffer = uniform_buff->buff;
		uniform_buff_info.offset = 0;
		uniform_buff_info.range = uniform_buff->buff_size;

		VkWriteDescriptorSet uniform_write = {};
		uniform_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uniform_write.dstSet = descp_set;
		uniform_write.dstBinding = 0;
		uniform_write.dstArrayElement = 0;
		uniform_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniform_write.descriptorCount = 1;
		uniform_write.pBufferInfo = &uniform_buff_info;

		descp_writes.push_back(uniform_write);
		uniform_update = false;
	}

	if (storage_update) {

		VkDescriptorBufferInfo storage_buff_info = {};
		storage_buff_info.buffer = storage_buff->buff;
		storage_buff_info.offset = 0;
		storage_buff_info.range = storage_buff->buff_size;

		VkWriteDescriptorSet storage_write = {};
		storage_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		storage_write.dstSet = descp_set;
		storage_write.dstBinding = 1;
		storage_write.dstArrayElement = 0;
		storage_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		storage_write.descriptorCount = 1;
		storage_write.pBufferInfo = &storage_buff_info;

		descp_writes.push_back(storage_write);
		storage_update = false;
	}

	if (descp_writes.size()) {

		vkUpdateDescriptorSets(dev->logical_device,
			(uint32_t) descp_writes.size(), descp_writes.data(), 0, NULL);
	}
}

void DescriptorSets::destroy()
{
	std::cout << "DescriptorSets destroy" << std::endl;

	vkDestroyDescriptorPool(dev->logical_device, descp_pool, NULL);
	vkDestroyDescriptorSetLayout(dev->logical_device, descp_layout, NULL);
	descp_pool = VK_NULL_HANDLE;
	descp_layout = VK_NULL_HANDLE;
}

DescriptorSets::~DescriptorSets()
{
	if (dev != nullptr) {

		this->destroy();
	}
}