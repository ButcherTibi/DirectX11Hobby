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

struct GPU_ElementsLayer {
	GPU_Draw border_rects;
	GPU_Draw border_tl_circles;
	GPU_Draw border_tr_circles;
	GPU_Draw border_br_circles;
	GPU_Draw border_bl_circles;

	GPU_Draw padding_rects;
	GPU_Draw padding_tl_circles;
	GPU_Draw padding_tr_circles;
	GPU_Draw padding_br_circles;
	GPU_Draw padding_bl_circles;
};
