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
	glm::vec2 pos;
	glm::vec4 color;

public:
	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

// Cicles Subpass
struct GPU_Circles_Vertex {
	glm::vec2 pos;
	glm::vec4 color;
	glm::vec2 center;
	float radius;

public:

	// aparently these are not a thing and slow on AMD ?
	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};


struct GPU_Uniform {
	float screen_width;
	float screen_height;
	float pad_0;
	float pad_1;
};


struct GPU_Draw {
	uint32_t vertex_count;
	size_t offset;
};

struct GPU_CornerCircles {
	uint32_t vertex_count;
	uint32_t first_vertex;
};

struct GPU_ElementsLayer {
	GPU_Draw border_rect;

	size_t border_circles_offset;
	std::array<GPU_CornerCircles, 4> border_circles;

	GPU_Draw padding_rect;

	size_t padding_circles_offset;
	std::array<GPU_CornerCircles, 4> padding_circles;
};
