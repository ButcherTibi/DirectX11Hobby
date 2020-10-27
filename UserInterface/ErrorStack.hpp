#pragma once

// Standard
#include <stdint.h>
#include <string>
#include <vector>


#define code_location \
	("ln = " + std::to_string(__LINE__) + \
	" fn = " + __func__ + \
	" in file " + __FILE__).c_str()

#define varName(variabile) \
	#variabile


enum class ErrorOrigins {
	NOT_SPECIFIED,
	VULKAN_ERROR_CODE,
	WINDOWS_ERROR_HANDLE,
	WINDOWS_LAST_ERROR
};


struct Error {
	int32_t err_code;

	std::string location;
	std::string msg;
	ErrorOrigins type = ErrorOrigins::NOT_SPECIFIED;
};

	
class ErrStack {
public:
	std::vector<Error> error_stack;

public:
	ErrStack();
	ErrStack(std::string location, std::string msg);
	ErrStack(std::string location, std::string msg, int32_t err_code, ErrorOrigins origin);

	void pushError(std::string location, std::string msg);

	Error lastError();

	bool isBad();

	void debugPrint();
};

std::string getLastError();

std::string asIs(char c);


#define checkErrStack(err_func, msg) \
	err_stack = err_func; \
	if (err_stack.isBad()) { \
		err_stack.pushError(code_location, msg); \
		return err_stack; \
	}


#define checkErrStack1(err_func) \
	err_stack = err_func; \
	if (err_stack.isBad()) { \
		err_stack.pushError(code_location, ""); \
		return err_stack; \
	}


#define checkHResult(hr_func, msg) \
	hr = hr_func; \
	if (hr != S_OK) { \
		return ErrStack(code_location, msg, hr, ErrorOrigins::WINDOWS_ERROR_HANDLE); \
	}


#define assert_cond(param, msg) \
	if (param != true) \
		printf("%s %s%s%s %s \n", code_location, \
			"WARNING: assertion failed for condition (", #param, ") isn't true, ", msg);
