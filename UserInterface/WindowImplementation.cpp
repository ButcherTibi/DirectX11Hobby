// Header
#include "NuiLibrary.hpp"


using namespace nui;


void Window::createText(TextCreateInfo& info)
{
	root->createText(info);
}

Flex* Window::createFlex(FlexCreateInfo& info)
{
	return root->createFlex(info);
}

Rect* Window::createRectangle(RectCreateInfo& info)
{
	return root->createRect(info);
}

Menu* Window::createMenu(MenuCreateInfo& info)
{
	return root->createMenu(info);
}

//
//Rect* Window::createRectangle(Element* parent)
//{
//	if (parent == nullptr) {
//		parent = std::get_if<Root>(&elements.front());
//	}
//
//	StoredElement& new_stored_elem = elements.emplace_back();
//
//	auto& new_rect = new_stored_elem.emplace<Rect>();
//	new_rect._window = this;
//	new_rect._parent = parent;
//	new_rect._self_element = std::prev(elements.end());
//	new_rect._initDefaultProperties();
//
//	// Specific
//	new_rect.coloring.elem = &new_rect;
//	new_rect.coloring.now = Rect::Coloring::FLAT;
//
//	new_rect.color.elem = &new_rect;
//
//	// Internal
//	new_rect._rect_render.init(this);
//
//	// Change
//	auto& new_change = changes.emplace_back();
//	auto& add_change = new_change.emplace<AddChange>();
//	add_change.parent = parent;
//	add_change.elem = &new_rect;
//
//	return &new_rect;
//}
//
//Stack* Window::createRelativeWrap(Element* parent_element)
//{
//	if (parent_element == nullptr) {
//		parent_element = std::get_if<Root>(&elements.front());
//	}
//
//	StoredElement& new_stored_elem = elements.emplace_back();
//
//	auto& new_rel = new_stored_elem.emplace<Stack>();
//	new_rel._window = this;
//	new_rel._parent = parent_element;
//	new_rel._self_element = std::prev(elements.end());
//	new_rel._init();
//
//	// Change
//	auto& new_change = changes.emplace_back();
//	auto& add_change = new_change.emplace<AddChange>();
//	add_change.parent = parent_element;
//	add_change.elem = &new_rel;
//
//	return &new_rel;
//}

//Menu* Window::createMenu(Element* parent_element)
//{
//	if (parent_element == nullptr) {
//		parent_element = std::get_if<Root>(&elements.front());
//	}
//
//	StoredElement& new_stored_elem = elements.emplace_back();
//
//	auto& new_menu = new_stored_elem.emplace<Menu>();
//	new_menu._window = this;
//	new_menu._parent = parent_element;
//	new_menu._self_element = std::prev(elements.end());
//	new_menu._initDefaultProperties();
//
//	// Specific
//	new_menu.titles_background_color.elem = &new_menu;
//	new_menu.select_background_color.elem = &new_menu;
//
//	// Internal
//	MenuItem& root = new_menu._items.emplace_back();
//	root.parent = nullptr;
//
//	new_menu._visible_menus.push_back(&root);
//
//	new_menu._menu_background_render.init(this);
//	new_menu._select_background_render.init(this);
//	new_menu._label_render.init(this);
//
//	// Change
//	auto& new_change = changes.emplace_back();
//	auto& add_change = new_change.emplace<AddChange>();
//	add_change.parent = parent_element;
//	add_change.elem = &new_menu;
//
//	return &new_menu;
//}
//
//void Window::deleteElement(Element* elem)
//{
//	assert_cond(elements.begin() != elem->_self_element, "root cannot be deleted");
//
//	auto& delete_change = changes.emplace_back().emplace<DeleteChange>();
//	delete_change.target = elem->_self_element;
//}
//
//void Window::deleteAllElements()
//{
//	// schedule all elements to be deleted except root
//
//	// skip first
//	auto iter = elements.begin();
//	iter++;
//
//	for (; iter != elements.end(); iter++) {
//
//		auto& delete_change = changes.emplace_back().emplace<DeleteChange>();
//		delete_change.target = iter;
//	}
//}

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
		}

		// Elements
		{
			auto past_root = elements.begin();
			past_root++;

			elements.erase(past_root, elements.end());
		}
	}

	// Create UI elements
	callback(this, nullptr);

	// Delete Unused retained states
	{
		for (auto iter = text_prevs.begin(); iter != text_prevs.end(); iter++) {

			if (iter->used == false) {
				text_prevs.erase(iter);
			}
		}

		for (auto iter = flex_prevs.begin(); iter != flex_prevs.end(); iter++) {

			if (iter->used == false) {
				flex_prevs.erase(iter);
			}
		}

		for (auto iter = rect_prevs.begin(); iter != rect_prevs.end(); iter++) {

			if (iter->used == false) {
				rect_prevs.erase(iter);
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

		for (StoredElement2* child : root->_children) {
			_downward_now_elems.push_back(child->base_elem);
		}

		while (_downward_now_elems.size()) {

			for (Element* now_elem : _downward_now_elems) {

				auto calc_size_for_axis = [now_elem](uint32_t axis) {

					auto& size = now_elem->size[axis];
					auto& _size = now_elem->_size[axis];

					switch (size.type) {
					case ElementSizeType::RELATIVE: {
						auto& _parent_size = now_elem->_parent->base_elem->_size[axis];
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

				for (StoredElement2* child : now_elem->_children) {
					_downward_next_elems.push_back(child->base_elem);
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
		}

		_upward_now_elems.clear();
		_upward_now_elems.reserve(_leafs.size());

		for (Element* leaf : _leafs) {
			_upward_now_elems.insert(leaf);
		}

		_upward_next_elems.clear();

		while (_upward_now_elems.size()) {

			for (Element* now_elem : _upward_now_elems) {

				now_elem->_calcSizeAndRelativeChildPositions();

				if (now_elem->_parent != nullptr) {
					_upward_next_elems.insert(now_elem->_parent->base_elem);
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
		passed_child.elem = elements.front().base_elem;

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

				for (StoredElement2* child : elem->_children) {

					PassedElement& next_elem = _next_pelems.emplace_back();
					next_elem.ancestor_pos = elem->_position;
					next_elem.ancestor_z_index = elem->_z_index;
					next_elem.elem = child->base_elem;
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

void Window::setEndEvent(WindowCallback callback, void* user_data)
{
	this->finalEvent = callback;
	this->final_event_user_data = user_data;
}

void Window::setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	root->_events.setKeyDownEvent(callback, key, user_data);
}

void Window::endMouseDeltaEffect()
{
	switch (delta_effect) {
	case DeltaEffectType::HIDDEN: {
		setLocalMousePosition(begin_mouse_x, begin_mouse_y);
		setMouseVisibility(true);
	}
	}

	delta_owner_elem = nullptr;
	ClipCursor(nullptr);
}

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
