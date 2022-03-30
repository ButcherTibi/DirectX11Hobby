#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Standard
#include <string>


namespace win32 {

	// Win32's GetLastError but as a human readable string
	std::string getLastError();


	// Wrap Handle so that it frees memory automatically
	class Handle {
	public:
		HANDLE handle = INVALID_HANDLE_VALUE;;

	public:
		Handle(HANDLE ms_handle);

		Handle& operator=(HANDLE ms_handle);

		bool isValid();

		~Handle();
	};


	/// <summary>
	/// Prints to the Output window in Visual Studio Code.
	/// Usefull when printing from Dynamic Linked Library
	/// </summary>
	/// <param name="message">= message to be printed</param>
	void printToOutput(std::string message);
}
