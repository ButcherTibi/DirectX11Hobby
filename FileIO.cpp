
#include <fstream>
#include <vector>

// Other
#include "ErrorStuff.h"

#include "FileIO.h"


size_t max_path = 1024;

uint32_t findEntryCount(std::string path)
{
	size_t last = path.size() - 1;
	uint32_t entry_count = 0;
	bool was_something = false;

	for (size_t i = 0; i < path.size(); i++) {

		if (i == last &&
			(path[i] != '\\' && path[i] != '/'))
		{
			// path = "e" or path = "entry"
			entry_count++;
		}
		else if (path[i] == '\\' || path[i] == '/') {

			// path = "entry\"
			if (was_something) {

				entry_count++;
				was_something = false;
			}
			// path = "\\\\\"
		}
		else {
			was_something = true;
		}
	}
	return entry_count;
}

bool isPathAbsolute(std::string path)
{
	if (path.find_first_of(":") == std::string::npos) {
		return false;
	}
	return true;
}

void push_path_to_vector(std::vector<std::string> &existing, std::string path)
{
	existing.reserve(existing.size() + findEntryCount(path));

	size_t last = path.size() - 1;
	std::string entry;
	bool was_something = false;

	for (uint32_t i = 0; i < path.size(); i++) {

		if (i == last &&
			(path[i] != '\\' && path[i] != '/'))
		{
			// path = "e" or path = "entry"
			entry.push_back(path[i]);
			existing.push_back(entry);
		}
		else if (path[i] == '\\' || path[i] == '/') {

			// path = "entry\"
			if (was_something) {

				existing.push_back(entry);
				entry.clear();
				was_something = false;
			}
			// path = "\\\\\"
		}
		else {
			was_something = true;
			entry.push_back(path[i]);
		}
	}
}

Path::Path() {};

Path::Path(std::string path)
{
	assert_cond(entries.size() == 0, "Path is already initialized");

	if (!path.size()) {
		return;
	}
	push_path_to_vector(this->entries, path);
}

bool Path::isAbsolute()
{
	if (!this->entries.size()) {
		return false;
	}
	return isPathAbsolute(entries[0]);
}

ErrStack Path::isDirectory(bool &is_dir)
{
	if (!this->entries.size()) {
		is_dir = false;
		return ErrStack();
	}

	std::vector<char> path = this->toWin32Path();
	LPCSTR path_lpcstr = path.data();

	DWORD file_atrib = GetFileAttributesA(path_lpcstr);
	if (file_atrib & INVALID_FILE_ATTRIBUTES) {
		return ErrStack(code_location, getLastError());
	}
	
	if (file_atrib & FILE_ATTRIBUTE_DIRECTORY) {
		is_dir = true;
	}
	else {
		is_dir = false;
	}
	return ErrStack();
}

bool Path::isAccesible()
{
	if (!this->isAbsolute() || !this->entries.size()) {
		return false;
	}

	std::vector<char> path = this->toWin32Path();
	LPCSTR path_lpcstr = path.data();

	if (GetFileAttributesA(path_lpcstr) == INVALID_FILE_ATTRIBUTES) {
		return false;
	}
	return true;
}

bool Path::hasExtension(std::string ext)
{
	std::string& last_entry = this->entries.back();
	uint64_t offset = last_entry.find_last_of('.');

	return last_entry.substr(offset + 1) == ext ? true : false;
}

void Path::push_back(std::string path)
{
	push_path_to_vector(this->entries, path);
}

void Path::push_back(Path path)
{
	for (std::string new_entry : path.entries) {
		this->entries.push_back(new_entry);
	}
}

void Path::pop_back(size_t count)
{
	for (size_t i = 0; i < count; i++) {
		entries.pop_back();
	}
}

void Path::push_front(Path path)
{
	for (std::string entry : this->entries) {
		path.entries.push_back(entry);
	}
	this->entries = path.entries;
}

void Path::push_front(std::string path)
{
	std::vector<std::string> append = this->entries;
	
	this->entries.clear();
	push_path_to_vector(this->entries, path);

	for (std::string entry : append) {
		this->entries.push_back(entry);
	}
}

void Path::pop_front(size_t count)
{
	this->entries.erase(entries.begin(), entries.begin() + count);
}

bool Path::erase(size_t start, size_t end)
{
	if (start < 0 || end > this->entries.size() || end < start) {
		return false;
	}
	this->entries.erase(entries.begin() + start, entries.begin() + end);
	return true;
}

void Path::removeExtension()
{
	std::string& last = entries.back();
	size_t last_dot = last.find_last_of('.');

	if (last_dot != std::string::npos) {
		last = last.substr(0, last_dot);
	}
}

std::vector<char> Path::toWin32Path()
{
	std::vector<char> path;

	// prepend "\\?\" for long format absolute paths 
	if (this->isAbsolute()) {
		path.push_back('\\');
		path.push_back('\\');
		path.push_back('?');
		path.push_back('\\');
	}

	if (this->entries.size()) {

		size_t last = entries.size() - 1;
		for (size_t i = 0; i < entries.size(); i++) {

			for (char letter : entries[i]) {
				path.push_back(letter);
			}

			if (i != last) {
				path.push_back('\\');
			}
		}
	}
	
	path.push_back('\0');
	return path;
}

std::string Path::toWindowsPath()
{
	std::string path;

	if (this->entries.size()) {

		size_t last = entries.size() - 1;
		for (size_t i = 0; i < entries.size(); i++) {

			for (char letter : entries[i]) {
				path.push_back(letter);
			}

			if (i != last) {
				path.push_back('\\');
			}
		}
	}
	return path;
}

template<typename T>
ErrStack Path::read(std::vector<T>& content)
{
	// create file handle
	std::vector<char> filename_vec = toWin32Path();
	LPCSTR filename_win = filename_vec.data();

	HANDLE file_handle = CreateFileA(filename_win,
		GENERIC_READ, // desired acces
		0,  // share mode
		NULL,  // security atributes
		OPEN_EXISTING,  // disposition
		FILE_ATTRIBUTE_NORMAL, // flags and atributes
		NULL);

	if (file_handle == INVALID_HANDLE_VALUE) {
		return ErrStack(ExtraError::FAILED_TO_READ_FILE, code_location,
			"failed to create file handle for path = " + this->toWindowsPath(), getLastError());
	}

	// find file size
	LARGE_INTEGER file_size;
	if (!GetFileSizeEx(file_handle, &file_size)) {

		CloseHandle(file_handle);
		return ErrStack(ExtraError::FAILED_TO_READ_FILE, code_location,
			"failed to find file size for path = " + this->toWindowsPath(), getLastError());
	}
	content.resize(file_size.QuadPart);

	// read file
	DWORD bytes_read;

	if (!ReadFile(file_handle, content.data(), (DWORD)file_size.QuadPart, &bytes_read, NULL)) {

		CloseHandle(file_handle);
		return ErrStack(ExtraError::FAILED_TO_READ_FILE, code_location,
			"failed to read path = " + this->toWindowsPath(), getLastError());
	}

	CloseHandle(file_handle);
	return ErrStack();
}
template ErrStack Path::read(std::vector<char>& content);
template ErrStack Path::read(std::vector<uint8_t>& content);

void Path::check()
{
	// 
}

ErrStack Path::getExePath(Path& exe_path)
{
	assert_cond(exe_path.entries.size() == 0, "");

	std::vector<char> exe_filename(max_path);

	if (!GetModuleFileNameA(NULL, exe_filename.data(), (DWORD)max_path)) {

		return ErrStack(code_location, getLastError());
	}
	exe_path.push_back(std::string(exe_filename.begin(), exe_filename.end()));

	return ErrStack();
}

ErrStack Path::getLocalFolder(Path& local_path)
{
	checkErrStack(getExePath(local_path), "");
	local_path.removeExtension();
	std::string solution_name = local_path.entries.back();

	local_path.pop_back(3);
	local_path.push_back(solution_name);

	return ErrStack();
}
