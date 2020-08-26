

// Header
#include "VulkanSystems.hpp"


namespace vks {

	ErrStack Buffer::create_(size_t size, 
		VkBuffer& new_buff, VmaAllocation& new_alloc, void*& new_mem)
	{
		VkBufferCreateInfo buff_info = {};
		buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buff_info.size = size;
		buff_info.usage = usage_;
		buff_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vma_info = {};
		vma_info.usage = mem_usage_;

		VkResult err = vmaCreateBuffer(logical_dev_->allocator, &buff_info, &vma_info,
			&new_buff, &new_alloc, &vma_r_info);
		if (err != VK_SUCCESS) {
			return ErrStack(code_location, "failed to create staging buffer");
		}

		VkMemoryPropertyFlags mem_flags;
		vmaGetMemoryTypeProperties(logical_dev_->allocator, vma_r_info.memoryType, &mem_flags);
		if (mem_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
			load_type_ = LoadType::MEMCPY;

			err = vmaMapMemory(logical_dev_->allocator, new_alloc, &new_mem);
			if (err != VK_SUCCESS) {
				return ErrStack(code_location, "failed to create staging buffer");
			}
		}
		else {
			load_type_ = LoadType::STAGING;

			this->mem = nullptr;
		}

		return ErrStack();
	}

	void Buffer::create(LogicalDevice* logical_dev, CommandPool* cmd_pool, StagingBuffer* staging,
		VkBufferUsageFlags usage, VmaMemoryUsage mem_usage)
	{
		this->logical_dev_ = logical_dev;
		this->cmd_pool_ = cmd_pool;
		this->staging_ = staging;
		this->usage_ = usage;
		this->mem_usage_ = mem_usage;

		if (!(usage & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			this->load_type_ = LoadType::STAGING;
		}
		else {
			this->load_type_ = LoadType::MEMCPY;
		}

		this->load_size_ = 0;

		staging->clear();
	}

	ErrStack Buffer::push(void* data, size_t size)
	{
		ErrStack err_stack;

		if (buff == VK_NULL_HANDLE) {
			checkErrStack1(create_(size, buff, buff_alloc, mem));
		}
		
		if (load_type_ == LoadType::STAGING) {

			checkErrStack1(staging_->push(data, size));
			load_size_ += size;
		}
		else {
			if (load_size_ + size > vma_r_info.size) {

				// Save old
				VkBuffer old_buffer = this->buff;
				VmaAllocation old_alloc = this->buff_alloc;
				void* old_mem = this->mem;

				VkBuffer new_buffer;
				VmaAllocation new_alloc;
				void* new_mem;

				// Create New
				checkErrStack1(create_(load_size_ + size, new_buffer, new_alloc, new_mem));

				// Copy old data to new buffer
				std::memcpy(new_mem, old_mem, this->load_size_);

				this->buff = new_buffer;
				this->buff_alloc = new_alloc;
				this->mem = new_mem;

				// Delete old
				vmaUnmapMemory(logical_dev_->allocator, old_alloc);
				vmaDestroyBuffer(logical_dev_->allocator, old_buffer, old_alloc);
			}

			void* dst = (uint8_t*)this->mem + this->load_size_;
			std::memcpy(dst, data, size);

			this->load_size_ += size;
		}
		return ErrStack();
	}

	ErrStack Buffer::flush()
	{
		ErrStack err_stack;

		if (load_type_ == LoadType::STAGING) {

			if (vma_r_info.size < this->load_size_) {

				vmaDestroyBuffer(logical_dev_->allocator, buff, buff_alloc);
				checkErrStack1(create_(this->load_size_, buff, buff_alloc, mem));
			}

			ErrStack err;
			{
				auto record = SingleCommandBuffer(logical_dev_, cmd_pool_, &err);

				VkBufferCopy regions = {};
				regions.size = load_size_;
				vkCmdCopyBuffer(record.cmd_buff, staging_->buff, buff, 1, &regions);
			}
		}

		load_size_ = 0;

		return ErrStack();
	}

	void Buffer::clear()
	{
		load_size_ = 0;
		staging_->clear();
	}

	void Buffer::destroy()
	{
		if (this->load_type_ == LoadType::MEMCPY) {
			vmaUnmapMemory(logical_dev_->allocator, this->buff_alloc);
		}

		vmaDestroyBuffer(logical_dev_->allocator, this->buff, this->buff_alloc);
		buff = VK_NULL_HANDLE;

		staging_->clear();
	}

	Buffer::~Buffer()
	{
		if (buff != VK_NULL_HANDLE) {
			destroy();
		}
	}

	ErrStack Buffer::setDebugName(std::string name)
	{
		return logical_dev_->setDebugName(
			reinterpret_cast<uint64_t>(buff), VK_OBJECT_TYPE_BUFFER, name + " VkBuffer");
	}

	ErrStack StagingBuffer::create_(size_t buff_size, 
		VkBuffer& new_buff, VmaAllocation& new_alloc, void*& new_mem)
	{
		VkBufferCreateInfo buff_info = {};
		buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buff_info.size = buff_size;
		buff_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buff_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vma_info = {};
		vma_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VkResult err = vmaCreateBuffer(logical_dev->allocator, &buff_info, &vma_info,
			&new_buff, &new_alloc, &vma_r_info);
		if (err != VK_SUCCESS) {
			return ErrStack(code_location, "failed to create staging buffer");
		}

		err = vmaMapMemory(logical_dev->allocator, new_alloc, &new_mem);
		if (err != VK_SUCCESS) {
			return ErrStack(code_location, "failed to create staging buffer");
		}

		return ErrStack();
	}

	ErrStack StagingBuffer::reserve(size_t size)
	{
		ErrStack err_stack;

		if (buff == VK_NULL_HANDLE) {
			checkErrStack1(create_(size, buff, buff_alloc, mem));
			load_size = 0;
		}
		else if (size > vma_r_info.size) {

			// Save old
			VkBuffer old_buffer = this->buff;
			VmaAllocation old_alloc = this->buff_alloc;
			void* old_mem = this->mem;

			this->buff = VK_NULL_HANDLE;
			this->buff_alloc = NULL;
			this->mem = nullptr;

			VkBuffer new_buffer = VK_NULL_HANDLE;
			VmaAllocation new_alloc = VK_NULL_HANDLE;
			void* new_mem = nullptr;

			// Create New
			checkErrStack1(create_(size, new_buffer, new_alloc, new_mem));

			// Copy old data to new buffer
			std::memcpy(new_mem, old_mem, this->load_size);

			this->buff = new_buffer;
			this->buff_alloc = new_alloc;
			this->mem = new_mem;

			// Delete old
			vmaUnmapMemory(logical_dev->allocator, old_alloc);
			vmaDestroyBuffer(logical_dev->allocator, old_buffer, old_alloc);
		}
		return ErrStack();
	}

	ErrStack StagingBuffer::push(void* data, size_t size)
	{
		ErrStack err_stack;

		if (buff == VK_NULL_HANDLE) {

			checkErrStack1(create_(size, buff, buff_alloc, mem));
		}
		else if (this->load_size + size > vma_r_info.size) {

			// Save old
			VkBuffer old_buffer = this->buff;
			VmaAllocation old_alloc = this->buff_alloc;
			void* old_mem = this->mem;

			VkBuffer new_buffer;
			VmaAllocation new_alloc;
			void* new_mem;

			// Create New
			checkErrStack1(create_(this->load_size + size, new_buffer, new_alloc, new_mem));

			// Copy old data to new
			std::memcpy(new_mem, old_mem, this->load_size);

			this->buff = new_buffer;
			this->buff_alloc = new_alloc;
			this->mem = new_mem;

			// Delete old
			vmaUnmapMemory(logical_dev->allocator, old_alloc);
			vmaDestroyBuffer(logical_dev->allocator, old_buffer, old_alloc);
		}

		void* dst = (uint8_t*)this->mem + this->load_size;
		std::memcpy(dst, data, size);

		this->load_size += size;

		return ErrStack();
	}

	void StagingBuffer::clear()
	{
		this->load_size = 0;
	}

	void StagingBuffer::destroy()
	{
		vmaUnmapMemory(logical_dev->allocator, this->buff_alloc);
		vmaDestroyBuffer(logical_dev->allocator, this->buff, this->buff_alloc);
		buff = VK_NULL_HANDLE;
		load_size = 0;
	}

	StagingBuffer::~StagingBuffer() 
	{
		if (buff != VK_NULL_HANDLE) {
			destroy();
		}
	}

	ErrStack StagingBuffer::setDebugName(std::string name)
	{
		return logical_dev->setDebugName(
			reinterpret_cast<uint64_t>(buff), VK_OBJECT_TYPE_BUFFER, name + " VkBuffer");
	}


	void cmdChangeImageLayout(VkCommandBuffer cmd_buff, VkImage img, VkImageLayout old_layout,
		VkImageLayout new_layout)
	{
		auto setUndefined = [](VkAccessFlags& access_mask, VkPipelineStageFlags& stage) {
			access_mask = 0;
			stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		};

		auto setTransferDst = [](VkAccessFlags& access_mask, VkPipelineStageFlags& stage) {
			access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
			stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		};

		auto setColorAttachWrite = [](VkAccessFlags& access_mask, VkPipelineStageFlags& stage) {
			access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		};

		auto setShaderReadFragment = [](VkAccessFlags& access_mask, VkPipelineStageFlags& stage) {
			access_mask = VK_ACCESS_SHADER_READ_BIT;
			stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		};

		VkPipelineStageFlags src_stage;
		VkPipelineStageFlags dst_stage;

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
			setUndefined(barrier.srcAccessMask, src_stage);

			if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				setTransferDst(barrier.dstAccessMask, dst_stage);
			}
			else if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				setColorAttachWrite(barrier.dstAccessMask, dst_stage);
			}
			else {
				printf("unsupported image layout transition");
			}
		}
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			setTransferDst(barrier.srcAccessMask, src_stage);

			if (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				setShaderReadFragment(barrier.dstAccessMask, dst_stage);
			}
			else if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				setColorAttachWrite(barrier.dstAccessMask, dst_stage);
			}
			else {
				printf("unsupported image layout transition");
			}
		}
		else {
			printf("unsupported image layout transition");
		}

		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = img;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(
			cmd_buff,
			src_stage, dst_stage,
			0,
			0, NULL,
			0, NULL,
			1, &barrier
		);
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

	ErrStack Image::recreate(LogicalDevice* logical_dev, VkImageCreateInfo* info, VmaMemoryUsage mem_usage)
	{
		VkResult vk_res{};

		if (this->img != VK_NULL_HANDLE) {
			this->destroy();
		}

		this->logical_dev = logical_dev;
		this->width = info->extent.width;
		this->height = info->extent.height;
		this->format = info->format;
		this->layout = info->initialLayout;
		this->mip_lvl = info->mipLevels;
		this->samples = info->samples;

		VmaAllocationCreateInfo alloc_create_info = {};
		alloc_create_info.usage = mem_usage;

		checkVkRes(vmaCreateImage(logical_dev->allocator, info, &alloc_create_info,
			&img, &alloc, &alloc_info),
			"failed to create image");

		VkMemoryPropertyFlags mem_flags;
		vmaGetMemoryTypeProperties(logical_dev->allocator, alloc_info.memoryType, &mem_flags);

		if (mem_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
			load_type = LoadType::MEMCPY;

			checkVkRes(vmaMapMemory(logical_dev->allocator, alloc, &mem), "");
		}
		else {
			load_type = LoadType::STAGING;

			this->mem = nullptr;
		}

		return ErrStack();
	}

	ErrStack Image::setDebugName(std::string name)
	{
		return logical_dev->setDebugName(
			reinterpret_cast<uint64_t>(img), VK_OBJECT_TYPE_IMAGE, name + " VkImage");
	}

	ErrStack Image::changeImageLayout(CommandPool* cmd_pool, VkImageLayout new_layout)
	{
		ErrStack err_stack;
		{
			auto record = SingleCommandBuffer(logical_dev, cmd_pool, &err_stack);
			checkErrStack(err_stack, "");

			cmdChangeImageLayout(record.cmd_buff, img,
				this->layout, new_layout);

			this->layout = new_layout;
		}

		return err_stack;
	}

	ErrStack Image::load(void* colors, size_t size, CommandPool* cmd_pool, Buffer* staging_buff,
		VkImageLayout layout_after_load)
	{
		ErrStack err_stack;

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
	}

	Image::~Image()
	{
		if (img != VK_NULL_HANDLE) {
			destroy();
		}
	}

	ErrStack ImageView::recreate(LogicalDevice* logical_dev, VkImageViewCreateInfo* info)
	{
		VkResult vk_res{};

		this->logical_dev = logical_dev;

		if (this->view != VK_NULL_HANDLE) {
			destroy();
		}

		checkVkRes(vkCreateImageView(logical_dev->logical_device, info, NULL, &view),
			"failed to create image view");

		return ErrStack();
	}

	ErrStack ImageView::setDebugName(std::string name)
	{
		return logical_dev->setDebugName(
			reinterpret_cast<uint64_t>(view), VK_OBJECT_TYPE_IMAGE, name + " VkImageView");
	}

	void ImageView::destroy()
	{
		vkDestroyImageView(logical_dev->logical_device, view, NULL);
		view = VK_NULL_HANDLE;
	}

	ImageView::~ImageView()
	{
		if (view != VK_NULL_HANDLE) {
			destroy();
		}
	}
}
