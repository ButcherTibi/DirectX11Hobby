#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef min
#undef max
#include <wrl\client.h>

// Vulkan
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#undef VK_USE_PLATFORM_WIN32_KHR

// DirectX 11
#include <d3d11_4.h>
#include <dxgi1_6.h>
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

// FreeType
#include "ft2build.h"
#include FT_FREETYPE_H

// Standard
#include <string>
#include <vector>
#include <array>
#include <list>
#include <variant>
#include <cmath>

// GLM
#include "glm\vec2.hpp"
#include "glm\vec4.hpp"

// Mine
#include "ErrorStack.hpp"
