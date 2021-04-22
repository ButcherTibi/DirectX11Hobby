#pragma once

// Standard
#include <stdint.h>
#include <string>
#include <vector>


enum class ErrorOrigins {
	NOT_SPECIFIED,
	VULKAN_ERROR_CODE,
	WINDOWS_ERROR_HANDLE,
	WINDOWS_LAST_ERROR,
	USER_ACTION
};


struct Error {
	int32_t err_code;

	std::string location;
	std::string msg;
	ErrorOrigins origin = ErrorOrigins::NOT_SPECIFIED;
};


class ErrStack {
public:
	std::vector<Error> error_stack;

public:
	ErrStack();
	ErrStack(std::string location, std::string msg);
	ErrStack(std::string location, std::string msg, int32_t err_code, ErrorOrigins origin);

	void pushError(std::string location, std::string msg);
	void pushError(std::string location, std::string msg, int32_t err_code, ErrorOrigins origin);

	Error lastError();

	bool isBad();

	void debugPrint();
};


std::string getLastError();

std::string asIs(char c);


#define code_location \
	("ln = " + std::to_string(__LINE__) + \
	" fn = " + __func__ + \
	" in file " + __FILE__).c_str()

#define varName(variabile) \
	#variabile


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

#define checkHResult1(hr_func) \
	hr = hr_func; \
	if (hr != S_OK) { \
		return ErrStack(code_location, "no message", hr, ErrorOrigins::WINDOWS_ERROR_HANDLE); \
	}


/* Exceptions */

inline void assert_cond(bool condition) {
#ifndef NDEBUG  // or _DEBUG
	if (condition != true) {
		throw std::exception();
	}
#endif
}

inline void assert_cond(bool condition, const char* msg) {
#ifndef NDEBUG  // or _DEBUG
	if (condition != true) {
		throw std::exception(msg);
	}
#endif
}


struct DirectX11Exception : std::exception {
	HRESULT hresult;  // the result handle of the DirectX11 call
	std::string message;  // message writen by hand by the programer at call site

	DirectX11Exception() = delete;
	DirectX11Exception(HRESULT hr, std::string msg);
};

void throwDX11(HRESULT hresult);
void throwDX11(HRESULT hresult, const char* message);


struct WindowsException : std::exception {
	std::string api_message;
	std::string message;

	WindowsException(std::string message_on_error);
};

// void throwErrStack(ErrStack err_stack);
