
// Standard
#include <execution>

// Header
#include "VulkanSystems.h"


ErrStack vks::RenderingComandBuffers::create(LogicalDevice* logical_dev, PhysicalDevice* phys_dev, uint32_t count)
{
	this->logical_dev = logical_dev;

	cmd_buff_tasks.resize((size_t)count);

	for (uint32_t i = 0; i < cmd_buff_tasks.size(); i++) {
		cmd_buff_tasks[i].idx = i;
	}

	std::atomic_bool is_err = false;

	std::for_each(std::execution::par, cmd_buff_tasks.begin(), cmd_buff_tasks.end(), 
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

	std::for_each(std::execution::par, cmd_buff_tasks.begin(), cmd_buff_tasks.end(), 
		[this, &is_err, info](CmdBufferTask& task) {

		VkCommandBufferBeginInfo buffer_begin_info = {};
		buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkResult vk_res = vkBeginCommandBuffer(task.cmd_buff, &buffer_begin_info);
		if (vk_res != VK_SUCCESS) {
			task.err = ErrStack(vk_res, code_location, "failed to begin command buffer");
			is_err.store(true);
			return;
		}

		VkRenderPassBeginInfo renderpass_begin_info = {};
		renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_begin_info.renderPass = info.renderpass->renderpass;
		renderpass_begin_info.framebuffer = info.frame_buffs->frame_buffs[task.idx];
		renderpass_begin_info.renderArea.offset = { 0, 0 };
		renderpass_begin_info.renderArea.extent.width = info.width;
		renderpass_begin_info.renderArea.extent.height = info.height;

		// Clear Values
		clear_vals.resize(4);	
		clear_vals[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clear_vals[1].depthStencil.depth = 1;
		clear_vals[2].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clear_vals[3].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderpass_begin_info.clearValueCount = (uint32_t)clear_vals.size();
		renderpass_begin_info.pClearValues = clear_vals.data();

		vkCmdBeginRenderPass(task.cmd_buff, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		{
			// 3D Subpass
			{			
				vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, 
					info.g3d_pipe_layout->pipe_layout, 0, 1, &info.g3d_descp->descp_set, 0, NULL);

				vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, info.g3d_pipe->pipeline);  // must go before buffers

				VkBuffer vertex_buffers[] = { info.g3d_vertex_buff->buff };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffers, offsets);

				vkCmdDraw(task.cmd_buff, info.g3d_vertex_count, 1, 0, 0);
			}

			vkCmdNextSubpass(task.cmd_buff, VK_SUBPASS_CONTENTS_INLINE);

			// UI Subpass
			{
				vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
					info.ui_pipe_layout->pipe_layout, 0, 1, &info.ui_descp->descp_set, 0, NULL);

				vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, info.ui_pipe->pipeline);

				VkBuffer vertex_buffs[] = { info.ui_vertex_buff->buff };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffs, offsets);

				vkCmdDraw(task.cmd_buff, info.ui_vertex_count, 1, 0, 0);
			}
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

ErrStack vks::Semaphore::create(LogicalDevice* logical_dev)
{
	this->logical_dev = logical_dev;

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
