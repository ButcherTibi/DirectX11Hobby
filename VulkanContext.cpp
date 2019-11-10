

#include "VulkanContext.h"


static ErrorStack find_layers(const std::vector<VkLayerProperties> &layer_props, const std::vector<const char*> &layers)
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

static ErrorStack find_extensions(const std::vector<VkExtensionProperties> &ext_props, const std::vector<const char*> &extensions)
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

std::string Instance::description()
{
	return "Vulkan object wrapper for VkInstance, also enables validation layer debuging";
}

void Instance::defaultValues()
{
	std::cout << "Instance defaultValues" << std::endl;

	validation_layers = { "VK_LAYER_LUNARG_standard_validation" };
	instance_extensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
	};

	// AppInfo
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Vulkan Aplication";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	// Instance Creation info
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
}

ErrorStack Instance::build()
{
	std::cout << "Instance build" << std::endl;

	ErrorStack result;
	VkResult vk_result;

	// Instance
	{
		// Layers
		{
			uint32_t layer_props_count = 0;

			vk_result = vkEnumerateInstanceLayerProperties(&layer_props_count, NULL);
			if (vk_result != VK_SUCCESS) {
				return ErrorStack(vk_result, code_location, "could not retrieve validation layer props count");
			}

			std::vector<VkLayerProperties> layer_props(layer_props_count);

			vk_result = vkEnumerateInstanceLayerProperties(&layer_props_count, layer_props.data());
			if (vk_result != VK_SUCCESS) {
				return ErrorStack(vk_result, code_location, "could not retrieve validation layer props");
			}

			result = find_layers(layer_props, validation_layers);
			if (result.isBad()) {
				return result;
			}
		}
		inst_info.enabledLayerCount = (uint32_t)(validation_layers.size());
		inst_info.ppEnabledLayerNames = validation_layers.data();

		// Extensions
		{
			uint32_t ext_props_count = 0;

			vk_result = vkEnumerateInstanceExtensionProperties(NULL, &ext_props_count, NULL);
			if (vk_result != VK_SUCCESS) {
				return ErrorStack(vk_result, ExtraError::FAILED_ENUMERATE_INSTANCE_EXTENSIONS, code_location, "could not retrieve instance extension props count");
			}

			std::vector<VkExtensionProperties> ext_props(ext_props_count);

			vk_result = vkEnumerateInstanceExtensionProperties(NULL, &ext_props_count, ext_props.data());
			if (vk_result != VK_SUCCESS) {
				return ErrorStack(vk_result, ExtraError::FAILED_ENUMERATE_INSTANCE_EXTENSIONS, code_location, "could not retrieve instance extension props");
			}

			result = find_extensions(ext_props, instance_extensions);
			if (result.isBad()) {
				return result;
			}

			instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		inst_info.enabledExtensionCount = (uint32_t)(instance_extensions.size());
		inst_info.ppEnabledExtensionNames = instance_extensions.data();

		vk_result = vkCreateInstance(&inst_info, NULL, &instance);
		if (vk_result != VK_SUCCESS) {
			return ErrorStack(vk_result, ExtraError::INSTANCE_CREATION_FAILURE, code_location, "could not create a Vulkan instance");
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

			vk_result = func(instance, &createInfo, NULL, &callback);
			if (vk_result != VK_SUCCESS) {
				return ErrorStack(vk_result, code_location, "debug callback creation failed");
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
	std::cout << "Instance destroy" << std::endl;

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
	instance = VK_NULL_HANDLE;
}

Instance::~Instance()
{
	destroy();
}

std::string Surface::description()
{
	return "Vulkan object wrapper for VkSurfaceKHR which provides a context for drawing on a window";
}

ErrorStack Surface::build()
{
	std::cout << "Surface build" << std::endl;

	VkWin32SurfaceCreateInfoKHR surface_info = {};
	surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_info.hinstance = hinstance;
	surface_info.hwnd = hwnd;

	VkResult vk_err = vkCreateWin32SurfaceKHR(inst->instance, &surface_info, NULL, &surface);
	if (vk_err != VK_SUCCESS) {
		return ErrorStack(vk_err, ExtraError::FAILED_TO_CREATE_WIN32_SURFACE, code_location, "failed to create win32 surface");
	}
	return ErrorStack();
}

void Surface::init(HINSTANCE hinstance, HWND hwnd, Instance* instance)
{
	std::cout << "Surface init" << std::endl;

	this->hinstance = hinstance;
	this->hwnd = hwnd;
	this->inst = instance;
}

void Surface::destroy()
{
	std::cout << "Surface destroy" << std::endl;

	vkDestroySurfaceKHR(inst->instance, surface, NULL);
	surface = VK_NULL_HANDLE;
}

Surface::~Surface()
{
	if (inst != nullptr) {
		destroy();
	}	
}

std::string Device::description()
{
	return "Vulkan object wrapper for Physical device + Logical Device association, "
		"handles choosing which physical device to use, enables device features and "
		"specifies physical device limitations";
}

bool Device::isDeviceSuitable(VkPhysicalDevice device)
{
	// Find Queue Familly Index
	{
		bool ok_queue_family = false;

		uint32_t family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, queue_families.data());

		for (uint32_t i = 0; i < family_count; i++) {

			VkQueueFamilyProperties family_prop = queue_families[i];

			if (family_prop.queueCount > 0) {

				// If queue family supports graphics operations
				bool queue_fam_found = false;
				if (family_prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					this->queue_fam_idx = i;
					queue_fam_found = true;
				}

				// If family can present to surface
				VkBool32 can_present = false;
				if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surf->surface, &can_present) != VK_SUCCESS) {

					std::cerr << code_location << "failed to test if physical device surface can present" << std::endl;
					return false;
				}
					
				if (queue_fam_found && can_present)
				{
					ok_queue_family = true;
					break;
				}
			}
		}

		if (!ok_queue_family) {
			return false;
		}
	}

	// Check for extension support
	{
		uint32_t extension_count;
		if (vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL) != VK_SUCCESS) {

			std::cerr << code_location << "failed to retrieve enumerate device extension properties count" << std::endl;
			return false;
		}

		std::vector<VkExtensionProperties> available_extensions(extension_count);
		if (vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, available_extensions.data()) != VK_SUCCESS) {

			std::cerr << code_location << "failed to retrieve enumerate device extension properties" << std::endl;
			return false;
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
			return false;
		}
	}

	// Check for device features
	{
		VkPhysicalDeviceFeatures available_features;
		vkGetPhysicalDeviceFeatures(device, &available_features);

		if (phys_dev_features.vertexPipelineStoresAndAtomics &&
			phys_dev_features.vertexPipelineStoresAndAtomics != available_features.vertexPipelineStoresAndAtomics) 
		{
			return false;  // put readonly to vertex shader storage buffer to not require this
		}
	}

	return true;
}

ErrorStack Device::buildPhysDevice()
{
	VkResult vkresult;

	// Physical Device
	{
		uint32_t deviceCount = 0;

		vkresult = vkEnumeratePhysicalDevices(inst->instance, &deviceCount, NULL);
		if (vkresult != VK_SUCCESS) {
			return ErrorStack(vkresult, ExtraError::FAILED_TO_ENUMERATE_PHYSICAL_DEVICES, code_location, "failed to enumerate physical devices");
		}

		if (deviceCount == 0) {
			return ErrorStack(ExtraError::NO_GPU_WITH_VULKAN_SUPPORT_FOUND, code_location, "failed to find GPUs with Vulkan support");
		}

		std::vector<VkPhysicalDevice> phys_devices(deviceCount);

		vkresult = vkEnumeratePhysicalDevices(inst->instance, &deviceCount, phys_devices.data());
		if (vkresult != VK_SUCCESS) {
			return ErrorStack(vkresult, ExtraError::FAILED_TO_ENUMERATE_PHYSICAL_DEVICES, code_location, "failed to enumerate physical devices");
		}

		for (VkPhysicalDevice phys_dev : phys_devices) {
			if (isDeviceSuitable(phys_dev)) {
				physical_device = phys_dev;
				break;
			}
		}

		if (physical_device == VK_NULL_HANDLE) {
			return ErrorStack(ExtraError::NO_SUITABLE_GPU_FOUND, code_location, "failed to find a suitable GPU");
		}
	}

	// Physical Props
	{
		vkGetPhysicalDeviceProperties(physical_device, &this->phys_dev_props);
		vkGetPhysicalDeviceMemoryProperties(physical_device, &this->mem_props);
	}

	VkResult vk_res;

	// Surface Props
	{
		uint32_t format_count;
		vk_res = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surf->surface, &format_count, NULL);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, ExtraError::FAILED_TO_GET_SURFACE_FORMATS, code_location, "failed to get physical device surface formats");
		}

		std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
		vk_res = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surf->surface, &format_count, surface_formats.data());
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, ExtraError::FAILED_TO_GET_SURFACE_FORMATS, code_location, "failed to get physical device surface formats");
		}

		bool format_found = false;
		for (VkSurfaceFormatKHR available_format : surface_formats) {

			if ((available_format.format == VK_FORMAT_R8G8B8A8_UNORM ||
				available_format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
				(available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
			{
				this->surface_format = available_format;
				format_found = true;
				break;
			}
		}

		if (!format_found) {
			return ErrorStack(ExtraError::NO_SUITABLE_SURFACE_FORMAT_FOUND, code_location, "failed to find suitable surface format");
		}
	}
		
	// Device Surface Presentation Modes
	{
		uint32_t count;
		vk_res = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surf->surface, &count, NULL);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, ExtraError::FAILED_TO_GET_PRESENTATION_MODES, code_location, "failed to detect supported presentation modes");
		}

		present_modes.resize(count);

		vk_res = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surf->surface, &count, present_modes.data());
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, ExtraError::FAILED_TO_GET_PRESENTATION_MODES, code_location, "failed to detect supported presentation modes");
		}
	}

	return ErrorStack();
}

ErrorStack Device::buildLogicalDevice()
{
	float queue_priority = 1.0f;

	// Graphics queue
	VkDeviceQueueCreateInfo graphics_queue_info = {};
	graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphics_queue_info.queueFamilyIndex = queue_fam_idx;
	graphics_queue_info.queueCount = 1;
	graphics_queue_info.pQueuePriorities = &queue_priority;

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	// Queue
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &graphics_queue_info;

	// Layers
	device_info.enabledLayerCount = (uint32_t)(inst->validation_layers.size());
	device_info.ppEnabledLayerNames = inst->validation_layers.data();

	// Extensions
	device_info.enabledExtensionCount = (uint32_t)(device_extensions.size());
	device_info.ppEnabledExtensionNames = device_extensions.data();

	// Features
	device_info.pEnabledFeatures = &phys_dev_features;

	VkResult vk_res = vkCreateDevice(physical_device, &device_info, NULL, &logical_device);
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, ExtraError::FAILED_CREATE_LOGICAL_DEVICE, code_location, "failed to create logical device");
	}

	// Queue creation
	vkGetDeviceQueue(logical_device, queue_fam_idx, 0, &queue);

	return ErrorStack();
}

ErrorStack Device::build()
{
	std::cout << "Device build" << std::endl;

	ErrorStack res = buildPhysDevice();
	if (res.isBad()) {
		return res;
	}

	res = buildLogicalDevice();
	if (res.isBad()) {
		return res;
	}

	return ErrorStack();
}

void Device::init(Instance *instance, Surface *surf)
{
	std::cout << "Device init" << std::endl;

	this->inst = instance;
	this->surf = surf;
}

void Device::destroy()
{
	std::cout << "Device destroy" << std::endl;

	vkDestroyDevice(logical_device, NULL);
}

Device::~Device()
{
	destroy();
}