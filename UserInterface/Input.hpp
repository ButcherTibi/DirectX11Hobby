#pragma once

// Standard
#include <chrono>

using SteadyTime = std::chrono::time_point<std::chrono::steady_clock>;

// will output float of second in duration
#define fsec_cast(duration_ns) \
	std::chrono::duration<float, std::ratio<1>>(duration_ns).count()

namespace nui {

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
		uint16_t virtual_key;

		bool is_down;
		bool first_message;
		bool first_frame;
		bool last_frame;
		SteadyTime start_time;
		SteadyTime end_time;

	public:
		uint64_t getDurationMiliSeconds();
	};

	class Input {
	public:
		// this list contains non-existent, reserved, unused virtual key codes
		std::array<KeyState, 0xFF> key_list;

		// Local Mouse Position
		uint16_t mouse_x;
		uint16_t mouse_y;

		// Mouse Delta Unbuffered
		int32_t mouse_delta_x;
		int32_t mouse_delta_y;

		// Mouse Wheel
		int16_t mouse_wheel_delta;

	public:

		void setKeyDownState(uint32_t wParam, uint32_t lParam);
		void setKeyUpState(uint32_t wParam);

		/* the Window Message Key Down events are not called every frame as such the end_time may lag behind */
		void startFrame();

		void debugPrint();

		/* unsets the first_frame set by WinProc */
		void endFrame();
	};
}