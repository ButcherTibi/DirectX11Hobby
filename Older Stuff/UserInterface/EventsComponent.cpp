module;

#include "DietWindows.hpp"
#include "Windows.h"

// Standard
#include <chrono>

// Mine
#include "ErrorStack.hpp"
#include "Input.hpp"
#include "Properties.hpp"
#include "utf_string.hpp"

module UserInterface;

using namespace nui;


void EventsComponent::_init(Window* window)
{
	_window = window;
	_mouse_event_state = MouseEventState::OUTSIDE;

	_onMouseEnter = nullptr;
	_onMouseHover = nullptr;
	_onMouseMove = nullptr;
	_onMouseScroll = nullptr;
	_onMouseLeave = nullptr;
}

void EventsComponent::setMouseEnterEvent(EventCallback callback, void* user_data)
{
	this->_onMouseEnter = callback;
	this->_mouse_enter_user_data = user_data;
}

void EventsComponent::setMouseHoverEvent(EventCallback callback, void* user_data)
{
	this->_onMouseHover = callback;
	this->_mouse_hover_user_data = user_data;
}

void EventsComponent::setMouseMoveEvent(EventCallback callback, void* user_data)
{
	this->_onMouseMove = callback;
	this->_mouse_move_user_data = user_data;
}

void EventsComponent::setMouseScrollEvent(EventCallback callback, void* user_data)
{
	this->_onMouseScroll = callback;
	this->_mouse_scroll_user_data = user_data;
}

void EventsComponent::setMouseLeaveEvent(EventCallback callback, void* user_data)
{
	this->_onMouseLeave = callback;
	this->_mouse_leave_user_data = user_data;
}

void deleteKeyCallback(uint32_t key, std::vector<Shortcut1KeyCallback>& keys_1)
{
	uint32_t i = 0;
	for (Shortcut1KeyCallback& shortcut : keys_1) {

		if (shortcut.key == key) {

			keys_1.erase(keys_1.begin() + i);
			return;
		}
		i++;
	}
}

void delete2KeysCallbacks(uint32_t key_0, uint32_t key_1, std::vector<Shortcut2KeysCallback>& keys_2)
{
	uint32_t i = 0;
	for (Shortcut2KeysCallback& shortcut : keys_2) {

		if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1) {

			keys_2.erase(keys_2.begin() + i);
			return;
		}
		i++;
	}
}

void delete3KeysCallbacks(uint32_t key_0, uint32_t key_1, uint32_t key_2, std::vector<Shortcut3KeysCallback>& keys_3)
{
	uint32_t i = 0;
	for (Shortcut3KeysCallback& shortcut : keys_3) {

		if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1 && shortcut.key_2 == key_2) {

			keys_3.erase(keys_3.begin() + i);
			return;
		}
		i++;
	}
}

void EventsComponent::setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	if (callback == nullptr) {
		deleteKeyCallback(key, _key_downs);
	}
	else {
		for (Shortcut1KeyCallback& shortcut : _key_downs) {

			if (shortcut.key == key) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut1KeyCallback& new_shortcut = _key_downs.emplace_back();
		new_shortcut.key = key;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeysDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, void* user_data)
{
	if (callback == nullptr) {
		delete2KeysCallbacks(key_0, key_1, _keys_2_downs);
	}
	else {
		for (Shortcut2KeysCallback& shortcut : _keys_2_downs) {

			if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut2KeysCallback& new_shortcut = _keys_2_downs.emplace_back();
		new_shortcut.key_0 = key_0;
		new_shortcut.key_1 = key_1;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeysDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, uint32_t key_2, void* user_data)
{
	if (callback == nullptr) {
		delete3KeysCallbacks(key_0, key_1, key_2, _keys_3_downs);
	}
	else {
		for (Shortcut3KeysCallback& shortcut : _keys_3_downs) {

			if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1 && shortcut.key_2 == key_2) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut3KeysCallback& new_shortcut = _keys_3_downs.emplace_back();
		new_shortcut.key_0 = key_0;
		new_shortcut.key_1 = key_1;
		new_shortcut.key_2 = key_2;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeyHeldDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	if (callback == nullptr) {
		deleteKeyCallback(key, _key_held_downs);
	}
	else {
		for (Shortcut1KeyCallback& shortcut : _key_held_downs) {

			if (shortcut.key == key) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut1KeyCallback& new_shortcut = _key_held_downs.emplace_back();
		new_shortcut.key = key;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeysHeldDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, void* user_data)
{
	if (callback == nullptr) {
		delete2KeysCallbacks(key_0, key_1, _keys_2_held_downs);
	}
	else {
		for (Shortcut2KeysCallback& shortcut : _keys_2_held_downs) {

			if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut2KeysCallback& new_shortcut = _keys_2_held_downs.emplace_back();
		new_shortcut.key_0 = key_0;
		new_shortcut.key_1 = key_1;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeysHeldDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, uint32_t key_2, void* user_data)
{
	if (callback == nullptr) {
		delete3KeysCallbacks(key_0, key_1, key_2, _keys_3_held_downs);
	}
	else {
		for (Shortcut3KeysCallback& shortcut : _keys_3_held_downs) {

			if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1 && shortcut.key_2 == key_2) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut3KeysCallback& new_shortcut = _keys_3_held_downs.emplace_back();
		new_shortcut.key_0 = key_0;
		new_shortcut.key_1 = key_1;
		new_shortcut.key_2 = key_2;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeyUpEvent(EventCallback callback, uint32_t key, void* user_data)
{
	if (callback == nullptr) {
		deleteKeyCallback(key, _key_ups);
	}
	else {
		for (Shortcut1KeyCallback& shortcut : _key_ups) {

			if (shortcut.key == key) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut1KeyCallback& new_shortcut = _key_ups.emplace_back();
		new_shortcut.key = key;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::beginMouseLoopDeltaEffect(Box2D& trap)
{
	Window::MouseDeltaEffect& delta = _window->mouse_delta_effect;

	if (delta.trap.size[0] != 0) {

		switch (delta.type) {
		case Window::MouseDeltaEffect::Type::HIDDEN: {
			_window->setMouseVisibility(true);
		}
		}

		ClipCursor(nullptr);
	}

	delta.type = Window::MouseDeltaEffect::Type::LOOP;
	delta.trap = trap;
}

void EventsComponent::beginMouseFixedDeltaEffect(Box2D& trap)
{
	Window::MouseDeltaEffect& delta = _window->mouse_delta_effect;

	if (delta.trap.size[0] != 0) {

		switch (delta.type) {
		case Window::MouseDeltaEffect::Type::HIDDEN: {
			_window->setMouseVisibility(true);
		}
		}

		ClipCursor(nullptr);
	}

	delta.type = Window::MouseDeltaEffect::Type::HIDDEN;
	delta.trap = trap;

	nui::Input& input = _window->input;
	delta.begin_mouse_x = input.mouse_x;
	delta.begin_mouse_y = input.mouse_y;
}

float EventsComponent::getInsideDuration()
{
	if (nui::frame_start_time < _mouse_enter_time) {
		return 0;
	}

	auto now = nui::frame_start_time;

	return (float)std::chrono::duration_cast<std::chrono::milliseconds>(
		now - _mouse_enter_time).count();
}

void EventsComponent::_emitInsideEvents(StoredElement2* self)
{
	SteadyTime& now = nui::frame_start_time;

	// if mouse was previously outside
	if (_mouse_event_state == MouseEventState::OUTSIDE) {

		_mouse_enter_time = now;

		if (_onMouseEnter != nullptr) {
			this->_onMouseEnter(_window, self, _mouse_enter_user_data);
		}
	}

	if (_onMouseHover != nullptr) {
		this->_onMouseHover(_window, self, _mouse_hover_user_data);
	}

	if (_onMouseMove != nullptr) {
		if (_window->input.mouse_delta_x || _window->input.mouse_delta_y) {
			this->_onMouseMove(_window, self, _mouse_move_user_data);
		}
	}

	if (_onMouseScroll != nullptr) {
		if (_window->input.mouse_wheel_delta) {
			this->_onMouseScroll(_window, self, _mouse_scroll_user_data);
		}
	}

	// Keys Down
	for (Shortcut1KeyCallback& shortcut : _key_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key];

		if (key_0.is_down && key_0.down_transition) {
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	for (Shortcut2KeysCallback& shortcut : _keys_2_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key_0];
		KeyState& key_1 = _window->input.key_list[shortcut.key_1];

		if (key_0.is_down && key_1.is_down &&
			key_0.getDuration_ms() > key_1.getDuration_ms() &&
			key_1.down_transition)
		{
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	for (Shortcut3KeysCallback& shortcut : _keys_3_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key_0];
		KeyState& key_1 = _window->input.key_list[shortcut.key_1];
		KeyState& key_2 = _window->input.key_list[shortcut.key_2];

		/*printf("key_0 = %d %d \n", key_0.getDuration_ms(), key_0.is_down);
		printf("key_1 = %d %d \n", key_1.getDuration_ms(), key_1.is_down);
		printf("key_2 = %d %d \n", key_2.getDuration_ms(), key_2.is_down);*/

		if (key_0.is_down && key_1.is_down && key_2.is_down &&
			key_0.getDuration_ms() > key_1.getDuration_ms() && key_1.getDuration_ms() > key_2.getDuration_ms() &&
			key_2.down_transition)
		{
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	// Keys Held Down
	for (Shortcut1KeyCallback& shortcut : _key_held_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key];

		if (key_0.is_down) {
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	for (Shortcut2KeysCallback& shortcut : _keys_2_held_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key_0];
		KeyState& key_1 = _window->input.key_list[shortcut.key_1];

		if (key_0.is_down && key_1.is_down &&
			key_0.getDuration_ms() > key_1.getDuration_ms())
		{
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	for (Shortcut3KeysCallback& shortcut : _keys_3_held_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key_0];
		KeyState& key_1 = _window->input.key_list[shortcut.key_1];
		KeyState& key_2 = _window->input.key_list[shortcut.key_2];

		if (key_0.is_down && key_1.is_down && key_2.is_down &&
			key_0.getDuration_ms() > key_1.getDuration_ms() && key_1.getDuration_ms() > key_2.getDuration_ms())
		{
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	// Keys UP
	for (Shortcut1KeyCallback& shortcut : _key_ups) {

		KeyState& key = _window->input.key_list[shortcut.key];

		if (!key.is_down && key.up_transition) {
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	_mouse_event_state = MouseEventState::INSIDE;
}

void EventsComponent::_emitOutsideEvents(StoredElement2* self)
{
	// was inside but now is outside
	if (_mouse_event_state == MouseEventState::INSIDE) {

		if (_onMouseLeave != nullptr) {
			this->_onMouseLeave(_window, self, _mouse_leave_user_data);
		}

		_mouse_event_state = MouseEventState::OUTSIDE;
	}

	// do nothing if already outside
}

void TextInputComponent::init(Window* _window)
{
	this->_window = _window;
	
	cursor_pos = display_text.after();
	selection_start = display_text.after();
	selection_length = 0;
}

void TextInputComponent::_reset()
{
	cursor_pos = display_text.last();

	selection_start = display_text.begin();
	selection_length = 0;
}

void TextInputComponent::set(int32_t value)
{
	auto& number_str = _window->instance->_cache_string;
	number_str = std::to_string(value);

	display_text = number_str;

	_reset();
}

void TextInputComponent::set(float value, uint32_t decimal_count)
{
	auto& number_str = _window->instance->_cache_string;
	number_str = std::to_string(value);
	size_t dot_location = number_str.find(".");

	if (dot_location != std::string::npos &&
		number_str.length() - 1 - dot_location > decimal_count)
	{
		number_str.resize(dot_location + decimal_count + 1);
	}

	display_text = number_str;

	_reset();
}

void TextInputComponent::set(utf8string& new_text)
{
	display_text = new_text;

	_reset();
}

void TextInputComponent::deselect()
{
	selection_start = display_text.begin();
	selection_length = 0;
}

void TextInputComponent::setCursorAtEnd()
{
	cursor_pos = display_text.last();
}

void TextInputComponent::respondToInput()
{
	Input& input = _window->input;

	if (display_text.isEmpty()) {

		// Add Only
		if (input.unicode_list.size() > 0) {

			utf8string new_content;

			for (CharacterKeyState& key : input.unicode_list) {
				if (key.down_transition) {
					new_content.push(key.code_point);
				}
			}

			display_text.insertAfter(cursor_pos, new_content);
			cursor_pos.next(new_content.length());

			deselect();
		}
	}
	else {

		// Selection And Cursor
		{
			// Start
			if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].down_transition) {

				mouse_x_start = input.mouse_x;
				mouse_x_end = mouse_x_start;
			}

			// End
			if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].is_down) {

				mouse_x_end = input.mouse_x;

				// Selection
				{
					int32_t start_x = mouse_x_start;
					int32_t end_x = mouse_x_end;

					if (mouse_x_start > mouse_x_end) {
						start_x = mouse_x_end;
						end_x = mouse_x_start;
					}

					utf8string_iter iter = display_text.begin();
					selection_start = display_text.after();
					selection_length = 0;

					for (uint32_t i = 0; i < text_instance.chars.size(); i++) {

						PositionedCharacter& character = text_instance.chars[i];
						int32_t character_middle = character.pos[0] + (character.chara->advance_X / 2);

						if (start_x < character_middle && character_middle < end_x) {

							if (selection_start.isAtNull()) {
								selection_start = iter;
								selection_length = 1;
							}
							else {
								selection_length += 1;
							}
						}

						iter.next();
					}
				}

				// Cursor
				{
					PositionedCharacter& first_char = text_instance.chars[0];
					PositionedCharacter& last_char = text_instance.chars.back();

					if (input.mouse_x < first_char.pos[0]) {
						cursor_pos = display_text.before();
					}
					else if (input.mouse_x > last_char.pos[0] + last_char.chara->advance_X) {
						cursor_pos = display_text.last();
					}
					else {
						utf8string_iter iter = display_text.begin();

						for (uint32_t i = 0; i < text_instance.chars.size(); i++) {

							PositionedCharacter& character = text_instance.chars[i];
							int32_t char_start = character.pos[0];
							int32_t char_mid = character.pos[0] + (character.chara->advance_X / 2);
							int32_t char_end = character.pos[0] + character.chara->advance_X;

							// if the mouse is to the left of the character
							if (char_start < input.mouse_x && input.mouse_x < char_mid) {

								iter.prev();
								cursor_pos = iter;
								break;
							}
							else if (char_mid < input.mouse_x && input.mouse_x < char_end) {
								cursor_pos = iter;
								break;
							}

							iter.next();
						}
					}
				}
			}
		}

		// Move Cursor
		{

		}

		// Add/Replace
		if (input.unicode_list.size() > 0) {

			utf8string new_content;

			for (CharacterKeyState& key : input.unicode_list) {
				if (key.down_transition) {
					new_content.push(key.code_point);
				}
			}

			// Replace Selection
			if (selection_length > 0) {

				display_text.replaceSelection(selection_start, selection_length, new_content);
				cursor_pos = selection_start;
				cursor_pos.next(new_content.length() - 1);
				deselect();
			}
			// Add at cursor
			else {
				display_text.insertAfter(cursor_pos, new_content);
				cursor_pos.next(new_content.length());
			}
		}
		// Delete
		else {

			// Delete Selection
			if (selection_length > 0) {

				if (input.isDownTransition(VirtualKeys::BACKSPACE) ||
					input.isDownTransition(VirtualKeys::DELETE))
				{
					display_text.erase(selection_start, selection_length);

					cursor_pos = selection_start;
					cursor_pos.prev();
					deselect();
				}
			}
			// Delete at cursor
			else {
				if (input.isDownTransition(VirtualKeys::BACKSPACE)) {

					if (cursor_pos > display_text.before()) {
						display_text.erase(cursor_pos, 1);
						cursor_pos.prev();
					}
				}
				else if (input.isDownTransition(VirtualKeys::DELETE)) {

					if (cursor_pos < display_text.last()) {

						utf8string_iter target = cursor_pos;
						target.next();
						display_text.erase(target, 1);
					}
				}
			}
		}
	}
}

void TextInputComponent::generateGPU_Data(uint32_t& r_width, uint32_t& r_height)
{
	Instance* inst = _window->instance;

	// Text
	PositionedCharacters chars_pos;
	chars_pos.chars = &text_instance.chars;
	{
		GlyphProperties props;
		props.font_family = &info.font_family;
		props.font_style = &info.font_style;
		props.font_size = info.font_size;
		props.line_height = info.line_height;

		inst->findAndPositionGlyphs(display_text, props, chars_pos);

		text_instance.color = info.text_color;
	}
	
	// Selection
	if (selection_length > 0) {
		
		int32_t character_index = selection_start.characterIndex();
		PositionedCharacter& start = text_instance.chars[character_index];
		PositionedCharacter& end = text_instance.chars[character_index + selection_length];
	}

	// Cursor
	{
		if (display_text.isEmpty()) {
			cursor_instance.pos[0] = 0;
		}
		else {
			int32_t character_index = cursor_pos.characterIndex();

			if (character_index == -1) {
				PositionedCharacter& character = text_instance.chars[0];
				cursor_instance.pos[0] = character.pos[0];
			}
			else {
				PositionedCharacter& character = text_instance.chars[character_index];
				cursor_instance.pos[0] = character.pos[0] + character.chara->advance_X;
			}
		}

		cursor_instance.pos[1] = 0;

		cursor_instance.size[0] = info.cursor_thickness;
		cursor_instance.size[1] = chars_pos.font_size->line_spacing;

		cursor_instance.color = info.cursor_color;
	}

	r_width = chars_pos.width;
	r_height = chars_pos.height;
}

void TextInputComponent::offsetPosition(std::array<int32_t, 2> offset)
{
	text_instance.offsetPosition(offset);
	cursor_instance.offsetPosition(offset);
}

void TextInputComponent::draw()
{
	Instance* inst = _window->instance;

	if (display_text.isEmpty()) {

		// Only draw the cursor
		inst->drawRect(_window, &cursor_instance);
	}
	else {
		// Selection
		{

		}

		// Text
		{
			std::vector<TextInstance*> instances = {
				&text_instance
			};

			inst->drawTexts(_window, instances);
		}

		// Cursor
		{
			inst->drawRect(_window, &cursor_instance);
		}
	}
}
