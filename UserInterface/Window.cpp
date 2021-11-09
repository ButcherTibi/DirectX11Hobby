module;

// Windows
#include "DietWindows.hpp"
#include "Windows.h"

// Standard
#include <chrono>

// DXGI
#include <dxgi1_6.h>

// Mine
#include "ErrorStack.hpp"
#include "Input.hpp"
#include "GPU_ShaderTypes.hpp"
#include "Properties.hpp"
#include "TimeStuff.hpp"


// Undefines
#undef RELATIVE
#undef ABSOLUTE

module UserInterface;


using namespace nui;


// std::list<Window*> _created_windows;


FORCEINLINE uint16_t getLowOrder(uint32_t param)
{
	return param & 0xFFFF;
}

FORCEINLINE uint16_t getHighOrder(uint32_t param)
{
	return param >> 16;
}

FORCEINLINE int16_t getSignedLowOrder(uint32_t param)
{
	return param & 0xFFFF;
}

FORCEINLINE int16_t getSignedHighOrder(uint32_t param)
{
	return param >> 16;
}

LRESULT CALLBACK nui::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ErrStack err_stack;

	for (Window* w : _created_windows) {
		if (w->hwnd == hwnd) {

			switch (uMsg) {

			case WM_SIZE: {

				switch (wParam) {
				case SIZE_MAXIMIZED:
				case SIZE_RESTORED: {
					w->win_messages.is_minimized = false;

					w->width = getLowOrder((uint32_t)lParam);
					w->height = getHighOrder((uint32_t)lParam);

					RECT client_rect;
					GetClientRect(hwnd, &client_rect);

					w->surface_width = (client_rect.right - client_rect.left);
					w->surface_height = (client_rect.bottom - client_rect.top);

					//w->_updateCPU();
					//w->_render();
					break;
				}

				case SIZE_MINIMIZED: {
					w->win_messages.is_minimized = true;
					break;
				}
				}

				return 0;
			}

			case WM_MOUSEMOVE: {
				w->input.mouse_x = getLowOrder((uint32_t)lParam);
				w->input.mouse_y = getHighOrder((uint32_t)lParam);

				auto& new_pos = w->input.mouse_pos_history.emplace_back();
				new_pos.x = w->input.mouse_x;
				new_pos.y = w->input.mouse_y;
				return 0;
			}

			case WM_MOUSEWHEEL: {
				w->input.mouse_wheel_delta += GET_WHEEL_DELTA_WPARAM(wParam);
				return 0;
			}

			case WM_KEYDOWN: {
				w->input.setKeyDownState((uint32_t)wParam, (uint32_t)lParam);
				return 0;
			}

			case WM_KEYUP: {
				w->input.setKeyUpState((uint32_t)wParam);
				return 0;
			}

			case WM_LBUTTONDOWN: {
				w->input.setKeyDownState(VirtualKeys::LEFT_MOUSE_BUTTON, 0);
				return 0;
			}

			case WM_LBUTTONUP: {
				w->input.setKeyUpState(VirtualKeys::LEFT_MOUSE_BUTTON);
				return 0;
			}

			case WM_RBUTTONDOWN: {
				w->input.setKeyDownState(VirtualKeys::RIGHT_MOUSE_BUTTON, 0);
				return 0;
			}

			case WM_RBUTTONUP: {
				w->input.setKeyUpState(VirtualKeys::RIGHT_MOUSE_BUTTON);
				return 0;
			}

			case WM_MBUTTONDOWN: {
				w->input.setKeyDownState(VirtualKeys::MIDDLE_MOUSE_BUTTON, 0);
				return 0;
			}

			case WM_MBUTTONUP: {
				w->input.setKeyUpState(VirtualKeys::MIDDLE_MOUSE_BUTTON);
				return 0;
			}

			case WM_INPUT: {
				uint32_t count;
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &count,
					sizeof(RAWINPUTHEADER));

				std::vector<uint8_t> raw_input(count);
				if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw_input.data(), &count,
					sizeof(RAWINPUTHEADER)) == (uint32_t)-1)
				{
					printf("failed to get raw input data \n %s \n %s \n", code_location, getLastError().c_str());
				}

				RAWINPUT* raw = (RAWINPUT*)raw_input.data();
				w->input.mouse_delta_x += raw->data.mouse.lLastX;
				w->input.mouse_delta_y += raw->data.mouse.lLastY;

				// printf("mouse delta = %d %d \n", w->input.mouse_delta_x, w->input.mouse_delta_y);
				return 0;
			}

			case WM_QUIT:
			case WM_CLOSE: {
				w->win_messages.should_close = true;
				return 0;
			}

			case WM_DESTROY: {
				// emergency exit do not save progress
				std::abort();
			}
			}
		}
	}
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}


void Window::_render()
{
	// don't render if display too small or window is not visible
	if (win_messages.is_minimized ||
		surface_width < 10 || surface_height < 10)
	{
		return;
	}

	auto& im_ctx3 = instance->im_ctx3;
	auto& dev5 = instance->dev5;

	// Load Character Atlas
	instance->_loadCharacterAtlasToTexture();

	// Constant buffer data
	{
		cbuff.setUint(GPU_ConstantsFields::SCREEN_WIDTH, surface_width);
		cbuff.setUint(GPU_ConstantsFields::SCREEN_HEIGHT, surface_height);
	}

	// Resize Attachments
	{
		DXGI_SWAP_CHAIN_DESC desc;
		swapchain1->GetDesc(&desc);

		if (desc.BufferDesc.Width != surface_width || desc.BufferDesc.Height != surface_height) {

			present_rtv->Release();
			present_img->Release();

			throwDX11(swapchain1->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, desc.Flags));

			throwDX11(swapchain1->GetBuffer(0, IID_PPV_ARGS(present_img.GetAddressOf())));

			throwDX11(dev5->CreateRenderTargetView(present_img.Get(), NULL, present_rtv.GetAddressOf()));
		}
	}

	// Viewport
	{
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)surface_width;
		viewport.Height = (float)surface_height;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
	}

	// Draw
	im_ctx3->ClearState();

	for (auto& stack : draw_stacks) {
		for (Element* elem : stack.second) {

			elem->_draw();
		}
	}

#if _DEBUG
	throwDX11(swapchain1->Present(0, 0));
#else
	// go fuck yourself DXGI, just ignore my presentation attempt if you can't handle it
	// NOTE: never rely on presentation being correct or incorrect as the context of presenting and it's
	// meaning can vary depending on Windows's behaviour and even version
	// Example:
	// if you minimize the app via buttons or context menu it works
	// if press the taskbar To Desktop button it crashes, apparently that was not a minimization
	swapchain1->Present(0, 0);
#endif
}

void Window::createText(Text::CreateInfo& info)
{
	root->createText(info);
}

Rect* Window::createRectangle(RectCreateInfo& info)
{
	return root->createRect(info);
}

void Window::createButton(Button::CreateInfo& info)
{
	root->createButton(info);
}

void Window::createSlider(Slider::CreateInfo& info)
{
	root->createSlider(info);
}

void Window::createDropdown(Dropdown::CreateInfo& info)
{
	root->createDropdown(info);
}

Flex* Window::createFlex(FlexCreateInfo& info)
{
	return root->createFlex(info);
}

Menu* Window::createMenu(MenuCreateInfo& info)
{
	return root->createMenu(info);
}

//TreeList* Window::createTreeList(TreeListCreateInfo& new_info)
//{
//	StoredElement2& new_elem = retained_elements.emplace_back();
//
//	auto& new_tree = new_elem.specific_elem.emplace<TreeList>();
//	new_tree._window = this;
//	new_tree._parent = nullptr;
//	new_tree._self = &new_elem;
//
//	new_elem.base_elem = &new_tree;
//
//	new_tree._calcNowState(&new_tree.base_elem_state, new_info);
//	new_tree.info = new_info;
//
//	// Init
//	TreeListItem& root_item = new_tree.items.emplace_back();
//	root_item.parent = 0xFFFF'FFFF;
//
//	return &new_tree;
//}
//
void Window::update(WindowCallback callback)
{
	// Calculate Delta Factor
	{
		SteadyTime now = std::chrono::steady_clock::now();

		delta_time = (float)toMs((now - frame_start_time)) / std::chrono::milliseconds(16).count();
		//printf("delta_time = %f \n", delta_time);
		frame_start_time = now;
	}

	// Reset Input
	{
		input.mouse_pos_history.clear();

		input.mouse_delta_x = 0;
		input.mouse_delta_y = 0;
		input.mouse_wheel_delta = 0;

		for (uint16_t virtual_key = 0; virtual_key < input.key_list.size(); virtual_key++) {

			KeyState& key = input.key_list[virtual_key];
			key.down_transition = false;
			key.up_transition = false;
		}
	}

	// Read Input
	MSG msg{};
	while (PeekMessageA(&msg, hwnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	// Calculate time for keys
	for (uint16_t virtual_key = 0; virtual_key < input.key_list.size(); virtual_key++) {

		KeyState& key = input.key_list[virtual_key];

		if (key.is_down) {
			key.end_time = frame_start_time;
		}
		else {
			key.end_time = key.start_time;
		}
	}

	// Emit events for previous frame input
	{
		bool emit_inside_events = true;

		for (auto i = draw_stacks.rbegin(); i != draw_stacks.rend(); ++i) {

			std::list<Element*>& elems = i->second;

			for (auto j = elems.rbegin(); j != elems.rend(); ++j) {

				Element* elem = *j;
				elem->_emitEvents(emit_inside_events);
			}
		}

		if (finalEvent != nullptr) {
			finalEvent(this, final_event_user_data);
		}
	}

	// Delete everything from previous frame
	{
		// Reset root element
		{
			root->_children.clear();

			for (auto& retained_elem : retained_elements) {
				retained_elem.base_elem->_parent = nullptr;
				retained_elem.base_elem->_children.clear();
			}
		}

		// Elements
		{
			elements.clear();
		}
	}

	// Create UI elements
	callback(this, nullptr);

	// Delete Unused retained states
	{
		// @TODO: refactor these in template like createElement
		for (auto iter = text_prevs.begin(); iter != text_prevs.end(); iter++) {

			if (iter->used == false) {
				text_prevs.erase(iter);
			}
		}	

		for (auto iter = rect_prevs.begin(); iter != rect_prevs.end(); iter++) {

			if (iter->used == false) {
				rect_prevs.erase(iter);
			}
		}

		for (auto iter = button_prevs.begin(); iter != button_prevs.end(); iter++) {

			if (iter->used == false) {
				button_prevs.erase(iter);
			}
		}

		for (auto iter = slider_prevs.begin(); iter != slider_prevs.end(); iter++) {

			if (iter->used == false) {
				slider_prevs.erase(iter);
			}
		}

		for (auto iter = dropdown_prevs.begin(); iter != dropdown_prevs.end(); iter++) {
			if (iter->used == false) {
				dropdown_prevs.erase(iter);
			}
		}

		for (auto iter = dx11_viewport_prevs.begin(); iter != dx11_viewport_prevs.end(); iter++) {
			if (iter->used == false) {
				dx11_viewport_prevs.erase(iter);
			}
		}

		for (auto iter = flex_prevs.begin(); iter != flex_prevs.end(); iter++) {

			if (iter->used == false) {
				flex_prevs.erase(iter);
			}
		}

		for (auto iter = menu_prevs.begin(); iter != menu_prevs.end(); iter++) {

			if (iter->used == false) {
				menu_prevs.erase(iter);
			}
		}
	}

	// Top Down Pass for calculating size for elements that have
	// - absolute size
	// - relative size dependent on computed parent size
	{
		_downward_now_elems.clear();
		_downward_next_elems.clear();

		root->_size[0] = surface_width;
		root->_size[1] = surface_height;

		for (Element* child : root->_children) {
			_downward_now_elems.push_back(child);
		}

		while (_downward_now_elems.size()) {

			for (Element* now_elem : _downward_now_elems) {

				auto calc_size_for_axis = [now_elem](uint32_t axis) {

					auto& size = now_elem->size[axis];
					auto& _size = now_elem->_size[axis];

					switch (size.type) {
					case ElementSizeType::RELATIVE: {
						auto& _parent_size = now_elem->_parent->_size[axis];
						_size = std::lroundf(_parent_size * size.relative_size);
						break;
					}

					case ElementSizeType::ABSOLUTE: {
						_size = size.absolute_size;
						break;
					}

					case ElementSizeType::FIT: {
						// size cannot be calculated at this pass
						_size = 0;
						break;
					}
					}
				};
				calc_size_for_axis(0);
				calc_size_for_axis(1);

				for (Element* child : now_elem->_children) {
					_downward_next_elems.push_back(child);
				}
			}

			_downward_now_elems.swap(_downward_next_elems);
			_downward_next_elems.clear();
		}
	}

	// Down Up Pass for elements that depend on children (starts from leafs)
	{
		// Gather Leafs
		{
			_leafs.clear();

			for (StoredElement2& stored_elem : elements) {

				if (stored_elem.base_elem->_children.size() == 0) {
					_leafs.push_back(stored_elem.base_elem);
				}
			}

			for (StoredElement2& stored_elem : retained_elements) {

				Element* elem = stored_elem.base_elem;

				// if element has no children and is attached to the graph
				if (elem->_children.size() == 0 && elem->_parent != nullptr) {
					_leafs.push_back(stored_elem.base_elem);
				}
			}
		}

		_upward_now_elems.clear();


		for (Element* leaf : _leafs) {
			_upward_now_elems.insert(leaf);
		}

		_upward_next_elems.clear();

		while (_upward_now_elems.size()) {

			for (Element* now_elem : _upward_now_elems) {

				now_elem->_calcSizeAndRelativeChildPositions();

				if (now_elem->_parent != nullptr) {
					_upward_next_elems.insert(now_elem->_parent);
				}
			}

			_upward_now_elems.swap(_upward_next_elems);
			_upward_next_elems.clear();
		}
	}

	// Convert relative to parent positions to screen coordinates
	// And establish drawing order
	{
		draw_stacks.clear();

		_now_pelems.clear();

		PassedElement& passed_child = _now_pelems.emplace_back();
		passed_child.ancestor_pos = { 0, 0 };
		passed_child.ancestor_z_index = 0;
		passed_child.elem = root;

		_next_pelems.clear();

		while (_now_pelems.size()) {

			for (PassedElement& now_elem : _now_pelems) {

				// convert position to screen space
				Element* elem = now_elem.elem;
				elem->_position[0] += now_elem.ancestor_pos[0];
				elem->_position[1] += now_elem.ancestor_pos[1];

				// convert z index to draw call order
				switch (elem->z_index.type) {
				case Z_IndexType::INHERIT: {
					elem->_z_index = now_elem.ancestor_z_index;
					break;
				}
				case Z_IndexType::RELATIVE: {
					elem->_z_index = now_elem.ancestor_z_index + elem->z_index.value;
					break;
				}
				case Z_IndexType::ABSOLUTE: {
					elem->_z_index = elem->z_index.value;
					break;
				}
				}

				draw_stacks[elem->_z_index].push_back(elem);

				for (Element* child : elem->_children) {

					PassedElement& next_elem = _next_pelems.emplace_back();
					next_elem.ancestor_pos = elem->_position;
					next_elem.ancestor_z_index = elem->_z_index;
					next_elem.elem = child;
				}
			}

			_now_pelems.swap(_next_pelems);
			_next_pelems.clear();
		}
	}

	// Mouse Delta Trap
	if (delta_owner_elem != nullptr) {

		// Trap the Mouse position
		int32_t local_top = delta_owner_elem->_position[1];
		int32_t local_bot = local_top + delta_owner_elem->_size[1];
		int32_t local_left = delta_owner_elem->_position[0];
		int32_t local_right = local_left + delta_owner_elem->_size[0];

		RECT client_rect = getClientRectangle();
		RECT new_trap;
		new_trap.top = client_rect.top + local_top;
		new_trap.bottom = client_rect.top + local_bot;
		new_trap.left = client_rect.left + local_left;
		new_trap.right = client_rect.left + local_right;

		ClipCursor(&new_trap);

		switch (delta_effect) {
		case DeltaEffectType::LOOP: {
			POINT mouse_screen_pos;
			GetCursorPos(&mouse_screen_pos);

			if (mouse_screen_pos.y <= new_trap.top) {
				setLocalMousePosition(input.mouse_x, local_bot - 1);
			}
			else if (mouse_screen_pos.y >= new_trap.bottom - 1) {
				setLocalMousePosition(input.mouse_x, local_top + 1);
			}
			else if (mouse_screen_pos.x <= new_trap.left) {
				setLocalMousePosition(local_right + 1, input.mouse_y);
			}
			else if (mouse_screen_pos.x >= new_trap.right - 1) {
				setLocalMousePosition(local_left + 1, input.mouse_y);
			}
			break;
		}

		case DeltaEffectType::HIDDEN: {
			setMouseVisibility(false);
			break;
		}
		}
	}

	// Draw the frame
	_render();

	// Frame Rate Limit
	{
		this->frame_used_time = std::chrono::steady_clock::now();
		SteadyTime target_end_time = frame_start_time + std::chrono::milliseconds(min_frame_duration_ms);

		// finished up early
		if (frame_used_time < target_end_time) {

			auto sleep_duration = std::chrono::duration_cast<std::chrono::milliseconds>(target_end_time - frame_used_time);
			Sleep((uint32_t)sleep_duration.count());
		}
	}
}

//void Window::setEndEvent(WindowCallback callback, void* user_data)
//{
//	this->finalEvent = callback;
//	this->final_event_user_data = user_data;
//}
//
//void Window::setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data)
//{
//	root->_events.setKeyDownEvent(callback, key, user_data);
//}
//
//void Window::endMouseDeltaEffect()
//{
//	switch (delta_effect) {
//	case DeltaEffectType::HIDDEN: {
//		setLocalMousePosition(begin_mouse_x, begin_mouse_y);
//		setMouseVisibility(true);
//	}
//	}
//
//	delta_owner_elem = nullptr;
//	ClipCursor(nullptr);
//}

RECT Window::getClientRectangle()
{
	RECT win_rect;
	GetWindowRect(hwnd, &win_rect);

	RECT client_rect;
	GetClientRect(hwnd, &client_rect);

	uint32_t border_thick = ((win_rect.right - win_rect.left) - client_rect.right) / 2;
	uint32_t header_height;
	{
		uint32_t win_height = win_rect.bottom - win_rect.top;
		uint32_t client_height = client_rect.bottom - client_rect.top;
		header_height = win_height - (client_height + border_thick * 2);
	}

	win_rect.left += border_thick;
	win_rect.right -= border_thick;
	win_rect.top += border_thick + header_height;
	win_rect.bottom -= border_thick;

	return win_rect;
}

bool Window::setLocalMousePosition(uint32_t x, uint32_t y)
{
	input.mouse_x = (uint16_t)x;
	input.mouse_y = (uint16_t)y;

	RECT client_rect = getClientRectangle();
	return SetCursorPos(client_rect.left + x, client_rect.top + y);
}

bool Window::untrapMousePosition()
{
	return ClipCursor(nullptr);
}

void Window::setMouseVisibility(bool is_visible)
{
	if (is_visible) {
		int32_t internal_display_counter = ShowCursor(true);
		while (internal_display_counter < 0) {
			internal_display_counter = ShowCursor(true);
		}
	}
	else {
		int32_t internal_display_counter = ShowCursor(false);
		while (internal_display_counter >= 0) {
			internal_display_counter = ShowCursor(false);
		}
	}
}
