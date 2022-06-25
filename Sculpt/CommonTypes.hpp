#pragma once

#include <chrono>

typedef uint8_t byte;
typedef uint32_t u32;

using SteadyTime = std::chrono::time_point<std::chrono::steady_clock>;
