#pragma once

// mine
#include "Renderer.h"


class AppLevel {
public:
	HWND hwnd = NULL;

	bool run_app_loop = true;
	float delta_time = 0.0f;

public:
	ErrorStack response();
};

extern AppLevel app_level;


// The usual entry point of a visual application under Windows
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow);
