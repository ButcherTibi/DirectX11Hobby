
#include <iostream>

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

ErrorStack::ErrorStack()
{
	//
}

ErrorStack::ErrorStack(Error err)
{
	this->error_stack.push_back(err);
}

ErrorStack::ErrorStack(std::string location, std::string msg)
{
	this->error_stack.push_back(Error(location, msg));
}

ErrorStack::ErrorStack(ExtraError res, std::string location, std::string msg)
{
	this->error_stack.push_back(Error(res, location, msg));
}

ErrorStack::ErrorStack(ExtraError res, std::string location, std::string msg, std::string win_msg)
{
	this->error_stack.push_back(Error(res, location, msg + " Windows Error: " + win_msg));
}

ErrorStack::ErrorStack(ExtraError err, std::string location, std::string msg, uint64_t i, std::vector<char>& text)
{
	uint64_t line = 1;
	uint64_t col = 1;

	for (uint64_t idx = 0; idx <= i; idx++) {

		char& c = text[idx];

		if (c == '\n') {
			line++;
			col = 1;
			continue;
		}
		printf("%I64u %c \n", col, c);
		col++;
	}

	this->error_stack.push_back(Error(err, location, msg + 
		" at ln= " + std::to_string(line) + " col= " + std::to_string(col - 1) ));
}

ErrorStack::ErrorStack(VkResult res, std::string location, std::string msg)
{
	this->error_stack.push_back(Error(res, location, msg));
}

ErrorStack::ErrorStack(VkResult res, ExtraError err, std::string location, std::string msg)
{
	this->error_stack.push_back(Error(res, location, msg));
}

void ErrorStack::report(std::string location, std::string msg)
{
	this->error_stack.push_back(Error(location, msg));
}

void ErrorStack::report(std::string location, std::string msg, uint64_t line, uint64_t col)
{
	this->error_stack.push_back(Error(location, msg +
		" at ln= " + std::to_string(line) + " col= " + std::to_string(col)));
}

Error ErrorStack::lastError()
{
	return this->error_stack.back();
}

bool ErrorStack::isOk()
{
	if (this->error_stack.size() > 0) {
		return false;
	}
	return true;
}

bool ErrorStack::isBad()
{
	if (this->error_stack.size() > 0) {
		return true;
	}
	return false;
}

void ErrorStack::debugPrint()
{
	std::cout << "ErrorStack :" << std::endl;
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
		return std::string("\\n");
	case '\r':
		return std::string("\\r");
	}

	return std::string(1, c);
}
