
// Standard
#include <thread>
#include <execution>
#include <algorithm>

// Other
#include "VulkanContext.h"
#include "VulkanFrames.h"

#include "VulkanCommandPool.h"


ComandPoolResource::ComandPoolResource()
{
	this->in_use = false;
	this->cmd_pool = VK_NULL_HANDLE;
};

ComandPoolResource::ComandPoolResource(const ComandPoolResource& cmd_pool)
{
	this->in_use = cmd_pool.in_use.load();
	this->cmd_pool = cmd_pool.cmd_pool;
};

ComandPoolAccess::ComandPoolAccess()
{
	this->res = nullptr;
}

ComandPoolAccess::ComandPoolAccess(ComandPoolResource* res)
{
	this->res = res;
}

VkCommandPool ComandPoolAccess::get()
{
	return res->cmd_pool;
}

void ComandPoolAccess::release()
{
	res->in_use.store(false);
	res = nullptr;
}

ComandPoolAccess::~ComandPoolAccess()
{
	if (res != nullptr) {
		this->release();
	}	
}

void ComandPools::init(Device *device)
{
	std::cout << "ComandPool init" << std::endl;

	this->dev = device;
	this->cmd_pools.resize(std::thread::hardware_concurrency());
}

ErrorStack ComandPools::build()
{
	std::cout << "ComandPool build" << std::endl;

	std::atomic<VkResult> err = VK_SUCCESS;

	std::for_each(std::execution::par, cmd_pools.begin(), cmd_pools.end(), [this, &err](ComandPoolResource& cmd_pool) {

		VkCommandPoolCreateInfo command_pool_info = {};
		command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_info.queueFamilyIndex = dev->queue_fam_idx;

		VkResult vk_res = vkCreateCommandPool(dev->logical_device, &command_pool_info, NULL, &cmd_pool.cmd_pool);
		if (vk_res != VK_SUCCESS) {
			err.store(vk_res);
		}
	});

	if (err.load() != VK_SUCCESS) {
		return ErrorStack(err.load(), code_location, "failed to create command pool");
	}

	return ErrorStack();
}

ErrorStack ComandPools::acquireComandPool(ComandPoolAccess& pool_access, std::chrono::nanoseconds max_usage_time,
	std::chrono::nanoseconds time_out)
{
	std::chrono::time_point<std::chrono::steady_clock> end_time = std::chrono::steady_clock::now() + time_out;

	while (end_time > std::chrono::steady_clock::now()) {

		for (auto& cmd_pool : cmd_pools) {

			bool expected = false;
			if (cmd_pool.in_use.compare_exchange_strong(expected, true)) {
				cmd_pool.end_time = std::chrono::steady_clock::now() + max_usage_time;
				pool_access = ComandPoolAccess(&cmd_pool);
				return ErrorStack();
			}
			else if (cmd_pool.end_time < std::chrono::steady_clock::now()) {
				return ErrorStack(ExtraError::CMD_POOL_USAGE_TIME_EXCEDED, code_location, "usage of comand pool exceded time constrait");
			}
		}
	}
	return ErrorStack(ExtraError::CMD_POOL_BUSY, code_location, "all comand pools are in use for too long");
}

void ComandPools::destroy()
{
	printf("ComandPools destroy \n");

	std::for_each(std::execution::par, cmd_pools.begin(), cmd_pools.end(), [this](ComandPoolResource& cmd_pool) {
		vkDestroyCommandPool(dev->logical_device, cmd_pool.cmd_pool, NULL);
		cmd_pool.cmd_pool = VK_NULL_HANDLE;
	});
}

ComandPools::~ComandPools()
{
	if (dev != nullptr) {
		destroy();
	}
}