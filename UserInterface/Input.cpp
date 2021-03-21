// Header
#include "Input.hpp"


using namespace nui;


uint64_t KeyState::getDuration_ms()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
}

#pragma warning(disable : 4100)
void Input::setKeyDownState(uint32_t wParam, uint32_t lParam)
{
	KeyState& key = key_list[wParam];

	// key changed from UP to DOWN
	if (!key.is_down) {
		key.is_down = true;
		key.start_time = std::chrono::steady_clock::now();
		key.down_transition = true;
	}

	// key was already DOWN do nothing
}
#pragma warning(default : 4100)

void Input::setKeyUpState(uint32_t wParam)
{
	KeyState& key = key_list[wParam];

	// key changed from DOWN to UP
	if (key.is_down) {
		key.is_down = false;
		key.up_transition = true;
	}

	// key was already UP do nothing
}

void Input::debugPrint()
{
	for (uint16_t virtual_key = 0; virtual_key < key_list.size(); virtual_key++) {

		KeyState& key = key_list[virtual_key];
		if (key.is_down) {

			printf("key down = %X for %f ms \n", virtual_key,
				fsec_cast(key.end_time - key.start_time));
		}
	}
}