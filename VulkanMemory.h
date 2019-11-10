#pragma once

// Standard
#include <array>
#include <vector>
#include <forward_list>
#include <atomic>
#include <mutex>


class Device;

class MemoryType;
class DeviceMemory;


// Wrapper for VkDeviceMemory
class Allocation
{
public:
	MemoryType* mem_type = nullptr;

	VkDeviceSize size = 0;
	VkDeviceMemory mem = VK_NULL_HANDLE;

	bool is_mapped = false;
	void* data;

	bool destroy_flag = false;
public:
	std::string name();

public:
	Allocation();
	Allocation(const Allocation& alloc);

	// Mapping
	ErrorStack map_whole();
	void unmap();

	// helper method
	void free();

	~Allocation();
};


// Represents a type of memory
// track and owns allocations
// specifies parameters to use when loading for abstracting memory types
class MemoryType
{
public:
	// Parent
	DeviceMemory* dev_mem;
	
	uint32_t type_idx;
	uint32_t heap_idx;
	VkMemoryPropertyFlags mem_type;

	VkDeviceSize usage;
	VkDeviceSize budget;

	std::mutex allocs_mutex;
	std::forward_list<Allocation> allocs;

	std::array<VkBufferUsageFlags, 5> buff_usages;

public:
	std::string name();

public:
	MemoryType();

	void init(DeviceMemory* dev_mem, VkMemoryPropertyFlags mem_type);

	ErrorStack allocateMemory(VkMemoryRequirements mem_req, Allocation*& r_alloc);

	// frees allocations marked for destruction
	void deallocateMemory();

	void destroy();

	VkDeviceSize getTrackedAllocsSize();

	// Helper functions
	void setBufferUsageToBasic(VkBufferUsageFlags staging = 0);
	void setBufferUsageToDest(VkBufferUsageFlags staging = 0);
};


// Hints for memory usage
enum MemoryUsage {
	GPU_BULK_DATA,
	GPU_UPLOAD,
	GPU_DOWNLOAD,
	MemoryUsage_enum_NOT_INITILIZED
};

// What sort of data will the buffer contain
enum BufferUsage {
	STAGING,
	VERTEX,
	INDEX,
	UNIFORM,
	STORAGE,
	BufferUsage_enum_NOT_INITILIZED
};


// Tracks total number of allocations across all memory types
// creates memory recomendatins according to hardware limitations
// able to allocate Buffers
class DeviceMemory 
{
public:
	// Parent 
	Device* dev = nullptr;

	std::mutex alloc_count_mutex;
	uint32_t allocation_count = 0;
 
	MemoryType device_local_visible_coherent_mem;
	MemoryType device_local_mem;
	MemoryType host_visible_coherent_mem;
	MemoryType host_visible_coherent_cached_mem;

	// Memory recomendations
	std::array<std::vector<MemoryType*>, 3> mem_recoms;

public:
	void init(Device* device);

	void build();
	void destroy();

	void decrementAllocationCount();
	void incrementAllocationCount();

	ErrorStack allocateBuffer(VkDeviceSize buff_size, MemoryUsage mem_usage,
		const std::vector<BufferUsage>& buff_usages,
		VkBuffer& r_buff, Allocation*& r_alloc);

	void deallocateBuffer(VkBuffer& buff, Allocation* alloc);

	// updates memory usage statistics
	void updateMemoryStats();
};