#include "pch.h"

// Header
#include "EventComp.hpp"


#include "NuiLibrary.hpp"


using namespace nui;


void EventComp::_create(Window* wnd, Node* source_node)
{
	window = wnd;
	source = source_node;

	last_mouse_x = 0xFFFF;
	last_mouse_y = 0xFFFF;
	mouse_state = MouseState::OFF;
	mouse_delta_state = MouseDeltaState::OFF;

	hover_event.source = source_node;
	enter_event.source = source_node;
	leave_event.source = source_node;
	move_event.source = source_node;

	onMouseHover = nullptr;
	onMouseEnter = nullptr;
	onMouseLeave = nullptr;
	onMouseMove = nullptr;
}

void EventComp::_endMouseDelta()
{
	window->untrapMousePosition();

	mouse_delta_state = MouseDeltaState::OFF;
}

void EventComp::_emitInsideEvents()
{
	auto now = std::chrono::steady_clock::now();

	// Mouse Entered for the first time
	if (mouse_state == MouseState::OFF) {

		mouse_enter_time = now;
		hover_event.duration = 0;

		// Mouse Enter
		if (onMouseEnter != nullptr) {

			enter_event.mouse_x = window->input.mouse_x;
			enter_event.mouse_y = window->input.mouse_y;
			this->onMouseEnter(enter_event);
		}
	}
	else {
		hover_event.duration = fsec_cast(now - mouse_enter_time);
	}

	// Mouse Hover
	if (onMouseHover != nullptr) {

		hover_event.mouse_x = window->input.mouse_x;
		hover_event.mouse_y = window->input.mouse_y;
		this->onMouseHover(hover_event);
	}

	// Mouse Move
	if (window->input.mouse_x != last_mouse_x ||
		window->input.mouse_y != last_mouse_y)
	{
		last_mouse_x = window->input.mouse_x;
		last_mouse_y = window->input.mouse_y;

		if (this->onMouseMove != nullptr) {

			move_event.mouse_x = window->input.mouse_x;
			move_event.mouse_y = window->input.mouse_y;
			this->onMouseMove(move_event);
		}
	}

	switch (mouse_delta_state) {
	// Mouse Delta Start
	case nui::MouseDeltaState::START: {

		window->trapLocalMousePosition(source->collider);

		mouse_delta_state = MouseDeltaState::NOW;
		break;
	}

	// Mouse Delta
	case nui::MouseDeltaState::NOW: {

		Input& input = window->input;
		BoundingBox2D<uint32_t>& trap = source->collider;

		if (input.mouse_y == trap.y0) {
			window->setLocalMousePosition(input.mouse_x, trap.y1 - 2);
		}
		else if (input.mouse_y == trap.y1 - 1) {
			window->setLocalMousePosition(input.mouse_x, trap.y0 + 1);
		}
		else if (input.mouse_x == trap.x0) {
			window->setLocalMousePosition(trap.x1 - 2, input.mouse_y);
		}
		else if(input.mouse_x == trap.x1 - 1) {
			window->setLocalMousePosition(trap.x0 + 1, input.mouse_y);
		}
		break;
	}

	// Mouse Delta End
	case nui::MouseDeltaState::END: {

		_endMouseDelta();
		break;
	}
	}

	// Key Down
	for (KeyDown& down : keys_down) {

		KeyState& state = window->input.key_list[down.key];
		if (state.is_down && state.first_frame) {
			down.callback(down.event);
		}
	}

	// Key held down
	for (KeyHeldDown& held : keys_held_down) {

		KeyState& state = window->input.key_list[held.key];
		KeyHeldDownEvent& event = held.event;
		if (state.is_down) {

			// First time down
			if (event.duration == std::numeric_limits<float>::max()) {
				event.duration = 0;
				held.start_time = now;
			}
			else {
				event.duration = fsec_cast(now - held.start_time);
			}

			held.callback(event);
		}
		else {
			event.duration = std::numeric_limits<float>::max();
		}
	}

	// Key Up
	for (KeyUp& up : keys_up) {

		KeyState& state = window->input.key_list[up.key];
		if (!state.is_down && state.last_frame) {
			up.callback(up.event);
		}
	}

	mouse_state = MouseState::ENTER;
}

void EventComp::_emitOutsideEvents()
{
	// Mouse Leave
	if (mouse_state != MouseState::OFF) {

		if (onMouseLeave != nullptr) {

			leave_event.mouse_x = window->input.mouse_x;
			leave_event.mouse_y = window->input.mouse_y;
			this->onMouseLeave(leave_event);
		}

		last_mouse_x = 0xFFFF;
		last_mouse_y = 0xFFFF;
		mouse_state = MouseState::OFF;
	}

	// Mouse Delta Ended
	if (mouse_delta_state != MouseDeltaState::OFF) {
		_endMouseDelta();
	}

	for (KeyHeldDown& held : keys_held_down) {
		held.event.duration = std::numeric_limits<float>::max();
	}
}

void EventComp::setMouseHoverEvent(MouseHoverCallback callback, void* user_ptr)
{
	this->onMouseHover = callback;
	this->hover_event.user_ptr = user_ptr;
}

void EventComp::setMouseEnterEvent(MouseEnterCallback callback, void* user_ptr)
{
	this->onMouseEnter = callback;
	this->enter_event.user_ptr = user_ptr;
}

void EventComp::setMouseLeaveEvent(MouseLeaveCallback callback, void* user_ptr)
{
	this->onMouseLeave = callback;
	this->leave_event.user_ptr = user_ptr;
}

void EventComp::setMouseMoveEvent(MouseMoveCallback callback, void* user_ptr)
{
	this->onMouseMove = callback;
	this->move_event.user_ptr = user_ptr;
}

void EventComp::beginMouseDelta()
{
	this->window->mouse_delta_owner = source;

	// don't restart calling Delta Started
	if (mouse_delta_state == MouseDeltaState::OFF) {
		mouse_delta_state = MouseDeltaState::START;
	}
}

void EventComp::endMouseDelta()
{
	this->window->mouse_delta_owner = nullptr;

	// don't restart calling Delta Ended
	if (mouse_delta_state == MouseDeltaState::NOW) {
		mouse_delta_state = MouseDeltaState::END;
	}
}

void EventComp::addKeyDownEvent(KeyDownCallback callback, uint32_t key, void* user_ptr)
{
	// TODO: if debug then check if key is valid

	KeyDown* new_down = nullptr;

	for (KeyDown& down : keys_down) {
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
	new_down->event.key = key;
	new_down->event.user_ptr = user_ptr;
}

void EventComp::addKeyHeldDownEvent(KeyHeldDownCallback callback, uint32_t key, void* user_ptr)
{
	// TODO: if debug then check if key is valid

	KeyHeldDown* new_held_down = nullptr;
	for (KeyHeldDown& down : keys_held_down) {
		if (down.key == key) {
			new_held_down = &down;
			break;
		}
	}

	if (new_held_down == nullptr) {
		new_held_down = &keys_held_down.emplace_back();
	}

	new_held_down->key = key;
	new_held_down->callback = callback;
	new_held_down->event.source = this->source;
	new_held_down->event.key = key;
	new_held_down->event.duration = 0;
	new_held_down->event.user_ptr = user_ptr;
}

void EventComp::addKeyUpEvent(KeyUpCallback callback, uint32_t key, void* user_ptr)
{
	// TODO: if debug then check if key is valid

	KeyUp* new_up = nullptr;
	for (KeyUp& up : keys_up) {
		if (up.key == key) {
			new_up = &up;
			break;
		}
	}

	if (new_up == nullptr) {
		new_up = &keys_up.emplace_back();
	}

	new_up->key = key;
	new_up->callback = callback;
	new_up->event.source = this->source;
	new_up->event.key = key;
	new_up->event.user_ptr = user_ptr;
}

bool EventComp::removeKeyDownEvent(uint32_t key)
{
	for (auto it = keys_down.begin(); it != keys_down.end(); ++it) {

		if (it->key == key) {
			keys_down.erase(it);
			return true;
		}
	}

	return false;
}

bool EventComp::removeKeyHeldDownEvent(uint32_t key)
{
	for (auto it = keys_held_down.begin(); it != keys_held_down.end(); ++it) {

		if (it->key == key) {
			keys_held_down.erase(it);
			return true;
		}
	}

	return false;
}

bool EventComp::removeKeyUpEvent(uint32_t key)
{
	for (auto it = keys_up.begin(); it != keys_up.end(); ++it) {

		if (it->key == key) {
			keys_up.erase(it);
			return true;
		}
	}

	return false;
}
