
#include "pch.h"

// header
#include "VulkanWrapper.hpp"

using namespace vkw;
using namespace nui;

nui::ErrStack Image::create(VulkanDevice* dev, ImageCreateInfo& info)
{
	VkResult vk_res{};
	nui::ErrStack err_stack{};

	this->dev = dev;
	this->mem_props = info.mem_props;

	this->format = info.format;
	this->width = info.width;
	this->height = info.height;
	this->depth = info.depth;
	this->samples = info.samples;
	this->current_layout = info.initialLayout;

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

	checkVkRes(vkCreateImage(dev->logical_dev, &vk_info, NULL, &img),
		"failed to create image");

	VkMemoryRequirements mem_req;
	vkGetImageMemoryRequirements(dev->logical_dev, img, &mem_req);

	this->alloc_size = mem_req.size;

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_req.size;
	checkErrStack1(dev->findMemoryType(mem_req.memoryTypeBits, mem_props, alloc_info.memoryTypeIndex));

	checkVkRes(vkAllocateMemory(dev->logical_dev, &alloc_info, NULL, &mem),
		"failed to allocate image memory");

	checkVkRes(vkBindImageMemory(dev->logical_dev, img, mem, 0),
		"failed to bind memory to buffer");

	if (mem_props & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {

		this->load_type = LoadType::MEMCPY;

		checkVkRes(vkMapMemory(dev->logical_dev, mem, 0, alloc_size, 0, &mapped_mem),
			"failed to map buffer memory");
	}
	else {
		this->load_type = LoadType::STAGING;
		this->mapped_mem = nullptr;
	}

	return err_stack;
}

nui::ErrStack Image::createView(ImageViewCreateInfo& info, ImageView& view)
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

	return nui::ErrStack();
}

void Image::destroy()
{
	if (this->load_type == LoadType::MEMCPY) {
		vkUnmapMemory(dev->logical_dev, mem);
	}

	vkFreeMemory(dev->logical_dev, mem, NULL);
	vkDestroyImage(dev->logical_dev, img, NULL);
	img = VK_NULL_HANDLE;
}

Image::~Image()
{
	if (img != VK_NULL_HANDLE) {
		destroy();
	}
}

ErrStack ImageView::load(void* data, size_t size)
{
	ErrStack err_stack;

	checkErrStack(image->dev->staging_buff.load(data, size),
		"failed to load pixel data into staging buffer");

	CommandList& cmd_list = *image->dev->cmd_list;

	checkErrStack1(cmd_list.beginRecording());
	{
		// if image not in right layout then transition
		if (image->current_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
			image->current_layout != VK_IMAGE_LAYOUT_GENERAL)
		{
			ImageBarrier to_transfer;
			to_transfer.view = this;
			to_transfer.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			to_transfer.wait_for_access = 0;
			to_transfer.wait_at_access = VK_ACCESS_TRANSFER_WRITE_BIT;

			cmd_list.cmdPipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				to_transfer);
		}

		cmd_list.cmdCopyBufferToImage(image->dev->staging_buff, size, *this);
	}
	checkErrStack1(cmd_list.endRecording());

	checkErrStack1(cmd_list.run());
	checkErrStack1(cmd_list.waitForExecution());

	return ErrStack();
}

ImageView::~ImageView()
{
	if (image != nullptr) {
		vkDestroyImageView(image->dev->logical_dev, view, NULL);
	}
}

nui::ErrStack Buffer::load(void* data, size_t size)
{
	ErrStack err_stack;

	auto create = [&](size_t new_size) -> ErrStack {

		VkResult vk_res{};

		VkBufferCreateInfo buff_info = {};
		buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buff_info.flags = this->flags;
		buff_info.size = new_size;
		buff_info.usage = this->usage;
		buff_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		checkVkRes(vkCreateBuffer(dev->logical_dev, &buff_info, NULL, &buff),
			"failed to create buffer");

		VkMemoryRequirements mem_req;
		vkGetBufferMemoryRequirements(dev->logical_dev, buff, &mem_req);

		this->alloc_size = mem_req.size;

		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_req.size;
		checkErrStack1(dev->findMemoryType(mem_req.memoryTypeBits, mem_props, alloc_info.memoryTypeIndex));

		checkVkRes(vkAllocateMemory(dev->logical_dev, &alloc_info, NULL, &mem),
			"failed to allocate buffer memory");

		checkVkRes(vkBindBufferMemory(dev->logical_dev, buff, mem, 0),
			"failed to bind memory to buffer");

		if (mem_props & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {

			this->load_type = LoadType::MEMCPY;

			checkVkRes(vkMapMemory(dev->logical_dev, mem, 0, alloc_size, 0, &mapped_mem),
				"failed to map buffer memory");
		}
		else {
			this->load_type = LoadType::STAGING;
			this->mapped_mem = nullptr;
		}

		this->size = size;

		return ErrStack();
	};

	if (buff == VK_NULL_HANDLE) {

		checkErrStack(create(size),
			"failed to create vulkan buffer");
	}
	else if (this->size < size) {

		destroy();
		checkErrStack(create(size),
			"failed to create vulkan buffer");
	}

	if (this->load_type == LoadType::STAGING) {

		checkErrStack1(dev->staging_buff.load(data, size));

		checkErrStack1(dev->cmd_list->beginRecording());
		{
			dev->cmd_list->cmdCopyBuffer(dev->staging_buff, *this);
		}
		checkErrStack1(dev->cmd_list->endRecording());

		checkErrStack1(dev->cmd_list->run());
		checkErrStack1(dev->cmd_list->waitForExecution());
	}
	else if (this->load_type == LoadType::MEMCPY) {
		std::memcpy(this->mapped_mem, data, size);
	}

	return err_stack;
}

void Buffer::destroy()
{
	if (load_type == LoadType::MEMCPY) {
		vkUnmapMemory(dev->logical_dev, mem);
	}

	vkFreeMemory(dev->logical_dev, mem, NULL);
	vkDestroyBuffer(dev->logical_dev, buff, NULL);
	buff = VK_NULL_HANDLE;
}

Buffer::~Buffer()
{
	if (buff != VK_NULL_HANDLE) {
		destroy();
	}
}

Sampler::~Sampler()
{
	if (dev != nullptr) {
		vkDestroySampler(dev->logical_dev, sampler, NULL);
	}
}

void VertexInput::addAtribute(VkFormat format, uint32_t offset)
{
	VkVertexInputAttributeDescription& atribute = atributes.emplace_back();
	atribute.location = 0xFFFF'FFFF;
	atribute.binding = binding.binding;
	atribute.format = format;
	atribute.offset = offset;
}

static ErrStack find_layers(std::vector<VkLayerProperties>& layer_props, std::vector<const char*>& layers)
{
	for (const char* layer : layers) {

		bool found = false;
		for (VkLayerProperties& layer_prop : layer_props) {
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

static ErrStack find_extensions(std::vector<VkExtensionProperties>& ext_props, std::vector<const char*>& extensions)
{
	for (const char* extension : extensions) {

		bool found = false;
		for (VkExtensionProperties& ext_prop : ext_props) {
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
		inst_info.enabledLayerCount = (uint32_t)info.validation_layers.size();
		inst_info.ppEnabledLayerNames = info.validation_layers.data();
		inst_info.enabledExtensionCount = (uint32_t)info.instance_extensions.size();
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
	ErrStack err_stack;
	VkResult vk_res{};

	device.inst = this;
	device.cmd_list = new CommandList();

	// Surface
	Surface& surface = device.surface;
	{
		surface.dev = &device;

		VkWin32SurfaceCreateInfoKHR vk_info = {};
		vk_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		vk_info.pNext = info.surface_pNext;
		vk_info.flags = info.surface_flags;
		vk_info.hinstance = info.hinstance;
		vk_info.hwnd = info.hwnd;

		checkVkRes(vkCreateWin32SurfaceKHR(instance, &vk_info, NULL, &surface.surface),
			"failed to create vulkan surface");
	}

	// Physical Device
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

				VkBool32 can_present;
				vkGetPhysicalDeviceSurfaceSupportKHR(phys_dev, i, surface.surface, &can_present);

				if ((queue_families[i].queueFlags & info.queue_flags) && 
					can_present)
				{
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
			for (const char* req_extension : info.extensions) {
				for (VkExtensionProperties& extension : available_extensions) {

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

	// Logical Device
	{
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
		device_info.pNext = info.device_pNext;
		device_info.flags = info.device_flags;
		device_info.queueCreateInfoCount = 1;
		device_info.pQueueCreateInfos = &vk_queue_info;
		device_info.enabledLayerCount = (uint32_t)info.layers.size();
		device_info.ppEnabledLayerNames = info.layers.data();
		device_info.enabledExtensionCount = (uint32_t)info.extensions.size();
		device_info.ppEnabledExtensionNames = info.extensions.data();
		device_info.pEnabledFeatures = &info.features;

		checkVkRes(vkCreateDevice(device.phys_dev, &device_info, NULL, &device.logical_dev),
			"failed to create logical device");
	}
	
	// Swapchain
	{	
		VkSurfaceCapabilitiesKHR capabilities;
		checkErrStack1(surface.getSurfaceCapabilities(capabilities));

		surface.minImageCount = capabilities.minImageCount;

		uint32_t format_counts;
		checkVkRes(vkGetPhysicalDeviceSurfaceFormatsKHR(device.phys_dev, surface.surface, &format_counts, NULL),
			"failed to find physical surface format count");

		std::vector<VkSurfaceFormatKHR> formats(format_counts);
		checkVkRes(vkGetPhysicalDeviceSurfaceFormatsKHR(device.phys_dev, surface.surface, &format_counts, formats.data()),
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

		checkVkRes(vkCreateSwapchainKHR(device.logical_dev, &swapchain_info, NULL, &surface.swapchain),
			"failed to create swapchain");

		// Image and Image View of the swapchain
		uint32_t image_count = 0;
		checkVkRes(vkGetSwapchainImagesKHR(device.logical_dev, surface.swapchain, &image_count, NULL),
			"failed to retrieve swapchain image count");

		surface.swapchain_images.resize(image_count);
		checkVkRes(vkGetSwapchainImagesKHR(device.logical_dev, surface.swapchain, &image_count, surface.swapchain_images.data()),
			"failed to retrieve swapchain images");

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

			checkVkRes(vkCreateImageView(device.logical_dev, &view_info, NULL, &surface.swapchain_views[i]),
				"failed to create swapchain image views");
		}
	}

	// Device Queue
	vkGetDeviceQueue(device.logical_dev, device.queue_fam_idx,
		0, &device.queue);

	vkGetPhysicalDeviceMemoryProperties(device.phys_dev, &device.phys_mem_props);

	// Staging Buffer
	{
		BufferCreateInfo buff_info;
		buff_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buff_info.mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		device.createBuffer(buff_info, device.staging_buff);
	}

	// Command List
	{
		CommandListCreateInfo cmd_list_info;
		cmd_list_info.cmd_pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		checkErrStack1(device.createCommandList(cmd_list_info, *device.cmd_list));
	}

	return err_stack;
}

Instance::~Instance()
{
	if (this->callback != VK_NULL_HANDLE) {

		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != NULL) {
			func(instance, callback, NULL);
		}
		else {
			printf("%s debug callback already freed \n", code_location);
		}
	}

	vkDestroyInstance(instance, NULL);
}

ErrStack VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t& mem_type)
{
	for (uint32_t i = 0; i < phys_mem_props.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (phys_mem_props.memoryTypes[i].propertyFlags & properties) == properties) {

			mem_type = i;
			return ErrStack();
		}
	}

	throw ErrStack(code_location, "failed to find suitable memory type");
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

Semaphore::~Semaphore()
{
	if (dev != nullptr) {
		vkDestroySemaphore(dev->logical_dev, semaphore, NULL);
	}
}

Fence::~Fence()
{	
	if (dev != nullptr) {
		vkDestroyFence(dev->logical_dev, fence, NULL);
	}
}

ErrStack VulkanDevice::createImage(ImageCreateInfo& info, Image& texture)
{
	ErrStack err_stack{};

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
	img_info.mem_props = info.mem_props;

	checkErrStack1(texture.create(this, img_info));

	return err_stack;
}

void VulkanDevice::createBuffer(BufferCreateInfo& info, Buffer& buffer)
{
	ErrStack err_stack;

	buffer.dev = this;
	buffer.flags = info.flags;
	buffer.usage = info.usage;
	buffer.mem_props = info.mem_props;
}

ErrStack VulkanDevice::createSampler(SamplerCreateInfo& info, Sampler& sampler)
{
	VkResult vk_res{};

	sampler.dev = this;

	VkSamplerCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	vk_info.flags = info.flags;
	vk_info.magFilter = info.magFilter;
	vk_info.minFilter = info.minFilter;
	vk_info.mipmapMode = info.mipmapMode;
	vk_info.addressModeU = info.addressModeU;
	vk_info.addressModeV = info.addressModeV;
	vk_info.addressModeW = info.addressModeW;
	vk_info.mipLodBias = info.mipLodBias;
	vk_info.anisotropyEnable = info.anisotropyEnable;
	vk_info.maxAnisotropy = info.maxAnisotropy;
	vk_info.compareEnable = info.compareEnable;
	vk_info.compareOp = info.compareOp;
	vk_info.minLod = info.minLod;
	vk_info.maxLod = info.maxLod;
	vk_info.borderColor = info.borderColor;
	vk_info.unnormalizedCoordinates = info.unnormalizedCoordinates;

	checkVkRes(vkCreateSampler(logical_dev, &vk_info, NULL, &sampler.sampler),
		"failed to create sampler");

	return ErrStack();
}

ErrStack VulkanDevice::createShader(std::vector<char>& spirv, VkShaderStageFlagBits stage, Shader& shader)
{
	return shader.create(this, spirv, stage);
}

void VulkanDevice::createDrawpass(DrawpassCreateInfo& info, Drawpass& drawpass)
{
	drawpass.dev = this;
	drawpass.surface = &surface;
}

ErrStack VulkanDevice::createCommandList(CommandListCreateInfo& info, CommandList& cmd_list)
{
	VkResult vk_res;
	ErrStack err_stack;

	cmd_list.dev = this;
	cmd_list.surface = info.surface;

	if (info.surface != nullptr) {
		cmd_list.tasks.resize(info.surface->minImageCount);

		checkErrStack1(createSemaphore(cmd_list.swapchain_img_acquired));
		checkErrStack1(createSemaphore(cmd_list.execution_finished));
		checkErrStack1(createFence(0, cmd_list.execution_finished_fence));
	}
	else {
		cmd_list.tasks.resize(1);

		checkErrStack1(createFence(0, cmd_list.execution_finished_fence));
	}

	for (uint32_t i = 0; i < cmd_list.tasks.size(); i++) {

		CommandTask& task = cmd_list.tasks[i];
		task.idx = i;

		VkCommandPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = info.cmd_pool_flags;
		pool_info.queueFamilyIndex = queue_fam_idx;

		checkVkRes(vkCreateCommandPool(logical_dev, &pool_info, NULL, &task.pool),
			"failed to create Command Pool");

		VkCommandBufferAllocateInfo buff_info = {};
		buff_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		buff_info.commandPool = task.pool;
		buff_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		buff_info.commandBufferCount = 1;

		checkVkRes(vkAllocateCommandBuffers(logical_dev, &buff_info, &task.buff),
			"failed to allocate Command Buffer");
	}

	return ErrStack();
}

nui::ErrStack VulkanDevice::createSemaphore(Semaphore& semaphore)
{
	VkResult vk_res{};

	semaphore.dev = this;

	VkSemaphoreCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	checkVkRes(vkCreateSemaphore(logical_dev, &vk_info, NULL, &semaphore.semaphore),
		"failed to create semaphore");

	return ErrStack();
}

nui::ErrStack VulkanDevice::createFence(VkFenceCreateFlags flags, Fence& fence)
{
	VkResult vk_res{};

	fence.dev = this;

	VkFenceCreateInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vk_info.flags = flags;

	checkVkRes(vkCreateFence(logical_dev, &vk_info, NULL, &fence.fence),
		"failed to create fence");

	return ErrStack();
}

VulkanDevice::~VulkanDevice()
{
	// Surface
	{
		for (VkImageView& view : surface.swapchain_views) {
			vkDestroyImageView(logical_dev, view, NULL);
		}

		vkDestroySwapchainKHR(logical_dev, surface.swapchain, NULL);

		vkDestroySurfaceKHR(inst->instance, surface.surface, NULL);
	}
	
	staging_buff.destroy();
	delete cmd_list;

	vkDeviceWaitIdle(logical_dev);
	vkDestroyDevice(logical_dev, NULL);
}
