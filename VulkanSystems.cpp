
// Mine
#include "MathUtils.h"

// Header
#include "VulkanSystems.h"


namespace vks {

	ErrorStack find_layers(const std::vector<VkLayerProperties>& layer_props, const std::vector<const char*>& layers)
	{
		for (const char* layer : layers) {

			bool found = false;
			for (VkLayerProperties layer_prop : layer_props) {
				if (!strcmp(layer, layer_prop.layerName)) {
					found = true;
					break;
				}
			}

			if (!found) {
				return ErrorStack(ExtraError::VALIDATION_LAYER_NOT_FOUND, code_location, "validation layer = " + std::string(layer) + " not found");
			}
		}
		return ErrorStack();
	}

	ErrorStack find_extensions(const std::vector<VkExtensionProperties>& ext_props, const std::vector<const char*>& extensions)
	{
		for (const char* extension : extensions) {

			bool found = false;
			for (VkExtensionProperties ext_prop : ext_props) {
				if (!strcmp(extension, ext_prop.extensionName)) {
					found = true;
					break;
				}
			}

			if (!found) {
				return ErrorStack(ExtraError::EXTENSION_NOT_FOUND, code_location, "instance extension = " + std::string(extension) + " not found");
			}
		}
		return ErrorStack();
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "Vulkan Debug: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	ErrorStack Instance::create()
	{
		VkResult vk_res;
		ErrorStack err;

		// Find Layers
		{
			uint32_t layer_props_count = 0;

			vk_res = vkEnumerateInstanceLayerProperties(&layer_props_count, NULL);
			if (vk_res != VK_SUCCESS) {
				return ErrorStack(vk_res, code_location, "could not retrieve validation layer props count");
			}

			std::vector<VkLayerProperties> layer_props(layer_props_count);

			vk_res = vkEnumerateInstanceLayerProperties(&layer_props_count, layer_props.data());
			if (vk_res != VK_SUCCESS) {
				return ErrorStack(vk_res, code_location, "could not retrieve validation layer props");
			}

			validation_layers = { "VK_LAYER_LUNARG_standard_validation" };

			err = find_layers(layer_props, validation_layers);
			if (err.isBad()) {
				return err;
			}
		}

		// Find Extensions
		{
			uint32_t ext_props_count = 0;

			vk_res = vkEnumerateInstanceExtensionProperties(NULL, &ext_props_count, NULL);
			if (vk_res != VK_SUCCESS) {
				return ErrorStack(vk_res, ExtraError::FAILED_ENUMERATE_INSTANCE_EXTENSIONS, code_location, "could not retrieve instance extension props count");
			}

			std::vector<VkExtensionProperties> ext_props(ext_props_count);

			vk_res = vkEnumerateInstanceExtensionProperties(NULL, &ext_props_count, ext_props.data());
			if (vk_res != VK_SUCCESS) {
				return ErrorStack(vk_res, ExtraError::FAILED_ENUMERATE_INSTANCE_EXTENSIONS, code_location, "could not retrieve instance extension props");
			}

			err = find_extensions(ext_props, instance_extensions);
			if (err.isBad()) {
				return err;
			}

			instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// Create Instance
		{
			// AppInfo
			VkApplicationInfo app_info = {};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = "Vulkan Aplication";
			app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.pEngineName = "No Engine";
			app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.apiVersion = VK_API_VERSION_1_0;

			// Instance Creation info
			VkInstanceCreateInfo inst_info = {};
			inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			inst_info.flags = 0;
			inst_info.pApplicationInfo = &app_info;
			inst_info.enabledLayerCount = (uint32_t)(validation_layers.size());
			inst_info.ppEnabledLayerNames = validation_layers.data();
			inst_info.enabledExtensionCount = (uint32_t)(instance_extensions.size());
			inst_info.ppEnabledExtensionNames = instance_extensions.data();

			vk_res = vkCreateInstance(&inst_info, NULL, &instance);
			if (vk_res != VK_SUCCESS) {
				return ErrorStack(vk_res, ExtraError::INSTANCE_CREATION_FAILURE, code_location, "could not create a Vulkan instance");
			}
		}

		// Debug callback
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = debug_msg_severity;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = debugCallback;
			createInfo.pUserData = NULL;

			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != NULL) {

				vk_res = func(instance, &createInfo, NULL, &callback);
				if (vk_res != VK_SUCCESS) {
					return ErrorStack(vk_res, code_location, "debug callback creation failed");
				}
			}
			else {
				return ErrorStack(ExtraError::DEBUG_EXTENSION_NOT_FOUND, code_location, "debug extension not present");
			}
		}

		// External Functions
		{
			getMemProps2 = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)
				vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties2KHR");

			if (getMemProps2 == NULL) {
				return ErrorStack(ExtraError::FAILED_TO_GET_EXTERNAL_FUNCTION, code_location, "failed to retrieve function pointer for "
					"vkGetPhysicalDeviceMemoryProperties2KHR");
			}
		}

		return ErrorStack();
	}

	void Instance::destroy()
	{
		if (this->callback != VK_NULL_HANDLE) {

			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != NULL) {
				func(instance, callback, NULL);
			}
			else {
				std::cout << code_location << "debug callback already freed" << std::endl;
			}
			callback = VK_NULL_HANDLE;
		}

		vkDestroyInstance(instance, NULL);
		this->instance = VK_NULL_HANDLE;
	}

	Instance::~Instance()
	{
		if (instance != VK_NULL_HANDLE) {
			destroy();
		}
	}


	ErrorStack Surface::create(Instance* instance, HINSTANCE hinstance, HWND hwnd)
	{
		this->instance = instance;

		VkWin32SurfaceCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		info.hinstance = hinstance;
		info.hwnd = hwnd;

		VkResult vk_res = vkCreateWin32SurfaceKHR(instance->instance, &info, NULL, &this->surface);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, code_location, "failed to create vulkan surface");
		}

		return ErrorStack();
	}

	void Surface::destroy()
	{
		vkDestroySurfaceKHR(instance->instance, this->surface, NULL);
		this->surface = VK_NULL_HANDLE;
	}

	Surface::~Surface()
	{
		if (surface != VK_NULL_HANDLE) {
			destroy();
		}
	}


	PhysicalDevice::PhysicalDevice()
	{
		phys_dev_features.samplerAnisotropy = VK_TRUE;
	}

	ErrorStack PhysicalDevice::create(Instance* instance, Surface* surface)
	{
		VkResult vk_res;
		ErrorStack err;

		uint32_t deviceCount = 0;

		vk_res = vkEnumeratePhysicalDevices(instance->instance, &deviceCount, NULL);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, ExtraError::FAILED_TO_ENUMERATE_PHYSICAL_DEVICES, code_location, "failed to enumerate physical devices");
		}

		if (deviceCount == 0) {
			return ErrorStack(ExtraError::NO_GPU_WITH_VULKAN_SUPPORT_FOUND, code_location, "failed to find GPUs with Vulkan support");
		}

		std::vector<VkPhysicalDevice> phys_devices(deviceCount);

		vk_res = vkEnumeratePhysicalDevices(instance->instance, &deviceCount, phys_devices.data());
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, ExtraError::FAILED_TO_ENUMERATE_PHYSICAL_DEVICES, code_location, "failed to enumerate physical devices");
		}

		for (VkPhysicalDevice phys_dev : phys_devices) {

			// Find Queue Familly Index
			{
				bool ok_queue_family = false;

				uint32_t family_count = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &family_count, nullptr);

				std::vector<VkQueueFamilyProperties> queue_families(family_count);
				vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &family_count, queue_families.data());

				for (uint32_t i = 0; i < family_count; i++) {

					VkQueueFamilyProperties& family_prop = queue_families[i];

					VkBool32 supported;
					vk_res = vkGetPhysicalDeviceSurfaceSupportKHR(phys_dev, i, surface->surface, &supported);
					if (vk_res != VK_SUCCESS) {
						return ErrorStack(vk_res, code_location, "failed to check if pshysical device can present");
					}

					if (family_prop.queueCount > 0 && family_prop.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
						supported == VK_TRUE)
					{
						this->queue_fam_idx = i;
						ok_queue_family = true;
						break;
					}
				}

				if (!ok_queue_family) {
					continue;
				}
			}

			// Check for extension support
			{
				uint32_t extension_count;
				if (vkEnumerateDeviceExtensionProperties(phys_dev, NULL, &extension_count, NULL) != VK_SUCCESS) {
					return ErrorStack(code_location, "failed to retrieve enumerate device extension properties count");
				}

				std::vector<VkExtensionProperties> available_extensions(extension_count);
				if (vkEnumerateDeviceExtensionProperties(phys_dev, NULL, &extension_count, available_extensions.data()) != VK_SUCCESS) {
					return ErrorStack(code_location, "failed to retrieve enumerate device extension properties");
				}

				uint32_t count = 0;
				for (const char* req_extension : device_extensions) {
					for (VkExtensionProperties extension : available_extensions) {

						if (!std::strcmp(req_extension, extension.extensionName)) {
							count++;
						}
					}
				}

				if (count < device_extensions.size()) {
					continue;
				}
			}

			// Check for device features
			{
				VkPhysicalDeviceFeatures available_features;
				vkGetPhysicalDeviceFeatures(phys_dev, &available_features);

				// to be macroed if need be
				if (phys_dev_features.samplerAnisotropy &&
					phys_dev_features.samplerAnisotropy != available_features.samplerAnisotropy)
				{
					continue;
				}
			}

			physical_device = phys_dev;
			break;
		}

		if (physical_device == VK_NULL_HANDLE) {
			return ErrorStack(ExtraError::NO_SUITABLE_GPU_FOUND, code_location, "failed to find a suitable GPU");
		}

		vkGetPhysicalDeviceProperties(physical_device, &this->phys_dev_props);
		vkGetPhysicalDeviceMemoryProperties(physical_device, &this->mem_props);

		return ErrorStack();
	}


	ErrorStack LogicalDevice::create(Instance* instance, PhysicalDevice* phys_dev)
	{
		// Graphics queue
		VkDeviceQueueCreateInfo graphics_queue_info = {};
		graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphics_queue_info.queueFamilyIndex = phys_dev->queue_fam_idx;
		graphics_queue_info.queueCount = 1;
		graphics_queue_info.pQueuePriorities = &queue_priority;

		VkDeviceCreateInfo device_info = {};
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		// Queue
		device_info.queueCreateInfoCount = 1;
		device_info.pQueueCreateInfos = &graphics_queue_info;

		// Layers
		device_info.enabledLayerCount = (uint32_t)(instance->validation_layers.size());
		device_info.ppEnabledLayerNames = instance->validation_layers.data();

		// Extensions
		device_info.enabledExtensionCount = (uint32_t)(phys_dev->device_extensions.size());
		device_info.ppEnabledExtensionNames = phys_dev->device_extensions.data();

		// Features
		device_info.pEnabledFeatures = &phys_dev->phys_dev_features;

		VkResult vk_res = vkCreateDevice(phys_dev->physical_device, &device_info, NULL, &logical_device);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, ExtraError::FAILED_CREATE_LOGICAL_DEVICE, code_location, "failed to create logical device");
		}

		// Queue creation
		vkGetDeviceQueue(logical_device, phys_dev->queue_fam_idx, 0, &queue);

		// VMA Allocator
		{
			VmaAllocatorCreateInfo alloc_info = {};
			alloc_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
			alloc_info.instance = instance->instance;
			alloc_info.physicalDevice = phys_dev->physical_device;
			alloc_info.device = logical_device;

			VkResult vk_res = vmaCreateAllocator(&alloc_info, &this->allocator);
			if (vk_res != VK_SUCCESS) {
				return ErrorStack(vk_res, code_location, "failed to create allocator");
			}
		}

		return ErrorStack();
	}

	void LogicalDevice::destroy()
	{
		vmaDestroyAllocator(this->allocator);

		vkDestroyDevice(logical_device, NULL);
		logical_device = VK_NULL_HANDLE;
	}

	LogicalDevice::~LogicalDevice()
	{
		if (logical_device != VK_NULL_HANDLE) {
			destroy();
		}
	}


	ErrorStack Swapchain::create(Surface* surface, PhysicalDevice* phys_dev, LogicalDevice* logical_dev, 
		uint32_t width, uint32_t height)
	{
		this->logical_device = logical_dev;
		this->desired_width = width;
		this->desired_height = height;

		// Surface Formats
		{
			std::vector<VkSurfaceFormatKHR> formats;

			uint32_t format_counts;
			checkVkRes(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev->physical_device, surface->surface, &format_counts, NULL),
				"failed to find physical surface format count");

			formats.resize((size_t)format_counts);
			checkVkRes(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev->physical_device, surface->surface, &format_counts, formats.data()),
				"failed to find physical surface format");

			bool found = false;
			for (VkSurfaceFormatKHR format : formats) {
				if (format.format == VK_FORMAT_R8G8B8A8_UNORM ||
					format.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
						surface_format = format;
						found = true;
						break;
					}
				}
			}

			if (!found) {
				return ErrorStack(ExtraError::NO_SUITABLE_SURFACE_FORMAT_FOUND, code_location, "failed to find suitable surface format");
			}
		}

		// Trim resolution
		{
			// Surface Capabilities
			checkVkRes(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev->physical_device, surface->surface, &capabilities),
				"failed to get physical device surface capabilities");

			VkExtent2D min_img_extent = capabilities.minImageExtent;
			VkExtent2D max_img_extent = capabilities.maxImageExtent;

			this->resolution.width = clamp(width, min_img_extent.width, max_img_extent.width);
			this->resolution.height = clamp(height, min_img_extent.height, max_img_extent.height);

			/* when minimizing minImageExtent and maxImageExtent will be zero
			 * at that point vkCreateSwapchainKHR will not like ANY resolution
			 * this triggers fewer warnings */
			if (!resolution.width || !resolution.height) {
				this->resolution.width = width;
				this->resolution.height = height;
			}

			printf("Swapchain Resolution = (%d, %d) \n", resolution.width, resolution.height);
		}

		VkSwapchainCreateInfoKHR swapchain_info = {};
		swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_info.surface = surface->surface;
		swapchain_info.minImageCount = capabilities.minImageCount;
		swapchain_info.imageFormat = surface_format.format;
		swapchain_info.imageColorSpace = surface_format.colorSpace;
		swapchain_info.imageExtent = resolution;
		swapchain_info.imageArrayLayers = 1;
		swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_info.preTransform = pre_transform;
		swapchain_info.compositeAlpha = composite_alpha;
		swapchain_info.presentMode = presentation_mode;
		swapchain_info.clipped = VK_TRUE;
		swapchain_info.oldSwapchain = NULL;

		checkVkRes(vkCreateSwapchainKHR(logical_dev->logical_device, &swapchain_info, NULL, &swapchain),
			"failed to create swapchain");

		uint32_t image_count = 0;
		checkVkRes(vkGetSwapchainImagesKHR(logical_device->logical_device, swapchain, &image_count, NULL),
			"failed to retrieve swap chain image count");

		images.resize(image_count);
		checkVkRes(vkGetSwapchainImagesKHR(logical_device->logical_device, swapchain, &image_count, images.data()),
			"failed to retrieve swap chain images");

		return ErrorStack();
	}

	void Swapchain::destroy()
	{
		vkDestroySwapchainKHR(logical_device->logical_device, swapchain, NULL);
		swapchain = VK_NULL_HANDLE;
	}

	Swapchain::~Swapchain()
	{
		if (swapchain != VK_NULL_HANDLE) {
			destroy();
		}
	}


	static ErrorStack findSupportedImageFormat(vks::PhysicalDevice* phys_dev, std::vector<DesiredImageProps> desires,
		DesiredImageProps& result)
	{
		VkResult vk_res;

		for (DesiredImageProps desire : desires) {

			vk_res = vkGetPhysicalDeviceImageFormatProperties(phys_dev->physical_device, desire.format, desire.type,
				desire.tiling, desire.usage, desire.flags, &desire.props_found);
			if (vk_res == VK_SUCCESS) {
				result = desire;
				return ErrorStack();
			}
			else if (vk_res == VK_ERROR_FORMAT_NOT_SUPPORTED) {
				continue;
			}
			else {
				return ErrorStack(vk_res, code_location, "failed to check for desired image properties");
			}
		}

		return ErrorStack(code_location, "desired image properties not supported");
	}


	ErrorStack ImageView::create(LogicalDevice* logical_dev, VkImage img, VkFormat format, 
		VkImageAspectFlags aspect)
	{
		this->logical_dev = logical_dev;

		VkComponentMapping component_mapping = {};
		component_mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		component_mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		component_mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		component_mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		VkImageSubresourceRange sub_resource = {};
		sub_resource.aspectMask = aspect;
		sub_resource.baseMipLevel = 0;
		sub_resource.levelCount = 1;
		sub_resource.baseArrayLayer = 0;
		sub_resource.layerCount = 1;

		VkImageViewCreateInfo imageview_info = {};
		imageview_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageview_info.image = img;
		imageview_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageview_info.format = format;
		imageview_info.components = component_mapping;
		imageview_info.subresourceRange = sub_resource;

		checkVkRes(vkCreateImageView(logical_dev->logical_device, &imageview_info, NULL, &img_view),
			"failed to create image view");

		return ErrorStack();
	}

	ErrorStack ImageView::create(LogicalDevice* logical_dev, Image* img, VkImageAspectFlags aspect)
	{
		return create(logical_dev, img->img, img->format, aspect);
	}

	void ImageView::destroy()
	{
		vkDestroyImageView(logical_dev->logical_device, img_view, NULL);
		img_view = VK_NULL_HANDLE;
	}

	ImageView::~ImageView()
	{
		if (img_view != VK_NULL_HANDLE) {
			destroy();
		}
	}


	ErrorStack Sampler::create(LogicalDevice* logical_dev)
	{
		this->logical_dev = logical_dev;

		VkSamplerCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.magFilter = VK_FILTER_LINEAR;
		info.minFilter = VK_FILTER_LINEAR;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.anisotropyEnable = VK_TRUE;
		info.maxAnisotropy = 16;
		info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		info.mipLodBias = 0.0f;
		info.minLod = 0.0f;
		info.maxLod = 0.0f;

		checkVkRes(vkCreateSampler(logical_dev->logical_device, &info, NULL, &sampler),
			"failed to create sampler");
		return ErrorStack();
	}

	void Sampler::destroy()
	{
		vkDestroySampler(logical_dev->logical_device, sampler, NULL);
		sampler = VK_NULL_HANDLE;
	}

	Sampler::~Sampler()
	{
		if (sampler != VK_NULL_HANDLE) {
			destroy();
		}
	}


	ErrorStack Renderpass::create(LogicalDevice* logical_dev, VkFormat present_format, 
		VkFormat depth_format)
	{
		this->logical_dev = logical_dev;

		// Description
		VkAttachmentDescription color_atach = {};
		color_atach.format = present_format;
		color_atach.samples = VK_SAMPLE_COUNT_1_BIT;  // for MSAA ?
		color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_atach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depth_atach = {};
		depth_atach.format = depth_format;
		depth_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_atach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_atach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Reference
		VkAttachmentReference color_attach_ref = {};
		color_attach_ref.attachment = 0;
		color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attach_ref = {};
		depth_attach_ref.attachment = 1;
		depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Subpass Description
		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_attach_ref;
		subpass_description.pDepthStencilAttachment = &depth_attach_ref;

		// Subpass Dependency
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;  // depth ?

		std::array<VkAttachmentDescription, 2> attachments;
		attachments[0] = color_atach;
		attachments[1] = depth_atach;

		VkRenderPassCreateInfo renderpass_info = {};
		renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderpass_info.attachmentCount = (uint32_t)attachments.size();
		renderpass_info.pAttachments = attachments.data();
		renderpass_info.subpassCount = 1;
		renderpass_info.pSubpasses = &subpass_description;
		renderpass_info.dependencyCount = 1;
		renderpass_info.pDependencies = &dependency;

		checkVkRes(vkCreateRenderPass(logical_dev->logical_device, &renderpass_info, NULL, &renderpass),
			"failed to create renderpass");
		return ErrorStack();
	}

	void Renderpass::destroy()
	{
		vkDestroyRenderPass(logical_dev->logical_device, renderpass, NULL);
		renderpass = VK_NULL_HANDLE;
	}

	Renderpass::~Renderpass()
	{
		if (renderpass != VK_NULL_HANDLE) {
			destroy();
		}
	}


	ErrorStack Framebuffers::create(LogicalDevice* logical_dev, std::vector<ImageView>& present_views,
		ImageView* depth_view, Renderpass* renderpass, uint32_t width, uint32_t height)
	{
		this->logical_dev = logical_dev;

		size_t img_count = present_views.size();
		frame_buffs.resize(img_count);

		for (size_t i = 0; i < img_count; i++) {

			std::array<VkImageView, 2> attachments = {
				present_views[i].img_view,
				depth_view->img_view
			};

			VkFramebufferCreateInfo framebuff_info = {};
			framebuff_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuff_info.renderPass = renderpass->renderpass;
			framebuff_info.attachmentCount = (uint32_t)attachments.size();
			framebuff_info.pAttachments = attachments.data();
			framebuff_info.width = width;
			framebuff_info.height = height;
			framebuff_info.layers = 1;

			checkVkRes(vkCreateFramebuffer(logical_dev->logical_device, &framebuff_info, NULL,
				&frame_buffs[i]), "failed to create framebuffer");
		}
		return ErrorStack();
	}

	void Framebuffers::destroy()
	{
		for (auto frame_buff : frame_buffs) {
			vkDestroyFramebuffer(logical_dev->logical_device, frame_buff, NULL);
		}
		frame_buffs.clear();
	}

	Framebuffers::~Framebuffers()
	{
		if (frame_buffs.size() > 0) {
			destroy();
		}
	}


	ErrorStack CommandPool::create(LogicalDevice* logical_dev, PhysicalDevice* phys_dev)
	{
		this->logical_dev = logical_dev;

		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.queueFamilyIndex = phys_dev->queue_fam_idx;

		checkVkRes(vkCreateCommandPool(logical_dev->logical_device, &info, NULL, &cmd_pool),
			"failed to create command pool");
		return ErrorStack();
	}

	void CommandPool::destroy()
	{
		vkDestroyCommandPool(logical_dev->logical_device, cmd_pool, NULL);
		cmd_pool = VK_NULL_HANDLE;
	}

	CommandPool::~CommandPool()
	{
		if (cmd_pool != VK_NULL_HANDLE) {
			destroy();
		}
	}


	SingleCommandBuffer::SingleCommandBuffer(LogicalDevice* logical_dev, CommandPool* cmd_pool, ErrorStack* r_err)
	{
		this->logical_dev = logical_dev;
		this->cmd_pool = cmd_pool;
		this->err = r_err;

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = cmd_pool->cmd_pool;
		allocInfo.commandBufferCount = 1;

		VkResult vk_res = vkAllocateCommandBuffers(logical_dev->logical_device, &allocInfo, &cmd_buff);
		if (vk_res != VK_SUCCESS) {
			*err = ErrorStack(vk_res, code_location, "failed to create single use command buffer");
			return;
		}

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vk_res = vkBeginCommandBuffer(cmd_buff, &beginInfo);
		if (vk_res != VK_SUCCESS) {
			*err = ErrorStack(vk_res, code_location, "failed to begin single use command buffer");
			return;
		}
	}

	SingleCommandBuffer::~SingleCommandBuffer()
	{
		VkResult vk_res = vkEndCommandBuffer(cmd_buff);
		if (vk_res != VK_SUCCESS) {
			*err = ErrorStack(vk_res, code_location, "failed to end single use command buffer");
			return;
		}

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd_buff;

		vk_res = vkQueueSubmit(logical_dev->queue, 1, &submitInfo, VK_NULL_HANDLE);
		if (vk_res != VK_SUCCESS) {
			*err = ErrorStack(vk_res, code_location, "failed to submit single use command buffer");
			return;
		}

		vk_res = vkQueueWaitIdle(logical_dev->queue);
		if (vk_res != VK_SUCCESS) {
			*err = ErrorStack(vk_res, code_location, "");
			return;
		}

		vkFreeCommandBuffers(logical_dev->logical_device, cmd_pool->cmd_pool, 1, &cmd_buff);
	}


	VkVertexInputBindingDescription GPUVertex::getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(GPUVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	std::array<VkVertexInputAttributeDescription, 7> GPUVertex::getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 7> attrs_descp = {};

		attrs_descp[0].binding = 0;
		attrs_descp[0].location = 0;
		attrs_descp[0].format = VK_FORMAT_R32_UINT;
		attrs_descp[0].offset = offsetof(GPUVertex, mesh_id);

		attrs_descp[1].binding = 0;
		attrs_descp[1].location = 1;
		attrs_descp[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attrs_descp[1].offset = offsetof(GPUVertex, pos);

		attrs_descp[2].binding = 0;
		attrs_descp[2].location = 2;
		attrs_descp[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attrs_descp[2].offset = offsetof(GPUVertex, vertex_normal);

		attrs_descp[3].binding = 0;
		attrs_descp[3].location = 3;
		attrs_descp[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attrs_descp[3].offset = offsetof(GPUVertex, tess_normal);

		attrs_descp[4].binding = 0;
		attrs_descp[4].location = 4;
		attrs_descp[4].format = VK_FORMAT_R32G32B32_SFLOAT;
		attrs_descp[4].offset = offsetof(GPUVertex, poly_normal);

		attrs_descp[5].binding = 0;
		attrs_descp[5].location = 5;
		attrs_descp[5].format = VK_FORMAT_R32G32_SFLOAT;
		attrs_descp[5].offset = offsetof(GPUVertex, uv);

		attrs_descp[6].binding = 0;
		attrs_descp[6].location = 6;
		attrs_descp[6].format = VK_FORMAT_R32G32B32_SFLOAT;
		attrs_descp[6].offset = offsetof(GPUVertex, color);

		return attrs_descp;
	}


	ErrorStack DescriptorSetLayout::create(LogicalDevice* logical_dev)
	{
		this->logical_dev = logical_dev;

		VkResult res;

		bindings.resize(3);

		VkDescriptorSetLayoutBinding uniform_bind = {};
		uniform_bind.binding = 0;
		uniform_bind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniform_bind.descriptorCount = 1;
		uniform_bind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[0] = uniform_bind;

		VkDescriptorSetLayoutBinding storage_bind = {};
		storage_bind.binding = 1;
		storage_bind.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		storage_bind.descriptorCount = 1;
		storage_bind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[1] = storage_bind;

		VkDescriptorSetLayoutBinding sampler_bind = {};
		sampler_bind.binding = 2;
		sampler_bind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sampler_bind.descriptorCount = 1;
		sampler_bind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[2] = sampler_bind;

		VkDescriptorSetLayoutCreateInfo descp_layout_info = {};
		descp_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descp_layout_info.bindingCount = (uint32_t)(bindings.size());
		descp_layout_info.pBindings = bindings.data();

		res = vkCreateDescriptorSetLayout(logical_dev->logical_device, &descp_layout_info, NULL, &descp_layout);
		if (res != VK_SUCCESS) {
			return ErrorStack(res, code_location, "failed to create descriptor set layout");
		}

		return ErrorStack();
	}

	void DescriptorSetLayout::destroy()
	{
		vkDestroyDescriptorSetLayout(logical_dev->logical_device, this->descp_layout, NULL);
		descp_layout = VK_NULL_HANDLE;
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		if (descp_layout != VK_NULL_HANDLE) {
			destroy();
		}
	}


	ErrorStack DescriptorPool::create(LogicalDevice* logical_dev)
	{
		this->logical_dev = logical_dev;

		std::array<VkDescriptorPoolSize, 3> pool_sizes;

		size_t i = 0;

		pool_sizes[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[i].descriptorCount = 1;  // swapchain image count ???

		i++;
		pool_sizes[i].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_sizes[i].descriptorCount = 1;

		i++;
		pool_sizes[i].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes[i].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descp_pool_info = {};
		descp_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descp_pool_info.maxSets = 1;
		descp_pool_info.poolSizeCount = (uint32_t)(pool_sizes.size());
		descp_pool_info.pPoolSizes = pool_sizes.data();

		VkResult vk_res = vkCreateDescriptorPool(logical_dev->logical_device, &descp_pool_info, NULL, &descp_pool);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, code_location, "failed to create descriptor pool");
		}

		return ErrorStack();
	}

	void DescriptorPool::destroy()
	{
		vkDestroyDescriptorPool(logical_dev->logical_device, descp_pool, NULL);
		descp_pool = VK_NULL_HANDLE;
	}

	DescriptorPool::~DescriptorPool()
	{
		if (descp_pool != VK_NULL_HANDLE) {
			destroy();
		}
	}


	ErrorStack DescriptorSet::create(LogicalDevice* logical_dev, DescriptorSetLayout* descp_layout, 
		DescriptorPool* descp_pool)
	{
		this->logical_dev = logical_dev;

		VkDescriptorSetAllocateInfo descp_sets_info = {};
		descp_sets_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descp_sets_info.descriptorPool = descp_pool->descp_pool;
		descp_sets_info.descriptorSetCount = 1;
		descp_sets_info.pSetLayouts = &descp_layout->descp_layout;

		VkResult res = vkAllocateDescriptorSets(logical_dev->logical_device, &descp_sets_info, &descp_set);
		if (res != VK_SUCCESS) {
			return ErrorStack(res, code_location, "failed to allocate descriptor sets");
		}
		return ErrorStack();
	}

	void DescriptorSet::update(Buffer* uniform_buff, Buffer* storage_buff, Sampler* sampler, 
		ImageView* img_view, VkImageLayout img_layout)
	{
		descp_writes.clear();

		// Uniform
		{
			VkDescriptorBufferInfo uniform_buff_info = {};
			uniform_buff_info.buffer = uniform_buff->buff;
			uniform_buff_info.offset = 0;
			uniform_buff_info.range = uniform_buff->buff_alloc_info.size;

			VkWriteDescriptorSet uniform_write = {};
			uniform_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uniform_write.dstSet = descp_set;
			uniform_write.dstBinding = 0;
			uniform_write.dstArrayElement = 0;
			uniform_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uniform_write.descriptorCount = 1;
			uniform_write.pBufferInfo = &uniform_buff_info;

			descp_writes.push_back(uniform_write);
		}

		// Storage
		{
			VkDescriptorBufferInfo storage_buff_info = {};
			storage_buff_info.buffer = storage_buff->buff;
			storage_buff_info.offset = 0;
			storage_buff_info.range = storage_buff->buff_alloc_info.size;

			VkWriteDescriptorSet storage_write = {};
			storage_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			storage_write.dstSet = descp_set;
			storage_write.dstBinding = 1;
			storage_write.dstArrayElement = 0;
			storage_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			storage_write.descriptorCount = 1;
			storage_write.pBufferInfo = &storage_buff_info;

			descp_writes.push_back(storage_write);
		}

		// Sampler
		{
			VkDescriptorImageInfo image_info = {};
			image_info.sampler = sampler->sampler;
			image_info.imageView = img_view->img_view;
			image_info.imageLayout = img_layout;

			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = descp_set;
			write.dstBinding = 2;
			write.dstArrayElement = 0;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.pImageInfo = &image_info;

			descp_writes.push_back(write);
		}

		if (descp_writes.size()) {
			vkUpdateDescriptorSets(logical_dev->logical_device,
				(uint32_t)descp_writes.size(), descp_writes.data(), 0, NULL);
		}
	}
	

}
