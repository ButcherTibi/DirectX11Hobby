#include "pch.h"

// Header
#include "NodeComp.hpp"

#include "NuiLibrary.hpp"


using namespace nui;


void NodeComp::_create(Window* w, Node* n)
{
	this->window = w;
	this->node = n;
}

Text* NodeComp::addText()
{
	Node& child_node = window->nodes.emplace_back();

	Text* new_text = child_node.createText();
	new_text->EventComp::_create(this->window, &child_node);
	//new_text->NodeComp::_create(this->window, &child_node);

	// User
	new_text->pos = { 0, 0 };
	new_text->size = 14;
	new_text->line_height = 0;
	new_text->color.rgba = { 1, 1, 1, 1 };

	// Parent ---> Child
	this->node->children.push_back(&child_node);

	// Parent <--- Child
	child_node.layer_idx = this->node->layer_idx + 1;
	child_node.parent = this->node;

	return new_text;
}

Wrap* NodeComp::addWrap()
{
	Node& child_node = window->nodes.emplace_back();

	Wrap* child_wrap = child_node.createWrap();
	child_wrap->EventComp::_create(this->window, &child_node);
	child_wrap->NodeComp::_create(this->window, &child_node);

	child_wrap->pos = { 0, 0 };
	child_wrap->width.type = ElementSizeType::FIT;
	child_wrap->height.type = ElementSizeType::FIT;
	child_wrap->overflow = Overflow::OVERFLOW;
	child_wrap->background_color.rgba = { 0, 0, 0, 0 };

	// Parent ---> Child
	this->node->children.push_back(&child_node);

	// Parent <--- Child
	child_node.layer_idx = this->node->layer_idx + 1;
	child_node.parent = this->node;

	return child_wrap;
}

Surface* NodeComp::addSurface()
{
	Node& child_node = window->nodes.emplace_back();

	Surface* child_surface = child_node.createSurface();
	child_surface->EventComp::_create(this->window, &child_node);
	child_surface->NodeComp::_create(this->window, &child_node);
	child_surface->_event.surface = child_surface;

	child_surface->pos = { 0, 0 };
	child_surface->width = 100.0f;
	child_surface->height = 100.0f;
	child_surface->gpu_callback = nullptr;

	// Parent ---> Child
	this->node->children.push_back(&child_node);

	// Parent <--- Child
	child_node.layer_idx = this->node->layer_idx + 1;
	child_node.parent = this->node;

	return child_surface;
}