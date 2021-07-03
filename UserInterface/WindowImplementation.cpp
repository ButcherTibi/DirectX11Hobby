// Header
#include "NuiLibrary.hpp"


using namespace nui;


Text* Window::createText(Element* parent)
{
	if (parent == nullptr) {
		parent = std::get_if<Root>(&elements.front());
	}

	StoredElement& new_stored_elem = elements.emplace_back();

	auto& new_text = new_stored_elem.emplace<Text>();
	new_text._window = this;
	new_text._parent = parent;
	new_text._self_elements = std::prev(elements.end());
	new_text._init();

	// Text Specific
	new_text.font_family = "Roboto";
	new_text.font_style = "Regular";
	new_text.font_size = 14;
	new_text.line_height = 0;
	new_text.color.rgba = { 1, 1, 1, 1 };

	// Change
	auto& new_change = changes.emplace_back();
	auto& add_change = new_change.emplace<AddChange>();
	add_change.parent = parent;
	add_change.elem = &new_text;

	return &new_text;
}

RelativeWrap* Window::createRelativeWrap(Element* parent_element)
{
	if (parent_element == nullptr) {
		parent_element = std::get_if<Root>(&elements.front());
	}

	StoredElement& new_stored_elem = elements.emplace_back();

	auto& new_rel = new_stored_elem.emplace<RelativeWrap>();
	new_rel._window = this;
	new_rel._parent = parent_element;
	new_rel._self_elements = std::prev(elements.end());
	new_rel._init();

	// Change
	auto& new_change = changes.emplace_back();
	auto& add_change = new_change.emplace<AddChange>();
	add_change.parent = parent_element;
	add_change.elem = &new_rel;

	return &new_rel;
}

Flex* Window::createGrid(Element* parent_element)
{
	if (parent_element == nullptr) {
		parent_element = std::get_if<Root>(&elements.front());
	}

	StoredElement& new_stored_elem = elements.emplace_back();

	auto& new_grid = new_stored_elem.emplace<Flex>();
	new_grid._window = this;
	new_grid._parent = parent_element;
	new_grid._self_elements = std::prev(elements.end());
	new_grid._init();

	new_grid.orientation = Flex::Orientation::ROW;
	new_grid.items_spacing = Flex::Spacing::START;
	new_grid.lines_spacing = Flex::Spacing::START;

	// Change
	auto& new_change = changes.emplace_back();
	auto& add_change = new_change.emplace<AddChange>();
	add_change.parent = parent_element;
	add_change.elem = &new_grid;

	return &new_grid;
}

Menu* Window::createMenu(Element* parent_element)
{
	if (parent_element == nullptr) {
		parent_element = std::get_if<Root>(&elements.front());
	}

	StoredElement& new_stored_elem = elements.emplace_back();

	auto& new_menu = new_stored_elem.emplace<Menu>();
	new_menu._window = this;
	new_menu._parent = parent_element;
	new_menu._self_elements = std::prev(elements.end());
	new_menu._init();

	// Change
	auto& new_change = changes.emplace_back();
	auto& add_change = new_change.emplace<AddChange>();
	add_change.parent = parent_element;
	add_change.elem = &new_menu;

	return &new_menu;
}

void Window::deleteElement(Element* elem)
{
	assert_cond(elements.begin() != elem->_self_elements, "root cannot be deleted");

	auto& delete_change = changes.emplace_back().emplace<DeleteChange>();
	delete_change.target = elem->_self_elements;
}

void Window::deleteAllElements()
{
	// schedule all elements to be deleted except root

	// skip first
	auto iter = elements.begin();
	iter++;

	for (; iter != elements.end(); iter++) {

		auto& delete_change = changes.emplace_back().emplace<DeleteChange>();
		delete_change.target = iter;
	}
}

void Window::setEndEvent(WindowCallback callback, void* user_data)
{
	this->finalEvent = callback;
	this->final_event_user_data = user_data;
}

void Window::setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	Root* root = std::get_if<Root>(&elements.front());
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
