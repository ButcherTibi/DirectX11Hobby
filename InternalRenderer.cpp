
// Header
#include "Internals.h"


using namespace nui_int;
using namespace nui;


ErrStack Internals::create(HWND hwnd, HINSTANCE hinstance)
{
	ErrStack err_stack{};
	
	// Draw Calls
	// for each layer in layers
	//   for each elem in layer
	//     draw border rect verts
	//     draw border circle verts, set mask R to used or not, set G to antialising amount
	//     draw padding rect verts
	//     draw padding circle verts, set mask R to used or not, set G to antialising amount
	//     -------------------------
	//     draw composite: blend/overwrite between border and padding using masks into 
	//     resulting composite

	return err_stack;
}

ErrStack Internals::draw()
{
	ErrStack err_stack{};



	return err_stack;
}

ErrStack Internals::generateGPU_Data()
{
	ErrStack err_stack{};

	return err_stack;
}
