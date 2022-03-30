#pragma once

// Standard
#include <array>
#include <vector>

// Vulkan
#include "vulkan/vulkan.h"


namespace dga {

	/* Common Base Objects */

	class NotCopyable {
	public:
		NotCopyable() = default;
		NotCopyable(NotCopyable const&) = delete;
		NotCopyable& operator=(NotCopyable const&) = delete;
	};

	class DeviceObject {
	public:
		VkDevice device = nullptr;
	};


	/* Resources */

	class Buffer {

	};

	class Texture {

	};


	class Shader : public DeviceObject , NotCopyable {
	public:
		VkShaderModule shader = nullptr;

	public:
		~Shader();
	};

	class VertexShader : public Shader { };
	class PixelShader : public Shader { };


	/* Management */

	struct VulkanPipelineInfo {
		// Input Assembly
		VkPipelineVertexInputStateCreateInfo vertex_input_info;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
		
		// Vertex Shader
		VkPipelineShaderStageCreateInfo vertex_stage;

		// Rasterizer
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewport_state;

		VkPipelineRasterizationStateCreateInfo rasterizer;

		VkPipelineMultisampleStateCreateInfo multisample;

		// Pixel Shader
		VkPipelineShaderStageCreateInfo pixel_stage;

		std::vector<VkPipelineShaderStageCreateInfo> stages;

		void _create();
	};

	class RasterizerPipeline : public DeviceObject, NotCopyable {
	public:
		VulkanPipelineInfo pipe_info;

	public:
		void _create();

		// Input Assembly
		void setPrimitiveTopology(VkPrimitiveTopology topology);

		// Vertex Shader
		void setVertexShader(VertexShader* vertex_shader);
		

		/* Rasterizer */
		
		void setViewportPosition(float x, float y);
		void setViewportSize(float width, float height);
		void setViewportDepth(float min_depth, float max_depth);
		void setScissorOffset(int32_t x, int32_t y);
		void setScissorExtend(uint32_t width, uint32_t height);

		void setPolygonMode(VkPolygonMode mode);
		void setCullMode(VkCullModeFlags flags);
		void setFrontFace(VkFrontFace front_face);

		void setMultisampleCount(VkSampleCountFlagBits flags);

		// @TODO: Depth test

		// Pixel Shader
		void setPixelShader(PixelShader* pixel_shader);

		// Blend State

	};


	class Device : NotCopyable {
	public:
		VkDevice device = nullptr;
		VkQueue graphics_queue = nullptr;

	public:
		void _createShader(std::vector<char8_t>& shader_spirv, Shader& r_shader);
		void createVertexShader(std::vector<char8_t>& shader_spirv, VertexShader& r_vertex_shader);
		void createPixelShader(std::vector<char8_t>& shader_spirv, PixelShader& r_pixel_shader);

		void createRasterizerPipeline(RasterizerPipeline& r_pipeline);

		~Device();
	};


	class Instance : NotCopyable {
	public:
		VkInstance instance = nullptr;

		std::array<const char*, 1> layers = {
			"VK_LAYER_KHRONOS_validation"
		};

		std::array<const char*, 1> extensions = {
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		};

		VkDebugUtilsMessengerEXT callback = VK_NULL_HANDLE;
		PFN_vkSetDebugUtilsObjectNameEXT setDebugUtilsObjectName;

	public:
		void create();

		void getBestDevice(Device& r_device);

		~Instance();
	};
}
