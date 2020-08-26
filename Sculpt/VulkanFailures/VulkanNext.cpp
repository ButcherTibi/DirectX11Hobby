
// header
#include "VulkanNext.hpp"


using namespace vnx;


ErrStack Image::create(VulkanDevice* dev, ImageCreateInfo& info)
{
	VkResult vk_res{};
	ErrStack err_stack{};

	this->dev = dev;

	VmaAllocationCreateInfo alloc_create_info = {};
	alloc_create_info.usage = info.mem_usage;

	VkImageCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	vk_info.flags = info.flags;
	vk_info.imageType = info.imageType;
	vk_info.format = info.format;
	vk_info.extent.width = info.width;
	vk_info.extent.height = info.height;
	vk_info.extent.depth = info.depth;
	vk_info.mipLevels = info.mipLevels;
	vk_info.arrayLayers = info.arrayLayers;
	vk_info.samples = info.samples;
	vk_info.tiling = info.tiling;
	vk_info.usage = info.usage;
	vk_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vk_info.initialLayout = info.initialLayout;

	checkVkRes(vmaCreateImage(dev->allocator, &vk_info, &alloc_create_info,
		&img, &alloc, &alloc_info),
		"failed to create texture");

	this->format = info.format;
	this->samples = info.samples;
	this->current_layout = info.initialLayout;

	VkMemoryPropertyFlags mem_flags;
	vmaGetMemoryTypeProperties(dev->allocator, alloc_info.memoryType, &mem_flags);

	if (mem_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {

		load_type = LoadType::MEMCPY;
		checkVkRes(vmaMapMemory(dev->allocator, alloc, &mem), "");
	}
	else {
		load_type = LoadType::STAGING;
		mem = nullptr;
	}

	return err_stack;
}

ErrStack Image::createView(ImageViewCreateInfo& info, ImageView& view)
{
	VkResult vk_res{};

	view = {};
	view.image = this;

	VkImageSubresourceRange subres = {};
	subres.aspectMask = info.subres_range.aspectMask;
	subres.baseMipLevel = info.subres_range.baseMipLevel;
	subres.levelCount = info.subres_range.levelCount;
	subres.baseArrayLayer = info.subres_range.baseArrayLayer;
	subres.layerCount = info.subres_range.layerCount;

	VkImageViewCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vk_info.flags = info.flags;
	vk_info.image = img;
	vk_info.viewType = info.viewType;
	vk_info.format = format;
	vk_info.components = info.components;
	vk_info.subresourceRange = subres;

	checkVkRes(vkCreateImageView(dev->logical_dev, &vk_info, NULL, &view.view), 
		"failed to create ImageView");

	view.info = info;

	return ErrStack();
}

void Image::destroy()
{
	if (this->load_type == LoadType::MEMCPY) {
		vmaUnmapMemory(dev->allocator, alloc);
	}

	vmaDestroyImage(dev->allocator, img, alloc);
	img = VK_NULL_HANDLE;
}

Image::~Image()
{
	if (img != VK_NULL_HANDLE) {
		destroy();
	}
}

ImageView::~ImageView()
{
	vkDestroyImageView(image->dev->logical_dev, view, NULL);
}

FrameImage::~FrameImage()
{
	if (surface != nullptr) {
		for (auto it = surface->frame_images.begin(); it != surface->frame_images.end(); it++) {
			if (*it == this) {
				surface->frame_images.erase(it);
				return;
			}
		}
	}
}

ErrStack Buffer::create(VulkanDevice* dev, BufferCreateInfo& info)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	VkBufferCreateInfo buff_info = {};
	buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buff_info.flags = info.flags;
	buff_info.size = info.size;
	buff_info.usage = info.usage;

	VmaAllocationCreateInfo vma_info = {};
	vma_info.usage = info.vma_usage;

	checkVkRes(vmaCreateBuffer(dev->allocator, &buff_info, &vma_info,
		&buff, &buff_alloc, &vma_r_info),
		"failed to create buffer");

	VkMemoryPropertyFlags mem_flags;
	vmaGetMemoryTypeProperties(dev->allocator, vma_r_info.memoryType, &mem_flags);
	if (mem_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
		load_type = LoadType::MEMCPY;

		checkVkRes(vmaMapMemory(dev->allocator, buff_alloc, &mem),
			"failed to map buffer");
	}
	else {
		load_type = LoadType::STAGING;
		this->mem = nullptr;
	}

	return err_stack;
}

static ErrStack find_layers(const std::vector<VkLayerProperties>& layer_props, const std::vector<char*>& layers)
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
			return ErrStack(code_location, "validation layer = " + std::string(layer) + " not found");
		}
	}
	return ErrStack();
}

static ErrStack find_extensions(const std::vector<VkExtensionProperties>& ext_props, const std::vector<char*>& extensions)
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
			return ErrStack(code_location, "instance extension = " + std::string(extension) + " not found");
		}
	}
	return ErrStack();
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	printf("Vulkan Debug: %s \n\n", pCallbackData->pMessage);

	return VK_FALSE;
}

ErrStack Instance::create(InstanceCreateInfo& info)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	// Find Layers
	{
		uint32_t layer_props_count = 0;

		checkVkRes(vkEnumerateInstanceLayerProperties(&layer_props_count, NULL),
			"could not retrieve validation layer props count");

		std::vector<VkLayerProperties> layer_props(layer_props_count);

		checkVkRes(vkEnumerateInstanceLayerProperties(&layer_props_count, layer_props.data()),
			"could not retrieve validation layer props");

		checkErrStack1(find_layers(layer_props, info.validation_layers));
	}

	// Find Extensions
	{
		uint32_t ext_props_count = 0;

		checkVkRes(vkEnumerateInstanceExtensionProperties(NULL, &ext_props_count, NULL),
			"could not retrieve instance extension props count");

		std::vector<VkExtensionProperties> ext_props(ext_props_count);

		checkVkRes(vkEnumerateInstanceExtensionProperties(NULL, &ext_props_count, ext_props.data()),
			"could not retrieve instance extension props");

		checkErrStack1(find_extensions(ext_props, info.instance_extensions));
	}

	// Create Instance
	{
		// AppInfo
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = info.app_info.pNext;
		app_info.pApplicationName = info.app_info.pApplicationName;
		app_info.applicationVersion = info.app_info.applicationVersion;
		app_info.pEngineName = info.app_info.pEngineName;
		app_info.engineVersion = info.app_info.engineVersion;
		app_info.apiVersion = info.app_info.apiVersion;

		// Instance Creation info
		VkInstanceCreateInfo inst_info = {};
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = info.pNext;
		inst_info.flags = info.flags;
		inst_info.pApplicationInfo = &app_info;
		inst_info.enabledLayerCount = info.validation_layers.size();
		inst_info.ppEnabledLayerNames = info.validation_layers.data();
		inst_info.enabledExtensionCount = info.instance_extensions.size();
		inst_info.ppEnabledExtensionNames = info.instance_extensions.data();

		checkVkRes(vkCreateInstance(&inst_info, NULL, &instance),
			"failed to create instance");
	}

	// Debug callback
	{
		VkDebugUtilsMessengerCreateInfoEXT dbg_info = {};
		dbg_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		dbg_info.pNext = info.debug_pnext;
		dbg_info.flags = info.debug_flags;
		dbg_info.messageSeverity = info.debug_msg_severity;
		dbg_info.messageType = info.debug_msg_type;
		dbg_info.pfnUserCallback = debugCallback;
		dbg_info.pUserData = NULL;

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != NULL) {

			checkVkRes(func(this->instance, &dbg_info, NULL, &callback),
				"debug callback creation failed");
		}
		else {
			return ErrStack(code_location, "debug extension not present");
		}
	}

	// External Functions
	{
		set_vkdbg_name_func = (PFN_vkSetDebugUtilsObjectNameEXT)
			vkGetInstanceProcAddr(this->instance, "vkSetDebugUtilsObjectNameEXT");
		if (set_vkdbg_name_func == NULL) {
			return ErrStack(code_location,
				"failed to retrieve function pointer for "
				"vkSetDebugUtilsObjectNameEXT");
		}
	}

	return err_stack;
}

ErrStack Instance::createDevice(DeviceCreateInfo& info, VulkanDevice& device)
{
	VkResult vk_res{};

	device = {};
	device.inst = this;

	uint32_t deviceCount = 0;
	checkVkRes(vkEnumeratePhysicalDevices(instance, &deviceCount, NULL),
		"failed to get physical devices count");

	if (deviceCount == 0) {
		return ErrStack(code_location, "failed to find GPUs with Vulkan support");
	}

	std::vector<VkPhysicalDevice> vk_phys_devs(deviceCount);
	checkVkRes(vkEnumeratePhysicalDevices(instance, &deviceCount, vk_phys_devs.data()),
		"failed to enumerate physical devices");

	std::vector<VkQueueFamilyProperties> queue_families;

	for (VkPhysicalDevice& phys_dev : vk_phys_devs) {	

		device.queue_fam_idx = -1;
		{
			uint32_t family_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &family_count, nullptr);
			if (!family_count) {
				continue;
			}

			queue_families.clear();
			queue_families.resize(family_count);
			vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &family_count, queue_families.data());
			for (uint32_t i = 0; i < family_count; i++) {
				if (queue_families[i].queueFlags & info.queue_flags) {

					device.queue_fam_idx = i;
					break;
				}
			}

			if (device.queue_fam_idx == -1) {
				// queue family not found for required queue flags
				continue;
			}
		}	

		// Has Extensions
		{
			uint32_t extension_count;
			if (vkEnumerateDeviceExtensionProperties(phys_dev, NULL, &extension_count, NULL) != VK_SUCCESS) {
				continue;
			}

			std::vector<VkExtensionProperties> available_extensions(extension_count);
			if (vkEnumerateDeviceExtensionProperties(phys_dev, NULL, &extension_count, available_extensions.data()) != VK_SUCCESS) {
				continue;
			}

			uint32_t count = 0;
			for (char* req_extension : info.extensions) {
				for (VkExtensionProperties extension : available_extensions) {

					if (!std::strcmp(req_extension, extension.extensionName)) {
						count++;
						break;
					}
				}
			}

			if (count < info.extensions.size()) {
				// not all extensions found
				continue;
			}
		}

		// Has Features
		{
			VkPhysicalDeviceFeatures available_features;
			vkGetPhysicalDeviceFeatures(phys_dev, &available_features);

			if (info.features.samplerAnisotropy &&
				info.features.samplerAnisotropy != available_features.samplerAnisotropy)
			{
				continue;
			}
		}

		// Physical Device Passed All Requirements
		device.phys_dev = phys_dev;
		break;
	}

	if (device.phys_dev == VK_NULL_HANDLE) {
		return ErrStack(code_location, "failed to find suitable GPU");
	}

	float queue_priority = 1;
	VkDeviceQueueCreateInfo vk_queue_info = {};
	vk_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vk_queue_info.pNext = NULL;
	vk_queue_info.flags = 0;
	vk_queue_info.queueFamilyIndex = device.queue_fam_idx;
	vk_queue_info.queueCount = 1;
	vk_queue_info.pQueuePriorities = &queue_priority;

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = info.pNext;
	device_info.flags = info.flags;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &vk_queue_info;
	device_info.enabledLayerCount = info.layers.size();
	device_info.ppEnabledLayerNames = info.layers.data();
	device_info.enabledExtensionCount = info.extensions.size();
	device_info.ppEnabledExtensionNames = info.extensions.data();
	device_info.pEnabledFeatures = &info.features;

	checkVkRes(vkCreateDevice(device.phys_dev, &device_info, NULL, &device.logical_dev),
		"failed to create logical device");

	// VMA Allocator
	{
		VmaAllocatorCreateInfo alloc_info = {};
		alloc_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		alloc_info.instance = instance;
		alloc_info.physicalDevice = device.phys_dev;
		alloc_info.device = device.logical_dev;

		VkResult vk_res = vmaCreateAllocator(&alloc_info, &device.allocator);
		if (vk_res != VK_SUCCESS) {
			return ErrStack(code_location, "failed to create allocator");
		}
	}

	vkGetDeviceQueue(device.logical_dev, device.queue_fam_idx,
		0, &device.queue);

	return ErrStack();
}

ErrStack VulkanDevice::createWin32Surface(SurfaceCreateInfo& info, Surface& surface)
{
	ErrStack err_stack{};
	VkResult vk_res{};

	surface = {};
	surface.dev = this;

	VkWin32SurfaceCreateInfoKHR vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vk_info.pNext = info.surface_pNext;
	vk_info.flags = info.surface_flags;
	vk_info.hinstance = info.hinstance;
	vk_info.hwnd = info.hwnd;

	checkVkRes(vkCreateWin32SurfaceKHR(inst->instance, &vk_info, NULL, &surface.surface),
		"failed to create vulkan surface");
	
	// Swapchain
	VkSurfaceCapabilitiesKHR capabilities;
	checkErrStack1(surface.getSurfaceCapabilities(capabilities));

	surface.minImageCount = capabilities.minImageCount;

	uint32_t format_counts;
	checkVkRes(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface.surface, &format_counts, NULL),
		"failed to find physical surface format count");

	std::vector<VkSurfaceFormatKHR> formats(format_counts);
	checkVkRes(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface.surface, &format_counts, formats.data()),
		"failed to find physical surface format");

	bool found = false;
	for (VkSurfaceFormatKHR format : formats) {
		if (format.format == VK_FORMAT_R8G8B8A8_UNORM ||
			format.format == VK_FORMAT_B8G8R8A8_UNORM)
		{
			surface.imageFormat = format.format;
			found = true;
			break;
		}
	}

	if (!found) {
		return ErrStack(code_location, "failed to find suitable surface format");
	}

	surface.width = capabilities.currentExtent.width;
	surface.height = capabilities.currentExtent.height;

	VkSwapchainCreateInfoKHR swapchain_info = {};
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.pNext = NULL;
	swapchain_info.flags = info.swapchain_flags;
	swapchain_info.surface = surface.surface;
	swapchain_info.minImageCount = surface.minImageCount;
	swapchain_info.imageFormat = surface.imageFormat;
	swapchain_info.imageColorSpace = surface.imageColorSpace;
	swapchain_info.imageExtent.width = surface.width;
	swapchain_info.imageExtent.height = surface.height;
	swapchain_info.imageArrayLayers = info.imageArrayLayers;
	swapchain_info.imageUsage = info.imageUsage;
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.preTransform = info.preTransform;
	swapchain_info.compositeAlpha = info.compositeAlpha;
	swapchain_info.presentMode = info.presentMode;
	swapchain_info.clipped = info.clipped;
	swapchain_info.oldSwapchain = NULL;

	checkVkRes(vkCreateSwapchainKHR(logical_dev, &swapchain_info, NULL, &surface.swapchain),
		"failed to create swapchain");

	// Image and Image View of the swapchain
	uint32_t image_count = 0;
	checkVkRes(vkGetSwapchainImagesKHR(logical_dev, surface.swapchain, &image_count, NULL),
		"failed to retrieve swap chain image count");

	surface.swapchain_images.resize(image_count);
	checkVkRes(vkGetSwapchainImagesKHR(logical_dev, surface.swapchain, &image_count, surface.swapchain_images.data()),
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

	surface.swapchain_views.resize(image_count);
	for (uint32_t i = 0; i < image_count; i++) {

		VkImageViewCreateInfo view_info = {};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = surface.swapchain_images[i];
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = surface.imageFormat;
		view_info.components = component_mapping;
		view_info.subresourceRange = res_range;

		checkVkRes(vkCreateImageView(logical_dev, &view_info, NULL, &surface.swapchain_views[i]),
			"failed to create swapchain image views");
	}

	return ErrStack();
}

ErrStack Surface::getSurfaceCapabilities(VkSurfaceCapabilitiesKHR& capabilities)
{
	VkResult vk_res{};

	checkVkRes(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev->phys_dev, surface, &capabilities),
		"failed to get physical device surface capabilities");

	return ErrStack();
}

ErrStack Shader::create(VulkanDevice* dev, std::vector<char>& spirv, VkShaderStageFlagBits stage)
{
	VkResult vk_res{};

	this->dev = dev;
	this->stage = stage;

	VkShaderModuleCreateInfo shader_module_info = {};
	shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_info.codeSize = spirv.size();
	shader_module_info.pCode = reinterpret_cast<uint32_t*>(spirv.data());

	checkVkRes(vkCreateShaderModule(dev->logical_dev, &shader_module_info, NULL, &this->shader),
		"failed to create shader module");

	return ErrStack();
}

Shader::~Shader()
{
	vkDestroyShaderModule(dev->logical_dev, shader, NULL);
}

ErrStack Surface::createFrameImage(FrameImageCreateInfo& info, FrameImage& frame_image)
{
	ErrStack err_stack{};

	frame_image = {};
	frame_image.surface = this;

	ImageCreateInfo img_info = {};
	img_info.flags = info.flags;
	img_info.imageType = info.imageType;
	img_info.format = info.format;
	img_info.width = width;
	img_info.height = height;
	img_info.depth = info.depth;
	img_info.mipLevels = info.mipLevels;
	img_info.arrayLayers = info.arrayLayers;
	img_info.samples = info.samples;
	img_info.tiling = info.tiling;
	img_info.usage = info.usage;
	img_info.initialLayout = info.initialLayout;
	img_info.mem_usage = info.mem_usage;

	checkErrStack1(frame_image.create(dev, img_info));

	frame_images.push_back(&frame_image);
	
	return err_stack;
}

ErrStack VulkanDevice::createTexture(TextureCreateInfo& info, Texture& texture)
{
	ErrStack err_stack{};

	texture = {};

	ImageCreateInfo img_info = {};
	img_info.flags = info.flags;
	img_info.imageType = info.imageType;
	img_info.format = info.format;
	img_info.width = info.width;
	img_info.height = info.height;
	img_info.depth = info.depth;
	img_info.mipLevels = info.mipLevels;
	img_info.arrayLayers = info.arrayLayers;
	img_info.samples = info.samples;
	img_info.tiling = info.tiling;
	img_info.usage = info.usage;
	img_info.initialLayout = info.initialLayout;
	img_info.mem_usage = info.mem_usage;

	checkErrStack1(texture.create(this, img_info));

	return err_stack;
}

ErrStack VulkanDevice::createBuffer(BufferCreateInfo& info, Buffer& buffer)
{
	ErrStack err_stack{};

	checkErrStack1(buffer.create(this, info));

	return err_stack;
}

ErrStack VulkanDevice::createShader(std::vector<char>& spirv, VkShaderStageFlagBits stage, Shader& shader)
{
	stage = {};
	return shader.create(this, spirv, stage);
}

void VulkanDevice::createRenderpass(Surface& surface, RenderpassCreateInfo& info, Renderpass& renderpass)
{
	renderpass.dev = this;
	renderpass.surface = &surface;
}
