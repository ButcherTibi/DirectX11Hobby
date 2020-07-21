
// Header
#include "Internals.h"


using namespace nui_int;
using namespace nui;


ErrStack Internals::create(HWND hwnd, HINSTANCE hinstance)
{
	ErrStack err_stack{};
	
	return err_stack;
}

ErrStack Internals::draw()
{
	ErrStack err_stack{};
	HRESULT hr{};

	
	UserInterface& ui = user_interface;

	ui.calcGraphLayout();
	checkErrStack1(generateGPU_Data());

	

	return err_stack;
}


ErrStack Internals::generateGPU_Data()
{
	ErrStack err_stack{};

	return err_stack;
}
