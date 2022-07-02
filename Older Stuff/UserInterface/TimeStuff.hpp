#pragma once

#include <chrono>


using SteadyTime = std::chrono::time_point<std::chrono::steady_clock>;


inline uint32_t toMs(std::chrono::nanoseconds duration_ns)
{
	return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(duration_ns).count();
}
