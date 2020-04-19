#pragma once

// Standard
#include <vector>
#include <array>

// GLM
#include "glm\vec3.hpp"
#include "glm\mat4x4.hpp"

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
		VkSurfaceCapabilitiesKHR capabilities;
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


	enum class LoadType {
		STAGING,
		MEMCPY,
		ENUM_NOT_INIT
	};


	class Buffer {
		LogicalDevice* logical_dev = nullptr;

	public:
		VkBuffer buff = VK_NULL_HANDLE;	
		void* mem;

		VmaAllocation buff_alloc;
		VmaAllocationInfo buff_alloc_info;

		LoadType load_type = LoadType::ENUM_NOT_INIT;
		size_t scheduled_load_size = 0;

	public:
		ErrStack create(LogicalDevice* logical_dev,
			VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage mem_usage);

		// add param for if reallocated
		ErrStack recreate(LogicalDevice* logical_dev, 
			VkDeviceSize min_size, VkBufferUsageFlags usage, VmaMemoryUsage mem_usage);

		ErrStack recreateStaging(LogicalDevice* logical_dev, VkDeviceSize min_size);

		ErrStack load(CommandPool* cmd_pool, Buffer* staging,
			void* data, size_t size);

		void scheduleLoad(size_t offset, Buffer* staging, void* data, size_t size);

		ErrStack flush(CommandPool* cmd_pool, Buffer* staging);

		/* destroy memory used for buffer */
		void destroy();

		~Buffer();

		ErrStack setDebugName(std::string name);
	};


	struct DesiredImageProps
	{
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkImageType type = VK_IMAGE_TYPE_2D;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage;

		// Not used
		VkImageFormatProperties props_found;
	};

	struct ImageCreateInfo {
		std::vector<DesiredImageProps>* desired_props;
		uint32_t width;
		uint32_t height;
		uint32_t mip_levels = 1;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

		// View
		VkImageAspectFlags aspect;
	};

	class Image {
		LogicalDevice* logical_dev = nullptr;

	public:
		VkImage img = VK_NULL_HANDLE;
		void* mem;

		VmaAllocation alloc;
		VmaAllocationInfo alloc_info;
		LoadType load_type = LoadType::ENUM_NOT_INIT;

		// Image Properties
		uint32_t width;
		uint32_t height;
		VkFormat format;
		VkImageLayout layout;
		uint32_t mip_lvl;
		uint32_t samples;

		// View
		VkImageView img_view = VK_NULL_HANDLE;

	private:
		ErrStack copyBufferToImage(CommandPool* cmd_pool, Buffer* buff);

	public:
		ErrStack recreate(LogicalDevice* logical_dev, PhysicalDevice* phys_dev,
			ImageCreateInfo& img_info, VmaMemoryUsage mem_usage);

		ErrStack setDebugName(std::string name);

		ErrStack changeImageLayout(CommandPool* cmd_pool, VkImageLayout new_layout);

		ErrStack load(void* colors, size_t size, CommandPool* cmd_pool, Buffer* staging_buff,
			VkImageLayout layout_after_load);

		void destroy();

		~Image();	
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
		ErrStack create(LogicalDevice* logical_dev, PhysicalDevice* phys_dev, VkFormat present_format,
			VkFormat depth_format);

		void destroy();

		~Renderpass();
	};


	struct FrameBufferCreateInfo {
		Image* g3d_color_MSAA_img = nullptr;
		Image* g3d_depth_img = nullptr;
		Image* g3d_color_resolve_img = nullptr;
		Image* ui_color_img = nullptr;
		Swapchain* swapchain = nullptr;
	};

	class Framebuffers {
		LogicalDevice* logical_dev = nullptr;
	public:

		std::vector<VkFramebuffer> frame_buffs;
	public:
		ErrStack create(LogicalDevice* logical_dev, FrameBufferCreateInfo& info,
			Renderpass* renderpass, uint32_t width, uint32_t height);

		void destroy();

		~Framebuffers();
	};


	struct DescriptorWrite {
		VkDescriptorImageInfo* img_info = NULL;  // layout must be the one at rendertime
		VkDescriptorBufferInfo* buff_info = NULL;

		uint32_t dstBinding;
		uint32_t dstArrayElement;
		uint32_t descriptorCount;
		VkDescriptorType descriptorType;
	};

	class Descriptor {
		LogicalDevice* logical_dev = nullptr;

	public:

		VkDescriptorSetLayout descp_layout = VK_NULL_HANDLE;
		VkDescriptorPool descp_pool = VK_NULL_HANDLE;
		VkDescriptorSet descp_set = VK_NULL_HANDLE;

		std::vector<VkWriteDescriptorSet> writes;

	public:
		ErrStack create(LogicalDevice* logical_dev, std::vector<VkDescriptorSetLayoutBinding>& bindings);

		ErrStack setDebugName(std::string name);

		void update(std::vector<DescriptorWrite>& descp_writes);

		void destroyLayout();
		void destroyPool();

		~Descriptor();
	};


	class PipelineLayout {
		LogicalDevice const* logical_dev = nullptr;
	public:

		VkPipelineLayout pipe_layout = VK_NULL_HANDLE;

	public:
		ErrStack create(LogicalDevice* logical_dev, Descriptor* descp);
		ErrStack create(LogicalDevice* logical_dev);

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

	struct GraphicsPipelineInfo {
		// Shaders
		std::vector<ShaderModule*> shader_modules;

		// Vertex Input
		VkVertexInputBindingDescription vertex_input_binding_descp = {};
		std::vector<VkVertexInputAttributeDescription> vertex_input_atribute_descp;

		VkPipelineVertexInputStateCreateInfo vert_input_stage_info = {};

		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};

		// Tesselation

		// Viewport state 
		VkViewport viewport = {};
		VkRect2D scissor = {};
		VkPipelineViewportStateCreateInfo viewport_state_info = {};

		// Rasterization state
		VkPipelineRasterizationStateCreateInfo raster_state_info = {};

		// MultiSample
		VkPipelineMultisampleStateCreateInfo multisample_state_info = {};

		// DepthSample
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};

		// Color Blending state
		std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments;
		VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
	};

	class GraphicsPipeline {
		LogicalDevice const* logical_dev = nullptr;
	public:	
		// Shader Stages
		std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

		// Vertex Input
		VkVertexInputBindingDescription vertex_input_binding_descp = {};
		std::vector<VkVertexInputAttributeDescription> vertex_input_atribute_descp;

		VkPipelineVertexInputStateCreateInfo vert_input_stage_info = {};

		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};

		// Viewport state 
		VkViewport viewport = {};
		VkRect2D scissor = {};
		VkPipelineViewportStateCreateInfo viewport_state_info = {};

		// Rasterization state
		VkPipelineRasterizationStateCreateInfo raster_state_info = {};

		// MultiSample
		VkPipelineMultisampleStateCreateInfo multisample_state_info = {};

		// DepthSample
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};

		// Color Blending state
		std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments;
		VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
		

		VkPipeline pipeline = VK_NULL_HANDLE;

	public:
		GraphicsPipeline();
		
		void setToDefault();

		// Helpers
		void configureFor3D();
		void configureForUserInterface();

		ErrStack recreate(LogicalDevice* logical_dev, ShaderModule* vertex_module, ShaderModule* frag_module,
			uint32_t width, uint32_t height,
			PipelineLayout* pipe_layout, Renderpass* renderpass, uint32_t subpass_idx);

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

	struct RenderingCmdBuffsUpdateInfo {
		Renderpass* renderpass;
		Framebuffers* frame_buffs;
		uint32_t width;
		uint32_t height;

		// 3D
		Descriptor* g3d_descp;
		PipelineLayout* g3d_pipe_layout;
		GraphicsPipeline* g3d_pipe;

		Buffer* g3d_vertex_buff;
		uint32_t g3d_vertex_count;

		// UI
		Descriptor* ui_descp;
		PipelineLayout* ui_pipe_layout;
		GraphicsPipeline* ui_pipe;

		std::vector<UI_DrawBatch>* ui_draw_batches;
		Buffer* ui_vertex_buff;
		Buffer* ui_storage_buff;

		// Compose
		Descriptor* compose_descp;
		PipelineLayout* compose_pipe_layout;
		GraphicsPipeline* compose_pipe;
	};

	class RenderingComandBuffers {
		LogicalDevice const* logical_dev = nullptr;
	public:
		std::vector<CmdBufferTask> cmd_buff_tasks;

	public:
		ErrStack recreate(LogicalDevice* logical_dev, PhysicalDevice* phys_dev, uint32_t count);

		ErrStack update(const RenderingCmdBuffsUpdateInfo& info);

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
		ErrStack create(LogicalDevice* logical_dev);

		void destroy();

		~Semaphore();
	};
}
