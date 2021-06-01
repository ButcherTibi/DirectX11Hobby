
// Header
#include "NuiLibrary.hpp"


using namespace nui;


//#pragma warning(4701 : disable)
void Window::_updateCPU()
{
	// not here not good
	std::vector<Element*> leafs;
	{
		for (StoredElement& stored_elem : elements) {
			
			Element* elem = getElementBase(&stored_elem);
			if (elem->_children.size() == 0) {
				leafs.push_back(elem);
			}
		}
	}

	// Events
	{
		bool emit_inside_events = true;

		for (auto i = draw_stacks.rbegin(); i != draw_stacks.rend(); ++i) {

			std::list<Element*>& elems = i->second;

			for (auto j = elems.rbegin(); j != elems.rend(); ++j) {

				Element* elem = *j;
				elem->_emitEvents(emit_inside_events);
			}
		}
	}

	// Top Down Pass for elements that depend on parents
	{
		std::vector<Element*> now_elems;

		Element* root = std::get_if<Root>(&elements.front());
		root->_size[0] = surface_width;
		root->_size[1] = surface_height;

		for (Element* child : root->_children) {
			now_elems.push_back(child);
		}

		std::vector<Element*> next_elems;

		while (now_elems.size()) {

			for (Element* now_elem : now_elems) {

				now_elem->_calcSizeRelativeToParent();

				for (Element* child : now_elem->_children) {
					next_elems.push_back(child);
				}
			}

			now_elems.swap(next_elems);
			next_elems.clear();
		}
	}

	// Down Up Pass for elements that depend on children (starts from leafs)
	{
		std::unordered_set<Element*> now_elems;
		now_elems.reserve(leafs.size());

		for (Element* leaf : leafs) {
			now_elems.insert(leaf);
		}

		std::unordered_set<Element*> next_elems;

		while (now_elems.size()) {

			for (Element* now_elem : now_elems) {

				now_elem->_calcSizeAndRelativeChildPositions(next_elems);
			}

			now_elems.swap(next_elems);
			next_elems.clear();
		}
	}

	// Convert local to parent positions to screen relative
	// And generate GPU Data
	{
		draw_stacks.clear();

		struct PassedElement {
			std::array<int32_t, 2> ancestor_pos;
			int32_t ancestor_z_position;
			uint32_t ancestor_z_index;
			Element* elem;
		};

		std::vector<PassedElement> now_pelems;
		{
			Root* root = std::get_if<Root>(&elements.front());

			for (Element* child : root->_children) {

				PassedElement& passed_child = now_pelems.emplace_back();
				passed_child.ancestor_pos = { 0, 0 };
				passed_child.ancestor_z_position = 0;
				passed_child.ancestor_z_index = 1;
				passed_child.elem = child;
			}
		}

		std::vector<PassedElement> next_pelems;

		while (now_pelems.size()) {

			for (PassedElement& now_elem : now_pelems) {

				// convert position to screen space
				Element* elem = now_elem.elem;
				elem->_position[0] += now_elem.ancestor_pos[0];
				elem->_position[1] += now_elem.ancestor_pos[1];
				elem->_generateGPU_Data();

				int32_t z_position;
				if (elem->z_index == 0) {
					elem->_z_index = now_elem.ancestor_z_index;
					z_position = now_elem.ancestor_z_position + 1;
				}
				else {
					elem->_z_index = elem->z_index;
					z_position = 0;
				}

				draw_stacks[elem->_z_index].push_back(elem);

				for (Element* child : elem->_children) {

					PassedElement& next_elem = next_pelems.emplace_back();
					next_elem.ancestor_pos = elem->_position;
					next_elem.ancestor_z_position = z_position;
					next_elem.ancestor_z_index = elem->_z_index;
					next_elem.elem = child;
				}
			}

			now_pelems.swap(next_pelems);
			next_pelems.clear();
		}
	}

	// Mouse Delta Trap
	if (delta_owner_elem != nullptr) {

		// Trap the Mouse position
		int32_t local_top = delta_owner_elem->_position[1];
		int32_t local_bot = local_top + delta_owner_elem->_size[1];
		int32_t local_left = delta_owner_elem->_position[0];
		int32_t local_right = local_left + delta_owner_elem->_size[0];

		RECT client_rect = getClientRectangle();
		RECT new_trap;
		new_trap.top = client_rect.top + local_top;
		new_trap.bottom = client_rect.top + local_bot;
		new_trap.left = client_rect.left + local_left;
		new_trap.right = client_rect.left + local_right;

		ClipCursor(&new_trap);

		switch (delta_effect) {
		case DeltaEffectType::LOOP: {
			POINT mouse_screen_pos;
			GetCursorPos(&mouse_screen_pos);

			if (mouse_screen_pos.y <= new_trap.top) {
				setLocalMousePosition(input.mouse_x, local_bot - 1);
			}
			else if (mouse_screen_pos.y >= new_trap.bottom - 1) {
				setLocalMousePosition(input.mouse_x, local_top + 1);
			}
			else if (mouse_screen_pos.x <= new_trap.left) {
				setLocalMousePosition(local_right + 1, input.mouse_y);
			}
			else if (mouse_screen_pos.x >= new_trap.right - 1) {
				setLocalMousePosition(local_left + 1, input.mouse_y);
			}
			break;
		}

		case DeltaEffectType::HIDDEN: {
			setMouseVisibility(false);
			break;
		}
		}
	}

	// Constant buffer data
	{
		glm::ivec2 screen_size = { surface_width, surface_height };
		gpu_constants.screen_size = toXM(screen_size);
	}

	// Viewport
	{
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)surface_width;
		viewport.Height = (float)surface_height;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
	}
}
//#pragma warning(4701 : default)
