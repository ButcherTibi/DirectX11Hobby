#pragma once

// Standard
#include <array>

// GLM
#include "glm\vec2.hpp"
#include "glm\vec3.hpp"
#include "glm\vec4.hpp"
#include "glm\mat4x4.hpp"


// 3D
class GPU_3D_Vertex {
public:
	uint32_t mesh_id;
	glm::vec3 pos;
	glm::vec3 vertex_normal;
	glm::vec3 tess_normal;
	glm::vec3 poly_normal;

	glm::vec2 uv;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions();
};

struct GPU_3D_Uniform {
	alignas(16) glm::vec3 camera_pos;
	alignas(16) glm::vec4 camera_rot;
	alignas(16) glm::mat4 camera_perspective;
	alignas(16) glm::vec3 camera_forward;
};

// these bad
struct GPU_3D_Instance {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec4 rot;
};


// UI
class GPU_UI_Vertex {
public:
	glm::vec2 local_pos;
	glm::uint local_vert_idx;

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

struct GPU_UI_Instance {
	glm::vec2 pos;
	float scale;
	float pad;
	glm::vec2 uvs[6];
};

struct UI_DrawBatch {
	uint32_t vert_idx_start;
	uint32_t vert_count;
	uint32_t inst_idx_start;
	uint32_t inst_count = 0;
};
