
// Mine
#include "MathUtils.h"

// Header
#include "VulkanSystems.h"


namespace vks {

	ErrStack find_layers(const std::vector<VkLayerProperties>& layer_props, const std::vector<const char*>& layers)
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
				return ErrStack(ExtraError::VALIDATION_LAYER_NOT_FOUND, code_location, "validation layer = " + std::string(layer) + " not found");
			}
		}
		return ErrStack();
	}

	ErrStack find_extensions(const std::vector<VkExtensionProperties>& ext_props, const std::vector<const char*>& extensions)
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
				return ErrStack(ExtraError::EXTENSION_NOT_FOUND, code_location, "instance extension = " + std::string(extension) + " not found");
			}
		}
		return ErrStack();
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "Vulkan Debug: " << pCallbackData->pMessage << std::endl << std::endl;

		return VK_FALSE;
	}

	ErrStack Instance::create()
	{
		VkResult vk_res;
		ErrStack err;

		// Find Layers
		{
			uint32_t layer_props_count = 0;

			vk_res = vkEnumerateInstanceLayerProperties(&layer_props_count, NULL);
			if (vk_res != VK_SUCCESS) {
				return ErrStack(vk_res, code_location, "could not retrieve validation layer props count");
			}

			std::vector<VkLayerProperties> layer_props(layer_props_count);

			vk_res = vkEnumerateInstanceLayerProperties(&layer_props_count, layer_props.data());
			if (vk_res != VK_SUCCESS) {
				return ErrStack(vk_res, code_location, "could not retrieve validation layer props");
			}

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
				return ErrStack(vk_res, ExtraError::FAILED_ENUMERATE_INSTANCE_EXTENSIONS, code_location, "could not retrieve instance extension props count");
			}

			std::vector<VkExtensionProperties> ext_props(ext_props_count);

			vk_res = vkEnumerateInstanceExtensionProperties(NULL, &ext_props_count, ext_props.data());
			if (vk_res != VK_SUCCESS) {
				return ErrStack(vk_res, ExtraError::FAILED_ENUMERATE_INSTANCE_EXTENSIONS, code_location, "could not retrieve instance extension props");
			}

			err = find_extensions(ext_props, instance_extensions);
			if (err.isBad()) {
				return err;
			}
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
				return ErrStack(vk_res, ExtraError::INSTANCE_CREATION_FAILURE, code_location, "could not create a Vulkan instance");
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
					return ErrStack(vk_res, code_location, "debug callback creation failed");
				}
			}
			else {
				return ErrStack(ExtraError::DEBUG_EXTENSION_NOT_FOUND, code_location, "debug extension not present");
			}
		}

		// External Functions
		{
			set_vkdbg_name_func = (PFN_vkSetDebugUtilsObjectNameEXT)
				vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
			if (set_vkdbg_name_func == NULL) {
				return ErrStack(ExtraError::FAILED_TO_GET_EXTERNAL_FUNCTION, code_location, 
					"failed to retrieve function pointer for "
					"vkSetDebugUtilsObjectNameEXT");
			}
		}

		return ErrStack();
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


	ErrStack Surface::create(Instance* instance, HINSTANCE hinstance, HWND hwnd)
	{
		this->instance = instance;

		VkWin32SurfaceCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		info.hinstance = hinstance;
		info.hwnd = hwnd;

		VkResult vk_res = vkCreateWin32SurfaceKHR(instance->instance, &info, NULL, &this->surface);
		if (vk_res != VK_SUCCESS) {
			return ErrStack(vk_res, code_location, "failed to create vulkan surface");
		}

		return ErrStack();
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
		phys_dev_features.sampleRateShading = VK_TRUE;
	}

	ErrStack PhysicalDevice::create(Instance* instance, Surface* surface)
	{
		VkResult vk_res;
		ErrStack err;

		uint32_t deviceCount = 0;

		vk_res = vkEnumeratePhysicalDevices(instance->instance, &deviceCount, NULL);
		if (vk_res != VK_SUCCESS) {
			return ErrStack(vk_res, ExtraError::FAILED_TO_ENUMERATE_PHYSICAL_DEVICES, code_location, "failed to enumerate physical devices");
		}

		if (deviceCount == 0) {
			return ErrStack(ExtraError::NO_GPU_WITH_VULKAN_SUPPORT_FOUND, code_location, "failed to find GPUs with Vulkan support");
		}

		std::vector<VkPhysicalDevice> phys_devices(deviceCount);

		vk_res = vkEnumeratePhysicalDevices(instance->instance, &deviceCount, phys_devices.data());
		if (vk_res != VK_SUCCESS) {
			return ErrStack(vk_res, ExtraError::FAILED_TO_ENUMERATE_PHYSICAL_DEVICES, code_location, "failed to enumerate physical devices");
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
						return ErrStack(vk_res, code_location, "failed to check if pshysical device can present");
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
					return ErrStack(code_location, "failed to retrieve enumerate device extension properties count");
				}

				std::vector<VkExtensionProperties> available_extensions(extension_count);
				if (vkEnumerateDeviceExtensionProperties(phys_dev, NULL, &extension_count, available_extensions.data()) != VK_SUCCESS) {
					return ErrStack(code_location, "failed to retrieve enumerate device extension properties");
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
			return ErrStack(ExtraError::NO_SUITABLE_GPU_FOUND, code_location, "failed to find a suitable GPU");
		}

		vkGetPhysicalDeviceProperties(physical_device, &this->phys_dev_props);
		vkGetPhysicalDeviceMemoryProperties(physical_device, &this->mem_props);


		{
			VkSampleCountFlags counts = phys_dev_props.limits.framebufferColorSampleCounts & phys_dev_props.limits.framebufferDepthSampleCounts;
			if (counts & VK_SAMPLE_COUNT_64_BIT) {
				this->max_MSAA = VK_SAMPLE_COUNT_64_BIT;
			}
			else if (counts & VK_SAMPLE_COUNT_32_BIT) {
				this->max_MSAA = VK_SAMPLE_COUNT_32_BIT;
			}
			else if (counts & VK_SAMPLE_COUNT_16_BIT) {
				this->max_MSAA = VK_SAMPLE_COUNT_16_BIT;
			}
			else if (counts & VK_SAMPLE_COUNT_8_BIT) {
				this->max_MSAA = VK_SAMPLE_COUNT_8_BIT;
			}
			else if (counts & VK_SAMPLE_COUNT_4_BIT) {
				this->max_MSAA = VK_SAMPLE_COUNT_4_BIT;
			}
			else if (counts & VK_SAMPLE_COUNT_2_BIT) {
				this->max_MSAA = VK_SAMPLE_COUNT_2_BIT;
			}
		}

		return ErrStack();
	}


	ErrStack LogicalDevice::create(Instance* instance, PhysicalDevice* phys_dev)
	{
		this->instance = instance;

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
			return ErrStack(vk_res, ExtraError::FAILED_CREATE_LOGICAL_DEVICE, code_location, "failed to create logical device");
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
				return ErrStack(vk_res, code_location, "failed to create allocator");
			}
		}

		return ErrStack();
	}

	ErrStack LogicalDevice::setDebugName(uint64_t obj, VkObjectType obj_type, std::string name)
	{
		VkDebugUtilsObjectNameInfoEXT info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		info.objectType = obj_type;
		info.objectHandle = obj;
		info.pObjectName = name.c_str();

		VkResult res = instance->set_vkdbg_name_func(this->logical_device, &info);
		if (res != VK_SUCCESS) {
			return ErrStack(code_location, "failed to set debug name");
		}

		return ErrStack();
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


	ErrStack Swapchain::create(Surface* surface, PhysicalDevice* phys_dev, LogicalDevice* logical_dev, 
		uint32_t width, uint32_t height)
	{
		this->logical_device = logical_dev;

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
				return ErrStack(ExtraError::NO_SUITABLE_SURFACE_FORMAT_FOUND, code_location, "failed to find suitable surface format");
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

		// Image and ImageViews
		{
			uint32_t image_count = 0;
			checkVkRes(vkGetSwapchainImagesKHR(logical_device->logical_device, swapchain, &image_count, NULL),
				"failed to retrieve swap chain image count");

			images.resize(image_count);
			checkVkRes(vkGetSwapchainImagesKHR(logical_device->logical_device, swapchain, &image_count, images.data()),
				"failed to retrieve swap chain images");

			VkComponentMapping component_mapping = {};
			component_mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			component_mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			component_mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			component_mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			VkImageSubresourceRange res_range = {};
			res_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			res_range.baseMipLevel = 0;
			res_range.levelCount = 1;
			res_range.baseArrayLayer = 0;
			res_range.layerCount = 1;;

			views.resize(image_count);
			for (uint32_t i = 0; i < image_count; i++) {

				VkImageViewCreateInfo view_info = {};
				view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				view_info.image = images[i];
				view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
				view_info.format = surface_format.format;
				view_info.components = component_mapping;
				view_info.subresourceRange = res_range;

				checkVkRes(vkCreateImageView(logical_dev->logical_device, &view_info, NULL, &views[i]),
					"failed to create swapchain image views");
			}
		}	

		return ErrStack();
	}

	ErrStack Swapchain::setDebugName(std::string name)
	{
		checkErrStack(logical_device->setDebugName(
			reinterpret_cast<uint64_t>(swapchain), VK_OBJECT_TYPE_SWAPCHAIN_KHR, "Swapchain"), "");

		for (size_t i = 0; i < images.size(); i++) {

			logical_device->setDebugName(
				reinterpret_cast<uint64_t>(images[i]), VK_OBJECT_TYPE_IMAGE,
				name + " VkImage[" + std::to_string(i) + "]");

			checkErrStack(logical_device->setDebugName(
				reinterpret_cast<uint64_t>(views[i]), VK_OBJECT_TYPE_IMAGE_VIEW, 
				name + " VkImageView[" + std::to_string(i) + "]"), "");
		}

		return ErrStack();
	}

	void Swapchain::destroy()
	{
		vkDestroySwapchainKHR(logical_device->logical_device, swapchain, NULL);
		swapchain = VK_NULL_HANDLE;

		for (size_t i = 0; i < views.size(); i++) {
			vkDestroyImageView(logical_device->logical_device, views[i], NULL);
		}
		views.clear();
	}

	Swapchain::~Swapchain()
	{
		if (swapchain != VK_NULL_HANDLE) {
			destroy();
		}
	}


	ErrStack Sampler::create(LogicalDevice* logical_dev, VkSamplerCreateInfo& info)
	{
		this->logical_dev = logical_dev;	

		checkVkRes(vkCreateSampler(logical_dev->logical_device, &info, NULL, &sampler),
			"failed to create sampler");
		return ErrStack();
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


	ErrStack Renderpass::create(LogicalDevice* logical_dev, PhysicalDevice* phys_dev, VkFormat present_format, 
		VkFormat depth_format)
	{
		this->logical_dev = logical_dev;

		// Descriptions
		// 3D
		VkAttachmentDescription color_atach = {};
		color_atach.format = present_format;
		color_atach.samples = phys_dev->max_MSAA;
		color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depth_atach = {};
		depth_atach.format = depth_format;
		depth_atach.samples = phys_dev->max_MSAA;
		depth_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_atach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription msaa_resolve_atach = {};
		msaa_resolve_atach.format = present_format;
		msaa_resolve_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		msaa_resolve_atach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		msaa_resolve_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		msaa_resolve_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		msaa_resolve_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		msaa_resolve_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		msaa_resolve_atach.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// UI
		VkAttachmentDescription ui_color_atach = {};
		ui_color_atach.format = present_format;
		ui_color_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		ui_color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		ui_color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		ui_color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		ui_color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		ui_color_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ui_color_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Composition
		VkAttachmentDescription compose_atach = {};
		compose_atach.format = present_format;
		compose_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		compose_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		compose_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		compose_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		compose_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		compose_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		compose_atach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Reference
		VkAttachmentReference g3d_color_attach_ref = {};
		g3d_color_attach_ref.attachment = 0;
		g3d_color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attach_ref = {};
		depth_attach_ref.attachment = 1;
		depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference msaa_resolve_read_attach_ref = {};
		msaa_resolve_read_attach_ref.attachment = 2;
		msaa_resolve_read_attach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference ui_color_attach_ref = {};
		ui_color_attach_ref.attachment = 3;
		ui_color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference ui_color_read_atach_ref = {};
		ui_color_read_atach_ref.attachment = 3;
		ui_color_read_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference compose_atach_ref = {};
		compose_atach_ref.attachment = 4;
		compose_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Attachments
		std::array<VkAttachmentDescription, 5> attachments = {
			color_atach, depth_atach, msaa_resolve_atach,
			ui_color_atach, compose_atach
		};

		// Subpass Description
		VkSubpassDescription g3d_subpass = {};
		g3d_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		g3d_subpass.colorAttachmentCount = 1;
		g3d_subpass.pColorAttachments = &g3d_color_attach_ref;
		g3d_subpass.pResolveAttachments = &msaa_resolve_read_attach_ref;
		g3d_subpass.pDepthStencilAttachment = &depth_attach_ref;

		// UI
		VkSubpassDescription ui_subpass = {};
		ui_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		ui_subpass.colorAttachmentCount = 1;
		ui_subpass.pColorAttachments = &ui_color_attach_ref;

		// Composition
		std::array<VkAttachmentReference, 2> input_atachments = {
			msaa_resolve_read_attach_ref, ui_color_read_atach_ref
		};

		VkSubpassDescription compose_subpass = {};
		compose_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		compose_subpass.inputAttachmentCount = (uint32_t)input_atachments.size();
		compose_subpass.pInputAttachments = input_atachments.data();
		compose_subpass.colorAttachmentCount = 1;
		compose_subpass.pColorAttachments = &compose_atach_ref;

		std::array<VkSubpassDescription, 3> subpass_descps = {
			g3d_subpass, ui_subpass, compose_subpass
		};

		// Subpass Dependency
		VkSubpassDependency g3d_compose_depend = {};
		g3d_compose_depend.srcSubpass = 0;
		g3d_compose_depend.dstSubpass = 2;
		g3d_compose_depend.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		g3d_compose_depend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		g3d_compose_depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		g3d_compose_depend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubpassDependency ui_compose_depend = {};
		ui_compose_depend.srcSubpass = 1;
		ui_compose_depend.dstSubpass = 2;
		ui_compose_depend.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		ui_compose_depend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		ui_compose_depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		ui_compose_depend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		std::array<VkSubpassDependency, 2> depends{
			g3d_compose_depend, ui_compose_depend
		};

		VkRenderPassCreateInfo renderpass_info = {};
		renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderpass_info.attachmentCount = (uint32_t)attachments.size();
		renderpass_info.pAttachments = attachments.data();
		renderpass_info.subpassCount = (uint32_t)subpass_descps.size();
		renderpass_info.pSubpasses = subpass_descps.data();
		renderpass_info.dependencyCount = (uint32_t)depends.size();
		renderpass_info.pDependencies = depends.data();

		checkVkRes(vkCreateRenderPass(logical_dev->logical_device, &renderpass_info, NULL, &renderpass),
			"failed to create renderpass");
		return ErrStack();
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


	ErrStack Framebuffers::create(LogicalDevice* logical_dev, FrameBufferCreateInfo& info,
		Renderpass* renderpass, uint32_t width, uint32_t height)
	{
		this->logical_dev = logical_dev;

		size_t img_count = info.swapchain->images.size();
		frame_buffs.resize(img_count);

		for (size_t i = 0; i < img_count; i++) {

			std::array<VkImageView, 5> attachments = {
				info.g3d_color_MSAA_img->img_view,
				info.g3d_depth_img->img_view,
				info.g3d_color_resolve_img->img_view,
				info.ui_color_img->img_view,
				info.swapchain->views[i]
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
		return ErrStack();
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


	ErrStack CommandPool::create(LogicalDevice* logical_dev, PhysicalDevice* phys_dev)
	{
		this->logical_dev = logical_dev;

		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.queueFamilyIndex = phys_dev->queue_fam_idx;

		checkVkRes(vkCreateCommandPool(logical_dev->logical_device, &info, NULL, &cmd_pool),
			"failed to create command pool");
		return ErrStack();
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


	SingleCommandBuffer::SingleCommandBuffer(LogicalDevice* logical_dev, CommandPool* cmd_pool, ErrStack* r_err)
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
			*err = ErrStack(vk_res, code_location, "failed to create single use command buffer");
			return;
		}

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vk_res = vkBeginCommandBuffer(cmd_buff, &beginInfo);
		if (vk_res != VK_SUCCESS) {
			*err = ErrStack(vk_res, code_location, "failed to begin single use command buffer");
			return;
		}
	}

	SingleCommandBuffer::~SingleCommandBuffer()
	{
		VkResult vk_res = vkEndCommandBuffer(cmd_buff);
		if (vk_res != VK_SUCCESS) {
			*err = ErrStack(vk_res, code_location, "failed to end single use command buffer");
			return;
		}

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd_buff;

		vk_res = vkQueueSubmit(logical_dev->queue, 1, &submitInfo, VK_NULL_HANDLE);
		if (vk_res != VK_SUCCESS) {
			*err = ErrStack(vk_res, code_location, "failed to submit single use command buffer");
			return;
		}

		vk_res = vkQueueWaitIdle(logical_dev->queue);
		if (vk_res != VK_SUCCESS) {
			*err = ErrStack(vk_res, code_location, "");
			return;
		}

		vkFreeCommandBuffers(logical_dev->logical_device, cmd_pool->cmd_pool, 1, &cmd_buff);
	}
	
	ErrStack Descriptor::create(LogicalDevice* logical_dev, std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		this->logical_dev = logical_dev;

		// Descriptor Layout
		{
			VkDescriptorSetLayoutCreateInfo descp_layout_info = {};
			descp_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descp_layout_info.bindingCount = (uint32_t)(bindings.size());
			descp_layout_info.pBindings = bindings.data();

			VkResult res = vkCreateDescriptorSetLayout(logical_dev->logical_device, &descp_layout_info, NULL, &descp_layout);
			if (res != VK_SUCCESS) {
				return ErrStack(res, code_location, "failed to create descriptor set layout");
			}
		}

		std::vector<VkDescriptorPoolSize> pools(bindings.size());
		for (uint32_t i = 0; i < bindings.size(); i++) {
			pools[i].type = bindings[i].descriptorType;
			pools[i].descriptorCount = bindings[i].descriptorCount;
		}

		// Descriptor Pool
		{
			VkDescriptorPoolCreateInfo descp_pool_info = {};
			descp_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descp_pool_info.maxSets = 1;
			descp_pool_info.poolSizeCount = (uint32_t)(pools.size());
			descp_pool_info.pPoolSizes = pools.data();

			VkResult vk_res = vkCreateDescriptorPool(logical_dev->logical_device, &descp_pool_info, NULL, &descp_pool);
			if (vk_res != VK_SUCCESS) {
				return ErrStack(vk_res, code_location, "failed to create descriptor pool");
			}
		}

		// Descriptor Sets
		{
			VkDescriptorSetAllocateInfo descp_sets_info = {};
			descp_sets_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descp_sets_info.descriptorPool = descp_pool;
			descp_sets_info.descriptorSetCount = 1;
			descp_sets_info.pSetLayouts = &descp_layout;

			VkResult res = vkAllocateDescriptorSets(logical_dev->logical_device, &descp_sets_info, &descp_set);
			if (res != VK_SUCCESS) {
				return ErrStack(res, code_location, "failed to allocate descriptor sizes");
			}
		}

		return ErrStack();
	}

	ErrStack Descriptor::setDebugName(std::string name)
	{
		checkErrStack1(logical_dev->setDebugName(
			reinterpret_cast<uint64_t>(descp_set), VK_OBJECT_TYPE_DESCRIPTOR_SET, 
			name + " VkDescriptorSet"));

		return ErrStack();
	}

	void Descriptor::update(std::vector<DescriptorWrite>& descp_writes)
	{
		this->writes.resize(descp_writes.size());

		for (size_t i = 0; i < descp_writes.size(); i++) {

			VkWriteDescriptorSet& vk_write = this->writes[i];
			DescriptorWrite& write = descp_writes[i];

			vk_write = {};
			vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			vk_write.dstSet = descp_set;
			vk_write.dstBinding = write.dstBinding;
			vk_write.dstArrayElement = write.dstArrayElement;
			vk_write.descriptorCount = write.descriptorCount;
			vk_write.descriptorType = write.descriptorType;
			vk_write.pImageInfo = write.img_info;
			vk_write.pBufferInfo = write.buff_info;
		}

		vkUpdateDescriptorSets(logical_dev->logical_device,
			(uint32_t)writes.size(), writes.data(), 0, NULL);
	}

	void Descriptor::destroyLayout()
	{
		vkDestroyDescriptorSetLayout(logical_dev->logical_device, this->descp_layout, NULL);
		descp_layout = VK_NULL_HANDLE;
	}

	void Descriptor::destroyPool()
	{
		vkDestroyDescriptorPool(logical_dev->logical_device, this->descp_pool, NULL);
		descp_pool = VK_NULL_HANDLE;
	}

	Descriptor::~Descriptor()
	{
		if (descp_layout != VK_NULL_HANDLE) {
			destroyLayout();
		}

		if (descp_pool != VK_NULL_HANDLE) {
			destroyPool();
		}
	}
}
