#pragma once

// Standard
#include <vector>
#include <array>

// GLM
#include "glm\vec3.hpp"
#include "glm\mat4x4.hpp"

// VMA
#include "vk_mem_alloc.h"

// Windows
#include <Windows.h>

// Vulkan
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

// Mine
#include "ErrorStuff.h"
#include "GPU_ShaderTypes.h"


namespace vks {

	class Instance {
	public:
		std::vector<const char*> validation_layers = {
			"VK_LAYER_LUNARG_standard_validation" 
		};
		std::vector<const char*> instance_extensions = { 
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		};

		VkInstance instance = VK_NULL_HANDLE;

		VkDebugUtilsMessageSeverityFlagsEXT debug_msg_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		VkDebugUtilsMessengerEXT callback = VK_NULL_HANDLE;

		PFN_vkSetDebugUtilsObjectNameEXT set_vkdbg_name_func;

	public:
		ErrStack create();

		void destroy();

		~Instance();
	};


	class Surface {
		Instance const* instance = nullptr;
	public:
		VkSurfaceKHR surface = VK_NULL_HANDLE;
	
	public:
		ErrStack create(Instance* instance, HINSTANCE hinstance, HWND hwnd);

		void destroy();

		~Surface();
	};


	class PhysicalDevice {
	public:
		VkPhysicalDeviceFeatures phys_dev_features = {};
		std::vector<const char*> device_extensions = { 
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_MEMORY_BUDGET_EXTENSION_NAME };

		VkPhysicalDevice physical_device;

		uint32_t queue_fam_idx;
		VkPhysicalDeviceProperties phys_dev_props;
		VkPhysicalDeviceMemoryProperties mem_props;

		VkSampleCountFlagBits max_MSAA;

	public:
		PhysicalDevice();
		ErrStack create(Instance* instance, Surface* surface);
	};


	class LogicalDevice {
		Instance* instance = nullptr;
	public:
		float queue_priority = 1.0f;

		VkDevice logical_device = VK_NULL_HANDLE;

		VkQueue queue;

		VmaAllocator allocator;

	public:
		ErrStack create(Instance* instance, PhysicalDevice* phys_dev);

		ErrStack setDebugName(uint64_t obj, VkObjectType obj_type, std::string name);

		void destroy();

		~LogicalDevice();
	};


	class Swapchain {
		LogicalDevice* logical_device = nullptr;

	public:
		//VkSurfaceCapabilitiesKHR capabilities;
		VkExtent2D resolution = {800, 600};

		VkSurfaceTransformFlagBitsKHR pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkPresentModeKHR presentation_mode = VK_PRESENT_MODE_FIFO_KHR;
		VkSurfaceFormatKHR surface_format;

		VkSwapchainKHR swapchain = VK_NULL_HANDLE;

		std::vector<VkImage> images;
		std::vector<VkImageView> views;

	public:
		ErrStack create(Surface* surface, PhysicalDevice* phys_dev, LogicalDevice* logical_dev,
			uint32_t width, uint32_t height);

		ErrStack setDebugName(std::string name);

		void destroy();

		~Swapchain();
	};


	class CommandPool {
		LogicalDevice* logical_dev = nullptr;
	public:

		VkCommandPool cmd_pool = VK_NULL_HANDLE;
	public:
		ErrStack create(LogicalDevice* logical_dev, PhysicalDevice* phys_dev);

		void destroy();

		~CommandPool();
	};


	class SingleCommandBuffer {
		LogicalDevice* logical_dev = nullptr;
		CommandPool* cmd_pool;

		ErrStack* err;
	public:
		VkCommandBuffer cmd_buff;
	public:
		SingleCommandBuffer(LogicalDevice* logical_dev, CommandPool* cmd_pool, ErrStack* r_err);
		~SingleCommandBuffer();
	};


	class StagingBuffer {
	public:
		LogicalDevice* logical_dev = nullptr;

		VkBuffer buff = VK_NULL_HANDLE;
		VmaAllocation buff_alloc;
		VmaAllocationInfo vma_r_info = {};
		void* mem;	
		size_t load_size = 0;

	public:
		ErrStack create_(size_t buff_size,
			VkBuffer& new_buff, VmaAllocation& new_alloc, void*& new_mem);

		ErrStack reserve(size_t size);

		ErrStack push(void* data, size_t size);

		void clear();

		void destroy();

		~StagingBuffer();

		ErrStack setDebugName(std::string name);
	};


	enum class LoadType {
		STAGING,
		MEMCPY,
	};


	class Buffer {
	public:
		LogicalDevice* logical_dev_ = nullptr;
		CommandPool* cmd_pool_ = nullptr;
		StagingBuffer* staging_ = nullptr;
		VkBufferUsageFlags usage_;
		VmaMemoryUsage mem_usage_;
		LoadType load_type_;
		size_t load_size_;

		VkBuffer buff = VK_NULL_HANDLE;
		void* mem;
		VmaAllocation buff_alloc;
		VmaAllocationInfo vma_r_info = {};

	public:
		ErrStack create_(size_t size, VkBuffer& new_buff, VmaAllocation& new_alloc, void*& new_mem);

		void create(LogicalDevice* logical_dev, CommandPool* cmd_pool, StagingBuffer* staging,
			VkBufferUsageFlags usage, VmaMemoryUsage mem_usage);

		ErrStack push(void* data, size_t size);

		ErrStack flush();

		void clear();

		void destroy();

		~Buffer();

		ErrStack setDebugName(std::string name);
	};


	void cmdChangeImageLayout(VkCommandBuffer cmd_buff, VkImage img, VkImageLayout old_layout,
		VkImageLayout new_layout);


	class Image {
		LogicalDevice* logical_dev = nullptr;

	public:
		VkImage img = VK_NULL_HANDLE;
		void* mem;

		VmaAllocation alloc;
		VmaAllocationInfo alloc_info;
		LoadType load_type;

		// Image Properties
		uint32_t width;
		uint32_t height;
		VkFormat format;
		VkImageLayout layout;
		uint32_t mip_lvl;
		uint32_t samples;

	private:
		ErrStack copyBufferToImage(CommandPool* cmd_pool, Buffer* buff);

	public:
		ErrStack recreate(LogicalDevice* logical_dev, VkImageCreateInfo* info, VmaMemoryUsage mem_usage);

		ErrStack setDebugName(std::string name);

		ErrStack changeImageLayout(CommandPool* cmd_pool, VkImageLayout new_layout);

		ErrStack load(void* colors, size_t size, CommandPool* cmd_pool, Buffer* staging_buff,
			VkImageLayout layout_after_load);

		void destroy();

		~Image();	
	};


	class ImageView {
		LogicalDevice* logical_dev = nullptr;

	public:
		VkImageView view;

	public:
		ErrStack recreate(LogicalDevice* logical_dev, VkImageViewCreateInfo* info);

		ErrStack setDebugName(std::string name);

		void destroy();

		~ImageView();
	};


	class Sampler {
		LogicalDevice const* logical_dev = nullptr;
	public:
		VkSampler sampler = VK_NULL_HANDLE;
	public:
		ErrStack create(LogicalDevice* logical_dev, VkSamplerCreateInfo& info);

		void destroy();

		~Sampler();
	};


	class Renderpass {
		LogicalDevice* logical_dev = nullptr;
	public:

		VkRenderPass renderpass = VK_NULL_HANDLE;
	public:
		ErrStack create(LogicalDevice* logical_dev, VkRenderPassCreateInfo* info);

		ErrStack setDebugName(std::string name);

		void destroy();

		~Renderpass();
	};


	class Framebuffer {
	public:
		LogicalDevice* logical_dev_ = nullptr;

		VkFramebuffer frame_buff;
	public:
		ErrStack create(LogicalDevice* logical_dev, Renderpass* renderpass,
			std::vector<VkImageView>& attachments, uint32_t width, uint32_t height);

		void destroy();

		ErrStack setDebugName(std::string name);

		~Framebuffer();
	};


	class DescriptorSetLayout {
		LogicalDevice* logical_dev = nullptr;

	public:
		VkDescriptorSetLayout descp_layout = VK_NULL_HANDLE;

	public:
		ErrStack create(LogicalDevice* logical_dev, std::vector<VkDescriptorSetLayoutBinding>& bindings);

		ErrStack setDebugName(std::string name);

		void destroy();

		~DescriptorSetLayout();
	};


	class DescriptorPool {
		LogicalDevice* logical_dev = nullptr;

	public:
		VkDescriptorPool descp_pool = VK_NULL_HANDLE;

	public:
		ErrStack create(LogicalDevice* logical_dev, std::vector<VkDescriptorPoolSize>& pools, uint32_t max_sets);

		ErrStack setDebugName(std::string name);

		void destroy();

		~DescriptorPool();
	};


	class DescriptorSet {
		LogicalDevice* logical_dev = nullptr;

	public:
		VkDescriptorSet descp_set = VK_NULL_HANDLE;

	public:
		ErrStack create(LogicalDevice* logical_dev, DescriptorPool* pool, DescriptorSetLayout* layout);

		ErrStack setDebugName(std::string name);

		void update(std::vector<VkWriteDescriptorSet>& writes);
	};


	class PipelineLayout {
		LogicalDevice const* logical_dev = nullptr;
	public:

		VkPipelineLayout pipe_layout = VK_NULL_HANDLE;

	public:
		ErrStack create(LogicalDevice* logical_dev, VkPipelineLayoutCreateInfo* info);

		void destroy();

		~PipelineLayout();
	};


	// shove the code back inside it
	class ShaderModule {
		LogicalDevice* logical_dev = nullptr;
	public:

		VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		VkShaderModule sh_module = VK_NULL_HANDLE;

	public:
		ErrStack recreate(LogicalDevice* logical_dev, std::vector<char>& code, VkShaderStageFlagBits stage);

		ErrStack setDebugName(std::string name);

		void destroy();

		~ShaderModule();
	};


	class GraphicsPipeline {
		LogicalDevice* logical_dev = nullptr;
	public:	
		
		VkPipeline pipeline = VK_NULL_HANDLE;

	public:
		ErrStack create(LogicalDevice* logical_dev, VkGraphicsPipelineCreateInfo* info);

		ErrStack setDebugName(std::string name);

		void destroy();

		~GraphicsPipeline();
	};


	struct CmdBufferTask
	{
		uint32_t idx;
		VkCommandPool cmd_pool;
		VkCommandBuffer cmd_buff;
		ErrStack err;
	};

	class RenderingComandBuffers {
		LogicalDevice const* logical_dev = nullptr;
	public:
		std::vector<CmdBufferTask> cmd_buff_tasks;

	public:
		ErrStack recreate(LogicalDevice* logical_dev, PhysicalDevice* phys_dev, uint32_t count);

		void destroy();

		~RenderingComandBuffers();
	};


	class Fence {
		LogicalDevice const* logical_dev = nullptr;
	public:
		VkFence fence = VK_NULL_HANDLE;

	public:
		ErrStack create(LogicalDevice* logical_dev, VkFenceCreateFlags flags = 0);

		ErrStack waitAndReset(uint64_t max_wait_time = UINT64_MAX);

		void destroy();

		~Fence();
	};


	class Semaphore {
		LogicalDevice const* logical_dev = nullptr;
	public:
		VkSemaphore semaphore = VK_NULL_HANDLE;
	
	public:
		ErrStack recreate(LogicalDevice* logical_dev);

		void destroy();

		~Semaphore();
	};
}
