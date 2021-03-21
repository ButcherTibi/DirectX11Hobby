#pragma once

// The precompiled header file

// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Windows undefs
#undef min
#undef max

// DirectX 11
#include <d3d11_4.h>
#include <dxgi1_6.h>
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

// Standard
#include <vector>
#include <array>
#include <list>
#include <set>

#include <string>

#include <cmath>

// GLM
#include <glm\vec2.hpp>
#include <glm\vec4.hpp>

// Mine
#include "ErrorStack.hpp"
