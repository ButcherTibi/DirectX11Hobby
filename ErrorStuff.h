#pragma once

// Standard
#include <vector>
#include <iostream>

// Vulkan
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>


#define code_location \
	"ln = " + std::to_string(__LINE__) + \
	" fn = " + __func__ + \
	" in file " + __FILE__


#define var_name(var) \
	std::to_string(#var)


/* Additional errors not part of the vulkan standard */
enum class ExtraError {
	// Input
	FAILED_TO_GET_RAW_INPUT_BUFFER,
	FAILED_TO_GET_RAW_INPUT_DATA,
	FAILED_TO_GET_CURSOR_SCREEN_POSITION,

	// Aplication
	FAILED_TO_GET_WINDOW_SIZE,

	// Importer
	FAILED_TO_PARSE_GLTF,

	// Instance
	VALIDATION_LAYER_NOT_FOUND,
	FAILED_ENUMERATE_INSTANCE_EXTENSIONS,
	EXTENSION_NOT_FOUND,
	INSTANCE_CREATION_FAILURE,
	DEBUG_EXTENSION_NOT_FOUND,
	FAILED_TO_GET_EXTERNAL_FUNCTION,

	// Surface
	FAILED_TO_CREATE_WIN32_SURFACE,

	// Device
	FAILED_TO_ENUMERATE_PHYSICAL_DEVICES,
	NO_GPU_WITH_VULKAN_SUPPORT_FOUND,
	NO_SUITABLE_GPU_FOUND,
	FAILED_TO_GET_SURFACE_FORMATS,
	NO_SUITABLE_SURFACE_FORMAT_FOUND,
	FAILED_TO_GET_SURFACE_CAPABILITIES,
	FAILED_TO_GET_PRESENTATION_MODES,
	FAILED_CREATE_LOGICAL_DEVICE,

	// DeviceMemory
	NO_SUITABLE_MEMORY_TYPE,
	TOO_MANY_MEMORY_ALOCATIONS,
	FAILURE_TO_MAP_MEMORY,

	// CmdPool
	FAILED_TO_CREATE_COMAND_POOL,
	CMD_POOL_USAGE_TIME_EXCEDED,
	CMD_POOL_BUSY,

	// Cmd Buff and Draw
	FAILED_TO_CREATE_COMAND_BUFFER,
	FAILED_TO_BEGIN_RECORDING_CMD_BUFF,
	FAILED_TO_END_RECORDING_CMD_BUFF,
	FAILED_TO_SUBMIT_QUEUE,
	FAILED_TO_ACQUIRE_IMAGE,
	FAILED_TO_PRESENT_IMAGE,

	// Syncronization
	FAILED_TO_WAIT_ON_FENCE,
	FAILED_TO_RESET_FENCE,

	OK,
};


/* Error containing error cause, code location and message */
struct Error
{
	VkResult vk_err = VK_RESULT_MAX_ENUM;
	ExtraError err = ExtraError::OK;

	std::string location;
	std::string msg;

	Error(std::string location, std::string msg);
	Error(ExtraError res, std::string location, std::string msg);
	Error(VkResult res, std::string location, std::string msg);
	Error(VkResult res, ExtraError err, std::string location, std::string msg);
};

// change this to return an idx to a global vector of error
// add time maybe ?
struct ErrorStack
{
	std::vector<Error> error_stack;

	ErrorStack();
	ErrorStack(Error err);
	ErrorStack(std::string location, std::string msg);
	ErrorStack(ExtraError res, std::string location, std::string msg);

	/* with Windows message */
	ErrorStack(ExtraError res, std::string location, std::string msg, std::string win_msg);

	/* i = index of character in text, 
	 * i gets converted to line and colum and appended to message */
	ErrorStack(ExtraError err, std::string location, std::string msg, uint64_t i, std::vector<char>& text);

	ErrorStack(VkResult res, std::string location, std::string msg);
	ErrorStack(VkResult res, ExtraError err, std::string location, std::string msg);

	void report(std::string location, std::string msg);
	void report(std::string location, std::string msg, uint64_t line, uint64_t col);

	Error lastError();

	bool isOk();
	bool isBad();

	void debugPrint();
};

/* returns the last Win32 error */
std::string getLastError();

std::string asIs(char c);


/* Assertions */

constexpr bool enable_runtime_assertions = true;

// prints a message, if condition is false
#define assert_cond(param, msg) \
	if constexpr(enable_runtime_assertions) \
		if (param != true) \
			std::cout << code_location << "WARNING: assertion failed for condition (" << #param << ") isnt true, " << msg << std::endl;