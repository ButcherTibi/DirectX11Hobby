
// Standard
#include <execution>

// Header
#include "VulkanSystems.h"


ErrorStack vks::RenderingComandBuffers::create(LogicalDevice* logical_dev, PhysicalDevice* phys_dev, uint32_t count)
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
			task.err = ErrorStack(code_location, "failed to create command pool");
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
			task.err = ErrorStack(res, code_location, "failed to allocate command buffers");
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

	return ErrorStack();
}

ErrorStack vks::RenderingComandBuffers::update(Renderpass* renderpass, Framebuffers* frame_buffs,
	uint32_t width, uint32_t height, PipelineLayout* pipe_layout, GraphicsPipeline* graphics_pipe,
	DescriptorSet* descp_set, Buffer* vertex_buff, uint32_t vertex_count)
{
	std::atomic_bool is_err = false;

	std::for_each(std::execution::par, cmd_buff_tasks.begin(), cmd_buff_tasks.end(), 
		[this, &is_err, renderpass, frame_buffs, width, height,
		pipe_layout, graphics_pipe, descp_set, vertex_buff, vertex_count](CmdBufferTask& task) {

		VkCommandBufferBeginInfo buffer_begin_info = {};
		buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkResult vk_res = vkBeginCommandBuffer(task.cmd_buff, &buffer_begin_info);
		if (vk_res != VK_SUCCESS) {
			task.err = ErrorStack(vk_res, code_location, "failed to begin command buffer");
			is_err.store(true);
			return;
		}

		VkRenderPassBeginInfo renderpass_begin_info = {};
		renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_begin_info.renderPass = renderpass->renderpass;
		renderpass_begin_info.framebuffer = frame_buffs->frame_buffs[task.idx];
		renderpass_begin_info.renderArea.offset = { 0, 0 };
		renderpass_begin_info.renderArea.extent.width = width;
		renderpass_begin_info.renderArea.extent.height = height;

		// Clear Values
		clear_vals.resize(2);
		clear_vals[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clear_vals[1].depthStencil = { 1.0f, 0 };
		renderpass_begin_info.clearValueCount = (uint32_t)clear_vals.size();
		renderpass_begin_info.pClearValues = clear_vals.data();

		vkCmdBeginRenderPass(task.cmd_buff, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout->pipe_layout, 0,
				1, &descp_set->descp_set, 0, NULL);

			vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipe->pipeline);  // must go before buffers

			VkBuffer vertex_buffers[] = { vertex_buff->buff };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffers, offsets);

			vkCmdDraw(task.cmd_buff, vertex_count, 1, 0, 0);
		}
		vkCmdEndRenderPass(task.cmd_buff);

		vk_res = vkEndCommandBuffer(task.cmd_buff);
		if (vk_res != VK_SUCCESS) {
			task.err = ErrorStack(vk_res, code_location, "failed to end command buffer");
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
	
	return ErrorStack();
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

ErrorStack vks::Fence::create(LogicalDevice* logical_dev, VkFenceCreateFlags flags)
{
	this->logical_dev = logical_dev;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = flags;

	checkVkRes(vkCreateFence(logical_dev->logical_device, &fence_info, NULL, &fence), 
		"failed to create fence");
	return ErrorStack();
}

ErrorStack vks::Fence::waitAndReset(uint64_t max_wait_time)
{
	checkVkRes(vkWaitForFences(logical_dev->logical_device, 1, &fence, VK_TRUE, UINT64_MAX),
		"failed to wait on fence");

	checkVkRes(vkResetFences(logical_dev->logical_device, 1, &fence),
		"failed to reset fence");
	
	return ErrorStack();
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

ErrorStack vks::Semaphore::create(LogicalDevice* logical_dev)
{
	this->logical_dev = logical_dev;

	VkSemaphoreCreateInfo sem_info = {};
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	checkVkRes(vkCreateSemaphore(logical_dev->logical_device, &sem_info, NULL, &semaphore),
		"failed to create semaphore");
	return ErrorStack();
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
