#include "pch.h"

// Header
#include "NuiLibrary.hpp"


using namespace nui;


ElementSize& ElementSize::operator=(uint32_t size_px)
{
	this->type = ElementSizeType::ABSOLUTE_SIZE;
	this->size = (float)size_px;
	return *this;
}

ElementSize& ElementSize::operator=(float percentage)
{
	this->type = ElementSizeType::RELATIVE_SIZE;
	this->size = percentage / 100.0f;
	return *this;
}

void ElementSize::setAbsolute(float size_px)
{
	this->type = ElementSizeType::ABSOLUTE_SIZE;
	this->size = size_px;
}

Color::Color() {}

Color::Color(int32_t red, int32_t green, int32_t blue, uint8_t alpha)
{
	this->rgba.r = (float)(red / 255);
	this->rgba.g = (float)(green / 255);
	this->rgba.b = (float)(blue / 255);
	this->rgba.a = (float)(alpha / 255);
}

Color::Color(double red, double green, double blue, double alpha)
{
	this->rgba.r = (float)red;
	this->rgba.g = (float)green;
	this->rgba.b = (float)blue;
	this->rgba.a = (float)alpha;
}

Color::Color(float red, float green, float blue, float alpha)
{
	this->rgba.r = red;
	this->rgba.g = green;
	this->rgba.b = blue;
	this->rgba.a = alpha;
}

Color::Color(int32_t hex)
{
	// white = 0x00'FF'FF'FF'FF
	float red = float(hex >> 16 & 0xFF);
	float green = float(hex >> 8 & 0xFF);
	float blue = float(hex & 0xFF);

	this->rgba.r = red / 255;
	this->rgba.g = green / 255;
	this->rgba.b = blue / 255;
	this->rgba.a = 1.0f;
}

Color Color::red()
{
	return Color(1.0f, 0.0f, 0.0f);
}

Color Color::green()
{
	return Color(0.0f, 1.0f, 0.0f);
}

Color Color::blue()
{
	return Color(0.0f, 0.0f, 1.0f);
}

Color Color::black()
{
	return Color(0.0f, 0.0f, 0.0f);
}

Color Color::white()
{
	return Color(1.0f, 1.0f, 1.0f);
}

Color Color::cyan()
{
	return Color(0.0f, 1.0f, 1.0f);
}

Color Color::magenta()
{
	return Color(1.0f, 0.0f, 1.0f);
}

Color Color::yellow()
{
	return Color(1.0f, 1.0f, 0.0f);

}

void Color::setRGBA_UNORM(float r, float g, float b, float a)
{
	this->rgba.r = r;
	this->rgba.g = g;
	this->rgba.b = b;
	this->rgba.a = a;
}

Text* NodeComponent::addText()
{
	Node& child_node = window->nodes.emplace_back();
	child_node.event_comp.create(this->window, &child_node);

	Text* new_text = child_node.createText();
	new_text->_node_comp.window = this->window;
	new_text->_node_comp.this_elem = &child_node;

	// User
	new_text->pos = { 0, 0 };
	new_text->size = 14;
	new_text->line_height = 0;
	new_text->color.rgba = { 1, 1, 1, 1 };

	// Parent ---> Child
	this->this_elem->children.push_back(&child_node);

	// Parent <--- Child
	child_node.layer_idx = this->this_elem->layer_idx + 1;
	child_node.parent = this->this_elem;

	return new_text;
}

Wrap* NodeComponent::addWrap()
{
	Node& child_node = window->nodes.emplace_back();
	child_node.event_comp.create(this->window, &child_node);

	Wrap* child_wrap = child_node.createWrap();
	child_wrap->_node_comp.window = this->window;
	child_wrap->_node_comp.this_elem = &child_node;

	child_wrap->pos = { 0, 0 };
	child_wrap->width.type = ElementSizeType::FIT;
	child_wrap->height.type = ElementSizeType::FIT;
	child_wrap->overflow = Overflow::OVERFLOW;
	child_wrap->background_color.rgba = { 0, 0, 0, 0 };

	// Parent ---> Child
	this->this_elem->children.push_back(&child_node);

	// Parent <--- Child
	child_node.layer_idx = this->this_elem->layer_idx + 1;
	child_node.parent = this->this_elem;

	return child_wrap;
}

Flex* NodeComponent::addFlex()
{
	Node& child_node = window->nodes.emplace_back();

	Flex* child_flex = child_node.createFlex();
	child_flex->node_comp.window = this->window;

	return child_flex;
}

Surface* NodeComponent::addSurface()
{
	Node& child_node = window->nodes.emplace_back();
	child_node.event_comp.create(this->window, &child_node);
	child_node.layer_idx = this->this_elem->layer_idx + 1;

	Surface* child_surface = child_node.createSurface();
	child_surface->_node_comp.window = this->window;
	child_surface->_node_comp.this_elem = &child_node;
	child_surface->_event.surface = child_surface;

	child_surface->pos = { 0, 0 };
	child_surface->width = 100.0f;
	child_surface->height = 100.0f;
	child_surface->overflow = Overflow::OVERFLOW;
	child_surface->callback = nullptr;

	// Parent ---> Child
	this->this_elem->children.push_back(&child_node);

	// Parent <--- Child	
	child_node.parent = this->this_elem;

	return child_surface;
}

Text* Wrap::addText()
{
	return _node_comp.addText();
}

Wrap* Wrap::addWrap()
{
	return _node_comp.addWrap();
}

Surface* Wrap::addSurface()
{
	return _node_comp.addSurface();
}

void Wrap::setOnMouseEnterEvent(MouseEnterCallback callback, void* user_data)
{
	_node_comp.this_elem->event_comp.setMouseEnterEvent(callback, user_data);
}

void Wrap::setOnMouseHoverEvent(MouseHoverCallback callback, void* user_data)
{
	_node_comp.this_elem->event_comp.setMouseHoverEvent(callback, user_data);
}

void Wrap::setOnMouseLeaveEvent(MouseLeaveCallback callback, void* user_data)
{
	_node_comp.this_elem->event_comp.setMouseLeaveEvent(callback, user_data);
}

void Wrap::setOnMouseMoveEvent(MouseMoveCallback callback, void* user_data)
{
	_node_comp.this_elem->event_comp.setMouseMoveEvent(callback, user_data);
}

bool Wrap::addKeyDownEvent(KeyDownCallback callback, uint32_t key, void* user_ptr)
{
	return _node_comp.this_elem->event_comp.addKeyDownEvent(callback, key, user_ptr);
}

bool Wrap::removeKeyDownEvent(uint32_t key)
{
	return _node_comp.this_elem->event_comp.removeKeyDownEvent(key);
}

void Text::setOnMouseEnterEvent(MouseEnterCallback callback, void* user_data)
{
	_node_comp.this_elem->event_comp.setMouseEnterEvent(callback, user_data);
}

void Text::setOnMouseHoverEvent(MouseHoverCallback callback, void* user_data)
{
	_node_comp.this_elem->event_comp.setMouseHoverEvent(callback, user_data);
}

void Text::setOnMouseLeaveEvent(MouseLeaveCallback callback, void* user_data)
{
	_node_comp.this_elem->event_comp.setMouseLeaveEvent(callback, user_data);
}

void Text::setOnMouseMoveEvent(MouseMoveCallback callback, void* user_data)
{
	_node_comp.this_elem->event_comp.setMouseMoveEvent(callback, user_data);
}

bool Text::addKeyDownEvent(KeyDownCallback callback, uint32_t key, void* user_ptr)
{
	return _node_comp.this_elem->event_comp.addKeyDownEvent(callback, key, user_ptr);
}

bool Text::removeKeyDownEvent(uint32_t key)
{
	return _node_comp.this_elem->event_comp.removeKeyDownEvent(key);
}

Root* Node::createRoot()
{
	return std::get_if<Root>(&elem);
}

Wrap* Node::createWrap()
{
	return &elem.emplace<Wrap>();
}

Flex* Node::createFlex()
{
	return &elem.emplace<Flex>();
}

Text* Node::createText()
{
	return &elem.emplace<Text>();
}

Surface* Node::createSurface()
{
	return &elem.emplace<Surface>();
}

Wrap* Window::addWrap()
{
	Root* root_wrap = std::get_if<Root>(&nodes.front().elem);

	return root_wrap->node_comp.addWrap();
}

Text* Window::addText()
{
	Root* root_wrap = std::get_if<Root>(&nodes.front().elem);

	return root_wrap->node_comp.addText();
}

Surface* Window::addSurface()
{
	Root* root_wrap = std::get_if<Root>(&nodes.front().elem);

	return root_wrap->node_comp.addSurface();
}