
#include "pch.h"

// Header
#include "FilePath.hpp"


using namespace io;


uint32_t io::max_path = 1024;


static uint32_t findEntryCount(std::string path)
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

static void pushPathToEntries(std::vector<std::string> &existing, std::string path)
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

void FilePath::recreateAbsolute(std::string path)
{
	entries.clear();
	pushPathToEntries(this->entries, path);
}

ErrStack FilePath::recreateRelative(std::string path)
{
	std::string exe_filename;
	exe_filename.resize(max_path);

	if (!GetModuleFileNameA(NULL, exe_filename.data(), (DWORD)max_path)) {
		return ErrStack(code_location, getLastError());
	}

	// trim excess
	for (uint32_t i = 0; i < exe_filename.size(); i++) {
		if (exe_filename[i] == '\0') {
			exe_filename.erase(i + 1, exe_filename.size() - (i + 1));
			break;
		}
	}

	// remove last 3 entries
	uint32_t slash_pos = 0;
	uint32_t slash_count = 0;
	for (int32_t i = exe_filename.size() - 1; i >= 0; i--) {

		char& c = exe_filename[i];

		if (c == '\\' || c == '/') {
			slash_count++;

			if (slash_count == 3) {
				slash_pos = i;
				break;
			}
		}
	}
	exe_filename.erase(slash_pos + 1, exe_filename.size() - (slash_pos + 1));

	assert_cond(path[0] != '/' && path[0] != '\\', "");

	// convert linux '/' to windows '\'
	for (char& c : path) {
		c = c == '/' ? '\\' : c;
	}

	exe_filename.append(path);

	push_back(exe_filename);

	return ErrStack();
}

bool FilePath::hasExtension(std::string extension)
{
	std::string last = entries.back();
	size_t dot_pos = last.find_last_of('.');
	std::string entry_ext = last.substr(dot_pos + 1, extension.size());

	return extension == entry_ext;
}

void FilePath::push_back(std::string path)
{
	pushPathToEntries(this->entries, path);
}

void FilePath::pop_back(size_t count)
{
	for (size_t i = 0; i < count; i++) {
		entries.pop_back();
	}
}

void FilePath::push_front(std::string path)
{
	std::vector<std::string> append = this->entries;
	
	this->entries.clear();
	pushPathToEntries(this->entries, path);

	for (std::string entry : append) {
		this->entries.push_back(entry);
	}
}

void FilePath::pop_front(size_t count)
{
	this->entries.erase(entries.begin(), entries.begin() + count);
}

void FilePath::erase(size_t start, size_t end)
{
	this->entries.erase(entries.begin() + start, entries.begin() + end);
}

std::string FilePath::toWindowsPath()
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
ErrStack FilePath::read(std::vector<T>& content)
{
	// create file handle
	std::string filename_vec = toWindowsPath();
	LPCSTR filename_win = filename_vec.data();

	HANDLE file_handle = CreateFile(filename_win,
		GENERIC_READ, // desired acces
		0,  // share mode
		NULL,  // security atributes
		OPEN_EXISTING,  // disposition
		FILE_ATTRIBUTE_NORMAL, // flags and atributes
		NULL);

	if (file_handle == INVALID_HANDLE_VALUE) {
		return ErrStack(code_location,
			"failed to create file handle for path = " + this->toWindowsPath());
	}

	// find file size
	LARGE_INTEGER file_size;
	if (!GetFileSizeEx(file_handle, &file_size)) {

		CloseHandle(file_handle);
		return ErrStack(code_location,
			"failed to find file size for path = " + this->toWindowsPath());
	}
	content.resize(file_size.QuadPart);

	// read file
	DWORD bytes_read;

	if (!ReadFile(file_handle, content.data(), (DWORD)file_size.QuadPart, &bytes_read, NULL)) {

		CloseHandle(file_handle);
		return ErrStack(code_location,
			"failed to read path = " + this->toWindowsPath());
	}

	CloseHandle(file_handle);
	return ErrStack();
}
template ErrStack FilePath::read(std::vector<char>& content);
template ErrStack FilePath::read(std::vector<uint8_t>& content);


template<typename T>
ErrStack io::readFile(std::string& path, std::vector<T>& content)
{
	// create file handle;
	HANDLE file_handle = CreateFileA(path.data(),
		GENERIC_READ, // desired acces
		0,  // share mode
		NULL,  // security atributes
		OPEN_EXISTING,  // disposition
		FILE_ATTRIBUTE_NORMAL, // flags and atributes
		NULL);  // template file

	if (file_handle == INVALID_HANDLE_VALUE) {
		return ErrStack(code_location,
			"failed to create file handle for path = " + path);
	}

	// find file size
	LARGE_INTEGER file_size;
	if (!GetFileSizeEx(file_handle, &file_size)) {

		CloseHandle(file_handle);
		return ErrStack(code_location,
			"failed to find file size for path = " + path);
	}
	content.resize(file_size.QuadPart);

	// read file
	DWORD bytes_read;

	if (!ReadFile(file_handle, content.data(), (DWORD)file_size.QuadPart, &bytes_read, NULL)) {

		CloseHandle(file_handle);
		return ErrStack(code_location,
			"failed to read path = " + path);
	}

	CloseHandle(file_handle);
	return ErrStack();
}
template ErrStack io::readFile(std::string& path, std::vector<char>& content);
template ErrStack io::readFile(std::string& path, std::vector<uint8_t>& content);


template<typename T>
ErrStack io::readLocalFile(std::string path, std::vector<T>& content)
{
	std::string exe_filename;
	exe_filename.resize(max_path);

	if (!GetModuleFileNameA(NULL, exe_filename.data(), (DWORD)max_path)) {
		return ErrStack(code_location, getLastError());
	}

	// trim excess
	for (uint32_t i = 0; i < exe_filename.size(); i++) {
		if (exe_filename[i] == '\0') {
			exe_filename.erase(i + 1, exe_filename.size() - (i + 1));
			break;
		}
	}

	// remove last 3 entries
	uint32_t slash_pos = 0;
	uint32_t slash_count = 0;
	for (int32_t i = exe_filename.size() - 1; i >= 0; i--) {

		char& c = exe_filename[i];

		if (c == '\\' || c == '/') {
			slash_count++;

			if (slash_count == 3) {
				slash_pos = i;
				break;
			}
		}
	}
	exe_filename.erase(slash_pos + 1, exe_filename.size() - (slash_pos + 1));

	assert_cond(path[0] != '/' && path[0] != '\\', "");

	// convert linux '/' to windows '\'
	for (char& c : path) {
		c = c == '/' ? '\\' : c;
	}

	exe_filename.append(path);

	return readFile(exe_filename, content);
}
template ErrStack io::readLocalFile(std::string path, std::vector<char>& content);
template ErrStack io::readLocalFile(std::string path, std::vector<uint8_t>& content);

//ErrStack FileSysPath::readLocal(std::vector<char>& content)
//{
//	std::string exe_filename;
//	exe_filename.resize(max_path);
//
//	if (!GetModuleFileNameA(NULL, exe_filename.data(), (DWORD)max_path)) {
//
//		return ErrStack(code_location, getLastError());
//	}
//
//	FileSysPath path(exe_filename);
//
//	std::string project_name;
//	{
//		std::string& last = path.entries.back();
//		size_t dot_pos = last.find_last_of('.');
//		project_name = last.substr(0, dot_pos);	
//	}
//
//	path.pop_back(3);
//	path.push_back(project_name);
//
//	path.push_back(*this);
//
//	return path.read(content);
//}
