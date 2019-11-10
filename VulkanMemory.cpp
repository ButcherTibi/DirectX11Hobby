
// Standard
#include <execution>

// Vulkan
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

// Other
#include "VulkanContext.h"
#include "ErrorStuff.h"

#include "VulkanMemory.h"


std::string Allocation::name()
{
	return "Alocation of size = " + size;
}

Allocation::Allocation()
{
	// std::cout << "Alocation constructor" << std::endl;
}

Allocation::Allocation(const Allocation& alloc)
{
	// std::cout << "Alocation constructor const ref" << std::endl;
}

// forgot or still not sure how it works
static int32_t findMemoryType(VkPhysicalDevice physical_device, uint32_t type_bits, VkMemoryPropertyFlags mem_props)
{
	VkPhysicalDeviceMemoryProperties device_mem_prop;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &device_mem_prop);

	for (uint32_t i = 0; i < device_mem_prop.memoryTypeCount; i++) {

		// does memory support type_bits (VERTEX, INDEX or TRANSFER_SRC) and
		// is memory (DEVICE_LOCAL, HOST_VISIBLE etc) 
		if ((1 << i) & type_bits &&
			(device_mem_prop.memoryTypes[i].propertyFlags & mem_props) == mem_props)
		{
			return i;
		}
	}
	return -1;
}

ErrorStack Allocation::map_whole()
{
	assert_cond(this->is_mapped == false, "double memory mapping");

	Device* dev = mem_type->dev_mem->dev;
	VkResult res = vkMapMemory(dev->logical_device, this->mem, 0, this->size, 0, &this->data);
	if (res != VK_SUCCESS) {
		return ErrorStack(res, ExtraError::FAILURE_TO_MAP_MEMORY, code_location, "failed to map memory");
	}
	this->is_mapped = true;
	return ErrorStack();
}

void Allocation::unmap()
{
	assert_cond(is_mapped == true, "unmapping unmapped memory");

	Device* dev = mem_type->dev_mem->dev;
	vkUnmapMemory(dev->logical_device, mem);
	is_mapped = false;
}

void Allocation::free()
{
	destroy_flag = true;
	mem_type->deallocateMemory();
}

Allocation::~Allocation()
{
	std::cout << "~Allocation" << std::endl;

	if (mem_type != nullptr && mem != VK_NULL_HANDLE) {
		Device* dev = mem_type->dev_mem->dev;
		vkFreeMemory(dev->logical_device, this->mem, NULL);

		mem_type->dev_mem->decrementAllocationCount();
	}
}

std::string MemoryType::name()
{
	if (mem_type & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
		return "MemoryType of Device Local type";
	}
	else if (mem_type & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) 
	{
		return "MemoryType of Device Local, Host Visible and Host Coherent type";
	}
	else if (mem_type & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
		return "MemoryType of Host Visible and Host Coherent type";
	}
	else if (mem_type & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
		VK_MEMORY_PROPERTY_HOST_CACHED_BIT)) {
		return "MemoryType of Host Visible, Host Coherent and Host Cached type";
	}
	return "MemoryType of unrecognized type";
}

MemoryType::MemoryType()
{
	this->dev_mem = nullptr;
	this->mem_type = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
	this->buff_usages = { VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM,
		VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM, 
		VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM, 
		VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM, 
		VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM};
}

void MemoryType::init(DeviceMemory* dev_mem, VkMemoryPropertyFlags mem_type)
{
	std::cout << "  MemoryType init" << std::endl;

	this->dev_mem = dev_mem;
	this->mem_type = mem_type;
	this->buff_usages = { VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM,
		VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM,
		VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM,
		VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM,
		VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM };
}

ErrorStack MemoryType::allocateMemory(VkMemoryRequirements mem_req, Allocation*& r_alloc)
{
	Device* dev = dev_mem->dev;
	uint32_t& alloc_count_limit = dev->phys_dev_props.limits.maxMemoryAllocationCount;

	// lock access to alloc count for > comparison
	{
		std::scoped_lock l(dev_mem->alloc_count_mutex);

		if (dev_mem->allocation_count + 1 > alloc_count_limit) {

			return ErrorStack(ExtraError::TOO_MANY_MEMORY_ALOCATIONS, code_location, "too many device alocations current = " +
				std::to_string(dev_mem->allocation_count) + ", max = " + std::to_string(alloc_count_limit));
		}
		else {
			dev_mem->allocation_count++;
		}
	}	

	uint32_t type_idx = findMemoryType(dev->physical_device, mem_req.memoryTypeBits, mem_type);
	if (type_idx == -1) {
		dev_mem->decrementAllocationCount();
		return ErrorStack(ExtraError::NO_SUITABLE_MEMORY_TYPE, code_location, "no suitable memory type index found for desired properties");
	}
	
	// register allocation to memory type
	{
		std::scoped_lock l(this->allocs_mutex);

		r_alloc = &allocs.emplace_front();
	}

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_req.size;
	alloc_info.memoryTypeIndex = type_idx;

	VkResult result = vkAllocateMemory(dev->logical_device, &alloc_info, nullptr, &r_alloc->mem);
	if (result != VK_SUCCESS) {
		dev_mem->decrementAllocationCount();

		r_alloc->free();
		return ErrorStack(result, code_location, "failed to allocate memory");
	}
	r_alloc->mem_type = this;
	r_alloc->size = mem_req.size;

	return ErrorStack();
}

void MemoryType::deallocateMemory()
{
	std::scoped_lock l(allocs_mutex);

	allocs.remove_if([](Allocation& alloc) {
		return alloc.destroy_flag;
	});
}

void MemoryType::setBufferUsageToBasic(VkBufferUsageFlags staging)
{
	buff_usages[BufferUsage::STAGING] = staging;
	buff_usages[BufferUsage::VERTEX] = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buff_usages[BufferUsage::INDEX] = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	buff_usages[BufferUsage::UNIFORM] = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buff_usages[BufferUsage::STORAGE] = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
}

void MemoryType::setBufferUsageToDest(VkBufferUsageFlags staging)
{
	buff_usages[BufferUsage::STAGING] = staging;
	buff_usages[BufferUsage::VERTEX] = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buff_usages[BufferUsage::INDEX] = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buff_usages[BufferUsage::UNIFORM] = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buff_usages[BufferUsage::STORAGE] = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
}

void MemoryType::destroy()
{
	std::cout << "  MemoryType destroy" << std::endl;

	allocs.clear();
}

VkDeviceSize MemoryType::getTrackedAllocsSize()
{
	std::scoped_lock l(allocs_mutex);

	std::atomic_uint64_t size = 0;

	std::for_each(std::execution::par, allocs.begin(), allocs.end(), [&size](Allocation& alloc) {
		size.fetch_add(alloc.size, std::memory_order_relaxed);
	});

	return size;
}

void DeviceMemory::build()
{
	std::cout << "DeviceMemory build" << std::endl;

	const VkMemoryPropertyFlags device_local_visible_coherent = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	const VkMemoryPropertyFlags device_local = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	const VkMemoryPropertyFlags host_visible_coherent = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	const VkMemoryPropertyFlags host_visible_coherent_cached = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
		VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

	// Initialize MemoryType
	{
		device_local_visible_coherent_mem.init(this, device_local_visible_coherent);
		device_local_visible_coherent_mem.setBufferUsageToBasic();

		device_local_mem.init(this, device_local);
		device_local_mem.setBufferUsageToDest();

		host_visible_coherent_mem.init(this, host_visible_coherent);
		host_visible_coherent_mem.setBufferUsageToBasic(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

		host_visible_coherent_cached_mem.init(this, host_visible_coherent_cached);
		host_visible_coherent_cached_mem.setBufferUsageToBasic(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	}

	// Initilize memory recomendations
	{
		std::vector<MemoryType*>* gpu_bulk = &mem_recoms[MemoryUsage::GPU_BULK_DATA];
		std::vector<MemoryType*>* gpu_upload = &mem_recoms[MemoryUsage::GPU_UPLOAD];
		std::vector<MemoryType*>* gpu_download = &mem_recoms[MemoryUsage::GPU_DOWNLOAD];

		// 1st best
		for (uint32_t i = 0; i < dev->mem_props.memoryTypeCount; i++) {

			uint32_t heap_idx = dev->mem_props.memoryTypes[i].heapIndex;

			switch (dev->mem_props.memoryTypes[i].propertyFlags) {

			case device_local:
				gpu_bulk->push_back(&device_local_mem);

				device_local_mem.type_idx = i;
				device_local_mem.heap_idx = heap_idx;
				break;

			case device_local_visible_coherent:
				gpu_upload->push_back(&device_local_visible_coherent_mem);

				device_local_visible_coherent_mem.type_idx = i;
				device_local_visible_coherent_mem.heap_idx = heap_idx;
				break;

			case host_visible_coherent:
				host_visible_coherent_mem.type_idx = i;
				host_visible_coherent_mem.heap_idx = heap_idx;
				break;

			case host_visible_coherent_cached:
				gpu_download->push_back(&host_visible_coherent_cached_mem);

				host_visible_coherent_cached_mem.type_idx = i;
				host_visible_coherent_cached_mem.heap_idx = heap_idx;
				break;
			}
		}

		// 2nd best
		for (uint32_t i = 0; i < dev->mem_props.memoryTypeCount; i++) {

			switch (dev->mem_props.memoryTypes[i].propertyFlags) {

			case device_local_visible_coherent:
				gpu_bulk->push_back(&device_local_visible_coherent_mem);
				break;

			case device_local:
				gpu_upload->push_back(&device_local_mem);
				break;

			case host_visible_coherent:
				gpu_download->push_back(&host_visible_coherent_mem);
				break;
			}
		}

		// 3rd best ?
	}
}

void DeviceMemory::init(Device *device)
{
	std::cout << "DeviceMemory init" << std::endl;

	this->dev = device;
}

void DeviceMemory::destroy()
{
	std::cout << "DeviceMemory destroy" << std::endl;

	device_local_visible_coherent_mem.destroy();
	device_local_mem.destroy();
	host_visible_coherent_mem.destroy();
	host_visible_coherent_cached_mem.destroy();

	for (auto& mem_recom : mem_recoms) {
		mem_recom.clear();
	}
}

void DeviceMemory::decrementAllocationCount()
{
	std::scoped_lock l(this->alloc_count_mutex);

	allocation_count--;
}

void DeviceMemory::incrementAllocationCount()
{
	std::scoped_lock l(alloc_count_mutex);

	allocation_count++;
}

ErrorStack DeviceMemory::allocateBuffer(VkDeviceSize buff_size, MemoryUsage mem_usage,
	const std::vector<BufferUsage>& buff_usages, VkBuffer& r_buff, Allocation*& r_alloc)
{
	for (MemoryType* mem_type : this->mem_recoms[mem_usage]) {

		// in case buffer has multiple usages eg. vertex and index
		VkBufferUsageFlags buff_usage_flags = 0;
		for (BufferUsage usage : buff_usages) {
			buff_usage_flags |= mem_type->buff_usages[usage];
		}

		VkBufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.size = buff_size;
		create_info.usage = buff_usage_flags;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult vkres = vkCreateBuffer(this->dev->logical_device, &create_info, NULL, &r_buff);
		if (vkres != VK_SUCCESS) {
			return ErrorStack(vkres, code_location, "failed to create buffer");
		}
		
		VkMemoryRequirements mem_req;
		vkGetBufferMemoryRequirements(dev->logical_device, r_buff, &mem_req);

		ErrorStack error = mem_type->allocateMemory(mem_req, r_alloc);
		if (error.isBad()) {

			// keep trying to allocate on another memory type
			if (error.lastError().vk_err == VK_ERROR_OUT_OF_DEVICE_MEMORY) {		
				vkDestroyBuffer(dev->logical_device, r_buff, NULL);
				continue;
			}
			else {
				return error;
			}
		}

		vkres = vkBindBufferMemory(dev->logical_device, r_buff, r_alloc->mem, 0);
		if (vkres != VK_SUCCESS) {

			vkDestroyBuffer(dev->logical_device, r_buff, NULL);
			r_alloc->free();
			return ErrorStack(vkres, code_location, "failed to bind memory to buffer");
		}
		break;
	}
	return ErrorStack();
}

void DeviceMemory::deallocateBuffer(VkBuffer& buff, Allocation* r_alloc)
{
	assert_cond(this->allocation_count > 0, "deallocation on no allocations");

	vkDestroyBuffer(this->dev->logical_device, buff, NULL);
	r_alloc->destroy_flag = true;
	r_alloc->mem_type->deallocateMemory();
}

void DeviceMemory::updateMemoryStats()
{
	VkPhysicalDeviceMemoryBudgetPropertiesEXT mem_budget_info = {};
	mem_budget_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;

	VkPhysicalDeviceMemoryProperties2 mem_props_info = {};
	mem_props_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
	mem_props_info.pNext = &mem_budget_info;

	dev->inst->getMemProps2(dev->physical_device, &mem_props_info);

	uint32_t idx = device_local_mem.heap_idx;
	device_local_mem.usage = mem_budget_info.heapUsage[idx];
	device_local_mem.budget = mem_budget_info.heapBudget[idx];

	idx = device_local_visible_coherent_mem.heap_idx;
	device_local_visible_coherent_mem.usage = mem_budget_info.heapUsage[idx];
	device_local_visible_coherent_mem.budget = mem_budget_info.heapBudget[idx];

	idx = host_visible_coherent_mem.heap_idx;
	host_visible_coherent_mem.usage = mem_budget_info.heapUsage[idx];
	host_visible_coherent_mem.budget = mem_budget_info.heapBudget[idx];

	idx = host_visible_coherent_cached_mem.heap_idx;
	host_visible_coherent_cached_mem.usage = mem_budget_info.heapUsage[idx];
	host_visible_coherent_cached_mem.budget = mem_budget_info.heapBudget[idx];
}
