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
	new_text._self = &new_stored_elem;
	new_text._init();

	// Text Specific
	new_text.font_family = "Roboto";
	new_text.font_style = "Regular";
	new_text.font_size = 14;
	new_text.line_height = 0;
	new_text.color.rgba = { 1, 1, 1, 1 };

	parent->_children.push_back(&new_text);
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
	new_rel._self = &new_stored_elem;
	new_rel._init();

	parent_element->_children.push_back(&new_rel);

	return &new_rel;
}

Grid* Window::createGrid(Element* parent_element)
{
	if (parent_element == nullptr) {
		parent_element = std::get_if<Root>(&elements.front());
	}

	StoredElement& new_stored_elem = elements.emplace_back();

	auto& new_grid = new_stored_elem.emplace<Grid>();
	new_grid._window = this;
	new_grid._parent = parent_element;
	new_grid._self = &new_stored_elem;
	new_grid._init();

	new_grid.orientation = GridOrientation::ROW;
	new_grid.items_spacing = GridSpacing::START;
	new_grid.lines_spacing = GridSpacing::START;

	parent_element->_children.push_back(&new_grid);

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
	new_menu._self = &new_stored_elem;
	new_menu._init();

	parent_element->_children.push_back(&new_menu);

	return &new_menu;
}

void Window::setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	Root* root = std::get_if<Root>(&elements.front());
	root->_events.setKeyDownEvent(callback, key, user_data);
}

void Window::endMouseDelta()
{
	this->delta_owner_elem = nullptr;
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

bool Window::trapLocalMousePosition(BoundingBox2D<uint32_t>& box)
{
	RECT client_rect = getClientRectangle();

	RECT rect;
	rect.left = client_rect.left + box.x0;
	rect.right = client_rect.left + box.x1;
	rect.top = client_rect.top + box.y0;
	rect.bottom = client_rect.top + box.y1;
	return ClipCursor(&rect);
}

bool Window::untrapMousePosition()
{
	return ClipCursor(nullptr);
}

void Window::hideMousePointer(bool hide)
{
	if (hide) {
		int32_t internal_display_counter = ShowCursor(false);
		while (internal_display_counter  >= 0) {
			internal_display_counter = ShowCursor(false);
		}
	}
	else {
		int32_t internal_display_counter = ShowCursor(true);
		while (internal_display_counter < 0) {
			internal_display_counter = ShowCursor(true);
		}
	}
}
