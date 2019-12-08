#pragma once

// Standard
#include <chrono>
#include <atomic>


// This thing is never used, should delete ?

struct ComandPoolResource
{
	std::atomic_bool in_use;
	std::chrono::time_point<std::chrono::steady_clock> end_time;
	
	VkCommandPool cmd_pool = VK_NULL_HANDLE;

	// std::atomic nonsense
	ComandPoolResource();
	ComandPoolResource(const ComandPoolResource& cmd_pool);
};


// exclusive access to a command pool
// if instance goes out of scope then access is relinquished
// can be replaced by unique ptr but this doesn't use heap
class ComandPoolAccess
{
	ComandPoolResource* res;

public:
	ComandPoolAccess();
	ComandPoolAccess(ComandPoolResource* res);

	VkCommandPool get();
	void release();
	~ComandPoolAccess();
};


class ComandPools
{
public:
	Device* dev = nullptr;

	// Content
	std::vector<ComandPoolResource> cmd_pools;

	void init(Device *device);

	ErrorStack build();

	// try get exclusive access to a command pool for max_acquire_dur
	// else block for time_out
	ErrorStack acquireComandPool(ComandPoolAccess& pool_access,
		std::chrono::nanoseconds max_acquire_dur = std::chrono::seconds(5),
		std::chrono::nanoseconds time_out = std::chrono::seconds(5));

	void destroy();

	~ComandPools();
};
