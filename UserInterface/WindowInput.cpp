#include "pch.h"

// Header
#include "WindowInput.hpp"


using namespace nui;


uint64_t KeyState::getDurationMiliSeconds()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
}

void Input::create()
{
	SteadyTime now = std::chrono::steady_clock::now();

	for (uint16_t virtual_key = 0; virtual_key < key_list.size(); virtual_key++) {

		KeyState& key = key_list[virtual_key];
		key.virtual_key = virtual_key;
		key.is_down = false;
		key.first_message = false;
		key.first_frame = false;
		key.start_time = now;
		key.end_time = now;
	}
}

void Input::startFrame()
{
	for (uint16_t virtual_key = 0; virtual_key < key_list.size(); virtual_key++) {

		KeyState& key = key_list[virtual_key];
		if (key.is_down) {

			if (!key.first_frame) {
				key.end_time = std::chrono::steady_clock::now();
			}
		}
	}
}

void Input::debugPrint()
{
	for (uint16_t virtual_key = 0; virtual_key < key_list.size(); virtual_key++) {

		KeyState& key = key_list[virtual_key];
		if (key.is_down) {

			printf("key down = %X for %f first_frame %d \n", key.virtual_key,
				fsec_cast(key.end_time - key.start_time),
				key.first_frame);
		}
	}
}

void Input::endFrame()
{
	for (uint16_t virtual_key = 0; virtual_key < key_list.size(); virtual_key++) {

		KeyState& key = key_list[virtual_key];
		key.first_frame = false;
	}
}