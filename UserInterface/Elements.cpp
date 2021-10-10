module;

// Standard
#include <chrono>
#include <variant>


// @C++_BUG: cannot use glm::mix
// fatal error LNK1179: invalid or corrupt file: duplicate COMDAT '??$mix@HM@glm@@YAHHHM@Z'
// 
// @C++_BUG_EXPLANATION: function was called incorectly with 3rd param as float instead of double
//   compiler performed implicit cast (I think) but the linker did not caught on

// GLM
#include "glm\common.hpp"
#include "glm\vec2.hpp"

// Error
#include "ErrorStack.hpp"

// Undefines
#undef RELATIVE
#undef ABSOLUTE

module NuiLibrary;


using namespace nui;


void Element::_calcNowState(ElementRetainedState* prev, ElementCreateInfo& next)
{
	for (uint32_t i = 0; i < 2; i++) {
		this->size[i] = prev->size[i].calc(next.size[i]);
		this->origin[i] = prev->origin[i].calc(next.origin[i]);
		this->relative_position[i] = prev->relative_position[i].calc(next.relative_position[i]);
	}

	this->z_index = next.z_index;
	this->flex_grow = prev->flex_grow.calc(next.flex_grow);
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
				_size[axis] = _children[0]->_size[axis];
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

//void Element::_shadowEmitEvents()
//{
//	
//}
//
//void Element::_shadowTopDownPass()
//{
//	assert_cond(_shadow_root != nullptr);
//
//	std::vector<Element*> now_elems = {
//		_shadow_root
//	};
//	std::vector<Element*> next_elems;
//
//	_shadow_leafs.clear();
//
//	while (now_elems.size()) {
//
//		for (Element* now_elem : now_elems) {
//
//			// needed later for bottom up pass
//			if (now_elem->_children.size() == 0) {
//				_shadow_leafs.insert(now_elem);
//			}
//
//			auto calc_size_for_axis = [now_elem](uint32_t axis) {
//
//				auto& size = now_elem->size[axis];
//				auto& _size = now_elem->_size[axis];
//
//				switch (size.type) {
//				case ElementSizeType::RELATIVE: {
//					auto& _parent_size = now_elem->_parent->_size[axis];
//					_size = std::lroundf(_parent_size * size.relative_size);
//					break;
//				}
//
//				case ElementSizeType::ABSOLUTE: {
//					_size = size.absolute_size;
//					break;
//				}
//
//				case ElementSizeType::FIT: {
//					// size cannot be calculated at this pass
//					_size = 0;
//					break;
//				}
//				}
//			};
//			calc_size_for_axis(0);
//			calc_size_for_axis(1);
//
//			for (Element* child : now_elem->_children) {
//				next_elems.push_back(child);
//			}
//		}
//
//		now_elems.swap(next_elems);
//		next_elems.clear();
//	}
//}
//
//void Element::_shadowBottomUpPass()
//{
//	auto& now_elems = _shadow_leafs;
//	std::unordered_set<Element*> next_elems;
//
//	while (now_elems.size()) {
//
//		for (Element* now_elem : now_elems) {
//
//			now_elem->_calcSizeAndRelativeChildPositions();
//
//			if (now_elem->_parent != nullptr) {
//				next_elems.insert(now_elem->_parent);
//			}
//		}
//
//		now_elems.swap(next_elems);
//		next_elems.clear();
//	}
//}
//
//void Element::_shadowCalcDrawPosition()
//{
//	_shadow_draw_stacks.clear();
//
//	std::vector<PassedElement> now_elems;
//
//	PassedElement& passed_child = now_elems.emplace_back();
//	passed_child.ancestor_pos = { 0, 0 };
//	passed_child.ancestor_z_index = 0;
//	passed_child.elem = _shadow_root;
//
//	std::vector<PassedElement> next_elems;
//
//	while (now_elems.size()) {
//
//		for (PassedElement& now_elem : now_elems) {
//
//			// convert position to screen space
//			Element* elem = now_elem.elem;
//			elem->_position[0] += now_elem.ancestor_pos[0];
//			elem->_position[1] += now_elem.ancestor_pos[1];
//
//			// convert z index to draw call order
//			switch (elem->z_index.type) {
//			case Z_IndexType::INHERIT: {
//				elem->_z_index = now_elem.ancestor_z_index;
//				break;
//			}
//			case Z_IndexType::RELATIVE: {
//				elem->_z_index = now_elem.ancestor_z_index + elem->z_index.value;
//				break;
//			}
//			case Z_IndexType::ABSOLUTE: {
//				elem->_z_index = elem->z_index.value;
//				break;
//			}
//			}
//
//			_shadow_draw_stacks[elem->_z_index].push_back(elem);
//
//			for (Element* child : elem->_children) {
//
//				PassedElement& next_elem = next_elems.emplace_back();
//				next_elem.ancestor_pos = elem->_position;
//				next_elem.ancestor_z_index = elem->_z_index;
//				next_elem.elem = child;
//			}
//		}
//
//		now_elems.swap(next_elems);
//		next_elems.clear();
//	}
//}
//
//void Element::_shadowDraw()
//{
//	for (auto& stack : _shadow_draw_stacks) {
//		for (Element* elem : stack.second) {
//
//			elem->_draw();
//		}
//	}
//}

//void Element::emitEvents()
//{
//
//}
//
//void Element::topDownPass()
//{
//
//}
//
//void Element::bottomUpPass()
//{
//
//}
//
//void Element::calcDrawPosition()
//{
//
//}
//
//void Element::draw()
//{
//
//}

void Container::createText(Text::CreateInfo& info)
{
	StoredElement2& new_entry = _window->elements.emplace_back();

	Text& new_text = new_entry.specific_elem.emplace<Text>();
	new_text._window = _window;
	new_text._parent = _self->base_elem;
	new_entry.base_elem = &new_text;
	new_text._self = &new_entry;

	Text::RetainedState* prev = nullptr;
	{
		for (Text::RetainedState& state : _window->text_prevs) {
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

		prev->info = info;
	}

	new_text._calcNowState(prev, info);
	new_text.state = prev;

	_children.push_back(&new_text);
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

Rect* Container::createRect(RectCreateInfo& info)
{
	StoredElement2& new_entry = _window->elements.emplace_back();

	Rect& new_rect = new_entry.specific_elem.emplace<Rect>();
	new_rect._window = _window;
	new_rect._parent = _self->base_elem;
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

	new_rect._calcNowState(prev, info);

	new_rect.color = prev->color.calc(info.color);

	_children.push_back(&new_rect);

	return &new_rect;
}

void Container::createButton(Button::CreateInfo& info)
{
	StoredElement2& new_entry = _window->elements.emplace_back();

	Button& new_btn = new_entry.specific_elem.emplace<Button>();
	new_btn._window = _window;
	new_btn._parent = _self->base_elem;
	new_entry.base_elem = &new_btn;
	new_btn._self = &new_entry;

	Button::RetainedState* prev = nullptr;
	{
		for (Button::RetainedState& state : _window->button_prevs) {
			if (state.id == info.id) {
				prev = &state;
				break;
			}
		}

		if (prev == nullptr) {
			prev = &_window->button_prevs.emplace_back();
		}

		prev->id = info.id;
		prev->used = true;

		prev->info = info;
	}

	new_btn._calcNowState(prev, info);
	new_btn.state = prev;

	_children.push_back(&new_btn);
}

void Container::createSlider(Slider::CreateInfo& info)
{
	StoredElement2& new_entry = _window->elements.emplace_back();

	Slider& new_slider = new_entry.specific_elem.emplace<Slider>();
	new_slider._window = _window;
	new_slider._parent = _self->base_elem;
	new_entry.base_elem = &new_slider;
	new_slider._self = &new_entry;

	Slider::RetainedState* prev = nullptr;
	{
		for (Slider::RetainedState& state : _window->slide_prevs) {
			if (state.id == info.id) {
				prev = &state;
				break;
			}
		}

		if (prev == nullptr) {
			prev = &_window->slide_prevs.emplace_back();
		}

		prev->id = info.id;
		prev->used = true;

		prev->info = info;
	}

	new_slider._calcNowState(prev, info);
	new_slider.state = prev;

	_children.push_back(&new_slider);
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

void Container::attachTreeList(TreeListCreateInfo& info, TreeList* tree_list)
{
	tree_list->_parent = _self->base_elem;
	_children.push_back(tree_list->_self->base_elem);

	tree_list->_calcNowState(&tree_list->base_elem_state, info);
	tree_list->info = info;
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
		for (Element* child : _children) {

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

//void Root::emitEvents()
//{
//
//}
//
//void Root::topDownPass()
//{
//	// @HERE
//}
//
//void Root::bottomUpPass()
//{
//
//}
//
//void Root::calcDrawPosition()
//{
//
//}
//
//void Root::draw()
//{
//
//}

void Text::_calcSizeAndRelativeChildPositions()
{
	TextProps new_inst;
	new_inst.text = state->info.text;
	new_inst.font_family = state->info.font_family;
	new_inst.font_style = state->info.font_style;
	new_inst.font_size = state->font_size.calc(state->info.font_size);
	new_inst.line_height = state->line_height.calc(state->info.line_height);

	Instance* inst = _window->instance;
	inst->findAndPositionGlyphs(new_inst, 0, 0, _size[0], _size[1], state->instance.chars);

	state->instance.color = state->color.calc(state->info.color);
}

void Text::_draw()
{
	Instance* inst = _window->instance;

	for (auto& pos_char : state->instance.chars) {
		pos_char.pos[0] += _position[0];
		pos_char.pos[1] += _position[1];
	}

	std::vector<TextInstance*> instances = { &state->instance };

	inst->drawTexts(_window, instances);
}

void Rect::_draw()
{
	Instance* inst = _window->instance;

	RectInstance rect_inst;
	rect_inst.pos = _position;
	rect_inst.size = _size;
	rect_inst.color = color;

	std::vector<RectInstance*> instances = { &rect_inst };

	inst->drawRects(_window, instances);
}

void Button::_emitEvents(bool&)
{
	// perform color blending here based on interaction
	
	Input& input = _window->input;
	auto s = state;

	if (state->box.isInside(input.mouse_x, input.mouse_y)) {

		if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].is_down) {

			// click
			if (state->was_down == false) {

				if (s->info.click.on.callback != nullptr) {
					s->info.click.on.callback(_window, _self, s->info.click.on.user_data);
				}
			}

			// pressed
			if (s->info.press.on.callback != nullptr) {
				s->info.press.on.callback(_window, _self, s->info.press.on.user_data);
			}

			s->border_instance.color = s->border_color.calc(s->info.press.border_color);
			s->background_instance.color = s->background_color.calc(s->info.press.background_color);
			s->text_instance.color = s->text_color.calc(s->info.press.text_color);
			s->was_down = true;
		}
		// hover
		else {
			s->border_instance.color = s->border_color.calc(s->info.hover.border_color);
			s->background_instance.color = s->background_color.calc(s->info.hover.background_color);
			s->text_instance.color = s->text_color.calc(s->info.hover.text_color);
			s->was_down = false;
		}

		if (s->info.hover.on.callback != nullptr) {
			s->info.hover.on.callback(_window, _self, s->info.hover.on.user_data);
		}
	}
	// default
	else {
		s->border_instance.color = s->border_color.calc(s->info.border.color);
		s->background_instance.color = s->background_color.calc(s->info.background_color);
		s->text_instance.color = s->text_color.calc(s->info.text_color);
		s->was_down = false;
	}
}

void Button::_calcSizeAndRelativeChildPositions()
{
	// if size fit use padding
	// else ignore padding and use computed

	Instance* inst = _window->instance;
	auto s = state;

	TextProps text_props;
	text_props.text = s->info.text;
	text_props.font_family = s->info.font_family;
	text_props.font_style = s->info.font_style;
	text_props.font_size = s->info.font_size;
	text_props.line_height = s->info.line_height;

	inst->findAndPositionGlyphs(text_props,
		_position[0], _position[1],
		s->box.size[0], s->box.size[1],
		s->text_instance.chars);

	s->text_instance.color = s->text_instance.color;

	auto adjust_text_position = [&](uint8_t axis) {

		uint32_t offset = 0xFFFF'FFFF;

		if (size[axis].type == ElementSizeType::FIT) {

			switch (axis) {
			case 0: {
				_size[axis] = s->info.border.thickness * 2 + s->info.padding.width() + s->box.size[axis];
				offset = s->info.border.thickness + s->info.padding.left;
				break;
			}
			case 1: {
				_size[axis] = s->info.border.thickness * 2 + s->info.padding.height() + s->box.size[axis];
				offset = s->info.border.thickness + s->info.padding.top;
				break;
			}
			}
		}
		else {
			offset = (_size[axis] - s->box.size[axis]) / 2;
		}

		for (auto& chara : s->text_instance.chars) {
			chara.pos[axis] += offset;
		}
	};

	adjust_text_position(0);
	adjust_text_position(1);

	state->box.size = _size;
}

void Button::_draw()
{
	Instance* inst = _window->instance;
	auto s = state;

	{
		state->box.pos = _position;
	}

	uint32_t border_thickness = s->info.border.thickness;

	// SimpleBorder
	{
		state->border_instance.screen_pos = _position;
		state->border_instance.size = _size;
		state->border_instance.thickness = border_thickness;

		std::vector<BorderInstance*> instances = { &state->border_instance };

		inst->drawBorder(_window, instances);
	}

	// Background
	{
		state->background_instance.pos = {
			_position[0] + (int32_t)border_thickness,
			_position[1] + (int32_t)border_thickness
		};
		state->background_instance.size = {
			_size[0] - (int32_t)(border_thickness * 2),
			_size[1] - (int32_t)(border_thickness * 2)
		};

		std::vector<RectInstance*> instances = { &state->background_instance };

		inst->drawRects(_window, instances);
	}

	// Text
	{
		for (auto& pos_char : state->text_instance.chars) {
			pos_char.pos[0] += _position[0];
			pos_char.pos[1] += _position[1];
		}

		std::vector<TextInstance*> instances = { &state->text_instance };

		inst->drawTexts(_window, instances);
	}
}

void Slider::_emitEvents(bool&)
{
	Input& input = _window->input;
	auto s = state;

	if (s->slider_box.isInside(input.mouse_x, input.mouse_y)) {

		// press
		if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].is_down) {

			// slider box is slightly larger for better usability than the track
			{
				int32_t track_left = s->slider_box.pos[0] + s->info.track_collider_additional_length;
				int32_t track_size = s->slider_box.size[0] - (s->info.track_collider_additional_length * 2);
				s->slider_fill_ratio = (float)(input.mouse_x - track_left) / track_size;
				s->slider_fill_ratio = glm::clamp(s->slider_fill_ratio, 0.f, 1.f);
			}

			switch (s->info.value_info.index()) {
			case 0: {
				auto value_info = std::get_if<CreateInfo::ValueCreateInfo<int32_t>>(&s->info.value_info);
				
				int32_t value = glm::mix(value_info->soft_min, value_info->soft_max, (double)s->slider_fill_ratio);

				if (s->info.thumb_adjust_callback != nullptr) {
					s->info.thumb_adjust_callback(_window, value, 0, s->info.thumb_adjust_user_data);
				}
				break;
			}

			case 1: {
				auto value_info = std::get_if<CreateInfo::ValueCreateInfo<float>>(&s->info.value_info);

				float value = glm::mix(value_info->soft_min, value_info->soft_max, (double)s->slider_fill_ratio);

				if (s->info.thumb_adjust_callback != nullptr) {
					s->info.thumb_adjust_callback(_window, 0, value, s->info.thumb_adjust_user_data);
				}
				break;
			}
			}

			// pressed style
			s->track_background_instance.color = s->track_background_color.calc(s->info.press.track_background_color);
			s->track_fill_instance.color = s->track_fill_color.calc(s->info.press.track_fill_color);
			s->circle_instance.color = s->thumb_color.calc(s->info.press.thumb_color);
		}
		// hover
		else {
			s->track_background_instance.color = s->track_background_color.calc(s->info.hover.track_background_color);
			s->track_fill_instance.color = s->track_fill_color.calc(s->info.hover.track_fill_color);
			s->circle_instance.color = s->thumb_color.calc(s->info.hover.thumb_color);
		}
	}
	// default
	else {
		s->track_background_instance.color = s->track_background_color.calc(s->info.track_background_color);
		s->track_fill_instance.color = s->track_fill_color.calc(s->info.track_fill_color);
		s->circle_instance.color = s->thumb_color.calc(s->info.thumb_color);
	}

	// return default init value
	if (s->slider_fill_ratio == -1.f) {

		switch (s->info.value_info.index()) {
		case 0: {
			auto value_info = std::get_if<CreateInfo::ValueCreateInfo<int32_t>>(&s->info.value_info);
			s->slider_fill_ratio = (float)inverseLerp(value_info->initial, value_info->soft_min, value_info->soft_max);
			break;
		}
		case 1: {
			auto value_info = std::get_if<CreateInfo::ValueCreateInfo<float>>(&s->info.value_info);
			s->slider_fill_ratio = inverseLerp(value_info->initial, value_info->soft_min, value_info->soft_max);
			break;
		}
		}
	}
}

void Slider::_calcSizeAndRelativeChildPositions()
{
	auto s = state;

	if (s->info.is_vertical == false) {

		s->circle_instance.radius = s->info.thumb_diameter / 2;

		if (size[0].type != ElementSizeType::FIT) {
			s->info.track_length = _size[0];
		}

		// Horizontal
		{
			s->track_background_instance.size[0] = s->info.track_length;
			s->track_fill_instance.size[0] = (uint32_t)(s->info.track_length * s->slider_fill_ratio);

			s->track_background_instance.pos[0] = 0;
			s->track_fill_instance.pos[0] = 0;
			s->circle_instance.pos[0] = s->track_fill_instance.size[0];

			s->slider_box.size[0] = s->info.track_length + (s->info.track_collider_additional_length * 2);

			_size[0] = s->info.track_length;
		}

		// Vertical
		uint32_t max_height = 0;
		{
			std::array<uint32_t, 1> heights;
			heights[0] = s->info.thumb_diameter;

			for (uint32_t height : heights) {
				if (height > max_height) {
					max_height = height;
				}
			}
		}

		s->track_background_instance.size[1] = s->info.track_background_thickness;
		s->track_fill_instance.size[1] = s->info.track_fill_thickness;

		s->track_background_instance.pos[1] = (max_height - s->info.track_background_thickness) / 2;
		s->track_fill_instance.pos[1] = (max_height - s->info.track_fill_thickness) / 2;

		s->circle_instance.pos[1] = max_height / 2;

		s->slider_box.size[1] = s->info.thumb_diameter + (s->info.track_collider_additional_thickness * 2);

		_size[1] = max_height;
	}
	else {
		throw std::exception();
	}
}

void Slider::_draw()
{
	Instance* inst = _window->instance;
	auto s = state;

	// Slider Box
	s->slider_box.pos = {
		_position[0] - (int32_t)s->info.track_collider_additional_length,
		_position[1] - (int32_t)s->info.track_collider_additional_thickness
	};

	// Track Background
	s->track_background_instance.pos[0] += _position[0];
	s->track_background_instance.pos[1] += _position[1];
	inst->drawRect(_window, &s->track_background_instance);

	// Track fill
	s->track_fill_instance.pos[0] += _position[0];
	s->track_fill_instance.pos[1] += _position[1];
	inst->drawRect(_window, &s->track_fill_instance);

	// Thumb
	s->circle_instance.pos[0] += _position[0];
	s->circle_instance.pos[1] += _position[1];
	inst->drawCircle(_window, &s->circle_instance);
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

			for (Element* child : _children) {

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

					Element* child = _children[child_idx];
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
			rect_instance.pos = _position;
			rect_instance.size = root_submenu.box.size;
			rect_instance.color = root_submenu.info.background_color;

			std::vector<RectInstance*> instances = { &rect_instance };

			inst->drawRects(_window, instances);
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
					rect_instance.pos = item.box.pos;
					rect_instance.size = item.box.size;
					rect_instance.color = item.info.background_hover_color;

					std::vector<RectInstance*> rect_instances = { &rect_instance };

					inst->drawRects(_window, rect_instances);

					if (state->visible_menus.size() > 1) {

						BorderInstance border_inst;
						border_inst.screen_pos = item.box.pos;
						border_inst.size = item.box.size;
						border_inst.thickness = root_submenu.info.border_thickness;
						border_inst.color = root_submenu.info.border_color;
						border_inst.bot = false;

						std::vector<BorderInstance*> border_instances = { &border_inst };

						inst->drawBorder(_window, border_instances);
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
				rect_instance.pos = {
					submenu.box.pos[0],
					submenu.box.pos[1]
				};
				rect_instance.size = {
					submenu.box.size[0],
					submenu.box.size[1]
				};
				rect_instance.color = submenu.info.background_color;

				std::vector<RectInstance*> rect_instances = { &rect_instance };

				inst->drawRects(_window, rect_instances);
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

					std::vector<BorderInstance*> border_instances = { &border_inst };
					inst->drawBorder(_window, border_instances);
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

					std::vector<BorderInstance*> border_instances = { &border_inst };
					inst->drawBorder(_window, border_instances);
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
						rect_instance.pos = {
							item.box.pos[0] + border_thick,
							item.box.pos[1]
						};
						rect_instance.size = {
							item.box.size[0] - (2 * border_thick),
							item.box.size[1]
						};
						rect_instance.color = item.info.background_hover_color;

						std::vector<RectInstance*> rect_instances = { &rect_instance };
						inst->drawRects(_window, rect_instances);
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

TreeListCreateInfo::TreeListCreateInfo()
{
	ElementCreateInfo::ElementCreateInfo();
	this->size[0] = 200;
	this->size[1] = 100.f;

	// Item
	this->item.padding = {};
	this->item.text_hover_color = Color::black();
	this->item.background_hover_color = Color::white();
	this->item.arrow.left_padding = 0;
	this->item.arrow.right_padding = 0;
	this->item.arrow.width = 14;
	this->item.arrow.height = 14;
	this->item.arrow.color = Color::white();
	this->item.arrow.hover_color = Color::black();

	// Layout
	this->indentation = 14;
}

TreeListItemHandle::TreeListItemHandle(uint32_t new_item_idx)
{
	this->item_idx = new_item_idx;
}

TreeListItemHandle TreeList::createItem(TreeListItemCreateInfo& new_info)
{
	TreeListItemHandle new_handle;
	new_handle.item_idx = 0;
	return createItem(new_handle, new_info);
}

TreeListItemHandle TreeList::createItem(TreeListItemHandle handle, TreeListItemCreateInfo& new_info)
{
	uint32_t self_idx = items.size();

	TreeListItem& parent = items[handle.item_idx];
	parent.children.push_back(self_idx);

	TreeListItem& new_item = items.emplace_back();
	new_item.treelist = this;
	new_item.parent = handle.item_idx;
	new_item.self_idx = self_idx;
	new_item.expanded = false;
	new_item.info = new_info;

	TreeListItemHandle new_handle;
	new_handle.item_idx = self_idx;
	return new_handle;
}

void TreeList::_calcSizeAndRelativeChildPositions()
{
	assert_cond(size[0].type != ElementSizeType::FIT &&
		size[1].type != ElementSizeType::FIT,
		"TreeList does not support fit size");

	Instance* inst = _window->instance;

	// Mark items as un-traversed
	{
		for (TreeListItem& item : items) {
			item._traversed = false;
		}
	}

	// Down first pass
	{
		_text_instances.clear();

		uint32_t next_item = 0;

		required_size = { 0, 0 };

		int32_t indent_level = 0;
		int32_t pen_y = 0;

		// render all items on return
		// if all child items have been traversed

		while (next_item != 0xFFFF'FFFF) {

			TreeListItem& item = items[next_item];

			bool all_children_traversed = true;
			for (uint32_t child_item_idx : item.children) {

				TreeListItem& child_item = items[child_item_idx];

				if (child_item._traversed == false) {

					child_item._traversed = true;
					all_children_traversed = false;

					int32_t pen_x = info.indentation * indent_level;
					uint32_t text_width;
					uint32_t text_height;

					// Text
					{
						inst->findAndPositionGlyphs(child_item.info.text,
							pen_x + info.item.padding.left,
							pen_y + info.item.padding.top,
							text_width, text_height,
							child_item._text.chars);

						child_item._text.color = child_item.info.text.color;

						_text_instances.push_back(&child_item._text);
					}

					child_item._label_size = {
						info.item.padding.left + text_width + info.item.padding.right,
						info.item.padding.top + text_height + info.item.padding.bot,
					};

					// TreeList Witdh
					{
						uint32_t new_width = pen_x + child_item._label_size[0];

						if (new_width > required_size[0]) {
							required_size[0] = new_width;
						}
					}

					// schedule down
					indent_level++;
					next_item = child_item_idx;

					pen_y += child_item._label_size[1];
					break;
				}
			}

			if (all_children_traversed) {

				// schedule up
				indent_level--;
				next_item = item.parent;
			}
		}

		required_size[1] = pen_y;
	}
}

void TreeList::_draw()
{
	Instance* inst = _window->instance;

	// render root background
	// render item background

	/*std::array<int32_t, 2> scroll_offset = { 0, 0 };
	{
		TreeListItem& root = items[0];

		uint32_t available_height = _size[1];
		uint32_t available_width = _size[0];

		if (root._background.size[1] > available_height) {
			available_width -= info.scroll_bar_thickness;
		}

		if (root._background.size[0] > available_width) {
			available_height -= info.scroll_bar_thickness;
		}

		if (root._background.size[1] > available_height) {

			uint32_t remainder = root._background.size[1] - available_height;
			scroll_offset[1] = (int32_t)(scroll_factor[1] * (float)remainder);
		}

		if (root._background.size[0] > available_width) {

			uint32_t remainder = root._background.size[0] - available_width;
			scroll_offset[0] = (int32_t)(scroll_factor[0] * (float)remainder);
		}
	}*/

	// Draw Treelist Background
	{
		RectInstance rect_inst;
		rect_inst.pos = _position;
		rect_inst.size = _size;
		rect_inst.color = info.background_color;

		inst->drawRect(_window, &rect_inst);
	}

	// Draw Hover Item Background
	{
		
	}

	// Render Text
	{
		for (TextInstance* text_inst : _text_instances) {
			for (PositionedCharacter& chara : text_inst->chars) {
				chara.pos[0] += _position[0];
				chara.pos[1] += _position[1];
			}
		}

		inst->drawTexts(_window, _text_instances);
	}
}
