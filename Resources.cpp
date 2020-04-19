

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

	ErrStack Buffer::create(LogicalDevice* logical_dev,
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

		return ErrStack();
	}

	ErrStack Buffer::recreate(LogicalDevice* logical_dev,
		VkDeviceSize min_size, VkBufferUsageFlags usage, VmaMemoryUsage mem_usage)
	{
		if (buff == VK_NULL_HANDLE) {
			return create(logical_dev, min_size, usage, mem_usage);
		}
		else if (buff_alloc_info.size < min_size) {

			destroy();
			return create(logical_dev, min_size, usage, mem_usage);
		}
		return ErrStack();
	}

	ErrStack Buffer::recreateStaging(LogicalDevice* logical_dev, VkDeviceSize min_size)
	{
		if (buff == VK_NULL_HANDLE) {
			return create(logical_dev, min_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		}
		else if (buff_alloc_info.size < min_size) {

			destroy();
			return create(logical_dev, min_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		}
		return ErrStack();
	}

	ErrStack Buffer::load(CommandPool* cmd_pool, Buffer* staging,
		void* data, size_t size)
	{
		assert_cond(size > 0, "load size must be larger than zero");

		if (load_type == LoadType::MEMCPY) {
			std::memcpy(mem, data, size);
			return ErrStack();
		}
		else {
			std::memcpy(staging->mem, data, size);

			ErrStack err;
			{
				auto record = SingleCommandBuffer(logical_dev, cmd_pool, &err);

				VkBufferCopy regions = {};
				regions.size = size;
				vkCmdCopyBuffer(record.cmd_buff, staging->buff, buff, 1, &regions);
			}
		}
		return ErrStack();
	}

	void Buffer::scheduleLoad(size_t offset, Buffer* staging, void* data, size_t size)
	{
		assert_cond(size > 0, "load size must be larger than zero");

		scheduled_load_size += size;

		if (load_type == LoadType::MEMCPY) {

			void* dst = (uint8_t*)this->mem + offset;
			std::memcpy(dst, data, size);
		}
		else {
			void* dst = (uint8_t*)staging->mem + offset;
			std::memcpy(dst, data, size);
		}
	}

	ErrStack Buffer::flush(CommandPool* cmd_pool, Buffer* staging)
	{
		ErrStack err;
		// Do nothing if memcpy because HOST_COHERENT

		if (load_type == LoadType::STAGING) {

			auto record = SingleCommandBuffer(logical_dev, cmd_pool, &err);

			VkBufferCopy regions = {};
			regions.size = scheduled_load_size;
			vkCmdCopyBuffer(record.cmd_buff, staging->buff, buff, 1, &regions);
		}

		scheduled_load_size = 0;
		return err;
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

	ErrStack Buffer::setDebugName(std::string name)
	{
		checkErrStack(logical_dev->setDebugName(
			reinterpret_cast<uint64_t>(buff), VK_OBJECT_TYPE_BUFFER, name + " VkBuffer"), 
			"");
		return ErrStack();
	}

	ErrStack findSupportedImageFormat(vks::PhysicalDevice* phys_dev, std::vector<DesiredImageProps> desires,
		DesiredImageProps& result)
	{
		VkResult vk_res;

		for (DesiredImageProps desire : desires) {

			vk_res = vkGetPhysicalDeviceImageFormatProperties(phys_dev->physical_device, desire.format, desire.type,
				desire.tiling, desire.usage, 0, &desire.props_found);
			if (vk_res == VK_SUCCESS) {
				result = desire;
				return ErrStack();
			}
			else if (vk_res == VK_ERROR_FORMAT_NOT_SUPPORTED) {
				continue;
			}
			else {
				return ErrStack(vk_res, code_location, "failed to check for desired image properties");
			}
		}

		return ErrStack(code_location, "desired image properties not supported");
	}

	ErrStack Image::copyBufferToImage(CommandPool* cmd_pool, Buffer* buff)
	{
		ErrStack err;
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
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(record.cmd_buff, buff->buff, img, layout,
			1, &region);

		return err;
	}

	ErrStack Image::recreate(LogicalDevice* logical_dev, PhysicalDevice* phys_dev,
		ImageCreateInfo& img_info, VmaMemoryUsage mem_usage)
	{
		if (this->img != VK_NULL_HANDLE) {
			this->destroy();
		}

		this->logical_dev = logical_dev;
		this->width = img_info.width;
		this->height = img_info.height;
		this->layout = VK_IMAGE_LAYOUT_UNDEFINED;
		this->mip_lvl = img_info.mip_levels;
		this->samples = img_info.samples;

		DesiredImageProps img_prop;
		checkErrStack(findSupportedImageFormat(phys_dev, *img_info.desired_props, img_prop), 
			"failed to find supported image format");

		VkImageCreateInfo vk_img_info = {};
		vk_img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		vk_img_info.flags = 0;
		vk_img_info.imageType = img_prop.type;
		vk_img_info.format = img_prop.format;
		this->format = img_prop.format;
		vk_img_info.extent.width = img_info.width;
		vk_img_info.extent.height = img_info.height;
		vk_img_info.extent.depth = 1;
		vk_img_info.mipLevels = img_info.mip_levels;
		vk_img_info.arrayLayers = 1;
		vk_img_info.samples = img_info.samples;
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

		// View
		VkComponentMapping component_mapping = {};
		component_mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		component_mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		component_mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		component_mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		VkImageSubresourceRange sub_resource = {};
		sub_resource.aspectMask = img_info.aspect;
		sub_resource.baseMipLevel = 0;
		sub_resource.levelCount = img_info.mip_levels;
		sub_resource.baseArrayLayer = 0;
		sub_resource.layerCount = 1;

		VkImageViewCreateInfo imageview_info = {};
		imageview_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageview_info.image = img;
		imageview_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageview_info.format = format;
		imageview_info.components = component_mapping;
		imageview_info.subresourceRange = sub_resource;

		checkVkRes(vkCreateImageView(logical_dev->logical_device, &imageview_info, NULL, &img_view),
			"failed to create image view");

		return ErrStack();
	}

	ErrStack Image::setDebugName(std::string name)
	{
		checkErrStack(logical_dev->setDebugName(
			reinterpret_cast<uint64_t>(img), VK_OBJECT_TYPE_IMAGE, name + " VkImage"), "");

		return logical_dev->setDebugName(
			reinterpret_cast<uint64_t>(img_view), VK_OBJECT_TYPE_IMAGE, name + " VkImageView");
	}

	ErrStack Image::changeImageLayout(CommandPool* cmd_pool, VkImageLayout new_layout)
	{
		ErrStack err;

		{
			auto record = SingleCommandBuffer(logical_dev, cmd_pool, &err);
			checkErrStack(err, "");

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = layout;
			barrier.newLayout = new_layout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = img;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = mip_lvl;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags src_stage;
			VkPipelineStageFlags dst_stage;

			if (layout == VK_IMAGE_LAYOUT_UNDEFINED) {

				barrier.srcAccessMask = 0;
				src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

				if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;			
					dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				}
				else if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {

					barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				}
				else {
					return ErrStack(code_location, "unsupported image layout transition");
				}
			}
			else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

				if (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				else {
					return ErrStack(code_location, "unsupported image layout transition");
				}
			}
			else {
				return ErrStack(code_location, "unsupported image layout transition");
			}

			vkCmdPipelineBarrier(
				record.cmd_buff,
				src_stage, dst_stage,
				0,
				0, NULL,
				0, NULL,
				1, &barrier
				);

			layout = new_layout;
		}

		return err;
	}

	ErrStack Image::load(void* colors, size_t size, CommandPool* cmd_pool, Buffer* staging_buff,
		VkImageLayout layout_after_load)
	{
		// Load Into Staging Buffer
		std::memcpy(staging_buff->mem, colors, size);

		// Load Staging Buffer into Image
		if (layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			checkErrStack(changeImageLayout(cmd_pool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL), 
				"failed to change image layout for optimal destination");
		}
		
		// Load Staging to Image
		checkErrStack(copyBufferToImage(cmd_pool, staging_buff),
			"failed to copy buffer to image");

		// Change layout
		if (this->layout != layout_after_load) {
			checkErrStack(changeImageLayout(cmd_pool, layout_after_load), 
				"failed to change image layout");
		}

		return ErrStack();
	}

	void Image::destroy()
	{
		if (this->load_type == LoadType::MEMCPY) {
			vmaUnmapMemory(logical_dev->allocator, alloc);
		}

		vmaDestroyImage(logical_dev->allocator, img, alloc);
		img = VK_NULL_HANDLE;

		this->load_type = LoadType::ENUM_NOT_INIT;

		vkDestroyImageView(logical_dev->logical_device, img_view, NULL);
		img_view = VK_NULL_HANDLE;
	}

	Image::~Image()
	{
		if (img != VK_NULL_HANDLE) {
			destroy();
		}
	}
}
