#pragma once

// Standard
#include <vector>
#include <array>

// GLM
#include "glm\vec3.hpp"
#include "glm\mat4x4.hpp"

// Mine
#include "ErrorStuff.h"


namespace vks {

	class Instance {
	public:
		std::vector<const char*> validation_layers;
		std::vector<const char*> instance_extensions = { VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
		};

		VkInstance instance = VK_NULL_HANDLE;

		VkDebugUtilsMessageSeverityFlagsEXT debug_msg_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		VkDebugUtilsMessengerEXT callback = VK_NULL_HANDLE;

		PFN_vkGetPhysicalDeviceMemoryProperties2KHR getMemProps2;

	public:
		ErrorStack create();

		void destroy();

		~Instance();
	};


	class Surface {
		Instance const* instance = nullptr;
	public:
		VkSurfaceKHR surface = VK_NULL_HANDLE;
	
	public:
		ErrorStack create(Instance* instance, HINSTANCE hinstance, HWND hwnd);

		void destroy();

		~Surface();
	};


	class PhysicalDevice {
	public:
		VkPhysicalDeviceFeatures phys_dev_features = {};
		std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_MEMORY_BUDGET_EXTENSION_NAME };

		VkPhysicalDevice physical_device;

		uint32_t queue_fam_idx;
		VkPhysicalDeviceProperties phys_dev_props;
		VkPhysicalDeviceMemoryProperties mem_props;

	public:
		PhysicalDevice();
		ErrorStack create(Instance* instance, Surface* surface);
	};


	class LogicalDevice {
	public:
		float queue_priority = 1.0f;

		VkDevice logical_device = VK_NULL_HANDLE;

		VkQueue queue;

		VmaAllocator allocator;

	public:
		ErrorStack create(Instance* instance, PhysicalDevice* phys_dev);

		void destroy();

		~LogicalDevice();
	};


	class Swapchain {
		LogicalDevice const* logical_device = nullptr;

	public:
		uint32_t desired_width;
		uint32_t desired_height;

		VkSurfaceCapabilitiesKHR capabilities;
		VkExtent2D resolution = {800, 600};

		VkSurfaceTransformFlagBitsKHR pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkPresentModeKHR presentation_mode = VK_PRESENT_MODE_FIFO_KHR;
		VkSurfaceFormatKHR surface_format;

		VkSwapchainKHR swapchain = VK_NULL_HANDLE;

		std::vector<VkImage> images;

	public:
		ErrorStack create(Surface* surface, PhysicalDevice* phys_dev, LogicalDevice* logical_dev,
			uint32_t width, uint32_t height);

		void destroy();

		~Swapchain();
	};


	class CommandPool {
		LogicalDevice* logical_dev = nullptr;
	public:

		VkCommandPool cmd_pool = VK_NULL_HANDLE;
	public:
		ErrorStack create(LogicalDevice* logical_dev, PhysicalDevice* phys_dev);

		void destroy();

		~CommandPool();
	};


	class SingleCommandBuffer {
		LogicalDevice* logical_dev = nullptr;
		CommandPool* cmd_pool;

		ErrorStack* err;
	public:
		VkCommandBuffer cmd_buff;
	public:
		SingleCommandBuffer(LogicalDevice* logical_dev, CommandPool* cmd_pool, ErrorStack* r_err);
		~SingleCommandBuffer();
	};


	enum class LoadType {
		STAGING,
		MEMCPY,
		ENUM_NOT_INIT
	};


	class Buffer {
		LogicalDevice const* logical_dev = nullptr;

	public:
		VkBuffer buff = VK_NULL_HANDLE;	
		void* mem;

		VmaAllocation buff_alloc;
		VmaAllocationInfo buff_alloc_info;

		// helper
		LoadType load_type = LoadType::ENUM_NOT_INIT;

	public:
		ErrorStack create(LogicalDevice* logical_dev,
			VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage mem_usage);

		ErrorStack createOrGrowStaging(LogicalDevice* logical_dev, VkDeviceSize min_size,
			VmaMemoryUsage mem_usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY);

		ErrorStack load(LogicalDevice* logical_dev, CommandPool* cmd_pool, Buffer* staging,
			void* data, size_t size);

		/* destroy memory used for buffer */
		void destroy();

		~Buffer();
	};


	struct DesiredImageProps
	{
		VkFormat format;
		VkImageType type;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkImageCreateFlags flags;

		// Not used
		VkImageFormatProperties props_found;
	};

	struct ImageCreateInfo {
		std::vector<DesiredImageProps>* desired_props;
		uint32_t width;
		uint32_t height;
	};

	class Image {
		LogicalDevice const* logical_dev = nullptr;

	public:
		VkImage img = VK_NULL_HANDLE;
		void* mem;

		VmaAllocation alloc;
		VmaAllocationInfo alloc_info;
		LoadType load_type = LoadType::ENUM_NOT_INIT;

		// Properties
		uint32_t width;
		uint32_t height;
		VkFormat format;
		VkImageLayout layout;
	public:
		ErrorStack create(LogicalDevice* logical_dev, VkImageCreateInfo& img_info, VmaMemoryUsage mem_usage);

		ErrorStack create(LogicalDevice* logical_dev, PhysicalDevice* phys_dev,
			ImageCreateInfo& img_info, VmaMemoryUsage mem_usage);

		// load maybe ?

		void destroy();

		~Image();
	};


	ErrorStack changeImageLayout(LogicalDevice* logical_dev, CommandPool* cmd_pool,
		Image* img, VkImageLayout new_layout);

	ErrorStack copyBufferToImage(LogicalDevice* logical_dev, CommandPool* cmd_pool, Buffer* buff, Image* img);


	class ImageView {
		LogicalDevice* logical_dev = nullptr;
	public:

		VkImageView img_view = VK_NULL_HANDLE;
	public:
		ErrorStack create(LogicalDevice* logical_dev, VkImage img, VkFormat format,
			VkImageAspectFlags aspect);

		ErrorStack create(LogicalDevice* logical_dev, Image* img, VkImageAspectFlags aspect);

		void destroy();

		~ImageView();
	};


	class Sampler {
		LogicalDevice const* logical_dev = nullptr;
	public:
		VkSampler sampler = VK_NULL_HANDLE;
	public:
		ErrorStack create(LogicalDevice* logical_dev);

		void destroy();

		~Sampler();
	};


	class Renderpass {
		LogicalDevice* logical_dev = nullptr;
	public:

		VkRenderPass renderpass = VK_NULL_HANDLE;
	public:
		ErrorStack create(LogicalDevice* logical_dev, VkFormat present_format,
			VkFormat depth_format);

		void destroy();

		~Renderpass();
	};


	class Framebuffers {
		LogicalDevice* logical_dev = nullptr;
	public:

		std::vector<VkFramebuffer> frame_buffs;
	public:
		ErrorStack create(LogicalDevice* logical_dev, std::vector<ImageView>& present_views,
			ImageView* depth_view, Renderpass* renderpass, uint32_t width, uint32_t height);

		void destroy();

		~Framebuffers();
	};


	/* Vertex Buffer Types */

	class GPUVertex {
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


	/* Uniform Buffer Types */

	struct GPUUniform {
		alignas(16) glm::vec3 camera_pos;
		alignas(16) glm::vec4 camera_rot;
		alignas(16) glm::mat4 camera_perspective;
		alignas(16) glm::vec3 camera_forward;
	};


	/* Storage Buffer Types */

	struct GPUMeshProperties {
		alignas(16) glm::vec3 pos;
		alignas(16) glm::vec4 rot;
	};


	class DescriptorSetLayout {
		LogicalDevice const* logical_dev = nullptr;
	public:
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		VkDescriptorSetLayout descp_layout = VK_NULL_HANDLE;

	public:
		ErrorStack create(LogicalDevice* logical_dev);

		void destroy();

		~DescriptorSetLayout();
	};


	class DescriptorPool {
		LogicalDevice const* logical_dev = nullptr;
	public:

		VkDescriptorPool descp_pool = VK_NULL_HANDLE;
	public:

		ErrorStack create(LogicalDevice* logical_dev);

		void destroy();

		~DescriptorPool();
	};


	class DescriptorSet {
		LogicalDevice const* logical_dev = nullptr;
	public:

		VkDescriptorSet descp_set = VK_NULL_HANDLE;
		std::vector<VkWriteDescriptorSet> descp_writes;

	public:
		ErrorStack create(LogicalDevice* logical_dev, DescriptorSetLayout* descp_layout,
			DescriptorPool* descp_pool);

		void update(Buffer* uniform_buff, Buffer* storage_buff, Sampler* sampler,
			ImageView* img_view, VkImageLayout img_layout);
	};


	class PipelineLayout {
		LogicalDevice const* logical_dev = nullptr;
	public:

		VkPipelineLayout pipe_layout = VK_NULL_HANDLE;

	public:
		ErrorStack create(LogicalDevice* logical_dev, DescriptorSetLayout* descp_layout);

		void destroy();

		~PipelineLayout();
	};


	class ShaderModule {
		LogicalDevice const* logical_dev = nullptr;
	public:

		VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		VkShaderModule sh_module = VK_NULL_HANDLE;

	public:
		ErrorStack create(LogicalDevice* logical_dev, std::vector<char>& code, VkShaderStageFlagBits stage);

		void destroy();

		~ShaderModule();
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

		ErrorStack create(LogicalDevice* logical_dev, ShaderModule* vertex_module, ShaderModule* frag_module,
			uint32_t width, uint32_t height, PipelineLayout* pipe_layout, Renderpass* renderpass);

		void destroy();

		~GraphicsPipeline();
	};


	struct CmdBufferTask
	{
		uint32_t idx;
		VkCommandPool cmd_pool;
		VkCommandBuffer cmd_buff;
		ErrorStack err;
	};

	class RenderingComandBuffers {
		LogicalDevice const* logical_dev = nullptr;
	public:
		std::vector<CmdBufferTask> cmd_buff_tasks;

		std::vector<VkClearValue> clear_vals;

	public:
		ErrorStack create(LogicalDevice* logical_dev, PhysicalDevice* phys_dev, uint32_t count);

		ErrorStack update(Renderpass* renderpass, Framebuffers* frame_buffs,
			uint32_t width, uint32_t height, PipelineLayout* pipe_layout, GraphicsPipeline* graphics_pipe,
			DescriptorSet* descp_set, Buffer* vertex_buff, uint32_t vertex_count);

		void destroy();

		~RenderingComandBuffers();
	};


	class Fence {
		LogicalDevice const* logical_dev = nullptr;
	public:
		VkFence fence = VK_NULL_HANDLE;

	public:
		ErrorStack create(LogicalDevice* logical_dev, VkFenceCreateFlags flags = 0);

		ErrorStack waitAndReset(uint64_t max_wait_time = UINT64_MAX);

		void destroy();

		~Fence();
	};


	class Semaphore {
		LogicalDevice const* logical_dev = nullptr;
	public:
		VkSemaphore semaphore = VK_NULL_HANDLE;
	
	public:
		ErrorStack create(LogicalDevice* logical_dev);

		void destroy();

		~Semaphore();
	};
}
