
// Standard
#include <iostream>
#include <execution>

#include "Input.h"


InputState input;


// Most Significant Bit (most left) of Windows's GLTF_SHORT
bool msbWinShort(SHORT val)
{
	size_t short_size = sizeof(SHORT);

	// 8 bit
	if (short_size == 1) {
		return val & 0x80;
	}
	// 16 bit
	else if (short_size == 2) {
		return val & 0x80'00;
	}
	// 32 bit not that short
	else if (short_size == 4) {
		return val & 0x80'00'00'00;
	}
	// 64 bit just in case
	else if (short_size == 8) {
		return val & 0x8000'0000'0000'0000;
	}

	std::cout << "The Windows SHORT isn't short at all" << std::endl;

	return false;
}

void InputKey::submit(bool state, steady_time time)
{
	if (is_down == state) {
		end_time = time;
	}
	else {
		start_time = time;
		end_time = time;
	}
	is_down = state;

	consumed = false;
}

float InputKey::getDuration()
{
	if (is_down) {
		return fsec_cast(this->end_time - this->start_time);
	}
	return 0.0f;
}

InputKey getTimeIntersection(const InputKey& t0, const InputKey& t1)
{
	InputKey ti;
	ti.start_time = t0.start_time <= t1.start_time ? t1.start_time : t0.start_time;
	ti.end_time = t0.end_time <= t1.end_time ? t0.end_time : t1.end_time;
	return ti;
}

InputState::InputState()
{
	// Letter Keys
	this->key_a.vk_key = WindowsVirtualKeys::VK_A;
	this->key_d.vk_key = WindowsVirtualKeys::VK_D;
	this->key_f.vk_key = WindowsVirtualKeys::VK_F;
	this->key_s.vk_key = WindowsVirtualKeys::VK_S;
	this->key_w.vk_key = WindowsVirtualKeys::VK_W;

	// Special Keys
	this->key_lcontrol.vk_key = VK_LCONTROL;

	// Mouse Keys
	this->key_mouse_left.vk_key = VK_LBUTTON;
	this->key_mouse_right.vk_key = VK_RBUTTON;
	this->key_mouse_middle.vk_key = VK_MBUTTON;
}

void InputState::listenForKey(InputKey* key)
{
	key->shortcuts++;

	// you cannot listen for a key multiple times
	bool found = false;
	for (InputKey* existing_key : key_logs) {
		if (existing_key == key) {
			found = true;
			break;
		}
	}

	if (!found) {
		key_logs.push_back(key);
	}
}

void InputState::unlistenForKey(InputKey* key)
{
	assert_cond(key->shortcuts > 0, "");
	key->shortcuts--;

	// keys only get unregisterd only if they are not used by other shortcuts
	if (!key->shortcuts) {

		for (uint32_t i = 0; i < key_logs.size(); i++) {

			if (key_logs[i] == key) {

				key_logs.erase(key_logs.begin() + i);
				return;
			}
		}
	}
}

void InputState::addShortcut(Shortcut* shortcut, InputKey* key)
{
	listenForKey(key);

	shortcut->keys[0] = key;
	shortcut->size = 1;

	shortcuts_1key.push_back(shortcut);
}

void InputState::addShortcut(Shortcut* shortcut, InputKey* key_0, InputKey* key_1)
{
	listenForKey(key_0);
	listenForKey(key_1);

	shortcut->keys[0] = key_0;
	shortcut->keys[1] = key_1;
	shortcut->size = 2;

	shortcuts_2keys.push_back(shortcut);
}

void InputState::addShortcut(Shortcut* shortcut, InputKey* key_0, InputKey* key_1, InputKey* key_2)
{
	listenForKey(key_0);
	listenForKey(key_1);
	listenForKey(key_2);

	shortcut->keys[0] = key_0;
	shortcut->keys[1] = key_1;
	shortcut->keys[2] = key_2;
	shortcut->size = 3;

	shortcuts_3keys.push_back(shortcut);
}

void InputState::removeShortcut(Shortcut* shortcut)
{
	shortcut->duration = -1.0f;

	switch (shortcut->size) {
	case 1:
		unlistenForKey(shortcut->keys[0]);

		for (uint32_t i = 0; i < shortcuts_1key.size(); i++) {

			Shortcut* existing = shortcuts_1key[i];

			if (existing == shortcut) {
				shortcut->size = 0;
				shortcuts_1key.erase(shortcuts_1key.begin() + i);
				return;
			}
		}
		break;

	case 2:
		unlistenForKey(shortcut->keys[0]);
		unlistenForKey(shortcut->keys[1]);

		for (uint32_t i = 0; i < shortcuts_2keys.size(); i++) {

			Shortcut* existing = shortcuts_2keys[i];

			if (existing == shortcut) {
				shortcut->size = 0;
				shortcuts_2keys.erase(shortcuts_2keys.begin() + i);
				return;
			}
		}
		break;

	case 3:
		unlistenForKey(shortcut->keys[0]);
		unlistenForKey(shortcut->keys[1]);
		unlistenForKey(shortcut->keys[2]);

		for (uint32_t i = 0; i < shortcuts_3keys.size(); i++) {

			Shortcut* existing = shortcuts_3keys[i];

			if (existing == shortcut) {
				shortcut->size = 0;
				shortcuts_3keys.erase(shortcuts_3keys.begin() + i);
				return;
			}
		}
		break;
	}
}

ErrStack InputState::update(const steady_time& time)
{
	// Keys
	std::for_each(std::execution::par_unseq, key_logs.begin(), key_logs.end(), [time](InputKey* key) {
	
		key->submit(GetAsyncKeyState(key->vk_key) & (1 << 15), time);
	});

	// Shortcuts
	{
		// 3 Key Shortcuts
		for (Shortcut* shortcut : shortcuts_3keys) {

			InputKey& key_0 = *shortcut->keys[0];
			InputKey& key_1 = *shortcut->keys[1];
			InputKey& key_2 = *shortcut->keys[2];

			if (key_0.is_down && key_1.is_down && key_2.is_down)
			{
				const InputKey& ti_01 = getTimeIntersection(key_0, key_1);
				const InputKey& ti_012 = getTimeIntersection(ti_01, key_2);

				if (ti_012.start_time < ti_012.end_time) {
					shortcut->duration = fsec_cast(ti_012.end_time - ti_012.start_time);

					key_0.consumed = true;
					key_1.consumed = true;
					key_2.consumed = true;

					continue;
				}
			}
			shortcut->duration = 0.0f;
		}

		// 2 Key Shortcuts
		for (Shortcut* shortcut : shortcuts_2keys) {

			InputKey& key_0 = *shortcut->keys[0];
			InputKey& key_1 = *shortcut->keys[1];

			if (!key_0.consumed && key_0.is_down && 
				!key_1.consumed && key_1.is_down) 
			{
				InputKey ti = getTimeIntersection(key_0, key_1);

				if (ti.start_time < ti.end_time) {
					shortcut->duration = fsec_cast(ti.end_time - ti.start_time);

					key_0.consumed = true;
					key_1.consumed = true;

					continue;
				}
			}
			shortcut->duration = 0.0f;
		}

		// 1 key Shortcuts
		for (Shortcut* shortcut : shortcuts_1key) {

			InputKey& key_0 = *shortcut->keys[0];

			if (!key_0.consumed) {
				shortcut->duration = shortcut->keys[0]->getDuration();
			}
			else {
				shortcut->duration = 0.0f;
			}
		}
	}

	// Cursor Screen position 
	{
		POINT point;
		if (!GetCursorPos(&point)) {
			return ErrStack(ExtraError::FAILED_TO_GET_CURSOR_SCREEN_POSITION, code_location, "failed to get mouse screen pos", getLastError());
		}
		this->screen_pos_x = point.x;
		this->screen_pos_y = point.y;
	}

	return ErrStack();
}
