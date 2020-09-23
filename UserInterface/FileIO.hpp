#pragma once

#include "ErrorStack.hpp"


namespace nui {

	extern uint32_t max_path;


	class FilePath {
	public:
		std::vector<std::string> entries;

	public:
		void recreateAbsolute(std::string path);
		ErrStack recreateRelative(std::string path);
		ErrStack recreateRelativeToSolution(std::string path);

		bool hasExtension(std::string extension);

		void push_back(std::string path);
		void pop_back(size_t count = 1);

		void push_front(std::string path);
		void pop_front(size_t count = 1);

		void erase(size_t start, size_t end);

		std::string toWindowsPath();

		template<typename T = char>
		ErrStack read(std::vector<T>& content);
	};
}
