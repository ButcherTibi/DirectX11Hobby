#pragma once

// Standard
#include <array>
#include <queue>
#include <chrono>

// mine
#include "ErrorStuff.h"

using steady_time = std::chrono::time_point<std::chrono::steady_clock>;

// will output float of second in duration
#define fsec_cast(duration_ns) \
	std::chrono::duration<float, std::ratio<1>>(duration_ns).count()


// Key codes for use ONLY with Windows's messages
enum WindowsVirtualKeys : int {
	// Numbers
	VK_0 = 0x30,
	VK_1,
	VK_2,
	VK_3,
	VK_4,
	VK_5,
	VK_6,
	VK_7,
	VK_8,
	VK_9 = 0x39,

	// Letters
	VK_A = 0x41,
	VK_B,
	VK_C,
	VK_D,
	VK_E,

	VK_F,
	VK_G,
	VK_H,
	VK_I,
	VK_J,

	VK_K,
	VK_L,
	VK_M,
	VK_N,
	VK_O,

	VK_P,
	VK_Q,
	VK_R,
	VK_S,
	VK_T,

	VK_U,
	VK_V,
	VK_W,
	VK_X,
	VK_Y,

	VK_Z = 0x5A
};


struct InputKey {
	int32_t vk_key;  // Virtual Windows key code

	bool is_down = false;
	steady_time start_time;  // time when key was pressed
	steady_time end_time;  // time when key was last pressed

	bool consumed = false;  // key input consumed
	uint32_t shortcuts = 0;  // how many shortcuts use this key

public:
	/* for how long was the key pressed */
	float getDuration();

	/* submit new state to update key */
	void submit(bool state, steady_time time);
};

/* Reminder for why I made this:
 * - 2 shortcuts using overlapping but different key combinations will not trigger simultaneously */
struct Shortcut
{
	std::array<InputKey*, 3> keys;
	uint32_t size = 0;  // number of bound keys

	float duration = 0;  // how long was this shortcut triggered
};


class InputState {
	// Keyboard
	std::vector<InputKey*> key_logs;

	// Shortcuts
	std::vector<Shortcut*> shortcuts_3keys;
	std::vector<Shortcut*> shortcuts_2keys;
	std::vector<Shortcut*> shortcuts_1key;

public:
	// Letter keys
	InputKey key_a;
	InputKey key_d;
	InputKey key_f;
	InputKey key_s;
	InputKey key_w;

	// Special keys
	InputKey key_lcontrol;

	// Mouse keys
	InputKey key_mouse_left;
	InputKey key_mouse_right;
	InputKey key_mouse_middle;

	// The cursors's screen position
	int32_t screen_pos_x;
	int32_t screen_pos_y;

	/* Current frame mouse deltas
	 * Initilized from WinProc */
	int32_t mouse_delta_x;
	int32_t mouse_delta_y;

	// Shortcuts
	Shortcut rotate_camera;
	Shortcut zoom_camera;
	Shortcut pan_camera;

	Shortcut focus_camera;

public:
	InputState();

	/* keys must be first registered to enable their update each frame */
	void listenForKey(InputKey* key);
	void unlistenForKey(InputKey* key);

	void addShortcut(Shortcut* shortcut, InputKey* key);
	void addShortcut(Shortcut* shortcut, InputKey* key_0, InputKey* key_1);
	void addShortcut(Shortcut* shortcut, InputKey* key_0, InputKey* key_1, InputKey* key_2);
	void removeShortcut(Shortcut* shortcut);

	/* updates the keys and mouse state */
	ErrStack update(const steady_time& time);
};

extern InputState input;
