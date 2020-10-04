#include "pch.h"

// Standard
#include <algorithm>

// Header
#include "NuiLibrary.hpp"


using namespace nui;


void CommonEventsComponent::create(Window* wnd, Node* source_node)
{
	window = wnd;
	source = source_node;

	last_mouse_x = 0xFFFF'FFFF;
	last_mouse_y = 0xFFFF'FFFF;
	mouse_enter_time = std::chrono::steady_clock::now();
	mouse_entered = false;
	mouse_left = true;

	hover_event.source = source_node;
	enter_event.source = source_node;
	leave_event.source = source_node;

	onMouseHover = nullptr;
	onMouseEnter = nullptr;
	onMouseLeave = nullptr;
}

void CommonEventsComponent::emitInsideEvents()
{
	auto now = std::chrono::steady_clock::now();

	// Mouse Entered for the first time
	if (!mouse_entered) {

		mouse_enter_time = std::chrono::steady_clock::now();
		hover_event.duration = 0;

		if (onMouseEnter != nullptr) {
			this->onMouseEnter(enter_event);
		}
	}
	else {
		hover_event.duration = fsec_cast(now - mouse_enter_time);
	}

	// Hover
	if (onMouseHover != nullptr) {
		this->onMouseHover(hover_event);
	}

	// Move
	if (window->mouse_x != last_mouse_x ||
		window->mouse_y != last_mouse_y)
	{
		last_mouse_x = window->mouse_x;
		last_mouse_y = window->mouse_y;

		if (this->onMouseMove != nullptr) {
			this->onMouseMove(move_event);
		}
	}

	// key Down
	for (KeyDown& down : keys_down) {

		KeyState& state = window->input.key_list[down.key];
		if (state.is_down && state.first_frame) {
			down.callback(down.event);
		}
	}

	mouse_entered = true;
	mouse_left = false;
}

void CommonEventsComponent::emitOutsideEvents()
{
	// Mouse Leave
	if (!mouse_left) {
		if (onMouseLeave != nullptr) {
			this->onMouseLeave(leave_event);
		}
	}

	last_mouse_x = 0xFFFF'FFFF;
	last_mouse_y = 0xFFFF'FFFF;
	mouse_entered = false;
	mouse_left = true;
}

void CommonEventsComponent::setMouseHoverEvent(MouseHoverCallback callback, void* user_ptr)
{
	this->onMouseHover = callback;
	this->hover_event.user_ptr = user_ptr;
}

void CommonEventsComponent::setMouseEnterEvent(MouseEnterCallback callback, void* user_ptr)
{
	this->onMouseEnter = callback;
	this->enter_event.user_ptr = user_ptr;
}

void CommonEventsComponent::setMouseLeaveEvent(MouseLeaveCallback callback, void* user_ptr)
{
	this->onMouseLeave = callback;
	this->leave_event.user_ptr = user_ptr;
}

void CommonEventsComponent::setMouseMoveEvent(MouseMoveCallback callback, void* user_ptr)
{
	this->onMouseMove = callback;
	this->move_event.user_ptr = user_ptr;
}

bool CommonEventsComponent::addKeyDownEvent(KeyDownCallback callback, uint32_t key, void* user_ptr)
{
	// if debug then check if key is valid

	KeyDown* new_down = nullptr;

	for (KeyDown& down: keys_down) {
		if (down.key == key) {
			new_down = &down;
			break;
		}
	}

	if (new_down == nullptr) {
		new_down = &keys_down.emplace_back();
	}

	new_down->key = key;
	new_down->callback = callback;
	new_down->event.source = this->source;
	new_down->event.user_ptr = user_ptr;

	return true;
}

bool CommonEventsComponent::removeKeyDownEvent(uint32_t key)
{
	for (auto it = keys_down.begin(); it != keys_down.end(); ++it) {

		if (it->key == key) {
			keys_down.erase(it);
			return true;
		}
	}

	return false;
}

void Window::_emitEvents()
{
	Node* first_hit_node = &nodes.front();	

	for (Node& node : nodes) {

		if (node.collider.isInside(mouse_x, mouse_y)) {

			if (node.layer_idx > first_hit_node->layer_idx) {
				first_hit_node = &node;
			}
		}
	}

	first_hit_node->event_comp.emitInsideEvents();

	for (Node& node : nodes) {

		if (&node != first_hit_node) {
			node.event_comp.emitOutsideEvents();
		}
	}
}