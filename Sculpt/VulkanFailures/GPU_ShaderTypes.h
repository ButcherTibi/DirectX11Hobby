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
	uint32_t elem_idx;

public:
	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};

// Cicles Subpass
struct GPU_Circles_Vertex {
	glm::vec2 pos;
	glm::vec4 color;
	glm::vec2 center;
	float radius;
	uint32_t elem_idx;

public:
	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();
};


struct GPU_Uniform {
	float screen_width;
	float screen_height;
	float pad_0;
	float pad_1;
};


struct GPU_Storage {
	// Border
	glm::vec4 border_color;
	
	glm::vec2 border_tl_center;
	float border_tl_radius;
	float _pad_0;

	glm::vec2 border_tr_center;
	float border_tr_radius;
	float _pad_1;

	glm::vec2 border_br_center;
	float border_br_radius;
	float _pad_2;

	glm::vec2 border_bl_center;
	float border_bl_radius;
	float _pad_3;

	// Padding
	glm::vec4 padding_color;

	glm::vec2 padding_tl_center;
	float padding_tl_radius;
	float _pad_4;

	glm::vec2 padding_tr_center;
	float padding_tr_radius;
	float _pad_5;

	glm::vec2 padding_br_center;
	float padding_br_radius;
	float _pad_6;

	glm::vec2 padding_bl_center;
	float padding_bl_radius;
	float _pad_7;
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
