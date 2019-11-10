
// Standard
#include <execution>

// Other
#include "VulkanContext.h"
#include "VulkanFrames.h"
#include "VulkanCommandPool.h"
#include "VulkanPipeline.h"
#include "VulkanBuffers.h"

#include "VulkanCommandBuffers.h"


ErrorStack ComandBuffers::build()
{
	printf("ComandBuffers build \n");

	cmd_buff_tasks.resize(frame_buffs->frame_buffers.size());

	for (uint32_t i = 0; i < cmd_buff_tasks.size(); i++) {
		cmd_buff_tasks[i].idx = i;
	}

	std::atomic_bool is_err = false;

	std::for_each(std::execution::par, cmd_buff_tasks.begin(), cmd_buff_tasks.end(), [this, &is_err](CmdBufferTask& task) {

		// Command Pool
		VkCommandPoolCreateInfo command_pool_info = {};
		command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_info.queueFamilyIndex = dev->queue_fam_idx;

		VkResult vk_res = vkCreateCommandPool(dev->logical_device, &command_pool_info, NULL, &task.cmd_pool);
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

		VkResult res = vkAllocateCommandBuffers(dev->logical_device, &command_buffer_info, &task.cmd_buff);
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

void ComandBuffers::init(Device* device,
	Swapchain* swapchain, Renderpass* renderpass,
	Buffer* vertex_buff, Buffer* index_buff,
	FrameBuffers* frame_buffers, DescriptorSets* descp_sets,
	GraphicsPipeline* pipeline)
{
	printf("ComandBuffers init \n");

	this->dev = device;

	this->swapchain = swapchain;
	this->renderpass = renderpass;	

	this->vertex_buff = vertex_buff;
	this->index_buff = index_buff;

	this->frame_buffs = frame_buffers;
	this->descp_sets = descp_sets;

	this->pipeline = pipeline;
}

ErrorStack ComandBuffers::updateRenderComands()
{
	printf("ComandBuffers updateRenderComands \n");

	std::atomic_bool is_err = false;

	std::for_each(std::execution::par, cmd_buff_tasks.begin(), cmd_buff_tasks.end(), [this, &is_err](CmdBufferTask& task) {

		VkCommandBufferBeginInfo buffer_begin_info = {};
		buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkResult vk_res = vkBeginCommandBuffer(task.cmd_buff, &buffer_begin_info);
		if (vk_res != VK_SUCCESS) {
			task.err = ErrorStack(vk_res, ExtraError::FAILED_TO_CREATE_COMAND_POOL, code_location, "failed to create command pool");
			is_err.store(true);
			return;
		}

		VkRenderPassBeginInfo renderpass_begin_info = {};
		renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_begin_info.renderPass = renderpass->renderpass;
		renderpass_begin_info.framebuffer = frame_buffs->frame_buffers[task.idx];
		renderpass_begin_info.renderArea.offset = { 0, 0 };
		renderpass_begin_info.renderArea.extent = swapchain->resolution;

		VkClearValue clear_value = { 0, 0, 0, 1 };
		renderpass_begin_info.clearValueCount = 1;
		renderpass_begin_info.pClearValues = &clear_value;

		vkCmdBeginRenderPass(task.cmd_buff, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline_layout, 0,
				1, &this->descp_sets->descp_set, 0, NULL);

			vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);  // must go before buffers

			VkBuffer vertex_buffers[] = { this->vertex_buff->buff };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffers, offsets);

			vkCmdBindIndexBuffer(task.cmd_buff, this->index_buff->buff, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(task.cmd_buff, (uint32_t)index_buff->buff_size / sizeof(uint32_t), 1, 0, 0, 0);
		}
		vkCmdEndRenderPass(task.cmd_buff);

		vk_res = vkEndCommandBuffer(task.cmd_buff);
		if (vk_res != VK_SUCCESS) {
			task.err = ErrorStack(vk_res, code_location, "failed to record command buffer");
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

void ComandBuffers::destroy()
{
	printf("ComandBuffers destroy \n");

	std::for_each(std::execution::par, cmd_buff_tasks.begin(), cmd_buff_tasks.end(), [this](CmdBufferTask& task) {

		vkFreeCommandBuffers(dev->logical_device, task.cmd_pool, 1, &task.cmd_buff);
		vkDestroyCommandPool(dev->logical_device, task.cmd_pool, NULL);
		task.cmd_pool = VK_NULL_HANDLE;
	});
}

ComandBuffers::~ComandBuffers()
{
	if (dev != nullptr) {
		destroy();
	}
}

void LoadComandBuffer::init(Device* device)
{
	this->dev = device;
}

ErrorStack LoadComandBuffer::build()
{
	printf("LoadComandBuffer build \n");

	ErrorStack err;
	
	VkCommandPoolCreateInfo command_pool_info = {};
	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_info.queueFamilyIndex = dev->queue_fam_idx;

	VkResult vk_res = vkCreateCommandPool(dev->logical_device, &command_pool_info, NULL, &cmd_pool);
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, ExtraError::FAILED_TO_CREATE_COMAND_POOL, code_location, "failed to create command pool");
	}

	VkCommandBufferAllocateInfo command_buffer_info = {};
	command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_info.commandPool = cmd_pool;
	command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_info.commandBufferCount = 1;

	VkResult res = vkAllocateCommandBuffers(dev->logical_device, &command_buffer_info, &cmd_buff);
	if (res != VK_SUCCESS) {
		return ErrorStack(res, ExtraError::FAILED_TO_CREATE_COMAND_BUFFER, code_location, "failed to allocate command buffers");
	}

	return ErrorStack();
}

void LoadComandBuffer::destroy()
{
	printf("LoadComandBuffer destroy \n");

	vkDestroyCommandPool(dev->logical_device, cmd_pool, NULL);
	cmd_pool = VK_NULL_HANDLE;
}

LoadComandBuffer::~LoadComandBuffer()
{
	if (dev != nullptr) {
		destroy();
	}
}

RecordCmdBuffer::RecordCmdBuffer(LoadComandBuffer* load_cmd_buff, VkCommandBufferUsageFlags flags, ErrorStack* r_err)
{
	this->load_cmd_buff = load_cmd_buff;
	this->err = r_err;

	VkCommandBufferBeginInfo comm_buff_begin_info = {};
	comm_buff_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	comm_buff_begin_info.flags = flags;

	VkResult vk_err = vkBeginCommandBuffer(this->load_cmd_buff->cmd_buff, &comm_buff_begin_info);
	if (vk_err != VK_SUCCESS) {
		*err = ErrorStack(vk_err, ExtraError::FAILED_TO_BEGIN_RECORDING_CMD_BUFF, code_location,
			"failed to begin recording command buffer");
	}
	*err = ErrorStack();
}

RecordCmdBuffer::~RecordCmdBuffer()
{
	if (!err->isBad()) {
		VkResult vk_err = vkEndCommandBuffer(load_cmd_buff->cmd_buff);
		if (vk_err != VK_SUCCESS) {
			*err = ErrorStack(vk_err, ExtraError::FAILED_TO_END_RECORDING_CMD_BUFF, code_location, "failed to end recording command buffer");
		}
		*err = ErrorStack();
	}	
}