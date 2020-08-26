#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef min
#undef max

// Vulkan
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

// FreeType
#include "ft2build.h"
#include FT_FREETYPE_H

// Standard
#include <string>
#include <vector>
#include <array>
#include <list>
#include <variant>

// GLM
#include "glm\vec2.hpp"
#include "glm\vec4.hpp"

// Mine
#include "ErrorStack.hpp"
