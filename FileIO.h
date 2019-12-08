#pragma once

// Standard
#include <vector>

// mine
#include "ErrorStuff.h"


extern size_t max_path;


class Path {
public:
	std::vector<std::string> entries;

public:
	Path();
	Path(std::string path);

public:
	/* Queries */

	/* does the path starts from drive letter */
	bool isAbsolute();

	/* does path point to a directory */
	ErrorStack isDirectory(bool &is_dir);

	/* try to access directory or file pointed by path */
	bool isAccesible();

	bool hasExtension(std::string ext);


	/* Modify */

	void push_back(Path path);
	void push_back(std::string path);
	void pop_back(size_t count = 1);

	void push_front(Path path);
	void push_front(std::string path);
	void pop_front(size_t count = 1);

	bool erase(size_t start, size_t end);


	/* Conversions */

	/* Win32 requires this path format = "\\?\C:\Directory\file.txt" (aparently) */
	std::vector<char> toWin32Path();

	/*  */
	std::string toString();

	/* Reads */

	/* reads the file pointed by this path
	 * intended for reading shader code and JSON */
	ErrorStack read(std::vector<char>& content);


	// debug
	void check();
};


// retrieves executable path
ErrorStack getExePath(Path& exe_path);
