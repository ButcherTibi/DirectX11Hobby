#pragma once

#undef DELETE

// Standard
#include <chrono>
#include <array>
#include <vector>


using SteadyTime = std::chrono::time_point<std::chrono::steady_clock>;


inline uint32_t toMs(std::chrono::nanoseconds duration_ns)
{
	return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(duration_ns).count();
}


namespace VirtualKeys {
	enum : uint16_t {
		// Numbers
		NUMBER_0 = 0x30,
		NUMBER_1,
		NUMBER_2,
		NUMBER_3,
		NUMBER_4,
		NUMBER_5,
		NUMBER_6,
		NUMBER_7,
		NUMBER_8,
		NUMBER_9 = 0x39,

		// Letters
		A = 0x41,
		B,
		C,
		D,
		E,

		F,
		G,
		H,
		I,
		J,

		K,
		L,
		M,
		N,
		O,

		P,
		Q,
		R,
		S,
		T,

		U,
		V,
		W,
		X,
		Y,

		Z = 0x5A,

		// Punctuation
		SEMICOLON = VK_OEM_1,
		SINGLE_QUOTE = VK_OEM_7,
		COMMA = VK_OEM_COMMA,
		DOT = VK_OEM_PERIOD,

		// Text Editing
		BACKSPACE = VK_BACK,
		DELETE = VK_DELETE,
		SPACE = VK_SEPARATOR,
		ENTER = VK_RETURN,

		// Symbols
		SQUARE_BRACKET_OPEN = VK_OEM_4,
		SQUARE_BRACKET_CLOSE = VK_OEM_6,
		TILDA = VK_OEM_3,
		MINUS = VK_OEM_MINUS,
		PLUS = VK_OEM_PLUS,
		FORDWARD_SLASH = VK_OEM_5,
		BACK_SLASH = VK_OEM_2,

		// Mouse
		LEFT_MOUSE_BUTTON = VK_LBUTTON,
		RIGHT_MOUSE_BUTTON = VK_RBUTTON,
		MIDDLE_MOUSE_BUTTON = VK_MBUTTON,

		// Function
		TAB = VK_TAB,
		CAPS_LOCK = VK_CAPITAL,
		SHIFT = VK_SHIFT,
		CONTROL = VK_CONTROL,

		F1 = VK_F1,
		F2 = VK_F2,
		F3 = VK_F3,
		F4 = VK_F4,

		F5 = VK_F5,
		F6 = VK_F6,
		F7 = VK_F7,
		F8 = VK_F8,

		F9 = VK_F9,
		F10 = VK_F10,
		F11 = VK_F11,
		F12 = VK_F12,
	};
}


struct KeyState {
	bool is_down;
	SteadyTime start_time;
	SteadyTime end_time;
	bool down_transition;
	bool up_transition;

public:
	uint64_t getDuration_ms()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	}
};


struct CharacterKeyState {
	uint32_t code_point;
	bool down_transition;
};


struct MousePosition {
	uint16_t x;
	uint16_t y;
};


// Note to self:
// If hold down a key the keyboard will not send a key down event every frame, so to measure key down time
// start the timer on key down and stop only on key up event.
// DO NOT extend time on each key down event or else a weird ~500 time delay will occur as the keyboard repeat function buffer
// fills up and generates a key down message

// The Input class contains all input state from mouse and keyboard
class Input {
public:
	// this list also contains non-existent, reserved, unused virtual key codes
	std::array<KeyState, 0xFF> key_list;

	// unicode characters pressed
	std::vector<CharacterKeyState> unicode_list;

	// Local Mouse Position
	uint16_t mouse_x;
	uint16_t mouse_y;
	std::vector<MousePosition> mouse_pos_history;

	// Mouse Delta Unbuffered
	// will contain the sum total of inputs within a frame
	int32_t mouse_delta_x;
	int32_t mouse_delta_y;

	// Mouse Wheel Delta
	int16_t mouse_wheel_delta;

public:
	void setKeyDownState(uint32_t wParam, uint32_t)
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

	void setKeyUpState(uint32_t wParam)
	{
		KeyState& key = key_list[wParam];

		// key changed from DOWN to UP
		if (key.is_down) {
			key.is_down = false;
			key.up_transition = true;
		}

		// key was already UP do nothing
	}

	bool isDownTransition(uint32_t key)
	{
		return key_list[key].down_transition;
	}

	void debugPrint()
	{
		for (uint16_t virtual_key = 0; virtual_key < key_list.size(); virtual_key++) {

			KeyState& key = key_list[virtual_key];
			if (key.is_down) {

				printf("key down = %X for %d ms \n", virtual_key,
					toMs(key.end_time - key.start_time));
			}
		}
	}
};
