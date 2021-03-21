// Header
#include "NuiLibrary.hpp"

// GLM
#include "glm\common.hpp"

// Mine
#include "BasicMath.h"


using namespace nui;


template<typename T>
bool AnimatedProperty<T>::isAnimated()
{
	return start_time != end_time;
}
template bool AnimatedProperty<float>::isAnimated();
template bool AnimatedProperty<glm::vec4>::isAnimated();

template<typename T>
T AnimatedProperty<T>::calculate(SteadyTime& now)
{
	switch (blend_func) {
	case TransitionBlendFunction::LINEAR: {
		if (end_time < now) {
			end_time = start_time;  // animation ended
			return end;
		}
		else {
			float elapsed = fsec_cast(now - start_time);
			float total = fsec_cast(end_time - start_time);

			return glm::mix(start, end, elapsed / (total));
		}
		break;
	}
	}

	throw std::exception();
	return T();
}
template float AnimatedProperty<float>::calculate(SteadyTime& now);
template glm::vec4 AnimatedProperty<glm::vec4>::calculate(SteadyTime& now);

void Element::setMouseEnterEvent(EventCallback callback, void* user_data)
{
	this->_onMouseEnter = callback;
	this->_mouse_enter_user_data = user_data;
}

void Element::setMouseHoverEvent(MouseHoverCallback callback, void* user_data)
{
	this->_onMouseHover = callback;
	this->_mouse_hover_user_data = user_data;
}

void Element::setMouseMoveEvent(EventCallback callback, void* user_data)
{
	this->_onMouseMove = callback;
	this->_mouse_move_user_data = user_data;
}

void Element::setMouseScrollEvent(EventCallback callback, void* user_data)
{
	this->_onMouseScroll = callback;
	this->_mouse_scroll_user_data = user_data;
}

void Element::setMouseLeaveEvent(EventCallback callback, void* user_data)
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

void Element::setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data)
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

void Element::setKeysDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, void* user_data)
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

void Element::setKeysDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, uint32_t key_2, void* user_data)
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

void Element::setKeyHeldDownEvent(EventCallback callback, uint32_t key, void* user_data)
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

void Element::setKeysHeldDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, void* user_data)
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

void Element::setKeysHeldDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, uint32_t key_2, void* user_data)
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

void Element::setKeyUpEvent(EventCallback callback, uint32_t key, void* user_data)
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

void Element::_init()
{
	for (uint8_t i = 0; i < 2; i++) {
		origin[i] = 0.f;
		relative_position[i] = 0;
		size[i].type = ElementSizeType::FIT;
	}

	z_index = 0;

	// Internal
	_mouse_event_state = MouseEventState::OUTSIDE;

	_onMouseEnter = nullptr;
	_onMouseHover = nullptr;
	_onMouseMove = nullptr;
	_onMouseScroll = nullptr;
	_onMouseLeave = nullptr;

	_events_emited = false;
}

void Element::_emitInsideEvents()
{
	SteadyTime& now = _window->instance->frame_start_time;

	// if mouse was previously outside
	if (_mouse_event_state == MouseEventState::OUTSIDE) {

		_mouse_enter_time = now;

		if (_onMouseEnter != nullptr) {
			this->_onMouseEnter(_window, _self, _mouse_enter_user_data);
		}
	}

	if (_onMouseHover != nullptr) {
		this->_onMouseHover(_window, _self, fsec_cast(now - _mouse_enter_time), _mouse_hover_user_data);
	}

	if (_onMouseMove != nullptr) {
		if (_window->input.mouse_delta_x || _window->input.mouse_delta_y) {
			this->_onMouseMove(_window, _self, _mouse_move_user_data);
		}
	}

	if (_onMouseScroll != nullptr) {
		if (_window->input.mouse_wheel_delta) {
			this->_onMouseScroll(_window, _self, _mouse_scroll_user_data);
		}
	}

	// Keys Down
	for (Shortcut1KeyCallback& shortcut : _key_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key];

		if (key_0.is_down && key_0.down_transition) {
			shortcut.callback(_window, _self, shortcut.user_data);
		}
	}

	for (Shortcut2KeysCallback& shortcut : _keys_2_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key_0];
		KeyState& key_1 = _window->input.key_list[shortcut.key_1];

		if (key_0.is_down && key_1.is_down &&
			key_0.getDuration_ms() > key_1.getDuration_ms() &&
			key_1.down_transition)
		{
			shortcut.callback(_window, _self, shortcut.user_data);
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
			shortcut.callback(_window, _self, shortcut.user_data);
		}
	}

	// Keys Held Down
	for (Shortcut1KeyCallback& shortcut : _key_held_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key];

		if (key_0.is_down) {
			shortcut.callback(_window, _self, shortcut.user_data);
		}
	}

	for (Shortcut2KeysCallback& shortcut : _keys_2_held_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key_0];
		KeyState& key_1 = _window->input.key_list[shortcut.key_1];

		if (key_0.is_down && key_1.is_down &&
			key_0.getDuration_ms() > key_1.getDuration_ms())
		{
			shortcut.callback(_window, _self, shortcut.user_data);
		}
	}

	for (Shortcut3KeysCallback& shortcut : _keys_3_held_downs) {

		KeyState& key_0 = _window->input.key_list[shortcut.key_0];
		KeyState& key_1 = _window->input.key_list[shortcut.key_1];
		KeyState& key_2 = _window->input.key_list[shortcut.key_2];

		if (key_0.is_down && key_1.is_down && key_2.is_down &&
			key_0.getDuration_ms() > key_1.getDuration_ms() && key_1.getDuration_ms() > key_2.getDuration_ms())
		{
			shortcut.callback(_window, _self, shortcut.user_data);
		}
	}

	// Keys UP
	for (Shortcut1KeyCallback& shortcut : _key_ups) {

		KeyState& key = _window->input.key_list[shortcut.key];

		if (!key.is_down && key.up_transition) {
			shortcut.callback(_window, _self, shortcut.user_data);
		}
	}

	_mouse_event_state = MouseEventState::INSIDE;

	//	switch (mouse_delta_state) {
	//	// Mouse Delta Start
	//	case nui::MouseDeltaState::START: {
	//
	//		// window->trapLocalMousePosition(source->collider);
	//
	//		mouse_delta_state = MouseDeltaState::NOW;
	//		break;
	//	}
	//
	//	// Mouse Delta
	//	case nui::MouseDeltaState::NOW: {
	//
	//		Input& input = window->input;
	//		// BoundingBox2D<uint32_t>& trap = source->collider;
	//
	//		if (input.mouse_y == trap.y0) {
	//			window->setLocalMousePosition(input.mouse_x, trap.y1 - 2);
	//		}
	//		else if (input.mouse_y == trap.y1 - 1) {
	//			window->setLocalMousePosition(input.mouse_x, trap.y0 + 1);
	//		}
	//		else if (input.mouse_x == trap.x0) {
	//			window->setLocalMousePosition(trap.x1 - 2, input.mouse_y);
	//		}
	//		else if(input.mouse_x == trap.x1 - 1) {
	//			window->setLocalMousePosition(trap.x0 + 1, input.mouse_y);
	//		}
	//		break;
	//	}
	//
	//	// Mouse Delta End
	//	case nui::MouseDeltaState::END: {
	//
	//		_endMouseDelta();
	//		break;
	//	}
	//	}
}

void Element::_emitOutsideEvents()
{
	// was inside but now is outside
	if (_mouse_event_state == MouseEventState::INSIDE) {

		if (_onMouseLeave != nullptr) {
			this->_onMouseLeave(_window, _self, _mouse_leave_user_data);
		}

		_mouse_event_state = MouseEventState::OUTSIDE;
	}

	// do nothing if already outside
}

void Element::_calcFirstPassSize(uint32_t axis)
{
	switch (size[axis].type) {
	case ElementSizeType::RELATIVE: {
		Element* parent = getElementBase(_parent);
		_size[axis] = std::lroundf(parent->_size[axis] * size[axis].relative_size);
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
}

void BackgroundElement::setColorTransition(Color& end_color, uint32_t idx, uint32_t duration)
{
	if (idx >= colors.size()) {
		throw std::exception("color at index does not exist");
	}

	auto& color_anim = _anim.colors[idx].color;
	color_anim.start = colors[idx].color.rgba;
	color_anim.end = end_color.rgba;
	color_anim.start_time = _window->instance->frame_start_time;
	color_anim.end_time = color_anim.start_time + std::chrono::milliseconds(duration);
	color_anim.blend_func = TransitionBlendFunction::LINEAR;
}

void BackgroundElement::setColorPositionTransition(float end_position, uint32_t idx, uint32_t duration)
{
	if (idx >= colors.size()) {
		throw std::exception("color at index does not exist");
	}

	auto& color_anim = _anim.colors[idx].pos;
	color_anim.start = colors[idx].pos;
	color_anim.end = end_position;
	color_anim.start_time = _window->instance->frame_start_time;
	color_anim.end_time = color_anim.start_time + std::chrono::milliseconds(duration);
	color_anim.blend_func = TransitionBlendFunction::LINEAR;
}

void BackgroundElement::setGradientAngleTransition(float end_gradient, uint32_t duration)
{
	auto& gradient = _anim.gradient_angle;
	gradient.start = gradient_angle;
	gradient.end = end_gradient;
	gradient.start_time = _window->instance->frame_start_time;
	gradient.end_time = gradient.start_time + std::chrono::milliseconds(duration);
	gradient.blend_func = TransitionBlendFunction::LINEAR;
}

void BackgroundElement::_generateGPU_Data(uint32_t& rect_verts_idx, uint32_t& rect_idxs_idx)
{
	if (coloring != BackgroundColoring::NONE) {

		SteadyTime& now = _window->instance->frame_start_time;

		// Drawcall
		_drawcall.vertex_start_idx = rect_verts_idx;
		_drawcall.index_start_idx = rect_idxs_idx;

		// Top Left
		GPU_RectVertex* vertex = &_window->rect_verts[rect_verts_idx];
		vertex->pos = toXM(0, 0);

		// Top Right
		vertex = &_window->rect_verts[rect_verts_idx + 1];
		vertex->pos = toXM(_size[0], 0);

		// Bot Right
		vertex = &_window->rect_verts[rect_verts_idx + 2];
		vertex->pos = toXM(_size[0], _size[1]);

		// Bot Left
		vertex = &_window->rect_verts[rect_verts_idx + 3];
		vertex->pos = toXM(0, _size[1]);

		// Tesselation 0 to 2
		_window->rect_idxs[rect_idxs_idx + 0] = rect_verts_idx + 0;
		_window->rect_idxs[rect_idxs_idx + 1] = rect_verts_idx + 1;
		_window->rect_idxs[rect_idxs_idx + 2] = rect_verts_idx + 2;

		_window->rect_idxs[rect_idxs_idx + 3] = rect_verts_idx + 2;
		_window->rect_idxs[rect_idxs_idx + 4] = rect_verts_idx + 3;
		_window->rect_idxs[rect_idxs_idx + 5] = rect_verts_idx + 0;

		// Rect Instance
		switch (coloring) {
		case BackgroundColoring::FLAT_FILL: {
			_colors[0] = toXM(colors[0].color.rgba);
			break;
		}

		case BackgroundColoring::LINEAR_GRADIENT: {

			float w = (float)_size[0];
			float h = (float)_size[1];

			_center.x = w / 2;
			_center.y = h / 2;

			BackgroundAnimated& anim = _anim;

			for (uint32_t i = 0; i < colors.size(); i++) {

				ColorStep& step = colors[i];
				ColorStepAnimated& step_anim = anim.colors[i];

				if (step_anim.color.isAnimated()) {
					step.color.rgba = step_anim.color.calculate(now);
				}

				if (step_anim.pos.isAnimated()) {
					step.pos = step_anim.pos.calculate(now);
				}
			}

			if (anim.gradient_angle.isAnimated()) {
				gradient_angle = anim.gradient_angle.calculate(now);
			}

			// GPU values
			for (uint32_t i = 0; i < colors.size(); i++) {
				_colors[i] = toXM(colors[i].color.rgba);
				_color_lenghts[i] = colors[i].pos;
			}

			_gradient_angle = toRadians(gradient_angle);
			break;
		}
		}

		rect_verts_idx += 4;
		rect_idxs_idx += 6;

		// Drawcall
		_drawcall.vertex_count = rect_verts_idx - _drawcall.vertex_start_idx;
		_drawcall.index_count = rect_idxs_idx - _drawcall.index_start_idx;
	}
}

void BackgroundElement::_draw()
{
	Instance* instance = _window->instance;
	Window* w = _window;
	ID3D11DeviceContext3* im_ctx3 = _window->instance->im_ctx3.Get();

	if (coloring == BackgroundColoring::NONE) {
		return;
	}

	// Drawcall Buffer
	w->rect_dbuff.setFloat4(GPU_RectDrawcallFields::POSITION_SIZE,
		(float)_position[0], (float)_position[1], (float)_size[0], (float)_size[1]);

	for (uint32_t i = 0; i < 4; i++) {
		w->rect_dbuff.setFloat4Array(GPU_RectDrawcallFields::COLORS, i, _colors[i]);
		w->rect_dbuff.setFloat4Array(GPU_RectDrawcallFields::COLOR_LENGHTS, i, _color_lenghts[i]);
	}

	w->rect_dbuff.setFloat(GPU_RectDrawcallFields::GRADIENT_ANGLE, _gradient_angle);

	// Input Assembly
	im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	im_ctx3->IASetInputLayout(instance->rect_input_layout.Get());

	std::array<ID3D11Buffer*, 1> vertex_buffs = {
		w->rect_vbuff.get()
	};
	std::array<uint32_t, 1> strides = {
		sizeof(GPU_RectVertex)
	};
	std::array<uint32_t, 1> offsets = {
		0
	};

	im_ctx3->IASetVertexBuffers(0, vertex_buffs.size(), vertex_buffs.data(), strides.data(), offsets.data());
	im_ctx3->IASetIndexBuffer(w->rect_idxbuff.get(), DXGI_FORMAT_R32_UINT, 0);

	// Vertex Shader
	std::array<ID3D11Buffer*, 1> vs_cbuffs = {
		w->cbuff.get()
	};

	im_ctx3->VSSetConstantBuffers(0, vs_cbuffs.size(), vs_cbuffs.data());

	im_ctx3->VSSetShader(instance->rect_vs.Get(), nullptr, 0);

	// Rasterizer
	im_ctx3->RSSetViewports(1, &w->viewport);

	im_ctx3->RSSetState(instance->solid_back_rs.Get());

	// Pixel Shader
	std::array<ID3D11Buffer*, 1> ps_cbuffs = {
		nullptr
	};
	im_ctx3->PSSetConstantBuffers(0, ps_cbuffs.size(), ps_cbuffs.data());

	ps_cbuffs[0] = w->rect_dbuff.get();
	im_ctx3->PSSetConstantBuffers(0, ps_cbuffs.size(), ps_cbuffs.data());

	switch (coloring) {
	case BackgroundColoring::FLAT_FILL: {
		im_ctx3->PSSetShader(instance->rect_flat_fill_ps.Get(), nullptr, 0);
		break;
	}
	case BackgroundColoring::LINEAR_GRADIENT: {
		im_ctx3->PSSetShader(instance->rect_gradient_linear_ps.Get(), nullptr, 0);
		break;
	}
	}

	// Output Merger
	std::array<float, 4> blend_factor = {
		0, 0, 0, 0
	};

	im_ctx3->OMSetBlendState(instance->blend_state.Get(), blend_factor.data(), 0xFFFF'FFFF);

	std::array<ID3D11RenderTargetView*, 1> srv = {
		w->present_rtv.Get()
	};

	im_ctx3->OMSetRenderTargets(srv.size(), srv.data(), nullptr);

	// Draw
	dx11::IndexedDrawcallParams& params = _drawcall;
	im_ctx3->DrawIndexed(params.index_count, params.index_start_idx, 0);
}

Text* Window::createText(Element* parent)
{
	if (parent == nullptr) {
		parent = std::get_if<Root>(&elements.front());
	}

	StoredElement& new_stored_elem = elements.emplace_back();

	auto& new_text = new_stored_elem.emplace<Text>();
	new_text._window = this;
	new_text._parent = parent->_self;
	new_text._self = &new_stored_elem;
	new_text._init();

	// Text Specific
	new_text.font_family = "Roboto";
	new_text.font_style = "Regular";
	new_text.font_size = 14;
	new_text.line_height = 0;
	new_text.color.rgba = { 1, 1, 1, 1 };

	parent->_children.insert(&new_stored_elem);
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
	new_rel._parent = parent_element->_self;
	new_rel._self = &new_stored_elem;
	new_rel._init();

	// Div Specific
	new_rel.coloring = BackgroundColoring::NONE;

	new_rel.colors.resize(2);
	for (ColorStep& step : new_rel.colors) {
		step.color.setRGBA_UNORM(0, 0, 0, 0);
	}
	new_rel.colors[0].pos = 0.0;
	new_rel.colors[1].pos = 1.0;

	for (uint8_t i = 2; i < new_rel.colors.size(); i++) {
		new_rel.colors[i].pos = 0;
	}

	new_rel.gradient_angle = 0;

	// Internal

	parent_element->_children.insert(&new_stored_elem);

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
	new_grid._parent = parent_element->_self;
	new_grid._self = &new_stored_elem;
	new_grid._init();

	new_grid.orientation = GridOrientation::ROW;
	new_grid.items_spacing = GridSpacing::START;
	new_grid.lines_spacing = GridSpacing::START;

	parent_element->_children.insert(&new_stored_elem);

	return &new_grid;
}

void Window::setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data)
{
	Root* root = std::get_if<Root>(&elements.front());
	root->setKeyDownEvent(callback, key, user_data);
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
