
// Header
#include "ErrorStack.hpp"


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
	err.origin = origin;
}

void ErrStack::pushError(std::string location, std::string msg)
{
	Error& err = this->error_stack.emplace_back();
	err.location = location;
	err.msg = msg;
}

void ErrStack::pushError(std::string location, std::string msg, int32_t err_code, ErrorOrigins origin)
{
	Error& err = this->error_stack.emplace_back();
	err.location = location;
	err.msg = msg;
	err.err_code = err_code;
	err.origin = origin;
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

		switch (error.origin) {
		case ErrorOrigins::NOT_SPECIFIED:
			printf("  type= NOT_SPECIFIED \n");
			printf("  err_code= %I32d \n", error.err_code);
			break;

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

std::string getLastError()
{
	LPSTR buffer;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		0,
		(LPSTR)&buffer,
		0,
		NULL
	);

	std::string error_msg = buffer;

	LocalFree(buffer);

	return error_msg;
}

std::string asIs(char c)
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

DirectX11Exception::DirectX11Exception(HRESULT hr, std::string msg)
{
	this->hresult = hr;
	this->message = msg;
}

void throwDX11(HRESULT hresult)
{
	if (hresult != S_OK) {
		throw DirectX11Exception(hresult, "exception message not set");
	}
}

void throwDX11(HRESULT hresult, const char* message)
{
	if (hresult != S_OK) {
		throw DirectX11Exception(hresult, std::string(message));
	}
}

WindowsException::WindowsException(std::string message_on_error)
{
	this->programer_message = message_on_error;
	this->api_message = getLastError();
}
