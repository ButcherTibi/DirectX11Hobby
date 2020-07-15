#pragma once

// Mine
#include "Internals.h"


namespace nui {

	class Nui {
	public:
		nui_int::Internals internals;
		
	public:
		ErrStack create(HWND hwnd);

		Element& getRoot();
		Flex& getRootElement();

		template<typename T>
		Element& addElement(Element& parent, T& new_elem);

		ErrStack draw();
	};
}
