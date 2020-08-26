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

	enum class LoadType {
		STAGING,
		MEMCPY,
	};

	//////////////////////////////// New APi //////////////////////////////////////

	class Instance2;
	class PhysicalDevice2;
	class LogicalDevice2;
	class Swapchain2;
	class Fence2;
	class Semaphore2;
	class CommandPool2;
	class Texture;
	class RawBuffer;


	struct SurfaceCreateInfo {
		const void* pNext = NULL;
		VkWin32SurfaceCreateFlagsKHR flags = 0;
		HINSTANCE hinstance;
		HWND hwnd;
	};

	class Surface2 {
	public:
		Instance2* instance;

		VkSurfaceKHR surface = VK_NULL_HANDLE;

	public:
		void destroy();

		~Surface2();
	};


	struct SwapchainCreateInfo {
		const void* pNext = NULL;
		VkSwapchainCreateFlagsKHR flags = 0;
		uint32_t minImageCount = 0;
		VkFormat imageFormat = VK_FORMAT_MAX_ENUM;
		VkColorSpaceKHR imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t imageArrayLayers = 1;
		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkSharingMode imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		std::vector<uint32_t> queue_family_indices;
		VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkBool32 clipped = VK_TRUE;
		VkSwapchainKHR oldSwapchain = NULL;
	};

	class Swapchain2 {
	public:
		LogicalDevice2* dev;

		SwapchainCreateInfo info;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;

		std::vector<VkImage> images;
		std::vector<VkImageView> views;

	public:
		void destroy();

		~Swapchain2();
	};


	struct FenceCreateInfo {
		const void* pNext = NULL;
		VkFenceCreateFlags flags = 0;
	};

	class Fence2 {
	public:
		LogicalDevice2* dev;

		VkFence fence = VK_NULL_HANDLE;

	public:
		ErrStack waitAndReset(uint64_t max_wait_time = UINT64_MAX);

		void destroy();
		~Fence2();
	};


	struct SemaphoreCreateInfo {
		const void* pNext = NULL;
		VkSemaphoreCreateFlags flags = 0;
	};

	class Semaphore2 {
	public:
		LogicalDevice2* dev;

		VkSemaphore semaphore = VK_NULL_HANDLE;

	public:
		void destroy();

		~Semaphore2();
	};


	struct CommandBufferCreateInfo {
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	};

	class CommandBuffer {
	public:
		CommandPool2* pool;

		VkCommandBuffer cmd_buff;
		Fence2 fence;

	public:
		ErrStack beginRecording(VkCommandBufferUsageFlags flags);

		void copyBuffer(RawBuffer* src, RawBuffer* dst);
		void changeTextureLayout(Texture* texture, VkImageLayout new_layout);

		ErrStack finishRecording(VkQueue queue);
	};


	struct CommandPoolCreateInfo {
		VkCommandPoolCreateFlags flags = 0;
		int32_t queueFamilyIndex = -1;
	};

	class CommandPool2 {
	public:
		LogicalDevice2* dev;

		VkCommandPool pool = VK_NULL_HANDLE;

	public:
		ErrStack createCommandBuffer(CommandBufferCreateInfo& info, CommandBuffer& command_buffer);

		void destroy();
		~CommandPool2();
	};


	struct ImageSubresourceRangeCreateInfo {
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		uint32_t baseMipLevel = 0;
		uint32_t levelCount = 1;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 1;
	};

	struct TextureViewCreateInfo {
		const void* pNext = NULL;
		VkImageViewCreateFlags     flags = 0;
		VkImageViewType            viewType = VK_IMAGE_VIEW_TYPE_2D;
		VkComponentMapping         components = {};
		ImageSubresourceRangeCreateInfo sub_res = {};
	};

	class TextureView {
	public:
		Texture* texture;

		VkImageView view = VK_NULL_HANDLE;

	public:
		void destroy();

		~TextureView();
	};


	struct TextureCreateInfo {
		VkImageCreateFlags flags = 0;
		VkImageType imageType = VK_IMAGE_TYPE_2D;
		VkFormat format = VK_FORMAT_MAX_ENUM;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		uint32_t queueFamilyIndexCount = 0;
		const uint32_t* pQueueFamilyIndices = NULL;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	};

	class Texture {
	public:
		LogicalDevice2* dev;

		VkImage img = VK_NULL_HANDLE;
		void* mem;

		VmaAllocation alloc;
		VmaAllocationInfo alloc_info;
		LoadType load_type;

		// Image Properties
		TextureCreateInfo info;
		VkImageLayout current_layout;

	public:
		ErrStack createView(TextureViewCreateInfo& info, TextureView& view);

		ErrStack setDebugName(std::string name);

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
		LogicalDevice2* dev;

		VkBuffer buff = VK_NULL_HANDLE;
		VmaAllocation buff_alloc;
		void* mem;

		// Props
		size_t size;
		BufferCreateInfo info;
		VmaAllocationInfo vma_r_info = {};
		LoadType load_type;

	public:
		ErrStack create(size_t size, BufferCreateInfo& info);
		ErrStack resize(size_t new_size);

		ErrStack loadStagedWait(size_t size, void* data, CommandBuffer& cmd_buff, RawBuffer& staging_buff);

		void loadMemcpy(size_t size, void* data);

		size_t getRequestedSize();
		size_t getAllocatedSize();

		void destroy();
		~RawBuffer();
	};


	template<typename T>
	class ConstBuffer : public RawBuffer {
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
	class VectorBuffer : public RawBuffer {
	public:
		ErrStack resize(size_t count);
		ErrStack load(std::vector<T>& values, RawBuffer& staging_buffer, CommandBuffer* cmd_buff = nullptr);
		size_t getCount();
	};

	template<typename T>
	ErrStack VectorBuffer<T>::resize(size_t count)
	{
		ErrStack err_stack{};

		checkErrStack1(RawBuffer::resize(sizeof(T) * count));

		return err_stack;
	}

	template<typename T>
	ErrStack VectorBuffer<T>::load(std::vector<T>& values, RawBuffer& staging_buffer, CommandBuffer* cmd_buff)
	{
		ErrStack err_stack{};

		size_t load_size = sizeof(T) * values.size();

		if (load_type == LoadType::STAGING) {

			if (cmd_buff == nullptr) {
				checkErrStack1(RawBuffer::loadStagedWait(load_size, values.data(), context->command_buff, staging_buffer));
			}
			else {
				checkErrStack1(RawBuffer::loadStagedWait(load_size, values.data(), cmd_buff, staging_buffer));
			}
		}
		else {
			loadMemcpy(load_size, values.data());
		}

		return err_stack;
	}

	template<typename T>
	size_t VectorBuffer<T>::getCount()
	{
		return RawBuffer::size / sizeof(T);
	}


	struct FramebuffersCreateInfo {
		uint32_t size = 0;
		uint32_t width = 0;
		uint32_t height = 0;

		VkFramebufferCreateFlags flags = 0;
		std::vector<TextureView*> atachments;
		uint32_t layers = 1;
	};

	class Framebuffers {
	public:
		LogicalDevice2* dev;

		std::vector<VkFramebuffer> framebuffs;

		void destroy();
		~Framebuffers();
	};


	struct AtachmentCreateInfo {
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkAttachmentDescriptionFlags flags = 0;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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
		LogicalDevice2* dev;

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
		LogicalDevice2* dev;

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
		LogicalDevice2* dev;

		VkPipelineLayout layout = VK_NULL_HANDLE;

		void destroy();
		~PipelineLayout2();
	};


	class Shader {
	public:
		LogicalDevice2* dev;

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
		float blendConstants[4] = { 0, 0, 0, 0 };
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
		LogicalDevice2* dev;

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
		LogicalDevice2* dev;

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


	struct RenderingCommandBuffersCreateInfo {
		uint32_t size = 0;
		CommandPoolCreateInfo pool_info = {};
		CommandBufferCreateInfo buff_info = {};
	};

	struct RenderingCommandTask {
		uint32_t idx;
		CommandPool2 pool;
		CommandBuffer buff;
		ErrStack err_stack;
	};

	class RenderingCommandBuffers2 {
	public:
		LogicalDevice2* dev;

		std::vector<RenderingCommandTask> tasks;
	};


	struct DeviceQueueCreateInfo {
		const void* pNext = NULL;
		VkDeviceQueueCreateFlags flags = 0;
		uint32_t queueFamilyIndex;
		std::vector<float> queue_priorities = {
			1
		};
	};

	struct ExtensionsSupportInfo {
		std::vector<char*> extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_MEMORY_BUDGET_EXTENSION_NAME
		};
	};

	struct FeaturesSupportInfo {
		VkPhysicalDeviceFeatures features = {};
	};

	struct LogicalDeviceCreateInfo {
		const void* pNext = NULL;
		VkDeviceCreateFlags flags = 0;
		std::vector<DeviceQueueCreateInfo> queue_infos;
		std::vector<char*> layers = {
			"VK_LAYER_LUNARG_standard_validation"
		};
		ExtensionsSupportInfo extensions = {};
		FeaturesSupportInfo features = {};
	};

	class LogicalDevice2 {
	public:
		PhysicalDevice2* phys_dev;

		VkDevice logical_device = VK_NULL_HANDLE;
		VmaAllocator allocator;

		// Default
		uint32_t default_queue_family;
		VkQueue default_queue;
		Swapchain2* default_swapchain = nullptr;

	public:
		ErrStack setDebugName(uint64_t obj, VkObjectType obj_type, std::string name);

		ErrStack createSwapchain(SwapchainCreateInfo& info, Swapchain2& swapchain);

		ErrStack createFence(FenceCreateInfo& info, Fence2& fence);
		ErrStack createSemaphore(SemaphoreCreateInfo& info, Semaphore2& semaphore);
		ErrStack createCommandPool(CommandPoolCreateInfo& info, CommandPool2& command_pool);

		ErrStack createTexture(TextureCreateInfo& info, Texture& texture);
		ErrStack createRawBuffer(size_t size, BufferCreateInfo& info, RawBuffer& raw_buffer);

		template<typename T>
		ErrStack createConstBuffer(T& value, BufferCreateInfo& info, ConstBuffer<T>& const_buffer);

		template<typename T>
		ErrStack createArrayBuffer(size_t count, BufferCreateInfo& info, VectorBuffer<T>& array_buffer);
	
		ErrStack createRenderpass(RenderpassCreateInfo& info, Renderpass2& renderpass);
		ErrStack createDescriptorSetLayout(DescriptorSetLayoutCreateInfo& info, DescriptorSetLayout2& descp_set_layout);
		ErrStack createPipelineLayout(PipelineLayoutCreateInfo& info, PipelineLayout2& pipe_layout);
		ErrStack createVertexShader(std::vector<char>& code, Shader& vertex_shader);
		ErrStack createFragmentShader(std::vector<char>& code, Shader& fragment_shader);
		ErrStack createGraphicsPipeline(GraphicsPipelineCreateInfo& info, GraphicsPipeline2& graphics_pipe);
		void createDescriptorPool(DescriptorPoolCreateInfo& info, DescriptorPool2& descp_pool);
		
		ErrStack createRenderingCommandBuffers(RenderingCommandBuffersCreateInfo& info, RenderingCommandBuffers2& rendering_cmd_buffers);
	
		void destroy();

		~LogicalDevice2();
	};

	template<typename T>
	ErrStack LogicalDevice2::createConstBuffer(T& value, BufferCreateInfo& info, ConstBuffer<T>& const_buffer)
	{
		ErrStack err_stack{};

		const_buffer = {};
		const_buffer.context = this;
		checkErrStack1(const_buffer.create(sizeof(T), info));

		return err_stack;
	}

	template<typename T>
	ErrStack LogicalDevice2::createArrayBuffer(size_t count, BufferCreateInfo& info, VectorBuffer<T>& array_buffer)
	{
		ErrStack err_stack{};

		array_buffer = {};
		array_buffer.context = this;
		checkErrStack1(array_buffer.create(sizeof(T) * count, info));

		return err_stack;
	}


	class PhysicalDevice2 {
	public:
		Instance2* instance;

		VkPhysicalDevice physical_device = VK_NULL_HANDLE;

	public:
		ErrStack getQueueFamilyIdxWithSurfaceSupport(Surface2& surface, uint32_t& r_found_idx);
		ErrStack checkForExtensions(ExtensionsSupportInfo& info);
		ErrStack checkForFeatures(FeaturesSupportInfo& info);
		ErrStack getSurfaceCapabilities(Surface2& surface, VkSurfaceCapabilitiesKHR& capabilities);

		ErrStack createLogicalDevice(LogicalDeviceCreateInfo& info, LogicalDevice2& logical_device);
	};


	struct VulkanApplicationInfo {
		const void* pNext = NULL;
		const char* pApplicationName = "Vulkan aplication name";
		uint32_t applicationVersion = VK_MAKE_VERSION(1, 0, 0);;
		const char* pEngineName = "Vulkan engine name";
		uint32_t engineVersion = VK_MAKE_VERSION(1, 0, 0);
		uint32_t apiVersion = VK_API_VERSION_1_0;
	};

	struct CreateInstanceInfo {
		std::vector<const char*> validation_layers = {
			"VK_LAYER_LUNARG_standard_validation"
		};
		std::vector<const char*> instance_extensions = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		};

		const void* pNext = NULL;
		VkInstanceCreateFlags flags = 0;
		VulkanApplicationInfo app_info = {};

		// Debug
		const void* debug_pnext = NULL;
		VkDebugUtilsMessengerCreateFlagsEXT debug_flags = 0;
		VkDebugUtilsMessageSeverityFlagsEXT debug_msg_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		VkDebugUtilsMessageTypeFlagsEXT debug_msg_type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	};

	class Instance2 {
	public:
		VkInstance instance = VK_NULL_HANDLE;

		VkDebugUtilsMessengerEXT callback = VK_NULL_HANDLE;
		PFN_vkSetDebugUtilsObjectNameEXT set_vkdbg_name_func;

		Surface2* default_surface = nullptr;

	public:
		ErrStack create(CreateInstanceInfo& info);

		ErrStack createWin32Surface(SurfaceCreateInfo& info, Surface2& surface);
		ErrStack getPhysicalDevices(std::vector<PhysicalDevice2>& phys_devs);

		void destroy();
		~Instance2();
	};
}
