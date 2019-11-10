#pragma once

// Mine
#include "VulkanContext.h"


class VulkanSync
{
public:
	Device* dev = nullptr;

	// Staged Load
	VkFence buff_upload_done = VK_NULL_HANDLE;

	// Draw
	VkSemaphore	img_acquired = VK_NULL_HANDLE;
	VkSemaphore render_ended = VK_NULL_HANDLE;
	VkFence render_ended_fence = VK_NULL_HANDLE;

public:
	void init(Device* device);

	ErrorStack build();

	void destroy();

	~VulkanSync();
};