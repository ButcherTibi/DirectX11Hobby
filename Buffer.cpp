

#include "ErrorStuff.h"
#include "VulkanContext.h"
#include "VulkanCommandPool.h"

#include "VulkanBuffers.h"


void Buffer::init(Device *device, DeviceMemory *device_memory, BufferUsage buff_usage, MemoryUsage mem_usage)
{
	this->dev = device;
	this->dev_mem = device_memory;

	this->buff_usage = buff_usage;
	this->mem_usage = mem_usage;
}

ErrorStack Buffer::build()
{
	std::cout << name() << " build" << std::endl;

	assert_cond(buff_size > 0, "buffer size must be larger than zero");

	ErrorStack err = dev_mem->allocateBuffer(buff_size, mem_usage, { buff_usage }, buff, alloc);
	if (err.isBad()) {
		err.pushError(code_location, "failed to allocate buffer");
	}

	// staging buffer required
	VkMemoryPropertyFlags mem_type = alloc->mem_type->mem_type;
	if ((mem_type & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == mem_type) {

		this->load_type = LoadType::STAGING;

		// create new staging
		if (staging_buff == VK_NULL_HANDLE) {

			err = dev_mem->allocateBuffer(buff_size, MemoryUsage::GPU_DOWNLOAD, { BufferUsage::STAGING }, staging_buff,
				staging_alloc);
			if (err.isBad()) {
				err.pushError(code_location, "failed to create new staging buffer");
				return err;
			}
			staging_buff_size = buff_size;

			err = staging_alloc->map_whole();
			if (err.isBad()) {
				err.pushError(code_location, "failed to map new staging buffer");
				return err;
			}
		}
		// recreate staging
		else if (buff_size > staging_buff_size) {

			dev_mem->deallocateBuffer(staging_buff, staging_alloc);

			err = dev_mem->allocateBuffer(buff_size, MemoryUsage::GPU_UPLOAD, { BufferUsage::STAGING }, staging_buff,
				staging_alloc);
			if (err.isBad()) {
				err.pushError(code_location, "failed to recreate staging buffer");
				return err;
			}
			staging_buff_size = buff_size;

			err = staging_alloc->map_whole();
			if (err.isBad()) {
				err.pushError(code_location, "failed to map new staging buffer");
				return err;
			}
		}
		// shrink staging if staging_buff_size - buff_size too large
	}
	// destroy staging if present
	else {
		this->load_type = LoadType::MEMCPY;

		err = alloc->map_whole();
		if (err.isBad()) {
			return err;
		}

		if (staging_buff != VK_NULL_HANDLE) {

			dev_mem->deallocateBuffer(staging_buff, staging_alloc);
			staging_buff = VK_NULL_HANDLE;
			staging_buff_size = 0;
		}	
	}

	return err;
}

void Buffer::destroy()
{
	std::cout << "Buffer destroy" << std::endl;

	dev_mem->deallocateBuffer(buff, alloc);
	buff = VK_NULL_HANDLE;

	this->load_type = LoadType::ENUM_NOT_INIT;
}

Buffer::~Buffer()
{
	if (this->dev_mem != nullptr) {

		if (buff != VK_NULL_HANDLE) {
			dev_mem->deallocateBuffer(buff, alloc);
		}
		if (staging_buff != VK_NULL_HANDLE) {
			dev_mem->deallocateBuffer(staging_buff, staging_alloc);
		}
	}
}

std::string Buffer::name()
{
	switch (buff_usage)
	{
	case STAGING:
		return "Staging Buffer";
	case VERTEX:
		return "Vertex Buffer";
	case INDEX:
		return "Index Buffer";
	case UNIFORM:
		return "Uniform Buffer";
	case STORAGE:
		return "Storage Buffer";
	case BufferUsage_enum_NOT_INITILIZED:
		return "Buffer with un-initilized usage";
	}
	return "Buffer with unrecognized usage";
}
