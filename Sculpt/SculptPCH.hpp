#pragma once

// The precompiled header file

// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Windows undefs
#undef min
#undef max

// Standard
#include <vector>
#include <array>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <variant>

#include <chrono>
#include <algorithm>

#include <cmath>

// GLM
#include "glm\vec3.hpp"
#include "glm\gtc\quaternion.hpp"
#include <glm\mat4x4.hpp>

#include "glm\geometric.hpp"
#include "glm\ext\quaternion_transform.hpp"
#include "glm\gtx\matrix_decompose.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\rotate_vector.hpp"

