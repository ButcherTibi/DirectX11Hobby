#pragma once

// Standard
#include <vector>
#include <set>

// Windows
#include <windows.h>

// Vulkan
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

// own
#include "ErrorStuff.h"


class Instance
{
public:
	// Content
	VkApplicationInfo app_info = {};
	VkInstanceCreateInfo inst_info = {};

	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT callback;

	std::vector<const char*> validation_layers;
	std::vector<const char*> instance_extensions;

	// Vulkan debug layer
	//bool enable_debug_layers = true;
	VkDebugUtilsMessageSeverityFlagsEXT debug_msg_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	// External Functions
	PFN_vkGetPhysicalDeviceMemoryProperties2KHR getMemProps2;

public:
	std::string description();

public:
	void defaultValues();

	ErrorStack build();

	void destroy();

	~Instance();
};


class Surface
{
public:
	Instance* inst = nullptr;
	HINSTANCE hinstance;
	HWND hwnd;

	// Content
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	bool builded = false;

public:
	std::string description();

public:
	void init(HINSTANCE hinstance, HWND hwnd, Instance* instance);

	ErrorStack build();
	void destroy();

	~Surface();
};

class Device
{
public:
	// Parents
	Instance* inst = nullptr;
	Surface* surf;

	// Physical device
	VkPhysicalDeviceFeatures phys_dev_features = {};
	std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_MEMORY_BUDGET_EXTENSION_NAME };

	VkPhysicalDevice physical_device;

	// Device
	VkPhysicalDeviceProperties phys_dev_props;
	VkPhysicalDeviceMemoryProperties mem_props;

	// Surface (maybe update these whenever swapchain changes ?)
	VkSurfaceFormatKHR surface_format;
	std::vector<VkPresentModeKHR> present_modes;

	// Logical devoce
	VkDevice logical_device = VK_NULL_HANDLE;

	uint32_t queue_fam_idx;
	VkQueue queue;

public:
	std::string description();

private:
	bool isDeviceSuitable(VkPhysicalDevice device);

public:
	void init(Instance* instance, Surface* surf);

	ErrorStack buildPhysDevice();
	ErrorStack buildLogicalDevice();
	ErrorStack build();

	void destroy();

	~Device();
};
