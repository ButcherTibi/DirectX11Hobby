

// Header
#include "VulkanSystems.h"


namespace vks {

	vks::LoadType findLoadType(LogicalDevice* logical_dev, uint32_t mem_type_idx)
	{
		VkMemoryPropertyFlags mem_flags;
		vmaGetMemoryTypeProperties(logical_dev->allocator, mem_type_idx, &mem_flags);

		// Staging buffer required
		if ((mem_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
			return LoadType::STAGING;
		}
		else {
			return LoadType::MEMCPY;
		}
		return LoadType::ENUM_NOT_INIT;
	}

	ErrorStack Buffer::create(LogicalDevice* logical_dev,
		VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage mem_usage)
	{
		this->logical_dev = logical_dev;

		assert_cond(size > 0, "buffer size must be larger than zero");

		VkBufferCreateInfo buff_info = {};
		buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buff_info.size = size;
		buff_info.usage = usage;
		buff_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo alloc_create_info = {};
		alloc_create_info.usage = mem_usage;

		checkVkRes(vmaCreateBuffer(logical_dev->allocator, &buff_info, &alloc_create_info,
			&buff, &this->buff_alloc, &buff_alloc_info),
			"failed to create buffer");

		load_type = findLoadType(logical_dev, buff_alloc_info.memoryType);
		if (load_type == LoadType::MEMCPY) {
			vmaMapMemory(logical_dev->allocator, buff_alloc, &mem);
		}

		return ErrorStack();
	}

	ErrorStack Buffer::createOrGrowStaging(LogicalDevice* logical_dev, VkDeviceSize min_size,
		VmaMemoryUsage mem_usage)
	{
		if (buff == VK_NULL_HANDLE) {
			return create(logical_dev, min_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, mem_usage);
		}
		else if (buff_alloc_info.size < min_size) {

			destroy();
			return create(logical_dev, min_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, mem_usage);
		}
		return ErrorStack();
	}

	ErrorStack Buffer::load(LogicalDevice* logical_dev, CommandPool* cmd_pool, Buffer* staging,
		void* data, size_t size)
	{
		assert_cond(size > 0, "load size must be larger than zero");

		if (load_type == LoadType::MEMCPY) {
			std::memcpy(mem, data, size);
			return ErrorStack();
		}
		else {
			std::memcpy(staging->mem, data, size);

			ErrorStack err;
			{
				auto record = SingleCommandBuffer(logical_dev, cmd_pool, &err);

				VkBufferCopy regions = {};
				regions.size = size;
				vkCmdCopyBuffer(record.cmd_buff, staging->buff, buff, 1, &regions);
			}
		}
		return ErrorStack();
	}

	void Buffer::destroy()
	{
		if (this->load_type == LoadType::MEMCPY) {
			vmaUnmapMemory(logical_dev->allocator, this->buff_alloc);
		}

		vmaDestroyBuffer(logical_dev->allocator, this->buff, this->buff_alloc);
		buff = VK_NULL_HANDLE;

		this->load_type = LoadType::ENUM_NOT_INIT;
	}

	Buffer::~Buffer()
	{
		if (buff != VK_NULL_HANDLE) {
			destroy();
		}
	}


	ErrorStack changeImageLayout(LogicalDevice* logical_dev, CommandPool* cmd_pool,
		Image* img, VkImageLayout new_layout)
	{
		ErrorStack err;

		auto record = SingleCommandBuffer(logical_dev, cmd_pool, &err);
		checkErrStack(err, "");

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = img->layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = img->img;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags src_stage;
		VkPipelineStageFlags dst_stage;

		if (img->layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (img->layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			return ErrorStack(code_location, "unsupported image layout transition");
		}

		vkCmdPipelineBarrier(
			record.cmd_buff,
			src_stage, dst_stage,
			0,
			0, NULL,
			0, NULL,
			1, &barrier
		);

		img->layout = new_layout;

		return err;
	}

	ErrorStack copyBufferToImage(LogicalDevice* logical_dev, CommandPool* cmd_pool, Buffer* buff, Image* img)
	{
		ErrorStack err;
		auto record = SingleCommandBuffer(logical_dev, cmd_pool, &err);

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {img->width, img->height, 1};

		vkCmdCopyBufferToImage(record.cmd_buff, buff->buff, img->img, img->layout,
			1, &region);

		return err;
	}


	ErrorStack Image::create(LogicalDevice* logical_dev, 
		VkImageCreateInfo& img_info, VmaMemoryUsage mem_usage)
	{
		this->logical_dev = logical_dev;
		this->width = img_info.extent.width;
		this->height = img_info.extent.height;
		this->layout = img_info.initialLayout;

		VmaAllocationCreateInfo alloc_create_info = {};
		alloc_create_info.usage = mem_usage;

		checkVkRes(vmaCreateImage(logical_dev->allocator, &img_info, &alloc_create_info, &img, &alloc, &alloc_info),
			"failed to create image");

		load_type = findLoadType(logical_dev, alloc_info.memoryType);
		if (load_type == LoadType::MEMCPY) {
			checkVkRes(vmaMapMemory(logical_dev->allocator, alloc, &mem), "");
		}

		return ErrorStack();
	}

	ErrorStack findSupportedImageFormat(vks::PhysicalDevice* phys_dev, std::vector<DesiredImageProps> desires,
		DesiredImageProps& result)
	{
		VkResult vk_res;

		for (DesiredImageProps desire : desires) {

			vk_res = vkGetPhysicalDeviceImageFormatProperties(phys_dev->physical_device, desire.format, desire.type,
				desire.tiling, desire.usage, desire.flags, &desire.props_found);
			if (vk_res == VK_SUCCESS) {
				result = desire;
				return ErrorStack();
			}
			else if (vk_res == VK_ERROR_FORMAT_NOT_SUPPORTED) {
				continue;
			}
			else {
				return ErrorStack(vk_res, code_location, "failed to check for desired image properties");
			}
		}

		return ErrorStack(code_location, "desired image properties not supported");
	}

	ErrorStack Image::create(LogicalDevice* logical_dev, PhysicalDevice* phys_dev,
		ImageCreateInfo& img_info, VmaMemoryUsage mem_usage)
	{
		this->logical_dev = logical_dev;
		this->width = img_info.width;
		this->height = img_info.height;
		this->layout = VK_IMAGE_LAYOUT_UNDEFINED;

		DesiredImageProps img_prop;
		checkErrStack(findSupportedImageFormat(phys_dev, *img_info.desired_props, img_prop), "");

		VkImageCreateInfo vk_img_info = {};
		vk_img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		vk_img_info.flags = img_prop.flags;
		vk_img_info.imageType = img_prop.type;
		vk_img_info.format = img_prop.format;
		this->format = img_prop.format;
		vk_img_info.extent.width = img_info.width;
		vk_img_info.extent.height = img_info.height;
		vk_img_info.extent.depth = 1;
		vk_img_info.mipLevels = 1;
		vk_img_info.arrayLayers = 1;
		vk_img_info.samples = VK_SAMPLE_COUNT_1_BIT;
		vk_img_info.tiling = img_prop.tiling;
		vk_img_info.usage = img_prop.usage;
		vk_img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vk_img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo alloc_create_info = {};
		alloc_create_info.usage = mem_usage;

		checkVkRes(vmaCreateImage(logical_dev->allocator, &vk_img_info, &alloc_create_info,
			&img, &alloc, &alloc_info),
			"failed to create image");

		this->load_type = findLoadType(logical_dev, alloc_info.memoryType);
		if (load_type == LoadType::MEMCPY) {
			checkVkRes(vmaMapMemory(logical_dev->allocator, alloc, &mem), "");
		}

		return ErrorStack();
	}

	void Image::destroy()
	{
		if (this->load_type == LoadType::MEMCPY) {
			vmaUnmapMemory(logical_dev->allocator, alloc);
		}

		vmaDestroyImage(logical_dev->allocator, img, alloc);
		img = VK_NULL_HANDLE;

		this->load_type = LoadType::ENUM_NOT_INIT;
	}

	Image::~Image()
	{
		if (img != VK_NULL_HANDLE) {
			destroy();
		}
	}
}
