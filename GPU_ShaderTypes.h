#pragma once

// Standard
#include <array>

// GLM
#include "glm\vec2.hpp"
#include "glm\vec3.hpp"
#include "glm\vec4.hpp"
#include "glm\mat4x4.hpp"

// Vulkan
#include "vulkan\vulkan.h"


// Rects Subpass
struct GPU_Rects_Vertex {
	glm::vec3 pos;
	glm::vec3 color;

public:
	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

// Cicles Subpass
struct GPU_Circles_Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 center;
	float radius;

public:
	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};

struct GPU_Uniform {
	float screen_width;
	float screen_height;
	float pad_0;
	float pad_1;
};
