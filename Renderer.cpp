
// Standard
#include <execution>

// mine
#include "MathTypes.h"
#include "FileIO.h"
#include "UIComponents.h"  // has to go
#include "MathUtils.h"

// Header
#include "Renderer.h"


// globals
VulkanRenderer renderer;

ErrStack VulkanRenderer::recreateSwapchain(uint32_t width, uint32_t height)
{
	ErrStack err_stack;

	if (swapchain.swapchain != VK_NULL_HANDLE) {
		swapchain.destroy();
	}

	checkErrStack1(swapchain.create(&surface, &phys_dev, &logical_dev,
		width, height));
	checkErrStack1(swapchain.setDebugName("swapchain"));

	return err_stack;
}

ErrStack VulkanRenderer::recreateFrameImagesAndViews(uint32_t width, uint32_t height)
{
	ErrStack err_stack;

	// Border Color Image
	{
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = swapchain.surface_format.format;
		info.extent.width = swapchain.resolution.width;
		info.extent.height = swapchain.resolution.height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		checkErrStack1(border_color_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY));
		checkErrStack1(border_color_img.setDebugName("border color"));
	}

	// Border Mask Image
	{
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = VK_FORMAT_R8G8_UNORM;
		info.extent.width = swapchain.resolution.width;
		info.extent.height = swapchain.resolution.height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		checkErrStack1(border_mask_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY));
		checkErrStack1(border_mask_img.setDebugName("border mask"));
	}

	// Padding Color Image
	{
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = swapchain.surface_format.format;
		info.extent.width = swapchain.resolution.width;
		info.extent.height = swapchain.resolution.height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		checkErrStack1(padding_color_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY));
		checkErrStack1(padding_color_img.setDebugName("padding color"));
	}

	// Padding Mask Image
	{
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = VK_FORMAT_R8G8_UNORM;
		info.extent.width = swapchain.resolution.width;
		info.extent.height = swapchain.resolution.height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		checkErrStack1(padding_mask_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY));
		checkErrStack1(padding_mask_img.setDebugName("padding mask"));
	}

	// Compose Color Image
	{
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = swapchain.surface_format.format;
		info.extent.width = swapchain.resolution.width;
		info.extent.height = swapchain.resolution.height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		checkErrStack1(compose_color_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY));
		checkErrStack1(compose_color_img.setDebugName("compose color"));
	}

	VkComponentMapping component_mapping = {};
	VkImageSubresourceRange sub_range = {};
	sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	sub_range.levelCount = 1;
	sub_range.layerCount = 1;

	// Border Color View
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = border_color_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = border_color_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack1(border_color_view.recreate(&logical_dev, &info));
		checkErrStack1(border_color_view.setDebugName("border"));
	}

	// Border Mask View
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = border_mask_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = border_mask_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack1(border_mask_view.recreate(&logical_dev, &info));
		checkErrStack1(border_mask_view.setDebugName("border mask"));
	}

	// Padding Color View
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = padding_color_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = padding_color_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack1(padding_color_view.recreate(&logical_dev, &info));
		checkErrStack1(padding_color_view.setDebugName("padding"));
	}

	// Padding Mask View
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = padding_mask_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = padding_mask_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack1(padding_mask_view.recreate(&logical_dev, &info));
		checkErrStack1(padding_mask_view.setDebugName("padding mask"));
	}

	// Compose Color View
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = compose_color_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = compose_color_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack1(compose_color_view.recreate(&logical_dev, &info));
		checkErrStack1(compose_color_view.setDebugName("compose"));
	}

	return err_stack;
}

void VulkanRenderer::updateUniformDescriptorSet()
{
	std::vector<VkWriteDescriptorSet> writes(1);
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = uniform_descp_set.descp_set;
	writes[0].dstBinding = 0;
	writes[0].dstArrayElement = 0;
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorBufferInfo uniform_buff_info = {};
	uniform_buff_info.buffer = uniform_buff.buff;
	uniform_buff_info.range = sizeof(GPU_Uniform);
	writes[0].pBufferInfo = &uniform_buff_info;

	uniform_descp_set.update(writes);
}

void VulkanRenderer::updateBorderCirclesDescriptorSet()
{
	std::vector<VkWriteDescriptorSet> writes(1);

	// Border Color Image
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = border_circles_descp_set.descp_set;
	writes[0].dstBinding = 0;
	writes[0].dstArrayElement = 0;
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

	VkDescriptorImageInfo mask = {};
	mask.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	mask.imageView = border_mask_view.view;
	writes[0].pImageInfo = &mask;

	border_circles_descp_set.update(writes);
}

void VulkanRenderer::updatePaddingCirclesDescriptorSet()
{
	std::vector<VkWriteDescriptorSet> writes(1);

	// Padding Color Image
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = padding_circles_descp_set.descp_set;
	writes[0].dstBinding = 0;
	writes[0].dstArrayElement = 0;
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

	VkDescriptorImageInfo mask = {};
	mask.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	mask.imageView = padding_mask_view.view;
	writes[0].pImageInfo = &mask;

	padding_circles_descp_set.update(writes);
}

void VulkanRenderer::updateComposeImagesDescriptorSet()
{
	std::vector<VkWriteDescriptorSet> writes(4);

	// Border Color Image
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = compose_descp_set.descp_set;
	writes[0].dstBinding = 0;
	writes[0].dstArrayElement = 0;
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

	VkDescriptorImageInfo border_color = {};
	border_color.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	border_color.imageView = border_color_view.view;
	writes[0].pImageInfo = &border_color;

	// Padding Color Image
	writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].dstSet = compose_descp_set.descp_set;
	writes[1].dstBinding = 1;
	writes[1].dstArrayElement = 0;
	writes[1].descriptorCount = 1;
	writes[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

	VkDescriptorImageInfo padding_color = {};
	padding_color.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	padding_color.imageView = padding_color_view.view;
	writes[1].pImageInfo = &padding_color;

	// Border Mask Image
	writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[2].dstSet = compose_descp_set.descp_set;
	writes[2].dstBinding = 2;
	writes[2].dstArrayElement = 0;
	writes[2].descriptorCount = 1;
	writes[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

	VkDescriptorImageInfo border_mask_color = {};
	border_mask_color.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	border_mask_color.imageView = border_mask_view.view;
	writes[2].pImageInfo = &border_mask_color;

	// Padding Mask Image
	writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[3].dstSet = compose_descp_set.descp_set;
	writes[3].dstBinding = 3;
	writes[3].dstArrayElement = 0;
	writes[3].descriptorCount = 1;
	writes[3].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

	VkDescriptorImageInfo padding_mask_color = {};
	padding_mask_color.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	padding_mask_color.imageView = padding_mask_view.view;
	writes[3].pImageInfo = &padding_mask_color;

	compose_descp_set.update(writes);
}

void VulkanRenderer::updateCopyDescriptorSet()
{
	std::vector<VkWriteDescriptorSet> writes(1);
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = copy_descp_set.descp_set;
	writes[0].dstBinding = 0;
	writes[0].dstArrayElement = 0;
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

	VkDescriptorImageInfo copy_img_info = {};
	copy_img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	copy_img_info.imageView = compose_color_view.view;
	writes[0].pImageInfo = &copy_img_info;

	copy_descp_set.update(writes);
}

ErrStack VulkanRenderer::recreateFramebuffers()
{
	ErrStack err_stack;

	// Border Rect
	{
		border_rect_frames.clear();
		border_rect_frames.resize(swapchain.images.size());
		std::vector<VkImageView> attachments = {
			border_color_view.view, border_mask_view.view
		};

		for (auto i = 0; i < border_rect_frames.size(); i++) {

			checkErrStack1(border_rect_frames[i].create(&logical_dev, &rect_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(border_rect_frames[i].setDebugName("border rect"));
		}
	}
	
	// Border Circles
	{
		border_circles_frames.clear();
		border_circles_frames.resize(swapchain.images.size());
		std::vector<VkImageView> attachments = {
			border_color_view.view, border_mask_view.view
		};

		for (auto i = 0; i < border_circles_frames.size(); i++) {

			checkErrStack1(border_circles_frames[i].create(&logical_dev, &circles_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(border_circles_frames[i].setDebugName("border circles"));
		}
	}

	// Padding Rect Framebuffers
	{
		padding_rect_frames.clear();
		padding_rect_frames.resize(swapchain.images.size());
		std::vector<VkImageView> attachments = {
			padding_color_view.view, padding_mask_view.view
		};

		for (auto& frame : padding_rect_frames) {

			checkErrStack1(frame.create(&logical_dev, &rect_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(frame.setDebugName("padding rect"));
		}
	}

	// Padding Circles Framebuffers
	{
		padding_circles_frames.clear();
		padding_circles_frames.resize(swapchain.images.size());
		std::vector<VkImageView> attachments = {
			padding_color_view.view, padding_mask_view.view
		};

		for (auto& frame : padding_circles_frames) {

			checkErrStack1(frame.create(&logical_dev, &circles_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(frame.setDebugName("padding circles"));
		}
	}

	// Compose Framebuffers
	{
		compose_frames.clear();
		compose_frames.resize(swapchain.images.size());

		for (auto i = 0; i < compose_frames.size(); i++) {

			vks::Framebuffer& frame = compose_frames[i];

			std::vector<VkImageView> attachments = {
				border_color_view.view, padding_color_view.view,
				border_mask_view.view, padding_mask_view.view,
				compose_color_view.view,
			};

			checkErrStack1(frame.create(&logical_dev, &compose_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(frame.setDebugName("compose"));
		}
	}

	// Copy Framebuffers
	{
		copy_frames.clear();
		copy_frames.resize(swapchain.images.size());

		for (auto i = 0; i < copy_frames.size(); i++) {

			vks::Framebuffer& frame = copy_frames[i];

			std::vector<VkImageView> attachments = {
				compose_color_view.view, swapchain.views[i]
			};

			checkErrStack1(frame.create(&logical_dev, &copy_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(frame.setDebugName("copy"));
		}
	}

	return err_stack;
}

ErrStack VulkanRenderer::recreateRenderingCommandBuffers()
{
	std::atomic_bool is_err = false;

	std::for_each(std::execution::seq, render_cmd_buffs.cmd_buff_tasks.begin(), render_cmd_buffs.cmd_buff_tasks.end(),
		[this, &is_err](vks::CmdBufferTask& task)
		{
			auto waitForImage = [&](vks::Image& image) {

				VkImageMemoryBarrier image_barrier = {};
				image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				image_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				image_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				image_barrier.image = image.img;
				image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				image_barrier.subresourceRange.baseMipLevel = 0;
				image_barrier.subresourceRange.levelCount = 1;
				image_barrier.subresourceRange.baseArrayLayer = 0;
				image_barrier.subresourceRange.layerCount = 1;

				std::array<VkImageMemoryBarrier, 1> barriers = {
					image_barrier
				};

				vkCmdPipelineBarrier(task.cmd_buff,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0,
					0, NULL, 0, NULL, barriers.size(), barriers.data());
			};

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
				vks::cmdChangeImageLayout(task.cmd_buff, compose_color_img.img,
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

				vkCmdClearColorImage(task.cmd_buff, compose_color_img.img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1, &range);

				vks::cmdChangeImageLayout(task.cmd_buff, compose_color_img.img,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			}

			VkViewport viewport = {};
			viewport.width = (float)swapchain.resolution.width;
			viewport.height = (float)swapchain.resolution.height;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.extent.width = swapchain.resolution.width;
			scissor.extent.height = swapchain.resolution.height;

			std::array<VkClearValue, 2> clear_vals;
			clear_vals[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
			clear_vals[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };

			for (GPU_ElementsLayer& layer : layers) {

				// Border Rect Renderpass
				VkRenderPassBeginInfo border_rect_info = {};
				border_rect_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				border_rect_info.renderPass = rect_renderpass.renderpass;
				border_rect_info.framebuffer = border_rect_frames[task.idx].frame_buff;
				border_rect_info.renderArea.offset = { 0, 0 };
				border_rect_info.renderArea.extent.width = swapchain.resolution.width;
				border_rect_info.renderArea.extent.height = swapchain.resolution.height;
				border_rect_info.clearValueCount = (uint32_t)clear_vals.size();
				border_rect_info.pClearValues = clear_vals.data();

				vkCmdBeginRenderPass(task.cmd_buff, &border_rect_info, VK_SUBPASS_CONTENTS_INLINE);
				{
					vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
						rect_pipe_layout.pipe_layout, 0, 1, &uniform_descp_set.descp_set, 0, NULL);

					vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, rect_pipe.pipeline);

					vkCmdSetViewport(task.cmd_buff, 0, 1, &viewport);
					vkCmdSetScissor(task.cmd_buff, 0, 1, &scissor);

					VkBuffer vertex_buffers[] = { border_rect_vertex_buff.buff };
					VkDeviceSize offsets[] = { layer.border_rect.offset };
					vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffers, offsets);
					vkCmdDraw(task.cmd_buff, layer.border_rect.vertex_count, 1, 0, 0);
				}
				vkCmdEndRenderPass(task.cmd_buff);

				// Padding Rect Renderpass
				VkRenderPassBeginInfo padding_rect_info = {};
				padding_rect_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				padding_rect_info.renderPass = rect_renderpass.renderpass;
				padding_rect_info.framebuffer = padding_rect_frames[task.idx].frame_buff;
				padding_rect_info.renderArea.offset = { 0, 0 };
				padding_rect_info.renderArea.extent.width = swapchain.resolution.width;
				padding_rect_info.renderArea.extent.height = swapchain.resolution.height;
				padding_rect_info.clearValueCount = (uint32_t)clear_vals.size();
				padding_rect_info.pClearValues = clear_vals.data();

				vkCmdBeginRenderPass(task.cmd_buff, &padding_rect_info, VK_SUBPASS_CONTENTS_INLINE);
				{
					vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
						rect_pipe_layout.pipe_layout, 0, 1, &uniform_descp_set.descp_set, 0, NULL);

					vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, rect_pipe.pipeline);

					vkCmdSetViewport(task.cmd_buff, 0, 1, &viewport);
					vkCmdSetScissor(task.cmd_buff, 0, 1, &scissor);

					VkBuffer vertex_buffers[] = { padding_rect_vertex_buff.buff };
					VkDeviceSize offsets[] = { layer.padding_rect.offset };
					vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffers, offsets);
					vkCmdDraw(task.cmd_buff, layer.padding_rect.vertex_count, 1, 0, 0);
				}
				vkCmdEndRenderPass(task.cmd_buff);

				// Wait for the border and padding to be writen then fragment access
				{
					VkImageMemoryBarrier border_barrier = {};
					border_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					border_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					border_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					border_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					border_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					border_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					border_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					border_barrier.image = border_color_img.img;
					border_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					border_barrier.subresourceRange.baseMipLevel = 0;
					border_barrier.subresourceRange.levelCount = 1;
					border_barrier.subresourceRange.baseArrayLayer = 0;
					border_barrier.subresourceRange.layerCount = 1;

					VkImageMemoryBarrier padding_barrier = {};
					padding_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					padding_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					padding_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					padding_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					padding_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					padding_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					padding_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					padding_barrier.image = padding_color_img.img;
					padding_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					padding_barrier.subresourceRange.baseMipLevel = 0;
					padding_barrier.subresourceRange.levelCount = 1;
					padding_barrier.subresourceRange.baseArrayLayer = 0;
					padding_barrier.subresourceRange.layerCount = 1;

					std::array<VkImageMemoryBarrier, 2> barriers = {
						border_barrier, padding_barrier
					};

					vkCmdPipelineBarrier(task.cmd_buff,
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						0,
						0, NULL, 0, NULL, barriers.size(), barriers.data());
				}

				// Border Circles Renderpass
				VkRenderPassBeginInfo border_circles_info = {};
				border_circles_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				border_circles_info.renderPass = circles_renderpass.renderpass;
				border_circles_info.framebuffer = border_circles_frames[task.idx].frame_buff;
				border_circles_info.renderArea.offset = { 0, 0 };
				border_circles_info.renderArea.extent.width = swapchain.resolution.width;
				border_circles_info.renderArea.extent.height = swapchain.resolution.height;
				border_circles_info.clearValueCount = (uint32_t)clear_vals.size();
				border_circles_info.pClearValues = clear_vals.data();

				vkCmdBeginRenderPass(task.cmd_buff, &border_circles_info, VK_SUBPASS_CONTENTS_INLINE);
				{
					std::array<VkDescriptorSet, 2> descp_sets = {
						uniform_descp_set.descp_set, border_circles_descp_set.descp_set
					};	
					vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
						circles_pipe_layout.pipe_layout, 0, descp_sets.size(), descp_sets.data(), 0, NULL);

					vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, circles_pipe.pipeline);
				
					vkCmdSetViewport(task.cmd_buff, 0, 1, &viewport);
					vkCmdSetScissor(task.cmd_buff, 0, 1, &scissor);

					VkBuffer vertex_buffers[] = { border_circles_vertex_buff.buff };
					VkDeviceSize offsets[] = { layer.border_circles_offset };
					vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffers, offsets);

					vkCmdDraw(task.cmd_buff, layer.border_circles[0].vertex_count, 1, layer.border_circles[0].first_vertex, 0);

					waitForImage(border_color_img);
					vkCmdDraw(task.cmd_buff, layer.border_circles[1].vertex_count, 1, layer.border_circles[1].first_vertex, 0);

					waitForImage(border_color_img);
					vkCmdDraw(task.cmd_buff, layer.border_circles[2].vertex_count, 1, layer.border_circles[2].first_vertex, 0);

					waitForImage(border_color_img);
					vkCmdDraw(task.cmd_buff, layer.border_circles[3].vertex_count, 1, layer.border_circles[3].first_vertex, 0);
				}
				vkCmdEndRenderPass(task.cmd_buff);

				// Padding Circles Renderpass
				VkRenderPassBeginInfo padding_circles_info = {};
				padding_circles_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				padding_circles_info.renderPass = circles_renderpass.renderpass;
				padding_circles_info.framebuffer = padding_circles_frames[task.idx].frame_buff;
				padding_circles_info.renderArea.offset = { 0, 0 };
				padding_circles_info.renderArea.extent.width = swapchain.resolution.width;
				padding_circles_info.renderArea.extent.height = swapchain.resolution.height;
				padding_circles_info.clearValueCount = (uint32_t)clear_vals.size();
				padding_circles_info.pClearValues = clear_vals.data();

				vkCmdBeginRenderPass(task.cmd_buff, &padding_circles_info, VK_SUBPASS_CONTENTS_INLINE);
				{
					std::array<VkDescriptorSet, 2> descp_sets = {
						uniform_descp_set.descp_set, padding_circles_descp_set.descp_set
					};
					vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
						circles_pipe_layout.pipe_layout, 0, descp_sets.size(), descp_sets.data(), 0, NULL);

					vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, circles_pipe.pipeline);

					vkCmdSetViewport(task.cmd_buff, 0, 1, &viewport);
					vkCmdSetScissor(task.cmd_buff, 0, 1, &scissor);

					VkBuffer vertex_buffers[] = { padding_circles_vertex_buff.buff };
					VkDeviceSize offsets[] = { layer.padding_circles_offset };
					vkCmdBindVertexBuffers(task.cmd_buff, 0, 1, vertex_buffers, offsets);

					vkCmdDraw(task.cmd_buff, layer.padding_circles[0].vertex_count, 1, layer.padding_circles[0].first_vertex, 0);

					waitForImage(padding_color_img);
					vkCmdDraw(task.cmd_buff, layer.padding_circles[1].vertex_count, 1, layer.padding_circles[1].first_vertex, 0);

					waitForImage(padding_color_img);
					vkCmdDraw(task.cmd_buff, layer.padding_circles[2].vertex_count, 1, layer.padding_circles[2].first_vertex, 0);

					waitForImage(padding_color_img);
					vkCmdDraw(task.cmd_buff, layer.padding_circles[3].vertex_count, 1, layer.padding_circles[3].first_vertex, 0);
				}
				vkCmdEndRenderPass(task.cmd_buff);

				// Wait for the border and padding to be writen then fragment access
				{
					VkImageMemoryBarrier border_barrier = {};
					border_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					border_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					border_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					border_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					border_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					border_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					border_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					border_barrier.image = border_color_img.img;
					border_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					border_barrier.subresourceRange.baseMipLevel = 0;
					border_barrier.subresourceRange.levelCount = 1;
					border_barrier.subresourceRange.baseArrayLayer = 0;
					border_barrier.subresourceRange.layerCount = 1;

					VkImageMemoryBarrier padding_barrier = {};
					padding_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					padding_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					padding_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					padding_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					padding_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					padding_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					padding_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					padding_barrier.image = padding_color_img.img;
					padding_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					padding_barrier.subresourceRange.baseMipLevel = 0;
					padding_barrier.subresourceRange.levelCount = 1;
					padding_barrier.subresourceRange.baseArrayLayer = 0;
					padding_barrier.subresourceRange.layerCount = 1;

					std::array<VkImageMemoryBarrier, 2> barriers = {
						border_barrier, padding_barrier
					};

					vkCmdPipelineBarrier(task.cmd_buff,
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						0,
						0, NULL, 0, NULL, barriers.size(), barriers.data());
				}

				// Compose Renderpass
				VkRenderPassBeginInfo compose_info = {};
				compose_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				compose_info.renderPass = compose_renderpass.renderpass;
				compose_info.framebuffer = compose_frames[task.idx].frame_buff;
				compose_info.renderArea.offset = { 0, 0 };
				compose_info.renderArea.extent.width = swapchain.resolution.width;
				compose_info.renderArea.extent.height = swapchain.resolution.height;

				std::array<VkClearValue, 3> compose_clears;
				compose_clears[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
				compose_clears[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
				compose_clears[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
				compose_info.clearValueCount = compose_clears.size();
				compose_info.pClearValues = compose_clears.data();

				vkCmdBeginRenderPass(task.cmd_buff, &compose_info, VK_SUBPASS_CONTENTS_INLINE);
				{
					vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
						compose_pipe_layout.pipe_layout, 0, 1, &compose_descp_set.descp_set, 0, NULL);
					vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, compose_pipe.pipeline);

					vkCmdDraw(task.cmd_buff, 6, 1, 0, 0);
				}
				vkCmdEndRenderPass(task.cmd_buff);

				// Wait for the border and padding to accessed then write to color atachment
				{
					VkImageMemoryBarrier border_barrier = {};
					border_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					border_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					border_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					border_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					border_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					border_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					border_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					border_barrier.image = border_color_img.img;
					border_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					border_barrier.subresourceRange.baseMipLevel = 0;
					border_barrier.subresourceRange.levelCount = 1;
					border_barrier.subresourceRange.baseArrayLayer = 0;
					border_barrier.subresourceRange.layerCount = 1;

					VkImageMemoryBarrier padding_barrier = {};
					padding_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					padding_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					padding_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					padding_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					padding_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					padding_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					padding_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					padding_barrier.image = padding_color_img.img;
					padding_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					padding_barrier.subresourceRange.baseMipLevel = 0;
					padding_barrier.subresourceRange.levelCount = 1;
					padding_barrier.subresourceRange.baseArrayLayer = 0;
					padding_barrier.subresourceRange.layerCount = 1;

					// compose image barrier not required because border and padding dependency

					std::array<VkImageMemoryBarrier, 2> barriers = {
						border_barrier, padding_barrier
					};

					vkCmdPipelineBarrier(task.cmd_buff,
						VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						0,
						0, NULL, 0, NULL, barriers.size(), barriers.data());
				}
			}

			// Wait for the compose image to be written by all layers then copy to swapchain
			{
				VkImageMemoryBarrier compose_barrier = {};
				compose_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				compose_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				compose_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				compose_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				compose_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				compose_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				compose_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				compose_barrier.image = compose_color_img.img;
				compose_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				compose_barrier.subresourceRange.baseMipLevel = 0;
				compose_barrier.subresourceRange.levelCount = 1;
				compose_barrier.subresourceRange.baseArrayLayer = 0;
				compose_barrier.subresourceRange.layerCount = 1;

				vkCmdPipelineBarrier(task.cmd_buff,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0, 0, NULL, 0, NULL, 1, &compose_barrier);
			}

			// Copy Renderpass
			VkRenderPassBeginInfo copy_info = {};
			copy_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			copy_info.renderPass = copy_renderpass.renderpass;
			copy_info.framebuffer = copy_frames[task.idx].frame_buff;
			copy_info.renderArea.offset = { 0, 0 };
			copy_info.renderArea.extent.width = swapchain.resolution.width;
			copy_info.renderArea.extent.height = swapchain.resolution.height;

			std::array<VkClearValue, 2> copy_clears;
			copy_clears[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
			copy_clears[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
			copy_info.clearValueCount = copy_clears.size();
			copy_info.pClearValues = copy_clears.data();

			vkCmdBeginRenderPass(task.cmd_buff, &copy_info, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindDescriptorSets(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
					copy_pipe_layout.pipe_layout, 0, 1, &copy_descp_set.descp_set, 0, NULL);
				vkCmdBindPipeline(task.cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, copy_pipe.pipeline);

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
		for (vks::CmdBufferTask& task : render_cmd_buffs.cmd_buff_tasks) {
			if (task.err.isBad()) {
				return task.err;
			}
		}
	}

	return ErrStack();
}

ErrStack VulkanRenderer::createContext(HINSTANCE* hinstance, HWND* hwnd)
{
	ErrStack err_stack;

	checkErrStack1(instance.create());
	checkErrStack1(surface.create(&instance, *hinstance, *hwnd));
	checkErrStack1(phys_dev.create(&instance, &surface));
	checkErrStack1(logical_dev.create(&instance, &phys_dev));

	return err_stack;
}

ErrStack VulkanRenderer::getPhysicalSurfaceResolution(uint32_t& width, uint32_t& height)
{
	ErrStack err_stack;

	VkSurfaceCapabilitiesKHR capabilities = {};

	VkResult vk_res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev.physical_device, surface.surface, &capabilities);
	if (vk_res != VK_SUCCESS) {
		return ErrStack(vk_res, code_location, "failed to get physical device surface capabilities");
	}

	VkExtent2D min_img_extent = capabilities.minImageExtent;
	VkExtent2D max_img_extent = capabilities.maxImageExtent;

	width = clamp(width, min_img_extent.width, max_img_extent.width);
	height = clamp(height, min_img_extent.height, max_img_extent.height);

	return err_stack;
}

ErrStack VulkanRenderer::recreate(uint32_t width, uint32_t height)
{
	ErrStack err_stack;

	// Command Pool
	checkErrStack1(cmd_pool.create(&logical_dev, &phys_dev));

	checkErrStack1(recreateSwapchain(width, height));

	checkErrStack1(recreateFrameImagesAndViews(swapchain.resolution.width, swapchain.resolution.height));

	// Uniform Descriptor Layout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings(1);
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		checkErrStack1(uniform_descp_layout.create(&logical_dev, bindings));
	}

	// Rect Renderpass
	{
		VkAttachmentDescription color_atach = {};
		color_atach.format = swapchain.surface_format.format;
		color_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_atach_ref = {};
		color_atach_ref.attachment = 0;
		color_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription mask_atach = {};
		mask_atach.format = VK_FORMAT_R8G8_UNORM;
		mask_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		mask_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		mask_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		mask_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		mask_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		mask_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		mask_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference mask_atach_ref = {};
		mask_atach_ref.attachment = 1;
		mask_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		std::array<VkAttachmentReference, 2> color_atachs = {
			color_atach_ref, mask_atach_ref
		};

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = color_atachs.size();
		subpass.pColorAttachments = color_atachs.data();

		std::array<VkAttachmentDescription, 2> attachments = {
			color_atach, mask_atach
		};

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = (uint32_t)attachments.size();
		info.pAttachments = attachments.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 0;

		checkErrStack(rect_renderpass.create(&logical_dev, &info),
			"failed to create renderpass");
		checkErrStack1(rect_renderpass.setDebugName("rects renderpass"));
	}

	// Rect Vertex Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Rects/vert.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(rect_vert_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_VERTEX_BIT));
	}

	// Rect Fragment Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Rects/frag.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(rect_frag_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	// Rect Pipeline Layout
	{
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &uniform_descp_layout.descp_layout;

		checkErrStack(rect_pipe_layout.create(&logical_dev, &info), "");
	}

	// Rect Pipeline
	{
		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
		shader_stages[0] = {};
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = rect_vert_module.stage;
		shader_stages[0].module = rect_vert_module.sh_module;
		shader_stages[0].pName = "main";

		shader_stages[1] = {};
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = rect_frag_module.stage;
		shader_stages[1].module = rect_frag_module.sh_module;
		shader_stages[1].pName = "main";

		// Vertex Input State
		VkVertexInputBindingDescription vert_input_binding_descp = {};
		vert_input_binding_descp = GPU_Rects_Vertex::getBindingDescription();

		std::vector<VkVertexInputAttributeDescription> vertex_input_atribute_descps;
		for (auto& atribute_descp : GPU_Rects_Vertex::getAttributeDescriptions()) {
			vertex_input_atribute_descps.push_back(atribute_descp);
		}

		VkPipelineVertexInputStateCreateInfo vert_input_stage_info = {};
		vert_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_stage_info.vertexBindingDescriptionCount = 1;
		vert_input_stage_info.pVertexBindingDescriptions = &vert_input_binding_descp;
		vert_input_stage_info.vertexAttributeDescriptionCount = (uint32_t)vertex_input_atribute_descps.size();
		vert_input_stage_info.pVertexAttributeDescriptions = vertex_input_atribute_descps.data();

		// Input Asembly State
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
		input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

		// Viewport
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.resolution.width;
		viewport.height = (float)swapchain.resolution.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = swapchain.resolution.width;
		scissor.extent.height = swapchain.resolution.height;

		VkPipelineViewportStateCreateInfo viewport_state_info = {};
		viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_info.viewportCount = 1;
		viewport_state_info.pViewports = &viewport;
		viewport_state_info.scissorCount = 1;
		viewport_state_info.pScissors = &scissor;

		// Rasterization
		VkPipelineRasterizationStateCreateInfo raster_state_info = {};
		raster_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_state_info.depthClampEnable = VK_FALSE;
		raster_state_info.rasterizerDiscardEnable = VK_FALSE;
		raster_state_info.polygonMode = VK_POLYGON_MODE_FILL;
		raster_state_info.cullMode = VK_CULL_MODE_NONE;
		raster_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster_state_info.depthBiasEnable = VK_FALSE;
		raster_state_info.lineWidth = 1.0f;

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
		multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_info.sampleShadingEnable = VK_FALSE;

		// Depth Stencil
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
		depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_info.depthTestEnable = VK_FALSE;
		depth_stencil_state_info.depthWriteEnable = VK_FALSE;
		depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_info.stencilTestEnable = VK_FALSE;

		// Color Blending
		VkPipelineColorBlendAttachmentState color_blend_attachment = {};
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendAttachmentState mask_blend_attach = {};
		mask_blend_attach.blendEnable = VK_FALSE;
		mask_blend_attach.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		mask_blend_attach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		mask_blend_attach.colorBlendOp = VK_BLEND_OP_ADD;
		mask_blend_attach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		mask_blend_attach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		mask_blend_attach.alphaBlendOp = VK_BLEND_OP_ADD;
		mask_blend_attach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		std::vector<VkPipelineColorBlendAttachmentState> atachs = {
			color_blend_attachment, mask_blend_attach
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
		color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_info.logicOpEnable = VK_FALSE;
		color_blend_state_info.logicOp = VK_LOGIC_OP_NO_OP;
		color_blend_state_info.attachmentCount = (uint32_t)atachs.size();
		color_blend_state_info.pAttachments = atachs.data();
		color_blend_state_info.blendConstants[0] = 0.0f;
		color_blend_state_info.blendConstants[1] = 0.0f;
		color_blend_state_info.blendConstants[2] = 0.0f;
		color_blend_state_info.blendConstants[3] = 0.0f;

		//  Dynamic State
		std::array<VkDynamicState, 2> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
		dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_info.dynamicStateCount = dynamic_states.size();
		dynamic_state_info.pDynamicStates = dynamic_states.data();

		// Pipeline 
		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = (uint32_t)shader_stages.size();
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.pVertexInputState = &vert_input_stage_info;
		pipeline_info.pInputAssemblyState = &input_assembly_state_info;
		pipeline_info.pTessellationState = NULL;
		pipeline_info.pViewportState = &viewport_state_info;
		pipeline_info.pRasterizationState = &raster_state_info;
		pipeline_info.pMultisampleState = &multisample_state_info;
		pipeline_info.pDepthStencilState = &depth_stencil_state_info;
		pipeline_info.pColorBlendState = &color_blend_state_info;
		pipeline_info.pDynamicState = &dynamic_state_info;
		pipeline_info.layout = rect_pipe_layout.pipe_layout;
		pipeline_info.renderPass = rect_renderpass.renderpass;
		pipeline_info.subpass = 0;

		checkErrStack(rect_pipe.create(&logical_dev, &pipeline_info), "");
		checkErrStack1(rect_pipe.setDebugName("border rect"));
	}

	// Circles Renderpass
	{
		VkAttachmentDescription color_atach = {};
		color_atach.format = swapchain.surface_format.format;
		color_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_atach_ref = {};
		color_atach_ref.attachment = 0;
		color_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription mask_atach = {};
		mask_atach.format = VK_FORMAT_R8G8_UNORM;
		mask_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		mask_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		mask_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		mask_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		mask_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		mask_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		mask_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference mask_in_atach_ref = {};
		mask_in_atach_ref.attachment = 1;
		mask_in_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference mask_out_atach_ref = {};
		mask_out_atach_ref.attachment = 1;
		mask_out_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		std::array<VkAttachmentReference, 2> out_atachs = {
			color_atach_ref, mask_out_atach_ref
		};

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = 1;
		subpass.pInputAttachments = &mask_in_atach_ref;
		subpass.colorAttachmentCount = out_atachs.size();
		subpass.pColorAttachments = out_atachs.data();

		std::array<VkAttachmentDescription, 2> attachments = {
			color_atach, mask_atach
		};

		VkSubpassDependency self_dependency = {};
		self_dependency.srcSubpass = 0;
		self_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		self_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		self_dependency.dstSubpass = 0;
		self_dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		self_dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		std::array<VkSubpassDependency, 1> subpass_dependencies = {
			self_dependency
		};

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = attachments.size();
		info.pAttachments = attachments.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = subpass_dependencies.size();
		info.pDependencies = subpass_dependencies.data();

		checkErrStack(circles_renderpass.create(&logical_dev, &info),
			"failed to create renderpass");
		checkErrStack1(circles_renderpass.setDebugName("circles renderpass"));
	}

	// Circles Vertex Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Circles/vert.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(circles_vert_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_VERTEX_BIT));
	}

	// Circles Fragment Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Circles/frag.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(circles_frag_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	// Circles Descriptor Layout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings(1);
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		checkErrStack1(circles_descp_layout.create(&logical_dev, bindings));
	}

	// Circles Pipeline Layout
	{
		std::array<VkDescriptorSetLayout, 2> descp_layouts = {
			uniform_descp_layout.descp_layout, circles_descp_layout.descp_layout
		};

		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = descp_layouts.size();
		info.pSetLayouts = descp_layouts.data();

		checkErrStack(circles_pipe_layout.create(&logical_dev, &info), "");
	}

	// Circles Pipeline
	{
		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
		shader_stages[0] = {};
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = circles_vert_module.stage;
		shader_stages[0].module = circles_vert_module.sh_module;
		shader_stages[0].pName = "main";

		shader_stages[1] = {};
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = circles_frag_module.stage;
		shader_stages[1].module = circles_frag_module.sh_module;
		shader_stages[1].pName = "main";

		// Vertex Input State
		VkVertexInputBindingDescription vert_input_binding_descp = {};
		vert_input_binding_descp = GPU_Circles_Vertex::getBindingDescription();

		std::vector<VkVertexInputAttributeDescription> vertex_input_atribute_descps;
		for (auto& atribute_descp : GPU_Circles_Vertex::getAttributeDescriptions()) {
			vertex_input_atribute_descps.push_back(atribute_descp);
		}

		VkPipelineVertexInputStateCreateInfo vert_input_stage_info = {};
		vert_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_stage_info.vertexBindingDescriptionCount = 1;
		vert_input_stage_info.pVertexBindingDescriptions = &vert_input_binding_descp;
		vert_input_stage_info.vertexAttributeDescriptionCount = (uint32_t)vertex_input_atribute_descps.size();
		vert_input_stage_info.pVertexAttributeDescriptions = vertex_input_atribute_descps.data();

		// Input Asembly State
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
		input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

		// Viewport
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.resolution.width;
		viewport.height = (float)swapchain.resolution.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = swapchain.resolution.width;
		scissor.extent.height = swapchain.resolution.height;

		VkPipelineViewportStateCreateInfo viewport_state_info = {};
		viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_info.viewportCount = 1;
		viewport_state_info.pViewports = &viewport;
		viewport_state_info.scissorCount = 1;
		viewport_state_info.pScissors = &scissor;

		// Rasterization
		VkPipelineRasterizationStateCreateInfo raster_state_info = {};
		raster_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_state_info.depthClampEnable = VK_FALSE;
		raster_state_info.rasterizerDiscardEnable = VK_FALSE;
		raster_state_info.polygonMode = VK_POLYGON_MODE_FILL;
		raster_state_info.cullMode = VK_CULL_MODE_NONE;
		raster_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster_state_info.depthBiasEnable = VK_FALSE;
		raster_state_info.lineWidth = 1.0f;

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
		multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_info.sampleShadingEnable = VK_FALSE;

		// Depth Stencil
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
		depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_info.depthTestEnable = VK_FALSE;
		depth_stencil_state_info.depthWriteEnable = VK_FALSE;
		depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_info.stencilTestEnable = VK_FALSE;

		// Color Blending
		VkPipelineColorBlendAttachmentState color_blend_atach = {};
		color_blend_atach.blendEnable = VK_FALSE;
		color_blend_atach.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_atach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_atach.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_atach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_atach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_atach.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_atach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendAttachmentState mask_blend_attach = {};
		mask_blend_attach.blendEnable = VK_FALSE;
		mask_blend_attach.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		mask_blend_attach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		mask_blend_attach.colorBlendOp = VK_BLEND_OP_ADD;
		mask_blend_attach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		mask_blend_attach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		mask_blend_attach.alphaBlendOp = VK_BLEND_OP_ADD;
		mask_blend_attach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		std::vector<VkPipelineColorBlendAttachmentState> atachs = {
			color_blend_atach, mask_blend_attach
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
		color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_info.logicOpEnable = VK_FALSE;
		color_blend_state_info.logicOp = VK_LOGIC_OP_NO_OP;
		color_blend_state_info.attachmentCount = (uint32_t)atachs.size();
		color_blend_state_info.pAttachments = atachs.data();
		color_blend_state_info.blendConstants[0] = 0.0f;
		color_blend_state_info.blendConstants[1] = 0.0f;
		color_blend_state_info.blendConstants[2] = 0.0f;
		color_blend_state_info.blendConstants[3] = 0.0f;

		//  Dynamic State
		std::array<VkDynamicState, 2> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
		dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_info.dynamicStateCount = dynamic_states.size();
		dynamic_state_info.pDynamicStates = dynamic_states.data();

		// Pipeline 
		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = (uint32_t)shader_stages.size();
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.pVertexInputState = &vert_input_stage_info;
		pipeline_info.pInputAssemblyState = &input_assembly_state_info;
		pipeline_info.pTessellationState = NULL;
		pipeline_info.pViewportState = &viewport_state_info;
		pipeline_info.pRasterizationState = &raster_state_info;
		pipeline_info.pMultisampleState = &multisample_state_info;
		pipeline_info.pDepthStencilState = &depth_stencil_state_info;
		pipeline_info.pColorBlendState = &color_blend_state_info;
		pipeline_info.pDynamicState = &dynamic_state_info;
		pipeline_info.layout = circles_pipe_layout.pipe_layout;
		pipeline_info.renderPass = circles_renderpass.renderpass;
		pipeline_info.subpass = 0;

		checkErrStack(circles_pipe.create(&logical_dev, &pipeline_info), "");
		checkErrStack1(circles_pipe.setDebugName("circles"));
	}

	// Compose Renderpass
	{
		VkAttachmentDescription border_color_atach = {};
		border_color_atach.format = swapchain.surface_format.format;
		border_color_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		border_color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		border_color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		border_color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		border_color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		border_color_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		border_color_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference border_color_in_atach_ref = {};
		border_color_in_atach_ref.attachment = 0;
		border_color_in_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription padding_color_atach = {};
		padding_color_atach.format = swapchain.surface_format.format;
		padding_color_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		padding_color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		padding_color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		padding_color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		padding_color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		padding_color_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		padding_color_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference padding_color_in_atach_ref = {};
		padding_color_in_atach_ref.attachment = 1;
		padding_color_in_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription border_mask_atach = {};
		border_mask_atach.format = VK_FORMAT_R8G8_UNORM;
		border_mask_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		border_mask_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		border_mask_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		border_mask_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		border_mask_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		border_mask_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		border_mask_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference border_mask_in_atach_ref = {};
		border_mask_in_atach_ref.attachment = 2;
		border_mask_in_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription padding_mask_atach = {};
		padding_mask_atach.format = VK_FORMAT_R8G8_UNORM;
		padding_mask_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		padding_mask_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		padding_mask_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		padding_mask_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		padding_mask_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		padding_mask_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		padding_mask_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference padding_mask_in_atach = {};
		padding_mask_in_atach.attachment = 3;
		padding_mask_in_atach.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::array<VkAttachmentReference, 4> input_attach_refs = {
			border_color_in_atach_ref, padding_color_in_atach_ref,
			border_mask_in_atach_ref, padding_mask_in_atach
		};

		VkAttachmentDescription compose_atach = {};
		compose_atach.format = swapchain.surface_format.format;
		compose_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		compose_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		compose_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		compose_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		compose_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		compose_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		compose_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference compose_atach_ref = {};
		compose_atach_ref.attachment = 4;
		compose_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = input_attach_refs.size();
		subpass.pInputAttachments = input_attach_refs.data();
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &compose_atach_ref;

		std::array<VkAttachmentDescription, 5> attachments = {
			border_color_atach, padding_color_atach,
			border_mask_atach, padding_mask_atach,
			compose_atach
		};

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = (uint32_t)attachments.size();
		info.pAttachments = attachments.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 0;

		checkErrStack(compose_renderpass.create(&logical_dev, &info),
			"failed to create compose renderpass");
	}

	// Compose Descriptor Layout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings(4);
		// Border Color Image
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Padding Color Image
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Border Mask Image
		bindings[2].binding = 2;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[2].descriptorCount = 1;
		bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Padding Mask Image
		bindings[3].binding = 3;
		bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[3].descriptorCount = 1;
		bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		checkErrStack1(compose_descp_layout.create(&logical_dev, bindings));
		checkErrStack1(compose_descp_layout.setDebugName("compose"));
	}

	// Compose Vertex Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Compose/vert.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(compose_vert_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_VERTEX_BIT));
	}

	// Compose Fragment Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Compose/frag.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(compose_frag_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	// Compose Pipeline Layout
	{
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &compose_descp_layout.descp_layout;

		checkErrStack1(compose_pipe_layout.create(&logical_dev, &info));
	}

	// Compose Pipeline
	{
		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
		shader_stages[0] = {};
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = compose_vert_module.stage;
		shader_stages[0].module = compose_vert_module.sh_module;
		shader_stages[0].pName = "main";

		shader_stages[1] = {};
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = compose_frag_module.stage;
		shader_stages[1].module = compose_frag_module.sh_module;
		shader_stages[1].pName = "main";

		// Vertex Input Description
		VkPipelineVertexInputStateCreateInfo vert_input_stage_info = {};
		vert_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_stage_info.vertexBindingDescriptionCount = 0;
		vert_input_stage_info.pVertexBindingDescriptions = NULL;
		vert_input_stage_info.vertexAttributeDescriptionCount = 0;
		vert_input_stage_info.pVertexAttributeDescriptions = NULL;

		// Input Asembly State
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
		input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

		// Viewport
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.resolution.width;
		viewport.height = (float)swapchain.resolution.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = swapchain.resolution.width;
		scissor.extent.height = swapchain.resolution.height;

		VkPipelineViewportStateCreateInfo viewport_state_info = {};
		viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_info.viewportCount = 1;
		viewport_state_info.pViewports = &viewport;
		viewport_state_info.scissorCount = 1;
		viewport_state_info.pScissors = &scissor;

		// Rasterization
		VkPipelineRasterizationStateCreateInfo raster_state_info = {};
		raster_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_state_info.depthClampEnable = VK_FALSE;
		raster_state_info.rasterizerDiscardEnable = VK_FALSE;
		raster_state_info.polygonMode = VK_POLYGON_MODE_FILL;
		raster_state_info.cullMode = VK_CULL_MODE_NONE;
		raster_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster_state_info.depthBiasEnable = VK_FALSE;
		raster_state_info.lineWidth = 1.0f;

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
		multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_info.sampleShadingEnable = VK_FALSE;

		// Depth Stencil
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
		depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_info.depthTestEnable = VK_FALSE;
		depth_stencil_state_info.depthWriteEnable = VK_FALSE;
		depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_info.stencilTestEnable = VK_FALSE;

		// Color Blending
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments = {
			color_blend_attachment
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
		color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_info.logicOpEnable = VK_FALSE;
		color_blend_state_info.logicOp = VK_LOGIC_OP_NO_OP;
		color_blend_state_info.attachmentCount = (uint32_t)color_blend_attachments.size();
		color_blend_state_info.pAttachments = color_blend_attachments.data();
		color_blend_state_info.blendConstants[0] = 0.0f;
		color_blend_state_info.blendConstants[1] = 0.0f;
		color_blend_state_info.blendConstants[2] = 0.0f;
		color_blend_state_info.blendConstants[3] = 0.0f;

		// Pipeline 
		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = (uint32_t)shader_stages.size();
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.pVertexInputState = &vert_input_stage_info;
		pipeline_info.pInputAssemblyState = &input_assembly_state_info;
		pipeline_info.pTessellationState = NULL;
		pipeline_info.pViewportState = &viewport_state_info;
		pipeline_info.pRasterizationState = &raster_state_info;
		pipeline_info.pMultisampleState = &multisample_state_info;
		pipeline_info.pDepthStencilState = &depth_stencil_state_info;
		pipeline_info.pColorBlendState = &color_blend_state_info;
		pipeline_info.pDynamicState = NULL;
		pipeline_info.layout = compose_pipe_layout.pipe_layout;
		pipeline_info.renderPass = compose_renderpass.renderpass;
		pipeline_info.subpass = 0;

		checkErrStack(compose_pipe.create(&logical_dev, &pipeline_info), "");
		checkErrStack1(compose_pipe.setDebugName("compose"));
	}

	// Copy Renderpass
	{
		VkAttachmentDescription compose_in_atach = {};
		compose_in_atach.format = swapchain.surface_format.format;
		compose_in_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		compose_in_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		compose_in_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		compose_in_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		compose_in_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		compose_in_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		compose_in_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference compose_in_atach_ref = {};
		compose_in_atach_ref.attachment = 0;
		compose_in_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription present_atach = {};
		present_atach.format = swapchain.surface_format.format;
		present_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		present_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		present_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		present_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		present_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		present_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		present_atach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference present_atach_ref = {};
		present_atach_ref.attachment = 1;
		present_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = 1;
		subpass.pInputAttachments = &compose_in_atach_ref;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &present_atach_ref;

		std::array<VkAttachmentDescription, 2> attachments = {
			compose_in_atach, present_atach
		};

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = (uint32_t)attachments.size();
		info.pAttachments = attachments.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 0;

		checkErrStack(copy_renderpass.create(&logical_dev, &info),
			"failed to create renderpass");
		checkErrStack1(copy_renderpass.setDebugName("copy renderpass"));
	}

	// Copy Descriptor Layout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings(1);
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		checkErrStack1(copy_descp_layout.create(&logical_dev, bindings));
	}

	// Copy Fragment Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Copy/frag.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(copy_frag_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	// Copy Pipeline Layout
	{
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &copy_descp_layout.descp_layout;

		checkErrStack1(copy_pipe_layout.create(&logical_dev, &info));
	}

	// Copy Pipeline
	{
		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
		shader_stages[0] = {};
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = compose_vert_module.stage;
		shader_stages[0].module = compose_vert_module.sh_module;
		shader_stages[0].pName = "main";

		shader_stages[1] = {};
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = copy_frag_module.stage;
		shader_stages[1].module = copy_frag_module.sh_module;
		shader_stages[1].pName = "main";

		// Vertex Input Description
		VkPipelineVertexInputStateCreateInfo vert_input_stage_info = {};
		vert_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_stage_info.vertexBindingDescriptionCount = 0;
		vert_input_stage_info.pVertexBindingDescriptions = NULL;
		vert_input_stage_info.vertexAttributeDescriptionCount = 0;
		vert_input_stage_info.pVertexAttributeDescriptions = NULL;

		// Input Asembly State
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
		input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

		// Viewport
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.resolution.width;
		viewport.height = (float)swapchain.resolution.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = swapchain.resolution.width;
		scissor.extent.height = swapchain.resolution.height;

		VkPipelineViewportStateCreateInfo viewport_state_info = {};
		viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_info.viewportCount = 1;
		viewport_state_info.pViewports = &viewport;
		viewport_state_info.scissorCount = 1;
		viewport_state_info.pScissors = &scissor;

		// Rasterization
		VkPipelineRasterizationStateCreateInfo raster_state_info = {};
		raster_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_state_info.depthClampEnable = VK_FALSE;
		raster_state_info.rasterizerDiscardEnable = VK_FALSE;
		raster_state_info.polygonMode = VK_POLYGON_MODE_FILL;
		raster_state_info.cullMode = VK_CULL_MODE_NONE;
		raster_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster_state_info.depthBiasEnable = VK_FALSE;
		raster_state_info.lineWidth = 1.0f;

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
		multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_info.sampleShadingEnable = VK_FALSE;

		// Depth Stencil
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
		depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_info.depthTestEnable = VK_FALSE;
		depth_stencil_state_info.depthWriteEnable = VK_FALSE;
		depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_info.stencilTestEnable = VK_FALSE;

		// Color Blending
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments = {
			color_blend_attachment
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
		color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_info.logicOpEnable = VK_FALSE;
		color_blend_state_info.logicOp = VK_LOGIC_OP_NO_OP;
		color_blend_state_info.attachmentCount = (uint32_t)color_blend_attachments.size();
		color_blend_state_info.pAttachments = color_blend_attachments.data();
		color_blend_state_info.blendConstants[0] = 0.0f;
		color_blend_state_info.blendConstants[1] = 0.0f;
		color_blend_state_info.blendConstants[2] = 0.0f;
		color_blend_state_info.blendConstants[3] = 0.0f;

		// Pipeline 
		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = (uint32_t)shader_stages.size();
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.pVertexInputState = &vert_input_stage_info;
		pipeline_info.pInputAssemblyState = &input_assembly_state_info;
		pipeline_info.pTessellationState = NULL;
		pipeline_info.pViewportState = &viewport_state_info;
		pipeline_info.pRasterizationState = &raster_state_info;
		pipeline_info.pMultisampleState = &multisample_state_info;
		pipeline_info.pDepthStencilState = &depth_stencil_state_info;
		pipeline_info.pColorBlendState = &color_blend_state_info;
		pipeline_info.pDynamicState = NULL;
		pipeline_info.layout = copy_pipe_layout.pipe_layout;
		pipeline_info.renderPass = copy_renderpass.renderpass;
		pipeline_info.subpass = 0;

		checkErrStack(copy_pipe.create(&logical_dev, &pipeline_info), "");
		checkErrStack1(copy_pipe.setDebugName("copy"));
	}

	// Command Buffers
	checkErrStack1(render_cmd_buffs.recreate(&logical_dev, &phys_dev, (uint32_t)swapchain.images.size()));

	// Buffers
	{
		common_staging_buff.logical_dev = &logical_dev;
		border_rect_staging_buff.logical_dev = &logical_dev;
		padding_rect_staging_buff.logical_dev = &logical_dev;
		border_circles_staging_buff.logical_dev = &logical_dev;
		padding_circles_staging_buff.logical_dev = &logical_dev;

		uniform_buff.create(&logical_dev, &cmd_pool, &common_staging_buff,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		border_rect_vertex_buff.create(&logical_dev, &cmd_pool, &border_rect_staging_buff,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		padding_rect_vertex_buff.create(&logical_dev, &cmd_pool, &padding_rect_staging_buff,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		border_circles_vertex_buff.create(&logical_dev, &cmd_pool, &border_circles_staging_buff,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		padding_circles_vertex_buff.create(&logical_dev, &cmd_pool, &padding_circles_staging_buff,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		// Generate GPU Data
		checkErrStack1(calc(*user_interface));
	}

	// Framebuffers
	checkErrStack1(recreateFramebuffers());

	// Descriptor Pools, Sets, Updates
	{
		// Descriptor Pool
		{
			std::vector<VkDescriptorPoolSize> sizes(2);
			sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			sizes[0].descriptorCount = 1;

			sizes[1].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			sizes[1].descriptorCount = 7;

			checkErrStack1(descp_pool.create(&logical_dev, sizes, 8));
			checkErrStack1(descp_pool.setDebugName("descp pool"));
		}

		checkErrStack1(uniform_descp_set.create(&logical_dev, &descp_pool, &uniform_descp_layout));
		checkErrStack1(border_circles_descp_set.create(&logical_dev, &descp_pool, &circles_descp_layout));
		checkErrStack1(padding_circles_descp_set.create(&logical_dev, &descp_pool, &circles_descp_layout));
		checkErrStack1(compose_descp_set.create(&logical_dev, &descp_pool, &compose_descp_layout));
		checkErrStack1(copy_descp_set.create(&logical_dev, &descp_pool, &copy_descp_layout));

		updateUniformDescriptorSet();
		updateBorderCirclesDescriptorSet();
		updatePaddingCirclesDescriptorSet();
		updateComposeImagesDescriptorSet();
		updateCopyDescriptorSet();
	}

	// Command Buffer Update
	checkErrStack1(recreateRenderingCommandBuffers());

	// Syncronization
	if (img_acquired.semaphore == VK_NULL_HANDLE) {
		checkErrStack(img_acquired.recreate(&logical_dev),
			"failed to create semaphore for acquiring image from swapchain");
	}

	if (rendering_ended_sem.semaphore == VK_NULL_HANDLE) {
		checkErrStack(rendering_ended_sem.recreate(&logical_dev),
			"failed to create semaphore for rendering ended");
	}

	return ErrStack();
}

ErrStack VulkanRenderer::calc(UserInterface& user_interface)
{
	ErrStack err_stack;

	// Set Screen Size
	{
		uniform_buff.clear();

		BasicElement* root_elem = std::get_if<BasicElement>(&user_interface.elems.front().elem);

		GPU_Uniform uniform;
		uniform.screen_width = root_elem->_contentbox_width;
		uniform.screen_height = root_elem->_contentbox_height;

		checkErrStack1(uniform_buff.push(&uniform, sizeof(GPU_Uniform)));
		checkErrStack1(uniform_buff.flush());
	}
	

	auto createChamferedRectangle = [&](glm::vec2 origin, float width, float height,
		float tl_radius, float tr_radius, float br_radius, float bl_radius,
		std::array<GPU_Rects_Vertex, 18>& rect_verts)
	{
		glm::vec2 vec2_origin{ origin.x, origin.y };

		glm::vec2 top_left_up = vec2_origin;
		glm::vec2 top_left_down = vec2_origin;

		glm::vec2 top_right_up = vec2_origin;
		glm::vec2 top_right_down = vec2_origin;

		glm::vec2 bot_right_up = vec2_origin;
		glm::vec2 bot_right_down = vec2_origin;

		glm::vec2 bot_left_up = vec2_origin;
		glm::vec2 bot_left_down = vec2_origin;

		// Box positions
		top_left_up.x += tl_radius;
		top_left_down.y += tl_radius;

		top_right_up.x += width - tr_radius;
		top_right_down.x += width;
		top_right_down.y += tr_radius;

		bot_right_up.x += width;
		bot_right_up.y += height - br_radius;
		bot_right_down.x += width - br_radius;
		bot_right_down.y += height;

		bot_left_up.y += height - bl_radius;
		bot_left_down.x += bl_radius;
		bot_left_down.y += height;

		// Box triangles
		rect_verts[0].pos = top_left_up;
		rect_verts[1].pos = top_right_up;
		rect_verts[2].pos = top_right_down;

		rect_verts[3].pos = top_left_up;
		rect_verts[4].pos = top_right_down;
		rect_verts[5].pos = bot_right_up;

		rect_verts[6].pos = top_left_up;
		rect_verts[7].pos = bot_right_up;
		rect_verts[8].pos = bot_right_down;

		rect_verts[9].pos = top_left_up;
		rect_verts[10].pos = bot_right_down;
		rect_verts[11].pos = bot_left_down;

		rect_verts[12].pos = top_left_up;
		rect_verts[13].pos = bot_left_down;
		rect_verts[14].pos = bot_left_up;

		rect_verts[15].pos = top_left_up;
		rect_verts[16].pos = bot_left_up;
		rect_verts[17].pos = top_left_down;
	};

	auto createCircle = [](glm::vec2 origin, float radius,
		std::array<GPU_Circles_Vertex, 24>& verts, size_t idx)
	{
		glm::vec2 vec2_origin{ origin.x, origin.y };

		glm::vec2 top_left = vec2_origin;
		glm::vec2 top_right = vec2_origin;
		glm::vec2 bot_right = vec2_origin;
		glm::vec2 bot_left = vec2_origin;

		float diameter = radius * 2;
		top_right.x += diameter;
		bot_right += diameter;
		bot_left.y += diameter;

		// Position
		verts[idx + 0].pos = top_left;
		verts[idx + 1].pos = top_right;
		verts[idx + 2].pos = bot_right;

		verts[idx + 3].pos = bot_left;
		verts[idx + 4].pos = top_left;
		verts[idx + 5].pos = bot_right;

		// Center and Radius
		glm::vec2 center = vec2_origin;
		center += radius;

		for (auto i = idx; i < idx + 6; i++) {
			verts[i].center = center;
			verts[i].radius = radius;
		}
	};

	this->layers.clear();
	this->layers.resize(user_interface.layers.size());
	auto ui_it = user_interface.layers.begin();

	border_rect_vertex_buff.clear();
	border_circles_vertex_buff.clear();
	padding_rect_vertex_buff.clear();
	padding_circles_vertex_buff.clear();

	for (auto l = 0; l < user_interface.layers.size(); l++) {

		ElementsLayer& ui_layer = *ui_it;
		GPU_ElementsLayer& gpu_layer = this->layers[l];

		gpu_layer.border_rect.offset = border_rect_vertex_buff.load_size_;

		gpu_layer.border_circles_offset = border_circles_vertex_buff.load_size_;
		gpu_layer.border_circles = {};

		gpu_layer.padding_rect.offset = padding_rect_vertex_buff.load_size_;

		gpu_layer.padding_circles_offset = padding_circles_vertex_buff.load_size_;
		gpu_layer.padding_circles = {};

		for (Element* elem : ui_layer.elems) {

			auto* basic = std::get_if<BasicElement>(&elem->elem);

			// Border Rect
			std::array<GPU_Rects_Vertex, 18> border_rect;
			glm::vec2 border_origin = basic->origin_;

			createChamferedRectangle(border_origin, basic->borderbox_width_, basic->borderbox_height_,
				basic->border_tl_radius, basic->border_tr_radius, basic->border_br_radius, basic->border_bl_radius,
				border_rect);

			for (GPU_Rects_Vertex& vert : border_rect) {
				vert.color = basic->border_color;
			}

			// Border Circles
			std::array<GPU_Circles_Vertex, 24> border_circles;
			{
				if (basic->border_top_thick_ && basic->border_left_thick_) {

					createCircle(border_origin, basic->border_tl_radius, border_circles, 0);

					gpu_layer.border_circles[0].vertex_count += 6;
				}

				if (basic->border_top_thick_ && basic->border_right_thick_) {

					glm::vec2 tr_origin = border_origin;
					tr_origin.x += basic->borderbox_width_ - (basic->border_tr_radius * 2);
					createCircle(tr_origin, basic->border_tr_radius, border_circles, 6);

					gpu_layer.border_circles[1].first_vertex += gpu_layer.border_circles[0].vertex_count;
					gpu_layer.border_circles[1].vertex_count += 6;
				}

				if (basic->border_bot_thick_ && basic->border_right_thick_) {

					glm::vec2 br_origin = border_origin;
					br_origin.x += basic->borderbox_width_ - (basic->border_br_radius * 2);
					br_origin.y += basic->borderbox_height_ - (basic->border_br_radius * 2);
					createCircle(br_origin, basic->border_br_radius, border_circles, 12);

					gpu_layer.border_circles[2].first_vertex += gpu_layer.border_circles[0].vertex_count + gpu_layer.border_circles[1].vertex_count;
					gpu_layer.border_circles[2].vertex_count += 6;
				}

				if (basic->border_bot_thick_ && basic->border_left_thick_) {

					glm::vec2 bl_origin = border_origin;
					bl_origin.y += basic->borderbox_height_ - (basic->border_bl_radius * 2);
					createCircle(bl_origin, basic->border_bl_radius, border_circles, 18);

					gpu_layer.border_circles[3].first_vertex += gpu_layer.border_circles[0].vertex_count + gpu_layer.border_circles[1].vertex_count + gpu_layer.border_circles[2].vertex_count;
					gpu_layer.border_circles[3].vertex_count += 6;
				}

				for (auto& vert : border_circles) {
					vert.color = basic->border_color;
				}
			}

			// Create Padding Box
			std::array<GPU_Rects_Vertex, 18> padding_rect;

			glm::vec2 padding_origin = border_origin;
			padding_origin.x += basic->border_left_thick_;
			padding_origin.y += basic->border_top_thick_;
			{
				createChamferedRectangle(padding_origin, basic->paddingbox_width_, basic->paddingbox_height_,
					basic->padding_tl_radius, basic->padding_tr_radius, basic->padding_br_radius, basic->padding_bl_radius,
					padding_rect);

				for (auto& vert : padding_rect) {
					vert.color = basic->background_color;
				}
			}

			// Create Padding Circles
			std::array<GPU_Circles_Vertex, 24> padding_circles;
			{
				glm::vec2 tr_origin = padding_origin;
				glm::vec2 br_origin = padding_origin;
				glm::vec2 bl_origin = padding_origin;

				tr_origin.x += basic->paddingbox_width_ - (basic->padding_tr_radius * 2);
				br_origin.x += basic->paddingbox_width_ - (basic->padding_br_radius * 2);
				br_origin.y += basic->paddingbox_height_ - (basic->padding_br_radius * 2);
				bl_origin.y += basic->paddingbox_height_ - (basic->padding_bl_radius * 2);

				createCircle(padding_origin, basic->padding_tl_radius, padding_circles, 0);
				createCircle(tr_origin, basic->padding_tr_radius, padding_circles, 6);
				createCircle(br_origin, basic->padding_br_radius, padding_circles, 12);
				createCircle(bl_origin, basic->padding_bl_radius, padding_circles, 18);

				for (auto& vert : padding_circles) {
					vert.color = basic->background_color;
				}

				gpu_layer.padding_circles[0].vertex_count += 6;

				gpu_layer.padding_circles[1].first_vertex += gpu_layer.padding_circles[0].vertex_count;
				gpu_layer.padding_circles[1].vertex_count += 6;

				gpu_layer.padding_circles[2].first_vertex += gpu_layer.padding_circles[0].vertex_count + gpu_layer.padding_circles[1].vertex_count;
				gpu_layer.padding_circles[2].vertex_count += 6;

				gpu_layer.padding_circles[3].first_vertex += gpu_layer.padding_circles[0].vertex_count + gpu_layer.padding_circles[1].vertex_count + gpu_layer.padding_circles[2].vertex_count;
				gpu_layer.padding_circles[3].vertex_count += 6;
			}

			// Load into buffers
			gpu_layer.border_rect.vertex_count += border_rect.size();
			gpu_layer.padding_rect.vertex_count += padding_rect.size();


			checkErrStack1(border_rect_vertex_buff.push(border_rect.data(), border_rect.size() * sizeof(GPU_Rects_Vertex)));
			checkErrStack1(border_circles_vertex_buff.push(border_circles.data(), border_circles.size() * sizeof(GPU_Circles_Vertex)));
			checkErrStack1(padding_rect_vertex_buff.push(padding_rect.data(), padding_rect.size() * sizeof(GPU_Rects_Vertex)));
			checkErrStack1(padding_circles_vertex_buff.push(padding_circles.data(), padding_circles.size() * sizeof(GPU_Circles_Vertex)));
		}

		++ui_it;
	}

	checkErrStack1(border_rect_vertex_buff.flush());
	checkErrStack1(border_circles_vertex_buff.flush());
	checkErrStack1(padding_rect_vertex_buff.flush());
	checkErrStack1(padding_circles_vertex_buff.flush());

	return ErrStack();
}

ErrStack VulkanRenderer::changeResolution(uint32_t new_width, uint32_t new_height)
{
	ErrStack err_stack;

	checkErrStack1(recreateSwapchain(new_width, new_height));
	checkErrStack1(recreateFrameImagesAndViews(swapchain.resolution.width, swapchain.resolution.height));

	checkErrStack1(calc(*user_interface));

	updateUniformDescriptorSet();
	updateBorderCirclesDescriptorSet();
	updatePaddingCirclesDescriptorSet();
	updateComposeImagesDescriptorSet();
	updateCopyDescriptorSet();

	checkErrStack1(recreateFramebuffers());
	checkErrStack1(recreateRenderingCommandBuffers());

	return err_stack;
}

ErrStack VulkanRenderer::draw()
{
	uint32_t image_index;

	checkVkRes(vkAcquireNextImageKHR(logical_dev.logical_device, swapchain.swapchain, UINT64_MAX,
		img_acquired.semaphore, VK_NULL_HANDLE, &image_index), 
		"failed to acquire next image in swapchain");

	// Render
	VkSubmitInfo queue_submit_info = {};
	queue_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	queue_submit_info.waitSemaphoreCount = 1;
	queue_submit_info.pWaitSemaphores = &img_acquired.semaphore;
	queue_submit_info.pWaitDstStageMask = wait_stages;
	queue_submit_info.commandBufferCount = 1;
	queue_submit_info.pCommandBuffers = &render_cmd_buffs.cmd_buff_tasks[image_index].cmd_buff;
	queue_submit_info.signalSemaphoreCount = 1;
	queue_submit_info.pSignalSemaphores = &rendering_ended_sem.semaphore;

	checkVkRes(vkQueueSubmit(logical_dev.queue, 1, &queue_submit_info, NULL),
		"failed to submit draw commands to gpu queue");

	// Present image
	VkSwapchainKHR swapChains[] = { swapchain.swapchain };

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &rendering_ended_sem.semaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapChains;
	present_info.pImageIndices = &image_index;
	present_info.pResults = NULL;

	checkVkRes(vkQueuePresentKHR(logical_dev.queue, &present_info),
		"failed to present image");

	return ErrStack();
}

ErrStack VulkanRenderer::waitForRendering()
{
	checkVkRes(vkDeviceWaitIdle(logical_dev.logical_device), "");
	return ErrStack();
}
