
#include "pch.h"

// Header
#include "VulkanWrapper.hpp"


using namespace nui;
using namespace vkw;


ErrStack CommandList::beginRecording()
{
	VkResult vk_res;

	for (auto& task : tasks) {

		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		checkVkRes(vkBeginCommandBuffer(task.buff, &info),
			"failed to begin command buffer");
	}

	return ErrStack();
}

void CommandList::cmdCopyBuffer(Buffer& src_buff, Buffer& dst_buff, size_t copy_size)
{
	assert_cond(copy_size <= dst_buff.size, "");

	for (auto& task : tasks) {

		VkBufferCopy copy = {};
		copy.size = copy_size;
		copy.srcOffset = 0;
		copy.dstOffset = 0;

		vkCmdCopyBuffer(task.buff, src_buff.buff, dst_buff.buff, 1, &copy);
	}
}

void CommandList::cmdCopyBufferToImage(Buffer& src_buff, ImageView& dst_view)
{
	for (auto& task : tasks) {

		ImageSubResourceRange& subres = dst_view.info.subres_range;

		VkBufferImageCopy copy;
		copy.bufferOffset = 0;
		copy.bufferRowLength = 0;
		copy.bufferImageHeight = 0;
		copy.imageSubresource.aspectMask = subres.aspectMask;
		copy.imageSubresource.mipLevel = subres.baseMipLevel;
		copy.imageSubresource.baseArrayLayer = subres.baseArrayLayer;
		copy.imageSubresource.layerCount = subres.layerCount;
		copy.imageOffset.x = 0;
		copy.imageOffset.y = 0;
		copy.imageOffset.z = 0;
		copy.imageExtent.width = dst_view.image->width;
		copy.imageExtent.height = dst_view.image->height;
		copy.imageExtent.depth = dst_view.image->depth;

		vkCmdCopyBufferToImage(task.buff, src_buff.buff,
			dst_view.image->img, dst_view.image->current_layout, 1, &copy);
	}
}

void CommandList::cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
	size_t image_barriers_count, ImageBarrier* image_barriers)
{
	std::vector<VkImageMemoryBarrier> vk_barriers(image_barriers_count);
	for (uint32_t i = 0; i < vk_barriers.size(); i++) {

		VkImageMemoryBarrier& vk_bar = vk_barriers[i];
		ImageBarrier& bar = image_barriers[i];

		vk_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_bar.srcAccessMask = bar.wait_for_access;
		vk_bar.dstAccessMask = bar.wait_at_access;
		vk_bar.oldLayout = bar.view->image->current_layout;

		if (bar.new_layout == VK_IMAGE_LAYOUT_MAX_ENUM) {
			vk_bar.newLayout = bar.view->image->current_layout;
		}
		else {
			vk_bar.newLayout = bar.new_layout;
		}

		vk_bar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vk_bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vk_bar.image = bar.view->image->img;

		auto& subres = bar.view->info.subres_range;
		vk_bar.subresourceRange.aspectMask = subres.aspectMask;
		vk_bar.subresourceRange.baseMipLevel = subres.baseMipLevel;
		vk_bar.subresourceRange.levelCount = subres.levelCount;
		vk_bar.subresourceRange.baseArrayLayer = subres.baseArrayLayer;
		vk_bar.subresourceRange.layerCount = subres.layerCount;

		// Remember the layout change
		bar.view->image->current_layout = bar.new_layout;
	}

	for (auto& task : tasks) {

		vkCmdPipelineBarrier(task.buff, wait_for_stage, wait_at_stage, 0,
			0, NULL,  // Memory Barriers
			0, NULL,  // Buffer Memory Barrier
			(uint32_t)vk_barriers.size(), vk_barriers.data()  // Image Memory Barrier
		);
	}
}

void CommandList::cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
	std::vector<ImageBarrier>& barriers)
{
	this->cmdPipelineBarrier(wait_for_stage, wait_at_stage, barriers.size(), barriers.data());
}

void CommandList::cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
	ImageBarrier& image_barrier)
{
	this->cmdPipelineBarrier(wait_for_stage, wait_at_stage, 1, &image_barrier);
}

void CommandList::cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
	size_t image_barriers_count, CustomImageBarrier* image_barriers)
{
	std::vector<VkImageMemoryBarrier> vk_barriers(image_barriers_count);
	for (uint32_t i = 0; i < vk_barriers.size(); i++) {

		VkImageMemoryBarrier& vk_bar = vk_barriers[i];
		CustomImageBarrier& bar = image_barriers[i];

		vk_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_bar.srcAccessMask = bar.wait_for_access;
		vk_bar.dstAccessMask = bar.wait_at_access;
		vk_bar.oldLayout = bar.img->current_layout;

		if (bar.new_layout == VK_IMAGE_LAYOUT_MAX_ENUM) {
			vk_bar.newLayout = bar.img->current_layout;
		}
		else {
			vk_bar.newLayout = bar.new_layout;
		}

		vk_bar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vk_bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vk_bar.image = bar.img->img;
		vk_bar.subresourceRange.aspectMask = bar.aspectMask;
		vk_bar.subresourceRange.baseMipLevel = bar.baseMipLevel;
		vk_bar.subresourceRange.levelCount = bar.levelCount;
		vk_bar.subresourceRange.baseArrayLayer = bar.baseArrayLayer;
		vk_bar.subresourceRange.layerCount = bar.layerCount;

		// Remember the layout change
		bar.img->current_layout = bar.new_layout;
	}

	for (auto& task : tasks) {

		vkCmdPipelineBarrier(task.buff, wait_for_stage, wait_at_stage, 0,
			0, NULL,  // Memory Barriers
			0, NULL,  // Buffer Memory Barrier
			(uint32_t)vk_barriers.size(), vk_barriers.data()  // Image Memory Barrier
		);
	}
}

void CommandList::cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
	std::vector<CustomImageBarrier>& image_barriers)
{
	cmdPipelineBarrier(wait_for_stage, wait_at_stage, image_barriers.size(), image_barriers.data());
}

void CommandList::cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
	CustomImageBarrier& image_barrier)
{
	cmdPipelineBarrier(wait_for_stage, wait_at_stage, 1, &image_barrier);
}

void CommandList::cmdSurfacePipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
	SurfaceBarrier& barrier)
{
	for (uint32_t i = 0; i < tasks.size(); i++) {

		CommandTask& task = tasks[i];

		// Transition surface images to TRANSFER DESTINATION
		VkImageMemoryBarrier bar = {};
		bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		bar.srcAccessMask = barrier.wait_for_access;
		bar.dstAccessMask = barrier.wait_at_access;
		bar.oldLayout = surface->swapchain_img_layout;
		bar.newLayout = barrier.new_layout;
		bar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bar.image = surface->swapchain_images[i];
		bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bar.subresourceRange.baseMipLevel = 0;
		bar.subresourceRange.levelCount = 1;
		bar.subresourceRange.baseArrayLayer = 0;
		bar.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(task.buff,
			wait_for_stage, wait_at_stage,
			0,
			0, NULL,
			0, NULL,
			1, &bar);

		surface->swapchain_img_layout = barrier.new_layout;
	}
}

void CommandList::cmdClearFloatImage(ImageView& view, float r, float g, float b, float a)
{
	for (auto& task : tasks) {

		VkClearColorValue clear_color;
		clear_color.float32[0] = r;
		clear_color.float32[1] = g;
		clear_color.float32[2] = b;
		clear_color.float32[3] = a;

		VkImageSubresourceRange vk_subres = {};
		auto& subres = view.info.subres_range;

		vk_subres.aspectMask = subres.aspectMask;
		vk_subres.baseMipLevel = subres.baseMipLevel;
		vk_subres.levelCount = subres.levelCount;
		vk_subres.baseArrayLayer = subres.baseArrayLayer;
		vk_subres.layerCount = subres.layerCount;

		vkCmdClearColorImage(task.buff, view.image->img, view.image->current_layout,
			&clear_color, 1, &vk_subres);
	}
}

void CommandList::cmdClearUIntImage(ImageView& view, uint32_t rgba)
{
	for (auto& task : tasks) {

		VkClearColorValue clear_color;
		clear_color.uint32[0] = (rgba & 0xFF'00'00'00) >> 24;
		clear_color.uint32[1] = (rgba & 0x00'FF'00'00) >> 16;
		clear_color.uint32[2] = (rgba & 0x00'00'FF'00) >> 8;
		clear_color.uint32[3] = (rgba & 0x00'00'00'FF);

		VkImageSubresourceRange vk_subres = {};
		auto& subres = view.info.subres_range;

		vk_subres.aspectMask = subres.aspectMask;
		vk_subres.baseMipLevel = subres.baseMipLevel;
		vk_subres.levelCount = subres.levelCount;
		vk_subres.baseArrayLayer = subres.baseArrayLayer;
		vk_subres.layerCount = subres.layerCount;

		vkCmdClearColorImage(task.buff, view.image->img, view.image->current_layout,
			&clear_color, 1, &vk_subres);
	}
}


void CommandList::cmdClearSurface(float r, float g, float b, float a)
{
	for (uint32_t i = 0; i < tasks.size(); i++) {

		VkClearColorValue clear_color;
		clear_color.float32[0] = r;
		clear_color.float32[1] = g;
		clear_color.float32[2] = b;
		clear_color.float32[3] = a;

		VkImageSubresourceRange vk_subres = {};
		vk_subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vk_subres.baseMipLevel = 0;
		vk_subres.levelCount = 1;
		vk_subres.baseArrayLayer = 0;
		vk_subres.layerCount = 1;

		vkCmdClearColorImage(tasks[i].buff, surface->swapchain_images[i], surface->swapchain_img_layout,
			&clear_color, 1, &vk_subres);
	}
}

void CommandList::cmdCopyImageToSurface(ImageView& view)
{
	for (uint32_t i = 0; i < tasks.size(); i++) {

		// Now we can finally copy
		VkImageSubresourceLayers subres = {};
		subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subres.mipLevel = 0;
		subres.baseArrayLayer = 0;
		subres.layerCount = 1;

		VkImageCopy cpy = {};
		cpy.srcSubresource = subres;
		cpy.srcOffset = {0, 0, 0};
		cpy.dstSubresource = subres;
		cpy.dstOffset = { 0, 0, 0 };
		cpy.extent.width = surface->width;
		cpy.extent.height = surface->height;
		cpy.extent.depth = 1;

		vkCmdCopyImage(tasks[i].buff,
			view.image->img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			surface->swapchain_images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &cpy);
	}
}

void CommandList::cmdBeginRenderpass(Drawpass& pass)
{
	for (auto& task : tasks) {

		VkRenderPassBeginInfo vk_info = {};
		vk_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vk_info.renderPass = pass.renderpass;
		vk_info.framebuffer = pass.framebuffs[task.idx];

		auto& scissors = pass.viewport_state.scissors;
		if (scissors.size()) {
			vk_info.renderArea.offset = scissors[0].offset;
			vk_info.renderArea.extent = scissors[0].extent;
		}
		else {
			vk_info.renderArea.offset = { 0, 0 };
			vk_info.renderArea.extent.width = surface->width;
			vk_info.renderArea.extent.height = surface->height;
		}

		vk_info.clearValueCount = (uint32_t)pass.clear_values.size();
		vk_info.pClearValues = pass.clear_values.data();

		vkCmdBeginRenderPass(task.buff, &vk_info, VK_SUBPASS_CONTENTS_INLINE);

		if (pass.descp_sets.size()) {
			vkCmdBindDescriptorSets(task.buff, VK_PIPELINE_BIND_POINT_GRAPHICS, pass.pipe_layout,
				0, (uint32_t)pass.descp_sets.size(), pass.descp_sets.data(), 0, NULL);
		}

		vkCmdBindPipeline(task.buff, VK_PIPELINE_BIND_POINT_GRAPHICS, pass.pipeline);
	}
}

void CommandList::cmdEndRenderpass()
{
	for (auto& task : tasks) {

		vkCmdEndRenderPass(task.buff);
	}
}

void CommandList::cmdBindVertexBuffer(Buffer& vertex_buff)
{
	for (auto& task : tasks) {

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(task.buff, 0, 1, &vertex_buff.buff, &offset);
	}
}

void CommandList::cmdBindVertexBuffers(Buffer& vertex_buff_0, Buffer& vertex_buff_1)
{
	for (auto& task : tasks) {

		std::array<VkDeviceSize, 2> offsets = {
			0, 0
		};

		std::array<VkBuffer, 2> buffs = {
			vertex_buff_0.buff, vertex_buff_1.buff
		};

		vkCmdBindVertexBuffers(task.buff, 0, (uint32_t)buffs.size(), buffs.data(), offsets.data());
	}
}

void CommandList::cmdBindIndexBuffer(Buffer& index_buff, VkIndexType index_type)
{
	for (auto& task : tasks) {

		vkCmdBindIndexBuffer(task.buff, index_buff.buff, 0, index_type);
	}
}

void CommandList::cmdDraw(uint32_t vertex_count, uint32_t start_idx)
{
	for (auto& task : tasks) {

		vkCmdDraw(task.buff, vertex_count, 1, start_idx, 0);
	}
}

void CommandList::cmdDrawIndexedInstanced(uint32_t index_count, uint32_t instance_count,
	uint32_t first_index, uint32_t first_instance)
{
	for (auto& task : tasks) {

		vkCmdDrawIndexed(task.buff, index_count, instance_count, first_index, 0, first_instance);
	}
}

ErrStack CommandList::endRecording()
{
	VkResult vk_res;

	for (auto& task : tasks) {

		checkVkRes(vkEndCommandBuffer(task.buff),
			"failed to end command buffer");
	}

	return ErrStack();
}

nui::ErrStack CommandList::run()
{
	VkResult vk_res{};

	if (surface != nullptr) {
		
		uint32_t image_index;
		
		checkVkRes(vkAcquireNextImageKHR(dev->logical_dev, surface->swapchain, UINT64_MAX,
			swapchain_img_acquired.semaphore, VK_NULL_HANDLE, &image_index), 
			"failed to acquire next image in swapchain");
		
		// Render
		VkSubmitInfo queue_submit_info = {};
		queue_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		queue_submit_info.waitSemaphoreCount = 1;
		queue_submit_info.pWaitSemaphores = &swapchain_img_acquired.semaphore;

		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		queue_submit_info.pWaitDstStageMask = wait_stages;
		queue_submit_info.commandBufferCount = 1;
		queue_submit_info.pCommandBuffers = &tasks[image_index].buff;
		queue_submit_info.signalSemaphoreCount = 1;
		queue_submit_info.pSignalSemaphores = &execution_finished.semaphore;
		
		checkVkRes(vkQueueSubmit(dev->queue, 1, &queue_submit_info, execution_finished_fence.fence),
			"failed to submit draw commands to gpu queue");
		
		// Present image	
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &execution_finished.semaphore;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &surface->swapchain;
		present_info.pImageIndices = &image_index;
		present_info.pResults = NULL;
		
		checkVkRes(vkQueuePresentKHR(dev->queue, &present_info),
			"failed to present image");
	}
	else {
		VkSubmitInfo queue_submit_info = {};
		queue_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		queue_submit_info.pWaitDstStageMask = wait_stages;
		queue_submit_info.commandBufferCount = 1;
		queue_submit_info.pCommandBuffers = &tasks[0].buff;

		checkVkRes(vkQueueSubmit(dev->queue, 1, &queue_submit_info, execution_finished_fence.fence),
			"failed to submit draw commands to gpu queue");
	}

	return ErrStack();
}

nui::ErrStack CommandList::waitForExecution(uint64_t timeout)
{
	VkResult vk_res{};

	checkVkRes(vkWaitForFences(dev->logical_dev, 1, &execution_finished_fence.fence, VK_TRUE, timeout),
		"failed to wait for queue submit");

	checkVkRes(vkResetFences(dev->logical_dev, 1, &execution_finished_fence.fence),
		"failed to reset fence");

	return ErrStack();
}

nui::ErrStack CommandList::finish(uint64_t timeout)
{
	ErrStack err_stack;

	checkErrStack1(this->endRecording());
	checkErrStack1(this->run());
	checkErrStack1(this->waitForExecution(timeout));

	return err_stack;
}

void CommandList::destroy()
{
	//printf("Command List destroy \n");

	for (auto& task : tasks) {
		vkDestroyCommandPool(dev->logical_dev, task.pool, NULL);
	}
	tasks.clear();
}

CommandList::~CommandList()
{
	if (dev != nullptr) {
		destroy();
	}
}
