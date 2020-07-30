#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdint.h>


class AppLevel {
public:
	uint32_t display_width;
	uint32_t display_height;

	bool run_app_loop = true;
};

extern AppLevel app_level;


// The usual entry point of a visual application under Windows
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow);
