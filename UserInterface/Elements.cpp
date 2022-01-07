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
#include "glm\glm.hpp"
#include "glm\vec2.hpp"

// Mine
#include "ErrorStack.hpp"
#include "MathStuff.hpp"
#include "Input.hpp"

// Undefines
#undef RELATIVE
#undef ABSOLUTE

module UserInterface;


using namespace nui;


void Element::_calcNowState(ElementRetainedState* prev, ElementCreateInfo& next)
{
	this->id = next.id;

	for (uint32_t i = 0; i < 2; i++) {
		this->size[i] = prev->size[i].calc(next.size[i]);
		this->origin[i] = prev->origin[i].calc(next.origin[i]);
		this->relative_position[i] = prev->relative_position[i].calc(next.relative_position[i]);
	}

	this->z_index = next.z_index;
	this->flex_grow = prev->flex_grow.calc(next.flex_grow);
}

//void Element::_emitEvents(bool&, bool&)
//{
//	// no events
//};

bool Element::_isInside()
{
	return false;
}

void Element::_emitInsideEvents(bool&, bool&)
{
	// no events
}

void Element::_emitOutsideEvents()
{
	// no events
}

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

////void Element::_shadowEmitEvents()
////{
////	
////}
////
////void Element::_shadowTopDownPass()
////{
////	assert_cond(_shadow_root != nullptr);
////
////	std::vector<Element*> now_elems = {
////		_shadow_root
////	};
////	std::vector<Element*> next_elems;
////
////	_shadow_leafs.clear();
////
////	while (now_elems.size()) {
////
////		for (Element* now_elem : now_elems) {
////
////			// needed later for bottom up pass
////			if (now_elem->_children.size() == 0) {
////				_shadow_leafs.insert(now_elem);
////			}
////
////			auto calc_size_for_axis = [now_elem](uint32_t axis) {
////
////				auto& size = now_elem->size[axis];
////				auto& _size = now_elem->_size[axis];
////
////				switch (size.type) {
////				case ElementSizeType::RELATIVE: {
////					auto& _parent_size = now_elem->_parent->_size[axis];
////					_size = std::lroundf(_parent_size * size.relative_size);
////					break;
////				}
////
////				case ElementSizeType::ABSOLUTE: {
////					_size = size.absolute_size;
////					break;
////				}
////
////				case ElementSizeType::FIT: {
////					// size cannot be calculated at this pass
////					_size = 0;
////					break;
////				}
////				}
////			};
////			calc_size_for_axis(0);
////			calc_size_for_axis(1);
////
////			for (Element* child : now_elem->_children) {
////				next_elems.push_back(child);
////			}
////		}
////
////		now_elems.swap(next_elems);
////		next_elems.clear();
////	}
////}
////
////void Element::_shadowBottomUpPass()
////{
////	auto& now_elems = _shadow_leafs;
////	std::unordered_set<Element*> next_elems;
////
////	while (now_elems.size()) {
////
////		for (Element* now_elem : now_elems) {
////
////			now_elem->_calcSizeAndRelativeChildPositions();
////
////			if (now_elem->_parent != nullptr) {
////				next_elems.insert(now_elem->_parent);
////			}
////		}
////
////		now_elems.swap(next_elems);
////		next_elems.clear();
////	}
////}
////
////void Element::_shadowCalcDrawPosition()
////{
////	_shadow_draw_stacks.clear();
////
////	std::vector<PassedElement> now_elems;
////
////	PassedElement& passed_child = now_elems.emplace_back();
////	passed_child.ancestor_pos = { 0, 0 };
////	passed_child.ancestor_z_index = 0;
////	passed_child.elem = _shadow_root;
////
////	std::vector<PassedElement> next_elems;
////
////	while (now_elems.size()) {
////
////		for (PassedElement& now_elem : now_elems) {
////
////			// convert position to screen space
////			Element* elem = now_elem.elem;
////			elem->_position[0] += now_elem.ancestor_pos[0];
////			elem->_position[1] += now_elem.ancestor_pos[1];
////
////			// convert z index to draw call order
////			switch (elem->z_index.type) {
////			case Z_IndexType::INHERIT: {
////				elem->_z_index = now_elem.ancestor_z_index;
////				break;
////			}
////			case Z_IndexType::RELATIVE: {
////				elem->_z_index = now_elem.ancestor_z_index + elem->z_index.value;
////				break;
////			}
////			case Z_IndexType::ABSOLUTE: {
////				elem->_z_index = elem->z_index.value;
////				break;
////			}
////			}
////
////			_shadow_draw_stacks[elem->_z_index].push_back(elem);
////
////			for (Element* child : elem->_children) {
////
////				PassedElement& next_elem = next_elems.emplace_back();
////				next_elem.ancestor_pos = elem->_position;
////				next_elem.ancestor_z_index = elem->_z_index;
////				next_elem.elem = child;
////			}
////		}
////
////		now_elems.swap(next_elems);
////		next_elems.clear();
////	}
////}
////
////void Element::_shadowDraw()
////{
////	for (auto& stack : _shadow_draw_stacks) {
////		for (Element* elem : stack.second) {
////
////			elem->_draw();
////		}
////	}
////}
//
////void Element::emitEvents()
////{
////
////}
////
////void Element::topDownPass()
////{
////
////}
////
////void Element::bottomUpPass()
////{
////
////}
////
////void Element::calcDrawPosition()
////{
////
////}
////
////void Element::draw()
////{
////
////}

bool Root::_isInside()
{
	return true;
}

void Root::_emitInsideEvents(bool&, bool&)
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
	auto s = state;

	RectInstance rect_inst;
	rect_inst.pos = _position;
	rect_inst.size = _size;
	rect_inst.color = s->color.calc(s->info.color);

	std::vector<RectInstance*> instances = { &rect_inst };

	inst->drawRects(_window, instances);
}

bool Button::_isInside()
{
	Input& input = _window->input;

	return state->box.isInside(input.mouse_x, input.mouse_y);
}

void Button::_emitInsideEvents(bool& r_allow_inside_events, bool& r_exclusive)
{
	Input& input = _window->input;
	auto s = state;

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

void Button::_emitOutsideEvents()
{
	auto s = state;

	s->border_instance.color = s->border_color.calc(s->info.border.color);
	s->background_instance.color = s->background_color.calc(s->info.background_color);
	s->text_instance.color = s->text_color.calc(s->info.text_color);
	s->was_down = false;
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

bool Slider::_isInside()
{
	Input& input = _window->input;
	return state->slider_box.isInside(input.mouse_x, input.mouse_y);
}

void Slider::_emitInsideEvents(bool& r_allow_inside_events, bool& r_exclusive)
{
	Input& input = _window->input;
	auto s = state;

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

void Slider::_emitOutsideEvents()
{
	auto s = state;

	s->track_background_instance.color = s->track_background_color.calc(s->info.track_background_color);
	s->track_fill_instance.color = s->track_fill_color.calc(s->info.track_fill_color);
	s->circle_instance.color = s->thumb_color.calc(s->info.thumb_color);

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

bool Slider2::_isInside()
{
	if (state->text_input._window == nullptr) {
		state->text_input.init(_window);
	}

	Input& input = _window->input;
	return state->box.isInside(input.mouse_x, input.mouse_y);
}

void Slider2::_emitInsideEvents(bool&, bool& r_exclusive)
{
	Input& input = _window->input;
	RetainedState* s = state;
	CreateInfo& info = s->info;

	switch (s->mode) {
	case _EditMode::SLIDER: {

		if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].down_transition) {
			s->mouse_start_x = input.mouse_x;
			s->mouse_start_y = input.mouse_y;

			s->moved_more = false;
			s->initial_fill_ratio = s->fill_ratio;
		}
		else if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].up_transition) {

			uint32_t mouse_delta = std::abs((int32_t)s->mouse_start_x - input.mouse_x);

			if (mouse_delta < info.value.movement_threshold && s->moved_more == false) {

				// set initial content of the input box
				s->text_input.set(s->value, info.value.decimal_places);
				s->text_input.deselect();

				s->mode = _EditMode::VALUE_EDIT;
				r_exclusive = true;
			}
			else {
				r_exclusive = false;
			}
		}

		if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].is_down) {

			// if mouse is moved beyond movement_threshold then upon returning to its
			// initial position do not trigger VALUE_EDIT mode
			uint32_t mouse_delta = std::abs((int32_t)s->mouse_start_x - input.mouse_x);

			if (mouse_delta > info.value.movement_threshold) {
				s->moved_more = true;
			}

			// fill ratio is based on the delta of fill ratio NOT directly mapped to mouse coordinates
			float delta_fill_ratio = ((float)s->mouse_start_x - input.mouse_x) / s->box.size[0];
			s->fill_ratio = s->initial_fill_ratio - delta_fill_ratio;
			s->fill_ratio = glm::clamp(s->fill_ratio, 0.f, 1.f);

			s->value = glm::mix(info.value.soft_min, info.value.soft_max, (double)s->fill_ratio);

			if (info.callback != nullptr) {
				info.callback(_window, s->value, info.user_data);
			}

			r_exclusive = true;
		}

		s->background_instance.color = s->background_color.calc(info.hover.background_color);
		s->fill_instance.color = s->fill_color.calc(info.hover.fill_color);
		s->label_instance.color = s->label_color.calc(info.hover.label_color);
		s->value_instance.color = s->value_color.calc(info.hover.value_color);
		break;
	}

	case _EditMode::VALUE_EDIT: {

		if (s->box.isInside(input.mouse_x, input.mouse_y) == false &&
			input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].down_transition)
		{
			s->mode = _EditMode::SLIDER;
			r_exclusive = false;
		}
		else {
			s->text_input.respondToInput();
		}
		break;
	}
	}
}

void Slider2::_emitOutsideEvents()
{
	auto s = state;
	CreateInfo& info = s->info;

	switch (s->mode) {
	case _EditMode::SLIDER: {
		s->background_instance.color = s->background_color.calc(info.background_color);
		s->fill_instance.color = s->fill_color.calc(info.fill_color);
		s->label_instance.color = s->label_color.calc(info.label.color);
		s->value_instance.color = s->value_color.calc(info.value.color);
		break;
	}

	case _EditMode::VALUE_EDIT: {
		s->value_instance.color = s->value_color.calc(info.value.color);
		break;
	}
	}
}

void Slider2::_calcSizeAndRelativeChildPositions()
{
	assert_cond(size[0].type != ElementSizeType::FIT &&
		size[1].type != ElementSizeType::FIT);

	Instance* inst = _window->instance;
	RetainedState* s = state;
	CreateInfo& info = s->info;

	// initial value
	if (s->fill_ratio == -1.f) {
		s->fill_ratio = inverseLerp(info.value.initial, info.value.soft_min, info.value.soft_max);
		s->value = info.value.initial;
	}

	switch (s->mode) {
	case _EditMode::SLIDER: {

		// Background
		{
			s->box.size = _size;

			s->background_instance.size = _size;

			s->fill_instance.size[0] = s->fill_ratio * _size[0];
			s->fill_instance.size[1] = _size[1];
		}

		// Label
		uint32_t text_width, text_height;
		{
			GlyphProperties glyph_props;
			glyph_props.font_family = &info.label.font_family;
			glyph_props.font_style = &info.label.font_style;
			glyph_props.font_size = info.label.font_size;
			glyph_props.line_height = info.label.line_height;
			glyph_props.offset_x = info.label.padding_left;

			inst->findAndPositionGlyphs(
				info.label.text, glyph_props,
				text_width, text_height,
				s->label_instance.chars);

			// Center the text vertically
			int32_t offset_y = (s->box.size[1] - text_height) / 2;
			s->label_instance.offsetPosition({ 0, offset_y });
		}

		// Value
		uint32_t value_width, value_height;
		{
			GlyphProperties glyph_props;
			glyph_props.font_family = &info.value.font_family;
			glyph_props.font_style = &info.value.font_style;
			glyph_props.font_size = info.value.font_size;
			glyph_props.line_height = info.value.line_height;

			inst->findAndPositionGlyphs(
				s->value, info.value.decimal_places, glyph_props,
				value_width, value_height,
				s->value_instance.chars
			);

			int32_t offset_x = s->box.size[0] - value_width - s->info.value.padding_right;
			int32_t offset_y = (s->box.size[1] - value_height) / 2;
			s->value_instance.offsetPosition({ offset_x, offset_y });
		}
		break;
	}

	case _EditMode::VALUE_EDIT: {
		
		// Background
		{
			s->box.size = _size;
			s->background_instance.size = _size;
		}

		// Value
		uint32_t value_width, value_height;
		{
			auto& input_info = s->text_input.info;
			input_info.font_family = info.value.font_family;
			input_info.font_style = info.value.font_style;
			input_info.font_size = info.value.font_size;
			input_info.line_height = info.value.line_height;

			s->text_input.generateGPU_Data(value_width, value_height);

			int32_t offset_x = (s->box.size[0] - value_width) / 2;
			int32_t offset_y = (s->box.size[1] - value_height) / 2;
			s->text_input.offsetPosition({ offset_x, offset_y });
		}
		break;
	}
	}
}

void Slider2::_draw()
{
	Instance* inst = _window->instance;
	auto s = state;

	switch (s->mode) {
	case _EditMode::SLIDER: {

		s->box.pos = _position;

		// Background
		{
			s->background_instance.pos = _position;
			s->fill_instance.pos = _position;

			inst->drawRect(_window, &s->background_instance);
			inst->drawRect(_window, &s->fill_instance);
		}

		// Label and Value
		{
			s->label_instance.offsetPosition(_position);
			s->value_instance.offsetPosition(_position);

			std::vector<TextInstance*> instances = {
				&s->label_instance, &s->value_instance
			};
			inst->drawTexts(_window, instances);
		}
		break;
	}

	case _EditMode::VALUE_EDIT: {
		
		// Background
		{
			s->background_instance.pos = _position;

			inst->drawRect(_window, &s->background_instance);
		}

		// Value
		s->text_input.offsetPosition(_position);

		s->text_input.draw();
		break;
	}
	}
}

bool Dropdown::_isInside()
{
	Input& input = _window->input;
	auto s = state;

	// closed
	if (s->is_open == false) {

		if (s->boxes[0].isInside(input.mouse_x, input.mouse_y)) {

			if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].up_transition) {
				s->is_open = true;
			}
		}
	}
	// open
	else {
		if (s->boxes[0].isInside(input.mouse_x, input.mouse_y)) {

			if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].up_transition) {
				s->is_open = false;
			}
			s->hover_index = 0xFFFF'FFFF;
		}
		else {
			s->hover_index = 0xFFFF'FFFF;

			for (uint32_t i = 1; i < s->boxes.size(); i++) {

				Box2D& box = s->boxes[i];

				if (box.isInside(input.mouse_x, input.mouse_y)) {

					if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].up_transition) {
						s->is_open = false;
						s->selected_index = i - 1;
					}

					s->hover_index = i - 1;
					break;
				}
			}

			// if mouse is outside
			if (s->hover_index == 0xFFFF'FFFF) {
				if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].up_transition) {
					s->is_open = false;
				}
			}
		}
	}

	return false;
}

void Dropdown::_calcSizeAndRelativeChildPositions()
{
	auto s = state;
	Instance* inst = _window->instance;

	assert_cond(size[1].type == ElementSizeType::FIT, "not supported");

	int32_t pen_y = 0;

	uint32_t max_text_width = 0;

	TextProps text_props;
	text_props.font_family = s->info.font_family;;
	text_props.font_style = s->info.font_style;
	text_props.font_size = s->info.font_size;
	text_props.line_height = s->info.line_height;

	s->text_instances.clear();
	s->background_instances.clear();
	s->boxes.clear();

	uint32_t res_index = 0;

	auto calc_option = [&](uint32_t option_index) {

		TextInstance& text_instance = s->text_instances[res_index];
		RectInstance& background_instance = s->background_instances[res_index];

		// Position text
		uint32_t text_width;
		uint32_t text_height;
		{
			text_props.text = s->info.options[option_index];

			inst->findAndPositionGlyphs(text_props,
				s->info.side_padding, pen_y + s->info.vertical_padding,
				text_width, text_height,
				text_instance.chars);
		}

		// Background
		{
			background_instance.pos = {
				0, pen_y
			};
			background_instance.size[1] = text_height + s->info.vertical_padding * 2;
		}

		// Selected Style
		if (res_index == 0) {
			auto& selected = s->info.selected;
			text_instance.color = s->text_color.calc(selected.text_color);
			background_instance.color = s->background_color.calc(selected.background_color);
		}
		// Hover or Default Style
		else {
			if (option_index == s->hover_index) {
				auto& hover = s->info.hover;
				text_instance.color = s->text_color.calc(hover.text_color);
				background_instance.color = s->background_color.calc(hover.background_color);
			}
			else {
				text_instance.color = s->text_color.calc(s->info.text_color);
				background_instance.color = s->background_color.calc(s->info.background_color);
			}
		}

		// Position box
		{
			Box2D& box = s->boxes[res_index];
			box.pos = { 0, pen_y };
			box.size[1] = text_height + s->info.vertical_padding * 2;
		}

		if (text_width > max_text_width) {
			max_text_width = text_width;
		}

		pen_y += text_height + s->info.vertical_padding * 2;

		res_index++;
	};

	// close
	if (s->is_open == false) {
		s->text_instances.resize(1);
		s->background_instances.resize(1);
		s->boxes.resize(1);

		calc_option(s->selected_index);
	}
	// open
	else {
		uint32_t rows = s->info.options.size() + 1;
		s->text_instances.resize(rows);
		s->background_instances.resize(rows);
		s->boxes.resize(rows);

		calc_option(s->selected_index);

		for (uint32_t option_idx = 0; option_idx < s->info.options.size(); option_idx++) {
			calc_option(option_idx);
		}
	}

	if (size[0].type == ElementSizeType::FIT) {

		for (uint32_t i = 0; i < s->boxes.size(); i++) {

			Box2D& box = s->boxes[i];
			box.size[0] = max_text_width + s->info.side_padding * 2;

			RectInstance& background_instance = s->background_instances[i];
			background_instance.size[0] = max_text_width + s->info.side_padding * 2;
		}

		_size[0] = s->boxes[0].size[0];
		_size[1] = s->boxes[0].size[1];
	}
	else {
		for (uint32_t i = 0; i < s->boxes.size(); i++) {

			Box2D& box = s->boxes[i];
			box.size[0] = _size[0];

			RectInstance& background_instance = s->background_instances[i];
			background_instance.size[0] = _size[0];
		}
	}
}

void Dropdown::_draw()
{
	Instance* inst = _window->instance;
	auto s = state;

	// Background
	{
		for (RectInstance& background_instance : s->background_instances) {
			background_instance.pos[0] += _position[0];
			background_instance.pos[1] += _position[1];
		}

		inst->drawRects(_window, s->background_instances);
	}

	// Text
	{
		for (TextInstance& text_instance : s->text_instances) {

			for (auto& pos_char : text_instance.chars) {
				pos_char.pos[0] += _position[0];
				pos_char.pos[1] += _position[1];
			}
		}

		ClipZone clip_zone;
		clip_zone.pos = { 0, 0 };
		clip_zone.size = { (uint32_t)_window->viewport.Width, (uint32_t)_window->viewport.Height };

		inst->drawTexts(_window, clip_zone, s->text_instances);
	}

	// Boxes
	for (Box2D& box : s->boxes) {
		box.pos[0] += _position[0];
		box.pos[1] += _position[1];
	}
}

void DirectX11_Viewport::_emitEvents(bool&, bool&)
{
	Input& input = _window->input;

	if (state->events._window == nullptr) {
		state->events._init(_window);
	}

	if (state->box.isInside(input.mouse_x, input.mouse_y)) {
		state->events._emitInsideEvents(_self);
	}
	else {
		state->events._emitOutsideEvents(_self);
	}
}

void DirectX11_Viewport::_draw()
{
	assert_cond(state->info.callback != nullptr);

	Instance* inst = _window->instance;

	// calculate collider
	state->box.pos = _position;
	state->box.size = _size;

	// Call external rendering function
	DirectX11_DrawEvent draw_event;
	draw_event.dev5 = inst->dev5.Get();
	draw_event.im_ctx3 = inst->im_ctx3.Get();
	draw_event.render_target_size = { _window->surface_width, _window->surface_height };
	draw_event.render_target = _window->present_rtv.Get();
	draw_event.viewport_pos = _position;
	draw_event.viewport_size = _size;

	auto& info = state->info;
	info.callback(_window, _self, draw_event, info.user_data);
}

////void BackgroundElement::setColorTransition(Color& end_color, uint32_t duration)
////{
////	auto& color_anim = _background_color;
////	color_anim.start = background_color.rgba;
////	color_anim.end = end_color.rgba;
////	color_anim.start_time = nui::frame_start_time;
////	color_anim.end_time = color_anim.start_time + std::chrono::milliseconds(duration);
////	color_anim.blend_func = TransitionBlendFunction::LINEAR;
////}
////
////void BackgroundElement::_init()
////{
////	Element::_initDefaultProperties();
////
////	coloring = BackgroundColoring::NONE;
////	background_color.setRGBA_UNORM();
////
////	_onRenderingSurface = nullptr;
////
////	_rect_render.init(_window);
////	_events._init(_window);
////}
////
////void BackgroundElement::_generateGPU_Data()
////{
////	switch (coloring) {
////	case BackgroundColoring::FLAT_FILL: {
////
////		SteadyTime& now = nui::frame_start_time;
////		background_color.rgba = _background_color.calculate(now);
////
////		_rect_render.reset();
////
////		RectInstance props;
////		props.screen_pos[0] = _position[0];
////		props.screen_pos[1] = _position[1];
////		props.size[0] = _size[0];
////		props.size[1] = _size[1];
////		props.color.rgba = background_color.rgba;
////		_rect_render.addInstance(props);
////
////		_rect_render.generateGPU_Data();
////		break;
////	}
////	}
////}
////
////void BackgroundElement::_draw()
////{
////	switch (coloring) {
////	case BackgroundColoring::FLAT_FILL: {		
////		_rect_render.draw();
////		break;
////	}
////
////	case BackgroundColoring::RENDERING_SURFACE: {
////
////		assert_cond(_onRenderingSurface != nullptr,
////			"RenderingSurface callback not set for BackgroundColoring::RENDERING_SURFACE");
////
////		Instance* instance = _window->instance;
////		ID3D11DeviceContext3* im_ctx3 = instance->im_ctx3.Get();
////
////		//auto clear_bindings = [&]() {
////
////		//	// Input Assembly
////		//	{
////		//		std::array<ID3D11Buffer*, 2> buffs = {
////		//			nullptr, nullptr
////		//		};
////		//		std::array<uint32_t, 2> strides = {
////		//			0, 0
////		//		};
////		//		std::array<uint32_t, 2> offsets = {
////		//			0, 0
////		//		};
////		//		im_ctx3->IASetVertexBuffers(0, buffs.size(), buffs.data(), strides.data(), offsets.data());
////		//	}
////		//};
////
////		im_ctx3->ClearState();
////
////		SurfaceEvent surface_event;
////		surface_event.dev5 = instance->dev5.Get();
////		surface_event.im_ctx3 = im_ctx3;
////
////		surface_event.render_target_width = _window->surface_width;
////		surface_event.render_target_height = _window->surface_height;
////		surface_event.compose_rtv = _window->present_rtv.Get();
////
////		surface_event.viewport_pos = { _position[0], _position[1] };
////		surface_event.viewport_size = { _size[0], _size[1] };
////
////		this->_onRenderingSurface(_window, &(*_self_element), surface_event, _surface_event_user_data);
////
////		im_ctx3->ClearState();
////		break;
////	}
////	}
////}
////
////void BackgroundElement::setRenderingSurfaceEvent(RenderingSurfaceCallback callback, void* user_data)
////{
////	this->_onRenderingSurface = callback;
////	this->_surface_event_user_data = user_data;
////}

bool Flex::_isInside()
{
	int32_t top = _position[1];
	int32_t bot = top + _size[1];
	int32_t left = _position[0];
	int32_t right = left + _size[0];

	Input& input = _window->input;

	return left <= input.mouse_x && input.mouse_x < right &&
		top <= input.mouse_y && input.mouse_y < bot;
}

void Flex::_emitInsideEvents(bool&, bool&)
{
	_events._emitInsideEvents(_self);
}

void Flex::_emitOutsideEvents()
{
	_events._emitOutsideEvents(_self);
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

uint32_t Menu::createItem(uint32_t parent_idx, MenuItemCreateInfo& new_info)
{
	MenuItem& parent = state->items[parent_idx];
	parent.child_items.push_back(state->items.size());

	MenuItem& new_item = state->items.emplace_back();
	new_item.parent = parent_idx;
	new_item.info = new_info;

	return state->items.size() - 1;
}

bool Menu::_isInside()
{
	Input& input = _window->input;

	auto f = [&]() -> void {

		for (int32_t i = state->visible_menus.size() - 1; i >= 0; i--) {

			uint32_t menu_idx = state->visible_menus[i].menu;
			MenuItem& menu = state->items[menu_idx];

			if (menu.menu_box.isInside(input.mouse_x, input.mouse_y)) {

				for (uint32_t item_idx : menu.child_items) {

					MenuItem& item = state->items[item_idx];

					if (item.item_box.isInside(input.mouse_x, input.mouse_y)) {

						// mark item to be hovered
						state->visible_menus[i].item = item_idx;

						// if item leads to submenu that needs to be displayed
						if (item.child_items.size()) {

							// submenu of titles
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
										state->visible_menus[i + 1].menu = item_idx;
										state->visible_menus[i + 1].item = 0xFFFF'FFFF;
									}
								}
								// display submenu if clicked
								else if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].down_transition) {

									state->visible_menus.resize(i + 2);
									state->visible_menus[i + 1].menu = item_idx;
									state->visible_menus[i + 1].item = 0xFFFF'FFFF;
								}
							}
							// display submenu if hovered
							else {
								state->visible_menus.resize(i + 2);
								state->visible_menus[i + 1].menu = item_idx;
								state->visible_menus[i + 1].item = 0xFFFF'FFFF;
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
				return;
			}
		}

		// cursor is not over menu at all

		// close menu because user is interacting with something else
		if (input.key_list[VirtualKeys::LEFT_MOUSE_BUTTON].down_transition) {

			state->visible_menus.resize(1);
			state->visible_menus[0].menu = 0;
			state->visible_menus[0].item = 0xFFFF'FFFF;
		}
		// stop highlighting if out of bounds
		else {
			state->visible_menus.back().item = 0xFFFF'FFFF;
		}
		// else keep displaing menu
	};
	f();

	return false;
}

void Menu::_calcSizeAndRelativeChildPositions()
{
	Instance* inst = _window->instance;

	// Diff the prev and current frame graph to see if they changed
	// if they did then reset visible state
	// else preserve for emit events
	{
		auto is_changed = [&]() -> bool {

			if (state->prev_items.size() != state->items.size()) {
				return true;
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

		state->prev_items = state->items;
	}

	// Menu Titles
	int32_t pen_x = 0;
	int32_t pen_y = 0;
	{
		MenuItem& root = state->items[0];

		for (uint32_t title_idx : root.child_items) {

			MenuItem& title = state->items[title_idx];

			title.item_box.pos = { pen_x, pen_y };

			inst->findAndPositionGlyphs(
				title.info.left_text,
				(int32_t)title.info.left_padding + pen_x, (int32_t)title.info.top_padding + pen_y,
				title.item_box.size[0], title.item_box.size[1],
				title._text.chars
			);

			title._text.color = title.info.left_text.color;

			title.item_box.size[0] += title.info.left_padding + title.info.right_padding;
			title.item_box.size[1] += title.info.top_padding + title.info.bot_padding;

			pen_x += title.item_box.size[0];

			// menu height
			if (title.item_box.size[1] > root.menu_box.size[1]) {
				root.menu_box.size[1] = title.item_box.size[1];
			}
		}

		root.menu_box.size[0] = pen_x;
	}

	// Submenus
	if (state->visible_menus.size() > 1) {

		for (uint32_t visible_idx = 1; visible_idx < state->visible_menus.size(); visible_idx++) {

			MenuVisibleMenus& visible = state->visible_menus[visible_idx];

			MenuItem& menu = state->items[visible.menu];
			uint32_t border_thick = menu.info.menu_border_thickness;

			// position title menu below
			if (visible_idx == 1) {
				pen_x = menu.item_box.pos[0];
				pen_y = menu.item_box.size[1];

				menu.menu_box.pos = { pen_x, pen_y };
			}
			// position submenu to the side
			else {
				pen_x = menu.item_box.pos[0] + menu.item_box.size[0] - border_thick;
				pen_y = menu.item_box.pos[1];

				menu.menu_box.pos = { pen_x, pen_y - (int32_t)border_thick };
			}

			uint32_t max_width = 0;

			// First pass to find out text size
			for (uint32_t item_idx : menu.child_items) {

				MenuItem& item = state->items[item_idx];
				MenuItemCreateInfo& info = item.info;

				item.item_box.pos = { pen_x, pen_y };

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
				item.item_box.size[1] = item_height;

				if (used_item_width > max_width) {
					max_width = used_item_width;
				}

				pen_y += item_height;
			}

			if (visible_idx == 1) {

				// make sure submenu is as wide as parent title
				if (max_width < menu.item_box.size[0]) {
					max_width = menu.item_box.size[0];
				}
			}

			// Second pass
			for (uint32_t item_idx : menu.child_items) {

				MenuItem& item = state->items[item_idx];
				MenuItemCreateInfo& info = item.info;

				item.item_box.size[0] = max_width;

				// Arrow
				item._arrow.screen_pos[0] = pen_x +
					(max_width - border_thick - info.right_padding - info.arrow_width);
			}

			menu.menu_box.size[0] = max_width;
			menu.menu_box.size[1] = pen_y - menu.menu_box.pos[1] + border_thick;

			pen_x += menu.menu_box.size[0];
		}
	}
}

void Menu::_draw()
{
	Instance* inst = _window->instance;

	// Menu titles
	{
		MenuItem& root = state->items[0];
		{
			RectInstance rect_instance;
			rect_instance.pos = _position;
			rect_instance.size = root.menu_box.size;
			rect_instance.color = root.info.menu_background_color;

			std::vector<RectInstance*> instances = { &rect_instance };

			inst->drawRects(_window, instances);
		}

		state->text_instances.clear();

		for (uint32_t title_idx : root.child_items) {

			MenuItem& title = state->items[title_idx];

			// update box position to screen
			title.item_box.pos[0] += _position[0];
			title.item_box.pos[1] += _position[1];

			if (title_idx == state->visible_menus[0].item) {

				RectInstance rect_instance;
				rect_instance.pos = title.item_box.pos;
				rect_instance.size = title.item_box.size;
				rect_instance.color = title.info.background_hover_color;

				std::vector<RectInstance*> rect_instances = { &rect_instance };

				inst->drawRects(_window, rect_instances);

				if (state->visible_menus.size() > 1) {

					BorderInstance border_inst;
					border_inst.screen_pos = title.item_box.pos;
					border_inst.size = title.item_box.size;
					border_inst.thickness = title.info.menu_border_thickness;
					border_inst.color = title.info.menu_border_color;
					border_inst.bot = false;

					std::vector<BorderInstance*> border_instances = { &border_inst };

					inst->drawBorder(_window, border_instances);
				}
			}

			// update character positions to screen
			for (PositionedCharacter& chara : title._text.chars) {

				chara.pos[0] += _position[0];
				chara.pos[1] += _position[1];
			}

			state->text_instances.push_back(&title._text);
		}

		inst->drawTexts(_window, state->text_instances);
	}

	// Submenu
	{
		for (uint32_t visible_idx = 1; visible_idx < state->visible_menus.size(); visible_idx++) {

			MenuVisibleMenus& visible = state->visible_menus[visible_idx];

			MenuItem& menu = state->items[visible.menu];
			int32_t border_thick = menu.info.menu_border_thickness;

			// Menu background
			{
				RectInstance rect_instance;
				rect_instance.pos = {
					menu.menu_box.pos[0],
					menu.menu_box.pos[1]
				};
				rect_instance.size = {
					menu.menu_box.size[0],
					menu.menu_box.size[1]
				};
				rect_instance.color = menu.info.menu_background_color;

				std::vector<RectInstance*> rect_instances = { &rect_instance };

				inst->drawRects(_window, rect_instances);
			}

			// Menu border
			{
				if (visible_idx == 1) {

					BorderInstance border_inst;
					border_inst.screen_pos = {
						menu.menu_box.pos[0],
						menu.menu_box.pos[1]
					};
					border_inst.size = {
						menu.menu_box.size[0],
						menu.menu_box.size[1]
					};
					border_inst.thickness = border_thick;
					border_inst.color = menu.info.menu_border_color;
					border_inst.top = false;

					std::vector<BorderInstance*> border_instances = { &border_inst };
					inst->drawBorder(_window, border_instances);
				}
				else {
					BorderInstance border_inst;
					border_inst.screen_pos = {
						menu.menu_box.pos[0],
						menu.menu_box.pos[1]
					};
					border_inst.size = {
						menu.menu_box.size[0],
						menu.menu_box.size[1]
					};
					border_inst.thickness = border_thick;
					border_inst.color = menu.info.menu_border_color;

					std::vector<BorderInstance*> border_instances = { &border_inst };
					inst->drawBorder(_window, border_instances);
				}
			}

			state->text_instances.clear();
			state->arrow_instances.clear();

			for (uint32_t item_idx : menu.child_items) {

				MenuItem& item = state->items[item_idx];

				// update box position to screen
				item.item_box.pos[0] += _position[0];
				item.item_box.pos[1] += _position[1];

				// Highlight
				if (item_idx == visible.item) {

					RectInstance rect_instance;
					rect_instance.pos = {
						item.item_box.pos[0] + border_thick,
						item.item_box.pos[1]
					};
					rect_instance.size = {
						item.item_box.size[0] - (2 * border_thick),
						item.item_box.size[1]
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
				if (item.child_items.size()) {

					item._arrow.screen_pos[0] += _position[0];
					item._arrow.screen_pos[1] += _position[1];

					state->arrow_instances.push_back(&item._arrow);
				}
			}

			inst->drawTexts(_window, state->text_instances);
			inst->drawArrows(_window, state->arrow_instances);
		}
	}
}

//TreeListCreateInfo::TreeListCreateInfo()
//{
//	ElementCreateInfo::ElementCreateInfo();
//	this->size[0] = 200;
//	this->size[1] = 100.f;
//
//	// Item
//	this->item.padding = {};
//	this->item.text_hover_color = Color::black();
//	this->item.background_hover_color = Color::white();
//	this->item.arrow.left_padding = 0;
//	this->item.arrow.right_padding = 0;
//	this->item.arrow.width = 14;
//	this->item.arrow.height = 14;
//	this->item.arrow.color = Color::white();
//	this->item.arrow.hover_color = Color::black();
//
//	// Layout
//	this->indentation = 14;
//}
//
//TreeListItemHandle::TreeListItemHandle(uint32_t new_item_idx)
//{
//	this->item_idx = new_item_idx;
//}
//
//TreeListItemHandle TreeList::createItem(TreeListItemCreateInfo& new_info)
//{
//	TreeListItemHandle new_handle;
//	new_handle.item_idx = 0;
//	return createItem(new_handle, new_info);
//}
//
//TreeListItemHandle TreeList::createItem(TreeListItemHandle handle, TreeListItemCreateInfo& new_info)
//{
//	uint32_t self_idx = items.size();
//
//	TreeListItem& parent = items[handle.item_idx];
//	parent.children.push_back(self_idx);
//
//	TreeListItem& new_item = items.emplace_back();
//	new_item.treelist = this;
//	new_item.parent = handle.item_idx;
//	new_item.self_idx = self_idx;
//	new_item.expanded = false;
//	new_item.info = new_info;
//
//	TreeListItemHandle new_handle;
//	new_handle.item_idx = self_idx;
//	return new_handle;
//}
//
//void TreeList::_calcSizeAndRelativeChildPositions()
//{
//	assert_cond(size[0].type != ElementSizeType::FIT &&
//		size[1].type != ElementSizeType::FIT,
//		"TreeList does not support fit size");
//
//	Instance* inst = _window->instance;
//
//	// Mark items as un-traversed
//	{
//		for (TreeListItem& item : items) {
//			item._traversed = false;
//		}
//	}
//
//	// Down first pass
//	{
//		_text_instances.clear();
//
//		uint32_t next_item = 0;
//
//		required_size = { 0, 0 };
//
//		int32_t indent_level = 0;
//		int32_t pen_y = 0;
//
//		// render all items on return
//		// if all child items have been traversed
//
//		while (next_item != 0xFFFF'FFFF) {
//
//			TreeListItem& item = items[next_item];
//
//			bool all_children_traversed = true;
//			for (uint32_t child_item_idx : item.children) {
//
//				TreeListItem& child_item = items[child_item_idx];
//
//				if (child_item._traversed == false) {
//
//					child_item._traversed = true;
//					all_children_traversed = false;
//
//					int32_t pen_x = info.indentation * indent_level;
//					uint32_t text_width;
//					uint32_t text_height;
//
//					// Text
//					{
//						inst->findAndPositionGlyphs(child_item.info.text,
//							pen_x + info.item.padding.left,
//							pen_y + info.item.padding.top,
//							text_width, text_height,
//							child_item._text.chars);
//
//						child_item._text.color = child_item.info.text.color;
//
//						_text_instances.push_back(&child_item._text);
//					}
//
//					child_item._label_size = {
//						info.item.padding.left + text_width + info.item.padding.right,
//						info.item.padding.top + text_height + info.item.padding.bot,
//					};
//
//					// TreeList Witdh
//					{
//						uint32_t new_width = pen_x + child_item._label_size[0];
//
//						if (new_width > required_size[0]) {
//							required_size[0] = new_width;
//						}
//					}
//
//					// schedule down
//					indent_level++;
//					next_item = child_item_idx;
//
//					pen_y += child_item._label_size[1];
//					break;
//				}
//			}
//
//			if (all_children_traversed) {
//
//				// schedule up
//				indent_level--;
//				next_item = item.parent;
//			}
//		}
//
//		required_size[1] = pen_y;
//	}
//}
//
//void TreeList::_draw()
//{
//	Instance* inst = _window->instance;
//
//	// render root background
//	// render item background
//
//	/*std::array<int32_t, 2> scroll_offset = { 0, 0 };
//	{
//		TreeListItem& root = items[0];
//
//		uint32_t available_height = _size[1];
//		uint32_t available_width = _size[0];
//
//		if (root._background.size[1] > available_height) {
//			available_width -= info.scroll_bar_thickness;
//		}
//
//		if (root._background.size[0] > available_width) {
//			available_height -= info.scroll_bar_thickness;
//		}
//
//		if (root._background.size[1] > available_height) {
//
//			uint32_t remainder = root._background.size[1] - available_height;
//			scroll_offset[1] = (int32_t)(scroll_factor[1] * (float)remainder);
//		}
//
//		if (root._background.size[0] > available_width) {
//
//			uint32_t remainder = root._background.size[0] - available_width;
//			scroll_offset[0] = (int32_t)(scroll_factor[0] * (float)remainder);
//		}
//	}*/
//
//	// Draw Treelist Background
//	{
//		RectInstance rect_inst;
//		rect_inst.pos = _position;
//		rect_inst.size = _size;
//		rect_inst.color = info.background_color;
//
//		inst->drawRect(_window, &rect_inst);
//	}
//
//	// Draw Hover Item Background
//	{
//		
//	}
//
//	// Render Text
//	{
//		for (TextInstance* text_inst : _text_instances) {
//			for (PositionedCharacter& chara : text_inst->chars) {
//				chara.pos[0] += _position[0];
//				chara.pos[1] += _position[1];
//			}
//		}
//
//		inst->drawTexts(_window, _text_instances);
//	}
//}
