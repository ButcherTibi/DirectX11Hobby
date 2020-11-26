#include "pch.h"

// Header
#include "Input.hpp"


using namespace nui;


uint64_t KeyState::getDurationMiliSeconds()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
}

void Input::setKeyDownState(uint32_t wParam, uint32_t lParam)
{
	KeyState& key = key_list[wParam];

	if (key.is_down) {
		key.end_time = std::chrono::steady_clock::now();
	}
	// key changed from UP to DOWN
	else {
		key.is_down = true;
		key.first_frame = true;
		key.start_time = std::chrono::steady_clock::now();
		key.end_time = key.start_time;
	}

	key.first_message = !(lParam & (1 << 30));
}

void Input::setKeyUpState(uint32_t wParam)
{
	KeyState& key = key_list[wParam];

	// key changed from DOWN to UP
	if (key.is_down) {
		key.is_down = false;
		key.last_frame = true;
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
	mouse_delta_x = 0;
	mouse_delta_y = 0;

	mouse_wheel_delta = 0;

	for (uint16_t virtual_key = 0; virtual_key < key_list.size(); virtual_key++) {

		KeyState& key = key_list[virtual_key];
		key.first_frame = false;
		key.last_frame = false;
	}
}