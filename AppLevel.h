#pragma once

// mine
#include "Renderer.h"


class WindowHandle
{
public:
	HWND hwnd = NULL;

	~WindowHandle();
};


class AppLevel
{
public:
	Renderer renderer;

	// Windows
	WindowHandle win_handle;

	bool run_app_loop = true;
	float delta_time = 0.0f;
public:
	ErrorStack response();
};

extern AppLevel app_level;


// The usual entry point of a visual application under Windows
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow);

// TODO:
// get delta time 
// scale values using input
// orbit camera around center
// move camera closer farther
// have settings for controls
