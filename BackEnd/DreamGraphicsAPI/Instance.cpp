// Header
#include "DreamGraphicsAPI.hpp"


using namespace dga;


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT,
	VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
    printf("Vulkan Debug: %s \n\n", pCallbackData->pMessage);

    return VK_FALSE;
}

void Instance::create()
{   
	// Instance
	{
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = nullptr;
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = nullptr;
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_3;

		// uint32_t extension_count = 0;
		// vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		
		// std::vector<VkExtensionProperties> extensions_props(extension_count);
		// vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions_props.data());

		VkInstanceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pApplicationInfo = &app_info;
		info.enabledLayerCount = (uint32_t)layers.size();
		info.ppEnabledLayerNames = layers.data();
		info.enabledExtensionCount = (uint32_t)extensions.size();
		info.ppEnabledExtensionNames = extensions.data();

		if (vkCreateInstance(&info, nullptr, &instance) != VK_SUCCESS) {
			throw;
		}
	}

	// Debug callback
	{
		auto createDebugUtilsMessanger = (PFN_vkCreateDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT");

		if (createDebugUtilsMessanger != nullptr) {

			VkDebugUtilsMessengerCreateInfoEXT info{};
			info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
			info.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			info.pfnUserCallback = debugCallback;
			info.pUserData = nullptr;

			VkResult vk_res = createDebugUtilsMessanger(instance, &info, NULL, &callback);
			if (vk_res != VK_SUCCESS) {
				throw;  // debug callback creation failed
			}
		}
		else {
			throw;  // function not found
		}
	}

	// External Functions
	{
		setDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)
			vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");

		if (setDebugUtilsObjectName == NULL) {
			throw;  // function not found
		}
	}
}

Instance::~Instance()
{
	if (this->callback != VK_NULL_HANDLE) {

		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != NULL) {
			func(instance, callback, NULL);
		}
		else {
			__debugbreak();  // debug callback already freed
		}
	}

	if (instance != nullptr) {
		vkDestroyInstance(instance, nullptr);
	}
}

void Instance:: getBestDevice(Device& r_device)
{
	uint32_t phys_dev_count;
	vkEnumeratePhysicalDevices(instance, &phys_dev_count, nullptr);

	std::vector<VkPhysicalDevice> phys_devs(phys_dev_count);
	vkEnumeratePhysicalDevices(instance, &phys_dev_count, phys_devs.data());

	VkPhysicalDevice best_phys_dev = nullptr;

	for (auto& phys_dev : phys_devs) {

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(phys_dev, &props);

		//VkPhysicalDeviceFeatures features;
		//vkGetPhysicalDeviceFeatures(phys_dev, &features);

		//VkPhysicalDeviceMemoryProperties mem_props;
		//vkGetPhysicalDeviceMemoryProperties(phys_dev, &mem_props);

		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			best_phys_dev = phys_dev;
		}
	}

	uint32_t best_queue_family_index = 0xFFFF'FFFF;
	{
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(best_phys_dev, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(best_phys_dev, &queue_family_count, queue_families.data());

		for (uint32_t i = 0; i < queue_family_count; i++) {

			VkQueueFamilyProperties& queue_family = queue_families[i];

			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				best_queue_family_index = i;
			}
		}
	}

	float queue_priority = 1.0f;

	// Logical Device
	{
		VkDeviceQueueCreateInfo queue_info{};
		queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info.queueFamilyIndex = best_queue_family_index;
		queue_info.queueCount = 1;
		queue_info.pQueuePriorities = &queue_priority;

		VkPhysicalDeviceFeatures features{};
		features.independentBlend = true;
		features.tessellationShader = true;
		features.samplerAnisotropy = true;

		VkDeviceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.queueCreateInfoCount = 1;
		info.pQueueCreateInfos = &queue_info;
		info.enabledLayerCount = (uint32_t)layers.size();
		info.ppEnabledLayerNames = layers.data();
		info.enabledExtensionCount = 0;
		info.ppEnabledExtensionNames = nullptr;
		info.pEnabledFeatures = &features;

		VkResult result = vkCreateDevice(best_phys_dev, &info, nullptr, &r_device.device);
		if (result != VK_SUCCESS) {
			throw;
		}
	}
	
	// Graphics Queue
	{
		vkGetDeviceQueue(r_device.device, best_queue_family_index, 0, &r_device.graphics_queue);
	}
}
