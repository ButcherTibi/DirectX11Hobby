#pragma once

#include "pch.h"


#include "VulkanWrapper.hpp"

namespace nui {

	struct GPU_CharacterVertex {
		glm::vec2 pos;
		glm::vec2 uv;

		static vkw::VertexInput getVertexInput(uint32_t binding = 0)
		{
			vkw::VertexInput vi;
			vi.binding.binding = binding;
			vi.binding.stride = sizeof(GPU_CharacterVertex);

			vi.addAtribute(VK_FORMAT_R32G32_SFLOAT, offsetof(GPU_CharacterVertex, pos));
			vi.addAtribute(VK_FORMAT_R32G32_SFLOAT, offsetof(GPU_CharacterVertex, uv));

			return vi;
		}
	};

	
	struct GPU_CharacterInstance {
		glm::vec4 color;
		glm::vec2 pos;
		float rasterized_size;
		float size;
		uint32_t parent_clip_mask;

		static vkw::VertexInput getVertexInput(uint32_t binding = 0)
		{
			vkw::VertexInput vi;
			vi.binding.binding = binding;
			vi.binding.stride = sizeof(GPU_CharacterInstance);
			vi.binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			vi.addAtribute(VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GPU_CharacterInstance, color));
			vi.addAtribute(VK_FORMAT_R32G32_SFLOAT, offsetof(GPU_CharacterInstance, pos));
			vi.addAtribute(VK_FORMAT_R32_SFLOAT, offsetof(GPU_CharacterInstance, rasterized_size));
			vi.addAtribute(VK_FORMAT_R32_SFLOAT, offsetof(GPU_CharacterInstance, size));
			vi.addAtribute(VK_FORMAT_R32_UINT, offsetof(GPU_CharacterInstance, parent_clip_mask));

			return vi;
		}
	};


	struct GPU_CommonsUniform {
		glm::vec4 screen_size;
	};


	struct GPU_WrapVertex {
		glm::vec2 pos;

		static vkw::VertexInput getVertexInput(uint32_t binding = 0)
		{
			vkw::VertexInput vi;
			vi.binding.binding = binding;
			vi.binding.stride = sizeof(GPU_WrapVertex);

			vi.addAtribute(VK_FORMAT_R32G32_SFLOAT, offsetof(GPU_CharacterVertex, pos));

			return vi;
		}
	};


	struct GPU_WrapInstance {
		glm::vec2 pos;
		glm::vec2 size;
		glm::vec4 color;
		uint32_t parent_clip_id;
		uint32_t child_clip_id;

		static vkw::VertexInput getVertexInput(uint32_t binding = 0)
		{
			vkw::VertexInput vi;
			vi.binding.binding = binding;
			vi.binding.stride = sizeof(GPU_WrapInstance);
			vi.binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			vi.addAtribute(VK_FORMAT_R32G32_SFLOAT, offsetof(GPU_WrapInstance, pos));
			vi.addAtribute(VK_FORMAT_R32G32_SFLOAT, offsetof(GPU_WrapInstance, size));
			vi.addAtribute(VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GPU_WrapInstance, color));
			vi.addAtribute(VK_FORMAT_R32_UINT, offsetof(GPU_WrapInstance, parent_clip_id));
			vi.addAtribute(VK_FORMAT_R32_UINT, offsetof(GPU_WrapInstance, child_clip_id));

			return vi;
		}
	};
}
