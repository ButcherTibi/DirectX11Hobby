#pragma once

// Standard
#include <vector>
#include <array>
#include <variant>

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


// attempt number 3 to domesticate Vulkan 
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


	//////////////////////////////// New APi //////////////////////////////////////

	class Context;
	class RawBuffer;
	class Texture;


	class VulkanFence {
	public:
		Context* context;

		VkFence fence = VK_NULL_HANDLE;

	public:
		ErrStack waitAndReset(uint64_t max_wait_time = UINT64_MAX);

		void destroy();
		~VulkanFence();
	};


	class CommandBuffer {
	public:
		Context* context = nullptr;

		VkCommandBuffer cmd_buff;
		VulkanFence fence;

	public:
		ErrStack beginRecording(VkCommandBufferUsageFlags flags);

		void copyBuffer(RawBuffer* src, RawBuffer* dst);
		void changeTextureLayout(Texture* texture, VkImageLayout new_layout);

		ErrStack finishRecording();
	};


	struct TextureCreateInfo {
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		uint32_t width = 0;
		uint32_t height = 0;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	};

	struct TextureViewCreateInfo {
		VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	};

	class Texture {
	public:
		Context* context = nullptr;

		VkImage img = VK_NULL_HANDLE;
		void* mem;

		VmaAllocation alloc;
		VmaAllocationInfo alloc_info;
		LoadType load_type;

		// Image Properties
		VkFormat format;
		uint32_t width;
		uint32_t height;
		VkImageLayout layout;

		VkImageView view = VK_NULL_HANDLE;

	public:
		ErrStack addView(const TextureViewCreateInfo& info);

		void destroy();
		~Texture();
	};


	struct BufferCreateInfo {
		VkBufferCreateFlags flags = 0;
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		uint32_t queueFamilyIndexCount = 0;
		const uint32_t* pQueueFamilyIndices = NULL;

		VmaMemoryUsage vma_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	};

	class RawBuffer {
	public:
		Context* context;

		VkBuffer buff = VK_NULL_HANDLE;
		VmaAllocation buff_alloc;
		void* mem;

		// Props
		BufferCreateInfo info;
		VmaAllocationInfo vma_r_info = {};
		LoadType load_type;

		ErrStack create(size_t size, BufferCreateInfo& info);
		ErrStack resize(size_t new_size);
		ErrStack loadStagedWait(size_t size, void* data, CommandBuffer& cmd_buff, RawBuffer& staging_buff);
		void loadMemcpy(size_t size, void* data);

		void destroy();
		~RawBuffer();
	};


	template<typename T>
	class ConstBuffer : RawBuffer {
	public:
		ErrStack load(T& value, RawBuffer& staging_buffer, CommandBuffer* cmd_buff = nullptr);
	};

	template<typename T>
	ErrStack ConstBuffer<T>::load(T& value, RawBuffer& staging_buffer, CommandBuffer* cmd_buff)
	{
		ErrStack err_stack{};

		size_t load_size = sizeof(T);

		if (load_type == LoadType::STAGING) {

			if (load_size > staging_buffer.vma_r_info.size) {
				checkErrStack1(staging_buffer.resize(load_size));
			}

			if (cmd_buff == nullptr) {
				checkErrStack1(loadStagedWait(load_size, &value, context->command_buff, staging_buffer));
			}
			else {
				checkErrStack1(loadStagedWait(load_size, &value, cmd_buff, staging_buffer));
			}
		}
		else {
			loadMemcpy(load_size, &value);
		}

		return err_stack;
	}


	template<typename T>
	class ArrayBuffer : RawBuffer {
	public:
		ErrStack load(std::vector<T>& values, RawBuffer& staging_buffer, CommandBuffer* cmd_buff = nullptr);
	};

	template<typename T>
	ErrStack ArrayBuffer<T>::load(std::vector<T>& values, RawBuffer& staging_buffer, CommandBuffer* cmd_buff)
	{
		ErrStack err_stack{};

		size_t load_size = sizeof(T) * values.size();

		if (load_type == LoadType::STAGING) {

			if (load_size > staging_buffer.vma_r_info.size) {
				checkErrStack1(staging_buffer.resize(load_size));
			}

			if (cmd_buff == nullptr) {
				checkErrStack1(loadStagedWait(load_size, values.data(), context->command_buff, staging_buffer));
			}
			else {
				checkErrStack1(loadStagedWait(load_size, values.data(), cmd_buff, staging_buffer));
			}
		}
		else {
			loadMemcpy(load_size, values.data());
		}

		return err_stack;
	}


	struct FramebuffersCreateInfo {
		VkFramebufferCreateFlags flags = 0;
		std::vector<VkImageView> atachments;
		uint32_t layers = 0;
	};

	class Framebuffers {
	public:
		Context* context = nullptr;

		std::vector<VkFramebuffer> framebuffs;

		void destroy();
		~Framebuffers();
	};


	struct AtachmentCreateInfo {
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkAttachmentDescriptionFlags flags = 0;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	};

	struct SubpassCreateInfo {
		VkSubpassDescriptionFlags flags = 0;
		VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
		std::vector<VkAttachmentReference> input_atachs;
		std::vector<VkAttachmentReference> color_atachs;
	};

	struct SubpassDependencyCreateInfo {
		uint32_t srcSubpass = 0;
		uint32_t dstSubpass = 0;
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkAccessFlags srcAccessMask = 0;
		VkAccessFlags dstAccessMask = 0;
		VkDependencyFlags dependencyFlags = 0;
	};

	struct RenderpassCreateInfo {
		VkRenderPassCreateFlags flags = 0;
		std::vector<AtachmentCreateInfo> atach_descps;
		std::vector<VkAttachmentReference> atach_refs;

		std::vector<SubpassCreateInfo> subpasses;
		std::vector<SubpassDependencyCreateInfo> subpass_deps;
	};

	class Renderpass2 {
	public:
		Context* context = nullptr;

		VkRenderPass renderpass = VK_NULL_HANDLE;

	public:
		ErrStack createFramebuffers(FramebuffersCreateInfo& info, Framebuffers& framebuffer);

		void destroy();
		~Renderpass2();
	};


	struct DescriptorSetLayoutBinding {
		uint32_t binding = 0;
		VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		uint32_t descriptorCount = 1;
		VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		VkSampler* pImmutableSamplers = NULL;
	};

	struct DescriptorSetLayoutCreateInfo {
		VkDescriptorSetLayoutCreateFlags flags = 0;
		std::vector<DescriptorSetLayoutBinding> bindings;
	};

	class DescriptorSetLayout2 {
	public:
		Context* context = nullptr;

		std::vector<DescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout layout = VK_NULL_HANDLE;

		void destroy();
		~DescriptorSetLayout2();
	};


	struct PushConstantRange {
		VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		uint32_t offset = 0;
		uint32_t size = 0;
	};

	struct PipelineLayoutCreateInfo {
		VkPipelineLayoutCreateFlags flags = 0;
		std::vector<DescriptorSetLayout2*> layouts;
		std::vector<PushConstantRange> push_const_ranges;
	};

	class PipelineLayout2 {
	public:
		Context* context = nullptr;

		VkPipelineLayout layout = VK_NULL_HANDLE;

		void destroy();
		~PipelineLayout2();
	};


	class Shader {
	public:
		Context* context;

		VkShaderStageFlagBits stage;
		VkShaderModule shader = VK_NULL_HANDLE;

		ErrStack create(std::vector<char>& code);

		void destroy();
		~Shader();
	};


	struct ShaderStageCreteInfo {
		VkPipelineShaderStageCreateFlags flags = 0;
		VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
		VkShaderModule module;
		const char* pName = "main";
		const VkSpecializationInfo* pSpecializationInfo = NULL;
	};

	struct VertexInput {
		VkVertexInputBindingDescription binding;
		std::vector<VkVertexInputAttributeDescription> atributes;
	};

	struct VertexInputCreateInfo {
		VkPipelineVertexInputStateCreateFlags flags = 0;
		VertexInput vertex_input;
	};

	struct InputAssemblyCreateInfo {
		VkPipelineInputAssemblyStateCreateFlags flags = 0;
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkBool32 primitiveRestartEnable = false;
	};

	struct ViewportCreateInfo {
		float x = 0;
		float y = 0;
		float width = 0;
		float height = 0;
		float minDepth = 0;
		float maxDepth = 1;
	};

	struct ScissorsCreateInfo {
		VkOffset2D offset = { 0, 0 };
		VkExtent2D extent = { 0, 0 };
	};

	struct ViewportStateCreateInfo {
		VkPipelineViewportStateCreateFlags flags = 0;
		std::vector<ViewportCreateInfo> viewports;
		std::vector<ScissorsCreateInfo> scissors;
	};

	struct RasterizationCreateInfo {
		VkPipelineRasterizationStateCreateFlags flags = 0;
		VkBool32 depthClampEnable = false;
		VkBool32 rasterizerDiscardEnable = false;
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
		VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
		VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
		VkBool32 depthBiasEnable = false;
		float depthBiasConstantFactor = 0;
		float depthBiasClamp = 0;
		float depthBiasSlopeFactor = 0;
		float lineWidth = 1;
	};

	struct MultisampleCreateInfo {
		VkPipelineMultisampleStateCreateFlags flags = 0;
		VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		VkBool32 sampleShadingEnable = false;
		float minSampleShading = 0;
		VkSampleMask* pSampleMask = NULL;
		VkBool32 alphaToCoverageEnable = false;
		VkBool32 alphaToOneEnable = false;
	};

	struct DepthStencilCreateInfo {
		VkPipelineDepthStencilStateCreateFlags flags = 0;
		VkBool32 depthTestEnable = false;
		VkBool32 depthWriteEnable = false;
		VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
		VkBool32 depthBoundsTestEnable = false;
		VkBool32 stencilTestEnable = false;
		VkStencilOpState front = {};
		VkStencilOpState back = {};
		float minDepthBounds = 0;
		float maxDepthBounds = 1;
	};

	struct ColorBlendAtachmentCreateInfo {
		VkBool32 blendEnable = false;
		VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
		VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
		VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;;
	};

	struct ColorBlendCreateInfo {
		VkPipelineColorBlendStateCreateFlags flags = 0;
		VkBool32 logicOpEnable = false;
		VkLogicOp logicOp = VK_LOGIC_OP_CLEAR;
		std::vector<ColorBlendAtachmentCreateInfo> attachments;
		float blendConstants[4] = { 0, 0, 0, 0};
	};

	struct DynamicStateCreateInfo {
		VkPipelineDynamicStateCreateFlags flags = 0;
		std::vector<VkDynamicState> dynamic_states;
	};

	struct GraphicsPipelineCreateInfo {
		std::vector<ShaderStageCreteInfo> stages;
		VertexInputCreateInfo vertex_input = {};
		InputAssemblyCreateInfo input_assembly = {};
		// TesselationCreateInfo tesselation = {};
		ViewportStateCreateInfo viewport_state = {};
		RasterizationCreateInfo rasterization = {};
		MultisampleCreateInfo multisample = {};
		DepthStencilCreateInfo depth_stencil = {};
		ColorBlendCreateInfo color_blend = {};
		DynamicStateCreateInfo dynamic_state = {};

		Renderpass2* renderpass;
		PipelineLayout2* layout;
		uint32_t subpass = 0;
		VkPipeline basePipelineHandle = NULL;
		int32_t basePipelineIndex = 0;
	};

	class GraphicsPipeline2 {
	public:
		Context* context = nullptr;

	public:
		VkPipeline pipeline = VK_NULL_HANDLE;

		void destroy();
		~GraphicsPipeline2();
	};


	class DescriptorPool2;

	struct DescriptorSetImageInfo {
		VkSampler        sampler = NULL;
		VkImageView      imageView;
		VkImageLayout    imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	};

	struct DescriptorSetBufferInfo {
		VkBuffer        buffer;
		VkDeviceSize    offset = 0;
		VkDeviceSize    range = VK_WHOLE_SIZE;
	};

	struct DescriptorSetWriteInfo {
		uint32_t dstBinding = 0;
		uint32_t dstArrayElement = 0;
		uint32_t descriptorCount = 1;
		VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		std::variant<DescriptorSetImageInfo, DescriptorSetBufferInfo> resource;

		VkDescriptorImageInfo _image_info;
		VkDescriptorBufferInfo _buff_info;
	};

	class DescriptorSet2 {
	public:
		DescriptorPool2* pool;

		VkDescriptorSet set;

	public:
		void update(std::vector<DescriptorSetWriteInfo> writes);
	};


	struct DescriptorPoolCreateInfo {
		VkDescriptorPoolCreateFlags flags = 0;
	};

	class DescriptorPool2 {
	public:
		Context* context = nullptr;

		VkDescriptorPoolCreateFlags flags;
		std::vector<DescriptorSetLayout2*> layouts;

		std::vector<VkDescriptorPoolSize> sizes;
		VkDescriptorPool pool;

		std::vector<DescriptorSet2> sets;

	public:
		void createDescriptorSet(DescriptorSetLayout2* layout, DescriptorSet2& set);
		ErrStack allocate();

		void destroy();
		~DescriptorPool2();
	};


	class Context {
	public:
		Instance instance;
		Surface surface;
		PhysicalDevice phys_dev;
		LogicalDevice logical_dev;

		CommandPool command_pool;
		CommandBuffer command_buff;

		uint32_t width;
		uint32_t height;
		Swapchain swapchain;

	public:
		ErrStack create(HWND hwnd, HINSTANCE hinstance);

		ErrStack getPhysicalSurfaceResolution(uint32_t& width, uint32_t& height);

		ErrStack createFence(VulkanFence& fence, VkFenceCreateFlags flags = 0);
		ErrStack createCommandBuffer(CommandBuffer& command_buffer);

		ErrStack createTexture(TextureCreateInfo& info, Texture& texture);
		ErrStack createRawBuffer();
		ErrStack createArrayBuffer();
		ErrStack createConstBuffer();

		ErrStack createRenderpass(RenderpassCreateInfo& info, Renderpass2& renderpass);

		ErrStack createDescriptorSetLayout(DescriptorSetLayoutCreateInfo& info, DescriptorSetLayout2& descp_set_layout);
		ErrStack createPipelineLayout(PipelineLayoutCreateInfo& info, PipelineLayout2& pipe_layout);
		ErrStack createVertexShader(std::vector<char>& code, Shader& vertex_shader);
		ErrStack createFragmentShader(std::vector<char>& code, Shader& fragment_shader);
		ErrStack createGraphicsPipeline(GraphicsPipelineCreateInfo& info, GraphicsPipeline2& graphics_pipe);

		void createDescriptorPool(DescriptorPoolCreateInfo& info, DescriptorPool2& descp_pool);
	};
}
