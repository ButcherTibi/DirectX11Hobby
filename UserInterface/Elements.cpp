// Header
#include "NuiLibrary.hpp"


using namespace nui;


void EventsComponent::setMouseEnterEvent(EventCallback callback, void* user_data)
{
	this->_onMouseEnter = callback;
	this->_mouse_enter_user_data = user_data;
}

void EventsComponent::setMouseHoverEvent(EventCallback callback, void* user_data)
{
	this->_onMouseHover = callback;
	this->_mouse_hover_user_data = user_data;
}

void EventsComponent::setMouseMoveEvent(EventCallback callback, void* user_data)
{
	this->_onMouseMove = callback;
	this->_mouse_move_user_data = user_data;
}

void EventsComponent::setMouseScrollEvent(EventCallback callback, void* user_data)
{
	this->_onMouseScroll = callback;
	this->_mouse_scroll_user_data = user_data;
}

void EventsComponent::setMouseLeaveEvent(EventCallback callback, void* user_data)
{
	this->_onMouseLeave = callback;
	this->_mouse_leave_user_data = user_data;
}

void deleteKeyCallback(uint32_t key, std::vector<Shortcut1KeyCallback>& keys_1)
{
	uint32_t i = 0;
	for (Shortcut1KeyCallback& shortcut : keys_1) {

		if (shortcut.key == key) {

			keys_1.erase(keys_1.begin() + i);
			return;
		}
		i++;
	}
}

void delete2KeysCallbacks(uint32_t key_0, uint32_t key_1, std::vector<Shortcut2KeysCallback>& keys_2)
{
	uint32_t i = 0;
	for (Shortcut2KeysCallback& shortcut : keys_2) {

		if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1) {

			keys_2.erase(keys_2.begin() + i);
			return;
		}
		i++;
	}
}

void delete3KeysCallbacks(uint32_t key_0, uint32_t key_1, uint32_t key_2, std::vector<Shortcut3KeysCallback>& keys_3)
{
	uint32_t i = 0;
	for (Shortcut3KeysCallback& shortcut : keys_3) {

		if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1 && shortcut.key_2 == key_2) {

			keys_3.erase(keys_3.begin() + i);
			return;
		}
		i++;
	}
}

void EventsComponent::setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	if (callback == nullptr) {
		deleteKeyCallback(key, _key_downs);
	}
	else {
		for (Shortcut1KeyCallback& shortcut : _key_downs) {

			if (shortcut.key == key) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut1KeyCallback& new_shortcut = _key_downs.emplace_back();
		new_shortcut.key = key;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeysDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, void* user_data)
{
	if (callback == nullptr) {
		delete2KeysCallbacks(key_0, key_1, _keys_2_downs);
	}
	else {
		for (Shortcut2KeysCallback& shortcut : _keys_2_downs) {

			if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut2KeysCallback& new_shortcut = _keys_2_downs.emplace_back();
		new_shortcut.key_0 = key_0;
		new_shortcut.key_1 = key_1;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeysDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, uint32_t key_2, void* user_data)
{
	if (callback == nullptr) {
		delete3KeysCallbacks(key_0, key_1, key_2, _keys_3_downs);
	}
	else {
		for (Shortcut3KeysCallback& shortcut : _keys_3_downs) {

			if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1 && shortcut.key_2 == key_2) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut3KeysCallback& new_shortcut = _keys_3_downs.emplace_back();
		new_shortcut.key_0 = key_0;
		new_shortcut.key_1 = key_1;
		new_shortcut.key_2 = key_2;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeyHeldDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	if (callback == nullptr) {
		deleteKeyCallback(key, _key_held_downs);
	}
	else {
		for (Shortcut1KeyCallback& shortcut : _key_held_downs) {

			if (shortcut.key == key) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut1KeyCallback& new_shortcut = _key_held_downs.emplace_back();
		new_shortcut.key = key;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeysHeldDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, void* user_data)
{
	if (callback == nullptr) {
		delete2KeysCallbacks(key_0, key_1, _keys_2_held_downs);
	}
	else {
		for (Shortcut2KeysCallback& shortcut : _keys_2_held_downs) {

			if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut2KeysCallback& new_shortcut = _keys_2_held_downs.emplace_back();
		new_shortcut.key_0 = key_0;
		new_shortcut.key_1 = key_1;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeysHeldDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, uint32_t key_2, void* user_data)
{
	if (callback == nullptr) {
		delete3KeysCallbacks(key_0, key_1, key_2, _keys_3_held_downs);
	}
	else {
		for (Shortcut3KeysCallback& shortcut : _keys_3_held_downs) {

			if (shortcut.key_0 == key_0 && shortcut.key_1 == key_1 && shortcut.key_2 == key_2) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut3KeysCallback& new_shortcut = _keys_3_held_downs.emplace_back();
		new_shortcut.key_0 = key_0;
		new_shortcut.key_1 = key_1;
		new_shortcut.key_2 = key_2;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::setKeyUpEvent(EventCallback callback, uint32_t key, void* user_data)
{
	if (callback == nullptr) {
		deleteKeyCallback(key, _key_ups);
	}
	else {
		for (Shortcut1KeyCallback& shortcut : _key_ups) {

			if (shortcut.key == key) {

				shortcut.callback = callback;
				shortcut.user_data = user_data;
				return;
			}
		}

		Shortcut1KeyCallback& new_shortcut = _key_ups.emplace_back();
		new_shortcut.key = key;
		new_shortcut.callback = callback;
		new_shortcut.user_data = user_data;
	}
}

void EventsComponent::beginMouseLoopDeltaEffect(Element* elem)
{
	if (_window->delta_owner_elem != nullptr) {

		switch (_window->delta_effect) {
		case Window::DeltaEffectType::HIDDEN: {
			_window->setMouseVisibility(true);
		}
		}

		ClipCursor(nullptr);
	}

	_window->delta_effect = Window::DeltaEffectType::LOOP;
	_window->delta_owner_elem = elem;
}

void EventsComponent::beginMouseFixedDeltaEffect(Element* elem)
{
	if (_window->delta_owner_elem != nullptr) {

		switch (_window->delta_effect) {
		case Window::DeltaEffectType::HIDDEN: {
			_window->setMouseVisibility(true);
		}
		}

		ClipCursor(nullptr);
	}

	_window->delta_effect = Window::DeltaEffectType::HIDDEN;
	_window->delta_owner_elem = elem;

	nui::Input& input = _window->input;
	_window->begin_mouse_x = input.mouse_x;
	_window->begin_mouse_y = input.mouse_y;
}

float EventsComponent::getInsideDuration()
{
	if (nui::frame_start_time < _mouse_enter_time) {
		return 0;
	}

	auto now = nui::frame_start_time;

	return (float)std::chrono::duration_cast<std::chrono::milliseconds>(
		now - _mouse_enter_time).count();
}

void EventsComponent::_init(Window* window)
{
	_window = window;
	_mouse_event_state = MouseEventState::OUTSIDE;

	_onMouseEnter = nullptr;
	_onMouseHover = nullptr;
	_onMouseMove = nullptr;
	_onMouseScroll = nullptr;
	_onMouseLeave = nullptr;
}

void EventsComponent::_emitInsideEvents(StoredElement2* self)
{
	SteadyTime& now = nui::frame_start_time;

	// if mouse was previously outside
	if (_mouse_event_state == MouseEventState::OUTSIDE) {

		_mouse_enter_time = now;

		if (_onMouseEnter != nullptr) {
			this->_onMouseEnter(_window, self, _mouse_enter_user_data);
		}
	}

	if (_onMouseHover != nullptr) {
		this->_onMouseHover(_window, self, _mouse_hover_user_data);
	}

	if (_onMouseMove != nullptr) {
		if (_window->input.mouse_delta_x || _window->input.mouse_delta_y) {
			this->_onMouseMove(_window, self, _mouse_move_user_data);
		}
	}

	if (_onMouseScroll != nullptr) {
		if (_window->input.mouse_wheel_delta) {
			this->_onMouseScroll(_window, self, _mouse_scroll_user_data);
		}
	}

	// Keys Down
	for (Shortcut1KeyCallback& shortcut : _key_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key];

		if (key_0.is_down && key_0.down_transition) {
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	for (Shortcut2KeysCallback& shortcut : _keys_2_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key_0];
		KeyState& key_1 = _window->input.key_list[shortcut.key_1];

		if (key_0.is_down && key_1.is_down &&
			key_0.getDuration_ms() > key_1.getDuration_ms() &&
			key_1.down_transition)
		{
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	for (Shortcut3KeysCallback& shortcut : _keys_3_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key_0];
		KeyState& key_1 = _window->input.key_list[shortcut.key_1];
		KeyState& key_2 = _window->input.key_list[shortcut.key_2];

		/*printf("key_0 = %d %d \n", key_0.getDuration_ms(), key_0.is_down);
		printf("key_1 = %d %d \n", key_1.getDuration_ms(), key_1.is_down);
		printf("key_2 = %d %d \n", key_2.getDuration_ms(), key_2.is_down);*/

		if (key_0.is_down && key_1.is_down && key_2.is_down &&
			key_0.getDuration_ms() > key_1.getDuration_ms() && key_1.getDuration_ms() > key_2.getDuration_ms() &&
			key_2.down_transition)
		{
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	// Keys Held Down
	for (Shortcut1KeyCallback& shortcut : _key_held_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key];

		if (key_0.is_down) {
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	for (Shortcut2KeysCallback& shortcut : _keys_2_held_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key_0];
		KeyState& key_1 = _window->input.key_list[shortcut.key_1];

		if (key_0.is_down && key_1.is_down &&
			key_0.getDuration_ms() > key_1.getDuration_ms())
		{
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	for (Shortcut3KeysCallback& shortcut : _keys_3_held_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key_0];
		KeyState& key_1 = _window->input.key_list[shortcut.key_1];
		KeyState& key_2 = _window->input.key_list[shortcut.key_2];

		if (key_0.is_down && key_1.is_down && key_2.is_down &&
			key_0.getDuration_ms() > key_1.getDuration_ms() && key_1.getDuration_ms() > key_2.getDuration_ms())
		{
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	// Keys UP
	for (Shortcut1KeyCallback& shortcut : _key_ups) {

		KeyState& key = _window->input.key_list[shortcut.key];

		if (!key.is_down && key.up_transition) {
			shortcut.callback(_window, self, shortcut.user_data);
		}
	}

	_mouse_event_state = MouseEventState::INSIDE;
}

void EventsComponent::_emitOutsideEvents(StoredElement2* self)
{
	// was inside but now is outside
	if (_mouse_event_state == MouseEventState::INSIDE) {

		if (_onMouseLeave != nullptr) {
			this->_onMouseLeave(_window, self, _mouse_leave_user_data);
		}

		_mouse_event_state = MouseEventState::OUTSIDE;
	}

	// do nothing if already outside
}

void Element::_emitEvents(bool&)
{
	// no events
};

void Element::_calcSizeAndRelativeChildPositions()
{
	if (_children.size()) {

		auto set_size = [&](uint32_t axis) {
			if (size[axis].type == ElementSizeType::FIT) {
				_size[axis] = _children[0]->base_elem->_size[axis];
			}
		};
		set_size(0);
		set_size(1);
	}
};

void Element::_draw()
{
	// no rendering commands present
};

Element* nui::getElementBase(StoredElement* elem)
{
	switch (elem->index()) {
	case ElementType::ROOT:
		return std::get_if<Root>(elem);
	case ElementType::TEXT:
		return std::get_if<Text>(elem);
	case ElementType::RELATIVE:
		return std::get_if<Stack>(elem);
	case ElementType::FLEX:
		return std::get_if<Flex>(elem);
	case ElementType::MENU:
		return std::get_if<Menu>(elem);
	case ElementType::RECT:
		return std::get_if<Rect>(elem);
	default:
		throw std::exception();
	}

	return nullptr;
}

TextCreateInfo::TextCreateInfo()
{
	this->text = std::string();
	this->font_family = "Roboto";
	this->font_style = "Regular";
	this->font_size = 14;
	this->line_height = 0xFFFF'FFFF;

	this->color = AnimatedProperty<Color>(Color(1.f, 1.f, 1.f));
}

void Container::assign(Element& elem, ElementRetainedState* prev, ElementCreateInfo& next)
{
	for (uint32_t i = 0; i < 2; i++) {
		elem.size[i] = prev->size[i].calc(next.size[i]);
		elem.origin[i] = prev->origin[i].calc(next.origin[i]);
		elem.relative_position[i] = prev->relative_position[i].calc(next.relative_position[i]);
	}

	elem.z_index = next.z_index;
	elem.flex_grow = prev->flex_grow.calc(next.flex_grow);
}

void Container::createText(TextCreateInfo& info)
{
	StoredElement2& new_entry = _window->elements.emplace_back();

	Text& new_text = new_entry.specific_elem.emplace<Text>();
	new_text._window = _window;
	new_text._parent = _self;
	new_entry.base_elem = &new_text;
	new_text._self = &new_entry;

	TextRetainedState* prev = nullptr;
	{
		for (TextRetainedState& state : _window->text_prevs) {
			if (state.id == info.id) {
				prev = &state;
				break;
			}
		}

		if (prev == nullptr) {
			prev = &_window->text_prevs.emplace_back();
		}

		prev->id = info.id;
		prev->used = true;
	}
	
	assign(new_text, prev, info);

	new_text.text = info.text;
	new_text.font_family = info.font_family;
	new_text.font_style = info.font_style;

	new_text.font_size = prev->font_size.calc(info.font_size);
	new_text.line_height = prev->line_height.calc(info.line_height);
	new_text.color = prev->color.calc(info.color);

	_children.push_back(&new_entry);
}

Flex* Container::createFlex(FlexCreateInfo& info)
{
	StoredElement2& new_entry = _window->elements.emplace_back();

	Flex& new_flex = new_entry.specific_elem.emplace<Flex>();
	new_flex._window = _window;
	new_flex._parent = _self;
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

	assign(new_flex, prev, info);

	new_flex.orientation = info.orientation;
	new_flex.items_spacing = info.items_spacing;
	new_flex.lines_spacing = info.lines_spacing;

	_children.push_back(&new_entry);

	return &new_flex;
}

Rect* Container::createRect(RectCreateInfo& info)
{
	StoredElement2& new_entry = _window->elements.emplace_back();

	Rect& new_rect = new_entry.specific_elem.emplace<Rect>();
	new_rect._window = _window;
	new_rect._parent = _self;
	new_entry.base_elem = &new_rect;
	new_rect._self = &new_entry;

	RectRetainedState* prev = nullptr;
	{
		for (RectRetainedState& state : _window->rect_prevs) {
			if (state.id == info.id) {
				prev = &state;
				break;
			}
		}

		if (prev == nullptr) {
			prev = &_window->rect_prevs.emplace_back();
		}

		prev->id = info.id;
		prev->used = true;
	}

	assign(new_rect, prev, info);

	new_rect.color = prev->color.calc(info.color);

	_children.push_back(&new_entry);

	return &new_rect;
}

Menu* Container::createMenu(MenuCreateInfo& info)
{
	StoredElement2& new_entry = _window->elements.emplace_back();

	Menu& new_menu = new_entry.specific_elem.emplace<Menu>();
	new_menu._window = _window;
	new_menu._parent = _self;
	new_menu._self = &new_entry;

	new_entry.base_elem = &new_menu;
	_children.push_back(&new_entry);

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

	assign(new_menu, prev, info);

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

void Root::_emitEvents(bool&)
{
	_events._emitInsideEvents(_self);
}

// works the same as relative wrap but doesn't need to calc size
void Root::_calcSizeAndRelativeChildPositions()
{
	auto calc_size = [&](uint32_t axis) {

		// Calculate child positions
		for (StoredElement2* stored_child : _children) {

			Element* child = stored_child->base_elem;

			int32_t& _child_pos = child->_position[axis];

			// Child origin
			_child_pos = std::lroundf(child->_size[axis] * -(child->origin[axis] / 100.f));

			auto& child_relative_pos = child->relative_position[axis];

			switch (child_relative_pos.type) {
			case ElementPositionType::RELATIVE: {
				_child_pos += std::lroundf(_size[axis] * child_relative_pos.relative_pos);
				break;
			}
			case ElementPositionType::ABSOLUTE: {
				_child_pos += child_relative_pos.absolute_pos;
				break;
			}
			}
		}
	};

	calc_size(0);
	calc_size(1);
}

void Root::_draw()
{
	Instance* instance = _window->instance;

	std::array<float, 4> clear_color = { 0, 0, 0, 0 };
	instance->im_ctx3->ClearRenderTargetView(_window->present_rtv.Get(), clear_color.data());
}

void Text::_calcSizeAndRelativeChildPositions()
{
	TextProps new_inst;
	new_inst.text = text;
	new_inst.font_family = font_family;
	new_inst.font_style = font_style;
	new_inst.font_size = font_size;
	new_inst.line_height = line_height;
	new_inst.color = color;

	Instance* inst = _window->instance;
	inst->findAndPositionGlyphs(new_inst, 0, 0, _size[0], _size[1], instance.chars);

	instance.color = color;
}

void Text::_draw()
{
	Instance* inst = _window->instance;

	for (auto& pos_char : instance.chars) {
		pos_char.pos[0] += _position[0];
		pos_char.pos[1] += _position[1];
	}

	inst->drawTexts(_window, std::vector<TextInstance*>{ &instance });
}

//
//void Stack::_calcSizeAndRelativeChildPositions()
//{
//	auto calc_child_positions = [&](uint32_t axis) {
//
//		uint32_t extent = 0;
//
//		// Calculate child positions
//		for (Element* child : _children) {
//
//			int32_t& _child_pos = child->_position[axis];
//
//			// Child origin
//			_child_pos = std::lroundf(child->_size[axis] * -(child->origin[axis].now / 100.f));
//
//			auto& child_relative_pos = child->relative_position[axis].now;
//
//			switch (child_relative_pos.type) {
//			case ElementPositionType::RELATIVE: {
//				_child_pos += std::lroundf(_size[axis] * child_relative_pos.relative_pos);
//				break;
//			}
//			case ElementPositionType::ABSOLUTE: {
//				_child_pos += child_relative_pos.absolute_pos;
//				break;
//			}
//			}
//
//			// how much does a child size extend in a certain direction
//			int32_t child_extent = _child_pos + child->_size[axis];
//			if (child_extent > (int32_t)extent) {
//				extent = child_extent;
//			}
//		}
//
//		switch (size[axis].get().type) {
//		case ElementSizeType::FIT: {
//			_size[axis] = extent;
//			break;
//		}
//		}
//	};
//
//	calc_child_positions(0);
//	calc_child_positions(1);
//}
//
//void Rect::_updateProperties()
//{
//	Element::_updateProperties();
//
//	coloring._update();
//	color._update();
//}
//
//void Rect::_generateGPU_Data()
//{
//	switch (coloring.get()) {
//	case Coloring::FLAT: {
//
//		_rect_render.reset();
//
//		RectInstance props;
//		props.screen_pos[0] = _position[0];
//		props.screen_pos[1] = _position[1];
//		props.size[0] = _size[0];
//		props.size[1] = _size[1];
//		props.color = color.get();
//		_rect_render.addInstance(props);
//
//		_rect_render.generateGPU_Data();
//		break;
//	}
//	}
//}
//
void Rect::_draw()
{
	Instance* inst = _window->instance;

	RectInstance rect_inst;
	rect_inst.screen_pos = _position;
	rect_inst.size = _size;
	rect_inst.color = color;

	inst->drawRects(_window, std::vector<RectInstance*>{ &rect_inst });
}
//
//void BackgroundElement::setColorTransition(Color& end_color, uint32_t duration)
//{
//	auto& color_anim = _background_color;
//	color_anim.start = background_color.rgba;
//	color_anim.end = end_color.rgba;
//	color_anim.start_time = nui::frame_start_time;
//	color_anim.end_time = color_anim.start_time + std::chrono::milliseconds(duration);
//	color_anim.blend_func = TransitionBlendFunction::LINEAR;
//}
//
//void BackgroundElement::_init()
//{
//	Element::_initDefaultProperties();
//
//	coloring = BackgroundColoring::NONE;
//	background_color.setRGBA_UNORM();
//
//	_onRenderingSurface = nullptr;
//
//	_rect_render.init(_window);
//	_events._init(_window);
//}
//
//void BackgroundElement::_generateGPU_Data()
//{
//	switch (coloring) {
//	case BackgroundColoring::FLAT_FILL: {
//
//		SteadyTime& now = nui::frame_start_time;
//		background_color.rgba = _background_color.calculate(now);
//
//		_rect_render.reset();
//
//		RectInstance props;
//		props.screen_pos[0] = _position[0];
//		props.screen_pos[1] = _position[1];
//		props.size[0] = _size[0];
//		props.size[1] = _size[1];
//		props.color.rgba = background_color.rgba;
//		_rect_render.addInstance(props);
//
//		_rect_render.generateGPU_Data();
//		break;
//	}
//	}
//}
//
//void BackgroundElement::_draw()
//{
//	switch (coloring) {
//	case BackgroundColoring::FLAT_FILL: {		
//		_rect_render.draw();
//		break;
//	}
//
//	case BackgroundColoring::RENDERING_SURFACE: {
//
//		assert_cond(_onRenderingSurface != nullptr,
//			"RenderingSurface callback not set for BackgroundColoring::RENDERING_SURFACE");
//
//		Instance* instance = _window->instance;
//		ID3D11DeviceContext3* im_ctx3 = instance->im_ctx3.Get();
//
//		//auto clear_bindings = [&]() {
//
//		//	// Input Assembly
//		//	{
//		//		std::array<ID3D11Buffer*, 2> buffs = {
//		//			nullptr, nullptr
//		//		};
//		//		std::array<uint32_t, 2> strides = {
//		//			0, 0
//		//		};
//		//		std::array<uint32_t, 2> offsets = {
//		//			0, 0
//		//		};
//		//		im_ctx3->IASetVertexBuffers(0, buffs.size(), buffs.data(), strides.data(), offsets.data());
//		//	}
//		//};
//
//		im_ctx3->ClearState();
//
//		SurfaceEvent surface_event;
//		surface_event.dev5 = instance->dev5.Get();
//		surface_event.im_ctx3 = im_ctx3;
//
//		surface_event.render_target_width = _window->surface_width;
//		surface_event.render_target_height = _window->surface_height;
//		surface_event.compose_rtv = _window->present_rtv.Get();
//
//		surface_event.viewport_pos = { _position[0], _position[1] };
//		surface_event.viewport_size = { _size[0], _size[1] };
//
//		this->_onRenderingSurface(_window, &(*_self_element), surface_event, _surface_event_user_data);
//
//		im_ctx3->ClearState();
//		break;
//	}
//	}
//}
//
//void BackgroundElement::setRenderingSurfaceEvent(RenderingSurfaceCallback callback, void* user_data)
//{
//	this->_onRenderingSurface = callback;
//	this->_surface_event_user_data = user_data;
//}

void Flex::_emitEvents(bool& allow_inside_events)
{
	if (allow_inside_events) {

		int32_t top = _position[1];
		int32_t bot = top + _size[1];
		int32_t left = _position[0];
		int32_t right = left + _size[0];

		Input& input = _window->input;

		if (left <= input.mouse_x && input.mouse_x < right &&
			top <= input.mouse_y && input.mouse_y < bot)
		{
			_events._emitInsideEvents(_self);
			allow_inside_events = false;
		}
		else {
			_events._emitOutsideEvents(_self);
			allow_inside_events = true;
		}
	}
	else {
		_events._emitOutsideEvents(_self);
		allow_inside_events = true;
	}
}

void Flex::_calcSizeAndRelativeChildPositions()
{
	auto calc_child_positions = [&](uint32_t x_axis, uint32_t y_axis) {

		struct GridLine {
			uint32_t start_idx;
			uint32_t count;

			uint32_t length;
			uint32_t thickness;

			float flex_grow_sum;
		};

		std::vector<GridLine> lines;

		// Group items into lines
		uint32_t fit_width = 0;
		uint32_t fit_height = 0;
		{
			uint32_t line_max_length;

			switch (size[x_axis].type) {
			case ElementSizeType::ABSOLUTE:
			case ElementSizeType::RELATIVE: {
				line_max_length = _size[x_axis];
				break;
			}

			case ElementSizeType::FIT: {
				line_max_length = 0xFFFF'FFFF;
				break;
			}
			default:
				throw std::exception();
			}

			GridLine* line = &lines.emplace_back();
			line->start_idx = 0;
			line->count = 0;
			line->length = 0;
			line->thickness = 0;
			line->flex_grow_sum = 0;

			for (StoredElement2* stored_child : _children) {

				Element* child = stored_child->base_elem;
				uint32_t child_length = child->_size[x_axis];
				uint32_t child_thickness = child->_size[y_axis];

				if ((line->length + child_length < line_max_length)) {

					line->length += child_length;

					if (child_thickness > line->thickness) {
						line->thickness = child_thickness;
					}

					line->count++;
					line->flex_grow_sum += child->flex_grow;
				}
				// element does not fit in current line so place to next
				else {
					uint32_t start_idx = line->start_idx + line->count;

					line = &lines.emplace_back();
					line->start_idx = start_idx;
					line->count = 1;
					line->length = child_length;
					line->thickness = child_thickness;
					line->flex_grow_sum = child->flex_grow;

					fit_height += line->thickness;
				}

				if (line->length > (int32_t)fit_width) {
					fit_width = line->length;
				}
			}

			fit_height += lines.front().thickness;
		}

		if (size[x_axis].type == ElementSizeType::FIT) {
			_size[x_axis] = fit_width;
		}

		if (size[y_axis].type == ElementSizeType::FIT) {
			_size[y_axis] = fit_height;
		}

		// Calculate positions of items
		{
			uint32_t y_step_space = _size[y_axis] - fit_height;

			int32_t y;
			uint32_t y_step = 0;

			switch (lines_spacing) {
			case FlexSpacing::START: {
				y = 0;
				break;
			}

			case FlexSpacing::END: {
				y = y_step_space;
				break;
			}

			case FlexSpacing::CENTER: {
				y = y_step_space / 2;
				break;
			}

			case FlexSpacing::SPACE_BETWEEN: {
				y = 0;
				y_step = y_step_space / (lines.size() - 1);
				break;
			}
			default:
				throw std::exception();
			}

			for (GridLine& line : lines) {

				uint32_t x_grow_size;
				uint32_t x_step_space;

				if (line.flex_grow_sum >= 1) {
					x_grow_size = _size[x_axis] - line.length;
					x_step_space = 0;
				}
				else {
					uint32_t remainder = _size[x_axis] - line.length;

					x_grow_size = (uint32_t)(remainder * line.flex_grow_sum);
					x_step_space = remainder - x_grow_size;
				}

				int32_t x;
				int32_t x_step = 0;

				switch (items_spacing) {
				case FlexSpacing::START: {
					x = 0;
					break;
				}

				case FlexSpacing::END: {
					x = x_step_space;
					break;
				}

				case FlexSpacing::CENTER: {
					x = x_step_space / 2;
					break;
				}

				case FlexSpacing::SPACE_BETWEEN: {
					x = 0;
					if (line.count > 1) {
						x_step = x_step_space / (line.count - 1);
					}
					else {
						x = x_step_space / 2;
					}
					break;
				}
				default:
					throw std::exception();
				}

				uint32_t end_idx = line.start_idx + line.count;
				for (uint32_t child_idx = line.start_idx; child_idx < end_idx; child_idx++) {

					Element* child = _children[child_idx]->base_elem;
					uint32_t& child_length = child->_size[x_axis];

					child->_position[x_axis] = x;
					child->_position[y_axis] = y;

					if (line.flex_grow_sum) {
						float adjusted_grow_factor = child->flex_grow / line.flex_grow_sum;
						child_length += (uint32_t)(x_grow_size * adjusted_grow_factor);
					}

					x += child_length + x_step;
				}

				y += line.thickness + y_step;
			}
		}
	};

	switch (orientation) {
	case FlexOrientation::ROW: {
		calc_child_positions(0, 1);
		break;
	}

	case FlexOrientation::COLUMN: {
		calc_child_positions(1, 0);
		break;
	}
	}
}

void Flex::setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	_events.setKeyDownEvent(callback, key, user_data);
}

void Flex::setKeyHeldDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	_events.setKeyHeldDownEvent(callback, key, user_data);
}

void Flex::setKeyUpEvent(EventCallback callback, uint32_t key, void* user_data)
{
	_events.setKeyUpEvent(callback, key, user_data);
}

void Flex::setMouseMoveEvent(EventCallback callback, void* user_data)
{
	_events.setMouseMoveEvent(callback, user_data);
}

void Flex::setMouseScrollEvent(EventCallback callback, void* user_data)
{
	_events.setMouseScrollEvent(callback, user_data);
}

void Flex::beginMouseLoopDeltaEffect()
{
	_events.beginMouseLoopDeltaEffect(this);
}

void Flex::beginMouseFixedDeltaEffect()
{
	_events.beginMouseFixedDeltaEffect(this);
}

SubMenuCreateInfo::SubMenuCreateInfo(Color& new_background_color)
{
	this->background_color = new_background_color;
}

uint32_t Menu::createSection(uint32_t parent_submenu_idx, MenuSectionCreateInfo& new_info)
{
	SubMenu& parent_submenu = state->submenus[parent_submenu_idx];
	parent_submenu.child_sections.push_back(state->sections.size());

	MenuSection& new_section = state->sections.emplace_back();
	new_section.info = new_info;

	return state->sections.size() - 1;
}

uint32_t Menu::createItem(uint32_t parent_section_idx, MenuItemCreateInfo& new_info)
{
	MenuSection& parent_section = state->sections[parent_section_idx];
	parent_section.child_items.push_back(state->items.size());

	MenuItem& new_item = state->items.emplace_back();
	new_item.sub_menu = 0xFFFF'FFFF;
	new_item.info = new_info;

	return state->items.size() - 1;
}

uint32_t Menu::createSubMenu(uint32_t parent_item_idx, SubMenuCreateInfo& new_info)
{
	MenuItem& parent_item = state->items[parent_item_idx];
	parent_item.sub_menu = state->submenus.size();

	SubMenu& new_submenu = state->submenus.emplace_back();
	new_submenu.info = new_info;

	return state->submenus.size() - 1;
}

uint32_t Menu::createTitle(MenuItemCreateInfo& info)
{
	return createItem(0, info);
}

void Menu::_emitEvents(bool& allow_inside_events)
{
	Input& input = _window->input;

	allow_inside_events = false;

	auto f = [&]() -> void {

		for (int32_t i = state->visible_menus.size() - 1; i >= 0; i--) {

			SubMenu& visible_menu = state->submenus[state->visible_menus[i].menu];

			if (visible_menu.box.isInside(input.mouse_x, input.mouse_y)) {

				for (uint32_t section_idx : visible_menu.child_sections) {

					MenuSection& section = state->sections[section_idx];

					for (uint32_t item_idx : section.child_items) {

						MenuItem& item = state->items[item_idx];

						if (item.box.isInside(input.mouse_x, input.mouse_y)) {

							// mark item to be hovered
							state->visible_menus[i].item = item_idx;

							// if item leads to submenu that needs to be displayed
							if (item.sub_menu != 0xFFFF'FFFF) {

								if (i == 0) {

									// if a submenu is already open
									if (state->visible_menus.size() > 1) {

										// hide title submenu if reclicked the same
										if (state->visible_menus[i].item == item_idx &&
											input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].down_transition) {

											state->visible_menus.resize(1);
											state->visible_menus[0].menu = 0;
											state->visible_menus[0].item = item_idx;
										}
										// show another one without requiring clicking
										else {
											state->visible_menus.resize(i + 2);
											state->visible_menus[i + 1].menu = item.sub_menu;
											state->visible_menus[i + 1].item = item_idx;
										}
									}
									// display submenu if clicked
									else if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].down_transition) {

										state->visible_menus.resize(i + 2);
										state->visible_menus[i + 1].menu = item.sub_menu;
										state->visible_menus[i + 1].item = item_idx;
									}
								}
								// display submenu if hovered
								else {
									state->visible_menus.resize(i + 2);
									state->visible_menus[i + 1].menu = item.sub_menu;
									state->visible_menus[i + 1].item = item_idx;
								}
							}
							// if item is endpoint
							else {
								// try to call item
								if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].down_transition) {

									if (item.info.callback != nullptr) {

										item.info.callback(_window, _self, nullptr);

										// hide menu
										state->visible_menus.resize(1);
										state->visible_menus[0].menu = 0;
										state->visible_menus[0].item = 0xFFFF'FFFF;
									}
								}
								// hide other submenu if hovering item
								else {
									state->visible_menus.resize(i + 1);
								}
							}
							return;
						}
					}
				}
				return;
			}
		}

		// cursor is not over menu at all

		// close menu because user is interacting with something else
		if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].down_transition) {

			state->visible_menus.resize(1);
			state->visible_menus[0].menu = 0;
			state->visible_menus[0].item = 0xFFFF'FFFF;

			allow_inside_events = true;
		}
		// stop highlighting if out of bounds
		else {
			state->visible_menus.back().item = 0xFFFF'FFFF;
		}
		// else keep displaing menu
	};
	f();
}

void Menu::_calcSizeAndRelativeChildPositions()
{
	Instance* inst = _window->instance;

	// Diff the prev and current frame graph to see if they changed
	// if they did then reset visible state
	// else preserve for emit events
	{
		auto is_changed = [&]() -> bool {

			if (state->prev_submenus.size() != state->submenus.size()) {
				return true;
			}

			if (state->prev_sections.size() != state->sections.size()) {
				return true;
			}

			if (state->prev_items.size() != state->items.size()) {
				return true;
			}

			for (uint32_t i = 0; i < state->submenus.size(); i++) {

				SubMenu& prev_submenu = state->prev_submenus[i];
				SubMenu& submenu = state->submenus[i];

				if (prev_submenu.child_sections != submenu.child_sections) {
					return true;
				}
			}

			for (uint32_t i = 0; i < state->sections.size(); i++) {

				MenuSection& prev_section = state->prev_sections[i];
				MenuSection& section = state->sections[i];

				if (prev_section.child_items != section.child_items) {
					return true;
				}
			}

			for (uint32_t i = 0; i < state->items.size(); i++) {

				MenuItem& prev_item = state->prev_items[i];
				MenuItem& item = state->items[i];

				if (prev_item.info.left_text.text != item.info.left_text.text) {
					return true;
				}
			}

			return false;
		};

		if (is_changed()) {

			state->visible_menus.resize(1);

			MenuVisibleMenus& visible = state->visible_menus[0];
			visible.menu = 0;
			visible.item = 0xFFFF'FFFF;
		}

		state->prev_submenus = state->submenus;
		state->prev_sections = state->sections;
		state->prev_items = state->items;
	}

	// Menu Titles
	int32_t pen_x = 0;
	int32_t pen_y = 0;
	{
		SubMenu& root_submenu = state->submenus[0];
		root_submenu.box = {};

		for (uint32_t section_idx : root_submenu.child_sections) {

			MenuSection& section = state->sections[section_idx];

			for (uint32_t item_idx : section.child_items) {

				MenuItem& item = state->items[item_idx];

				item.box.pos = { pen_x, pen_y };

				inst->findAndPositionGlyphs(
					item.info.left_text,
					(int32_t)item.info.left_padding + pen_x, (int32_t)item.info.top_padding + pen_y,
					item.box.size[0], item.box.size[1],
					item._text.chars
				);

				item._text.color = item.info.left_text.color;

				item.box.size[0] += item.info.left_padding + item.info.right_padding;
				item.box.size[1] += item.info.top_padding + item.info.bot_padding;

				pen_x += item.box.size[0];

				// menu height
				if (item.box.size[1] > root_submenu.box.size[1]) {
					root_submenu.box.size[1] = item.box.size[1];
				}
			}
		}

		root_submenu.box.size[0] = pen_x;
	}

	// Submenus
	if (state->visible_menus.size() > 1) {

		for (uint32_t visible_idx = 1; visible_idx < state->visible_menus.size(); visible_idx++) {

			MenuVisibleMenus& visible = state->visible_menus[visible_idx];

			MenuItem* parent_item;
			{
				MenuVisibleMenus& prev_visible = state->visible_menus[visible_idx - 1];
				parent_item = &state->items[prev_visible.item];
			}

			SubMenu& submenu = state->submenus[visible.menu];
			uint32_t border_thick = submenu.info.border_thickness;

			if (visible_idx == 1) {
				pen_x = parent_item->box.pos[0];
				pen_y = state->submenus[0].box.size[1];

				submenu.box.pos = { pen_x, pen_y };

				pen_y += border_thick;
			}
			else {
				pen_x = parent_item->box.pos[0] + parent_item->box.size[0] - border_thick;
				pen_y = parent_item->box.pos[1];

				submenu.box.pos = { pen_x, pen_y - (int32_t)border_thick };
			}
			
			uint32_t max_width = 0;

			// First pass to find out text size
			for (uint32_t section_idx : submenu.child_sections) {

				MenuSection& section = state->sections[section_idx];

				for (uint32_t item_idx : section.child_items) {

					MenuItem& item = state->items[item_idx];
					MenuItemCreateInfo& info = item.info;

					item.box.pos = { pen_x, pen_y };

					uint32_t text_width;
					uint32_t text_height;

					// Text
					{
						inst->findAndPositionGlyphs(
							item.info.left_text,
							pen_x + (int32_t)(border_thick + item.info.left_padding),
							pen_y + (int32_t)(item.info.top_padding),
							text_width, text_height,
							item._text.chars
						);

						item._text.color = item.info.left_text.color;
					}

					// Item Size
					uint32_t item_height = info.top_padding + text_height + info.bot_padding;
					uint32_t used_item_width =
						border_thick +
						info.left_padding + text_width + info.arrow_left_padding +
						info.arrow_width + info.right_padding +
						border_thick;
					
					// Arrow
					{
						item._arrow.screen_pos[1] = pen_y + (item_height - info.arrow_height) / 2;
						item._arrow.size = { info.arrow_width, info.arrow_height };

						if (item_idx == visible.item) {
							item._arrow.color = info.arrow_highlight_color;
						}
						else {
							item._arrow.color = info.arrow_color;
						}
					}

					// item.box.size[0] = we cannot know how large item will be on first pass
					item.box.size[1] = item_height;

					if (used_item_width > max_width) {
						max_width = used_item_width;
					}

					pen_y += item_height;
				}
			}

			// make sure submenu is as wide as parent item
			if (max_width < parent_item->box.size[0]) {
				max_width = parent_item->box.size[0];
			}

			// Second pass
			for (uint32_t section_idx : submenu.child_sections) {

				MenuSection& section = state->sections[section_idx];

				for (uint32_t item_idx : section.child_items) {

					MenuItem& item = state->items[item_idx];
					MenuItemCreateInfo& info = item.info;

					item.box.size[0] = max_width;

					// Arrow
					item._arrow.screen_pos[0] = pen_x +
						(max_width - border_thick - info.right_padding - info.arrow_width);
				}
			}

			submenu.box.size[0] = max_width;
			submenu.box.size[1] = pen_y - submenu.box.pos[1] + border_thick;

			pen_x += submenu.box.size[0];
		}
	}
}

void Menu::_draw()
{
	Instance* inst = _window->instance;

	// Menu titles
	{
		SubMenu& root_submenu = state->submenus[0];
		{
			RectInstance rect_instance;
			rect_instance.screen_pos = _position;
			rect_instance.size = root_submenu.box.size;
			rect_instance.color = root_submenu.info.background_color;

			inst->drawRects(_window, std::vector<RectInstance*>{ &rect_instance });
		}	

		state->text_instances.clear();

		for (uint32_t section_idx : root_submenu.child_sections) {

			MenuSection& section = state->sections[section_idx];

			for (uint32_t item_idx : section.child_items) {

				MenuItem& item = state->items[item_idx];

				// update box position to screen
				item.box.pos[0] += _position[0];
				item.box.pos[1] += _position[1];

				if (item_idx == state->visible_menus[0].item) {

					RectInstance rect_instance;
					rect_instance.screen_pos = item.box.pos;
					rect_instance.size = item.box.size;
					rect_instance.color = item.info.highlight_color;

					inst->drawRects(_window, std::vector<RectInstance*>{ &rect_instance });

					if (state->visible_menus.size() > 1) {

						BorderInstance border_inst;
						border_inst.screen_pos = item.box.pos;
						border_inst.size = item.box.size;
						border_inst.thickness = root_submenu.info.border_thickness;
						border_inst.color = root_submenu.info.border_color;
						border_inst.bot = false;

						inst->drawBorder(_window, std::vector<BorderInstance*>{ &border_inst });
					}
				}

				// update character positions to screen
				for (PositionedCharacter& chara : item._text.chars) {

					chara.pos[0] += _position[0];
					chara.pos[1] += _position[1];
				}

				state->text_instances.push_back(&item._text);
			}
		}

		inst->drawTexts(_window, state->text_instances);
	}

	// Submenu
	{
		for (uint32_t visible_idx = 1; visible_idx < state->visible_menus.size(); visible_idx++) {

			MenuVisibleMenus& visible = state->visible_menus[visible_idx];

			SubMenu& submenu = state->submenus[visible.menu];
			int32_t border_thick = submenu.info.border_thickness;

			// Menu background
			{
				RectInstance rect_instance;
				rect_instance.screen_pos = { 
					submenu.box.pos[0],
					submenu.box.pos[1]
				};
				rect_instance.size = {
					submenu.box.size[0],
					submenu.box.size[1]
				};
				rect_instance.color = submenu.info.background_color;

				inst->drawRects(_window, std::vector<RectInstance*>{ &rect_instance });
			}

			// Menu border
			{
				if (visible_idx == 1) {

					BorderInstance border_inst;
					border_inst.screen_pos = {
						submenu.box.pos[0],
						submenu.box.pos[1]
					};
					border_inst.size = {
						submenu.box.size[0],
						submenu.box.size[1]
					};
					border_inst.thickness = border_thick;
					border_inst.color = submenu.info.border_color;
					border_inst.top = false;

					inst->drawBorder(_window, std::vector<BorderInstance*>{ &border_inst });
				}
				else {
					BorderInstance border_inst;
					border_inst.screen_pos = {
						submenu.box.pos[0],
						submenu.box.pos[1]
					};
					border_inst.size = {
						submenu.box.size[0],
						submenu.box.size[1]
					};
					border_inst.thickness = border_thick;
					border_inst.color = submenu.info.border_color;

					inst->drawBorder(_window, std::vector<BorderInstance*>{ &border_inst });
				}
			}

			state->text_instances.clear();
			state->arrow_instances.clear();

			for (uint32_t section_idx : submenu.child_sections) {

				MenuSection& section = state->sections[section_idx];

				for (uint32_t item_idx : section.child_items) {

					MenuItem& item = state->items[item_idx];

					// update box position to screen
					item.box.pos[0] += _position[0];
					item.box.pos[1] += _position[1];

					// Highlight
					if (item_idx == visible.item) {

						RectInstance rect_instance;
						rect_instance.screen_pos = {
							item.box.pos[0] + border_thick,
							item.box.pos[1]
						};
						rect_instance.size = {
							item.box.size[0] - (2 * border_thick),
							item.box.size[1]
						};
						rect_instance.color = item.info.highlight_color;

						inst->drawRects(_window, std::vector<RectInstance*>{ &rect_instance });
					}

					// Text
					for (PositionedCharacter& chara : item._text.chars) {

						chara.pos[0] += _position[0];
						chara.pos[1] += _position[1];
					}

					state->text_instances.push_back(&item._text);

					// Arrow
					if (item.sub_menu != 0xFFFF'FFFF) {

						item._arrow.screen_pos[0] += _position[0];
						item._arrow.screen_pos[1] += _position[1];

						state->arrow_instances.push_back(&item._arrow);
					}
				}
			}

			inst->drawTexts(_window, state->text_instances);
			inst->drawArrows(_window, state->arrow_instances);
		}
	}
}
