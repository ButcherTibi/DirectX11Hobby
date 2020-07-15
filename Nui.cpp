
// Header
#include "Nui.h"


using namespace nui;
using namespace nui_int;


ErrStack Nui::create(HWND hwnd)
{
	return this->internals.create(hwnd);
}

Element& Nui::getRoot()
{
	return this->internals.user_interface.getRoot();
}

Flex& Nui::getRootElement()
{
	return this->internals.user_interface.getRootElement();
}

template<typename T>
Element& Nui::addElement(Element& parent, T& new_elem)
{
	return this->internals.user_interface.addElement(parent, new_elem);
}
template Element& Nui::addElement(Element& parent, Flex& new_elem);

ErrStack Nui::draw()
{
	return this->internals.draw();
}
