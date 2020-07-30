#pragma once

// Standard
#include <vector>
#include <array>
#include <variant>

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


namespace vnx {

	enum class LoadType {
		STAGING,
		MEMCPY,
	};

	class Instance;
	class VulkanDevice;
	class Surface;
	class CommandList;


	struct SurfaceCreateInfo {
		void* surface_pNext = NULL;
		VkWin32SurfaceCreateFlagsKHR surface_flags = 0;
		HINSTANCE hinstance;
		HWND hwnd;

		VkSwapchainCreateFlagsKHR swapchain_flags = 0;
		uint32_t imageArrayLayers = 1;
		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkBool32 clipped = true;
	};

	class Surface {
	public:
		VulkanDevice* dev;

		VkSurfaceKHR surface = VK_NULL_HANDLE;

		uint32_t minImageCount;
		VkFormat imageFormat;
		VkColorSpaceKHR imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		uint32_t width;
		uint32_t height;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;

		std::vector<VkImage> swapchain_images;
		std::vector<VkImageView> swapchain_views;

		std::vector<FrameImage*> frame_images;

	public:
		ErrStack getSurfaceCapabilities(VkSurfaceCapabilitiesKHR& capabilities);

		ErrStack createFrameImage(FrameImageCreateInfo& info, FrameImage& frame_image);
	};


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
		ImageSubResourceRange subres_range = {};
	};

	class ImageView {
	public:
		Image* image;

		VkImageView view = VK_NULL_HANDLE;

	public:
		~ImageView();
	};


	struct ImageCreateInfo {
		VkImageCreateFlags flags = 0;
		VkImageType imageType = VK_IMAGE_TYPE_2D;
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		uint32_t width;
		uint32_t height;
		uint32_t depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	};

	class Image {
	public:
		VulkanDevice* dev;

		VkImage img = VK_NULL_HANDLE;
		void* mem;

		VmaAllocation alloc;
		VmaAllocationInfo alloc_info;
		LoadType load_type;

		// Image Properties
		VkFormat format;
		VkSampleCountFlagBits samples;
		VkImageLayout current_layout;

	public:
		ErrStack create(VulkanDevice* dev, ImageCreateInfo& info);

		ErrStack createView(ImageViewCreateInfo& info, ImageView& view);

		void destroy();
		~Image();
	};


	struct FrameImageCreateInfo {
		VkImageCreateFlags flags = 0;
		VkImageType imageType = VK_IMAGE_TYPE_2D;
		VkFormat format = VK_FORMAT_MAX_ENUM;
		uint32_t depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	};

	class FrameImage : public Image {
	public:
		Surface* surface = nullptr;

	public:
		~FrameImage();
	};


	struct TextureCreateInfo {
		VkImageCreateFlags flags = 0;
		VkImageType imageType = VK_IMAGE_TYPE_2D;
		VkFormat format = VK_FORMAT_MAX_ENUM;
		uint32_t width;
		uint32_t height;
		uint32_t depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	};

	class Texture : public Image {
	public:

	};


	struct BufferCreateInfo {
		VkBufferCreateFlags flags = 0;
		VkDeviceSize size;
		VkBufferUsageFlags usage;

		VmaMemoryUsage vma_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	};

	class Buffer {
	public:
		VulkanDevice* dev;

		VkBuffer buff = VK_NULL_HANDLE;
		VmaAllocation buff_alloc;
		void* mem;

		// Props
		size_t size;
		VkBufferUsageFlags usage;
		VmaAllocationInfo vma_r_info = {};
		LoadType load_type;

	public:
		ErrStack create(VulkanDevice* dev, BufferCreateInfo& info);
	};


	class VertexInput {
		VkVertexInputBindingDescription binding;
		std::vector<VkVertexInputAttributeDescription> atributes;
	};


	class Shader {
	public:
		VulkanDevice* dev;

		VkShaderStageFlagBits stage;
		VkShaderModule shader = VK_NULL_HANDLE;

	public:
		ErrStack create(VulkanDevice* dev, std::vector<char>& spirv, VkShaderStageFlagBits stage);
		~Shader();
	};


	struct ReadAttachmentInfo {
		ImageView* view;

		// Renderpass
		VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		VkImageLayout final_fayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout in_use_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Descriptor Layout
		uint32_t set = 0;
		uint32_t binding = 0;
		uint32_t descriptor_count = 1;  // maybe delete ?
		uint32_t dst_array_element = 0;  // maybe delete

	};

	struct WriteAttachmentInfo {

	};

	struct ReadWriteAttachment {

	};

	struct ReadAttachment {
		ImageView* image;

		VkAttachmentDescriptionFlags flags = 0;	
		VkFormat format; // image format
		VkSampleCountFlagBits samples;  // image samples
		VkAttachmentLoadOp loadOp;  // Load Op load
		VkAttachmentStoreOp             storeOp;  // property
		VkAttachmentLoadOp              stencilLoadOp;  // property
		VkAttachmentStoreOp             stencilStoreOp; // property
		VkImageLayout                   initialLayout;  // image layout
		VkImageLayout                   finalLayout;  // property

		// Atachment reference
		// curent layout = property

		// Framebuffer

		uint32_t              binding;  // property
		VkDescriptorType      descriptorType;  // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT 
		uint32_t              descriptorCount;  // property
		VkShaderStageFlags    stageFlags;  // property
		const VkSampler* pImmutableSamplers;  // not implemented

		// increment VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT pool size

		uint32_t                         dstBinding;  // same property
		uint32_t                         dstArrayElement;  // property
		uint32_t                         descriptorCount;  // property
		VkDescriptorType                 descriptorType;  // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
		const VkDescriptorImageInfo* pImageInfo;  //
		VkImageView      imageView;  // image view
		VkImageLayout    imageLayout;  // same property
	};

	struct RenderpassCreateInfo {
		// input atachments
		// output atachments
	};

	class Renderpass {
	public:
		CommandList* cmd_list;

		// Renderpass
		std::vector<VkAttachmentDescription> atach_descps;
		std::vector<VkAttachmentReference> atach_refs;
		std::vector<VkSubpassDescription> subpass_descp;
		VkRenderPass renderpass;

		// Framebuffs
		std::vector<VkImageView> img_views;
		std::vector<VkFramebuffer> framebuffers;

	public:
		void addReadAttachment(ReadAttachmentInfo& info);
	};


	struct DeviceCreateInfo {
		VkQueueFlags queue_flags = VK_QUEUE_GRAPHICS_BIT;
		std::vector<char*> extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_MEMORY_BUDGET_EXTENSION_NAME
		};
		VkPhysicalDeviceFeatures features = {};

		void* pNext = NULL;
		VkDeviceCreateFlags flags = 0;
		std::vector<char*> layers = {
			"VK_LAYER_LUNARG_standard_validation"
		};
	};

	class VulkanDevice {
	public:
		Instance* inst;

		VkPhysicalDevice phys_dev = VK_NULL_HANDLE;
		VkDevice logical_dev = VK_NULL_HANDLE;

		VmaAllocator allocator;
		int32_t queue_fam_idx = -1;
		VkQueue queue;

	public:
		ErrStack createWin32Surface(SurfaceCreateInfo& info, Surface& surface);
		
		ErrStack createTexture(TextureCreateInfo& info, Texture& texture);
		ErrStack createBuffer(BufferCreateInfo& info, Buffer& buffer);
		ErrStack createShader(std::vector<char>& spirv, VkShaderStageFlagBits stage, Shader& shader);

		// ErrStack createCommandList(Surface& surface, CommandListCreateInfo& info, CommandList& cmd_list);
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
		std::vector<char*> validation_layers = {
			"VK_LAYER_LUNARG_standard_validation"
		};
		std::vector<char*> instance_extensions = {
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
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	};

	class Instance {
	public:
		VkInstance instance = VK_NULL_HANDLE;

		VkDebugUtilsMessengerEXT callback = VK_NULL_HANDLE;
		PFN_vkSetDebugUtilsObjectNameEXT set_vkdbg_name_func;

	public:
		ErrStack create(InstanceCreateInfo& info);

		ErrStack createDevice(DeviceCreateInfo& info, VulkanDevice& device);
	};
}