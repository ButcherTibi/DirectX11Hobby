#include "pch.h"

// Header
#include "NuiLibrary.hpp"


using namespace nui;


Wrap* Window::addWrap()
{
	Root* root_wrap = std::get_if<Root>(&nodes.front().elem);

	return root_wrap->addWrap();
}

Text* Window::addText()
{
	Root* root_wrap = std::get_if<Root>(&nodes.front().elem);

	return root_wrap->addText();
}

Surface* Window::addSurface()
{
	Root* root_wrap = std::get_if<Root>(&nodes.front().elem);

	return root_wrap->addSurface();
}

RECT Window::getClientRectangle()
{
	RECT win_rect;
	GetWindowRect(hwnd, &win_rect);

	RECT client_rect;
	GetClientRect(hwnd, &client_rect);

	uint32_t border_thick = ((win_rect.right - win_rect.left) - client_rect.right) / 2;
	uint32_t header_height;
	{
		uint32_t win_height = win_rect.bottom - win_rect.top;
		uint32_t client_height = client_rect.bottom - client_rect.top;
		header_height = win_height - (client_height + border_thick * 2);
	}

	win_rect.left += border_thick;
	win_rect.right -= border_thick;
	win_rect.top += border_thick + header_height;
	win_rect.bottom -= border_thick;

	return win_rect;
}

bool Window::setLocalMousePosition(uint32_t x, uint32_t y)
{
	RECT client_rect = getClientRectangle();
	return SetCursorPos(client_rect.left + x, client_rect.top + y);
}

bool Window::trapLocalMousePosition(BoundingBox2D<uint32_t>& box)
{
	RECT client_rect = getClientRectangle();

	RECT rect;
	rect.left = client_rect.left + box.x0;
	rect.right = client_rect.left + box.x1;
	rect.top = client_rect.top + box.y0;
	rect.bottom = client_rect.top + box.y1;
	return ClipCursor(&rect);
}

bool Window::untrapMousePosition()
{
	return ClipCursor(nullptr);
}

void Window::hideMousePointer(bool hide)
{
	if (hide) {
		int32_t internal_display_counter = ShowCursor(false);
		while (internal_display_counter  >= 0) {
			internal_display_counter = ShowCursor(false);
		}
	}
	else {
		int32_t internal_display_counter = ShowCursor(true);
		while (internal_display_counter < 0) {
			internal_display_counter = ShowCursor(true);
		}
	}
}

void Window::getMouseDelta(int32_t& mouse_delta_x, int32_t& mouse_delta_y)
{
	mouse_delta_x = this->input.mouse_delta_x;
	mouse_delta_y = this->input.mouse_delta_y;
}
