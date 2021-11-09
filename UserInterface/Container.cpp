module;

#include <string>

module UserInterface;

using namespace nui;


template<typename T, typename T_CreateInfo, typename T_RetainedState>
T* Container::createElement(T_CreateInfo& info, std::list<T_RetainedState>& states)
{
	StoredElement2& new_entry = _window->elements.emplace_back();

	T& new_elem = new_entry.specific_elem.emplace<T>();
	new_elem._window = _window;
	new_elem._parent = _self->base_elem;
	new_elem._self = &new_entry;

	new_entry.base_elem = &new_elem;

	T_RetainedState* prev = nullptr;
	{
		// find or create new state
		for (T_RetainedState& state : states) {
			if (state.id == info.id) {
				prev = &state;
				break;
			}
		}

		if (prev == nullptr) {
			prev = &states.emplace_back();
			prev->id = info.id;
		}

		// Element Retained State
		prev->used = true;

		// Specific Element Retained State
		prev->info = info;
	}

	new_elem._calcNowState(prev, info);
	new_elem.state = prev;

	_children.push_back(&new_elem);

	return &new_elem;
}


void Container::createText(Text::CreateInfo& info)
{
	createElement<Text, Text::CreateInfo, Text::RetainedState>
		(info, _window->text_prevs);
}

Rect* Container::createRect(RectCreateInfo& info)
{
	return createElement<Rect, RectCreateInfo, Rect::RetainedState>
		(info, _window->rect_prevs);
}

void Container::createButton(Button::CreateInfo& info)
{
	createElement<Button, Button::CreateInfo, Button::RetainedState>
		(info, _window->button_prevs);
}

Flex* Container::createFlex(FlexCreateInfo& info)
{
	StoredElement2& new_entry = _window->elements.emplace_back();

	Flex& new_flex = new_entry.specific_elem.emplace<Flex>();
	new_flex._window = _window;
	new_flex._parent = _self->base_elem;
	new_entry.base_elem = &new_flex;
	new_flex._self = &new_entry;

	FlexRetainedState* prev = nullptr;
	{
		for (FlexRetainedState& state : _window->flex_prevs) {
			if (state.id == info.id) {
				prev = &state;
				break;
			}
		}

		if (prev == nullptr) {
			prev = &_window->flex_prevs.emplace_back();
		}

		prev->id = info.id;
		prev->used = true;
	}

	new_flex._calcNowState(prev, info);

	new_flex.orientation = info.orientation;
	new_flex.items_spacing = info.items_spacing;
	new_flex.lines_spacing = info.lines_spacing;

	_children.push_back(&new_flex);

	return &new_flex;
}

void Container::createSlider(Slider::CreateInfo& info)
{
	createElement<Slider, Slider::CreateInfo, Slider::RetainedState>
		(info, _window->slider_prevs);
}

void Container::createDropdown(Dropdown::CreateInfo& info)
{
	createElement<Dropdown, Dropdown::CreateInfo, Dropdown::RetainedState>
		(info, _window->dropdown_prevs);
}

void Container::createDirectX11_Viewport(DirectX11_Viewport::CreateInfo& info)
{
	createElement<DirectX11_Viewport, DirectX11_Viewport::CreateInfo, DirectX11_Viewport::RetainedState>
		(info, _window->dx11_viewport_prevs);
}

Menu* Container::createMenu(MenuCreateInfo& info)
{
	StoredElement2& new_entry = _window->elements.emplace_back();

	Menu& new_menu = new_entry.specific_elem.emplace<Menu>();
	new_menu._window = _window;
	new_menu._parent = _self->base_elem;
	new_menu._self = &new_entry;

	new_entry.base_elem = &new_menu;
	_children.push_back(&new_menu);

	MenuRetainedState* prev = nullptr;
	{
		for (MenuRetainedState& state : _window->menu_prevs) {
			if (state.id == info.id) {
				prev = &state;
				break;
			}
		}

		if (prev == nullptr) {
			prev = &_window->menu_prevs.emplace_back();
		}

		prev->id = info.id;
		prev->used = true;

		prev->submenus.clear();
		prev->sections.clear();
		prev->items.clear();

		SubMenu& root_menu = prev->submenus.emplace_back();
		root_menu.child_sections.push_back(0);

		prev->sections.resize(1);
	}

	new_menu._calcNowState(prev, info);

	{
		SubMenu& root_submenu = prev->submenus[0];
		root_submenu.info.background_color = info.titles_background_color;
		root_submenu.info.border_thickness = info.titles_border_thickness;
		root_submenu.info.border_color = info.titles_border_color;
	}

	// Init
	new_menu.state = prev;

	return &new_menu;
}

//void Container::attachTreeList(TreeListCreateInfo& info, TreeList* tree_list)
//{
//	tree_list->_parent = _self->base_elem;
//	_children.push_back(tree_list->_self->base_elem);
//
//	tree_list->_calcNowState(&tree_list->base_elem_state, info);
//	tree_list->info = info;
//}
