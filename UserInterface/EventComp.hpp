#pragma once

// Standard
#include <chrono>


using SteadyTime = std::chrono::time_point<std::chrono::steady_clock>;


namespace nui {

	// Forward
	class Node;
	class Window;


	struct MouseHoverEvent {
		Node* source;
		uint16_t mouse_x;
		uint16_t mouse_y;
		float duration;
		void* user_ptr;
	};

	struct MouseEnterEvent {
		Node* source;
		uint16_t mouse_x;
		uint16_t mouse_y;
		void* user_ptr;
	};

	struct MouseLeaveEvent {
		Node* source;
		uint16_t mouse_x;
		uint16_t mouse_y;
		void* user_ptr;
	};

	struct MouseMoveEvent {
		Node* source;
		uint16_t mouse_x;
		uint16_t mouse_y;
		void* user_ptr;
	};

	struct MouseDeltaEvent {
		Node* source;
		int32_t delta_x;
		int32_t delta_y;
		float duration;
		void* user_ptr;
	};

	struct KeyDownEvent {
		Node* source;
		uint32_t key;
		void* user_ptr;
	};

	struct KeyHeldDownEvent {
		Node* source;
		uint32_t key;
		float duration;
		void* user_ptr;
	};

	struct KeyUpEvent {
		Node* source;
		uint32_t key;
		void* user_ptr;
	};

	typedef void (*MouseHoverCallback)(MouseHoverEvent& hover_event);
	typedef void (*MouseEnterCallback)(MouseEnterEvent& hover_event);
	typedef void (*MouseLeaveCallback)(MouseLeaveEvent& hover_event);
	typedef void (*MouseMoveCallback)(MouseMoveEvent& move_event);

	typedef void (*MouseDeltaBeginCallback)(MouseDeltaEvent& mouse_delta_begin_event);
	typedef void (*MouseDeltaCallback)(MouseDeltaEvent& mouse_delta_event);
	typedef void (*MouseDeltaEndCallback)(MouseDeltaEvent& mouse_delta_end_event);

	typedef void (*KeyDownCallback)(KeyDownEvent& key_down_event);
	typedef void (*KeyHeldDownCallback)(KeyHeldDownEvent& key_held_down_event);
	typedef void (*KeyUpCallback)(KeyUpEvent& key_up_event);

	struct KeyDown {
		uint32_t key;
		KeyDownEvent event;
		KeyDownCallback callback;
	};

	struct KeyHeldDown {
		uint32_t key;
		SteadyTime start_time;
		KeyHeldDownEvent event;
		KeyHeldDownCallback callback;
	};

	struct KeyUp {
		uint32_t key;
		KeyUpEvent event;
		KeyUpCallback callback;
	};

	enum class MouseState {
		OFF,
		ENTER,
		LEAVE
	};

	enum class MouseDeltaState {
		OFF,
		START,
		NOW,
		END
	};

	class EventComp {
	public:
		Window* window;
		Node* source;

		uint16_t last_mouse_x;
		uint16_t last_mouse_y;
		SteadyTime mouse_enter_time;
		MouseState mouse_state;
		MouseDeltaState mouse_delta_state;

		// Event State
		MouseHoverEvent hover_event;
		MouseEnterEvent enter_event;
		MouseLeaveEvent leave_event;
		MouseMoveEvent move_event;

		// Callbacks
		void (*onMouseHover)(MouseHoverEvent& hover_event);
		void (*onMouseEnter)(MouseEnterEvent& enter_event);
		void (*onMouseLeave)(MouseLeaveEvent& leave_event);
		void (*onMouseMove)(MouseMoveEvent& move_event);

		std::list<KeyDown> keys_down;
		std::list<KeyHeldDown> keys_held_down;
		std::list<KeyUp> keys_up;

	public:

	public:
		void _create(Window* wnd, Node* node);

		/* mouse delta is ended by explicit end call or by being outside */
		void _endMouseDelta();

		void _emitInsideEvents();
		void _emitOutsideEvents();

		void setMouseHoverEvent(MouseHoverCallback callback, void* user_ptr = nullptr);
		void setMouseEnterEvent(MouseEnterCallback callback, void* user_ptr = nullptr);
		void setMouseLeaveEvent(MouseLeaveCallback callback, void* user_ptr = nullptr);
		void setMouseMoveEvent(MouseMoveCallback callback, void* user_ptr = nullptr);

		void beginMouseDelta();
		void endMouseDelta();

		void addKeyDownEvent(KeyDownCallback callback, uint32_t key, void* user_ptr = nullptr);
		void addKeyHeldDownEvent(KeyHeldDownCallback callback, uint32_t key, void* user_ptr = nullptr);
		void addKeyUpEvent(KeyUpCallback callback, uint32_t key, void* user_ptr = nullptr);

		bool removeKeyDownEvent(uint32_t key);
		bool removeKeyHeldDownEvent(uint32_t key);
		bool removeKeyUpEvent(uint32_t key);
	};
}
