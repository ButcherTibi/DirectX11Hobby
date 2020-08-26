
#include "pch.h"

// Header
#include "ErrorStack.hpp"


using namespace nui;


ErrStack::ErrStack()
{

}

ErrStack::ErrStack(std::string location, std::string msg)
{
	Error& err = this->error_stack.emplace_back();
	err.location = location;
	err.msg = msg;
}

ErrStack::ErrStack(std::string location, std::string msg, int32_t err_code, ErrorOrigins origin)
{
	Error& err = this->error_stack.emplace_back();
	err.err_code = err_code;
	err.location = location;
	err.msg = msg;
	err.type = origin;
}

void ErrStack::pushError(std::string location, std::string msg)
{
	Error& err = this->error_stack.emplace_back();
	err.location = location;
	err.msg = msg;
}

Error ErrStack::lastError()
{
	return error_stack.back();
}

bool ErrStack::isBad()
{
	if (error_stack.size()) {
		return true;
	}
	return false;
}

void ErrStack::debugPrint()
{
	printf("ErrStack: \n");
	for (size_t i = 0; i < error_stack.size(); i++) {

		Error& error = error_stack[i];
		printf("%I64d. \n", i);
		printf("  %s \n", error.location.c_str());
		printf("  msg= %s \n", error.msg.c_str());

		switch (error.type) {
		case ErrorOrigins::VULKAN_ERROR_CODE:
			printf("  type= VULKAN_ERROR_CODE \n");
			printf("  err_code= %I32d 0x%I32x \n", error.err_code, error.err_code);
			break;

		case ErrorOrigins::WINDOWS_ERROR_HANDLE:
			printf("  type= WINDOWS_ERROR_HANDLE \n");
			printf("  err_code= %I32d 0x%I32x \n", error.err_code, error.err_code);
			break;
		}
	}
}

std::string nui::getLastError()
{
	LPSTR buffer;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		LANG_SYSTEM_DEFAULT,
		(LPSTR)&buffer,
		0,
		NULL
	);

	std::string error_msg = std::string(buffer);

	LocalFree(buffer);

	return error_msg;
}

std::string nui::asIs(char c)
{
	switch (c)
	{
	case '\n':
		return std::string("'\\n'");
	case '\r':
		return std::string("'\\r'");
	}

	return "'" + std::string(1, c) + "'";
}
