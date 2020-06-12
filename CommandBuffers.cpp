
// Standard
#include <execution>

// Header
#include "VulkanSystems.h"


ErrStack vks::RenderingComandBuffers::recreate(LogicalDevice* logical_dev, PhysicalDevice* phys_dev, uint32_t count)
{
	if (this->cmd_buff_tasks.size()) {
		destroy();
	}

	this->logical_dev = logical_dev;

	cmd_buff_tasks.resize((size_t)count);

	for (uint32_t i = 0; i < cmd_buff_tasks.size(); i++) {
		cmd_buff_tasks[i].idx = i;
	}

	std::atomic_bool is_err = false;

	std::for_each(std::execution::seq, cmd_buff_tasks.begin(), cmd_buff_tasks.end(),
		[this, &is_err, &phys_dev, &logical_dev](CmdBufferTask& task) {

		// Command Pool
		VkCommandPoolCreateInfo command_pool_info = {};
		command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		command_pool_info.queueFamilyIndex = phys_dev->queue_fam_idx;

		VkResult vk_res = vkCreateCommandPool(logical_dev->logical_device, &command_pool_info, NULL, &task.cmd_pool);
		if (vk_res != VK_SUCCESS) {
			task.err = ErrStack(code_location, "failed to create command pool");
			is_err.store(true);
		}

		// Command Buffer
		VkCommandBufferAllocateInfo command_buffer_info = {};
		command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_info.commandPool = task.cmd_pool;
		command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_info.commandBufferCount = 1;

		VkResult res = vkAllocateCommandBuffers(logical_dev->logical_device, &command_buffer_info, &task.cmd_buff);
		if (res != VK_SUCCESS) {
			task.err = ErrStack(res, code_location, "failed to allocate command buffers");
			is_err.store(true);
		}
	});

	if (is_err.load()) {
		for (auto& task : cmd_buff_tasks) {
			if (task.err.isBad()) {
				return task.err;
			}
		}
	}

	return ErrStack();
}

ErrStack vks::RenderingComandBuffers::update(const RenderingCmdBuffsUpdateInfo& info)
{
	std::atomic_bool is_err = false;

	std::for_each(std::execution::seq, cmd_buff_tasks.begin(), cmd_buff_tasks.end(), 
		[this, &is_err, &info](CmdBufferTask& task) {

		VkCommandBufferBeginInfo buffer_begin_info = {};
		buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;	

		VkResult vk_res = vkBeginCommandBuffer(task.cmd_buff, &buffer_begin_info);
		if (vk_res != VK_SUCCESS) {
			task.err = ErrStack(vk_res, code_location, "failed to begin command buffer");
			is_err.store(true);
			return;
		}

		// Clear Compose Image
		{
			cmdChangeImageLayout(task.cmd_buff, info.compose_img->img, 
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkImageMemoryBarrier barrier = {};
			VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

			VkClearColorValue clear = {};
			clear.float32[0] = 0;
			clear.float32[1] = 0;
			clear.float32[2] = 0;
			clear.float32[3] = 0;

			VkImageSubresourceRange range = {};
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseArrayLayer = 0;
			range.layerCount = 1;
			range.baseMipLevel = 0;
			range.levelCount = 1;

			vkCmdClearColorImage(task.cmd_buff, info.compose_img->img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1, &range);

			cmdChangeImageLayout(task.cmd_buff, info.compose_img->img,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}	

		std::array<VkClearValue, 1> clear_vals;
		clear_vals[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };

		for (GPU_ElementsLayer& layer : *info.layers) {

			// Border Rect Renderpass
			VkRenderPassBeginInfo border_rect_info = {};
			border_rect_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			border_rect_info.renderPass = info.rect_renderpass->renderpass;
			border_rect_info.framebuffer = info.border_rect_frames[0][task.idx].frame_buff;
			border_rect_info.renderArea.offset = { 0, 0 };
			border_rect_info.renderArea.extent.width = info.width;
			border_rect_info.renderArea.extent.height = info.height;
			border_rect_info.clearValueCount = (uint32_t)clear_vals.size();
			border_rect_info.pClearValues = clear_vals.data();

			vkCmdBeginRenderPass(task.cmd_buff, &border_rect_info, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
					info.rect_pipe_layout->pipe_layout, 0, 1, &info.uniform_descp_set->descp_set, 0, NULL);
				vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, info.rect_pipe->pipeline);

				VkBuffer vertex_buffers[] = { info.border_rect_vertex_buff->buff };
				VkDeviceSize offsets[] = { layer.border_rect.offset };
				vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffers, offsets);
				vkCmdDraw(task.cmd_buff, layer.border_rect.vertex_count, 1, 0, 0);
			}
			vkCmdEndRenderPass(task.cmd_buff);

			// Padding Rect Renderpass
			VkRenderPassBeginInfo padding_rect_info = {};
			padding_rect_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			padding_rect_info.renderPass = info.rect_renderpass->renderpass;
			padding_rect_info.framebuffer = info.padding_rect_frames[0][task.idx].frame_buff;
			padding_rect_info.renderArea.offset = { 0, 0 };
			padding_rect_info.renderArea.extent.width = info.width;
			padding_rect_info.renderArea.extent.height = info.height;
			padding_rect_info.clearValueCount = (uint32_t)clear_vals.size();
			padding_rect_info.pClearValues = clear_vals.data();

			vkCmdBeginRenderPass(task.cmd_buff, &padding_rect_info, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
					info.rect_pipe_layout->pipe_layout, 0, 1, &info.uniform_descp_set->descp_set, 0, NULL);
				vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, info.rect_pipe->pipeline);

				VkBuffer vertex_buffers[] = { info.padding_rect_vertex_buff->buff };
				VkDeviceSize offsets[] = { layer.padding_rect.offset };
				vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffers, offsets);
				vkCmdDraw(task.cmd_buff, layer.padding_rect.vertex_count, 1, 0, 0);
			}
			vkCmdEndRenderPass(task.cmd_buff);

			// Wait for the color attachment to writen then read in shader
			vkCmdPipelineBarrier(task.cmd_buff,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 0, NULL, 0, NULL, 0, NULL);

			// Border Circles Renderpass
			VkRenderPassBeginInfo border_circles_info = {};
			border_circles_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			border_circles_info.renderPass = info.circles_renderpass->renderpass;
			border_circles_info.framebuffer = info.border_circles_frames[0][task.idx].frame_buff;
			border_circles_info.renderArea.offset = { 0, 0 };
			border_circles_info.renderArea.extent.width = info.width;
			border_circles_info.renderArea.extent.height = info.height;
			border_circles_info.clearValueCount = (uint32_t)clear_vals.size();
			border_circles_info.pClearValues = clear_vals.data();

			vkCmdBeginRenderPass(task.cmd_buff, &border_circles_info, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
					info.circles_pipe_layout->pipe_layout, 0, 1, &info.uniform_descp_set->descp_set, 0, NULL);
				vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, info.circles_pipe->pipeline);

				VkBuffer vertex_buffers[] = { info.border_circles_vertex_buff->buff };
				VkDeviceSize offsets[] = { layer.border_circles.offset };
				vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffers, offsets);
				vkCmdDraw(task.cmd_buff, layer.border_circles.vertex_count, 1, 0, 0);
			}
			vkCmdEndRenderPass(task.cmd_buff);

			// Padding Circles Renderpass
			VkRenderPassBeginInfo padding_circles_info = {};
			padding_circles_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			padding_circles_info.renderPass = info.circles_renderpass->renderpass;
			padding_circles_info.framebuffer = info.padding_circles_frames[0][task.idx].frame_buff;
			padding_circles_info.renderArea.offset = { 0, 0 };
			padding_circles_info.renderArea.extent.width = info.width;
			padding_circles_info.renderArea.extent.height = info.height;
			padding_circles_info.clearValueCount = (uint32_t)clear_vals.size();
			padding_circles_info.pClearValues = clear_vals.data();

			vkCmdBeginRenderPass(task.cmd_buff, &padding_circles_info, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
					info.circles_pipe_layout->pipe_layout, 0, 1, &info.uniform_descp_set->descp_set, 0, NULL);
				vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, info.circles_pipe->pipeline);

				VkBuffer vertex_buffers[] = { info.padding_circles_vertex_buff->buff };
				VkDeviceSize offsets[] = { layer.padding_circles.offset };
				vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffers, offsets);
				vkCmdDraw(task.cmd_buff, layer.padding_circles.vertex_count, 1, 0, 0);
			}
			vkCmdEndRenderPass(task.cmd_buff);

			// Wait for the border and padding images to be written then read in fragment shader
			vkCmdPipelineBarrier(task.cmd_buff,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 0, NULL, 0, NULL, 0, NULL);

			// Compose Renderpass
			VkRenderPassBeginInfo compose_info = {};
			compose_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			compose_info.renderPass = info.compose_renderpass->renderpass;
			compose_info.framebuffer = info.compose_frames[0][task.idx].frame_buff;
			compose_info.renderArea.offset = { 0, 0 };
			compose_info.renderArea.extent.width = info.width;
			compose_info.renderArea.extent.height = info.height;

			std::array<VkClearValue, 3> compose_clears;
			compose_clears[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
			compose_clears[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
			compose_clears[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
			compose_info.clearValueCount = compose_clears.size();
			compose_info.pClearValues = compose_clears.data();

			vkCmdBeginRenderPass(task.cmd_buff, &compose_info, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
					info.compose_pipe_layout->pipe_layout, 0, 1, &info.compose_descp_set->descp_set, 0, NULL);
				vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, info.compose_pipe->pipeline);

				vkCmdDraw(task.cmd_buff, 6, 1, 0, 0);
			}
			vkCmdEndRenderPass(task.cmd_buff);
		}

		// Wait for the compose image to be written by all layers then copy to swapchain
		vkCmdPipelineBarrier(task.cmd_buff,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, NULL, 0, NULL, 0, NULL);

		// Copy Renderpass
		VkRenderPassBeginInfo copy_info = {};
		copy_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		copy_info.renderPass = info.copy_renderpass->renderpass;
		copy_info.framebuffer = info.copy_frames[0][task.idx].frame_buff;
		copy_info.renderArea.offset = { 0, 0 };
		copy_info.renderArea.extent.width = info.width;
		copy_info.renderArea.extent.height = info.height;

		std::array<VkClearValue, 2> copy_clears;
		copy_clears[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		copy_clears[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		copy_info.clearValueCount = copy_clears.size();
		copy_info.pClearValues = copy_clears.data();

		vkCmdBeginRenderPass(task.cmd_buff, &copy_info, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
				info.copy_pipe_layout->pipe_layout, 0, 1, &info.copy_descp_set->descp_set, 0, NULL);
			vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, info.copy_pipe->pipeline);

			vkCmdDraw(task.cmd_buff, 6, 1, 0, 0);
		}
		vkCmdEndRenderPass(task.cmd_buff);

		vk_res = vkEndCommandBuffer(task.cmd_buff);
		if (vk_res != VK_SUCCESS) {
			task.err = ErrStack(vk_res, code_location, "failed to end command buffer");
			is_err.store(true);
			return;
		}
	});

	if (is_err.load()) {
		for (CmdBufferTask& task : cmd_buff_tasks) {
			if (task.err.isBad()) {
				return task.err;
			}
		}
	}
	
	return ErrStack();
}

void vks::RenderingComandBuffers::destroy()
{
	std::for_each(std::execution::par, cmd_buff_tasks.begin(), cmd_buff_tasks.end(), 
		[this](CmdBufferTask& task) {

		vkFreeCommandBuffers(logical_dev->logical_device, task.cmd_pool, 1, &task.cmd_buff);
		vkDestroyCommandPool(logical_dev->logical_device, task.cmd_pool, NULL);
	});
	cmd_buff_tasks.clear();
}

vks::RenderingComandBuffers::~RenderingComandBuffers()
{
	if (cmd_buff_tasks.size()) {
		destroy();
	}
}

ErrStack vks::Fence::create(LogicalDevice* logical_dev, VkFenceCreateFlags flags)
{
	this->logical_dev = logical_dev;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = flags;

	checkVkRes(vkCreateFence(logical_dev->logical_device, &fence_info, NULL, &fence), 
		"failed to create fence");
	return ErrStack();
}

ErrStack vks::Fence::waitAndReset(uint64_t max_wait_time)
{
	checkVkRes(vkWaitForFences(logical_dev->logical_device, 1, &fence, VK_TRUE, UINT64_MAX),
		"failed to wait on fence");

	checkVkRes(vkResetFences(logical_dev->logical_device, 1, &fence),
		"failed to reset fence");
	
	return ErrStack();
}

void vks::Fence::destroy()
{
	vkDestroyFence(logical_dev->logical_device, fence, NULL);
	fence = VK_NULL_HANDLE;
}

vks::Fence::~Fence()
{
	if (fence != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack vks::Semaphore::recreate(LogicalDevice* logical_dev)
{
	this->logical_dev = logical_dev;

	if (this->semaphore != VK_NULL_HANDLE) {
		destroy();
	}

	VkSemaphoreCreateInfo sem_info = {};
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	checkVkRes(vkCreateSemaphore(logical_dev->logical_device, &sem_info, NULL, &semaphore),
		"failed to create semaphore");
	return ErrStack();
}

void vks::Semaphore::destroy()
{
	vkDestroySemaphore(logical_dev->logical_device, semaphore, NULL);
	semaphore = VK_NULL_HANDLE;
}

vks::Semaphore::~Semaphore()
{
	if (semaphore != VK_NULL_HANDLE) {
		destroy();
	}
}
