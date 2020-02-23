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
	ErrStack isDirectory(bool &is_dir);

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

	void removeExtension();

	/* Conversions */

	/* Win32 requires this path format = "\\?\C:\Directory\file.txt" (aparently) */
	std::vector<char> toWin32Path();

	/*  */
	std::string toWindowsPath();

	/* Reads */

	/* reads the file pointed by this path
	 * intended for reading shader code and JSON */
	template<typename T = char>
	ErrStack read(std::vector<T>& content);
	
	/*  */

	static ErrStack getExePath(Path& exe_path);
	static ErrStack getLocalFolder(Path& local_path);


	/*  */

	// debug
	void check();
};
