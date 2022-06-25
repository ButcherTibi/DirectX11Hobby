#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "../CommonTypes.hpp"
#include "../Input.hpp"


class Window {
public:
	WNDCLASSW window_class;  // Win32 Window class used on creation
	HWND hwnd;  // handle to the window
	
	HCURSOR cursor;

	// Window Size
	u32 width;
	u32 height;

	struct Messages {
		bool should_close;
		bool is_minimized;
	};
	Messages win_messages;

public:
	void init();

	// get rect of the actual rendered surface excludes border shadows and top bar
	RECT getClientRectangle();

	// Mouse
	bool setLocalMousePosition(u32 x, u32 y);
	bool trapMousePosition(u32 x, u32 y);
	bool untrapMousePosition();
	void setMouseVisibility(bool is_visible);
};
