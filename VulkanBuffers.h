#pragma once

// GLM
#include "glm/vec3.hpp"
#include "glm/mat4x2.hpp"

// Mine
#include "VulkanMemory.h"


// Forward declarations
class Device;
class ComandPools;


enum class LoadType {
	STAGING,
	MEMCPY,
	ENUM_NOT_INIT
};


class Buffer
{
public:
	Device const* dev = nullptr;  // device that owns the buffer
	DeviceMemory* dev_mem;

	BufferUsage buff_usage = BufferUsage_enum_NOT_INITILIZED;
	MemoryUsage mem_usage = MemoryUsage_enum_NOT_INITILIZED;

	VkDeviceSize buff_size = 0;
	VkBuffer buff = VK_NULL_HANDLE;
	Allocation* alloc = nullptr;

	VkDeviceSize staging_buff_size = 0;
	VkBuffer staging_buff = VK_NULL_HANDLE;
	Allocation* staging_alloc = nullptr;

	// helper
	LoadType load_type = LoadType::ENUM_NOT_INIT;
public:
	void init(Device* device, DeviceMemory* device_memory, BufferUsage buff_usage, MemoryUsage mem_usage);

	ErrorStack build();

	void destroy();

	~Buffer();

	std::string name();
};


// Vertex Buffer
class GPUVertex 
{
public:
	uint32_t mesh_id;
	glm::vec3 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};


// Storage
struct GPUStorage
{
	alignas(16) glm::vec3 mesh_pos;
	alignas(16) glm::vec4 mesh_rot;
};


// Uniform
struct GPUUniform 
{
	alignas(16) glm::vec3 camera_pos;
	alignas(16) glm::vec4 camera_rot;
	alignas(16) glm::mat4 camera_perspective;
};


class DescriptorSets
{
public:
	Device* dev = nullptr;

	// Bounded
	bool uniform_update = false;
	bool storage_update = false;

	Buffer* uniform_buff;
	Buffer* storage_buff;

	VkDescriptorSetLayout descp_layout = VK_NULL_HANDLE;
	VkDescriptorPool descp_pool = VK_NULL_HANDLE;
	VkDescriptorSet descp_set = VK_NULL_HANDLE;
	std::vector<VkWriteDescriptorSet> descp_writes;

public:
	ErrorStack buildDescpSet();

	void update();

	void destroy();

	~DescriptorSets();
};
