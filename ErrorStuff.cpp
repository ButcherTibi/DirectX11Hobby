


// Header
#include "ErrorStuff.h"


Error::Error(std::string location, std::string msg)
{
	this->vk_err = VK_RESULT_MAX_ENUM;
	this->err = ExtraError::OK;

	this->location = location;
	this->msg = msg;
}

Error::Error(ExtraError res, std::string location, std::string msg)
{
	this->vk_err = VK_RESULT_MAX_ENUM;
	this->err = res;

	this->location = location;
	this->msg = msg;
}

Error::Error(VkResult res, std::string location, std::string msg)
{
	this->vk_err = res;
	this->err = ExtraError::OK;

	this->location = location;
	this->msg = msg;
}

Error::Error(VkResult res, ExtraError err, std::string location, std::string msg)
{
	this->vk_err = res;
	this->err = err;

	this->location = location;
	this->msg = msg;
}

ErrStack::ErrStack()
{
	
}

ErrStack::ErrStack(Error err)
{
	error_stack.push_back(err);
}

ErrStack::ErrStack(std::string location, std::string msg)
{
	error_stack.push_back(Error(location, msg));
}

ErrStack::ErrStack(ExtraError res, std::string location, std::string msg)
{
	error_stack.push_back(Error(res, location, msg));
}

ErrStack::ErrStack(ExtraError res, std::string location, std::string msg, std::string win_msg)
{
	error_stack.push_back(Error(res, location, msg + " Windows Error: " + win_msg));
}

ErrStack::ErrStack(VkResult res, std::string location, std::string msg)
{
	error_stack.push_back(Error(res, location, msg));
}

ErrStack::ErrStack(VkResult res, ExtraError err, std::string location, std::string msg)
{
	error_stack.push_back(Error(res, location, msg));
}

void ErrStack::pushError(std::string location, std::string msg)
{
	error_stack.push_back(Error(location, msg));
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
	std::cout << "ErrStack :" << std::endl;
	for (size_t i = 0; i < error_stack.size(); i++) {

		Error& error = error_stack[i];
		std::cout << i << "." << std::endl;
		std::cout << "  " << error.location << std::endl;
		std::cout << "  msg = " << error.msg << std::endl;
	}
}

std::string getLastError()
{
	LPSTR buffer;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		LANG_SYSTEM_DEFAULT,
		(LPSTR)& buffer,
		0,
		NULL
	);

	std::string error_msg = std::string(buffer);

	LocalFree(buffer);

	return error_msg;
}

/* escapes control caracters */
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

#if (_DEBUG)
	uint64_t debug_trigger_0 = 0;
#endif
