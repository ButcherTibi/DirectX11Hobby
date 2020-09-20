#pragma once

#include "pch.h"


namespace vkw {

	enum class LoadType {
		STAGING,
		MEMCPY,
	};

	class Instance;
	class VulkanDevice;
	class Surface;
	class Image;


	struct ImageSubResourceRange {
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		uint32_t baseMipLevel = 0;
		uint32_t levelCount = 1;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 1;
	};

	struct ImageViewCreateInfo {
		const void* pNext = NULL;
		VkImageViewCreateFlags flags = 0;
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
		VkComponentMapping components = {};
		ImageSubResourceRange subres_range;
	};

	class ImageView {
	public:
		Image* image = nullptr;

		ImageViewCreateInfo info;

		VkImageView view = VK_NULL_HANDLE;

	public:
		nui::ErrStack load(void* data, size_t size, VkImageLayout final_layout = VK_IMAGE_LAYOUT_MAX_ENUM);

		~ImageView();
	};


	struct ImageCreateInfo {
		VkImageCreateFlags flags = 0;
		VkImageType imageType = VK_IMAGE_TYPE_2D;
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	};

	class Image {
	public:
		VulkanDevice* dev;

		VkImage img = VK_NULL_HANDLE;
		VkDeviceMemory mem;
		void* mapped_mem;

		VkMemoryPropertyFlags mem_props;
		VkDeviceSize alloc_size;
		LoadType load_type;

		// Image Properties
		VkFormat format;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		VkSampleCountFlagBits samples;
		VkImageLayout current_layout;

	public:
		nui::ErrStack create(VulkanDevice* dev, ImageCreateInfo& info);

		nui::ErrStack createView(ImageViewCreateInfo& info, ImageView& view);

		void destroy();
		~Image();
	};


	struct SurfaceCreateInfo {
		VkSwapchainCreateFlagsKHR swapchain_flags = 0;
		uint32_t imageArrayLayers = 1;
		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkBool32 clipped = true;
	};

	class Surface {
	public:
		VulkanDevice* dev = nullptr;

		// Info
		SurfaceCreateInfo info;

		VkSurfaceKHR surface = VK_NULL_HANDLE;

		uint32_t minImageCount;
		VkFormat imageFormat;
		VkColorSpaceKHR imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		uint32_t width;
		uint32_t height;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;

		std::vector<VkImage> swapchain_images;
		std::vector<VkImageView> swapchain_views;
		VkImageLayout swapchain_img_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	public:
		nui::ErrStack getSurfaceCapabilities(VkSurfaceCapabilitiesKHR& capabilities);

		nui::ErrStack createSwapchain();
		nui::ErrStack recreateSwapchain();
		void destroySwapchain();
	};


	struct BufferCreateInfo {
		VkBufferCreateFlags flags = 0;
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	};

	class Buffer {
	public:
		VulkanDevice* dev = nullptr;

		VkBuffer buff = VK_NULL_HANDLE;
		VkDeviceMemory mem;
		void* mapped_mem;

		// Props
		VkBufferCreateFlags flags = 0;
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags mem_props;
		VkDeviceSize size;

		LoadType load_type;

	public:
		nui::ErrStack load(void* data, size_t size);

		void destroy();

		~Buffer();
	};


	struct SamplerCreateInfo {
		VkSamplerCreateFlags    flags = 0;
		VkFilter                magFilter = VK_FILTER_NEAREST;
		VkFilter                minFilter = VK_FILTER_NEAREST;
		VkSamplerMipmapMode     mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		VkSamplerAddressMode    addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		VkSamplerAddressMode    addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		VkSamplerAddressMode    addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		float                   mipLodBias = 0.0f;
		VkBool32                anisotropyEnable = VK_FALSE;
		float                   maxAnisotropy = 16.0f;
		VkBool32                compareEnable = VK_FALSE;
		VkCompareOp             compareOp = VK_COMPARE_OP_LESS;
		float                   minLod = 0.0f;
		float                   maxLod = 0.0f;
		VkBorderColor           borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		VkBool32                unnormalizedCoordinates = VK_FALSE;
	};

	class Sampler {
	public:
		VulkanDevice* dev = nullptr;

		VkSampler sampler = VK_NULL_HANDLE;

	public:
		~Sampler();
	};


	class VertexInput {
	public:
		VkVertexInputBindingDescription binding = {
			0, 0xFFFF'FFFF, VK_VERTEX_INPUT_RATE_VERTEX
		};
		std::vector<VkVertexInputAttributeDescription> atributes;

	public:
		void addAtribute(VkFormat format, uint32_t offset);
	};


	class Shader {
	public:
		VulkanDevice* dev;

		VkShaderStageFlagBits stage;
		VkShaderModule shader = VK_NULL_HANDLE;

	public:
		nui::ErrStack create(VulkanDevice* dev, std::vector<char>& spirv, VkShaderStageFlagBits stage);
		~Shader();
	};


	void clearColorFloatValue(VkClearValue& clear_value,
		float r = 0, float g = 0, float b = 0, float a = 0);

	void clearColorUIntValue(VkClearValue& clear_value,
		uint32_t r = 0, uint32_t g = 0, uint32_t b = 0, uint32_t a = 0);


	struct FramebufferCreateInfo {
		std::vector<ImageView*> attachments;
		uint32_t framebuffer_count = 0;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t layers = 1;
	};

	class Framebuffer {
	public:
		VulkanDevice* dev;

		uint32_t width;
		uint32_t height;

		std::vector<VkFramebuffer> framebuffs;

	public:

		~Framebuffer();
	};


	struct ReadAttachmentInfo {
		ImageView* example_view = nullptr;

		// Renderpass
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;
		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_GENERAL;
		VkImageLayout final_fayout = VK_IMAGE_LAYOUT_GENERAL;

		// Descriptor Layout
		uint32_t set = 0;
		uint32_t binding = 0;
		uint32_t descriptor_count = 1;
		VkShaderStageFlags stages = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::string name;
	};

	struct WriteAttachmentInfo {
		ImageView* example_view = nullptr;

		// Renderpass
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_GENERAL;
		VkImageLayout final_layout = VK_IMAGE_LAYOUT_GENERAL;

		// Color Blend Atachment
		bool blendEnable = false;
		VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
		VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
		VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	};

	struct PresentAttachmentInfo {
		// Renderpass
		VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;

		// Color Blend Atachment
		bool blendEnable = false;
		VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
		VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
		VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	};

	struct ReadWriteAttachment {
		ImageView* example_view = nullptr;

		// Renderpass
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkImageLayout final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Descriptor Layout
		uint32_t set = 0;
		uint32_t binding = 0;
		uint32_t descriptor_count = 1;
		VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT;

		// Color Blend Atachment
		bool blendEnable = false;
		VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
		VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
		VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	};

	struct StorageBufferBinding {
		uint32_t set = 0;
		uint32_t binding = 0;
		uint32_t descriptor_count = 1;
		VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT;
	};

	struct UniformBufferBinding {
		uint32_t set = 0;
		uint32_t binding = 0;
		uint32_t descriptor_count = 1;
		VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT;
	};

	struct CombinedImageSamplerBinding {
		uint32_t set = 0;
		uint32_t binding = 0;
		uint32_t descriptor_count = 1;
		VkShaderStageFlags stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	};

	struct InputAssemblyState {
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		bool primitiveRestartEnable = false;
	};

	struct Viewport {
		float x = 0;
		float y = 0;
		float width = 0;
		float height = 0;
		float minDepth = 0;
		float maxDepth = 1;
	};

	struct Scissor {
		VkOffset2D offset = { 0, 0 };
		VkExtent2D extent = { 0, 0 };
	};

	struct ViewportState {
		std::vector<Viewport> viewports;
		std::vector<Scissor> scissors;
	};

	struct RasterizationState {
		bool depthClampEnable = false;
		bool rasterizerDiscardEnable = false;
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
		VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
		VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
		VkBool32 depthBiasEnable = false;
		float depthBiasConstantFactor = 0;
		float depthBiasClamp = 0;
		float depthBiasSlopeFactor = 0;
		float lineWidth = 1;
	};

	struct MultisampleState {
		VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		bool sampleShadingEnable = false;
		float minSampleShading = 0;
		std::vector<VkSampleMask> sample_mask;
		bool alphaToCoverageEnable = false;
		bool alphaToOneEnable = false;
	};

	struct DepthStencilState {
		bool depthTestEnable = false;
		bool depthWriteEnable = false;
		VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
		bool depthBoundsTestEnable = false;
		bool stencilTestEnable = false;
		VkStencilOpState front;
		VkStencilOpState back;
		float minDepthBounds = 0;
		float maxDepthBounds = 1;
	};

	struct ColorBlendState {
		bool logicOpEnable = false;
		VkLogicOp logicOp = VK_LOGIC_OP_AND;
		float blendConstants[4] = { 0, 0, 0, 0 };
	};

	struct DescriptorLayout {
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout layout;

		std::string name;
	};

	struct UpdateBufferDescriptor {
		uint32_t dst_array_element = 0;
		uint32_t descriptor_count = 1;

		// Resource
		Buffer* buffer;
		VkDeviceSize offset = 0;
		VkDeviceSize range = VK_WHOLE_SIZE;
	};

	struct InputAttachmentDescriptor {
		uint32_t dst_array_element = 0;
		uint32_t descriptor_count = 1;

		// Resource
		ImageView* view;
	};

	struct CombinedImageSamplerDescriptor {
		uint32_t dst_array_element = 0;
		uint32_t descriptor_count = 1;

		// Resource
		ImageView* view;
		Sampler* sampler;
	};

	class Drawpass {
	public:
		VulkanDevice* dev;

		// Renderpass
		std::vector<VkAttachmentDescription> atach_descps;
		std::vector<VkAttachmentReference> atach_refs;
		std::vector<VkAttachmentReference> input_atachs;
		std::vector<VkAttachmentReference> color_atachs;
		VkRenderPass renderpass = VK_NULL_HANDLE;

		// Descriptor Set Layout
		std::vector<DescriptorLayout> layouts;

		// Descriptor Pool
		std::vector<VkDescriptorPoolSize> descp_sizes;
		VkDescriptorPool descp_pool;

		// Pipeline Layout
		VkPipelineLayout pipe_layout;

		// Pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaders;
		std::vector<VkPipelineColorBlendAttachmentState> blend_atachs;
		VkPipeline pipeline;

		// Command Renderpass
		std::vector<VkClearValue> clear_values;

		// Builded
		std::vector<VkDescriptorSetLayout> descp_layouts;
		std::vector<VkDescriptorSet> descp_sets;

	public:
		// Internal
		void addDescriptorTypeToPool(VkDescriptorType type, uint32_t count);

	public:
		// Renderpass Attachements
		nui::ErrStack addReadColorAttachment(ReadAttachmentInfo& info);

		nui::ErrStack addWriteColorAttachment(WriteAttachmentInfo& info);

		nui::ErrStack addPresentAttachment(PresentAttachmentInfo& info);

		nui::ErrStack addReadWriteColorAttachment(ReadWriteAttachment& info);

		nui::ErrStack bindStorageBuffer(StorageBufferBinding& info);

		nui::ErrStack bindUniformBuffer(UniformBufferBinding& info);

		nui::ErrStack bindCombinedImageSampler(CombinedImageSamplerBinding& info);

		// Vertex Input
		std::vector<VertexInput> vertex_inputs;

		// Input Assembly
		InputAssemblyState input_assembly_state;

		// Vertex Shader
		void setVertexShader(Shader& vertex_shader);

		// Rasterization
		ViewportState viewport_state;
		RasterizationState rasterization_state;

		// Fragment Shader
		void setFragmentShader(Shader& fragment_shader);
		MultisampleState multisample_state;
		DepthStencilState depth_stencil_state;

		// Color Blend
		ColorBlendState color_blend_state;

		// Dynamic State
		std::vector<VkDynamicState> dynamic_state;

	public:
		nui::ErrStack build();

		nui::ErrStack createFramebuffer(FramebufferCreateInfo& info, Framebuffer& frame_buff);

		void updateStorageBufferDescriptor(uint32_t set, uint32_t binding, UpdateBufferDescriptor& info);
		void updateUniformBufferDescriptor(uint32_t set, uint32_t binding, UpdateBufferDescriptor& info);
		void updateInputAttachmentDescriptor(uint32_t set, uint32_t binding, InputAttachmentDescriptor& info);
		void updateCombinedImageSamplerDescriptor(uint32_t set, uint32_t binding, CombinedImageSamplerDescriptor& info);

		~Drawpass();
	};


	class Semaphore {
	public:
		VulkanDevice* dev = nullptr;

		VkSemaphore semaphore;

	public:
		~Semaphore();
	};


	class Fence {
	public:
		VulkanDevice* dev = nullptr;

		VkFence fence;

	public:
		~Fence();
	};


	struct ImageBarrier {
		ImageView* view;

		VkImageLayout new_layout = VK_IMAGE_LAYOUT_MAX_ENUM;
		VkAccessFlags wait_for_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		VkAccessFlags wait_at_access = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	};

	struct CustomImageBarrier {
		Image* img;

		VkImageLayout new_layout = VK_IMAGE_LAYOUT_MAX_ENUM;
		VkAccessFlags wait_for_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		VkAccessFlags wait_at_access = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		uint32_t baseMipLevel = 0;
		uint32_t levelCount = 1;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 1;
	};

	struct SurfaceBarrier {
		VkImageLayout new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAccessFlags wait_for_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		VkAccessFlags wait_at_access = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	};

	struct CommandTask {
		uint32_t idx;
		VkCommandPool pool;
		VkCommandBuffer buff;
	};

	struct CommandListCreateInfo {
		VkCommandPoolCreateFlags cmd_pool_flags = 0;

		Surface* surface = nullptr;
	};

	class CommandList {
	public:
		VulkanDevice* dev = nullptr;
		Surface* surface;
	
		std::vector<CommandTask> tasks;

		Semaphore swapchain_img_acquired;
		Semaphore execution_finished;
		Fence execution_finished_fence;

	public:
		nui::ErrStack beginRecording();

		void cmdCopyBufferToBuffer(Buffer& src_buff, Buffer& dst_buff, size_t copy_size);

		void cmdCopyBufferToImage(Buffer& src_buff, ImageView& dst_view);

		void cmdCopyImageToImage(ImageView& src_view, ImageView& dst_view);

		// Pipeline Barriers where same subresource is assumed
		void cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
			size_t image_barriers_count, ImageBarrier* image_barriers);

		void cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
			std::vector<ImageBarrier>& image_barriers);

		void cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
			ImageBarrier& image_barrier);

		void cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
			ImageBarrier& image_barrier_0, ImageBarrier& image_barrier_1);

		void cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkAccessFlags wait_for_access,
			VkPipelineStageFlags wait_at_stage, VkAccessFlags wait_at_access,
			size_t images_count, ImageView** images);

		// Custom Pipeline Barriers
		void cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
			size_t image_barriers_count, CustomImageBarrier* image_barriers);

		void cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
			std::vector<CustomImageBarrier>& image_barriers);

		void cmdPipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
			CustomImageBarrier& image_barrier);

		// Pipeline Barrier for surface
		void cmdSurfacePipelineBarrier(VkPipelineStageFlags wait_for_stage, VkPipelineStageFlags wait_at_stage,
			SurfaceBarrier& barrier);

		void cmdClearFloatImage(ImageView& view, float r = 0, float g = 0, float b = 0, float a = 0);

		void cmdClearUIntImage(ImageView& view, uint32_t rgba = 0);

		void cmdClearSurface(float r = 0, float g = 0, float b = 0, float a = 0);

		void cmdCopyImageToSurface(ImageView& view);

		void cmdBeginRenderpass(Drawpass& drawpass, Framebuffer& frame_buffer);

		void cmdEndRenderpass();

		void cmdBindVertexBuffer(Buffer& vertex_buff);
		void cmdBindVertexBuffers(Buffer& vertex_buff_0, Buffer& vertex_buff_1);

		void cmdBindIndexBuffer(Buffer& index_buff, VkIndexType index_type = VK_INDEX_TYPE_UINT32);

		void cmdDraw(uint32_t vertex_count, uint32_t start_idx = 0);

		void cmdDrawIndexedInstanced(uint32_t index_count, uint32_t instance_count,
			uint32_t first_index, uint32_t first_instance);

		nui::ErrStack endRecording();

		nui::ErrStack run();

		nui::ErrStack waitForExecution(uint64_t timeout = 0xFFFFFFFF'FFFFFFFF);

		nui::ErrStack finish(uint64_t timeout = 0xFFFFFFFF'FFFFFFFF);

		void destroy();

		~CommandList();
	};


	struct DeviceCreateInfo {
		// Surface
		void* surface_pNext = NULL;
		VkWin32SurfaceCreateFlagsKHR surface_flags = 0;
		HINSTANCE hinstance;
		HWND hwnd;

		VkSwapchainCreateFlagsKHR swapchain_flags = 0;
		uint32_t imageArrayLayers = 1;
		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkBool32 clipped = true;

		// Physical Device
		VkQueueFlags queue_flags = VK_QUEUE_GRAPHICS_BIT;
		std::vector<const char*> extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_MEMORY_BUDGET_EXTENSION_NAME
		};
		VkPhysicalDeviceFeatures features = {};

		// Logical Device
		void* device_pNext = NULL;
		VkDeviceCreateFlags device_flags = 0;
		std::vector<const char*> layers = {
			"VK_LAYER_LUNARG_standard_validation"
		};
	};

	class VulkanDevice {
	public:
		Instance* inst;

		VkPhysicalDevice phys_dev = VK_NULL_HANDLE;
		VkDevice logical_dev = VK_NULL_HANDLE;

		PFN_vkSetDebugUtilsObjectNameEXT set_vkdbg_name_func = VK_NULL_HANDLE;

		VkPhysicalDeviceMemoryProperties phys_mem_props;
		int32_t queue_fam_idx = -1;
		VkQueue queue;

		Surface surface;

		Buffer staging_buff;
		CommandList* cmd_list;

	public:
		nui::ErrStack recreateSwapchain();

		nui::ErrStack createImage(ImageCreateInfo& info, Image& texture);
		void createBuffer(BufferCreateInfo& info, Buffer& buffer);
		nui::ErrStack createSampler(SamplerCreateInfo& info, Sampler& sampler);
		nui::ErrStack createShader(std::vector<char>& spirv, VkShaderStageFlagBits stage, Shader& shader);

		void createDrawpass(Drawpass& drawpass);
		nui::ErrStack createCommandList(CommandListCreateInfo& info, CommandList& cmd_list);
		
		// Internal
		nui::ErrStack findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t& mem_type);
		nui::ErrStack createSemaphore(Semaphore& semaphore);
		nui::ErrStack createFence(VkFenceCreateFlags flags, Fence& fence);

		nui::ErrStack setDebugName(uint64_t vulkan_obj_handle, VkObjectType vulkan_obj_type, std::string& name);

		~VulkanDevice();
	};


	struct VulkanAplicationInfo {
		const void* pNext = NULL;
		const char* pApplicationName = "Vulkan Application Name";
		uint32_t applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		const char* pEngineName = "Vulkan Engine Name";
		uint32_t engineVersion = VK_MAKE_VERSION(1, 0, 0);
		uint32_t apiVersion = VK_MAKE_VERSION(1, 0, 0);
	};

	struct InstanceCreateInfo {
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
		VulkanAplicationInfo app_info = {};

		const void* debug_pnext = NULL;
		VkDebugUtilsMessengerCreateFlagsEXT debug_flags = 0;
		VkDebugUtilsMessageSeverityFlagsEXT debug_msg_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		VkDebugUtilsMessageTypeFlagsEXT debug_msg_type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	};

	class Instance {
	public:
		VkInstance instance = VK_NULL_HANDLE;

		VkDebugUtilsMessengerEXT callback = VK_NULL_HANDLE;

	public:
		nui::ErrStack create(InstanceCreateInfo& info);

		nui::ErrStack createDevice(DeviceCreateInfo& info, VulkanDevice& device);

		~Instance();
	};
}

//
//ErrStack Window::draw()
//{
//	ErrStack err_stack;
//
//	if (!rendering_configured) {
//
//		rendering_configured = true;
//
//		// Characters Vertex Buffer
//		{
//			vkw::BufferCreateInfo info;
//			info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//			dev.createBuffer(info, chars_vbuff);
//		}
//
//		// Characters Index Buffer
//		{
//			vkw::BufferCreateInfo info;
//			info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//			dev.createBuffer(info, chars_idxbuff);
//		}
//
//		// Character Instance Buffer
//		{
//			vkw::BufferCreateInfo info;
//			info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//			dev.createBuffer(info, chars_instabuff);
//		}
//
//		// Wrap Vertex Buffer
//		{
//			vkw::BufferCreateInfo info;
//			info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//			dev.createBuffer(info, wrap_vbuff);
//		}
//
//		// Wrap Index Buffer
//		{
//			vkw::BufferCreateInfo info;
//			info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//			dev.createBuffer(info, wrap_idxbuff);
//		}
//
//		// Wrap Instance Buffer
//		{
//			vkw::BufferCreateInfo info;
//			info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//			dev.createBuffer(info, wrap_instabuff);
//		}
//
//		// Character Uniform Buffer
//		{
//			vkw::BufferCreateInfo info;
//			info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//			dev.createBuffer(info, text_ubuff);
//
//			common_uniform.screen_size.x = (float)dev.surface.width;
//			common_uniform.screen_size.y = (float)dev.surface.height;
//			checkErrStack1(text_ubuff.load(&common_uniform, sizeof(GPU_CommonsUniform)));
//		}
//
//		// Composition Image
//		{
//			vkw::ImageCreateInfo info;
//			info.format = dev.surface.imageFormat;
//			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
//			info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
//			checkErrStack(dev.createImage(info, composition_img),
//				"failed to create composition image");
//		}
//
//		// Parents Clip Image
//		{
//			vkw::ImageCreateInfo info;
//			info.format = VK_FORMAT_R32_UINT;
//			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
//				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//			info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
//			checkErrStack1(dev.createImage(info, parents_clip_mask_img));
//
//			vkw::ImageViewCreateInfo view_info;
//			checkErrStack1(parents_clip_mask_img.createView(view_info, parents_clip_mask_view));
//		}
//
//		// Next Parents Clip Image
//		{
//			vkw::ImageCreateInfo info;
//			info.format = VK_FORMAT_R32_UINT;
//			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
//				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//			info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
//			checkErrStack1(dev.createImage(info, next_parents_clip_mask_img));
//
//			vkw::ImageViewCreateInfo view_info;
//			checkErrStack1(next_parents_clip_mask_img.createView(view_info, next_parents_clip_mask_view));
//		}
//
//		// Character Atlas Texture
//		{
//			vkw::ImageCreateInfo info;
//			info.format = VK_FORMAT_R8_UNORM;
//			info.width = instance->char_atlas.atlas.tex_size;
//			info.height = instance->char_atlas.atlas.tex_size;
//			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//			checkErrStack(dev.createImage(info, char_atlas_tex),
//				"failed to create character atlas texture");
//
//			vkw::ImageViewCreateInfo view_info;
//			checkErrStack(char_atlas_tex.createView(view_info, char_atlas_view),
//				"failed to create character atlas view");
//
//			TextureAtlas& char_atlas = instance->char_atlas.atlas;
//			checkErrStack(char_atlas_view.load(char_atlas.colors.data(), char_atlas.colors.size(), VK_IMAGE_LAYOUT_GENERAL),
//				"failed to load character atlas data into texture");
//		}
//
//		// Composition Image View
//		{
//			vkw::ImageViewCreateInfo info;
//			checkErrStack(composition_img.createView(info, composition_view),
//				"failed to create composition view");
//		}
//
//		// Sampler
//		{
//			vkw::SamplerCreateInfo info;
//			checkErrStack(dev.createSampler(info, text_sampler),
//				"failed to create sampler");
//		}
//
//		checkErrStack1(generateGPU_Data());
//
//		std::vector<char> spirv;
//
//		// Character Vertex Shader
//		{
//			FilePath path;
//			checkErrStack1(path.recreateRelativeToSolution("UserInterface/Shaders/Text/vert.spv"));
//
//			spirv.clear();
//			checkErrStack1(path.read(spirv));
//
//			checkErrStack1(dev.createShader(spirv, VK_SHADER_STAGE_VERTEX_BIT, text_vs));
//		}
//
//		// Character Fragment Shader
//		{
//			FilePath path;
//			checkErrStack1(path.recreateRelativeToSolution("UserInterface/Shaders/Text/frag.spv"));
//
//			spirv.clear();
//			checkErrStack1(path.read(spirv));
//
//			checkErrStack1(dev.createShader(spirv, VK_SHADER_STAGE_FRAGMENT_BIT, text_fs));
//		}
//
//		// Wrap Vertex Shader
//		{
//			FilePath path;
//			checkErrStack1(path.recreateRelativeToSolution("UserInterface/Shaders/Wrap/vert.spv"));
//
//			spirv.clear();
//			checkErrStack1(path.read(spirv));
//
//			checkErrStack1(dev.createShader(spirv, VK_SHADER_STAGE_VERTEX_BIT, wrap_vs));
//		}
//
//		// Wrap Fragment Shader
//		{
//			FilePath path;
//			checkErrStack1(path.recreateRelativeToSolution("UserInterface/Shaders/Wrap/frag.spv"));
//
//			spirv.clear();
//			checkErrStack1(path.read(spirv));
//
//			checkErrStack1(dev.createShader(spirv, VK_SHADER_STAGE_FRAGMENT_BIT, wrap_fs));
//		}
//
//		// Text Pass
//		{
//			dev.createDrawpass(text_pass);
//
//			vkw::CombinedImageSamplerBinding sampler_info;
//			text_pass.bindCombinedImageSampler(sampler_info);
//
//			vkw::UniformBufferBinding ubuff_info;
//			ubuff_info.set = 1;
//			text_pass.bindUniformBuffer(ubuff_info);
//
//			vkw::ReadAttachmentInfo parents_clip_info;
//			parents_clip_info.format = parents_clip_mask_img.format;
//			parents_clip_info.set = 2;
//			text_pass.addReadColorAttachment(parents_clip_info);
//
//			vkw::WriteAttachmentInfo compose_info;
//			compose_info.format = composition_img.format;
//			compose_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
//			compose_info.blendEnable = true;
//			text_pass.addWriteColorAttachment(compose_info);
//
//			text_pass.vertex_inputs.push_back(GPU_CharacterVertex::getVertexInput());
//			text_pass.vertex_inputs.push_back(GPU_CharacterInstance::getVertexInput(1));
//
//			text_pass.setVertexShader(text_vs);
//
//			text_pass.setFragmentShader(text_fs);
//
//			checkErrStack1(text_pass.build());
//		}
//
//		// Text Pass Descriptors
//		{
//			vkw::CombinedImageSamplerDescriptor sampler_info;
//			sampler_info.sampler = &text_sampler;
//			sampler_info.view = &char_atlas_view;
//			text_pass.updateCombinedImageSamplerDescriptor(0, 0, sampler_info);
//
//			vkw::UpdateBufferDescriptor ubuff_info;
//			ubuff_info.buffer = &text_ubuff;
//			text_pass.updateUniformBufferDescriptor(1, 0, ubuff_info);
//
//			vkw::InputAttachmentDescriptor parents_clip_info;
//			parents_clip_info.view = &parents_clip_mask_view;
//			text_pass.updateInputAttachmentDescriptor(2, 0, parents_clip_info);
//		}
//
//		// Text Framebuffer
//		{
//			vkw::FramebufferCreateInfo info;
//			info.attachments = {
//				&parents_clip_mask_view,
//				&composition_view
//			};
//			checkErrStack1(text_pass.createFramebuffer(info, text_framebuff));
//		}
//
//		// Wrap Pass
//		{
//			dev.createDrawpass(wrap_pass);
//
//			vkw::UniformBufferBinding ubuff_info;
//			wrap_pass.bindUniformBuffer(ubuff_info);
//
//			vkw::ReadAttachmentInfo parents_clip_info;
//			parents_clip_info.format = parents_clip_mask_img.format;
//			parents_clip_info.set = 1;
//			wrap_pass.addReadColorAttachment(parents_clip_info);
//
//			vkw::WriteAttachmentInfo compose_info;
//			compose_info.format = composition_img.format;
//			compose_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
//			wrap_pass.addWriteColorAttachment(compose_info);
//
//			vkw::WriteAttachmentInfo next_parents_clip_info;
//			next_parents_clip_info.format = next_parents_clip_mask_img.format;
//			next_parents_clip_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
//			wrap_pass.addWriteColorAttachment(next_parents_clip_info);
//
//			wrap_pass.vertex_inputs.push_back(GPU_WrapVertex::getVertexInput());
//			wrap_pass.vertex_inputs.push_back(GPU_WrapInstance::getVertexInput(1));
//
//			wrap_pass.setVertexShader(wrap_vs);
//
//			wrap_pass.setFragmentShader(wrap_fs);
//
//			checkErrStack1(wrap_pass.build());
//		}
//
//		// Wrap Pass Descriptors
//		{
//			vkw::UpdateBufferDescriptor ubuff_info;
//			ubuff_info.buffer = &text_ubuff;
//			wrap_pass.updateUniformBufferDescriptor(0, 0, ubuff_info);
//
//			vkw::InputAttachmentDescriptor parents_clip_info;
//			parents_clip_info.view = &parents_clip_mask_view;
//			wrap_pass.updateInputAttachmentDescriptor(1, 0, parents_clip_info);
//		}
//
//		// Wrap Pass Framebuffer
//		{
//			vkw::FramebufferCreateInfo info;
//			info.attachments = {
//				&parents_clip_mask_view,
//				&composition_view,
//				&next_parents_clip_mask_view
//			};
//			checkErrStack1(wrap_pass.createFramebuffer(info, wrap_framebuff));
//		}
//
//		// Command List
//		{
//			vkw::CommandListCreateInfo info = {};
//			info.surface = &dev.surface;
//
//			checkErrStack1(dev.createCommandList(info, cmd_list));
//			checkErrStack1(cmd_list.beginRecording());
//
//			cmd_list.cmdClearFloatImage(composition_view);
//			cmd_list.cmdClearUIntImage(parents_clip_mask_view);
//			cmd_list.cmdClearUIntImage(next_parents_clip_mask_view);
//			{
//				std::array<vkw::ImageView*, 3> views = {
//					&composition_view, &parents_clip_mask_view, &next_parents_clip_mask_view
//				};
//				cmd_list.cmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
//					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, (VkAccessFlagBits)0, views.size(), views.data());
//			}
//
//			std::vector<Node*> now_nodes = {
//				&nodes.front()
//			};
//			std::vector<Node*> next_nodes;
//
//			while (now_nodes.size()) {
//
//				for (Node* node : now_nodes) {
//
//					switch (node->type) {
//					case NodeType::TEXT: {
//						Text* text = (Text*)node->elem;
//
//						if (!text->drawcalls.size()) {
//							break;
//						}
//
//						cmd_list.cmdBeginRenderpass(text_pass, text_framebuff);
//						{
//							cmd_list.cmdBindVertexBuffers(chars_vbuff, chars_instabuff);
//							cmd_list.cmdBindIndexBuffer(chars_idxbuff);
//
//							for (CharacterDrawcall& drawcall : text->drawcalls) {
//								cmd_list.cmdDrawIndexedInstanced(6, (uint32_t)drawcall.instances.size(),
//									drawcall.chara->index_start_idx, drawcall.instance_start_idx);
//
//							}
//						}
//						cmd_list.cmdEndRenderpass();
//						break;
//					}
//
//					case NodeType::WRAP: {
//						Wrap* wrap = (Wrap*)node->elem;
//
//						cmd_list.cmdBeginRenderpass(wrap_pass, wrap_framebuff);
//						{
//							cmd_list.cmdBindVertexBuffers(wrap_vbuff, wrap_instabuff);
//							cmd_list.cmdBindIndexBuffer(wrap_idxbuff);
//
//							cmd_list.cmdDrawIndexedInstanced(6, 1, 0, wrap->drawcall.instance_idx);
//						}
//						cmd_list.cmdEndRenderpass();
//						break;
//					}
//					}
//
//					// TODO: barrier, wait for writes to complete for composition, and next parents
//
//					for (Node* child : node->children) {
//						next_nodes.push_back(child);
//					}
//				}
//
//				now_nodes = next_nodes;
//				next_nodes.clear();
//
//				cmd_list.cmdCopyImageToImage(next_parents_clip_mask_view, parents_clip_mask_view);
//				cmd_list.cmdClearUIntImage(next_parents_clip_mask_view);
//			}	
//
//			// Copy Compose Image to Surface
//			{
//				vkw::SurfaceBarrier dst;
//				dst.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//				dst.wait_for_access = 0;
//				dst.wait_at_access = VK_ACCESS_TRANSFER_WRITE_BIT;
//				cmd_list.cmdSurfacePipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, dst);
//
//				cmd_list.cmdCopyImageToSurface(composition_view);
//
//				dst = {};
//				dst.new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//				dst.wait_for_access = VK_ACCESS_TRANSFER_WRITE_BIT;
//				dst.wait_at_access = 0;
//				cmd_list.cmdSurfacePipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
//					dst);
//			}
//
//			checkErrStack1(cmd_list.endRecording());
//		}
//	}
//	else {
//		if (width != dev.surface.width || height != dev.surface.height) {
//
//			// resize images and their views
//			// resize renderpass
//			// recreate c
//		}
//	}
//
//	checkErrStack1(cmd_list.run());
//	checkErrStack1(cmd_list.waitForExecution());
//
//	return ErrStack();
//}
