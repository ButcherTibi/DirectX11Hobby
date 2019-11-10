

#include "VulkanSyncronization.h"

void VulkanSync::init(Device* device)
{
	std::cout << "FrameSync init" << std::endl;

	this->dev = device;
}

ErrorStack VulkanSync::build()
{
	printf("VulkanSync build \n");

	VkResult vk_res;

	// Staged Load
	{
		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		vk_res = vkCreateFence(dev->logical_device, &fence_info, NULL, &buff_upload_done);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, code_location, "failed to create buff_upload_done fence");
		}
	}

	// Draw Sync
	{
		VkSemaphoreCreateInfo sem_info = {};
		sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vk_res = vkCreateSemaphore(dev->logical_device, &sem_info, NULL, &img_acquired);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, code_location, "failed to create img_acquired semaphore");
		}

		vk_res = vkCreateSemaphore(dev->logical_device, &sem_info, NULL, &render_ended);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, code_location, "failed to create render_ended semaphore");
		}

		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		vk_res = vkCreateFence(dev->logical_device, &fence_info, NULL, &render_ended_fence);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, code_location, "failed to create render_ended_fence fence");
		}
	}

	return ErrorStack();
}

void VulkanSync::destroy()
{
	printf("VulkanSync destroy \n");

	// Load
	vkDestroyFence(dev->logical_device, buff_upload_done, NULL);

	// Draw
	vkDestroySemaphore(dev->logical_device, img_acquired, NULL);
	vkDestroySemaphore(dev->logical_device, render_ended, NULL);
	vkDestroyFence(dev->logical_device, render_ended_fence, NULL);
}

VulkanSync::~VulkanSync()
{
	if (this->dev != nullptr) {

		destroy();
	}
}