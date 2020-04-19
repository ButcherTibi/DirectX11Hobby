#pragma once

// Standard
#include <vector>


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
	FAILED_TO_READ_FILE,

	// Importer
	FAILED_TO_PARSE_JSON,
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
	FAILED_TO_GET_SURFACE_FORMATS_COUNT,
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
struct ErrStack
{
	std::vector<Error> error_stack;

	ErrStack();
	ErrStack(Error err);

	ErrStack(std::string location, std::string msg);

	ErrStack(ExtraError res, std::string location, std::string msg);
	ErrStack(ExtraError res, std::string location, std::string msg, std::string win_msg);

	ErrStack(VkResult res, std::string location, std::string msg);
	ErrStack(VkResult res, ExtraError err, std::string location, std::string msg);

	void pushError(std::string location, std::string msg);

	Error lastError();

	bool isBad();

	void debugPrint();
};

/* returns the last Win32 error */
std::string getLastError();

std::string asIs(char c);


#define checkErrStack(err_stack, msg) \
	if (err_stack.isBad()) { \
		err_stack.pushError(code_location, msg); \
		return err_stack; \
	}

#define checkErrStack1(err_stack) \
	if (err_stack.isBad()) { \
		err_stack.pushError(code_location, ""); \
		return err_stack; \
	}

#define checkVkRes(vk_res, msg) \
	if (vk_res != VK_SUCCESS) { \
		return ErrStack(vk_res, code_location, msg); \
	}

/* Debug variable */

#if (_DEBUG)
	extern uint64_t debug_trigger_0;
#endif


/* Assertions */

#if (_DEBUG)
	constexpr bool enable_runtime_assertions = true;
#else
	constexpr bool enable_runtime_assertions = false;
#endif

// prints a message, if condition is false
#define assert_cond(param, msg) \
	if constexpr(enable_runtime_assertions) \
		if (param != true) \
			std::cout << code_location << "WARNING: assertion failed for condition (" << #param << ") isnt true, " << msg << std::endl;