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

void EventsComponent::beginMouseDelta(Element* elem)
{
	_window->delta_owner_elem = elem;
}

float EventsComponent::getInsideDuration()
{
	if (_window->instance->frame_start_time < _mouse_enter_time) {
		return 0;
	}

	auto now = _window->instance->frame_start_time;

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

void EventsComponent::_emitInsideEvents(StoredElement* self)
{
	SteadyTime& now = _window->instance->frame_start_time;

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

void EventsComponent::_emitOutsideEvents(StoredElement* self)
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

void Element::_init()
{
	for (uint8_t i = 0; i < 2; i++) {
		origin[i] = 0.f;
		relative_position[i] = 0;
		size[i].type = ElementSizeType::FIT;
	}

	z_index = 0;
}

void Element::_calcSizeRelativeToParent()
{
	auto calc_size_for_axis = [&](uint32_t axis) {
		switch (size[axis].type) {
		case ElementSizeType::RELATIVE: {
			_size[axis] = std::lroundf(_parent->_size[axis] * size[axis].relative_size);
			break;
		}

		case ElementSizeType::ABSOLUTE: {
			_size[axis] = size[axis].absolute_size;
			break;
		}

		case ElementSizeType::FIT: {
			// size cannot be calculated at this pass
			_size[axis] = 0;
			break;
		}
		}
	};

	calc_size_for_axis(0);
	calc_size_for_axis(1);
}

#pragma warning(disable : 4100)
void Element::_emitEvents(bool allow_inside_events, std::vector<EventsPassedElement>& r_next_children)
{
	for (Element* child : _children) {
		EventsPassedElement& passed_child = r_next_children.emplace_back();
		passed_child.allow_inside_event = allow_inside_events;
		passed_child.elem = child;
	}
};

void Element::_calcSizeAndRelativeChildPositions(std::unordered_set<Element*>& r_next_parents)
{
	// assume size is relative or absolute so it's already calculated
	// and don't alter child positions

	r_next_parents.insert(_parent);
};

void Element::_generateGPU_Data()
{
	// no resource is used
};

void Element::_draw()
{
	// no rendering commands present
};
#pragma warning(default : 4100)

Element* nui::getElementBase(StoredElement* elem)
{
	switch (elem->index()) {
	case ElementType::ROOT:
		return std::get_if<Root>(elem);
	case ElementType::TEXT:
		return std::get_if<Text>(elem);
	case ElementType::RELATIVE_WRAP:
		return std::get_if<RelativeWrap>(elem);
	case ElementType::GRID:
		return std::get_if<Grid>(elem);
	case ElementType::MENU:
		return std::get_if<Menu>(elem);
	}

	return nullptr;
}

void Root::_emitEvents(bool allow_inside_events, std::vector<EventsPassedElement>& r_next_children)
{
	_events._emitInsideEvents(_self);

	for (Element* child : _children) {
		EventsPassedElement& passed_child = r_next_children.emplace_back();
		passed_child.allow_inside_event = allow_inside_events;
		passed_child.elem = child;
	}
}

#pragma warning(disable : 4100)
void Root::_calcSizeAndRelativeChildPositions(std::unordered_set<Element*>& r_next_parents)
{
	auto calc_size = [&](uint32_t axis) {

		// Calculate child positions
		for (Element* child : _children) {

			int32_t& _child_pos = child->_position[axis];

			// Child origin
			_child_pos = std::lroundf(child->_size[axis] * -(child->origin[axis] / 100.f));

			switch (child->relative_position[axis].type) {
			case ElementPositionType::RELATIVE: {
				_child_pos += std::lroundf(_size[axis] * child->relative_position[axis].relative_pos);
				break;
			}
			case ElementPositionType::ABSOLUTE: {
				_child_pos += child->relative_position[axis].absolute_pos;
				break;
			}
			}
		}
	};

	calc_size(0);
	calc_size(1);

	// Root element has no parents
}
#pragma warning(default : 4100)

void RelativeWrap::_calcSizeAndRelativeChildPositions(std::unordered_set<Element*>& r_next_parents)
{
	auto calc_child_positions = [&](uint32_t axis) {

		uint32_t extent = 0;

		// Calculate child positions
		for (Element* child : _children) {

			int32_t& _child_pos = child->_position[axis];

			// Child origin
			_child_pos = std::lroundf(child->_size[axis] * -(child->origin[axis] / 100.f));

			switch (child->relative_position[axis].type) {
			case ElementPositionType::RELATIVE: {
				_child_pos += std::lroundf(_size[axis] * child->relative_position[axis].relative_pos);
				break;
			}
			case ElementPositionType::ABSOLUTE: {
				_child_pos += child->relative_position[axis].absolute_pos;
				break;
			}
			}

			// how much does a child size extend in a certain direction
			int32_t child_extent = _child_pos + child->_size[axis];
			if (child_extent > (int32_t)extent) {
				extent = child_extent;
			}
		}

		switch (size[axis].type) {
		case ElementSizeType::FIT: {
			_size[axis] = extent;
			break;
		}
		}
	};

	calc_child_positions(0);
	calc_child_positions(1);

	r_next_parents.insert(_parent);
}

void BackgroundElement::setColorTransition(Color& end_color, uint32_t duration)
{
	auto& color_anim = _background_color;
	color_anim.start = background_color.rgba;
	color_anim.end = end_color.rgba;
	color_anim.start_time = _window->instance->frame_start_time;
	color_anim.end_time = color_anim.start_time + std::chrono::milliseconds(duration);
	color_anim.blend_func = TransitionBlendFunction::LINEAR;
}

void BackgroundElement::_init()
{
	Element::_init();

	coloring = BackgroundColoring::NONE;
	background_color.setRGBA_UNORM();

	_onRenderingSurface = nullptr;

	_rect_render.init(_window);
	_events._init(_window);
}

void BackgroundElement::_generateGPU_Data()
{
	switch (coloring) {
	case BackgroundColoring::FLAT_FILL: {

		SteadyTime& now = _window->instance->frame_start_time;
		background_color.rgba = _background_color.calculate(now);

		_rect_render.reset();

		RectInstance props;
		props.screen_pos[0] = _position[0];
		props.screen_pos[1] = _position[1];
		props.size[0] = _size[0];
		props.size[1] = _size[1];
		props.color.rgba = background_color.rgba;
		_rect_render.addInstance(props);

		_rect_render.generateGPU_Data();
		break;
	}
	}
}

void BackgroundElement::_draw()
{
	switch (coloring) {
	case BackgroundColoring::FLAT_FILL: {		
		_rect_render.draw();
		break;
	}

	case BackgroundColoring::RENDERING_SURFACE: {

		assert_cond(_onRenderingSurface != nullptr,
			"RenderingSurface callback not set for BackgroundColoring::RENDERING_SURFACE");

		Instance* instance = _window->instance;
		ID3D11DeviceContext3* im_ctx3 = instance->im_ctx3.Get();

		//auto clear_bindings = [&]() {

		//	// Input Assembly
		//	{
		//		std::array<ID3D11Buffer*, 2> buffs = {
		//			nullptr, nullptr
		//		};
		//		std::array<uint32_t, 2> strides = {
		//			0, 0
		//		};
		//		std::array<uint32_t, 2> offsets = {
		//			0, 0
		//		};
		//		im_ctx3->IASetVertexBuffers(0, buffs.size(), buffs.data(), strides.data(), offsets.data());
		//	}
		//};

		im_ctx3->ClearState();

		SurfaceEvent surface_event;
		surface_event.dev5 = instance->dev5.Get();
		surface_event.im_ctx3 = im_ctx3;

		surface_event.render_target_width = _window->surface_width;
		surface_event.render_target_height = _window->surface_height;
		surface_event.compose_rtv = _window->present_rtv.Get();

		surface_event.viewport_pos = { _position[0], _position[1] };
		surface_event.viewport_size = { _size[0], _size[1] };

		this->_onRenderingSurface(_window, _self, surface_event, _surface_event_user_data);

		im_ctx3->ClearState();
		break;
	}
	}
}

void BackgroundElement::setRenderingSurfaceEvent(RenderingSurfaceCallback callback, void* user_data)
{
	this->_onRenderingSurface = callback;
	this->_surface_event_user_data = user_data;
}

void Grid::_emitEvents(bool allow_inside_events, std::vector<EventsPassedElement>& r_next_children)
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

	for (Element* child : _children) {

		EventsPassedElement& passed_child = r_next_children.emplace_back();
		passed_child.allow_inside_event = allow_inside_events;
		passed_child.elem = child;
	}
}

void Grid::_calcSizeAndRelativeChildPositions(std::unordered_set<Element*>& r_next_parents)
{
	auto calc_child_positions = [&](uint32_t x_axis, uint32_t y_axis) {

		struct GridLine {
			uint32_t end_idx;
			uint32_t count;

			uint32_t line_length;
			uint32_t line_thickness;
		};

		std::vector<GridLine> lines;

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

		// Group items into lines
		uint32_t width = 0;
		uint32_t height = 0;
		{
			int32_t x = 0;
			uint32_t line_thickness = 0;
			uint32_t child_idx = 0;
			uint32_t child_count = 0;

			for (Element* child : _children) {

				if ((x + child->_size[x_axis] < line_max_length) ||
					child == _children.front())
				{
					x += child->_size[x_axis];

					if (child->_size[y_axis] > line_thickness) {
						line_thickness = child->_size[y_axis];
					}
				}
				// element does not fit in current line so place to next
				else {
					GridLine& new_line = lines.emplace_back();
					new_line.end_idx = child_idx;
					new_line.count = child_count;
					new_line.line_length = x;
					new_line.line_thickness = line_thickness;

					height += line_thickness;

					x = child->_size[x_axis];
					line_thickness = child->_size[y_axis];

					child_count = 0;
				}

				if (x > (int32_t)width) {
					width = x;
				}

				child_idx++;
				child_count++;
			}

			GridLine& new_line = lines.emplace_back();
			new_line.end_idx = child_idx;
			new_line.count = child_count;
			new_line.line_length = x;
			new_line.line_thickness = line_thickness;

			height += line_thickness;
		}

		// Calculate X positions of items
		{
			int32_t x;
			int32_t step = 0;

			switch (items_spacing) {
			case GridSpacing::START: {
				x = 0;
				break;
			}

			case GridSpacing::END: {
				x = _size[x_axis] - lines.front().line_length;
				break;
			}

			case GridSpacing::CENTER: {
				x = (_size[x_axis] - lines.front().line_length) / 2;
				break;
			}

			case GridSpacing::SPACE_BETWEEN: {

				x = 0;
				GridLine& first_line = lines.front();
				if (first_line.count > 1) {
					step = (_size[x_axis] - first_line.line_length) / (first_line.count - 1);
				}
				break;
			}
			default:
				throw std::exception();
			}

			uint32_t item_idx = 0;
			uint32_t line_idx = 0;

			for (Element* child : _children) {

				GridLine* line = &lines[line_idx];

				if (item_idx < line->end_idx) {
					child->_position[x_axis] = x;
					x += child->_size[x_axis] + step;
				}
				else {
					switch (items_spacing) {
					case GridSpacing::START: {
						x = 0;
						break;
					}

					case GridSpacing::END: {
						x = _size[x_axis] - line->line_length;
						break;
					}

					case GridSpacing::CENTER: {
						x = (_size[x_axis] - line->line_length) / 2;
						break;
					}

					case GridSpacing::SPACE_BETWEEN: {
						x = 0;

						line = &lines[line_idx + 1];

						if (line->count > 1) {
							step = (_size[x_axis] - line->line_length) / (line->count - 1);
						}
						else {
							step = 0;
						}
						break;
					}
					}

					child->_position[x_axis] = x;
					x += child->_size[x_axis] + step;

					line_idx++;
				}

				item_idx++;
			}
		}

		// Calculate Y positions of lines
		{
			int32_t y;
			uint32_t step = 0;

			switch (lines_spacing) {
			case GridSpacing::START: {
				y = 0;
				break;
			}

			case GridSpacing::END: {
				y = _size[y_axis] - height;
				break;
			}

			case GridSpacing::CENTER: {
				y = (_size[y_axis] - height) / 2;
				break;
			}

			case GridSpacing::SPACE_BETWEEN: {
				y = 0;
				if (lines.size() > 1) {
					step = (_size[y_axis] - height) / (lines.size() - 1);
				}
				break;
			}
			default:
				throw std::exception();
			}

			uint32_t item_idx = 0;
			uint32_t line_idx = 0;

			for (Element* child : _children) {

				GridLine& line = lines[line_idx];

				if (item_idx < line.end_idx) {
					child->_position[y_axis] = y;
				}
				else {
					y += lines[line_idx].line_thickness + step;
					child->_position[y_axis] = y;

					line_idx++;
				}

				item_idx++;
			}
		}

		if (size[x_axis].type == ElementSizeType::FIT) {
			_size[x_axis] = width;
		}

		if (size[y_axis].type == ElementSizeType::FIT) {
			_size[y_axis] = height;
		}
	};

	switch (orientation) {
	case GridOrientation::ROW: {
		calc_child_positions(0, 1);
		break;
	}

	case GridOrientation::COLUMN: {
		calc_child_positions(1, 0);
		break;
	}
	}

	r_next_parents.insert(_parent);
}

void Grid::setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	_events.setKeyDownEvent(callback, key, user_data);
}

void Grid::setKeyHeldDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	_events.setKeyHeldDownEvent(callback, key, user_data);
}

void Grid::setKeyUpEvent(EventCallback callback, uint32_t key, void* user_data)
{
	_events.setKeyUpEvent(callback, key, user_data);
}

void Grid::setMouseScrollEvent(EventCallback callback, void* user_data)
{
	_events.setMouseScrollEvent(callback, user_data);
}

void Grid::beginMouseDelta()
{
	_events.beginMouseDelta(this);
}

void Menu::_init()
{
	Element::_init();

	MenuItem& root = _items.emplace_back();
	root._parent = nullptr;

	_menu_background_render.init(_window);
	_select_background_render.init(_window);
	_label_render.init(_window);

	_visible_menus.push_back(&root);
}

// #pragma warning(default : 4100)
void Menu::_emitEvents(bool allow_inside_events, std::vector<EventsPassedElement>& r_next_children)
{
	Input& input = _window->input;

	bool found = false;
	
	for (int32_t i = _visible_menus.size() - 1; i >= 0; i--) {

		MenuItem* visible_menu = _visible_menus[i];

		for (MenuItem* item : visible_menu->_children) {

			if (item->_label_box.isInside(input.mouse_x, input.mouse_y)) {

				// emit inside event
				printf("item = %s \n", item->text.c_str());

				found = true;
				_visible_menus.resize(i + 2);
				_visible_menus[i + 1] = item;
				
				//printf("count = %lld \n", _visible_menus.size());

				break;
			}
		}
	}

	if (found) {
		// emit outside events for everyone except visible_menu
	}
	else {
		_visible_menus.resize(1);
		// emit outside events for everyone
	}
}

void Menu::_calcSizeAndRelativeChildPositions(std::unordered_set<Element*>& r_next_parents)
{
	_menu_background_render.reset();
	_select_background_render.reset();
	_label_render.reset();

	uint32_t title_menu_height = 0;
	uint32_t title_menu_width = 0;

	// Menu Titles
	int32_t pen_x = _position[0];
	int32_t pen_y = _position[1];
	{
		MenuItem* root = _visible_menus.front();
		for (MenuItem* item : root->_children) {

			uint32_t width;
			uint32_t height;

			TextInstance props;
			props.pos[0] = item->left_padding + pen_x;
			props.pos[1] = item->top_padding + pen_y;
			props.text = item->text;
			props.font_family = item->font_family;
			props.font_style = item->font_style;
			props.font_size = item->font_size;
			props.line_height = item->line_height;
			props.color = item->text_color;
			_label_render.addInstance(props, width, height);

			item->_label_box.pos[0] = pen_x;
			item->_label_box.pos[1] = pen_y;

			item->_label_box.size[0] = item->left_padding + width + item->right_padding;
			item->_label_box.size[1] = item->top_padding + height + item->bot_padding;

			pen_x += item->_label_box.size[0];

			if (item->_label_box.size[1] > title_menu_height) {
				title_menu_height = item->_label_box.size[1];
			}

			title_menu_width += item->_label_box.size[0];
		}

		for (MenuItem* item : root->_children) {
			item->_label_box.size[1] = title_menu_height;
		}

		RectInstance props;
		props.screen_pos = _position;
		props.size = { title_menu_width, title_menu_height };
		props.color = titles_background_color;

		_menu_background_render.addInstance(props);
	}

	// Menus
	if (_visible_menus.size() > 1) {

		pen_x = _visible_menus[1]->_label_box.pos[0];
		pen_y = _position[1] + title_menu_height;

		for (uint32_t i = 1; i < _visible_menus.size(); i++) {

			MenuItem* menu = _visible_menus[i];

			int32_t menu_x = pen_x;
			int32_t menu_y = pen_y;
			uint32_t menu_width = 0;

			for (MenuItem* item : menu->_children) {

				uint32_t width;
				uint32_t height;

				TextInstance props;
				props.pos[0] = item->left_padding + pen_x;
				props.pos[1] = item->top_padding + pen_y;
				props.text = item->text;
				props.font_family = item->font_family;
				props.font_style = item->font_style;
				props.font_size = item->font_size;
				props.line_height = item->line_height;
				props.color = item->text_color;
				_label_render.addInstance(props, width, height);

				item->_label_box.pos[0] = pen_x;
				item->_label_box.pos[1] = pen_y;

				item->_label_box.size[0] = item->left_padding + width + item->right_padding;  // not final, will be menu width 
				item->_label_box.size[1] = item->top_padding + height + item->bot_padding;

				pen_y += item->_label_box.size[1];

				if (item->_label_box.size[0] > menu_width) {
					menu_width = item->_label_box.size[0];
				}
			}

			// Labels
			for (MenuItem* item : menu->_children) {
				item->_label_box.size[0] = menu_width;
			}

			// Selection
			{
				RectInstance props;
				props.screen_pos = menu->_label_box.pos;
				props.size = menu->_label_box.size;
				props.color = select_background_color;

				_select_background_render.addInstance(props);
			}

			// Menu Background
			if (menu->_children.size()) {

				menu->_menu_box.pos = { menu_x, menu_y };
				menu->_menu_box.size = { menu_width, (uint32_t)(pen_y - menu_y) };

				RectInstance props;
				props.screen_pos = menu->_menu_box.pos;
				props.size = menu->_menu_box.size;
				props.color = menu->menu_background_color;

				_menu_background_render.addInstance(props);
			}

			// Next
			pen_x += menu_width;

			if (i + 1 < _visible_menus.size()) {

				pen_y = _visible_menus[i + 1]->_label_box.pos[1];

				menu->_menu_box.pos = { menu_x, menu_y };
				menu->_menu_box.size = { menu_width, (uint32_t)(pen_y - menu_y) };

				RectInstance props;
				props.screen_pos = menu->_menu_box.pos;
				props.size = menu->_menu_box.size;
				props.color = menu->menu_background_color;

				_menu_background_render.addInstance(props);
			}
		}
	}

	// The size of the menu
	_size[0] = title_menu_width;
	_size[1] = title_menu_height;

	r_next_parents.insert(_parent);
}

void Menu::_generateGPU_Data()
{
	_menu_background_render.generateGPU_Data();
	_select_background_render.generateGPU_Data();
	_label_render.generateGPU_Data();
}

void Menu::_draw()
{
	_menu_background_render.draw();
	_select_background_render.draw();
	_label_render.draw();
}

MenuItem* Menu::addItem(MenuItem* parent)
{
	if (parent == nullptr) {	
		parent = &_items.front();
	}

	MenuItem& new_item = _items.emplace_back();
	new_item._parent = parent;

	// Label
	new_item.text = "";
	new_item.font_family = "Roboto";
	new_item.font_style = "Regular";
	new_item.font_size = 14;
	new_item.line_height = 0;
	new_item.text_color.rgba = { 1, 1, 1, 1 };

	new_item.top_padding = 0;
	new_item.bot_padding = 0;
	new_item.left_padding = 0;
	new_item.right_padding = 0;

	// Menu
	new_item.menu_background_color.setRGBA_UNORM(0, 0, 0, 0);

	parent->_children.push_back(&new_item);

	return &new_item;
}
